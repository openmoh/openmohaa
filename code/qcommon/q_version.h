/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

This file is part of OpenMoHAA source code.

OpenMoHAA source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMoHAA source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMoHAA source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// q_version.h -- file versioning

#pragma once

#include <q_version.generated.h>

//
// Version
// 
// These values are the only one that must be set for the version
#define PRODUCT_VERSION_MAJOR			0
#define PRODUCT_VERSION_MINOR			60
#define PRODUCT_VERSION_PATCH			0
#define PRODUCT_VERSION_SUFFIX			"alpha"

//
// Generated version info
//

#ifdef GIT_REVISION_BUILD_NUMBER
#   define PRODUCT_VERSION_BUILD_NUMBER GIT_REVISION_BUILD_NUMBER
#else
#   define PRODUCT_VERSION_BUILD_NUMBER 0
#endif

#ifdef GIT_BRANCH_NAME
#   define PRODUCT_VERSION_BRANCH GIT_BRANCH_NAME
#endif

#ifdef GIT_REVISION_HASH
#   define PRODUCT_VERSION_REVISION GIT_REVISION_HASH
#endif

#ifdef GIT_REVISION_HASH_ABBREVIATED
#   define PRODUCT_VERSION_REVISION_SHORT GIT_REVISION_HASH_ABBREVIATED
#endif

#ifdef GIT_REVISION_DATE
#   define PRODUCT_VERSION_DATE GIT_REVISION_DATE
#else
#   define PRODUCT_VERSION_DATE __DATE__
#endif

#define Q3_VERSION PRODUCT_VERSION_FULL

//
// Version display
//
#define PRODUCT_VERSION_NUMBER_STRING	XSTRING(PRODUCT_VERSION_MAJOR) "." XSTRING(PRODUCT_VERSION_MINOR) "." XSTRING(PRODUCT_VERSION_PATCH)

#define PRODUCT_VERSION                 PRODUCT_VERSION_NUMBER_STRING

#ifdef PRODUCT_VERSION_SUFFIX
#   define PRODUCT_VERSION_1        PRODUCT_VERSION "-" PRODUCT_VERSION_SUFFIX
#else
#   define PRODUCT_VERSION_1        PRODUCT_VERSION
#endif

#   define PRODUCT_VERSION_2        PRODUCT_VERSION_1 "+" XSTRING(PRODUCT_VERSION_BUILD_NUMBER)

#ifdef PRODUCT_VERSION_REVISION
#   define PRODUCT_VERSION_3        PRODUCT_VERSION_2 "." PRODUCT_VERSION_REVISION_SHORT
#else
#   define PRODUCT_VERSION_3        PRODUCT_VERSION_2
#endif

#define PRODUCT_VERSION_FULL        PRODUCT_VERSION_3

#define PRODUCT_NAME_FULL		    PRODUCT_NAME " - v" PRODUCT_VERSION_1

/*
//
// Version
// 
// These values are the only one that must be set for the version
extern const unsigned int PRODUCT_VERISON_MAJOR;
extern const unsigned int PRODUCT_VERISON_MINOR;
extern const unsigned int PRODUCT_VERISON_PATCH;
extern const char* PRODUCT_VERSION_STAGE;

extern const char* PRODUCT_VERSION_BRANCH;
extern const char* PRODUCT_VERSION_REVISION_HASH;
extern const char* PRODUCT_VERSION_REVISION_DATE;
*/
