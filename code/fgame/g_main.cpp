/*
===========================================================================
Copyright (C) 2024 the OpenMoHAA team

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

//

#include "g_local.h"
#include "g_spawn.h"
#include "g_phys.h"
#include "debuglines.h"
#include "entity.h"
#include "gamecmds.h"
#include "dm_manager.h"
#include "player.h"
#include "scriptmaster.h"
#include "scriptexception.h"
#include "lightstyleclass.h"
#include "lodthing.h"
#include "viewthing.h"
#include "smokesprite.h"
#include "playerbot.h"
#include "g_bot.h"
#include "navigation_recast_load.h"

#include "../corepp/tiki.h"

#ifdef WIN32
#    include <intrin.h>
#endif

#define SAVEGAME_VERSION   80
#define PERSISTANT_VERSION 2

static char G_ErrorMessage[4096];
profGame_t  G_profStruct;

qboolean  LoadingSavegame = false;
qboolean  LoadingServer   = false;
Archiver *currentArc      = NULL;

game_export_t globals;
game_import_t gi;

gentity_t active_edicts;
gentity_t free_edicts;

int sv_numtraces   = 0;
int sv_numpmtraces = 0;

int           g_protocol    = 0;
target_game_e g_target_game = target_game_e::TG_INVALID;
gentity_t    *g_entities;
qboolean      g_iInThinks     = 0;
qboolean      g_bBeforeThinks = qfalse;
static float  g_fMsecPerClock = 0;

usercmd_t  *current_ucmd;
usereyes_t *current_eyeinfo;
Player     *g_pPlayer;

gclient_t g_clients[MAX_CLIENTS];

void (*SV_Error)(int type, const char *fmt, ...);
void *(*SV_Malloc)(int size);
void (*SV_Free)(void *ptr);

qboolean LevelArchiveValid(Archiver& arc);
void     ClosePlayerLogFile(void);

/*
===============
G_Printf
===============
*/
void QDECL G_Printf(const char *fmt, ...)
{
    va_list argptr;
    char    text[1024];

    va_start(argptr, fmt);
    Q_vsnprintf(text, sizeof(text), fmt, argptr);
    va_end(argptr);

    gi.Printf(text);
}

/*
===============
G_Error
===============
*/
void QDECL G_Error(const char *fmt, ...)
{
    va_list argptr;
    char    text[1024];

    va_start(argptr, fmt);
    Q_vsnprintf(text, sizeof(text), fmt, argptr);
    va_end(argptr);

    gi.Error(ERR_DROP, text);
}

/*
===============
G_Error
===============
*/
void QDECL G_Error(int type, const char *fmt, ...)
{
    va_list argptr;
    char    text[1024];

    va_start(argptr, fmt);
    Q_vsnprintf(text, sizeof(text), fmt, argptr);
    va_end(argptr);

    // need to manually crash otherwise visual studio fuck up with the stack pointer...
    //*( int * )0 = 0;

    assert(!text);
}

/*
===============
G_ExitWithError

Calls the server's error function with the last error that occurred.
Should only be called after an exception.
===============
*/
void G_ExitWithError(const char *error)
{
    //ServerError( ERR_DROP, error );

    Q_strncpyz(G_ErrorMessage, error, sizeof(G_ErrorMessage));

    globals.errorMessage = G_ErrorMessage;
}

void G_RemapTeamShaders(void)
{
#ifdef MISSIONPACK
    char  string[1024];
    float f = level.time * 0.001;
    Com_sprintf(string, sizeof(string), "team_icon/%s_red", g_redteam.string);
    AddRemap("textures/ctf2/redteam01", string, f);
    AddRemap("textures/ctf2/redteam02", string, f);
    Com_sprintf(string, sizeof(string), "team_icon/%s_blue", g_blueteam.string);
    AddRemap("textures/ctf2/blueteam01", string, f);
    AddRemap("textures/ctf2/blueteam02", string, f);
    gi.setConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
#endif
}

void G_SetFogInfo(int cull, float distance, vec3_t farplane_color)
{
    // cg.farplane_cull cg.farplane_distance cg.farplane_color[3]
    //	gi.SetConfigstring(CS_FOGINFO,va("%i %f %f %f %f",0,4096.f,1.f,1.f,1.f));
}

void G_AllocGameData(void)
{
    int i;

    // de-allocate from previous level
    G_DeAllocGameData();

    // Initialize debug lines
    G_AllocDebugLines();

    // Initialize debug strings
    G_AllocDebugStrings();

    // initialize all entities for this game
    game.maxentities = maxentities->integer;

    g_entities = (gentity_t *)gi.Malloc(game.maxentities * sizeof(g_entities[0]));

    // clear out the entities
    memset(g_entities, 0, game.maxentities * sizeof(g_entities[0]));
    globals.gentities    = g_entities;
    globals.max_entities = game.maxentities;

    // Add all the edicts to the free list
    LL_Reset(&free_edicts, next, prev);
    LL_Reset(&active_edicts, next, prev);

    for (i = 0; i < game.maxentities; i++) {
        LL_Add(&free_edicts, &g_entities[i], next, prev);
    }

    // initialize all clients for this game
    game.clients = (gclient_t *)gi.Malloc(game.maxclients * sizeof(game.clients[0]));
    memset(game.clients, 0, game.maxclients * sizeof(game.clients[0]));

    for (i = 0; i < game.maxclients; i++) {
        // set client fields on player ents
        g_entities[i].client = game.clients + i;

        G_InitClientPersistant(&game.clients[i]);
    }

    globals.num_entities = game.maxclients;

    // Tell the server about our data
    gi.LocateGameData(
        g_entities, globals.num_entities, sizeof(gentity_t), &game.clients[0].ps, sizeof(game.clients[0])
    );
}

void G_DeAllocGameData(void)
{
    // Free up debug lines
    G_DeAllocDebugLines();

    // Added in OPM
    //  Free up debug strings
    G_DeAllocDebugStrings();

    // free up the entities
    if (g_entities) {
        gi.Free(g_entities);
        g_entities = NULL;
    }

    // free up the clients
    if (game.clients) {
        gi.Free(game.clients);
        game.clients = NULL;
    }
}

