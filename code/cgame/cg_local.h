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
#include "../qcommon/q_shared.h"
#include "../renderercommon/tr_types.h"
#include "../game/bg_public.h"
#include "../qcommon/tiki_local.h"
#include "cg_public.h"


// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

#define	FADE_TIME			200
#define	PULSE_TIME			200
#define	DAMAGE_DEFLECT_TIME	100
#define	DAMAGE_RETURN_TIME	400
#define DAMAGE_TIME			500
#define	LAND_DEFLECT_TIME	150
#define	LAND_RETURN_TIME	300
#define	STEP_TIME			200
#define	DUCK_TIME			100
#define	PAIN_TWITCH_TIME	200
#define	WEAPON_SELECT_TIME	1400
#define	ZOOM_TIME			150
#define	ITEM_BLOB_TIME		200
#define	MUZZLE_FLASH_TIME	20
#define	SINK_TIME			1000		// time for fragments to sink into ground before going away
#define	ATTACKER_HEAD_TIME	10000

#define	MAX_STEP_CHANGE		32

#define	MAX_VERTS_ON_POLY	10
#define	MAX_MARK_POLYS		256

#define STAT_MINUS			10	// num frame for '-' stats digit

#define	CHAR_WIDTH			32
#define	CHAR_HEIGHT			48
#define	TEXT_ICON_SPACE		4

#define	TEAMCHAT_WIDTH		80
#define TEAMCHAT_HEIGHT		8

// very large characters
#define	GIANT_WIDTH			32
#define	GIANT_HEIGHT		48

#define	NUM_CROSSHAIRS		3

#define TEAM_OVERLAY_MAXNAME_WIDTH	12
#define TEAM_OVERLAY_MAXLOCATION_WIDTH	16

#define	DEFAULT_MODEL			"american_army"

//=================================================

typedef enum  {
	SMT_NONE,
	SMT_YELLOW,		// below compass, color yellow
	SMT_CHAT,		// right of compass, color grey
	SMT_WHITE,		// below compass, color white
	SMT_DEATH,		// right of compass, color red
	SMT_UNKNOWN,	// don't know if exists. observe with breakpoint
	SMT_MAX
} serverMessageType_t;

//=================================================

// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s {
	entityState_t	currentState;	// from cg.frame
	entityState_t	nextState;		// from cg.nextFrame, if available
	qboolean		interpolate;	// true if next is valid to interpolate to
	qboolean		currentValid;	// true if cg.frame holds this entity

	int				snapShotTime;	// last time this entity was found in a snapshot

	int				errorTime;		// decay the error from this time
	vec3_t			errorOrigin;
	vec3_t			errorAngles;

	qboolean		extrapolated;	// false if origin / angles is an interpolation
	vec3_t			rawOrigin;
	vec3_t			rawAngles;

	vec3_t			beamEnd;

	// exact interpolated position of entity on this frame
	vec3_t			lerpOrigin;
	vec3_t			lerpAngles;

	bone_t			*bones; // su44: for tiki models

	vec4_t			color;

	int				bFootOnGround_Right;
	int				bFootOnGround_Left;
} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independantly from all server transmitted entities

typedef struct markPoly_s {
	struct markPoly_s	*prevMark, *nextMark;
	int			time;
	qhandle_t	markShader;
	qboolean	alphaFade;		// fade alpha instead of rgb
	float		color[4];
	poly_t		poly;
	polyVert_t	verts[MAX_VERTS_ON_POLY];
} markPoly_t;


typedef enum {
	LE_SKIP,
	LE_MARK,
	LE_EXPLOSION,
	LE_SPRITE_EXPLOSION,
	LE_FRAGMENT,
	LE_MOVE_SCALE_FADE,
	LE_FALL_SCALE_FADE,
	LE_FADE_RGB,
	LE_SCALE_FADE,
	LE_SCOREPLUM,
#ifdef MISSIONPACK
	LE_KAMIKAZE,
	LE_INVULIMPACT,
	LE_INVULJUICED,
	LE_SHOWREFENTITY
#endif
} leType_t;

