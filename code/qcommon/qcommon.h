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
// qcommon.h -- definitions common between client and server, but not game.or ref modules
#ifndef _QCOMMON_H_
#define _QCOMMON_H_

#include "cm_public.h"
#include "alias.h"

//Ignore __attribute__ on non-gcc platforms
#ifndef __GNUC__
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

//#define	PRE_RELEASE_DEMO

//============================================================================

//
// msg.c
//
typedef struct msg_s {
	qboolean	allowoverflow;	// if false, do a Com_Error
	qboolean	overflowed;		// set to true if the buffer size failed (with allowoverflow set)
	qboolean	oob;			// set to true if the buffer size failed (with allowoverflow set)
	byte	*data;
	size_t	maxsize;
	size_t	cursize;
	int		readcount;
	int		bit;				// for bitwise reads and writes
} msg_t;

void MSG_Init (msg_t *buf, byte *data, size_t length);
void MSG_InitOOB( msg_t *buf, byte *data, size_t length );
void MSG_Clear (msg_t *buf);
void MSG_WriteData (msg_t *buf, const void *data, size_t length);
void MSG_Bitstream(msg_t* buf);
qboolean MSG_IsProtocolVersion15();

// TTimo
// copy a msg_t in case we need to store it as is for a bit
// (as I needed this to keep an msg_t from a static var for later use)
// sets data buffer as MSG_Init does prior to do the copy
void MSG_Copy(msg_t *buf, byte *data, int length, msg_t *src);

struct usercmd_s;
struct entityState_s;
struct playerState_s;

void MSG_WriteBits( msg_t *msg, int value, int bits );

void MSG_WriteChar (msg_t *sb, int c);
void MSG_WriteByte (msg_t *sb, int c);
void MSG_WriteShort (msg_t *sb, int c);

void MSG_WriteSVC( msg_t *sb, int c );

void MSG_WriteLong (msg_t *sb, int c);
void MSG_WriteFloat (msg_t *sb, float f);
void MSG_WriteString (msg_t *sb, const char *s);
void MSG_WriteBigString (msg_t *sb, const char *s);
void MSG_WriteScrambledString(msg_t* sb, const char* s);
void MSG_WriteScrambledBigString(msg_t* sb, const char* s);
void MSG_WriteAngle16 (msg_t *sb, float f);
int MSG_HashKey(const char *string, int maxlen);
void MSG_WriteEntityNum(msg_t* sb, short number);

void	MSG_BeginReading (msg_t *sb);
void	MSG_BeginReadingOOB(msg_t *sb);

int		MSG_ReadBits( msg_t *msg, int bits );

int		MSG_ReadChar (msg_t *sb);
int		MSG_ReadByte (msg_t *sb);
int		MSG_ReadSVC (msg_t *sb);
int		MSG_ReadShort (msg_t *sb);
int		MSG_ReadLong (msg_t *sb);

void MSG_ReadDir( msg_t *msg, vec3_t dir );
float MSG_ReadCoord( msg_t *msg );
void MSG_GetNullEntityState(entityState_t *nullState);

float	MSG_ReadFloat (msg_t *sb);
char	*MSG_ReadString (msg_t *sb);
char	*MSG_ReadBigString (msg_t *sb);
char	*MSG_ReadStringLine( msg_t *sb );
char* MSG_ReadScrambledString(msg_t* msg);
char* MSG_ReadScrambledBigString(msg_t* msg);
float	MSG_ReadAngle8( msg_t *sb );
float	MSG_ReadAngle16 (msg_t *sb);
void	MSG_ReadData (msg_t *sb, void *buffer, int size);
unsigned short MSG_ReadEntityNum(msg_t* sb);


void MSG_WriteDeltaUsercmd( msg_t *msg, struct usercmd_s *from, struct usercmd_s *to );
void MSG_ReadDeltaUsercmd( msg_t *msg, struct usercmd_s *from, struct usercmd_s *to );

void MSG_WriteDeltaUsercmdKey( msg_t *msg, int key, usercmd_t *from, usercmd_t *to );
void MSG_ReadDeltaUsercmdKey( msg_t *msg, int key, usercmd_t *from, usercmd_t *to );

void MSG_WriteDeltaEntity( msg_t *msg, struct entityState_s *from, struct entityState_s *to
						   , qboolean force, float frameTime);

void MSG_ReadSounds (msg_t *msg, server_sound_t *sounds, int *snapshot_number_of_sounds);
void MSG_WriteSounds (msg_t *msg, server_sound_t *sounds, int snapshot_number_of_sounds);

void MSG_ReadDeltaEyeInfo(msg_t *msg, usereyes_t *from, usereyes_t *to);

void MSG_ReadDeltaEntity(msg_t* msg, entityState_t* from, entityState_t* to,
	int number, float frameTime);

void MSG_WriteDeltaEyeInfo (msg_t  *msg, usereyes_t *from, usereyes_t *to);

void MSG_WriteDeltaPlayerstate(msg_t* msg, struct playerState_s* from, struct playerState_s* to, float frameTime);
void MSG_ReadDeltaPlayerstate(msg_t* msg, struct playerState_s* from, struct playerState_s* to, float frameTime);

float MSG_ReadServerFrameTime(msg_t* msg, gameState_t* gameState);
void MSG_WriteServerFrameTime(msg_t* msg, float value);

void MSG_ReportChangeVectors_f( void );

//====================
// TA features
//====================

int MSG_PackAngle(float angle, int bits);
int MSG_PackAnimTime(float time, int bits);
int MSG_PackAnimWeight(float weight, int bits);
int MSG_PackScale(float scale, int bits);
int MSG_PackAlpha(float alpha, int bits);
int MSG_PackCoord(float coord);
int MSG_PackCoordExtra(float coord);
qboolean MSG_DeltaNeeded(const void* fromField, const void* toField, int fieldType, int bits, int size);

void MSG_WritePackedAngle(msg_t* msg, float value, int bits);
void MSG_WritePackedAnimTime(msg_t* msg, float fromValue, float toValue, float frameTime, int bits);
void MSG_WritePackedAnimWeight(msg_t* msg, float value, int bits);
void MSG_WritePackedScale(msg_t* msg, float value, int bits);
void MSG_WritePackedAlpha(msg_t* msg, float value, int bits);
void MSG_WritePackedCoord(msg_t* msg, float fromValue, float toValue, int bits);
void MSG_WritePackedCoordExtra(msg_t* msg, float fromValue, float toValue, int bits);
void MSG_WritePackedVelocity(msg_t* msg, float value, int bits);
void MSG_WritePackedSimple(msg_t* msg, int value, int bits);