/*
============
G_InitGame

============
*/
void G_InitGame(int levelTime, int randomSeed)
{
    G_Printf("==== InitGame ====\n");
    G_Printf("gamename: %s\n", GAMEVERSION);
    G_Printf("gamedate: %s\n", __DATE__);

    g_protocol    = gi.Cvar_Get("com_protocol", "", 0)->integer;
    g_target_game = (target_game_e)gi.Cvar_Get("com_target_game", "0", 0)->integer;

    srand(randomSeed);

    CVAR_Init();

    game.Vars()->ClearList();

    // set some level globals
    level.svsStartTime = levelTime;

    G_InitConsoleCommands();

    Director.Reset();
    Actor::Init();
    G_BotInit();

    sv_numtraces   = 0;
    sv_numpmtraces = 0;

    if (developer->integer && g_gametype->integer == GT_SINGLE_PLAYER) {
        Viewmodel.Init();
        LODModel.Init();
    }

    game.maxentities = maxentities->integer;
    if (game.maxclients * 8 > maxentities->integer) {
        game.maxentities = game.maxclients * 8;
    }

    game.maxclients = maxclients->integer;

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        game.maxclients += sv_maxbots->integer;
    }

    L_InitEvents();

    G_AllocGameData();

    if (g_target_game < TG_MOHTA) {
        // Added in OPM
        //  This frees alias list to avoid filling up memory
        gi.GlobalAlias_Clear();
    }
}

/*
============
G_SpawnEntities

============
*/
void G_SpawnEntities(char *entities, int svsTime)
{
    level.SpawnEntities(entities, svsTime);
}

/*
===============
G_UseLargeLightmap

Added in 2.0
===============
*/
qboolean G_UseLargeLightmap(const char* mapName) {
	char buffer[MAX_QPATH];

	Com_sprintf(buffer, sizeof(buffer), "maps/%s_sml.bsp", mapName);

	if (gi.FS_ReadFile(buffer, NULL, qtrue) == -1) {
		return qtrue;
	}

	return gi.Cvar_Get("r_largemap", "0", 0)->integer;
}

/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame()
{
    gi.Printf("==== ShutdownGame ====\n");

    // write all the client session data so we can get it back
    G_WriteSessionData();

    ClosePlayerLogFile();

    level.CleanUp();

    L_ShutdownEvents();

    G_DeAllocGameData();
}

//===================================================================

void QDECL Com_Error(int level, const char *error, ...)
{
    va_list argptr;
    char    text[1024];

    va_start(argptr, error);
    Q_vsnprintf(text, sizeof(text), error, argptr);
    va_end(argptr);

    G_Error("%s", text);
}

void QDECL Com_Printf(const char *msg, ...)
{
    va_list argptr;
    char    text[1024];

    va_start(argptr, msg);
    Q_vsnprintf(text, sizeof(text), msg, argptr);
    va_end(argptr);

    gi.DPrintf("%s", text);
}

/*
================
G_Precache

Calls precache scripts
================
*/
void G_Precache(void)
{
    level.Precache();
}

/*
================
G_Precache

Called when server finished initializating
================
*/
void G_ServerSpawned(void)
{
    try {
        G_BotPostInit();

        level.ServerSpawned();
    } catch (const ScriptException& e) {
        G_ExitWithError(e.string.c_str());
    }
}

