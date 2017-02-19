typedef unsigned int qboolean;
typedef unsigned int DWORD;
typedef float vec3_t[3];

typedef DWORD solid_t;
typedef unsigned char byte;

#define	CVAR_ARCHIVE			1
#define	CVAR_USERINFO			2
#define	CVAR_SERVERINFO			4
#define	CVAR_SYSTEMINFO			8
#define	CVAR_INIT				16
#define	CVAR_LATCH				32
#define	CVAR_ROM				64
#define	CVAR_USERCREATED		128
#define	CVAR_TEMP				256
#define CVAR_CHEAT				512

#define MAX_MSGLEN				49152
#define MAX_ZPATH				256

#define PACKET_BACKUP			32

#define	SVF_NOCLIENT			0x00000001
#define SVF_CLIENTMASK			0x00000002
#define SVF_BOT					0x00000008
#define	SVF_BROADCAST			0x00000020
#define	SVF_PORTAL				0x00000040
#define	SVF_USE_CURRENT_ORIGIN	0x00000080
#define SVF_SINGLECLIENT		0x00000100
#define SVF_NOSERVERINFO		0x00000200
#define SVF_CAPSULE				0x00000400
#define SVF_NOTSINGLECLIENT		0x00000800

#define	RF_THIRD_PERSON			( 1<<0 )
#define	RF_FIRST_PERSON			( 1<<1 )
#define RF_DEPTHHACK			( 1<<2 )
#define RF_VIEWLENSFLARE		( 1<<3 )
#define	RF_FRAMELERP			( 1<<4 )
#define	RF_BEAM					( 1<<5 )
#define RF_DONTDRAW				( 1<<7 )
#define RF_LENSFLARE			( 1<<8 )
#define RF_EXTRALIGHT			( 1<<9 )
#define RF_DETAIL				( 1<<10 )
#define RF_SHADOW				( 1<<11 )
#define	RF_PORTALSURFACE		( 1<<12 )
#define	RF_SKYORIGIN			( 1<<13 )
#define	RF_SKYENTITY			( 1<<14 )
#define	RF_LIGHTOFFSET			( 1<<15 )
#define	RF_CUSTOMSHADERPASS		( 1<<16 )
#define	RF_MINLIGHT				( 1<<17 )
#define	RF_FULLBRIGHT			( 1<<18 )
#define RF_LIGHTINGORIGIN		( 1<<19 )
#define	RF_SHADOWPLANE			( 1<<20 )
#define	RF_WRAPFRAMES			( 1<<21 )
#define RF_PORTALENTITY			( 1<<22 )
#define RF_DUALENTITY			( 1<<23 )
#define RF_ADDITIVEDLIGHT		( 1<<24 )
#define RF_LIGHTSTYLEDLIGHT		( 1<<25 )
#define RF_SHADOWPRECISE		( 1<<26 )
#define RF_INVISIBLE			( 1<<27 )

#define BUTTON_GROUP_FIRE_PRIMARY   0x0001
#define BUTTON_GROUP_FIRE_SECONDARY 0x0002
#define BUTTON_GROUP_RUN            0x0004
#define BUTTON_GROUP_USE            0x0008
#define BUTTON_GROUP_LEAN_LEFT      0x0010
#define BUTTON_GROUP_LEAN_RIGHT     0x0020
#define BUTTON_GROUP_DROP_WEAPON    0x0780
#define BUTTON_GROUP_VIEW           0x4000
#define BUTTON_GROUP_MOVE           0xC000

#define EF_STAND				2
#define EF_CROUCHING			4
#define EF_PRONE				8
#define EF_TELEPORT				32
#define EF_ALLIES				128
#define EF_AXIS					256
#define EF_DEAD					512

#define MAX_ACTIVESNAPSHOTS		2
#define MAX_BONECONTROLLERS		5
#define MAX_DOWNLOADWINDOW		8
#define MAX_COLORRANGES			10
#define MAX_PREDICTEDEVENTS		16
#define MAX_FRAMEINFOS			16
#define MAX_ANIMLAST			16
#define MAX_WEAPONS				16
#define MAX_SOUNDBUFFER			20
#define MAX_OBJECTIVES			20
#define MAX_PLAYERSTATS			32
#define MAX_NAMELENGTH			32
#define MAX_MAPAREABYTES		32
#define MAX_SURFACES			32
#define MAX_QPATH				64
#define MAX_CLIENTS				64
#define MAX_LOCATIONS			64
#define MAX_SERVERSOUNDS		64
#define	MAX_STRINGTOKENS		256
#define	MAX_OSPATH				256
#define MAX_SOUNDS				512
#define MAX_RELIABLECOMMANDS	512
#define MAX_MODELS				1024
#define MAX_ENTITIESINSNAPSHOT	1024
#define MAX_INFOSTRING			1350
#define MAX_STRINGCHARS			2048
#define MAX_CONFIGSTRINGS		2736
#define MAX_EXTSTRINGCHARS		4096
#define MAX_GAMESTATECHARS		32000

#define CS_MAXSOUNDS_BITS		8
#define CS_MAXLIGHTSTYLES		32
#define CS_MAXAMMOCOUNT			32
#define CS_MAXACTIVEITEMS		32
#define CS_MAXINVENTORY			32
#define CS_MAXAMMO				32
#define CS_MAXLOCATIONS			64
#define CS_MAXIMAGES			64
#define	CS_MAXMODELS			384

#define	CS_MAXSOUNDS			( 1<<MAX_SOUNDS_BITS )
#define CS_MAXITEMS				( CS_MAXINVENTORY + CS_MAXAMMO )

#define	CS_SERVERINFO			0
#define	CS_SYSTEMINFO			1
#define	CS_NAME					2
#define	CS_SOUNDTRACK			8
#define CS_FOGINFO				9
#define CS_SKYINFO				10
#define	CS_GAMEVERSION			11
#define	CS_LEVELSTARTTIME		12
#define	CS_MODELS				32
#define	CS_PLAYERS				1684

#define	CS_SOUNDS				( CS_MODELS + CS_MAXMODELS )
#define CS_IMAGES				( CS_SOUNDS + CS_MAXSOUNDS )
#define CS_LIGHTSTYLES			( CS_IMAGES + CS_MAXIMAGES )
#define CS_AMMO					( CS_PLAYERS + MAX_CLIENTS )
#define CS_LOCATIONS			( CS_AMMO + CS_MAXAMMO )

#define CONTENTS_SOLID			0x1
#define CONTENTS_LAVA			0x8
#define CONTENTS_SLIME			0x10
#define CONTENTS_WATER			0x20
#define CONTENTS_FOG			0x40
#define CONTENTS_AREAPORTAL		0x8000
#define CONTENTS_PLAYERCLIP		0x10000
#define CONTENTS_MONSTERCLIP	0x20000
#define CONTENTS_WEAPONCLIP		0x40000
#define CONTENTS_SHOOTABLEONLY	0x100000
#define CONTENTS_ORIGIN			0x1000000
#define CONTENTS_BODY			0x2000000
#define CONTENTS_CORPSE			0x4000000
#define CONTENTS_DETAIL			0x8000000
#define CONTENTS_STRUCTURAL		0x10000000
#define CONTENTS_TRANSLUCENT	0x20000000
#define CONTENTS_TRIGGER        0x40000000
#define CONTENTS_NODROP         0x80000000



// MASK TYPES
#define MASK_ALL                        -1
#define MASK_SOLID                        1

#define MASK_COLLISION                    0x26000B21
#define MASK_PERMANENTMARK                0x40000001
#define MASK_AUTOCALCLIFE                0x40002021
#define MASK_EXPLOSION                    0x40040001
#define MASK_TREADMARK                    0x42012B01
#define MASK_THIRDPERSON                0x42012B39
#define MASK_FOOTSTEP                    0x42022901
#define MASK_BEAM                        0x42042B01
#define MASK_VISIBLE                    0x42042B01
#define MASK_VEHICLE                    0x42042B01
#define MASK_BULLET                        0x42042B21
#define MASK_SHOT                        0x42042BA1
#define MASK_CROSSHAIRSHADER            0x42092B01
#define MASK_TRACER                        0x42142B21  

/*
#define	SURF_NODAMAGE			( 1<<0 )	// never give falling damage
#define	SURF_SLICK				( 1<<1 )	// effects game physics
#define	SURF_SKY				   ( 1<<2 )	// lighting from environment map
#define	SURF_LADDER				( 1<<3 ) // ladder surface
#define	SURF_NOIMPACT			( 1<<4 ) // don't make missile explosions
#define	SURF_NOMARKS			( 1<<5 ) // don't leave missile marks
#define  SURF_CASTSHADOW      ( 1<<6 ) // used in conjunction with nodraw allows surface to be not drawn but still cast shadows
#define  SURF_NODRAW          ( 1<<7 ) // don't generate a drawsurface at all
#define	SURF_NOLIGHTMAP		( 1<<10 )// surface doesn't need a lightmap
#define  SURF_ALPHASHADOW     ( 1<<11 )// do per-pixel shadow tests based on the texture
#define	SURF_NOSTEPS			( 1<<13 )// no footstep sounds
#define	SURF_NONSOLID			( 1<<14 )// don't collide against curves with this set
#define  SURF_RICOCHET        ( 1<<15 )// ricochet bullets


#define  SURF_TYPE_WOOD       ( 1<<16 )// wood surface
#define  SURF_TYPE_METAL      ( 1<<17 )// metal surface
#define  SURF_TYPE_ROCK       ( 1<<18 )// stone surface
#define  SURF_TYPE_DIRT       ( 1<<19 )// dirt surface
#define  SURF_TYPE_GRILL      ( 1<<20 )// metal grill surface
#define  SURF_TYPE_ORGANIC    ( 1<<21 )// oraganic (grass, loamy dirt)
#define  SURF_TYPE_SQUISHY    ( 1<<22 )// squishy (swamp dirt, flesh)

#define	SURF_NODLIGHT			( 1<<23 )// don't dlight even if solid (solid lava, skies)
#define	SURF_HINT   			( 1<<24 )// choose this plane as a partitioner

#define  SURF_PATCH           ( 1<<29 )
#define  SURF_KEEP            (SURF_PATCH)

#define MASK_SURF_TYPE        (SURF_TYPE_WOOD|SURF_TYPE_METAL|SURF_TYPE_ROCK|SURF_TYPE_DIRT|SURF_TYPE_GRILL|SURF_TYPE_ORGANIC|SURF_TYPE_SQUISHY)
*/


#define SURF_NODAMAGE                    0x1
#define SURF_SLICK                        0x2
#define SURF_SKY                        0x4
#define SURF_LADDER                        0x8
#define SURF_NOIMPACT                    0x10
#define SURF_NOMARKS                    0x20
#define SURF_CASTSHADOW                    0x40
#define SURF_PAPER                        0x2000
#define SURF_WOOD                        0x4000
#define SURF_METAL                        0x8000
#define SURF_STONE                        0x10000
#define SURF_DIRT                        0x20000
#define SURF_METALGRILL                    0x40000
#define SURF_GRASS                        0x80000
#define SURF_MUD                        0x100000
#define SURF_PUDDLE                        0x200000
#define SURF_GLASS                        0x400000
#define SURF_GRAVEL                        0x800000
#define SURF_SAND                        0x1000000
#define SURF_FOLIAGE                    0x2000000
#define SURF_SNOW                        0x4000000
#define SURF_CARPET                        0x8000000
#define SURF_BACKSIDE                    0x10000000
#define SURF_NODLIGHT                    0x20000000
#define SURF_HINT                        0x40000000

#define MOVE_POS_IDLE			(1<<0)
#define MOVE_POS_CROUCH			(1<<1)
#define MOVE_POS_FALL			(1<<3)	// NOT SURE
#define MOVE_POS_WALK			(1<<4)			
#define MOVE_POS_RUN			(1<<5)


//#define FRAME_TIME 0.050f

typedef void ( *xcommand_t )( );
static char *LaunchAppName;

typedef int qhandle_t;
typedef int sfxHandle_t;
typedef int clipHandle_t;
typedef int cvarHandle_t;
typedef int fileHandle_t;
typedef int qboolean;

typedef float vec2_t[2];
typedef float vec3_t[3];
typedef float vec4_t[4];
typedef float vec5_t[5];

typedef enum soundChannel_s
{
	CHAN_AUTO,
	CHAN_BODY,
	CHAN_ITEM,
	CHAN_WEAPONIDLE,
	CHAN_VOICE,
	CHAN_LOCAL,
	CHAN_WEAPON,
	CHAN_DIALOG_SECONDARY,
	CHAN_DIALOG,
	CHAN_MENU,
	CHAN_MAX

} soundChannel_t;

typedef enum connstate_s
{
	CA_UNINITIALIZED,
	CA_DISCONNECTED,
	CA_AUTHORIZING,
	CA_CONNECTING,
	CA_CHALLENGING,
	CA_CONNECTED,
	CA_LOADING,
	CA_PRIMED,
	CA_ACTIVE,
	CA_CINEMATIC

} connstate_t;

typedef enum soundArgsType_s
{
	CHANARGS_NONE,
	CHANARGS_MOD,
	CHANARGS_OVERRIDE

} soundArgsType_t;

typedef enum SkelVec_Axis_s
{
	svX,
	svY,
	svZ,
	svW

} SkelVec_Axis_t;

typedef enum YPR_Axes_s
{
	Vec3YAW,
	Vec3PITCH,
	Vec3ROLL

} YPR_Axes_t;

typedef enum damage_s
{
	DAMAGE_NO,
	DAMAGE_YES,
	DAMAGE_AIM

} damage_t;

typedef enum movecontrol_s
{
	MOVECONTROL_NONE,
	MOVECONTROL_USER,
	MOVECONTROL_LEGS,
	MOVECONTROL_USER_MOVEANIM,
	MOVECONTROL_ANIM,
	MOVECONTROL_ABSOLUTE,
	MOVECONTROL_HANGING,
	MOVECONTROL_ROPE_GRAB,
	MOVECONTROL_ROPE_RELEASE,
	MOVECONTROL_ROPE_MOVE,
	MOVECONTROL_PICKUPENEMY,
	MOVECONTROL_PUSH,
	MOVECONTROL_CLIMBWALL,
	MOVECONTROL_USEANIM,
	MOVECONTROL_CROUCH,
	MOVECONTROL_LOOPUSEANIM,
	MOVECONTROL_USEOBJECT,
	MOVECONTROL_COOLOBJECT

} movecontrol_t;

typedef enum music_mood_s
{
	mood_none,
	mood_normal,
	mood_action,
	mood_suspense,
	mood_mystery,
	mood_success,
	mood_failure,
	mood_surprise,
	mood_special,
	mood_aux1,
	mood_aux2,
	mood_aux3,
	mood_aux4,
	mood_aux5,
	mood_aux6,
	mood_aux7,
	mood_totalnumber

} music_mood_t;

typedef enum painDirection_s
{
	PAIN_NONE,
	PAIN_FRONT,
	PAIN_LEFT,
	PAIN_RIGHT,
	PAIN_REAR

} painDirection_t;

typedef enum eax_mode_s
{
	eax_generic,
	eax_paddedcell,
	eax_room,
	eax_bathroom,
	eax_livingroom,
	eax_stoneroom,
	eax_auditorium,
	eax_concerthall,
	eax_cave,
	eax_arena,
	eax_hangar,
	eax_carpetedhallway,
	eax_hallway,
	eax_stonecorridor,
	eax_alley,
	eax_forest,
	eax_city,
	eax_mountains,
	eax_quarry,
	eax_plain,
	eax_parkinglot,
	eax_sewerpipe,
	eax_underwater,
	eax_drugged,
	eax_dizzy,
	eax_psychotic,
	eax_totalnumber

} eax_mode_t;

typedef enum emittype_s
{
	emit_point,
	emit_area,
	emit_spotlight,
	emit_sun
} emittype_t;

typedef enum errorParm_e
{
	ERR_FATAL,
	ERR_DROP,
	ERR_DISCONNECT,
	ERR_NEEDCD

} errorParm_t;

typedef enum fsMode_e
{
	FS_READ,
	FS_WRITE,
	FS_APPEND,
	FS_APPENDSYNC

} fsMode_t;

typedef enum cbufExec_e
{
	EXEC_NOW,
	EXEC_INSERT,
	EXEC_APPEND

} cbufExec_t;

typedef enum
{
	PRINT_ALL,
	PRINT_DEVELOPER,
	PRINT_DEVELOPER_2,
	PRINT_WARNING,
	PRINT_ERROR

} printParm_t;

typedef enum fsOrigin_e
{
	FS_SEEKCUR,
	FS_SEEKEND,
	FS_SEEKSET

} fsOrigin_t;

typedef enum firetype_s
{
	FT_NONE,
	FT_BULLET,
	FT_FAKEBULLET,
	FT_PROJECTILE,
	FT_MELEE,
	FT_SPECIAL_PROJECTILE,
	FT_CLICKITEM,
	FT_HEAVY

} firetype_t;

typedef enum voicetype_s
{
	PVT_NONE_SET,
	PVT_ALLIED_START,
	PVT_ALLIED_AIRBORNE,
	PVT_ALLIED_MANON,
	PVT_ALLIED_SAS,
	PVT_ALLIED_PILOT,
	PVY_ALLIED_ARMY,
	PVT_ALLIED_RANGER,
	PVT_ALLIED_END,
	PVT_AXIS_START,
	PVT_AXIS_AXIS1,
	PVT_AXIS_AXIS2,
	PVT_AXIS_AXIS3,
	PVT_AXIS_AXIS4,
	PVT_AXIS_AXIS5,
	PVT_AXIS_END

} voicetype_t;

typedef enum bodypart_s
{
	legs,
	torso

} bodypart_t;

typedef enum weaponstate_s
{
	WEAPON_READY,		// HOLSTERED
	WEAPON_FIRING,		// READY
	WEAPON_LOWERING,	//
	WEAPON_RAISING,		//
	WEAPON_HOLSTERED,	//
	WEAPON_RELOADING,	//
	WEAPON_CHANGING		//

} weaponstate_t;

typedef enum cameratype_s
{
	CAMERA_TOPDOWN,
	CAMERA_BEHIND,
	CAMERA_FRONT,
	CAMERA_SIDE,
	CAMERA_BEHIND_FIXED,
	CAMERA_SIDE_LEFT,
	CAMERA_SIDE_RIGHT,
	CAMERA_BEHIND_NOPITCH

} cameratype_t;

typedef enum AIRanges_s
{
	RANGE_SHORT,
	RANGE_MEDIUM,
	RANGE_LONG,
	RANGE_SNIPER,
	RANGE_NUM_RANGES

} AIRanges_t;

