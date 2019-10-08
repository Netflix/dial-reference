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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <regex.h>
#include "dial_server.h"
#include "url_lib.h"
#include "nf_callbacks.h"

extern char *spAppNetflix;
extern char spNetflix[];
static char *defaultLaunchParam = "source_type=12";

// Adding 40 bytes for defaultLaunchParam, plus additional characters that
// may get added.
//
// dial_server.c ensures the payload is <= DIAL_MAX_PAYLOAD but since we
// URL-encode it, the total string length may triple.
//
// dial_server.c ensures the additional data URL is <= DIAL_MAX_ADDITIONALURL.
static char sQueryParam[3 * DIAL_MAX_PAYLOAD + DIAL_MAX_ADDITIONALURL + 40];

int isAppRunning( char *pzName, char *pzCommandPattern );
int shouldRelaunch(DIALServer *pServer, const char *pAppName, const char *args );
pid_t runApplication( const char * const args[], DIAL_run_t *run_id );

DIALStatus netflix_start(DIALServer *ds, const char *appname,
                                const char *payload, const char* query_string,
                                const char *additionalDataUrl,
                                DIAL_run_t *run_id, void *callback_data) {
    int shouldRelaunchApp = 0;
    int appPid = 0;

    // only launch Netflix if it isn't running
    appPid = isAppRunning( spAppNetflix, NULL );
    shouldRelaunchApp = shouldRelaunch( ds, appname, payload );

    // construct the payload to determine if it has changed from the previous launch
    memset( sQueryParam, 0, sizeof(sQueryParam) );
    strncpy( sQueryParam, defaultLaunchParam, sizeof(sQueryParam) - 1);
        if(strlen(payload))
    {
        char * pUrlEncodedParams;
        pUrlEncodedParams = url_encode( payload );
        if( pUrlEncodedParams ){
            if (strlen(sQueryParam) + sizeof("&dial=") + strlen(pUrlEncodedParams) < sizeof(sQueryParam)) {
                strcat( sQueryParam, "&dial=" );
                strcat( sQueryParam, pUrlEncodedParams );
                free( pUrlEncodedParams );
            } else {
                free( pUrlEncodedParams );
                return kDIALStatusError;
            }

        }
    }

    if(strlen(additionalDataUrl)){
        if (strlen(sQueryParam) + sizeof("&") + strlen(additionalDataUrl) < sizeof(sQueryParam)) {
            strcat(sQueryParam, "&");
            strcat(sQueryParam, additionalDataUrl);
        } else {
            return kDIALStatusError;
        }
    }

    printf("appPid = %s, shouldRelaunch = %s queryParams = %s\n",
          appPid?"TRUE":"FALSE",
          shouldRelaunchApp?"TRUE":"FALSE",
          sQueryParam );

    // if its not running, launch it.  The Netflix application should
    // never be relaunched
    if( !appPid ){
        const char * const netflix_args[] = {spNetflix, "-Q", sQueryParam, 0};
        return runApplication( netflix_args, run_id );
    }
    else return kDIALStatusRunning;
}

DIALStatus netflix_hide(DIALServer *ds, const char *app_name,
                               DIAL_run_t *run_id, void *callback_data)
{
    return (isAppRunning( spAppNetflix, NULL )) ? kDIALStatusRunning : kDIALStatusStopped;
}

DIALStatus netflix_status(DIALServer *ds, const char *appname,
                                 DIAL_run_t run_id, int* pCanStop, void *callback_data) {
    // Netflix application can stop
    *pCanStop = 1;
    
    waitpid((pid_t)(long)run_id, NULL, WNOHANG); // reap child
    
    return isAppRunning( spAppNetflix, NULL ) ? kDIALStatusRunning : kDIALStatusStopped;
}

void netflix_stop(DIALServer *ds, const char *appname, DIAL_run_t run_id,
                         void *callback_data) {
    int pid;
    pid = isAppRunning( spAppNetflix, NULL );
    if( pid ){
            printf("Killing pid %d\n", pid);
            kill((pid_t)pid, SIGTERM);
            waitpid((pid_t)pid, NULL, 0); // reap child
        }
}


