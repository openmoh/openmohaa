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
#include "../game/bg_public.h"
#include "cm_public.h"
#include "cg_public.h"

#ifdef __cplusplus
extern "C"
   {
#endif

typedef struct 
   {
   int      anim;
   int      frame;
   int      oldanim;
   int      oldframe;
   int      starttime;
   float    framelerp;
   } lerpstate_t;

typedef struct 
   {
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
   int      next_evaluate_time;

   // driven variables
   int      last_driven_time;
   vec3_t   last_origin;
   vec3_t   frame_delta;      // normalized frame delta
   float    frame_distance;   // float distance till next frame
   float    current_distance; // current distance traveled 

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
	entityState_t	currentState;  	// from cg.frame
	entityState_t	nextState;		   // from cg.nextFrame, if available
   qboolean       teleported;       // true if the entity was just teleported
	qboolean		   interpolate;	   // true if next is valid to interpolate to
	qboolean		   currentValid;	   // true if cg.frame holds this entity

	int				miscTime;
	int				snapShotTime;

	int				errorTime;		   // decay the error from this time
	vec3_t			errorOrigin;
	vec3_t			errorAngles;
	
	qboolean		   extrapolated;	   // false if origin / angles is an interpolation
	vec3_t			rawOrigin;
	vec3_t			rawAngles;

	vec3_t			beamEnd;

	// exact interpolated position of entity on this frame
	vec3_t			lerpOrigin;
	vec3_t			lerpAngles;

	sfxHandle_t		tikiLoopSound;
	float				tikiLoopSoundVolume;
    float				tikiLoopSoundMinDist;
    float				tikiLoopSoundMaxDist;
    float				tikiLoopSoundPitch;
	int					tikiLoopSoundFlags;

   float          color[4];
   float          client_color[4];  // the color set by client commands
   int            clientFlags;

	int				splash_last_spawn_time;
	int				splash_still_count;

	quat_t			bone_quat[NUM_BONE_CONTROLLERS];
	float			animLastTimes[MAX_FRAMEINFOS];
	int				animLast[MAX_FRAMEINFOS];
	int				animLastWeight;
	int				usageIndexLast;
	qboolean		bFootOnGround_Right;
	qboolean		bFootOnGround_Left;
	int				iNextLandTime;
} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independantly from all server transmitted entities

#define	MAX_VERTS_ON_POLY	10
#define	MAX_MARK_POLYS		64

typedef struct markPoly_s
   {
	struct markPoly_s	   *prevMark, *nextMark;
	int			         time;
   int                  lightstyle;
	qhandle_t	         markShader;
	qboolean	            alphaFade;		// fade alpha instead of rgb
   qboolean             fadein;
	float		            color[4];
	poly_t		         poly;
	polyVert_t	         verts[MAX_VERTS_ON_POLY];
   } markPoly_t;

#define	MAX_CUSTOM_SOUNDS	32
typedef struct {
	char			   name[MAX_QPATH];
	char			   data[MAX_QPATH];

	qhandle_t		legsModel;
	qhandle_t		legsSkin;

	qhandle_t		torsoModel;
	qhandle_t		torsoSkin;

	qhandle_t		headModel;
	qhandle_t		headSkin;

	struct sfx_s	*sounds[MAX_CUSTOM_SOUNDS];

	vec3_t			color;
	vec3_t			color2;
} clientInfo_t;


//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after
 
typedef struct {
	int			clientFrame;		   // incremented each frame
	
	qboolean	   demoPlayback;
	qboolean	   levelShot;		      // taking a level menu screenshot

	// there are only one or two snapshot_t that are relevent at a time
	int			latestSnapshotNum;	// the number of snapshots the client system has received
	int			latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet
	snapshot_t	*snap;				   // cg.snap->serverTime <= cg.time
	snapshot_t	*nextSnap;			   // cg.nextSnap->serverTime > cg.time, or NULL
	snapshot_t	activeSnapshots[2];

	float		   frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	qboolean	   thisFrameTeleport;
	qboolean	   nextFrameTeleport;
	qboolean	   nextFrameCameraCut;

	int			frametime;		      // cg.time - cg.oldTime

	int			time;			         // this is the time value that the client
								            // is rendering at.
	int			oldTime;		         // time at last frame, used for missile trails and prediction checking

	int			physicsTime;	      // either cg.snap->time or cg.nextSnap->time

	int			timelimitWarnings;	// 5 min, 1 min, overtime

	qboolean	renderingThirdPerson;	// during deaths, chasecams, etc

	// prediction state
	qboolean	hyperspace;				   // true if prediction has hit a trigger_teleport
	playerState_t	predicted_player_state;
	qboolean	validPPS;				   // clear until the first call to CG_PredictPlayerState
	int			predictedErrorTime;
	vec3_t		predictedError;

	float		   stepChange;			   // for stair up smoothing
	int			stepTime;

	float		   duckChange;			   // for duck viewheight smoothing
	int			duckTime;

	float		   landChange;			   // for landing hard
	int			landTime;

	// input state sent to server
	int			weaponSelect;

	// auto rotating items
	vec3_t		autoAngles;
	vec3_t		autoAxis[3];
	vec3_t		autoAnglesSlow;
	vec3_t		autoAxisSlow[3];
	vec3_t		autoAnglesFast;
	vec3_t		autoAxisFast[3];

	// view rendering
	refdef_t	   refdef;
   vec3_t      playerHeadPos;       // position of the players head
	vec3_t		refdefViewAngles;		// will be converted to refdef.viewaxis
   vec3_t      currentViewPos;      // current position of the camera
   vec3_t      currentViewAngles;   // current angles of the camera

	float			SoundOrg[3];		   // position from where sound should be played
	vec3_t		SoundAxis[3];		   // axis from where sound should be played

   int         lastCameraTime;      // last time the camera moved
   float       lerpCameraTime;      // is the camera currently lerping from camera to view or vice versa
   qboolean    inCameraView;        // are we currently in a camera view
   vec3_t      camera_origin;       // lerped camera_origin
   vec3_t      camera_angles;       // lerped camera_angles
   float       camera_fov;          // lerped camera_fov
   int         lastCameraFlags;     // last Camera flags for interpolation testing

	// zoom key
	qboolean	   zoomed;
	int		   zoomTime;
	float	   	zoomSensitivity;

	// information screen text during loading
	char		infoScreenText[MAX_STRING_CHARS];
	qboolean	showInformation;

	// scoreboard
	int			scoresRequestTime;
	int			numScores;
	int			teamScores[2];
	qboolean	   showScores;
	int			scoreFadeTime;
	char		   killerName[MAX_NAME_LENGTH];

	// centerprinting
	int			centerPrintTime;
	int			centerPrintCharWidth;
	int			centerPrintY;
	char		   centerPrint[1024];
	int			centerPrintLines;

	// low ammo warning state
	qboolean	lowAmmoWarning;		// 1 = low, 2 = empty

	// kill timers for carnage reward
	int			lastKillTime;

	// crosshair client ID
	int			crosshairClientNum;
	int			crosshairClientTime;

   float			crosshair_offset;

	// powerup active flashing
	int			powerupActive;
	int			powerupTime;

	// attacking player
	int			attackerTime;

	// warmup countdown
	int			warmupCount;

	//==========================

	int			itemPickup;
	int			itemPickupTime;
	int			itemPickupBlendTime;	// the pulse around the crosshair is timed seperately

	int			weaponSelectTime;
	int			weaponAnimation;
	int			weaponAnimationTime;

	// blend blobs
	float		   damageTime;
	float		   damageX, damageY, damageValue;

	// status bar head
	float		   headYaw;
	float		   headEndPitch;
	float		   headEndYaw;
	int			headEndTime;
	float		   headStartPitch;
	float		   headStartYaw;
	int			headStartTime;

	// view movement
	float		v_dmg_time;
	float		v_dmg_pitch;
	float		v_dmg_roll;

	vec3_t		kick_angles;	// weapon kicks
	vec3_t		kick_origin;

	// temp working variables for player view
	float		bobfracsin;
	int			bobcycle;
	float		xyspeed;

	// development tool
	refEntity_t		testModelEntity;
	char			testModelName[MAX_QPATH];
	qboolean		testGun;

   // farplane parameters
   float       farplane_distance;
   vec3_t      farplane_color;
   qboolean    farplane_cull;

   // portal sky parameters
   qboolean    sky_portal;
   float       sky_alpha;
   vec3_t      sky_origin;
	vec3_t		sky_axis[3];			// rotation vectors
} cg_t;


typedef struct {
   qhandle_t   backTileShader;
   qhandle_t   lagometerShader;
   qhandle_t   shadowMarkShader;
   qhandle_t   wakeMarkShader;
   qhandle_t   leftTargetShader;
   qhandle_t   rightTargetShader;
   qhandle_t   itemRingShader;
   qhandle_t   leftTargetModel;
   qhandle_t   rightTargetModel;
   qhandle_t   pausedShader;
   qhandle_t   levelExitShader;
   } media_t;

// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
typedef struct {
	gameState_t		gameState;			// gamestate from server
	glconfig_t		glconfig;			// rendering configuration
	float			   screenXScale;		// derived from glconfig
	float			   screenYScale;
	float			   screenXBias;

	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested

	qboolean		   localServer;		// detected on startup by checking sv_running
	int				levelStartTime;   // time that game was started

	// parsed from serverinfo
	gametype_t		gametype;
	int				dmflags;
	int				teamflags;
	int				fraglimit;
	int				timelimit;
	int				maxclients;
   int            cinematic;
	char			   mapname[MAX_QPATH];

	//
	// locally derived information from gamestate
	//
	qhandle_t		model_draw[MAX_MODELS];
   sfxHandle_t		sound_precache[MAX_SOUNDS];
	int				numInlineModels;
	qhandle_t		inlineDrawModel[MAX_MODELS];
	vec3_t			inlineModelMidpoints[MAX_MODELS];

   // TIKI handles for all models
   int            model_tiki[ MAX_MODELS ];

	clientInfo_t	clientinfo[MAX_CLIENTS];

   media_t        media;
   } cgs_t;

//==============================================================================

extern cvar_t* paused;
extern cvar_t* developer;

extern	cgs_t			         cgs;
extern	cg_t			         cg;
extern	clientGameImport_t	cgi;
extern	centity_t		      cg_entities[MAX_GENTITIES];
extern	markPoly_t		      cg_markPolys[MAX_MARK_POLYS];

extern cvar_t* cg_animSpeed;
extern cvar_t* cg_debugAnim;
extern cvar_t* cg_debugAnimWatch;
extern cvar_t* cg_errorDecay;
extern cvar_t* cg_nopredict;
extern cvar_t* cg_showmiss;
extern cvar_t* cg_addMarks;
extern cvar_t* cg_maxMarks;
extern cvar_t* cg_viewsize;
extern cvar_t* cg_3rd_person;
extern cvar_t* cg_drawviewmodel;
extern cvar_t* cg_synchronousClients;
extern cvar_t* cg_stereoSeparation;
extern cvar_t* cg_stats;
extern cvar_t* cg_lagometer;
extern cvar_t* r_lerpmodels;
extern cvar_t* cg_cameraheight;
extern cvar_t* cg_cameradist;
extern cvar_t* cg_cameraverticaldisplacement;
extern cvar_t* cg_camerascale;
extern cvar_t* cg_shadows;
extern cvar_t* cg_hidetempmodels;
extern cvar_t* cg_traceinfo;
extern cvar_t* cg_debugFootsteps;
extern cvar_t* cg_smoothClients;
extern cvar_t* cg_smoothClientsTime;
extern cvar_t* pmove_fixed;
extern cvar_t* pmove_msec;
extern cvar_t* cg_pmove_msec;
extern cvar_t* dm_playermodel;
extern cvar_t* dm_playergermanmodel;
extern cvar_t* cg_forceModel;
extern cvar_t* cg_animationviewmodel;
extern cvar_t* cg_hitmessages;
extern cvar_t* cg_acidtrip;
extern cvar_t* cg_hud;
extern cvar_t* cg_huddraw_force;
extern cvar_t* cg_drawsvlag;
extern cvar_t* vm_offset_max;
extern cvar_t* vm_offset_speed;
extern cvar_t* vm_sway_front;
extern cvar_t* vm_sway_side;
extern cvar_t* vm_sway_up;
extern cvar_t* vm_offset_air_front;
extern cvar_t* vm_offset_air_side;
extern cvar_t* vm_offset_air_up;
extern cvar_t* vm_offset_crouch_front;
extern cvar_t* vm_offset_crouch_side;
extern cvar_t* vm_offset_crouch_up;
extern cvar_t* vm_offset_rocketcrouch_front;
extern cvar_t* vm_offset_rocketcrouch_side;
extern cvar_t* vm_offset_rocketcrouch_up;
extern cvar_t* vm_offset_shotguncrouch_front;
extern cvar_t* vm_offset_shotguncrouch_side;
extern cvar_t* vm_offset_shotguncrouch_up;
extern cvar_t* vm_offset_vel_base;
extern cvar_t* vm_offset_vel_front;
extern cvar_t* vm_offset_vel_side;
extern cvar_t* vm_offset_vel_up;
extern cvar_t* vm_offset_upvel;
extern cvar_t* vm_lean_lower;
extern cvar_t* voiceChat;
extern cvar_t* cg_shadowscount;
extern cvar_t* cg_shadowdebug;

//
// cg_main.c
//
void        CG_ProcessConfigString( int num );
const char *CG_ConfigString( int index );
void        CG_AddToTeamChat( const char *str );
void        CG_NewClientinfo( int clientNum );
sfxHandle_t	CG_CustomSound( int entityNum, const char *soundName );
int         CG_CrosshairPlayer( void );
int         CG_LastAttacker( void );
void        CG_Init( clientGameImport_t *imported, int serverMessageNum, int serverCommandSequence );
void        CG_Shutdown( void );
void        CG_ServerRestarted( void );

//
// cg_modelanim.cpp
//
void CG_ModelAnim(centity_t *cent, qboolean bDoShaderTime);
void CG_ClearModelAnimation( int tikihandle, animstate_t * state, int animationNumber, int time, vec3_t origin, int entnum );
void CG_AttachEntity( refEntity_t *entity, refEntity_t *parent, dtiki_t* tiki, int tagnum, qboolean use_angles, vec3_t attach_offset );

//
// cg_commands.cpp
//
void     CG_Event( centity_t *cent );
void     CG_UpdateEntityEmitters(int entnum, refEntity_t *ent, centity_t *cent );
void     CG_RemoveClientEntity( int number, int tikihandle, centity_t *cent );
void     CG_UpdateTestEmitter( void );
void     CG_AddTempModels( void );
void     CG_ResetTempModels( void );
void     CG_InitializeCommandManager( void );
void     CG_ProcessInitCommands( int tikihandle );
qboolean CG_Command_ProcessFile( const char * filename, qboolean quiet );
void     CG_RestartCommandManager( int timedelta );
void     CG_FlushCommandManager( void );
void     CG_ProcessEntityCommands( int            tikihandle,
                                   int            frame,
                                   int            anim,
                                   int            entnum,
                                   refEntity_t		*ent,
                                   centity_t      *cent
                                 );
void CG_ClientCommands
   (
   int            tikihandle,
   int            frame,
   int            anim,
   animstate_t    *state,
   refEntity_t		*ent,
   centity_t      *cent
   );

void CG_Splash( centity_t *cent );

void CG_EventList_f( void );
void CG_EventHelp_f( void ); 
void CG_DumpEventHelp_f( void ); 
void CG_PendingEvents_f( void );
void CG_ClassList_f( void );
void CG_ClassTree_f( void );
void CG_ClassEvents_f( void );
void CG_DumpClassEvents_f( void );
void CG_DumpAllClasses_f( void );

void L_InitEvents( void );
void L_ShutdownEvents( void );


//
// cg_view.c
//
void CG_TestModel_f (void);
void CG_TestGun_f (void);
void CG_TestModelNextFrame_f (void);
void CG_TestModelPrevFrame_f (void);
void CG_TestModelNextSkin_f (void);
void CG_TestModelPrevSkin_f (void);

void CG_ZoomDown_f( void );
void CG_ZoomUp_f( void );

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );


