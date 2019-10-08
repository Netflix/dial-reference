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
    if (!(a)) { \
      printf("\033[31m  [%s] failed: %s  \033[0m\n", #a, m); \
      printf("\033[31m  %s -> FAILED  \033[0m\n", __func__); \
      return; \
    } \
  } while (0)

#define EXPECT_EQ(a, b) \
  do { \
    if (a != b) { \
      printf("\033[31m  expected [%s == %s] \033[0m\n", #a, #b); \
      printf("\033[31m  a = \"%ld\" \033[0m\n", (unsigned long) a); \
      printf("\033[31m  b = \"%ld\" \033[0m\n", (unsigned long) b); \
      printf("\033[31m  %s -> FAILED \033[0m\n", __func__); \
      return; \
    } \
  } while (0)

#define EXPECT_STREQ(a, b) \
  do { \
    if (strcmp(a, b)) { \
      printf("\033[31m  expected [%s == %s] \033[0m\n", #a, #b); \
      printf("\033[31m  a = \"%s\" \033[0m\n", a); \
      printf("\033[31m  b = \"%s\" \033[0m\n", b); \
      printf("\033[31m  %s -> FAILED \033[0m\n", __func__); \
      return; \
    } \
  } while (0)

#define START_SUITE() \
  printf("== %s ==\n", __FILE__)

#define DONE() \
  printf("\033[32m  %s -> OK \033[0m\n", __func__)

#endif /* SRC_SERVER_TESTS_TEST_H_ */
