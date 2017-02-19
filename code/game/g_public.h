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
//

// g_public.h -- game module information visible to server

#define	GAME_API_VERSION	12

// entity->svFlags
// the server does not know how to interpret most of the values
// in entityStates (level eType), so the game must explicitly flag
// special server behaviors
#define	SVF_NOCLIENT			0x00000001	// don't send entity to clients, even if it has effects

// TTimo
// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=551
#define SVF_CLIENTMASK 0x00000002

#define	SVF_BROADCAST			0x00000004	// send to all connected clients
#define SVF_BOT					0x00000008	// set if the entity is a bot
#define	SVF_PORTAL				0x00000040	// merge a second pvs at origin2 into snapshots
#define	SVF_USE_CURRENT_ORIGIN	0x00000080	// entity->r.currentOrigin instead of entity->s.origin
											// for link position (missiles and movers)
#define SVF_SINGLECLIENT		0x00000100	// only send to a single client (entityShared_t->singleClient)
#define SVF_NOSERVERINFO		0x00000200	// don't send CS_SERVERINFO updates to this client
											// so that it can be updated for ping tools without
											// lagging clients
#define SVF_CAPSULE				0x00000400	// use capsule for collision detection instead of bbox
#define SVF_NOTSINGLECLIENT		0x00000800	// send entity to everyone but one client
											// (entityShared_t->singleClient)

#define SVF_USEBBOX				0x00001000
#define SVF_SENDONCE			0x00004000

#define SVF_HIDEOWNER			0x00008000
#define SVF_SENDPVS				0x00010000

typedef struct gclient_s gclient_t;
typedef struct dtiki_s dtiki_t;
typedef struct tiki_cmd_s tiki_cmd_t;
typedef struct dtikianim_s dtikianim_t;
typedef struct tiki_s tiki_t;

typedef struct tikiAnim_s tikiAnim_t;
typedef struct baseshader_s baseshader_t;
typedef struct AliasListNode_s AliasListNode_t;
typedef void( *xcommand_t ) ( void );

typedef struct {
	// su44: sharedEntity_t::s is used instead of it
	//entityState_t	s;				// communicated by server to clients

	qboolean	linked;				// qfalse if not in any good cluster
	int			linkcount;

	int			svFlags;			// SVF_NOCLIENT, SVF_BROADCAST, etc

	// only send to this client when SVF_SINGLECLIENT is set	
	// if SVF_CLIENTMASK is set, use bitmask for clients to send to (maxclients must be <= 32, up to the mod to enforce this)
	int			singleClient;

	int			lastNetTime;

	qboolean	bmodel;				// if false, assume an explicit mins / maxs bounding box
	// only set by gi.SetBrushModel
	vec3_t		mins, maxs;
	int			contents;			// CONTENTS_TRIGGER, CONTENTS_SOLID, CONTENTS_BODY, etc
	// a non-solid entity should set to 0

	vec3_t		absmin, absmax;		// derived from mins/maxs and origin + rotation

	float		radius;

	// currentOrigin will be used for all collision detection and world linking.
	// it will not necessarily be the same as the trajectory evaluation for the current
	// time, because each entity must be moved one at a time after time is advanced
	// to avoid simultanious collision issues
	vec3_t		currentOrigin;

	int			areanum;

	vec3_t		currentAngles;

	// when a trace call is made and passEntityNum != ENTITYNUM_NONE,
	// an ent will be excluded from testing if:
	// ent->s.number == passEntityNum	(don't interact with self)
	// ent->s.ownerNum = passEntityNum	(don't interact with your own missiles)
	// entity[ent->s.ownerNum].ownerNum = passEntityNum	(don't interact with other missiles from owner)
	int			ownerNum;
} entityShared_t;

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct {
	char			userinfo[ MAX_INFO_STRING ];
	char			netname[ MAX_NAME_LENGTH ];
	char			playermodel[ MAX_QPATH ];
	char			playergermanmodel[ MAX_QPATH ];
	char			weapon[ MAX_QPATH ];

	float			enterTime;

	teamtype_t		team;
	int				kills;

	char			ip[ 17 ];
	int				port;
} clientPersistant_t;


// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
struct gclient_s {
	// ps MUST be the first element, because the server expects it
	playerState_t			ps;				// communicated by server to clients

#ifdef GAME_DLL
	// the rest of the structure is private to game
	int						ping;
	clientPersistant_t		pers;
	float					cmd_angles[ 3 ];

	int						lastActiveTime;
	int						activeWarning;

	struct gentity_s		*ent;
	struct gclient_s		*next;
	struct gclient_s		*prev;
#endif
};

typedef struct gentity_s {
	entityState_t		s;				// communicated by server to clients

	struct gclient_s	*client;			// NULL if not a client
	qboolean			inuse;

	entityShared_t		r;				// shared by both the server system and game

	solid_t				solid;
	dtiki_t				*tiki;
	float				mat[ 3 ][ 3 ];

	// Leave reminder to the game dll
#if defined( GAME_DLL )
	class Entity		*entity;
	float				freetime;
	float				spawntime;
	float				radius2;
	char				entname[ 64 ];
	int					clipmask;

	struct gentity_s	*next;
	struct gentity_s	*prev;
#endif
} gentity_t;

// movers are things like doors, plats, buttons, etc
typedef enum {
	MOVER_POS1,
	MOVER_POS2,
	MOVER_1TO2,
	MOVER_2TO1
} moverState_t;

typedef struct profVar_s
{
	int start;
	int clockCycles;
	int calls;
	float time;
	float totTime;
	float avgTime;
	float low;
	float high;

} profVar_t;

typedef struct profGame_s
{
	profVar_t AI;
	profVar_t MmoveSingle;
	profVar_t thread_timingList;
	profVar_t gamescripts;
	profVar_t GProbe1;
	profVar_t GProbe2;
	profVar_t GProbe3;
	profVar_t FindPath;
	profVar_t vector_length;
	profVar_t script_trace;
	profVar_t internal_trace;
	profVar_t PreAnimate;
	profVar_t PostAnimate;

} profGame_t;

//===============================================================

