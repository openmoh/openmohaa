/*
===========================================================================
Copyright (C) 2025 the OpenMoHAA team

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

#include "sys_curl.h"
#include "sys_loadlib.h"
#include "../qcommon/qcommon.h"

curlImport_t curlImport;

static void    *cURLLib = NULL;
static qboolean cURLValid;
cvar_t         *sys_curllib = NULL;

#ifdef WIN32
#    define DEFAULT_CURL_LIB   "libcurl.dll"
#    define ALTERNATE_CURL_LIB "libcurl-3.dll"
#elif defined(__APPLE__)
#    define DEFAULT_CURL_LIB "libcurl.4.dylib"
#    define ALTERNATE_CURL_LIB "libcurl.3.dylib"
#else
#    define DEFAULT_CURL_LIB   "libcurl.so.4"
#    define ALTERNATE_CURL_LIB "libcurl.so.3"
#endif

/*
=================
GPA
=================
*/
static void *GPA(char *str)
{
    void *rv;

    rv = Sys_LoadFunction(cURLLib, str);
    if (!rv) {
        Com_Printf("Can't load symbol %s\n", str);
        cURLValid = qfalse;
        return NULL;
    } else {
        Com_DPrintf("Loaded symbol %s (0x%p)\n", str, rv);
        return rv;
    }
}

void Sys_InitCurl()
{
#ifdef HAS_LIBCURL
    if (cURLLib) {
        return;
    }

    sys_curllib = Cvar_Get("sys_curllib", DEFAULT_CURL_LIB, 0);

    Com_Printf("Loading \"%s\"...\n", sys_curllib->string);
    if (!(cURLLib = Sys_LoadDll(sys_curllib->string, qtrue))) {
#ifdef ALTERNATE_CURL_LIB
        // On some linux distributions there is no libcurl.so.3, but only libcurl.so.4. That one works too.
        if (!(cURLLib = Sys_LoadDll(ALTERNATE_CURL_LIB, qtrue)))
#endif
            return;
    }

    cURLValid = qtrue;

    curlImport.qcurl_version = GPA("curl_version");

    curlImport.qcurl_getenv              = GPA("curl_getenv");
    curlImport.qcurl_free                = GPA("curl_free");
    curlImport.qcurl_slist_append        = GPA("curl_slist_append");
    curlImport.qcurl_slist_free_all      = GPA("curl_slist_free_all");
    curlImport.qcurl_version_info        = GPA("curl_version_info");
    curlImport.qcurl_easy_strerror       = GPA("curl_easy_strerror");
    curlImport.qcurl_share_strerror      = GPA("curl_share_strerror");
    curlImport.qcurl_easy_pause          = GPA("curl_easy_pause");
    curlImport.qcurl_easy_escape         = GPA("curl_easy_escape");
    curlImport.qcurl_easy_unescape       = GPA("curl_easy_unescape");
    curlImport.qcurl_easy_init           = GPA("curl_easy_init");
    curlImport.qcurl_easy_setopt         = GPA("curl_easy_setopt");
    curlImport.qcurl_easy_perform        = GPA("curl_easy_perform");
    curlImport.qcurl_easy_cleanup        = GPA("curl_easy_cleanup");
    curlImport.qcurl_easy_getinfo        = GPA("curl_easy_getinfo");
    curlImport.qcurl_easy_duphandle      = GPA("curl_easy_duphandle");
    curlImport.qcurl_easy_reset          = GPA("curl_easy_reset");
    curlImport.qcurl_easy_recv           = GPA("curl_easy_recv");
    curlImport.qcurl_easy_send           = GPA("curl_easy_send");
    curlImport.qcurl_easy_upkeep         = GPA("curl_easy_upkeep");
    curlImport.qcurl_multi_init          = GPA("curl_multi_init");
    curlImport.qcurl_multi_add_handle    = GPA("curl_multi_add_handle");
    curlImport.qcurl_multi_remove_handle = GPA("curl_multi_remove_handle");
    curlImport.qcurl_multi_fdset         = GPA("curl_multi_fdset");
    curlImport.qcurl_multi_wait          = GPA("curl_multi_wait");
    curlImport.qcurl_multi_poll          = GPA("curl_multi_poll");
    curlImport.qcurl_multi_wakeup        = GPA("curl_multi_wakeup");
    curlImport.qcurl_multi_perform       = GPA("curl_multi_perform");
    curlImport.qcurl_multi_cleanup       = GPA("curl_multi_cleanup");
    curlImport.qcurl_multi_info_read     = GPA("curl_multi_info_read");
    curlImport.qcurl_multi_strerror      = GPA("curl_multi_strerror");
    curlImport.qcurl_multi_socket_action = GPA("curl_multi_socket_action");
    curlImport.qcurl_multi_timeout       = GPA("curl_multi_timeout");
    curlImport.qcurl_multi_setopt        = GPA("curl_multi_setopt");
    curlImport.qcurl_multi_assign        = GPA("curl_multi_assign");
    //curlImport.qcurl_multi_get_handles   = GPA("curl_multi_get_handles");
    curlImport.qcurl_pushheader_bynum    = GPA("curl_pushheader_bynum");
    curlImport.qcurl_pushheader_byname   = GPA("curl_pushheader_byname");
    //curlImport.qcurl_multi_waitfds       = GPA("curl_multi_waitfds");

    if (!cURLValid) {
        Sys_ShutdownCurl();
    }
#endif
}

void Sys_ShutdownCurl()
{
    if (cURLLib) {
        Com_Printf("Unloading cURL...\n");

        Sys_UnloadLibrary(cURLLib);
        cURLLib   = NULL;
        cURLValid = qfalse;
        memset(&curlImport, 0, sizeof(curlImport));
    }
}