float MSG_UnpackAngle(int value, int bits);
float MSG_UnpackAnimTime(int packed);
float MSG_UnpackAnimWeight(int result, int bits);
float MSG_UnpackScale(int packed);
float MSG_UnpackAlpha(int packed, int bits);
float MSG_UnpackCoord(int packed, int bits);
float MSG_UnpackCoordExtra(int packed, int bits);

//============================================================================

/*
==============================================================

NET

==============================================================
*/

#define NET_ENABLEV4            0x01
#define NET_ENABLEV6            0x02
// if this flag is set, always attempt ipv6 connections instead of ipv4 if a v6 address is found.
#define NET_PRIOV6              0x04
// disables ipv6 multicast support if set.
#define NET_DISABLEMCAST        0x08

#define	PACKET_BACKUP	32	// number of old messages that must be kept on client and
							// server for delta comrpession and ping estimation
#define	PACKET_MASK		(PACKET_BACKUP-1)

#define	MAX_PACKET_USERCMDS		32		// max number of usercmd_t in a packet

#define	PORT_ANY			-1

#define	MAX_RELIABLE_COMMANDS	512			// max string commands buffered for restransmit

typedef enum {
	NA_BAD,					// an address lookup failed
    NA_BOT,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX,
    NA_BROADCAST_IPX,
    NA_IP6,
    NA_MULTICAST6,
    NA_UNSPEC
} netadrtype_t;

typedef enum {
	//NS_MASTER,
	NS_CLIENT,
	NS_SERVER
} netsrc_t;

#define NET_ADDRSTRMAXLEN 48	// maximum length of an IPv6 address string including trailing '\0'
typedef struct {
    netadrtype_t	type;

    byte	ip[4];
    byte	ip6[16];
    byte	ipx[10];

    unsigned short	port;
    unsigned long	scope_id;	// Needed for IPv6 link-local addresses
} netadr_t;

void		NET_Init( void );
void		NET_Shutdown( void );
void		NET_Restart_f( void );
void		NET_Config( qboolean enableNetworking );
void		NET_FlushPacketQueue(void);
void		NET_SendPacket (netsrc_t sock, size_t length, const void *data, netadr_t to);
void		QDECL NET_OutOfBandPrint( netsrc_t net_socket, netadr_t adr, const char *format, ...) __attribute__ ((format (printf, 3, 4)));
void		QDECL NET_OutOfBandData( netsrc_t sock, netadr_t adr, byte *format, int len );

qboolean	NET_CompareAdr (netadr_t a, netadr_t b);
qboolean	NET_CompareBaseAdrMask(netadr_t a, netadr_t b, int netmask);
qboolean	NET_CompareBaseAdr (netadr_t a, netadr_t b);
qboolean	NET_IsLocalAddress (netadr_t adr);
const char	*NET_AdrToString (netadr_t a);
const char	*NET_AdrToStringwPort (netadr_t a);
int		NET_StringToAdr ( const char *s, netadr_t *a, netadrtype_t family);
qboolean	NET_GetLoopPacket (netsrc_t sock, netadr_t *net_from, msg_t *net_message);
void		NET_JoinMulticast6(void);
void		NET_LeaveMulticast6(void);
void		NET_Sleep(int msec);


#define	MAX_MSGLEN				49152		// max length of a message, which may
											// be fragmented into multiple packets

#define MAX_DOWNLOAD_WINDOW		48	// ACK window of 48 download chunks. Cannot set this higher, or clients
						// will overflow the reliable commands buffer
#define MAX_DOWNLOAD_BLKSIZE		1024	// 896 byte block chunks

#define NETCHAN_GENCHECKSUM(challenge, sequence) ((challenge) ^ ((sequence) * (challenge)))
 

/*
Netchan handles packet fragmentation and out of order / duplicate suppression
*/

typedef struct {
	netsrc_t	sock;

	int			dropped;			// between last packet and previous

	netadr_t	remoteAddress;
	int			qport;				// qport value to write when transmitting

	// sequencing variables
	int			incomingSequence;
	int			outgoingSequence;

	// incoming fragment assembly buffer
	int			fragmentSequence;
	int			fragmentLength;	
	byte		fragmentBuffer[MAX_MSGLEN];

	// outgoing fragment buffer
	// we need to space out the sending of large fragmented messages
	qboolean	unsentFragments;
	size_t		unsentFragmentStart;
	size_t		unsentLength;
    byte		unsentBuffer[MAX_MSGLEN];

    int			challenge;
    int			lastSentTime;
    int			lastSentSize;

#ifdef LEGACY_PROTOCOL
	qboolean	compat;
#endif
} netchan_t;

/*
Packet profiling
*/

#define NETPROF_PACKET_FRAGMENTED 1
#define NETPROF_PACKET_DROPPED    2
#define NETPROF_PACKET_MESSAGE    4

#define NETPROX_MAX_PACKETS       64

typedef struct {
	int updateTime;
	int size;
	int flags;
} netprofpacket_t;

typedef struct {
	int updateTime;
	int index;
	netprofpacket_t packets[NETPROX_MAX_PACKETS];
	int lastCalcTime;
	int lowestUpdateTime;
	int highestUpdateTime;
	int totalProcessed;
	int totalSize;
	int numFragmented;
	int numDropped;
	int numConnectionLess;
	int totalLengthConnectionLess;
	int packetsPerSec;
	int bytesPerSec;
	int percentFragmented;
	int percentDropped;
	int percentConnectionLess;
	float latency;
} netprofpacketlist_t;

typedef struct {
	qboolean initialized;
	int rate;
	netprofpacketlist_t outPackets;
	netprofpacketlist_t inPackets;
} netprofclient_t;

void Netchan_Init( int qport );
void Netchan_Setup(netsrc_t sock, netchan_t *chan, netadr_t adr, int qport, int challenge, qboolean compat);

void Netchan_Transmit( netchan_t *chan, size_t length, const byte *data, netprofpacketlist_t *packetlist );
void Netchan_TransmitNextFragment( netchan_t *chan, netprofpacketlist_t *packetlist );

qboolean Netchan_Process( netchan_t *chan, msg_t *msg, netprofpacketlist_t *packetlist );

void NetProfileAddPacket(netprofpacketlist_t* list, size_t size, int flags);
void NetProfileSetPacketFlags(netprofpacketlist_t* list, int flags);
void NetProfileCalcStats(netprofpacketlist_t* list, int time);

/*
==============================================================

PROTOCOL

==============================================================
*/

#define	PROTOCOL_VERSION			17
#define PROTOCOL_LEGACY_VERSION		PROTOCOL_VERSION
// su44: MoHAA v 1.00 uses protocol version 6

