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

#ifndef DIALSERVER_H
#define DIALSERVER_H

//#define DEBUG
#ifdef DEBUG
#define ATRACE(...) printf(__VA_ARGS__)
#else
#define ATRACE(...)
#endif

#include <string>
#include <vector>

using namespace std;

class DialServer
{
public:
    /**
     * Dial Server ctor
     *
     * @param[in] location dd.xml LOCATION header
     * @param[in] appsUrl Parsed out Application URL
     * @param[in] location dd.xml LOCATION header
     * empty string to find any server
     *
     */
    DialServer( string location, string appsUrl, string dd_xml ) : 
    m_location(location),
    m_appsUrl(appsUrl),
    found(true),
    m_ddxml(dd_xml) 
    {}

    ~DialServer() { ATRACE("%s\n", __FUNCTION__); }

    /**
     * Get the DIAL Server location
     *
     * @return Location of the server (http://<IP_ADDR>:<PORT>/dd.xml)
     */
    string getLocation() { return m_location; }

    /**
     * Get the DIAL Server IP address
     *
     * @return IP address of the server (X.X.X.X)
     */
    string getIpAddress();

    /**
     * Get the DIAL REST endpoint
     *
     * @return Location of the server (http://<IP_ADDR>:<PORT>/apps)
     */
    string getAppsUrl() { return m_appsUrl; }

    /**
     * Get the DIAL friendly name
     *
     * @return true if successful, false otherwise
     */
    bool getFriendlyName( string& name );

    /**
     * Get the DIAL UUID
     *
     * @return true if successful, false otherwise
     */
    bool getUuid( string& uuid );

    /**
     * Launch a DIAL application
     *
     * @param[in] application Name of the application to launch
     * @param[in] payload launch POST data
     * @param[out] responseHeaders Returns the HTTP response headers
     * @param[out] responseBody Returns the HTTP response body
     *
     * @return 0 if successful, !0 otherwise
     */
    int launchApplication(
        string &application,
        string &payload, 
        string &responseHeaders, 
        string &responseBody );

    /**
     * Get the status of a DIAL application
     *
     * @param[in] application Name of the application to query
     * @param[out] responseHeaders Returns the HTTP response headers
     * @param[out] responseBody Returns the HTTP response body
     *
     * @return 0 if successful, !0 otherwise
     */
    int getStatus(
        string &application,
        string &responseHeaders, 
        string &responseBody );

    /**
     * Stop an application.
     *
     * @param[in] application Name of the application to stop
     * @param[out] responseHeaders Returns the HTTP response headers
     *
     * @return 0 if successful, !0 otherwise
     */
    int stopApplication(
        string &application,
        string &responseHeaders );


    /**  ********************* **/
    /**  Convenience functions **/
    /**  ********************* **/

    /**
     * Extract a header from the response
     *
     * @param[in] responseHeaders Response headers
     * @param[in] header Header value to extract
     * @param[out] value Value of the header provided
     *
     * @return 0 if successful, !0 otherwise
     */
    int getHttpResponseHeader( 
        string &responseHeaders,
        string &header,
        string &value );

    /**
     * Returns true if the server has been recently found
     *
     * @return true if successful, false otherwise
     */
    bool isFound() { return found; }

    /**
     * Sets the "found status" of this server
     */
    void setFound(bool b) { found = b; }

private:
    int sendCommand( string &url, int command, string &payload, 
        string &responseHeaders, string &responseBody );
    string m_location;
    string m_appsUrl;
    string m_ipAddr;
    bool found;
    string m_ddxml; // information about the device
    string m_stopEndPoint;
};

#endif // DIALSERVER_H
