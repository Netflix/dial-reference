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

#ifndef DIAL_OPTIONS_H
#define DIAL_OPTIONS_H

#define DATA_PATH_OPTION "-D"
#define DATA_PATH_OPTION_LONG "--data-path"
#define DATA_PATH_DESCRIPTION "Path to netflix secure store"

#define NETFLIX_PATH_OPTION "-N"
#define NETFLIX_PATH_OPTION_LONG "--netflix-path"
#define NETFLIX_PATH_DESCRIPTION "Path to Netflix application"

#define FRIENDLY_NAME_OPTION "-F"
#define FRIENDLY_NAME_OPTION_LONG "--friendly-name"
#define FRIENDLY_NAME_DESCRIPTION "Device Friendly Name"

#define MODELNAME_OPTION "-M"
#define MODELNAME_OPTION_LONG "--model-name"
#define MODELNAME_DESCRIPTION "Model name of the device"

#define UUID_OPTION "-U"
#define UUID_OPTION_LONG "--uuid-name"
#define UUID_DESCRIPTION "UUID of the device"

struct dial_options
{
    const char * pOption;
    const char * pLongOption;
    const char * pOptionDescription;
}dial_options_t;

struct dial_options gDialOptions[] = 
{
    {
        DATA_PATH_OPTION,
        DATA_PATH_OPTION_LONG,
        DATA_PATH_DESCRIPTION
    },
    {
        NETFLIX_PATH_OPTION,
        NETFLIX_PATH_OPTION_LONG,
        NETFLIX_PATH_DESCRIPTION
    },
    {
        FRIENDLY_NAME_OPTION,
        FRIENDLY_NAME_OPTION_LONG,
        FRIENDLY_NAME_DESCRIPTION,
    },
    {
        MODELNAME_OPTION,
        MODELNAME_OPTION_LONG,
        MODELNAME_DESCRIPTION
    },
    {
        UUID_OPTION,
        UUID_OPTION_LONG,
        UUID_DESCRIPTION
    }
};

#endif