typedef enum keyNum_e
{
	K_TAB=9,
	K_ENTER=13,
	K_ESCAPE=27,
	K_SPACE=32,

	K_BACKSPACE=127,
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

typedef enum glDriverType_e
{
	GLDRV_ICD,
	GLDRV_STANDALONE,
	GLDRV_VOODOO

} glDriverType_t;

typedef enum textureCompression_s
{
	TC_NONE,
	TC_S3TC

} textureCompression_t;

typedef enum boneType_s
{
	SKELBONE_ROTATION,
	SKELBONE_POSROT,
	SKELBONE_IKSHOULDER,
	SKELBONE_IKELBOW,
	SKELBONE_IKWRIST,
	SKELBONE_HOSEROT,
	SKELBONE_AVROT,
	SKELBONE_ZERO,
	SKELBONE_NUMBONETYPES,
	SKELBONE_WORLD,
	SKELBONE_HOSEROTBOTH,
	SKELBONE_HOSEROTPARENT

} boneType_t;

typedef enum hoseRotType_s
{
	HRTYPE_PLAIN,
	HRTYPE_ROTATEPARENT180Y,
	HRTYPE_ROTATEBOTH180Y

} hoseRotType_t;

typedef enum glHardwareType_e
{
	GLHW_GENERIC,
	GLHW_3DFX2D3D,
	GLHW_ATI,
	GLHW_TNT,
	GLHW_TNT2,
	GLHW_GEFORCE,
	GLHW_GEFORCE2,
	GLHW_GEFORCE3,
	GLHW_KYRO

} glHardwareType_t;

typedef enum musicMood_e
{
	MOOD_NONE,
	MOOD_NORMAL,
	MOOD_ACTION,
	MOOD_SUSPENSE,
	MOOD_MYSTERY,
	MOOD_SUCCESS,
	MOOD_FAILURE,
	MOOD_SURPRISE,
	MOOD_SPECIAL,
	MOOD_AUX1,
	MOOD_AUX2,
	MOOD_AUX3,
	MOOD_AUX4,
	MOOD_AUX5,
	MOOD_AUX6,
	MOOD_AUX7,

} musicMood_t;

typedef enum dlightType_e
{
	LENSFLARE=1,
	VIEWLENSFLARE,
	ADDITIVE=4

} dlighttype_t;

typedef enum stereoFrame_e
{
	STEREO_CENTER,
	STEREO_LEFT,
	STEREO_RIGHT

} stereoFrame_t;

typedef enum mapSurfaceType_s
{
	MST_BAD,
	MST_PLANAR,
	MST_PATCH,
	MST_TRIANGLE_SOUP,
	MST_FLARE,
	MST_TERRAIN

} mapSurfaceType_t;

typedef enum { SF_SKIP, SF_FACE, SF_GRID, SF_POLY, SF_MARK_FRAG, SF_FLARE, SF_ENTITY, SF_DISPLAY_LIST, SF_TIKI_SKEL, SF_TIKI_STATIC, SF_SWIPE, SF_SPRITE, SF_TERRAIN_PATCH, SF_NUM_SURFACE_TYPES, SF_MAX = 2147483647 } surfaceType_t;

typedef enum moveAnimType_s
{
	MOVEANIMTYPE_USEACTION,
	MOVEANIMTYPE_STAND_IDLE = 0,
	MOVEANIMTYPE_STAND_WALK,
	MOVEANIMTYPE_STAND_RUN,
	MOVEANIMTYPE_CROUCH_IDLE,
	MOVEANIMTYPE_CROUCH_WALK,
	MOVEANIMTYPE_CROUCH_RUN,
	MOVEANIMTYPE_PRONE_IDLE,
	MOVEANIMTYPE_PRONE_WALK,
	MOVEANIMTYPE_PRONE_ROLL,
	NUM_MOVEANIMTYPES

} moveAnimType_t;

typedef enum healthType_s
{
	HEALTHTYPE_NONE,
	HEALTHTYPE_GOOD,
	HEALTHTYPE_BAD

} healthType_t;

typedef enum skelType_s
{
	SKELTYPE_HUMAN,
	SKELTYPE_STATIC

} skelType_t;

typedef enum playerStat_e
{
	STAT_HEALTH,
	STAT_MAXHEALTH,
	STAT_WEAPONS,
	STAT_EQUIPPEDWEAPON,
	STAT_AMMO,
	STAT_MAXAMMO,
	STAT_CLIPAMMO,
	STAT_MAXCLIPAMMO,
	STAT_INZOOM,
	STAT_CROSSHAIR,
	STAT_LASTPAIN,

	STAT_BOSSHEALTH=12,
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

	STAT_HIGHESTSCORE=25,
	STAT_ATTACKERCLIENT,
	STAT_INFOCLIENT,
	STAT_INFOCLIENTHEALTH,
	STAT_DAMAGEDIR,
	STAT_LASTSTAT,

} playerStat_t;

typedef enum
{
	NA_BOT,
	NA_BAD,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX,
	NA_BROADCAST_IPX

} netAdrType_t;

typedef enum gameType_e
{
	GT_SINGLEPLAYER,
	GT_FREEFORALL,
	GT_TEAMDEATHMATCH,
	GT_TEAMROUNDS,
	GT_OBJECTIVE,

} gameType_t;

typedef enum refEntityType_e
{
	RT_MODEL,
	RT_SPRITE,
	RT_BEAM,
	RT_PORTALSURFACE,

} refEntityType_t;

typedef enum persEnum_s
{
	PERS_SCORE,
	PERS_TEAM

} persEnum_t;

typedef enum meansOfDeath_s
{
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
	MOD_TOTAL_NUMBER

} meansOfDeath_t;

typedef enum entityType_e
{
	ET_MODELANIMSKEL,
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
	ET_EVENTONLY,
	ET_RAIN,
	ET_LEAF,
	ET_SPEAKER,
	ET_PUSH_TRIGGER,
	ET_TELEPORTTRIGGER,
	ET_DECAL,
	ET_EMITTER,
	ET_ROPE,
	ET_EVENTS,
	ET_EXECCOMMANDS

} entityType_t;

typedef enum teamType_e
{
	TEAM_NONE,
	TEAM_SPECTATOR,
	TEAM_FREEFORALL,
	TEAM_ALLIES,
	TEAM_AXIS

} teamType_t;

typedef enum svc_ops_e_s
{
	svc_bad,
	svc_nop,
	svc_gamestate,
	svc_configstring,
	svc_baseline,
	svc_serverCommand,
	svc_download,
	svc_snapshot,
	svc_centerprint,
	svc_locprint,
	svc_cgameMessage,
	svc_EOF

} svc_ops_e_t;

typedef enum clc_ops_e_t
{
	clc_bad,
	clc_nop,
	clc_move,
	clc_moveNoDelta,
	clc_clientCommand,
	clc_EOF

} clc_ops_e_s;

typedef enum
{
	NS_CLIENT,
	NS_SERVER

} netSrc_t;

typedef enum memtag_s
{
	TAG_FREE,
	TAG_CONST,
	TAG_GENERAL,
	TAG_BOTLIB,
	TAG_RENDERER,
	TAG_GAME,
	TAG_CGAME,
	TAG_CLIENT,
	TAG_TIKI,
	TAG_STRINGS_AND_COMMANDS,
	TAG_SOUND,
	TAG_STATIC,
	TAG_STATIC_RENDERER,
	TAG_SKEL,
	TAG_TEMP,
	TAG_EXE,
	TAG_NUM_TOTAL_TAGS

} memtag_t;

typedef enum joystickAxis_s
{
	AXIS_SIDE,
	AXIS_FORWARD,
	AXIS_UP,
	AXIS_ROLL,
	AXIS_YAW,
	AXIS_PITCH,
	MAX_JOYSTICK_AXIS

} joystickAxis_t;

typedef enum
{
	CS_FREE,
	CS_ZOMBIE,
	CS_CONNECTED,
	CS_PRIMED,
	CS_ACTIVE

} clientState_t;

typedef enum sysEventType_s
{
	SE_NONE,
	SE_KEY,
	SE_CHAR,
	SE_MOUSE,
	SE_JOYSTICK_AXIS,
	SE_CONSOLE,
	SE_PACKET

} sysEventType_t;

typedef enum viewmodelanim_s
{
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
	VMA_NUMANIMATIONS

} viewmodelanim_t;

typedef enum const_str_s
{
	STRING_EMPTY = 1,
	STRING_TOUCH,
	STRING_BLOCK,
	STRING_TRIGGER,
	STRING_USE,
	STRING_DAMAGE,
	STRING_LOCATION,
	STRING_SAY,
	STRING_FAIL,
	STRING_BUMP,
	STRING_DEFAULT,
	STRING_ALL,
	STRING_MOVE_ACTION,
	STRING_RESUME,
	STRING_OPEN,
	STRING_CLOSE,
	STRING_PICKUP,
	STRING_REACH,
	STRING_START,
	STRING_TELEPORT,
	STRING_MOVE,
	STRING_MOVE_END,
	STRING_MOVETO,
	STRING_WALKTO,
	STRING_RUNTO,
	STRING_CROUCHTO,
	STRING_CRAWLTO,
	STRING_STOP,
	STRING_RESET,
	STRING_PRESPAWN,
	STRING_SPAWN,
	STRING_PLAYERSPAWN,
	STRING_SKIP,
	STRING_ROUNDSTART,
	STRING_VISIBLE,
	STRING_NOT_VISIBLE,
	STRING_DONE,
	STRING_ANIMDONE,
	STRING_UPPERANIMDONE,
	STRING_SAYDONE,
	STRING_FLAGGEDANIMDONE,
	STRING_IDLE,
	STRING_WALK,
	STRING_SHUFFLE,
	STRING_ANIM_CROUCH_SCR,
	STRING_FORGOT,
	STRING_JOG_HUNCH,
	STRING_JOG_HUNCH_RIFLE,
	STRING_KILLED,
	STRING_ALARM,
	STRING_SCRIPTCLASS,
	STRING_FACT_SCRIPT_FACTORY,
	STRING_DEATH, STRING_DEATH_FALL_TO_KNEES, STRING_ENEMY, STRING_DEAD, STRING_MOOD, STRING_PATROL, STRING_RUNNER, STRING_FOLLOW, STRING_ACTION, STRING_MOVE_BEGIN, STRING_ACTION_BEGIN, STRING_ACTION_END, STRING_SUCCESS, STRING_ENTRY, STRING_EXIT, STRING_PATH, STRING_NODE, STRING_ASK_COUNT, STRING_ATTACKER, STRING_USECOVER, STRING_WAITCOVER, STRING_VOID, STRING_END, STRING_ATTACK, STRING_NEAR, STRING_PAPERS, STRING_CHECK_PAPERS, STRING_TIMEOUT, STRING_HOSTILE, STRING_LEADER, STRING_GAMEMAP, STRING_BORED, STRING_NERVOUS, STRING_CURIOUS, STRING_ALERT, STRING_GREET, STRING_DEFEND, STRING_ANIM, STRING_ANIM_SCRIPTED, STRING_ANIM_CURIOUS, STRING_ANIMLOOP, STRING_UNDEFINED, STRING_NOTSET, STRING_INCREMENT, STRING_DECREMENT, STRING_TOGGLE, STRING_NORMAL, STRING_SUSPENSE, STRING_MYSTERY, STRING_SURPRISE, STRING_ANIM_CROUCH_RUN_SCR, STRING_ANIM_AIM_SCR, STRING_ANIM_SHOOT_SCR, STRING_ANIM_MG42_SHOOT_SCR, STRING_ANIM_MG42_IDLE_SCR, STRING_ANIM_MG42_RELOAD_SCR, STRING_DRIVE, STRING_GLOBAL_WEAPON_SCR, STRING_GLOBAL_MOVETO_SCR, STRING_GLOBAL_ANIM_SCR, STRING_GLOBAL_ANIM_SCRIPTED_SCR, STRING_GLOBAL_ANIM_NOCLIP_SCR, STRING_GLOBAL_WALKTO_SCR, STRING_GLOBAL_RUNTO_SCR, STRING_AIMAT,
	STRING_GLOBAL_DISABLE_AI_SCR,
 STRING_GLOBAL_CROUCHTO_SCR, STRING_GLOBAL_CRAWLTO_SCR, STRING_GLOBAL_KILLED_SCR, STRING_GLOBAL_PAIN_SCR, STRING_PAIN, STRING_TRACK, STRING_HASENEMY, STRING_ANIM_COWER_SCR, STRING_ANIM_STAND_SCR, STRING_ANIM_IDLE_SCR, STRING_ANIM_SURPRISE_SCR, STRING_ANIM_STANDSHOCK_SCR, STRING_ANIM_STANDIDENTIFY_SCR, STRING_ANIM_STANDFLINCH_SCR, STRING_ANIM_DOG_IDLE_SCR, STRING_ANIM_DOG_ATTACK_SCR, STRING_ANIM_DOG_CURIOUS_SCR, STRING_ANIM_DOG_CHASE_SCR, STRING_CANNON, STRING_GRENADE, STRING_HEAVY, STRING_ITEM, STRING_ITEMS, STRING_ITEM1, STRING_ITEM2, STRING_ITEM3, STRING_ITEM4, STRING_STAND, STRING_MG, STRING_PISTOL, STRING_RIFLE, STRING_SMG, STRING_TURNTO, STRING_STANDING, STRING_CROUCHING, STRING_PRONE, STRING_OFFGROUND, STRING_WALKING, STRING_RUNNING, STRING_FALLING, STRING_ANIM_NOTHING, STRING_ANIM_DIRECT, STRING_ANIM_PATH, STRING_ANIM_WAYPOINT, STRING_ANIM_DIRECT_NOGRAVITY, STRING_EMOTION_NONE, STRING_EMOTION_NEUTRAL, STRING_EMOTION_WORRY, STRING_EMOTION_PANIC, STRING_EMOTION_FEAR, STRING_EMOTION_DISGUST, STRING_EMOTION_ANGER, STRING_EMOTION_AIMING, STRING_EMOTION_DETERMINED, STRING_EMOTION_DEAD, STRING_EMOTION_CURIOUS, STRING_ANIM_EMOTION_SCR, STRING_FORCEANIM, STRING_FORCEANIM_SCRIPTED, STRING_TURRET, STRING_COVER, STRING_ANIM_PAIN_SCR, STRING_ANIM_KILLED_SCR, STRING_ANIM_ATTACK_SCR, STRING_ANIM_SNIPER_SCR, STRING_KNEES, STRING_CRAWL, STRING_FLOOR, STRING_ANIM_PATROL_SCR, STRING_ANIM_RUN_SCR, STRING_CROUCH, STRING_CROUCHWALK, STRING_CROUCHRUN, STRING_ANIM_CROUCH_WALK_SCR, STRING_ANIM_WALK_SCR, STRING_ANIM_PRONE_SCR, STRING_ANIM_RUNAWAYFIRING_SCR, STRING_ANIM_RUN_SHOOT_SCR, STRING_ANIM_RUNTO_ALARM_SCR, STRING_ANIM_RUNTO_CASUAL_SCR, STRING_ANIM_RUNTO_COVER_SCR, STRING_ANIM_RUNTO_DANGER_SCR, STRING_ANIM_RUNTO_DIVE_SCR, STRING_ANIM_RUNTO_FLEE_SCR, STRING_ANIM_RUNTO_INOPEN_SCR, STRING_ANIM_DISGUISE_SALUTE_SCR, STRING_ANIM_DISGUISE_WAIT_SCR, STRING_ANIM_DISGUISE_PAPERS_SCR, STRING_ANIM_DISGUISE_ENEMY_SCR, STRING_ANIM_DISGUISE_HALT_SCR, STRING_ANIM_DISGUISE_ACCEPT_SCR, STRING_ANIM_DISGUISE_DENY_SCR,
 STRING_ANIM_CORNERLEFT_SCR,
STRING_ANIM_CORNERRIGHT_SCR, STRING_ANIM_OVERATTACK_SCR, STRING_ANIM_CONTINUE_LAST_ANIM_SCR, STRING_FLAGGED, STRING_ANIM_FULLBODY_SCR, STRING_INTERNAL, STRING_DISGUISE_SALUTE, STRING_DISGUISE_SENTRY, STRING_DISGUISE_OFFICER, STRING_DISGUISE_ROVER, STRING_DISGUISE_NONE, STRING_MACHINEGUNNER, STRING_DISGUISE, STRING_DOG_IDLE, STRING_DOG_ATTACK, STRING_DOG_CURIOUS, STRING_DOG_GRENADE, STRING_ANIM_GRENADERETURN_SCR, STRING_ANIM_GRENADEKICK_SCR, STRING_ANIM_GRENADETHROW_SCR, STRING_ANIM_GRENADETOSS_SCR, STRING_ANIM_GRENADEMARTYR_SCR, STRING_MOVEDONE, STRING_AIM, STRING_ONTARGET, STRING_UNARMED, STRING_BALCONY_IDLE, STRING_BALCONY_CURIOUS, STRING_BALCONY_ATTACK, STRING_BALCONY_DISGUISE, STRING_BALCONY_GRENADE, STRING_BALCONY_PAIN, STRING_BALCONY_KILLED, STRING_WEAPONLESS, STRING_DEATH_BALCONY_INTRO, STRING_DEATH_BALCONY_LOOP, STRING_DEATH_BALCONY_OUTTRO, STRING_SOUNDDONE, STRING_NOCLIP, STRING_GERMAN, STRING_AMERICAN, STRING_SPECTATOR, STRING_FREEFORALL, STRING_ALLIES, STRING_AXIS, STRING_DRAW, STRING_KILLS, STRING_ALLIESWIN, STRING_AXISWIN, STRING_ANIM_SAY_CURIOUS_SIGHT_SCR, STRING_ANIM_SAY_CURIOUS_SOUND_SCR, STRING_ANIM_SAY_GRENADE_SIGHTED_SCR, STRING_ANIM_SAY_KILL_SCR, STRING_ANIM_SAY_MANDOWN_SCR, STRING_ANIM_SAY_SIGHTED_SCR, STRING_VEHICLEANIMDONE, STRING_POSTTHINK, STRING_TURNDONE, STRING_ANIM_NO_KILLED_SCR, STRING_MG42, STRING_MP40, STRING_LENGTH_

} const_str_t;

typedef enum weaponhand_s
{
	WEAPON_MAIN,
	WEAPON_OFFHAND,
	WEAPON_ERROR

} weaponhand_t;

typedef enum firemode_s
{
	FIRE_PRIMARY,
	FIRE_SECONDARY,
	MAX_FIREMODES,
	FIRE_ERROR

} firemode_t;

typedef enum entity_event_s
{
	EV_NONE,
	EV_FALL_SHORT,
	EV_FALL_MEDIUM,
	EV_FALL_FAR,
	EV_FALL_FATAL,
	EV_TERMINAL_VELOCITY,
	EV_WATER_TOUCH,
	EV_WATER_LEAVE,
	EV_WATER_UNDER,
	EV_WATER_CLEAR,
	EV_LAST_PREDICTED

} entity_event_t;

typedef enum pmType_e
{
	PM_NORMAL,
	PM_CLIMBWALL,
	PM_NOCLIP,
	PM_DEAD

} pmType_t;

typedef struct cLZ77_s {
	unsigned char *ip;
	unsigned char *op;
	unsigned char *in_end;
	unsigned char *ip_end;
	unsigned char *ii;
	unsigned char *m_pos;
	unsigned int m_off;
	unsigned int m_len;
	unsigned int dindex;
} cLZ77_t;

typedef struct tm_unz_s 
{
	unsigned int tm_sec;
	unsigned int tm_min;
	unsigned int tm_hour;
	unsigned int tm_mday;
	unsigned int tm_mon;
	unsigned int tm_year;
} tm_unz;

/* unz_global_info structure contain global data about the ZIPfile
   These data comes from the end of central dir */
typedef struct unz_global_info_s
{
	unsigned long number_entry;
	unsigned long size_comment;
} unz_global_info;

typedef struct unz_file_info_s
{
    unsigned long version;
    unsigned long version_needed;
    unsigned long flag;
    unsigned long compression_method;
    unsigned long dosDate;
    unsigned long crc;
    unsigned long compressed_size;
    unsigned long uncompressed_size;
    unsigned long size_filename;
    unsigned long size_file_extra;
    unsigned long size_file_comment;

    unsigned long disk_num_start;
    unsigned long internal_fa;
    unsigned long external_fa;

    tm_unz tmu_date;
} unz_file_info;

typedef struct unz_file_info_internal_s
{
    unsigned long offset_curfile;
} unz_file_info_internal;

typedef struct z_stream_s {
    unsigned char    *next_in;
    unsigned int     avail_in;
    unsigned long    total_in;

    unsigned char    *next_out;
    unsigned int     avail_out;
    unsigned long    total_out;

    char     *msg;
    struct internal_state *state;

    void *zalloc;
    void *zfree;
    unsigned char*     opaque;

    int     data_type;
    unsigned long   adler;
    unsigned long   reserved;
} z_stream;

typedef z_stream *z_streamp;

typedef struct
{
	char  *read_buffer;
	z_stream stream;

	unsigned long pos_in_zipfile;
	unsigned long stream_initialised;

	unsigned long offset_local_extrafield;
	unsigned int  size_local_extrafield;
	unsigned long pos_local_extrafield;

	unsigned long crc32;
	unsigned long crc32_wait;
	unsigned long rest_read_compressed;
	unsigned long rest_read_uncompressed;
	FILE* file;
	unsigned long compression_method;
	unsigned long byte_before_the_zipfile;
} file_in_zip_read_info_s;

typedef struct
{
	FILE* file;
	unz_global_info gi;
	unsigned long byte_before_the_zipfile;
	unsigned long num_file;
	unsigned long pos_in_central_dir;
	unsigned long current_file_ok;
	unsigned long central_pos;

	unsigned long size_central_dir;
	unsigned long offset_central_dir;

	unz_file_info cur_file_info;
	unz_file_info_internal cur_file_info_internal;
    file_in_zip_read_info_s* pfile_in_zip_read;
	unsigned char*	tmpFile;
	int	tmpPos,tmpSize;
} unz_s;

typedef struct fileInPack_s {
	char					*name;
	unsigned long			pos;
	struct	fileInPack_s*	next;
} fileInPack_t;

typedef struct {
	char			pakFilename[MAX_OSPATH];
	char			pakBasename[MAX_OSPATH];
	char			pakGamename[MAX_OSPATH];
	int				handle;
	int				checksum;
	int				pure_checksum;
	int				numfiles;
	int				referenced;
	int				hashSize;
	fileInPack_t*	*hashTable;
	fileInPack_t*	buildBuffer;
} pack_t;

typedef struct {
	char		path[MAX_OSPATH];
	char		gamedir[MAX_OSPATH];
} directory_t;

typedef struct searchpath_s {
	struct searchpath_s *next;

	pack_t		*pack;
	directory_t	*dir;
} searchpath_t;

typedef struct sysEvent_s
{
  int evTime;
  sysEventType_t evType;
  int evValue;
  int evValue2;
  int evPtrLength;
  void *evPtr;

} sysEvent_t;

typedef struct nodetype
{
	struct	nodetype *left, *right, *parent;
	struct	nodetype *next, *prev;
	struct	nodetype **head;
	int		weight;
	int		symbol;

} node_t;

#define HMAX 256

typedef struct
{
	int			blocNode;
	int			blocPtrs;
	node_t*		tree;
	node_t*		lhead;
	node_t*		ltail;
	node_t*		loc[HMAX+1];
	node_t**	freelist;
	node_t		nodeList[768];
	node_t*		nodePtrs[768];

} huff_t;

typedef struct
{
	huff_t		compressor;
	huff_t		decompressor;

} huffman_t;

typedef struct polyVert_s
{
  float xyz[3];
  float st[2];
  unsigned char modulate[4];

} polyVert_t;

typedef struct poly_s
{

  qhandle_t hShader;
  int numVerts;
  polyVert_t *verts;

} poly_t;

typedef struct client_persistant_s
{
	char userinfo[MAX_INFOSTRING];
	char netname[32];
	char playermodel[64];
	char playergermanmodel[64];
	char weapon[64];
	clientState_t connectionState;
	teamType_t team;
	int kills;

} client_persistant_t;

typedef struct cplane_s
{
	vec3_t normal;
	float dist;
	BYTE type;
	BYTE signBits;
	BYTE pad[2];

} cplane_t;

typedef struct userEyes_s
{
	signed char ofs[3];
	float angles[2];

} userEyes_t;

typedef struct outPacket_s
{
  int p_cmdNumber;
  int p_serverTime;
  int p_realtime;
  userEyes_t p_eyeinfo;

} outPacket_t;

typedef struct trace_s
{
	qboolean allSolid;
	qboolean startSolid;
	float fraction;
	vec3_t endPos;
	cplane_t plane;
	int surfaceFlags;
	int shaderNum;
	int contents;
	int entityNum;
	int location;
	struct gentity_s *ent;

} trace_t;

typedef struct {
	vec3_t		boxmins, boxmaxs;
	const float	*mins;
	const float *maxs;
	const float	*start;
	vec3_t		end;
	trace_t		trace;
	int			passEntityNum;
	int			contentmask;
	qboolean	cylinder;
	qboolean	traceDeep;
} moveclip_t;

typedef struct playerState_s
{
	int commandTime;
	pmType_t pmType;
	int bobCycle;
	int pmFlags;
	int pmRunTime;
	vec3_t origin;
	vec3_t velocity;
	int gravity;
	int speed;
	int deltaAngles[3];
	int groundEntityNum;
	qboolean walking;
	qboolean groundPlane;
	int feetFalling;
	vec3_t fallDir;
	trace_t groundTrace;
	int clientNum;
	vec3_t viewAngles;
	int viewHeight;
	float leanAngle;
	int viewModelAnim;
	int viewModelAnimChanged;
	playerStat_e stats[MAX_PLAYERSTATS];
	int activeItems[8];
	int ammoNameIndex[MAX_WEAPONS];
	int ammoAmount[MAX_WEAPONS];
	int maxAmmoAmount[MAX_WEAPONS];
	musicMood_t currentMusicMood;
	musicMood_t fallbackMusicMood;
	float musicVolume;
	float musicVolumeFadeTime;
	int reverbType;
	float reverbLevel;
	vec4_t blend;
	float fov;
	vec3_t cameraOrigin;
	vec3_t cameraAngles;
	float cameraTime;
	vec3_t cameraOffset;
	vec3_t cameraPosOfs;
	int cameraFlags;
	vec3_t damageAngles;
	int ping;
	vec3_t eyePos;

} playerState_t;

extern playerState_t ps;

typedef struct gclient_s
{
  playerState_t ps;
  int ping;
  client_persistant_t pers;
  float cmd_angles[3];
  int lastActiveTime;
  int activeWarning;

} gclient_t;

typedef struct server_sound_s
{
  float origin[3];
  int entity_number;
  int channel;
  short int sound_index;
  float volume;
  float min_dist;
  float maxDist;
  float pitch;
  qboolean stop_flag;
  qboolean streamed;

} server_sound_t;

typedef struct clSnapshot_s
{
  qboolean valid;
  int snapFlags;
  int serverTime;
  int serverTimeResidual;
  int messageNum;
  int deltaNum;
  int ping;
  unsigned char areamask[32];
  int cmdNum;
  playerState_t ps;
  int numEntities;
  int parseEntitiesNum;
  int serverCommandNum;
  int number_of_sounds;
  server_sound_t sounds[64];

} clSnapshot_t;

typedef struct qtime_s
{
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;

} qtime_t;

typedef struct random_data_s
{
  int *fptr;
  int *rptr;
  int *state;
  int rand_type;
  int rand_deg;
  int rand_sep;
  int *end_ptr;

} random_data_s;

typedef struct drand48_data_s
{
  short unsigned int x[3];
  short unsigned int a[3];
  short unsigned int c;
  short unsigned int old_x[3];
  int init;

} drand48_data_t;

typedef struct aliasListNode_s
{
	char aliasName[40];
	char realName[128];
	float weight;
	BYTE stopFlag;
	struct AliasListNode_s *next;
	float pitch;
	float volume;
	float pitchMod;
	float volumeMod;
	float dist;
	float maxDist;
	int channel;
	int streamed;
	char *subtitle;

} aliasListNode_t;

typedef struct aliasList_s
{
	char name[40];
	qboolean dirty;
	int numInList;
	aliasListNode_t **sortedList;
	aliasListNode_t *dataList;

} aliasList_t;

typedef struct usercmd_s
{
	int serverTime;
	BYTE msec;
	short unsigned int buttons;
	short int angles[3];
	signed char forwardmove;
	signed char rightmove;
	signed char upmove;

} userCmd_t;

typedef struct pmove_s
{
  playerState_t *ps;
  userCmd_t cmd;
  int tracemask;
  qboolean noFootsteps;
  int framecount;
  int numtouch;
  int touchents[32];
  int moveresult;
  qboolean stepped;
  int pmoveEvent;
  float mins[3];
  float maxs[3];
  int watertype;
  int waterlevel;
  int pmove_fixed;
  int pmove_msec;
  void (*trace) ();
  int (*pointcontents) ();

} pmove_t;

typedef struct pml_s {
  float forward[3];
  float left[3];
  float up[3];
  float flat_forward[3];
  float flat_left[3];
  float flat_up[3];
  float forward_speed;
  float side_speed;
  float frametime;
  int msec;
  int walking;
  int groundPlane;
  struct {
    int allsolid;
    int startsolid;
    float fraction;
    float endpos[3];
    struct cplane_s plane;
    int surfaceFlags;
    int shaderNum;
    int contents;
    int entityNum;
    int location;
    struct gentity_s *ent;
  } groundTrace;
  float impactSpeed;
  float previous_origin[3];
  float previous_velocity[3];
  int previous_waterlevel;
} pml_t;

typedef struct trajectory_s
{
	int trTime;
	vec3_t trDelta;

} trajectory_t;

typedef struct frameInfo_s
{
	int index;
	float time;
	float weight;

} frameInfo_t;

typedef struct gameState_s
{
	int stringOffsets[MAX_CONFIGSTRINGS];
	char stringData[MAX_GAMESTATECHARS];
	int dataCount;

} gameState_t;

typedef struct entityState_s
{
	int number;
	entityType_t eType;
	int eFlags;
	trajectory_t pos;
	vec3_t netOrigin;
	vec3_t origin;
	vec3_t origin2;
	vec3_t netAngles;
	vec3_t angles;
	int constantLight;
	int loopSound;
	float loopSoundVolume;
	float loopSoundMinDist;
	float loopSoundMaxDist;
	float loopSoundPitch;
	int loopSoundFlags;
	int parent;
	int tagNum;
	qboolean attachUseAngles;
	vec3_t attachOffset;
	int beamEntNum;
	int modelIndex;
	int usageIndex;
	int skinNum;
	int wasFrame;
	frameInfo_t frameInfo[MAX_FRAMEINFOS];
	float actionWeight;
	int boneTag[MAX_BONECONTROLLERS];
	vec3_t boneAngles[MAX_BONECONTROLLERS];
	vec4_t boneQuat[MAX_BONECONTROLLERS];
	BYTE surfaces[MAX_SURFACES];
	int clientNum;
	int groundEntityNum;
	int solid;
	float scale;
	float alpha;
	int renderFx;
	float shaderData[2];
	float shaderTime;
	vec4_t quat;
	vec3_t eyeVector;

} entityState_t;

typedef struct
{
	entityState_t s;
	qboolean linked;
	int	linkcount;
	int svFlags;
	int	singleClient;
	qboolean bmodel;
	vec3_t mins, maxs;
	int	contents;
	vec3_t absmin, absmax;
	vec3_t currentOrigin;
	vec3_t currentAngles;
	int	ownerNum;

} entityShared_t;

typedef struct clientActive_s
{
  int timeoutcount;
  clSnapshot_t snap;
  int serverTime;
  int oldServerTime;
  int serverStartTime;
  int oldFrameServerTime;
  int serverTimeDelta;
  qboolean extrapolatedSnapshot;
  qboolean newSnapshots;
  gameState_t gameState;
  char mapname[64];
  int parseEntitiesNum;
  int mouseDx[2];
  int mouseDy[2];
  int mouseIndex;
  int mousex;
  int mousey;
  unsigned int mouseButtons;
  int joystickAxis[6];
  userCmd_t cmds[128];
  int cmdNumber;
  outPacket_t outPackets[32];
  vec3_t viewangles;
  int serverId;
  clSnapshot_t snapshots[32];
  entityState_t entityBaselines[1024];
  entityState_t parseEntities[2048];

} clientActive_t;

typedef struct netAdr_s
{
	netAdrType_t type;
	BYTE ip[4];
	BYTE ipx[10];
	short unsigned int port;

} netAdr_t;

typedef struct netChan_s
{
	netSrc_t sock;
	int dropped;
	netAdr_t remoteAddress;
	int qport;
	int incomingSequence;
	int outgoingSequence;
	int fragmentSequence;
	int fragmentLength;
	BYTE fragmentBuffer[49152];
	qboolean unsentFragments;
	int unsentFragmentStart;
	int unsentLength;
	BYTE unsentBuffer[49152];

} netChan_t;

typedef struct clientConnection_s
{
	int			clientNum;
	int			lastPacketSentTime;
	int			lastPacketTime;
	netAdr_t	serverAddress;
	int			z_unknown1;
	int			connectTime;				// for connection retransmits
	int			connectPacketCount;			// for display on connection dialog
	char		serverMessage[MAX_STRINGTOKENS];	// for display on connection dialog
	int			challenge;					// from the server to use for connecting
	int			checksumFeed;				// from the server for checksum calculations
	int			reliableSequence;
	int			reliableAcknowledge;		// the last one the server has executed
	char		reliableCommands[MAX_RELIABLECOMMANDS][MAX_STRINGCHARS/2];
	int			serverMessageSequence;
	char		serverCommands[MAX_RELIABLECOMMANDS][MAX_STRINGCHARS/2];
	int			serverCommandSequence;
	fileHandle_t download;
	char		downloadTempName[MAX_OSPATH];
	char		downloadName[MAX_OSPATH];
	int			downloadNumber;
	int			downloadBlock;	// block we are waiting for
	int			downloadCount;	// how many bytes we got
	int			downloadSize;	// how many bytes we got
	char		downloadList[MAX_INFOSTRING]; // list of paks we need to download
	qboolean	downloadRestart;	// if true, we need to do another FS_Restart because we downloaded a pak
	char		demoName[MAX_QPATH];
	qboolean	demorecording;
	qboolean	demoplaying;
	qboolean	demowaiting;	// don't record until a non-delta message is received
	qboolean	firstDemoFrameSkipped;
	fileHandle_t	demofile;
	int			timeDemoFrames;		// counter of rendered frames
	int			timeDemoStart;		// cls.realtime before first frame
	int			timeDemoBaseTime;	// 0x00FDA880 - each frame will be at this time + frameNum * 50

	BYTE		z_unknown2[0x1803C]; //-- netchan_t struct

} clientConnection_t;

typedef struct {
  netAdr_t adr;
  int start;
  int time;
  char info[1350];
} ping_t;

typedef struct
{
  float x;
  float y;
  float w;
  float h;

} rectDef_t;

typedef rectDef_t Rectangles;

// FIXME: do something to separate text vs window stuff
typedef struct
{
  Rectangles rect;                 // client coord rectangle
  Rectangles rectClient;           // screen coord rectangle
  const char *name;               //
  const char *group;              // if it belongs to a group
  const char *cinematicName;		  // cinematic name
  int cinematic;								  // cinematic handle
  int style;                      //
  int border;                     //
  int ownerDraw;									// ownerDraw style
	int ownerDrawFlags;							// show flags for ownerdraw items
  float borderSize;               //
  int flags;                      // visible, focus, mouseover, cursor
  Rectangles rectEffects;          // for various effects
  Rectangles rectEffects2;         // for various effects
  int offsetTime;                 // time based value for various effects
  int nextTime;                   // time next effect should cycle
  vec4_t foreColor;               // text color
  vec4_t backColor;               // border color
  vec4_t borderColor;             // border color
  vec4_t outlineColor;            // border color
  qhandle_t background;           // background asset

} windowDef_t;

typedef windowDef_t Window;

typedef struct Class_s
{
	void		*vftable;
	void		*safePtrList;
} Class_t, Class;

typedef struct Event_s {
	Class				baseClass;
	int					fromScript;
	unsigned short		eventnum;
	unsigned short		dataSize;
	void				*data;
} Event_t, Event;

typedef struct Event_s {
	Class				baseClass;
	unsigned short		eventnum;
	unsigned short		dataSize;
	void				*data;
} Event2;

typedef struct ResponseDef_s {
	Event_t			*event;
	void			( __stdcall *response )( void *this, Event_t *ev );
	void			( __stdcall *response2 )( void *this, Event_t *ev );
} ResponseDef;

typedef struct ClassDef_s {
	const char			*classname;
	const char			*classID;
	const char			*superclass;
	void				*(*newInstance)(void);
	int					classSize;
	DWORD				*responses;
	DWORD				**responseLookup;
	struct ClassDef_s	*super;
	struct ClassDef_s	*next;
	struct ClassDef_s	*prev;
} ClassDef;

typedef struct Class_vtable_s {
	DWORD zero;
	void ( *_delete)( void *this );
	ClassDef *( *classinfo )( void *this );
	void ( *Archive)( void *arc );
} Class_vtable;

typedef struct Class_vtable_win_s {
	ClassDef *( *classinfo )( void *this );
	void ( *_delete)( void *this );
	void ( *Archive)( void *arc );
} Class_vtable_win;

typedef struct Class_linux_s {
	DWORD				unknown;
	Class_vtable		*vtable;
} Class_linux;

typedef struct Class_win_s {
	Class_vtable_win	*vtable;
	DWORD				unknown;
} Class_win;

typedef struct
{
	vec4_t	color;
	float	low;
	float	high;

} colorRangeDef_t;

typedef struct itemDef_s
{
  Window window;
  Rectangles textRect;
  int type;
  int alignment;
  int textalignment;
  float textalignx;
  float textaligny;
  float textscale;
  int textStyle;
  const char *text;
  void *parent;
  qhandle_t asset;
  const char *mouseEnterText;
  const char *mouseExitText;
  const char *mouseEnter;
  const char *mouseExit;
  const char *action;
  const char *onFocus;
  const char *leaveFocus;
  const char *cvar;
  const char *cvarTest;
  const char *enableCvar;
  int cvarFlags;
  sfxHandle_t clickSound;
  sfxHandle_t stopSound;
  int numColors;
  colorRangeDef_t colorRanges[MAX_COLORRANGES];
  float special;
  int cursorPos;
  void *typeData;

} itemDef_t;


typedef struct cvar_s
{
	char *name;
	char *string;
	char *resetString;
	char *latchedString;
	int flags;
	qboolean modified;
	int modificationCount;
	float value;
	int integer;
	struct cvar_s *next;
	struct cvar_s *hashNext;

} cvar_t;

typedef struct {
	cvar_t		**cvar;
	char		*name;		// removed const
	char		*value;		// removed const
	int			flags;
} regCvar_t;

typedef struct vmCvar_s
{
	cvarHandle_t handle;
	int modificationCount;
	float value;
	int integer;
	char string[256];

} vmCvar_t;

typedef struct letterLoc_s
{
	vec2_t pos;
	vec2_t size;

} letterLoc_t;

typedef struct fontHeader_s
{
	int indirection[256];
	letterLoc_t locations[256];
	char name[MAX_QPATH];
	float height;
	float aspectRatio;
	void *shader;
	int trHandle;

} fontheader_t;

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
	char string[MAX_STRINGCHARS];
	char fontName[MAX_QPATH];
	fontheader_t *font;

} hdElement_t;

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

} clientAnim_t;

typedef struct stopWatch_s
{
	int startTime;
	int endTime;

} stopWatch_t;

/*
typedef struct sigset_s
{
  long unsigned int __val[32];

} sigset_t;
*/

typedef struct sigvec_s
{
  int sv_mask;
  int sv_flags;

} sigvec_t;

typedef struct timespec_s
{
  long int tv_sec;
  long int tv_nsec;

} timespec_t;

typedef struct orientation_s
{
	vec3_t origin;
	vec3_t axis[3];

} orientation_t;

typedef struct orientationr_s {
  float origin[3];
  float axis[3][3];
  float viewOrigin[3];
  float modelMatrix[16];
} orientationr_t;

typedef struct savegamestruct_s
{
	int version;
	int time;
	int mapTime;
	char comment[64];
	char mapName[64];
	char saveName[64];
	char tm_filename[64];
	int tm_loopcount;
	int tm_offset;

} savegamestruct_t;

typedef struct media_s
{
	qhandle_t backTileShader;
	qhandle_t lagometerShader;
	qhandle_t shadowMarkShader;
	qhandle_t footShadowMarkShader;
	qhandle_t wakeMarkShader;
	qhandle_t pausedShader;
	qhandle_t levelExitShader;
	qhandle_t zoomOverlayShader;
	qhandle_t kar98TopOverlayShader;
	qhandle_t kar98BottomOverlayShader;
	qhandle_t binocularsOverlayShader;
	fontheader_t *hudDrawFont;
	fontheader_t *attackerFont;
	fontheader_t *objectiveFont;
	qhandle_t objectivesBackShader;
	qhandle_t checkedBoxShader;
	qhandle_t uncheckedBoxShader;

} media_t;

