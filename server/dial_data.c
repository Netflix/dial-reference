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
/*
 * Functions related to storing/retrieving and manipulating DIAL data.
 */
#include "dial_data.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


char dial_data_dir[256] = DIAL_DATA_DIR;

void set_dial_data_dir(const char *data_dir) {
    strncpy(dial_data_dir, data_dir, 255);
}

/**
 * Returns the path where data is stored for the given app.
 *
 * The DIAL data directory must have been already set.
 *
 * @param app_name application name.
 * @return the location of the application path within the DIAL data
 *         directory or NULL if memory could not be allocated.
 * @see set_dial_data_dir(const char*)
 */
static char* getAppPath(char *app_name) {
    size_t name_size = strlen(app_name) + sizeof(dial_data_dir) + 1;
    char* filename = (char*) malloc(name_size);
    if (filename == NULL) {
        return NULL;
    }
    filename[0] = 0;
    strncat(filename, dial_data_dir, name_size);
    strncat(filename, app_name, name_size - sizeof(dial_data_dir));
    return filename;
}

void store_dial_data(char *app_name, DIALData *data) {
    char* filename = getAppPath(app_name);
    if (filename == NULL) {
        printf("Cannot open DIAL data output file, out-of-memory.");
        exit(1);
    }
    FILE *f = fopen(filename, "w");
    free(filename); filename = NULL;
    if (f == NULL) {
        printf("Cannot open DIAL data output file: %s\n", filename);
        exit(1);
    }
    for (DIALData *first = data; first != NULL; first = first->next) {
        // truncate because we have limits on length when retrieving.
        fprintf(f, "%.*s %.*s\n", DIAL_KEY_OR_VALUE_MAX_LEN, first->key, DIAL_KEY_OR_VALUE_MAX_LEN, first->value);
    }
    fclose(f);
}

DIALData *retrieve_dial_data(char *app_name) {
    char* filename = getAppPath(app_name);
    if (filename == NULL) {
        return NULL; // no dial data found, that's fine
    }
    FILE *f = fopen(filename, "r");
    free(filename); filename = NULL;
    if (f == NULL) {
        return NULL; // no dial data found, that's fine
    }
    DIALData *result = NULL;
    char key[DIAL_KEY_OR_VALUE_MAX_LEN + 1] = {0,};
    char value[DIAL_KEY_OR_VALUE_MAX_LEN + 1] = {0,};
    int err = 0;
    while (fscanf(f, "%" DIAL_KEY_OR_VALUE_MAX_LEN_STR "s %" DIAL_KEY_OR_VALUE_MAX_LEN_STR "s\n", key, value) != EOF) {
        DIALData *newNode = (DIALData *) malloc(sizeof(DIALData));
        if (newNode == NULL) {
            err = 1;
            break;
        }

        newNode->key = (char *) calloc(strlen(key) + 1, sizeof(char));
        if (newNode->key == NULL) {
            err = 1;
            free(newNode); newNode = NULL;
            break;
        }
        strncpy(newNode->key, key, strlen(key));

        newNode->value = (char *) calloc(strlen(value) + 1, sizeof(char));
        if (newNode->value == NULL) {
            err = 1;
            free(newNode->key); newNode->key = NULL;
            free(newNode); newNode = NULL;
            break;
        }
        strncpy(newNode->value, value, strlen(value));
        newNode->next = result;
        result = newNode;
    }
    fclose(f);
    if (err) {
        free_dial_data(&result);
        result = NULL;
    }
    return result;
}

void free_dial_data(DIALData **dialData)
{
    DIALData *curNode=NULL;
    while (*dialData != NULL) {
        curNode = *dialData;
        *dialData = curNode->next;

        free(curNode->key); curNode->key = NULL;
        free(curNode->value); curNode->value = NULL;
        free(curNode); curNode = NULL;
    }
}
