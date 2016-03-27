#ifndef __CGAMEX86_H__
#define __CGAMEX86_H__

#include "hook.h"
#include "renderer_api.h"
#include "../game/bg_public.h"

#define MAX_CVARS 255;

typedef struct {
	cvar_t		**cvar;
	const char		*name;
	const char		*value;
	int			flags;
} regCvar_t;

typedef struct centity_s
{
	entityState_t currentState;
	entityState_t nextState;
	int teleported;
	int interpolate;
	int currentValid;
	int miscTime;
	int snapShotTime;
	int errorTime;
	vec3_t errorOrigin;
	vec3_t errorAngles;
	int extrapolated;
	vec3_t rawOrigin;
	vec3_t rawAngles;
	vec3_t beamEnd;
	vec3_t lerpOrigin;
	vec3_t lerpAngles;
	int tikiLoopSound;
	float tikiLoopSoundVolume;
	float tikiLoopSoundMinDist;
	float tikiLoopSoundMaxDist;
	float tikiLoopSoundPitch;
	int tikiLoopSoundFlags;
	vec4_t color;
	vec4_t clientColor;
	int clientFlags;
	int splashLastSpawnTime;
	int splashStillCount;
	vec4_t boneQuat[ 5 ];
	float animLastTimes[ MAX_FRAMEINFOS ];
	int animLast[ MAX_FRAMEINFOS ];
	int animLastWeight;
	int usageIndexLast;
	int footOnGround_Right;
	int footOnGround_Left;
	int nextLandTime;

} centity_t;

typedef void ( __stdcall *CG_PlaySound_f )( char *sound_name, float *origin, int channel,
	float volume, float min_distance, float pitch, int argstype );
extern CG_PlaySound_f CG_PlaySound;

typedef void ( *CG_Trace_t )(trace_t *results, const vec3_t start, const vec3_t mins ,
	const vec3_t maxs, const vec3_t end, int headnode, int brushmask, int cylinder,
	qboolean bClipEntities, char *description);
extern CG_Trace_t CG_Trace;

void CG_ParseCGMessage2( void );

class CPlayer;

extern uintptr_t dwCGameBase;
extern pmove_t * pm;

extern cvar_t *developer;
extern cvar_t *timescale;

extern cvar_t *cg_debugview;
extern cvar_t *cg_hud;
extern cvar_t *cl_run;
extern cvar_t *fs_basepath;
extern cvar_t *r_anaglyph;
extern cvar_t *ui_hud;
extern cvar_t *vm_offset_air_up;
extern cvar_t *vm_offset_air_side;
extern cvar_t *vm_offset_air_front;
extern cvar_t *vm_offset_crouch_side;
extern cvar_t *vm_offset_vel_up;
extern cvar_t *vm_offset_vel_side;
extern cvar_t *vm_offset_vel_front;

extern cvar_t *s_volume;

extern cvar_t *cg_showopcodes;
extern cvar_t *cg_scriptcheck;
extern cvar_t *cl_scriptfiles;

extern regCvar_t r_cvars[];

class Entity;

extern qboolean reborn;

centity_t * CG_GetEntity(int ent_num);
Entity *R_SpawnModel( const char *model, vec3_t origin, vec3_t angles );

const char *CG_GetServerInfo();
const char *CG_GetServerVariable( const char *variable );

#define ITEM_MAX_NAME			255
#define ITEM_MAX_PREFIX			255		// 255 is enough for a list of new items

typedef struct serverSound_s
{
	vec3_t origin;
	int entityNumber;
	int channel;
	short int soundIndex;
	float volume;
	float minDist;
	float maxDist;
	float pitch;
	qboolean stopFlag;
	qboolean streamed;

} serverSound_t;

typedef struct snapshot_s
{
	int snapFlags;
	int ping;
	int serverTime;
	unsigned char areaMask[ 32 ];
	playerState_t ps;
	int numEntities;
	entityState_t entities[ 1024 ];
	int numServerCommands;
	int serverCommandSequence;
	int numberOfSounds;
	serverSound_t sounds[ 64 ];

} snapshot_t;

