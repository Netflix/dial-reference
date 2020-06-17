/*
 * Copyright (c) 2014-2020 Netflix, Inc.
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

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <regex.h>

#include "dial_server.h"
#include "dial_options.h"
#include <signal.h>
#include <stdbool.h>

#include "url_lib.h"
#include "nf_callbacks.h"
#include "system_callbacks.h"

#define BUFSIZE 256

char *spAppNetflix = "netflix";      // name of the netflix executable
static char *spDefaultNetflix = "../../../src/platform/qt/netflix";
static char *spDefaultData="../../../src/platform/qt/data";
static char *spNfDataDir = "NF_DATA_DIR=";
static char *spDefaultFriendlyName = "DIAL server sample";
static char *spDefaultModelName = "NOT A VALID MODEL NAME";
static char *spDefaultUuid = "deadbeef-dead-beef-dead-beefdeadbeef";
static char spDataDir[BUFSIZE];
char spNetflix[BUFSIZE];
static char spFriendlyName[BUFSIZE];
static char spModelName[BUFSIZE];
static char spUuid[BUFSIZE];
extern bool wakeOnWifiLan;
static int gDialPort;

char spSleepPassword[BUFSIZE];

static char *spAppYouTube = "chrome";
static char *spAppYouTubeMatch = "chrome.*google-chrome-dial";
static char *spAppYouTubeExecutable = "/opt/google/chrome/google-chrome";
static char *spYouTubePS3UserAgent = "--user-agent="
    "Mozilla/5.0 (PS3; Leanback Shell) AppleWebKit/535.22 (KHTML, like Gecko) "
    "Chrome/19.0.1048.0 LeanbackShell/01.00.01.73 QA Safari/535.22 Sony PS3/ "
    "(PS3, , no, CH)";

int doesMatch( char* pzExp, char* pzStr)
{
    regex_t exp;
    int ret;
    int match = 0;
    if ((ret = regcomp( &exp, pzExp, REG_EXTENDED ))) {
        char errbuf[1024] = {0,};
        regerror(ret, &exp, errbuf, sizeof(errbuf));
        fprintf( stderr, "regexp error: %s", errbuf );
    } else {
        regmatch_t matches[1];
        if( regexec( &exp, pzStr, 1, matches, 0 ) == 0 ) {
            match = 1;
        }
    }
    regfree(&exp);
    return match;
}

void signalHandler(int signal)
{
    switch(signal)
    {
        case SIGTERM:
            // just ignore this, we don't want to die
            break;
    }
}

/*
 * This function will walk /proc and look for the application in
 * /proc/<PID>/comm. and /proc/<PID>/cmdline to find it's command (executable
 * name) and command line (if needed).
 * Implementors can override this function with an equivalent.
 */
int isAppRunning( char *pzName, char *pzCommandPattern ) {
  DIR* proc_fd = opendir("/proc");
  if( proc_fd != NULL ) {
    struct dirent* procEntry;
    while((procEntry=readdir(proc_fd)) != NULL) {
      if( doesMatch( "^[0-9][0-9]*$", procEntry->d_name ) ) {
        char exePath[384] = {0,};
        char link[256] = {0,};
        char cmdlinePath[384] = {0,};
        char buffer[1024] = {0,};
        int len;
        snprintf( exePath, sizeof(exePath), "/proc/%s/exe", procEntry->d_name);
        snprintf( cmdlinePath, sizeof(cmdlinePath), "/proc/%s/cmdline", procEntry->d_name);

        if( (len = readlink( exePath, link, sizeof(link)-1)) != -1 ) {
          char executable[256] = {0,};
          strncpy( executable, pzName, sizeof(executable) - 2 );
          strcat( executable, "$" );
          // TODO: Make this search for EOL to prevent false positives
          if( !doesMatch( executable, link ) ) {
            continue;
          }
          // else //fall through, we found it
        }
        else continue;

        if (pzCommandPattern != NULL) {
          FILE *cmdline = fopen(cmdlinePath, "r");
          if (!cmdline) {
            continue;
          }
          if (fgets(buffer, 1024, cmdline) == NULL) {
            fclose(cmdline);
            continue;
          }
          fclose(cmdline);

          if (!doesMatch( pzCommandPattern, buffer )) {
            continue;
          }
        }
        int d_name = atoi(procEntry->d_name);
        closedir(proc_fd);
        return d_name;
      }
    }

    closedir(proc_fd);
  } else {
    printf("/proc failed to open\n");
  }
  return 0;
}

