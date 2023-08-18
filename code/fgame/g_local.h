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
// g_local.h -- local definitions for game module

#ifndef __G_LOCAL_H__
#define __G_LOCAL_H__

#include "q_shared.h"
#include "bg_public.h"
#include "g_public.h"

//==================================================================

// the "gameversion" client command will print this plus compile date
#define GAMEVERSION                "OPENMOHAA"

#define BODY_QUEUE_SIZE            8

#define INFINITE                   1000000

#define FRAMETIME                  level.frametime // float sec
#define CARNAGE_REWARD_TIME        3000
#define REWARD_SPRITE_TIME         2000

#define INTERMISSION_DELAY_TIME    1000
#define SP_INTERMISSION_DELAY_TIME 5000

// flags
#define FL_FLY              (1 << 0)
#define FL_SWIM             (1 << 1) // implied immunity to drowining
#define FL_INWATER          (1 << 2)
#define FL_GODMODE          (1 << 3)
#define FL_NOTARGET         (1 << 4)
#define FL_PARTIALGROUND    (1 << 5) // not all corners are valid
#define FL_TEAMSLAVE        (1 << 6) // not the first on the team
#define FL_NO_KNOCKBACK     (1 << 7)
#define FL_THINK            (1 << 8)
#define FL_BLOOD            (1 << 9)  // when hit, it should bleed.
#define FL_DIE_GIBS         (1 << 10) // when it dies, it should gib
#define FL_DIE_EXPLODE      (1 << 11) // when it dies, it will explode
#define FL_ROTATEDBOUNDS    (1 << 12) // model uses rotated mins and maxs
#define FL_DONTSAVE         (1 << 13) // don't add to the savegame
#define FL_IMMOBILE         (1 << 14) // entity has been immobolized somehow
#define FL_PARTIAL_IMMOBILE (1 << 15) // entity has been immobolized somehow
#define FL_POSTTHINK        (1 << 17) // call a think function after the physics have been run
#define FL_TOUCH_TRIGGERS   (1 << 18) // should this entity touch triggers
#define FL_AUTOAIM          (1 << 19) // Autoaim on this entity
#define FL_ANIMATE          (1 << 20) // entity has been immobolized somehow
#define FL_UNKNOWN2         (1 << 21)

#define SOUND_RADIUS        1500.0f // Sound travel distance for AI

//============================================================================

#include "g_main.h"

class str;
class Vector;
class Archiver;
class Entity;
template<typename T>
class Container;

typedef enum {
    SPECTATOR_NOT,
    SPECTATOR_FREE,
    SPECTATOR_FOLLOW,
    SPECTATOR_SCOREBOARD
} spectatorState_t;

typedef enum {
    TEAM_BEGIN, // Beginning a team game, spawn at base
    TEAM_ACTIVE // Now actively playing
} playerTeamStateState_t;

typedef struct {
    playerTeamStateState_t state;

    int location;

    int captures;
    int basedefense;
    int carrierdefense;
    int flagrecovery;
    int fragcarrier;
    int assists;

    float lasthurtcarrier;
    float lastreturnedflag;
    float flagsince;
    float lastfraggedcarrier;
} playerTeamState_t;

// the auto following clients don't follow a specific client
// number, but instead follow the first two active players
#define FOLLOW_ACTIVE1 -1
#define FOLLOW_ACTIVE2 -2

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in G_InitSessionData() / G_ReadSessionData() / G_WriteSessionData()
typedef struct {
    teamtype_t       sessionTeam;
    int              spectatorTime; // for determining next-in-line to play
    spectatorState_t spectatorState;
    int              spectatorClient; // for chasecam and follow mode
    int              wins, losses;    // tournament stats
    qboolean         teamLeader;      // true when this client is a team leader
} clientSession_t;

//
#define MAX_NETNAME    32
#define MAX_VOTE_COUNT 3

typedef enum {
    CS_FREE,
    CS_ZOMBIE,
    CS_CONNECTED,
    CS_PRIMED,
    CS_ACTIVE

} clientState_t;

//
// g_spawn.c
//
qboolean G_SpawnString(const char *key, const char *defaultString, char **out);
// spawn string returns a temporary reference, you must CopyString() if you want to keep it
qboolean    G_SpawnFloat(const char *key, const char *defaultString, float *out);
qboolean    G_SpawnInt(const char *key, const char *defaultString, int *out);
qboolean    G_SpawnVector(const char *key, const char *defaultString, float *out);
void        G_SpawnEntitiesFromString(void);
char       *G_NewString(const char *string);
const char *G_FixTIKIPath(const char *in);
void        G_GetNullEntityState(entityState_t *nullState);

