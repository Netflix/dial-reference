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
 */
static char* getAppPath(char *app_name) {
    size_t name_size = strlen(app_name) + sizeof(dial_data_dir) + 1;
    char* filename = (char*) malloc(name_size);
    filename[0] = 0;
    strncat(filename, dial_data_dir, name_size);
    strncat(filename, app_name, name_size - sizeof(dial_data_dir));
    return filename;
}

void store_dial_data(char app_name[], DIALData *data) {
    char* filename = getAppPath(app_name);
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        printf("Cannot open DIAL data output file: %s\n", filename);
        exit(1);
    }
    free(filename);
    for (DIALData *first = data; first != NULL; first = first->next) {
        fprintf(f, "%s %s\n", first->key, first->value);
    }
    fclose(f);
}

DIALData *retrieve_dial_data(char *app_name) {
    char* filename = getAppPath(app_name);
    FILE *f = fopen(filename, "r");
    free(filename);
    if (f == NULL) {
        return NULL; // no dial data found, that's fine
    }
    DIALData *result = NULL;
    char key[256];
    char value[256];
    while (fscanf(f, "%255s %255s\n", key, value) != EOF) {
        DIALData *newNode = (DIALData *) malloc(sizeof(DIALData));
        newNode->key = (char *) calloc(1, strlen(key));
        strncpy(newNode->key, key, strlen(key));
        newNode->value = (char *) calloc(1, strlen(value));
        strncpy(newNode->value, value, strlen(value));
        newNode->next = result;
        result = newNode;
    }
    fclose(f);
    return result;
}

void free_dial_data(DIALData **dialData)
{
    DIALData *curNode=NULL;
    while (*dialData != NULL) {
        curNode = *dialData;
        *dialData =curNode->next;

        free(curNode->key);
        free(curNode->value);
        free(curNode);
    }
}