pid_t runApplication( const char * const args[], DIAL_run_t *run_id ) {
  pid_t pid = fork();
  if (pid != -1) {
    if (!pid) { // child
      putenv(spDataDir);
      printf("Execute:\n");
      for(int i = 0; args[i]; ++i) {
        printf(" %d) %s\n", i, args[i]);
      }
      if( execv(*args, (char * const *) args) == -1) {
		printf("%s failed to launch\n", *args);
		perror("Failed to Launch \n");
      }
    } else {
      *run_id = (void *)(long)pid; // parent PID
    }
    return kDIALStatusRunning;
  } else {
    return kDIALStatusStopped;
  }
}


/* Compare the applications last launch parameters with the new parameters.
 * If they match, return false
 * If they don't match, return true
 */
int shouldRelaunch(
    DIALServer *pServer,
    const char *pAppName,
    const char *args )
{
    return ( strncmp( DIAL_get_payload(pServer, pAppName), args, DIAL_MAX_PAYLOAD ) != 0 );
}

static DIALStatus youtube_start(DIALServer *ds, const char *appname,
                                const char *payload, const char* query_string,
                                const char *additionalDataUrl,
                                DIAL_run_t *run_id, void *callback_data) {
    printf("\n\n ** LAUNCH YouTube ** with payload %s\n\n", payload);

    char url[512] = {0,}, data[512] = {0,};
    if (strlen(payload) && strlen(additionalDataUrl)) {
        snprintf( url, sizeof(url), "https://www.youtube.com/tv?%s&%s", payload, additionalDataUrl);
    } else if (strlen(payload)) {
        snprintf( url, sizeof(url), "https://www.youtube.com/tv?%s", payload);
    } else {
      snprintf( url, sizeof(url), "https://www.youtube.com/tv");
    }
    snprintf( data, sizeof(data), "--user-data-dir=%s/.config/google-chrome-dial", getenv("HOME") );

    const char * const youtube_args[] = { spAppYouTubeExecutable,
      spYouTubePS3UserAgent,
      data, "--app", url, NULL
    };
    runApplication( youtube_args, run_id );

    return kDIALStatusRunning;
}

static DIALStatus youtube_hide(DIALServer *ds, const char *app_name,
                                        DIAL_run_t *run_id, void *callback_data)
{
    return (isAppRunning( spAppYouTube, spAppYouTubeMatch )) ? kDIALStatusRunning : kDIALStatusStopped;
}
        
static DIALStatus youtube_status(DIALServer *ds, const char *appname,
                                 DIAL_run_t run_id, int *pCanStop, void *callback_data) {
    // YouTube can stop
    *pCanStop = 1;
    return isAppRunning( spAppYouTube, spAppYouTubeMatch ) ? kDIALStatusRunning : kDIALStatusStopped;
}

static void youtube_stop(DIALServer *ds, const char *appname, DIAL_run_t run_id,
                         void *callback_data) {
    printf("\n\n ** KILL YouTube **\n\n");
    pid_t pid;
    if ((pid = isAppRunning( spAppYouTube, spAppYouTubeMatch ))) {
        kill(pid, SIGTERM);
    }
}

void run_ssdp(int port, const char *pFriendlyName, const char * pModelName, const char *pUuid);

static void printUsage()
{
    int i, numberOfOptions = sizeof(gDialOptions) / sizeof(dial_options_t);
    printf("usage: dialserver <options>\n");
    printf("options:\n");
    for( i = 0; i < numberOfOptions; i++ )
    {
        printf("        %s|%s [value]: %s\n",
            gDialOptions[i].pOption,
            gDialOptions[i].pLongOption,
            gDialOptions[i].pOptionDescription );
    }
}

static void setValue( char * pSource, char dest[] )
{
    // Destination is always one of our static buffers with size BUFSIZE
    memset( dest, 0, BUFSIZE );
    memcpy( dest, pSource, strlen(pSource) );
}

static void setDataDir(char *pData)
{
    setValue( spNfDataDir, spDataDir );
    strncat(spDataDir, pData, sizeof(spDataDir) - 1);
}

