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

#ifndef DIALCLIENTINPUT_H
#define DIALCLIENTINPUT_H

#include <string>
#include <vector>

using namespace std;

class DialClientInput
{
public:
    DialClientInput() {}
    ~DialClientInput() {}

    /**
     * Called to initialize
     * 
     * @param[in] file Name of the file to load
     * @return true if successful, false otherwise
     */
    bool init( string file );

    /**
     * The DialClient application can use this to add applications passed on 
     * the command line. If the application is already in the list, it will 
     * drop the add.
     * 
     * @param[in] application Name of the application to add.
     *
     * @return true if successful, false otherwise
     */
    bool addApplication( string &application );

    /**
     * Get the next action to execute
     * 
     * @param[out] command Command to execute
     * @param[out] parameters Parameters for the command
     */
    bool getNextAction( string& command, vector<string>& parameters );

    /**
     * Get a list of valid applications defined in the input file.
     * 
     * @param[out] list List of valid applications
     */
    void getApplicationList(vector<string> &list);

    /**
     * Get a list of applications used for error tests (should not exist).
     * 
     * @param[out] list List of error applications
     */
    void getErrorApplicationList(vector<string> &list);

    static string getDefaultFilename() { return "./dialclient_input.txt"; }

private:
    vector<string>                  _applist;
    vector<string>                  _errorapplist;
    vector< pair<string,string> >   _actions;
};

#endif // DIALCLIENTINPUT_H