//
// cg_drawtools.c
//
void CG_AdjustFrom640( float *x, float *y, float *w, float *h );
void CG_TileClear( void );
void CG_Draw2D( void );

//
// cg_draw.c
//
void CG_AddLagometerFrameInfo( void );
void CG_AddLagometerSnapshotInfo( snapshot_t *snap );
void CG_CenterPrint( const char *str, int y, int charWidth );
void CG_DrawHead( float x, float y, float w, float h, int clientNum, vec3_t headAngles );
void CG_DrawActive( stereoFrame_t stereoView );
void CG_DrawFlagModel( float x, float y, float w, float h, int team );

//
// cg_predict.c
//
void     CG_BuildSolidList( void );
int      CG_PointContents( const vec3_t point, int passEntityNum );
void     CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, 
                   const vec3_t maxs, const vec3_t end, int skipNumber,
                   int mask, qboolean cylinder, qboolean cliptoentities, const char * description );
void     CG_PredictPlayerState( void );


//
// cg_ents.c
//
void CG_SetEntitySoundPosition( centity_t *cent );
void CG_AddPacketEntities( void );
void CG_Beam( centity_t *cent );
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out );

void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName );
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName );
void CG_GetOrigin( centity_t *cent, vec3_t origin );
void CG_EntityEffects( centity_t *cent );