typedef enum {
	LEF_PUFF_DONT_SCALE  = 0x0001,			// do not scale size over time
	LEF_TUMBLE			 = 0x0002,			// tumble over time, used for ejecting shells
	LEF_SOUND1			 = 0x0004,			// sound 1 for kamikaze
	LEF_SOUND2			 = 0x0008			// sound 2 for kamikaze
} leFlag_t;

typedef enum {
	LEMT_NONE,
	LEMT_BURN,
	LEMT_BLOOD
} leMarkType_t;			// fragment local entities can leave marks on walls

typedef enum {
	LEBS_NONE,
	LEBS_BLOOD,
	LEBS_BRASS
} leBounceSoundType_t;	// fragment local entities can make sounds on impacts

typedef struct localEntity_s {
	struct localEntity_s	*prev, *next;
	leType_t		leType;
	int				leFlags;

	int				startTime;
	int				endTime;
	int				fadeInTime;

	float			lifeRate;			// 1.0 / (endTime - startTime)

	trajectory_t	pos;
	trajectory_t	angles;

	float			bounceFactor;		// 0.0 = no bounce, 1.0 = perfect

	float			color[4];

	float			radius;

	float			light;
	vec3_t			lightColor;

	leMarkType_t		leMarkType;		// mark to leave on fragment impact
	leBounceSoundType_t	leBounceSoundType;

	refEntity_t		refEntity;

	tiki_t *tiki;
} localEntity_t;

//======================================================================

typedef struct {
	int team;
	char name[64];
} clientInfo_t;

// su44: MoHAA rain/snow effect
typedef struct {
	float density;
	float speed;
	int speed_vary;
	int slant;
	float length;
	float min_dist;
	float width;
	char shader[16][2048];
	int numshaders;
} crain_t;

#define MAX_REWARDSTACK		10
#define MAX_SOUNDBUFFER		20

//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