//
// g_cmds.c
//
void Cmd_Score_f(gentity_t *ent);
void StopFollowing(gentity_t *ent);
void BroadcastTeamChange(gclient_t *client, int oldTeam);
void SetTeam(gentity_t *ent, char *s);
void Cmd_FollowCycle_f(gentity_t *ent, int dir);

//
// g_items.c
//

//
// g_utils.c
//

float G_Random(void);
float G_Random(float n);
float G_CRandom(void);
float G_CRandom(float n);

int        G_ModelIndex(const char *name);
int        G_SoundIndex(const char *name);
void       G_TeamCommand(teamtype_t team, char *cmd);
void       G_KillBox(gentity_t *ent);
qboolean   M_CheckBottom(Entity *ent);
gentity_t *G_PickTarget(char *targetname);
void       G_UseTargets(gentity_t *ent, gentity_t *activator);
void       G_SetMovedir(vec3_t angles, vec3_t movedir);

void       G_InitGentity(gentity_t *e);
gentity_t *G_Spawn(void);
gentity_t *G_TempEntity(vec3_t origin, int event);
void       G_Sound(gentity_t *ent, int channel, int soundIndex);
void       G_FreeEntity(gentity_t *e);
qboolean   G_entitiesFree(void);

Entity *G_NextEntity(Entity *ent);

void G_CalcBoundsOfMove(Vector& start, Vector& end, Vector& mins, Vector& maxs, Vector *minbounds, Vector *maxbounds);

float PlayersRangeFromSpot(Entity *ent);

Entity *findradius(Entity *startent, Vector org, float rad);
Entity *findclientsinradius(Entity *startent, Vector org, float rad);

void G_TouchTriggers(gentity_t *ent);
void G_TouchSolids(gentity_t *ent);

void G_CenterPrintToAllClients(const char *pszString);
void G_PrintToAllClients(const char *pszString, int iType = 1);
void G_PrintDeathMessage(
    const char *s1, const char *s2, const char *attackerName, const char *victimName, Player *victim, const char *type
);
void G_PrintDeathMessage_Old(const char *pszString);
void G_WarnPlayer(class Player *player, const char *format, ...);

int G_WeaponIndex(const char *name);

class SkelMat4;

orientation_t G_TIKI_Orientation(gentity_t *edict, int num);
SkelMat4     *G_TIKI_Transform(gentity_t *edict, int num);
qboolean      G_TIKI_IsOnGround(gentity_t *edict, int num, float threshold);
qboolean      G_Command_ProcessFile(const char *filename, qboolean quiet);

char *G_TimeString(float fTime);
char *CanonicalTikiName(const char *szInName);

float *tv(float x, float y, float z);
char  *vtos(const vec3_t v);

void        G_SetOrigin(gentity_t *ent, vec3_t origin);
void        AddRemap(const char *oldShader, const char *newShader, float timeOffset);
const char *BuildShaderStateConfig(void);

void G_DebugTargets(Entity *e, const str& from);
void G_DebugDamage(float damage, Entity *victim, Entity *attacker, Entity *inflictor);
void G_DebugString(Vector pos, float scale, float r, float g, float b, const char *pszText, ...);

qboolean KillBox(Entity *ent);

void        ChangeMusic(const char *current, const char *fallback, qboolean force);
void        ChangeMusicVolume(float volume, float fade_time);
void        RestoreMusicVolume(float fade_time);
void        ChangeSoundtrack(const char *soundtrack);
void        RestoreSoundtrack(void);
int         G_AIEventTypeFromString(const char *pszType);
const char *G_AIEventStringFromType(int iType);
float       G_AIEventRadius(int iType);
void        G_BroadcastAIEvent(Entity *originator, Vector origin, char *pszType);
void        G_BroadcastAIEvent(Entity *originator, Vector origin, int iType, float radius);

void CloneEntity(Entity *dest, Entity *src);
void G_ArchiveEdict(Archiver& arc, gentity_t *edict);

const char *G_LocationNumToDispString(int iLocation);
str         G_WeaponClassNumToName(int num);
int         G_WeaponClassNameToNum(str name);

//Checks for the intermission to exit.
void G_CheckIntermissionExit(void);
void G_CheckStartRules(void);
void G_CheckExitRules(void);

