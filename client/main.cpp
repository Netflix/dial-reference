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
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

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
        string uuid, name, macAddr;
        int wolTimeOut;
        (*it)->getFriendlyName( name );
        (*it)->getUuid( uuid );
        macAddr    =(*it)->getMacAddress();
        wolTimeOut =(*it)->getWakeOnLanTimeout();
        printf("%d: Server IP[%s] UUID[%s] FriendlyName[%s] MacAddress[%s] WakeOnLanTimeout[%d]\n", 
               i+1, (*it)->getIpAddress().c_str(),
               uuid.c_str(), name.c_str(), macAddr.c_str(), wolTimeOut);
    }
}

static DialServer* getServerFromUser( vector<DialServer*> list )
{
    DialServer* pServer;
    // show a list to the user
    char buf[80] = {0,};
    vector<DialServer*>::iterator it;

    printf("Found Multiple servers\n");
    printf("0: Rescan and list DIAL servers\n");
    printServerList(list);
    printf("Enter server: ");
    scanf("%s", buf);
    unsigned int server = atoi(buf);
    if( server > 0 && server <= list.size()){
        pServer = list[server-1];
    }else{
        pServer = NULL;
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
            printf("%zu available server(s): \n", list.size());
            printServerList(list);
        }
    }
    else
    {
        printf("No servers available\n");
    }
}

static void sendMagic(string macAddr)
{
    unsigned char tosend[102];
    unsigned char mac[6];

    /** first 6 bytes of 255 **/
    for(int i = 0; i < 6; i++) {
        tosend[i] = 0xFF;
    }

    /** store mac address **/
    printf("sending magic packet to: ");
    for (int i=0; i<6; i++){
        mac[i]=(unsigned char)(strtoul(macAddr.substr(i*3, 2).c_str(), NULL, 16));
        printf("%02x:", mac[i]);
    }
    printf("\n");

    /** append it 16 times to packet **/
    for(int i = 1; i <= 16; i++) {
        memcpy(&tosend[i * 6], &mac, 6 * sizeof(unsigned char));
    }

    int udpSocket;
    struct sockaddr_in udpClient, udpServer;
    int broadcast = 1;

    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

    /** you need to set this so you can broadcast **/
    if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
        perror("setsockopt (SO_BROADCAST)");
        exit(1);
    }
    udpClient.sin_family = AF_INET;
    udpClient.sin_addr.s_addr = INADDR_ANY;
    udpClient.sin_port = 0;

    bind(udpSocket, (struct sockaddr*)&udpClient, sizeof(udpClient));

    /** set server end point (the broadcast addres)**/
    udpServer.sin_family = AF_INET;
    udpServer.sin_addr.s_addr = inet_addr("255.255.255.255");
    udpServer.sin_port = htons(9);

    /** send the packet **/
    sendto(udpSocket, &tosend, sizeof(unsigned char) * 102, 0, (struct sockaddr*)&udpServer, sizeof(udpServer));
}


int handleUser(DialDiscovery *pDial) {
    int processInputOuter = 1;        
    char buf[80];
    vector<DialServer*> list;
    
    while (processInputOuter){
        pDial->getServerList(list);
        if( list.size() == 0 ){
            printf("No servers available\n");
            return 1;
        }
        DialServer* pServer = getServerFromUser( list );
        if (pServer==NULL){
            pDial->send_mcast();
            continue;
        }

        int processInput = 1;        
        while(processInput){
            string responseHeaders, responseBody, payload;
            string netflix = "Netflix";
            string youtube = "YouTube";
            
            memset(buf, 0, 80);
            printf("0. Rescan and list DIAL servers\n");
            printf("1. Launch Netflix\n");
            printf("2. Hide Netflix\n");
            printf("3. Stop Netflix\n");
            printf("4. Netflix status\n");
            printf("5. Launch YouTube\n");
            printf("6. Hide YouTube\n");
            printf("7. Stop YouTube\n");
            printf("8. YouTube status\n");
            printf("9. Run conformance tests\n");
            printf("10. Wake up on lan/wlan\n");
            printf("11. QUIT\n");
            printf("Command (0:1:2:3:4:5:6:7:8:9:10:11): ");
            scanf("%s", buf);
            switch( atoi(buf) )
                {
                case 0:
                    {
                        pDial->send_mcast();
                        processInput=0;
                    }break;
                case 1:
                    printf("Launch Netflix\n");
                    pServer->launchApplication( netflix, payload, responseHeaders, responseBody );
                    break;
                case 2:
                    printf("Hide Netflix\n");
                    pServer->hideApplication( netflix, responseHeaders, responseBody );
                    break;
                case 3:
                    printf("Stop Netflix\n");
                    pServer->stopApplication( netflix, responseHeaders );
                    break;
                case 4:
                    printf("Netflix Status: \n");
                    pServer->getStatus( netflix, responseHeaders, responseBody );
                    printf("RESPONSE: \n%s\n", responseBody.c_str());
                    break;
                case 5:
                    printf("Launch YouTube\n");
                    pServer->launchApplication( youtube, payload, responseHeaders, responseBody );
                    break;
                case 6:
                    printf("Hide YouTube\n");
                    pServer->hideApplication( youtube, responseHeaders, responseBody );
                    break;
                case 7:
                    printf("Stop YouTube\n");
                    pServer->stopApplication( youtube, responseHeaders );
                    break;
                case 8:
                    printf("YouTube Status: \n");
                    pServer->getStatus( youtube, responseHeaders, responseBody );
                    break;
                case 9:
                    runConformance();
                    break;
                case 10:
                    printf("Sending the magic packet\n");
                    sendMagic(pServer->getMacAddress());
                    break;
                case 11:
                    processInput = 0;
                    processInputOuter = 0;
                    break;
                default:
                    printf("Invalid, try again\n");
                }
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