typedef struct {
	int			clientFrame;		// incremented each frame

	int			clientNum;

	qboolean	demoPlayback;
	qboolean	levelShot;			// taking a level menu screenshot

	// there are only one or two snapshot_t that are relevent at a time
	int			latestSnapshotNum;	// the number of snapshots the client system has received
	int			latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet

	snapshot_t	*snap;				// cg.snap->serverTime <= cg.time
	snapshot_t	*nextSnap;			// cg.nextSnap->serverTime > cg.time, or NULL
	snapshot_t	activeSnapshots[2];

	float		frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	qboolean	thisFrameTeleport;
	qboolean	nextFrameTeleport;

	int			frametime;		// cg.time - cg.oldTime

	int			time;			// this is the time value that the client
								// is rendering at.
	int			oldTime;		// time at last frame, used for missile trails and prediction checking

	int			physicsTime;	// either cg.snap->time or cg.nextSnap->time

	int			timelimitWarnings;	// 5 min, 1 min, overtime
	int			fraglimitWarnings;

	qboolean	mapRestart;			// set on a map restart to set back the weapon

	qboolean	renderingThirdPerson;		// during deaths, chasecams, etc

	// prediction state
	qboolean	hyperspace;				// true if prediction has hit a trigger_teleport
	playerState_t	predictedPlayerState;
	centity_t		predictedPlayerEntity;
	qboolean	validPPS;				// clear until the first call to CG_PredictPlayerState
	int			predictedErrorTime;
	vec3_t		predictedError;

	float		stepChange;				// for stair up smoothing
	int			stepTime;

	float		duckChange;				// for duck viewheight smoothing
	int			duckTime;

	float		landChange;				// for landing hard
	int			landTime;

	// auto rotating items
	vec3_t		autoAngles;
	vec3_t		autoAxis[3];
	vec3_t		autoAnglesFast;
	vec3_t		autoAxisFast[3];

	// view rendering
	refdef_t	refdef;
	vec3_t		refdefViewAngles;		// will be converted to refdef.viewaxis

	// zoom key
	qboolean	zoomed;
	int			zoomTime;
	float		zoomSensitivity;

	// information screen text during loading
	char		infoScreenText[MAX_STRING_CHARS];

	// scoreboard
	char		aScore[MAX_STRING_CHARS];
	int			scoresRequestTime;
	int			numScores;
	int			selectedScore;
	int			teamScores[2];
	qboolean	showScores;
	qboolean	scoreBoardShowing;
	int			scoreFadeTime;
	char		killerName[MAX_NAME_LENGTH];

	// centerprinting
	int			centerPrintTime;
	int			centerPrintCharWidth;
	int			centerPrintY;
	char		centerPrint[1024];
	int			centerPrintLines;

	// locationprinting
	int			locationPrintTime;
	int			locationPrintCharWidth;
	int			locationPrintX;
	int			locationPrintY;
	char		locationPrint[1024];
	int			locationPrintLines;

	// Game Messages
#define MAX_GAMEMESSAGES		4
#define MAX_CHATDEATHMESSAGES	8

	int						gameMessageTime;
	char					gameMessages[MAX_GAMEMESSAGES][MAX_QPATH];
	serverMessageType_t		gameMessageTypes[MAX_GAMEMESSAGES];
	int						gameMessagePtr1;
	int						gameMessagePtr2;

	int						chatDeathMessageTime;
	char					chatDeathMessages[MAX_CHATDEATHMESSAGES][MAX_QPATH];
	serverMessageType_t		chatDeathMessageTypes[MAX_CHATDEATHMESSAGES];
	int						chatDeathMessagePtr1;
	int						chatDeathMessagePtr2;

	// kill timers for carnage reward
	int			lastKillTime;

	// attacking player
	int			attackerTime;
	int			voiceTime;

	// warmup countdown
	int			warmup;
	int			warmupCount;

	//==========================

	int			itemPickup;
	int			itemPickupTime;
	int			itemPickupBlendTime;	// the pulse around the crosshair is timed seperately

	int			weaponSelectTime;
	int			weaponAnimation;
	int			weaponAnimationTime;

	// blend blobs
	float		damageTime;
	float		damageX, damageY, damageValue;

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
	int     nextOrbitTime;

	float fCurrentViewBobAmp;
	float fCurrentViewBobPhase;

	// su44: MoHAA fog
	float		farplane_distance;
	float		farplane_color[3];
	qboolean	farplane_cull;

	crain_t rain;

	int iWeaponCommand;
	int iWeaponCommandSend;

	//qboolean cameraMode;		// if rendering from a loaded camera


	// development tool
	refEntity_t		testModelEntity;
	char			testModelName[MAX_QPATH];
	char			testModelAnim[MAX_QPATH]; // su44: for testing TIKI model animations
	qboolean		testGun;
	float			testModelAnimSpeedScale;

} cg_t;


// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct {
	qhandle_t	whiteShader;
	qhandle_t	blackShader;

	qhandle_t	connectionShader;
	qhandle_t	balloonShader;

	qhandle_t	crosshairShader[3];

	qhandle_t	shadowMarkShader;
	qhandle_t backTileShader, wakeMarkShader;




	// su44: MoHAA zoom overlay shaders
	qhandle_t	zoomOverlayShader;
	qhandle_t	kar98TopOverlayShader;
	qhandle_t	kar98BottomOverlayShader;
	qhandle_t	binocularsOverlayShader;
	
	// font to use in cgame texts
	fontInfo_t		facfont;
	fontInfo_t		verdana;
} cgMedia_t;


// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
typedef struct {
	gameState_t		gameState;			// gamestate from server
	glconfig_t		glconfig;			// rendering configuration
	float			screenXScale;		// derived from glconfig
	float			screenYScale;
	float			screenXBias;

	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested

	qboolean		localServer;		// detected on startup by checking sv_running

	// parsed from serverinfo
	gametype_t		gametype;
	int				dmflags;
	int				teamflags;
	int				fraglimit;
	int				capturelimit;
	int				timelimit;
	int				maxclients;
	char			mapname[MAX_QPATH];

	int				levelStartTime;

	//
	// locally derived information from gamestate
	//
	qhandle_t		gameModels[MAX_MODELS];
	sfxHandle_t		gameSounds[MAX_SOUNDS];
	tiki_t	*		gameTIKIs[MAX_MODELS];

	int				numInlineModels;
	qhandle_t		inlineDrawModel[MAX_MODELS];
	vec3_t			inlineModelMidpoints[MAX_MODELS];

	clientInfo_t	clientinfo[MAX_CLIENTS];

	// teamchat width is *3 because of embedded color codes
	char			teamChatMsgs[TEAMCHAT_HEIGHT][TEAMCHAT_WIDTH*3+1];
	int				teamChatMsgTimes[TEAMCHAT_HEIGHT];
	int				teamChatPos;
	int				teamLastChatPos;

	int cursorX;
	int cursorY;
	qboolean eventHandling;
	qboolean mouseCaptured;
	qboolean sizingHud;
	void *capturedItem;
	qhandle_t activeCursor;

	// media
	cgMedia_t		media;

	int loadingStage;

} cgs_t;

//==============================================================================

extern	cgs_t			cgs;
extern	cg_t			cg;
extern	centity_t		cg_entities[MAX_GENTITIES];
extern	markPoly_t		cg_markPolys[MAX_MARK_POLYS];

extern	cvar_t			*cg_centertime;
extern	cvar_t			*cg_locationtime;
extern	cvar_t			*cg_runpitch;
extern	cvar_t			*cg_runroll;
extern	cvar_t			*cg_bobup;
extern	cvar_t			*cg_bobpitch;
extern	cvar_t			*cg_bobroll;
extern	cvar_t			*cg_swingSpeed;
extern	cvar_t			*cg_shadows;
extern	cvar_t			*cg_gibs;
extern	cvar_t			*cg_drawTimer;
extern	cvar_t			*cg_drawFPS;
extern	cvar_t			*cg_drawSnapshot;
extern	cvar_t			*cg_draw3dIcons;
extern	cvar_t			*cg_drawIcons;
extern	cvar_t			*cg_drawAmmoWarning;
extern	cvar_t			*cg_drawCrosshair;
extern	cvar_t			*cg_drawCrosshairNames;
extern	cvar_t			*cg_drawRewards;
extern	cvar_t			*cg_drawTeamOverlay;
extern	cvar_t			*cg_teamOverlayUserinfo;
extern	cvar_t			*cg_crosshairX;
extern	cvar_t			*cg_crosshairY;
extern	cvar_t			*cg_crosshairSize;
extern	cvar_t			*cg_crosshairHealth;
extern	cvar_t			*cg_drawStatus;
extern	cvar_t			*cg_draw2D;
extern	cvar_t			*cg_animSpeed;
extern	cvar_t			*cg_debugAnim;
extern	cvar_t			*cg_debugPosition;
extern	cvar_t			*cg_debugEvents;
extern	cvar_t			*cg_railTrailTime;
extern	cvar_t			*cg_errorDecay;
extern	cvar_t			*cg_nopredict;
extern	cvar_t			*cg_noPlayerAnims;
extern	cvar_t			*cg_showmiss;
extern	cvar_t			*cg_footsteps;
extern	cvar_t			*cg_addMarks;
extern	cvar_t			*cg_brassTime;
extern	cvar_t			*cg_gun_frame;
extern	cvar_t			*cg_gun_x;
extern	cvar_t			*cg_gun_y;
extern	cvar_t			*cg_gun_z;
extern	cvar_t			*cg_drawGun;
extern	cvar_t			*cg_viewsize;
extern	cvar_t			*cg_tracerChance;
extern	cvar_t			*cg_tracerWidth;
extern	cvar_t			*cg_tracerLength;
extern	cvar_t			*cg_autoswitch;
extern	cvar_t			*cg_ignore;
extern	cvar_t			*cg_simpleItems;
extern	cvar_t			*cg_fov;
extern	cvar_t			*cg_zoomFov;
extern	cvar_t			*cg_thirdPersonRange;
extern	cvar_t			*cg_thirdPersonAngle;
extern	cvar_t			*cg_thirdPerson;
extern	cvar_t			*cg_stereoSeparation;
extern	cvar_t			*cg_lagometer;
extern	cvar_t			*cg_drawAttacker;
extern	cvar_t			*cg_synchronousClients;
extern	cvar_t			*cg_teamChatTime;
extern	cvar_t			*cg_teamChatHeight;
extern	cvar_t			*cg_stats;
extern	cvar_t			*cg_forceModel;
extern	cvar_t			*cg_buildScript;
extern	cvar_t			*cg_paused;
extern	cvar_t			*cg_blood;
extern	cvar_t			*cg_predictItems;
extern	cvar_t			*cg_deferPlayers;
extern	cvar_t			*cg_drawFriend;
extern	cvar_t			*cg_teamChatsOnly;
extern	cvar_t			*cg_noVoiceChats;
extern	cvar_t			*cg_noVoiceText;
extern  cvar_t			*cg_scorePlum;
extern	cvar_t			*cg_smoothClients;
extern	cvar_t			*pmove_fixed;
extern	cvar_t			*pmove_msec;
//extern	cvar_t		*cg_pmove_fixed;
extern	cvar_t		*cg_cameraOrbit;
extern	cvar_t		*cg_cameraOrbitDelay;
extern	cvar_t		*cg_timescaleFadeEnd;
extern	cvar_t		*cg_timescaleFadeSpeed;
extern	cvar_t		*cg_timescale;
extern	cvar_t		*cg_cameraMode;
extern  cvar_t		*cg_smallFont;
extern  cvar_t		*cg_bigFont;
extern	cvar_t		*cg_noTaunt;
extern	cvar_t		*cg_noProjectileTrail;
extern	cvar_t		*cg_trueLightning;

