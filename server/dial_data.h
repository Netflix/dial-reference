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
 * Defines functions for persisting and retrieving DIAL data.
 */

#ifndef SRC_SERVER_DIAL_DATA_H_
#define SRC_SERVER_DIAL_DATA_H_

/*
 * Slash-terminated directory of where to persist the DIAL data.
 */
#define DIAL_DATA_DIR "/tmp/"

/*
 * The maximum DIAL data payload accepted per the 'DIAL extension for smooth
 * pairing' specification'.
 */
#define DIAL_DATA_MAX_PAYLOAD (4096) /* 4 KB */

/*
 * Url path where DIAL data should be posted according to the 'DIAL extension
 * for smooth pairing' specification.
 */
#define DIAL_DATA_URI "/dial_data"

/**
 * The DIAL data key and value values cannot contain any spaces. They are
 * expected to be URL-escaped strings, so any spaces would be represented as
 * the '+' character. They have a max length of 255 characters.
 *
 * THE STRINGS key AND value POINT TO MUST BE DYNAMICALLY ALLOCATED
 */
struct DIALData_ {
    struct DIALData_ *next;
    char *key;
    char *value;
};

typedef struct DIALData_ DIALData;

#define DIAL_KEY_OR_VALUE_MAX_LEN (255)
#define DIAL_KEY_OR_VALUE_MAX_LEN_STR "255"

/**
 * Store the DIAL data key/value pairs in the application data store.
 *
 * Will exit immediately if the data output file cannot be accessed due to
 * out-of-memory or I/O errors.
 *
 * Keys and values are truncated to DIAL_KEY_OR_VALUE_MAX_LEN.
 *
 * @param app_name application name.
 * @param data pointer to head of DIAL data linked list.
 */
void store_dial_data(char *app_name, DIALData *data);

/**
 * Retrieve the DIAL data key/value pairs from the application data store.
 *
 * @param app_name application name.
 * @return data pointer to head of DIAL data linked list or NULL if
 *         there is no valid data or if the data output file cannot be accessed
 *         due to out-of-memory or I/O errors.
 */
DIALData *retrieve_dial_data(char *app_name);

/**
 * Set the DIAL data directory.
 *
 * @param data_dir the DIAL data directory path, which must include the
 *        trailing directory separator (e.g. '/' character).
 */
void set_dial_data_dir(const char *data_dir);

/**
 * Frees the DIAL data linked list memory.
 *
 * @param dialData pointer to the DIAL data linked list.
 */
void free_dial_data(DIALData **dialData);

#endif /* SRC_SERVER_DIAL_DATA_H_ */