// Update the time limit
void G_UpdateMatchEndTime(void);

void G_MoveClientToIntermission(Entity *ent);
void G_DisplayScores(Entity *ent);
void G_HideScores(Entity *ent);
void G_DisplayScoresToAllClients(void);
void G_HideScoresToAllClients(void);

//
// caching commands
//
void G_ProcessCacheInitCommands(dtiki_t *tiki);

void G_SetTrajectory(gentity_t *ent, vec3_t org);

//
// g_combat.c
//
qboolean CanDamage(gentity_t *targ, vec3_t origin);
void     G_Damage(
        gentity_t *targ,
        gentity_t *inflictor,
        gentity_t *attacker,
        vec3_t     dir,
        vec3_t     point,
        int        damage,
        int        dflags,
        int        mod
    );
qboolean G_RadiusDamage(vec3_t origin, gentity_t *attacker, float damage, float radius, gentity_t *ignore, int mod);
int      G_InvulnerabilityEffect(gentity_t *targ, vec3_t dir, vec3_t point, vec3_t impactpoint, vec3_t bouncedir);
void     body_die(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath);
void     TossClientItems(gentity_t *self);
#ifdef MISSIONPACK
void TossClientPersistantPowerups(gentity_t *self);
#endif
void TossClientCubes(gentity_t *self);

// damage flags
#define DAMAGE_NONE          0
#define DAMAGE_RADIUS        0x00000001 // damage was indirect
#define DAMAGE_NO_ARMOR      0x00000002 // armour does not protect from this damage
#define DAMAGE_ENERGY        0x00000004 // damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK  0x00000008 // do not affect velocity, just view angles
#define DAMAGE_BULLET        0x00000010 // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION 0x00000020 // armor, shields, invulnerability, and godmode have no effect
#define DAMAGE_NO_SKILL      0x00000040 // damage is not affected by skill level

//
// g_missile.c
//
void G_RunMissile(gentity_t *ent);

