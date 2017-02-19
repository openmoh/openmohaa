/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// common.c -- misc functions used in client and server

#include "q_shared.h"
#include "qcommon.h"
#include <setjmp.h>
#ifndef _WIN32
#include <netinet/in.h>
#include <sys/stat.h> // umask
#else
#include <winsock.h>
#endif

#include <baseimp.h>

extern "C" {

baseImport_t bi;
cvar_t *sv_scriptfiles;
cvar_t *g_scriptcheck;
cvar_t *g_showopcodes;

int demo_protocols[] =
{ 8, 0 };

#define MAX_NUM_ARGVS	50

//wombat: i had to increase these quite a bit
//to keep up with larger client_t.
//i reckon this has bad effect on memory usage
// update: put it back unless we really need it large
#define MIN_DEDICATED_COMHUNKMEGS 1
#define MIN_COMHUNKMEGS		56
#define DEF_COMHUNKMEGS		96
#define DEF_COMZONEMEGS		128
#define XSTRING(x)				STRING(x)
#define STRING(x)					#x
#define DEF_COMHUNKMEGS_S	XSTRING(DEF_COMHUNKMEGS)
#define DEF_COMZONEMEGS_S	XSTRING(DEF_COMZONEMEGS)

int		com_argc;
char	*com_argv[MAX_NUM_ARGVS+1];

jmp_buf abortframe;		// an ERR_DROP occured, exit the entire frame


FILE *debuglogfile;
static fileHandle_t logfile;
fileHandle_t	com_journalFile;			// events are written here
fileHandle_t	com_journalDataFile;		// config files are written here

cvar_t	*paused;
cvar_t	*config;
cvar_t	*fps;
float	currentfps;
cvar_t	*com_speeds;
cvar_t	*developer;
cvar_t	*com_dedicated;
cvar_t	*com_timescale;
cvar_t	*com_fixedtime;
cvar_t	*com_dropsim;		// 0.0 to 1.0, simulated packet drops
cvar_t	*com_journal;
cvar_t	*com_maxfps;
cvar_t	*com_timedemo;
cvar_t	*com_sv_running;
cvar_t	*com_cl_running;
cvar_t	*com_viewlog;
cvar_t	*com_logfile;		// 1 = buffer log, 2 = flush after each print
cvar_t	*com_showtrace;
cvar_t	*com_shortversion;
cvar_t	*com_version;
cvar_t	*autopaused;
cvar_t	*deathmatch;
cvar_t	*com_blood;
cvar_t	*com_buildScript;	// for automated data building scripts
cvar_t	*com_introPlayed;
cvar_t  *cl_packetdelay;
cvar_t  *sv_packetdelay;
cvar_t	*com_cameraMode;
cvar_t	*com_ansiColor;
cvar_t	*com_unfocused;
cvar_t	*com_minimized;
cvar_t	*precache;

// com_speeds times
int		time_game;
int		time_frontend;		// renderer frontend time
int		time_backend;		// renderer backend time

int			com_frameTime;
int			com_frameMsec;
int			com_frameNumber;

qboolean	com_errorEntered;
qboolean	com_fullyInitialized;

char	com_errorMessage[MAXPRINTMSG];

void Com_WriteConfig_f( void );
void CIN_CloseAllVideos( void );

//============================================================================

static char	*rd_buffer;
static int	rd_buffersize;
static void	(*rd_flush)( char *buffer );

void Com_BeginRedirect (char *buffer, int buffersize, void (*flush)( char *) )
{
	if (!buffer || !buffersize || !flush)
		return;
	rd_buffer = buffer;
	rd_buffersize = buffersize;
	rd_flush = flush;

	*rd_buffer = 0;
}

void Com_EndRedirect (void)
{
	if ( rd_flush ) {
		rd_flush(rd_buffer);
	}

	rd_buffer = NULL;
	rd_buffersize = 0;
	rd_flush = NULL;
}

#ifndef _COM_NOPRINTF

/*
=============
Com_Printf

Both client and server can use this, and it will output
to the apropriate place.

A raw string should NEVER be passed as fmt, because of "%f" type crashers.
=============
*/
void QDECL Com_Printf( const char *fmt, ... ) {
	va_list		argptr;
	char		msg[MAXPRINTMSG];
  static qboolean opening_qconsole = qfalse;


	va_start (argptr,fmt);
	Q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	if ( rd_buffer ) {
		if ((strlen (msg) + strlen(rd_buffer)) > (rd_buffersize - 1)) {
			rd_flush(rd_buffer);
			*rd_buffer = 0;
		}
		Q_strcat(rd_buffer, rd_buffersize, msg);
    // TTimo nooo .. that would defeat the purpose
		//rd_flush(rd_buffer);
		//*rd_buffer = 0;
		return;
	}

#ifndef DEDICATED
	CL_ConsolePrint( msg );
#endif

	// echo to dedicated console and early console
	Sys_Print( msg );

	// logfile
	if ( com_logfile && com_logfile->integer ) {
    // TTimo: only open the qconsole.log if the filesystem is in an initialized state
    //   also, avoid recursing in the qconsole.log opening (i.e. if fs_debug is on)
		if ( !logfile && FS_Initialized() && !opening_qconsole) {
			struct tm *newtime;
			time_t aclock;

      opening_qconsole = qtrue;

			time( &aclock );
			newtime = localtime( &aclock );

			logfile = FS_FOpenTextFileWrite( "qconsole.log" );

			if(logfile)
			{
				Com_Printf( "logfile opened on %s\n", asctime( newtime ) );

				if ( com_logfile->integer > 1 )
				{
					// force it to not buffer so we get valid
					// data even if we are crashing
					FS_ForceFlush(logfile);
				}
			}
			else
			{
				Com_Printf("Opening qconsole.log failed!\n");
				Cvar_SetValue("logfile", 0);
			}

      opening_qconsole = qfalse;
		}
		if ( logfile && FS_Initialized()) {
			FS_Write(msg, strlen(msg), logfile);
		}
	}
}


/*
================
Com_DPrintf

A Com_Printf that only shows up if the "developer" cvar is set
================
*/
void QDECL Com_DPrintf( const char *fmt, ...) {
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	if ( !developer || !developer->integer ) {
		return;			// don't confuse non-developers with techie stuff...
	}

	va_start (argptr,fmt);
	Q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	Com_Printf ("%s", msg);
}

#endif

/*
=============
Com_Error

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void QDECL Com_Error( int code, const char *fmt, ... ) {
	va_list		argptr;
	static int	lastErrorTime;
	static int	errorCount;
	int			currentTime;

#ifdef _DEBUG
	*( int * )0 = 0;
	assert( 0 );
#endif

	Cvar_Set( "com_errorCode", va( "%i", code ) );

	// when we are running automated scripts, make sure we
	// know if anything failed
	if ( com_buildScript && com_buildScript->integer ) {
		code = ERR_FATAL;
	}

	// if we are getting a solid stream of ERR_DROP, do an ERR_FATAL
	currentTime = Sys_Milliseconds();
	if ( currentTime - lastErrorTime < 100 ) {
		if ( ++errorCount > 3 ) {
			code = ERR_FATAL;
		}
	} else {
		errorCount = 0;
	}
	lastErrorTime = currentTime;

	if ( com_errorEntered ) {
		SyScriptError( "recursive error after: %s", com_errorMessage );
	}
	com_errorEntered = qtrue;

	va_start (argptr,fmt);
	vsprintf (com_errorMessage,fmt,argptr);
	va_end (argptr);

	if (code != ERR_DISCONNECT && code != ERR_NEED_CD)
		Cvar_Set("com_errorMessage", com_errorMessage);

	if (code == ERR_DISCONNECT || code == ERR_SERVERDISCONNECT) {
		CL_Disconnect( qtrue );
		CL_FlushMemory( );
		// make sure we can get at our local stuff
		FS_PureServerSetLoadedPaks("", "");
		com_errorEntered = qfalse;
		longjmp (abortframe, -1);
	} else if (code == ERR_DROP) {
		Com_Printf ("********************\nERROR: %s\n********************\n", com_errorMessage);
		SV_Shutdown (va("Server crashed: %s",  com_errorMessage));
		CL_Disconnect( qtrue );
		CL_FlushMemory( );
		FS_PureServerSetLoadedPaks("", "");
		com_errorEntered = qfalse;
		longjmp (abortframe, -1);
	} else if ( code == ERR_NEED_CD ) {
		SV_Shutdown( "Server didn't have CD" );
		if ( com_cl_running && com_cl_running->integer ) {
			CL_Disconnect( qtrue );
			CL_FlushMemory( );
			com_errorEntered = qfalse;
			CL_CDDialog();
		} else {
			Com_Printf("Server didn't have CD\n" );
		}
		FS_PureServerSetLoadedPaks("", "");
		longjmp (abortframe, -1);
	} else {
		CL_Shutdown ();
		SV_Shutdown (va("Server fatal crashed: %s", com_errorMessage));
	}

	Com_Shutdown ();

	SyScriptError ("%s", com_errorMessage);
}


/*
=============
Com_Quit_f

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void Com_Quit_f( void ) {
	// don't try to shutdown if we are in a recursive error
	if ( !com_errorEntered ) {
		SV_Shutdown ("Server quit");
		CL_Shutdown ();
		Com_Shutdown ();
		FS_Shutdown(qtrue);
	}
	Sys_Quit ();
}



/*
============================================================================

COMMAND LINE FUNCTIONS

+ characters seperate the commandLine string into multiple console
command lines.

All of these are valid:

quake3 +set test blah +map test
quake3 set test blah+map test
quake3 set test blah + map test

============================================================================
*/

#define	MAX_CONSOLE_LINES	32
int		com_numConsoleLines;
char	*com_consoleLines[MAX_CONSOLE_LINES];

/*
==================
Com_ParseCommandLine

Break it up into multiple console lines
==================
*/
void Com_ParseCommandLine( char *commandLine ) {
    int inq = 0;
    com_consoleLines[0] = commandLine;
    com_numConsoleLines = 1;

    while ( *commandLine ) {
        if (*commandLine == '"') {
            inq = !inq;
        }
        // look for a + seperating character
        // if commandLine came from a file, we might have real line seperators
        if ( (*commandLine == '+' && !inq) || *commandLine == '\n'  || *commandLine == '\r' ) {
            if ( com_numConsoleLines == MAX_CONSOLE_LINES ) {
                return;
            }
            com_consoleLines[com_numConsoleLines] = commandLine + 1;
            com_numConsoleLines++;
            *commandLine = 0;
        }
        commandLine++;
    }
}


/*
===================
Com_SafeMode

Check for "safe" on the command line, which will
skip loading of q3config.cfg
===================
*/
qboolean Com_SafeMode( void ) {
	int		i;

	for ( i = 0 ; i < com_numConsoleLines ; i++ ) {
		Cmd_TokenizeString( com_consoleLines[i] );
		if ( !Q_stricmp( Cmd_Argv(0), "safe" )
			|| !Q_stricmp( Cmd_Argv(0), "cvar_restart" ) ) {
			com_consoleLines[i][0] = 0;
			return qtrue;
		}
	}
	return qfalse;
}


/*
===============
Com_StartupVariable

Searches for command line parameters that are set commands.
If match is not NULL, only that cvar will be looked for.
That is necessary because cddir and basedir need to be set
before the filesystem is started, but all other sets should
be after execing the config and default.
===============
*/
void Com_StartupVariable( const char *match ) {
	int		i;
	char	*s;
	cvar_t	*cv;

	for (i=0 ; i < com_numConsoleLines ; i++) {
		Cmd_TokenizeString( com_consoleLines[i] );
		if ( strcmp( Cmd_Argv(0), "set" ) ) {
			continue;
		}

		s = Cmd_Argv(1);
		if ( !match || !strcmp( s, match ) ) {
			Cvar_Set( s, Cmd_Argv(2) );
			cv = Cvar_Get( s, "", 0 );
			cv->flags |= CVAR_USER_CREATED;
//			com_consoleLines[i] = 0;
		}
	}
}


/*
=================
Com_AddStartupCommands

Adds command line parameters as script statements
Commands are seperated by + signs

Returns qtrue if any late commands were added, which
will keep the demoloop from immediately starting
=================
*/
qboolean Com_AddStartupCommands( void ) {
	int		i;
	qboolean	added;

	added = qfalse;
	// quote every token, so args with semicolons can work
	for (i=0 ; i < com_numConsoleLines ; i++) {
		if ( !com_consoleLines[i] || !com_consoleLines[i][0] ) {
			continue;
		}

		// set commands won't override menu startup
		if ( Q_stricmpn( com_consoleLines[i], "set", 3 ) ) {
			added = qtrue;
		}
		Cbuf_AddText( com_consoleLines[i] );
		Cbuf_AddText( "\n" );
	}

	return added;
}


//============================================================================

void Info_Print( const char *s ) {
	char	key[512];
	char	value[512];
	char	*o;
	int		l;

	if (*s == '\\')
		s++;
	while (*s)
	{
		o = key;
		while (*s && *s != '\\')
			*o++ = *s++;

		l = o - key;
		if (l < 20)
		{
			Com_Memset (o, ' ', 20-l);
			key[20] = 0;
		}
		else
			*o = 0;
		Com_Printf ("%s", key);

		if (!*s)
		{
			Com_Printf ("MISSING VALUE\n");
			return;
		}

		o = value;
		s++;
		while (*s && *s != '\\')
			*o++ = *s++;
		*o = 0;

		if (*s)
			s++;
		Com_Printf ("%s\n", value);
	}
}

/*
============
Com_StringContains
============
*/
const char *Com_StringContains(const char *str1, const char *str2, int casesensitive) {
	intptr_t len;
	int i, j;

	len = strlen(str1) - strlen(str2);
	for (i = 0; i <= len; i++, str1++) {
		for (j = 0; str2[j]; j++) {
			if (casesensitive) {
				if (str1[j] != str2[j]) {
					break;
				}
			}
			else {
				if (toupper(str1[j]) != toupper(str2[j])) {
					break;
				}
			}
		}
		if (!str2[j]) {
			return str1;
		}
	}
	return NULL;
}

/*
============
Com_Filter
============
*/
int Com_Filter(const char *filter, const char *name, int casesensitive)
{
	char buf[MAX_TOKEN_CHARS];
	const char *ptr;
	int i, found;

	while(*filter) {
		if (*filter == '*') {
			filter++;
			for (i = 0; *filter; i++) {
				if (*filter == '*' || *filter == '?') break;
				buf[i] = *filter;
				filter++;
			}
			buf[i] = '\0';
			if (strlen(buf)) {
				ptr = Com_StringContains(name, buf, casesensitive);
				if (!ptr) return qfalse;
				name = ptr + strlen(buf);
			}
		}
		else if (*filter == '?') {
			filter++;
			name++;
		}
		else if (*filter == '[' && *(filter+1) == '[') {
			filter++;
		}
		else if (*filter == '[') {
			filter++;
			found = qfalse;
			while(*filter && !found) {
				if (*filter == ']' && *(filter+1) != ']') break;
				if (*(filter+1) == '-' && *(filter+2) && (*(filter+2) != ']' || *(filter+3) == ']')) {
					if (casesensitive) {
						if (*name >= *filter && *name <= *(filter+2)) found = qtrue;
					}
					else {
						if (toupper(*name) >= toupper(*filter) &&
							toupper(*name) <= toupper(*(filter+2))) found = qtrue;
					}
					filter += 3;
				}
				else {
					if (casesensitive) {
						if (*filter == *name) found = qtrue;
					}
					else {
						if (toupper(*filter) == toupper(*name)) found = qtrue;
					}
					filter++;
				}
			}
			if (!found) return qfalse;
			while(*filter) {
				if (*filter == ']' && *(filter+1) != ']') break;
				filter++;
			}
			filter++;
			name++;
		}
		else {
			if (casesensitive) {
				if (*filter != *name) return qfalse;
			}
			else {
				if (toupper(*filter) != toupper(*name)) return qfalse;
			}
			filter++;
			name++;
		}
	}
	return qtrue;
}

/*
============
Com_FilterPath
============
*/
int Com_FilterPath(char *filter, char *name, int casesensitive)
{
	int i;
	char new_filter[MAX_QPATH];
	char new_name[MAX_QPATH];

	for (i = 0; i < MAX_QPATH-1 && filter[i]; i++) {
		if ( filter[i] == '\\' || filter[i] == ':' ) {
			new_filter[i] = '/';
		}
		else {
			new_filter[i] = filter[i];
		}
	}
	new_filter[i] = '\0';
	for (i = 0; i < MAX_QPATH-1 && name[i]; i++) {
		if ( name[i] == '\\' || name[i] == ':' ) {
			new_name[i] = '/';
		}
		else {
			new_name[i] = name[i];
		}
	}
	new_name[i] = '\0';
	return Com_Filter(new_filter, new_name, casesensitive);
}

/*
================
Com_RealTime
================
*/
int Com_RealTime(qtime_t *qtime) {
	time_t t;
	struct tm *tms;

	t = time(NULL);
	if (!qtime)
		return t;
	tms = localtime(&t);
	if (tms) {
		qtime->tm_sec = tms->tm_sec;
		qtime->tm_min = tms->tm_min;
		qtime->tm_hour = tms->tm_hour;
		qtime->tm_mday = tms->tm_mday;
		qtime->tm_mon = tms->tm_mon;
		qtime->tm_year = tms->tm_year;
		qtime->tm_wday = tms->tm_wday;
		qtime->tm_yday = tms->tm_yday;
		qtime->tm_isdst = tms->tm_isdst;
	}
	return t;
}

/*
========================
CopyString

 NOTE:	never write over the memory CopyString returns because
		memory from a memstatic_t might be returned
========================
*/
char *CopyString( const char *in ) {
	char	*out;

#ifndef _DEBUG_MEM
	if (!in[0]) {
		return ( char * )Z_EmptyStringPointer();
	}
	else if (!in[1]) {
		if (in[0] >= '0' && in[0] <= '9') {
			return ( char * )Z_NumberStringPointer( *in );
		}
	}
#endif

	out = ( char * )Z_Malloc( strlen( in ) + 1 );
	strcpy( out, in );
	return out;
}

/*
====================
B_Malloc
====================
*/
void *B_Malloc( size_t size ) {
	return Z_Malloc( size );
}

/*
====================
B_Free
====================
*/
void B_Free( void *ptr ) {
	return Z_Free( ptr );
}

/*
====================
Com_FillBaseImports
====================
*/
void Com_FillBaseImports( void ) {
	bi.Printf = Com_Printf;
	bi.DPrintf = Com_DPrintf;
	bi.Error = Com_Error;
	bi.Malloc = B_Malloc;
	bi.Free = B_Free;
	bi.Milliseconds = Sys_Milliseconds;
	bi.FS_ReadFile = FS_ReadFileEx;
	bi.FS_FreeFile = FS_FreeFile;
	bi.FS_CanonicalFilename = FS_CanonicalFilename;
	bi.FS_FileNewer = FS_FileNewer;
	bi.FS_Flush = FS_Flush;
	bi.FS_FOpenFileAppend = FS_FOpenFileAppend;
	bi.FS_FOpenFileWrite = FS_FOpenFileWrite;
	bi.FS_FreeFileList = FS_FreeFileList;
	bi.FS_ListFiles = FS_ListFiles;
	bi.FS_PrepFileWrite = FS_PrepFileWrite;
	bi.FS_Read = FS_Read;
	bi.FS_Seek = FS_Seek;
	bi.FS_Tell = FS_FTell;
	bi.FS_Write = FS_Write;
	bi.FS_WriteFile = FS_WriteFile;
}

void CL_ShutdownCGame( void );
void CL_ShutdownUI( void );
void SV_ShutdownGameProgs( void );


/*
===================================================================

EVENTS AND JOURNALING

In addition to these events, .cfg files are also copied to the
journaled file
===================================================================
*/

#define	MAX_PUSHED_EVENTS	            1024
static int com_pushedEventsHead = 0;
static int com_pushedEventsTail = 0;
static sysEvent_t	com_pushedEvents[MAX_PUSHED_EVENTS];

/*
=================
Com_InitJournaling
=================
*/
void Com_InitJournaling( void ) {
	Com_StartupVariable( "journal" );
	com_journal = Cvar_Get ("journal", "0", CVAR_INIT);
	if ( !com_journal->integer ) {
		return;
	}

	if ( com_journal->integer == 1 ) {
		Com_Printf( "Journaling events\n");
		com_journalFile = FS_FOpenFileWrite( "journal.dat" );
		com_journalDataFile = FS_FOpenFileWrite( "journaldata.dat" );
	} else if ( com_journal->integer == 2 ) {
		Com_Printf( "Replaying journaled events\n");
		FS_FOpenFileRead( "journal.dat", &com_journalFile, qtrue, qtrue );
		FS_FOpenFileRead( "journaldata.dat", &com_journalDataFile, qtrue, qtrue );
	}

	if ( !com_journalFile || !com_journalDataFile ) {
		Cvar_Set( "com_journal", "0" );
		com_journalFile = 0;
		com_journalDataFile = 0;
		Com_Printf( "Couldn't open journal files\n" );
	}
}

/*
========================================================================

EVENT LOOP

========================================================================
*/

#define MAX_QUEUED_EVENTS  256
#define MASK_QUEUED_EVENTS ( MAX_QUEUED_EVENTS - 1 )

static sysEvent_t  eventQueue[ MAX_QUEUED_EVENTS ];
static int         eventHead = 0;
static int         eventTail = 0;
static byte        sys_packetReceived[ MAX_MSGLEN ];

/*
================
Com_QueueEvent

A time of 0 will get the current time
Ptr should either be null, or point to a block of data that can
be freed by the game later.
================
*/
void Com_QueueEvent( int time, sysEventType_t type, int value, int value2, size_t ptrLength, void *ptr )
{
	sysEvent_t  *ev;

	ev = &eventQueue[ eventHead & MASK_QUEUED_EVENTS ];

	if ( eventHead - eventTail >= MAX_QUEUED_EVENTS )
	{
		Com_Printf("Com_QueueEvent: overflow\n");
		// we are discarding an event, but don't leak memory
		if ( ev->evPtr )
		{
			Z_Free( ev->evPtr );
		}
		eventTail++;
	}

	eventHead++;

	if ( time == 0 )
	{
		time = Sys_Milliseconds();
	}

	ev->evTime = time;
	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
}

/*
================
Com_GetSystemEvent

================
*/
sysEvent_t Com_GetSystemEvent( void )
{
	sysEvent_t  ev;
	char        *s;
	msg_t       netmsg;
	netadr_t    adr;

	// return if we have data
	if ( eventHead > eventTail )
	{
		eventTail++;
		return eventQueue[ ( eventTail - 1 ) & MASK_QUEUED_EVENTS ];
	}

	// check for console commands
	s = Sys_ConsoleInput();
	if ( s )
	{
		char	*b;
		size_t	len;

		len = strlen( s ) + 1;
		b = ( char * )Z_Malloc( len );
		strcpy( b, s );
		Com_QueueEvent( 0, SE_CONSOLE, 0, 0, len, b );
	}

	// check for network packets
	MSG_Init( &netmsg, sys_packetReceived, sizeof( sys_packetReceived ) );
	if ( Sys_GetPacket ( &adr, &netmsg ) )
	{
		netadr_t	*buf;
		size_t		len;

		// copy out to a seperate buffer for qeueing
		len = sizeof( netadr_t ) + netmsg.cursize;
		buf = ( netadr_t * )Z_Malloc( len );
		*buf = adr;
		memcpy( buf+1, netmsg.data, netmsg.cursize );
		Com_QueueEvent( 0, SE_PACKET, 0, 0, len, buf );
	}

	// return if we have data
	if ( eventHead > eventTail )
	{
		eventTail++;
		return eventQueue[ ( eventTail - 1 ) & MASK_QUEUED_EVENTS ];
	}

	// create an empty event to return
	memset( &ev, 0, sizeof( ev ) );
	ev.evTime = Sys_Milliseconds();

	return ev;
}

/*
=================
Com_GetRealEvent
=================
*/
sysEvent_t	Com_GetRealEvent( void ) {
	size_t		r;
	sysEvent_t	ev;

	// either get an event from the system or the journal file
	if ( com_journal->integer == 2 ) {
		r = FS_Read( &ev, sizeof(ev), com_journalFile );
		if ( r != sizeof(ev) ) {
			Com_Error( ERR_FATAL, "Error reading from journal file" );
		}
		if ( ev.evPtrLength ) {
			ev.evPtr = Z_Malloc( ev.evPtrLength );
			r = FS_Read( ev.evPtr, ev.evPtrLength, com_journalFile );
			if ( r != ev.evPtrLength ) {
				Com_Error( ERR_FATAL, "Error reading from journal file" );
			}
		}
	} else {
		ev = Com_GetSystemEvent();

		// write the journal value out if needed
		if ( com_journal->integer == 1 ) {
			r = FS_Write( &ev, sizeof(ev), com_journalFile );
			if ( r != sizeof(ev) ) {
				Com_Error( ERR_FATAL, "Error writing to journal file" );
			}
			if ( ev.evPtrLength ) {
				r = FS_Write( ev.evPtr, ev.evPtrLength, com_journalFile );
				if ( r != ev.evPtrLength ) {
					Com_Error( ERR_FATAL, "Error writing to journal file" );
				}
			}
		}
	}

	return ev;
}


/*
=================
Com_InitPushEvent
=================
*/
void Com_InitPushEvent( void ) {
  // clear the static buffer array
  // this requires SE_NONE to be accepted as a valid but NOP event
  memset( com_pushedEvents, 0, sizeof(com_pushedEvents) );
  // reset counters while we are at it
  // beware: GetEvent might still return an SE_NONE from the buffer
  com_pushedEventsHead = 0;
  com_pushedEventsTail = 0;
}


/*
=================
Com_PushEvent
=================
*/
void Com_PushEvent( sysEvent_t *event ) {
	sysEvent_t		*ev;
	static int printedWarning = 0;

	ev = &com_pushedEvents[ com_pushedEventsHead & (MAX_PUSHED_EVENTS-1) ];

	if ( com_pushedEventsHead - com_pushedEventsTail >= MAX_PUSHED_EVENTS ) {

		// don't print the warning constantly, or it can give time for more...
		if ( !printedWarning ) {
			printedWarning = qtrue;
			Com_Printf( "WARNING: Com_PushEvent overflow\n" );
		}

		if ( ev->evPtr ) {
			Z_Free( ev->evPtr );
		}
		com_pushedEventsTail++;
	} else {
		printedWarning = qfalse;
	}

	*ev = *event;
	com_pushedEventsHead++;
}

/*
=================
Com_GetEvent
=================
*/
sysEvent_t	Com_GetEvent( void ) {
	if ( com_pushedEventsHead > com_pushedEventsTail ) {
		com_pushedEventsTail++;
		return com_pushedEvents[ (com_pushedEventsTail-1) & (MAX_PUSHED_EVENTS-1) ];
	}
	return Com_GetRealEvent();
}

/*
=================
Com_RunAndTimeServerPacket
=================
*/
void Com_RunAndTimeServerPacket( netadr_t *evFrom, msg_t *buf ) {
	int		t1, t2, msec;

	t1 = 0;

	if ( com_speeds->integer ) {
		t1 = Sys_Milliseconds ();
	}

	SV_PacketEvent( *evFrom, buf );

	if ( com_speeds->integer ) {
		t2 = Sys_Milliseconds ();
		msec = t2 - t1;
		if ( com_speeds->integer == 3 ) {
			Com_Printf( "SV_PacketEvent time: %i\n", msec );
		}
	}
}

/*
=================
Com_EventLoop

Returns last event time
=================
*/
int Com_EventLoop( void ) {
	sysEvent_t	ev;
	netadr_t	evFrom;
	byte		bufData[MAX_MSGLEN];
	msg_t		buf;

	MSG_Init( &buf, bufData, sizeof( bufData ) );

	while ( 1 ) {
		NET_FlushPacketQueue();
		ev = Com_GetEvent();

		// if no more events are available
		if ( ev.evType == SE_NONE ) {
			// manually send packet events for the loopback channel
			while ( NET_GetLoopPacket( NS_CLIENT, &evFrom, &buf ) ) {
				CL_PacketEvent( evFrom, &buf );
			}

			while ( NET_GetLoopPacket( NS_SERVER, &evFrom, &buf ) ) {
				// if the server just shut down, flush the events
				if ( com_sv_running->integer ) {
					Com_RunAndTimeServerPacket( &evFrom, &buf );
				}
			}

			return ev.evTime;
		}


		switch ( ev.evType ) {
		default:
			Com_Error( ERR_FATAL, "Com_EventLoop: bad event type %i", ev.evType );
			break;
        case SE_NONE:
            break;
		case SE_KEY:
			CL_KeyEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
		case SE_CHAR:
			CL_CharEvent( ev.evValue );
			break;
		case SE_MOUSE:
			CL_MouseEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
		case SE_JOYSTICK_AXIS:
			CL_JoystickEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
		case SE_CONSOLE:
			Cbuf_AddText( (char *)ev.evPtr );
			Cbuf_AddText( "\n" );
			break;
		case SE_PACKET:
			// this cvar allows simulation of connections that
			// drop a lot of packets.  Note that loopback connections
			// don't go through here at all.
			if ( com_dropsim->value > 0 ) {
				static int seed;

				if ( Q_random( &seed ) < com_dropsim->value ) {
					break;		// drop this packet
				}
			}

			evFrom = *(netadr_t *)ev.evPtr;
			buf.cursize = ev.evPtrLength - sizeof( evFrom );

			// we must copy the contents of the message out, because
			// the event buffers are only large enough to hold the
			// exact payload, but channel messages need to be large
			// enough to hold fragment reassembly
			if ( (unsigned)buf.cursize > buf.maxsize ) {
				Com_Printf("Com_EventLoop: oversize packet\n");
				continue;
			}
			Com_Memcpy( buf.data, (byte *)((netadr_t *)ev.evPtr + 1), buf.cursize );
			if ( com_sv_running->integer ) {
				Com_RunAndTimeServerPacket( &evFrom, &buf );
			} else {
				CL_PacketEvent( evFrom, &buf );
			}
			break;
		}

		// free any block data
		if ( ev.evPtr ) {
			Z_Free( ev.evPtr );
		}
	}

	return 0;	// never reached
}

/*
================
Com_Milliseconds

Can be used for profiling, but will be journaled accurately
================
*/
int Com_Milliseconds (void) {
	sysEvent_t	ev;

	// get events and push them until we get a null event with the current time
	do {

		ev = Com_GetRealEvent();
		if ( ev.evType != SE_NONE ) {
			Com_PushEvent( &ev );
		}
	} while ( ev.evType != SE_NONE );

	return ev.evTime;
}

//============================================================================

/*
=============
Com_Error_f

Just throw a fatal error to
test error shutdown procedures
=============
*/
static void Com_Error_f (void) {
	if ( Cmd_Argc() > 1 ) {
		Com_Error( ERR_DROP, "Testing drop error" );
	} else {
		Com_Error( ERR_FATAL, "Testing fatal error" );
	}
}


/*
=============
Com_Freeze_f

Just freeze in place for a given number of seconds to test
error recovery
=============
*/
static void Com_Freeze_f (void) {
	float	s;
	int		start, now;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "freeze <seconds>\n" );
		return;
	}
	s = atof( Cmd_Argv(1) );

	start = Com_Milliseconds();

	while ( 1 ) {
		now = Com_Milliseconds();
		if ( ( now - start ) * 0.001 > s ) {
			break;
		}
	}
}

