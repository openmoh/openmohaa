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
// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

#pragma once

#include "q_shared.h"
#include "tr_types.h"
#include "../fgame/bg_public.h"
#include "cm_public.h"
#include "cg_public.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        int   anim;
        int   frame;
        int   oldanim;
        int   oldframe;
        int   starttime;
        float framelerp;
    } lerpstate_t;

    typedef struct {
        lerpstate_t base;

        // crossblend variables
        lerpstate_t crossblend;
        int         crossblend_totaltime;
        float       crossblend_lerp;

        // global variables
        int      numframes;
        int      time_per_frame;
        qboolean driven;

        // non-driven variables
        int next_evaluate_time;

        // driven variables
        int    last_driven_time;
        vec3_t last_origin;
        vec3_t frame_delta;      // normalized frame delta
        float  frame_distance;   // float distance till next frame
        float  current_distance; // current distance traveled

        // command variables
        qboolean has_commands;
        int      last_cmd_frame;
        int      last_cmd_anim;
        int      last_cmd_time;
    } animstate_t;

    //=================================================

#define CF_UPDATESWIPE        0x0000001
#define CF_COMMANDS_PROCESSED 0x0000002

    // centity_t have a direct corespondence with gentity_t in the game, but
    // not all of the gentity_t will be communicated to the client
    typedef struct centity_s {
        entityState_t currentState; // from cg.frame
        entityState_t nextState;    // from cg.nextFrame, if available
        qboolean      teleported;   // true if the entity was just teleported
        qboolean      interpolate;  // true if next is valid to interpolate to
        qboolean      currentValid; // true if cg.frame holds this entity

        int miscTime;
        int snapShotTime;

        int    errorTime; // decay the error from this time
        vec3_t errorOrigin;
        vec3_t errorAngles;

        qboolean extrapolated; // false if origin / angles is an interpolation
        vec3_t   rawOrigin;
        vec3_t   rawAngles;

        vec3_t beamEnd;

        // exact interpolated position of entity on this frame
        vec3_t lerpOrigin;
        vec3_t lerpAngles;

        sfxHandle_t tikiLoopSound;
        float       tikiLoopSoundVolume;
        float       tikiLoopSoundMinDist;
        float       tikiLoopSoundMaxDist;
        float       tikiLoopSoundPitch;
        int         tikiLoopSoundFlags;

        float color[4];
        float client_color[4]; // the color set by client commands
        int   clientFlags;

        int splash_last_spawn_time;
        int splash_still_count;

        quat_t   bone_quat[NUM_BONE_CONTROLLERS];
        float    animLastTimes[MAX_FRAMEINFOS];
        int      animLast[MAX_FRAMEINFOS];
        int      animLastWeight;
        int      usageIndexLast;
        qboolean bFootOnGround_Right;
        qboolean bFootOnGround_Left;
        int      iNextLandTime;
    } centity_t;

    //======================================================================

    // local entities are created as a result of events or predicted actions,
    // and live independantly from all server transmitted entities

#define MAX_VERTS_ON_POLY 8
#define MAX_MARK_POLYS    64
#define MAX_TREAD_MARKS   16

    typedef struct markPoly_s {
        struct markPoly_s* nextPoly;
        int numVerts;
        polyVert_t verts[MAX_VERTS_ON_POLY];
        int iIndex;
    } markPoly_t;

    typedef struct markObj_s {
        struct markObj_s* prevMark;
        struct markObj_s* nextMark;
        int time;
        int lastVisTime;
        int leafnum;
        vec3_t pos;
        float radius;
        int markShader;
        int alphaFade;
        int fadein;
        markPoly_t* markPolys;
    } markObj_t;

#define MAX_CUSTOM_SOUNDS 32

    typedef struct {
        int  team;
        char name[64];
    } clientInfo_t;

    typedef struct cobjective_s {
        char text[MAX_STRING_CHARS];
        int  flags;
    } cobjective_t;

