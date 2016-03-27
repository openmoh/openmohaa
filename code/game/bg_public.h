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
// bg_public.h -- definitions shared by both the server game and client game modules

// because games can change separately from the main system version, we need a
// second version that must match between game and cgame

#ifndef __BG_PUBLIC_H__
#define __BG_PUBLIC_H__

#define	GAME_VERSION		"mohaa-base-1"

#define	DEFAULT_GRAVITY		800

#define	SCORE_NOT_PRESENT	-9999	// for the CS_SCORES[12] when only one player is present

#define	VOTE_TIME			30000	// 30 seconds before vote times out

#define	MINS_Z				0
#define  MINS_X               -15
#define  MINS_Y               -15
#define  MAXS_X               15
#define  MAXS_Y               15

#define	MINS_Z				   0		// IneQuation: bounding box and viewheights to match MoHAA
#define  MAXS_Z               96

#define DEAD_MINS_Z				32
#define CROUCH_MAXS_Z			49
#define DEFAULT_VIEWHEIGHT		82
#define CROUCH_RUN_VIEWHEIGHT	64
#define JUMP_START_VIEWHEIGHT	52
#define CROUCH_VIEWHEIGHT		48
#define PRONE_VIEWHEIGHT		16
#define DEAD_VIEWHEIGHT			8

//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
//

// CS_SERVERINFO and CS_SYSTEMINFO are defined in q_shared.h

#define	CS_MESSAGE				2		// from the map worldspawn's message field
#define	CS_SAVENAME				3		// current save
#define	CS_MOTD					4		// g_motd string for server message of the day
#define	CS_WARMUP				5		// server time when the match will be restarted

#define	CS_MUSIC				8		// MUSIC_NewSoundtrack(cs)
#define CS_FOGINFO				9		// cg.farplane_cull cg.farplane_distance cg.farplane_color[3]
#define CS_SKYINFO				10		// cg.sky_alpha cg.sky_portal

#define	CS_GAME_VERSION			11
#define	CS_LEVEL_START_TIME		12		// so the timer only shows the current level cgs.levelStartTime

#define CS_CURRENT_OBJECTIVE	13

#define CS_RAIN_DENSITY			14		// cg.rain
#define CS_RAIN_SPEED			15
#define CS_RAIN_SPEEDVARY		16
#define CS_RAIN_SLANT			17
#define CS_RAIN_LENGTH			18
#define CS_RAIN_MINDIST			19
#define CS_RAIN_WIDTH			20
#define CS_RAIN_SHADER			21
#define CS_RAIN_NUMSHADERS		22

#define CS_MATCHEND				26		// cgs.matchEndTime

#define	CS_MODELS				32
#define CS_OBJECTIVES			(CS_MODELS+MAX_MODELS) // 1056
#define	CS_SOUNDS				(CS_OBJECTIVES+MAX_OBJECTIVES) // 1076

#define CS_IMAGES				(CS_SOUNDS+MAX_SOUNDS) // 1588
#define MAX_IMAGES				64

#define CS_LIGHTSTYLES			(CS_IMAGES+MAX_IMAGES) //1652
#define CS_PLAYERS				(CS_LIGHTSTYLES+MAX_LIGHTSTYLES) // 1684

#define CS_WEAPONS				(CS_PLAYERS+MAX_CLIENTS) // su44 was here
#define CS_TEAMS				1876
#define CS_GENERAL_STRINGS		1877
#define CS_SPECTATORS			1878
#define CS_ALLIES				1879
#define CS_AXIS					1880
#define CS_SOUNDTRACK			1881

#define CS_TEAMINFO				1

#define CS_MAX					(CS_PARTICLES+MAX_LOCATIONS)
#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	GT_SINGLE_PLAYER,	// single player ffa
	GT_FFA,				// free for all
	GT_TEAM,			// team deathmatch
	GT_TEAM_ROUNDS,
	GT_OBJECTIVE,
	GT_MAX_GAME_TYPE
} gametype_t;

//
// scale to use when evaluating entityState_t::constantLight scale
//
#define CONSTANTLIGHT_RADIUS_SCALE 8

typedef enum { GENDER_MALE, GENDER_FEMALE, GENDER_NEUTER } gender_t;