extern	cvar_t		*vm_offset_max;
extern	cvar_t		*vm_offset_speed;
extern	cvar_t		*vm_sway_front;
extern	cvar_t		*vm_sway_side;
extern	cvar_t		*vm_sway_up;
extern	cvar_t		*vm_offset_air_front;
extern	cvar_t		*vm_offset_air_side;
extern	cvar_t		*vm_offset_air_up;
extern	cvar_t		*vm_offset_crouch_front;
extern	cvar_t		*vm_offset_crouch_side;
extern	cvar_t		*vm_offset_crouch_up;
extern	cvar_t		*vm_offset_rocketcrouch_front;
extern	cvar_t		*vm_offset_rocketcrouch_side;
extern	cvar_t		*vm_offset_rocketcrouch_up;
extern	cvar_t		*vm_offset_shotguncrouch_front;
extern	cvar_t		*vm_offset_shotguncrouch_side;
extern	cvar_t		*vm_offset_shotguncrouch_up;
extern	cvar_t		*vm_offset_vel_base;
extern	cvar_t		*vm_offset_vel_front;
extern	cvar_t		*vm_offset_vel_side;
extern	cvar_t		*vm_offset_vel_up;
extern	cvar_t		*vm_offset_upvel;
extern	cvar_t		*vm_lean_lower;
extern	cvar_t		*cg_debugCGMessages; // su44: for debuging in cg_parsemsg.c

// UBERSOUND
#define UBERSOUND_FILE	"ubersound/ubersound.scr"
#define UBERDIALOG_FILE	"ubersound/uberdialog.scr"
// su44: vanilla AA uberdialog.scr is 870195
// wombat: it is larger in German version
#define UBERSOUND_SIZE	1048576 // 300000
#define MUSIC_SIZE		10000
#define MAPSTRING_LENGTH 512

