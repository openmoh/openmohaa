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

#pragma once

#define GAME_VERSION "mohaa-base-1"

static const unsigned int MINS_X = -15;
static const unsigned int MINS_Y = -15;
static const unsigned int MINS_Z = 0; // IneQuation: bounding box and viewheights to match MoHAA
static const unsigned int MAXS_X = 15;
static const unsigned int MAXS_Y = 15;
static const unsigned int MAXS_Z = 96;

static const unsigned int DEAD_MINS_Z           = 32;
static const unsigned int CROUCH_MAXS_Z         = 49;
static const unsigned int DEFAULT_VIEWHEIGHT    = 82;
static const unsigned int CROUCH_RUN_VIEWHEIGHT = 64;
static const unsigned int JUMP_START_VIEWHEIGHT = 52;
static const unsigned int CROUCH_VIEWHEIGHT     = 48;
static const unsigned int PRONE_VIEWHEIGHT      = 16;
static const unsigned int DEAD_VIEWHEIGHT       = 8;

//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
//

// CS_SERVERINFO and CS_SYSTEMINFO are defined in q_shared.h

#define CS_MESSAGE  2 // from the map worldspawn's message field
#define CS_SAVENAME 3 // current save
#define CS_MOTD     4 // g_motd string for server message of the day
#define CS_WARMUP   5 // server time when the match will be restarted

// 2 values were removed in team tactics
#define CS_MUSIC             6 // MUSIC_NewSoundtrack(cs)
#define CS_FOGINFO           7 // cg.farplane_cull cg.farplane_distance cg.farplane_color[3]
#define CS_SKYINFO           8 // cg.sky_alpha cg.sky_portal

#define CS_GAME_VERSION      9
#define CS_LEVEL_START_TIME  10 // so the timer only shows the current level cgs.levelStartTime

#define CS_CURRENT_OBJECTIVE 11

#define CS_RAIN_DENSITY      12 // cg.rain
#define CS_RAIN_SPEED        13
#define CS_RAIN_SPEEDVARY    14
#define CS_RAIN_SLANT        15
#define CS_RAIN_LENGTH       16
#define CS_RAIN_MINDIST      17
#define CS_RAIN_WIDTH        18
#define CS_RAIN_SHADER       19
#define CS_RAIN_NUMSHADERS   20

//
// Team tactics
//
#define CS_VOTE_TIME      21
#define CS_VOTE_STRING    22
#define CS_VOTE_YES       23
#define CS_VOTE_NO        24
#define CS_VOTE_UNDECIDED 25
//

#define CS_MATCHEND        26 // cgs.matchEndTime

#define CS_MODELS          32
#define CS_OBJECTIVES      (CS_MODELS + MAX_MODELS)         // 1056
#define CS_SOUNDS          (CS_OBJECTIVES + MAX_OBJECTIVES) // 1076

#define CS_IMAGES          (CS_SOUNDS + MAX_SOUNDS) // 1588
#define MAX_IMAGES         64

#define CS_LIGHTSTYLES     (CS_IMAGES + MAX_IMAGES)           //1652
#define CS_PLAYERS         (CS_LIGHTSTYLES + MAX_LIGHTSTYLES) // 1684

#define CS_WEAPONS         (CS_PLAYERS + MAX_CLIENTS) // su44 was here
#define CS_TEAMS           1892
#define CS_GENERAL_STRINGS 1893
#define CS_SPECTATORS      1894
#define CS_ALLIES          1895
#define CS_AXIS            1896
#define CS_SOUNDTRACK      1881

#define CS_TEAMINFO        1

