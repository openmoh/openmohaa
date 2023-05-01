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

// DESCRIPTION:
// client game public interfaces

#ifndef __CG_PUBLIC_H__
#define __CG_PUBLIC_H__

#ifdef __cplusplus
extern "C"
   {
#endif

#define  MAGIC_UNUSED_NUMBER 7777
#define    CMD_BACKUP            64    
#define    CMD_MASK            (CMD_BACKUP - 1)
// allow a lot of command backups for very fast systems
// multiple commands may be combined into a single packet, so this
// needs to be larger than PACKET_BACKUP


#define    MAX_ENTITIES_IN_SNAPSHOT    256

// snapshots are a view of the server at a given time

// Snapshots are generated at regular time intervals by the server,
// but they may not be sent if a client's rate level is exceeded, or
// they may be dropped by the network.
typedef struct {
    int                snapFlags;            // SNAPFLAG_RATE_DELAYED, etc
    int                ping;

    int                serverTime;        // server time the message is valid for (in msec)

    byte                areamask[MAX_MAP_AREA_BYTES];        // portalarea visibility bits

    playerState_t    ps;                        // complete information about the current player at this time

    int                numEntities;            // all of the entities that need to be presented
    entityState_t    entities[MAX_ENTITIES_IN_SNAPSHOT];    // at the time of this snapshot

    int                numServerCommands;        // text based server commands to execute when this
    int                serverCommandSequence;    // snapshot becomes current

    int                number_of_sounds;
    server_sound_t    sounds[ MAX_SERVER_SOUNDS ];
} snapshot_t;

typedef struct stopWatch_s
{
    int startTime;
    int endTime;

} stopWatch_t;

typedef struct AliasList_s AliasList_t;
typedef struct AliasListNode_s AliasListNode_t;
typedef struct dtiki_s dtiki_t;
typedef struct tiki_cmd_s tiki_cmd_t;

/*
==================================================================

functions imported from the main executable

==================================================================
*/

#define    CGAME_IMPORT_API_VERSION    3

/*
==================================================================

functions exported to the main executable

==================================================================
*/

typedef struct 
   {
    int            apiversion;
    
    //============== general services ==================

    // print message on the local console
    void             (*Printf)( const char *fmt, ...);
    void             (*DPrintf)( const char *fmt, ...);
    void             (*DebugPrintf)( const char *fmt, ...);

    // managed memory allocation
    void             *(*Malloc)( int size );
    void             (*Free)( void *block );

    // abort the game
    void             (*Error)( int errorLevel, const char *fmt, ...);

    // milliseconds should only be used for profiling, never
    // for anything game related.  Get time from CG_ReadyToBuildScene.
    int              (*Milliseconds)( void );

    // localization
    const char* (*LV_ConvertString)(const char* string);

    // console variable interaction
    cvar_t *       (*Cvar_Get)( const char *var_name, const char *value, int flags );
    void           (*Cvar_Set)( const char *var_name, const char *value );

    // ClientCommand and ConsoleCommand parameter access
    int              (*Argc)( void );
    char *         (*Argv)( int n );
    char *         (*Args)( void );    // concatenation of all argv >= 1
   void           (*AddCommand)( const char *cmd );
    void          (*Cmd_Stuff)(const char* text);
    void          (*Cmd_Execute)(cbufExec_t execWhen, const char* text);
    void          (*Cmd_TokenizeString)(const char* textIn);

    // a -1 return means the file does not exist
    // NULL can be passed for buf to just determine existance
    int              (*FS_ReadFile)( const char *name, void **buf );
    void             (*FS_FreeFile)( void *buf );
    void             (*FS_WriteFile)( const char *qpath, const void *buffer, int size );
   void           (*FS_WriteTextFile)( const char *qpath, const void *buffer, int size );
    // add commands to the local console as if they were typed in
    // for map changing, etc.  The command is not executed immediately,
    // but will be executed in order the next time console commands
    // are processed
    void             (*SendConsoleCommand)( const char *text );

    // =========== client specific functions ===============

    // send a string to the server over the network
   int          (*MSG_ReadBits)(int bits);
   int          (*MSG_ReadChar)();
   int          (*MSG_ReadByte)();
   int          (*MSG_ReadSVC)();
   int          (*MSG_ReadShort)();
   int          (*MSG_ReadLong)();
   float        (*MSG_ReadFloat)();
   char*        (*MSG_ReadString)();
   char*        (*MSG_ReadStringLine)();
   float        (*MSG_ReadAngle8)();
   float        (*MSG_ReadAngle16)();
   void         (*MSG_ReadData)(void* data, int len);
   float        (*MSG_ReadCoord)();
   void         (*MSG_ReadDir)(vec3_t dir);
    void        (*SendClientCommand)( const char *s );

   // CM functions
    void           (*CM_LoadMap)( const char *name );
    clipHandle_t   (*CM_InlineModel)( int index );        // 0 = world, 1+ = bmodels
    int               (*CM_NumInlineModels)( void );
    int               (*CM_PointContents)( const vec3_t p, int headnode );
    int               (*CM_TransformedPointContents)( const vec3_t p, int headnode, vec3_t origin, vec3_t angles );
    void            (*CM_BoxTrace)( trace_t *results, const vec3_t start, const vec3_t end,
                                          const vec3_t mins, const vec3_t maxs,
                                          int headnode, int brushmask, qboolean cylinder );
   void             (*CM_TransformedBoxTrace)( trace_t *results, const vec3_t start, const vec3_t end,
                                                    const vec3_t mins, const vec3_t maxs,
                                                    int headnode, int brushmask,
                                                   const vec3_t origin, const vec3_t angles, 
                                             qboolean cylinder
                                           );
   clipHandle_t   (*CM_TempBoxModel)( const vec3_t mins, const vec3_t maxs, int contents );
   void         (*CM_PrintBSPFileSizes)();
   qboolean     (*CM_LeafInPVS)(int leaf1, int leaf2);
   int          (*CM_PointLeafnum)(vec3_t p);
   int          (*R_MarkFragments)( int numPoints, const vec3_t *points, const vec3_t projection, 
                                       int maxPoints, vec3_t pointBuffer,
                                       int maxFragments, markFragment_t *fragmentBuffer, float fRadiusSquared );
   int          (*R_MarkFragmentsForInlineModel)(clipHandle_t bmodel, vec3_t vAngles, vec3_t vOrigin,
                                        int numPoints, const vec3_t *points, const vec3_t projection, 
                                        int maxPoints, vec3_t pointBuffer,
                                        int maxFragments, markFragment_t *fragmentBuffer, float fRadiusSquared );

   void     (*R_GetInlineModelBounds)(int index, vec3_t mins, vec3_t maxs);
   void     (*R_GetLightingForDecal)(vec3_t light, vec3_t facing, vec3_t origin);
   void     (*R_GetLightingForSmoke)(vec3_t light, vec3_t origin);
   int      (*R_GatherLightSources)(vec3_t pos, vec3_t* lightPos, vec3_t* lightIntensity, int maxLights);

    // =========== sound function calls ===============

    void             (*S_StartSound)(vec3_t origin, int entNum, int entChannel, sfxHandle_t sfxHandle, float volume, float minDist, float pitch, float maxDist, int streamed);
    void             (*S_StartLocalSound)(char* soundName, qboolean forceLoad);
    void             (*S_StopSound)( int entnum, int channel );
    void             (*S_ClearLoopingSounds)(void);
    void             (*S_AddLoopingSound)(vec3_t origin, vec3_t velocity, sfxHandle_t sfxHandle, float volume, float minDist, float maxDist, float pitch, int flags);
    void             (*S_Respatialize)( int entityNum, vec3_t origin, vec3_t axis[3] );
    void             (*S_BeginRegistration)( void );
    sfxHandle_t        (*S_RegisterSound)( const char *sample, int streamed );
    void             (*S_EndRegistration)( void );
    void                (*S_UpdateEntity)( int entityNum, const vec3_t origin, const vec3_t velocity, qboolean use_listener );
    void                (*S_SetReverb)( int reverb_type, float reverb_level );
   void           (*S_SetGlobalAmbientVolumeLevel)( float volume );
   float        (*S_GetSoundTime)(sfxHandle_t handle);
   int          (*S_ChannelNameToNum)(char* name);
   char*        (*S_ChannelNumToName)(int channel);
   int          (*S_IsSoundPlaying)(int channelNumber, char* name);

    // =========== music function calls ===============

    void            (*MUSIC_NewSoundtrack)( const char *name );
    void            (*MUSIC_UpdateMood)( int current_mood, int fallback_mood );
    void            (*MUSIC_UpdateVolume)( float volume, float fade_time );

    // =========== camera function calls ===============

    float *          (*get_camera_offset)( qboolean *lookactive, qboolean *resetview );

    // =========== renderer function calls ================
   void           (*R_ClearScene)( void );
   void           (*R_RenderScene)( const refdef_t *fd );
   void             (*R_LoadWorldMap)(const char* mapname);
   void             (*R_PrintBSPFileSizes)();
   int              (*MapVersion)();
   int              (*R_MapVersion)();
   qhandle_t    (*R_RegisterModel)( const char *name );
   qhandle_t    (*R_SpawnEffectModel)(const char* name, vec3_t pos, vec3_t axis[3]);
   qhandle_t    (*R_RegisterServerModel)(const char* name);
   void         (*R_UnregisterServerModel)(qhandle_t hModel);
    qhandle_t      (*R_RegisterSkin)( const char *name );
    qhandle_t      (*R_RegisterShader)( const char *name );
    qhandle_t      (*R_RegisterShaderNoMip)( const char *name );
   void           (*R_AddRefEntityToScene)( refEntity_t *ent );
   void           (*R_AddRefSpriteToScene)( refEntity_t *ent );
   void           (*R_AddLightToScene)( vec3_t origin, float intensity, float r, float g, float b, int type );
   void             (*R_AddPolyToScene)(qhandle_t hShader, int numVerts, const polyVert_t* verts, int renderfx);
   void             (*R_AddTerrainMarkToScene)(int terrainIndex, qhandle_t hShader, int numVerts, polyVert_t* verts, int renderFx);
    void             (*R_SetColor)( const vec4_t rgba );    // NULL = 1,1,1,1
    void             (*R_DrawStretchPic) ( float x, float y, float w, float h, 
                                          float s1, float t1, float s2, float t2, 
                                        qhandle_t hShader );    // 0 = white
    fontheader_t* (*R_LoadFont)(const char* name);
    void            (*R_DrawString)(fontheader_t* font, char* text, float x, float y, int maxLen, qboolean virtualScreen);
   refEntity_t *  (*R_GetRenderEntity)( int entityNumber );
   void              (*R_ModelBounds)( clipHandle_t model, vec3_t mins, vec3_t maxs );
   float           (*R_ModelRadius)( clipHandle_t model );
   float          (*R_Noise)( float x, float y, float z, float t );
   void           (*R_DebugLine)(vec3_t start, vec3_t end, float r, float g, float b, float alpha);
   baseshader_t* (*GetShader)(int shaderNum);
   // =========== Swipes =============
   void           (*R_SwipeBegin) ( float thistime, float life, qhandle_t shader );
   void           (*R_SwipePoint) ( vec3_t p1, vec3_t p2, float time );
   void           (*R_SwipeEnd) ( void );
   int            (*R_GetShaderWidth)( qhandle_t shader );
   int            (*R_GetShaderHeight)( qhandle_t shader );
   void           (*R_DrawBox)( float x, float y, float w, float h );

    // =========== data shared with the client =============
    void             (*GetGameState)( gameState_t *gamestate );
    int             (*GetSnapshot)( int snapshotNumber, snapshot_t *snapshot );
    int             (*GetServerStartTime)();
    void            (*SetTime)(float time);
   void              (*GetCurrentSnapshotNumber)( int *snapshotNumber, int *serverTime );
   void              (*GetGlconfig)( glconfig_t *glconfig );

    // will return false if the number is so old that it doesn't exist in the buffer anymore
    qboolean          (*GetParseEntityState)( int parseEntityNumber, entityState_t *state );
    int              (*GetCurrentCmdNumber)( void );        // returns the most recent command number
                                                                  // which is the local predicted command for
                                                                  // the following frame
    qboolean          (*GetUserCmd)( int cmdNumber, usercmd_t *ucmd );
   qboolean          (*GetServerCommand)( int serverCommandNumber );

   // ALIAS STUFF
   qboolean            (*Alias_Add)( const char * alias, const char * name, const char *parameters );
   qboolean         (*Alias_ListAdd)(AliasList_t* list, const char* alias, const char* name, const char* parameters);
   const char*      (*Alias_FindRandom)(const char* alias, AliasListNode_t **ret);
   char*            (*Alias_ListFindRandom)(AliasList_t* list, const char* alias, AliasListNode_t** ret);
   void                (*Alias_Dump)( void );
   void                (*Alias_Clear)( void );
   AliasList_t*     (*AliasList_New)(char* name);
   void             (*Alias_ListFindRandomRange)(AliasList_t* list, char* alias, int* minIndex, int* maxIndex, float* totalWeight);
   AliasList_t*     (*Alias_GetGlobalList)();

   // ==================== UI STUFF ==========================
   void             (*UI_ShowMenu)(char* name, qboolean bForce);
   void             (*UI_HideMenu)(char* name, qboolean bForce);
   int              (*UI_FontStringWidth)(fontheader_t* font, char* string, int maxLen);

   int              (*Key_StringToKeynum)(char* str);
   char*            (*Key_KeynumToBindString)(int keyNum);
   void             (*Key_GetKeysForCommand)(char* command, int* key1, int* key2);

   // ==================== TIKI STUFF ==========================
   // TIKI SPECIFIC STUFF
   dtiki_t*         (*R_Model_GetHandle)(qhandle_t handle);
   int              (*TIKI_NumAnims)(dtiki_t* pmdl);
   void             (*TIKI_CalculateBounds)(dtiki_t* pmdl, float scale, vec3_t mins, vec3_t maxs);
   char*            (*TIKI_Name)(dtiki_t* tiki);
   void*            (*TIKI_GetSkeletor)(dtiki_t* tiki, int entNum);
   void             (*TIKI_SetEyeTargetPos)(dtiki_t* tiki, int entNum, vec3_t pos);

   // ANIM SPECIFIC STUFF
   const char *   (*Anim_NameForNum) (dtiki_t* tiki, int animnum );
   int                (*Anim_NumForName) (dtiki_t* tiki, const char * name );
   int                (*Anim_Random) (dtiki_t* tiki, const char * name );
   int                (*Anim_NumFrames) (dtiki_t* tiki, int animnum );
   float                (*Anim_Time) (dtiki_t* tiki, int animnum);
   float                (*Anim_Frametime) (dtiki_t* tiki, int animnum);
   void                (*Anim_Delta) (dtiki_t* tiki, int animnum, vec3_t delta );
   int                (*Anim_Flags) (dtiki_t* tiki, int animnum);
   int                (*Anim_FlagsSkel) (dtiki_t* tiki, int animnum);
   int                (*Anim_CrossblendTime) (dtiki_t* tiki, int animnum );
   qboolean            (*Anim_HasCommands) (dtiki_t* tiki, int animnum );

   // FRAME SPECIFIC STUFF
   qboolean       (*Frame_Commands) (dtiki_t* tiki, int animnum, int framenum, tiki_cmd_t * tiki_cmd );
   qboolean       (*Frame_CommandsTime) (dtiki_t* tiki, int animnum, float start, float end, tiki_cmd_t * tiki_cmd );

   // SURFACE SPECIFIC STUFF
   int                (*Surface_NameToNum) (dtiki_t* tiki, const char * name );
   //const char *   (*Surface_NumToName) (dtiki_t* tiki, int num );
   //int                (*Surface_Flags) (dtiki_t* tiki, int num );
   //int                (*Surface_NumSkins) (dtiki_t* tiki, int num );

   // TAG SPECIFIC STUFF
   int                (*Tag_NumForName) (dtiki_t* tiki, const char * name );
   const char *   (*Tag_NameForNum) (dtiki_t* tiki, int num );
   void (*ForceUpdatePose)(refEntity_t* model);
   orientation_t (*TIKI_Orientation)(refEntity_t* model, int tagNum);
   qboolean (*TIKI_IsOnGround)(refEntity_t* model, int tagNum, float threshold);

   // MISCELLANEOUS SPECIFIC STUFF
   void (*UI_ShowScoreBoard)(const char* menuName);
   void (*UI_HideScoreBoard)();
   void (*UI_SetScoreBoardItem)(int itemNumber, const char* data1,
                                const char* data2, const char* data3,
                                const char* data4, const char* data5,
                                const char* data6, const char* data7,
                                const char* data8, const float* textColor,
                                const float* backColor, qboolean isHeader);
   void (*UI_DeleteScoreBoardItems)(int maxIndex);
   void (*UI_ToggleDMMessageConsole)(int consoleMode);
   dtiki_t* (*TIKI_FindTiki)(const char* path);
   void (*LoadResource)(const char* name);
   void (*FS_CanonicalFilename)(const char* name);

   cvar_t* fsDebug;
   hdelement_t* HudDrawElements;
   clientAnim_t* anim;
   stopWatch_t* stopWatch;

} clientGameImport_t;


/*
==================================================================

functions exported to the main executable

==================================================================
*/

typedef struct {
    void     (*CG_Init)( clientGameImport_t *imported, int serverMessageNum, int serverCommandSequence, int clientNum );
   void     (*CG_Shutdown)( void );
   void     (*CG_DrawActiveFrame)( int serverTime, int frameTime, stereoFrame_t stereoView, qboolean demoPlayback );
   qboolean (*CG_ConsoleCommand)( void );
   void     (*CG_GetRendererConfig)( void );
   void     (*CG_Draw2D)( void );
   void     (*CG_EyePosition)(vec3_t* eyePos);
   void     (*CG_EyeOffset)(vec3_t* eyeOffset);
   void     (*CG_EyeAngles)(vec3_t* eyeAngles);
   float    (*CG_SensitivityScale)();
   void     (*CG_ParseCGMessage)();
   void     (*CG_RefreshHudDrawElements)();
   void     (*CG_HudDrawShader)(int info);
   void     (*CG_HudDrawFont)(int info);
   int      (*CG_GetParent)(int entNum);
   float    (*CG_GetObjectiveAlpha)();
   int      (*CG_PermanentMark)(vec3_t origin, vec3_t dir, float orientation, float sScale, float tScale, float red, float green, float blue, float alpha, qboolean doLighting, float sCenter, float tCenter, markFragment_t* markFragments, void* polyVerts);
   int      (*CG_PermanentTreadMarkDecal)(treadMark_t* treadMark, qboolean startSegment, qboolean doLighting, markFragment_t* markFragments, void* polyVerts);
   int      (*CG_PermanentUpdateTreadMark)(treadMark_t* treadMark, float alpha, float minSegment, float maxSegment, float maxOffset, float texScale);
   void     (*CG_ProcessInitCommands)(dtiki_t* tiki, refEntity_t* ent);
   void     (*CG_EndTiki)(dtiki_t* tiki);
   const char*    (*CG_GetColumnName)(int columnNum, int* columnWidth);
   void     (*CG_GetScoreBoardColor)(float* red, float* green, float* blue, float* alpha);
   void     (*CG_GetScoreBoardFontColor)(float* red, float* green, float* blue, float* alpha);
   int      (*CG_GetScoreBoardDrawHeader)();
   void     (*CG_GetScoreBoardPosition)(float* x, float* y, float* width, float* height);
   int      (*CG_WeaponCommandButtonBits)();
   int      (*CG_CheckCaptureKey)(int key, qboolean down, unsigned int time);

   // FIXME
   //prof_cgame_t* profStruct;

   qboolean (*CG_Command_ProcessFile)(char* name, qboolean quiet, dtiki_t* curTiki);

} clientGameExport_t;

#ifdef CGAME_DLL
#ifdef WIN32
__declspec(dllexport)
#else
__attribute__((visibility("default")))
#endif
#endif
clientGameExport_t* GetCGameAPI(void);

#ifdef __cplusplus
}
#endif

#endif // __CG_PUBLIC_H__