typedef struct ubersound_s {
	char				name[MAX_QPATH];
	char				wavfile[128];
	float				basevol;
	float				volmod;
	float				basepitch;
	float				pitchmod;
	float				minDist;
	float				maxDist;
	soundChannel_t		channel;
	qboolean			loaded;	// loaded or streamed
	char				mapstring[MAPSTRING_LENGTH];
	char				subtitle[512]; // su44: for uberdialog.scr loading

	sfxHandle_t			sfxHandle;
	qboolean			hasLoaded;
	int					variations; // some sounds have enumerated variations
	struct ubersound_s* nextVariation;
	struct ubersound_s*	hashNext;
} ubersound_t;

//
// cg_main.c
//
extern	clientGameImport_t		cgi;
extern	clientGameExport_t		cge;

const char *CG_ConfigString( int index );
const char *CG_Argv( int arg );

void QDECL CG_Printf( const char *msg, ... );
void QDECL CG_Error( const char *msg, ... );

void CG_StartMusic( void );

void CG_UpdateCvars( void );

int CG_CrosshairPlayer( void );
int CG_LastAttacker( void );
void CG_LoadMenus(const char *menuFile);
void CG_KeyEvent(int key, qboolean down);
void CG_MouseEvent(int x, int y);
void CG_EventHandling(int type);
void CG_RankRunFrame( void );
void CG_SetScoreSelection(void *menu);

//
// cg_view.c
//
void CG_TestModel_f (void);
void CG_TestGun_f (void);
void CG_TestModelNextFrame_f (void);
void CG_TestModelPrevFrame_f (void);
void CG_TestModelNextSkin_f (void);
void CG_TestModelPrevSkin_f (void);
void CG_TestModelAnim_f (void);
void CG_TestModelAnimSpeedScale_f (void);
void CG_ZoomDown_f( void );
void CG_ZoomUp_f( void );
void CG_AddBufferedSound( sfxHandle_t sfx);

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );
void CG_Draw2D( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );


//
// cg_drawtools.c
//
void CG_AdjustFrom640( float *x, float *y, float *w, float *h );
void CG_FillRect( float x, float y, float width, float height, const float *color );
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader );
int CG_DrawStrlen( const char *str );

float	*CG_FadeColor( int startMsec, int totalMsec );
float *CG_TeamColor( int team );
void CG_TileClear( void );
void CG_ColorForHealth( vec4_t hcolor );
void CG_GetColorForHealth( int health, int armor, vec4_t hcolor );

void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color );
void CG_DrawRect( float x, float y, float width, float height, float size, const float *color );
void CG_DrawSides(float x, float y, float w, float h, float size);
void CG_DrawTopBottom(float x, float y, float w, float h, float size);
void CG_DrawBigString( int x, int y, const char *s, float alpha );
void CG_DrawStringExt( int x, int y, const char *string, const float *setColor,
	qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars );


//
// cg_draw.c, cg_newDraw.c
//
extern	int sortedTeamPlayers[TEAM_MAXOVERLAY];
extern	int	numSortedTeamPlayers;
extern	int drawTeamOverlayModificationCount;
extern  char systemChat[256];
extern  char teamChat1[256];
extern  char teamChat2[256];

void CG_AddLagometerFrameInfo( void );
void CG_AddLagometerSnapshotInfo( snapshot_t *snap );
void CG_CenterPrint( const char *str, int y, int charWidth );
void CG_LocationPrint( const char *str, int x, int y, int charWidth );
void CG_DrawActive( stereoFrame_t stereoView );
void CG_DrawFlagModel( float x, float y, float w, float h, int team, qboolean force2D );
void CG_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle);
void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style);
int CG_Text_Width(const char *text, float scale, int limit);
int CG_Text_Height(const char *text, float scale, int limit);
void CG_SelectPrevPlayer( void );
void CG_SelectNextPlayer( void );
float CG_GetValue(int ownerDraw);
qboolean CG_OwnerDrawVisible(int flags);
void CG_RunMenuScript(char **args);
void CG_ShowResponseHead( void );
void CG_SetPrintString(int type, const char *p);
void CG_InitTeamChat( void );
void CG_GetTeamColor(vec4_t *color);
const char *CG_GetGameStatusText( void );
const char *CG_GetKillerText( void );
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles );
void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader);
void CG_CheckOrderPending( void );
const char *CG_GameTypeString( void );
qboolean CG_YourTeamHasFlag( void );
qboolean CG_OtherTeamHasFlag( void );
qhandle_t CG_StatusHandle(int task);
void	CG_DrawBox( int x, int y, int w, int h, qboolean ctrCoord );
void	CG_DrawStdBox( int x, int y, int w, int h, qboolean ctrCoord );