/*
=================
Com_Crash_f

A way to force a bus error for development reasons
=================
*/
static void Com_Crash_f( void ) {
	* ( int * ) 0 = 0x12345678;
}

// TTimo: centralizing the cl_cdkey stuff after I discovered a buffer overflow problem with the dedicated server version
//   not sure it's necessary to have different defaults for regular and dedicated, but I don't want to risk it
//   https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=470
#ifndef DEDICATED
char	cl_cdkey[34] = "                                ";
#else
char	cl_cdkey[34] = "123456789";
#endif

/*
=================
Com_Init
=================
*/
void Com_Init( char *commandLine ) {
	int			iStart;
	int			iEnd;
	const char	*s;
	char		configname[ 128 ];

	Com_Printf( "--- Common Initialization ---\n" );

	iStart = Sys_Milliseconds();

	Com_Printf( "%s %s %s\n", Q3_VERSION, PLATFORM_STRING, __DATE__ );

	if ( setjmp (abortframe) ) {
		SyScriptError ("Error during initialization");
	}

	// prepare enough of the subsystems to handle
	// cvar and command buffer management
	Com_ParseCommandLine( commandLine );

	Com_FillBaseImports();

	Swap_Init();
	Cbuf_Init();
	Z_InitMemory();

	Cmd_Init();
	Cvar_Init();

	// override anything from the config files with command line args
	Com_StartupVariable( NULL );

	// get the developer cvar set as early as possible
	Com_StartupVariable( "developer" );

	developer = Cvar_Get( "developer", "0", CVAR_ARCHIVE );

	// done early so bind command exists
	CL_InitKeyCommands();

	FS_InitFilesystem ();

	Com_InitJournaling();

	TIKI_Begin();

	Cbuf_Execute( 0 );

	Cbuf_AddText( "exec default.cfg\n" );

	Cbuf_AddText( "exec menu.cfg\n" );

	Cbuf_AddText ("exec newconfig.cfg\n");

	cvar_global_force = qtrue;
	Cbuf_Execute( 0 );
	cvar_global_force = qfalse;

	strncpy( configname, "omconfig.cfg", sizeof( configname ) );

	// override anything from the config files with command line args
	Com_StartupVariable( "config" );

	config = Cvar_Get( "config", "openmohaa.cfg", 0 );

	if( strlen( config->string ) <= 1 )
	{
		long size = sizeof( configname );
		LoadRegistryInfo(1, "config", configname, &size );
		COM_DefaultExtension( configname, sizeof( configname ), ".cfg" );
	}
	else
	{
		size_t len = strlen( config->string );

		strncpy( configname, config->string, len >= sizeof( configname ) ? sizeof( configname ) : len );
		COM_StripExtension( configname, configname, sizeof( configname ) );
		strcat( configname, ".cfg" );
		SaveRegistryInfo( 1, "config", configname, sizeof( configname ) );
	}

	SaveRegistryInfo( 1, "config", configname, sizeof( configname ) );
	Cvar_Set( "config", configname );
	Com_Printf( "Config: %s\n", configname );

	Cbuf_AddText( va( "exec configs/%s\n", configname ) );

	if( Com_SafeMode() )
	{
		Cbuf_AddText( "exec safemode.cfg\n" );
	}
	else if( IsFirstRun() || IsNewConfig() )
	{
		Cbuf_AddText( "exec newconfig.cfg\n" );
		cvar_modifiedFlags |= CVAR_ARCHIVE;
	}

	Cbuf_AddText( "exec localized.cfg\n" );
	Cbuf_AddText( "exec autoexec.cfg\n" );
	Cbuf_Execute( 0 );
	Com_StartupVariable( NULL );
	Cvar_Set( "config", configname );

	// if any archived cvars are modified after this, we will trigger a writing
	// of the config file
	cvar_modifiedFlags &= ~CVAR_ARCHIVE;

	if( developer && developer->integer ) {
		Cmd_AddCommand( "error", Com_Error_f );
		Cmd_AddCommand( "crash", Com_Crash_f );
		Cmd_AddCommand( "freeze", Com_Freeze_f );
	}
	Cmd_AddCommand( "quit", Com_Quit_f );
	Cmd_AddCommand( "changeVectors", MSG_ReportChangeVectors_f );

#ifndef DEDICATED
	Cmd_AddCommand( "writeconfig", Com_WriteConfig_f );
#endif

	//
	// init commands and vars
	//
	dumploadedanims = Cvar_Get( "dumploadedanims", "0", 0 );
	low_anim_memory = Cvar_Get( "low_anim_memory", "0", 0 );
	showLoad = Cvar_Get( "showLoad", "0", 0 );
	convertAnims = Cvar_Get( "convertAnim", "0", 0 );
	com_maxfps = Cvar_Get( "com_maxfps", "85", CVAR_ARCHIVE );
	deathmatch = Cvar_Get( "deathmatch", "0", 0 );
	paused = Cvar_Get( "paused", "0", 64 );
	autopaused = Cvar_Get( "autopaused", "1", CVAR_ARCHIVE );
	fps = Cvar_Get( "fps", "0", CVAR_ARCHIVE );
	com_timescale = Cvar_Get( "timescale", "1", CVAR_CHEAT | CVAR_SYSTEMINFO );
	com_fixedtime = Cvar_Get( "fixedtime", "0", CVAR_CHEAT );
	com_showtrace = Cvar_Get( "com_showtrace", "0", CVAR_CHEAT );
	com_dropsim = Cvar_Get( "com_dropsim", "0", CVAR_CHEAT );
	com_viewlog = Cvar_Get( "viewlog", "0", CVAR_CHEAT );
	com_speeds = Cvar_Get( "com_speeds", "0", 0 );
	com_timedemo = Cvar_Get( "timedemo", "0", CVAR_CHEAT );
	com_dedicated = Cvar_Get( "dedicated", "0", CVAR_LATCH );
	com_sv_running = Cvar_Get( "sv_running", "0", CVAR_ROM );
	com_cl_running = Cvar_Get( "cl_running", "0", CVAR_ROM );
	com_buildScript = Cvar_Get( "com_buildScript", "0", 0 );
	cl_packetdelay = Cvar_Get( "cl_packetdelay", "0", 0 );
	sv_packetdelay = Cvar_Get( "sv_packetdelay", "0", 0 );

	if( com_dedicated->integer )
	{
		if( !com_viewlog->integer ) {
			Cvar_Set( "viewlog", "1" );
		}
	}

	s = va( "%s %s %s", Q3_VERSION, PLATFORM_STRING, __DATE__ );
	com_version = Cvar_Get( "version", s, CVAR_ROM | CVAR_SERVERINFO );
	com_shortversion = Cvar_Get( "shortversion",  PRODUCT_VERSION, CVAR_ROM );

	Sys_Init();
	Netchan_Init( Com_Milliseconds() & 0xffff );	// pick a port value that should be nice and random
	SV_Init();

	com_dedicated->modified = qfalse;

	if( com_dedicated->integer )
	{
		Sys_CloseMutex();
		NET_Init();
	}
	else
	{
		CL_Init();
		Sys_ShowConsole( com_viewlog->integer, qfalse );
	}

	// set com_frameTime so that if a map is started on the
	// command line it will still be able to count on com_frameTime
	// being random enough for a serverid
	com_frameTime = Com_Milliseconds();

	// add + commands from command line
	Com_AddStartupCommands();

	com_fullyInitialized = qtrue;

	SaveRegistryInfo( qtrue, "basepath", fs_basepath->string, 128 );

	RecoverLostAutodialData();

	iEnd = Sys_Milliseconds();
	Com_Printf( "--- Common Initialization Complete --- %i ms\n", iEnd - iStart );
}