// maintain a list of compatible protocols for demo playing
// NOTE: that stuff only works with two digits protocols
extern int demo_protocols[];

// fixme : we need a new master server address

#define	UPDATE_SERVER_NAME	"updatemoh.2015.com"
// override on command line, config files etc.
#ifndef MASTER_SERVER_NAME
#define MASTER_SERVER_NAME	"master.2015.com"
#endif
#ifndef AUTHORIZE_SERVER_NAME
#define	AUTHORIZE_SERVER_NAME	"authorize.2015.com"
#endif

#define	PORT_MASTER			27950
#define	PORT_UPDATE			27951
#ifndef PORT_AUTHORIZE
#define	PORT_AUTHORIZE		27952
#endif
#define	PORT_SERVER			12203
#define	NUM_SERVER_PORTS	4		// broadcast scan this many ports after
									// PORT_SERVER so a single machine can
									// run multiple servers


// the svc_strings[] array in cl_parse.c should mirror this
//
// server to client
//
enum svc_ops_e {
	svc_bad,
	svc_nop,
	svc_gamestate,
	svc_configstring,			// [short] [string] only in gamestate messages
	svc_baseline,				// only in gamestate messages
	svc_serverCommand,			// [string] to be executed by client game module
	svc_download,				// [short] size [size bytes]
	svc_snapshot,
	svc_centerprint,
	svc_locprint,
	svc_cgameMessage,
	svc_EOF,

// new commands, supported only by ioquake3 protocol but not legacy
	svc_voipSpeex,     // not wrapped in USE_VOIP, so this value is reserved.
	svc_voipOpus,      //
};

//
// client to server
//
enum clc_ops_e {
	clc_bad,
	clc_nop, 		
	clc_move,				// [[usercmd_t]
	clc_moveNoDelta,		// [[usercmd_t]
	clc_clientCommand,		// [string] message
	clc_EOF,

// new commands, supported only by ioquake3 protocol but not legacy
	clc_voipSpeex,   // not wrapped in USE_VOIP, so this value is reserved.
	clc_voipOpus,    //
};

/*
==============================================================

VIRTUAL MACHINE

==============================================================
*/

typedef struct vm_s vm_t;

typedef enum {
	VMI_NATIVE,
	VMI_BYTECODE,
	VMI_COMPILED
} vmInterpret_t;

typedef enum {
	TRAP_MEMSET = 100,
	TRAP_MEMCPY,
	TRAP_STRNCPY,
	TRAP_SIN,
	TRAP_COS,
	TRAP_ATAN2,
	TRAP_SQRT,
	TRAP_MATRIXMULTIPLY,
	TRAP_ANGLEVECTORS,
	TRAP_PERPENDICULARVECTOR,
	TRAP_FLOOR,
	TRAP_CEIL,

	TRAP_TESTPRINTINT,
	TRAP_TESTPRINTFLOAT
} sharedTraps_t;

typedef intptr_t (QDECL *vmMainProc)(int callNum, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11);

void	VM_Init( void );
vm_t	*VM_Create( const char *module, intptr_t (*systemCalls)(intptr_t *), 
				   vmInterpret_t interpret );
// module should be bare: "cgame", not "cgame.dll" or "vm/cgame.qvm"

void	VM_Free( vm_t *vm );
void	VM_Clear(void);
void	VM_Forced_Unload_Start(void);
void	VM_Forced_Unload_Done(void);
vm_t	*VM_Restart(vm_t *vm, qboolean unpure);

intptr_t		QDECL VM_Call( vm_t *vm, int callNum, ... );

void	VM_Debug( int level );

void	*VM_ArgPtr( intptr_t intValue );
void	*VM_ExplicitArgPtr( vm_t *vm, intptr_t intValue );

#define	VMA(x) VM_ArgPtr(args[x])
static ID_INLINE float _vmf(intptr_t x)
{
    floatint_t fi;
    fi.i = (int)x;
    return fi.f;
}
#define	VMF(x)	_vmf(args[x])


/*
==============================================================

CMD

Command text buffering and command execution

==============================================================
*/

/*

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but entire text
files can be execed.

*/

void Cbuf_Init (void);
// allocates an initial text buffer that will grow as needed

void Cbuf_AddText( const char *text );
// Adds command text at the end of the buffer, does NOT add a final \n

void Cbuf_ExecuteText(int exec_when, const char *text );
// this can be used in place of either Cbuf_AddText or Cbuf_InsertText

void Cbuf_Execute (int msec);
// Pulls off \n terminated lines of text from the command buffer and sends
// them through Cmd_ExecuteString.  Stops when the buffer is empty.
// Normally called once per frame, but may be explicitly invoked.
// Do not call inside a command function, or current args will be destroyed.

void Cmd_WriteAliases( fileHandle_t f );

//===========================================================================

/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

*/

typedef void (*xcommand_t) (void);

void	Cmd_Init (void);

void	Cmd_AddCommand( const char *cmd_name, xcommand_t function );
// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory
// if function is NULL, the command will be forwarded to the server
// as a clc_clientCommand instead of executed locally

void	Cmd_RemoveCommand( const char *cmd_name );

typedef void (*completionFunc_t)( const char *args, int argNum );

// don't allow VMs to remove system commands
void	Cmd_RemoveCommandSafe( const char *cmd_name );

void	Cmd_CommandCompletion( void(*callback)(const char *s) );
// callback with each valid string
void Cmd_SetCommandCompletionFunc( const char *command,
	completionFunc_t complete );
void Cmd_CompleteArgument( const char *command, char *args, int argNum );
void Cmd_CompleteCfgName( const char *args, int argNum );
void	Cmd_CommandCompletion( void(*callback)(const char *s) );
const char* Cmd_CompleteCommandByNumber(const char* partial, int number);
// callback with each valid string

int		Cmd_Argc (void);
char	*Cmd_Argv (int arg);
void	Cmd_ArgvBuffer( int arg, char *buffer, int bufferLength );
char	*Cmd_Args (void);
char	*Cmd_ArgsFrom( int arg );
void	Cmd_ArgsBuffer( char *buffer, int bufferLength );
char	*Cmd_Cmd (void);
void	Cmd_Args_Sanitize( void );
// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg > argc, so string operations are allways safe.

void	Cmd_TokenizeString( const char *text );
void	Cmd_TokenizeStringIgnoreQuotes( const char *text_in );
// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.

void	Cmd_ExecuteString( const char *text );
// Parses a single line of text into arguments and tries to execute it
// as if it was typed at the console


/*
==============================================================

CVAR

==============================================================
*/

