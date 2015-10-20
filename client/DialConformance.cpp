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

#include <iostream>
#include <fstream>
#include <sstream>

#include "DialConformance.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Number of milliseconds to sleep between launches when doing a launch=ALL
#define LAUNCH_SLEEP 6000
#define MAX_PARAMETER_LENGTH 60

using namespace std;

/* 
 * This class will be reponsible for reporting the conformance test results.
 * Requirements:
 *      * Results should be in HTML
 *      * Results should be easily readable
 *      * Results should clearly distinguish between pass and fail
 *      * If there is a failure, there should be ample information provided 
 *      for debugging
 */
class OutputWriter
{
public:
    OutputWriter( string fileName ) :
    _filename(fileName),
    _pCurrentTest(NULL),
    _isComplete(false)
    {
    }
    ~OutputWriter()
    { }

    void setOutputFile( string fileName )
    {
        _filename = fileName;
    }

    void setTest( string testName )
    {
        assert( _pCurrentTest == NULL );
        _pCurrentTest = new Test(testName);
        _tests.push_back(_pCurrentTest);
    }
    void clearTest() 
    { 
        // still holding a ref in the test vector.  That will get deleted
        // when the test is complete
        _pCurrentTest = NULL; 
    }

    void setError( string error ) { _pCurrentTest->addError(error); }
    void setResult( bool result ) { _pCurrentTest->setResult( result ); }

    void start( string& friendlyname, string& uuid, string ipaddress )
    {
        _file.open(_filename.c_str());
        _file << "<html>\n<body>\n";
        _file << "<font size=\"6\"><b>FriendlyName: " << friendlyname;
        _file << "<br>\nIP: " << ipaddress;
        _file << "<br>\nUUID: " << uuid;

        {
            time_t rawtime;
            struct tm * timeinfo;
            char buffer [80];
            
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            
            //strftime (buffer,80,"%I:%M%p.",timeinfo);
            strftime (buffer,80,"%c",timeinfo);
            _file << "<br>\nTime of Run: " << buffer << "</b></font>";
        }
        _file << "<table border=\"10\">\n";
        _file << "<tr>\n";
        _file << "<th> RESULT </th>";           //header 1
        _file << "<th> Test Executed </th>";    //header 2
        _file << "<th> Errors </th>";    //header 2
        _file << "</tr>\n";
    }

    void complete()
    {
        vector<Test*>::iterator it;
        for( it = _tests.begin(); it < _tests.end(); it++ )
        {
            write("<tr>");
            // write the name and the result
            stringstream result;
            bool testResult = (*it)->getResult();
            result << "<td><b> <font color=\"" << (testResult ? "green" : "red") << "\"> ";
            result << (testResult ? "SUCCESS" : "FAIL") << " </b></font></td>\n<td>" << (*it)->getName() << "</td>";
            write( result.str() );

            // write errors if they exist
            write("<td>");
            vector<string> errors = (*it)->getErrors();
            if( !errors.empty() )
            {
                vector<string>::iterator it2;
                write("<b>");
                for( it2 = errors.begin(); it2 < errors.end(); it2++ )
                    write( (*it2) );
                write("</b>");
            }
            else
                write("NO ERROR");
            write("</td>\n</tr>");

            // delete the test
            delete (*it);
        }
        _file << "</table></body>\n</html>\n";
        _file.close();

        _isComplete = true;
    }

private:
    // Abstraction of a test
    class Test
    {
    public:
        Test(string& name) :
            _testname(name),
            _testPassed(false) {}
        ~Test() {}

        string getName()                {   return _testname; }
        void setResult( bool result )   {   _testPassed = result; }
        bool getResult()                {   return _testPassed; }

        void addError( string error )   {   _errors.push_back(error); }
        vector<string> getErrors()      {   return _errors; }

    private:
        string _testname;
        bool _testPassed;
        vector<string> _errors;
    };

    string _filename;
    ofstream _file;
    vector<Test*> _tests;
    Test* _pCurrentTest;
    bool _isComplete;

    int write( string line )
    {
        _file << line.c_str() << "\n";
        return line.length()+1;
    }
};

static const string defaultOutputFile = "./report.html";
static OutputWriter gWriter(defaultOutputFile);