//==================================================================

void Com_WriteConfigToFile( const char *filename ) {
	fileHandle_t	f;
	char			szFullName[ 256 ];

	memcpy( szFullName, "configs/", 9 );
	strcat( szFullName, filename );
	COM_StripExtension( szFullName, szFullName, sizeof( szFullName ) );
	strcat( szFullName, ".cfg" );

	f = FS_FOpenFileWrite( szFullName );
	if ( !f ) {
		Com_Printf ("Couldn't write %s.\n", filename );
		return;
	}

	FS_Printf( f, "// generated by openmohaa\n" );
	FS_Printf( f, "//\n" );
	FS_Printf( f, "// Key Bindings\n" );
	FS_Printf( f, "//\n" );
	Key_WriteBindings( f );
	FS_Printf( f, "//\n" );
	FS_Printf( f, "// Cvars\n" );
	FS_Printf( f, "//\n" );
	Cvar_WriteVariables( f );
	FS_Printf( f, "//\n" );
	FS_Printf( f, "// Aliases\n" );
	FS_Printf( f, "//\n" );
	Cmd_WriteAliases( f );

	FS_FCloseFile( f );
}


/*
===============
Com_WriteConfiguration

Writes key bindings and archived cvars to config file if modified
===============
*/
void Com_WriteConfiguration( void ) {
#ifndef DEDICATED
	cvar_t	*fs;
#endif
	// if we are quiting without fully initializing, make sure
	// we don't write out anything
	if ( !com_fullyInitialized ) {
		return;
	}

	if ( !(cvar_modifiedFlags & CVAR_ARCHIVE ) ) {
		return;
	}
	cvar_modifiedFlags &= ~CVAR_ARCHIVE;

	Com_WriteConfigToFile( config->string );

	// not needed for dedicated
#ifndef DEDICATED
	fs = Cvar_Get ("fs_game", "", CVAR_INIT|CVAR_SYSTEMINFO );
#endif
}