/*

cvar_t variables are used to hold scalar or string variables that can be changed
or displayed at the console or prog code as well as accessed directly
in C code.

The user can access cvars from the console in three ways:
r_draworder			prints the current value
r_draworder 0		sets the current value to 0
set r_draworder 0	as above, but creates the cvar if not present

Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.

The are also occasionally used to communicated information between different
modules of the program.

*/

// sets the default value of a variable
void Cvar_SetDefault( cvar_t *var, const char *var_value );

cvar_t *Cvar_Get( const char *var_name, const char *value, int flags );
// creates the variable if it doesn't exist, or returns the existing one
// if it exists, the value will not be changed, but flags will be ORed in
// that allows variables to be unarchived without needing bitflags
// if value is "", the value will not override a previously set value.

void	Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
// basically a slightly modified Cvar_Get for the interpreted modules

void	Cvar_Update( vmCvar_t *vmCvar );
// updates an interpreted modules' version of a cvar

void 	Cvar_Set( const char *var_name, const char *value );
// will create the variable with no flags if it doesn't exist

cvar_t	*Cvar_Set2( const char *var_name, const char *value, qboolean force );
// will create the variable with no flags if it doesn't exist
// can force

cvar_t	*Cvar_Next( cvar_t *var );
// gets the next var

void Cvar_SetLatched( const char *var_name, const char *value);
// don't set the cvar immediately

void	Cvar_SetValue( const char *var_name, float value );
// expands value to a string and calls Cvar_Set

cvar_t	*Cvar_FindVar( const char *var_name );
float	Cvar_VariableValue( const char *var_name );
int		Cvar_VariableIntegerValue(const char* var_name);
float	Cvar_VariableFloatValue(const char* var_name);
// returns 0 if not defined or non numeric

char	*Cvar_VariableString( const char *var_name );
void	Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

// find a compelte variable
const char *Cvar_CompleteVariable( const char *partial );
const char *Cvar_CompleteVariableByNumber( const char *partial, int number );
// returns an empty string if not defined

int	Cvar_Flags(const char *var_name);
// returns CVAR_NONEXISTENT if cvar doesn't exist or the flags of that particular CVAR.

void	Cvar_CommandCompletion( void(*callback)(const char *s) );
// callback with each valid string

void 	Cvar_Reset( const char *var_name );
void 	Cvar_ForceReset(const char *var_name);

void	Cvar_SetCheatState( void );
// reset all testing vars to a safe value

qboolean Cvar_Command( void );
// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

void 	Cvar_WriteVariables( fileHandle_t f );
// writes lines containing "set variable value" for all variables
// with the archive flag set to true.

void	Cvar_Init( void );

char	*Cvar_InfoString( int bit );
char	*Cvar_InfoString_Big( int bit );
// returns an info string containing all the cvars that have the given bit set
// in their flags ( CVAR_USERINFO, CVAR_SERVERINFO, CVAR_SYSTEMINFO, etc )
void	Cvar_InfoStringBuffer( int bit, char *buff, int buffsize );
void Cvar_CheckRange( cvar_t *cv, float minVal, float maxVal, qboolean shouldBeIntegral );
void Cvar_SetDescription( cvar_t *var, const char *var_description );

void	Cvar_Restart(qboolean unsetVM);
void	Cvar_Restart_f( void );
void	Cvar_SaveGameRestart_f( void );

void	Cvar_CompleteCvarName(const char* args, int argNum);

extern	int			cvar_modifiedFlags;
extern	qboolean	cvar_global_force;
// whenever a cvar is modifed, its flags will be OR'd into this, so
// a single check can determine if any CVAR_USERINFO, CVAR_SERVERINFO,
// etc, variables have been modified since the last check.  The bit
// can then be cleared to allow another change detection.

/*
==============================================================

FILESYSTEM

No stdio calls should be used by any part of the game, because
we need to deal with all sorts of directory and seperator char
issues.
==============================================================
*/

// referenced flags
// these are in loop specific order so don't change the order
#define FS_GENERAL_REF	0x01
#define FS_UI_REF		0x02
#define FS_CGAME_REF	0x04
#define FS_QAGAME_REF	0x08
// number of id paks that will never be autodownloaded from baseq3
#define NUM_ID_PAKS		5

#define	MAX_FILE_HANDLES	64

#ifdef DEDICATED
#	define Q3CONFIG_CFG "q3config_server.cfg"
#else
#	define Q3CONFIG_CFG "q3config.cfg"
#endif

qboolean FS_Initialized( void );

void	FS_InitFilesystem( void );
void	FS_Shutdown( qboolean closemfp );

qboolean	FS_ConditionalRestart( int checksumFeed, qboolean disconnect );
void	FS_Restart( int checksumFeed );
// shutdown and restart the filesystem so changes to fs_gamedir can take effect

void FS_AddGameDirectory(const char *path, const char *dir);
void FS_AddGameDirectory2(const char *path, const char *dir, qboolean original_paks_priority);

char	**FS_ListFilteredFiles( const char *path, const char *extension, const char *filter, qboolean wantSubs, int *numfiles, qboolean allowNonPureFilesOnDisk );
char	**FS_ListFiles( const char *directory, const char *extension, qboolean wantSubs, int *numfiles );
// directory should not have either a leading or trailing /
// if extension is "/", only subdirectories will be returned
// the returned files will not include any directories or /

void	FS_FreeFileList( char **list );

qboolean FS_FileExists( const char *file );

void FS_CorrectCase(char* path);

char   *FS_BuildOSPath( const char *base, const char *game, const char *qpath );

qboolean FS_CreatePath(char *OSPath);

int		FS_LoadStack( void );

int		FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize );
int		FS_GetModList(  char *listbuf, int bufsize );

void	FS_GetModDescription(const char* modDir, char* description, int descriptionLen);

fileHandle_t	FS_FOpenFileWrite( const char *qpath );
fileHandle_t	FS_FOpenFileAppend( const char *filename );
fileHandle_t	FS_FCreateOpenPipeFile( const char *filename );
fileHandle_t	FS_FOpenTextFileWrite(const char* qpath);
// will properly create any needed paths and deal with seperater character issues

void	FS_DeleteFile( const char *filename );
void	FS_CanonicalFilename( char *filename );
fileHandle_t FS_SV_FOpenFileWrite( const char *filename );
long	FS_SV_FOpenFileRead( const char *filename, fileHandle_t *fp );
void	FS_SV_Rename( const char *from, const char *to, qboolean safe );
long	FS_FOpenFileRead(const char* filename, fileHandle_t* file, qboolean uniqueFILE, qboolean quiet);
// if uniqueFILE is true, then a new FILE will be fopened even if the file
// is found in an already open pak file.  If uniqueFILE is false, you must call
// FS_FCloseFile instead of fclose, otherwise the pak FILE would be improperly closed
// It is generally safe to always set uniqueFILE to true, because the majority of
// file IO goes through FS_ReadFile, which Does The Right Thing already.