#if 1
typedef struct gameImport_s
{
	void ( *Printf )( const char *format, ... );
	void ( *DPrintf )( const char *format, ... );
	void ( *DPrintf2 )( const char *format, ... );
	void ( *DebugPrintf )( const char *format, ... );
	void ( *Error )( int level, const char *format, ... );
	int ( *Milliseconds )( );
	const char * ( *LV_ConvertString )( const char *string );
	void * ( *Malloc )( size_t size );
	void ( *Free )( void *ptr );
	cvar_t * ( *Cvar_Get )( const char *varName, const char *varValue, int varFlags );
	void ( *Cvar_Set )( const char *varName, const char *varValue );
	cvar_t *( *cvar_set2 )( const char *varName, const char *varValue, qboolean force );
	cvar_t *( *NextCvar )( cvar_t *var );
	int ( *Argc )( );
	char * ( *Argv )( int arg );
	char * ( *Args )( );
	void ( *AddCommand )( const char *cmdName, xcommand_t cmdFunction );
	int ( *FS_ReadFile )( const char *qpath, void **buffer, qboolean quiet );
	void ( *FS_FreeFile )( void *buffer );
	size_t( *FS_WriteFile )( const char *qpath, const void *buffer, size_t size );
	fileHandle_t ( *FS_FOpenFileWrite )( const char *fileName );
	fileHandle_t ( *FS_FOpenFileAppend )( const char *fileName );
	const char *( *FS_PrepFileWrite )( const char *fileName );
	size_t( *FS_Write )( void *buffer, size_t size, fileHandle_t fileHandle );
	size_t( *FS_Read )( void *buffer, size_t len, fileHandle_t fileHandle );
	void ( *FS_FCloseFile )( fileHandle_t fileHandle );
	int ( *FS_Tell )( fileHandle_t fileHandle );
	int ( *FS_Seek )( fileHandle_t fileHandle, long int offset, fsOrigin_t origin );
	void ( *FS_Flush )( fileHandle_t fileHandle );
	int ( *FS_FileNewer )( const char *source, const char *destination );
	void ( *FS_CanonicalFilename )( char *fileName );
	char **( *FS_ListFiles )( const char *qpath, const char *extension, qboolean wantSubs, int *numFiles );
	void ( *FS_FreeFileList )( char **list );
	const char *( *GetArchiveFileName )( char *fileName, char *extension );
	void ( *SendConsoleCommand )( const char *text );
	void ( *DebugGraph )( float value, int color );
	void ( *SendServerCommand )( int client, const char *format, ... );
	void ( *DropClient )( int client, const char *reason );
	void ( *MSG_WriteBits )( int value, int bits );
	void ( *MSG_WriteChar )( int c );
	void ( *MSG_WriteByte )( int c );
	void ( *MSG_WriteSVC )( int c );
	void ( *MSG_WriteShort )( int c );
	void ( *MSG_WriteLong )( int c );
	void ( *MSG_WriteFloat )( float f );
	void ( *MSG_WriteString )( const char *s );
	void ( *MSG_WriteAngle8 )( float f );
	void ( *MSG_WriteAngle16 )(float f );
	void ( *MSG_WriteCoord )( float f );
	void ( *MSG_WriteDir )( vec3_t dir );
	void ( *MSG_StartCGM )( int type );
	void ( *MSG_EndCGM )( );
	void ( *MSG_SetClient )( int client );
	void ( *SetBroadcastVisible )( vec3_t pos, vec3_t posB );
	void ( *SetBroadcastHearable )( vec3_t pos, vec3_t posB );
	void ( *SetBroadcastAll )( );
	void ( *SetConfigstring )( int index, const char *val );
	char *( *GetConfigstring )( int index );
	void ( *SetUserinfo )( int index, char *val );
	void ( *GetUserinfo )( int index, char *buffer, int bufferSize );
	void ( *SetBrushModel )( gentity_t *ent, const char *name );
	void ( *ModelBoundsFromName )( char *name, vec3_t mins, vec3_t maxs );
	qboolean ( *SightTraceEntity )( gentity_t *touch, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int contentMask, qboolean cylinder );
	qboolean ( *SightTrace )( vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int passEntityNum, int passEntityNum2, int contentMask, qboolean cylinder );
	void ( *Trace )( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask, qboolean cylinder, qboolean traceDeep );
	baseshader_t * ( *GetShader )( int shaderNum );
	int ( *PointContents )( const vec3_t p, int passEntityNum );
	int ( *PointBrushnum )( vec3_t p, clipHandle_t model );
	void ( *AdjustAreaPortalState )( gentity_t *ent, qboolean open );
	int ( *AreaForPoint )( vec3_t pos );
	qboolean ( *AreasConnected )( int area1, int area2);
	qboolean ( *InPVS )( float *p1, float *p2 );
	void ( *LinkEntity )( gentity_t *gEnt );
	void ( *UnlinkEntity )( gentity_t *gEnt );
	int ( *AreaEntities )( const vec3_t mins, const vec3_t maxs, int *list, int maxcount );
	void ( *ClipToEntity )( trace_t *tr, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int entityNum, int contentMask );
	int ( *imageindex )( const char *name );
	int ( *itemindex )( const char *name );
	int ( *soundindex )( const char *name, int streamed );
	dtiki_t *( *TIKI_RegisterModel )( const char *path );
	dtiki_t *( *modeltiki )( const char *name );
	dtikianim_t *( *modeltikianim )( const char *name );
	void ( *SetLightStyle )( int index, const char *data );
	const char *( *GameDir )( );
	qboolean ( *setmodel )( gentity_t *ent, const char *name );
	void ( *clearmodel )( gentity_t *ent );
	int ( *TIKI_NumAnims )( dtiki_t *tiki );
	int ( *NumSurfaces )( dtiki_t *tiki );
	int ( *NumTags )( dtiki_t *tiki );
	void( *CalculateBounds )( dtiki_t *tiki, float scale, vec3_t mins, vec3_t maxs );
	void *( *TIKI_GetSkeletor )( dtiki_t *tiki, int entNum );
	const char *( *Anim_NameForNum )( dtiki_t *tiki, int animNum );
	int ( *Anim_NumForName )( dtiki_t *tiki, const char *name );
	int ( *Anim_Random )( dtiki_t *tiki, const char *name );
	int ( *Anim_NumFrames )( dtiki_t *tiki, int animNum );
	float ( *Anim_Time )( dtiki_t *tiki, int animNum );
	float ( *Anim_Frametime )( dtiki_t *tiki, int animNum );
	float ( *Anim_CrossTime )( dtiki_t *tiki, int animNum );
	void ( *Anim_Delta )( dtiki_t *tiki, int animNum, float *delta );
	qboolean ( *Anim_HasDelta )( dtiki_t *tiki, int animNum );
	void ( *Anim_DeltaOverTime )( dtiki_t *tiki, int animNum, float time1, float time2, float *delta );
	int ( *Anim_Flags )( dtiki_t *tiki, int animNum );
	int ( *Anim_FlagsSkel )( dtiki_t *tiki, int animNum );
	qboolean ( *Anim_HasCommands )( dtiki_t *tiki, int animNum );
	int ( *NumHeadModels )( const char *model );
	void ( *GetHeadModel )( const char *model, int num, char *name );
	int ( *NumHeadSkins )( const char *model );
	void ( *GetHeadSkin )( const char *model, int num, char *name );
	qboolean ( *Frame_Commands )( dtiki_t *tiki, int animNum, int frameNum, tiki_cmd_t *tikiCmds );
	int ( *Surface_NameToNum )( dtiki_t *tiki, const char *name );
	const char * ( *Surface_NumToName )( dtiki_t *tiki, int surfacenum );
	int ( *Tag_NumForName )( dtiki_t *pmdl, const char *name );
	const char * ( *Tag_NameForNum )(  dtiki_t *pmdl, int tagNum);
	orientation_t ( *TIKI_OrientationInternal )( dtiki_t *tiki, int entNum, int tagNum, float scale );
	void *( *TIKI_TransformInternal )( dtiki_t *tiki, int entNum, int tagNum );
	qboolean ( *TIKI_IsOnGroundInternal )( dtiki_t *tiki, int entNum, int num, float threshold );
	void ( *TIKI_SetPoseInternal )( dtiki_t *tiki, int entNum, frameInfo_t *frameInfo, int *boneTag, vec4_t *boneQuat, float actionWeight );
	const char *( *CM_GetHitLocationInfo )( int location, float *radius, float *offset );
	const char *( *CM_GetHitLocationInfoSecondary )( int location, float *radius, float *offset );

	qboolean	( *Alias_Add )( dtiki_t *pmdl, const char *alias, const char *name, const char *parameters );
	const char	*( *Alias_FindRandom )( dtiki_t *tiki, const char *alias, AliasListNode_t **ret );
	void		( *Alias_Dump )( dtiki_t *tiki );
	void		( *Alias_Clear )( dtiki_t *tiki );
	void		( *Alias_UpdateDialog )( dtikianim_t *tiki, const char *alias );

	const char	*( *TIKI_NameForNum )( dtiki_t *tiki );

	qboolean	( *GlobalAlias_Add )( const char *alias, const char *name, const char *parameters );
	const char	*( *GlobalAlias_FindRandom )( const char *alias, AliasListNode_t **ret );
	void		( *GlobalAlias_Dump )( );
	void		( *GlobalAlias_Clear )( );

	void ( *centerprintf )( gentity_t *ent, const char *format, ... );
	void ( *locationprintf )( gentity_t *ent, int x, int y, const char *format, ... );
	void ( *Sound )( vec3_t *org, int entNum, int channel, const char *soundName, float volume, float minDist, float pitch, float maxDist, int streamed );
	void ( *StopSound )( int entNum, int channel );
	float ( *SoundLength )( int channel, const char *name );
	unsigned char * ( *SoundAmplitudes )( int channel, const char *name );
	int( *S_IsSoundPlaying )( int channel, const char *name );
	short unsigned int ( *CalcCRC )( unsigned char *start, int count );

	void **DebugLines;
	int *numDebugLines;
	void **DebugStrings;
	int *numDebugStrings;

	void ( *LocateGameData )( gentity_t *gEnts, int numGEntities, int sizeofGEntity, playerState_t *clients, int sizeofGameClient );
	void ( *SetFarPlane )(int farPlane );
	void ( *SetSkyPortal )( qboolean skyPortal );
	void ( *Popmenu )( int client , int i );
	void ( *Showmenu )( int client, const char *name, qboolean force );
	void ( *Hidemenu )( int client, const char *name, qboolean force );
	void ( *Pushmenu )( int client, const char *name );
	void ( *HideMouseCursor )( int client );
	void ( *ShowMouseCursor )( int client );
	const char * ( *MapTime )( );
	void ( *LoadResource )( char *name );
	void ( *ClearResource )( );
	int ( *Key_StringToKeynum )( char *str );
	const char *( *Key_KeynumToBindString )( int keyNum );
	void ( *Key_GetKeysForCommand )( const char *command, int *key1, int *key2 );
	void ( *ArchiveLevel )( qboolean loading );
	void ( *AddSvsTimeFixup )( int *piTime );
	void ( *HudDrawShader )( int info, char *name );
	void ( *HudDrawAlign )( int info, int horizontalAlign, int verticalAlign );
	void ( *HudDrawRect )( int info, int x, int y, int width, int height );
	void ( *HudDrawVirtualSize )( int info, qboolean virtualScreen);
	void ( *HudDrawColor )( int info, float *color );
	void ( *HudDrawAlpha )( int info, float alpha );
	void ( *HudDrawString )( int info, char *string );
	void ( *HudDrawFont )( int info, char *fontName );
	qboolean ( *SanitizeName )( char *oldName, char *newName );

	cvar_t *fsDebug;

	// New functions will start from here

} gameImport_t;
#else

