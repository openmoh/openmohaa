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

#ifndef __CG_PUBLIC_H__
#define __CG_PUBLIC_H__

#define	CMD_BACKUP			64
#define	CMD_MASK			(CMD_BACKUP - 1)
// allow a lot of command backups for very fast systems
// multiple commands may be combined into a single packet, so this
// needs to be larger than PACKET_BACKUP


#define	MAX_ENTITIES_IN_SNAPSHOT	1024
#define	MAX_SOUNDS_IN_SNAPSHOT		64
#define MAX_HUDDRAW_ELEMENTS		256

// snapshots are a view of the server at a given time

// Snapshots are generated at regular time intervals by the server,
// but they may not be sent if a client's rate level is exceeded, or
// they may be dropped by the network.
typedef struct {
	int				snapFlags;			// SNAPFLAG_RATE_DELAYED, etc
	int				ping;

	int				serverTime;		// server time the message is valid for (in msec)

	byte			areamask[MAX_MAP_AREA_BYTES];		// portalarea visibility bits

	playerState_t	ps;						// complete information about the current player at this time

	int				numEntities;			// all of the entities that need to be presented
	entityState_t	entities[MAX_ENTITIES_IN_SNAPSHOT];	// at the time of this snapshot

	int				numServerCommands;		// text based server commands to execute when this
	int				serverCommandSequence;	// snapshot becomes current

	int				number_of_sounds;
	server_sound_t	sounds[MAX_SOUNDS_IN_SNAPSHOT];
} snapshot_t;

typedef struct stopwatch_s {
	int iStartTime;
	int iEndTime;
} stopwatch_t;

typedef struct hdelement_s {
	qhandle_t hShader;
	char shaderName[ 64 ];
	int iX;
	int iY;
	int iWidth;
	int iHeight;
	float vColor[ 4 ];
	int iHorizontalAlign;
	int iVerticalAlign;
	qboolean bVirtualScreen;
	char string[ 2048 ];
	char fontName[ 64 ];
	fontheader_t *pFont;
} hdelement_t;

/*
==================================================================

clientAnim_t structure, wiped each new gamestate
reserved for viewmodelanim

==================================================================
*/
typedef struct clientAnim_s
{
	frameInfo_t		vmFrameInfo[ MAX_FRAMEINFOS ];
	int				lastVMAnim;
	int				lastVMAnimChanged;

	int				currentVMAnimSlot;
	int				currentVMDuration;

	qboolean		crossBlending;

	int				lastEquippedWeaponStat;
	char			lastActiveItem[ 80 ];
	int				lastAnimPrefixIndex;

	vec3_t			currentVMPosOffset;

	refEntity_t		ref;
	dtiki_t			*tiki;

} clientAnim_t;

enum {
  CGAME_EVENT_NONE,
  CGAME_EVENT_TEAMMENU,
  CGAME_EVENT_SCOREBOARD,
  CGAME_EVENT_EDITHUD
};


/*
==================================================================

functions imported from the main executable

==================================================================
*/

#define	CGAME_IMPORT_API_VERSION	4