#define CS_MAX             (CS_PARTICLES + MAX_LOCATIONS)
#if (CS_MAX) > MAX_CONFIGSTRINGS
#    error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum {
        GT_SINGLE_PLAYER, // single player
        GT_FFA,           // free for all
        GT_TEAM,          // team deathmatch
        GT_TEAM_ROUNDS,
        GT_OBJECTIVE,
        // Team Assault game mode
        GT_TOW,
        // Team Tactics game mode
        GT_LIBERATION,
        GT_MAX_GAME_TYPE
    } gametype_t;

    //
    // scale to use when evaluating entityState_t::constantLight scale
    //
    static const float CONSTANTLIGHT_RADIUS_SCALE = 8;

    typedef enum {
        GENDER_MALE,
        GENDER_FEMALE,
        GENDER_NEUTER
    } gender_t;

    enum vmAnim_e {
        VM_ANIM_DISABLED,
        VM_ANIM_IDLE,
        VM_ANIM_CHARGE,
        VM_ANIM_FIRE,
        VM_ANIM_FIRE_SECONDARY,
        VM_ANIM_RECHAMBER,
        VM_ANIM_RELOAD,
        VM_ANIM_RELOAD_SINGLE,
        VM_ANIM_RELOAD_END,
        VM_ANIM_PULLOUT,
        VM_ANIM_PUTAWAY,
        VM_ANIM_LADDERSTEP,
        VM_ANIM_IDLE_0,
        VM_ANIM_IDLE_1,
        VM_ANIM_IDLE_2,
    };

    // su44: playerState_t::activeItems[8] slots
    // they are sent as 16 bits
    // TODO: find out rest of them
    enum itemtype_e {
        ITEM_AMMO,
        ITEM_WEAPON
    };

    /*
===================================================================================

PMOVE MODULE

The pmove code takes a player_state_t and a usercmd_t and generates a new player_state_t
and some other output data.  Used for local prediction on the client game and true
movement on the server game.
===================================================================================
*/

    static const unsigned int MAX_CLIP_PLANES = 5;
    static const float        MIN_WALK_NORMAL = 0.7f; // can't walk on very steep slopes

    static const float STEPSIZE = 18;

    static const float OVERCLIP = 1.001f;

    // su44: our pmtype_t enum must be the same
    // as in MoHAA, because playerState_t::pm_type
    // is send over the net and used by cgame
    // for movement prediction
    typedef enum {
        PM_NORMAL,    // can accelerate and turn
        PM_CLIMBWALL, // su44: I think it's used for ladders
        PM_NOCLIP,    // noclip movement
        PM_DEAD,      // no acceleration or turning, but free falling
    } pmtype_t;

    typedef enum {
        EV_NONE,

        EV_FALL_SHORT,
        EV_FALL_MEDIUM,
        EV_FALL_FAR,
        EV_FALL_FATAL,
        EV_TERMINAL_VELOCITY,

        EV_WATER_TOUCH, // foot touches
        EV_WATER_LEAVE, // foot leaves
        EV_WATER_UNDER, // head touches
        EV_WATER_CLEAR, // head leaves

        EV_LAST_PREDICTED // just a marker point

        // events generated by non-players or never predicted
    } entity_event_t;

#if 0
// pmove->pm_flags
#    define PMF_DUCKED         (1 << 0)
#    define PMF_JUMP_HELD      (1 << 1)
#    define PMF_BACKWARDS_JUMP 0        //(1<<3)	// go into backwards land
#    define PMF_BACKWARDS_RUN  0        //(1<<4)	// coast down to backwards run
#    define PMF_TIME_LAND      (1 << 5) // pm_time is time before rejump
#    define PMF_TIME_KNOCKBACK (1 << 6) // pm_time is an air-accelerate only time
#    define PMF_TIME_WATERJUMP (1 << 8) // pm_time is waterjump
#    define PMF_USE_ITEM_HELD  (1 << 10)
#    define PMF_GRAPPLE_PULL   (1 << 11) // pull towards grapple location
#    define PMF_FOLLOW         (1 << 12) // spectate following another player
#    define PMF_SCOREBOARD     (1 << 13) // spectate as a scoreboard
#    define PMF_INVULEXPAND    (1 << 14) // invulnerability sphere set to full size
#    define PMF_FROZEN         (1 << 15)
#    define PMF_NO_MOVE        (1 << 16)
#    define PMF_NO_PREDICTION  (1 << 17)
#    define PMF_NO_GRAVITY     (1 << 18)
#    define PMF_LEVELEXIT      (1 << 20) // use camera view instead of ps view
#endif

// moh pm_flags
#define PMF_DUCKED        (1 << 0)
#define PMF_VIEW_PRONE    (1 << 1)
#define PMF_DAMAGE_ANGLES (1 << 1) // in protocol >= 15
//
// 2 Flags removed in team tactics
//
#define PMF_SPECTATING      (1 << 2)
#define PMF_RESPAWNED       (1 << 3)
#define PMF_NO_PREDICTION   (1 << 4)
#define PMF_FROZEN          (1 << 5)
#define PMF_INTERMISSION    (1 << 6)
#define PMF_SPECTATE_FOLLOW (1 << 7)
#define PMF_CAMERA_VIEW     (1 << 7) // use camera view instead of ps view
#define PMF_NO_MOVE         (1 << 8)
#define PMF_VIEW_DUCK_RUN   (1 << 9)
#define PMF_VIEW_JUMP_START (1 << 10)
#define PMF_LEVELEXIT       (1 << 11)
#define PMF_NO_GRAVITY      (1 << 12)
#define PMF_NO_HUD          (1 << 13)
#define PMF_UNKNOWN         (1 << 14)
#define PMF_NO_LEAN         (1 << 15)

    // moveposflags
    enum moveposflags_e {
        MPF_POSITION_STANDING  = (1 << 0),
        MPF_POSITION_CROUCHING = (1 << 1),
        MPF_POSITION_PRONE     = (1 << 2),
        MPF_POSITION_OFFGROUND = (1 << 3),
        MPF_MOVEMENT_WALKING   = (1 << 4),
        MPF_MOVEMENT_RUNNING   = (1 << 5),
        MPF_MOVEMENT_FALLING   = (1 << 6),
    };