//
// cg_player.c
//
void CG_Player( centity_t *cent );
void CG_PlayerSprites( centity_t *cent );
void CG_ExtractPlayerTeam( centity_t *cent );
void CG_ResetPlayerEntity( centity_t *cent );
void CG_NewClientInfo( int clientNum );
sfxHandle_t	CG_CustomSound( int clientNum, const char *soundName );

//
// cg_predict.c
//
void CG_BuildSolidList( void );
int	CG_PointContents( const vec3_t point, int passEntityNum );
void CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask );
void CG_PlayerTrace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
					 int skipNumber, int mask, int capsule, qboolean traceDeep );
void CG_PredictPlayerState( void );

//
// cg_ents.c
//
void CG_SetEntitySoundPosition( centity_t *cent );
void CG_AddPacketEntities( void );
void CG_Beam( centity_t *cent );
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out );


//
// cg_weapons.c
//
void CG_NextWeapon_f( void );
void CG_PrevWeapon_f( void );
void CG_UseLastWeapon_f( void );
void CG_HolsterWeapon_f( void );
void CG_DropWeapon_f( void );
void CG_ToggleItem_f( void );

void CG_FireWeapon( centity_t *cent );
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir, int soundType );
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum );
void CG_ShotgunFire( entityState_t *es );
void CG_Bullet( vec3_t origin, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum );

void CG_RailTrail( clientInfo_t *ci, vec3_t start, vec3_t end );
void CG_AddViewWeapon (playerState_t *ps);
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team );
int CG_WeaponCommandButtonBits();

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
					float radius, qboolean temporary );

//
// cg_localents.c
//
void	CG_InitLocalEntities( void );
localEntity_t	*CG_AllocLocalEntity( void );
void	CG_AddLocalEntities( void );

//
// cg_effects.c
//
localEntity_t *CG_SmokePuff( const vec3_t p,
				   const vec3_t vel,
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader );
void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing );
#ifdef MISSIONPACK
void CG_KamikazeEffect( vec3_t org );
void CG_ObeliskExplode( vec3_t org, int entityNum );
void CG_ObeliskPain( vec3_t org );
void CG_InvulnerabilityImpact( vec3_t org, vec3_t angles );
void CG_InvulnerabilityJuiced( vec3_t org );
void CG_LightningBoltBeam( vec3_t start, vec3_t end );
#endif
void CG_ScorePlum( int client, vec3_t org, int score );

void CG_GibPlayer( vec3_t playerOrigin );
void CG_BigExplode( vec3_t playerOrigin );

void CG_Bleed( vec3_t origin, int entityNum );

localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir,
								qhandle_t hModel, qhandle_t shader, int msec,
								qboolean isSprite );

//
// cg_snapshot.c
//
void CG_ProcessSnapshots( void );

//
// cg_info.c
//
void CG_LoadingString( const char *s );
void CG_LoadingClient( int clientNum );
void CG_DrawInformation( void );

//
// cg_scoreboard.c
//
qboolean CG_DrawOldScoreboard( void );
void CG_DrawOldTourneyScoreboard( void );

//
// cg_consolecmds.c
//
qboolean CG_ConsoleCommand( void );
void CG_InitConsoleCommands( void );