// su44: vma indexes are sent as 4 bits
// see playerState_t::iViewModelAnim
typedef enum {
	VMA_IDLE,
	VMA_CHARGE,
	VMA_FIRE,
	VMA_FIRE_SECONDARY,
	VMA_RECHAMBER,
	VMA_RELOAD,
	VMA_RELOAD_SINGLE,
	VMA_RELOAD_END,
	VMA_PULLOUT,
	VMA_PUTAWAY,
	VMA_LADDERSTEP,
	VMA_ENABLE,
	VMA_CUSTOMANIMATION,
	VMA_NUMANIMATIONS
} viewmodelanim_t;

// su44: playerState_t::activeItems[8] slots
// they are sent as 16 bits
// TODO: find out rest of them
#define ITEM_AMMO		0	// current ammo
#define ITEM_WEAPON		1	// current mainhand weapon

/*
===================================================================================

PMOVE MODULE

The pmove code takes a player_state_t and a usercmd_t and generates a new player_state_t
and some other output data.  Used for local prediction on the client game and true
movement on the server game.
===================================================================================
*/

#define	MAX_CLIP_PLANES	   5
#define	MIN_WALK_NORMAL	   0.7f		   // can't walk on very steep slopes

#define	STEPSIZE		         18

#define  WATER_TURBO_SPEED    1.35f
#define  WATER_TURBO_TIME     1200
#define  MINIMUM_RUNNING_TIME 800
#define  MINIMUM_WATER_FOR_TURBO  90

#define	OVERCLIP		         1.001f

// su44: our pmtype_t enum must be the same
// as in MoHAA, because playerState_t::pm_type
// is send over the net and used by cgame
// for movement prediction
typedef enum {
	PM_NORMAL,		// can accelerate and turn
	PM_CLIMBWALL, // su44: I think it's used for ladders
	PM_NOCLIP,		// noclip movement
	PM_DEAD,		// no acceleration or turning, but free falling
} pmtype_t;

typedef enum {
	EV_NONE,

	EV_FALL_SHORT,
	EV_FALL_MEDIUM,
	EV_FALL_FAR,
	EV_FALL_FATAL,
	EV_TERMINAL_VELOCITY,

	EV_WATER_TOUCH,   // foot touches
	EV_WATER_LEAVE,   // foot leaves
	EV_WATER_UNDER,   // head touches
	EV_WATER_CLEAR,   // head leaves

	EV_LAST_PREDICTED      // just a marker point

	// events generated by non-players or never predicted
} entity_event_t;

#if 0
// pmove->pm_flags
#define	PMF_DUCKED				(1<<0)
#define	PMF_JUMP_HELD			(1<<1)
#define	PMF_BACKWARDS_JUMP		0 //(1<<3)	// go into backwards land
#define	PMF_BACKWARDS_RUN		0 //(1<<4)	// coast down to backwards run
#define	PMF_TIME_LAND			(1<<5)	// pm_time is time before rejump
#define	PMF_TIME_KNOCKBACK		(1<<6)	// pm_time is an air-accelerate only time
#define	PMF_TIME_WATERJUMP		(1<<8)	// pm_time is waterjump
#define	PMF_USE_ITEM_HELD		(1<<10)
#define PMF_GRAPPLE_PULL		(1<<11)	// pull towards grapple location
#define PMF_FOLLOW				(1<<12)	// spectate following another player
#define PMF_SCOREBOARD			(1<<13)	// spectate as a scoreboard
#define PMF_INVULEXPAND			(1<<14)	// invulnerability sphere set to full size
#define PMF_FROZEN				(1<<15)
#define PMF_NO_MOVE				(1<<16)
#define PMF_NO_PREDICTION		(1<<17)
#define PMF_NO_GRAVITY			(1<<18)
#define	PMF_LEVELEXIT			(1<<20)		// use camera view instead of ps view
#endif

// moh pm_flags
#define	PMF_DUCKED				(1<<0)
#define	PMF_VIEW_PRONE			(1<<1)
#define PMF_SPECTATING			(1<<4)
#define	PMF_RESPAWNED			(1<<5)
#define	PMF_NO_PREDICTION		(1<<6)
#define	PMF_FROZEN				(1<<7)
#define	PMF_INTERMISSION		(1<<8)
#define PMF_SPECTATE_FOLLOW		(1<<9)
#define	PMF_CAMERA_VIEW			(1<<9)		// use camera view instead of ps view
#define	PMF_NO_MOVE				(1<<10)
#define PMF_VIEW_DUCK_RUN		(1<<11)
#define	PMF_VIEW_JUMP_START		(1<<12)
#define	PMF_LEVELEXIT			(1<<13)
#define	PMF_NO_GRAVITY			(1<<14)
#define	PMF_NO_HUD				(1<<15)
#define	PMF_UNKNOWN				(1<<16)