#define MAXTOUCH 32

    enum moveresult_e {
        MOVERESULT_NONE,    // nothing blocking
        MOVERESULT_TURNED,  // move blocked, but player turned to avoid it
        MOVERESULT_BLOCKED, // move blocked by slope or wall
        MOVERESULT_HITWALL  // player ran into wall
    };

    typedef struct {
        // state (in / out)
        playerState_t *ps;

        // command (in)
        usercmd_t cmd;
        int       tracemask;   // collide against these types of surfaces
        int       debugLevel;  // if set, diagnostic output will be printed
        qboolean  noFootsteps; // if the game is setup for no footsteps by the server
        qboolean  alwaysAllowLean;

        int framecount;

        // results (out)
        int numtouch;
        int touchents[MAXTOUCH];

        int      moveresult; // indicates whether 2the player's movement was blocked and how
        qboolean stepped;

        int    pmoveEvent; // events predicted on client side
        vec3_t mins, maxs; // bounding box size

        int watertype;
        int waterlevel;

        float xyspeed;

        // for fixed msec Pmove
        int pmove_fixed;
        int pmove_msec;

        // callbacks to test the world
        // these will be different functions during game and cgame
        void (*trace)(
            trace_t     *results,
            const vec3_t start,
            const vec3_t mins,
            const vec3_t maxs,
            const vec3_t end,
            int          passEntityNum,
            int          contentMask,
            int          capsule,
            qboolean     traceDeep
        );
        int (*pointcontents)(const vec3_t point, int passEntityNum);

        //
        // Openmohaa addition
        //
        int   protocol;
        float leanSpeed;
        float leanRecoverSpeed;
        float leanAdd;
        float leanMax;
    } pmove_t;

    // if a full pmove isn't done on the client, you can just update the angles
    void PM_GetMove(float *pfForward, float *pfRight);
    void PM_UpdateViewAngles(playerState_t *ps, const usercmd_t *cmd);
    void Pmove_GroundTrace(pmove_t *pmove);
    void Pmove(pmove_t *pmove);
    void
    PmoveAdjustAngleSettings(vec_t *vViewAngles, vec_t *vAngles, playerState_t *pPlayerState, entityState_t *pEntState);
    void PmoveAdjustAngleSettings_Client(
        vec_t *vViewAngles, vec_t *vAngles, playerState_t *pPlayerState, entityState_t *pEntState
    );

    //===================================================================================

    static const unsigned int MAX_LETTERBOX_SIZE = 0x7fff;

    enum weaponclass_e {
        WEAPON_CLASS_PISTOL  = (1 << 0),
        WEAPON_CLASS_RIFLE   = (1 << 1),
        WEAPON_CLASS_SMG     = (1 << 2),
        WEAPON_CLASS_MG      = (1 << 3),
        WEAPON_CLASS_GRENADE = (1 << 4),
        WEAPON_CLASS_HEAVY   = (1 << 5),
        WEAPON_CLASS_CANNON  = (1 << 6),
        WEAPON_CLASS_ITEM    = (1 << 7),
        WEAPON_CLASS_ITEM1   = (1 << 8),
        WEAPON_CLASS_ITEM2   = (1 << 9),
        WEAPON_CLASS_ITEM3   = (1 << 10),
        WEAPON_CLASS_ITEM4   = (1 << 11),
        WEAPON_CLASS_ANY_ITEM =
            (WEAPON_CLASS_ITEM | WEAPON_CLASS_ITEM1 | WEAPON_CLASS_ITEM2 | WEAPON_CLASS_ITEM3 | WEAPON_CLASS_ITEM4),
        WEAPON_CLASS_ITEM_SLOT_BITS =
            (WEAPON_CLASS_ITEM1 | WEAPON_CLASS_ITEM2 | WEAPON_CLASS_ITEM3 | WEAPON_CLASS_ITEM4),
        WEAPON_CLASS_PRIMARY   = (!(WEAPON_CLASS_PISTOL | WEAPON_CLASS_GRENADE)),
        WEAPON_CLASS_SECONDARY = (WEAPON_CLASS_PISTOL | WEAPON_CLASS_GRENADE),
        WEAPON_CLASS_THROWABLE = (WEAPON_CLASS_GRENADE | WEAPON_CLASS_ITEM),
        WEAPON_CLASS_MISC =
            (WEAPON_CLASS_CANNON | WEAPON_CLASS_ITEM | WEAPON_CLASS_ITEM1 | WEAPON_CLASS_ITEM2 | WEAPON_CLASS_ITEM3
             | WEAPON_CLASS_ITEM4),
        WEAPON_CLASS_ITEMINDEX = (WEAPON_CLASS_ITEM1 | WEAPON_CLASS_ITEM2 | WEAPON_CLASS_ITEM3 | WEAPON_CLASS_ITEM4)
    };

    // player_state->persistant[] indexes
    // these fields are the only part of player_state that isn't
    // cleared on respawn
    // NOTE: may not have more than 16
    typedef enum {
        PERS_SCORE, // !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
        PERS_TEAM   // player team
    } persEnum_t;

