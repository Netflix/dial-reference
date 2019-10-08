/*
 * Copyright (c) 2014-2019 Netflix, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NETFLIX, INC. AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NETFLIX OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "../dial_data.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "test.h"
#include "test_dial_data.h"

/*
 * All tests should malloc
 */

int key_value_pairs = 3;
char *keys[] = {"key3", "key2", "key3"};
char *values[] = {"value1", "value2", "value3"};

void test_read_dial_missing_data() {
    DIALData *data = retrieve_dial_data("NON_EXISTENT_DATA");
    EXPECT(NULL == data, "retrieve_dial_data() returns NULL\n");
    DONE();
}

void test_write_dial_data() {
    DIALData *result = NULL;
    for (int i = 0; i < key_value_pairs; ++i) {
        DIALData *node = (DIALData *) malloc(sizeof(DIALData));
        node->key = malloc(strlen(keys[i]) + 1);
        node->value = malloc(strlen(values[i]) + 1);
        strcpy(node->key, keys[i]);
        strcpy(node->value, values[i]);

        node->next = result;
        result = node;
    }
    store_dial_data("YouTube", result);

    DIALData *readBack = retrieve_dial_data("YouTube");
    DIALData *datum = readBack;
    for (int i = 0; datum != NULL; datum = datum->next, i++) {
        EXPECT_STREQ(datum->key, keys[i]);
        EXPECT_STREQ(datum->value, values[i]);
    }

    free_dial_data(&result);
    free_dial_data(&readBack);

    DONE();
}

void test_write_kv_larger_than_max_len() {
    // result contains k & v both larger than DIAL_KEY_OR_VALUE_MAX_LEN
    DIALData *result = (DIALData *) calloc(1, sizeof(DIALData));
    result->key = calloc(DIAL_KEY_OR_VALUE_MAX_LEN * 2, sizeof(char));
    result->value = calloc(DIAL_KEY_OR_VALUE_MAX_LEN * 2, sizeof(char));
    memset(result->key, 'k', DIAL_KEY_OR_VALUE_MAX_LEN * 2 - 1);
    memset(result->value, 'v', DIAL_KEY_OR_VALUE_MAX_LEN * 2 - 1);

    store_dial_data("YouTube", result);

    DIALData *readBack = retrieve_dial_data("YouTube");
    EXPECT(NULL != readBack, "retrieve_dial_data should not be NULL\n");

    EXPECT_EQ(readBack->key[0], 'k');
    EXPECT_EQ(readBack->value[0], 'v');
    EXPECT_EQ(strlen(readBack->key), DIAL_KEY_OR_VALUE_MAX_LEN);
    EXPECT_EQ(strlen(readBack->value), DIAL_KEY_OR_VALUE_MAX_LEN);

    free_dial_data(&result);
    free_dial_data(&readBack);

    DONE();
}

void test_write_empty_kv() {
    DIALData *result = (DIALData *) calloc(1, sizeof(DIALData));
    result->key = calloc(1, sizeof(char));
    result->value = calloc(1, sizeof(char));

    store_dial_data("YouTube", result);

    DIALData *readBack = retrieve_dial_data("YouTube");
    EXPECT(NULL != readBack, "retrieve_dial_data should not be NULL\n");

    EXPECT_EQ(readBack->key[0], '\0');
    EXPECT_EQ(readBack->value[0], '\0');

    free_dial_data(&result);
    free_dial_data(&readBack);

    DONE();
}

void test_dial_data_suite() {
    START_SUITE();

    test_read_dial_missing_data();
    test_write_dial_data();
    test_write_kv_larger_than_max_len();
}