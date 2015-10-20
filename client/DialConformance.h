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

#ifndef DIALCONFORMANCE_H
#define DIALCONFORMANCE_H

#include <memory>
#include "DialServer.h"
#include "DialClientInput.h"

using namespace std;

class DialConformance
{
public:
    /**
     * Create a singleton
     */
    static DialConformance* create();

    /**
     * Get a pointer to the singleton
     */
    static DialConformance* instance(void);

    ~DialConformance();

    /**
     * Run the test
     *
     * @param[in] pServer DIAL server to test
     * @param[in] appList List of applications from the command line
     * @param[in] inputFile Input file used to drive tests
     * @param[in] outputFile File for writing the report.  Use an
     * empty string to use the default (report.html).
     *
     * @return 0 if successful, !0 otherwise
     */
    int run( 
        DialServer* pServer,
        vector<string>& appList,
        string &inputFile,
        string &outputFile );
private:
    DialConformance();
    DialServer*                 _pServer;
    static DialConformance*     sConformance;
    DialClientInput             _input;

    /**
     * Class to manage state for each application supported by the DIAL 
     * server.  The application list is managed by the input file.
     */
    class Application{
    public:
        enum State
        {
            STOPPED,
            LAUNCHING,  // application has just been launched, state 
                        // has not been confirmed
            LAUNCHED,
            STOPPING    // application has been stopped, state has not
                        // been confirmed
        };

        Application( string& name, DialServer *pServer, bool isErrorApp ) :
        _isErrorApp(isErrorApp),
        _pServer(pServer),
        _state(STOPPED),
        _stopurl(""),
        _name(name) {}
        ~Application(){}       
        
        /**
         * State Getter/Setters
         */
        string getName()    {   return _name; }
        State getState()    {   return _state; }

        /**
         * Returns true if the application is not a valid application
         */
        bool isErrorApp()   {   return _isErrorApp; }

        /**
         * Launch an this application using the server that was stored when
         * the class was created.
         *
         * @param[in] payload Data that is put into the POST data.
         * @param[out] responseHeaders HTML response headers
         * @param[out] responseBody HTML response body
         */
        void launch(
            string& payload,
            string& responseHeaders,
            string& responseBody )
        {
            _pServer->launchApplication( 
                        _name, payload, responseHeaders, responseBody );
            // TODO: Set state

            // find Location in the header, store the stop url
            if( !responseHeaders.empty() )
            {
                size_t pos, tmp = responseHeaders.find("Location");
                if( tmp != responseHeaders.npos )
                {
                    pos = responseHeaders.find("http", tmp);
                    size_t posEnd = responseHeaders.find("\n", pos+1);
                    if( posEnd != responseHeaders.npos )
                        _stopurl = responseHeaders.substr( pos, posEnd );
        
                    // chomp off the \r\n chars
                    DialConformance::chomp( _stopurl );
                    ATRACE("StopURL = %s********\n", _stopurl.c_str());
                }
            }
        }

        /**
         * Get the status of this application
         *
         * @param[out] responseHeaders HTML response headers
         * @param[out] responseBody HTML response body
         */
        void status(
            string& responseHeaders,
            string& responseBody )
        { _pServer->getStatus( _name, responseHeaders, responseBody ); }

        /**
         * Stop the application
         *
         * @param[out] responseHeaders HTML response headers
         */
        void stop(
            string& responseHeaders )
        { stop(_stopurl, responseHeaders ); }

        /**
         * Stop the application using a custom stop URL.
         *
         * @param[out] responseHeaders HTML response headers
         */
        void stop( string& stopurl, string& responseHeaders)
        {
            if( !stopurl.empty() )
            {
                _pServer->stopApplication( stopurl, responseHeaders );
            }
#ifdef DEBUG
            else ATRACE("%s: Not sending stop, stop URL is empty\n", __FUNCTION__);
#endif
        }

    private:
        bool _isErrorApp;
        DialServer* _pServer;
        State _state;
        string _stopurl;
        string _name;
    };
    // list of applications
    vector<Application*> _apps;

    // Internal helpers
    void run_internal( DialServer* pServer );
    bool execute_command( string& command, vector<string>& params );

    // Get the Application pointer from an application string
    Application* getApplication( string& command );

    // extract the payload from a list of parameters
    void getPayload(vector<string>& params, string& payload );

    // Command execution functions
    bool execute_launch( Application* pApp, vector<string>& params );
    bool execute_status( Application* pApp, vector<string>& params );
    bool execute_stop( Application* pApp, vector<string>& params );

    // Helper function to extract a parameter.
    void extractParamValue( string& param, string& value );

    // Validation functions
    bool validateParams( vector<string>& params, 
            string& responseHeaders, string& responseBody );
    bool validateHttpResponse( string& headers, string& params );
    bool validateHttpHeaders( string& headers, string& params );
    bool validateResponseBody( string& headers, string& params );

public:
    // Helper function to chomp off the carriage return line feed.
    static void chomp(string& str)
    {
        string crlf("\r\n");
        size_t pos = str.find_last_not_of( crlf );
        if( pos != str.npos )
        {
            ATRACE("CHOMP\n");
            str.erase(pos+1);
        }
    }
};

#endif  // DIALCONFORMANCE_H