/*
================
G_AddGEntity


================
*/
void G_AddGEntity(gentity_t *edict, qboolean showentnums)
{
    unsigned long long start, end;
    Entity            *ent = edict->entity;

    if (g_timeents->integer) {
        start = clock();
        G_RunEntity(ent);
        end = clock();

        gi.DebugPrintf(
            "%d: <%s> '%s'(%d) : %d clocks, %.1f msec\n",
            level.framenum,
            ent->getClassname(),
            ent->targetname.c_str(),
            end - start,
            g_fMsecPerClock
        );
    } else {
        G_RunEntity(ent);
    }

    // remove the entity in case of invalid server flags
    if ((edict->r.svFlags & SVF_SENDONCE) && (edict->r.svFlags & SVF_SENT)) {
        ent->PostEvent(EV_Remove, 0);
    }

    if (showentnums) {
        G_DrawDebugNumber(ent->origin + Vector(0, 0, ent->maxs.z), ent->entnum, 2.0f, 1.0f, 1.0f, 0.0f);
    }

    if (g_entinfo->integer
        && (g_pPlayer && (edict->r.lastNetTime >= level.inttime - 200 || ent->IsSubclassOfPlayer()))) {
        float fDist = (g_pPlayer->centroid - g_pPlayer->EyePosition()).length();

        if (fDist != 0.0f) {
            float fDot = _DotProduct(g_vEyeDir, (g_pPlayer->centroid - g_pPlayer->EyePosition()));
            ent->ShowInfo(0, fDist);
        }
    }
}

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame(int levelTime, int frameTime)
{
    gentity_t         *edict;
    int                num;
    qboolean           showentnums;
    unsigned long long start;
    unsigned long long end;
    static int         processed[MAX_GENTITIES] = {0};
    static int         processedFrameID         = 0;

    try {
        g_iInThinks = 0;

        if (g_showmem->integer) {
            DisplayMemoryUsage();
        }

        // exit intermissions
        if (level.exitintermission) {
            if (level.nextmap != level.current_map) {
                G_ExitLevel();
            } else {
                G_RestartLevelWithDelay(0.1f);
                level.nextmap          = "";
                level.intermissiontime = 0;
                level.exitintermission = qfalse;
            }

            return;
        }

        level.setFrametime(frameTime);
        level.setTime(levelTime);

        if (level.intermissiontime || level.died_already) {
            L_ProcessPendingEvents();
            G_ClientDoBlends();

            if (g_gametype->integer != GT_SINGLE_PLAYER && g_maxintermission->value != 0.0f) {
                if (level.time - level.intermissiontime > g_maxintermission->value) {
                    level.exitintermission = true;
                }
            }

            return;
        }

        if (g_scripttrace->integer) {
            gi.DPrintf2("====SERVER FRAME==========================================================================\n");
        }

        g_bBeforeThinks = true;
        Director.AllowPause(false);

        // Process most of the events before the physics are run
        // so that we can affect the physics immediately
        L_ProcessPendingEvents();

        Director.AllowPause(true);
        Director.Pause();
        Director.SetTime(level.inttime);

        //
        // treat each object in turn
        //
        for (edict = active_edicts.next; edict != &active_edicts; edict = edict->next) {
            assert(edict);
            assert(edict->inuse);
            assert(edict->entity);

            Actor *actor = static_cast<Actor *>(edict->entity);
            if (actor->IsSubclassOfActor()) {
                actor->m_bUpdateAnimDoneFlags = false;
                if (actor->m_bAnimating) {
                    actor->PreAnimate();
                }
            }
        }

        g_iInThinks++;
        Director.Unpause();
        g_iInThinks--;

        // Process any pending events that got posted during the script code
        L_ProcessPendingEvents();

        path_checksthisframe = 0;

        // Reset debug lines
        G_InitDebugLines();
        G_InitDebugStrings();

        PathManager.ShowNodes();

        G_Navigation_Frame();

        showentnums = (sv_showentnums->integer && (g_gametype->integer == GT_SINGLE_PLAYER || sv_cheats->integer));

        g_iInThinks++;

        G_UpdateSmokeSprites();
        level.UpdateBadPlaces();

        processedFrameID++;

        if (g_entinfo->integer) {
            g_pPlayer = (Player *)G_GetEntity(0);

            if (!g_pPlayer->IsSubclassOfPlayer()) {
                g_pPlayer = NULL;
            } else {
                Vector vAngles = g_pPlayer->GetViewAngles();
                vAngles.AngleVectorsLeft(&g_vEyeDir);
            }
        }

        if (g_timeents->integer) {
            g_fMsecPerClock = 1.0f / gi.Cvar_Get("CPS", "1", 0)->value;
            start           = clock();
        }

        G_BotFrame();

        for (edict = active_edicts.next; edict != &active_edicts; edict = edict->next) {
            for (num = edict->s.parent; num != ENTITYNUM_NONE; num = g_entities[num].s.parent) {
                if (processed[num] == processedFrameID) {
                    break;
                }

                processed[num] = processedFrameID;
                G_AddGEntity(&g_entities[num], showentnums);
            }

            if (processed[edict - g_entities] != processedFrameID) {
                processed[edict - g_entities] = processedFrameID;
                G_AddGEntity(edict, showentnums);
            }
        }

        if (g_timeents->integer) {
            gi.cvar_set("g_timeents", va("%d", g_timeents->integer - 1));
            end = clock();

            gi.DebugPrintf(
                "\n%i total: %d (%.1f)\n-----------------------\n",
                level.framenum,
                end - start,
                static_cast<float>(end - start) * g_fMsecPerClock
            );
        }

        g_iInThinks--;
        g_bBeforeThinks = qfalse;

        // Process any pending events that got posted during the physics code.
        L_ProcessPendingEvents();
        level.DoEarthquakes();

        // build the playerstate_t structures for all players
        G_ClientEndServerFrames();

        level.Unregister(STRING_POSTTHINK);

        // Process any pending events that got posted during the script code
        L_ProcessPendingEvents();

        // show how many traces the game code is doing
        if (sv_traceinfo->integer) {
            if (sv_traceinfo->integer == 3) {
                if (sv_drawtrace->integer <= 1) {
                    gi.DebugPrintf("%0.2f : Total traces %3d\n", level.time, sv_numtraces);
                } else {
                    gi.DebugPrintf(
                        "%0.2f : Total traces %3d    pmove traces %3d\n", level.time, sv_numtraces, sv_numpmtraces
                    );
                }
            } else {
                if (sv_drawtrace->integer <= 1) {
                    gi.DebugPrintf("%0.2f : Total traces %3d\n", level.time, sv_numtraces);
                } else {
                    gi.DebugPrintf(
                        "%0.2f : Total traces %3d    pmove traces %3d\n", level.time, sv_numtraces, sv_numpmtraces
                    );
                }
            }
        }

        // reset out count of the number of game traces
        sv_numtraces   = 0;
        sv_numpmtraces = 0;

        level.framenum++;

        if (developer->integer) {
            G_ClientDrawBoundingBoxes();
            G_ClientDrawTags();
        }

        G_UpdateMatchEndTime();
        G_CheckExitRules();
        G_CheckStartRules();

        gi.setConfigstring(CS_WARMUP, va("%.0f", dmManager.GetMatchStartTime()));

        if (g_gametype->integer != GT_SINGLE_PLAYER) {
            level.CheckVote();
        }

        if (g_animdump->integer) {
            for (edict = active_edicts.next; edict != &active_edicts; edict = edict->next) {
                Animate *anim = (Animate *)edict->entity;

                if (anim->IsSubclassOfAnimate()) {
                    anim->DumpAnimInfo();
                }
            }
        }

        if (g_shownpc->integer) {
            if (g_shownpc->integer > 1) {
                g_shownpc->integer--;
            } else {
                int numActiveAllies = 0;
                int numActiveAxis   = 0;
                int numAllies       = 0;
                int numAxis         = 0;

                for (edict = active_edicts.next; edict != &active_edicts; edict = edict->next) {
                    Actor *actor;

                    if (edict->entity->IsSubclassOfActor()) {
                        actor = static_cast<Actor *>(edict->entity);

                        if (actor->health > 0) {
                            if (actor->m_Team == TEAM_AMERICAN) {
                                numAllies++;
                                if (actor->m_bDoAI) {
                                    numActiveAllies++;
                                }
                            } else {
                                numAxis++;
                                if (actor->m_bDoAI) {
                                    numActiveAxis++;
                                }
                            }
                        }
                    }
                }

                gi.locationprintf(
                    &g_entities[0],
                    94,
                    28,
                    va("NPCS: Allies %d(%d) Axis %d(%d)", numActiveAllies, numAllies, numActiveAxis, numAxis)
                );
                g_shownpc->integer = 60;
            }
        }

        if (level.Spawned()) {
            //
            // Added in OPM
            //
            // Add or delete bots that were added using addbot/removebot
            G_SpawnBots();
        }
    }

    catch (const char *error) {
        G_ExitWithError(error);
    }
}

/*
=================
G_ClientDrawBoundingBoxes
=================
*/
void G_ClientDrawBoundingBoxes(void)
{
    gentity_t *edict;
    Entity    *ent;
    Vector     eye;

    // don't show bboxes during deathmatch
    if ((!sv_showbboxes->integer) || (g_gametype->integer != GT_SINGLE_PLAYER && !sv_cheats->integer)) {
        return;
    }

    if (sv_showbboxes->integer) {
        edict = g_entities;
        ent   = edict->entity;
        if (ent) {
            eye = ent->origin;
            ent = findradius(NULL, eye, 1000);
            while (ent) {
                ent->DrawBoundingBox(sv_showbboxes->integer);
                ent = findradius(ent, eye, 1000);
            }
        }
    }
}

