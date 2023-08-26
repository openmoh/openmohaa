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

#include "../qcommon/q_shared.h"
#include "sv_gqueryreporting.h"

#include "common/gsPlatformSocket.h"
#include "common/gsPlatformUtil.h"
#include "gutil.h"

static char *queries[] = {"basic", "info", "rules", "players", "status", "packets", "echo", "secure"};

static qr_implementation_t  static_rec;
static qr_implementation_t *current_rec = &static_rec;
struct sockaddr_in          hbaddr;
char                        qr_hostname[64];

void        qr_check_queries(qr_t qrec);
void        qr_check_send_heartbeat(qr_t qrec);
static void parse_query(qr_t qrec, char *query, struct sockaddr *sender);
static void send_heartbeat(qr_t qrec, int statechanged);

void qr_process_queries(qr_t qrec)
{
    if (!qrec) {
        qrec = current_rec;
    }

    qr_check_send_heartbeat(qrec);
    qr_check_queries(qrec);
}

void qr_process_queries_no_heartbeat(qr_t qrec)
{
    if (!qrec) {
        qrec = current_rec;
    }

    qr_check_queries(qrec);
}

void qr_check_queries(qr_t qrec)
{
#define INBUF_LEN 256
    static char        indata[INBUF_LEN]; //256 byte input buffer
    struct sockaddr_in saddr;
    int                error;

#if defined(_LINUX)
    unsigned int saddrlen = sizeof(struct sockaddr_in);
#else
    int saddrlen = sizeof(struct sockaddr_in);
#endif

    while (CanReceiveOnSocket((SOCKET)qrec->hbsock)) {
        //else we have data
        error = (int)recvfrom((SOCKET)qrec->hbsock, indata, (INBUF_LEN - 1), 0, (struct sockaddr *)&saddr, &saddrlen);

        if (gsiSocketIsNotError(error)) {
            indata[error] = '\0';
            parse_query(qrec, indata, (struct sockaddr *)&saddr);
        }
    }
}

void qr_check_send_heartbeat(qr_t qrec)
{
    unsigned long tc;

    tc = current_time();

    if ((SOCKET)qrec->hbsock != INVALID_SOCKET) {
        if (tc - qrec->lastheartbeat <= 300000 && qrec->lastheartbeat && tc >= qrec->lastheartbeat) {
            if (qrec->no_query > 0 && tc - qrec->lastheartbeat > 30000) {
                send_heartbeat(qrec, 0);
                if (qrec->no_query++ > 10) {
                    qrec->no_query = 0;
                }
            }
        } else {
            send_heartbeat(qrec, 0);
        }
    }
}

void qr_send_statechanged(qr_t qrec)
{
    if (!qrec) {
        qrec = current_rec;
    }
    send_heartbeat(qrec, 1);
}

void qr_shutdown(qr_t qrec)
{
    if (qrec == NULL) {
        qrec = current_rec;
    }
    if (INVALID_SOCKET != (SOCKET)qrec->hbsock && (SOCKET)qrec->querysock) //if we own the socket
    {
        closesocket((SOCKET)qrec->hbsock);
    }
    qrec->hbsock        = (void *)INVALID_SOCKET;
    qrec->lastheartbeat = 0;
    if (qrec->querysock) //if we own the socket
    {
        SocketShutDown();
    }
    if (qrec != &static_rec) //need to free it, it was dynamically allocated
    {
        free(qrec);
    }
}

/* Return a sockaddrin for the given host (numeric or DNS) and port)
Returns the hostent in savehent if it is not NULL */
static int get_sockaddrin(const char *host, int port, struct sockaddr_in *saddr, struct hostent **savehent)
{
    struct hostent *hent = NULL;

    saddr->sin_family = AF_INET;
    saddr->sin_port   = htons((unsigned short)port);
    if (host == NULL) {
        saddr->sin_addr.s_addr = INADDR_ANY;
    } else {
        saddr->sin_addr.s_addr = inet_addr(host);
    }

    if (saddr->sin_addr.s_addr == INADDR_NONE && strcmp(host, "255.255.255.255") != 0) {
        hent = gethostbyname(host);
        if (!hent) {
            return 0;
        }
        saddr->sin_addr.s_addr = *(unsigned int *)hent->h_addr_list[0];
    }
    if (savehent != NULL) {
        *savehent = hent;
    }
    return 1;
}

/* value_for_key: this returns a value for a certain key in s, where s is a string
containing key\value pairs. If the key does not exist, it returns  NULL.
Note: the value is stored in a common buffer. If you want to keep it, make a copy! */
static char *value_for_key(const char *s, const char *key)
{
    static int  valueindex;
    char       *pos, *pos2;
    char        keyspec[256] = "\\";
    static char value[2][256];

    valueindex ^= 1;
    strcat(keyspec, key);
    strcat(keyspec, "\\");
    pos = strstr(s, keyspec);
    if (!pos) {
        return NULL;
    }
    pos += strlen(keyspec);
    pos2 = value[valueindex];
    while (*pos && *pos != '\\') {
        *pos2++ = *pos++;
    }
    *pos2 = '\0';
    return value[valueindex];
}

