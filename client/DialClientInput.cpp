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
#include "DialClientInput.h"
#include "DialServer.h"
#include <iostream>
#include <fstream>

using namespace std;

bool DialClientInput::init(std::string file)
{
    if( file.empty() ) file = DialClientInput::getDefaultFilename();
    ATRACE("DialClientInput::%s, opening %s\n", __FUNCTION__, file.c_str());
    string line;
    ifstream myfile (file.c_str());
    if (myfile.is_open())
    {
        // first fill _actions with commands from the input file
        while ( myfile.good() )
        {
            getline (myfile,line);
            if( line[0] != '#' && !line.empty() )
            {
                if( line.find("addApplication") != line.npos )
                {
                    // add an application
                    size_t pos = line.find_first_of('=');
                    _applist.push_back( line.substr(pos+1, line.length()));
                    ATRACE("Adding app: %s\n", line.substr(pos+1, line.length()).c_str());
                }
                else if( line.find("addErrorApplication") != line.npos )
                {
                    // add an application
                    size_t pos = line.find_first_of('=');
                    _errorapplist.push_back( line.substr(pos+1, line.length()));
                    ATRACE("Adding error app: %s\n", line.substr(pos+1, line.length()).c_str());
                }
                else
                {
                    // add a command
                    size_t pos = line.find_first_of(" ");
                    std::string params = line.substr(pos+1, line.length());
                    std::pair<std::string, std::string> 
                            action_to_push(line.substr(0, pos), 
                            pos == line.npos ? "":params);
                    _actions.push_back(action_to_push);
                    ATRACE("command: %s params: %s\n", 
                            line.substr(0, pos).c_str(), params.c_str() );
                }
            }
#ifdef DEBUG
            //else ATRACE("COMMENT: %s\n", line.c_str());
#endif
        }
        myfile.close();
    }
    else 
    {
        fprintf(stderr, "Unable to open file\n");
        return false;
    }
    return true;
}

bool DialClientInput::addApplication( string& application )
{
    // see if the application exits
    vector<string>::iterator it;
    for( it = _applist.begin(); it < _applist.end(); it ++ )
        if( !((*it).compare( application )) ) break;

    // if not, add it
    if( it < _applist.end() ) _applist.push_back(application);
    else return false;  // already in the list

    return true;
}

bool DialClientInput::getNextAction( string& command, vector<string>& parameters )
{
    if (_actions.empty()) return false;

    pair<string, string> action = _actions.front();
    _actions.erase(_actions.begin());

    command = action.first;
    string params = action.second;
    size_t pos = params.find_first_of(" "), start = 0;
    // Parse out the parameters from the string
    while (1)
    {
        parameters.push_back(params.substr( start, pos-start ));
        if( pos == params.npos ) break;
        start = pos+1;
        pos = params.find_first_of(" ", start);
    }

    return true;
}

void DialClientInput::getApplicationList( vector<string> &list )
{
    list = _applist;
}


void DialClientInput::getErrorApplicationList(vector<string> &list)
{
    list = _errorapplist;
}