typedef struct glConfig_s
{
	char rendererString[MAX_STRINGCHARS];
	char vendorString[MAX_STRINGCHARS];
	char versionString[MAX_STRINGCHARS];
	char extensionsString[MAX_EXTSTRINGCHARS];
	int maxTextureSize;
	int maxActiveTextures;
	int colorBits;
	int depthBits;
	int stencilBits;
	glDriverType_t driverType;
	glHardwareType_t hardwareType;
	qboolean deviceSupportsGamma;
	int textureCompression;
	qboolean textureEnvAddAvailable;
	int vidWidth;
	int vidHeight;
	float windowAspect;
	int displayFrequency;
	qboolean isFullscreen;
	qboolean stereoEnabled;
	qboolean smpActive;
	int registerCombinerAvailable;
	qboolean secondaryColorAvailable;
	qboolean var;
	qboolean fence;

} glConfig_t;

typedef struct pcx_t
{
  char manufacturer;
  char version;
  char encoding;
  char bits_per_pixel;
  short unsigned int xmin;
  short unsigned int ymin;
  short unsigned int xmax;
  short unsigned int ymax;
  short unsigned int hres;
  short unsigned int vres;
  unsigned char palette[48];
  char reserved;
  char color_planes;
  short unsigned int bytes_per_line;
  short unsigned int palette_type;
  char filler[58];
  unsigned char data;

} pcx_t;

typedef struct TargaHeader_s
{
  unsigned char id_length;
  unsigned char colormap_type;
  unsigned char image_type;
  short unsigned int colormap_index;
  short unsigned int colormap_length;
  unsigned char colormap_size;
  short unsigned int x_origin;
  short unsigned int y_origin;
  short unsigned int width;
  short unsigned int height;
  unsigned char pixel_size;
  unsigned char attributes;

} TargaHeader;

typedef struct skelChannelList_s
{
	short int m_numChannels;
	short int m_numLocalFromGlobal;
	short int *m_chanLocalFromGlobal;
	short int m_chanGlobalFromLocal[200];

} skelChannelList_t;

typedef skelChannelList_s skelChannelList_c;

typedef struct {
	float x;
	float y;
	float z;
} SkelVec3;

typedef struct {
	float x;
	float y;
	float z;
	float w;
} SkelVec4;

typedef struct {
	float val[ 3 ][ 3 ];
} SkelMat3;

typedef struct {
	float val[ 4 ][ 3 ];
} SkelMat4;

typedef struct {
	float x;
	float y;
	float z;
	float w;
} SkelQuat;

typedef struct {
	SkelVec3 bounds[ 2 ];
	float radius;
	SkelVec3 delta;
	float angleDelta;
	int iOfsChannels; 
} skelAnimFileFrame_t;

typedef struct {
	int ident;
	int version;
	int flags;
	int nBytesUsed;
	float frameTime;
	SkelVec3 totalDelta;
	float totalAngleDelta;
	int numChannels;
	int ofsChannelNames;
	int numFrames;
	skelAnimFileFrame_t frame[ 1 ];
} skelAnimDataFileHeader_t;

typedef struct {
	SkelVec3 bounds[ 2 ];
	float radius;
	SkelVec3 delta;
	float angleDelta;
	float( *pChannels )[ 4 ];
} skelAnimGameFrame_t;

typedef struct {
	short int nFrameNum;
	short int nPrevFrameIndex;
	float pChannelData[ 4 ];
} skanGameFrame;

typedef struct {
	short int nFramesInChannel;
	skanGameFrame *ary_frames;
} skanChannelHdr;

typedef struct skelAnimDataGameHeader_s {
	int flags;
	int nBytesUsed;
	char bHasDelta;
	char bHasMorph;
	short bHasUpper;
	int numFrames;
	SkelVec3 totalDelta;
	float totalAngleDelta;
	float frameTime;
	skelChannelList_c channelList;
	SkelVec3 bounds[ 2 ];
	skelAnimGameFrame_t *m_frame;
	short int nTotalChannels;
	skanChannelHdr ary_channels[ 1 ];
} skelAnimDataGameHeader_t;

typedef struct skelAnimDataGameHeader2_s {
	int flags;
	char bHasDelta;
	char bHasMorph;
	short bHasUpper;
	int numFrames;
	SkelVec3 totalDelta;
	float totalAngleDelta;
	float frameTime;
	SkelVec3 bounds[ 2 ];
	skelAnimGameFrame_t *m_frame;
	short int nTotalChannels;
	skelChannelList_c channelList;
	skanChannelHdr ary_channels[ 1 ];
} skelAnimDataGameHeader2_t;

typedef struct ChannelName_s {
	char			name[ 32 ];
	short int		channelNum;
} ChannelName_t;

typedef struct ChannelNameTable_s {
	short int m_iNumChannels;
	ChannelName_t m_Channels[2048];
	short int m_lookup[2048];
} ChannelNameTable_t;

typedef struct skelAnimTime_s
{
	float seconds;

} skelAnimTime_t;

typedef struct skelAnimFrame_s
{
	float radius;
	vec3_t bounds[2];
	vec3_t bones[4];

} skelAnimFrame_t;

typedef struct skelBone_Base_s {
	qboolean				m_isDirty;
	struct skelBone_Base_s	*m_parent;
	SkelMat4				m_cachedValue;
	float					*m_controller;
	void					*_vptr;
} skelBone_Base;

typedef struct {
	skelBone_Base skelBone_Base_parent;
} skelBone_World;

typedef struct {
	skelBone_Base skelBone_Base_parent;
} skelBone_Zero;

typedef struct {
	skelBone_Base skelBone_Base_parent;
	SkelVec3 m_baseValue;
	int m_quatChannel;
} skelBone_Rotation;

typedef struct {
	skelBone_Base skelBone_Base_parent;
	int m_quatChannel;
	int m_offsetChannel;
} skelBone_PosRot;

typedef struct {
	skelBone_PosRot skelBone_PosRot_parent;
} skelBone_Root;

typedef struct {
	skelBone_Base skelBone_Base_parent;
	struct skelBone_IKwrist *m_wrist;
	float m_upperLength;
	float m_lowerLength;
	SkelVec3 m_baseValue;
	float m_cosElbowAngle;
	SkelVec3 m_wristPos;
	SkelQuat m_wristAngle;
} skelBone_IKshoulder;

typedef struct {
	skelBone_Base skelBone_Base_parent;
	skelBone_IKshoulder *m_shoulder;
} skelBone_IKelbow;

typedef struct {
	skelBone_Base skelBone_Base_parent;
	skelBone_IKshoulder *m_shoulder;
	int m_quatChannel;
	int m_offsetChannel;
} skelBone_IKwrist;

typedef struct {
	skelBone_Base skelBone_Base_parent;
	SkelVec3 m_basePos;
	SkelQuat m_cachedQuat;
	skelBone_Base *m_reference1;
	skelBone_Base *m_reference2;
	float m_bone2weight;
} skelBone_AvRot;

typedef struct {
	skelBone_Base skelBone_Base_parent;
	SkelVec3 m_basePos;
	SkelQuat m_cachedQuat;
	skelBone_Base *m_target;
	float m_bendRatio;
	float m_bendMax;
	float m_spinRatio;
} skelBone_HoseRot;

typedef struct {
	skelBone_HoseRot skelBone_HoseRot_parent;
} skelBone_HoseRotBoth;

typedef struct {
	skelBone_HoseRot skelBone_HoseRot_parent;
} skelBone_HoseRotParent;

typedef struct ftx_s
{
  int width;
  int height;
  int has_alpha;
} ftx_t;

typedef struct fcm_s
{
  int iWidth;
  int iHeight;

} fcm_t;

typedef struct varnode_s
{
	short unsigned int flags;
} varnode_t;

typedef struct varnodeUnpacked_u {
	byte flags;
	byte unused_0;
	byte unused_1;
	byte unused_2;
} varnodeUnpacked_t;

typedef unsigned short terraInt;

typedef struct terrainVert_s {
	vec3_t xyz;
	vec2_t texCoords[ 2 ];
	float fVariance;
	float fHgtAvg;
	float fHgtAdd;
	unsigned int uiDistRecalc;
	terraInt nRef;
	terraInt iVertArray;
	byte *pHgt;
	terraInt iNext;
	terraInt iPrev;
} terrainVert_t;

typedef struct terraTri_s {
	unsigned short iPt[ 3 ];
	terraInt nSplit;
	unsigned int uiDistRecalc;
	struct cTerraPatchUnpacked_s *patch;
	varnodeUnpacked_t *varnode;
	terraInt index;
	byte lod;
	byte byConstChecks;
	terraInt iLeft;
	terraInt iRight;
	terraInt iBase;
	terraInt iLeftChild;
	terraInt iRightChild;
	terraInt iParent;
	terraInt iPrev;
	terraInt iNext;
} terraTri_t;

typedef struct srfTerrain_s {
	surfaceType_t surfaceType;
	terraInt iVertHead;
	terraInt iTriHead;
	terraInt iTriTail;
	terraInt iMergeHead;
	int nVerts;
	int nTris;
	int lmapSize;
	int dlightBits[ 2 ];
	float lmapStep;
	int dlightMap[ 2 ];
	byte *lmData;
	float lmapX;
	float lmapY;
} srfTerrain_t;

typedef struct cTerraPatch_s {
	byte	flags;
	byte	lmapScale;
	byte	s;
	byte	t;

	float	texCoord[ 2 ][ 2 ][ 2 ];

	char	x;
	char	y;

	short			iBaseHeight;
	unsigned short	iShader;
	unsigned short	iLightMap;

	short	iNorth;
	short	iEast;
	short	iSouth;
	short	iWest;

	varnode_t		varTree[ 2 ][ 63 ];

	unsigned char	heightmap[ 9 * 9 ];
} cTerraPatch_t;

typedef struct cTerraPatchUnpacked_s {
	srfTerrain_t drawinfo;
	int viewCount;
	int visCountCheck;
	int visCountDraw;
	int frameCount;
	unsigned int uiDistRecalc;
	float s;
	float t;
	float texCoord[ 2 ][ 2 ][ 2 ];
	float x0;
	float y0;
	float z0;
	float zmax;
	shader_t *shader;
	short int iNorth;
	short int iEast;
	short int iSouth;
	short int iWest;
	struct cTerraPatchUnpacked_s *pNextActive;
	varnodeUnpacked_t varTree[ 2 ][ 63 ];
	unsigned char heightmap[ 81 ];
	byte flags;
	byte byDirty;
} cTerraPatchUnpacked_t;

typedef struct cStaticModel_s
{
  char model[128];
  float origin[3];
  float angles[3];
  float scale;
  int firstVertexData;
  int numVertexData;

} cStaticModel_t;

typedef struct cStaticModelUnpacked_s {
	qboolean useSpecialLighting;
	qboolean bLightGridCalculated;
	qboolean bRendered;
	char model[ 128 ];
	vec3_t origin;
	vec3_t angles;
	vec3_t axis[ 3 ];
	float scale;
	int firstVertexData;
	int numVertexData;
	int visCount;
	dtiki_t *tiki;
	sphere_dlight_t dlights[ 32 ];
	int numdlights;
	float radius;
	float cull_radius;
	int iGridLighting;
	float lodpercentage[ 2 ];
} cStaticModelUnpacked_t;

typedef struct gamelump_s
{
  void *buffer;
  int length;

} gamelump_t;

typedef struct lump_s
{
  int fileofs;
  int filelen;

} lump_t;

typedef struct dheader_s
{
  int ident;
  int version;
  int checksum;
  lump_t lumps[28];

} dheader_t;

typedef struct dmodel_s
{
  float mins[3];
  float maxs[3];
  int firstSurface;
  int numSurfaces;
  int firstBrush;
  int numBrushes;

} dmodel_t;

typedef struct dshader_s
{
  char shader[64];
  int surfaceFlags;
  int contentFlags;
  int subdivisions;
  char fenceMaskImage[64];

} dshader_t;

typedef struct dplane_s
{
  float normal[3];
  float dist;

} dplane_t;

typedef struct dnode_s
{
  int planeNum;
  int children[2];
  int mins[3];
  int maxs[3];

} dnode_t;

typedef struct dleaf_s
{
  int cluster;
  int area;
  int mins[3];
  int maxs[3];
  int firstLeafSurface;
  int numLeafSurfaces;
  int firstLeafBrush;
  int numLeafBrushes;
  int firstTerraPatch;
  int numTerraPatches;
  int firstStaticModel;
  int numStaticModels;

} dleaf_t;

typedef struct dsideequation_s
{
  float fSeq[4];
  float fTeq[4];

} dsideequation_t;

typedef struct dbrushside_s
{
  int planeNum;
  int shaderNum;
  int equationNum;

} dbrushside_t;

typedef struct dbrush_s
{
  int firstSide;
  int numSides;
  int shaderNum;

} dbrush_t;

typedef struct dfog_s
{
  char shader[64];
  int brushNum;
  int visibleSide;

} dfog_t;

typedef struct drawVert_s
{
  float xyz[3];
  float st[2];
  float lightmap[2];
  float normal[3];
  unsigned char color[4];

} drawVert_t;

typedef struct drawSoupVert_s
{
  float xyz[3];
  float st[2];
  int collapseMap;
  float lodExtra;
  float normal[3];
  unsigned char color[4];

} drawSoupVert_t;

typedef struct dsurface_s
{
  int shaderNum;
  int fogNum;
  int surfaceType;
  int firstVert;
  int numVerts;
  int firstIndex;
  int numIndexes;
  int lightmapNum;
  int lightmapX;
  int lightmapY;
  int lightmapWidth;
  int lightmapHeight;
  float lightmapOrigin[3];
  float lightmapVecs[3][3];
  int patchWidth;
  int patchHeight;
  float subdivisions;

} dsurface_t;

typedef struct dlightGrid_s
{
  float origin[3];
  float axis[3];
  int bounds[3];

} dlightGrid_t;

typedef struct dlightdef_s
{
  int lightIntensity;
  int lightAngle;
  int lightmapResolution;
  qboolean twoSided;
  qboolean lightLinear;
  float lightColor[3];
  float lightFalloff;
  float backsplashFraction;
  float backsplashDistance;
  float lightSubdivide;
  qboolean autosprite;

} dlightdef_t;

typedef struct mapspherel_s
{
  float origin[3];
  float color[3];
  float intensity;
  int leaf;
  qboolean needs_trace;
  qboolean spot_light;
  float spot_dir[3];
  float spot_radiusbydistance;

} mapspherel_t;

typedef struct miptex_s
{
  char name[32];
  unsigned int width;
  unsigned int height;
  unsigned int offsets[4];
  char animname[32];
  int flags;
  int contents;
  int value;

} miptex_t;

typedef struct gridrow_s
{
  BYTE size;
  BYTE *data;

} gridrow_t;

typedef struct msg_s
{
  qboolean allowoverflow;
  qboolean overflowed;
  qboolean oob;
  BYTE *data;
  int maxsize;
  int cursize;
  int readcount;
  int bit;

} msg_t;

typedef struct tiki_singlecmd_s
{
  int num_args;
  char **args;

} tiki_singlecmd_t;

typedef struct tiki_cmd_s
{
  int num_cmds;
  tiki_singlecmd_t cmds[128];

} tiki_cmd_t;

typedef struct dtikicmd_s
{
	int frame_num;
	int num_args;
	char **args;

} dtikicmd_t;

typedef struct dtikianimdef_s
{
	char alias[48];
	float weight;
	float blendtime;
	int flags;
	int num_client_cmds;
	dtikicmd_t *client_cmds;
	int num_server_cmds;
	dtikicmd_t *server_cmds;

} dtikianimdef_t;

typedef struct dtikianim_s
{
	char *name;
	int num_anims;
	void *alias_list;
	int num_client_initcmds;
	dtikicmd_t *client_initcmds;
	int num_server_initcmds;
	dtikicmd_t *server_initcmds;
	BYTE *modelData;
	int modelDataSize;
	vec3_t mins;
	vec3_t maxs;
	short int *m_aliases;
	char *headmodels;
	char *headskins;
	qboolean bIsCharacter;
	dtikianimdef_t *animdefs[1];

} dtikianim_t;

typedef struct dtikiSurface_s
{
  char name[64];
  char shader[4][64];
  int hShader[4];
  int numskins;
  int flags;
  float damage_multiplier;

} dtikiSurface_t;

typedef struct skelHeader_s
{
  int ident;
  int version;
  char name[64];
  int numSurfaces;
  int numBones;
  int ofsBones;
  int ofsSurfaces;
  int ofsEnd;
  int lodIndex[10];
  int numBoxes;
  int ofsBoxes;
  int numMorphTargets;
  int ofsMorphTargets;

} skelHeader_t;

typedef struct skelBoneName_s
{
  short int parent;
  short int boxIndex;
  int flags;
  char name[64];

} skelBoneName_t;

typedef struct skelSurface_s
{
  int ident;
  char name[64];
  int numTriangles;
  int numVerts;
  int staticSurfProcessed;
  int ofsTriangles;
  int ofsVerts;
  int ofsCollapse;
  int ofsEnd;
  int ofsCollapseIndex;

} skelSurface_t;

typedef short int skelIndex_t;

typedef struct skelWeight_s
{
  int boneIndex;
  float boneWeight;
  float offset[3];

} skelWeight_t;

typedef struct skelVertex_s
{
  float normal[3];
  float texCoords[2];
  int numWeights;
  skelWeight_t weights[1];

} skelVertex_t;

typedef struct skeletorVertex_s
{
	float normal[3];
	float texCoords[2];
	int numWeights;
	int numMorphs;

} skeletorVertex_t;

typedef struct skelSurfaceGame_s
{
  int ident;
  char name[64];
  int numTriangles;
  int numVerts;
  float (*pStaticXyz)[4];
  float (*pStaticNormal)[4];
  float (*pStaticTexCoords)[2][2];
  skelIndex_t *pTriangles;
  skeletorVertex_t *pVerts;
  skelIndex_t *pCollapse;
  struct skelSurfaceGame_s *pNext;
  skelIndex_t *pCollapseIndex;

} skelSurfaceGame_t;

typedef struct boneData_s
{
	short channel;
	int boneType;
	short parent;
	short numChannels;
	short numRefs;
	short channelIndex[2];
	short refIndex[2];
	float offset[3];
	float length;
	float weight;
	float bendRatio;
	float bendMax;
	float spinRatio;
	int pad1;

} boneData_t;

typedef struct skelAnimFrame_s {
	float radius;
	SkelVec3 bounds[ 2 ];
	SkelMat4 bones[ 1 ];
} skelAnimFrame_t;

typedef struct {
	float seconds;
} skelAnimTime_t;

typedef struct {
	float weight;
	skelAnimDataGameHeader_t *pAnimationData;
	skelAnimGameFrame_t *frame;
} skelAnimBlendFrame_t;

typedef struct {
	float weight;
	skelAnimDataGameHeader_t *pAnimationData;
	int frame;
} skanBlendInfo;

typedef struct {
	short int numMovementFrames;
	short int numActionFrames;
	float actionWeight;
	skanBlendInfo m_blendInfo[ 64 ];
} skelAnimStoreFrameList_c;

typedef struct {
	struct dtiki_s				*m_Tiki;

	SkelVec3 m_frameBounds[ 2 ];
	float m_frameRadius;
	skelAnimStoreFrameList_c m_frameList;
	short int m_targetLookLeft;
	short int m_targetLookRight;
	short int m_targetLookUp;
	short int m_targetLookDown;
	short int m_targetLookCrossed;
	short int m_targetBlink;
	short int m_timeNextBlink;
	short int m_headBoneIndex;
	float m_eyeTargetPos[ 3 ];
	float m_eyePrevTargetPos[ 3 ];
	skelBone_Base *m_leftFoot;
	skelBone_Base *m_rightFoot;
	skelChannelList_c m_morphTargetList;
	skelBone_Base **m_bone;
} skeletor_c;

typedef struct boneFileData_s {
	char name[ 32 ];
	char parent[ 32 ];
	boneType_t boneType;
	int ofsBaseData;
	int ofsChannelNames;
	int ofsBoneNames;
	int ofsEnd;
} boneFileData_t;

typedef struct skelHitBox_s {
	int boneIndex;
} skelHitBox_t;

typedef struct {
	int ident;
	int version;
	char name[ 64 ];
	int numSurfaces;
	int numBones;
	int ofsBones;
	int ofsSurfaces;
	int ofsEnd;
	int lodIndex[ 10 ];
	int numBoxes;
	int ofsBoxes;
	int numMorphTargets;
	int ofsMorphTargets;
} skelBaseHeader_t;

typedef struct lodCurvePoint_s
{
  float pos;
  float val;

} lodCurvePoint_t;

typedef struct lodCurvePointConstants_s
{
  float base;
  float scale;
  float cutoff;

} lodCurvePointConstants_t;

typedef struct lodControl_s
{
  float minMetric;
  float maxMetric;
  lodCurvePoint_t curve[5];
  lodCurvePointConstants_t consts[4];

} lodControl_t;

typedef struct skelHeaderGame_s
{
  int version;
  char name[64];
  int numSurfaces;
  int numBones;
  boneData_t *pBones;
  skelSurfaceGame_t *pSurfaces;
  int lodIndex[10];
  int numBoxes;
  skelHitBox_t *pBoxes;
  lodControl_t *pLOD;
  int numMorphTargets;
  char *pMorphTargets;

} skelHeaderGame_t;

typedef struct tikiSurface_s
{
  int ident;
  char name[64];
  int numFrames;
  int numVerts;
  int minLod;
  int numTriangles;
  int ofsTriangles;
  int ofsCollapseMap;
  int ofsSt;
  int ofsXyzNormals;
  int ofsEnd;

} tikiSurface_t;

typedef struct dtikisurface_s {
	char name[ 64 ];
	char shader[ 4 ][ 64 ];
	int hShader[ 4 ];
	int numskins;
	int flags;
	float damage_multiplier;
} dtikisurface_t;

typedef struct tiki_tess_data_s
{
  int ident;
  tikiSurface_t *newsurface;
  tikiSurface_t *oldsurface;

} tiki_tess_data_t;

typedef struct skeletorVertexGame_s
{
  float normal[3];
  float texCoords[2];
  int numWeights;
  int numMorphs;

} skeletorVertexGame_s;

typedef struct staticSurface_s
{
  int ident;
  int ofsStaticData;
  skelSurfaceGame_t *surface;
  int meshNum;

} staticSurface_t;

typedef struct skeletorMorph_s
{
  int morphIndex;
  float offset[3];

} skeletorMorph_t;

typedef struct skelBone_s
{
  short int shortQuat[4];
  short int shortOffset[3];
  short int pad;

} skelBone_t;

typedef struct skelFrame_s
{
  float bounds[2][3];
  float radius;
  float delta[3];
  skelBone_t bones[1];

} skelFrame_t;

typedef struct skelAnimHeader_s
{
  int ident;
  int version;
  char name[64];
  int type;
  int numFrames;
  int numbones;
  float totaltime;
  float frametime;
  float totaldelta[3];
  int ofsFrames;

} skelAnimHeader_t;

typedef struct skelBoneCache_s
{
  float offset[4];
  float matrix[3][4];

} skelBoneCache_t;

typedef struct skelcache_s
{
  char path[96];
  skelHeaderGame_t *skel;
  int size;
  int numuses;

} skelcache_t;

typedef struct boneFileData_s
{
  char name[32];
  char parent[32];
  boneType_t boneType;
  int ofsBaseData;
  int ofsChannelNames;
  int ofsBoneNames;
  int ofsEnd;

} boneFileData_t;

typedef struct tikiSingleCmd_s
{
	int numArgs;
	char **args;

} tikiSingleCmd_t;

typedef struct tikiCmd_s
{
	int numCmds;
	tikiSingleCmd_t cmds[128];

} tikiCmd_t;

typedef struct tikiFrame_s
{
	vec3_t bounds[2];
	vec3_t scale;
	vec3_t offset;
	vec3_t delta;
	float radius;
	float frameTime;

} tikiFrame_t;

typedef struct dtiki_s
{
	char *name;
	dtikianim_t *a;
	void *skeletor;
	int numSurfaces;
	dtikiSurface_t *surfaces;
	float loadScale;
	float lodScale;
	float lodBias;
	vec3_t lightOffset;
	vec3_t loadOrigin;
	float radius;
	skelChannelList_t boneList;
	int numMeshes;
	short int mesh[1];

} dtiki_t;

typedef struct {
	char path[ 100 ];
	skelAnimDataGameHeader_t *data;
	int numusers;
	int lookup;
} skeletorCacheEntry_t;

typedef struct {
	int indexes[ 3 ];
} tikiTriangle_t;

typedef struct {
	float st[ 2 ];
} tikiSt_t;

typedef struct {
	short unsigned int xyz[ 3 ];
	short int normal;
} tikiXyzNormal_t;

typedef struct {
	float origin[ 3 ];
	float axis[ 3 ][ 3 ];
} tikiTagData_t;

typedef struct {
	char name[ 64 ];
} tikiTag_t;

typedef struct {
	qboolean valid;
	int surface;
	float position[ 3 ];
	float normal[ 3 ];
	float damage_multiplier;
} tikimdl_intersection_t;

typedef struct {
	int indexes[ 3 ];
} skelTriangle_t;

typedef struct dtikicmd_s {
	int frame_num;
	int num_args;
	char **args;
} dtikicmd_t;

typedef struct dloadframecmd_s {
	int frame_num;
	int num_args;
	char **args;
	char location[ 256 ];
} dloadframecmd_t;

typedef struct {
	int num_args;
	char **args;
} dloadinitcmd_t;

typedef struct {
	char name[ 32 ];
	char shader[ 4 ][ 64 ];
	int numskins;
	int flags;
	float damage_multiplier;
} dloadsurface_t;

typedef struct {
	char *alias;
	char name[ 128 ];
	char location[ 256 ];
	float weight;
	float blendtime;
	int flags;
	int num_client_cmds;
	int num_server_cmds;
	dloadframecmd_t *loadservercmds[ 32 ];
	dloadframecmd_t *loadclientcmds[ 128 ];
} dloadanim_t;

typedef struct {
	char name[ 256 ];
	char macro[ 256 ];
} tiki_macro_t;

typedef struct {
	char *mark_script_p;
	qboolean mark_tokenready;
	char mark_token[ 256 ];
} tiki_mark_t;

typedef struct TikiScript_s {
	qboolean error;
	qboolean tokenready;
	struct TikiScript_s *include;
	struct TikiScript_s *parent;
	char filename[ 256 ];
	char *script_p;
	char *end_p;
	tiki_macro_t macros[ 48 ];
	int nummacros;
	int line;
	char token[ 256 ];
	qboolean releaseBuffer;
	tiki_mark_t mark[ 64 ];
	int mark_pos;
	char *buffer;
	int length;
	char path[ 64 ];
} TikiScript;

typedef struct {
	char *path;
	TikiScript tikiFile;
	dloadanim_t *loadanims[ 4095 ];
	dloadinitcmd_t *loadserverinitcmds[ 160 ];
	dloadinitcmd_t *loadclientinitcmds[ 160 ];
	int skelIndex_ld[ 12 ];
	int numanims;
	int numserverinitcmds;
	int numclientinitcmds;
	char headmodels[ 4096 ];
	char headskins[ 4096 ];
	qboolean bIsCharacter;
	struct msg_s *modelBuf;
	unsigned char modelData[ 8192 ];
	qboolean bInIncludesSection;
} dloaddef_t;

typedef struct Container_s {
	void	*objlist;
	int		numobjects;
	int		maxobjects;
} Container_t, Container;

typedef struct Stack_s {
	void *head;
} Stack;

typedef struct
{
	char		*buffer;
	int			ref;
	int			alloced;
	int			len;

} strdata_t;

typedef struct str_s
{
    strdata_t *s;

} str;

typedef vec3_t Vector;

typedef struct spawnsort_s {
	void *spawnpoint;
	float fMetric;
} spawnsort_t;

typedef struct SafePtr2_s {
	struct Class *ptr;
    struct SafePtr2_s *prev;
    struct SafePtr2_s *next;
	void *_vptr$;
} SafePtr2_t, SafePtr;

typedef struct ContainerClass_s {
	Class_t baseClass;
	Container_t value;
} ContainerClass_t;

typedef struct eventInfo
{
	void		*ev;
	const char	*command;
	int			flags;
	const char	*formatspec;
	const char	*argument_names;
	const char	*documentation;
	int			returnType;
	struct eventInfo_s	*prev2;
	struct eventInfo_s	*prev;
} eventInfo_t;

typedef struct command_s {
	const char 		*command;
	int				flags;
	const char		*formatspec;
	const char		*argument_names;
	const char		*documentation;
	int				returnType;
} command_t;

typedef struct Entry_s {
	int						key;
	int						value;
	struct Entry_s			*next;
} Entry;

typedef struct con_set_s {
	Entry						**table;
	unsigned int				tableLength;
	unsigned int				threshold;
	unsigned int				count;
	short unsigned int			tableLengthIndex;
	Entry						*defaultEntry;
} con_set;

typedef struct con_arrayset_s {
	Entry						**table;
	unsigned int				tableLength;
	Entry						**reverseTable;
	unsigned int				threshold;
	unsigned int				count;
	short unsigned int			tableLengthIndex;
	Entry						*defaultEntry;
} con_arrayset;

typedef struct con_map_s {
	con_set			conset;
} con_map;

typedef struct con_set_enum_s {
	con_set			*m_Set;
	unsigned int	m_Index;
	Entry			*m_CurrentEntry;
	Entry			*m_NextEntry;
} con_set_enum_t;

typedef struct con_map_enum_s {
	con_set_enum_t	m_Set_Enum;
} con_map_enum_t;

typedef struct con_timer_s {
	Class_t				baseClass;
	Container_t			m_Elements;
	bool				m_bDirty;
	int					inttime;
} con_timer;

typedef struct ScriptVariable_s {
	BYTE	un1[3];
	BYTE	type;
	void	*m_data;
} ScriptVariable_t;

typedef struct ScriptThreadLabel_s {
	void			*m_Script;
	str				m_Label;
} ScriptThreadLabel_t;

typedef struct Listener_s
{
	Class_t				baseClass;
	void				*m_NotifyList;
	void				*m_WaitForList;
	void				*vars;
} Listener_t, Listener;

typedef struct Listener2_s
{
	Class_t				baseClass;
} Listener2_t, Listener2;

typedef struct SimpleEntity_s {
	Listener_t baseListener;
	Vector origin;
	Vector angles;
	int entflags;
	str target;
	str targetname;
	Vector centroid;
} SimpleEntity_t;

