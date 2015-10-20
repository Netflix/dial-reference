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

#include "DialServer.h"
#include <curl/curl.h>

using namespace std;

enum DIAL_COMMAND{
    COMMAND_LAUNCH,
    COMMAND_STATUS,
    COMMAND_KILL
};

static size_t header_cb(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    if ((size * nmemb) != 0) {
        string newHeader((char*)ptr);
        string *header = static_cast<string*>(userdata);
        header->append(newHeader);
        ATRACE("%s: Adding header: %s", __FUNCTION__, newHeader.c_str());
    }
    return (size * nmemb);
}

static size_t receiveData(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    if ((size * nmemb) != 0) {
        string *body= static_cast<string*>(userdata);
        body->append((char*)ptr);
        ATRACE("%s: Adding to Body: %s", __FUNCTION__, (char*)ptr);
    }
    return (size * nmemb);
}

int DialServer::sendCommand( 
        string &url, 
        int command, 
        string &payload, 
        string &responseHeaders, 
        string &responseBody ) 
{
    CURL *curl;
    CURLcode res = CURLE_OK;

    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) 
    {
        fprintf(stderr, "curl_global_init() failed\n");
        return 0;
    }

    if ((curl = curl_easy_init()) == NULL) 
    {
        fprintf(stderr, "curl_easy_init() failed\n");
        curl_global_cleanup();
        return 0;
    }

    if (command == COMMAND_LAUNCH) 
    {
        curl_easy_setopt(curl, CURLOPT_POST, true);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload.size());
        if( payload.size() )
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        }
#ifdef DEBUG
        else ATRACE("Sending empty POST\n");
#endif
    } 
    else if (command == COMMAND_KILL)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    ATRACE("Sending %s:%s\n", 
        command == COMMAND_LAUNCH ? "LAUNCH" :
            (command == COMMAND_KILL ? "KILL" : "STATUS"),
        url.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_cb);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseHeaders);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receiveData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return (res == CURLE_OK);
}

string DialServer::getIpAddress()
{
    // m_appsUrl=http://192.168.1.103:36269/apps/
    if( m_ipAddr.empty() )
    {
        size_t begin = m_appsUrl.find("//"); 
        if( begin != m_appsUrl.npos )
        {
            begin += 2; // move to the start of the IP address
            size_t end = m_appsUrl.find(":", begin);
            if( end != m_appsUrl.npos )
            {
                m_ipAddr = m_appsUrl.substr( begin, end-begin );
                ATRACE("IP ADDRESS: %s\n", m_ipAddr.c_str() );
            }
        }
    }
    return m_ipAddr;
}

bool DialServer::getFriendlyName( string& name )
{
    bool retval = false;
    if( !m_ddxml.empty() )
    {
        string friendlyName = "<friendlyName>";
        size_t pos;
        if( ( pos = m_ddxml.find( friendlyName ) ) != m_ddxml.npos )
        {
            string friendlyNameEnd = "</friendlyName>";
            size_t end = m_ddxml.find( friendlyNameEnd );
            name = m_ddxml.substr( pos + friendlyName.size(), 
                        (end - (pos + friendlyName.size())) );
            ATRACE("***Friendly name=%s***", name.c_str());
            retval = true;
        }
#ifdef DEBUG
        else
        {
            ATRACE("Friendly name not found\n%s\n", m_ddxml.c_str());
        }
#endif
    }

    return retval;
}

bool DialServer::getUuid( string& uuid )
{
    bool retval = false;
    if( !m_ddxml.empty() )
    {
        string udn = "<UDN>";
        size_t pos;
        if( ( pos = m_ddxml.find( udn ) ) != m_ddxml.npos )
        {
            string udnEnd = "</UDN>";
            size_t end = m_ddxml.find( udnEnd );
            uuid = m_ddxml.substr( pos + udn.size(), 
                        (end - (pos + udn.size())) );
            ATRACE("***UUID=%s***", uuid.c_str() );
            retval = true;
        }
#ifdef DEBUG
        else
        {
            ATRACE("Friendly name not found\n%s\n", m_ddxml.c_str());
        }
#endif
    }

    return retval;
}

int DialServer::launchApplication(
    string &application,
    string &payload, 
    string &responseHeaders, 
    string &responseBody )
{
    ATRACE("%s: Launch %s\n", __FUNCTION__, application.c_str());
    string appUrl = m_appsUrl;
    sendCommand( appUrl.append(application), COMMAND_LAUNCH, payload, responseHeaders, responseBody);
    return 0;
}

int DialServer::getStatus(
    string &application,
    string &responseHeaders, 
    string &responseBody )
{
    ATRACE("%s: GetStatus %s\n", __FUNCTION__, application.c_str());
    string emptyPayload;
    string appUrl = m_appsUrl;
    sendCommand( appUrl.append(application), COMMAND_STATUS, emptyPayload, responseHeaders, responseBody );

    ATRACE("Body: %s\n", responseBody.c_str());
    unsigned found = responseBody.find("href=");
    if( found != string::npos )
    {
        // The start of href is after href= and the quote
        unsigned href_start = found + 5 + 1;

        // get the body from the start of href to the end, then find 
        // the last quote delimiter.
        string tmp = responseBody.substr( href_start );
        unsigned href_end = tmp.find("\"");
        m_stopEndPoint = responseBody.substr( href_start, href_end );
    }
    return 0;
}

int DialServer::stopApplication(
    string &application,
    string &responseHeaders )
{
    ATRACE("%s: Quit %s\n", __FUNCTION__, application.c_str());
    string emptyPayload, responseBody;  // dropping this
    string appUrl = m_appsUrl;

    // just call status to update the run endpoint
    getStatus( application, responseHeaders, responseBody );

    sendCommand( 
            (appUrl.append(application)).append("/"+m_stopEndPoint), 
            COMMAND_KILL, emptyPayload, responseHeaders, responseBody );
    return 0;
}

int DialServer::getHttpResponseHeader( 
    string &responseHeaders,
    string &header,
    string &value )
{
    return 0;
}
