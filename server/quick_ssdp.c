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

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "mongoose.h"
#include <stdbool.h>

// TODO: Partners should define this port
#define SSDP_PORT (56790)
static char gBuf[4096];

// TODO: Partners should get the friendlyName from the system and insert here.
// TODO: Partners should get the UUID from the system and insert here.
static const char ddxml[] = ""
    "<?xml version=\"1.0\"?>"
    "<root"
    " xmlns=\"urn:schemas-upnp-org:device-1-0\""
    " xmlns:r=\"urn:restful-tv-org:schemas:upnp-dd\">"
    " <specVersion>"
    " <major>1</major>"
    " <minor>0</minor>"
    " </specVersion>"
    " <device>"
    " <deviceType>urn:schemas-upnp-org:device:tvdevice:1</deviceType>"
    " <friendlyName>%s</friendlyName>"
    " <manufacturer> </manufacturer>"
    " <modelName>%s</modelName>"
    " <UDN>uuid:%s</UDN>"
    " </device>"
    "</root>";

// TODO: Partners should use appropriate timeout (in seconds) if hardware supports WoL.
static const short wakeup_timeout = 10;

// TODO: Partners should get the UUID from the system and insert here.
static const char ssdp_reply[] = "HTTP/1.1 200 OK\r\n"
    "LOCATION: http://%s:%d/dd.xml\r\n"
    "CACHE-CONTROL: max-age=1800\r\n"
    "EXT:\r\n"
    "BOOTID.UPNP.ORG: 1\r\n"
    "SERVER: Linux/2.6 UPnP/1.1 quick_ssdp/1.1\r\n"
    "ST: urn:dial-multiscreen-org:service:dial:1\r\n"
    "USN: uuid:%s::"
    "urn:dial-multiscreen-org:service:dial:1\r\n"
    "%s"
    "\r\n";

static const char wakeup_header[] = "WAKEUP: MAC=%s;Timeout=%d\r\n";
#define STR_TIMEOUTLEN 6 /* Longest is 32767 */
#define HW_ADDRSTRLEN 18
static char ip_addr[INET6_ADDRSTRLEN] = "127.0.0.1";
static int dial_port = 0;
static int my_port = 0;
static char friendly_name[256];
static char uuid[256];
static char model_name[256];
static struct mg_context *ctx;

bool wakeOnWifiLan=true;

static void *request_handler(enum mg_event event,
                             struct mg_connection *conn,
                             const struct mg_request_info *request_info) {
    if (event == MG_NEW_REQUEST) {
        if (!strcmp(request_info->uri, "/dd.xml") &&
            !strcmp(request_info->request_method, "GET")) {
            mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/xml\r\n"
                      "Application-URL: http://%s:%d/apps/\r\n"
                      "\r\n", ip_addr, dial_port);
            mg_printf(conn, ddxml, friendly_name, model_name, uuid);
        } else {
            mg_send_http_error(conn, 404, "Not Found", "Not Found");
        }
        return "done";
    }
    return NULL;
}

/**
 * Returns the local hardware address (e.g. MAC address). On macOS the "en0"
 * interface is used. On other platforms the first non-loopback interface is
 * used.
 *
 * As a side-effect, the local global ip_addr is also populated.
 *
 * (Are these choices of interface really the right ones? Seems risky for
 * multi-homed systems.)
 *
 * @return the local hardware address or NULL if it does not exist, cannot
 *         be retrieved, or out-of-memory. The caller must free the returned
 *         memory.
 */
static char * get_local_address() {
    struct ifconf ifc;
    char buf[4096];
    char * hw_addr = NULL;
    int s, i;
    if (-1 == (s = socket(AF_INET, SOCK_DGRAM, 0))) {
        perror("socket");
        exit(1);
    }
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (0 > ioctl(s, SIOCGIFCONF, &ifc)) {
        perror("SIOCGIFCONF");
        exit(1);
    }
    if (ifc.ifc_len == sizeof(buf)) {
        fprintf(stderr, "SIOCGIFCONF output too long");
        exit(1);
    }
    for (i = 0; i < ifc.ifc_len/sizeof(ifc.ifc_req[0]); i++) {
        strncpy(ip_addr,
               inet_ntoa(((struct sockaddr_in *)(&ifc.ifc_req[i].ifr_addr))->sin_addr), sizeof(ip_addr) - 1);
        if (0 > ioctl(s, SIOCGIFFLAGS, &ifc.ifc_req[i])) {
            perror("SIOCGIFFLAGS");
            exit(1);
        }
        if (ifc.ifc_req[i].ifr_flags & IFF_LOOPBACK) {
            // don't use loopback interfaces
            continue;
        }
        if (0 > ioctl(s, SIOCGIFHWADDR, &ifc.ifc_req[i])) {
            perror("SIOCGIFHWADDR");
            exit(1);
        }
        // FIXME: How do I figure out what type of interface this is in order to
        // cast the struct sockaddr ifr_hraddr to the proper type and extract the
        // hardware address?
        //
        // Make sure this is correct for the target device and platform.
        hw_addr = (char*)malloc(HW_ADDRSTRLEN + 1);
        if (hw_addr == NULL)
            break;
        sprintf(hw_addr, "%02x:%02x:%02x:%02x:%02x:%02x",
                (unsigned char)ifc.ifc_req[i].ifr_hwaddr.sa_data[0],
                (unsigned char)ifc.ifc_req[i].ifr_hwaddr.sa_data[1],
                (unsigned char)ifc.ifc_req[i].ifr_hwaddr.sa_data[2],
                (unsigned char)ifc.ifc_req[i].ifr_hwaddr.sa_data[3],
                (unsigned char)ifc.ifc_req[i].ifr_hwaddr.sa_data[4],
                (unsigned char)ifc.ifc_req[i].ifr_hwaddr.sa_data[5]);
        break;
    }
    close(s);
    return hw_addr;
}