// entityState_t->eFlags
// su44: eFlags used in MoHAA
#define EF_EVERYFRAME 0x00000008
#define EF_CLIMBWALL  0x00000010
#define EF_UNARMED    0x00000020 // su44: this player has his weapon holstered
#define EF_LINKANGLES 0x00000040
#define EF_ALLIES     0x00000080 // su44: this player is in allies team
#define EF_AXIS       0x00000100 // su44: this player is in axis team
#define EF_ANY_TEAM   (EF_ALLIES | EF_AXIS)
#define EF_DEAD       0x00000200 // don't draw a foe marker over players with EF_DEAD
#define EF_NO_LERP    0x00001000 // don't lerp animations

//
// Team assault flags
//
#define EF_PLAYER_IN_MENU   0x00000400 // Player is in menu
#define EF_PLAYER_TALKING   0x00000800 // Player is talking
#define EF_PLAYER_ARTILLERY 0x00002000 // Player is using an artillery

// su44: q3 remnants
#define EF_TICKING               0x00000002 // used to make players play the prox mine ticking sound
#define EF_TELEPORT_BIT          0x00000004 // toggled every time the origin abruptly changes
#define EF_AWARD_EXCELLENT       0x00000008 // draw an excellent sprite
#define EF_PLAYER_EVENT          0x00000010
#define EF_BOUNCE                0x00000010 // for missiles
#define EF_AWARD_GAUNTLET        0x00000040 // draw a gauntlet sprite
#define EF_MOVER_STOP            0x00000400 // will push otherwise
#define EF_AWARD_CAP             0x00000800 // draw the capture sprite
#define EF_TALK                  0x00001000 // draw a talk balloon
#define EF_CONNECTION            0x00002000 // draw a connection trouble sprite
#define EF_VOTED                 0x00004000 // already cast a vote
#define EF_AWARD_IMPRESSIVE      0x00008000 // draw an impressive sprite
#define EF_AWARD_DEFEND          0x00010000 // draw a defend sprite
#define EF_AWARD_ASSIST          0x00020000 // draw a assist sprite
#define EF_AWARD_DENIED          0x00040000 // denied
#define EF_TEAMVOTED             0x00080000 // already cast a team vote
#define EF_ANTISBJUICE           0x00200000 // anti sucknblow juice
#define EF_DONT_PROCESS_COMMANDS 0x00400000 // don't process client commands for this entity

//===================================================================================

//
// Tag specific flags
//
#define TAG_NUMBITS 10 // number of bits required to send over network
#define TAG_MASK    ((1 << 10) - 1)