#define MAX_RAIN_SHADERS 16

    typedef struct crain_s {
        float density;
        float speed;
        int   speed_vary;
        int   slant;
        float length;
        float min_dist;
        float width;
        char  shader[MAX_RAIN_SHADERS][MAX_STRING_CHARS];
        int   numshaders;
    } crain_t;

    //======================================================================

    // all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
    // occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

    typedef struct {
        int clientFrame; // incremented each frame
        int clientNum;

        qboolean demoPlayback;
        qboolean levelShot; // taking a level menu screenshot

        // there are only one or two snapshot_t that are relevent at a time
        int         latestSnapshotNum;  // the number of snapshots the client system has received
        int         latestSnapshotTime; // the time from latestSnapshotNum, so we don't need to read the snapshot yet
        snapshot_t *snap;               // cg.snap->serverTime <= cg.time
        snapshot_t *nextSnap;           // cg.nextSnap->serverTime > cg.time, or NULL
        snapshot_t  activeSnapshots[2];

        float
            frameInterpolation; // (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

        qboolean thisFrameTeleport;
        qboolean nextFrameTeleport;
        qboolean nextFrameCameraCut;

        int frametime; // cg.time - cg.oldTime

        int time; // this is the time value that the client
        // is rendering at.

        int physicsTime; // either cg.snap->time or cg.nextSnap->time

        qboolean renderingThirdPerson; // during deaths, chasecams, etc

        // prediction state
        qboolean      hyperspace; // true if prediction has hit a trigger_teleport
        playerState_t predicted_player_state;
        qboolean      validPPS; // clear until the first call to CG_PredictPlayerState
        int           predictedErrorTime;
        vec3_t        predictedError;

        // input state sent to server
        int iWeaponCommand;
        int iWeaponCommandSend;

        // auto rotating items
        vec3_t autoAngles;
        vec3_t autoAxis[3];
        vec3_t autoAnglesSlow;
        vec3_t autoAxisSlow[3];
        vec3_t autoAnglesFast;
        vec3_t autoAxisFast[3];

        // view rendering
        refdef_t refdef;
        vec3_t   playerHeadPos;     // position of the players head
        vec3_t   refdefViewAngles;  // will be converted to refdef.viewaxis
        vec3_t   currentViewPos;    // current position of the camera
        vec3_t   currentViewAngles; // current angles of the camera

        float    fRefFovXSin;
        float    fRefFovXCos;
        float    fRefFovYSin;
        float    fRefFovYCos;
        float    fCurrentViewHeight;
        float    fCurrentViewBobPhase;
        float    fCurrentViewBobAmp;

        // player model
        dtiki_t  *pLastPlayerWorldModel;
        dtiki_t  *pPlayerFPSModel;
        qhandle_t hPlayerFPSModelHandle;
        qboolean  bFPSModelLastFrame;
        qboolean  bFPSOnGround;
        dtiki_t  *pAlliedPlayerModel;
        qhandle_t hAlliedPlayerModelHandle;
        dtiki_t  *pAxisPlayerModel;
        qhandle_t hAxisPlayerModelHandle;

        // view eyes
        vec3_t vOffsetViewAngles;
        vec3_t vLastHeadAngles;
        vec3_t vLastViewAngles;
        vec3_t vEyeOffsetMax;
        float  fEyeOffsetFrac;

        float  SoundOrg[3];  // position from where sound should be played
        vec3_t SoundAxis[3]; // axis from where sound should be played

        vec3_t camera_origin; // lerped camera_origin
        vec3_t camera_angles; // lerped camera_angles
        float  camera_fov;    // lerped camera_fov

        // zoom key
        float zoomSensitivity;

        // information screen text during loading
        qboolean bIntermissionDisplay;

        // scoreboard
        int      scoresRequestTime;
        qboolean showScores;
        char     scoresMenuName[MAX_STRING_TOKENS];
        int      iInstaMessageMenu;

        // centerprinting
        int  centerPrintTime;
        int  centerPrintCharWidth;
        int  centerPrintY;
        char centerPrint[1024];
        int  centerPrintLines;

        // gameplay
        int matchStartTime;

        // development tool
        refEntity_t testModelEntity;
        char        testModelName[MAX_QPATH];
        qboolean    testGun;

        // farplane parameters
        float    farplane_distance;
        float    farplane_bias;
        vec3_t   farplane_color;
        qboolean farplane_cull;
        float    skyboxFarplane;
        qboolean renderTerrain;
        float    farclipOverride;
        vec3_t   farplaneColorOverride;

        // portal sky parameters
        qboolean sky_portal;
        float    sky_alpha;
        vec3_t   sky_origin;
        vec3_t   sky_axis[3]; // rotation vectors
        float    skyboxSpeed;

        // weapon viewkick recoil
        float viewkick[2];
        float viewkickRecenter;
        float viewkickMinDecay;
        float viewkickMaxDecay;

        // objectives
        cobjective_t Objectives[20];
        float        ObjectivesAlphaTime;
        float        ObjectivesBaseAlpha;
        float        ObjectivesDesiredAlpha;
        float        ObjectivesCurrentAlpha;

        // misc
        crain_t      rain;
        clientInfo_t clientinfo[MAX_CLIENTS];
    } cg_t;

    typedef struct {
        qhandle_t     backTileShader;
        qhandle_t     lagometerShader;
        qhandle_t     shadowMarkShader;
        qhandle_t     footShadowMarkShader;
        qhandle_t     wakeMarkShader;
        qhandle_t     pausedShader;
        qhandle_t     levelExitShader;
        qhandle_t     zoomOverlayShader;
        qhandle_t     kar98TopOverlayShader;
        qhandle_t     kar98BottomOverlayShader;
        qhandle_t     binocularsOverlayShader;
        fontheader_t *hudDrawFont;
        fontheader_t *attackerFont;
        fontheader_t *objectiveFont;
        qhandle_t     objectivesBackShader;
        qhandle_t     checkedBoxShader;
        qhandle_t     uncheckedBoxShader;
    } media_t;

    // The client game static (cgs) structure hold everything
    // loaded or calculated from the gamestate.  It will NOT
    // be cleared when a tournement restart is done, allowing
    // all clients to begin playing instantly
    typedef struct {
        gameState_t gameState;    // gamestate from server
        glconfig_t  glconfig;     // rendering configuration
        float       screenXScale; // derived from glconfig
        float       screenYScale;
        float       screenXBias;

        int serverCommandSequence; // reliable command stream counter
        int processedSnapshotNum;  // the number of snapshots cgame has requested

        qboolean localServer;    // detected on startup by checking sv_running
        int      levelStartTime; // time that game was started

        // parsed from serverinfo
        int        matchEndTime;
        int        serverLagTime;
        gametype_t gametype;
        int        dmflags;
        int        teamflags;
        int        fraglimit;
        int        timelimit;
        int        maxclients;
        int        cinematic;
        int        mapChecksum;
        char       mapname[MAX_QPATH];

        //
        // locally derived information from gamestate
        //
        qhandle_t   model_draw[MAX_MODELS];
        sfxHandle_t sound_precache[MAX_SOUNDS];
        int         numInlineModels;
        qhandle_t   inlineDrawModel[MAX_MODELS];
        vec3_t      inlineModelMidpoints[MAX_MODELS];

        media_t media;
    } cgs_t;

    //==============================================================================

    extern cvar_t *paused;
    extern cvar_t *developer;

    extern cgs_t              cgs;
    extern cg_t               cg;
    extern clientGameImport_t cgi;
    extern int                cg_protocol;
    extern centity_t          cg_entities[MAX_GENTITIES];
    extern markPoly_t         *cg_markPolys;

    extern cvar_t *cg_animSpeed;
    extern cvar_t *cg_debugAnim;
    extern cvar_t *cg_debugAnimWatch;
    extern cvar_t *cg_errorDecay;
    extern cvar_t *cg_nopredict;
    extern cvar_t *cg_showmiss;
    extern cvar_t *cg_addMarks;
    extern cvar_t *cg_maxMarks;
    extern cvar_t *cg_viewsize;
    extern cvar_t *cg_3rd_person;
    extern cvar_t *cg_drawviewmodel;
    extern cvar_t *cg_synchronousClients;
    extern cvar_t *cg_stereoSeparation;
    extern cvar_t *cg_stats;
    extern cvar_t *cg_lagometer;
    extern cvar_t *r_lerpmodels;
    extern cvar_t *cg_cameraheight;
    extern cvar_t *cg_cameradist;
    extern cvar_t *cg_cameraverticaldisplacement;
    extern cvar_t *cg_camerascale;
    extern cvar_t *cg_shadows;
    extern cvar_t *cg_hidetempmodels;
    extern cvar_t *cg_traceinfo;
    extern cvar_t *cg_debugFootsteps;
    extern cvar_t *cg_smoothClients;
    extern cvar_t *cg_smoothClientsTime;
    extern cvar_t *pmove_fixed;
    extern cvar_t *pmove_msec;
    extern cvar_t *cg_pmove_msec;
    extern cvar_t *dm_playermodel;
    extern cvar_t *dm_playergermanmodel;
    extern cvar_t *cg_forceModel;
    extern cvar_t *cg_animationviewmodel;
    extern cvar_t *cg_hitmessages;
    extern cvar_t *cg_acidtrip;
    extern cvar_t *cg_hud;
    extern cvar_t *cg_huddraw_force;
    extern cvar_t *cg_drawsvlag;
    extern cvar_t *vm_offset_max;
    extern cvar_t *vm_offset_speed;
    extern cvar_t *vm_sway_front;
    extern cvar_t *vm_sway_side;
    extern cvar_t *vm_sway_up;
    extern cvar_t *vm_offset_air_front;
    extern cvar_t *vm_offset_air_side;
    extern cvar_t *vm_offset_air_up;
    extern cvar_t *vm_offset_crouch_front;
    extern cvar_t *vm_offset_crouch_side;
    extern cvar_t *vm_offset_crouch_up;
    extern cvar_t *vm_offset_rocketcrouch_front;
    extern cvar_t *vm_offset_rocketcrouch_side;
    extern cvar_t *vm_offset_rocketcrouch_up;
    extern cvar_t *vm_offset_shotguncrouch_front;
    extern cvar_t *vm_offset_shotguncrouch_side;
    extern cvar_t *vm_offset_shotguncrouch_up;
    extern cvar_t *vm_offset_vel_base;
    extern cvar_t *vm_offset_vel_front;
    extern cvar_t *vm_offset_vel_side;
    extern cvar_t *vm_offset_vel_up;
    extern cvar_t *vm_offset_upvel;
    extern cvar_t *vm_lean_lower;
    extern cvar_t *voiceChat;
    extern cvar_t *cg_shadowscount;
    extern cvar_t *cg_shadowdebug;

    //
    // cg_main.c
    //
    void        CG_ProcessConfigString(int num);
    const char *CG_ConfigString(int index);
    void        CG_AddToTeamChat(const char *str);
    void        CG_NewClientinfo(int clientNum);
    sfxHandle_t CG_CustomSound(int entityNum, const char *soundName);
    int         CG_CrosshairPlayer(void);
    int         CG_LastAttacker(void);
    void        CG_Init(clientGameImport_t *imported, int serverMessageNum, int serverCommandSequence, int clientNum);
    void        CG_Shutdown(void);
    void        CG_ServerRestarted(void);
    void        CG_ParseFogInfo(const char *str);

    //
    // cg_modelanim.cpp
    //
    void CG_ModelAnim(centity_t *cent, qboolean bDoShaderTime);
    void CG_AttachEntity(
        refEntity_t *entity, refEntity_t *parent, dtiki_t *tiki, int tagnum, qboolean use_angles, vec3_t attach_offset
    );

    //
    // cg_commands.cpp
    //
    void     CG_Event(centity_t *cent);
    void     CG_UpdateEntityEmitters(int entnum, refEntity_t *ent, centity_t *cent);
    void     CG_RemoveClientEntity(int number, dtiki_t *tiki, centity_t *cent);
    void     CG_UpdateTestEmitter(void);
    void     CG_InitializeCommandManager(void);
    void     CG_ProcessInitCommands(dtiki_t *tiki, refEntity_t *ent);
    void     CG_ProcessCacheInitCommands(dtiki_t *tiki);
    void     CG_EndTiki(dtiki_t *tiki);
    qboolean CG_Command_ProcessFile(const char *filename, qboolean quiet, dtiki_t *curTiki);
    void     CG_RestartCommandManager();
    void     CG_CleanUpTempModels();
    qboolean CG_ProcessEntityCommands(int frame, int anim, int entnum, refEntity_t *ent, centity_t *cent);
    void     CG_ClientCommands(refEntity_t *ent, centity_t *cent, int slot);
    float    RandomizeRange(float start, float end);

    //
    // cg_tempmodels.cpp
    //
    extern cvar_t *cg_showtempmodels;
    extern cvar_t *cg_detail;
    extern cvar_t *cg_effectdetail;
    extern cvar_t *cg_effect_physicsrate;

    void CG_AddTempModels(void);
    void CG_ResetTempModels(void);

    void CG_Splash(centity_t *cent);

    void CG_EventList_f(void);
    void CG_EventHelp_f(void);
    void CG_DumpEventHelp_f(void);
    void CG_PendingEvents_f(void);
    void CG_ClassList_f(void);
    void CG_ClassTree_f(void);
    void CG_ClassEvents_f(void);
    void CG_DumpClassEvents_f(void);
    void CG_DumpAllClasses_f(void);

    void L_InitEvents(void);
    void L_ShutdownEvents(void);

    //
    // cg_view.c
    //
    void CG_TestModel_f(void);
    void CG_TestGun_f(void);
    void CG_TestModelNextFrame_f(void);
    void CG_TestModelPrevFrame_f(void);
    void CG_TestModelNextSkin_f(void);
    void CG_TestModelPrevSkin_f(void);

    void CG_ZoomDown_f(void);
    void CG_ZoomUp_f(void);

    void  CG_EyePosition(vec3_t *o_vPos);
    void  CG_EyeOffset(vec3_t *o_vOfs);
    void  CG_EyeAngles(vec3_t *o_vAngles);
    float CG_SensitivityScale();
    void  CG_AddLightShow();
    qboolean CG_FrustumCullSphere(const vec3_t vPos, float fRadius);
    void  CG_OffsetFirstPersonView(refEntity_t *pREnt, qboolean bUseWorldPosition);
    void  CG_DrawActiveFrame(int serverTime, int frameTime, stereoFrame_t stereoView, qboolean demoPlayback);

    //
    // cg_viewmodelanim.c
    //
    int  CG_GetVMAnimPrefixIndex();
    void CG_ViewModelAnimation(refEntity_t *pModel);
    void CG_CalcViewModelMovement(float fViewBobPhase, float fViewBobAmp, vec_t *vVelocity, vec_t *vMovement);

    //
    // cg_drawtools.c
    //
    void CG_AdjustFrom640(float *x, float *y, float *w, float *h);
    void CG_TileClear(void);
    void CG_DrawOverlayTopBottom(qhandle_t handleTop, qhandle_t handleBottom, float fAlpha);
    void CG_DrawOverlayMiddle(qhandle_t handle, float fAlpha);
    void CG_DrawOverlayFullScreen(qhandle_t handle, float fAlpha);
    void CG_DrawZoomOverlay();
    void CG_HudDrawShader(int iInfo);
    void CG_HudDrawFont(int iInfo);
    void CG_RefreshHudDrawElements();
    void CG_HudDrawElements();
    void CG_InitializeObjectives();
    void CG_DrawObjectives();
    void CG_Draw2D(void);

    //
    // cg_draw.c
    //
    void CG_AddLagometerFrameInfo(void);
    void CG_AddLagometerSnapshotInfo(snapshot_t *snap);
    void CG_CenterPrint(const char *str, int y, int charWidth);
    void CG_DrawHead(float x, float y, float w, float h, int clientNum, vec3_t headAngles);
    void CG_DrawActive(stereoFrame_t stereoView);
    void CG_DrawFlagModel(float x, float y, float w, float h, int team);

    //
    // cg_predict.c
    //
    void CG_BuildSolidList(void);
    int CG_GetBrushEntitiesInBounds(int iMaxEnts, centity_t** pEntList, const vec3_t vMins, const vec3_t vMaxs);
    int  CG_PointContents(const vec3_t point, int passEntityNum);
    void CG_ClipMoveToEntities(
        const vec3_t start,
        const vec3_t mins,
        const vec3_t maxs,
        const vec3_t end,
        int          skipNumber,
        int          mask,
        trace_t     *tr,
        qboolean     cylinder
    );
    void CG_Trace(
        trace_t     *result,
        const vec3_t start,
        const vec3_t mins,
        const vec3_t maxs,
        const vec3_t end,
        int          skipNumber,
        int          mask,
        qboolean     cylinder,
        qboolean     cliptoentities,
        const char  *description
    );
    void CG_PredictPlayerState(void);

    //
    // cg_ents.c
    //
    void BG_EvaluateTrajectory(const trajectory_t *tr, int atTime, const vec3_t base, vec3_t result);
    void CG_SetEntitySoundPosition(centity_t *cent);
    void CG_AddPacketEntities(void);
    void CG_Beam(centity_t *cent);
    void CG_AdjustPositionForMover(const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out);

    void CG_PositionEntityOnTag(refEntity_t *entity, const refEntity_t *parent, qhandle_t parentModel, char *tagName);
    void
    CG_PositionRotatedEntityOnTag(refEntity_t *entity, const refEntity_t *parent, qhandle_t parentModel, char *tagName);
    void CG_GetOrigin(centity_t *cent, vec3_t origin);
    void CG_EntityEffects(centity_t *cent);

    //
    // cg_marks.c
    //
    void CG_InitMarks(void);
    void CG_AddMarks(void);
    void CG_ImpactMark(
        qhandle_t    markShader,
        const vec3_t origin,
        const vec3_t dir,
        float        orientation,
        float        fSScale,
        float        fTScale,
        float        red,
        float        green,
        float        blue,
        float        alpha,
        qboolean     alphaFade,
        qboolean     temporary,
        qboolean     dolighting,
        qboolean     fadein,
        float        fSCenter,
        float        fTCenter
    );
    void CG_ImpactMarkSimple(
        qhandle_t    markShader,
        const vec3_t origin,
        const vec3_t dir,
        float        orientation,
        float        fRadius,
        float        red,
        float        green,
        float        blue,
        float        alpha,
        qboolean     alphaFade,
        qboolean     temporary,
        qboolean     dolighting,
        qboolean     fadein
    );
    void CG_AssembleFinalMarks(
        vec3_t           *markPoints,
        markFragment_t   *markFragments,
        int              numFragments,
        qboolean         (*PerPolyCallback)(const vec3_t* markPoints, markFragment_t* mf, polyVert_t* verts, void* pCustom),
        int              (*GetLeafCallback)(markFragment_t* mf, void* pCustom),
        void             *pCustom,
        vec3_t           pos,
        float            radius,
        qhandle_t        markShader,
        qboolean         fadein,
        qboolean         alphaFade
    );
    qboolean CG_CheckMakeMarkOnEntity(int iEntIndex);
    void CG_InitTestTreadMark();
    void CG_AddTreadMarks();
    int  CG_PermanentMark(
         const vec3_t    origin,
         const vec3_t    dir,
         float           orientation,
         float           fSScale,
         float           fTScale,
         float           red,
         float           green,
         float           blue,
         float           alpha,
         qboolean        dolighting,
         float           fSCenter,
         float           fTCenter,
         markFragment_t *pMarkFragments,
         void           *pVoidPolyVerts
     );

    int CG_PermanentTreadMarkDecal(
        treadMark_t    *pTread,
        qboolean        bStartSegment,
        qboolean        dolighting,
        markFragment_t *pMarkFragments,
        void           *pVoidPolyVerts
    );

    int CG_PermanentUpdateTreadMark(
        treadMark_t *pTread, float fAlpha, float fMinSegment, float fMaxSegment, float fMaxOffset, float fTexScale
    );

    //
    // cg_snapshot.c
    //
    void CG_ProcessSnapshots(void);

    //
    // cg_consolecmds.c
    //
    qboolean CG_ConsoleCommand(void);
    void     CG_InitConsoleCommands(void);
    void     CG_AddTestModel(void);
    void     CG_Mapinfo_f();
    void     CG_PushMenuTeamSelect_f(void);
    void     CG_PushMenuWeaponSelect_f(void);
    void     CG_UseWeaponClass_f(void);
    void     CG_NextWeapon_f(void);
    void     CG_PrevWeapon_f(void);
    void     CG_UseLastWeapon_f(void);
    void     CG_HolsterWeapon_f(void);
    void     CG_DropWeapon_f(void);
    void     CG_ToggleItem_f(void);
    int      CG_WeaponCommandButtonBits(void);
    void     CG_ScoresDown_f(void);
    void     CG_ScoresUp_f(void);

    //
    // cg_servercmds.c
    //
    void CG_ExecuteNewServerCommands(int latestSequence, qboolean differentServer);
    void CG_ParseServerinfo(void);

    //
    // cg_playerstate.c
    //
    void CG_TransitionPlayerState(playerState_t *ps, playerState_t *ops);

    //
    // cg_player.cpp
    //
    void CG_ResetPlayerEntity(centity_t *cent);
    void CG_Player(centity_t *cent);

    //
    // cg_sound.cpp
    //
    void CG_ProcessSound(server_sound_t *sound);

    //
    // cg_beam.cpp
    //
    void CG_AddBeams(void);
    void CG_MultiBeamBegin(void);
    void CG_MultiBeamAddPoints(
        vec3_t start, vec3_t end, int numsegments, int flags, float minoffset, float maxoffset, qboolean addstartpoint
    );
    void CG_MultiBeamEnd(
        float scale, int renderfx, const char *beamshadername, byte modulate[4], int flags, int owner, float life
    );
    void CG_CreateBeam(
        const vec3_t  start,
        const vec3_t  dir,
        int           owner,
        qhandle_t     hModel,
        float         alpha,
        float         scale,
        int           flags,
        float         length,
        int           life,
        qboolean      create,
        const vec3_t  endpointvec,
        int           min_offset,
        int           max_offset,
        int           overlap,
        int           subdivisions,
        int           delay,
        const char    *beamshader,
        float         modulate[4],
        int           numspherebeams,
        float         sphereradius,
        int           toggledelay,
        float         endalpha,
        int           renderfx,
        const char    *name
    );

    void CG_KillBeams(int entity_number);
    void CG_Rope(centity_t *cent);
    void CG_RestartBeams(int timedelta);

    //
    // cg_nature.cpp
    //
    void CG_Emitter(centity_t *cent);
    void CG_Rain(centity_t *cent);

    //
    // cg_testemitter.cpp
    void CG_UpdateTestEmitter(void);
    void CG_SortEffectCommands();
    void CG_TriggerTestEmitter_f(void);
    void CG_DumpEmitter_f(void);
    void CG_LoadEmitter_f(void);
    void CG_PrevEmitterCommand_f(void);
    void CG_NextEmitterCommand_f(void);
    void CG_NewEmitterCommand_f(void);
    void CG_DeleteEmitterCommand_f(void);
    void CG_InitTestEmitter(void);
    void CG_TestEmitter_f(void);
    void CG_DumpEmitter_f(void);

    //
    // cg_beam.cpp
    void RemoveBeamList(int owner);
    void CG_MultiBeam(centity_t *cent);

    //
    // cg_lightstyles.cpp
    void CG_SetLightStyle(int num, const char *s);