//
// cg_marks.c
//
void	CG_InitMarkPolys( void );
void	CG_AddMarks( void );
void	CG_ImpactMark( qhandle_t markShader, 
				    const vec3_t origin, const vec3_t dir, 
					float orientation, 
				    float r, float g, float b, float a, 
					qboolean alphaFade, 
					float radius, qboolean temporary,
               int lightstyle, qboolean fadein );

//
// cg_snapshot.c
//
void CG_ProcessSnapshots( void );

//
// cg_consolecmds.c
//
qboolean CG_ConsoleCommand( void );
void CG_InitConsoleCommands( void );
void CG_AddTestModel( void );

//
// cg_servercmds.c
//
void CG_ExecuteNewServerCommands( int latestSequence );
void CG_ParseServerinfo( void );

//
// cg_playerstate.c
//
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops );

//
// cg_player.cpp
//
void CG_ResetPlayerEntity( centity_t *cent );
void CG_Player( centity_t *cent );

//
// cg_sound.cpp
//
void CG_ProcessSound( server_sound_t *sound );

//
// cg_beam.cpp
//
void CG_AddBeams( void );
void CG_MultiBeamBegin( void );
void CG_MultiBeamAddPoints
   (
   vec3_t   start,
   vec3_t   end,
   int      numsegments,
   int      flags,
   float    minoffset,
   float    maxoffset,
   qboolean addstartpoint
   );