DialConformance *DialConformance::sConformance = 0;

DialConformance* DialConformance::create(void)
{
    assert( DialConformance::sConformance == 0 );
    return new DialConformance();
}

DialConformance* DialConformance::instance(void)
{
    return DialConformance::sConformance;
}

DialConformance::DialConformance() 
{
    assert( DialConformance::sConformance == 0 );
    DialConformance::sConformance = this;
}

DialConformance::~DialConformance()
{
    assert( DialConformance::sConformance == this );
    DialConformance::sConformance = 0;
}

void DialConformance::extractParamValue( 
    string& param, string& value )
{
    // TODO: Add support for quotes after the equals
    size_t posBegin = param.find("=");
    value = param.substr(posBegin+1, param.length()-posBegin);
}

bool DialConformance::validateParams( vector<string>& params, 
            string& responseHeaders, string& responseBody )
{
    bool testPassed = true;
    vector<string>::iterator it;
    for( it = params.begin(); it < params.end() && testPassed == true; it++ )
    {
        ATRACE("%s:: params:%s \n", __FUNCTION__, (*it).c_str());
        if( (*it).find("httpresponse=") != (*it).npos )
            testPassed = validateHttpResponse( responseHeaders, *it );
        if( (*it).find("httpresponseheader=") != (*it).npos )
            testPassed = validateHttpHeaders( responseHeaders, *it );
        if( (*it).find("resultbody=") != (*it).npos )
            testPassed = validateResponseBody( responseBody, *it );
    }
    return testPassed;
}


bool DialConformance::validateHttpResponse( 
    string& headers, string& param )
{
    bool retval = false;
    string responseCode;
    extractParamValue( param, responseCode );

    ATRACE("%s: Searching for %s\n", __FUNCTION__, responseCode.c_str());
    if( headers.find(responseCode) != headers.npos )
    {
        ATRACE("%s: %s FOUND\n", __FUNCTION__, responseCode.c_str());
        retval = true;
    }
    else
    {
        ATRACE("%s: %s not found\n", __FUNCTION__, responseCode.c_str());
        stringstream error;
        error << responseCode << " not found\n";
        gWriter.setError( error.str() );
    }
    return retval;
}

bool DialConformance::validateHttpHeaders( 
    string& headers, string& param )
{
    bool retval = false;
    string responseCode;
    extractParamValue( param, responseCode ); 

    ATRACE("%s: Searching for %s\n", __FUNCTION__, responseCode.c_str());
    if( headers.find(responseCode) != headers.npos )
    {
        ATRACE("%s: %s FOUND\n", __FUNCTION__, responseCode.c_str());
        retval = true;
    }
    else
    {
        ATRACE("%s: %s not found\n", __FUNCTION__, responseCode.c_str());
        stringstream error;
        error << responseCode << " not found\n";
        gWriter.setError( error.str() );
    }
    return retval;
}

bool DialConformance::validateResponseBody( 
    string& body, string& param )
{
    bool retval = false;
    string responseCode;
    extractParamValue( param, responseCode ); 

    ATRACE("%s: Searching for %s\n", __FUNCTION__, responseCode.c_str());
    if( body.find(responseCode) != body.npos )
    {
        ATRACE("%s: %s FOUND\n", __FUNCTION__, responseCode.c_str());
        retval = true;
    }
    else
    {
        ATRACE("%s: %s not found\n", __FUNCTION__, responseCode.c_str());
        stringstream error;
        error << responseCode << " not found\n";
        error << "response: " << body;
        gWriter.setError( error.str() );
    }
    return retval;
}

void DialConformance::getPayload(vector<string>& params, string& payload )
{
    vector<string>::iterator it;
    for( it = params.begin(); it < params.end(); it++ )
    {
        if( (*it).find("param") != (*it).npos )
        {
            // Check to see if the param is using quotes.  If it is, split on
            // that.  If not, split on a space
            size_t posEnd, pos = (*it).find( "=" );
            if( (*it)[pos+1] == '\"' )
            {
                posEnd = (*it).find( "\"", pos+2 );
                payload = (*it).substr( pos+2, posEnd-(pos+2) );
            }
            else
            {
                posEnd = (*it).find( " ", pos );
                payload = (*it).substr( pos+1, posEnd );
            }
            ATRACE("payload = %s\n", payload.c_str());
        }
    }
}