// moveposflags
#define MPF_POSITION_STANDING	(1<<0)
#define MPF_POSITION_CROUCHING	(1<<1)
#define MPF_POSITION_PRONE		(1<<2)
#define MPF_POSITION_OFFGROUND	(1<<3)

#define MPF_MOVEMENT_WALKING	(1<<4)
#define MPF_MOVEMENT_RUNNING	(1<<5)
#define MPF_MOVEMENT_FALLING	(1<<6)

#define	MAXTOUCH	32

#define  MOVERESULT_NONE     0            // nothing blocking
#define  MOVERESULT_TURNED   1            // move blocked, but player turned to avoid it
#define  MOVERESULT_BLOCKED  2            // move blocked by slope or wall
#define  MOVERESULT_HITWALL  3            // player ran into wall

typedef struct {
	// state (in / out)
	playerState_t	*ps;

	// command (in)
	usercmd_t	cmd;
	int			tracemask;			// collide against these types of surfaces
	int			debugLevel;			// if set, diagnostic output will be printed
	qboolean	noFootsteps;		// if the game is setup for no footsteps by the server

	int			framecount;

	// results (out)
	int			numtouch;
	int			touchents[ MAXTOUCH ];

	int         moveresult;       // indicates whether 2the player's movement was blocked and how
	qboolean	stepped;

	int			pmoveEvent;			// events predicted on client side
	vec3_t		mins, maxs;			// bounding box size

	int			watertype;
	int			waterlevel;

	float		xyspeed;

	// for fixed msec Pmove
	int			pmove_fixed;
	int			pmove_msec;

	// callbacks to test the world
	// these will be different functions during game and cgame
	void		(*trace)( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask, int capsule, qboolean traceDeep );
	int			(*pointcontents)( const vec3_t point, int passEntityNum );
} pmove_t;

// if a full pmove isn't done on the client, you can just update the angles
void PM_GetMove( float *pfForward, float *pfRight );
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd );
void Pmove_GroundTrace( pmove_t *pmove );
void Pmove( pmove_t *pmove );
void PmoveAdjustAngleSettings( vec_t *vViewAngles, vec_t *vAngles, playerState_t *pPlayerState, entityState_t *pEntState );
void PmoveAdjustAngleSettings_Client( vec_t *vViewAngles, vec_t *vAngles, playerState_t *pPlayerState, entityState_t *pEntState );

//===================================================================================


// player_state->stats[] indexes
// NOTE: may not have more than 32
typedef enum {
	STAT_HEALTH,
	STAT_MAX_HEALTH,
	STAT_WEAPONS,
	STAT_EQUIPPED_WEAPON,
	STAT_AMMO,	
	STAT_MAXAMMO,
	STAT_CLIPAMMO,		// ammo in current weapon clip
	STAT_MAXCLIPAMMO,	// maxammo in current weapon clip
	STAT_INZOOM,
	STAT_CROSSHAIR,
	STAT_LAST_PAIN,		// Last amount of damage the player took
	STAT_UNUSED_2,
	STAT_BOSSHEALTH,	// if we are fighting a boss, how much health he currently has
	STAT_CINEMATIC,		// This is set when we go into cinematics
	STAT_ADDFADE,		// This is set when we need to do an addblend for the fade
	STAT_LETTERBOX,		// This is set to the fraction of the letterbox
	STAT_COMPASSNORTH,
	STAT_OBJECTIVELEFT,
	STAT_OBJECTIVERIGHT,
	STAT_OBJECTIVECENTER,
	STAT_TEAM,
	STAT_KILLS,
	STAT_DEATHS,
	STAT_UNUSED_3,
	STAT_UNUSED_4,
	STAT_HIGHEST_SCORE,
	STAT_ATTACKERCLIENT, 
	STAT_INFOCLIENT,
	STAT_INFOCLIENT_HEALTH,
	STAT_DAMAGEDIR,
	STAT_LAST_STAT
} statIndex_t;

#define MAX_LETTERBOX_SIZE        0x7fff