/*
===============
Com_WriteConfig_f

Write the config file to a specific name
===============
*/
void Com_WriteConfig_f( void ) {
	char	filename[MAX_QPATH];

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: writeconfig <filename>\n" );
		return;
	}

	Q_strncpyz( filename, Cmd_Argv( 1 ), sizeof( filename ) );
	COM_DefaultExtension( filename, sizeof( filename ), ".cfg" );
	Com_Printf( "Writing %s.\n", filename );
	Com_WriteConfigToFile( filename );
}

/*
================
Com_ModifyMsec
================
*/
int Com_ModifyMsec( int msec ) {
	int		clampTime;

	//
	// modify time for debugging values
	//
	if ( com_fixedtime->integer ) {
		msec = com_fixedtime->integer;
	} else if ( com_timescale->value ) {
		msec *= com_timescale->value;
	}

	// don't let it scale below 1 msec
	if ( msec < 1 && com_timescale->value) {
		msec = 1;
	}

	if ( com_dedicated->integer ) {
		// dedicated servers don't want to clamp for a much longer
		// period, because it would mess up all the client's views
		// of time.
		if (com_sv_running->integer && msec > 500)
			Com_Printf( "Hitch warning: %i msec frame time\n", msec );

		clampTime = 5000;
	} else
	if ( !com_sv_running->integer ) {
		// clients of remote servers do not want to clamp time, because
		// it would skew their view of the server's time temporarily
		clampTime = 5000;
	} else {
		// for local single player gaming
		// we may want to clamp the time to prevent players from
		// flying off edges when something hitches.
		clampTime = 200;
	}

	if ( msec > clampTime ) {
		msec = clampTime;
	}

	return msec;
}