#ifdef __cplusplus
    qboolean CG_LightStyleColor(int style, int realtime, vec4_t color, qboolean clamp = qfalse);
#else
qboolean CG_LightStyleColor(int style, int realtime, vec4_t color, qboolean clamp);
#endif
    void CG_ClearLightStyles(void);
    int  CG_RegisterLightStyle(const char *name);

    //
    // cg_scoreboard.cpp
    void CG_GetScoreBoardColor(float *fR, float *fG, float *fB, float *fA);
    void CG_GetScoreBoardFontColor(float *fR, float *fG, float *fB, float *fA);
    void CG_GetScoreBoardPosition(float *fX, float *fY, float *fW, float *fH);
    int  CG_GetScoreBoardDrawHeader();
    //const char* CG_GetColumnName(int iColumnNum, int* iColumnWidth);
    void CG_PrepScoreBoardInfo();
    void CG_ParseScores();
    void CG_InitScoresAPI(clientGameExport_t *cge);

    //
    // cg_specialfx.cpp
    void CG_Footstep(const char *szTagName, centity_t *ent, refEntity_t *pREnt, int iRunning, int iEquipment);
    void CG_MeleeImpact(vec3_t vStart, vec3_t vEnd);
    void CG_LandingSound(centity_t *ent, refEntity_t *pREnt, float volume, int iEquipment);
    void CG_BodyFallSound(centity_t *ent, refEntity_t *pREnt, float volume);
    void CG_InitializeSpecialEffectsManager();
    void CG_AddPendingEffects();

    //
    // cg_swipe.cpp
    void CG_ClearSwipes(void);

    //
    // cg_ui.cpp
    void CG_MessageMode_f(void);
    void CG_MessageMode_All_f(void);
    void CG_MessageMode_Team_f(void);
    void CG_MessageMode_Private_f(void);
    void CG_MessageSingleAll_f(void);
    void CG_MessageSingleTeam_f(void);
    void CG_MessageSingleClient_f(void);
    void CG_InstaMessageMain_f(void);
    void CG_InstaMessageGroupA_f(void);
    void CG_InstaMessageGroupB_f(void);
    void CG_InstaMessageGroupC_f(void);
    void CG_InstaMessageGroupD_f(void);
    void CG_InstaMessageGroupE_f(void);
    void CG_HudPrint_f(void);
    int  CG_CheckCaptureKey(int key, qboolean down, unsigned int time);

    //
    // cg_vehicle.cpp
    void CG_Vehicle(centity_t *cent);

    //
    // cg_volumetricsmoke.cpp
    void CG_ResetVSSSources(void);
    void CG_AddVSSSources(void);

    extern cvar_t *vss_draw;
    extern cvar_t *vss_physics_fps;
    extern cvar_t *vss_repulsion_fps;
    extern cvar_t *vss_maxcount;
    extern cvar_t *vss_color;
    extern cvar_t *vss_showsources;
    extern cvar_t *vss_wind_x;
    extern cvar_t *vss_wind_y;
    extern cvar_t *vss_wind_z;
    extern cvar_t *vss_wind_strength;
    extern cvar_t *vss_movement_dampen;
    extern cvar_t *vss_maxvisible;
    extern cvar_t *vss_gridsize;
    extern cvar_t *vss_default_r;
    extern cvar_t *vss_default_g;
    extern cvar_t *vss_default_b;
    extern cvar_t *vss_lighting_fps;

#ifdef __cplusplus
}
#endif