static void packet_send(qr_t qrec, struct sockaddr *addr, char *buffer)
{
    char keyvalue[80];

    if (!strlen(buffer)) {
        return;
    }

    sprintf(keyvalue, "\\queryid\\%d.%d", qrec->queryid, qrec->packetnumber++);
    strcat(buffer, keyvalue);

    sendto((SOCKET)qrec->querysock, buffer, (int)strlen(buffer), 0, addr, sizeof(*addr));
    *buffer = 0;
}

static void buffer_send(qr_t qrec, struct sockaddr *sender, char *buffer, char *newdata)
{
    int   bcount;
    int   i;
    char *lastkey;
    char *pos;

    bcount = 0;
    if (strlen(newdata) + strlen(buffer) < MAX_INFO_STRING) {
        strcat(buffer, newdata);
        return;
    }

    pos = newdata;
    while (strlen(pos) > MAX_INFO_STRING) {
        lastkey = pos;

        for (i = 0; i < MAX_INFO_STRING; ++i) {
            if (pos[i] == '\\') {
                if (!(bcount % 2)) {
                    lastkey = pos + i;
                }

                ++bcount;
            }
        }

        if (lastkey == pos) {
            return;
        }

        *lastkey = 0;
        buffer_send(qrec, sender, buffer, pos);
        *lastkey = '\\';
        pos      = lastkey;
        bcount   = 0;

        if (strlen(buffer) + strlen(lastkey) < MAX_INFO_STRING) {
            strcat(buffer, pos);
            return;
        }
    }

    packet_send(qrec, sender, buffer);
    strcpy(buffer, pos);
}

static void send_basic(qr_t qrec, struct sockaddr *sender, char *outbuf)
{
    char keyvalue[1400] = {0};

    qrec->qr_basic_callback(keyvalue, 1400, qrec->udata);
    buffer_send(qrec, sender, outbuf, keyvalue);
}

static void send_info(qr_t qrec, struct sockaddr *sender, char *outbuf)
{
    char keyvalue[1400] = {0};

    qrec->qr_info_callback(keyvalue, 1400, qrec->udata);
    buffer_send(qrec, sender, outbuf, keyvalue);
}

static void send_rules(qr_t qrec, struct sockaddr *sender, char *outbuf)
{
    char keyvalue[1400] = {0};

    qrec->qr_rules_callback(keyvalue, 1400, qrec->udata);
    buffer_send(qrec, sender, outbuf, keyvalue);
}

static void send_players(qr_t qrec, struct sockaddr *sender, char *outbuf)
{
    char keyvalue[1400] = {0};

    qrec->qr_players_callback(keyvalue, 1400, qrec->udata);
    buffer_send(qrec, sender, outbuf, keyvalue);
}

static void send_echo(qr_t qrec, struct sockaddr *sender, char *outbuf, const char *echostr)
{
    char keyvalue[1400] = {0};

    if (strlen(echostr) <= 50) {
        sprintf(keyvalue, "\\echoresponse\\%s", echostr);
        buffer_send(qrec, sender, outbuf, keyvalue);
    }
}

static void send_final(qr_t qrec, struct sockaddr *sender, char *outbuf, char *validation)
{
    char keyvalue[256];
    char encrypted_val[128];
    char encoded_val[200];
    int  keylen;

    if (*validation) {
        keylen = (int)strlen(validation);

        if (keylen > 128) {
            return;
        }

        strcpy(encrypted_val, validation);

        gs_encrypt((uchar *)qrec->secret_key, (int)strlen(qrec->secret_key), encrypted_val, keylen);
        gs_encode(encrypted_val, keylen, encoded_val);

        sprintf(keyvalue, "\\validate\\%s", encoded_val);
        buffer_send(qrec, sender, outbuf, keyvalue);
    }

    sprintf(keyvalue, "\\final\\");
    buffer_send(qrec, sender, outbuf, keyvalue);
    packet_send(qrec, sender, outbuf);
}

static void parse_query(qr_t qrec, char *query, struct sockaddr *sender)
{
    query_t     querytype    = qtunknown;
    char        buffer[1400] = {0};
    const char *value;
    char        validation[256] = {0};

    if (!qrec) {
        qrec = current_rec;
    }

    if (*query == ';') {
        // custom handler
        qrec->qr_custom_handler(query, sender);
        return;
    }

    qrec->packetnumber = 0;
    qrec->queryid++;

    if (qrec->no_query > 0) {
        qrec->no_query = 0;
    }

    for (querytype = qtbasic; querytype <= qtsecure; ++querytype) {
        value = value_for_key(query, queries[querytype - 1]);
        if (value) {
            switch (querytype) {
            case qtbasic:
                send_basic(qrec, sender, buffer);
                break;
            case qtinfo:
                send_info(qrec, sender, buffer);
                break;
            case qtrules:
                send_rules(qrec, sender, buffer);
                break;
            case qtplayers:
                send_players(qrec, sender, buffer);
                break;
            case qtstatus:
                send_basic(qrec, sender, buffer);
                send_info(qrec, sender, buffer);
                send_rules(qrec, sender, buffer);
                send_players(qrec, sender, buffer);
                break;
            case qtpackets:
                send_basic(qrec, sender, buffer);
                packet_send(qrec, sender, buffer);
                send_info(qrec, sender, buffer);
                packet_send(qrec, sender, buffer);
                send_rules(qrec, sender, buffer);
                packet_send(qrec, sender, buffer);
                send_players(qrec, sender, buffer);
                break;
            case qtecho:
                send_echo(qrec, sender, buffer, value);
                break;
            case qtsecure:
                strcpy(validation, value);
                break;
            default:
                continue;
            }
        }
    }

    send_final(qrec, sender, buffer, validation);
}