bool DialConformance::execute_launch( Application* pApp, vector<string>& params )
{
    string payload, responseHeaders, responseBody;
    getPayload( params, payload );
    pApp->launch( payload, responseHeaders, responseBody );
    return (validateParams( params, responseHeaders, responseBody ));
}

bool DialConformance::execute_status( Application* pApp, vector<string>& params )
{
    ATRACE("%s::%d\n", __FUNCTION__, __LINE__);
    string responseHeaders, responseBody;
    pApp->status( responseHeaders, responseBody );

    bool retval = validateParams( params, responseHeaders, responseBody );
    if( retval && (responseHeaders.find("200") != responseHeaders.npos) )
    {
        // if the status was successful
        // Ensure the response body contains the application name
        if( responseBody.find( pApp->getName() ) == responseBody.npos )
        {
            retval = false;
            stringstream error;
            error << "Reponse XML did not contain application name: " << pApp->getName();
            gWriter.setError( error.str() );
            retval = false;
        }
        if( responseBody.find( "urn:dial-multiscreen-org:schemas:dial" ) 
                    == responseBody.npos )
        {
            gWriter.setError( "Response Body did not contain DIAL service "
                    "string: urn:dial-multiscreen-org:schemas:dial" );
            retval = false;
        }
    }
    return retval;
}

bool DialConformance::execute_stop( Application* pApp, vector<string>& params )
{
    ATRACE("%s::%d\n", __FUNCTION__, __LINE__);
    string responseHeaders, responseBody;
    pApp->stop(responseHeaders);
    return (validateParams( params, responseHeaders, responseBody ));
}

DialConformance::Application* DialConformance::getApplication( string& command )
{
    Application* pApp = NULL;

    // extract the application
    string app;
    size_t pos = command.find("=");
    app = command.substr(pos+1, command.length());
    assert( app.length() > 0 );

    // find the application
    vector<Application*>::iterator it;
    for( it = _apps.begin(); it < _apps.end(); it++ )
    {
        ATRACE("Comparing: %s.compare(%s)\n", (*it)->getName().c_str(), app.c_str());
        if( (*it)->getName().compare(app) == 0)
        {
            pApp = (*it);
            break;
        }
    }

    assert(pApp != NULL);
    return pApp;
}

bool DialConformance::execute_command( string& command, vector<string>& params )
{
    ATRACE("%s::%s\n", __FUNCTION__, command.c_str());
    bool retval = false;
    if( command.find("launch") != command.npos )
        retval = execute_launch(getApplication(command), params);
    else if ( command.find("status") != command.npos )
        retval = execute_status(getApplication(command), params);
    else if ( command.find("stop") != command.npos )
        retval = execute_stop(getApplication(command), params);
    else if ( command.find("sleep") != command.npos ) 
    {
        string time;
        size_t pos = command.find("=");
        time = command.substr(pos+1, command.length());
        unsigned long milliseconds = atoi(time.c_str());
        ATRACE("Sleeping %ld milliseconds \n", milliseconds);
        usleep(milliseconds*1000);
        retval = true;
    }
    else ATRACE("Can't execute command: %s\n", command.c_str());
    return retval;
}