#define ITEM_NAME_AMMO				0
#define ITEM_NAME_WEAPON			1

#define STAT_DEAD_YAW 5 // su44: Is there a DEAD_YAW stat in MoHAA?

// player_state->persistant[] indexes
// these fields are the only part of player_state that isn't
// cleared on respawn
// NOTE: may not have more than 16
typedef enum { 
	PERS_SCORE,		// !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
	PERS_TEAM		// player team
} persEnum_t;

// entityState_t->eFlags
// su44: eFlags used in MoHAA
#define EF_CLIMBWALL		0x00000010
#define EF_UNARMED			0x00000020		// su44: this player has his weapon holstered
#define EF_LINKANGLES		0x00000040
#define EF_ALLIES			0x00000080		// su44: this player is in allies team
#define EF_AXIS				0x00000100		// su44: this player is in axis team
#define EF_DEAD				0x00000200		// don't draw a foe marker over players with EF_DEAD


// su44: q3 remnants
#define EF_TICKING					0x00000002		// used to make players play the prox mine ticking sound
#define	EF_TELEPORT_BIT				0x00000004		// toggled every time the origin abruptly changes
#define	EF_AWARD_EXCELLENT			0x00000008		// draw an excellent sprite
#define EF_PLAYER_EVENT				0x00000010
#define	EF_BOUNCE					0x00000010		// for missiles
#define	EF_AWARD_GAUNTLET			0x00000040		// draw a gauntlet sprite
#define	EF_MOVER_STOP				0x00000400		// will push otherwise
#define EF_AWARD_CAP				0x00000800		// draw the capture sprite
#define	EF_TALK						0x00001000		// draw a talk balloon
#define	EF_CONNECTION				0x00002000		// draw a connection trouble sprite
#define	EF_VOTED					0x00004000		// already cast a vote
#define	EF_AWARD_IMPRESSIVE			0x00008000		// draw an impressive sprite
#define	EF_AWARD_DEFEND				0x00010000		// draw a defend sprite
#define	EF_AWARD_ASSIST				0x00020000		// draw a assist sprite
#define EF_AWARD_DENIED				0x00040000		// denied
#define EF_TEAMVOTED				0x00080000		// already cast a team vote
#define	EF_EVERYFRAME				0x00100000		// def commands will be run every client frame
#define	EF_ANTISBJUICE				0x00200000		// anti sucknblow juice
#define EF_DONT_PROCESS_COMMANDS	0x00400000		// don't process client commands for this entity

// these defines could be deleted sometime when code/game/ is cleared of Q3A stuff
#define TEAM_FREE 0
#define TEAM_RED 4
#define TEAM_BLUE 3
#define TEAM_NUM_TEAMS 5

//===================================================================================

// flip the togglebit every time an animation
// changes so a restart of the same anim can be detected
#define	ANIM_TOGGLEBIT		(1<<9)
#define	ANIM_BLEND        (1<<10)
#define  ANIM_NUMBITS      11

// server side anim bits
#define  ANIM_SERVER_EXITCOMMANDS_PROCESSED (1<<12)

#define  ANIM_MASK         ( ~( ANIM_TOGGLEBIT | ANIM_BLEND | ANIM_SERVER_EXITCOMMANDS_PROCESSED ) )

// if FRAME_EXPLICIT is set, don't auto animate
#define	FRAME_EXPLICIT	   512
#define  FRAME_MASK        ( ~FRAME_EXPLICIT )

//
// Tag specific flags
//
#define TAG_NUMBITS        10                    // number of bits required to send over network
#define TAG_MASK           ( ( 1 << 10 ) - 1 )


//
// Camera Flags
//
#define CF_CAMERA_ANGLES_ABSOLUTE		( 1 << 0 )
#define CF_CAMERA_ANGLES_IGNORE_PITCH	( 1 << 1 )
#define CF_CAMERA_ANGLES_IGNORE_YAW		( 1 << 2 )
#define CF_CAMERA_ANGLES_ALLOWOFFSET	( 1 << 3 )
#define CF_CAMERA_ANGLES_TURRETMODE		( 1 << 4 )
#define CF_CAMERA_CUT_BIT				( 1 << 7 ) // this bit gets toggled everytime we do a hard camera cut

typedef enum {
	TEAM_NONE,
	TEAM_SPECTATOR,
	TEAM_FREEFORALL,
	TEAM_ALLIES,
	TEAM_AXIS
} teamtype_t;