qboolean CL_FinishedIntro( void );

/*
=================
Com_Frame
=================
*/
void Com_Frame( void ) {

	int		msec, minMsec;
	static int	lastTime;
	int key;

	int		timeBeforeFirstEvents;
	int           timeBeforeServer;
	int           timeBeforeEvents;
	int           timeBeforeClient;
	int           timeAfter;





	if ( setjmp (abortframe) ) {
		return;			// an ERR_DROP was thrown
	}

	timeBeforeFirstEvents =0;
	timeBeforeServer =0;
	timeBeforeEvents =0;
	timeBeforeClient = 0;
	timeAfter = 0;


	// old net chan encryption key
	key = 0x87243987;

#ifndef DEDICATED
	// write config file if anything changed
	Com_WriteConfiguration();
#endif

	//
	// main event loop
	//
	if ( com_speeds->integer ) {
		timeBeforeFirstEvents = Sys_Milliseconds ();
	}

	// we may want to spin here if things are going too fast
	if ( !com_dedicated->integer && com_maxfps->integer > 0 && !com_timedemo->integer ) {
		minMsec = 1000 / com_maxfps->integer;
	} else {
		minMsec = 1;
	}
	do {
		com_frameTime = Com_EventLoop();
		if ( lastTime > com_frameTime ) {
			lastTime = com_frameTime;		// possible on first frame
		}
		msec = com_frameTime - lastTime;
	} while ( msec < minMsec );

	if( com_dedicated->integer || CL_FinishedIntro() )
	{
		Cbuf_Execute( 0 );
	}

	lastTime = com_frameTime;

	// mess with msec if needed
	com_frameMsec = msec;
	msec = Com_ModifyMsec( msec );

	//
	// server side
	//
	if ( com_speeds->integer ) {
		timeBeforeServer = Sys_Milliseconds ();
	}

	SV_Frame( msec );

	// if "dedicated" has been modified, start up
	// or shut down the client system.
	// Do this after the server may have started,
	// but before the client tries to auto-connect
	if ( com_dedicated->modified ) {
		// get the latched value
		Cvar_Get( "dedicated", "0", 0 );
		com_dedicated->modified = qfalse;
		if ( !com_dedicated->integer ) {
			SV_Shutdown( "dedicated set to 0" );
			CL_FlushMemory();
		}
	}

#ifndef DEDICATED
	//
	// client system
	//
	//
	// run event loop a second time to get server to client packets
	// without a frame of latency
	//
	if ( com_speeds->integer ) {
		timeBeforeEvents = Sys_Milliseconds ();
	}
	Com_EventLoop();
	Cbuf_Execute (msec);


	//
	// client side
	//
	if ( com_speeds->integer ) {
		timeBeforeClient = Sys_Milliseconds ();
	}

	CL_Frame( msec );

	if ( com_speeds->integer ) {
		timeAfter = Sys_Milliseconds ();
	}
#endif

	//
	// report timing information
	//
	if ( com_speeds->integer ) {
		int			all, sv, ev, cl;

		all = timeAfter - timeBeforeServer;
		sv = timeBeforeEvents - timeBeforeServer;
		ev = timeBeforeServer - timeBeforeFirstEvents + timeBeforeClient - timeBeforeEvents;
		cl = timeAfter - timeBeforeClient;
		sv -= time_game;
		cl -= time_frontend + time_backend;

		Com_Printf ("frame:%i all:%3i sv:%3i ev:%3i cl:%3i gm:%3i rf:%3i bk:%3i\n",
					 com_frameNumber, all, sv, ev, cl, time_game, time_frontend, time_backend );
	}

	//
	// trace optimization tracking
	//
	if ( com_showtrace->integer ) {

		extern	int c_traces, c_brush_traces, c_patch_traces, c_terrain_patch_traces;
		extern	int	c_pointcontents;

		Com_Printf ("%4i traces  (%ib %ip %itp) %4i points\n", c_traces,
			c_brush_traces, c_patch_traces, c_terrain_patch_traces, c_pointcontents);
		c_traces = 0;
		c_brush_traces = 0;
		c_patch_traces = 0;
		c_terrain_patch_traces = 0;
		c_pointcontents = 0;
	}

	// old net chan encryption key
	key = lastTime * 0x87243987;

	com_frameNumber++;
}