typedef struct Entity_s
{
	SimpleEntity_t baseSimple;
    int entnum;
    int radnum;
    struct gentity_s *edict;
    gclient_t *client;
    int spawnflags;
    str model;
    Vector mins;
    Vector maxs;
    Vector absmin;
    Vector absmax;
    Vector velocity;
    Vector accel;
    Vector avelocity;
    Vector aaccel;
    Vector size;
    int moveType;
    int mass;
    float gravity;
    float orientation[3][3];
    struct gentity_s *groundentity;
    struct cplane_s groundplane;
    int groundcontents;
    int numchildren;
    int children[8];
    float lightRadius;
    str moveteam;
    struct Entity_s *teamchain;
    struct Entity_s  *teammaster;
    struct Entity_s  *bindmaster;
    qboolean bind_use_my_angles;
    Vector localorigin;
    Vector localangles;
    str killtarget;
    float health;
    float max_health;
    int deadflag;
    int flags;
    int watertype;
    int waterlevel;
    damage_t takedamage;
    DWORD enemy[4];
    float pain_finished;
    float damage_debounce_time;
    int damage_type;
    int m_iNumGlues;
    DWORD m_pGlues[32];
    int m_pGluesFlags[8];
    struct Entity_s *m_pGlueMaster;
    qboolean m_bGlueAngles;
    qboolean detach_at_death;
    float stealthMovementScale;
    DWORD *m_BlockedPaths;
    int m_iNumBlockedPaths;
    Container_t immunities;

} Entity_t;

/* ==============================================================================================================*/
typedef struct gentity_s
{
	entityState_t s;
	gclient_t *client;
	qboolean inuse;
	qboolean linked;
	int linkcount;
	int svflags;
	int singleClient;
	int lastNetTime;
	qboolean isModel;
	vec3_t mins, maxs;
	int	contents;
	vec3_t absmin, absmax;
	float radius;
	vec3_t centroid;
	int areanum;
	vec3_t currentAngles;
	int ownerNum;
	solid_t solid;
	dtiki_t *tiki;
	float mat[3][3];
	struct Entity_s	*entity;
	float freetime;
	float spawntime;
	float radius2;
	char entname[64];
	int clipmask;
	struct gentity_s *next;
	struct gentity_s *prev;

} gentity_t;

typedef struct treadMark_s
{
	int referenceNumber;
	int lastTime;
	qhandle_t treadShader;
	int state;
	float width;
	vec3_t startDir;
	vec3_t startVerts[2];
	float startTexCoord;
	float startAlpha;
	vec3_t midPos;
	vec3_t midVerts[2];
	float midTexCoord;
	float midAlpha;
	vec3_t endPos;
	vec3_t endVerts[2];
	float endTexCoord;
	float endAlpha;

} treadMark_t;

typedef struct markFragment_s
{
	int firstPoint;
	int numPoints;
	int index;

} markFragment_t;

typedef struct baseShader_s
{
	char shader[MAX_QPATH];
	int surfaceFlags;
	int contentFlags;

} baseShader_t;

typedef struct clientInfo_s
{
	teamType_t team;

} clientInfo_t;

typedef struct objective_s
{
	char text[MAX_STRINGCHARS];
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
	char shader[16][MAX_STRINGCHARS];
	int numShaders;

} rain_t;

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

typedef struct debugLine_s
{
	vec3_t start;
	vec3_t end;
	float color[3];
	float alpha;
	float width;
	short unsigned int factor;
	short unsigned int pattern;

} debugLine_t;

typedef struct debugString_s
{
	char text[MAX_QPATH];
	vec3_t pos;
	float scale;
	vec4_t color;

} debugString_t;

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

typedef struct refEntity_s
{
	entityType_t reType;
	int renderFx;
	int model;
	int oldModel;
	vec3_t lightingOrigin;
	int parentEntity;
	vec3_t axis[3];
	int nonNormalizedAxes;
	vec3_t origin;
	frameInfo_t frameInfo[MAX_FRAMEINFOS];
	float actionWeight;
	short int wasframe;
	float scale;
	vec3_t oldOrigin;
	int skinNum;
	int customShader;
	unsigned char shaderRGBA[4];
	float shaderTexCoord[2];
	float shaderTime;
	int entityNumber;
	unsigned char surfaces[MAX_SURFACES];
	float shaderData[2];
	int *boneTag;
	vec4_t *boneQuat;
	tikiFrame_t *of;
	tikiFrame_t *nf;
	dtiki_t *tiki;
	int boneStart;
	int morphStart;
	int hasMorph;
	float radius;
	float rotation;

} refEntity_t;

typedef struct refDef_s
{
	int x;
	int y;
	int width;
	int height;
	float fovX;
	float fovY;
	vec3_t viewOrg;
	vec3_t viewAxis[3];
	int time;
	int rdFlags;
	BYTE areaMask[MAX_MAPAREABYTES];
	float farplaneDistance;
	float farplaneColor[3];
	qboolean farplaneCull;
	qboolean skyPortal;
	float skyAlpha;
	vec3_t skyOrigin;
	vec3_t skyAxis[3];

} refDef_t;

typedef struct snapshot_s
{
	int snapFlags;
	int ping;
	int serverTime;
	BYTE areaMask[MAX_MAPAREABYTES];
	playerState_t ps;
	int numEntities;
	entityState_t entities[MAX_ENTITIESINSNAPSHOT];
	int numServerCommands;
	int serverCommandSequence;
	int numberOfSounds;
	serverSound_t sounds[MAX_SERVERSOUNDS];

} snapshot_t;

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
	vec4_t boneQuat[MAX_BONECONTROLLERS];
	float animLastTimes[MAX_ANIMLAST];
	int animLast[MAX_ANIMLAST];
	int animLastWeight;
	int usageIndexLast;
	int footOnGround_Right;
	int footOnGround_Left;
	int nextLandTime;

} centity_t;

typedef struct cgs_s
{
	gameState_t gameState;
	glConfig_t glConfig;
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
	snapshot_t activeSnapshots[MAX_ACTIVESNAPSHOTS];
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
	objective_t objectives[MAX_OBJECTIVES];
	float objectivesAlphaTime;
	float objectivesBaseAlpha;
	float objectivesDesiredAlpha;
	float objectivesCurrentAlpha;
	rain_t rain;
	clientInfo_t clientinfo[MAX_CLIENTS];

} cg_t;

typedef struct clientSnapshot_s
{
  int areabytes;
  unsigned char areabits[32];
  playerState_t ps;
  int num_entities;
  int first_entity;
  int messageSent;
  int messageAcked;
  int messageSize;

} clientSnapshot_t;

typedef struct client_s
{
	clientState_t state;
	char userinfo[MAX_INFOSTRING];
	int reliableSequence;
	int reliableAcknowledge;
	char reliableCommands[MAX_RELIABLECOMMANDS][MAX_STRINGCHARS];
	int reliableSent;
	int messageAcknowledge;
	int gamestateMessageNum;
	int challenge;
	struct usercmd_s lastUsercmd;
	struct userEyes_s lastEyeinfo;
	int lastMessageNum;
	int lastClientCommand;
	char  lastClientCommandString[MAX_STRINGCHARS];
	struct gentity_s *gentity;
	char name[MAX_NAMELENGTH];
	char downloadName[MAX_QPATH];
	fileHandle_t download;
	int downloadSize;
	int downloadCount;
	int downloadClientBlock;
	int downloadCurrentBlock;
	int downloadXmitBlock;
	unsigned char *downloadBlocks[MAX_DOWNLOADWINDOW];
	int downloadBlockSize[MAX_DOWNLOADWINDOW];
	qboolean downloadEOF;
	int downloadSendTime;
	int deltaMessage;
	int nextReliableTime;
	int lastPacketTime;
	int lastConnectTime;
	int nextSnapshotTime;
	qboolean rateDelayed;
	int timeoutCount;
	clientSnapshot_t frames[PACKET_BACKUP];
	int ping;
	int rate;
	int snapshotMsec;
	netChan_t netchan;
	server_sound_t sounds[64];
	int numberOfSounds;
	qboolean locprint;
	int locprintX;
	int locprintY;
	char stringToPrint[256];

} client_t;

typedef struct {
	byte	ip[4];
	unsigned short	port;
} serverAddress_t;

typedef struct {
	netAdr_t	adr;
	char	  	hostName[ 32 ];
	char	  	mapName[ 32 ];
	char	  	game[ 32 ];
	char		gameTypeString[ 32 ];
	int			netType;
	int			gameType;
	int		  	clients;
	int		  	maxClients;
	int			minPing;
	int			maxPing;
	int			ping;
	qboolean	visible;
} serverInfo_t;

typedef struct clientStatic_s {
	int			startStage;
	connstate_t	state;
	int			loading;
	int			keyCatchers;
	qboolean	vid_restart;
	qboolean	cddialog;
	qboolean	no_menus;

	char		servername[256];

	qboolean	rendererRegistered;
	qboolean	cgameStarted;
	qboolean	uiStarted;
	qboolean	timeScaled;

	int			framecount;
	int			frametime;

	int			realtime;
	int			realFrametime;

	int				numlocalservers;
	serverInfo_t	localServers[128];
	qboolean		bNewLocalServerInfo;

	int			numglobalservers;
	serverInfo_t  globalServers[2048];

	int			numGlobalServerAddresses;
	serverAddress_t		globalServerAddresses[2048];

	int			numfavoriteservers;
	serverInfo_t	favoriteServers[128];

	int			nummplayerservers;
	serverInfo_t	mplayerServers[128];

	int pingUpdateSource;

	int masterNum;

	netAdr_t	updateServer;
	char		updateChallenge[1024];
	char		updateInfoString[1350];

	netAdr_t	authorizeServer;

	glConfig_t	glconfig;
	int total_tris;
	int total_verts;
	int total_texels;
	int world_tris;
	int world_verts;
	int character_lights;
	hdElement_t HudDrawElements[256];
	clientAnim_t anim;
	stopWatch_t stopwatch;
} clientStatic_t;

typedef struct svEntity_s
{
	struct worldSector_s *worldSector;
	struct svEntity_s *nextEntityInWorldSector;
	entityState_t baseline;
	int numClusters;
	int clusternums[16];
	int lastCluster;
	int areanum;
	int areanum2;
	int snapshotCounter;

} svEntity_t;

typedef struct snapshotEntityNumbers_s
{
    /*offset:0x000000 */ int numSnapshotEntities;
    /*offset:0x000004 */ int snapshotEntities[1024];

}    /*sizeof:0x001004 */ snapshotEntityNumbers_t;  

typedef struct worldSector_s
{
	int axis;
	float dist;
	struct worldSector_s *children[2];
	svEntity_t *entities;

} worldSector_t;

typedef struct g_worldSector_s {
	worldSector_t	g[ 32 ];
} g_worldSector_t;

typedef struct areaParms_s
{
	float *mins;
	float *maxs;
	int *list;
	int count;
	int maxcount;

} areaParms_t;

typedef struct pointer_fixup_s
{
  void **ptr;
  int index;
  int type;

} pointer_fixup_t;

typedef struct prof_var_s
{
  int start;
  int clockcycles;
  int calls;
  float time;
  float totTime;
  float avgTime;
  float low;
  float high;

} prof_var_t;

typedef struct prof_game_s
{
  prof_var_t AI;
  prof_var_t MmoveSingle;
  prof_var_t thread_timingList;
  prof_var_t gamescripts;
  prof_var_t GProbe1;
  prof_var_t GProbe2;
  prof_var_t GProbe3;
  prof_var_t FindPath;
  prof_var_t vector_length;
  prof_var_t script_trace;
  prof_var_t internal_trace;
  prof_var_t PreAnimate;
  prof_var_t PostAnimate;

} prof_game_t;

typedef struct prof_cgame_s
{
  prof_var_t CG_AddCEntity;
  prof_var_t CG_DrawActiveFrame;
  prof_var_t CG_AddPacketEntities;
  prof_var_t CG_DrawActive;
  prof_var_t CG_EntityShadow;
  prof_var_t CG_Rain;
  prof_var_t CG_AddBeams;
  prof_var_t CG_ModelAnim;
  prof_var_t CG_ProcessSnapshots;
  prof_var_t CProbe1;
  prof_var_t CProbe2;
  prof_var_t CProbe3;

} prof_cgame_t;

typedef struct prof_exe_s
{
  prof_var_t CL_Frame;
  prof_var_t S_Update;
  prof_var_t RE_RenderScene;
  prof_var_t SCR_UpdateScreen;
  prof_var_t UpdateStereoSide;
  prof_var_t UI_Update;
  prof_var_t UpdateViews;
  prof_var_t Display;
  prof_var_t Display_1;
  prof_var_t Display_2;
  prof_var_t Display_3;
  prof_var_t GetAllBonesFrame;
  prof_var_t rb_surfaceTable;
  prof_var_t RB_DrawTerrainTris;
  prof_var_t RB_BeginSurface;
  prof_var_t RB_EndSurface;
  prof_var_t RB_RenderDrawSurfList;
  prof_var_t RB_SkelMesh;
  prof_var_t RB_StaticMesh;
  prof_var_t RB_DrawBuffer;
  prof_var_t R_AddEntitySurfaces;
  prof_var_t R_AddStaticModelSurfaces;
  prof_var_t R_AddTerrainSurfaces;
  prof_var_t R_AddWorldSurfaces;
  prof_var_t R_MarkLeaves;
  prof_var_t R_RecursiveWorldNode;
  prof_var_t R_RenderView;
  prof_var_t SphereLighting;
  prof_var_t SV_Frame;
  prof_var_t RB_ExecuteRenderCommands;
  prof_var_t Probe1;
  prof_var_t Probe2;
  prof_var_t Probe3;
  prof_var_t R_IssueRenderCommands;
  prof_var_t R_AddSkelSurfaces;
  prof_var_t EndFrame;
  prof_var_t LocBasedDmg;
  prof_var_t Total;
  prof_var_t SV_BuildClientSnapshot;
  prof_var_t SV_AddEntitiesVisibleFromPoint;
  prof_var_t CopySnapshot;
  prof_var_t CL_ParseSnapshot;

} prof_exe_t;

typedef struct prof_s
{
  float time;
  prof_var_t winTime;
  prof_var_t start_game;
  prof_var_t AI;
  prof_var_t MmoveSingle;
  prof_var_t thread_timingList;
  prof_var_t gamescripts;
  prof_var_t GProbe1;
  prof_var_t GProbe2;
  prof_var_t GProbe3;
  prof_var_t FindPath;
  prof_var_t vector_length;
  prof_var_t script_trace;
  prof_var_t internal_trace;
  prof_var_t PreAnimate;
  prof_var_t PostAnimate;
  prof_var_t start_cgame;
  prof_var_t CG_AddCEntity;
  prof_var_t CG_DrawActiveFrame;
  prof_var_t CG_AddPacketEntities;
  prof_var_t CG_DrawActive;
  prof_var_t CG_EntityShadow;
  prof_var_t CG_Rain;
  prof_var_t CG_AddBeams;
  prof_var_t CG_ModelAnim;
  prof_var_t CG_ProcessSnapshots;
  prof_var_t CProbe1;
  prof_var_t CProbe2;
  prof_var_t CProbe3;
  prof_var_t start_exe;
  prof_var_t CL_Frame;
  prof_var_t S_Update;
  prof_var_t RE_RenderScene;
  prof_var_t SCR_UpdateScreen;
  prof_var_t UpdateStereoSide;
  prof_var_t UI_Update;
  prof_var_t UpdateViews;
  prof_var_t Display;
  prof_var_t Display_1;
  prof_var_t Display_2;
  prof_var_t Display_3;
  prof_var_t GetAllBonesFrame;
  prof_var_t rb_surfaceTable;
  prof_var_t RB_DrawTerrainTris;
  prof_var_t RB_BeginSurface;
  prof_var_t RB_EndSurface;
  prof_var_t RB_RenderDrawSurfList;
  prof_var_t RB_SkelMesh;
  prof_var_t RB_StaticMesh;
  prof_var_t RB_DrawBuffer;
  prof_var_t R_AddEntitySurfaces;
  prof_var_t R_AddStaticModelSurfaces;
  prof_var_t R_AddTerrainSurfaces;
  prof_var_t R_AddWorldSurfaces;
  prof_var_t R_MarkLeaves;
  prof_var_t R_RecursiveWorldNode;
  prof_var_t R_RenderView;
  prof_var_t SphereLighting;
  prof_var_t SV_Frame;
  prof_var_t RB_ExecuteRenderCommands;
  prof_var_t Probe1;
  prof_var_t Probe2;
  prof_var_t Probe3;
  prof_var_t R_IssueRenderCommands;
  prof_var_t R_AddSkelSurfaces;
  prof_var_t EndFrame;
  prof_var_t LocBasedDmg;
  prof_var_t Total;
  prof_var_t SV_BuildClientSnapshot;
  prof_var_t SV_AddEntitiesVisibleFromPoint;
  prof_var_t CopySnapshot;
  prof_var_t CL_ParseSnapshot;

} prof_t;

struct SafePtr_s
{
    struct SafePtr_s *prev;
    struct SafePtr_s *next;
};

typedef struct Animate_s
{
	struct			Entity_s baseEntity;
	int				animFlags[16];
	float			syncTime;
	float			syncRate;
	int				pauseSyncTime;
	Event_t			*doneEvents[16];
	float			animtimes[16];
	float			frametimes[16];
	Vector			frame_delta;

} Animate;

typedef struct Container_s
{
	void			*objlist;
	int				numobjects;
	int				maxobjects;
} Container;

typedef struct ActiveWeapon_s {
	Class_t baseClass;
	SafePtr2_t weapon;
	weaponhand_t hand;
} ActiveWeapon_t;

typedef struct Sentient_s
{
	Animate baseAnimate;

	Container_t inventory;
	Container_t ammo_inventory;
	float LMRF;
	SafePtr2_t newWeapon;
	int poweruptype;
	int poweruptimer;
	Vector offset_color;
	Vector offset_delta;
	float offset_time;
	float charge_start_time;
	str blood_model;
	SafePtr2_t activeWeaponList[ 2 ];
	ActiveWeapon_t newActiveWeapon;
	SafePtr2_t holsteredWeapon;
	bool weapons_holstered_by_code;
	ActiveWeapon_t lastActiveWeapon;
	float m_fDamageMultipliers[ 19 ];
	SafePtr2_t m_pVehicle;
	SafePtr2_t m_pTurret;
	SafePtr2_t m_pLadder;
	str m_sHelmetSurface1;
	str m_sHelmetSurface2;
	str m_sHelmetTiki;
	float m_fHelmetSpeed;
	bool m_bDontDropWeapons;
	Vector mTargetPos;
	float mAccuracy;
	SafePtr2_t m_pNextSquadMate;
	SafePtr2_t m_pPrevSquadMate;
	struct Sentient_s *m_NextSentient;
	struct Sentient_s *m_PrevSentient;
	int m_Team;
	int m_iAttackerCount;
	SafePtr2_t m_pLastAttacker;
	SafePtr2_t m_Enemy;
	float m_fPlayerSightLevel;
	char m_bIsDisguised;
	char m_bHasDisguise;
	int m_ShowPapersTime;
	int m_iLastHitTime;
	int m_iThreatBias;
	Vector gunoffset;
	Vector eyeposition;
	int viewheight;
	Vector m_vViewVariation;
	int means_of_death;
	char in_melee_attack;
	char in_block;
	char in_stun;
	char on_fire;
	float on_fire_stop_time;
	float next_catch_on_fire_time;
	int on_fire_tagnums[ 3 ];
	SafePtr2_t fire_owner;
	bool attack_blocked;
	float attack_blocked_time;
	float max_mouth_angle;
	int max_gibs;
	float next_bleed_time;
	char m_bFootOnGround_Right;
	char m_bFootOnGround_Left;

} Sentient_t;

typedef struct pathway_s {
	byte numBlockers;
	short int node;
	short int fallheight;
	float dist;
	float dir[2];
	float pos1[3];
	float pos2[3];
} pathway_t;

typedef struct PathNode_s {
	SimpleEntity_t baseSimple;
	int findCount;
	pathway_t *Child;
	int numChildren;
	int virtualNumChildren;
	float f;
	float h;
	float g;
	struct PathNode_t *Parent;
	bool inopen;
	struct PathNode_t *PrevNode;
	struct PathNode_t *NextNode;
	short int pathway;
	float *m_PathPos;
	float dist;
	float dir[2];
	int nodeflags;
	SafePtr2_t pLastClaimer;
	int iAvailableTime;
	int nodenum;
	short int m_Depth;
} PathNode_t;

typedef struct PathInfo_s {
	void *_vptr;
	bool bAccurate;
	float point[3];
	float dist;
	float dir[2];
} PathInfo_t;

typedef struct MapCell_s {
	Class_t baseClass;
	int numnodes;
	short int *nodes;
} MapCell_t;

typedef struct FallPath_s {
  byte length;
  byte currentPos;
  byte loop;
  float startTime;
  float pos[1][3];
} FallPath_t;

typedef struct cSpline_4_512_s {
	Class_t baseClass;

	int m_iPoints;
	float m_vPoints[512][4];
	int m_iPointFlags[512];
} cSpline_4_512_t;

typedef struct ActorPath_s {
	PathInfo_t *m_path;
	int m_pathlen;
	PathInfo_t *m_pathpos;
	PathInfo_t *m_startpathpos;
	float m_delta[2];
	bool m_Side;
	int m_Time;
	float m_TotalDist;
	short int m_FallHeight;
	short m_HasCompleteLookahead;
	float m_fLookAhead;
	bool m_bChangeLookAhead;
} ActorPath_t;

typedef struct SimpleActor_s {
	Sentient_t baseSentient;
	int m_eAnimMode;
	ScriptThreadLabel_t m_Anim;
	SafePtr2_t m_pAnimThread;
	int m_eNextAnimMode;
	const_str_t m_csNextAnimString;
	ScriptThreadLabel_t m_NextAnimLabel;
	bool m_bNextForceStart;
	float m_fCrouchWeight;
	bool m_YawAchieved;
	float m_DesiredYaw;
	char m_bHasDesiredLookDest;
	char m_bHasDesiredLookAngles;
	short align1;
	Vector m_vDesiredLookDest;
	float m_DesiredLookAngles[3];
	float m_DesiredGunDir[3];
	ActorPath_t m_Path;
	float m_Dest[3];
	float m_NoClipDest[3];
	float path_failed_time;
	float m_fPathGoalTime;
	bool m_bStartPathGoalEndAnim;
	const_str_t m_csPathGoalEndAnimScript; 
	qboolean m_walking;
	qboolean m_groundPlane;
	float m_groundPlaneNormal[3];
	Vector watch_offset;
	bool m_bThink;
	int m_PainTime;
	int m_eEmotionMode;
	float m_fAimLimit_up;
	float m_fAimLimit_down;
	int m_ChangeMotionAnimIndex;
	int m_ChangeActionAnimIndex;
	int m_ChangeSayAnimIndex;

	unsigned int m_weightType[16];
	float m_weightBase[16];
	float m_weightCrossBlend[16];
	char m_AnimMotionHigh;
	char m_AnimActionHigh;
	short m_AnimDialogHigh;

	int hit_obstacle_time;
	float obstacle_vel[2];
	const_str_t m_csAnimName;
	const_str_t m_csSayAnim;
	const_str_t m_csUpperAnim;
	const_str_t m_csCurrentPosition;
	int m_bPathErrorTime;
	PathNode_t *m_NearestNode;
	Vector m_vNearestNodePos;
	int m_bUpdateAnimDoneFlags;
	float m_maxspeed;
	const_str_t m_csMood;
	const_str_t m_csIdleMood;
	int m_iMotionSlot;
	int m_iActionSlot;
	int m_iSaySlot;
	char m_bLevelMotionAnim;
	char m_bLevelActionAnim;
	byte m_bLevelSayAnim;
	byte m_bNextLevelSayAnim;
	char m_bMotionAnimSet;
	char m_bActionAnimSet;
	char m_bSayAnimSet;
	char m_bAimAnimSet;
	int m_iVoiceTime;
	bool m_bDoAI;
	ScriptThreadLabel_t m_PainHandler;
	ScriptThreadLabel_t m_DeathHandler;
	ScriptThreadLabel_t m_AttackHandler;
	ScriptThreadLabel_t m_SniperHandler;
	float m_fCrossblendTime;
} SimpleActor_t;

typedef struct ActorEnemy_s {
	Class_t baseClass;
	int m_iAddTime;
	float m_fLastLookTime;
	float m_fVisibility;
	float m_fTotalVisibility;
	int m_iThreat;
	SafePtr2_t m_pEnemy;
	float m_fCurrentRangeSquared;
	Vector m_vLastKnownPos;
	int m_iLastSightChangeTime;
	bool m_bVisible;
} ActorEnemy_t;

typedef struct ActorEnemySet_s {
	Class_t baseClass;
	Container_t m_Enemies;
	int m_iCheckCount;
	SafePtr2_t m_pCurrentEnemy;
	float m_fCurrentVisibility;
	int m_iCurrentThreat;
} ActorEnemySet_t;

typedef enum
{
	AI_GREN_TOSS_NONE,
	AI_GREN_TOSS_THROW,
	AI_GREN_TOSS_ROLL,
	AI_GREN_TOSS_HINT,
	AI_GREN_KICK
} eGrenadeTossMode;

typedef struct GlobalFuncs_s {
	void ( *ThinkState )( void );
	bool pfn_or_virtual1;
	void ( *BeginState )( void );
	bool pfn_or_virtual2;
	void ( *ResumeState )( void );
	bool pfn_or_virtual3;
	void ( *EndState )( void );
	bool pfn_or_virtual4;
	void ( *SuspendState )( void );
	bool pfn_or_virtual5;
	void ( *un3 )( void );
	bool pfn_or_virtual6;
	void ( *FinishedAnimation )( void );
	bool pfn_or_virtual7;
	void ( *PostShoot )( void );
	bool pfn_or_virtual8;
	void ( *Pain )( Event_t *ev );
	bool pfn_or_virtual9;
	void ( *Killed )( Event_t *ev, bool bPlayDeathAnim );
	bool pfn_or_virtual10;
	bool ( *PassesTransitionConditions )( void );
	bool pfn_or_virtual11;
	void ( *ShowInfo )( void );
	bool pfn_or_virtual12;
	void ( *PathnodeClaimRevoked )( void );
	bool pfn_or_virtual13;
	void ( *ReceiveAIEvent )( vec3_t event_origin, int iType, Entity_t *originator, float fDistSquared, float fMaxDistSquared );
	bool pfn_or_virtual14;
	bool ( *IsState )( int state );
	bool pfn_or_virtual15;
} GlobalFuncs_t;

typedef struct Actor_s {
	SimpleActor_t baseSimpleActor;
	int m_ThinkMap[9];
	int m_ThinkStates[4];
	int m_Think[4];
	int m_ThinkLevel;
	int m_ThinkState;
	int m_State;
	int m_iStateTime;
	char m_bLockThinkState;
	char align1;
	short m_bDirtyThinkState;
	char *m_pszDebugState;
	char m_bAnimating;
	char align2;
	short m_bDog;
	int mVoiceType;
	char m_bSilent;
	char m_bNoSurprise;
	char m_bMumble;
	char m_bBreathSteam;
	const_str_t m_csHeadModel;
	const_str_t m_csHeadSkin;
	const_str_t m_csWeapon;
	const_str_t m_csLoadOut;
	SafePtr2_t m_FavoriteEnemy;
	int m_iEnemyCheckTime;
	int m_iEnemyChangeTime;
	int m_iEnemyVisibleCheckTime;
	int m_iEnemyVisibleChangeTime;
	int m_iLastEnemyVisibleTime;
	int m_iEnemyFovCheckTime;
	int m_iEnemyFovChangeTime;
	Vector m_vLastEnemyPos;
	int m_iLastEnemyPosChangeTime;
	float m_fMaxShareDistSquared;
	int m_iCanShootCheckTime;
	char m_bCanShootEnemy;
	char m_bDesiredEnableEnemy;
	char m_bEnableEnemy;
	char m_bEnablePain;
	char m_bNoLongPain;
	char m_bNewEnemy;
	char m_bEnemyIsDisguised;
	char m_bEnemyVisible;
	char m_bEnemyInFOV;
	char m_bForceAttackPlayer;
	char m_bAutoAvoidPlayer;
	char m_bNoIdleAfterAnim;
	int m_bAnimScriptSet;
	const_str_t m_csAnimScript;
	int m_AnimMode;
	float m_fDfwRequestedYaw;
	float m_fDfwDerivedYaw;
	Vector m_vDfwPos;
	float m_fDfwTime;
	int m_iGunPositionCheckTime;
	Vector m_vGunPosition;
	int m_iWallDodgeTimeout;
	float m_PrevObstacleNormal[2];
	int m_WallDir;
	float m_fMoveDoneRadiusSquared;
	int m_iOriginTime;
	char m_bFaceEnemy;
	char m_bDoPhysics;
	char m_bBecomeRunner;
	char m_bPatrolWaitTrigger;
	int m_bScriptGoalValid;
	Vector m_vScriptGoal;
	int m_iNextWatchStepTime;
	SafePtr2_t m_patrolCurrentNode;
	const_str_t m_csPatrolCurrentAnim;
	int m_iSquadStandTime;
	float m_fInterval;
	int m_iIntervalDirTime;
	Vector m_vIntervalDir;
	int m_sCurrentPathNodeIndex;
	int m_PainState;
	int m_iCuriousTime;
	int m_iCuriousLevel;
	int m_iCuriousAnimHint;
	int m_iNextDisguiseTime;
	int m_iDisguisePeriod;
	float m_fMaxDisguiseDistSquared;
	int m_iEnemyShowPapersTime;
	ScriptThreadLabel_t m_DisguiseAcceptThread;
	int m_iDisguiseLevel;
	SafePtr2_t m_AlarmNode;
	ScriptThreadLabel_t m_AlarmThread;
	int m_iRunHomeTime;
	int m_bTurretNoInitialCover;
	PathNode_t *m_pPotentialCoverNode[16];
	int m_iPotentialCoverCount;
	PathNode_t *m_pCoverNode;
	const_str_t m_csSpecialAttack;
	char m_bInReload;
	char m_bNeedReload;
	char mbBreakSpecialAttack;
	char m_bGrenadeBounced;
	SafePtr2_t m_pGrenade;
	Vector m_vGrenadePos;
	int m_iFirstGrenadeTime;
	int m_eGrenadeState;
	eGrenadeTossMode m_eGrenadeMode;
	Vector m_vGrenadeVel;
	Vector m_vKickDir;
	FallPath_t *m_pFallPath;
	float m_fBalconyHeight;
	int m_bNoPlayerCollision;
	float m_fNoticeTimeScale;
	float m_fMaxNoticeTimeScale;
	ActorEnemySet_t m_PotentialEnemies;
	float m_fSight;
	float m_fHearing;
	float m_fSoundAwareness;
	float m_fGrenadeAwareness;
	int m_iIgnoreSoundsMask;
	float m_fFov;
	float m_fFovDot;
	int m_iEyeUpdateTime;
	Vector m_vEyeDir;
	int m_iNextLookTime;
	float m_fLookAroundFov;
	SafePtr2_t m_pLookEntity;
	int m_iLookFlags;
	SafePtr2_t m_pPointEntity;
	SafePtr2_t m_pTurnEntity;
	float m_fTurnDoneError;
	float m_fAngleYawSpeed;
	SafePtr2_t m_aimNode;
	int m_eDontFaceWallMode;
	int m_iLastFaceDecideTime;
	float m_vOriginHistory[4][2];
	int m_iCurrentHistory;
	char m_bHeadAnglesAchieved;
	char m_bLUpperArmAnglesAchieved;
	char m_bTorsoAnglesAchieved;
	char align3;
	float m_fHeadMaxTurnSpeed;
	float m_vHeadDesiredAngles[3];
	float m_fLUpperArmTurnSpeed;
	float m_vLUpperArmDesiredAngles[3];
	float m_fTorsoMaxTurnSpeed;
	float m_fTorsoCurrentTurnSpeed;
	float m_vTorsoDesiredAngles[3];
	Vector m_vHome;
	SafePtr2_t m_pTetherEnt;
	float m_fMinDistance;
	float m_fMinDistanceSquared;
	float m_fMaxDistance;
	float m_fMaxDistanceSquared;
	float m_fLeash; 
	float m_fLeashSquared;
	bool m_bFixedLeash;
} Actor_t;