//
// cg_servercmds.c
//
void CG_ExecuteNewServerCommands( int latestSequence );
void CG_ParseServerinfo( void );
void CG_SetConfigValues( void );

//
// cg_playerstate.c
//
void CG_Respawn( void );
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops );
void CG_CheckChangedPredictableEvents( playerState_t *ps );

//
// cg_modelanim.c
//
void CG_ModelAnim( centity_t *cent );
void CG_BoneLocal2World(bone_t *b, vec3_t pos, vec3_t rot, vec3_t outPos, vec3_t outRot);
void CG_CentBoneLocal2World(bone_t *b, centity_t *cent, vec3_t outPos, vec3_t outRot);
void CG_CentBoneIndexLocal2World(int boneIndex, centity_t *cent, vec3_t outPos, vec3_t outRot);

//
// cg_viewmodelanim.c
//
void CG_RegisterItemName( int index, const char *str );
const char *CG_GetItemName( int index );
void CG_AddViewModelAnimAttachment( refEntity_t *ent, centity_t *cent );
void CG_ViewModelAnim();

//
// cg_eventSystem.c
//
void CG_InitEventSystem(); // called once from CG_Init
void CG_ProcessEventText(centity_t *ent, const char *eventText);
void CG_PostEvent(centity_t *ent, const char *eventText, int eventDelay);
void CG_ProcessPendingEvents(); // called every frame

//===============================================

typedef enum {
  SYSTEM_PRINT,
  CHAT_PRINT,
  TEAMCHAT_PRINT
} q3print_t;

void	CG_ClearParticles (void);
void	CG_AddParticles (void);
void	CG_ParticleSnow (qhandle_t pshader, vec3_t origin, vec3_t origin2, int turb, float range, int snum);
void	CG_ParticleSmoke (qhandle_t pshader, centity_t *cent);
void	CG_AddParticleShrapnel (localEntity_t *le);
void	CG_ParticleSnowFlurry (qhandle_t pshader, centity_t *cent);
void	CG_ParticleBulletDebris (vec3_t	org, vec3_t vel, int duration);
void	CG_ParticleSparks (vec3_t org, vec3_t vel, int duration, float x, float y, float speed);
void	CG_ParticleDust (centity_t *cent, vec3_t origin, vec3_t dir);
void	CG_ParticleMisc (qhandle_t pshader, vec3_t origin, int size, int duration, float alpha);
void	CG_ParticleExplosion (char *animStr, vec3_t origin, vec3_t vel, int duration, int sizeStart, int sizeEnd);
extern qboolean		initparticles;
int CG_NewParticleArea ( int num );

// cg_ubersound.c
void			CG_LoadUbersound( void );
ubersound_t*	CG_GetUbersound( const char *name );
const char*		CG_LoadMusic( const char *musicfile );

// cg_parsemsg.c
void CG_ParseCGMessage();
void CG_AddBulletTracers();
void CG_AddBulletImpacts();
void CG_HudDrawElements();

// cg_beams.c
void CG_InitBeams();
void CG_AddBeams();
void CG_CreateBeam(vec3_t start, vec3_t dir, int owner, qhandle_t hModel, float alpha, 
   float scale, int flags, float length, int life, qboolean create, vec3_t endpointvec,
   int min_offset, int max_offset, int overlap, int numSubdivisions, int delay,
   const char *beamshadername, byte modulate[4], int numspherebeams, float sphereradius,
   int toggledelay, float endalpha, int renderfx, const char *name);
void CG_Rope(centity_t *cent); // afaik that's not used in MoHAA, but who cares?
void CG_MultiBeam(centity_t *cent);

// cg_rain.c
void CG_InitRainEffect();
void CG_RainCSUpdated(int num, const char *str);
void CG_Rain(centity_t *cent);

// cg_specialfx.c
void CG_MeleeImpact(float *vStart, float *vEnd);
void CG_Footstep(char *szTagName, centity_t *ent, /*refEntity_t *pREnt,*/
	int iRunning, int iEquipment);