#if 0
typedef struct clientGameImport_s {

	// print message on the local console
	void( *Printf )( const char *fmt, ... );
	void( *DPrintf )( const char *fmt, ... );

	// abort the game
	void( *Error )( int level, const char *fmt, ... );

	// milliseconds should only be used for performance tuning, never
	// for anything game related.  Get time from the CG_DrawActiveFrame parameter
	int( *Milliseconds )( void );

	void		*( *Malloc )( int size );
	void( *Free )( void *ptr );

	// console variable interaction
	cvar_t		*( *Cvar_Get )( const char *varName, const char *varValue, int varFlags );
	void( *Cvar_Register )( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
	void( *Cvar_Update )( vmCvar_t *vmCvar );
	void( *Cvar_Set )( const char *var_name, const char *value );
	void( *Cvar_VariableStringBuffer )( const char *var_name, char *buffer, int bufsize );

	// ServerCommand and ConsoleCommand parameter access
	int( *Argc )( void );
	char		*( *Argv )( int n );
	char		*( *Args )( void );

	// filesystem access
	// returns length of file
	int( *FS_FOpenFile )( const char *qpath, fileHandle_t *f, fsMode_t mode );
	int ( *FS_Read )( void *buffer, int len, fileHandle_t f );
	int ( *FS_Write )( const void *buffer, int len, fileHandle_t f );
	void ( *FS_FCloseFile )( fileHandle_t f );
	int ( *FS_GetFileList )( const char *path, const char *extension, char *listbuf, int bufsize );
	int ( *FS_Seek )( fileHandle_t f, long offset, fsOrigin_t origin ); // fsOrigin_t
	int ( *FS_ReadFile )( const char *name, void **buffer, qboolean quiet );
	void ( *FS_FreeFile )( void *buffer );
	int ( *FS_WriteFile )( const char *qpath, const void *buffer, int size );
	int ( *FS_FOpenFileWrite )( const char *name );
	void ( *FS_Flush )( fileHandle_t f );

	// add commands to the local console as if they were typed in
	// for map changing, etc.  The command is not executed immediately,
	// but will be executed in order the next time console commands
	// are processed
	void( *SendConsoleCommand )( const char *text );

	// register a command name so the console can perform command completion.
	// FIXME: replace this with a normal console command "defineCommand"?
	void( *AddCommand )( const char *cmdName, xcommand_t function );

	// send a string to the server over the network
	void( *SendClientCommand )( const char *s );

	// force a screen update, only used during gamestate load
	void( *UpdateScreen )( void );

	// model collision
	void( *CM_LoadMap )( const char *mapname );
	int( *CM_NumInlineModels )( void );
	clipHandle_t( *CM_InlineModel )( int index );		// 0 = world, 1+ = bmodels
	clipHandle_t( *CM_TempBoxModel )( const vec3_t mins, const vec3_t maxs, qboolean capsule );
	int( *CM_PointContents )( const vec3_t p, clipHandle_t model );
	int( *CM_TransformedPointContents )( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
	void( *CM_BoxTrace )( trace_t *results, const vec3_t start, const vec3_t end,
		const vec3_t mins, const vec3_t maxs,
		clipHandle_t model, int brushmask, qboolean capsule );
	void( *CM_TransformedBoxTrace )( trace_t *results, const vec3_t start, const vec3_t end,
		const vec3_t mins, const vec3_t maxs,
		clipHandle_t model, int brushmask,
		const vec3_t origin, const vec3_t angles, qboolean capsule );

	// Returns the projection of a polygon onto the solid brushes in the world
	int( *CM_MarkFragments )( int numPoints, const vec3_t *points,
		const vec3_t projection,
		int maxPoints, vec3_t pointBuffer,
		int maxFragments, markFragment_t *fragmentBuffer );

	// normal sounds will have their volume dynamically changed as their entity
	// moves and the listener moves
	void( *S_StartSound )( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx );
	void( *S_StopLoopingSound )( int entnum );

	// a local sound is always played full volume
	void( *S_StartLocalSound )( sfxHandle_t sfx, int channelNum );
	void( *S_ClearLoopingSounds )( qboolean killall );
	void( *S_AddLoopingSound )( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
	void( *S_AddRealLoopingSound )( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
	void( *S_UpdateEntityPosition )( int entityNum, const vec3_t origin );

	// respatialize recalculates the volumes of sound as they should be heard by the
	// given entityNum and position
	void( *S_Respatialize )( int entityNum, const vec3_t origin, vec3_t axis[ 3 ], int inwater );
	sfxHandle_t( *S_RegisterSound )( const char *sample, qboolean compressed );		// returns buzz if not found
	void( *S_StartBackgroundTrack )( const char *intro, const char *loop );	// empty name stops music
	void( *S_StopBackgroundTrack )( void );


	void( *R_LoadWorldMap )( const char *mapname );

	// all media should be registered during level startup to prevent
	// hitches during gameplay
	qhandle_t( *R_RegisterModel )( const char *name );			// returns rgb axis if not found
	qhandle_t( *R_RegisterSkin )( const char *name );			// returns all white if not found
	qhandle_t( *R_RegisterShader )( const char *name );			// returns all white if not found
	qhandle_t( *R_RegisterShaderNoMip )( const char *name );			// returns all white if not found

	// a scene is built up by calls to R_ClearScene and the various R_Add functions.
	// Nothing is drawn until R_RenderScene is called.
	void( *R_ClearScene )( void );
	void( *R_AddRefEntityToScene )( const refEntity_t *re );

	// polys are intended for simple wall marks, not really for doing
	// significant construction
	void( *R_AddPolyToScene )( qhandle_t hShader, int numVerts, const polyVert_t *verts );
	void( *R_AddPolysToScene )( qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys );
	void( *R_AddLightToScene )( const vec3_t org, float intensity, float r, float g, float b );
	int( *R_LightForPoint )( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );
	void( *R_RenderScene )( const refdef_t *fd );
	void( *R_SetColor )( const float *rgba );	// NULL = 1,1,1,1
	void( *R_DrawStretchPic )( float x, float y, float w, float h,
		float s1, float t1, float s2, float t2, qhandle_t hShader );
	void( *R_ModelBounds )( clipHandle_t model, vec3_t mins, vec3_t maxs );
	int( *R_LerpTag )( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame,
		float frac, const char *tagName );
	void( *R_RemapShader )( const char *oldShader, const char *newShader, const char *timeOffset );

	// The glconfig_t will not change during the life of a cgame.
	// If it needs to change, the entire cgame will be restarted, because
	// all the qhandle_t are then invalid.
	void( *GetGlconfig )( glconfig_t *glconfig );

	// the gamestate should be grabbed at startup, and whenever a
	// configstring changes
	void( *GetGameState )( gameState_t *gamestate );

	// cgame will poll each frame to see if a newer snapshot has arrived
	// that it is interested in.  The time is returned seperately so that
	// snapshot latency can be calculated.
	void( *GetCurrentSnapshotNumber )( int *snapshotNumber, int *serverTime );

	// a snapshot get can fail if the snapshot (or the entties it holds) is so
	// old that it has fallen out of the client system queue
	qboolean( *GetSnapshot )( int snapshotNumber, snapshot_t *snapshot );

	// retrieve a text command from the server stream
	// the current snapshot will hold the number of the most recent command
	// qfalse can be returned if the client system handled the command
	// argc )() / argv )() can be used to examine the parameters of the command
	qboolean( *GetServerCommand )( int serverCommandNumber );

	// returns the most recent command number that can be passed to GetUserCmd
	// this will always be at least one higher than the number in the current
	// snapshot, and it may be quite a few higher if it is a fast computer on
	// a lagged connection
	int( *GetCurrentCmdNumber )( void );

	qboolean( *GetUserCmd )( int cmdNumber, usercmd_t *ucmd );

	// used for the weapon select and zoom
	void( *SetUserCmdValue )( int stateValue, float sensitivityScale );

	int( *MemoryRemaining )( void );
	void( *R_RegisterFont )( const char *fontName, int pointSize, fontInfo_t *font );
	qboolean( *Key_IsDown )( int keynum );
	int( *Key_GetCatcher )( void );
	void( *Key_SetCatcher )( int catcher );
	int( *Key_GetKey )( const char *binding );

	// ley0k: animation system
	const char *( *Anim_NameForNum )( tiki_t *tiki, int animNum );
	int( *Anim_NumForName )( tiki_t *tiki, const char *name );
	int( *Anim_Random )( tiki_t *tiki, const char *name );
	int( *Anim_NumFrames )( tiki_t *tiki, int animNum );
	float( *Anim_Time )( tiki_t *tiki, int animNum );
	float( *Anim_Frametime )( tiki_t *tiki, int animNum );
	int			( *Anim_Delta )( tiki_t *tiki, int animNum, vec3_t delta );
	int			( *Anim_Flags )( tiki_t *tiki, int animNum );
	float		( *Anim_CrossblendTime )( tiki_t *tiki, int animNum );
	qboolean	( *Anim_HasCommands )( tiki_t *tiki, int animNum );
	qboolean	( *Frame_Commands )( tiki_t *tiki, int animNum, int frameNum, tiki_cmd_t *tikiCmds );
	qboolean( *Frame_CommandsTime )( tiki_t *pmdl, int animNum, float start, float end, tiki_cmd_t *tikiCmd );

	// ley0k: TIKI tags
	int			( *Tag_NumForName )( tiki_t *tiki, const char * name );
	const char	*( *Tag_NameForNum )( tiki_t *tiki, int num );

	// IneQuation, wombat
	int			( *R_Text_Width )( fontInfo_t *font, const char *text, int limit, qboolean useColourCodes );
	int			( *R_Text_Height )( fontInfo_t *font, const char *text, int limit, qboolean useColourCodes );
	void		( *R_Text_Paint )( fontInfo_t *font, float x, float y, float scale, float alpha, const char *text, float adjust, int limit, qboolean useColourCodes, qboolean is640 );
	void		( *R_Text_PaintChar )( fontInfo_t *font, float x, float y, float scale, int c, qboolean is640 );
	// su44: MoHAA TIKI model system API
	tiki_t*		( *TIKI_RegisterModel )( const char *fname );
	bone_t*		( *TIKI_GetBones )( int numBones );
	void		( *TIKI_SetChannels )( struct tiki_s *tiki, int animIndex, float animTime, float animWeight, bone_t *bones );
	void		( *TIKI_AppendFrameBoundsAndRadius )( struct tiki_s *tiki, int animIndex, float animTime, float *outRadius, vec3_t outBounds[ 2 ] );
	void		( *TIKI_Animate )( struct tiki_s *tiki, bone_t *bones );
	int			( *TIKI_GetBoneNameIndex )( const char *boneName );
	int			( *TIKI_GetAnimIndex )( tiki_t *tiki, const char *animName ); // returns -1 if not found
	int			( *TIKI_GetBoneIndex )( tiki_t *tiki, const char *boneName ); // returns -1 if not found
	void		( *SetEyeInfo )( vec3_t origin, vec3_t angles );
	// su44: these are here only for cg_parsemsg.c
	int			( *MSG_ReadBits )( int bits );
	int			( *MSG_ReadByte )();
	int			( *MSG_ReadShort )();
	float		( *MSG_ReadCoord )();
	void		( *MSG_ReadDir )( vec3_t dir );
	char		*( *MSG_ReadString )();
	// su44: I need these for cg_beam.c
	int			( *R_GetShaderWidth )( qhandle_t shader );
	int			( *R_GetShaderHeight )( qhandle_t shader );

	int				( *CIN_PlayCinematic )( const char *arg0, int xpos, int ypos, int width, int height, int bits );
	e_status		( *CIN_StopCinematic )( int handle );
	e_status		( *CIN_RunCinematic )( int handle );
	void			( *CIN_DrawCinematic )( int handle );
	void			( *CIN_SetExtents )( int handle, int x, int y, int w, int h );

	qboolean		( *loadCamera )( const char *name );
	void			( *startCamera )( int time );
	qboolean		( *getCameraInfo )( int time, vec3_t *origin, vec3_t *angles );

	qboolean		( *GetEntityToken )( char *buffer, int bufferSize );

	clientAnim_t	*anim;
} clientGameImport_t;
#endif

typedef struct clientGameImport_s {

	int apiVersion;

	void ( *Printf )( const char *fmt, ... );
	void ( *DPrintf )( const char *fmt, ... );
	void ( *DebugPrintf )( const char *fmt, ... );
	void * ( *Malloc )( size_t size );
	void ( *Free )( void *ptr );
	void ( *Error )( int level, const char *fmt, ... );
	int ( *Milliseconds )( );
	const char * ( *LV_ConvertString )( const char *string );
	cvar_t * ( *Cvar_Get )( const char *varName, const char *varValue, int varFlags );
	void ( *Cvar_Set )( const char *varName, const char *varValue );
	int ( *Argc )( );
	char * ( *Argv )( int arg );
	char * ( *Args )( );
	void ( *AddCommand )( const char *cmdName );
	void ( *Cmd_Stuff )( const char *text );
	void ( *Cmd_Execute )( int exec_when, const char *text );
	void ( *Cmd_TokenizeString )( const char *textIn );
	int ( *FS_ReadFile )( const char *qpath, void **buffer, qboolean quiet );
	void ( *FS_FreeFile )( void *buffer );
	int ( *FS_WriteFile )( const char *qpath, const void *buffer, int size );
	void ( *FS_WriteTextFile )( const char *qpath, const void *buffer, int size );
	void ( *SendConsoleCommand )( const char *text );
	int ( *MSG_ReadBits )( int bits );
	int ( *MSG_ReadChar )( );
	int ( *MSG_ReadByte )( );
	int ( *MSG_ReadSVC )( );
	int ( *MSG_ReadShort )( );
	int ( *MSG_ReadLong )( );
	float ( *MSG_ReadFloat )( );
	char * ( *MSG_ReadString )( );
	char * ( *MSG_ReadStringLine )( );
	float ( *MSG_ReadAngle8 )( );
	float ( *MSG_ReadAngle16 )( );
	void ( *MSG_ReadData )( void *data, int len );
	float ( *MSG_ReadCoord )( );
	void ( *MSG_ReadDir )( vec3_t dir );
	void ( *SendClientCommand )( const char *text );
	void ( *CM_LoadMap )( const char *name );
	clipHandle_t ( *CM_InlineModel )( int index );
	int ( *CM_NumInlineModels )( );
	int ( *CM_PointContents )( const vec3_t point, clipHandle_t model );
	int ( *CM_TransformedPointContents )( const vec3_t point, clipHandle_t model, const vec3_t origin, const vec3_t angles );
	void ( *CM_BoxTrace )( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, int model, int brushMask, int cylinder );
	void ( *CM_TransformedBoxTrace )( trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, int model, int brushMask, const vec3_t origin, const vec3_t angles, int cylinder );
	clipHandle_t ( *CM_TempBoxModel )( const vec3_t mins, const vec3_t maxs, int contents );
	void ( *CM_PrintBSPFileSizes )( );
	qboolean ( *CM_LeafInPVS )( int leaf1, int leaf2 );
	int ( *CM_PointLeafnum )( const vec3_t p );
	int ( *R_MarkFragments )( int numPoints, const vec3_t *points, const vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer, float fRadiusSquared );
	int ( *R_MarkFragmentsForInlineModel )( clipHandle_t bmodel, vec3_t vAngles, vec3_t vOrigin, int numPoints, vec3_t *points, vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer, float fRadiusSquared );
	void ( *R_GetInlineModelBounds )( int index, vec3_t mins, vec3_t maxs );
	void ( *R_GetLightingForDecal )( vec3_t light, vec3_t facing, vec3_t origin );
	void ( *R_GetLightingForSmoke )( vec3_t light, vec3_t origin );
	int ( *R_GatherLightSources )( vec3_t pos, vec3_t *lightPos, vec3_t *lightIntensity, int maxLights );
	void ( *S_StartSound )( const vec3_t origin, int entNum, int entChannel, sfxHandle_t sfxHandle, float volume, float minDist, float pitch, float maxDist, int streamed );
	void ( *S_StartLocalSound )( const char *sound_name );
	void ( *S_StopSound )( int entNum, int channel );
	void ( *S_ClearLoopingSounds )( );
	void ( *S_AddLoopingSound )( const vec3_t origin, const vec3_t velocity, sfxHandle_t sfxHandle, float volume, float minDist, float maxDist, float pitch, int flags );
	void ( *S_Respatialize )( int entityNum, const vec3_t head, vec3_t axis[3] );
	void ( *S_BeginRegistration )( );
	sfxHandle_t ( *S_RegisterSound )( const char *name, qboolean streamed );
	void ( *S_EndRegistration )( );
	void ( *S_UpdateEntity )( int entityNum, const vec3_t origin, const vec3_t velocity, qboolean useListener );
	void ( *S_SetReverb )( int reverbType, float reverbLevel );
	void ( *S_SetGlobalAmbientVolumeLevel )( float volume );
	float ( *S_GetSoundTime )( sfxHandle_t handle );
	int ( *S_ChannelNameToNum )( const char *name );
	const char * ( *S_ChannelNumToName )( int channel );
	int ( *S_IsSoundPlaying )( int channelNumber, const char *name );
	void ( *MUSIC_NewSoundtrack )( const char *name );
	void( *MUSIC_UpdateMood )( int current, int fallback );
	void ( *MUSIC_UpdateVolume )( float volume, float fadeTime );
	float * ( *get_camera_offset )( qboolean *lookActive, qboolean *resetView );
	void ( *R_ClearScene )( );
	void ( *R_RenderScene )( const refdef_t *fd );
	void ( *R_LoadWorldMap )( const char *name );
	void ( *R_PrintBSPFileSizes )( );
	int ( *MapVersion )( );
	int ( *R_MapVersion )( );
	qhandle_t ( *R_RegisterModel )( const char *name );
	qhandle_t ( *R_SpawnEffectModel )( const char *name, vec3_t pos, vec3_t axis[3] );
	qhandle_t ( *R_RegisterServerModel )( const char *name );
	void ( *R_UnregisterServerModel )( qhandle_t hModel );
	qhandle_t ( *R_RegisterSkin )( const char *name );
	qhandle_t ( *R_RegisterShader )( const char *name );
	qhandle_t ( *R_RegisterShaderNoMip )( const char *name );
	void ( *R_AddRefEntityToScene )( const refEntity_t *ent, int parentEntityNumber );
	void ( *R_AddRefSpriteToScene )( const refEntity_t *ent );
	void ( *R_AddLightToScene )( const vec3_t org, float intensity, float r, float g, float b, int type );
	qboolean ( *R_AddPolyToScene )( qhandle_t hShader, int numVerts, const polyVert_t *verts, int renderFx );
	void ( *R_AddTerrainMarkToScene )( int terrainIndex, qhandle_t hShader, int numVerts, polyVert_t *verts, int renderFx );
	void ( *R_SetColor )( const vec4_t rgba );
	void ( *R_DrawStretchPic )( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
	fontheader_t * ( *R_LoadFont )( const char *name );
	void( *R_DrawString )( fontheader_t *font, const char *text, float x, float y, int maxLen, qboolean virtualScreen );
	refEntity_t * ( *R_GetRenderEntity )( int entityNumber );
	void ( *R_ModelBounds )( clipHandle_t model, vec3_t mins, vec3_t maxs );
	float ( *R_ModelRadius )( qhandle_t handle );
	float ( *R_Noise )( float x, float y, float z, float t );
	void ( *R_DebugLine )( vec3_t start, vec3_t end, float r, float g, float b, float alpha );
	baseshader_t * ( *GetShader )( int shaderNum );
	void ( *R_SwipeBegin )( float thistime, float life, qhandle_t shader );
	void ( *R_SwipePoint )( vec3_t point1, vec3_t point2, float time );
	void ( *R_SwipeEnd )( );
	int ( *R_GetShaderWidth )( qhandle_t hShader );
	int ( *R_GetShaderHeight )( qhandle_t hShader );
	void ( *R_DrawBox )( float x, float y, float w, float h );
	void ( *GetGameState )( gameState_t *gameState );
	int ( *GetSnapshot )( int snapshotNumber, snapshot_t *snapshot );
	int ( *GetServerStartTime )( );
	void ( *SetTime )( int time );
	void ( *GetCurrentSnapshotNumber )( int *snapshotNumber, int *serverTime );
	void ( *GetGlconfig )( glconfig_t *glConfig );
	qboolean ( *GetParseEntityState )( int parseEntityNumber, entityState_t *state );
	int ( *GetCurrentCmdNumber )( );
	qboolean ( *GetUserCmd )( int cmdNumber, usercmd_t *userCmd );
	qboolean ( *GetServerCommand )( int serverCommandNumber, qboolean differentServer );
	qboolean ( *Alias_Add )( const char *alias, const char *name, const char *parameters );
	qboolean( *Alias_ListAdd )( AliasList_t *list, const char *alias, const char *name, const char *parameters );
	const char * ( *Alias_FindRandom )( const char *alias, AliasListNode_t **ret );
	const char * ( *Alias_ListFindRandom )( AliasList_t *list, const char *alias, AliasListNode_t **ret );
	void ( *Alias_Dump )( );
	void ( *Alias_Clear )( );
	AliasList_t * ( *AliasList_New )( const char *name );
	void( *Alias_ListFindRandomRange )( AliasList_t *list, const char *alias, int *minIndex, int *maxIndex, float *totalWeight );
	AliasList_t * ( *Alias_GetGlobalList )( );
	void ( *UI_ShowMenu )( const char *name, qboolean bForce );
	void ( *UI_HideMenu )( const char *name, qboolean bForce );
	int ( *UI_FontStringWidth )( fontheader_t *font, const char *string, int maxLen );
	int ( *Key_StringToKeynum )( const char *str );
	const char * ( *Key_KeynumToBindString )( int keyNum );
	void ( *Key_GetKeysForCommand )( const char *command, int *key1, int *key2 );
	dtiki_t * ( *R_Model_GetHandle )( qhandle_t handle );
	int ( *TIKI_NumAnims )( dtiki_t *pmdl );
	void ( *TIKI_CalculateBounds )( dtiki_t *pmdl, float scale, vec3_t mins, vec3_t maxs );
	const char * ( *TIKI_Name )( dtiki_t *tiki );
	void * ( *TIKI_GetSkeletor )( dtiki_t *tiki, int entNum );
	void ( *TIKI_SetEyeTargetPos )( dtiki_t *tiki, int entNum, vec3_t pos );
	const char * ( *Anim_NameForNum )( dtiki_t *tiki, int animNum );
	int ( *Anim_NumForName )( dtiki_t *tiki, const char *name );
	int ( *Anim_Random )( dtiki_t *tiki, const char *name );
	int ( *Anim_NumFrames )( dtiki_t *tiki, int animNum );
	float ( *Anim_Time )( dtiki_t *tiki, int animNum );
	float ( *Anim_Frametime )( dtiki_t *tiki, int animNum );
	void ( *Anim_Delta )( dtiki_t *tiki, int animNum, vec3_t delta );
	int ( *Anim_Flags )( dtiki_t *tiki, int animNum );
	int ( *Anim_FlagsSkel )( dtiki_t *tiki, int animNum );
	float ( *Anim_CrossblendTime )( dtiki_t *tiki, int animNum );
	qboolean ( *Anim_HasCommands )( dtiki_t *tiki, int animNum );
	qboolean ( *Frame_Commands )( dtiki_t *tiki, int animNum, int frameNum, tiki_cmd_t *tikiCmds );
	qboolean ( *Frame_CommandsTime )( dtiki_t *pmdl, int animNum, float start, float end, tiki_cmd_t *tikiCmd );
	int ( *Surface_NameToNum )( dtiki_t *pmdl, const char *name );
	int ( *Tag_NumForName )( dtiki_t *pmdl, const char *name );
	const char * ( *Tag_NameForNum )( dtiki_t *pmdl, int animNum );
	void ( *ForceUpdatePose )( refEntity_t *model );
	orientation_t ( *TIKI_Orientation )( refEntity_t *model, int tagNum );
	qboolean ( *TIKI_IsOnGround )( refEntity_t *model, int tagNum, float threshold );
	void ( *UI_ShowScoreBoard )( const char *menuName );
	void ( *UI_HideScoreBoard )( );
	void ( *UI_SetScoreBoardItem )( int itemNumber, const char *data1, const char *data2, const char *data3, const char *data4, const char *data5, const char *data6, const char *data7, const char *data8, vec3_t textColor, vec3_t backColor, qboolean isHeader );
	void ( *UI_DeleteScoreBoardItems )( int maxIndex );
	void ( *UI_ToggleDMMessageConsole )( int consoleMode );
	dtiki_t * ( *TIKI_FindTiki )( const char *path );
	void ( *LoadResource )( const char *name );
	void ( *FS_CanonicalFilename )( char *name );

	cvar_t *fsDebug;
	hdelement_t *hudDrawElements;
	clientAnim_t *anim;
	stopwatch_t *stopWatch;
	void *pUnknownVar;
} clientGameImport_t;

/*
==================================================================

functions exported to the main executable

==================================================================
*/

typedef struct clientGameExport_s {
	void		( *CG_Init )( clientGameImport_t *imported, int serverMessageNum, int serverCommandSequence, int clientNum );
	void		( *CG_Shutdown )( void );
	void		( *CG_DrawActiveFrame )( int serverTime, int frametime, stereoFrame_t stereoView, qboolean demoPlayback );
	qboolean	( *CG_ConsoleCommand )( void );
	void		( *CG_GetRendererConfig )( void );
	void		( *CG_Draw2D )( void );
	void		( *CG_EyePosition )( vec3_t *eyePos );
	void		( *CG_EyeOffset )( vec3_t *eyeOffset );
	void		( *CG_EyeAngles )( vec3_t *eyeAngles );
	float		( *CG_SensitivityScale )( );
	void		( *CG_ParseCGMessage )( );
	void		( *CG_RefreshHudDrawElements )( );
	void		( *CG_HudDrawShader )( int info );
	void		( *CG_HudDrawFont )( int info );
	int			( *CG_GetParent )( int entNum );
	float		( *CG_GetObjectiveAlpha )( );
	int			( *CG_PermanentMark )( vec3_t origin, vec3_t dir, float orientation, float sScale, float tScale, float red, float green, float blue, float alpha, qboolean doLighting, float sCenter, float tCenter, markFragment_t *markFragments, void *polyVerts );
	int			( *CG_PermanentTreadMarkDecal )( treadMark_t *treadMark, qboolean startSegment, qboolean doLighting, markFragment_t *markFragments, void *polyVerts );
	int			( *CG_PermanentUpdateTreadMark )( treadMark_t *treadMark, float alpha, float minSegment, float maxSegment, float maxOffset, float texScale );
	void		( *CG_ProcessInitCommands )( dtiki_t *tiki, refEntity_t *ent );
	void		( *CG_EndTiki )( dtiki_t *tiki );
	char		*( *CG_GetColumnName )( int columnNum, int *columnWidth );
	void		( *CG_GetScoreBoardColor )( float *red, float *green, float *blue, float *alpha );
	void		( *CG_GetScoreBoardFontColor )( float *red, float *green, float *blue, float *alpha );
	int			( *CG_GetScoreBoardDrawHeader )( );
	void		( *CG_GetScoreBoardPosition )( float *x, float *y, float *width, float *height );
	int			( *CG_WeaponCommandButtonBits )( );
	int			( *CG_CheckCaptureKey )( int key, qboolean down, unsigned int time );

	qboolean	( *CG_Command_ProcessFile )( char *name, qboolean quiet, dtiki_t *curTiki );

} clientGameExport_t;

//----------------------------------------------

#endif // __CG_PUBLIC_H__