/*
=================
G_ClientDrawTags

Added in 2.0.

This draws tag position and orientation
for entities in a radius of 1000 units.
=================
*/
void G_ClientDrawTags(void)
{
    Entity       *player;
    Entity       *ent;
    const char   *tagName;
    Vector        origin;
    orientation_t ori;
    int           numTags;
    int           i;

    if (!sv_showtags->string || !sv_showtags->string[0]) {
        return;
    }

    if (sv_showtags->string[0] != '*' && !sv_showtags->string[1] && sv_showtags->integer == 0) {
        // The variable is defined but set to 0
        return;
    }

    if (g_gametype->integer != GT_SINGLE_PLAYER && !sv_cheats->integer) {
        return;
    }

    player = G_GetEntity(0);
    origin = player->origin;

    for (ent = findradius(NULL, origin, 1000); ent; ent = findradius(ent, origin, 1000)) {
        if (!ent->edict->tiki) {
            continue;
        }

        if (ent == player) {
            continue;
        }

        numTags = gi.TIKI_NumTags(ent->edict->tiki);
        for (i = 0; i < numTags; i++) {
            tagName = gi.Tag_NameForNum(ent->edict->tiki, i);

            if (Q_stricmp(sv_showtags->string, "*")) {
                //
                // If it's not a wildcard, check to see if
                // the tag name partially matches
                //
                if (Q_stricmpn(tagName, sv_showtags->string, strlen(sv_showtags->string))) {
                    continue;
                }
            }

            ent->GetTagPositionAndOrientation(i, &ori);

            G_DebugString(ori.origin + Vector(0, 0, 8), 1, 1, 1, 0, "%s", tagName);
            G_DebugCircle(ori.origin, 10, 1, 1, 1, 1, true);
            G_DebugLine(ori.origin, ori.origin + Vector(ori.axis[0]) * 32, 1, 0, 0, 1);
            G_DebugLine(ori.origin, ori.origin + Vector(ori.axis[1]) * 32, 0, 1, 0, 1);
            G_DebugLine(ori.origin, ori.origin + Vector(ori.axis[2]) * 32, 0, 0, 1, 1);
        }
    }
}

// Used to tell the server about the edict pose, such as the player pose
// so that G_Trace with tracedeep will set the location
void G_UpdatePoseInternal(gentity_t *edict)
{
    if (edict->s.number != ENTITYNUM_NONE) {
        if (level.skel_index[edict->s.number] == level.frame_skel_index) {
            // no need to update
            return;
        }

        level.skel_index[edict->s.number] = level.frame_skel_index;
    }

    gi.TIKI_SetPoseInternal(
        edict->tiki, edict->s.number, edict->s.frameInfo, edict->s.bone_tag, edict->s.bone_quat, edict->s.actionWeight
    );
}

orientation_t G_TIKI_Orientation(gentity_t *edict, int num)
{
    orientation_t orient;

    G_UpdatePoseInternal(edict);

    orient = gi.TIKI_OrientationInternal(edict->tiki, edict->s.number, num, edict->s.scale);

    return orient;
}

SkelMat4 *G_TIKI_Transform(gentity_t *edict, int num)
{
    G_UpdatePoseInternal(edict);
    return (SkelMat4 *)gi.TIKI_TransformInternal(edict->tiki, edict->s.number, num);
}

qboolean G_TIKI_IsOnGround(gentity_t *edict, int num, float threshold)
{
    G_UpdatePoseInternal(edict);
    return gi.TIKI_IsOnGroundInternal(edict->tiki, edict->s.number, num, threshold);
}

void G_PrepFrame(void) {}

void G_RegisterSoundsForFile(const char *name)
{
    int startTime;
    int endTime;

    Com_Printf("\n\n-----------PARSING '%s' (SERVER)------------\n", name);
    Com_Printf(
        "Any SetCurrentTiki errors means that tiki wasn't prefetched and tiki-specific sounds for it won't work. To "
        "fix prefetch the tiki. Ignore if you don't use that tiki on this level.\n"
    );

    startTime = gi.Milliseconds();
    G_Command_ProcessFile(name, qfalse);
    endTime = gi.Milliseconds();

    Com_Printf("Parse/Load time: %f seconds.\n", (float)(endTime - startTime) / 1000.0);
    Com_Printf("-------------PARSING '%s' DONE (SERVER)---------------\n\n", name);
}

int qsort_compare_strings(const void *s1, const void *s2)
{
    return Q_stricmp(*(const char **)s1, *(const char **)s2);
}

void G_RegisterSounds(void)
{
    char **fileList;
    int    numFiles;
    int    i;

    if (g_target_game >= TG_MOHTA) {
        gi.GlobalAlias_Clear();
    } else if (!dedicated->integer) {
        // In 1.11 and below, the entire ubersound folder is parsed
        // only in dedicated mode
        return;
    }

    fileList = gi.FS_ListFiles("ubersound/", "scr", qfalse, &numFiles);
    qsort(fileList, numFiles, sizeof(char *), &qsort_compare_strings);

    for (i = 0; i < numFiles; i++) {
        // Added in 2.0
        //  Since this version, all files in the ubersound folder
        //  are parsed
        G_RegisterSoundsForFile(va("ubersound/%s", fileList[i]));
    }

    gi.FS_FreeFileList(fileList);
}

void G_Restart(void)
{
    G_InitWorldSession();

    // Added in 2.0
    G_ResetSmokeSprites();

    //
    // Added in OPM
    //

    G_RestartBots();

    level.Restart();
}

void G_SetFrameNumber(int framenum)
{
    level.frame_skel_index = framenum;
}

void G_SetMap(const char *mapname)
{
    level.SetMap(mapname);
}

void G_SetTime(int svsStartTime, int svsTime)
{
    if (level.svsStartTime != svsTime) {
        gi.setConfigstring(CS_LEVEL_START_TIME, va("%i", svsTime));
    }

    level.svsStartTime = svsStartTime;
    level.setTime(svsTime);
}

/*
=================
G_LevelArchiveValid
=================
*/
qboolean G_LevelArchiveValid(const char *filename)
{
    try {
        qboolean ret;

        Archiver arc;

        if (!arc.Read(filename)) {
            return qfalse;
        }

        ret = LevelArchiveValid(arc);

        arc.Close();

        return ret;
    }

    catch (const char *error) {
        G_ExitWithError(error);
        return qfalse;
    }
}

void G_SoundCallback(int entNum, soundChannel_t channelNumber, const char *name)
{
    gentity_t *ent    = &g_entities[entNum];
    Entity    *entity = ent->entity;

    if (!entity) {
        ScriptError(
            "ERROR:  wait on playsound only works on entities that still exist when the sound is done playing."
        );
    }

    entity->CancelEventsOfType(EV_SoundDone);

    Event *ev = new Event(EV_SoundDone);
    ev->AddInteger(channelNumber);
    ev->AddString(name);
    entity->PostEvent(ev, level.frametime);
}