typedef struct State_s
{
    DWORD dummy[2];
    DWORD condition_indexes[3];
    DWORD statemap[1];
    str name;
    str nextState;
    movecontrol_t movetype;
    cameratype_t cameratype;
    float minTime;
    float maxTime;
    DWORD legAnims[3];
    DWORD m_ActionAnims[3];
    int m_iActionAnimType;
    DWORD states[3];
    DWORD entryCommands[3];
    DWORD exitCommands[3];

} State;

typedef struct Event_GAMEDLL_s
{
	DWORD dummy[2];
	qboolean fromScript;
	short unsigned int eventnum;
	short unsigned int dataSize;
    DWORD *dummy2;

} Event_GAMEDLL;


typedef enum serverState_e
{
	SS_DEAD,
	SS_LOADING,
	SS_LOADING2,
	SS_GAME

} serverState_t;

typedef struct challenge_s
{
	netAdr_t adr;
	int challenge;
	int time;
	int pingTime;
	int firstTime;
	qboolean connected;

} challenge_t;

typedef struct sgSfx_s
{
	int flags;
	char name[64];

} sgSfx_t;

typedef struct sgChannelbase_s
{
	qboolean isPlaying;
	int status;
	sgSfx_t sfx;
	int entNum;
	int entChannel;
	float origin[3];
	float volume;
	int baseRate;
	float newPitchMult;
	float minDist;
	float maxDist;
	int startTime;
	int time;
	int nextCheckObstructionTime;
	int endTime;
	int flags;
	int offset;
	int loopCount;

} sgChannelbase_t;

typedef struct sgSoundSystem_s
{
	sgChannelbase_t channels[96];

} sgSoundSystem_t;

typedef struct {
	int		iFlags;
	char	szName[ MAX_QPATH ];
} sfxsavegame_t;

typedef struct {
	qboolean		bPlaying;
	int				iStatus;

	sfxsavegame_t	sfx;

	int				iEntNum;
	int				iEntChannel;

	vec3_t			vOrigin;
	float			fVolume;
	int				iBaseRate;
	float			fNewPitchMult;
	float			fMinDist;
	float			fMaxDist;

	int				iStartTime;
	int				iTime;
	int				iNextCheckObstructionTime;
	int				iEndTime;

	int				iFlags;
	int				iOffset;
	int				iLoopCount;
} channelbasesavegame_t;

typedef struct{
	channelbasesavegame_t Channels[ 96 ];
} soundsystemsavegame_t;

typedef struct serverStatic_s
{
	qboolean initialized;
	int snapFlagServerBit;
	int time;
	int startTime;
	int lastTime;
	int serverLagTime;
	qboolean autosave;
	int mapTime;
	client_t *clients;
	int iNumClients;
	int numSnapshotEntities;
	int nextSnapshotEntities;
	entityState_t *snapshotEntities;
	int nextHeartbeatTime;
	challenge_t challenges[1024];
	netAdr_t redirectAddress;
	netAdr_t authorizeAddress;
	char gameName[64];
	char mapName[64];
	char rawServerName[64];
	int areaBitsWarningTime;
	qboolean soundsNeedLoad;
	char tmFileName[64];
	int tmMoopcount;
	int tmOffset;
	sgSoundSystem_t soundSystem;

} serverStatic_t;


typedef struct server_s
{
	serverState_t state;
	qboolean restarting;
	int serverId;
	int restartedServerId;
	int checksumFeed;
	int snapshotCounter;
	int timeResidual;
	int nextFrameTime;
	cStaticModel_t *models[1024];
	char *configstrings[2736];
	svEntity_t svEntities[1024];
	int farplane;
	qboolean skyportal;
	char *entityParsePoint;
	gentity_t *gentities;
	int gentitySize;
	int num_entities;
	playerState_t *gameClients;
	int gameClientSize;

} server_t;

typedef struct {
	cplane_t	*plane;
	int			children[2];		// negative numbers are leafs
} cNode_t;

typedef struct {
	int			cluster;
	int			area;

	int			firstLeafBrush;
	int			numLeafBrushes;

	int			firstLeafSurface;
	int			numLeafSurfaces;
	int			firstLeafTerrain;
	int			numLeafTerrains;
} cLeaf_t;

typedef struct cmodel_s {
	vec3_t		mins, maxs;
	cLeaf_t		leaf;
} cmodel_t;

typedef struct {
	cplane_t			*plane;
	int					surfaceFlags;
	int					shaderNum;

	dsideequation_t		*pEq;
} cbrushside_t;

typedef struct {
	int			shaderNum;
	int			contents;
	vec3_t		bounds[2];
	int			numsides;
	cbrushside_t	*sides;
	int			checkcount;
} cbrush_t;

typedef struct {
	int			checkcount;
	int			surfaceFlags;
	int			contents;

	int			shaderNum;
	int			subdivisions;

	struct patchCollide_s	*pc;
} cPatch_t;

typedef struct {
	float	plane[4];
	int		signbits;
} patchPlane_t;

typedef struct {
	int			surfacePlane;
	int			numBorders;
	int			borderPlanes[4+6+16];
	int			borderInward[4+6+16];
	qboolean	borderNoAdjust[4+6+16];
} facet_t;

typedef struct patchCollide_s {
	vec3_t	bounds[2];
	int		numPlanes;
	patchPlane_t	*planes;
	int		numFacets;
	facet_t	*facets;
} patchCollide_t;

typedef struct {
	int			floodnum;
	int			floodvalid;
} cArea_t;

typedef struct {
	int			checkcount;
	struct terPatchCollide_s	*tc;
} cterPatch_t;

typedef struct cfencemask_s {
	char name[ 64 ];
	int iWidth;
	int iHeight;
	byte *pData;
	struct cfencemask_s *pNext;
} cfencemask_t;

typedef struct {
	char shader[64];
	int surfaceFlags;
	int contentFlags;
	cfencemask_t *mask;
} cshader_t;

typedef struct terrainCollideSquare_s {
	float plane[2][4];
	int eMode;
} terrainCollideSquare_t;

struct terrainCollide_s {
	vec3_t vBounds[2];
	terrainCollideSquare_t squares[8][8];
};

typedef struct {
	int checkcount;
	int surfaceFlags;
	int contents;
	int shaderNum;
	struct terrainCollide_s tc;
} cTerrain_t;

typedef struct
{
	qboolean	use;
	float		radius;
	vec3_t		offset;
} sphere_t;

typedef struct {
	vec3_t		start;
	vec3_t		end;
	vec3_t		size[2];
	vec3_t		offsets[8];
	float		maxOffset;
	vec3_t		extents;
	vec3_t		bounds[2];
	float		height;
	float		radius;
	int			contents;
	qboolean	isPoint;
	trace_t		trace;
} traceWork_t;

typedef struct leafList_s {
	int		count;
	int		maxcount;
	qboolean	overflowed;
	int		*list;
	vec3_t	bounds[2];
	int		lastLeaf;
	void	(*storeLeafs)( struct leafList_s *ll, int nodenum );
} leafList_t;

typedef struct terrainCollideSquare_s {
	float plane[ 2 ][ 4 ];
	int eMode;
} terrainCollideSquare_t;

typedef struct terrainCollide_s {
	vec3_t vBounds[ 2 ];
	terrainCollideSquare_t squares[ 8 ][ 8 ];
} terrainCollide_t;

typedef struct varnodeIndex_s {
	unsigned short iTreeAndMask;
	unsigned short iNode;
} varnodeIndex_t;

static int modeTable[ 8 ];

typedef struct worknode_s {
	int i0;
	int j0;
	int i1;
	int j1;
	int i2;
	int j2;
} worknode_t;

typedef struct pointtrace_s {
	traceWork_t *tw;
	terrainCollide_t *tc;
	vec3_t vStart;
	vec3_t vEnd;
	int i;
	int j;
	float fSurfaceClipEpsilon;
} pointtrace_t;

typedef struct {
  char name[64];
  int numShaders;
  cfencemask_t *fencemasks;
  cshader_t *shaders;
  int numSideEquations;
  dsideequation_t *sideequations;
  int numBrushSides;
  cbrushside_t *brushsides;
  int numPlanes;
  cplane_t *planes;
  int numNodes;
  cNode_t *nodes;
  int numLeafs;
  cLeaf_t *leafs;
  int numLeafBrushes;
  int *leafbrushes;
  int numLeafSurfaces;
  int *leafsurfaces;
  int numLeafTerrains;
  cTerrain_t **leafterrains;
  int numSubModels;
  cmodel_t *cmodels;
  int numBrushes;
  cbrush_t *brushes;
  int numClusters;
  int clusterBytes;
  byte *visibility;
  qboolean vised;
  int numEntityChars;
  char *entityString;
  int numAreas;
  cArea_t *areas;
  int *areaPortals;
  int numSurfaces;
  cPatch_t **surfaces;
  int numTerrain;
  cTerrain_t *terrain;
  int floodvalid;
  int checkcount;
} clipMap_t;

typedef struct memblock_s {
	int size;
	struct memblock_s *prev;
	struct memblock_s *next;
	int id;
} memblock_t;

typedef struct memconstant_s {
	memblock_t b;
	unsigned char mem[2];
} memconstant_t;

typedef struct cmdalias_s {
	struct cmdalias_s *next;
	char name[32];
	char *value;
} cmdalias_t;

typedef struct {
	str		m_refName;
	str		m_locName;
	int		m_r1_rep;
	int		m_r2_rep;
	int		m_r3_rep;
	int		m_l1_rep;
	int		m_l2_rep;
	int		m_l3_rep;
} loc_entry_t;

typedef struct cLocalization_s {
	Container_t m_entries;
	Container_t missing;
} cLocalization;

typedef enum { SS_BAD, SS_PORTAL, SS_PORTALSKY, SS_ENVIRONMENT, SS_OPAQUE, SS_DECAL, SS_SEE_THROUGH, SS_BANNER, SS_UNDERWATER, SS_BLEND0, SS_BLEND1, SS_BLEND2, SS_BLEND3, SS_BLEND6, SS_STENCIL_SHADOW, SS_ALMOST_NEAREST, SS_NEAREST } shaderSort_t;
typedef enum { GF_NONE, GF_SIN, GF_SQUARE, GF_TRIANGLE, GF_SAWTOOTH, GF_INVERSE_SAWTOOTH, GF_NOISEANIM, GF_NOISESTATIC, GF_NOISE } genFunc_t;
typedef enum { DEFORM_NONE, DEFORM_WAVE, DEFORM_NORMALS, DEFORM_BULGE, DEFORM_MOVE, DEFORM_AUTOSPRITE, DEFORM_AUTOSPRITE2, DEFORM_LIGHTGLOW, DEFORM_FLAP_S, DEFORM_FLAP_T } deform_t;
typedef enum { USE_S_COORDS, USE_T_COORDS } texDirection_t;
typedef enum { AGEN_IDENTITY, AGEN_SKIP, AGEN_ENTITY, AGEN_ONE_MINUS_ENTITY, AGEN_VERTEX, AGEN_ONE_MINUS_VERTEX, AGEN_LIGHTING_SPECULAR, AGEN_WAVEFORM, AGEN_PORTAL, AGEN_NOISE, AGEN_DOT, AGEN_ONE_MINUS_DOT, AGEN_CONSTANT, AGEN_GLOBAL_ALPHA, AGEN_SKYALPHA, AGEN_ONE_MINUS_SKYALPHA, AGEN_SCOORD, AGEN_TCOORD, AGEN_DIST_FADE, AGEN_ONE_MINUS_DIST_FADE, AGEN_DOT_VIEW, AGEN_ONE_MINUS_DOT_VIEW } alphaGen_t;
typedef enum { CGEN_BAD, CGEN_IDENTITY, CGEN_IDENTITY_LIGHTING, CGEN_ENTITY, CGEN_ONE_MINUS_ENTITY, CGEN_EXACT_VERTEX, CGEN_VERTEX, CGEN_ONE_MINUS_VERTEX, CGEN_WAVEFORM, CGEN_MULTIPLY_BY_WAVEFORM, CGEN_LIGHTING_GRID, CGEN_LIGHTING_SPHERICAL, CGEN_CONSTANT, CGEN_NOISE, CGEN_GLOBAL_COLOR, CGEN_STATIC, CGEN_SCOORD, CGEN_TCOORD, CGEN_DOT, CGEN_ONE_MINUS_DOT } colorGen_t;
typedef enum { TCGEN_BAD, TCGEN_IDENTITY, TCGEN_LIGHTMAP, TCGEN_TEXTURE, TCGEN_ENVIRONMENT_MAPPED, TCGEN_VECTOR, TCGEN_ENVIRONMENT_MAPPED2, TCGEN_SUN_REFLECTION } texCoordGen_t;

typedef struct {
  genFunc_t func;
  float base;
  float amplitude;
  float phase;
  float frequency;
} waveForm_t;

typedef enum { SS_BAD, SS_PORTAL, SS_PORTALSKY, SS_ENVIRONMENT, SS_OPAQUE, SS_DECAL, SS_SEE_THROUGH, SS_BANNER, SS_UNDERWATER, SS_BLEND0, SS_BLEND1, SS_BLEND2, SS_BLEND3, SS_BLEND6, SS_STENCIL_SHADOW, SS_ALMOST_NEAREST, SS_NEAREST } shaderSort_t;
typedef enum { GF_NONE, GF_SIN, GF_SQUARE, GF_TRIANGLE, GF_SAWTOOTH, GF_INVERSE_SAWTOOTH, GF_NOISEANIM, GF_NOISESTATIC, GF_NOISE } genFunc_t;
typedef enum { DEFORM_NONE, DEFORM_WAVE, DEFORM_NORMALS, DEFORM_BULGE, DEFORM_MOVE, DEFORM_AUTOSPRITE, DEFORM_AUTOSPRITE2, DEFORM_LIGHTGLOW, DEFORM_FLAP_S, DEFORM_FLAP_T } deform_t;
typedef enum { USE_S_COORDS, USE_T_COORDS } texDirection_t;
typedef enum { AGEN_IDENTITY, AGEN_SKIP, AGEN_ENTITY, AGEN_ONE_MINUS_ENTITY, AGEN_VERTEX, AGEN_ONE_MINUS_VERTEX, AGEN_LIGHTING_SPECULAR, AGEN_WAVEFORM, AGEN_PORTAL, AGEN_NOISE, AGEN_DOT, AGEN_ONE_MINUS_DOT, AGEN_CONSTANT, AGEN_GLOBAL_ALPHA, AGEN_SKYALPHA, AGEN_ONE_MINUS_SKYALPHA, AGEN_SCOORD, AGEN_TCOORD, AGEN_DIST_FADE, AGEN_ONE_MINUS_DIST_FADE, AGEN_DOT_VIEW, AGEN_ONE_MINUS_DOT_VIEW } alphaGen_t;
typedef enum { CGEN_BAD, CGEN_IDENTITY, CGEN_IDENTITY_LIGHTING, CGEN_ENTITY, CGEN_ONE_MINUS_ENTITY, CGEN_EXACT_VERTEX, CGEN_VERTEX, CGEN_ONE_MINUS_VERTEX, CGEN_WAVEFORM, CGEN_MULTIPLY_BY_WAVEFORM, CGEN_LIGHTING_GRID, CGEN_LIGHTING_SPHERICAL, CGEN_CONSTANT, CGEN_NOISE, CGEN_GLOBAL_COLOR, CGEN_STATIC, CGEN_SCOORD, CGEN_TCOORD, CGEN_DOT, CGEN_ONE_MINUS_DOT } colorGen_t;
typedef enum { TCGEN_BAD, TCGEN_IDENTITY, TCGEN_LIGHTMAP, TCGEN_TEXTURE, TCGEN_ENVIRONMENT_MAPPED, TCGEN_VECTOR, TCGEN_ENVIRONMENT_MAPPED2, TCGEN_SUN_REFLECTION } texCoordGen_t;
typedef enum { TMOD_NONE, TMOD_TRANSFORM, TMOD_TURBULENT, TMOD_SCROLL, TMOD_SCALE, TMOD_STRETCH, TMOD_ROTATE, TMOD_ENTITY_TRANSLATE, TMOD_NOISE, TMOD_OFFSET, TMOD_PARALLAX, TMOD_MACRO, TMOD_WAVETRANS, TMOD_WAVETRANT, TMOD_BULGETRANS } texMod_t;

typedef struct {
  deform_t deformation;
  float moveVector[3];
  waveForm_t deformationWave;
  float deformationSpread;
  float bulgeWidth;
  float bulgeHeight;
  float bulgeSpeed;
} deformStage_t;

typedef struct image_s {
  char imgName[64];
  int width;
  int height;
  int uploadWidth;
  int uploadHeight;
  unsigned int texnum;
  int frameUsed;
  int bytesUsed;
  int internalFormat;
  int TMU;
  int numMipmaps;
  qboolean dynamicallyUpdated;
  qboolean allowPicmip;
  qboolean force32bit;
  unsigned int wrapClampModeX;
  unsigned int wrapClampModeY;
  int r_sequence;
  int UseCount;
  struct image_s *next;
} image_t;

typedef struct {
  texMod_t type;
  waveForm_t wave;
  float matrix[2][2];
  float translate[2];
  float scale[2];
  float rate[2];
  float scroll[2];
  float rotateSpeed;
  float rotateStart;
  float rotateCoef;
} texModInfo_t;

typedef struct {
  image_t *image[64];
  int numImageAnimations;
  float imageAnimationSpeed;
  float imageAnimationPhase;
  texCoordGen_t tcGen;
  float tcGenVectors[2][3];
  int numTexMods;
  texModInfo_t *texMods;
  int isLightmap;
  int vertexLightmap;
  unsigned char flags;
} textureBundle_t;

typedef struct {
  int active;
  textureBundle_t bundle[2];
  int multitextureEnv;
  waveForm_t rgbWave;
  colorGen_t rgbGen;
  waveForm_t alphaWave;
  alphaGen_t alphaGen;
  unsigned int stateBits;
  int noMipMaps;
  int noPicMip;
  int force32bit;
  float alphaMin;
  float alphaMax;
  float specOrigin[3];
  unsigned char colorConst[4];
  unsigned char alphaConst;
  unsigned char alphaConstMin;
} shaderStage_t;

typedef enum { CT_FRONT_SIDED, CT_BACK_SIDED, CT_TWO_SIDED } cullType_t;

typedef struct {
  float cloudHeight;
  image_t *outerbox[6];
  image_t *innerbox[6];
} skyParms_t;

typedef enum { SPRITE_PARALLEL, SPRITE_PARALLEL_ORIENTED, SPRITE_ORIENTED, SPRITE_PARALLEL_UPRIGHT } spriteType_t;

typedef struct {
  spriteType_t type;
  float scale;
} spriteParms_t;

typedef struct shader_t {
  char name[64];
  int lightmapIndex;
  int index;
  int sortedIndex;
  float sort;
  int defaultShader;
  int explicitlyDefined;
  int surfaceFlags;
  int contentFlags;
  int entityMergable;
  int isSky;
  skyParms_t sky;
  spriteParms_t sprite;
  int isPortalSky;
  float subdivisions;
  float fDistRange;
  float fDistNear;
  float light;
  cullType_t cullType;
  int polygonOffset;
  int needsNormal;
  int needsST1;
  int needsST2;
  int needsColor;
  int numDeforms;
  deformStage_t deforms[3];
  int numUnfoggedPasses;
  shaderStage_t *unfoggedStages[8];
  int needsLGrid;
  int needsLSpherical;
  int stagesWithAlphaFog;
  int flags;
  void *optimalStageIteratorFunc;
  float timeOffset;
  struct shader_s *next;
} shader_t;

typedef enum { LIGHT_POINT, LIGHT_DIRECTIONAL, LIGHT_SPOT, LIGHT_SPOT_FAST } lighttype_t;
typedef struct reallightinfo_s {
  vec3_t color;
  lighttype_t eType;
  float fIntensity;
  float fDist;
  float fSpotSlope;
  float fSpotConst;
  float fSpotScale;
  vec3_t vOrigin;
  vec3_t vDirection;
} reallightinfo_t;

typedef struct light_s {
	char char0;
	_DWORD dword4;
	vec3_t m_vOrigin;
	vec3_t m_vSpotDir;
	_BYTE gap20[4];
	bool m_bLinear;
	float m_fFalloff;
	_BYTE gap2C[4];
	float m_fRealIntensity;
	vec3_t m_vColor;
	float m_fSpotRadiusByDistance;
	_BYTE gap44[4];
	_DWORD dword48;
	void *m_pShader;
	float m_fScale;
	float m_fOverbright;
	float m_fDist;
} light_t;

typedef float cube_entry_t[3][4];

typedef struct {
  vec3_t origin;
  vec3_t worldOrigin;
  vec3_t traceOrigin;
  float radius;
  struct mnode_s *leaves[8];
  void *TessFunction;
  union ambient {
    unsigned char level[4];
    int value;
  };
  int numRealLights;
  reallightinfo_t light[32];
  qboolean bUsesCubeMap;
  cube_entry_t cubemap[24];
} sphereor_t;

typedef struct {
  vec3_t origin;
  vec3_t color;
  float intensity;
  struct mnode_s *leaf;
  qboolean needs_trace;
  qboolean spot_light;
  vec3_t spot_dir;
  float spot_radiusbydistance;
  int reference_count;
} spherel_t;

typedef struct {
  struct mnode_s *cntNode;
  int inUse;
  struct msurface_s *skySurfs[32];
  int numSurfs;
  float offset[3];
  float mins[3];
  float maxs[3];
} portalsky_t;

typedef struct {
  float transformed[3];
  int index;
} sphere_dlight_t;

typedef struct msurface_s {
  int viewCount;
  int frameCount;
  shader_t *shader;
  surfaceType_t *data;
} msurface_t;

typedef struct mnode_s {
  int contents;
  int visframe;
  float mins[3];
  float maxs[3];
  struct mnode_s *parent;
  cplane_t *plane;
  struct mnode_s *children[2];
  int cluster;
  int area;
  spherel_t **lights;
  int numlights;
  msurface_t **firstmarksurface;
  int nummarksurfaces;
  int firstTerraPatch;
  int numTerraPatches;
  int firstStaticModel;
  int numStaticModels;
  void **pFirstMarkFragment;
  int iNumMarkFragment;
} mnode_t;

typedef struct {
  char name[64];
  char baseName[64];
  int dataSize;
  int numShaders;
  dshader_t *shaders;
  int numBmodels;
  bmodel_t *bmodels;
  int numplanes;
  cplane_t *planes;
  int numnodes;
  int numDecisionNodes;
  mnode_t *nodes;
  int numsurfaces;
  msurface_t *surfaces;
  int nummarksurfaces;
  msurface_t **marksurfaces;
  vec3_t lightGridMins;
  vec3_t lightGridSize;
  vec3_t lightGridOOSize;
  int lightGridBounds[3];
  short unsigned int *lightGridOffsets;
  byte *lightGridData;
  byte lightGridPalette[768];
  int numTerraPatches;
  struct cTerraPatchUnpacked_s *terraPatches;
  struct cTerraPatchUnpacked_s *activeTerraPatches;
  int numVisTerraPatches;
  struct cTerraPatchUnpacked_s **visTerraPatches;
  int numStaticModelData;
  unsigned char *staticModelData;
  int numStaticModels;
  struct cStaticModelUnpacked_s *staticModels;
  int numVisStaticModels;
  struct cStaticModelUnpacked_s **visStaticModels;
  int numClusters;
  int clusterBytes;
  byte *vis;
  byte *novis;
  byte *lighting;
} world_t;

typedef struct dlight_s {
  float origin[3];
  float color[3];
  float radius;
  dlighttype_t type;
  float transformed[3];
} dlight_t;

typedef struct trRefEntity_s {
  refEntity_t e;
  float axisLength;
  qboolean needDlights;
  qboolean bLightGridCalculated;
  int iGridLighting;
  float lodpercentage[2];
  qboolean sphereCalculated;
  int lightingSphere;
} trRefEntity_t;

typedef struct depthfog_s {
  float len;
  float oolen;
  int enabled;
  int extrafrustums;
} depthfog_t;

typedef struct viewParms_s {
  orientationr_t or;
  orientationr_t world;
  float pvsOrigin[3];
  qboolean isPortal;
  qboolean isMirror;
  int frameSceneNum;
  int frameCount;
  cplane_t portalPlane;
  int viewportX;
  int viewportY;
  int viewportWidth;
  int viewportHeight;
  float fovX;
  float fovY;
  float projectionMatrix[16];
  cplane_s frustum[5];
  float visBounds[2][3];
  float zFar;
  depthfog_t fog;
  float farplane_distance;
  float farplane_color[3];
  qboolean farplane_cull;
} viewParms_t;

typedef struct refSprite_s {
  surfaceType_t surftype;
  qhandle_t hModel;
  int shaderNum;
  float origin[3];
  float scale;
  float axis[3][3];
  unsigned char shaderRGBA[4];
  int renderfx;
  float shaderTime;
} refSprite_t;

typedef struct {
  int x;
  int y;
  int width;
  int height;
  float fov_x;
  float fov_y;
  float vieworg[3];
  float viewaxis[3][3];
  int time;
  int rdflags;
  unsigned char areamask[32];
  int areamaskModified;
  float floatTime;
  int num_entities;
  trRefEntity_t *entities;
  int num_sprites;
  refSprite_t *sprites;
  int num_dlights;
  dlight_s *dlights;
  int numTerMarks;
  struct srfMarkFragment_s *terMarks;
  int numPolys;
  struct srfPoly_s *polys;
  int numDrawSurfs;
  struct drawSurf_s *drawSurfs;
  int numSpriteSurfs;
  struct drawSurf_s *spriteSurfs;
  int numStaticModels;
  struct cStaticModelUnpacked_s *staticModels;
  int numStaticModelData;
  unsigned char *staticModelData;
  int sky_portal;
  float sky_alpha;
  float sky_origin[3];
  float sky_axis[3][3];
} trRefdef_t;

typedef struct {
  int c_sphere_cull_patch_in;
  int c_sphere_cull_patch_clip;
  int c_sphere_cull_patch_out;
  int c_box_cull_patch_in;
  int c_box_cull_patch_clip;
  int c_box_cull_patch_out;
  int c_sphere_cull_md3_in;
  int c_sphere_cull_md3_clip;
  int c_sphere_cull_md3_out;
  int c_box_cull_md3_in;
  int c_box_cull_md3_clip;
  int c_box_cull_md3_out;
  int c_leafs;
  int c_dlightSurfaces;
  int c_dlightSurfacesCulled;
  int c_dlightMaps;
  int c_dlightTexels;
} frontEndCounters_t;

typedef struct {
  int c_surfaces;
  int c_shaders;
  int c_vertexes;
  int c_indexes;
  int c_totalIndexes;
  int c_characterlights;
  float c_overDraw;
  int c_dlightVertexes;
  int c_dlightIndexes;
  int c_flareAdds;
  int c_flareTests;
  int c_flareRenders;
} backEndCounters_t;

/*typedef struct {
  drawSurf_s drawSurfs[65536];
  drawSurf_s spriteSurfs[32768];
  dlight_s dlights[32];
  trRefEntity_s entities[1023];
  srfMarkFragment_t *terMarks;
  srfPoly_t *polys;
  polyVert_t *polyVerts;
  refSprite_s sprites[2048];
  cStaticModelUnpacked_t *staticModels;
  unsigned char *staticModelData
  renderCommandList_t commands;
} backEndData_t;*/

typedef struct {
  int smpFrame;
  trRefdef_t refdef;
  viewParms_t viewParms;
  orientationr_t or; 
  backEndCounters_t pc;
  int isHyperspace;
  trRefEntity_t *currentEntity;
  int skyRenderedThisView;
  sphereor_t spheres[128];
  unsigned char numSpheresUsed;
  unsigned char align[3];
  sphereor_t *currentSphere;
  sphereor_t spareSphere;
  sphereor_t hudSphere;
  cStaticModelUnpacked_t *currentStaticModel;
  int dsStreamVert;
  int in2D;
  unsigned char color2D[4];
  int vertexes2D;
  trRefEntity_t entity2D;
  int backEndMsec;
  float shaderStartTime;
} backEndState_t;