int		FS_FileIsInPAK(const char *filename, int *pChecksum );
// returns 1 if a file is in the PAK file, otherwise -1

size_t	FS_Write( const void *buffer, size_t len, fileHandle_t f );

size_t	FS_Read( void *buffer, size_t len, fileHandle_t f );
// properly handles partial reads and reads from other dlls

void	FS_FCloseFile( fileHandle_t f );
// note: you can't just fclose from another DLL, due to MS libc issues

long		FS_ReadFileEx(const char* qpath, void** buffer, qboolean quiet);
long		FS_ReadFile(const char* qpath, void** buffer);
const char	*FS_Gamedir( void );
// returns the length of the file
// a null buffer will just return the file length without loading
// as a quick check for existance. -1 length == not present
// A 0 byte will always be appended at the end, so string ops are safe.
// the buffer should be considered read-only, because it may be cached
// for other uses.

void	FS_ForceFlush( fileHandle_t f );
// forces flush on files we're writing to.

void	FS_FreeFile( void *buffer );
// frees the memory returned by FS_ReadFile


const char	*FS_PrepFileWrite( const char *filename );
// prepares the file to be written

int     FS_WriteFile( const char *qpath, const void *buffer, int size );
// writes a complete file, creating any subdirectories needed

void	FS_WriteTextFile( const char *qpath, const void *buffer, int size );
// writes a complete text file, creating any subdirectories needed

long FS_filelength(fileHandle_t f);
// doesn't work for files that are opened from a pack file

int		FS_FTell( fileHandle_t f );
// where are we?

void	FS_Flush( fileHandle_t f );

void 	QDECL FS_Printf( fileHandle_t f, const char *fmt, ... ) __attribute__ ((format (printf, 2, 3)));
// like fprintf

int		FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
// opens a file for reading, writing, or appending depending on the value of mode

int		FS_Seek( fileHandle_t f, long offset, int origin );
// seek on a file (doesn't work for zip files!!!!!!!!)

qboolean FS_FilenameCompare( const char *s1, const char *s2 );
qboolean FS_FileNewer( const char *source, const char *destination );

const char *FS_GamePureChecksum( void );
// Returns the checksum of the pk3 from which the server loaded the qagame.qvm

const char *FS_LoadedPakNames( void );
const char *FS_LoadedPakChecksums( void );
const char *FS_LoadedPakPureChecksums( void );
// Returns a space separated string containing the checksums of all loaded pk3 files.
// Servers with sv_pure set will get this string and pass it to clients.

const char *FS_ReferencedPakNames( void );
const char *FS_ReferencedPakChecksums( void );
const char *FS_ReferencedPakPureChecksums( void );
// Returns a space separated string containing the checksums of all loaded 
// AND referenced pk3 files. Servers with sv_pure set will get this string 
// back from clients for pure validation 

void FS_ClearPakReferences( int flags );
// clears referenced booleans on loaded pk3s

void FS_PureServerSetReferencedPaks( const char *pakSums, const char *pakNames );
void FS_PureServerSetLoadedPaks( const char *pakSums, const char *pakNames );
// If the string is empty, all data sources will be allowed.
// If not empty, only pk3 files that match one of the space
// separated checksums will be checked for files, with the
// sole exception of .cfg files.

qboolean FS_CheckDirTraversal(const char *checkdir);
qboolean FS_idPak(const char *pak, const char *base);
qboolean FS_ComparePaks( char *neededpaks, int len, qboolean dlstring );

void FS_Rename( const char *from, const char *to );

void FS_Remove( const char *osPath );

void	FS_FilenameCompletion( const char *dir, const char *ext,
		qboolean stripExt, void(*callback)(const char *s), qboolean allowNonPureFilesOnDisk );

const char* FS_GetCurrentGameDir();
void	FS_GetRelativeFilename( const char *currentDirectory, const char *absoluteFilename, char *out, size_t destlen );

void FS_FileTime(const char* filename, char* date, char* size);

extern cvar_t* fs_debug;
extern cvar_t* fs_mapdir;

/*
==============================================================

Edit fields and command line history/completion

==============================================================
*/

#define	MAX_EDIT_LINE	256
typedef struct {
	int		cursor;
	int		scroll;
	int		widthInChars;
	char	buffer[MAX_EDIT_LINE];
} field_t;

void Field_Clear( field_t *edit );
void Field_AutoComplete( field_t *edit );
void Field_CompleteKeyname( void );
void Field_CompleteFilename( const char *dir,
		const char *ext, qboolean stripExt, qboolean allowNonPureFilesOnDisk );
void Field_CompleteCommand( char *cmd,
		qboolean doCommands, qboolean doCvars );
void Field_CompletePlayerName( const char **names, int count );
/*
==============================================================

MISC

==============================================================
*/

// returnbed by Sys_GetProcessorId
#define CPUID_GENERIC			0			// any unrecognized processor

#define CPUID_AXP				0x10

#define CPUID_INTEL_UNSUPPORTED	0x20			// Intel 386/486
#define CPUID_INTEL_PENTIUM		0x21			// Intel Pentium or PPro
#define CPUID_INTEL_MMX			0x22			// Intel Pentium/MMX or P2/MMX
#define CPUID_INTEL_KATMAI		0x23			// Intel Katmai

#define CPUID_AMD_3DNOW			0x30			// AMD K6 3DNOW!

/*
==============================================================

MISC

==============================================================
*/

// vsnprintf is ISO/IEC 9899:1999
// abstracting this to make it portable
#ifdef _WIN32
#define Q_vsnprintf _vsnprintf
#define Q_snprintf _snprintf
#else
#define Q_vsnprintf vsnprintf
#define Q_snprintf snprintf
#endif

// centralizing the declarations for cl_cdkey
// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=470
extern char cl_cdkey[34];

// returned by Sys_GetProcessorFeatures
typedef enum
{
  CF_RDTSC      = 1 << 0,
  CF_MMX        = 1 << 1,
  CF_MMX_EXT    = 1 << 2,
  CF_3DNOW      = 1 << 3,
  CF_3DNOW_EXT  = 1 << 4,
  CF_SSE        = 1 << 5,
  CF_SSE2       = 1 << 6,
  CF_ALTIVEC    = 1 << 7
} cpuFeatures_t;