qboolean G_Command_ProcessFile(const char *filename, qboolean quiet)
{
    char       *buffer;
    const char *bufstart;
    const char *token;
    int         numTokens = 0;
    char        tempName[65];

    if (gi.FS_ReadFile(filename, (void **)&buffer, quiet) == -1) {
        return qfalse;
    }

    if (!quiet) {
        gi.DPrintf("G_Command_ProcessFile: %s\n", filename);
    }

    bufstart = buffer;

    Com_sprintf(tempName, sizeof(tempName), "m%s", filename);
    gi.LoadResource(tempName);

    while (1) {
        // grab each line as we go
        token = COM_ParseExt(&buffer, qtrue);
        if (!token[0]) {
            break;
        }

        if (!Q_stricmp(token, "end") || !Q_stricmp(token, "server")) {
            // skip the line
            while (1) {
                token = COM_ParseExt(&buffer, qfalse);
                if (!token[0]) {
                    break;
                }
            }
            continue;
        }

        // Create the event
        Event ev(token);

        // get the rest of the line
        while (1) {
            token = COM_ParseExt(&buffer, qfalse);
            if (!token[0]) {
                break;
            }

            ev.AddToken(token);
        }

        Director.ProcessEvent(ev);
    }

    gi.FS_FreeFile((void *)bufstart);

    Com_sprintf(tempName, sizeof(tempName), "o%s", filename);
    gi.LoadResource(tempName);

    return qtrue;
}

qboolean G_AllowPaused(void)
{
    return (!level.exitintermission) && (level.intermissiontime == 0.0f) && (!level.died_already);
}

void G_UpdateMatchEndTime(void)
{
    int endtime = 0;

    if (dmManager.GameHasRounds() && dmManager.GetRoundLimit()) {
        endtime = dmManager.GetMatchStartTime() * 1000.0f + (level.svsStartTime + 60000 * dmManager.GetRoundLimit());
    } else if (timelimit->integer) {
        endtime = level.svsStartTime + 60000 * timelimit->integer;
    }

    if (level.svsEndTime != endtime) {
        level.svsEndTime = endtime;
        gi.setConfigstring(CS_MATCHEND, va("%i", endtime));
    }
}

void G_ArchiveFloat(float *fl)
{
    currentArc->ArchiveFloat(fl);
}

void G_ArchiveInteger(int *i)
{
    currentArc->ArchiveInteger(i);
}

void G_ArchiveString(char *s)
{
    if (currentArc->Loading()) {
        str string;
        currentArc->ArchiveString(&string);
        strcpy(s, string.c_str());
    } else {
        str string = s;
        currentArc->ArchiveString(&string);
    }
}

void G_ArchiveSvsTime(int *pi)
{
    currentArc->ArchiveSvsTime(pi);
}

void G_ArchivePersistantData(Archiver& arc)
{
    gentity_t *ed;
    int        i;

    for (i = 0; i < game.maxclients; i++) {
        Entity *ent;

        ed = &g_entities[i];
        if (!ed->inuse || !ed->entity) {
            continue;
        }

        ent = ed->entity;
        if (!ent->IsSubclassOfPlayer()) {
            continue;
        }
        ((Player *)ent)->ArchivePersistantData(arc);
    }
}

void G_ArchivePersistant(const char *name, qboolean loading)
{
    int      version;
    Archiver arc;

    if (loading) {
        if (!arc.Read(name, qfalse)) {
            return;
        }

        arc.ArchiveInteger(&version);
        if (version < PERSISTANT_VERSION) {
            gi.Printf("Persistent data from an older version (%d) of MOHAA.\n", version);
            arc.Close();
            return;
        } else if (version > PERSISTANT_VERSION) {
            gi.DPrintf("Persistent data from newer version %d of MOHAA.\n", version);
            arc.Close();
            return;
        }

        if (game.Vars()->size()) {
            Com_Printf("^~^~^ game variables being cleared:\n");
            game.Vars()->Print(&Com_Printf);

            if (g_scriptcheck->integer) {
                Com_Error(ERR_DROP, "game variables being cleared");
            }
        }

        game.Vars()->ClearList();
    } else {
        arc.Create(name);

        version = PERSISTANT_VERSION;
        arc.ArchiveInteger(&version);
    }

    arc.ArchiveObject(game.Vars());
    G_ArchivePersistantData(arc);

    arc.Close();
}

qboolean G_ReadPersistant(const char *name)
{
    try {
        G_ArchivePersistant(name, qtrue);
    }

    catch (const char *error) {
        G_ExitWithError(error);
    }
    return qfalse;
}

/*
============
G_WritePersistant

This will be called whenever the game goes to a new level,

A single player death will automatically restore from the
last save position.
============
*/

void G_WritePersistant(const char *name)
{
    try {
        G_ArchivePersistant(name, qfalse);
    }

    catch (const char *error) {
        G_ExitWithError(error);
    }
}

void G_Cleanup(qboolean samemap)
{
    gi.Printf("==== CleanupGame ====\n");

    G_WriteSessionData();

    level.CleanUp(samemap, qtrue);
}

void ArchiveAliases(Archiver& arc)
{
    int                      i;
    byte                     another;
    AliasList_t             *alias_list;
    AliasListNode_t         *alias_node;
    str                      alias_name;
    str                      model_name;
    const char              *name;
    dtikianim_t             *modelanim;
    Container<dtikianim_t *> animlist;

    if (arc.Saving()) {
        for (i = 0; i < MAX_MODELS; i++) {
            name = gi.getConfigstring(CS_MODELS + i);
            if (name && *name && *name != '*') {
                const char *p = name;

                while (true) {
                    p = strchr(name, '|');
                    if (!p) {
                        break;
                    }
                    name = p + 1;
                }

                modelanim = gi.modeltikianim(name);
                if (modelanim && !animlist.IndexOfObject(modelanim)) {
                    animlist.AddObject(modelanim);

                    alias_list = (AliasList_t *)modelanim->alias_list;
                    if (alias_list) {
                        alias_node = alias_list->data_list;

                        if (alias_node) {
                            another = true;
                            arc.ArchiveByte(&another);

                            alias_name = name;
                            arc.ArchiveString(&alias_name);

                            for (; alias_node != NULL; alias_node = alias_node->next) {
                                another = true;
                                arc.ArchiveByte(&another);

                                alias_name = alias_node->alias_name;
                                arc.ArchiveString(&alias_name);
                            }

                            another = false;
                            arc.ArchiveByte(&another);
                        }
                    }
                }
            }
        }

        another = false;
        arc.ArchiveByte(&another);
    } else {
        arc.ArchiveByte(&another);

        while (another) {
            arc.ArchiveString(&model_name);

            modelanim = gi.modeltikianim(model_name.c_str());

            arc.ArchiveByte(&another);

            while (another) {
                // Read in aliases

                arc.ArchiveString(&alias_name);

                gi.Alias_UpdateDialog(modelanim, alias_name.c_str());

                arc.ArchiveByte(&another);
            }

            arc.ArchiveByte(&another);
        }
    }
}

