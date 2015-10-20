/*
 * Copyright (c) 2014 Netflix, Inc.
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

int key_value_pairs = 3;
char *keys[] = {"key1", "key2", "key3"};
char *values[] = {"value1", "value2", "value3"};

void test_read_dial_data() {
    DIALData *data = retrieve_dial_data("dial_data");
    for (int i = 0; data != NULL; data = data->next, i++) {
        EXPECT_STREQ(data->key, keys[2 - i]);
        EXPECT_STREQ(data->value, values[2 - i]);
    }
    DONE();
}

void test_write_dial_data() {
    DIALData *result = NULL;
    for (int i = 0; i < key_value_pairs; ++i) {
        DIALData *node = (DIALData *) malloc(sizeof(DIALData));
        node->key = keys[i];
        node->value = values[i];
        node->next = result;
        result = node;
    }
    store_dial_data("YouTube", result);

    DIALData *readBack = retrieve_dial_data("YouTube");

    for (int i = 0; readBack != NULL; readBack = readBack->next, i++) {
        EXPECT_STREQ(readBack->key, keys[i]);
        EXPECT_STREQ(readBack->value, values[i]);
    }

    DONE();
}
