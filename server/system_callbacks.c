/*
 * Copyright (c) 2018-2019 Netflix, Inc.
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
#include <string.h>
#include <stdio.h>
#include "system_callbacks.h"

extern char spSleepPassword[];

DIALStatus system_start(DIALServer *ds, const char *appname, const char *payload, const char* query_string,
                        const char *additionalDataUrl, DIAL_run_t *run_id, void *callback_data) {

    /* Can't launch system app */
    if (0 == query_string) {
        return kDIALStatusErrorForbidden;
    }

    /* Only sleep is supported action */
    if (0 != strncmp( query_string, "action=sleep", sizeof("action=sleep") - 1)) {
        return kDIALStatusErrorNotImplemented;   // Only "sleep" is valid action
    }

    if (strlen(spSleepPassword) != 0) {

        /* Look for key */
        char *key_value = strchr(query_string, '&');
        if ( key_value == NULL || *key_value == '\0' ) {
            return kDIALStatusErrorForbidden;   // No key specified.
        }

        /* Look for sleep password */
        *key_value++ = '\0';
        char str[512];
        snprintf(str, 512, "key=%s", spSleepPassword);
        printf(" str: %s \n", str);
        if (0 != strncmp( key_value, "key=TEST", sizeof("key=TEST") - 1)) {
            return kDIALStatusErrorUnauth;  // Invalid key
        }
    }
    /* Sleep not implemented in reference implementation */
    return kDIALStatusErrorNotImplemented;
}

DIALStatus system_hide(DIALServer *ds, const char *app_name,
                       DIAL_run_t *run_id, void *callback_data) {
    // Always hidden
    return kDIALStatusHide;
}

DIALStatus system_status(DIALServer *ds, const char *appname,
                         DIAL_run_t run_id, int* pCanStop, void *callback_data) {
    // Always hidden
    return kDIALStatusHide;
}