//
// Camera Flags
//
#define CF_CAMERA_ANGLES_ABSOLUTE     (1 << 0)
#define CF_CAMERA_ANGLES_IGNORE_PITCH (1 << 1)
#define CF_CAMERA_ANGLES_IGNORE_YAW   (1 << 2)
#define CF_CAMERA_ANGLES_ALLOWOFFSET  (1 << 3)
#define CF_CAMERA_ANGLES_TURRETMODE   (1 << 4)
#define CF_CAMERA_CUT_BIT             (1 << 7) // this bit gets toggled everytime we do a hard camera cut

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
        //
        // Team assault
        //
        MOD_AAGUN,
        //
        // Team tactics
        //
        MOD_LANDMINE,
        MOD_TOTAL_NUMBER,

    } meansOfDeath_t;

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
        ET_EXEC_COMMANDS
    } entityType_t;

    // player_state->stats[] indexes
    // NOTE: may not have more than 32
    typedef enum {
        STAT_HEALTH,
        STAT_MAXHEALTH,
        STAT_WEAPONS,
        STAT_EQUIPPED_WEAPON,
        STAT_AMMO,
        STAT_MAXAMMO,
        STAT_CLIPAMMO,
        STAT_MAXCLIPAMMO,
        STAT_INZOOM,
        STAT_CROSSHAIR,
        STAT_LAST_PAIN,
        STAT_NEXTHEALTH, // was unused until 2.0
        STAT_BOSSHEALTH,
        STAT_CINEMATIC,
        STAT_ADDFADE,
        STAT_LETTERBOX,
        STAT_COMPASSNORTH,
        STAT_OBJECTIVELEFT,
        STAT_OBJECTIVERIGHT,
        STAT_OBJECTIVECENTER,
        STAT_TEAM,
        STAT_KILLS,
        STAT_DEATHS,
        STAT_VEHICLE_HEALTH,     // was unused until 2.0
        STAT_VEHICLE_MAX_HEALTH, // was unused until 2.0
        STAT_HIGHEST_SCORE,
        STAT_ATTACKERCLIENT,
        STAT_INFOCLIENT,
        STAT_INFOCLIENT_HEALTH,
        STAT_DAMAGEDIR,
        STAT_SECONDARY_AMMO, // added in 2.0
        STAT_LAST_STAT
    } playerstat_t;

    extern const char *means_of_death_strings[];

    typedef enum {
        TEAM_NONE,
        TEAM_SPECTATOR,
        TEAM_FREEFORALL,
        TEAM_ALLIES,
        TEAM_AXIS
    } teamtype_t;

    typedef enum {
        SWT_NORMAL,
        SWT_FUSE,
        SWT_FUSE_WET
    } stopWatchType_t;

//---------------------------------------------------------

// g_dmflags->integer flags
#define DF_NO_HEALTH              (1 << 0)
#define DF_NO_POWERUPS            (1 << 1)
#define DF_WEAPONS_STAY           (1 << 2)
#define DF_NO_FALLING             (1 << 3)
#define DF_INSTANT_ITEMS          (1 << 4)
#define DF_SAME_LEVEL             (1 << 5)
#define DF_SKINTEAMS              (1 << 6)
#define DF_MODELTEAMS             (1 << 7)
#define DF_FRIENDLY_FIRE          (1 << 8)
#define DF_SPAWN_FARTHEST         (1 << 9)
#define DF_FORCE_RESPAWN          (1 << 10)
#define DF_NO_ARMOR               (1 << 11)
#define DF_FAST_WEAPONS           (1 << 12)
#define DF_NOEXIT                 (1 << 13)
#define DF_INFINITE_AMMO          (1 << 14)
#define DF_FIXED_FOV              (1 << 15)
#define DF_NO_DROP_WEAPONS        (1 << 16)
#define DF_NO_FOOTSTEPS           (1 << 17)
#define DF_ALLOW_LEAN_MOVEMENT    (1 << 18)
#define DF_OLD_SNIPER             (1 << 19)
#define DF_DISALLOW_KAR98_MORTAR  (1 << 20)

#define DF_WEAPON_LANDMINE_ALWAYS (1 << 21)
#define DF_WEAPON_NO_RIFLE        (1 << 22)
#define DF_WEAPON_NO_SNIPER       (1 << 23)
#define DF_WEAPON_NO_SMG          (1 << 24)
#define DF_WEAPON_NO_MG           (1 << 25)
#define DF_WEAPON_NO_ROCKET       (1 << 26)
#define DF_WEAPON_NO_SHOTGUN      (1 << 27)
#define DF_WEAPON_NO_LANDMINE     (1 << 28)

#define DM_FLAG(flag)             (g_gametype->integer != GT_SINGLE_PLAYER && ((int)dmflags->integer & (flag)))

// content masks
#define MASK_ALL (-1)
#define MASK_SOLID \
    (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_UNKNOWN2 | CONTENTS_NOBOTCLIP | CONTENTS_BBOX | CONTENTS_FENCE)
#define MASK_SAFESOLID (CONTENTS_BODY | CONTENTS_UNKNOWN2 | CONTENTS_NOBOTCLIP | CONTENTS_BBOX)
#define MASK_USABLE    (CONTENTS_SOLID | CONTENTS_BBOX | CONTENTS_NOBOTCLIP | CONTENTS_UNKNOWN2 | CONTENTS_BODY)
#define MASK_PLAYERSOLID                                                                                          \
    (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_BODY | CONTENTS_TRIGGER | CONTENTS_FENCE | CONTENTS_UNKNOWN2 \
     | CONTENTS_NOBOTCLIP | CONTENTS_BBOX)
