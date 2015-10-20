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

#include <string>
#include "DialDiscovery.h"
#include "DialConformance.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace std;

static DialDiscovery* gpDiscovery;
static bool gUseMenu = true;

// TODO: Make it possible to pass applications from the command line
static vector<string> gAppList;
static string gOutputFile;
static string gInputFile;

// IP address of the DIAL server
static string gIpAddress;

static void printServerList( vector<DialServer*> list )
{
    int i;
    vector<DialServer*>::iterator it;
    for( i = 0, it = list.begin(); it < list.end(); it++, i++ )
    {
        string uuid, name;
        (*it)->getFriendlyName( name );
        (*it)->getUuid( uuid );
        printf("%Zu: Server IP[%s] UUID[%s] FriendlyName[%s] \n", 
            i+1, (*it)->getIpAddress().c_str(),
            uuid.c_str(), name.c_str() );
    }
}

static DialServer* getServerFromUser( vector<DialServer*> list )
{
    DialServer* pServer;
    // show a list to the user
    if( list.size() > 1 )
    {
        char buf[80] = {0,};
        vector<DialServer*>::iterator it;

        printf("Found Multiple servers\n");
        printServerList(list);
        printf("Enter server: ");
        scanf("%s", buf);
        unsigned int server = atoi(buf);
        assert( server > 0 && server <= list.size() );
        pServer = list[server-1];
    }
    else
    {
        pServer = list.front();
    }
    return pServer;
}

static void runConformance()
{
    vector<DialServer*> list;
    gpDiscovery->getServerList(list);

    if( list.size() )
    {
        DialServer *pServer = NULL;
        if( !gIpAddress.empty() )
        {
            pServer = NULL;
            vector<DialServer*>::iterator it;
            for( it = list.begin(); it < list.end(); it ++ )
            {
                if( gIpAddress.compare((*it)->getIpAddress()) == 0 )
                {
                    ATRACE("Found server %s in the list of servers\n", 
                                gIpAddress.c_str() );
                    pServer = (*it);
                    break;
                }
            }
        }
        else
        {
            pServer = getServerFromUser( list );
        }

        if( pServer )
        {
            string name;
            bool serverExists = pServer->getFriendlyName(name);
            assert( serverExists );
            string uuid;
            pServer->getUuid( uuid );
            printf("\nRunning conformance against: IP[%s] UUID[%s] FriendlyName[%s] \n", 
                    pServer->getIpAddress().c_str(),
                    uuid.c_str(), name.c_str() );
            DialConformance::instance()->run( 
                pServer, 
                gAppList, 
                gInputFile, 
                gOutputFile );
        }
        else 
        {
            printf("DIAL server not found\n");
            printf("%Zu available server(s): \n", list.size());
            printServerList(list);
        }
    }
    else
    {
        printf("No servers available\n");
    }
}

int handleUser(DialDiscovery *pDial) {
    int processInput = 1;
    char buf[80];
    vector<DialServer*> list;

    pDial->getServerList(list);
    if( list.size() == 0 )
    {
        printf("No servers available\n");
        return 1;
    }
    DialServer* pServer = getServerFromUser( list );

    while(processInput)
    {
        string responseHeaders, responseBody, payload;
        string netflix = "Netflix";
        string youtube = "YouTube";

        memset(buf, 0, 80);
        printf("0. List DIAL servers\n");
        printf("1. Launch Netflix\n");
        printf("2. Kill Netflix\n");
        printf("3. Netflix status\n");
        printf("4. Launch YouTube\n");
        printf("5. Kill YouTube\n");
        printf("6. YouTube status\n");
        printf("7. Run conformance tests\n");
        printf("8. QUIT\n");
        printf("Command (0:1:2:3:4:5:6:7:8): ");
        scanf("%s", buf);
        switch( atoi(buf) )
        {
            case 0:
            {
                printf("\n\n******** %Zu servers found ********\n\n", list.size());
                for( unsigned int i = 0; i < list.size(); i++ )
                {
                    string name;
                    list[i]->getFriendlyName(name);
                    printf("Server %Zu: %s\n", i+1, name.c_str());
                }
                printf("\n*********************************\n\n");
            }break;
            case 1:
                printf("Launch Netflix\n");
                pServer->launchApplication( netflix, payload, responseHeaders, responseBody );
                break;
            case 2:
                printf("Kill Netflix\n");
                pServer->stopApplication( netflix, responseHeaders );
                break;
            case 3:
                printf("Netflix Status: \n");
                pServer->getStatus( netflix, responseHeaders, responseBody );
                printf("RESPONSE: \n%s\n", responseBody.c_str());
                break;
            case 4:
                printf("Launch YouTube\n");
                pServer->launchApplication( youtube, payload, responseHeaders, responseBody );
                break;
            case 5:
                printf("Kill YouTube\n");
                pServer->stopApplication( youtube, responseHeaders );
                break;
            case 6:
                printf("YouTube Status: \n");
                pServer->getStatus( youtube, responseHeaders, responseBody );
                break;
            case 7:
                runConformance();
                break;
            case 8:
                processInput = 0;
                break;
            default:
                printf("Invalid, try again\n");
        }
    } 
    return 0;
}

static const char usage[] = "\n"
" If no option is specified, the program will run a conformance test.\n"
"\n"
"usage: dialclient <option>\n"
" Option Parameter              Description\n"
" -h     none                   Usage menu\n"
" -s     filename (optional)    Run conformance test.  Use filename as\n"
"                               the input, if provided\n"
" -o     filename               Reporter output file (./report.html)\n"
" -a     ip_address             IP address of DIAL server (used for conformance\n"
"                               testing)\n"
"\n";

inline void notSupported( string s )
{
    printf( "%s not supported", s.c_str() );
    printf( "%s\n", usage );
    exit(0);
}

int parseArgs( int argc, char* argv[] )
{
    for( int i = 1; i < argc; i++ )
    {
        string input(argv[i]);
        if( input[0] != '-' ) notSupported(input);
        switch(input[1])
        {
        case 's':
            gUseMenu = false;
            if( argv[i+1] != NULL && argv[i+1][0] != '-' )
            {
                //filename provided
                gInputFile = argv[++i];
            }
            break;
        case 'o':
            gOutputFile = argv[++i];
            break;
        case 'a':
            gIpAddress = argv[++i];
            break;
        case 'h':
            printf("%s", usage);
            exit(0);
            break;
        default:
            notSupported(input);
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    parseArgs(argc, argv);

    gpDiscovery = DialDiscovery::create();
    gpDiscovery->init();
    DialConformance::create();

    // Sleep for 2 seconds to allow DIAL servers to response to MSEARCH.
    sleep(2);

    if ( gUseMenu )
    {
        return handleUser(gpDiscovery);
    }

    // not using the menu, just run the conformance test.
    runConformance();
    return 0;
}