// How many players on the overlay
#define TEAM_MAXOVERLAY		32

// means of death
// su44: changed to MoHAA's
typedef enum {
	MOD_NONE,
	MOD_SUICIDE,
	MOD_CRUSH,
	MOD_CRUSH_EVERY_FRAME,
	MOD_TELEFRAG,
	MOD_LAVA,
	MOD_SLIME,
	MOD_FALLING,
	MOD_LAST_SELF_INFLICTED,
	MOD_EXPLOSION,
	MOD_EXPLODEWALL,
	MOD_ELECTRIC,
	MOD_ELECTRICWATER,
	MOD_THROWNOBJECT,
	MOD_GRENADE,
	MOD_BEAM,
	MOD_ROCKET,
	MOD_IMPACT,
	MOD_BULLET,
	MOD_FAST_BULLET,
	MOD_VEHICLE,
	MOD_FIRE,
	MOD_FLASHBANG,
	MOD_ON_FIRE,
	MOD_GIB,
	MOD_IMPALE,
	MOD_BASH,
	MOD_SHOTGUN,
	MOD_TOTAL_NUMBER,

} meansOfDeath_t;

extern const char *means_of_death_strings[];

//---------------------------------------------------------

// g_dmflags->integer flags
#define DF_NO_HEALTH				(1<<0)
#define DF_NO_POWERUPS   		(1<<1)
#define DF_WEAPONS_STAY			(1<<2)
#define DF_NO_FALLING			(1<<3)
#define DF_INSTANT_ITEMS		(1<<4)
#define DF_SAME_LEVEL			(1<<5)
#define DF_SKINTEAMS				(1<<6)
#define DF_MODELTEAMS			(1<<7)
#define DF_FRIENDLY_FIRE		(1<<8)
#define DF_SPAWN_FARTHEST		(1<<9)
#define DF_FORCE_RESPAWN		(1<<10)
#define DF_NO_ARMOR				(1<<11)
#define DF_FAST_WEAPONS			(1<<12)
#define DF_NOEXIT             (1<<13)
#define DF_INFINITE_AMMO      (1<<14)
#define DF_FIXED_FOV          (1<<15)
#define DF_NO_DROP_WEAPONS    (1<<16)
#define DF_NO_FOOTSTEPS			(1<<17)

#define DM_FLAG( flag ) ( g_gametype->integer && ( ( int )dmflags->integer & ( flag ) ) )