typedef struct {
  qboolean registered;
  int visCount;
  int frameCount;
  int sceneCount;
  int viewCount;
  int smpFrame;
  int frameSceneNum;
  qboolean worldMapLoaded;
  world_t *world;
  byte *externalVisData;
  image_t *defaultImage;
  image_t *scratchImage;
  image_t *dlightImage;
  image_t *flareImage;
  image_t *whiteImage;
  image_t *identityLightImage;
  image_t *dlightImages[15];
  shader_t *defaultShader;
  shader_t *shadowShader;
  shader_t *projectionShadowShader;
  shader_t *flareShader;
  shader_t *sunShader;
  int numLightmaps;
  image_t *lightmaps[256];
  trRefEntity_t *currentEntity;
  trRefEntity_t worldEntity;
  int currentEntityNum;
  int currentSpriteNum;
  int shiftedEntityNum;
  int shiftedIsStatic;
  model_t *currentModel;
  viewParms_t viewParms;
  float identityLight;
  int identityLightByte;
  int overbrightBits;
  int overbrightShift;
  float overbrightMult;
  qboolean needsLightScale;
  orientationr_t or;
  portalsky_t portalsky;
  qboolean skyRendered;
  qboolean portalRendered;
  trRefdef_t refdef;
  int viewCluster;
  float sunLight[3];
  float sunDirection[3];
  frontEndCounters_t pc;
  int frontEndMsec;
  model_s models[700];
  int numModels;
  int numImages;
  image_s images[1536];
  int numShaders;
  shader_t *shaders[2048];
  shader_t *sortedShaders[2048];
  float sinTable[1024];
  float squareTable[1024];
  float triangleTable[1024];
  float sawToothTable[1024];
  float inverseSawToothTable[1024];
  spherel_t sSunLight;
  spherel_t sLights[1532];
  int numSLights;
  int rendererhandle;
  qboolean shadersParsed;
  int frame_skel_index;
  int skel_index[1024];
  fontheader_t *pFontDebugStrings;
} trGlobals_t;

typedef unsigned char color4ub_t[4];
typedef struct stageVars {
  unsigned char colors[1000][4];
  float texcoords[2][1000][2];
} stageVars_t;

typedef struct shaderCommands_s {
  unsigned int indexes[6000];
  float xyz[1000][4];
  float normal[1000][4];
  float texCoords[1000][2][2];
  unsigned char vertexColors[1000][4];
  int vertexDlightBits[1000];
  stageVars_t svars;
  unsigned char constantColor255[1000][4];
  float alphaFogDensity[1000];
  shader_t *shader;
  float shaderTime;
  int dlightBits;
  int dlightMap;
  int numIndexes;
  int numVertexes;
  int numPasses;
  void (*currentStageIteratorFunc) ();
  shaderStage_t **xstages;
  qboolean no_global_fog;
  qboolean vertexColorValid;
} shaderCommands_t;

typedef struct refImport_s
{
	void ( *Printf )( char *format, ... );
	void ( *Error )( errorParm_t errorCode, char *format, ... );
	int ( *Milliseconds )( );
	char * ( *LV_ConvertString )( char *string );
	void ( *Hunk_Clear )( );
	void * ( *Hunk_Alloc )( int size );
	void * ( *Hunk_AllocateTempMemory )( int size );
	void ( *Hunk_FreeTempMemory )( void *buffer );
	void * ( *Malloc )( int size );
	void ( *Free )( void *ptr );
	void ( *Clear )( );
	cvar_t * ( *Cvar_Get )( char *varName, char *varValue, int varFlags );
	void ( *Cvar_Set )( char *varName, char *varValue );
	void ( *Cvar_SetDefault )( cvar_t *var, char *varValue );
	void ( *Cmd_AddCommand )( char *cmdName, xcommand_t cmdFunction );
	void ( *Cmd_RemoveCommand )( char *cmdName );
	int ( *Argc )( );
	char * ( *Argv )( int arg );
	void ( *Cmd_ExecuteText )( cbufExec_t execWhen, char *text );
	void ( *CM_DrawDebugSurface )( void ( *drawPoly )( ) );
	int ( *FS_OpenFile )( char *qpath, fsMode_t mode );
	int ( *FS_Read )( void *buffer, int len, fileHandle_t fileHandle );
	void ( *FS_CloseFile )( fileHandle_t fileHandle );
	int ( *FS_Seek )( fileHandle_t fileHandle, long int offset, fsOrigin_t origin );
	int ( *FS_FileIsInPAK )( char *fileName, int *checksum );
	int ( *FS_ReadFile )( char *qpath, void **buffer );
	int ( *FS_ReadFileEx )( char *qpath, void **buffer, qboolean quiet );
	void ( *FS_FreeFile )( void *buffer );
	char ** ( *FS_ListFiles )( char *qpath, char *extension, qboolean wantSubs, int *numFiles );
	void ( *FS_FreeFileList )( char **list );
	int ( *FS_WriteFile )( char *qpath, void *buffer, int size );
	qboolean ( *FS_FileExists )( char *file );
	void ( *CM_BoxTrace )( trace_t *results, vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs, int model, int brushMask, int cylinder );
	int ( *CM_TerrainSquareType )( int terrainPatch, int i, int j );
	char * ( *CM_EntityString )( );
	char * ( *CM_MapTime )( );
	int ( *CG_PermanentMark )( vec3_t origin, vec3_t dir, float orientation, float sScale, float tScale, float red, float green, float blue, float alpha, qboolean doLighting, float sCenter, float tCenter, markFragment_t *markFragments, void *polyVerts );
	int ( *CG_PermanentTreadMarkDecal )( treadMark_t *treadMark, qboolean startSegment, qboolean doLighting, markFragment_t *markFragments, void *polyVerts );
	int ( *CG_PermanentUpdateTreadMark )( treadMark_t *treadMark, float alpha, float minSegment, float maxSegment, float maxOffset, float texScale );
	void ( *CG_ProcessInitCommands )( dtiki_t *tiki, refEntity_t *ent );
	void ( *CG_EndTiki )( dtiki_t *tiki );
	void ( *SetPerformanceCounters )( int totalTris, int totalVerts, int totalTexels, int worldTris, int worldVerts, int characterLights );

	debugLine_t **debugLines;
	int *numDebugLines;
	debugString_t **debugStrings;
	int *numDebugStrings;

	orientation_t ( *TIKI_OrientationInternal )( dtiki_t *tiki, int entNum, int tagNum, float scale );
	qboolean ( *TIKI_IsOnGroundInternal )( dtiki_t *tiki, int entNum, int tagNum, float thresHold );
	void ( *TIKI_SetPoseInternal )( void *skeletor, frameInfo_t *frameInfo, int *boneTag, vec4_t boneQuat[4], float actionWeight );
	void * ( *TIKI_Alloc )( int size );
	float ( *GetRadiusInternal )( dtiki_t *tiki, int entNum, float scale );
	float ( *GetCentroidRadiusInternal )( dtiki_t *tiki, int entNum, float scale, vec3_t centroid );
	void ( *GetFrameInternal )( dtiki_t *tiki, int entNum, skelAnimFrame_t *newFrame );

} refImport_t;

