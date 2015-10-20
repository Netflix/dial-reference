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
// Macros to simplify testing functions.

#ifndef SRC_SERVER_TESTS_TEST_H_
#define SRC_SERVER_TESTS_TEST_H_

#define EXPECT(a, m) \
  do { \
    if (!a) { \
      printf("[%s] failed: %s\n", #a, m); \
      printf("%s -> FAILED\n", __func__); \
      return; \
    } \
  } while (0)

#define EXPECT_STREQ(a, b) \
  do { \
    if (strcmp(a, b)) { \
      printf("expected [%s == %s]\n", #a, #b); \
      printf(" a = \"%s\"\n", a); \
      printf(" b = \"%s\"\n", b); \
      printf("%s -> FAILED\n", __func__); \
      return; \
    } \
  } while (0)

#define DONE() \
  printf("%s -> OK\n", __func__)

#endif /* SRC_SERVER_TESTS_TEST_H_ */
