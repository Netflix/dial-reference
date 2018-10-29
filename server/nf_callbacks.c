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

// Adding 20 bytes for prepended source_type for Netflix
static char sQueryParam[DIAL_MAX_PAYLOAD+DIAL_MAX_ADDITIONALURL+40];

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
    strcat( sQueryParam, defaultLaunchParam );
    if(strlen(payload))
    {
        char * pUrlEncodedParams;
        pUrlEncodedParams = url_encode( payload );
        if( pUrlEncodedParams ){
            strcat( sQueryParam, "&dial=");
            strcat( sQueryParam, pUrlEncodedParams );
            free( pUrlEncodedParams );
        }
    }

    if(strlen(additionalDataUrl)){
        strcat(sQueryParam, "&");
        strcat(sQueryParam, additionalDataUrl);
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