void DialConformance::run_internal( DialServer* pServer )
{
    string command;
    vector<string> params;
    bool retval;
    string friendlyName, uuid;

    // only execute the test if we have a friendly name and UUID
    retval = pServer->getFriendlyName( friendlyName );
    if (retval) pServer->getUuid( uuid );
    if (retval) gWriter.start( friendlyName, uuid, pServer->getIpAddress() );
    while( retval && _input.getNextAction(command, params) )
    {
        vector<string> commandlist;

        // Check the command here, if it is command=ALL, then we need to loop through
        // all of the valid applications
        size_t pos;
        if( (pos = command.find("=ALL")) != command.npos )
        {
            vector<Application*>::iterator it;
            for( it = _apps.begin(); it < _apps.end(); it++ )
            {
                // strip off ALL and append the application name
                string newCommand = command.substr(0, pos+1);
                newCommand.append( (*it)->getName() );
                Application *pApp = getApplication( newCommand );
                if( pApp != NULL && (!pApp->isErrorApp()) )
                {
                    ATRACE("Adding command %s\n", newCommand.c_str());
                    commandlist.push_back( newCommand );
                }

                if( newCommand.find("launch") != newCommand.npos )
                {
                    // If we are adding launch commands, insert a sleep so that we don't overwhelm
                    // the target
                    stringstream sleep;
                    sleep << "sleep=" << LAUNCH_SLEEP;
                    commandlist.push_back( sleep.str() );
                }
            }
        }
        else
        {
            ATRACE("Adding command %s\n", command.c_str());
            commandlist.push_back(command);
        }

        vector<string>::iterator it;
        for( it = commandlist.begin(); it < commandlist.end(); it++ )
        {
            // construct the name of the test
            ATRACE("***********************" "\n%s: Command: %s \n", 
                    __FUNCTION__, (*it).c_str());
            stringstream test;
            test << "Command:" << (*it);
    
            // The input code will always push a parameter.  If there are no
            // parameters, it will push an empty string, so test for that here.
            if( params.size() > 0 && !params[0].empty() ) test << " Params:";
    
            vector<string>::iterator it2;
            for( it2 = params.begin(); it2 < params.end(); it2++)
            {
                ATRACE("param: %s ", (*it2).c_str());

                // check to see if we have a really large parameter, if we do, 
                // truncate it to a reasonable length
                string param = (*it2);
                if( param.size() > MAX_PARAMETER_LENGTH )
                {
                    param = (*it2).substr(0, MAX_PARAMETER_LENGTH-3 );
                    param.append("...");
                }
                test << param << " ";
            }

            if( (*it).find("sleep") == (*it).npos )
            {
                // set the test name
                gWriter.setTest( test.str() );
    
                // run the test
                bool retval = execute_command( *it, params );
                printf("%s\n", retval ? "SUCCESS":"FAILED");
        
                // set the result and prep for the next test
                gWriter.setResult( retval );
                gWriter.clearTest();
            }
            else
            {
                //just sleep here
                string time;
                size_t pos = (*it).find("=");
                time = (*it).substr(pos+1, command.length());
                unsigned long milliseconds = atoi(time.c_str());
                ATRACE("Sleeping %ld milliseconds \n", milliseconds);
                usleep(milliseconds*1000);
            }
        }

        // clear the parameters for the next loop
        params.clear();
    }
    if( retval ) gWriter.complete();
}

int DialConformance::run(
        DialServer* pServer,
        vector<string>& appList,
        string &inputFile,
        string &outputFile )
{
    printf("Running conformance tests from %s and printing report to %s\n\n",
                    inputFile.empty() ? 
                              DialClientInput::getDefaultFilename().c_str() : 
                              inputFile.c_str(), 
                    outputFile.empty() ? 
                              defaultOutputFile.c_str() : 
                              outputFile.c_str() );
    //read the input file
    _input.init(inputFile);
    if( !outputFile.empty() ) gWriter.setOutputFile( outputFile );

    // Add applications passed by the client (command line)
    vector<string>::iterator it;
    for( it = appList.begin(); it < appList.end(); it++ ) _input.addApplication(*it);

    // build the application list
    vector<string> listOfApps;
    _input.getApplicationList(listOfApps); // get the list of apps
    for( it = listOfApps.begin(); it < listOfApps.end(); it++ )
    {
        _apps.push_back( new Application(*it, pServer, false) );
        ATRACE("Adding application: %s\n", (*it).c_str());
    }

    _input.getErrorApplicationList(listOfApps); // get the list of apps
    for( it = listOfApps.begin(); it < listOfApps.end(); it++ )
    {
        _apps.push_back( new Application(*it, pServer, true) );
        ATRACE("Adding Error application: %s\n", (*it).c_str());
    }

    // run the test
    run_internal( pServer );

    // clean up the application list
    vector<Application*>::iterator appit;
    for ( appit = _apps.begin() ; appit < _apps.end(); appit++ ) delete (*appit);

    return 0;
}