#define MASK_GUNTARGET                                                                                               \
    (CONTENTS_SOLID | CONTENTS_LADDER | CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_CLAYPIDGEON | CONTENTS_BBOX        \
     | CONTENTS_NOBOTCLIP | CONTENTS_UNKNOWN2 | CONTENTS_UNKNOWN3 | CONTENTS_FENCE | CONTENTS_BODY | CONTENTS_CORPSE \
     | CONTENTS_TRIGGER)
#define MASK_FOOTSHADOW   (CONTENTS_FENCE | CONTENTS_SLIME | CONTENTS_LAVA | CONTENTS_SOLID)
#define MASK_PLAYERSTART  (CONTENTS_SOLID | CONTENTS_BBOX | CONTENTS_NOBOTCLIP | CONTENTS_UNKNOWN2 | CONTENTS_BODY)
#define MASK_VIEWSOLID    (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_FENCE | CONTENTS_TRIGGER)
#define MASK_DEADSOLID    (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_CORPSE | CONTENTS_NOTTEAM2 | CONTENTS_FENCE)
#define MASK_MONSTERSOLID (CONTENTS_SOLID | CONTENTS_MONSTERCLIP | CONTENTS_BODY)
#define MASK_WATER        (CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME)
#define MASK_OPAQUE       (CONTENTS_SOLID | CONTENTS_SLIME | CONTENTS_LAVA)
#define MASK_SHOT                                                                                                   \
    (CONTENTS_SOLID | CONTENTS_BBOX | CONTENTS_NOBOTCLIP | CONTENTS_UNKNOWN2 | CONTENTS_FENCE | CONTENTS_WEAPONCLIP \
     | CONTENTS_BODY | CONTENTS_TRIGGER)
#define MASK_SHOT_TRIG                                                                                               \
    (CONTENTS_SOLID | CONTENTS_CLAYPIDGEON | CONTENTS_WATER | CONTENTS_BBOX | CONTENTS_NOBOTCLIP | CONTENTS_UNKNOWN2 \
     | CONTENTS_FENCE | CONTENTS_WEAPONCLIP | CONTENTS_BODY | CONTENTS_TRIGGER)
#define MASK_PROJECTILE                                                                                           \
    (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_TRIGGER | CONTENTS_FENCE | CONTENTS_WEAPONCLIP | CONTENTS_UNKNOWN2 \
     | CONTENTS_NOBOTCLIP | CONTENTS_BBOX)
#define MASK_MELEE                                                                                                \
    (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_TRIGGER | CONTENTS_WEAPONCLIP | CONTENTS_FENCE | CONTENTS_UNKNOWN2 \
     | CONTENTS_NOBOTCLIP | CONTENTS_BBOX | CONTENTS_NOTTEAM1)
#define MASK_PATHSOLID                                                                                      \
    (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_BODY | CONTENTS_FENCE | CONTENTS_UNKNOWN2 | CONTENTS_BBOX \
     | CONTENTS_MONSTERCLIP)
#define MASK_LOOK        (CONTENTS_SOLID | CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_FENCE)
#define MASK_CAMERASOLID (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_BODY | MASK_WATER)
#define MASK_BEAM        (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_PLAYERCLIP | CONTENTS_BODY | CONTENTS_FENCE)
#define MASK_LADDER \
    (CONTENTS_SOLID | CONTENTS_LADDER | CONTENTS_TRIGGER | CONTENTS_PLAYERCLIP | CONTENTS_BODY | CONTENTS_FENCE)
#define MASK_AUTOCALCLIFE (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_FENCE)
#define MASK_EXPLOSION    (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_WEAPONCLIP)
#define MASK_SOUND        (CONTENTS_SOLID | CONTENTS_TRANSLUCENT)
#define MASK_VEHICLE                                                                                                 \
    (CONTENTS_SOLID | CONTENTS_BBOX | CONTENTS_NOBOTCLIP | CONTENTS_UNKNOWN2 | CONTENTS_FENCE | CONTENTS_VEHICLECLIP \
     | CONTENTS_BODY | CONTENTS_TRIGGER)
#define MASK_VEHICLE_TIRES (CONTENTS_SOLID | CONTENTS_BBOX | CONTENTS_FENCE | CONTENTS_VEHICLECLIP | CONTENTS_TRIGGER)
#define MASK_CLICK \
    (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_UNKNOWN3 | CONTENTS_UNKNOWN2 | CONTENTS_NOBOTCLIP | CONTENTS_BBOX)
#define MASK_CANSEE \
    (CONTENTS_SOLID | CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_FENCE | CONTENTS_WEAPONCLIP | CONTENTS_BODY)
#define MASK_CANSEE_NOENTS (CONTENTS_SOLID | CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_FENCE | CONTENTS_WEAPONCLIP)
#define MASK_ITEM          (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_PLAYERCLIP | CONTENTS_FENCE)
#define MASK_TRANSITION \
    (CONTENTS_SOLID | CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_UNKNOWN2 | CONTENTS_NOTTEAM1 | CONTENTS_WEAPONCLIP)