typedef struct gameImport_s {
	//============== general Quake services ==================

	void	( *Printf )( const char *fmt, ... );
	void	( *DPrintf )( const char *fmt, ... );
	void	( *Error )( int level, const char *fmt, ... );
	int		( *Milliseconds )( void );
	const char *( *LV_ConvertString )( const char *string );
	void*	( *Malloc )( int size );
	void	( *Free )( void *ptr );
	int		( *Argc )( void );
	char	*( *Argv )( int n );
	const char	*( *Args )();
	int		( *FS_FOpenFile )( const char *qpath, fileHandle_t *f, fsMode_t mode );
	void	( *FS_Read )( void *buffer, int len, fileHandle_t f );
	int		( *FS_Write )( const void *buffer, int len, fileHandle_t f );
	void	( *FS_FCloseFile )( fileHandle_t f );
	int		( *FS_GetFileList )( const char *path, const char *extension, char *listbuf, int bufsize );
	int		( *FS_Seek )( fileHandle_t f, long offset, int origin ); // fsOrigin_t
	int		( *FS_Tell )( fileHandle_t f );
	int		( *FS_ReadFile )( const char *name, void **buffer, qboolean something );
	void	( *FS_FreeFile )( void *buffer );
	int		( *FS_WriteFile )( const char *qpath, const void *buffer, int size );
	fileHandle_t ( *FS_FOpenFileWrite )( const char *name );
	fileHandle_t ( *FS_FOpenFileAppend )( const char *fileName );
	void	( *FS_Flush )( fileHandle_t f );
	void	( *SendConsoleCommand )( const char *text );
	cvar_t	*( *Cvar_Get )( const char *varName, const char *varValue, int varFlags );
	void	( *Cvar_Register )( vmCvar_t *cvar, const char *var_name, const char *value, int flags );
	void	( *Cvar_Update )( vmCvar_t *cvar );
	void	( *Cvar_Set )( const char *var_name, const char *value );
	int		( *Cvar_VariableIntegerValue )( const char *var_name );
	float	( *Cvar_VariableValue )( const char *var_name );
	void	( *Cvar_VariableStringBuffer )( const char *var_name, char *buffer, int bufsize );
	void	( *LocateGameData )( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *gameClients, int sizeofGameClient );
	void	( *DropClient )( int clientNum, const char *reason );
	void	( *SendServerCommand )( int clientNum, const char *text, ... );
	void	( *SetConfigstring )( int num, const char *string );
	char	*( *GetConfigstring )( int num );
	void	( *GetUserinfo )( int num, char *buffer, int bufferSize );
	void	( *SetUserinfo )( int num, const char *buffer );
	void	( *GetServerinfo )( char *buffer, int bufferSize );

	void	( *AddCommand )( const char *cmd, xcommand_t function );

	//
	// MODEL UTILITY FUNCTIONS
	//
	qboolean	( *IsModel )( int index );
	void		( *SetBrushModel )( gentity_t *ent, const char *name );

	// DEF SPECIFIC STUFF
	int			( *NumSkins )( dtiki_t *tiki );
	int			( *NumSurfaces )( dtiki_t *tiki );
	int			( *NumTags )( dtiki_t *tiki );
	qboolean	( *Frame_Commands )( dtiki_t *tiki, int animNum, int frameNum, tikiCmd_t *tiki_cmd );
	void		( *CalculateBounds )( dtiki_t *tiki, float scale, vec3_t mins, vec3_t maxs );

	void	( *Trace )( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask, int capsule, qboolean deepTrace );
	int		( *PointContents )( const vec3_t point, int passEntityNum );
	int		( *PointBrushnum )( vec3_t p, clipHandle_t model );
	qboolean ( *InPVS )( const vec3_t p1, const vec3_t p2 );
	qboolean ( *InPVSIgnorePortals )( const vec3_t p1, const vec3_t p2 );
	void	( *AdjustAreaPortalState )( gentity_t *ent, qboolean open );
	qboolean ( *AreasConnected )( int area1, int area2 );
	void	( *LinkEntity )( gentity_t *ent );
	void	( *UnlinkEntity )( gentity_t *ent );
	int		( *AreaEntities )( const vec3_t mins, const vec3_t maxs, int *list, int maxcount );
	void	( *ClipToEntity )( trace_t *trace, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int entityNum, int contentmask );
	int		( *EntitiesInBox )( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount );
	qboolean ( *EntityContact )( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
	int		( *BotAllocateClient )( void );
	void	( *BotFreeClient )( int clientNum );
	void	( *GetUsercmd )( int clientNum, usercmd_t *cmd );
	qboolean	( *GetEntityToken )( char *buffer, int bufferSize );

	int		( *DebugPolygonCreate )( int color, int numPoints, vec3_t *points );
	void	( *DebugPolygonDelete )( int id );

	int		( *BotLibSetup )( void );
	int		( *BotLibShutdown )( void );
	int		( *BotLibVarSet )( char *var_name, char *value );
	int		( *BotLibVarGet )( char *var_name, char *value, int size );
	int		( *BotLibDefine )( char *string );
	int		( *BotLibStartFrame )( float time );
	int		( *BotLibLoadMap )( const char *mapname );
	int		( *BotLibUpdateEntity )( int ent, void *bue );
	int		( *BotLibTest )( int parm0, char *parm1, vec3_t parm2, vec3_t parm3 );

	int		( *BotGetSnapshotEntity )( int clientNum, int sequence );
	int		( *BotGetServerCommand )( int clientNum, char *message, int size );
	void	( *BotUserCommand )( int client, usercmd_t *ucmd );

	int		( *AAS_BBoxAreas )( vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas );
	int		( *AAS_AreaInfo )( int areanum, void *info );
	void	( *AAS_EntityInfo )( int entnum, void *info );

	int		( *AAS_Initialized )( void );
	void	( *AAS_PresenceTypeBoundingBox )( int presencetype, vec3_t mins, vec3_t maxs );
	float	( *AAS_Time )( void );

	int		( *AAS_PointAreaNum )( vec3_t point );
	int		( *AAS_PointReachabilityAreaIndex )( vec3_t point );
	int		( *AAS_TraceAreas )( vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas );

	int		( *AAS_PointContents )( vec3_t point );
	int		( *AAS_NextBSPEntity )( int ent );
	int		( *AAS_ValueForBSPEpairKey )( int ent, char *key, char *value, int size );
	int		( *AAS_VectorForBSPEpairKey )( int ent, char *key, vec3_t v );
	int		( *AAS_FloatForBSPEpairKey )( int ent, char *key, float *value );
	int		( *AAS_IntForBSPEpairKey )( int ent, char *key, int *value );

	int		( *AAS_AreaReachability )( int areanum );

	int		( *AAS_AreaTravelTimeToGoalArea )( int areanum, vec3_t origin, int goalareanum, int travelflags );
	int		( *AAS_EnableRoutingArea )( int areanum, int enable );
	int		( *AAS_PredictRoute )( void /*struct aas_predictroute_s*/ *route, int areanum, vec3_t origin,
		int goalareanum, int travelflags, int maxareas, int maxtime,
		int stopevent, int stopcontents, int stoptfl, int stopareanum );

	int		( *AAS_AlternativeRouteGoals )( vec3_t start, int startareanum, vec3_t goal, int goalareanum, int travelflags,
		void /*struct aas_altroutegoal_s*/ *altroutegoals, int maxaltroutegoals,
		int type );
	int		( *AAS_Swimming )( vec3_t origin );
	int		( *AAS_PredictClientMovement )( void *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize );


	void	( *EA_Say )( int client, char *str );
	void	( *EA_SayTeam )( int client, char *str );
	void	( *EA_Command )( int client, char *command );

	void	( *EA_Action )( int client, int action );
	void	( *EA_Gesture )( int client );
	void	( *EA_Talk )( int client );
	void	( *EA_Attack )( int client );
	void	( *EA_Use )( int client );
	void	( *EA_Respawn )( int client );
	void	( *EA_Crouch )( int client );
	void	( *EA_MoveUp )( int client );
	void	( *EA_MoveDown )( int client );
	void	( *EA_MoveForward )( int client );
	void	( *EA_MoveBack )( int client );
	void	( *EA_MoveLeft )( int client );
	void	( *EA_MoveRight )( int client );
	void	( *EA_SelectWeapon )( int client, int weapon );
	void	( *EA_Jump )( int client );
	void	( *EA_DelayedJump )( int client );
	void	( *EA_Move )( int client, vec3_t dir, float speed );
	void	( *EA_View )( int client, vec3_t viewangles );

	void	( *EA_EndRegular )( int client, float thinktime );
	void	( *EA_GetInput )( int client, float thinktime, void *input );
	void	( *EA_ResetInput )( int client );

	int		( *BotLoadCharacter )( char *charfile, float skill );
	void	( *BotFreeCharacter )( int character );
	float	( *Characteristic_Float )( int character, int index );
	float	( *Characteristic_BFloat )( int character, int index, float min, float max );
	int		( *Characteristic_Integer )( int character, int index );
	int		( *Characteristic_BInteger )( int character, int index, int min, int max );
	void	( *Characteristic_String )( int character, int index, char *buf, int size );

	int		( *BotAllocChatState )( void );
	void	( *BotFreeChatState )( int handle );
	void	( *BotQueueConsoleMessage )( int chatstate, int type, char *message );
	void	( *BotRemoveConsoleMessage )( int chatstate, int handle );
	int		( *BotNextConsoleMessage )( int chatstate, void *cm );
	int		( *BotNumConsoleMessages )( int chatstate );
	void	( *BotInitialChat )( int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
	int		( *BotNumInitialChats )( int chatstate, char *type );
	int		( *BotReplyChat )( int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 );
	int		( *BotChatLength )( int chatstate );
	void	( *BotEnterChat )( int chatstate, int client, int sendto );
	void	( *BotGetChatMessage )( int chatstate, char *buf, int size );
	int		( *StringContains )( char *str1, char *str2, int casesensitive );
	int		( *BotFindMatch )( char *str, void *match, unsigned long int context );
	void	( *BotMatchVariable )( void *match, int variable, char *buf, int size );
	void	( *UnifyWhiteSpaces )( char *string );
	void	( *BotReplaceSynonyms )( char *string, unsigned long int context );
	int		( *BotLoadChatFile )( int chatstate, char *chatfile, char *chatname );
	void	( *BotSetChatGender )( int chatstate, int gender );
	void	( *BotSetChatName )( int chatstate, char *name, int client );
	void	( *BotResetGoalState )( int goalstate );
	void	( *BotRemoveFromAvoidGoals )( int goalstate, int number );
	void	( *BotResetAvoidGoals )( int goalstate );
	void	( *BotPushGoal )( int goalstate, void *goal );
	void	( *BotPopGoal )( int goalstate );
	void	( *BotEmptyGoalStack )( int goalstate );
	void	( *BotDumpAvoidGoals )( int goalstate );
	void	( *BotDumpGoalStack )( int goalstate );
	void	( *BotGoalName )( int number, char *name, int size );
	int		( *BotGetTopGoal )( int goalstate, void *goal );
	int		( *BotGetSecondGoal )( int goalstate, void *goal );
	int		( *BotChooseLTGItem )( int goalstate, vec3_t origin, int *inventory, int travelflags );
	int		( *BotChooseNBGItem )( int goalstate, vec3_t origin, int *inventory, int travelflags, void *ltg, float maxtime );
	int		( *BotTouchingGoal )( vec3_t origin, void *goal );
	int		( *BotItemGoalInVisButNotVisible )( int viewer, vec3_t eye, vec3_t viewangles, void *goal );
	int		( *BotGetNextCampSpotGoal )( int num, void *goal );
	int		( *BotGetMapLocationGoal )( char *name, void *goal );
	int		( *BotGetLevelItemGoal )( int index, char *classname, void *goal );
	float	( *BotAvoidGoalTime )( int goalstate, int number );
	void	( *BotSetAvoidGoalTime )( int goalstate, int number, float avoidtime );
	void	( *BotInitLevelItems )( void );
	void	( *BotUpdateEntityItems )( void );
	int		( *BotLoadItemWeights )( int goalstate, char *filename );
	void	( *BotFreeItemWeights )( int goalstate );
	void	( *BotInterbreedGoalFuzzyLogic )( int parent1, int parent2, int child );
	void	( *BotSaveGoalFuzzyLogic )( int goalstate, char *filename );
	void	( *BotMutateGoalFuzzyLogic )( int goalstate, float range );
	int		( *BotAllocGoalState )( int state );
	void	( *BotFreeGoalState )( int handle );

	void	( *BotResetMoveState )( int movestate );
	void	( *BotMoveToGoal )( void *result, int movestate, void *goal, int travelflags );
	int		( *BotMoveInDirection )( int movestate, vec3_t dir, float speed, int type );
	void	( *BotResetAvoidReach )( int movestate );
	void	( *BotResetLastAvoidReach )( int movestate );
	int		( *BotReachabilityArea )( vec3_t origin, int testground );
	int		( *BotMovementViewTarget )( int movestate, void *goal, int travelflags, float lookahead, vec3_t target );
	int		( *BotPredictVisiblePosition )( vec3_t origin, int areanum, void *goal, int travelflags, vec3_t target );
	int		( *BotAllocMoveState )( void );
	void	( *BotFreeMoveState )( int handle );
	void	( *BotInitMoveState )( int handle, void *initmove );
	void	( *BotAddAvoidSpot )( int movestate, vec3_t origin, float radius, int type );

	int		( *BotChooseBestFightWeapon )( int weaponstate, int *inventory );
	void	( *BotGetWeaponInfo )( int weaponstate, int weapon, void *weaponinfo );
	int		( *BotLoadWeaponWeights )( int weaponstate, char *filename );
	int		( *BotAllocWeaponState )( void );
	void	( *BotFreeWeaponState )( int weaponstate );
	void	( *BotResetWeaponState )( int weaponstate );

	int		( *GeneticParentsAndChildSelection )( int numranks, float *ranks, int *parent1, int *parent2, int *child );

	void	( *centerprintf )( gentity_t *ent, const char *format, ... );

	int		( *imageindex )( const char *name );
	int		( *itemindex )( const char *name );
	int		( *soundindex )( const char *name, qboolean streamed );
	dtiki_t	*( *modeltiki )( const char *name );

	// ley0k: sky stuff
	void			( *SetFarPlane )( int farPlane );
	void			( *SetSkyPortal )( qboolean skyPortal );

	qboolean		( *Alias_Add )( dtiki_t *tiki, const char * alias, const char * name, const char *parameters );
	const char		*( *Alias_FindRandom )( dtiki_t *tiki, const char * alias, aliasListNode_t **ret );
	void			( *Alias_Dump )( dtiki_t *tiki );
	void			( *Alias_Clear )( dtiki_t *tiki );
	void			( *Alias_UpdateDialog )( dtiki_t *tiki, const char *alias, int number_of_times_played, byte been_played_this_loop, int last_time_played );

	// ANIM SPECIFIC STUFF
	const char		*( *Anim_NameForNum )( dtiki_t *tiki, int animnum );
	int				( *Anim_NumForName )( dtiki_t *tiki, const char * name );
	int				( *Anim_Random )( dtiki_t *tiki, const char * name );
	int				( *Anim_NumFrames )( dtiki_t *tiki, int animnum );
	float			( *Anim_Frametime )( dtiki_t *tiki, int animnum );
	float			( *Anim_CrossTime )( dtiki_t *tiki, int animNum );
	float			( *Anim_Time )( dtiki_t *tiki, int animnum );
	void			( *Anim_Delta )( dtiki_t *tiki, int animnum, vec3_t delta );
	void			( *Anim_AbsoluteDelta )( dtiki_t *tiki, int animnum, vec3_t delta );
	int				( *Anim_Flags )( dtiki_t *tiki, int animnum );
	qboolean		( *Anim_HasCommands ) ( dtiki_t *tiki, int animnum );

	// Sound
	void			( *Sound )( vec3_t *org, int entnum, int channel, const char *sound_name, float volume, float min_dist, float pitch, float maxDist, qboolean streamed );
	void			( *StopSound )( int entnum, int channel );
	float			( *SoundLength ) ( int channel, const char *path );
	byte			*( *SoundAmplitudes )( int channel, const char * name );

	// GLOBAL ALIAS SYSTEM
	qboolean		( *GlobalAlias_Add )( const char * alias, const char * name, const char *parameters );
	const char		*( *GlobalAlias_FindRandom )( const char * alias, aliasListNode_t **ret );
	void			( *GlobalAlias_Dump )( void );
	void			( *GlobalAlias_Clear )( void );

	// su44
	dtiki_t*		( *TIKI_RegisterModel )( const char *fname );
	int			( *TIKI_NumAnims )( dtiki_t *tiki );

	// SURFACE SPECIFIC STUFF
	int			( *Surface_NameToNum )( dtiki_t *tiki, const char * name );
	const char	*( *Surface_NumToName )( dtiki_t *tiki, int num );
	int			( *Surface_Flags )( dtiki_t *tiki, int num );
	int			( *Surface_NumSkins )( dtiki_t *tiki, int num );

	// TAG SPECIFIC STUFF
	int			( *Tag_NumForName )( dtiki_t *tiki, const char * name );
	const char	*( *Tag_NameForNum )( dtiki_t *tiki, int num );
	orientation_t	( *Tag_Orientation )( dtiki_t *tiki, int anim, int frame, int num, float scale, int *bone_tag, vec4_t *bone_quat );
	orientation_t	( *Tag_OrientationEx )( dtiki_t *tiki, int anim, int frame, int num, float scale, int *bone_tag, vec4_t *bone_quat,
		int crossblend_anim, int crossblend_frame, float crossblend_lerp, qboolean uselegs, qboolean usetorso, int torso_anim, int torso_frame,
		int torso_crossblend_anim, int torso_crossblend_frame, float torso_crossblend_lerp );

	// su44: these are for cg message writing
	void	( *MSG_WriteBits )( int value, int bits );
	void	( *MSG_WriteChar )( int c );
	void	( *MSG_WriteByte )( int c );
	void	( *MSG_WriteSVC )( int c );
	void	( *MSG_WriteShort )( int c );
	void	( *MSG_WriteLong )( int c );
	void	( *MSG_WriteFloat )( float f );
	void	( *MSG_WriteString )( const char *s );
	void	( *MSG_WriteAngle8 )( float f );
	void	( *MSG_WriteAngle16 )( float f );
	void	( *MSG_WriteCoord )( float f );
	void	( *MSG_WriteDir )( const vec_t *dir );
	void	( *MSG_StartCGM )( int type );
	void	( *MSG_EndCGM )();
	void	( *MSG_SetClient )( int iClient );
	void	( *SetBroadcastAll )();
	void	( *SetBroadcastVisible )( const vec_t *vPos, const vec_t *vPosB );
	void	( *SetBroadcastHearable )( const vec_t *vPos, const vec_t *vPosB );

	void **DebugLines;
	int *numDebugLines;
	void **DebugStrings;
	int *numDebugStrings;

} gameImport_t;

#endif

typedef struct gameExport_s {
	int			apiversion;

	// the init function will only be called when a game starts,
	// not each time a level is loaded.  Persistant data for clients
	// and the server can be allocated in init
	void			( *Init )( int startTime, int randomSeed );
	void			( *Shutdown ) ( void );
	void			( *Cleanup ) ( qboolean samemap );
	void			( *Precache )( void );

	void			( *SetMap )( const char *mapName );
	void			( *Restart )( );
	void			( *SetTime )( int svsStartTime, int svsTime );

	// each new level entered will cause a call to SpawnEntities
	void			( *SpawnEntities ) ( char *entstring, int levelTime );

	// return NULL if the client is allowed to connect, otherwise return
	// a text string with the reason for denial
	char		   *( *ClientConnect )( int clientNum, qboolean firstTime );

	void			( *ClientBegin )( gentity_t *ent, usercmd_t *cmd );
	void			( *ClientUserinfoChanged )( gentity_t *ent, const char *userinfo );
	void			( *ClientDisconnect )( gentity_t *ent );
	void			( *ClientCommand )( gentity_t *ent );
	void			( *ClientThink )( gentity_t *ent, usercmd_t *cmd, usereyes_t *eyeinfo );

	void			( *BotBegin )( gentity_t *ent);
	void			( *BotThink )( gentity_t *ent, int msec );

	void			( *PrepFrame )( void );
	void			( *RunFrame )( int levelTime, int frameTime );

	void			( *ServerSpawned )( void );

	void			( *RegisterSounds )( );
	qboolean		( *AllowPaused )( );

	// ConsoleCommand will be called when a command has been issued
	// that is not recognized as a builtin function.
	// The game can issue gi.argc() / gi.argv() commands to get the command
	// and parameters.  Return qfalse if the game doesn't recognize it as a command.
	qboolean		( *ConsoleCommand )( void );

	void			( *ArchivePersistant )( const char *name, qboolean loading );

	// ReadLevel is called after the default map information has been
	// loaded with SpawnEntities, so any stored client spawn spots will
	// be used when the clients reconnect.
	void			( *WriteLevel )( const char *filename, qboolean autosave );
	qboolean		( *ReadLevel )( const char *filename );
	qboolean		( *LevelArchiveValid )( const char *filename );

	void			( *ArchiveInteger )( int *i );
	void			( *ArchiveFloat )( float *fl );
	void			( *ArchiveString )( char *s );
	void			( *ArchiveSvsTime )( int *pi );
	orientation_t	( *TIKI_Orientation )( gentity_t *edict, int num );
	void			( *DebugCircle )( float *org, float radius, float r, float g, float b, float alpha, qboolean horizontal );
	void			( *SetFrameNumber )( int frameNumber );
	void			( *SoundCallback )( int entNum, soundChannel_t channelNumber, const char *name );

	//
	// global variables shared between game and server
	//

	// The gentities array is allocated in the game dll so it
	// can vary in size from one game to another.
	// 
	// The size will be fixed when ge->Init() is called
	// the server can't just use pointer arithmetic on gentities, because the
	// server's sizeof(struct gentity_s) doesn't equal gentitySize
	profGame_t			*profStruct;
	struct gentity_s	*gentities;
	int					gentitySize;
	int					num_entities;		// current number, <= max_entities
	int					max_entities;

	const char        *errorMessage;
} gameExport_t;

#ifdef __cplusplus
extern "C"
#endif
gameExport_t* GetGameAPI( gameImport_t *import );

#if 0
//
// system traps provided by the main engine
//
typedef enum {
	//============== general Quake services ==================

	G_PRINT,		//  )( const char *string );
	// print message on the local console

	G_ERROR,		//  )( const char *string );
	// abort the game

	G_MILLISECONDS,	//  )( void );
	// get current time for profiling reasons
	// this should NOT be used for any game related tasks,
	// because it is not journaled

	// console variable interaction
	G_CVAR_REGISTER,	//  )( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
	G_CVAR_UPDATE,	//  )( vmCvar_t *vmCvar );
	G_CVAR_SET,		//  )( const char *var_name, const char *value );
	G_CVAR_VARIABLE_INTEGER_VALUE,	//  )( const char *var_name );

	G_CVAR_VARIABLE_STRING_BUFFER,	//  )( const char *var_name, char *buffer, int bufsize );

	G_ARGC,			//  )( void );
	// ClientCommand and ServerCommand parameter access

	G_ARGV,			//  )( int n, char *buffer, int bufferLength );

	G_FS_FOPEN_FILE,	//  )( const char *qpath, fileHandle_t *file, fsMode_t mode );
	G_FS_READ,		//  )( void *buffer, int len, fileHandle_t f );
	G_FS_WRITE,		//  )( const void *buffer, int len, fileHandle_t f );
	G_FS_FCLOSE_FILE,		//  )( fileHandle_t f );

	G_SEND_CONSOLE_COMMAND,	//  )( const char *text );
	// add commands to the console as if they were typed in
	// for map changing, etc


	//=========== server specific functionality =============

	G_LOCATE_GAME_DATA,		//  )( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
	//							playerState_t *clients, int sizeofGameClient );
	// the game needs to let the server system know where and how big the gentities
	// are, so it can look at them directly without going through an interface

	G_DROP_CLIENT,		//  )( int clientNum, const char *reason );
	// kick a client off the server with a message

	G_SEND_SERVER_COMMAND,	//  )( int clientNum, const char *fmt, ... );
	// reliably sends a command string to be interpreted by the given
	// client.  If clientNum is -1, it will be sent to all clients

	G_SET_CONFIGSTRING,	//  )( int num, const char *string );
	// config strings hold all the index strings, and various other information
	// that is reliably communicated to all clients
	// All of the current configstrings are sent to clients when
	// they connect, and changes are sent to all connected clients.
	// All confgstrings are cleared at each level start.

	G_GET_CONFIGSTRING,	//  )( int num, char *buffer, int bufferSize );

	G_GET_USERINFO,		//  )( int num, char *buffer, int bufferSize );
	// userinfo strings are maintained by the server system, so they
	// are persistant across level loads, while all other game visible
	// data is completely reset

	G_SET_USERINFO,		//  )( int num, const char *buffer );

	G_GET_SERVERINFO,	//  )( char *buffer, int bufferSize );
	// the serverinfo info string has all the cvars visible to server browsers

	G_SET_BRUSH_MODEL,	//  )( gentity_t *ent, const char *name );
	// sets mins and maxs based on the brushmodel name

	G_TRACE,	// ( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
	// collision detection against all linked entities

	G_POINT_CONTENTS,	//  )( const vec3_t point, int passEntityNum );
	// point contents against all linked entities

	G_IN_PVS,			//  )( const vec3_t p1, const vec3_t p2 );

	G_IN_PVS_IGNORE_PORTALS,	//  )( const vec3_t p1, const vec3_t p2 );

	G_ADJUST_AREA_PORTAL_STATE,	//  )( gentity_t *ent, qboolean open );

	G_AREAS_CONNECTED,	//  )( int area1, int area2 );

	G_LINKENTITY,		//  )( gentity_t *ent );
	// an entity will never be sent to a client or used for collision
	// if it is not passed to linkentity.  If the size, position, or
	// solidity changes, it must be relinked.

	G_UNLINKENTITY,		//  )( gentity_t *ent );		
	// call before removing an interactive entity

	g_entities_IN_BOX,	//  )( const vec3_t mins, const vec3_t maxs, gentity_t **list, int maxcount );
	// EntitiesInBox will return brush models based on their bounding box,
	// so exact determination must still be done with EntityContact

	G_ENTITY_CONTACT,	//  )( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
	// perform an exact check against inline brush models of non-square shape

	// access for bots to get and free a server client (FIXME?)
	G_BOT_ALLOCATE_CLIENT,	//  )( void );

	G_BOT_FREE_CLIENT,	//  )( int clientNum );

	G_GET_USERCMD,	//  )( int clientNum, usercmd_t *cmd )

	G_GET_ENTITY_TOKEN,	// qboolean  )( char *buffer, int bufferSize )
	// Retrieves the next string token from the entity spawn text, returning
	// false when all tokens have been parsed.
	// This should only be done at GAME_INIT time.

	G_FS_GETFILELIST,
	G_DEBUG_POLYGON_CREATE,
	G_DEBUG_POLYGON_DELETE,
	G_REAL_TIME,
	G_SNAPVECTOR,

	G_TRACECAPSULE,	// ( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask );
	G_ENTITY_CONTACTCAPSULE,	//  )( const vec3_t mins, const vec3_t maxs, const gentity_t *ent );
	
	// 1.32
	G_FS_SEEK,

	BOTLIB_SETUP = 200,				//  )( void );
	BOTLIB_SHUTDOWN,				//  )( void );
	BOTLIB_LIBVAR_SET,
	BOTLIB_LIBVAR_GET,
	BOTLIB_PC_ADD_GLOBAL_DEFINE,
	BOTLIB_START_FRAME,
	BOTLIB_LOAD_MAP,
	BOTLIB_UPDATENTITY,
	BOTLIB_TEST,

	BOTLIB_GET_SNAPSHOT_ENTITY,		//  )( int client, int ent );
	BOTLIB_GET_CONSOLE_MESSAGE,		//  )( int client, char *message, int size );
	BOTLIB_USER_COMMAND,			//  )( int client, usercmd_t *ucmd );

	BOTLIB_AAS_ENABLE_ROUTING_AREA = 300,
	BOTLIB_AAS_BBOX_AREAS,
	BOTLIB_AAS_AREA_INFO,
	BOTLIB_AAS_ENTITY_INFO,

	BOTLIB_AAS_INITIALIZED,
	BOTLIB_AAS_PRESENCE_TYPE_BOUNDING_BOX,
	BOTLIB_AAS_TIME,

	BOTLIB_AAS_POINT_AREA_NUM,
	BOTLIB_AAS_TRACE_AREAS,

	BOTLIB_AAS_POINT_CONTENTS,
	BOTLIB_AAS_NEXT_BSP_ENTITY,
	BOTLIB_AAS_VALUE_FOR_BSP_EPAIR_KEY,
	BOTLIB_AAS_VECTOR_FOR_BSP_EPAIR_KEY,
	BOTLIB_AAS_FLOAT_FOR_BSP_EPAIR_KEY,
	BOTLIB_AAS_INT_FOR_BSP_EPAIR_KEY,

	BOTLIB_AAS_AREA_REACHABILITY,

	BOTLIB_AAS_AREA_TRAVEL_TIME_TO_GOAL_AREA,

	BOTLIB_AAS_SWIMMING,
	BOTLIB_AAS_PREDICT_CLIENT_MOVEMENT,

	BOTLIB_EA_SAY = 400,
	BOTLIB_EA_SAY_TEAM,
	BOTLIB_EA_COMMAND,

	BOTLIB_EA_ACTION,
	BOTLIB_EA_GESTURE,
	BOTLIB_EA_TALK,
	BOTLIB_EA_ATTACK,
	BOTLIB_EA_USE,
	BOTLIB_EA_RESPAWN,
	BOTLIB_EA_CROUCH,
	BOTLIB_EA_MOVE_UP,
	BOTLIB_EA_MOVE_DOWN,
	BOTLIB_EA_MOVE_FORWARD,
	BOTLIB_EA_MOVE_BACK,
	BOTLIB_EA_MOVE_LEFT,
	BOTLIB_EA_MOVE_RIGHT,

	BOTLIB_EA_SELECT_WEAPON,
	BOTLIB_EA_JUMP,
	BOTLIB_EA_DELAYED_JUMP,
	BOTLIB_EA_MOVE,
	BOTLIB_EA_VIEW,

	BOTLIB_EA_END_REGULAR,
	BOTLIB_EA_GET_INPUT,
	BOTLIB_EA_RESET_INPUT,


	BOTLIB_AI_LOAD_CHARACTER = 500,
	BOTLIB_AI_FREE_CHARACTER,
	BOTLIB_AI_CHARACTERISTIC_FLOAT,
	BOTLIB_AI_CHARACTERISTIC_BFLOAT,
	BOTLIB_AI_CHARACTERISTIC_INTEGER,
	BOTLIB_AI_CHARACTERISTIC_BINTEGER,
	BOTLIB_AI_CHARACTERISTIC_STRING,

	BOTLIB_AI_ALLOC_CHAT_STATE,
	BOTLIB_AI_FREE_CHAT_STATE,
	BOTLIB_AI_QUEUE_CONSOLE_MESSAGE,
	BOTLIB_AI_REMOVE_CONSOLE_MESSAGE,
	BOTLIB_AI_NEXT_CONSOLE_MESSAGE,
	BOTLIB_AI_NUM_CONSOLE_MESSAGE,
	BOTLIB_AI_INITIAL_CHAT,
	BOTLIB_AI_REPLY_CHAT,
	BOTLIB_AI_CHAT_LENGTH,
	BOTLIB_AI_ENTER_CHAT,
	BOTLIB_AI_STRING_CONTAINS,
	BOTLIB_AI_FIND_MATCH,
	BOTLIB_AI_MATCH_VARIABLE,
	BOTLIB_AI_UNIFY_WHITE_SPACES,
	BOTLIB_AI_REPLACE_SYNONYMS,
	BOTLIB_AI_LOAD_CHAT_FILE,
	BOTLIB_AI_SET_CHAT_GENDER,
	BOTLIB_AI_SET_CHAT_NAME,

	BOTLIB_AI_RESET_GOAL_STATE,
	BOTLIB_AI_RESET_AVOID_GOALS,
	BOTLIB_AI_PUSH_GOAL,
	BOTLIB_AI_POP_GOAL,
	BOTLIB_AI_EMPTY_GOAL_STACK,
	BOTLIB_AI_DUMP_AVOID_GOALS,
	BOTLIB_AI_DUMP_GOAL_STACK,
	BOTLIB_AI_GOAL_NAME,
	BOTLIB_AI_GET_TOP_GOAL,
	BOTLIB_AI_GET_SECOND_GOAL,
	BOTLIB_AI_CHOOSE_LTG_ITEM,
	BOTLIB_AI_CHOOSE_NBG_ITEM,
	BOTLIB_AI_TOUCHING_GOAL,
	BOTLIB_AI_ITEM_GOAL_IN_VIS_BUT_NOT_VISIBLE,
	BOTLIB_AI_GET_LEVEL_ITEM_GOAL,
	BOTLIB_AI_AVOID_GOAL_TIME,
	BOTLIB_AI_INIT_LEVEL_ITEMS,
	BOTLIB_AI_UPDATE_ENTITY_ITEMS,
	BOTLIB_AI_LOAD_ITEM_WEIGHTS,
	BOTLIB_AI_FREE_ITEM_WEIGHTS,
	BOTLIB_AI_SAVE_GOAL_FUZZY_LOGIC,
	BOTLIB_AI_ALLOC_GOAL_STATE,
	BOTLIB_AI_FREE_GOAL_STATE,

	BOTLIB_AI_RESET_MOVE_STATE,
	BOTLIB_AI_MOVE_TO_GOAL,
	BOTLIB_AI_MOVE_IN_DIRECTION,
	BOTLIB_AI_RESET_AVOID_REACH,
	BOTLIB_AI_RESET_LAST_AVOID_REACH,
	BOTLIB_AI_REACHABILITY_AREA,
	BOTLIB_AI_MOVEMENT_VIEW_TARGET,
	BOTLIB_AI_ALLOC_MOVE_STATE,
	BOTLIB_AI_FREE_MOVE_STATE,
	BOTLIB_AI_INIT_MOVE_STATE,

	BOTLIB_AI_CHOOSE_BEST_FIGHT_WEAPON,
	BOTLIB_AI_GET_WEAPON_INFO,
	BOTLIB_AI_LOAD_WEAPON_WEIGHTS,
	BOTLIB_AI_ALLOC_WEAPON_STATE,
	BOTLIB_AI_FREE_WEAPON_STATE,
	BOTLIB_AI_RESET_WEAPON_STATE,

	BOTLIB_AI_GENETIC_PARENTS_AND_CHILD_SELECTION,
	BOTLIB_AI_INTERBREED_GOAL_FUZZY_LOGIC,
	BOTLIB_AI_MUTATE_GOAL_FUZZY_LOGIC,
	BOTLIB_AI_GET_NEXT_CAMP_SPOT_GOAL,
	BOTLIB_AI_GET_MAP_LOCATION_GOAL,
	BOTLIB_AI_NUM_INITIAL_CHATS,
	BOTLIB_AI_GET_CHAT_MESSAGE,
	BOTLIB_AI_REMOVE_FROM_AVOID_GOALS,
	BOTLIB_AI_PREDICT_VISIBLE_POSITION,

	BOTLIB_AI_SET_AVOID_GOAL_TIME,
	BOTLIB_AI_ADD_AVOID_SPOT,
	BOTLIB_AAS_ALTERNATIVE_ROUTE_GOAL,
	BOTLIB_AAS_PREDICT_ROUTE,
	BOTLIB_AAS_POINT_REACHABILITY_AREA_INDEX,

	BOTLIB_PC_LOAD_SOURCE,
	BOTLIB_PC_FREE_SOURCE,
	BOTLIB_PC_READ_TOKEN,
	BOTLIB_PC_SOURCE_FILE_AND_LINE,

	G_TIKI_REGISTERMODEL,
	G_TIKI_GETBONES,
	G_TIKI_SETCHANNELS,
	G_TIKI_APPENDFRAMEBOUNDSANDRADIUS,
	G_TIKI_ANIMATE,
	G_TIKI_GETBONENAMEINDEX,
	G_MSG_WRITEBITS,
	G_MSG_WRITECHAR,
	G_MSG_WRITEBYTE,
	G_MSG_WRITESVC,
	G_MSG_WRITESHORT,
	G_MSG_WRITELONG,
	G_MSG_WRITEFLOAT,
	G_MSG_WRITESTR,
	G_MSG_WRITEANGLE8,
	G_MSG_WRITEANGLE16,
	G_MSG_WRITECOORD,
	G_MSG_WRITEDIR,
	G_MSG_STARTCGM,
	G_MSG_ENDCGM,
	G_MSG_SETCLIENT,
	G_MSG_BROADCASTALL,
	G_MSG_BROADCASTVISIBLE,
	G_MSG_BROADCASTHEARABLE,
} gameImport_t;


//
// functions exported by the game subsystem
//
typedef enum {
	GAME_INIT,	//  )( int levelTime, int randomSeed, int restart );
	// init and shutdown will be called every single level
	// The game should call G_GET_ENTITY_TOKEN to parse through all the
	// entity configuration text and spawn gentities.

	GAME_SHUTDOWN,	// (void);

	GAME_CLIENT_CONNECT,	//  )( int clientNum, qboolean firstTime, qboolean isBot );
	// return NULL if the client is allowed to connect, otherwise return
	// a text string with the reason for denial

	GAME_CLIENT_BEGIN,				//  )( int clientNum );

	GAME_CLIENT_USERINFO_CHANGED,	//  )( int clientNum );

	GAME_CLIENT_DISCONNECT,			//  )( int clientNum );

	GAME_CLIENT_COMMAND,			//  )( int clientNum );

	GAME_CLIENT_THINK,				//  )( int clientNum );

	GAME_RUN_FRAME,					//  )( int levelTime );

	GAME_CONSOLE_COMMAND,			//  )( void );
	// ConsoleCommand will be called when a command has been issued
	// that is not recognized as a builtin function.
	// The game can issue gi.argc() / gi.argv() commands to get the command
	// and parameters.  Return qfalse if the game doesn't recognize it as a command.

	BOTAI_START_FRAME				//  )( int time );
} gameExport_t;
#endif
