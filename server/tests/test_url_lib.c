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
#include "../url_lib.h"
#include "../dial_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "test.h"

void test_smartstrncpy() {
    char* src1 = "Hello ";
    char* src2 = "world!";
    char* src3 = " Trunc ated";
    char dest[128] = {0, };

    char* p = (char *) dest;
    p = smartstrncpy(p, src1, 128);
    EXPECT_STREQ(dest, "Hello ");
    p = smartstrncpy(p, src2, dest + 128 - p);

    EXPECT_STREQ(dest, "Hello world!");

    p = smartstrncpy(p, src3, 6);
    EXPECT_STREQ(dest, "Hello world! Trunc");

    DONE();
}

void test_urldecode() {
    char* param = "%26bla+r";
    char dest[128] = {0, };

    EXPECT(urldecode(dest, param, 128), "Failed to decode.");
    EXPECT_STREQ(dest, "&bla r");
    DONE();
}

void test_parse_app_name() {
    char *app_name;
    EXPECT((app_name = parse_app_name(NULL)), "Failed to extract app_name");
    EXPECT_STREQ(app_name, "unknown");
    free(app_name);
    EXPECT((app_name = parse_app_name("")), "Failed to extract app_name");
    EXPECT_STREQ(app_name, "unknown");
    free(app_name);
    EXPECT((app_name = parse_app_name("/")), "Failed to extract app_name");
    EXPECT_STREQ(app_name, "unknown");
    free(app_name);
    EXPECT((app_name = parse_app_name("/apps/YouTube/DialData")),
           "Failed to extract app_name");
    EXPECT_STREQ(app_name, "YouTube");
    free(app_name);
    EXPECT((app_name = parse_app_name("//")), "Failed to extract app_name");
    EXPECT_STREQ(app_name, "");
    free(app_name);
    EXPECT((app_name = parse_app_name("/invalid")),
           "Failed to extract app_name");
    EXPECT_STREQ(app_name, "unknown");
    free(app_name);
    DONE();
}

void test_parse_params() {
    EXPECT(!parse_params(""), "Empty query string should generate no params");
    EXPECT(!parse_params(NULL), "Null query, should generate no params");

    DIALData *result = parse_params("a=b");
    EXPECT_STREQ(result->key, "a");
    EXPECT_STREQ(result->value, "b");
    free_dial_data(&result);

    result = parse_params("?a=b");
    EXPECT_STREQ(result->key, "a");
    EXPECT_STREQ(result->value, "b");
    free_dial_data(&result);

    result = parse_params("?a=b&c=d");
    EXPECT_STREQ(result->key, "c");
    EXPECT_STREQ(result->value, "d");
    EXPECT_STREQ(result->next->key, "a");
    EXPECT_STREQ(result->next->value, "b");
    free_dial_data(&result);

    result = parse_params("ሳ=€");
    EXPECT_STREQ(result->key, "ሳ");
    EXPECT_STREQ(result->value, "€");
    free_dial_data(&result);

    char query_string[1024] = {0, };
    char *current = query_string;
    for (int i = 0; i < 25; ++i) {
        current = smartstrncpy(current, "a=b&", 256);
    }
    result = parse_params(query_string);
    int length = 0;
    for (DIALData *current = result; current != NULL; current = current->next) {
        length++;
    }
    EXPECT((length == 25), "25 params should have been parsed");
    free_dial_data(&result);

    DONE();
}

void test_parse_params_malformatted() {
    EXPECT(NULL == parse_params("abcdefghijkl"), "no params expected");
    EXPECT(NULL == parse_params("\u2639"), "no params expected");
    DONE();
}

void test_url_lib_suite() {
    START_SUITE();
    test_smartstrncpy();
    test_urldecode();
    test_parse_app_name();
    test_parse_params();
    test_parse_params_malformatted();
}