// centralized and cleaned, that's the max string you can send to a Com_Printf / Com_DPrintf (above gets truncated)
#define	MAXPRINTMSG	4096


#define	MAX_RENDER_STRINGS			8
#define	MAX_RENDER_STRING_LENGTH	32

char		*CopyString( const char *in );
void		Info_Print( const char *s );

void		Com_BeginRedirect (char *buffer, int buffersize, void (*flush)(char *));
void		Com_EndRedirect( void );
void 		QDECL Com_Printf( const char *fmt, ... ) __attribute__ ((format (printf, 1, 2)));
void 		QDECL Com_DPrintf( const char *fmt, ... ) __attribute__ ((format (printf, 1, 2)));
void 		QDECL Com_Error( int code, const char *fmt, ... ) __attribute__ ((noreturn, format(printf, 2, 3)));
void 		Com_Quit_f( void ) __attribute__ ((noreturn));
void		Com_GameRestart(int checksumFeed, qboolean disconnect);

int			Com_Milliseconds( void );	// will be journaled properly
unsigned	Com_BlockChecksum( const void *buffer, int length );
char		*Com_MD5File(const char *filename, int length, const char *prefix, int prefix_len);
int			Com_HashKey(char *string, int maxlen);
int			Com_Filter(const char *filter, const char *name, int casesensitive);
int			Com_FilterPath(const char *filter, const char *name, int casesensitive);
int			Com_RealTime(qtime_t *qtime);
qboolean	Com_SafeMode( void );
void		Com_RunAndTimeServerPacket(netadr_t *evFrom, msg_t *buf);

qboolean	Com_IsVoipTarget(uint8_t *voipTargets, int voipTargetsSize, int clientNum);

void		Com_StartupVariable( const char *match );
// checks for and removes command line "+set var arg" constructs
// if match is NULL, all set commands will be executed, otherwise
// only a set with the exact name.  Only used during startup.
qboolean		Com_PlayerNameToFieldString( char *str, int length, const char *name );
qboolean		Com_FieldStringToPlayerName( char *name, int length, const char *rawname );
int QDECL	Com_strCompare( const void *a, const void *b );


extern	float currentfps;
extern	cvar_t	*paused;
extern	cvar_t	*config;
extern	cvar_t	*fps;
extern	cvar_t	*developer;
extern	cvar_t	*com_dedicated;
extern	cvar_t	*com_speeds;
extern	cvar_t	*com_timescale;
extern	cvar_t	*com_sv_running;
extern	cvar_t	*com_cl_running;
extern	cvar_t	*com_viewlog;			// 0 = hidden, 1 = visible, 2 = minimized
extern	cvar_t	*com_shortversion;
extern	cvar_t	*com_version;
extern	cvar_t	*autopaused;
extern	cvar_t	*deathmatch;
extern	cvar_t	*com_blood;
extern	cvar_t	*com_buildScript;		// for building release pak files
extern	cvar_t	*com_radar_range;
extern	cvar_t	*console_spam_filter;
extern	cvar_t	*com_journal;
extern	cvar_t	*com_cameraMode;
extern	cvar_t	*com_ansiColor;
extern	cvar_t	*com_unfocused;
extern	cvar_t	*com_maxfpsUnfocused;
extern	cvar_t	*com_minimized;
extern	cvar_t	*com_maxfpsMinimized;
extern	cvar_t	*com_altivec;
extern	cvar_t	*com_standalone;
extern	cvar_t	*com_gamename;
extern	cvar_t	*com_basegame;
extern	cvar_t	*com_homepath;
extern	cvar_t	*com_altivec;

// both client and server must agree to pause
extern	cvar_t	*cl_paused;
extern	cvar_t	*sv_paused;

extern	cvar_t	*cl_packetdelay;
extern	cvar_t	*sv_packetdelay;

extern	cvar_t* com_protocol;
#ifdef LEGACY_PROTOCOL
extern	cvar_t* com_legacyprotocol;
#endif
#ifndef DEDICATED
extern  cvar_t* con_autochat;
#endif
extern	cvar_t* com_target_version;
extern	cvar_t* com_target_game;
extern	cvar_t* com_target_demo;
extern	cvar_t* com_updatecheck_enabled;
extern	cvar_t* com_updatecheck_interval;

extern	int		protocol_version_demo;
extern	int		protocol_version_full;

// com_speeds times
extern	int		time_game;
extern	int		time_frontend;
extern	int		time_backend;		// renderer backend time

extern	int		com_frameTime;
extern	int		com_frameMsec;

extern	qboolean	com_errorEntered;
extern	qboolean	com_fullyInitialized;
extern	qboolean	com_gotOriginalConfig;

extern	fileHandle_t	com_journalFile;
extern	fileHandle_t	com_journalDataFile;

typedef enum {
	TAG_FREE,
	TAG_CONST,
	TAG_GENERAL,
	TAG_BOTLIB,
	TAG_RENDERER,				// memory allocated from renderer
	TAG_GAME,					// memory allocated from game
	TAG_CGAME,					// memory allocated from client game
	TAG_CLIENT,					// memory allocated from client
	TAG_TIKI,					// su44: memory allocated by qcommon/tiki.c code
	TAG_STRINGS_AND_COMMANDS,
	TAG_SOUND,					// memory allocated from sound
	TAG_STATIC,
	TAG_STATIC_RENDERER,
	TAG_SKEL,					// memory allocated from skeletor
	TAG_TEMP,					// temporary memory
	TAG_EXE,
	TAG_NUM_TOTAL_TAGS
} memtag_t;

/*

--- low memory ----
server vm
server clipmap
---mark---
renderer initialization (shaders, etc)
UI vm
cgame vm
renderer map
renderer models

---free---

temp file loading
--- high memory ---

*/

#if defined(_DEBUG) && !defined(BSPC)
	#define ZONE_DEBUG
#endif

#ifdef ZONE_DEBUG
#define Z_TagMalloc(size, tag)			Z_TagMallocDebug(size, tag, #size, __FILE__, __LINE__)
#define Z_Malloc(size)					Z_MallocDebug(size, #size, __FILE__, __LINE__)
#define S_Malloc(size)					S_MallocDebug(size, #size, __FILE__, __LINE__)
void *Z_TagMallocDebug( int size, int tag, const char *label, const char *file, int line );	// NOT 0 filled memory
void *Z_MallocDebug( int size, const char *label, const char *file, int line );			// returns 0 filled memory
void *S_MallocDebug( int size, const char *label, const char *file, int line );			// returns 0 filled memory
#else
void *Z_TagMalloc( int size, int tag );	// NOT 0 filled memory
void *Z_Malloc( int size );			// returns 0 filled memory
void *S_Malloc( int size );			// NOT 0 filled memory only for small allocations
#endif
void Z_Free( void *ptr );
void Z_FreeTags( int tag );
int Z_AvailableMemory( void );
void Z_LogHeap( void );

