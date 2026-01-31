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

/******
gqueryreporting.h
GameSpy Query & Reporting SDK 
  
Copyright 2000 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, CA 92614
(949)798-4200
Fax(949)798-4299
******

 Please see the GameSpy Query & Reporting SDK documentation for more 
 information

******/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/********
ERROR CONSTANTS
---------------
These constants are returned from qr_init to signal an error condition
***************/

#define E_GOA_WSOCKERROR 1
#define E_GOA_BINDERROR  2
#define E_GOA_DNSERROR   3
#define E_GOA_CONNERROR  4
/*********
NUM_PORTS_TO_TRY
----------------
This value is the maximum number of ports that will be scanned to
find an open query port, starting from the value passed to qr_init
as the base port. Generally there is no reason to modify this value.
***********/
#define NUM_PORTS_TO_TRY 100

/********
DEFINES
********/
#define MASTER_PORT     qr_get_master_port(0)
//#define MASTER_ADDR     "master." GSI_DOMAIN_NAME
#define MASTER_ADDR     qr_get_master_host(0)
#define FIRST_HB_TIME   30000  /* 30 sec */
#define HB_TIME         300000 /* 5 minutes */
#define HB_SEND_DELAY   0      /* currently send to all masters at once, unless it causes problem */
#define MAX_FIRST_COUNT 10     /* 10 tries = 5 minutes */
#define MAX_DATA_SIZE   1400
#define INBUF_LEN       256
#define BUF_SIZE        1400

/* The hostname of the master server.
If the app resolves the hostname, an
IP can be stored here before calling
qr_init */
extern char qr_hostname[64];

/**
 * @brief Custom function used to return the master host, based on game settings
 * 
 * @return const char* The full master server address
 */
extern unsigned int qr_get_num_masters();
extern const char *qr_get_master_host(int index);
extern int qr_get_master_port(int index);

/********
qr_querycallback_t
-------------------
This is the prototype for the callback functions your game needs to
implement for each of the four basic query types. The callback works the
same for each query type. 

[outbuf] is a pre-allocated buffer for you to place the query reply. It's size is
[maxlen] (default is 1400). If you need larger, you can adjust the 
	defines in gqueryreporting.c
[userdata] is the pointer that was passed into qr_init. You can use this for an
	object or structure pointer if needed.

Simply fill outbuf with the correct data for the query type (consult the sample
apps and the GameSpy Developer Spec). 
outbuf should be a NULL terminated ANSI string.
********/
typedef void (*qr_querycallback_t)(char* outbuf, int maxlen, void* userdata);

typedef void (*qr_cdkey_process_t)(char* buf, int len, struct sockaddr* fromaddr);

/***********
qr_t
----
This abstract type is used to instantiate multiple instances of the
Query & Reporting SDK (for example, if you are running multiple servers
in the same process).
For most games, you can ignore this value and pass NULL in to all functions
that require it. A single global instance will be used, similar to how the
original Developer SDK worked
************/
typedef struct qr_implementation_s *qr_t;
/************
QR_INIT
--------
This creates/binds the sockets needed for heartbeats and queries/replies.
[qrec] if not null, will be filled with the qr_t instance for this server.
	If you are not using more than one instance of the Query & Reporting SDK you
	can pass in NULL for this value.
[ip] is an optional parameter that determines which dotted IP address to bind to on
	a multi-homed machine. You can pass NULL to bind to all IP addresses.
[baseport] is the port to accept queries on. If baseport is not available, the
	Query and Reporting SDK will scan for an available port in the range of 
	baseport -> baseport + NUM_PORTS_TO_TRY
	Optionally, you can pass in 0 to have a port chosen automatically
	(makes it harder for debugging/testing).
[gamename] is the unique gamename that you were given
[secretkey] is your unique secret key
[qr_*_callback] are your data callback functions, this cannot be NULL
[userdata] is an optional, implementation specific parameter that will be
	passed to all callback functions. Use it to store an object or structure
	pointer if needed.

Returns
0 is successful, otherwise one of the E_GOA constants above.
************/
int qr_init(/*[out]*/ qr_t* qrec,
            const char* ip,
            int baseport,
            const char* gamename,
            const char* secret_key,
            qr_querycallback_t qr_basic_callback,
            qr_querycallback_t qr_info_callback,
            qr_querycallback_t qr_rules_callback,
            qr_querycallback_t qr_players_callback,
            void* userdata);

/*******************
QR_PROCESS_QUERIES
-------------------
This function should be called somewhere in your main program loop to
process any pending server queries and send a heartbeat if 5 minutes has
elapsed.

Query replies are very latency sensative, so you should make sure this
function is called at least every 100ms while your game is in progress.
The function has very low overhead and should not cause any performance
problems.
Unless you are using multiple instances of the SDK, you should pass NULl
for qrec.
The no_heartbeat version will not send any heartbeats to the master - use
this if you only want to advertise your server on the LAN.
********************/
void qr_process_queries(qr_t qrec);
void qr_process_queries_no_heartbeat(qr_t qrec);

/*****************
QR_SEND_STATECHANGED
--------------------
This function forces a \statechanged\ heartbeat to be sent immediately.
Use it any time you have changed the gamestate of your game to signal the
master to update your status.
Also use it before your game exits by changing the gamestate to "exiting"
and sending a statechanged heartbeat. This will insure that your game
is removed from the list promptly.
Unless you are using multiple instances of the SDK, you should pass NULl
for qrec.
*******************/
void qr_send_statechanged(qr_t qrec);

/*****************
QR_SHUTDOWN
------------
This function closes the sockets created in qr_init and takes care of
any misc. cleanup. You should try to call it when before exiting the server
if qr_init was called.
If you pass in a qrec that was returned from qr_init, all resources associated
with that qrec will be freed. If you passed NULL into qr_int, you can pass
NULL in here as well.
******************/
void qr_shutdown(qr_t qrec);

void qr_check_queries(qr_t qrec);

void qr_send_exiting(qr_t qrec);

int get_master_count();

void clear_master_list();

int get_sockaddrin(const char *host, int port, struct sockaddr_in *saddr, struct hostent **savehent);

void add_master(struct sockaddr_in *addr);

unsigned int qr_get_port(qr_t qrec);

#ifdef __cplusplus
}
#endif
