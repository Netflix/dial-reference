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
#include "url_lib.h"
#include "dial_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static const char * unknown_str = "unknown";

char* smartstrncpy(char* dest, char* src, size_t max_chars) {
    size_t copied;
    for (copied = 0; copied < max_chars; copied++, dest++, src++) {
        *dest = *src;
        if (*dest == 0)
            break;
    }
    *dest = 0;
    return dest;
}

/**
 * Convert a two-character hex representation into its ASCII equivalent
 * and copy it into dest.
 *
 * @param dest the destination character buffer.
 * @param a the first hex character.
 * @param b the second hex character.
 * @return 1 if the character was converted and copied, 0 if the hex
 *         characters are invalid.
 */
static int append_char_from_hex(char* dest, char a, char b) {
    if ('a' <= a && a <= 'f')
        a = 10 + a - 'a';
    else if ('A' <= a && a <= 'F')
        a = 10 + a - 'A';
    else if ('0' <= a && a <= '9')
        a = a - '0';
    else
        return 0;

    if ('a' <= b && b <= 'f')
        b = 10 + b - 'a';
    else if ('A' <= b && b <= 'F')
        b = 10 + b - 'A';
    else if ('0' <= b && b <= '9')
        b = b - '0';
    else
        return 0;

    *dest = (char) (16 * a) + b;
    return 1;
}

int urldecode(char *dst, const char *src, size_t max_size) {
    size_t len = 0;

    while (len < max_size && *src) {
        if (*src == '+') {
            *dst = ' ';
        } else if (*src == '%') {
            if (!*(++src) || !*(++src) || !append_char_from_hex(dst, *(src - 1), *src)) {
                *dst = '\0';
                return 0;
            }
        } else {
            *dst = *src;
        }
        ++dst;
        ++src;
        ++len;
    }
    *dst = '\0';
    return len;
}

void xmlencode(char *dst, const char *src, size_t max_size) {
    size_t current_size = 0;
    while (*src && current_size < max_size) {
        switch (*src) {
            case '&':
                if (current_size + 5 >= max_size)
                    break;
                dst = smartstrncpy(dst, "&amp;", max_size - current_size);
                current_size += 5;
                break;
            case '\"':
                if (current_size + 6 >= max_size)
                    break;
                dst = smartstrncpy(dst, "&quot;", max_size - current_size);
                current_size += 6;
                break;
            case '\'':
                if (current_size + 6 >= max_size)
                    break;
                dst = smartstrncpy(dst, "&apos;", max_size - current_size);
                current_size += 6;
                break;
            case '<':
                if (current_size + 4 >= max_size)
                    break;
                dst = smartstrncpy(dst, "&lt;", max_size - current_size);
                current_size += 4;
                break;
            case '>':
                if (current_size + 4 >= max_size)
                    break;
                dst = smartstrncpy(dst, "&gt;", max_size - current_size);
                current_size += 4;
                break;
            default:
                *dst++ = *src;
                current_size++;
                break;
        }
        src++;
    }
    *dst = '\0';
}

char *parse_app_name(const char *uri) {
    char *unknown = NULL;
    if (uri == NULL) {
        unknown = (char*)calloc(strlen(unknown_str) + 1, sizeof(char));
        if (unknown == NULL) {
            return NULL;
        }
        strncpy(unknown, unknown_str, strlen(unknown_str) + 1);
        return unknown;
    }
    char *slash = strrchr(uri, '/');
    if (slash == NULL || slash == uri) {
        unknown = (char*)calloc(strlen(unknown_str) + 1, sizeof(char));
        if (unknown == NULL) {
            return NULL;
        }
        strncpy(unknown, unknown_str, strlen(unknown_str) + 1);
        return unknown;
    }
    char *begin = slash;
    while ((begin != uri) && (*--begin != '/'))
        ;
    if (*begin == '/') {
        begin++;  // skip the slash
    }
    char *result = (char *) calloc(slash - begin+1, sizeof(char));
    if (result == NULL) {
        return NULL;
    }
    strncpy(result, begin, slash - begin);
    result[slash-begin]='\0';
    return result;
}

char *parse_param(char *query_string, char *param_name) {
    if (query_string == NULL) {
        return NULL;
    }
    char * start;
    if ((start = strstr(query_string, param_name)) == NULL) {
        return NULL;
    }
    while (*start && (*start++ != '='))
        ;
    char *end = start;
    while (*end && (*end != '&'))
        end++;
    int result_size = end - start;
    char *result = malloc(result_size + 1);
    if (result == NULL) {
        return NULL;
    }
    result[0] = '\0';
    strncpy(result, start, result_size);
    result[result_size] = '\0';
    return result;
}

DIALData *parse_params(char * query_string) {
    if (query_string == NULL || strlen(query_string) <= 2) {
        return NULL;
    }
    if (query_string[0] == '?') {
        query_string++;  // skip leading question mark
    }
    DIALData *result = NULL;
    int err = 0;
    char *query_string_dup = strdup(query_string);
    char * name_value = strtok(query_string_dup, "&");
    while (name_value != NULL) {
        DIALData *tmp = (DIALData *) malloc(sizeof(DIALData));
        if (tmp == NULL) {
            err = 1;
            break;
        }
        size_t name_value_length = strlen(name_value);
        tmp->key = (char *) calloc(name_value_length + 1, sizeof(char));
        if (tmp->key == NULL) {
            free(tmp); tmp = NULL;
            err = 1;
            break;
        }
        tmp->value = (char *) calloc(name_value_length + 1, sizeof(char));
        if (tmp->value == NULL) {
            free(tmp->key); tmp->key = NULL;
            free(tmp); tmp = NULL;
            err = 1;
            break;
        }
        int match = sscanf(name_value, "%[^=]=%s", tmp->key, tmp->value);
        if (match != 2) {
            free(tmp->value); tmp->value = NULL;
            free(tmp->key); tmp->key = NULL;
            free(tmp); tmp = NULL;
            err = 1;
            break;
        }
        tmp->next = result;
        result = tmp;

        name_value = strtok(NULL, "&");  // read next token
    }
    free(query_string_dup);
    if (err) {
        free_dial_data(&result); result = NULL;
    }
    return result;
}

/* The URL encoding source code was obtained here:
 * http://www.geekhideout.com/urlcode.shtml
 */

/* Converts a hex character to its integer value */
char from_hex(char ch) {
    return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
    static char hex[] = "0123456789abcdef";
    return hex[code & 15];
}


/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(const char *str) {
    const char *pstr;
    char *buf, *pbuf;
    pstr = str;
    buf = malloc(strlen(str) * 3 + 1);
    pbuf = buf;
    if( buf )
    {
        while (*pstr) {
            if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
                *pbuf++ = *pstr;
            else if (*pstr == ' ')
                *pbuf++ = '+';
            else
                *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
            pstr++;
        }
        *pbuf = '\0';
    }
    return buf;
}

/*
 * End of URL ENCODE source
 */