void Hunk_Clear( void );
void Hunk_ClearToMark( void );
void Hunk_SetMark( void );
qboolean Hunk_CheckMark( void );
void Hunk_ClearTempMemory( void );
void *Hunk_AllocateTempMemory( int size );
void Hunk_FreeTempMemory( void *buf );
int	Hunk_MemoryRemaining( void );
void Hunk_Log( void);

void Com_TouchMemory( void );

const char* Z_EmptyStringPointer(void);
const char* Z_NumberStringPointer(int iNum);

// commandLine should not include the executable name (argv[0])
void Com_Init( char *commandLine );
void Com_Frame( void );
void Com_Shutdown( void );

qboolean Com_SanitizeName( const char *pszOldName, char *pszNewName, size_t bufferSize );
const char *Com_GetArchiveFileName( const char *filename, const char *extension );
const char *Com_GetArchiveFolder();
void Com_WipeSavegame( const char *savename );
qboolean Com_ShiftedStrStr(const char* shifted, const char* name, int offset);
qboolean COM_IsMapValid(const char* name);
void Com_SwapSaveStruct(savegamestruct_t* save);


/*
==============================================================

CLIENT / SERVER SYSTEMS

==============================================================
*/

//
// client interface
//
void CL_InitKeyCommands( void );
// the keyboard binding interface must be setup before execing
// config files, but the rest of client startup will happen later

void CL_Init( void );
void CL_Disconnect();
void CL_AbnormalDisconnect();
void CL_Shutdown(const char *finalmsg, qboolean disconnect, qboolean quit);
qboolean CL_FinishedIntro(void);
void CL_FinishedStartStage(void);
void CL_SetFrameNumber(int frameNumber);
void CL_Frame( int msec );
qboolean CL_GameCommand( void );
void CL_KeyEvent (int key, qboolean down, unsigned time);

void CL_Key_ForceCommand(int key, unsigned int time);

void CL_CharEvent( int key );
// char events are for field typing, not game control

void CL_GetMouseState( int *x, int *y, unsigned int *buttons );

void CL_SetMousePos( int x, int y );

void CL_MouseEvent( int dx, int dy, int time );

void CL_ClearButtons( void );

void CL_JoystickEvent( int axis, int value, int time );

void CL_PacketEvent( netadr_t from, msg_t *msg );

void CL_MapLoading( qboolean flush, const char *pszMapName );
// do a screen update before starting to load a map
// when the server is going to load a new map, the entire hunk
// will be cleared, so the client must shutdown cgame, ui, and
// the renderer

void	CL_ForwardCommandToServer( const char *string );
// adds the current command line as a clc_clientCommand to the client message.
// things like godmode, noclip, etc, are commands directed to the server,
// so when they are typed in at the console, they will need to be forwarded.

qboolean CL_UseLargeLightmap(const char* mapName);
// returns true if the standard BSP file should be used

void CL_CDDialog( void );
// bring up the "need a cd to play" dialog

void CL_FlushMemory( void );
// dump all memory on an error

void CL_ShutdownAll(qboolean shutdownRef);
// shutdown client

void CL_InitRef(void);
// initialize renderer interface

void CL_StartHunkUsers( qboolean rendererOnly );
// start all the client stuff using the hunk

void CL_Snd_Shutdown(void);
// Restart sound subsystem

void Key_KeynameCompletion( void(*callback)(const char *s) );
// for keyname autocompletion

void Key_WriteBindings( fileHandle_t f );
// for writing the config files

void S_ClearSoundBuffer( void );
// call before filesystem access

float *get_camera_offset( qboolean *lookactive, qboolean *resetview );

// AVI files have the start of pixel lines 4 byte-aligned
#define AVI_LINE_PADDING 4

//
// server interface
//
void SV_ServerLoaded( void );
void SV_Init( void );
void SV_Shutdown( const char *finalmsg );
void SV_SetFrameNumber(int frameNumber);
void SV_Frame( int msec );
void SV_PacketEvent( netadr_t from, msg_t *msg );
int SV_FrameMsec(void);
void SV_CheckSaveGame(void);
qboolean SV_GameCommand(void);
int SV_SendQueuedPackets(void);

//
// input interface
//
void IN_Init(void *windowData);
void IN_Frame(void);
void IN_Shutdown(void);
void IN_Restart(void);

void Com_Pause();
void Com_Unpause();
void Com_FakePause();
void Com_FakeUnpause();
void Com_Pause_f();
void SV_SoundCallback( int entnum, int channel_number, const char *name );

int R_CountTextureMemory();
/*
==============================================================

NON-PORTABLE SYSTEM SERVICES

==============================================================
*/

typedef enum {
	AXIS_SIDE,
	AXIS_FORWARD,
	AXIS_UP,
	AXIS_ROLL,
	AXIS_YAW,
	AXIS_PITCH,
	MAX_JOYSTICK_AXIS
} joystickAxis_t;

typedef enum {
	// SE_NONE must be zero
	SE_NONE = 0,	// evTime is still valid
	SE_KEY,		// evValue is a key code, evValue2 is the down flag
	SE_CHAR,	// evValue is an ascii char
	SE_MOUSE,	// evValue and evValue2 are reletive signed x / y moves
	SE_JOYSTICK_AXIS,	// evValue is an axis number and evValue2 is the current state (-127 to 127)
	SE_CONSOLE,	// evPtr is a char*
	SE_PACKET	// evPtr is a netadr_t followed by data bytes to evPtrLength
} sysEventType_t;

typedef struct {
	int				evTime;
	sysEventType_t	evType;
	int				evValue, evValue2;
	size_t			evPtrLength;	// bytes of data pointed to by evPtr, for journaling
	void			*evPtr;			// this must be manually freed if not NULL
} sysEvent_t;

sysEvent_t	Sys_GetEvent(void);

void		Com_QueueEvent( int time, sysEventType_t type, int value, int value2, size_t ptrLength, void *ptr );
int			Com_EventLoop( void );
sysEvent_t	Com_GetSystemEvent( void );

void	Sys_Init (void);

// general development dll loading for virtual machine testing
void	Sys_UnloadDll( void *dllHandle );

qboolean Sys_DllExtension(const char* name);

void	Sys_UnloadGame( void );
void	*Sys_GetGameAPI( void *parms );

void	Sys_UnloadCGame( void );
void	*Sys_GetCGameAPI( void *parms );