static void handle_mcast(char *hw_addr) {
    int s, one = 1, bytes;
    socklen_t addrlen;
    struct sockaddr_in saddr = {0};
    struct ip_mreq mreq = {0};
    char wakeup_buf[sizeof(wakeup_header) + HW_ADDRSTRLEN + STR_TIMEOUTLEN] = {0, };
    char send_buf[sizeof(ssdp_reply) + INET_ADDRSTRLEN + 256 + 256 + sizeof(wakeup_buf)] = {0,};
    int send_size;
    if (-1 < wakeup_timeout && wakeOnWifiLan) {
        snprintf(wakeup_buf, sizeof(wakeup_buf), wakeup_header, hw_addr, wakeup_timeout);
    }
    send_size = snprintf(send_buf, sizeof(send_buf), ssdp_reply, ip_addr, my_port, uuid, wakeup_buf);
    if (-1 == (s = socket(AF_INET, SOCK_DGRAM, 0))) {
        perror("socket");
        exit(1);
    }
    if (-1 == setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
        perror("reuseaddr");
        exit(1);
    }
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("239.255.255.250");
    saddr.sin_port = htons(1900);

    if (-1 == bind(s, (struct sockaddr *)&saddr, sizeof(saddr))) {
        perror("bind");
        exit(1);
    }
    mreq.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
    mreq.imr_interface.s_addr = inet_addr(ip_addr);
    if (-1 == setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                         &mreq, sizeof(mreq))) {
        perror("add_membership");
        exit(1);
    }
    //printf("Starting Multicast handling on 239.255.255.250\n");
    while (1) {
        addrlen = sizeof(saddr);
        if (-1 == (bytes = recvfrom(s, gBuf, sizeof(gBuf) - 1, 0,
                                    (struct sockaddr *)&saddr, &addrlen))) {
            perror("recvfrom");
            continue;
        }
        gBuf[bytes] = 0;
        // sophisticated SSDP parsing algorithm
        if (!strstr(gBuf, "urn:dial-multiscreen-org:service:dial:1"))
            {
#if 0 // use for debugging
                printf("Dropping: \n");
                {
                    int i;
                    for (i = 0; i < bytes; i++)
                        {
                            putchar(gBuf[i]);
                        }
                }
                printf("\n##### End of DROP #######\n");
#endif
                continue;
        }
        printf("Sending SSDP reply to %s:%d\n",
               inet_ntoa(saddr.sin_addr), ntohs(saddr.sin_port));
        if (-1 == sendto(s, send_buf, send_size, 0, (struct sockaddr *)&saddr, addrlen)) {
            perror("sendto");
            continue;
        }
    }
}

void run_ssdp(int port, const char *pFriendlyName, const char * pModelName, const char *pUuid) {
    char *hw_addr = NULL;
    struct sockaddr sa;
    socklen_t len = sizeof(sa);
    if(pFriendlyName) {
        strncpy(friendly_name, pFriendlyName, sizeof(friendly_name));
        friendly_name[sizeof(friendly_name) - 1] = '\0';
    } else {
        strcpy(friendly_name, "DIAL server sample");
    }
    if(pModelName) {
        strncpy(model_name, pModelName, sizeof(model_name));
        model_name[sizeof(model_name) - 1] = '\0';
    } else {
        strcpy(model_name, "deadbeef-dead-beef-dead-beefdeadbeef");
    }
    if(pUuid) {
        strncpy(uuid, pUuid, sizeof(uuid));
        uuid[sizeof(uuid) - 1] = '\0';
    } else {
        strcpy(uuid, "deadbeef-dead-beef-dead-beefdeadbeef");
    }
    dial_port = port;
    hw_addr = get_local_address();
    if (hw_addr == NULL) {
        printf("Unable to retrieve hardware address.");
        return;
    }
    ctx = mg_start(&request_handler, NULL, SSDP_PORT);
    if (ctx == NULL) {
        printf("Unable to start SSDP master listening thread.");
    } else {
        if (mg_get_listen_addr(ctx, &sa, &len)) {
            my_port = ntohs(((struct sockaddr_in *)&sa)->sin_port);
        }
        printf("SSDP listening on %s:%d\n", ip_addr, my_port);
        handle_mcast(hw_addr);
    }
    free(hw_addr); hw_addr = NULL;
}