/*
=================
Com_Shutdown
=================
*/
void Com_Shutdown (void) {
	if (logfile) {
		FS_FCloseFile (logfile);
		logfile = 0;
	}

	if ( com_journalFile ) {
		FS_FCloseFile( com_journalFile );
		com_journalFile = 0;
	}

}

qboolean Com_SanitizeName( const char *pszOldName, char *pszNewName )
{
	int i;
	qboolean bBadName = false;
	const char *p = pszOldName;

	if( *pszOldName && *pszOldName <= ' ' )
	{
		bBadName = true;

		while( *p && *p <= ' ' )
		{
			p++;
		}
	}

	i = 0;
	for( i = 0; *p && *p >= ' '; p++, i++ )
	{
		if( *p == '~' || *p == '`' )
		{
			pszNewName[ i ] = '*';
			bBadName = true;
		}
		else if( *p == '\"' )
		{
			pszNewName[ i ] = '\'';
			bBadName = true;
		}
		else if( *p == '\\' )
		{
			pszNewName[ i ] = '/';
			bBadName = true;
		}
		else if( *p == ';' )
		{
			pszNewName[ i ] = ':';
			bBadName = true;
		}
		else
		{
			pszNewName[ i ] = *p;
		}
	}

	if( i > 0 && pszNewName[ i - 1 ] <= ' ' )
	{
		bBadName = true;
		do
		{
			p++;
		} while( i > 0 && pszNewName[ i - 1 ] <= ' ' );
	}
	pszNewName[ i ] = 0;

	if( !i )
	{
		memcpy( pszNewName, "*** Blank Name ***", sizeof( "*** Blank Name ***" ) );
		bBadName = true;
	}

	if( *p )
		bBadName = true;

	return bBadName;
}