/*
=================
LevelArchiveValid
=================
*/
qboolean LevelArchiveValid(Archiver& arc)
{
    int version;
    int savegame_version;

    // read the version number
    arc.ArchiveInteger(&version);
    arc.ArchiveInteger(&savegame_version);

    if (version < GAME_API_VERSION) {
        gi.Printf("Savegame from an older version (%d) of MOHAA.\n", version);
        return qfalse;
    } else if (version > GAME_API_VERSION) {
        gi.Printf("Savegame from version %d of MOHAA.\n", version);
        return qfalse;
    }

    if (savegame_version < SAVEGAME_VERSION) {
        gi.Printf("Savegame from an older version (%d) of MoHAA.\n", version);
        return qfalse;
    } else if (savegame_version > SAVEGAME_VERSION) {
        gi.Printf("Savegame from version %d of MoHAA.\n", version);
        return qfalse;
    }
    return qtrue;
}

/*
=================
G_ArchiveLevel

=================
*/
qboolean G_ArchiveLevel(
    const char *filename, byte **savedCgameState, size_t *savedCgameStateSize, qboolean autosave, qboolean loading
)
{
    try {
        int         i;
        int         num;
        Archiver    arc;
        gentity_t  *edict;
        char        szSaveName[MAX_STRING_TOKENS];
        const char *pszSaveName;
        cvar_t     *cvar;

        COM_StripExtension(filename, szSaveName, sizeof(szSaveName));
        pszSaveName = COM_SkipPath(szSaveName);

        gi.cvar_set("g_lastsave", pszSaveName);

        if (loading) {
            LoadingSavegame = true;

            arc.Read(filename);
            if (!LevelArchiveValid(arc)) {
                arc.Close();
                return qfalse;
            }

            // Read in the pending events.  These are read in first in case
            // later objects need to post events.
            L_UnarchiveEvents(arc);
        } else {
            int temp;

            arc.Create(filename);

            // write out the version number
            temp = GAME_API_VERSION;
            arc.ArchiveInteger(&temp);
            temp = SAVEGAME_VERSION;
            arc.ArchiveInteger(&temp);

            // Write out the pending events.  These are written first in case
            // later objects need to post events when reading the archive.
            L_ArchiveEvents(arc);
        }

        if (!arc.Saving()) {
            arc.ArchiveInteger(&num);
            *savedCgameStateSize = num;

            if (*savedCgameStateSize) {
                *savedCgameState = (byte *)gi.Malloc(*savedCgameStateSize);
            } else {
                *savedCgameState = NULL;
            }
        } else {
            num = (int)*savedCgameStateSize;
            arc.ArchiveInteger(&num);
        }
        arc.ArchiveRaw(*savedCgameState, *savedCgameStateSize);

        if (arc.Saving()) {
            str s;

            num = 0;
            for (cvar = gi.NextCvar(NULL); cvar != NULL; cvar = gi.NextCvar(cvar)) {
                if (cvar->flags & CVAR_SAVEGAME) {
                    num++;
                }
            }

            arc.ArchiveInteger(&num);
            for (cvar = gi.NextCvar(NULL); cvar != NULL; cvar = gi.NextCvar(cvar)) {
                if (cvar->flags & CVAR_SAVEGAME) {
                    s = cvar->name;
                    arc.ArchiveString(&s);

                    s = cvar->string;
                    arc.ArchiveString(&s);

                    arc.ArchiveBoolean(&cvar->modified);
                    arc.ArchiveInteger(&cvar->modificationCount);
                    arc.ArchiveFloat(&cvar->value);
                    arc.ArchiveInteger(&cvar->integer);
                }
            }
        } else {
            str sName, sValue;

            arc.ArchiveInteger(&num);
            for (int i = 0; i < num; i++) {
                arc.ArchiveString(&sName);
                arc.ArchiveString(&sValue);

                cvar = gi.cvar_set2(sName, sValue, qfalse);

                arc.ArchiveBoolean(&cvar->modified);
                arc.ArchiveInteger(&cvar->modificationCount);
                arc.ArchiveFloat(&cvar->value);
                arc.ArchiveInteger(&cvar->integer);
            }
        }

        // archive the game object
        arc.ArchiveObject(&game);

        // archive Level
        arc.ArchiveObject(&level);

        // archive camera paths
        arc.ArchiveObject(&CameraMan);

        // archive paths
        arc.ArchiveObject(&PathManager);

        // archive script controller
        arc.ArchiveObject(&Director);

        // archive lightstyles
        arc.ArchiveObject(&lightStyles);

        if (arc.Saving()) {
            // count the entities
            num = 0;
            for (edict = active_edicts.next; edict != &active_edicts; edict = edict->next) {
                if (edict == world->edict || !edict->entity || (edict->entity->flags & FL_DONTSAVE)) {
                    continue;
                }

                num++;
            }
        }

        // archive all the entities
        arc.ArchiveInteger(&globals.num_entities);
        arc.ArchiveInteger(&num);

        if (arc.Saving()) {
            // write out the world
            arc.ArchiveObject(world);

            for (edict = active_edicts.next; edict != &active_edicts; edict = edict->next) {
                if (edict == world->edict || !edict->entity || (edict->entity->flags & FL_DONTSAVE)) {
                    continue;
                }

                arc.ArchiveObject(edict->entity);
            }
        } else {
            // Tell the server about our data
            gi.LocateGameData(
                g_entities, globals.num_entities, sizeof(gentity_t), &game.clients[0].ps, sizeof(game.clients[0])
            );

            // read in the world
            arc.ReadObject();

            // load pathnodes
            PathSearch::LoadNodes();

            for (i = 0; i < num; i++) {
                arc.ReadObject();
            }
        }

        //
        // simple archived entities
        //
        if (!arc.Loading()) {
            num = level.m_SimpleArchivedEntities.NumObjects();
        }
        arc.ArchiveInteger(&num);

        if (arc.Saving()) {
            for (i = 1; i <= num; i++) {
                arc.ArchiveObject(level.m_SimpleArchivedEntities.ObjectAt(i));
            }
        } else {
            for (i = 1; i <= num; i++) {
                arc.ReadObject();
            }
        }

        ArchiveAliases(arc);

        // Added in 2.0
        G_ArchiveSmokeSprites(arc);

        currentArc = &arc;
        gi.ArchiveLevel(arc.Loading());
        currentArc = NULL;

        PathSearch::ArchiveDynamic(arc);

        if (arc.Loading()) {
            // Make sure all code that needs to setup the player after they have been loaded is run

            for (i = 0; i < game.maxclients; i++) {
                edict = &g_entities[i];

                if (edict->inuse && edict->entity) {
                    Player *player = (Player *)edict->entity;
                    player->Loaded();
                }
            }
        }

        if (arc.Loading()) {
            arc.Close();
            LoadingSavegame = false;
            gi.Printf(HUD_MESSAGE_YELLOW "%s\n", gi.LV_ConvertString("Game Loaded"));
        } else {
            arc.Close();
            gi.Printf(HUD_MESSAGE_YELLOW "%s\n", gi.LV_ConvertString("Game Saved"));
        }

        return qtrue;
    }

    catch (const char *error) {
        G_ExitWithError(error);
    }
    return qfalse;
}

