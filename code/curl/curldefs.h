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

#pragma once

#include "../qcommon/q_shared.h"

#ifdef HAS_LIBCURL

#    include <stdint.h>
#    include <stdlib.h>
#    include <curl/curl.h>

//
// This structure is guaranteed to work on cURL 7.88.1
//
typedef struct {
    char *(*qcurl_version)(void);
    char *(*qcurl_getenv)(const char *variable);
    void (*qcurl_free)(void *p);
    struct curl_slist *(*qcurl_slist_append)(struct curl_slist *list, const char *data);
    void (*qcurl_slist_free_all)(struct curl_slist *list);
    struct curl_version_info_data *(*qcurl_version_info)(CURLversion);
    const char *(*qcurl_easy_strerror)(CURLcode);
    const char *(*qcurl_share_strerror)(CURLSHcode);
    CURLcode (*qcurl_easy_pause)(CURL *handle, int bitmask);
    char *(*qcurl_easy_escape)(CURL *handle, const char *string, int length);
    char *(*qcurl_easy_unescape)(CURL *handle, const char *string, int length, int *outlength);
    CURL *(*qcurl_easy_init)(void);
    CURLcode (*qcurl_easy_setopt)(CURL *curl, CURLoption option, ...);
    CURLcode (*qcurl_easy_perform)(CURL *curl);
    void (*qcurl_easy_cleanup)(CURL *curl);
    CURLcode (*qcurl_easy_getinfo)(CURL *curl, CURLINFO info, ...);
    CURL *(*qcurl_easy_duphandle)(CURL *curl);
    void (*qcurl_easy_reset)(CURL *curl);
    CURLcode (*qcurl_easy_recv)(CURL *curl, void *buffer, size_t buflen, size_t *n);
    CURLcode (*qcurl_easy_send)(CURL *curl, const void *buffer, size_t buflen, size_t *n);
    CURLcode (*qcurl_easy_upkeep)(CURL *curl);
    CURLM *(*qcurl_multi_init)(void);
    CURLMcode (*qcurl_multi_add_handle)(CURLM *multi_handle, CURL *curl_handle);
    CURLMcode (*qcurl_multi_remove_handle)(CURLM *multi_handle, CURL *curl_handle);
    CURLMcode (*qcurl_multi_fdset)(
        CURLM *multi_handle, fd_set *read_fd_set, fd_set *write_fd_set, fd_set *exc_fd_set, int *max_fd
    );
    CURLMcode (*qcurl_multi_wait)(
        CURLM *multi_handle, struct curl_waitfd extra_fds[], unsigned int extra_nfds, int timeout_ms, int *ret
    );
    CURLMcode (*qcurl_multi_poll)(
        CURLM *multi_handle, struct curl_waitfd extra_fds[], unsigned int extra_nfds, int timeout_ms, int *ret
    );
    CURLMcode (*qcurl_multi_wakeup)(CURLM *multi_handle);
    CURLMcode (*qcurl_multi_perform)(CURLM *multi_handle, int *running_handles);
    CURLMcode (*qcurl_multi_cleanup)(CURLM *multi_handle);
    CURLMsg *(*qcurl_multi_info_read)(CURLM *multi_handle, int *msgs_in_queue);
    const char *(*qcurl_multi_strerror)(CURLMcode);
    CURLMcode (*qcurl_multi_socket_action)(CURLM *multi_handle, curl_socket_t s, int ev_bitmask, int *running_handles);
    CURLMcode (*qcurl_multi_timeout)(CURLM *multi_handle, long *milliseconds);
    CURLMcode (*qcurl_multi_setopt)(CURLM *multi_handle, CURLMoption option, ...);
    CURLMcode (*qcurl_multi_assign)(CURLM *multi_handle, curl_socket_t sockfd, void *sockp);
    // 8.4.0
    //CURL **(*qcurl_multi_get_handles)(CURLM *multi_handle);
    char *(*qcurl_pushheader_bynum)(struct curl_pushheaders *h, size_t num);
    char *(*qcurl_pushheader_byname)(struct curl_pushheaders *h, const char *name);
    // 8.8.0
    //CURLMcode (*qcurl_multi_waitfds)(CURLM *multi, struct curl_waitfd *ufds, unsigned int size, unsigned int *fd_count);
} curlImport_t;

static inline qboolean Com_IsCurlImportValid(curlImport_t *import)
{
    return import && import->qcurl_version != NULL;
}

#else

//
// No cURL at all
//

typedef void *curlImport_t;

static inline qboolean Com_IsCurlImportValid(curlImport_t *import)
{
    return qfalse;
}

#endif