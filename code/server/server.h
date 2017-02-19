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
// server.h

#ifndef __SERVER_H__
#define __SERVER_H__

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../game/bg_public.h"
#include "../game/g_public.h"

//=============================================================================

#define	PERS_SCORE				0		// !!! MUST NOT CHANGE, SERVER AND
										// GAME BOTH REFERENCE !!!

#define	MAX_ENT_CLUSTERS	16

#ifdef __cplusplus
extern "C" {
#endif

typedef struct svEntity_s {
	struct worldSector_s *worldSector;
	struct svEntity_s *nextEntityInWorldSector;
	
	entityState_t	baseline;		// for delta compression of initial sighting
	int			numClusters;		// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			lastCluster;		// if all the clusters don't fit in clusternums
	int			areanum, areanum2;
	int			snapshotCounter;	// used to prevent double adding from portal views
} svEntity_t;

typedef enum {
	SS_DEAD,			// no map loaded
	SS_LOADING,			// spawning level entities
	SS_LOADING2,
	SS_GAME				// actively running
} serverState_t;

typedef struct {
	serverState_t	state;
	qboolean		restarting;			// if true, send configstring changes during SS_LOADING
	int				serverId;			// changes each server start
	int				restartedServerId;	// serverId before a map_restart
	int				checksumFeed;		// the feed key that we use to compute the pure checksum strings
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=475
	// the serverId associated with the current checksumFeed (always <= serverId)
	int       checksumFeedServerId;	
	int				snapshotCounter;	// incremented for each snapshot built
	int				timeResidual;		// <= 1000 / sv_frame->value
	int				nextFrameTime;		// when time > nextFrameTime, process world
	struct cmodel_s	*models[MAX_MODELS];
	char			*configstrings[MAX_CONFIGSTRINGS];
	svEntity_t		svEntities[MAX_GENTITIES];

	int				farplane;
	qboolean		skyportal;

	char			*entityParsePoint;	// used during game VM init

	// the game virtual machine will update these on init and changes
	gentity_t		*gentities;
	int				gentitySize;
	int				num_entities;		// current number, <= MAX_GENTITIES

	playerState_t	*gameClients;
	int				gameClientSize;		// will be > sizeof(playerState_t) due to game private data
} server_t;





typedef struct {
	int				areabytes;
	byte			areabits[MAX_MAP_AREA_BYTES];		// portalarea visibility bits
	playerState_t	ps;
	int				num_entities;
	int				first_entity;		// into the circular sv_packet_entities[]
										// the entities MUST be in increasing state number
										// order, otherwise the delta compression will fail
	int				messageSent;		// time the message was transmitted
	int				messageAcked;		// time the message was acked
	size_t			messageSize;		// used to rate drop packets
} clientSnapshot_t;

typedef enum {
	CS_FREE,		// can be reused for a new connection
	CS_ZOMBIE,		// client has been disconnected, but don't reuse
					// connection for a couple seconds
	CS_CONNECTED,	// has been assigned to a client_t, but no gamestate yet
	CS_PRIMED,		// gamestate has been sent, but client hasn't sent a usercmd
	CS_ACTIVE		// client is fully in game
} clientState_t;

typedef struct netchan_buffer_s {
	msg_t           msg;
	byte            msgBuffer[MAX_MSGLEN];
	struct netchan_buffer_s *next;
} netchan_buffer_t;

typedef struct client_s {
	clientState_t	state;
	char			userinfo[MAX_INFO_STRING];		// name, etc

	char			reliableCommands[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];
	int				reliableSequence;		// last added reliable message, not necesarily sent or acknowledged yet
	int				reliableAcknowledge;	// last acknowledged reliable message
	int				reliableSent;			// last sent reliable message, not necesarily acknowledged yet
	int				messageAcknowledge;

	int				gamestateMessageNum;	// netchan->outgoingSequence of gamestate
	int				challenge;

	usercmd_t		lastUsercmd;

	usereyes_t lastEyeinfo;

	int				lastMessageNum;		// for delta compression
	int				lastClientCommand;	// reliable client message sequence
	char			lastClientCommandString[MAX_STRING_CHARS];
	gentity_t		*gentity;			// SV_GentityNum(clientnum)
	char			name[MAX_NAME_LENGTH];			// extracted from userinfo, high bits masked

	// downloading
	char			downloadName[MAX_QPATH]; // if not empty string, we are downloading
	fileHandle_t	download;			// file being downloaded
	size_t			downloadSize;		// total bytes (can't use EOF because of paks)
	size_t			downloadCount;		// bytes sent
	int				downloadClientBlock;	// last block we sent to the client, awaiting ack
	int				downloadCurrentBlock;	// current block number
	int				downloadXmitBlock;	// last block we xmited
	unsigned char	*downloadBlocks[MAX_DOWNLOAD_WINDOW];	// the buffers for the download blocks
	size_t			downloadBlockSize[MAX_DOWNLOAD_WINDOW];
	qboolean		downloadEOF;		// We have sent the EOF block
	int				downloadSendTime;	// time we last got an ack from the client

	int				deltaMessage;		// frame last client usercmd message
	int				nextReliableTime;	// svs.time when another reliable command will be allowed
	int				lastPacketTime;		// svs.time when packet was last received
	int				lastConnectTime;	// svs.time when connection started
	int				nextSnapshotTime;	// send another snapshot when svs.time >= nextSnapshotTime
	qboolean		rateDelayed;		// true if nextSnapshotTime was set based on rate instead of snapshotMsec
	int				timeoutCount;		// must timeout a few frames in a row so debugging doesn't break
	clientSnapshot_t	frames[PACKET_BACKUP];	// updates can be delta'd from here
	int				ping;
	int				rate;				// bytes / second
	int				snapshotMsec;		// requests a snapshot every snapshotMsec unless rate choked
	int				pureAuthentic;
	qboolean  gotCP; // TTimo - additional flag to distinguish between a bad pure checksum, and no cp command at all
	netchan_t		netchan;
	// TTimo
	// queuing outgoing fragmented messages to send them properly, without udp packet bursts
	// in case large fragmented messages are stacking up
	// buffer them into this queue, and hand them out to netchan as needed
	netchan_buffer_t *netchan_start_queue;
	netchan_buffer_t **netchan_end_queue;

	int				oldServerTime;
	qboolean			csUpdated[MAX_CONFIGSTRINGS+1];	

	server_sound_t server_sounds[ MAX_SERVER_SOUNDS ];
	int number_of_server_sounds;
	qboolean locprint;
	int XOffset;
	int YOffset;
	char centerprint[ 256 ];
} client_t;

//=============================================================================


// MAX_CHALLENGES is made large to prevent a denial
// of service attack that could cycle all of them
// out before legitimate users connected
#define	MAX_CHALLENGES	1024

#define	AUTHORIZE_TIMEOUT	5000

typedef struct {
	netadr_t	adr;
	int			challenge;
	int			time;				// time the last packet was sent to the autherize server
	int			pingTime;			// time the challenge response was sent to client
	int			firstTime;			// time the adr was first used, for authorize timeout checks
	qboolean	connected;
} challenge_t;


#define	MAX_MASTERS	8				// max recipients for heartbeat packets

typedef struct {
	int iFlags;
	char szName[ 64 ];
} soundSfx_t;

typedef struct {
	qboolean bPlaying;
	int iStatus;
	soundSfx_t sfx;
	int iEntNum;
	int iEntChannel;
	float vOrigin[ 3 ];
	float fVolume;
	int iBaseRate;
	float fNewPitchMult;
	float fMinDist;
	float fMaxDist;
	int iStartTime;
	int iTime;
	int iNextCheckObstructionTime;
	int iEndTime;
	int iFlags;
	int iOffset;
	int iLoopCount;
} soundChan_t;

typedef struct {
	soundChan_t Channels[ 96 ];
} soundSystem_t;

// this structure will be cleared only when the game dll changes
typedef struct {
	qboolean		initialized;				// sv_init has completed

	int				snapFlagServerBit;			// ^= SNAPFLAG_SERVERCOUNT every SV_SpawnServer()

	int				time;						// will be strictly increasing across level changes
	int				startTime;
	int				lastTime;
	int				serverLagTime;
	qboolean		autosave;
	int				mapTime;

	client_t		*clients;					// [sv_maxclients->integer];
	int				iNumClients;
	int				numSnapshotEntities;		// sv_maxclients->integer*PACKET_BACKUP*MAX_PACKET_ENTITIES
	int				nextSnapshotEntities;		// next snapshotEntities to use
	entityState_t	*snapshotEntities;		// [numSnapshotEntities]
	int				nextHeartbeatTime;
	challenge_t		challenges[MAX_CHALLENGES];	// to prevent invalid IPs from connecting
	netadr_t		redirectAddress;			// for rcon return messages
	netadr_t		authorizeAddress;			// for rcon return messages
	char			gameName[ MAX_QPATH ];
	char			mapName[ MAX_QPATH ];
	char			rawServerName[ MAX_QPATH ];
	int				areabits_warning_time;
	qboolean		soundsNeedLoad;
	char			tm_filename[ MAX_QPATH ];
	int				tm_loopcount;
	int				tm_offset;

	soundSystem_t	soundSystem;
} serverStatic_t;

//=============================================================================

extern cvar_t	*sv_mapname;
extern serverStatic_t	svs;			// persistant server info across maps
extern server_t		sv;					// cleared each map
extern gameExport_t	*ge;				// game exports

#define	MAX_MASTER_SERVERS	5

extern cvar_t	*sv_fps;
extern cvar_t	*sv_timeout;
extern cvar_t	*sv_zombietime;
extern cvar_t	*sv_rconPassword;
extern cvar_t	*sv_privatePassword;
extern cvar_t	*sv_allowDownload;
extern cvar_t	*sv_maxclients;

extern cvar_t	*sv_privateClients;
extern cvar_t	*sv_hostname;
extern cvar_t	*sv_master[ MAX_MASTER_SERVERS ];
extern cvar_t	*sv_reconnectlimit;
extern cvar_t	*sv_showloss;
extern cvar_t	*sv_padPackets;
extern cvar_t	*sv_killserver;
extern cvar_t	*sv_mapChecksum;
extern cvar_t	*sv_serverid;
extern cvar_t	*sv_maxRate;
extern cvar_t	*sv_minPing;
extern cvar_t	*sv_maxPing;
extern cvar_t	*sv_pure;
extern cvar_t	*sv_floodProtect;
extern cvar_t	*sv_maplist;
extern cvar_t	*sv_drawentities;
extern cvar_t	*sv_deeptracedebug;
extern cvar_t	*g_gametype;
extern cvar_t	*g_gametypestring;
extern cvar_t	*sv_chatter;
extern cvar_t	*sv_gamename;
extern cvar_t	*sv_location;
extern cvar_t	*sv_lanForceRate; // dedicated 1 (LAN) server forces local client rates to 99999 (bug #491)
extern cvar_t	*sv_strictAuth;

extern debugline_t *DebugLines;
extern int numDebugLines;
extern debugstring_t *DebugStrings;
extern int numDebugStrings;

//===========================================================

//
// sv_main.c
//
void SV_FinalMessage( const char *message );
void QDECL SV_SendServerCommand( client_t *cl, const char *fmt, ...);


void SV_AddOperatorCommands (void);
void SV_RemoveOperatorCommands (void);


void SV_MasterHeartbeat (void);
void SV_MasterShutdown (void);

void SV_ArchiveHudDrawElements( qboolean loading );
void SV_HudDrawShader( int iInfo, char *name );
void SV_HudDrawAlign( int iInfo, int iHorizontalAlign, int iVerticalAlign );
void SV_HudDrawRect( int iInfo, int iX, int iY, int iWidth, int iHeight );
void SV_HudDrawVirtualSize( int iInfo, qboolean bVirtualScreen );
void SV_HudDrawColor( int iInfo, vec3_t vColor );
void SV_HudDrawAlpha( int iInfo, float alpha );
void SV_HudDrawString( int iInfo, const char *string );
void SV_HudDrawFont( int iInfo, const char *name );
void SV_ArchiveViewModelAnimation( qboolean loading /* 0x8 */ );
void SV_ArchiveStopwatch( qboolean loading );
void SV_ArchivePersistantFile( qboolean loading );
void SV_ArchiveLevel( qboolean loading );
qboolean SV_ArchiveLevelFile( qboolean loading, qboolean autosave );
void S_Save( fileHandle_t f );
void S_Load( fileHandle_t f );
qboolean SV_ArchiveServerFile( qboolean loading, qboolean autosave );
void SV_Loadgame_f( void );
void SV_SavegameFilename( const char *name, char *fileName, int length );
qboolean SV_AllowSaveGame( void );
qboolean SV_DoSaveGame();
void SV_SaveGame( const char *gamename, qboolean autosave );
void SV_Savegame_f( void );
void SV_CheckSaveGame( void );
void SV_Autosavegame_f( void );

//
// sv_init.c
//
void SV_ClearSvsTimeFixups( void );
void SV_FinishSvsTimeFixups( void );
void SV_AddSvsTimeFixup( int *piTime );
void SV_SetConfigstring( int index, const char *val );
char *SV_GetConfigstring( int index );
int SV_FindIndex( const char *name, int start, int max, qboolean create );
int SV_ModelIndex( const char *name );
void SV_ClearModel( int index );
int SV_SoundIndex( const char *name, qboolean streamed );
int SV_ImageIndex( const char *name );
int SV_ItemIndex( const char *name );
void SV_SetLightStyle( int index, const char *data );
void SV_UpdateConfigstrings( client_t *client );

void SV_SetUserinfo( int index, const char *val );
void SV_GetUserinfo( int index, char *buffer, int bufferSize );

void SV_ChangeMaxClients( void );
void SV_SpawnServer( const char *server, qboolean loadgame, qboolean restart, qboolean bTransition );



//
// sv_client.c
//
void SV_GetChallenge( netadr_t from );

void SV_DirectConnect( netadr_t from );

void SV_AuthorizeIpPacket( netadr_t from );

void SV_ExecuteClientMessage( client_t *cl, msg_t *msg );
void SV_UserinfoChanged( client_t *cl );

void SV_ClientEnterWorld( client_t *client, usercmd_t *cmd );
void SV_DropClient( client_t *drop, const char *reason );

void SV_ExecuteClientCommand( client_t *cl, const char *s, qboolean clientOK );
void SV_ClientThink( client_t *cl, usercmd_t *cmd );

void SV_WriteDownloadToClient( client_t *cl , msg_t *msg );

//
// sv_ccmds.c
//
void SV_Heartbeat_f( void );

//
// sv_snapshot.c
//
void SV_AddServerCommand( client_t *client, const char *cmd );
void SV_UpdateServerCommandsToClient( client_t *client, msg_t *msg );
void SV_WriteFrameToClient (client_t *client, msg_t *msg);
void SV_SendMessageToClient( msg_t *msg, client_t *client );
void SV_SendClientMessages( void );
void SV_SendClientSnapshot( client_t *client );

//
// sv_game.c
//
void SV_ClearModelUserCounts( void );
int	SV_NumForGentity( gentity_t *ent );
gentity_t *SV_GentityNum( int num );
playerState_t *SV_GameClientNum( int num );
svEntity_t	*SV_SvEntityForGentity( gentity_t *gEnt );
gentity_t *SV_GEntityForSvEntity( svEntity_t *svEnt );
void		SV_InitGameProgs ( void );
void		SV_ShutdownGameProgs ( void );
qboolean	SV_inPVS (const vec3_t p1, const vec3_t p2);
// su44: MoHAA game -> cgame messages
void SV_WriteCGMToClient (client_t *client, msg_t *msg);
void SV_InitAllCGMessages ();

//
// sv_bot.c
//
void		SV_BotFrame( int time );
int			SV_BotAllocateClient(void);
void		SV_BotFreeClient( int clientNum );

void		SV_BotInitCvars(void);
int			SV_BotLibSetup( void );
int			SV_BotLibShutdown( void );
int			SV_BotGetSnapshotEntity( int client, int ent );
int			SV_BotGetConsoleMessage( int client, char *buf, int size );

//
// sv_snd.c
//
void SV_Sound( vec3_t *org, int entnum, int channel, const char *sound_name, float volume, float mindist, float pitch, float maxdist, qboolean streamed );
void SV_ClearSounds( client_t *client );
void SV_StopSound( int entnum, int channel );

int BotImport_DebugPolygonCreate(int color, int numPoints, vec3_t *points);
void BotImport_DebugPolygonDelete(int id);

//============================================================
//
// high level object sorting to reduce interaction tests
//

void SV_ClearWorld (void);
// called after the world model has been loaded, before linking any entities

void SV_UnlinkEntity( gentity_t *ent );
// call before removing an entity, and before trying to move one,
// so it doesn't clip against itself

void SV_LinkEntity( gentity_t *ent );
// Needs to be called any time an entity changes origin, mins, maxs,
// or solid.  Automatically unlinks if needed.
// sets ent->v.absmin and ent->v.absmax
// sets ent->leafnums[] for pvs determination even if the entity
// is not solid


clipHandle_t SV_ClipHandleForEntity( const gentity_t *ent );


void SV_SectorList_f( void );


int SV_AreaEntities( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount );
// fills in a table of entity numbers with entities that have bounding boxes
// that intersect the given area.  It is possible for a non-axial bmodel
// to be returned that doesn't actually intersect the area on an exact
// test.
// returns the number of pointers filled in
// The world entity is never returned in this list.

baseshader_t *SV_GetShaderPointer( int iShaderNum );
int SV_PointContents( const vec3_t p, int passEntityNum );
// returns the CONTENTS_* value from the world and all entities at the given point.

qboolean SV_SightTraceEntity( gentity_t *touch, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int contentmask, qboolean cylinder );
qboolean SV_SightTrace( const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int passEntityNum2, int contentmask, qboolean cylinder );
void SV_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask, qboolean cylinder, qboolean traceDeep );
void SV_TraceDeep( trace_t *results, const vec3_t vStart, const vec3_t vEnd, int iBrushMask, gentity_t *touch );
// mins and maxs are relative

// if the entire move stays in a solid volume, trace.allsolid will be set,
// trace.startsolid will be set, and trace.fraction will be 0

// if the starting point is in a solid, it will be allowed to move out
// to an open area

// passEntityNum is explicitly excluded from clipping checks (normally ENTITYNUM_NONE)


void SV_ClipToEntity( trace_t *trace, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int entityNum, int contentmask );
// clip to a specific entity

//
// sv_net_chan.c
//
void SV_Netchan_Transmit( client_t *client, msg_t *msg);
void SV_Netchan_TransmitNextFragment( client_t *client );
qboolean SV_Netchan_Process( client_t *client, msg_t *msg );

#ifdef __cplusplus
}
#endif

#endif // __SERVER_H__