typedef struct refExport_s
{
	void ( *Shutdown )( );
	void ( *BeginRegistration )( glConfig_t *glConfigOut );
	qhandle_t ( *RegisterModel )( char *name );
	qhandle_t ( *SpawnEffectModel )( char *name, vec3_t pos, vec3_t axis[3] );
	qhandle_t ( *RegisterServerModel )( char *name );
	void ( *UnregisterServerModel )( qhandle_t model );
	qhandle_t ( *RegisterShader )( char *name );
	qhandle_t ( *RegisterShaderNoMip )( char *name );
	qhandle_t ( *RefreshShaderNoMip )( char *name );
	void ( *EndRegistration )( );
	void ( *FreeModels )( );
	void ( *SetWorldVisData )( BYTE *vis );
	void ( *LoadWorld )( char *name );
	void ( *PrintBSPFileSizes )( );
	int ( *MapVersion )( );
	void ( *ClearScene )( );
	void ( *AddRefEntityToScene )( refEntity_t *ent, int parentEntityNumber );
	void ( *AddRefSpriteToScene )( refEntity_t *ent );
	qboolean ( *AddPolyToScene )( qhandle_t hShader, int numVerts, polyVert_t *verts, int renderFx );
	void ( *AddTerrainMarkToScene )( int terrainIndex, qhandle_t hShader, int numVerts, polyVert_t *verts, int renderfx );
	void ( *AddLightToScene )( vec3_t org, float intensity, float r, float g, float b, dlighttype_t type );
	void ( *RenderScene )( refDef_t *fd );
	refEntity_t * ( *GetRenderEntity )( int entityNumber );
	void ( *SavePerformanceCounters )( );
	void ( *SetColor )( vec4_t color );
	void ( *Set2DWindow )( int x, int y, int w, int h, float left, float right, float bottom, float top, float n, float f );
	void ( *DrawStretchPic )( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
	void ( *DrawTilePic )( float x, float y, float w, float h, qhandle_t hShader );
	void ( *DrawTilePicOffset )( float x, float y, float w, float h, qhandle_t hShader, int offsetX, int offsetY );
	void ( *DrawTrianglePic )( vec2_t *points, vec2_t *texCoords, qhandle_t hShader );
	void ( *DrawBackground )( int cols, int rows, int bgr, BYTE *data );
	void ( *DrawStretchRaw )( int x, int y, int w, int h, int cols, int rows, int components, BYTE *data );
	void ( *DebugLine )( vec3_t start, vec3_t end, float r, float g, float b, float alpha );
	void ( *DrawBox )( float x, float y, float w, float h );
	void ( *AddBox )( float x, float y, float w, float h );
	void ( *BeginFrame )( stereoFrame_t stereoFrame );
	void ( *Scissor )( int x, int y, int width, int height );
	void ( *DrawLineLoop )( vec2_t *points, int count, int stippleFactor, int stippleMask );
	void ( *EndFrame )( int *frontEndMsec, int *backEndMsec );
	int ( *MarkFragments )( int numPoints, vec3_t *points, vec3_t projection, int maxPoints, float *pointBuffer, int maxFragments, markFragment_t *fragmentBuffer, float radiusSquared );
	int ( *MarkFragmentsForInlineModel )( clipHandle_t bmodel, vec3_t angles, vec3_t origin, int numPoints, vec3_t *points, vec3_t projection, int maxPoints, float *pointBuffer, int maxFragments, markFragment_t *fragmentBuffer, float radiusSquared );
	void ( *GetInlineModelBounds )( int index, vec3_t mins, vec3_t maxs );
	void ( *GetLightingForDecal )( vec3_t light, vec3_t facing, vec3_t origin );
	void ( *GetLightingForSmoke )( vec3_t light, vec3_t origin );
	int ( *R_GatherLightSources )( vec3_t pos, vec3_t *lightPos, vec3_t *lightIntensity, int maxLights );
	void ( *ModelBounds )( qhandle_t handle, vec3_t mins, vec3_t maxs );
	float ( *ModelRadius )( qhandle_t handle );
	dtiki_t * ( *R_Model_GetHandle )( qhandle_t handle );
	void ( *DrawString )( fontheader_t *font, char *text, float x, float y, int maxLen, qboolean virtualScreen );
	float ( *GetFontHeight )( fontheader_t *font );
	float ( *GetFontStringWidth )( fontheader_t *font, char *string );
	fontheader_t * ( *LoadFont )( char *name );
	void ( *SwipeBegin )( float thisTime, float life, qhandle_t hShader );
	void ( *SwipePoint )( vec3_t point1, vec3_t point2, float time );
	void ( *SwipeEnd )( );
	void ( *SetRenderTime )( int t );
	float ( *Noise )( vec3_t vec, int len );
	qboolean ( *SetMode )( int mode, glConfig_t *glConfig );
	void ( *SetFullscreen )( qboolean fullScreen );
	int ( *GetShaderWidth )( qhandle_t hShader );
	int ( *GetShaderHeight )( qhandle_t hShader );
	char * ( *GetGraphicsInfo )( );
	void ( *ForceUpdatePose )( refEntity_t *model );
	orientation_t ( *TIKI_Orientation )( refEntity_t *model, int tagNum );
	qboolean ( *TIKI_IsOnGround )( refEntity_t *model );
	void ( *SetFrameNumber )( int frameNumber );

} refExport_t;

typedef struct clientGameImport_s
{
	int apiVersion;

	void ( *Printf )( char *fmt, ... );
	void ( *DPrintf )( char *fmt, ... );
	void ( *DebugPrintf )( char *fmt, ... );
	void * ( *Malloc )( int size );
	void ( *Free )( void *ptr );
	void ( *Error )( errorParm_t code, char *fmt, ... );
	int ( *Milliseconds )( );
	char * ( *LV_ConvertString )( char *string );
	cvar_t * ( *Cvar_Get )( char *varName, char *varValue, int varFlags );
	void ( *Cvar_Set )( char *varName, char *varValue );
	int ( *Argc )( );
	char * ( *Argv )( int arg );
	char * ( *Args )( );
	void ( *AddCommand )( char *cmdName, xcommand_t cmdFunction );
	void ( *Cmd_Stuff )( const char *text );
	void ( *Cmd_Execute )( cbufExec_t execWhen, char *text );
	void ( *Cmd_TokenizeString )( char *textIn );
	int ( *FS_ReadFile )( char *qpath, void **buffer );
	void ( *FS_FreeFile )( void *buffer );
	int ( *FS_WriteFile )( char *qpath, void *buffer, int size );
	void ( *FS_WriteTextFile )( char *qpath, void *buffer, int size );
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
	void ( *S_StartLocalSound )( char *soundName, qboolean forceLoad );
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
	void ( *MUSIC_UpdateMood )( musicMood_t current, musicMood_t fallback );
	void ( *MUSIC_UpdateVolume )( float volume, float fadeTime );
	float * ( *get_camera_offset )( qboolean *lookActive, qboolean *resetView );
	void ( *R_ClearScene )( );
	void ( *R_RenderScene )( refDef_t *fd );
	void ( *R_LoadWorldMap )( char *name );
	void ( *R_PrintBSPFileSizes )( );
	int ( *MapVersion )( );
	int ( *R_MapVersion )( );
	qhandle_t ( *R_RegisterModel )( char *name );
	qhandle_t ( *R_SpawnEffectModel )( char *name, vec3_t pos, vec3_t axis[3] );
	qhandle_t ( *R_RegisterServerModel )( char *name );
	void ( *R_UnregisterServerModel )( qhandle_t hModel );
	qhandle_t ( *R_RegisterSkin )( char *name );
	qhandle_t ( *R_RegisterShader )( char *name );
	qhandle_t ( *R_RegisterShaderNoMip )( char *name );
	void ( *R_AddRefEntityToScene )( refEntity_t *ent, int parentEntityNumber );
	void ( *R_AddRefSpriteToScene )( refEntity_t *ent );
	void ( *R_AddLightToScene )( vec3_t org, float intensity, float r, float g, float b, dlighttype_t type );
	qboolean ( *R_AddPolyToScene )( qhandle_t hShader, int numVerts, polyVert_t *verts, int renderFx );
	void ( *R_AddTerrainMarkToScene )( int terrainIndex, qhandle_t hShader, int numVerts, polyVert_t *verts, int renderFx );
	void ( *R_SetColor )( vec4_t rgba );
	void ( *R_DrawStretchPic )( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
	fontheader_t * ( *R_LoadFont )( const char *name );
	void ( *R_DrawString )( fontheader_t *font, char *text, float x, float y, int maxLen, qboolean virtualScreen );
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
	void ( *GetGlconfig )( glConfig_t *glConfig );
	qboolean ( *GetParseEntityState )( int parseEntityNumber, entityState_t *state );
	int ( *GetCurrentCmdNumber )( );
	qboolean ( *GetUserCmd )( int cmdNumber, userCmd_t *userCmd );
	qboolean ( *GetServerCommand )( int serverCommandNumber, qboolean differentServer );
	qboolean ( *Alias_Add )( char *alias, char *name, char *parameters );
	qboolean ( *Alias_ListAdd )( aliasList_t *list, char *alias, char *name, char *parameters );
	char * ( *Alias_FindRandom )( char *alias, aliasListNode_t **ret );
	char * ( *Alias_ListFindRandom )( aliasList_t *list, char *alias, aliasListNode_t **ret );
	void ( *Alias_Dump )( );
	void ( *Alias_Clear )( );
	aliasList_t * ( *AliasList_New )( char *name );
	void ( *Alias_ListFindRandomRange )( aliasList_t *list, char *alias, int *minIndex, int *maxIndex, float *totalWeight );
	aliasList_t * ( *Alias_GetGlobalList )( );
	void ( *UI_ShowMenu )( char *name, qboolean bForce );
	void ( *UI_HideMenu )( char *name, qboolean bForce );
	int ( *UI_FontStringWidth )( fontheader_t *font, char *string, int maxLen );
	int ( *Key_StringToKeynum )( char *str );
	char * ( *Key_KeynumToBindString )( int keyNum );
	void ( *Key_GetKeysForCommand )( char *command, int *key1, int *key2 );
	dtiki_t * ( *R_Model_GetHandle )( qhandle_t handle );
	int ( *TIKI_NumAnims )( dtiki_t *pmdl );
	void ( *TIKI_CalculateBounds )( dtiki_t *pmdl, float scale, vec3_t mins, vec3_t maxs );
	char * ( *TIKI_Name )( dtiki_t *tiki, int entNum );
	void * ( *TIKI_GetSkeletor )( dtiki_t *tiki, int entNum );
	void ( *TIKI_SetEyeTargetPos )( dtiki_t *tiki, int entNum, vec3_t pos );
	char * ( *Anim_NameForNum )( dtiki_t *tiki, int animNum );
	int ( *Anim_NumForName )( dtiki_t *tiki, char *name );
	int ( *Anim_Random )( dtiki_t *tiki, char *name );
	int ( *Anim_NumFrames )( dtiki_t *tiki, int animNum );
	float ( *Anim_Time )( dtiki_t *tiki, int animNum );
	float ( *Anim_Frametime )( dtiki_t *tiki, int animNum );
	void ( *Anim_Delta )( dtiki_t *tiki, int animNum, vec3_t delta );
	int ( *Anim_Flags )( dtiki_t *tiki, int animNum );
	int ( *Anim_FlagsSkel )( dtiki_t *tiki, int animNum );
	float ( *Anim_CrossblendTime )( dtiki_t *tiki, int animNum );
	qboolean ( *Anim_HasCommands )( dtiki_t *tiki, int animNum );
	qboolean ( *Frame_Commands )( dtiki_t *tiki, int animNum, int frameNum, tikiCmd_t *tikiCmds );
	qboolean ( *Frame_CommandsTime )( dtiki_t *pmdl, int animNum, float start, float end, tikiCmd_t *tikiCmd );
	int ( *Surface_NameToNum )( dtiki_t *pmdl, char *name );
	int ( *Tag_NumForName )( dtiki_t *pmdl, char *name );
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

typedef struct gameImport_s
{
	void ( *Printf )( char *format, ... );
	void ( *DPrintf )( char *format, ... );
	void ( *DPrintf2 )( char *format, ... );
	void ( *DebugPrintf )( char *format, ... );
	void ( *Error )( errorParm_t code, char *format, ... );
	int ( *Milliseconds )( );
	char * ( *LV_ConvertString )( char *string );
	void * ( *Malloc )( int size );
	void ( *Free )( void *ptr );
	cvar_t * ( *Cvar_Get )( char *varName, char *varValue, int varFlags );
	void ( *Cvar_Set )( char *varName, char *varValue );
	cvar_t *( *cvar_set2 )( char *varName, char *varValue, qboolean force );
	cvar_t *( *NextCvar )( cvar_t *var );
	int ( *Argc )( );
	char * ( *Argv )( int arg );
	char * ( *Args )( );
	void ( *AddCommand )( char *cmdName, xcommand_t cmdFunction );
	int ( *FS_ReadFile )( char *qpath, void **buffer );
	void ( *FS_FreeFile )( void *buffer );
	int ( *FS_WriteFile )( char *qpath, void *buffer, int size );
	fileHandle_t ( *FS_FOpenFileWrite )( char *fileName );
	fileHandle_t ( *FS_FOpenFileAppend )( char *fileName );
	char *( *FS_PrepFileWrite )( char *fileName );
	int ( *FS_Write )( char *qpath, void *buffer, int size );
	int ( *FS_Read )( void *buffer, int len, fileHandle_t fileHandle );
	void ( *FS_FCloseFile )( fileHandle_t fileHandle );
	int ( *FS_FTell )( fileHandle_t fileHandle );
	int ( *FS_FSeek )( fileHandle_t fileHandle, long int offset, fsOrigin_t origin );
	void ( *FS_Flush )( fileHandle_t fileHandle );
	int ( *FS_FileNewer )( char *source, char *destination );
	void ( *FS_CanonicalFilename )( char *fileName );
	char **( *FS_ListFiles )( char *qpath, char *extension, qboolean wantSubs, int *numFiles );
	void ( *FS_FreeFileList )( char **list );
	char *( *GetArchiveFileName )( char *fileName, char *extension );
	void ( *SendConsoleCommand )( const char *text );
	void ( *DebugGraph )( float value, int color );
	void ( *SendServerCommand )( int client, char *format, ... );
	void ( *DropClient )( int client, char *reason );
	void ( *MSG_WriteBits )( int value, int bits );
	void ( *MSG_WriteChar )( int c );
	void ( *MSG_WriteByte )( int c );
	void ( *MSG_WriteSVC )( int c );
	void ( *MSG_WriteShort )( int c );
	void ( *MSG_WriteLong )( int c );
	void ( *MSG_WriteFloat )( float f );
	void ( *MSG_WriteString )( char *s );
	void ( *MSG_WriteAngle8 )( float f );
	void ( *MSG_WriteAngle16 )(float f );
	void ( *MSG_WriteCoord )( float f );
	void ( *MSG_WriteDir )( vec3_t dir );
	void ( *MSG_StartCGM )( int type );
	void ( *MSG_EndCGM )( );
	void ( *MSG_SetClient )( int client );
	void ( *MSG_SetBroadcastVisible )( vec3_t pos, vec3_t posB );
	void ( *MSG_SetBroadcastHearable )( vec3_t pos, vec3_t posB );
	void ( *MSG_SetBroadcastAll )( );
	void ( *setConfigstring )( int index, char *val );
	char *( *getConfigstring )( int index );
	void ( *setUserinfo )( int index, char *val );
	void ( *getUserinfo )( int index, char *buffer, int bufferSize );
	void ( *SetBrushModel )( gentity_t *ent, char *name );
	void ( *ModelBoundsFromName )( char *name, vec3_t mins, vec3_t maxs );
	qboolean ( *SightTraceEntity )( gentity_t *touch, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int contentMask, qboolean cylinder );
	qboolean ( *SightTrace )( vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int passEntityNum, int passEntityNum2, int contentMask, qboolean cylinder );
	void ( *trace )( trace_t *results, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int passEntityNum, int contentMask, qboolean cylinder, qboolean traceDeep );
	baseShader_t * ( *GetShader )( int shaderNum );
	int ( *pointcontents )( vec3_t p, int passEntityNum );
	int ( *pointbrushnum )( vec3_t p, clipHandle_t model );
	void ( *AdjustAreaPortalState )( gentity_t *ent, qboolean open );
	int ( *AreaForPoint )( vec3_t pos );
	qboolean ( *AreasConnected )( int area1, int area2);
	qboolean ( *inPVS )( float *p1, float *p2 );
	void ( *linkentity )( gentity_t *gEnt );
	void ( *unlinkentity )( gentity_t *gEnt );
	int ( *AreaEntities )( worldSector_t *node, areaParms_t *ap );
	void ( *ClipToEntity )( trace_t *tr, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int entityNum, int contentMask );
	int ( *imageindex )( char *name );
	int ( *itemindex )( char *name );
	int ( *soundindex )( char *name, int streamed );
	dtiki_t *( *RegisterTiki )( char *path );
	dtiki_t *( *modeltiki )( char *name );
	dtikianim_t *( *modeltikianim )( char *name );
	void ( *SetLightStyle )( int index, char *data );
	char *( *GameDir )( );
	qboolean ( *setmodel )( gentity_t *ent, char *name );
	void ( *clearmodel )( gentity_t *ent );
	int ( *TIKI_NumAnims )( dtiki_t *tiki );
	int ( *TIKI_NumSurfaces )( dtiki_t *tiki );
	int ( *TIKI_NumTags )( dtiki_t *tiki );
	void( *TIKI_CalculateBounds )( dtiki_t *tiki, float scale, vec3_t mins, vec3_t maxs );
	void *( *TIKI_GetSkeletor )( dtiki_t *tiki, int entNum );
	char *( *Anim_NameForNum )( dtiki_t *tiki, int animNum );
	int ( *Anim_NumForName )( dtiki_t *tiki, char *name );
	int ( *Anim_Random )( dtiki_t *tiki, char *name );
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
	int ( *NumHeadModels )( char *model );
	void ( *GetHeadModel )( char *model, int num, char *name );
	int ( *NumHeadSkins )( char *model );
	void ( *GetHeadSkin )( char *model, int num, char *name );
	qboolean ( *Frame_Commands )( dtiki_t *tiki, int animNum, int frameNum, tikiCmd_t *tikiCmds );
	int ( *Surface_NameToNum )( dtiki_t *tiki, char *name );
	char * ( *Surface_NumToName )( dtiki_t *tiki, int surfacenum );
	int ( *Tag_NumForName )( dtiki_t *pmdl, char *name );
	char * ( *Tag_NameForNum )(  dtiki_t *pmdl, int tagNum);
	orientation_t ( *TIKI_OrientationInternal )( dtiki_t *tiki, int entNum, int tagNum, float scale );
	float ** ( *TIKI_TransformInternal )( dtiki_t *tiki, int entNum, int tagNum );
	qboolean ( *TIKI_IsOnGroundInternal )( dtiki_t *tiki, int entNum, int num, float threshold );
	void ( *TIKI_SetPoseInternal )( dtiki_t *tiki, int entNum, frameInfo_t *frameInfo, int *boneTag, vec4_t *boneQuat, float actionWeight );
	char * ( *CM_GetHitLocationInfo )( int location, float *radius, float *offset );
	char * ( *CM_GetHitLocationInfoSecondary )( int location, float *radius, float *offset );
	qboolean ( *Alias_Add )( dtiki_t *pmdl, char *alias, char *name, char *parameters );
	char * ( *Alias_FindRandom )( dtiki_t *tiki, char *alias, aliasListNode_t **ret );
	void ( *Alias_Dump )( dtiki_t *tiki );
	void ( *Alias_Clear )( dtiki_t *tiki );
	void ( *Alias_UpdateDialog )( dtikianim_t *tiki, char *alias );
	char * ( *TIKI_NameForNum )( dtiki_t *tiki );
	qboolean ( *GlobalAlias_Add )( char *alias, char *name, char *parameters );
	char * ( *GlobalAlias_FindRandom )( char *alias, aliasListNode_t **ret );
	void ( *GlobalAlias_Dump )( );
	void ( *GlobalAlias_Clear )( );	void ( *centerprintf )( gentity_t *ent, char *format, ... );
	void ( *locationprintf )( gentity_t *ent, int x, int y, char *format, ... );
	void ( *Sound )( vec3_t *org, int entNum, soundChannel_t channel, char *soundName, float volume, float minDist, float pitch, float maxDist, int streamed );
	void ( *StopSound )( int entNum, soundChannel_t channel );
	float ( *SoundLength )( soundChannel_t channel, char *name );
	BYTE * ( *SoundAmplitudes )( soundChannel_t channel, char *name );
	int ( *S_IsSoundPlaying )( soundChannel_t channel, char *name );
	short unsigned int ( *CalcCRC )( unsigned char *start, int count );

	debugLine_t **DebugLines;
	int *numDebugLines;
	debugString_t **DebugStrings;
	int *numDebugStrings;

	void ( *LocateGameData )( gentity_t *gEnts, int numGEntities, int sizeofGEntity, playerState_t *clients, int sizeofGameClient );
	void ( *SetFarPlane )(int farPlane );
	void ( *SetSkyPortal )( qboolean skyPortal );
	void ( *Popmenu )( int client , int i );
	void ( *Showmenu )( int client, char *name, qboolean force );
	void ( *Hidemenu )( int client, char *name, qboolean force );
	void ( *Pushmenu )( int client, char *name );
	void ( *HideMouseCursor )( int client );
	void ( *ShowMouseCursor )( int client );
	char * ( *MapTime )( );
	void ( *LoadResource )( char *name );
	void ( *ClearResource )( );
	int ( *Key_StringToKeynum )( char *str );
	char * ( *Key_KeynumToBindString )( int keyNum );
	void ( *Key_GetKeysForCommand )( char *command, int *key1, int *key2 );
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

} gameImport_t;

typedef struct gameExport_s
{
	int apiVersion;

	void ( *Init )( int svsStartTime, int randomSeed );
	void ( *Shutdown )( );
	void ( *Cleanup )( qboolean sameMap );
	void ( *Precache )( );
	void ( *SetMap )( char *mapName );
	void ( *Restart )( );
	void ( *SetTime )( int svsStartTime, int svsTime );
	void ( *SpawnEntities )( char *entities, int svsTime);
	char *( *ClientConnect )( int clientNum, qboolean firstTime );
	void ( *ClientBegin )( gentity_t *ent, userCmd_t *cmd );
	void ( *ClientUserinfoChanged )( gentity_t *ent, char *userInfo );
	void ( *ClientDisconnect )( gentity_t *ent );
	void ( *ClientCommand )( gentity_t *ent );
	void ( *ClientThink )( gentity_t *ent, userCmd_t *ucmd, userEyes_t *eyeInfo );
	void ( *BotBegin )( gentity_t *ent, userCmd_t *cmd );
	void ( *BotThink )( gentity_t *ent, userCmd_t *ucmd, userEyes_t *eyeInfo );
	void ( *PrepFrame )( );
	void ( *RunFrame )( int svsTime, int frameTime );
	void ( *ServerSpawned )( );
	void ( *RegisterSounds )( );
	qboolean ( *AllowPaused )( );
	qboolean ( *ConsoleCommand )( );
	void ( *ArchivePersistant )( char *name, qboolean loading );
	void ( *WriteLevel )( char *fileName, qboolean autoSave );
	qboolean ( *ReadLevel )( char *fileName );
	qboolean ( *LevelArchiveValid )( char *fileName );
	void ( *ArchiveInteger )( int *i );
	void ( *ArchiveFloat )( float *fl );
	void ( *ArchiveString )( char *s );
	void ( *ArchiveSvsTime )( int *pi );
	orientation_t ( *TIKI_Orientation )( gentity_t *edict, int num );
	void ( *DebugCircle )( float *org, float radius, float r, float g, float b, float alpha, qboolean horizontal );
	void ( *SetFrameNumber )( int frameNumber );
	void ( *SoundCallback )( int entNum, soundChannel_t channelNumber, char *name );

	profGame_t *profStruct;
	gentity_t *gentities;
	int gentitySize;
	int numEntities;
	int maxEntities;
	char *errorMessage;

} gameExport_t;

/*
These are custom function non related to MOHAA
these are just here to make the code easier to follow
*/







//---------------------------------------------------------
/*		These are list of funcitons I need to work on

	SkelVec3();
	SkelVec4();
	SkelMat3();
	SkelMat4();
	Entity_t();
	FileRead();
	Weapon();
	Item();
*/


/* Moved the custom gamex86.c structs so it's easier to find and modify when needed */
/* Razorapid */

#define		ACCESSLEVEL_PROTECTNAME		(1<<0)
#define		ACCESSLEVEL_MAPCHANGE		(1<<1)
#define		ACCESSLEVEL_RESTART			(1<<2)
#define		ACCESSLEVEL_GAMETYPE		(1<<3)

#define		ACCESSLEVEL_FRAGLIMIT		(1<<4)
#define		ACCESSLEVEL_TIMELIMIT		(1<<4)

#define		ACCESSLEVEL_KICK			(1<<5)
#define		ACCESSLEVEL_BADCMD			(1<<6)
#define		ACCESSLEVEL_BAN				(1<<7)
#define		ACCESSLEVEL_REMOVEBAN		(1<<8)
#define		ACCESSLEVEL_CHATFILTER		(1<<9)
#define		ACCESSLEVEL_ADMINPROCMD		(1<<10)
#define		ACCESSLEVEL_LISTADMINS		(1<<11)
#define		ACCESSLEVEL_RCON			(1<<12)

#define		ACCESSLEVEL_MAX	16383
#define		ACCESSLEVEL_MAX_STR	"16383"

#define LOG_DEBUG		"DEBUG"
#define LOG_INFO		"INFO"
#define LOG_WARNING		"WARNING"
#define LOG_ERROR		"ERROR"


#define MEM_DEBUG 0   // print debug messages at memory allocations/frees
#define LOG_LEVEL LOG_INFO   // only log messages with level equal to or higher than this will be logged
#define CUSTOM_SIGNAL_HANDLERS 1   // uses custom signal handlers to print stack trace info when the application crashes (linux only)

#define REBORN_VERSION "1.117"
#define REBORN_VERSION_FLOAT 1.117

#define REBORN_MASTER_PORT 27900
#define	HEARTBEAT_MSEC	300*1000 // 5 mins
#define	HEARTBEAT_GAME	"mohrb"
#define MD5_MOHAA_SERVER_ORIG "6e6c402831306025d75ada19a66fc156"
#define MD5_GAMEX_ORIG	"06546e0c5345617b22a340c85110b574"

typedef unsigned short sa_family_t;

typedef struct {
	const char	*cmd;
	void	(*function)(gentity_t *ent);
} clientCommand_t;

typedef struct {
	const char *cmd;
	void (*function)(void);
} consoleCommand_t;

typedef struct {
	int		serverCommand;
	const char	*cmd;
	char	*cmdtrue;
	char	*cmdfalse;
	cvar_t	**cvar;
	char	*(*function)( char *cmd, char *args, gentity_t *ent );
} clientCommandFilter_t;

typedef struct {
	char	name[64];
	int		anyOccurence;
} namefilter_t;

typedef struct {
	char	name[64];
	char	pass[64];
} protnamefilter_t;

typedef union {
	char	word[64];
} chatfilter_t;

typedef struct {
	unsigned	mask;
	unsigned	compare;
} ipfilter_t;

typedef struct {
	char	ip[32];
} ipfilter2_t;

typedef struct {
	char	votecmd[64];
	char	voteargs[2048];
} votefilter_t;

typedef struct {
	char	mapname[64];
} mapfilter_t;

typedef struct {
	qboolean	active;
	char		pass[64];
	char		strAccessRights[32];
	int			AccessRights;
	int			general;
} admin_t;

typedef struct {
	char	login[64];
	char	pass[64];
	char	namepass[64];
	char	strAccessRights[32];
	int		AccessRights;
	qboolean	active;
	int		clientnum; //if active
	int general;
} adminlist_t;

// Struct for saving client data across map changes using Shared Memory
typedef struct clientdataShared_s
{
	int isAdmin;
	int adminNum;
	int disablechat;
	int disabletaunt;
	int chatpeniltypoints;

} clientdataShared_t;

typedef struct patchdataShared_s
{
	clientdataShared_t sharedClientData[MAX_CLIENTS];
	DWORD			   allowedConnectionlessPacketIPs[5];
	int				   allowedConnectionlessPacketCount;

} patchdataShared_t;

typedef struct servervote_s
{
	int		voteActive;		// flag used to determine if there is some active vote called
							// when players are connecting or disconnecting
							// this will prevent from disconnect and re-vote action
							// and if used to determine if vote has expired
	
	char	voterName[256];
	int		voterClientNum;
	
	char	voteCmd[256];
	char	voteArgs[1024];
	int		voteArgsCount;

	int		yesCount;
	int		noCount;
	int		maxVotesCount;

	char	votedYes[4096];
	char	votedNo[4096];
	int		votersClientNums;
	int		votedYesClientNums;
	int		votedNoClientNums;

	int		voteTime;
	int		expireTime;

} servervote_t;


typedef enum fileaccesstype_s
{
	FAT_R,
	FAT_W,
	FAT_A,
	FAT_RP,
	FAT_WP,
	FAT_AP,
	FAT_RB,
	FAT_WB,
	FAT_AB,
	FAT_RPB,
	FAT_WPB,
	FAT_APB

} fileaccesstype_t;

typedef struct filedesc_s
{
	int inUse;
	FILE *f;
	fileaccesstype_t accessType;

} filedesc_t;

typedef union qfile_gus {
	FILE*		o;
	void		*z;
} qfile_gut;

typedef struct qfile_us {
	qfile_gut	file;
	qboolean	unique;
} qfile_ut;

typedef struct {
	qfile_ut	handleFiles;
	qboolean	handleSync;
	int			baseOffset;
	int			fileSize;
	int			zipFilePos;
	qboolean	zipFile;
	char		name[MAX_ZPATH];
} fileHandleData_t;

typedef struct cmd_function_s
{
	struct cmd_function_s	*next;
	char					*name;
	xcommand_t				function;
	//completionFunc_t	complete;
	DWORD				complete;
} cmd_function_t;

typedef struct strdata_s
{
	char	*buffer;
	int		ref;
	int		alloced;
	int		len;
} strdata;

typedef struct str_s
{
	strdata		*s;
} str;

typedef struct Condition_s {
	char *name;
	short int __delta;
	short int __index;
	void *__pfn_or_delta2;
} Condition_t;

typedef struct Conditional_s {
	struct Class_s baseClass;
	qboolean result;
	qboolean previous_result;
	bool checked;
	Condition_t condition;
	Container_t parmList;
} Conditional;

typedef struct StateMap_s
{
	struct Class_s				baseClass;
	Container					stateList;
	void						*current_conditions;
	Container					*current_conditionals;
	str							filename;
} StateMap;

typedef struct Player_s
{
	struct Sentient_s		baseSentient;
	StateMap				*statemap_Legs;
	StateMap				*statemap_Torso;
	State					*currentState_Legs;
	State					*currentState_Torso;
	str						last_torso_anim_name;
	str						last_leg_anim_name;
	str						partAnim[2];
	int						m_iPartSlot[2];
	float					m_fPartBlends[2];
	str						partOldAnim[2];
	float					partBlendMult[2];
	char					animdone_Legs;
	char					animdone_Torso;
	Container				legs_conditionals;
	Container				torso_conditionals;
	Conditional				*m_pLegsPainCond;
	Conditional				*m_pTorsoPainCond;
	float					m_fLastDeltaTime;
	qboolean				m_bActionAnimPlaying;
	int						m_iBaseActionAnimSlot;
	str						m_sActionAnimName;
	int						m_iActionAnimType;
	qboolean				m_bActionAnimDone;
	str						m_sOldActionAnimName;
	float					m_fOldActionAnimWeight;
	float					m_fOldActionAnimFadeTime;
	int						m_iOldActionAnimType;
	qboolean				m_bMovementAnimPlaying;
	int						m_iBaseMovementAnimSlot;
	str						m_sMovementAnimName;
	str						m_sOldMovementAnimName;
	float					m_fOldMovementWeight;
	float					m_fOldMovementFadeTime;
	str						m_sSlotAnimNames[16];
	int						movecontrol;
    int						m_iMovePosFlags;
	int						last_camera_type;
	vec3_t					oldvelocity;
	vec3_t					old_v_angle;
	vec3_t					oldorigin;
	float					animspeed;
	float					airspeed;
	vec3_t					m_vPushVelocity;
	float					blend[4];
	float					fov;
	float					selectedfov;
	qboolean				m_iInZoomMode;
	vec3_t					vAngle;
	vec3_t					vViewPos;
	vec3_t					vViewAngle;
    int						buttons;
    int						newButtons;
    int						serverNewButtons;
	float					respawn_time;
	int						lastAttackButton;
	float					damage_blood;
	float					damage_alpha;
	vec3_t					damage_blend;
	vec3_t					damage_from;
	vec3_t					damage_angles;
	float					damage_count;
	float					damage_yaw;
	float					next_painsound_time;
	str						waitForState;
	SafePtr2_t				camera;
	SafePtr2_t				actor_camera;
	SafePtr2_t				actor_to_watch;
	qboolean				actor_camera_right;
	qboolean				starting_actor_camera_right;
	int						music_current_mood;
	int						music_fallback_mood;
	float					music_current_volume;
	float					music_saved_volume;
	float					music_volume_fade_time;
	int						reverb_type;
	float					reverb_level;
	qboolean				gibbed;
	float					pain;
	int						pain_dir;
	int						pain_type;
	int						pain_location;
	qboolean				take_pain;
	int						nextpaintime;
	char					knockdown;
	char					canfall;
	char					falling;
	int						feetfalling;
	vec3_t					falldir;
	char					mediumimpact;
	char					hardimpact;
	qboolean				music_forced;
	userCmd_t				lastUcmd;
	userEyes_t				lastEyeInfo;
	float					animheight;
	vec3_t					yaw_forward;
	vec3_t					yaw_left;
	SafePtr2_t				atobject;
    float					atobject_dist;
	Vector					atobject_dir;
	SafePtr2_t				toucheduseanim;
	int						useanim_numloops;
	SafePtr2_t				useitem_in_use;
	float					move_left_vel;
	float					move_right_vel;
	float					move_backward_vel;
	float					move_forward_vel;
	float					move_up_vel;
	float					move_down_vel;
	int						moveresult;
	float					damage_multiplier;
	voicetype_t				m_voiceType;
	int						num_deaths;
	int						num_kills;
	int						num_won_matches;
	int						num_lost_matches;
	char					m_bTempSpectator;
	char					m_bSpectator;
	char					m_bAllowFighting;
	char					m_bReady;
	int						m_iPlayerSpectating;
	teamType_t				dm_team;
	SafePtr2_t				teamPtr;
    float					m_fTeamSelectTime;
	DWORD					*m_pLastSpawnpoint;
	bool					voted;
	int						votecount;
	float					m_fWeapSelectTime;
	float					fAttackerDispTime;
	SafePtr2_t				pAttackerDistPointer;
	int						m_iInfoClient;
	int						m_iInfoClientHealth;
	float					m_fInfoClientTime;
	int						m_iNumObjectives;
	int						m_iObjectivesCompleted;
	str						m_sPerferredWeaponOverride;
	Vector					mvTrail[2];
	Vector					mvTrailEyes[2];
	int						mCurTrailOrigin;
	int						mLastTrailTime;
	int						statsHitsTaken;
	int						statsEnemiesKilled;
	int						statsObjectsDestroyed;
	int						statsShotsFired;
	int						statsHits;
	float					statsAccuracy;
	float					statsTimeUsed;
	int						statsHeadShots;
	int						statsTorsoShots;
	int						statsLeftLegShots;
	int						statsRightLegShots;
	int						statsGroinShots;
	int						statsLeftArmShots;
	int						statsRightArmShots;
	int						un1[20];
} Player;

typedef struct {
  float bounds[2][3];
  msurface_t *firstSurface;
  int numSurfaces;
  void **pFirstMarkFragment;
  int iNumMarkFragment;
  int frameCount;
  qboolean hasLightmap;
} bmodel_t;

typedef struct {
  float width;
  float height;
  float origin_x;
  float origin_y;
  float scale;
  shader_t *shader;
} sprite_t;

typedef enum { MOD_BAD, MOD_BRUSH, MOD_TIKI, MOD_SPRITE } modtype_t;
typedef struct model_s {
  char name[128];
  modtype_t type;
  int index;
  qboolean serveronly;
  union {
    bmodel_t *bmodel;
    dtiki_t *tiki;
    sprite_t *sprite;
  } d;
} model_t;

typedef union sval_u {
	int type;
	char *stringValue;
	float floatValue;
	int intValue;
	char charValue;
	unsigned char byteValue;
	unsigned char *posValue;
	int MaxVarStackOffset;
	int HasExternal;
	union sval_u *node;
	unsigned int sourcePosValue;
} sval_t;

typedef struct {
	const char		*opcodename;
	int				opcodelength;
	char			opcodestackoffset;
	char			isexternal;
	char			alignment[3];
} opcode_t;

typedef struct {
	unsigned char	opcode;
	char			VarStackOffset;
} opcode_info_t;

typedef struct Entry_s {
	int			key;
	int			value;
	int			index;
	struct Entry_s *next;
} Entry_t;

typedef struct {
	Entry_t				**table;
	unsigned int		tableLength;
	Entry_t				**reverseTable;
	unsigned int		threshold;
	unsigned int		count;
	short unsigned int	tableLengthIndex;
	Entry_t				*defaultEntry;
} con_arrayset_t;

typedef struct ClassDef_s
{
	const char			*classname;
	const char			*classID;
	const char			*superclass;
	void				*(*newInstance)(void);
	int					classSize;
	void				*responses;
	void				**responseLookup;
	struct ClassDef_s	*super;
	struct ClassDef_s	*next;
	struct ClassDef_s	*prev;
} ClassDef_t;

typedef struct ScriptMaster_s {
	Listener_t				baseListener;

	int						stackCount;			// 0x0014 (0020)

	ScriptVariable_t		avar_Stack[1025];	// 0x0018 (0024)
	Event_t					fastEvent[11];		// 0x2020 (8224)

	ScriptVariable_t		*pTop;				// 0x20FC (8444)

	unsigned int			cmdCount;			// 0x2100 (8448) - cmd count
	int						cmdTime;			// 0x2104 (8452) - Elapsed time since the maximum reached LOCALSTACK_SIZE
	int						maxTime;			// 0x2108 (8456) - Maximum time for LOCALSTACK_SIZE

	SafePtr2_t				m_PreviousThread;	// 0x210C (8460) - parm.previousthread
	SafePtr2_t				m_CurrentThread;	// 0x211C (8476) - Current thread

	con_map					m_GameScripts;
	Container_t				m_menus;
	con_timer				timingList;
	con_arrayset			StringDict;
	int						iPaused;
} ScriptMaster_t;

typedef struct ScriptClass_s {
	Listener_t			baseListener;
	void				*m_Script;
	SafePtr2_t			m_Self;
	struct ScriptVM_s	*m_Threads;
} ScriptClass_t;

typedef struct ScriptVM_s {
	void			*next;

	struct ScriptThread_s	*m_Thread;
	ScriptClass_t	*m_ScriptClass;

	void			*m_Stack;

	ScriptVariable_t m_ReturnValue;

	unsigned char	*m_PrevCodePos;
	unsigned char	*m_CodePos;

	unsigned char	state;
	unsigned char	m_ThreadState;
} ScriptVM_t;

typedef struct ScriptPointer_s {
	Container_t list;
} ScriptPointer_t;

typedef struct AbstractScript_s {
	int				m_Filename;
	char			*m_SourceBuffer;
	unsigned int	m_SourceLength;
	con_map			*m_ProgToSource;
} AbstractScript_t;

typedef struct StateScript_s {
	Class_t baseClass;
	con_set label_list;
} StateScript_t;

typedef struct CatchBlock_s {
	StateScript_t m_StateScript;
	unsigned char *m_TryStartCodePos;
	unsigned char *m_TryEndCodePos;
} CatchBlock_t;

typedef struct GameScript_s {
	void				*_vfptr;
	int					m_Filename;
	char				*m_SourceBuffer;
	unsigned int		m_SourceLength;
	con_map				*m_ProgToSource;
	Container_t			m_CatchBlocks;
	StateScript_t		m_State;
	unsigned char		*m_ProgBuffer;
	unsigned int		m_ProgLength;
} GameScript_t;

typedef struct ScriptThread_s {
	Listener_t	baseListener;
	ScriptVM_t	*m_ScriptVM;
} ScriptThread_t;

typedef struct earthquake_s {
	int		duration;
	float	magnitude;
	char	no_rampup;
	char	no_rampdown;

	int		starttime;
	int		endtime;
	SafePtr2_t	thread;
} earthquake_t;

typedef struct Game_s {
	Listener_t		baseListener;
	gclient_t		*clients;
	qboolean		autosaved;
	int				maxclients;
	int				maxentities;
} Game_t;

typedef struct Level_s {
	Listener_t		baseListener;
	char *current_map;

	int mHealthPopCount;

	str m_mapscript;
	str m_precachescript;
	str m_pathfile;
	str m_mapfile;

	int spawn_entnum;
	int spawnflags;

	int framenum;
	int inttime;
	int intframetime;

	float time;
	float frametime;

	int		svsTime;
	float	svsFloatTime;
	int		svsStartTime;
	int		svsEndTime;

	str level_name;
	str mapname;
	str spawnpoint;
	str nextmap;

	int m_iCuriousVoiceTime;
	int m_iAttackEntryAnimTime;

	qboolean playerfrozen;

	float		intermissiontime;
	int			intermissiontype;
	int			exitintermission;

	int total_secrets;
	int found_secrets;

	trace_t impact_trace;

	float earthquake_magnitude;

	qboolean cinematic;

	qboolean	ai_on;
	qboolean	m_bAlarm;
	qboolean	mbNoDropHealth;
	int			m_iPapersLevel;

	qboolean mission_failed;
	qboolean died_already;
	qboolean near_exit;

	Vector	water_color;
	Vector	lava_color;
	float	water_alpha;
	float	lava_alpha;

	str current_soundtrack;
	str saved_soundtrack;

	// Screen state
	Vector			m_fade_color;
	float			m_fade_alpha;
	float			m_fade_time;
	float			m_fade_time_start;
	int				m_fade_type;
	int				m_fade_style;

	float			m_letterbox_fraction;
	float			m_letterbox_time;
	float			m_letterbox_time_start;
	int				m_letterbox_dir;

	Container_t automatic_cameras;

	int m_numArenas;

	float	m_voteTime;
	int		m_voteYes;
	int		m_voteNo;
	int		m_numVoters;
	str		m_voteString;

	Vector m_intermission_origin;
	Vector m_intermission_angle;

	int frame_skel_index;
	int skel_index[ 1024 ];

	qboolean	m_LoopProtection;

	Container_t m_SimpleArchivedEntities;
	struct Sentient_s *m_HeadSentient[ 2 ];

	earthquake_t	earthquakes[ 10 ];
	int				num_earthquakes;

	Vector m_vObjectiveLocation;
	bool spawning;
} Level_t;

typedef enum
{
	OP_DONE,
	OP_BOOL_JUMP_FALSE4,
	OP_BOOL_JUMP_TRUE4,
	OP_VAR_JUMP_FALSE4,
	OP_VAR_JUMP_TRUE4,

	OP_BOOL_LOGICAL_AND,
	OP_BOOL_LOGICAL_OR,
	OP_VAR_LOGICAL_AND,
	OP_VAR_LOGICAL_OR,

	OP_BOOL_TO_VAR,

	OP_JUMP4,
	OP_JUMP_BACK4,

	OP_STORE_INT0,
	OP_STORE_INT1,
	OP_STORE_INT2,
	OP_STORE_INT3,
	OP_STORE_INT4,

	OP_BOOL_STORE_FALSE,
	OP_BOOL_STORE_TRUE,

	OP_STORE_STRING,
	OP_STORE_FLOAT,
	OP_STORE_VECTOR,
	OP_CALC_VECTOR,
	OP_STORE_NULL,
	OP_STORE_NIL,

	OP_EXEC_CMD0,
	OP_EXEC_CMD1,
	OP_EXEC_CMD2,
	OP_EXEC_CMD3,
	OP_EXEC_CMD4,
	OP_EXEC_CMD5,
	OP_EXEC_CMD_COUNT1,

	OP_EXEC_CMD_METHOD0,
	OP_EXEC_CMD_METHOD1,
	OP_EXEC_CMD_METHOD2,
	OP_EXEC_CMD_METHOD3,
	OP_EXEC_CMD_METHOD4,
	OP_EXEC_CMD_METHOD5,
	OP_EXEC_CMD_METHOD_COUNT1,

	OP_EXEC_METHOD0,
	OP_EXEC_METHOD1,
	OP_EXEC_METHOD2,
	OP_EXEC_METHOD3,
	OP_EXEC_METHOD4,
	OP_EXEC_METHOD5,
	OP_EXEC_METHOD_COUNT1,

	OP_LOAD_GAME_VAR,
	OP_LOAD_LEVEL_VAR,
	OP_LOAD_LOCAL_VAR,
	OP_LOAD_PARM_VAR,
	OP_LOAD_SELF_VAR,
	OP_LOAD_GROUP_VAR,
	OP_LOAD_OWNER_VAR,
	OP_LOAD_FIELD_VAR,
	OP_LOAD_ARRAY_VAR,
	OP_LOAD_CONST_ARRAY1,

	OP_STORE_FIELD_REF,				
	OP_STORE_ARRAY_REF,

	OP_MARK_STACK_POS,

	OP_STORE_PARAM,

	OP_RESTORE_STACK_POS,

	OP_LOAD_STORE_GAME_VAR,
	OP_LOAD_STORE_LEVEL_VAR,
	OP_LOAD_STORE_LOCAL_VAR,
	OP_LOAD_STORE_PARM_VAR,
	OP_LOAD_STORE_SELF_VAR,
	OP_LOAD_STORE_GROUP_VAR,
	OP_LOAD_STORE_OWNER_VAR,

	OP_STORE_GAME_VAR,				
	OP_STORE_LEVEL_VAR,
	OP_STORE_LOCAL_VAR,
	OP_STORE_PARM_VAR,
	OP_STORE_SELF_VAR,
	OP_STORE_GROUP_VAR,
	OP_STORE_OWNER_VAR,
	OP_STORE_FIELD,
	OP_STORE_ARRAY,
	OP_STORE_GAME,
	OP_STORE_LEVEL,
	OP_STORE_LOCAL,
	OP_STORE_PARM,
	OP_STORE_SELF,
	OP_STORE_GROUP,
	OP_STORE_OWNER,

	OP_BIN_BITWISE_AND,
	OP_BIN_BITWISE_OR,
	OP_BIN_BITWISE_EXCL_OR,
	OP_BIN_EQUALITY,
	OP_BIN_INEQUALITY,
	OP_BIN_LESS_THAN,
	OP_BIN_GREATER_THAN,
	OP_BIN_LESS_THAN_OR_EQUAL,
	OP_BIN_GREATER_THAN_OR_EQUAL,
	OP_BIN_PLUS,
	OP_BIN_MINUS,
	OP_BIN_MULTIPLY,
	OP_BIN_DIVIDE,
	OP_BIN_PERCENTAGE,

	OP_UN_MINUS,
	OP_UN_COMPLEMENT,
	OP_UN_TARGETNAME,
	OP_BOOL_UN_NOT,
	OP_VAR_UN_NOT,
	OP_UN_CAST_BOOLEAN,
	OP_UN_INC,
	OP_UN_DEC,
	OP_UN_SIZE,

	OP_SWITCH,

	OP_PREVIOUS,
} opcode_e;

/*typedef enum sval_type_s
{
	sval_none,
	sval_un1,
	sval_statement_list,
	sval_label,
	sval_case,
	sval_negative,
	sval_assignment,
	sval_if,
	sval_ifelse,
	sval_while,
	sval_and,
	sval_or,
	sval_cmd_method,
	sval_cmd_method_ret,
	sval_cmd,
	sval_cmd_default_ret,
	sval_field,
	sval_store_method,
	sval_store_string,
	sval_store_integer,
	sval_store_float,
	sval_calc_vector,
	sval_store_null,
	sval_store_nil,
	sval_func1,
	sval_operation,
	sval_not,
	sval_array,
	sval_constarray,
	sval_makearray,
	sval_catch,
	sval_switch,
	sval_break,
	sval_continue
} sval_type_e;*/

typedef struct ScriptArrayHolder_s {
	con_set arrayValue;
	int refCount;
} ScriptArrayHolder_t;

typedef struct ScriptConstArrayHolder_s {
	ScriptVariable_t *constArrayValue;
	int refCount;
	int size;
} ScriptConstArrayHolder_t;

typedef struct DM_Team_s {
	Listener_t	baseListener;

	Container_t m_spawnpoints;
	Container_t m_players;

	int			m_maxplayers;

	str			m_teamname;

	int			m_teamnumber;

	int			teamType;

	int			m_countdown;

	int			m_teamwins;
	int			m_wins_in_a_row;

	int			m_iKills;
	int			m_iDeaths;

	qboolean	m_bHasSpawnedPlayers;
} DM_Team_t;

typedef struct DM_Manager_s {
	Listener_t		baseListener;
	Container_t		m_players;
	Container_t		m_teams;

	DM_Team_t		m_team_spectator;
	DM_Team_t		m_team_freeforall;
	DM_Team_t		m_team_allies;
	DM_Team_t		m_team_axis;

	float			m_fRoundTime;
	float			m_fRoundEndTime;

	char			m_bAllowRespawns;
	char			m_bRoundBasedGame;
	char			m_bIgnoringClockForBomb;
	char			pad1;

	int				m_iTeamWin;
	int				m_iDefaultRoundLimit;

	int				m_csTeamClockSide;
	int				m_csTeamBombPlantSide;

	int				m_iNumTargetsToDestroy;
	int				m_iNumTargetsDestroyed;

	int				m_iNumBombsPlanted;
	int				m_iTotalMapTime;

} DM_Manager_t;

typedef struct Trigger_s {
	Animate baseAnimate;
	float wait;
	float delay;
	float trigger_time;
	qboolean triggerActivated;
	int count;
	int noise;
	int message;
	ScriptThreadLabel_t m_Thread;
	SafePtr2_t activator;
	int respondto;
	qboolean useTriggerDir;
	float triggerCone;
	Vector triggerDir;
	float triggerDirYaw;
	qboolean triggerable;
	qboolean removable;
	qboolean edgeTriggered;
	int multiFaceted;
} Trigger_t;

typedef struct Item_s {
	Trigger_t			baseTrigger;
	SafePtr2_t			owner;
	qboolean			respawnable;
	qboolean			playrespawn;
	float				respawntime;
	str					dialog_needed;
	int					item_index;
	str					item_name;
	int					maximum_amount;
	int					amount;
	str					sPickupSound;
	qboolean			no_remove;
} Item_t;

typedef struct Weapon_s {
	Item_t			baseItem;
	int m_iAnimSlot;
	qboolean attached;
	float nextweaponsoundtime;
	float m_fLastFireTime;
	str current_attachToTag;
	str attachToTag_main;
	str attachToTag_offhand;
	str holster_attachToTag;
	float lastScale;
	Vector lastAngles;
	qboolean lastValid;
	qboolean auto_putaway;
	qboolean use_no_ammo;
	qboolean crosshair;
	int m_iZoom;
	float m_fZoomSpreadMult;
	qboolean m_bAutoZoom;
	qboolean m_bSemiAuto;
	float maxrange;
	float minrange;
	str viewmodel;
	int weaponstate;
	int rank;
	int order;
	SafePtr2_t last_owner;
	float last_owner_trigger_time;
	qboolean notdroppable;
	int aimanim;
	int aimframe;
	Vector holsterOffset;
	Vector holsterAngles;
	float holsterScale;
	float next_noise_time;
	float next_noammo_time;
	qboolean m_bShouldReload;
	float fire_delay[2];
	str ammo_type[2];
	int ammorequired[2];
	int startammo[2];
	str projectileModel[2];
	float bulletdamage[2];
	float bulletcount[2];
	float bulletrange[2];
	float bulletknockback[2];
	float projectilespeed[2];
	Vector bulletspread[2];
	Vector bulletspreadmax[2];
	firetype_t firetype[2];
	int ammo_clip_size[2];
	int ammo_in_clip[2];
	float max_charge_time[2];
	float min_charge_time[2];
	meansOfDeath_t meansofdeath[2];
	int loopfire[2];
	str worldhitspawn[2];
	int tracerfrequency[2];
	int tracercount[2];
	Vector viewkickmin[2];
	Vector viewkickmax[2];
	int quiet[2];
	float m_fFireSpreadMultAmount[2];
	float m_fFireSpreadMultFalloff[2];
	float m_fFireSpreadMultCap[2];
	float m_fFireSpreadMultTimeCap[2];
	float m_fFireSpreadMultTime[2];
	float m_fFireSpreadMult[2];
	qboolean m_bShareClip;
	qboolean m_bCanPartialReload;
	qboolean autoaim;
	float charge_fraction;
	qboolean putaway;
	int firemodeindex;
	int weapon_class;
	const_str_t m_csWeaponGroup;
	float m_fMovementSpeed;
	str m_sAmmoPickupSound;
	str m_NoAmmoSound;
	int m_iNumShotsFired;
	int m_iNumHits;
	float m_fAccuracy;
	float m_fTimeUsed;
	int m_iNumHeadShots;
	int m_iNumTorsoShots;
	int m_iNumLeftLegShots;
	int m_iNumRightLegShots;
	int m_iNumGroinShots;
	int m_iNumLeftArmShots;
	int m_iNumRightArmShots;
	int mAIRange;
	SafePtr2_t aim_target;

} Weapon_t;

typedef struct Projectile_s {
	Animate baseAnimate;

	float fov;
	int owner;
	float speed;
	float minspeed;
	float damage;
	float knockback;
	float life;
	float dmlife;
	float minlife;
	float dlight_radius;
	float charge_fraction;

	SafePtr2_t weap;
	Vector dlight_color;
	Vector addvelocity;
	meansOfDeath_t meansofdeath;
	struct FuncBeam_t *m_beam;
	int projFlags;
	str bouncesound;
	str bouncesound_metal;
	str bouncesound_hard;
	str bouncesound_water;
	float fLastBounceTime;
	str impactmarkshader;
	str impactmarkorientation;
	float impactmarkradius;
	str explosionmodel;
	SafePtr2_t target;
	float fDrunk;
	float fDrunkRate;
	char addownervelocity;
	char can_hit_owner;
	char remove_when_stopped;
	char m_bExplodeOnTouch;
	char m_bHurtOwnerOnly;
	char align1[3];
	int m_iSmashThroughGlass;
} Projectile_t;

typedef struct Explosion_s {
	Projectile_t baseProjectile;
	float flash_r;
	float flash_g;
	float flash_b;
	float flash_a;
	float flash_radius;
	float flash_time;
	int flash_type;
	float radius_damage;
	bool hurtOwnerOnly;
	float radius;
	qboolean constant_damage;
	qboolean damage_every_frame;
} Explosion_t;

typedef struct Mover_s {
	Trigger_t baseTrigger;
	Vector finaldest;
	Vector angledest;
	Event_t *endevent;
	int moveflags;
} Mover_t;

typedef struct ScriptSlave_s {
	Mover_t baseMover;
	float attack_finished;
	int dmg;
	int dmg_means_of_death;
	qboolean commandswaiting;
	Vector TotalRotation;
	Vector NewAngles;
	Vector NewPos;
	Vector ForwardDir;
	float speed;
	void *waypoint;
	float traveltime;
	void *splinePath;
	float splineTime;
	cSpline_4_512_t *m_pCurPath;
	int m_iCurNode;
	float m_fCurSpeed;
	float m_fIdealSpeed;
	Vector m_vIdealPosition;
	Vector m_vIdealDir;
	float m_fIdealAccel;
	float m_fIdealDistance;
	float m_fLookAhead;
	qboolean splineangles;
	qboolean ignoreangles;
	qboolean ignorevelocity;
	qboolean moving;
} ScriptSlave_t;

typedef struct Door_s {
	ScriptSlave_t baseScriptSlave;
	str sound_open_start;
	str sound_open_end;
	str sound_close_start;
	str sound_close_end;
	str sound_message;
	str sound_locked;
	float lastblocktime;
	float angle;
	Vector dir;
	float diropened;
	int state;
	int previous_state;
	int trigger;
	int nextdoor;
	SafePtr2_t master;
	float next_locked_time;
	bool m_bAlwaysAway;
	qboolean locked;
} Door_t;

typedef struct RotatingDoor_s {
	Door_t baseDoor;
	float angle;
	Vector startangle;
	int init_door_direction;
} RotatingDoor_t;

typedef struct FileRead_s {
	Class_t baseClass;
	str filename;
	size_t length;
	byte *buffer;
	byte *pos;
} FileRead_t;

typedef struct Archiver_s {
	Class_t baseClass;
	Container_t classpointerList;
	Container_t fixupList;
	str filename;
	qboolean fileerror;
	fileHandle_t file;
	FileRead_t readfile;
	int archivemode;
	int numclassespos;
	qboolean harderror;
	int m_iNumBytesIO;
} Archiver_t;

typedef struct EventQueueNode_s { /* size 36 id 806 */
	Event_t *event;
	int inttime;
	int flags;
	SafePtr2_t m_sourceobject;
	struct EventQueueNode_s *prev;
	struct EventQueueNode_s *next;
} EventQueueNode_t;

typedef struct FuncLadder_s {
	Entity_t	baseEntity;
	Vector		m_vFacingAngles;
	Vector		m_vFacingDir;
} FuncLadder_t;

typedef struct BSplineControlPoint_s {
	Class_t baseClass;
	float roll;
	Vector position;
	Vector orientation;
	float speed;
} BSplineControlPoint_t;

typedef struct BSpline_s {
	Class_t baseClass;
	BSplineControlPoint_t *control_points;
	int num_control_points;
	int loop_control_point;
	int curvetype;
	qboolean has_orientation;
} BSpline_t;

typedef struct SplinePath_s {
	Entity_t baseEntity;
	struct SplinePath_s *owner;
	struct SplinePath_s *next;
	struct SplinePath_s *loop;
	str loop_name;

	float speed;
	float fov;
	float fadeTime;
	qboolean doWatch;
	str watchEnt;
	str triggertarget;
} SplinePath_t;

typedef struct CameraMoveState_s {
	Class_t baseClass;
	Vector pos;
	Vector movedir;
	Vector angles;
	BSpline_t cameraPath;
	SafePtr2_t splinePath;
	SafePtr2_t currentNode;
	SafePtr2_t loopNode;
	float cameraTime;
	int lastTime;
	int newTime;
	qboolean followingpath;
	SafePtr2_t followEnt;
	SafePtr2_t orbitEnt;
} CameraMoveState_t;

typedef struct CameraWatchState_s {
	Class_t baseClass;
	Vector watchAngles;
	SafePtr2_t watchEnt;
	qboolean watchNodes;
	qboolean watchPath;
} CameraWatchState_t;

typedef struct CameraState_s {
	Class_t baseClass;
	CameraMoveState_t move;
	CameraWatchState_t watch;
	float fov;
} CameraState_t;

typedef struct Camera_s {
	Entity_t	baseEntity;
	float follow_yaw;
	qboolean follow_yaw_fixed;
	float follow_dist;
	int follow_mask;
	float camera_speed;
	float camera_fov;
	float orbit_height;
	qboolean orbit_dotrace;
	float auto_fov;
	float automatic_startTime;
	float automatic_stopTime;
	float automatic_radius;
	float automatic_maxFOV;
	qboolean automatic_active;
	Container_t automatic_states;
	Vector m_vPosOffset;
	
	CameraState_t currentstate;
	CameraState_t newstate;
	float watchTime;
	float followTime;
	float fovTime;
	float fadeTime;
	float fovFadeTime;
	float followFadeTime;
	float watchFadeTime;
	str nextCamera;
	qboolean showcamera;
} Camera_t;

typedef struct WindowObject_s {
	Entity_t	baseEntity;
	int			m_iDebrisType;
	str			m_sBrokenModel;
} WindowObject_t;

typedef struct VehicleBase_s {
	Animate baseAnimate;
	struct VehicleBase_s *vlink;
	Vector offset;
} VehicleBase_t;

typedef struct cVehicleSlot_s {
	Class_t baseClass;
	SafePtr2_t ent;
	int flags;
	int boneindex;
	int enter_boneindex;
	int prev_takedamage;
	int prev_solid;
	int prev_contents;
} cVehicleSlot_t;

typedef struct cTurretSlot_s {
	cVehicleSlot_t basecVehicleSlot;
	int owner_prev_takedamage;
	int owner_prev_solid;
	int owner_prev_contents;
} cTurretSlot_t;

typedef struct vehicleState_s {
	float origin[3];
	float velocity[3];
	int groundEntityNum;
	qboolean walking;
	qboolean groundPlane;
	trace_t groundTrace;
	int entityNum;
	float desired_dir[2];
	qboolean hit_obstacle;
	float hit_origin[3];
	float obstacle_normal[3];
	qboolean useGravity; 
} vehicleState_t;

typedef struct vmove_s {
	vehicleState_t *vs;
	float frametime;
	float desiredspeed;
	int tracemask;
	int numtouch;
	int touchents[ 32 ];
	float mins[ 3 ];
	float maxs[ 3 ];
} vmove_t;

typedef struct vml_s  {
	qboolean validGroundTrace;
	float previous_origin[3];
	float previous_velocity[3];
} vml_t;

typedef struct gridpoint_s {
	Vector origin;
	Vector neworigin;
	Vector origindelta;
	Vector changed;
	Vector newvel;
	gentity_t *groundentity;
	vmove_t vm;
	vehicleState_t vs;
} gridpoint_t;

typedef struct cMoveGrid_s {
	Class_t baseClass;
	gridpoint_t *GridPoints;
	vmove_t v;
	float orientation[3][3];
	int m_iXRes;
	int m_iYRes;
	int m_iZRes;
} cMoveGrid_t;

typedef struct TurretGun_s {
	Weapon_t baseWeapon;
	char m_bFakeBullets;
	char m_bPlayerUsable;
	char m_bUsable;
	char m_bRestable;
	Vector m_vIdleCheckOffset;
	Vector m_vViewOffset;
	Vector m_vUserViewAng;
	Vector m_vUserLastCmdAng;
	float m_fIdlePitchSpeed;
	float m_fTurnSpeed;
	float m_fPitchUpCap;
	float m_fPitchDownCap;
	float m_fStartYaw;
	float m_fMaxYawOffset;
	float m_fUserDistance;
	float m_fMinBurstTime;
	float m_fMaxBurstTime;
	float m_fMinBurstDelay;
	float m_fMaxBurstDelay;
	float m_fFireToggleTime;
	int m_iFiring;
	int m_iIdleHitCount;
	SafePtr2_t m_pUserCamera;
	float m_fViewJitter;
	float m_fCurrViewJitter;
	Vector m_Aim_offset;
	SafePtr2_t m_pViewModel;
	ScriptThreadLabel_t m_UseThread;

	bool m_bHadOwner;
} TurretGun_t;

typedef struct VehicleTurretGun_s {
	TurretGun_t baseTurretGun;

	Vector m_vTargetAngles;
	Vector m_vLocalAngles;
	float m_mBaseOrientation[3][3];
	Vector m_vBaseAngles;
	Vector m_vLastBaseAngles;
	SafePtr2_t m_pBaseEntity;
	int m_iPitchBone;
	char m_bBOIsSet;
	char m_bUseRemoteControl;
	short align1;
	SafePtr2_t m_pRemoteOwner;
	int m_iBarrelTag;
	int m_iEyeBone;
	SafePtr2_t m_pVehicleOwner;
	bool m_bRemoveOnDeath;
	SafePtr2_t m_pCollisionEntity;
	bool m_bLocked;
	int  m_eSoundState;
	float m_fNextSoundState;
	str m_sSoundSet;
	
	Vector m_vBarrelPos;
	Vector m_vLastBarrelPos;
} VehicleTurretGun_t;

typedef struct VehicleSoundEntity_s {
	Entity_t baseEntity;

	SafePtr2_t m_pVehicle;
	bool m_bDoSoundStuff;
	int m_iTraceSurfaceFlags;
} VehicleSoundEntity_t;

typedef struct Vehicle_s {
	VehicleBase_t baseVehicleBase;
	cVehicleSlot_t driver; /* bitsize 384, bitpos 7328 */
	cVehicleSlot_t lastdriver; /* bitsize 384, bitpos 7712 */
	cVehicleSlot_t Passengers[32]; /* bitsize 12288, bitpos 8096 */
	cTurretSlot_t Turrets[8]; /* bitsize 3840, bitpos 20384 */
	int numPassengers; /* bitsize 32, bitpos 24224 */
	int numTurrets; /* bitsize 32, bitpos 24256 */
	int moveresult; /* bitsize 32, bitpos 24288 */
	int isBlocked; /* bitsize 32, bitpos 24320 */
	int m_iFrameCtr; /* bitsize 32, bitpos 24352 */
	int m_iGear; /* bitsize 32, bitpos 24384 */
	int m_iRPM; /* bitsize 32, bitpos 24416 */
	int m_iLastTiresUpdate; /* bitsize 32, bitpos 24448 */
	void *m_pAlternatePath; /* bitsize 32, bitpos 24480 */
	int m_iAlternateNode; /* bitsize 32, bitpos 24512 */
	void *m_pCurPath; /* bitsize 32, bitpos 24544 */
	int m_iCurNode; /* bitsize 32, bitpos 24576 */
	void *m_pNextPath; /* bitsize 32, bitpos 24608 */
	int m_iNextPathStartNode; /* bitsize 32, bitpos 24640 */
	float maxturnrate; /* bitsize 32, bitpos 24672 */
	float currentspeed; /* bitsize 32, bitpos 24704 */
	float turnangle; /* bitsize 32, bitpos 24736 */
	float turnimpulse; /* bitsize 32, bitpos 24768 */
	float moveimpulse; /* bitsize 32, bitpos 24800 */
	float prev_moveimpulse; /* bitsize 32, bitpos 24832 */
	float jumpimpulse; /* bitsize 32, bitpos 24864 */
	float speed; /* bitsize 32, bitpos 24896 */
	float conesize; /* bitsize 32, bitpos 24928 */
	float maxtracedist; /* bitsize 32, bitpos 24960 */
	float airspeed; /* bitsize 32, bitpos 24992 */
	float m_fGearRatio[6]; /* bitsize 192, bitpos 25024 */
	float m_fMass; /* bitsize 32, bitpos 25216 */
	float m_fFrontMass; /* bitsize 32, bitpos 25248 */
	float m_fBackMass; /* bitsize 32, bitpos 25280 */
	float m_fWheelBase; /* bitsize 32, bitpos 25312 */
	float m_fWheelFrontLoad; /* bitsize 32, bitpos 25344 */
	float m_fWheelFrontInnerLoad; /* bitsize 32, bitpos 25376 */
	float m_fWheelFrontOutterLoad; /* bitsize 32, bitpos 25408 */
	float m_fWheelFrontDist; /* bitsize 32, bitpos 25440 */
	float m_fWheelFrontSuspension; /* bitsize 32, bitpos 25472 */
	float m_fWheelBackLoad; /* bitsize 32, bitpos 25504 */
	float m_fWheelBackInnerLoad; /* bitsize 32, bitpos 25536 */
	float m_fWheelBackOutterLoad; /* bitsize 32, bitpos 25568 */
	float m_fWheelBackDist; /* bitsize 32, bitpos 25600 */
	float m_fWheelBackSuspension; /* bitsize 32, bitpos 25632 */
	float m_fCGHeight; /* bitsize 32, bitpos 25664 */
	float m_fBankAngle; /* bitsize 32, bitpos 25696 */
	float m_fTread; /* bitsize 32, bitpos 25728 */
	float m_fTrackWidth; /* bitsize 32, bitpos 25760 */
	float m_fTireFriction; /* bitsize 32, bitpos 25792 */
	float m_fDrag; /* bitsize 32, bitpos 25824 */
	float m_fTireRadius; /* bitsize 32, bitpos 25856 */
	float m_fFrontBrakes; /* bitsize 32, bitpos 25888 */
	float m_fBackBrakes; /* bitsize 32, bitpos 25920 */
	float m_fRollingResistance; /* bitsize 32, bitpos 25952 */
	float m_fTireRotationalSpeed; /* bitsize 32, bitpos 25984 */
	float m_fFrontBrakingForce; /* bitsize 32, bitpos 26016 */
	float m_fBackBrakingForce; /* bitsize 32, bitpos 26048 */
	float m_fBrakingPerformance; /* bitsize 32, bitpos 26080 */
	float m_fLastTurn; /* bitsize 32, bitpos 26112 */
	float m_fTangForce; /* bitsize 32, bitpos 26144 */
	float m_fInertia; /* bitsize 32, bitpos 26176 */
	float m_fDifferentialRatio; /* bitsize 32, bitpos 26208 */
	float m_fGearEfficiency; /* bitsize 32, bitpos 26240 */
	float m_fMaxTraction; /* bitsize 32, bitpos 26272 */
	float m_fTractionForce; /* bitsize 32, bitpos 26304 */
	float m_fAccelerator; /* bitsize 32, bitpos 26336 */
	float m_fTorque; /* bitsize 32, bitpos 26368 */
	float m_fDownForce; /* bitsize 32, bitpos 26400 */
	float m_fUpForce; /* bitsize 32, bitpos 26432 */
	float m_fLeftForce; /* bitsize 32, bitpos 26464 */
	float m_fRightForce; /* bitsize 32, bitpos 26496 */
	float m_fForwardForce; /* bitsize 32, bitpos 26528 */
	float m_fBackForce; /* bitsize 32, bitpos 26560 */
	float m_fBouncyCoef; /* bitsize 32, bitpos 26592 */
	float m_fSpringyCoef; /* bitsize 32, bitpos 26624 */
	float m_fYawMin; /* bitsize 32, bitpos 26656 */
	float m_fYawMax; /* bitsize 32, bitpos 26688 */
	float m_fRollMin; /* bitsize 32, bitpos 26720 */
	float m_fRollMax; /* bitsize 32, bitpos 26752 */
	float m_fZMin; /* bitsize 32, bitpos 26784 */
	float m_fZMax; /* bitsize 32, bitpos 26816 */
	float m_fYawCoef; /* bitsize 32, bitpos 26848 */
	float m_fRollCoef; /* bitsize 32, bitpos 26880 */
	float m_fZCoef; /* bitsize 32, bitpos 26912 */
	float m_fShaderOffset; /* bitsize 32, bitpos 26944 */
	float m_fSoundMinSpeed; /* bitsize 32, bitpos 26976 */
	float m_fSoundMinPitch; /* bitsize 32, bitpos 27008 */
	float m_fSoundMaxSpeed; /* bitsize 32, bitpos 27040 */
	float m_fSoundMaxPitch; /* bitsize 32, bitpos 27072 */
	float m_fVolumeMinSpeed; /* bitsize 32, bitpos 27104 */
	float m_fVolumeMinPitch; /* bitsize 32, bitpos 27136 */
	float m_fVolumeMaxSpeed; /* bitsize 32, bitpos 27168 */
	float m_fVolumeMaxPitch; /* bitsize 32, bitpos 27200 */
	float m_fStopStartDistance; /* bitsize 32, bitpos 27232 */
	float m_fStopStartSpeed; /* bitsize 32, bitpos 27264 */
	float m_fStopEndDistance; /* bitsize 32, bitpos 27296 */
	float m_fSkidAngle; /* bitsize 32, bitpos 27328 */
	float m_fSkidLeftForce; /* bitsize 32, bitpos 27360 */
	float m_fSkidRightForce; /* bitsize 32, bitpos 27392 */
	str weaponName; /* bitsize 32, bitpos 27424 */
	str m_sAnimationSet; /* bitsize 32, bitpos 27456 */
	str m_sSoundSet; /* bitsize 32, bitpos 27488 */
	str m_sExplosionModel; /* bitsize 32, bitpos 27520 */
	Vector last_origin; /* bitsize 96, bitpos 27552 */
	Vector seatangles; /* bitsize 96, bitpos 27648 */
	Vector seatoffset; /* bitsize 96, bitpos 27744 */
	Vector driveroffset; /* bitsize 96, bitpos 27840 */
	Vector Corners[4]; /* bitsize 384, bitpos 27936 */
	Vector v_angle; /* bitsize 96, bitpos 28320 */
	Vector yaw_forward; /* bitsize 96, bitpos 28416 */
	Vector yaw_left; /* bitsize 96, bitpos 28512 */
	Vector ground_normal; /* bitsize 96, bitpos 28608 */
	Vector base_angles; /* bitsize 96, bitpos 28704 */
	Vector prev_velocity; /* bitsize 96, bitpos 28800 */
	Vector real_velocity; /* bitsize 96, bitpos 28896 */
	Vector prev_origin; /* bitsize 96, bitpos 28992 */
	Vector real_acceleration; /* bitsize 96, bitpos 29088 */
	Vector prev_acceleration; /* bitsize 96, bitpos 29184 */
	Vector m_vOldMins; /* bitsize 96, bitpos 29280 */
	Vector m_vOldMaxs; /* bitsize 96, bitpos 29376 */
	Vector m_vCG; /* bitsize 96, bitpos 29472 */
	Vector m_vAngularAcceleration; /* bitsize 96, bitpos 29568 */
	Vector m_vAngularVelocity; /* bitsize 96, bitpos 29664 */
	Vector m_vAngles; /* bitsize 96, bitpos 29760 */
	Vector m_vFrontNormal; /* bitsize 96, bitpos 29856 */
	Vector m_vBackNormal; /* bitsize 96, bitpos 29952 */
	Vector m_vFrontAngles; /* bitsize 96, bitpos 30048 */
	Vector m_vBackAngles; /* bitsize 96, bitpos 30144 */
	Vector m_vBaseNormal; /* bitsize 96, bitpos 30240 */
	Vector m_vBaseAngles; /* bitsize 96, bitpos 30336 */
	Vector m_vPrevNormal; /* bitsize 96, bitpos 30432 */
	Vector m_vResistance; /* bitsize 96, bitpos 30528 */
	Vector m_vWheelForce; /* bitsize 96, bitpos 30624 */
	Vector m_vTangForce; /* bitsize 96, bitpos 30720 */
	Vector m_vForce; /* bitsize 96, bitpos 30816 */
	Vector m_vAcceleration; /* bitsize 96, bitpos 30912 */
	Vector m_vOriginOffset; /* bitsize 96, bitpos 31008 */
	Vector m_vOriginOffset2; /* bitsize 96, bitpos 31104 */
	Vector m_vOriginCornerOffset; /* bitsize 96, bitpos 31200 */
	Vector m_vAnglesOffset; /* bitsize 96, bitpos 31296 */
	Vector m_vSaveAngles; /* bitsize 96, bitpos 31392 */
	Vector m_vSkidOrigin; /* bitsize 96, bitpos 31488 */
	qboolean drivable; /* bitsize 32, bitpos 31584 */
	qboolean locked; /* bitsize 32, bitpos 31616 */
	qboolean hasweapon; /* bitsize 32, bitpos 31648 */
	qboolean showweapon; /* bitsize 32, bitpos 31680 */
	qboolean steerinplace; /* bitsize 32, bitpos 31712 */
	qboolean jumpable; /* bitsize 32, bitpos 31744 */
	qboolean m_bWheelSpinning; /* bitsize 32, bitpos 31776 */
	qboolean m_bIsSkidding; /* bitsize 32, bitpos 31808 */
	qboolean m_bIsBraking; /* bitsize 32, bitpos 31840 */
	qboolean m_bBackSlipping; /* bitsize 32, bitpos 31872 */
	qboolean m_bFrontSlipping; /* bitsize 32, bitpos 31904 */
	qboolean m_bAutomatic; /* bitsize 32, bitpos 31936 */
	qboolean m_bThinkCalled; /* bitsize 32, bitpos 31968 */
	qboolean m_bMovementLocked; /* bitsize 32, bitpos 32000 */
	qboolean m_bRemoveOnDeath; /* bitsize 32, bitpos 32032 */
	qboolean m_bStopEnabled; /* bitsize 32, bitpos 32064 */
	qboolean m_bEnableSkidding; /* bitsize 32, bitpos 32096 */
	vmove_t  *vm; /* bitsize 32, bitpos 32128 */
	vehicleState_t  vs; /* bitsize 1184, bitpos 32160 */
	cMoveGrid_t *m_sMoveGrid; /* bitsize 32, bitpos 33344 */
	float m_fIdealSpeed; /* bitsize 32, bitpos 33376 */
	Vector m_vIdealPosition; /* bitsize 96, bitpos 33408 */
	Vector m_vIdealDir; /* bitsize 96, bitpos 33504 */
	float m_fIdealAccel; /* bitsize 32, bitpos 33600 */
	float m_fIdealDistance; /* bitsize 32, bitpos 33632 */
	float m_fLookAhead; /* bitsize 32, bitpos 33664 */
	bool m_bAutoPilot; /* bitsize 8, bitpos 33696 */
	SafePtr2_t m_pCollisionEntity; /* bitsize 128, bitpos 33728 */
	int m_eSoundState; /* bitsize 32, bitpos 33856 */
	float m_fNextSoundState; /* bitsize 32, bitpos 33888 */
	SafePtr2_t m_pVehicleSoundEntities[4]; /* bitsize 512, bitpos 33920 */
	Vector m_vTireEnd[4]; /* bitsize 384, bitpos 34432 */
	char m_bTireHit[4]; /* bitsize 32, bitpos 34816 */
	Vector m_vNormalSum; /* bitsize 96, bitpos 34848 */
	int m_iNumNormals; /* bitsize 32, bitpos 34944 */
} Vehicle_t;

typedef struct Parm_s {
	Listener_t baseListener;
	SafePtr2_t other;
	SafePtr2_t owner;
	qboolean movedone;
	qboolean movefail;
	qboolean motionfail;
	qboolean upperfail;
	qboolean sayfail;
} Parm_t;

typedef struct LODMaster_s {
	Listener_t baseListener;
	SafePtr2_t m_current;
} LODMaster_t;

typedef struct LODSlave_s {
	Animate baseAnimate;
	Vector m_baseorigin;
	float m_scale;
} LODSlave_t;

typedef struct CrateObject_s {
	Entity_t baseEntity;

	float m_fMoveTime;
	int m_iDebrisType;
	Vector m_vJitterAngles;
	Vector m_vStartAngles;
	float m_fJitterScale;
} CrateObject_t;

typedef struct BarrelObject_s {
	Entity_t baseEntity;

	int m_iBarrelType;
	float m_fFluidAmount;
	float m_fHeightFluid;
	int m_bLeaksActive[ 4 ];
	Vector m_vLeaks[ 4 ];
	Vector m_vLeakNorms[ 4 ];
	Vector m_vJitterAngles;
	Vector m_vStartAngles;
	float m_fJitterScale;
	float m_fLastEffectTime;
	float m_fDamageSoundTime;
} BarrelObject_t;

typedef struct ViewJitter_s {
	Trigger_t	baseTrigger;

	float		m_fRadius;
	float		m_fEdgeEffect;
	Vector		m_vJitterStrength;
	float		m_fDuration;
	Vector		m_vTimeDecay;
	qboolean	m_bDoneDeath;
	float		m_fTimeRunning;
} ViewJitter_t;

typedef struct World_s {
	Entity_t baseEntity;
	Container_t m_targetListContainer;
	qboolean world_dying;
	
	float farplane_distance;
	Vector farplane_color;
	qboolean farplane_cull;
	float sky_alpha;
	qboolean sky_portal;
	float m_fAIVisionDistance;
	float m_fNorth;
} World_t;

typedef struct mmove_s {
	float		origin[ 3 ];
	float		velocity[ 3 ];

	qboolean	walking;
	qboolean	groundPlane;
	float		groundPlaneNormal[ 3 ];

	float		frametime;

	float		desired_speed;
	int			tracemask;
	float		desired_dir[ 2 ];

	int			entityNum;

	float		mins[ 3 ];
	float		maxs[ 3 ];

	int			numtouch;
	int			touchents[ 32 ];

	qboolean	hit_obstacle;
	float		hit_origin[ 3 ];
	int			hit_temp_obstacle;

	float		obstacle_normal[ 3 ];
} mmove_t;

typedef struct {
	qboolean		validGroundTrace;
	trace_t			groundTrace;
	float			previous_origin[ 3 ];
	float			previous_velocity[ 3 ];
} mml_t;

typedef struct FuncBeam_s {
	ScriptSlave_t baseScriptSlave;
	SafePtr2_t end;
	SafePtr2_t origin_target;
	float damage;
	float life;
	Vector end_point;
	qboolean use_angles;
	float shootradius;
	str shader;
} FuncBeam_t;

typedef struct pushed_s {
	Entity_t *ent;
	Vector localorigin;
	Vector origin;
	Vector localangles;
	Vector angles;
	float deltayaw;
} pushed_t;

typedef struct block_s {
  unsigned char data[256][52];
  unsigned char prev_data[256];
  unsigned char next_data[256];
  short int free_data;
  short int used_data;
  struct block_s *prev_block;
  struct block_s *next_block;
} block_t;

typedef struct MEM_BlockAlloc_s {
  block_t *m_FreeBlock;
  block_t *m_StartUsedBlock;
  block_t *m_StartFullBlock;
  unsigned int m_BlockCount;
} MEM_BlockAlloc_t;

typedef struct MEM_BlockAlloc_enum_s {
	MEM_BlockAlloc_t					*m_Owner;
	block_s								*m_CurrentBlock;
	unsigned char						m_CurrentData;
	int									m_CurrentBlockType;
} MEM_BlockAlloc_enum_t;

typedef struct MEM_TempAlloc_s {
	unsigned char *m_CurrentMemoryBlock;
	int m_CurrentMemoryPos;
} MEM_TempAlloc_t;

#define MAX_ALIAS_NAME_LENGTH 40
#define MAX_REAL_NAME_LENGTH 128
#define MAX_ALIASLIST_NAME_LENGTH 40

typedef struct AliasListNode_s {
	char alias_name[ MAX_ALIAS_NAME_LENGTH ];
	char real_name[ MAX_REAL_NAME_LENGTH ];
	float weight;

	// Static alias info
	byte stop_flag;
	struct AliasListNode_s *next;

	// Global alias info
	float pitch;
	float volume;
	float pitchMod;
	float volumeMod;
	float dist;
	float maxDist;
	int channel;
	int streamed;
	char *subtitle;
} AliasListNode_t;

typedef struct AliasList_s
   {
   char name[ MAX_ALIASLIST_NAME_LENGTH ];
   qboolean    dirty;
   int         num_in_list;
   AliasListNode_t ** sorted_list;
   AliasListNode_t * data_list;
   } AliasList_t;
