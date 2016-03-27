#ifndef __B_LOCAL_H__
#define __B_LOCAL_H__

#include "g_local.h"

//
// This section should be moved to QFILES.H
//

#define NAVFILE_ID			(('I')+('N'<<8)+('A'<<16)+('V'<<24))
#define NAVFILE_VERSION		2

typedef struct {
	unsigned	id;
	unsigned	version;
	unsigned	surfaceCount;
	unsigned	neighborCount;
} navheader_t;


#define MAX_SURFACES	4096

#define SF_PUSH				0x00000001
#define SF_WATERLEVEL1		0x00000002
#define SF_WATERLEVEL2		0x00000004
#define SF_WATER_NOAIR		0x00000008
#define SF_DUCK				0x00000010
#define SF_PAIN				0x00000020
#define SF_TELEPORTER		0x00000040
#define SF_PLATHIGH			0x00000080
#define SF_PLATLOW			0x00000100

typedef struct {
	vec3_t		origin;
	vec2_t		absmin;
	vec2_t		absmax;
	int			parm;
	unsigned	flags;
	unsigned	neighborCount;
	unsigned	neighborIndex;
} nsurface_t;


#define MAX_NEIGHBORS	16384

#define NF_JUMP				0x00000001
#define NF_DUCK				0x00000002
#define NF_PLAT				0x00000004
#define NF_FALL1			0x00000008
#define NF_FALL2			0x00000010

typedef struct {
	vec3_t		origin;
	vec2_t		absmin;		// region within this surface that is the portal to the other surface
	vec2_t		absmax;
	int			surfaceNum;
	unsigned	flags;		// jump, prerequisite button, will take falling damage, etc...
	float		cost;
	unsigned	filler;		// to be used as a "string index" into an array of targetnames for buttons, etc
} nneighbor_t;

#define WARNING		"\033" "3" "WARNING: "
#define ERROR		"\033" "1" "ERROR: "


// file buffers

extern	char botParms[0x10000];
extern	char *navFileData;


//
// Navigation susbsystem
//

#define NAVF_DUCK			0x00000001
#define NAVF_JUMP			0x00000002
#define NAVF_HOLD			0x00000004
#define NAVF_SLOW			0x00000008

#define METHOD_EXPONENTIAL	1
#define METHOD_LINEAR		2
#define METHOD_LOGRITHMIC	3
#define METHOD_ALWAYS		4
#define METHOD_NEVER		5

// combat maneuvers
#define CM_NONE				0

#define CM_CLOSE_DISTANCE	1
#define CM_OPEN_DISTANCE	2
#define CM_HOLD_DISTANCE	3

#define CM_GET_ITEM			4
#define CM_RUN_AWAY			5

#define CM_CIRCLE			6
#define CM_DUCK				7

typedef enum {SPOT_ORIGIN, SPOT_HEAD, SPOT_WEAPON, SPOT_LEGS, SPOT_GROUND} spot_t;

#define BOT_TIME_TO_LOSE_SIGHT	2000

#define SF_FROMMAPFILE	0x80000000

#define DEBUG_LEVEL_DETAIL	4
#define DEBUG_LEVEL_INFO	3
#define DEBUG_LEVEL_WARNING	2
#define DEBUG_LEVEL_ERROR	1
#define DEBUG_LEVEL_NONE	0


//
// b_main.c
//
void Debug_Printf( cvar_t *cv, int level, char *fmt, ... );
void Debug_BotPrintf( gentity_t *bot, cvar_t *cv, int debugLevel, char *fmt, ... );


//
// b_ai.c
//
extern cvar_t	*debugBotAI;
extern cvar_t	*debugBotFreeze;

void Bot_InitAI( void );
void Bot_Pain( gentity_t *bot, gentity_t *other, int damage );
void Bot_Touch( gentity_t *bot, gentity_t *other, trace_t *trace );
void BotSpawn( gentity_t *bot );
void Bot_Fetch_f( void );

//
// b_nav.c
//
extern char				*navFileData;
extern int				surfaceCount;
extern nsurface_t		*surface;
extern int				neighborCount;
extern nneighbor_t		*neighbor;
extern byte				*route;


void Nav_InitPreSpawn( void );
void Nav_InitPostSpawn( void );
void Nav_Shutdown( void );
void Nav_ShowPath( gentity_t *bot );
int Nav_GroundSurfaceNumber( gentity_t *ent );
int Nav_ItemSurfaceNumber( gentity_t *ent );
int Nav_EntitySurfaceNumber( gentity_t *ent );
int Nav_MoveToGoal( gentity_t *bot, vec3_t dir, int *flags );

//
// b_items.c
//
void Nav_InitRoamList( void );
qboolean WeaponIsOnLevel( int weapon );
gentity_t *Nav_ChooseRoamGoal( void );

//
// b_files.c
//
void Bot_ParseParms( const char *botName, gentity_t *bot, char *userinfo );
void Bot_LoadParms( void );
void Nav_LoadRoutes( void );
void Nav_SaveRoutes( void );

#endif
