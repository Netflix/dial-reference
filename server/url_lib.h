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
/* Utility functions for dealing with URLs */

#ifndef URLLIB_H_
#define URLLIB_H_

#include "dial_data.h"
#include <stddef.h>

/**
 * Copy a maximum of max_chars characters from src into dest,
 * and return a pointer to the terminating NULL in dest.
 *
 * @param dest destination buffer.
 * @param src string written into the destination buffer
 * @param max_chars maximum number of characters to put into the destination
 *        buffer, excluding the trailing NULL. Must be less than or equal to
 *        the number of bytes available at dest - 1.
 * @return a pointer to the end of the written string (the location of the
 *         terminating NULL).
 */
char* smartstrncpy(char* dest, char* src, size_t max_chars);

/**
 * URL-unescape the source string into the destination string, up to the
 * maximum number of destination characters.
 *
 * @param dst raw string buffer.
 * @param src URL-escaped source string.
 * @param max_size maximum number of characters to put into the destination
 *        buffer, excluding the trailing NULL. Must be less than or equal to
 *        sizeof(dst) - 1.
 * @return the length of the raw string excluding the trailing NULL. Will be
 *         0 if there were no characters to unescape or if the source string
 *         was malformed.
 */
int urldecode(char *dst, const char *src, size_t max_size);

/**
 * XML-escape the source string into the destination string, up to the maximum
 * number of destination characters.
 *
 * @param dst XML-escaped string buffer.
 * @param src raw source string.
 * @param max_size maximum number of characters to put into the destination
 *        buffer, excluding the trailing NULL. Must be less than or equal to
 *        sizeof(dst) - 1.
 * @return the length of the XML-escaped string excluding the trailing NULL.
 */
void xmlencode(char *dst, const char *src, size_t max_size);

/**
 * Return the value in the query string for the requested parameter name.
 *
 * @param query_string the URL query string.
 * @param param_name the parameter name.
 * @return the parameter value or NULL if out-of-memory. The caller must free
 *         the returned memory.
 */
char *parse_param(char *query_string, char *param_name);

/**
 * Parse the application name out of the full URI, for example
 * /app/YouTube/dial_data. The application name identified as the string
 * appearing before the last trailing slash, possibly prefixed with a slash,
 * so /apps/<app_name>/dial_data and YouTube would be the application name in
 * the previous example.
 *
 * If your DIAL server uses a different path format you will need to change
 * this method to match.
 *
 * @param uri the URI, there must be a trailing slash.
 * @return the application name, or "unknown" if two slashes cannot be found
 *         or the application name is zero-length, or NULL if out-of-memory.
 *         The caller must free the returned memory.
 */
char *parse_app_name(const char *uri);

/**
 * Return a linked list of DIAL data constructed from the name/value parameter
 * pairs of the provided query string.
 *
 * This function must be called while holding a mutex and is not itself
 * thread-safe.
 *
 * @param query_string the URL query string.
 * @return the DIAL data or NULL if there is none (e.g. parse error) or out-of-
 *         memory. The caller must free the returned memory.
 */
DIALData *parse_params(char * query_string);

/**
 * Return the URL-escaped version of the provided string, which may be as
 * large as 3x the size of the provided string, plus a trailing NULL byte.
 *
 * @return the URL-escaped version of the provided string. The caller must
 *         free the returned string.
 */
char *url_encode(const char *str);

#endif  // URLLIB_H_