const char *Com_GetArchiveFileName( const char *filename, const char *extension )
{
	static char name[ 256 ];
	Com_sprintf( name, sizeof( name ), "%s/%s.%s", Com_GetArchiveFolder(), filename, extension );
	return name;
}

const char *Com_GetArchiveFolder()
{
	static char name[ 256 ];
	char trimmedconfig[ 256 ];

	COM_StripExtension( config->string, trimmedconfig, sizeof( trimmedconfig ) );
	Com_sprintf( name, sizeof( name ), "%s/%s", "save", trimmedconfig );
	return name;
}

void Com_WipeSavegame( const char *savename )
{
	Com_DPrintf( "Com_WipeSaveGame(%s)\n", savename );
	FS_DeleteFile( Com_GetArchiveFileName( savename, "sav" ) );
	FS_DeleteFile( Com_GetArchiveFileName( savename, "ssv" ) );
	FS_DeleteFile( Com_GetArchiveFileName( savename, "tga" ) );
}

//------------------------------------------------------------------------

/*
===========================================
command line completion
===========================================
*/

/*
==================
Field_Clear
==================
*/
void Field_Clear( field_t *edit ) {
  memset(edit->buffer, 0, MAX_EDIT_LINE);
	edit->cursor = 0;
	edit->scroll = 0;
}

static const char *completionString;
static char shortestMatch[MAX_TOKEN_CHARS];
static int	matchCount;
// field we are working on, passed to Field_AutoComplete(&g_consoleCommand for instance)
static field_t *completionField;

/*
===============
FindMatches

===============
*/
static void FindMatches( const char *s ) {
	int		i;

	if ( Q_stricmpn( s, completionString, strlen( completionString ) ) ) {
		return;
	}
	matchCount++;
	if ( matchCount == 1 ) {
		Q_strncpyz( shortestMatch, s, sizeof( shortestMatch ) );
		return;
	}

	// cut shortestMatch to the amount common with s
	for ( i = 0 ; shortestMatch[i] ; i++ ) {
		if ( i >= strlen( s ) ) {
			shortestMatch[i] = 0;
			break;
		}

		if ( tolower(shortestMatch[i]) != tolower(s[i]) ) {
			shortestMatch[i] = 0;
		}
	}
}

/*
===============
PrintMatches

===============
*/
static void PrintMatches( const char *s ) {
	if ( !Q_stricmpn( s, shortestMatch, strlen( shortestMatch ) ) ) {
		Com_Printf( "    %s\n", s );
	}
}

/*
===============
PrintCvarMatches

===============
*/
static void PrintCvarMatches( const char *s ) {
	char value[ TRUNCATE_LENGTH ];

	if ( !Q_stricmpn( s, shortestMatch, strlen( shortestMatch ) ) ) {
		Com_TruncateLongString( value, Cvar_VariableString( s ) );
		Com_Printf( "    %s = \"%s\"\n", s, value );
	}
}

/*
===============
Field_FindFirstSeparator
===============
*/
static char *Field_FindFirstSeparator( char *s )
{
	int i;

	for( i = 0; i < strlen( s ); i++ )
	{
		if( s[ i ] == ';' )
			return &s[ i ];
	}

	return NULL;
}

#ifndef DEDICATED
/*
===============
Field_CompleteKeyname
===============
*/
static void Field_CompleteKeyname( void )
{
	matchCount = 0;
	shortestMatch[ 0 ] = 0;

	Key_KeynameCompletion( FindMatches );

	if( matchCount == 0 )
		return;

	Q_strncpyz( &completionField->buffer[ strlen( completionField->buffer ) -
		strlen( completionString ) ], shortestMatch,
		sizeof( completionField->buffer ) );
	completionField->cursor = ( int )strlen( completionField->buffer );

	if( matchCount == 1 )
	{
		Q_strcat( completionField->buffer, sizeof( completionField->buffer ), " " );
		completionField->cursor++;
		return;
	}

	Com_Printf( "]%s\n", completionField->buffer );

	Key_KeynameCompletion( PrintMatches );
}
#endif