// content masks
#define	MASK_ALL				(-1)
#define	MASK_SOLID				(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_UNKNOWN2|CONTENTS_NOBOTCLIP|CONTENTS_BBOX)
#define	MASK_USABLE				(CONTENTS_SOLID|CONTENTS_BODY)
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY|CONTENTS_TRIGGER|CONTENTS_FENCE|CONTENTS_UNKNOWN2|CONTENTS_NOBOTCLIP|CONTENTS_NOTTEAM2)
#define	MASK_DEADSOLID			(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_CORPSE|CONTENTS_NOTTEAM2|CONTENTS_FENCE)
#define	MASK_MONSTERSOLID		(CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_BODY)
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA)
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_TRIGGER|CONTENTS_BODY|CONTENTS_FENCE|CONTENTS_WEAPONCLIP|CONTENTS_UNKNOWN2|CONTENTS_NOBOTCLIP|CONTENTS_WATER|CONTENTS_NOTTEAM1|CONTENTS_NOTTEAM2)
#define	MASK_PROJECTILE			(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_TRIGGER|CONTENTS_FENCE|CONTENTS_WEAPONCLIP|CONTENTS_UNKNOWN2|CONTENTS_NOBOTCLIP|CONTENTS_BBOX)
#define	MASK_MELEE     			(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_TRIGGER|CONTENTS_WEAPONCLIP|CONTENTS_FENCE|CONTENTS_UNKNOWN2|CONTENTS_NOBOTCLIP|CONTENTS_BBOX|CONTENTS_NOTTEAM1)
#define	MASK_PATHSOLID			(CONTENTS_SOLID|CONTENTS_TRIGGER|CONTENTS_BODY|CONTENTS_FENCE|CONTENTS_UNKNOWN2|CONTENTS_BBOX|CONTENTS_MONSTERCLIP)
#define	MASK_CAMERASOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY|MASK_WATER)
#define MASK_BEAM				(CONTENTS_SOLID|CONTENTS_TRIGGER|CONTENTS_PLAYERCLIP|CONTENTS_BODY|CONTENTS_FENCE)
#define MASK_LADDER				(CONTENTS_SOLID|CONTENTS_LADDER|CONTENTS_TRIGGER|CONTENTS_PLAYERCLIP|CONTENTS_BODY|CONTENTS_FENCE)
#define MASK_AUTOCALCLIFE		(CONTENTS_SOLID|CONTENTS_TRIGGER|CONTENTS_FENCE)
#define MASK_EXPLOSION			(CONTENTS_SOLID|CONTENTS_TRIGGER|CONTENTS_WEAPONCLIP)
#define MASK_SOUND				(CONTENTS_SOLID|CONTENTS_TRANSLUCENT)
#define MASK_VEHICLE			(CONTENTS_SOLID|CONTENTS_TRIGGER|CONTENTS_VEHICLECLIP|CONTENTS_FENCE)
#define MASK_CLICK				(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_UNKNOWN3|CONTENTS_UNKNOWN2|CONTENTS_NOBOTCLIP|CONTENTS_BBOX)
#define MASK_CANSEE				(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_WEAPONCLIP|CONTENTS_UNKNOWN2|CONTENTS_NOBOTCLIP|CONTENTS_SLIME|CONTENTS_LAVA|CONTENTS_BBOX)
#define MASK_ITEM				(CONTENTS_SOLID|CONTENTS_TRIGGER|CONTENTS_PLAYERCLIP|CONTENTS_FENCE)
#define MASK_TRANSITION			(CONTENTS_SOLID|CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_UNKNOWN2|CONTENTS_NOTTEAM1|CONTENTS_WEAPONCLIP)
#define MASK_TARGETPATH			(CONTENTS_SOLID|CONTENTS_TRIGGER|CONTENTS_MONSTERCLIP|CONTENTS_FENCE|CONTENTS_UNKNOWN2|CONTENTS_BBOX)
#define MASK_LINE				(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_UNKNOWN2|CONTENTS_NOBOTCLIP|CONTENTS_BBOX)
#define MASK_VEHICLETURRET		(CONTENTS_SOLID|CONTENTS_TRIGGER|CONTENTS_FENCE|CONTENTS_VEHICLECLIP|CONTENTS_MONSTERCLIP|CONTENTS_PLAYERCLIP)

// mohaa mask
/*
#define MASK_ALL							-1
#define MASK_SOLID							1

#define MASK_COLLISION						0x26000B21
#define MASK_PERMANENTMARK					0x40000001
#define MASK_AUTOCALCLIFE					0x40002021
#define MASK_EXPLOSION						0x40040001
#define MASK_TREADMARK						0x42012B01
#define MASK_THIRDPERSON					0x42012B39
#define MASK_FOOTSTEP						0x42022901
#define MASK_BEAM							0x42042B01
#define MASK_VISIBLE						0x42042B01
#define MASK_VEHICLE						0x42042B01
#define MASK_BULLET							0x42042B21
#define MASK_SHOT							0x42042BA1
#define MASK_CROSSHAIRSHADER				0x42092B01
#define MASK_TRACER							0x42142B21
*/

//
// entityState_t->eType
//
typedef enum {
	ET_MODELANIM_SKEL,
	ET_MODELANIM,
	ET_VEHICLE,
	ET_PLAYER,
	ET_ITEM,
	ET_GENERAL,
	ET_MISSILE,
	ET_MOVER,
	ET_BEAM,
	ET_MULTIBEAM,
	ET_PORTAL,
	ET_EVENT_ONLY,
	ET_RAIN,
	ET_LEAF,
	ET_SPEAKER,
	ET_PUSH_TRIGGER,
	ET_TELEPORT_TRIGGER,
	ET_DECAL,
	ET_EMITTER,
	ET_ROPE,
	ET_EVENTS,
	ET_EXEC_COMMANDS,
	ET_SPRITE
} entityType_t;

// su44; yes, I know there is no q3 trajectory_t in MoHAA,
// but I need it for fgame rotating doors code.
void	BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result );
void	BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result );

void	BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap );
void	BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, qboolean snap );

#ifdef __cplusplus
}
#endif

#endif /* bg_public.h */