typedef struct baseShader_s
{
	char shader[ MAX_QPATH ];
	int surfaceFlags;
	int contentFlags;

} baseShader_t;

typedef struct clientInfo_s
{
	int team;

} clientInfo_t;

typedef struct objective_s
{
	char text[ MAX_STRING_CHARS ];
	int flags;

} objective_t;

typedef struct rain_s
{
	float density;
	float speed;
	int speedVary;
	int slant;
	float length;
	float minDist;
	float width;
	char shader[ 16 ][ MAX_STRING_CHARS ];
	int numShaders;

} rain_t;

typedef struct cgs_s
{
	gameState_t gameState;
	glconfig_t glConfig;
	float screenXScale;
	float screenYScale;
	float screenXBias;
	int serverCommandSequence;
	int processedSnapshotNum;
	qboolean localServer;
	int levelStartTime;
	int matchEndTime;
	int serverLagTime;
	int gameType;
	int dmFlags;
	int teamFlags;
	int fragLimit;
	int timeLimit;
	int maxClients;
	int cinematic;
	char mapName[MAX_QPATH];
	int modelDraw[MAX_MODELS];
	int soundPrecache[MAX_SOUNDS];
	int numInlineModels;
	int inlineDrawModel[MAX_MODELS];
	vec3_t inlineModelMidpoints[MAX_MODELS];
	media_t media;

} cgs_t;

typedef struct cg_s
{
	int clientFrame;
	int clientNum;
	int demoPlayback;
	int levelShot;
	int latestSnapshotNum;
	int latestSnapshotTime;
	snapshot_t *snap;
	snapshot_t *nextSnap;
	snapshot_t activeSnapshots[ 2 ];
	float frameInterpolation;
	int thisFrameTeleport;
	int nextFrameTeleport;
	int nextFrameCameraCut;
	int frametime;
	int time;
	int physicsTime;
	int renderingThirdPerson;
	int hyperSpace;
	playerState_t predictedPlayerState;
	int validPPS;
	int predictedErrorTime;
	vec3_t predictedError;
	int weaponCommand;
	int weaponCommandSend;
	vec3_t autoAngles;
	vec3_t autoAxis[3];
	vec3_t autoAnglesSlow;
	vec3_t autoAxisSlow[3];
	vec3_t autoAnglesFast;
	vec3_t autoAxisFast[3];
	refDef_t refdef;
	vec3_t playerHeadPos;
	vec3_t refdefViewAngles;
	vec3_t currentViewPos;
	vec3_t currentViewAngles;
	float currentViewHeight;
	float currentViewBobPhase;
	float currentViewBobAmp;
	dtiki_t *lastPlayerWorldModel;
	dtiki_t *playerFPSModel;
	int playerFPSModelHandle;
	int fpsModelLastFrame;
	int fpsOnGround;
	dtiki_t *alliedPlayerModel;
	int alliedPlayerModelHandle;
	dtiki_t *axisPlayerModel;
	int axisPlayerModelHandle;
	vec3_t offsetViewAngles;
	vec3_t lastHeadAngles;
	vec3_t lastViewAngles;
	vec3_t eyeOffsetMax;
	float eyeOffsetFrac;
	vec3_t soundOrg;
	vec3_t soundAxis[3];
	vec3_t cameraOrigin;
	vec3_t cameraAngles;
	float cameraFov;
	float zoomSensitivity;
	int intermissionDisplay;
	int scoresRequestTime;
	int showScores;
	char scoresMenuName[256];
	int instaMessageMenu;
	int centerPrintTime;
	int centerPrintCharWidth;
	int centerPrintY;
	char centerPrint[1024];
	int centerPrintLines;
	int matchStartTime;
	refEntity_t testModelEntity;
	char testModelName[MAX_QPATH];
	int testGun;
	float farplaneDistance;
	float farplaneColor[3];
	int farplaneCull;
	int skyPortal;
	float skyAlpha;
	vec3_t skyOrigin;
	vec3_t skyAxis[3];
	vec2_t viewKick;
	float viewKickRecenter;
	float viewKickMinDecay;
	float viewKickMaxDecay;
	objective_t objectives[ MAX_OBJECTIVES ];
	float objectivesAlphaTime;
	float objectivesBaseAlpha;
	float objectivesDesiredAlpha;
	float objectivesCurrentAlpha;
	rain_t rain;
	clientInfo_t clientinfo[MAX_CLIENTS];

} cg_t;