void CG_MultiBeamEnd
   (
   float       scale,
   int         renderfx,
   const char  *beamshadername,
   byte        modulate[4],
   int         flags,
   int         owner,
   float       life
   );
void CG_CreateBeam
   (
   vec3_t         start,
   vec3_t         dir, 
   int            owner,
   qhandle_t      hModel,
   float          alpha, 
   float          scale, 
   int            flags, 
   float          length,
   int            life,
   qboolean       create,
   vec3_t         endpointvec,
   int            min_offset,
   int            max_offset,
   int            overlap,
   int            subdivisions,
   int            delay,
   const char     *beamshader,
   byte           modulate[4],
   int            numspherebeams,
   float          sphereradius,
   int            toggledelay,
   float          endalpha,
   int            renderfx,
   const char     *name
   );

void CG_KillBeams( int entity_number );
void CG_Rope( centity_t *cent );
void CG_RestartBeams( int timedelta );


//
// cg_nature.cpp
//
void CG_Emitter( centity_t *cent );

//
// cg_testemitter.cpp
void CG_InitTestEmitter( void );
void CG_TestEmitter_f( void );
void CG_DumpEmitter_f( void );
void CG_InitializeClientEmitters( void );

//
// cg_beam.cpp
void RemoveBeamList( int owner );
void CG_MultiBeam( centity_t *cent );

//
// cg_lightstyles.cpp
void CG_SetLightStyle( int num, const char *s );
#ifdef __cplusplus
qboolean CG_LightStyleColor( int style, int realtime, float color[4], qboolean clamp = qfalse );
#else
qboolean CG_LightStyleColor( int style, int realtime, float color[4], qboolean clamp );
#endif
void CG_ClearLightStyles( void );
int  CG_RegisterLightStyle( const char * name );

//
// cg_specialfx.cpp
void CG_Footstep( centity_t * ent, float volume );

//
// cg_swipe.cpp
void CG_ClearSwipes( void );


#ifdef __cplusplus
}
#endif