static void send_heartbeat(qr_t qrec, int statechanged)
{
    char buf[256];

    sprintf(buf, "\\heartbeat\\%d\\gamename\\%s", qrec->qport, qrec->gamename);

    if (statechanged) {
        sprintf(buf + strlen(buf), "\\statechanged\\%d", statechanged);
    }

    sendto((SOCKET)qrec->hbsock, buf, (int)strlen(buf), 0, (const struct sockaddr *)&qrec->master_saddr, sizeof(qrec->master_saddr));
    qrec->lastheartbeat = current_time();
}

static int do_connect(void *sock, char *addr, int port, struct sockaddr_in *master_saddr)
{
    get_sockaddrin(addr, port, master_saddr, NULL);
    return 0;
}

void init_qrec(
    qr_t              *qrec,
    int                baseport,
    SOCKET             hbsock,
    SOCKET             querysock,
    const char         *gamename,
    const char         *secret_key,
    qr_querycallback_t qr_basic_callback,
    qr_querycallback_t qr_info_callback,
    qr_querycallback_t qr_rules_callback,
    qr_querycallback_t qr_players_callback,
    void              *userdata
)
{
    qr_t qr;

    if (qrec) {
        qr    = (qr_t)malloc(sizeof(qr_implementation_t));
        *qrec = qr;
    } else {
        qr = &static_rec;
    }

    strcpy(qr->gamename, gamename);
    strcpy(qr->secret_key, secret_key);

    qr->qport               = baseport;
    qr->hbsock              = (void *)hbsock;
    qr->querysock           = (void *)querysock;
    qr->queryid             = 1;
    qr->no_query            = 1;
    qr->udata               = userdata;
    qr->qr_basic_callback   = qr_basic_callback;
    qr->qr_info_callback    = qr_info_callback;
    qr->lastheartbeat       = 0;
    qr->packetnumber        = 0;
    qr->qr_players_callback = qr_players_callback;
    qr->qr_rules_callback   = qr_rules_callback;
    qr->qr_custom_handler   = NULL;
}

#define NUM_PORTS_TO_TRY 100

int qr_init(
    qr_t              *qrec,
    const char         *ip,
    int                baseport,
    const char         *gamename,
    const char         *secret_key,
    qr_querycallback_t qr_basic_callback,
    qr_querycallback_t qr_info_callback,
    qr_querycallback_t qr_rules_callback,
    qr_querycallback_t qr_players_callback,
    void              *userdata
)
{
    struct sockaddr_in saddr;
    SOCKET             hbsock;
    int                maxport;
    int                lasterror = 0;

#if defined(_LINUX)
    unsigned int saddrlen;
#else
    int saddrlen;
#endif

    hbsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (hbsock == INVALID_SOCKET) {
        return 1;
    }

    maxport = baseport + NUM_PORTS_TO_TRY;
    while (baseport < maxport) {
        get_sockaddrin(ip, baseport, &saddr, NULL);
        if (saddr.sin_addr.s_addr == htonl(0x7F000001)) { //localhost -- we don't want that!
            saddr.sin_addr.s_addr = INADDR_ANY;
        }

        lasterror = bind(hbsock, (struct sockaddr *)&saddr, sizeof(saddr));
        if (lasterror == 0) {
            break; //we found a port
        }
        baseport++;
    }

    if (lasterror != 0) //we weren't able to find a port
    {
        return 2;
    }

    if (baseport == 0) //we bound it dynamically
    {
        saddrlen = sizeof(saddr);

        lasterror = getsockname(hbsock, (struct sockaddr *)&saddr, &saddrlen);

        if (lasterror) {
            return 2;
        }
        baseport = ntohs(saddr.sin_port);
    }

    init_qrec(
        qrec,
        baseport,
        hbsock,
        hbsock,
        gamename,
        secret_key,
        qr_basic_callback,
        qr_info_callback,
        qr_rules_callback,
        qr_players_callback,
        userdata
    );

    if (!qrec) {
        qrec = &current_rec;
    }

    return do_connect((void *)hbsock, MASTER_SERVER_HOST, 27900, &(*qrec)->master_saddr);
}