void	Sys_UnloadUI( void );
void	*Sys_GetUIAPI( void );

//bot libraries
void	Sys_UnloadBotLib( void );
void	*Sys_GetBotLibAPI( void *parms );

char	*Sys_GetCurrentUser( void );

void	QDECL Sys_Error( const char *error, ...) __attribute__ ((format (printf, 1, 2)));
void	Sys_Quit (void);
char	*Sys_GetClipboardData( void );	// note that this isn't journaled...

void	Sys_Print( const char *msg );

// Sys_Milliseconds should only be used for profiling purposes,
// any game related timing information should come from event timestamps
int		Sys_Milliseconds (void);

qboolean Sys_RandomBytes( byte *string, int len );

// the system console is shown when a dedicated server is running
void	Sys_DisplaySystemConsole( qboolean show );

cpuFeatures_t Sys_GetProcessorFeatures( void );

void	Sys_SetErrorText( const char *text );

void	Sys_SendPacket( int length, const void *data, netadr_t to );

qboolean	Sys_StringToAdr( const char *s, netadr_t *a, netadrtype_t family );
qboolean Sys_GetPacket( netadr_t *net_from, msg_t *net_message );

//Does NOT parse port numbers, only base addresses.

qboolean	Sys_IsLANAddress (netadr_t adr);
void		Sys_ShowIP(void);

FILE	*Sys_FOpen( const char *ospath, const char *mode );
qboolean Sys_Mkdir( const char *path );
FILE	*Sys_Mkfifo( const char *ospath );
char	*Sys_Cwd( void );
void	Sys_SetDefaultInstallPath(const char *path);
char	*Sys_DefaultInstallPath(void);
char	*Sys_SteamPath(void);
char	*Sys_GogPath(void);
char	*Sys_MicrosoftStorePath(void);

#ifdef __APPLE__
char    *Sys_DefaultAppPath(void);
#endif

void  Sys_SetDefaultHomePath(const char *path);
char	*Sys_DefaultHomePath(void);
const char *Sys_Dirname( char *path );
const char *Sys_Basename( char *path );
char *Sys_ConsoleInput( void );

void	SetProgramPath( const char *path );
char	*Sys_DefaultBasePath( void );
char	*Sys_DefaultUserPath( void );
char	*Sys_DefaultOutputPath( void );

char **Sys_ListFiles( const char *directory, const char *extension, const char *filter, int *numfiles, qboolean wantsubs );
void	Sys_FreeFileList( char **list );
void	Sys_Sleep(int msec);

void Sys_InitStreamThread(void);
void Sys_ShutdownStreamThread(void);
void Sys_BeginStreamedFile(fileHandle_t f, int readAhead);
void Sys_EndStreamedFile(fileHandle_t f);
int Sys_StreamedRead(void* buffer, int size, int count, fileHandle_t f);
void Sys_StreamSeek(fileHandle_t f, int offset, int origin);

qboolean Sys_LowPhysicalMemory(void);

void Sys_SetEnv(const char* name, const char* value);

void SetNormalThreadPriority( void );
void SetBelowNormalThreadPriority( void );

void RecoverLostAutodialData( void );

void Sys_CloseMutex( void );
void Sys_ShowConsole( int visLevel, qboolean quitOnClose );
void Sys_DebugPrint(const char* message);
void Sys_PumpMessageLoop( void );

qboolean SaveRegistryInfo( qboolean user, const char *pszName, void *pvBuf, long lSize );
qboolean LoadRegistryInfo( qboolean user, const char *pszName, void *pvBuf, long *plSize );

qboolean IsFirstRun( void );
qboolean IsNewConfig( void );
qboolean IsSafeMode( void );
void ClearNewConfigFlag( void );
const char *Sys_GetWholeClipboard( void );
void Sys_SetClipboard( const char *contents );

typedef enum
{
	DR_YES = 0,
	DR_NO = 1,
	DR_OK = 0,
	DR_CANCEL = 1
} dialogResult_t;

typedef enum
{
	DT_INFO,
	DT_WARNING,
	DT_ERROR,
	DT_YES_NO,
	DT_OK_CANCEL
} dialogType_t;

dialogResult_t Sys_Dialog( dialogType_t type, const char *message, const char *title );

void Sys_RemovePIDFile( const char *gamedir );
void Sys_InitPIDFile( const char *gamedir );

void Sys_ProcessBackgroundTasks();

/* This is based on the Adaptive Huffman algorithm described in Sayood's Data
 * Compression book.  The ranks are not actually stored, but implicitly defined
 * by the location of a node within a doubly-linked list */

#define NYT HMAX					/* NYT = Not Yet Transmitted */
#define INTERNAL_NODE (HMAX+1)

typedef struct nodetype {
	struct	nodetype *left, *right, *parent; /* tree structure */ 
	struct	nodetype *next, *prev; /* doubly-linked list */
	struct	nodetype **head; /* highest ranked node in block */
	int		weight;
	int		symbol;
} node_t;

#define HMAX 256 /* Maximum symbol */

typedef struct {
	int			blocNode;
	int			blocPtrs;

	node_t*		tree;
	node_t*		lhead;
	node_t*		ltail;
	node_t*		loc[HMAX+1];
	node_t**	freelist;

	node_t		nodeList[768];
	node_t*		nodePtrs[768];
} huff_t;

typedef struct {
	huff_t		compressor;
	huff_t		decompressor;
} huffman_t;

void	Huff_Compress(msg_t *buf, int offset);
void	Huff_Decompress(msg_t *buf, int offset);
void	Huff_Init(huffman_t *huff);
void	Huff_addRef(huff_t* huff, byte ch);
int		Huff_Receive (node_t *node, int *ch, byte *fin);
void	Huff_transmit (huff_t *huff, int ch, byte *fout, int maxoffset);
void	Huff_offsetReceive (node_t *node, int *ch, byte *fin, int *offset, int maxoffset);
void	Huff_offsetTransmit (huff_t *huff, int ch, byte *fout, int *offset, int maxoffset);
void	Huff_putBit( int bit, byte *fout, int *offset);
int		Huff_getBit( byte *fout, int *offset);

extern huffman_t clientHuffTables;

#define	SV_ENCODE_START		4
#define SV_DECODE_START		12
#define	CL_ENCODE_START		12
#define CL_DECODE_START		4

// flags for sv_allowDownload and cl_allowDownload
#define DLF_ENABLE 1
#define DLF_NO_REDIRECT 2
#define DLF_NO_UDP 4
#define DLF_NO_DISCONNECT 8

#ifdef __cplusplus
}
#endif

#endif // _QCOMMON_H_
