/*
 * (c) 1997-2015 Netflix, Inc.  All content herein is protected by
 * U.S. copyright and other applicable intellectual property laws and
 * may not be copied without the express permission of Netflix, Inc.,
 * which reserves all rights.  Reuse of any of this content for any
 * purpose without the permission of Netflix, Inc. is strictly
 * prohibited.
 */

#ifndef DIAL_VERSION_H
#define DIAL_VERSION_H

#undef DIAL_VERSION_MAJOR
#define DIAL_VERSION_MAJOR 1

#undef DIAL_VERSION_MINOR
#define DIAL_VERSION_MINOR 0

#undef DIAL_VERSION_PATCH
#define DIAL_VERSION_PATCH 5

#undef DIAL_VERSION_NUMBER_STR2
#define DIAL_VERSION_NUMBER_STR2(M) #M
#undef DIAL_VERSION_NUMBER_STR
#define DIAL_VERSION_NUMBER_STR(M) DIAL_VERSION_NUMBER_STR2(M)

#undef DIAL_VERSION_NUMBER
#define DIAL_VERSION_NUMBER DIAL_VERSION_NUMBER_STR(DIAL_VERSION_MAJOR) "." DIAL_VERSION_NUMBER_STR(DIAL_VERSION_MINOR) "." DIAL_VERSION_NUMBER_STR(DIAL_VERSION_PATCH)

#undef DIAL_VERSION_STRING
#ifdef DIAL_VERSION_SUFFIX
# define DIAL_VERSION_STRING DIAL_VERSION_NUMBER "-" DIAL_VERSION_SUFFIX
#else
# define DIAL_VERSION_STRING DIAL_VERSION_NUMBER
#endif


#endif