void runDial(void)
{
    DIALServer *ds;
    ds = DIAL_create();
    if (ds == NULL) {
        printf("Unable to create DIAL server.\n");
        return;
    }
    
    struct DIALAppCallbacks cb_nf;
    cb_nf.start_cb = netflix_start;
    cb_nf.hide_cb = netflix_hide;
    cb_nf.stop_cb = netflix_stop;
    cb_nf.status_cb = netflix_status;
    struct DIALAppCallbacks cb_yt = {youtube_start, youtube_hide, youtube_stop, youtube_status};
    struct DIALAppCallbacks cb_system = {system_start, system_hide, NULL, system_status};

#if defined DEBUG
    if (DIAL_register_app(ds, "Netflix", &cb_nf, NULL, 1, "https://netflix.com https://www.netflix.com https://port.netflix.com:123 proto://*") == -1 ||
        DIAL_register_app(ds, "YouTube", &cb_yt, NULL, 1, "https://youtube.com https://www.youtube.com https://*.youtube.com:443 https://port.youtube.com:123 package:com.google.android.youtube package:com.google.ios.youtube proto:*") == -1 ||
#else
    if (DIAL_register_app(ds, "Netflix", &cb_nf, NULL, 1, "https://netflix.com https://www.netflix.com") == -1 ||
        DIAL_register_app(ds, "YouTube", &cb_yt, NULL, 1, "https://youtube.com https://*.youtube.com package:*") == -1 ||
#endif
        DIAL_register_app(ds, "system", &cb_system, NULL, 1, "") == -1)
    {
        printf("Unable to register DIAL applications.\n");
    } else if (!DIAL_start(ds)) {
        printf("Unable to start DIAL master listening thread.\n");
    } else {
        gDialPort = DIAL_get_port(ds);
        printf("launcher listening on gDialPort %d\n", gDialPort);
        run_ssdp(gDialPort, spFriendlyName, spModelName, spUuid);
        
        DIAL_stop(ds);
    }
    free(ds);
}

static void processOption( int index, char * pOption )
{
    switch(index)
    {
    case 0: // Data path
        memset( spDataDir, 0, sizeof(spDataDir) );
        setDataDir( pOption );
        break;
    case 1: // Netflix path
        setValue( pOption, spNetflix );
        break;
    case 2: // Friendly name
        setValue( pOption, spFriendlyName );
        break;
    case 3: // Model Name
        setValue( pOption, spModelName );
        break;
    case 4: // UUID
        setValue( pOption, spUuid );
        break;
    case 5:
        if (strcmp(pOption, "on")==0) {
            wakeOnWifiLan=true;
        } else if (strcmp(pOption, "off") == 0) {
            wakeOnWifiLan=false;
        } else {
            fprintf(stderr, "Option %s is not valid for %s",
                    pOption, WAKE_OPTION_LONG);
            exit(1);
        }
        break;
    case 6:
        setValue( pOption, spSleepPassword );
        break;
    default:
        // Should not get here
        fprintf( stderr, "Option %d not valid\n", index);
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    struct sigaction action;
    action.sa_handler = signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGTERM, &action, NULL);

    srand(time(NULL));
    int i;
    i = isAppRunning(spAppNetflix, NULL );
    printf("Netflix is %s\n", i ? "Running":"Not Running");
    i = isAppRunning( spAppYouTube, spAppYouTubeMatch );
    printf("YouTube is %s\n", i ? "Running":"Not Running");

    // set all defaults
    setValue(spDefaultFriendlyName, spFriendlyName );
    setValue(spDefaultModelName, spModelName );
    setValue(spDefaultUuid, spUuid );
    setValue(spDefaultNetflix, spNetflix );
    setDataDir(spDefaultData);

    // Process command line options
    // Loop through pairs of command line options.
    for( i = 1; i < argc; i+=2 )
    {
        int numberOfOptions = sizeof(gDialOptions) / sizeof(dial_options_t);
        while( --numberOfOptions >= 0 )
        {
            int shortLen, longLen;
            shortLen = strlen(gDialOptions[numberOfOptions].pOption);
            longLen = strlen(gDialOptions[numberOfOptions].pLongOption);
            if( ( ( strncmp( argv[i], gDialOptions[numberOfOptions].pOption, shortLen ) == 0 ) ||
                ( strncmp( argv[i], gDialOptions[numberOfOptions].pLongOption, longLen ) == 0 ) ) &&
                ( (i+1) < argc ) )
            {
                processOption( numberOfOptions, argv[i+1] );
                break;
            }
        }
        // if we don't find an option in our list, bail out.
        if( numberOfOptions < 0 )
        {
            printUsage();
            exit(1);
        }
    }
    runDial();

    return 0;
}