typedef struct uidef_s
{
    int time;
    int vidWidth;
    int vidHeight;
    int mouseX;
    int mouseY;
    unsigned int mouseFlags;
    qboolean uiHasMouse;

} uidef_t;

//
// cl_input
//
typedef struct {
	int			down[2];		// key nums holding it down
	unsigned	downtime;		// msec timestamp
	unsigned	msec;			// msec down this frame if both a down and up happened
	qboolean	active;			// current state
	qboolean	wasPressed;		// set when down, not cleared when up
} kbutton_t;

typedef struct qkey_s
{
    qboolean down;
    int repeats;
    char *binding;

} qkey_t;

typedef struct fontHandle_s
{
	int	bla;
} fontHandle_t;

typedef struct clientAnim_s
{
	frameInfo_t vmFrameInfo[MAX_FRAMEINFOS];
	int lastVMAnim;
	int lastVMAnimChanged;
	int currentVMAnimSlot;
	int currentVMDuration;
	qboolean crossBlending;
	int lastEquippedWeaponStat;
	char lastActiveItem[80];
	int lastAnimPrefixIndex;
	vec3_t currentVMPosOffset;
	unsigned char un1[72];
	dtiki_t * tiki;

} clientAnim_t;

typedef struct hdElement_s
{
	qhandle_t shader;
	char shaderName[MAX_QPATH];
	int x;
	int y;
	int width;
	int height;
	vec4_t color;
	int horizontalAlign;
	int verticalAlign;
	qboolean virtualScreen;
	char string[MAX_STRING_CHARS];
	char fontName[MAX_QPATH];
	fontHeader_t *font;

} hdElement_t;

typedef struct stopWatch_s
{
	int startTime;
	int endTime;

} stopWatch_t;

typedef enum keyNum_e
{
	K_TAB = 9,
	K_ENTER = 13,
	K_ESCAPE = 27,
	K_SPACE = 32,

	K_BACKSPACE = 127,
	K_COMMAND,
	K_CAPSLOCK,
	K_POWER,
	K_PAUSE,
	K_UPARROW,
	K_DOWNARROW,
	K_LEFTARROW,
	K_RIGHTARROW,
	K_LALT,
	K_LCTRL,
	K_LSHIFT,
	K_RALT,
	K_RCTRL,
	K_RSHIFT,
	K_INS,
	K_DEL,
	K_PGDN,
	K_PGUP,
	K_HOME,
	K_END,
	K_F1,
	K_F2,
	K_F3,
	K_F4,
	K_F5,
	K_F6,
	K_F7,
	K_F8,
	K_F9,
	K_F10,
	K_F11,
	K_F12,
	K_F13,
	K_F14,
	K_F15,
	K_KPHOME,
	K_KPUPARROW,
	K_KPPGUP,
	K_KPLEFTARROW,
	K_KP5,
	K_KPRIGHTARROW,
	K_KPEND,
	K_KPDOWNARROW,
	K_KPPGDN,
	K_KPENTER,
	K_KPINS,
	K_KPDEL,
	K_KPSLASH,
	K_KPMINUS,
	K_KPPLUS,
	K_KPNUMLOCK,
	K_KPSTAR,
	K_KPEQUALS,
	K_MOUSE1,
	K_MOUSE2,
	K_MOUSE3,
	K_MOUSE4,
	K_MOUSE5,
	K_MWHEELDOWN,
	K_MWHEELUP,
	K_JOY1,
	K_JOY2,
	K_JOY3,
	K_JOY4,
	K_JOY5,
	K_JOY6,
	K_JOY7,
	K_JOY8,
	K_JOY9,
	K_JOY10,
	K_JOY11,
	K_JOY12,
	K_JOY13,
	K_JOY14,
	K_JOY15,
	K_JOY16,
	K_JOY17,
	K_JOY18,
	K_JOY19,
	K_JOY20,
	K_JOY21,
	K_JOY22,
	K_JOY23,
	K_JOY24,
	K_JOY25,
	K_JOY26,
	K_JOY27,
	K_JOY28,
	K_JOY29,
	K_JOY30,
	K_JOY31,
	K_JOY32,
	K_AUX1,
	K_AUX2,
	K_AUX3,
	K_AUX4,
	K_AUX5,
	K_AUX6,
	K_AUX7,
	K_AUX8,
	K_AUX9,
	K_AUX10,
	K_AUX11,
	K_AUX12,
	K_AUX13,
	K_AUX14,
	K_AUX15,
	K_AUX16,
	K_LWINKEY,
	K_RWINKEY,
	K_MENUKEY,
	K_LASTKEY

} keyNum_t;