/*
=================
G_WriteLevel

=================
*/
void G_WriteLevel(const char *filename, qboolean autosave, byte **savedCgameState, size_t *savedCgameStateSize)
{
    game.autosaved = autosave;
    G_ArchiveLevel(filename, savedCgameState, savedCgameStateSize, autosave, qfalse);
    game.autosaved = false;
}

/*
=================
G_ReadLevel

SpawnEntities will already have been called on the
level the same way it was when the level was saved.

That is necessary to get the baselines set up identically.

The server will have cleared all of the world links before
calling ReadLevel.

No clients are connected yet.
=================
*/
qboolean G_ReadLevel(const char *filename, byte **savedCgameState, size_t *savedCgameStateSize)
{
    qboolean status;

    status = G_ArchiveLevel(filename, savedCgameState, savedCgameStateSize, qfalse, qtrue);
    // if the level load failed make sure that these variables are not set
    if (!status) {
        LoadingSavegame = false;
        LoadingServer   = false;
    }
    return status;
}

/*
================
GetGameAPI

Gets game imports and returns game exports
================
*/
extern "C" game_export_t *GetGameAPI(game_import_t *import)
{
    gi = *import;

    globals.apiversion = GAME_API_VERSION;

    globals.AllowPaused       = G_AllowPaused;
    globals.ArchiveFloat      = G_ArchiveFloat;
    globals.ArchiveInteger    = G_ArchiveInteger;
    globals.ArchivePersistant = G_ArchivePersistant;
    globals.ArchiveString     = G_ArchiveString;
    globals.ArchiveSvsTime    = G_ArchiveSvsTime;

    globals.BotBegin = G_BotBegin;
    globals.BotThink = G_BotThink;

    globals.Cleanup = G_Cleanup;

    globals.ClientCommand = G_ClientCommand;
    globals.ClientConnect = G_ClientConnect;

    globals.ClientBegin = G_ClientBegin;
    globals.ClientThink = G_ClientThink;

    globals.ClientDisconnect = G_ClientDisconnect;
    globals.ClientThink      = G_ClientThink;

    globals.ClientUserinfoChanged = G_ClientUserinfoChanged;

    globals.ConsoleCommand = G_ConsoleCommand;

    globals.DebugCircle  = G_DebugCircle;
    globals.errorMessage = NULL;

    globals.gentities   = g_entities;
    globals.gentitySize = sizeof(g_entities[0]);

    globals.Init = G_InitGame;

    globals.LevelArchiveValid = G_LevelArchiveValid;

    globals.Precache      = G_Precache;
    globals.SpawnEntities = G_SpawnEntities;

    globals.PrepFrame = G_PrepFrame;

    globals.profStruct     = &G_profStruct;
    globals.ReadLevel      = G_ReadLevel;
    globals.WriteLevel     = G_WriteLevel;
    globals.RegisterSounds = G_RegisterSounds;
    globals.Restart        = G_Restart;

    globals.RunFrame = G_RunFrame;

    globals.ServerSpawned = G_ServerSpawned;

    globals.SetFrameNumber = G_SetFrameNumber;
    globals.SetMap         = G_SetMap;
    globals.SetTime        = G_SetTime;

    globals.Shutdown = G_ShutdownGame;

    globals.SoundCallback    = G_SoundCallback;
    globals.SpawnEntities    = G_SpawnEntities;
    globals.TIKI_Orientation = G_TIKI_Orientation;

    globals.GetNumSimulatedPlayers   = G_GetNumBots;
    globals.GetSimulatedPlayersSkill = G_GetBotSkill;

    return &globals;
}

/*
=================
G_ClientEndServerFrames
=================
*/
void G_ClientEndServerFrames(void)
{
    int        i;
    gentity_t *ent;

    // calc the player views now that all pushing
    // and damage has been added
    for (i = 0; i < game.maxclients; i++) {
        ent = g_entities + i;
        if (!ent->inuse || !ent->client || !ent->entity) {
            continue;
        }

        ent->entity->EndFrame();
    }
}

/*
===============
G_ClientDoBlends
===============
*/
void G_ClientDoBlends(void)
{
    gentity_t *edict;
    int        i;

    for (i = 0, edict = g_entities; i < game.maxclients; i++, edict++) {
        if (!edict->inuse || !edict->client || !edict->entity) {
            continue;
        }

        edict->entity->CalcBlend();
    }
}

/*
===============
FindIntermissionPoint
===============
*/
void FindIntermissionPoint(void)
{
    SimpleEntity *sent;
    SimpleEntity *starget;
    vec3_t        dir;

    sent = static_cast<SimpleEntity *>(G_FindClass(NULL, "info_player_intermission"));
    if (!sent) {
        level.m_intermission_origin = vec_zero;
        level.m_intermission_angle  = vec_zero;
        return;
    }

    level.m_intermission_origin = sent->origin;
    level.m_intermission_angle  = sent->angles;

    if (!sent->target.length()) {
        return;
    }

    starget = static_cast<SimpleEntity *>(G_FindTarget(NULL, sent->Target()));
    if (!starget) {
        return;
    }

    VectorSubtract(starget->origin, level.m_intermission_origin, dir);
    vectoangles(dir, level.m_intermission_angle);
}

/*
===============
G_MoveClientToIntermission
===============
*/
void G_MoveClientToIntermission(Entity *ent)
{
    G_DisplayScores(ent);
    ent->flags |= FL_IMMOBILE;
}

/*
===============
G_DisplayScores
===============
*/
void G_DisplayScores(Entity *ent)
{
    ent->client->ps.pm_flags |= PMF_INTERMISSION;
}

/*
===============
G_HideScores
===============
*/
void G_HideScores(Entity *ent)
{
    ent->client->ps.pm_flags &= ~PMF_INTERMISSION;
}

/*
===============
G_BeginIntermission2
===============
*/
void G_BeginIntermission2(void)
{
    gentity_t *client;
    Entity    *ent;
    int        i;

    if (level.intermissiontime) {
        return;
    }

    level.playerfrozen     = qtrue;
    level.intermissiontime = level.time;

    ent = (Entity *)G_FindClass(NULL, "info_player_intermission");

    G_FadeSound(4.0f);

    if (ent) {
        SetCamera(ent, 0.5f);
    } else {
        G_FadeOut(2.0f);
    }

    for (i = 0, client = g_entities; i < game.maxclients; i++, client++) {
        if (!client->inuse || !client->entity || !client->client) {
            continue;
        }

        G_MoveClientToIntermission(client->entity);
    }

    // Added in OPM
    level.EnterIntermission();
}