#define MASK_TARGETPATH \
    (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_MONSTERCLIP | CONTENTS_FENCE | CONTENTS_UNKNOWN2 | CONTENTS_BBOX)
#define MASK_ACTORPATH (CONTENTS_SOLID | CONTENTS_FENCE | CONTENTS_MONSTERCLIP | CONTENTS_TRIGGER)
#define MASK_GRENADEPATH                                                                                            \
    (CONTENTS_SOLID | CONTENTS_BBOX | CONTENTS_NOBOTCLIP | CONTENTS_UNKNOWN2 | CONTENTS_FENCE | CONTENTS_WEAPONCLIP \
     | CONTENTS_BODY | CONTENTS_TRIGGER)
#define MASK_LINE (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_UNKNOWN2 | CONTENTS_NOBOTCLIP | CONTENTS_BBOX)
#define MASK_VEHICLETURRET                                                                            \
    (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_FENCE | CONTENTS_VEHICLECLIP | CONTENTS_MONSTERCLIP \
     | CONTENTS_PLAYERCLIP)
#define MASK_VOLUMETRIC_SMOKE                                                                                        \
    (CONTENTS_TRANSLUCENT | CONTENTS_CORPSE | CONTENTS_BODY | CONTENTS_UNKNOWN2 | CONTENTS_NOBOTCLIP | CONTENTS_BBOX \
     | CONTENTS_WATER)
#define MASK_SMOKE_SPRITE (CONTENTS_SOLID | CONTENTS_TRIGGER | CONTENTS_FENCE)
#define MASK_LANDMINE                                                                                               \
    (CONTENTS_SOLID | CONTENTS_BBOX | CONTENTS_NOBOTCLIP | CONTENTS_UNKNOWN2 | CONTENTS_FENCE | CONTENTS_PLAYERCLIP \
     | CONTENTS_WEAPONCLIP | CONTENTS_BODY | CONTENTS_DETAIL | CONTENTS_STRUCTURAL | CONTENTS_TRIGGER)
#define MASK_LANDMINE_PLACE \
    (CONTENTS_SOLID | CONTENTS_FENCE | CONTENTS_PLAYERCLIP | CONTENTS_DETAIL | CONTENTS_STRUCTURAL | CONTENTS_TRIGGER)
#define MASK_BATTLELANGUAGE                                                                        \
    (CONTENTS_SOLID | CONTENTS_BBOX | CONTENTS_NOBOTCLIP | CONTENTS_UNKNOWN2 | CONTENTS_PLAYERCLIP \
     | CONTENTS_WEAPONCLIP | CONTENTS_BODY | CONTENTS_TRIGGER)
#define MASK_SCRIPT_SLAVE                                                                            \
    (CONTENTS_LADDER | CONTENTS_CLAYPIDGEON | CONTENTS_BBOX | CONTENTS_NOBOTCLIP | CONTENTS_UNKNOWN2 \
     | CONTENTS_UNKNOWN3 | CONTENTS_BODY | CONTENTS_CORPSE)
#define MASK_CARRYABLETURRET \
    (CONTENTS_SOLID | CONTENTS_BBOX | CONTENTS_NOBOTCLIP | CONTENTS_UNKNOWN2 | CONTENTS_FENCE | CONTENTS_BODY)
#define MASK_AITURRET                                                                                         \
    (CONTENTS_SOLID | CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_BBOX | CONTENTS_NOBOTCLIP | CONTENTS_UNKNOWN2 \
     | CONTENTS_FENCE | CONTENTS_WEAPONCLIP | CONTENTS_BODY)
#define MASK_TRANSPARENT (CONTENTS_SOLID | CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_BBOX | CONTENTS_FENCE)
#define MASK_MOVEVEHICLE                                                                                             \
    (CONTENTS_LADDER | CONTENTS_CLAYPIDGEON | CONTENTS_BBOX | CONTENTS_NOBOTCLIP | CONTENTS_UNKNOWN3 | CONTENTS_BODY \
     | CONTENTS_CORPSE)
#define MASK_MOVEINFO (CONTENTS_SOLID | CONTENTS_FENCE)
#define MASK_AI_CANSEE                                                                                    \
    (CONTENTS_SOLID | CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_BBOX | CONTENTS_UNKNOWN2 | CONTENTS_FENCE \
     | CONTENTS_WEAPONCLIP)
