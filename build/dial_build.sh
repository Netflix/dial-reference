#!/bin/bash -e
#
# (c) 1997-2012 Netflix, Inc.  All content herein is protected by
# U.S. copyright and other applicable intellectual property laws and
# may not be copied without the express permission of Netflix, Inc.,
# which reserves all rights.  Reuse of any of this content for any
# purpose without the permission of Netflix, Inc. is strictly
# prohibited.

BUILD_PREFIX="DIAL"
BUILD_MAJOR=
BUILD_MINOR=
BUILD_PATCH=
BUILD_SUFFIX=
BUILD_NUMBER=
COPY_SOURCE_VERSION_H=

if [ -n "${P4_CHANGELIST}" ]; then
  #echo "Using information from Jenkins environment to determine RELEASE number: $P4_CHANGELIST"
  BUILD_NUMBER="${P4_CHANGELIST}"
fi

if [ -z "$COPY_SOURCE_VERSION_H" ]; then
    VERSION_H="Version.h"
    COPY_SOURCE_VERSION_H=`find ${COPY_SOURCE_DIR} -name ${VERSION_H}`
    if [ -z "$COPY_SOURCE_VERSION_H" ]; then
        echo "${VERSION_H} cannot be found!"
        exit 1
    elif echo "$COPY_VERSION" | grep --quiet ' '; then
        echo "Too many ${VERSION_H} found!"
        exit 1
    fi
fi

BUILD_MAJOR=`grep -m1 -i "^# *define *${BUILD_PREFIX}_VERSION_MAJOR" "$COPY_SOURCE_VERSION_H" | sed "s,/[/*].*$,," | awk '{print $(NF)}'`
if [ -z "$BUILD_MAJOR" ]; then
    echo "Failed to determine version major of current source." >&2
    exit 1
fi

if [ -z "$BUILD_MINOR" ]; then
    BUILD_MINOR=`grep -m1 -i "^# *define *${BUILD_PREFIX}_VERSION_MINOR" "$COPY_SOURCE_VERSION_H" | sed "s,/[/*].*$,," | awk '{print $(NF)}'`
    if [ -z "$BUILD_MINOR" ]; then
        echo "Failed to determine version minor of current source." >&2
        exit 1
    fi
fi

if [ -z "$BUILD_PATCH" ]; then
    BUILD_PATCH=`grep -m1 -i "^# *define *${BUILD_PREFIX}_VERSION_PATCH" "$COPY_SOURCE_VERSION_H" | sed "s,/[/*].*$,," | awk '{print $(NF)}'`
    if [ -z "$BUILD_PATCH" ]; then
        echo "Failed to determine version minor of current source." >&2
        exit 1
    fi
fi

BUILD_STRING="${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_PATCH}-${BUILD_NUMBER}"
echo "${BUILD_STRING}"

tar czvhf ${WORKSPACE}/DIAL-$BUILD_STRING.tar.gz *

# Package the binaries
cp ${WORKSPACE}/src/dial/client/dialclient ${WORKSPACE}/build
cp ${WORKSPACE}/src/dial/server/dialserver ${WORKSPACE}/build
cd ${WORKSPACE}/build
tar czvhf ${WORKSPACE}/DIAL-$BUILD_STRING-binaries.tar.gz *