gentity_t *fire_blaster(gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_plasma(gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_grenade(gentity_t *self, vec3_t start, vec3_t aimdir);
gentity_t *fire_rocket(gentity_t *self, vec3_t start, vec3_t dir);
gentity_t *fire_bfg(gentity_t *self, vec3_t start, vec3_t dir);
gentity_t *fire_grapple(gentity_t *self, vec3_t start, vec3_t dir);
#ifdef MISSIONPACK
gentity_t *fire_nail(gentity_t *self, vec3_t start, vec3_t forward, vec3_t right, vec3_t up);
gentity_t *fire_prox(gentity_t *self, vec3_t start, vec3_t aimdir);
#endif

//
// g_mover.c
//
void G_RunMover(gentity_t *ent);
void Touch_DoorTrigger(gentity_t *ent, gentity_t *other, trace_t *trace);
void Use_BinaryMover(gentity_t *ent, gentity_t *other, gentity_t *activator);

//
// g_trigger.c
//
void trigger_teleporter_touch(gentity_t *self, gentity_t *other, trace_t *trace);

//
// g_misc.c
//
void TeleportPlayer(gentity_t *player, vec3_t origin, vec3_t angles);
#ifdef MISSIONPACK
void DropPortalSource(gentity_t *ent);
void DropPortalDestination(gentity_t *ent);
#endif

//
// g_weapon.c
//
qboolean LogAccuracyHit(gentity_t *target, gentity_t *attacker);
void     SnapVectorTowards(vec3_t v, vec3_t to);
qboolean CheckGauntletAttack(gentity_t *ent);
void     Weapon_HookFree(gentity_t *ent);
void     Weapon_HookThink(gentity_t *ent);
void     CalcMuzzlePoint(gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint);

//
// g_client.c
//
gentity_t         *G_Find(gentity_t *from, int fieldofs, const char *match);
teamtype_t         TeamCount(int ignoreClientNum, int team);
int                TeamLeader(int team);
teamtype_t         PickTeam(int ignoreClientNum);
void               SetClientViewAngle(gentity_t *ent, vec3_t angle);
class PlayerStart *SelectSpawnPoint(Player *player);
void               CopyToBodyQue(gentity_t *ent);
void               respawn(gentity_t *ent);
void               BeginIntermission(void);
void               InitClientPersistant(gclient_t *client);
void               InitClientResp(gclient_t *client);
void               InitBodyQue(void);
void               ClientSpawn(gentity_t *ent);
void               player_die(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
void               AddScore(gentity_t *ent, vec3_t origin, int score);
void               CalculateRanks(void);

//
// g_svcmds.c
//
qboolean G_ConsoleCommand(void);
void     G_ProcessIPBans(void);
qboolean G_FilterPacket(char *from);

//
// g_weapon.c
//
void FireWeapon(gentity_t *ent);
#ifdef MISSIONPACK
void G_StartKamikaze(gentity_t *ent);
#endif

//
// p_hud.c
//
void MoveClientToIntermission(gentity_t *client);
void G_SetStats(gentity_t *ent);
void DeathmatchScoreboardMessage(gentity_t *client);

//
// g_cmds.c
//

//
// g_pweapon.c
//

//
// g_main.c
//
void       FindIntermissionPoint(void);
void       SetLeader(int team, int client);
void       CheckTeamLeader(int team);
void       G_RunThink(gentity_t *ent);
void QDECL G_LogPrintf(const char *fmt, ...);
void       SendScoreboardMessageToAllClients(void);
void QDECL G_Printf(const char *fmt, ...);
void QDECL G_Error(const char *fmt, ...);
void QDECL G_Error(errorParm_t type, const char *fmt, ...);

//
// g_client.c
//
void        G_BotConnect(int clientNum);
const char *G_ClientConnect(int clientNum, qboolean firstTime);
void        G_ClientUserinfoChanged(gentity_t *ent, const char *userinfo);
void        G_ClientDisconnect(gentity_t *ent);
void        G_ClientBegin(gentity_t *ent, usercmd_t *cmd);
void        G_SetClientConfigString(gentity_t *ent);
void        G_ClientCommand(gentity_t *ent);

//
// g_active.c
//
void G_ClientThink(gentity_t *ent, usercmd_t *cmd, usereyes_t *eyeinfo);
void ClientEndFrame(gentity_t *ent);
void G_ClientEndServerFrames(void);
void G_RunClient(gentity_t *ent);

//
// g_team.c
//
qboolean OnSameTeam(gentity_t *ent1, gentity_t *ent2);
void     Team_CheckDroppedItem(gentity_t *dropped);
qboolean CheckObeliskAttack(gentity_t *obelisk, gentity_t *attacker);

//
// g_mem.c
//
void *G_Alloc(int size);
void  G_InitMemory(void);
void  Svcmd_GameMem_f(void);

//
// g_session.c
//
void G_ReadSessionData(gclient_t *client);
void G_InitSessionData(gclient_t *client, char *userinfo);

void G_InitWorldSession(void);
void G_WriteSessionData(void);

//
// g_arenas.c
//
void UpdateTournamentInfo(void);
void SpawnModelsOnVictoryPads(void);
void Svcmd_AbortPodium_f(void);

//
// g_bot.c
//
void G_BotBegin(gentity_t *ent);
void G_BotThink(gentity_t *ent, int msec);

typedef struct mmove_s {
    vec3_t origin;
    vec3_t velocity;

    qboolean walking;
    qboolean groundPlane;
    vec3_t   groundPlaneNormal;

    float frametime;

    float desired_speed;
    int   tracemask;
    float desired_dir[2];

    int entityNum;

    vec3_t mins;
    vec3_t maxs;

    int numtouch;
    int touchents[MAXTOUCH];

    qboolean hit_obstacle;
    vec3_t   hit_origin;
    int      hit_temp_obstacle;

    vec3_t obstacle_normal;
} mmove_t;

// g_mmove.cpp
void MmoveSingle(mmove_t *mmove);

// g_vmove.cpp
void VmoveSingle(struct vmove_s *mmove);

// ai_main.c
#define MAX_FILEPATH 144

//bot settings
typedef struct bot_settings_s {
    char  characterfile[MAX_FILEPATH];
    float skill;
    char  team[MAX_FILEPATH];
} bot_settings_t;

int  BotAISetup(int restart);
int  BotAIShutdown();
int  BotAILoadMap(int restart);
int  BotAISetupClient(int client, struct bot_settings_s *settings, qboolean restart);
int  BotAIShutdownClient(int client, qboolean restart);
int  BotAIStartFrame(int time);
void BotTestAAS(vec3_t origin);

extern gentity_t *g_entities;
#define FOFS(x) ((size_t) & (((gentity_t *)0)->x))

#include "g_utils.h"

#endif /* g_local.h */