#define MASK_CORNER_NODE \
    (CONTENTS_SOLID | CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_UNKNOWN2 | CONTENTS_WEAPONCLIP | CONTENTS_BODY)

    void BG_EvaluateTrajectoryDelta(const trajectory_t *tr, int atTime, vec3_t result);

    void BG_PlayerStateToEntityState(playerState_t *ps, entityState_t *s, qboolean snap);
    void BG_PlayerStateToEntityStateExtraPolate(playerState_t *ps, entityState_t *s, int time, qboolean snap);

    enum cg_message_ver_15_e {
        CGM_NONE,
        CGM_BULLET_1,
        CGM_BULLET_2,
        CGM_BULLET_3,
        CGM_BULLET_4,
        CGM_BULLET_5,
        CGM_BULLET_6,
        CGM_BULLET_7,
        CGM_BULLET_8,
        CGM_BULLET_9,
        CGM_BULLET_10,
        CGM_BULLET_11,
        CGM_MELEE_IMPACT,
        CGM_EXPLOSION_EFFECT_1,
        CGM_EXPLOSION_EFFECT_2,
        CGM_EXPLOSION_EFFECT_3,
        CGM_EXPLOSION_EFFECT_4,
        CGM_PADDING_1,
        CGM_MAKE_EFFECT_1,
        CGM_MAKE_EFFECT_2,
        CGM_MAKE_EFFECT_3,
        CGM_MAKE_EFFECT_4,
        CGM_MAKE_EFFECT_5,
        CGM_MAKE_EFFECT_6,
        CGM_MAKE_EFFECT_7,
        CGM_MAKE_EFFECT_8,
        CGM_MAKE_CRATE_DEBRIS,
        CGM_MAKE_WINDOW_DEBRIS,
        CGM_BULLET_NO_BARREL_1,
        CGM_BULLET_NO_BARREL_2,
        CGM_HUDDRAW_SHADER,
        CGM_HUDDRAW_ALIGN,
        CGM_HUDDRAW_RECT,
        CGM_HUDDRAW_VIRTUALSIZE,
        CGM_HUDDRAW_COLOR,
        CGM_HUDDRAW_ALPHA,
        CGM_HUDDRAW_STRING,
        CGM_HUDDRAW_FONT,
        CGM_NOTIFY_KILL,
        CGM_NOTIFY_HIT,
        CGM_VOICE_CHAT,
        CGM_UNKNOWN_1,
    };

    enum cg_message_ver_6_e {
        CGM6_NONE,
        CGM6_BULLET_1,
        CGM6_BULLET_2,
        CGM6_BULLET_3,
        CGM6_BULLET_4,
        CGM6_BULLET_5,
        CGM6_BULLET_6,
        CGM6_BULLET_7,
        CGM6_BULLET_8,
        CGM6_BULLET_9,
        CGM6_BULLET_10,
        CGM6_MELEE_IMPACT,
        CGM6_EXPLOSION_EFFECT_1,
        CGM6_EXPLOSION_EFFECT_2,
        CGM6_PADDING_1,
        CGM6_MAKE_EFFECT_1,
        CGM6_MAKE_EFFECT_2,
        CGM6_MAKE_EFFECT_3,
        CGM6_MAKE_EFFECT_4,
        CGM6_MAKE_EFFECT_5,
        CGM6_MAKE_EFFECT_6,
        CGM6_MAKE_EFFECT_7,
        CGM6_MAKE_EFFECT_8,
        CGM6_MAKE_CRATE_DEBRIS,
        CGM6_MAKE_WINDOW_DEBRIS,
        CGM6_BULLET_NO_BARREL_1,
        CGM6_BULLET_NO_BARREL_2,
        CGM6_HUDDRAW_SHADER,
        CGM6_HUDDRAW_ALIGN,
        CGM6_HUDDRAW_RECT,
        CGM6_HUDDRAW_VIRTUALSIZE,
        CGM6_HUDDRAW_COLOR,
        CGM6_HUDDRAW_ALPHA,
        CGM6_HUDDRAW_STRING,
        CGM6_HUDDRAW_FONT,
        CGM6_NOTIFY_KILL,
        CGM6_NOTIFY_HIT,
        CGM6_VOICE_CHAT,
    };

    int BG_MapCGMToProtocol(int protocol, int messageNumber);

#ifdef __cplusplus
}
#endif

typedef enum {
    OBJ_FLAG_NONE,
    OBJ_FLAG_HIDDEN    = 1,
    OBJ_FLAG_CURRENT   = 2,
    OBJ_FLAG_COMPLETED = 4
} objective_flags_t;

typedef enum {
    OBJ_STATUS_NONE = 0,
    OBJ_STATUS_HIDDEN,
    OBJ_STATUS_CURRENT,
    OBJ_STATUS_COMPLETED
} objective_status_t;