/*
===============
Field_CompleteFilename
===============
*/
static void Field_CompleteFilename( const char *dir,
		const char *ext, qboolean stripExt )
{
	size_t pos;

	matchCount = 0;
	shortestMatch[ 0 ] = 0;

	FS_FilenameCompletion( dir, ext, stripExt, FindMatches );

	if( matchCount == 0 )
		return;

	pos = strlen( completionField->buffer ) - strlen( completionString );
	Q_strncpyz( &completionField->buffer[ pos ], shortestMatch,
		sizeof( completionField->buffer ) - pos );
	completionField->cursor = ( int )strlen( completionField->buffer );

	if( matchCount == 1 )
	{
		Q_strcat( completionField->buffer, sizeof( completionField->buffer ), " " );
		completionField->cursor++;
		return;
	}

	Com_Printf( "]%s\n", completionField->buffer );

	FS_FilenameCompletion( dir, ext, stripExt, PrintMatches );
}

/*
===============
Field_CompleteCommand
===============
*/
static void Field_CompleteCommand( char *cmd,
		qboolean doCommands, qboolean doCvars )
{
	int		completionArgument = 0;
	char	*p;

	// Skip leading whitespace and quotes
	cmd = Com_SkipCharset( cmd, " \"" );

	Cmd_TokenizeStringIgnoreQuotes( cmd );
	completionArgument = Cmd_Argc( );

	// If there is trailing whitespace on the cmd
	if( *( cmd + strlen( cmd ) - 1 ) == ' ' )
	{
		completionString = "";
		completionArgument++;
	}
	else
		completionString = Cmd_Argv( completionArgument - 1 );

#ifndef DEDICATED
	// Unconditionally add a '\' to the start of the buffer
	if( completionField->buffer[ 0 ] &&
			completionField->buffer[ 0 ] != '\\' )
	{
		if( completionField->buffer[ 0 ] != '/' )
		{
			// Buffer is full, refuse to complete
			if( strlen( completionField->buffer ) + 1 >=
				sizeof( completionField->buffer ) )
				return;

			memmove( &completionField->buffer[ 1 ],
				&completionField->buffer[ 0 ],
				strlen( completionField->buffer ) + 1 );
			completionField->cursor++;
		}

		completionField->buffer[ 0 ] = '\\';
	}
#endif

	if( completionArgument > 1 )
	{
		const char *baseCmd = Cmd_Argv( 0 );

#ifndef DEDICATED
		// This should always be true
		if( baseCmd[ 0 ] == '\\' || baseCmd[ 0 ] == '/' )
			baseCmd++;
#endif

		if( ( p = Field_FindFirstSeparator( cmd ) ) )
		{
			// Compound command
			Field_CompleteCommand( p + 1, qtrue, qtrue );
		}
		else
		{
			// FIXME: all this junk should really be associated with the respective
			// commands, instead of being hard coded here
			if( ( !Q_stricmp( baseCmd, "map" ) ||
						!Q_stricmp( baseCmd, "devmap" ) ||
						!Q_stricmp( baseCmd, "spmap" ) ||
						!Q_stricmp( baseCmd, "spdevmap" ) ) &&
					completionArgument == 2 )
			{
				Field_CompleteFilename( "maps", "bsp", qtrue );
			}
			else if( ( !Q_stricmp( baseCmd, "exec" ) ||
						!Q_stricmp( baseCmd, "writeconfig" ) ) &&
					completionArgument == 2 )
			{
				Field_CompleteFilename( "", "cfg", qfalse );
			}
			else if( !Q_stricmp( baseCmd, "condump" ) &&
					completionArgument == 2 )
			{
				Field_CompleteFilename( "", "txt", qfalse );
			}
			else if( ( !Q_stricmp( baseCmd, "toggle" ) ||
						!Q_stricmp( baseCmd, "vstr" ) ||
						!Q_stricmp( baseCmd, "set" ) ||
						!Q_stricmp( baseCmd, "seta" ) ||
						!Q_stricmp( baseCmd, "setu" ) ||
						!Q_stricmp( baseCmd, "sets" ) ) &&
					completionArgument == 2 )
			{
				// Skip "<cmd> "
				p = Com_SkipTokens( cmd, 1, " " );

				if( p > cmd )
					Field_CompleteCommand( p, qfalse, qtrue );
			}
#ifndef DEDICATED
			else if( !Q_stricmp( baseCmd, "demo" ) && completionArgument == 2 )
			{
				char demoExt[ 16 ];

				Com_sprintf( demoExt, sizeof( demoExt ), ".dm_%d", PROTOCOL_VERSION );
				Field_CompleteFilename( "demos", demoExt, qtrue );
			}
			else if( !Q_stricmp( baseCmd, "rcon" ) && completionArgument == 2 )
			{
				// Skip "rcon "
				p = Com_SkipTokens( cmd, 1, " " );

				if( p > cmd )
					Field_CompleteCommand( p, qtrue, qtrue );
			}
			else if( !Q_stricmp( baseCmd, "bind" ) )
			{
				if( completionArgument == 2 )
				{
					// Skip "bind "
					p = Com_SkipTokens( cmd, 1, " " );

					if( p > cmd )
						Field_CompleteKeyname( );
				}
				else if( completionArgument >= 3 )
				{
					// Skip "bind <key> "
					p = Com_SkipTokens( cmd, 2, " " );

					if( p > cmd )
						Field_CompleteCommand( p, qtrue, qtrue );
				}
			}
			// su44: spawn command can be used only in non-dedicated build
			else if( !Q_stricmp( baseCmd, "spawn" ) && completionArgument == 2 )
			{
				Field_CompleteFilename( "models", "tik", qfalse );
			}
#endif
			// su44: these commands can be used in dedicated build as well
			else if( ( !Q_stricmp( baseCmd, "tiki" ) ||
						!Q_stricmp( baseCmd, "tiki_dumpbones" ) ||
						!Q_stricmp( baseCmd, "tiki_info" ) ) &&
					completionArgument == 2 )
			{
				Field_CompleteFilename( "models", "tik", qfalse );
			}
		}
	}
	else
	{
		size_t completionOffset;

		if( completionString[0] == '\\' || completionString[0] == '/' )
			completionString++;

		matchCount = 0;
		shortestMatch[ 0 ] = 0;

		if( strlen( completionString ) == 0 )
			return;

		if( doCommands )
			Cmd_CommandCompletion( FindMatches );

		if( doCvars )
			Cvar_CommandCompletion( FindMatches );

		if( matchCount == 0 )
			return; // no matches

		completionOffset = strlen( completionField->buffer ) - strlen( completionString );

		Q_strncpyz( &completionField->buffer[ completionOffset ], shortestMatch,
			sizeof( completionField->buffer ) - completionOffset );

		completionField->cursor = ( int )strlen( completionField->buffer );

		if( matchCount == 1 )
		{
			Q_strcat( completionField->buffer, sizeof( completionField->buffer ), " " );
			completionField->cursor++;
			return;
		}

		Com_Printf( "]%s\n", completionField->buffer );

		// run through again, printing matches
		if( doCommands )
			Cmd_CommandCompletion( PrintMatches );

		if( doCvars )
			Cvar_CommandCompletion( PrintCvarMatches );
	}
}

/*
===============
Field_AutoComplete

Perform Tab expansion
===============
*/
void Field_AutoComplete( field_t *field )
{
	completionField = field;

	Field_CompleteCommand( completionField->buffer, qtrue, qtrue );
}

/*
==================
Com_RandomBytes

fills string array with len radom bytes, peferably from the OS randomizer
==================
*/
void Com_RandomBytes( byte *string, int len )
{
	int i;

	if( Sys_RandomBytes( string, len ) )
		return;

	Com_Printf( "Com_RandomBytes: using weak randomization\n" );
	srand( time( 0 ) );
	for( i = 0; i < len; i++ )
		string[i] = (unsigned char)( rand() % 255 );
}

}