// Dump memory to a file then read it
typedef struct tmpArchive
{

} tmpArchive;

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

typedef struct profCGame_s
{
	profVar_t CG_AddCEntity;
	profVar_t CG_DrawActiveFrame;
	profVar_t CG_AddPacketEntities;
	profVar_t CG_DrawActive;
	profVar_t CG_EntityShadow;
	profVar_t CG_Rain;
	profVar_t CG_AddBeams;
	profVar_t CG_ModelAnim;
	profVar_t CG_ProcessSnapshots;
	profVar_t CProbe1;
	profVar_t CProbe2;
	profVar_t CProbe3;

} profCGame_t;

typedef struct clientGameImport_s
{
	int apiVersion;

	void ( *Printf )( const char *fmt, ... );
	void ( *DPrintf )( const char *fmt, ... );
	void ( *DebugPrintf )( const char *fmt, ... );
	void * ( *Malloc )( size_t size );
	void ( *Free )( void *ptr );
	void ( *Error )( errorParm_t code, const char *fmt, ... );
	int ( *Milliseconds )( );
	char * ( *LV_ConvertString )( const char *string );
	cvar_t * ( *Cvar_Get )( const char *varName, const char *varValue, int varFlags );
	void ( *Cvar_Set )( const char *varName, const char *varValue );
	int ( *Argc )( );
	char * ( *Argv )( int arg );
	char * ( *Args )( );
	void ( *AddCommand )( char *cmdName, xcommand_t cmdFunction );
	void ( *Cmd_Stuff )( const char *text );
	void ( *Cmd_Execute )( cbufExec_t execWhen, char *text );
	void ( *Cmd_TokenizeString )( char *textIn );
	int ( *FS_ReadFile )( const char *qpath, void **buffer, qboolean bSilent );
	void ( *FS_FreeFile )( void *buffer );
	int ( *FS_WriteFile )( const char *qpath, void *buffer, int size );
	void ( *FS_WriteTextFile )( const char *qpath, void *buffer, int size );
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
	void ( *CM_LoadMap )( char *name );
	clipHandle_t ( *CM_InlineModel )( int index );
	int ( *CM_NumInlineModels )( );
	int ( *CM_PointContents )( vec3_t point, clipHandle_t model );
	int ( *CM_TransformedPointContents )( vec3_t point, clipHandle_t model, vec3_t origin, vec3_t angles );
	void ( *CM_BoxTrace )( trace_t *results, vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs, int model, int brushMask, int cylinder );
	void ( *CM_TransformedBoxTrace )( trace_t *results, vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs, int model, int brushMask, vec3_t origin, vec3_t angles, int cylinder );
	clipHandle_t ( *CM_TempBoxModel )( vec3_t mins, vec3_t maxs, int contents );
	void ( *CM_PrintBSPFileSizes )( );
	qboolean ( *CM_LeafInPVS )( int leaf1, int leaf2 );
	int ( *CM_PointLeafnum )( vec3_t p );
	int ( *R_MarkFragments )( int numPoints, vec3_t *points, vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer, float fRadiusSquared );
	int ( *R_MarkFragmentsForInlineModel )( clipHandle_t bmodel, vec3_t vAngles, vec3_t vOrigin, int numPoints, vec3_t *points, vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer, float fRadiusSquared );
	void ( *R_GetInlineModelBounds )( int index, vec3_t mins, vec3_t maxs );
	void ( *R_GetLightingForDecal )( vec3_t light, vec3_t facing, vec3_t origin );
	void ( *R_GetLightingForSmoke )( vec3_t light, vec3_t origin );
	int ( *R_GatherLightSources )( vec3_t pos, vec3_t *lightPos, vec3_t *lightIntensity, int maxLights );
	void ( *S_StartSound )( vec3_t origin, int entNum, int entChannel, sfxHandle_t sfxHandle, float volume, float minDist, float pitch, float maxDist, int streamed );
	void ( *S_StartLocalSound )( char *soundName, int channel );
	void ( *S_StopSound )( int entNum, int channel );
	void ( *S_ClearLoopingSounds )( );
	void ( *S_AddLoopingSound )( vec3_t origin, vec3_t velocity, sfxHandle_t sfxHandle, float volume, float minDist, float maxDist, float pitch, int flags );
	void ( *S_Respatialize )( int entityNum, vec3_t head, vec3_t axis[3] );
	void ( *S_BeginRegistration )( );
	sfxHandle_t ( *S_RegisterSound )( char *name, int streamed, qboolean forceLoad );
	void ( *S_EndRegistration )( );
	void ( *S_UpdateEntity )( int entityNum, vec3_t origin, vec3_t velocity, qboolean useListener );
	void ( *S_SetReverb )( int reverbType, float reverbLevel );
	void ( *S_SetGlobalAmbientVolumeLevel )( float volume );
	float ( *S_GetSoundTime )( sfxHandle_t handle );
	int ( *S_ChannelNameToNum )( char *name );
	char * ( *S_ChannelNumToName )( int channel );
	int ( *S_IsSoundPlaying )( int channelNumber, char *name );
	void ( *MUSIC_NewSoundtrack )( char *name );
	void( *MUSIC_UpdateMood )( music_mood_t current, music_mood_t fallback );
	void ( *MUSIC_UpdateVolume )( float volume, float fadeTime );
	float * ( *get_camera_offset )( qboolean *lookActive, qboolean *resetView );
	void ( *R_ClearScene )( );
	void ( *R_RenderScene )( refDef_t *fd );
	void ( *R_LoadWorldMap )( char *name );
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
	void ( *R_AddRefEntityToScene )( refEntity_t *ent, int parentEntityNumber );
	void ( *R_AddRefSpriteToScene )( refEntity_t *ent );
	void ( *R_AddLightToScene )( vec3_t org, float intensity, float r, float g, float b, dlightType_t type );
	qboolean ( *R_AddPolyToScene )( qhandle_t hShader, int numVerts, polyVert_t *verts, int renderFx );
	void ( *R_AddTerrainMarkToScene )( int terrainIndex, qhandle_t hShader, int numVerts, polyVert_t *verts, int renderFx );
	void ( *R_SetColor )( vec4_t rgba );
	void ( *R_DrawStretchPic )( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
	fontHeader_t * ( *R_LoadFont )( const char *name );
	void ( *R_DrawString )( fontHeader_t *font, const char *text, float x, float y, int maxLen, qboolean virtualScreen );
	refEntity_t * ( *R_GetRenderEntity )( int entityNumber );
	void ( *R_ModelBounds )( clipHandle_t model, vec3_t mins, vec3_t maxs );
	float ( *R_ModelRadius )( qhandle_t handle );
	float ( *R_Noise )( float x, float y, float z, float t );
	void ( *R_DebugLine )( vec3_t start, vec3_t end, float r, float g, float b, float alpha );
	baseShader_t * ( *GetShader )( int shaderNum );
	void ( *R_SwipeBegin )( float thistime, float life, qhandle_t shader );
	void ( *R_SwipePoint )( vec3_t point1, vec3_t point2, float time );
	void ( *R_SwipeEnd )( );
	int ( *R_GetShaderWidth )( qhandle_t hShader );
	int ( *R_GetShaderHeight )( qhandle_t hShader );
	void ( *R_DrawBox )( float x, float y, float w, float h );
	void ( *GetGameState )( gameState_t *gameState );
	int ( *GetSnapshot )( int snapshotNumber, snapshot_t *snapshot );
	int ( *GetServerStartTime )( );
	void ( *SetTime )( float time );
	void ( *GetCurrentSnapshotNumber )( int *snapshotNumber, int *serverTime );
	void ( *GetGlconfig )( glconfig_t *glConfig );
	qboolean ( *GetParseEntityState )( int parseEntityNumber, entityState_t *state );
	int ( *GetCurrentCmdNumber )( );
	qboolean ( *GetUserCmd )( int cmdNumber, usercmd_t *userCmd );
	qboolean ( *GetServerCommand )( int serverCommandNumber, qboolean differentServer );
	qboolean ( *Alias_Add )( char *alias, char *name, char *parameters );
	qboolean ( *Alias_ListAdd )( AliasList_t *list, const char *alias, const char *name, const char *parameters );
	char * ( *Alias_FindRandom )( const char *alias, AliasListNode_t **ret );
	char * ( *Alias_ListFindRandom )( AliasList_t *list, const char *alias, AliasListNode_t **ret );
	void ( *Alias_Dump )( );
	void ( *Alias_Clear )( );
	AliasList_t * ( *AliasList_New )( char *name );
	void ( *Alias_ListFindRandomRange )( AliasList_t *list, char *alias, int *minIndex, int *maxIndex, float *totalWeight );
	AliasList_t * ( *Alias_GetGlobalList )( );
	void ( *UI_ShowMenu )( char *name, qboolean bForce );
	void ( *UI_HideMenu )( char *name, qboolean bForce );
	int ( *UI_FontStringWidth )( fontHeader_t *font, char *string, int maxLen );
	int ( *Key_StringToKeynum )( char *str );
	char * ( *Key_KeynumToBindString )( int keyNum );
	void ( *Key_GetKeysForCommand )( const char *command, int *key1, int *key2 );
	dtiki_t * ( *R_Model_GetHandle )( qhandle_t handle );
	int ( *TIKI_NumAnims )( dtiki_t *pmdl );
	void ( *TIKI_CalculateBounds )( dtiki_t *pmdl, float scale, vec3_t mins, vec3_t maxs );
	char * ( *TIKI_Name )( dtiki_t *tiki, int entNum );
	void * ( *TIKI_GetSkeletor )( dtiki_t *tiki, int entNum );
	void ( *TIKI_SetEyeTargetPos )( dtiki_t *tiki, int entNum, vec3_t pos );
	char * ( *Anim_NameForNum )( dtiki_t *tiki, int animNum );
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
	char * ( *Tag_NameForNum )( dtiki_t *pmdl, int animNum );
	void ( *ForceUpdatePose )( refEntity_t *model );
	orientation_t ( *TIKI_Orientation )( orientation_t *orientation, refEntity_t *model, int tagNum );
	qboolean ( *TIKI_IsOnGround )( refEntity_t *model, int tagNum, float threshold );
	void ( *UI_ShowScoreBoard )( const char *menuName );
	void ( *UI_HideScoreBoard )( );
	void ( *UI_SetScoreBoardItem )( int itemNumber, char *data1, char *data2, char *data3, char *data4, char *data5, char *data6, char *data7, char *data8, float *textColor, float *backColor, qboolean isHeader );
	void ( *UI_DeleteScoreBoardItems )( int maxIndex );
	void ( *UI_ToggleDMMessageConsole )( int consoleMode );
	dtiki_t * ( *TIKI_FindTiki )( char *path );
	void ( *LoadResource )( char *name );
	void ( *FS_CanonicalFilename )( char *name );

	cvar_t *fsDebug;
	hdElement_t *hudDrawElements;
	clientAnim_t *anim;
	stopWatch_t *stopWatch;

	int( *FS_Read )( void *buffer, int len, fileHandle_t fileHandle );
	void( *FS_FCloseFile )( fileHandle_t fileHandle );
	int( *FS_Tell )( fileHandle_t fileHandle );
	int( *FS_Seek )( fileHandle_t fileHandle, long int offset, fsOrigin_t origin );
	fileHandle_t( *FS_FOpenFileWrite )( const char *fileName );
	int( *FS_Write )( void *buffer, int size, fileHandle_t fileHandle );
	char **( *FS_ListFiles )( const char *qpath, const char *extension, qboolean wantSubs, int *numFiles );
	void( *FS_FreeFileList )( char **list );

} clientGameImport_t;

typedef struct clientGameExport_s
{
	void ( *CG_Init )( clientGameImport_t *imported, int serverMessageNum, int serverCommandSequence, int clientNum );
	void ( *CG_Shutdown )( );
	void ( *CG_DrawActiveFrame )( int serverTime, int frameTime, stereoFrame_t stereoView, qboolean demoPlayback );
	qboolean ( *CG_ConsoleCommand )( );
	void ( *CG_GetRendererConfig )( );
	void ( *CG_Draw2D )( );
	void ( *CG_EyePosition )( vec3_t *eyePos );
	void ( *CG_EyeOffset )( vec3_t *eyeOffset );
	void ( *CG_EyeAngles )( vec3_t *eyeAngles );
	float ( *CG_SensitivityScale )( );
	void ( *CG_ParseCGMessage )( );
	void ( *CG_RefreshHudDrawElements )( );
	void ( *CG_HudDrawShader )( int info );
	void ( *CG_HudDrawFont )( int info );
	int ( *CG_GetParent )( int entNum );
	float ( *CG_GetObjectiveAlpha )( );
	int ( *CG_PermanentMark )( vec3_t origin, vec3_t dir, float orientation, float sScale, float tScale, float red, float green, float blue, float alpha, qboolean doLighting, float sCenter, float tCenter, markFragment_t *markFragments, void *polyVerts );
	int ( *CG_PermanentTreadMarkDecal )( treadMark_t *treadMark, qboolean startSegment, qboolean doLighting, markFragment_t *markFragments, void *polyVerts );
	int ( *CG_PermanentUpdateTreadMark )( treadMark_t *treadMark, float alpha, float minSegment, float maxSegment, float maxOffset, float texScale );
	void ( *CG_ProcessInitCommands )( dtiki_t *tiki, refEntity_t *ent );
	void ( *CG_EndTiki )( dtiki_t *tiki );
	char * ( *CG_GetColumnName )( int columnNum, int *columnWidth );
	void ( *CG_GetScoreBoardColor )( float *red, float *green, float *blue, float *alpha );
	void ( *CG_GetScoreBoardFontColor )( float *red, float *green, float *blue, float *alpha );
	int ( *CG_GetScoreBoardDrawHeader )( );
	void ( *CG_GetScoreBoardPosition )( float *x, float *y, float *width, float *height );
	int ( *CG_WeaponCommandButtonBits )( );
	int ( *CG_CheckCaptureKey )( keyNum_t key, qboolean down, unsigned int time );

	profCGame_t *profStruct;

	qboolean ( *CG_Command_ProcessFile )( char *name, qboolean quiet, dtiki_t *curTiki );

} clientGameExport_t;

clientGameExport_t * GetCGameAPI( );

extern cg_t					*cg;
extern cgs_t				*cgs;
extern centity_t			*cg_entities;
extern centity_t			*cent;
extern float				*viewAngles;

extern int cg_frametime;

extern clientGameExport_t	cge;
extern clientGameImport_t	cgi;

typedef char* (__cdecl * CG_CONFIGSTRING)(int ID);
extern CG_CONFIGSTRING CG_ConfigString;

typedef char* (__cdecl *SPRINTF2)(char * Format, ...);
extern SPRINTF2 sprintf2;

typedef int (__cdecl *Q_STRCMPI)(char * Str1, char * Str2);
extern Q_STRCMPI Q_strcmpi;

typedef void ( __cdecl *pfnCG_CastFootShadow )( vec3_t lightPos, vec3_t lightIntensity, int tagNum, refEntity_t *ref );
extern pfnCG_CastFootShadow CG_CastFootShadow;

#endif /* __CGAMEX86_H */