/*
===============
G_BeginIntermission
===============
*/
void G_BeginIntermission(const char *map_name, INTTYPE_e transtype, bool no_fade)
{
    Entity    *camera;
    Entity    *node;
    Event     *ev;
    gentity_t *client;
    int        i;

    if (level.intermissiontime || g_gametype->integer != GT_SINGLE_PLAYER) {
        return;
    }

    level.intermissiontime = level.time;
    level.intermissiontype = transtype;

    if (!no_fade) {
        G_FadeOut(2.0f);
    }

    G_FadeSound(4.0f);

    level.nextmap = map_name;

    camera = (Entity *)G_FindClass(NULL, "info_player_intermission");
    if (camera) {
        SetCamera(camera, 0.5f);

        ev = new Event(EV_Camera_Orbit);

        node = (Entity *)G_FindTarget(NULL, "endnode1");
        if (node && node->IsSubclassOfEntity()) {
            ev->AddEntity(node);
            camera->ProcessEvent(ev);
            camera->ProcessEvent(EV_Camera_Cut);
        }
    }

    for (i = 0, client = g_entities; i < game.maxclients; client++, i++) {
        if (!client->inuse || !client->entity) {
            continue;
        }

        client->entity->flags |= FL_IMMOBILE;
        client->entity->PostEvent(EV_Player_EnterIntermission, 3.0f);
    }

    // Added in OPM
    level.EnterIntermission();
}

/*
===============
G_ExitLevel
===============
*/
void G_ExitLevel(void)
{
    static const char *seps = " ,\n\r";
    char               command[256];
    int                j;
    gentity_t         *ent;

    // Close the player log file if necessary
    ClosePlayerLogFile();

    // Kill the sounds
    Com_sprintf(command, sizeof(command), "stopsound\n");
    gi.SendConsoleCommand(command);

    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        if (sv_nextmap->string && sv_nextmap->string[0]) {
            // The nextmap cvar was set (possibly by a vote - so go ahead and use it)
            level.nextmap = sv_nextmap->string;
            gi.cvar_set("nextmap", "");
        } else if (sv_maplist->string && sv_maplist->string[0]) {
            // Use the next map in the maplist
            char    *s, *t;
            qboolean bLevelSet = false;
            str      sFirst;

            s = strdup(sv_maplist->string);
            for (t = strtok(s, seps); t; t = strtok(NULL, seps)) {
                if (!Q_stricmp(t, level.mapname.c_str())) {
                    // It's in the list, go to the next one
                    t = strtok(NULL, seps);
                    if (t) {
                        level.nextmap = t;
                    } else {
                        if (sFirst.length()) {
                            // End of list, go to first one
                            level.nextmap = sFirst;
                        } else {
                            // There isn't a first one, same level
                            level.nextmap = level.mapname;
                        }
                    }

                    bLevelSet = true;
                    break;
                }

                if (!sFirst.length()) {
                    sFirst = t;
                }
            }

            free(s);

            if (!bLevelSet) {
                // No level set, use the first map
                level.nextmap = sFirst;
                bLevelSet     = true;
            }
        }

        // level.nextmap should be set now, but if it isn't use the same map
        if (!level.nextmap.length()) {
            level.nextmap = level.mapname;
        }

        if (level.nextmap == level.mapname) {
            // Stay on the same map if it's the same as the current map
            Com_sprintf(command, sizeof(command), "restart\n");
        }
        // Added in 2.15
        //  Execute a cvar command with `vstr`
        //else if (!Q_stricmpn(level.nextmap, "vstr", 4)) {
        // Fixed in OPM
        //  Match the exact vstr command
        else if (!Q_stricmpn(level.nextmap, "vstr ", 5)) {
            Q_strncpyz(command, level.nextmap, sizeof(command));
        } else {
            // Use the level.nextmap variable
            Com_sprintf(command, sizeof(command), "gamemap \"%s\"\n", level.nextmap.c_str());
        }

        gi.SendConsoleCommand(command);
    } else {
        Com_sprintf(command, sizeof(command), "gamemap \"%s\"\n", level.nextmap.c_str());
        gi.SendConsoleCommand(command);
    }

    // Tell all the clients that the level is done
    for (j = 0; j < game.maxclients; j++) {
        ent = &g_entities[j];
        if (!ent->inuse || !ent->entity) {
            continue;
        }

        ent->entity->ProcessEvent(EV_Player_EndLevel);
    }

    level.nextmap = "";

    level.exitintermission = 0;
    level.intermissiontime = 0;

    G_ClientEndServerFrames();
}

/*
===============
G_CheckIntermissionExit
===============
*/
void G_CheckIntermissionExit(void)
{
    if (!level.exitintermission && g_maxintermission->value > level.time - level.intermissiontime) {
        return;
    }

    if (level.nextmap != level.current_map) {
        G_ExitLevel();
    } else {
        G_RestartLevelWithDelay(0.1f);

        level.nextmap          = "";
        level.intermissiontime = 0;
        level.exitintermission = qfalse;
    }
}

/*
===============
G_ExitIntermission
===============
*/
void G_ExitIntermission(void)
{
    level.exitintermission = qtrue;
}

/*
===============
G_CheckStartRules
===============
*/
void G_CheckStartRules(void)
{
    if ((!dmManager.IsGameActive()) && (!dmManager.WaitingForPlayers())) {
        dmManager.StartRound();
    }
}

/*
===============
G_CheckExitRules
===============
*/
void G_CheckExitRules(void)
{
    if (g_gametype->integer != GT_SINGLE_PLAYER) {
        if (level.intermissiontime == 0.0f) {
            dmManager.CheckEndMatch();
        } else {
            G_CheckIntermissionExit();
        }
    }
}

/*
===============
G_DisplayScoresToAllClients
===============
*/
void G_DisplayScoresToAllClients(void)
{
    gentity_t *ent;
    int        i;

    for (i = 0, ent = g_entities; i < game.maxclients; ent++, i++) {
        if (!ent->inuse || !ent->entity) {
            continue;
        }

        G_DisplayScores(ent->entity);
    }
}

/*
===============
G_HideScoresToAllClients
===============
*/
void G_HideScoresToAllClients(void)
{
    gentity_t *ent;
    int        i;

    for (i = 0, ent = g_entities; i < game.maxclients; ent++, i++) {
        if (!ent->inuse || !ent->entity) {
            continue;
        }

        G_HideScores(ent->entity);
    }
}
