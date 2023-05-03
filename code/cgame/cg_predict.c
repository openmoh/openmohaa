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
// this file generates cg.predicted_player_state by either
// interpolating between snapshots from the server or locally predicting
// ahead the client's movement. It also handles local physics interaction,
// like fragments bouncing off walls

#include "cg_local.h"

static pmove_t cg_pmove;

static int cg_numSolidEntities;
static centity_t* cg_solidEntities[MAX_ENTITIES_IN_SNAPSHOT];
static int cg_numTriggerEntities;
// static	centity_t	*cg_triggerEntities[MAX_ENTITIES_IN_SNAPSHOT];

/*
====================
CG_BuildSolidList

When a new cg.snap has been set, this function builds a sublist
of the entities that are actually solid, to make for more
efficient collision detection
====================
*/
void CG_BuildSolidList(void)
{
    int i;
    centity_t* cent;
    snapshot_t* snap;
    entityState_t* ent;

    cg_numSolidEntities = 0;
    cg_numTriggerEntities = 0;

    if (cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport) {
        snap = cg.nextSnap;
    } else {
        snap = cg.snap;
    }

    for (i = 0; i < snap->numEntities; i++) {
        cent = &cg_entities[snap->entities[i].number];
        ent = &cent->currentState;

        if (ent->eType == ET_ITEM || ent->eType == ET_PUSH_TRIGGER ||
            ent->eType == ET_TELEPORT_TRIGGER) {
            /*
                           cg_triggerEntities[cg_numTriggerEntities] = cent;
                           cg_numTriggerEntities++;
            */
            continue;
        }

        if (cent->nextState.solid) {
            cg_solidEntities[cg_numSolidEntities] = cent;
            cg_numSolidEntities++;
            continue;
        }
    }
}

/*
====================
CG_ClipMoveToEntities

====================
*/
static void CG_ClipMoveToEntities(const vec3_t start, const vec3_t mins,
                                  const vec3_t maxs, const vec3_t end,
                                  int skipNumber, int mask, trace_t* tr,
                                  qboolean cylinder)

{
    int i;
    trace_t trace;
    entityState_t* ent;
    clipHandle_t cmodel;
    vec3_t bmins, bmaxs;
    vec3_t origin, angles;
    centity_t* cent;

    for (i = 0; i < cg_numSolidEntities; i++) {
        cent = cg_solidEntities[i];
        ent = &cent->currentState;

        if (ent->number == skipNumber) {
            continue;
        }

        if (ent->solid == SOLID_BMODEL) {
            // special value for bmodel
            cmodel = cgi.CM_InlineModel(ent->modelindex);
            if (!cmodel) {
                continue;
            }
            VectorCopy(cent->lerpAngles, angles);
            VectorCopy(cent->lerpOrigin, origin);
        } else {
            IntegerToBoundingBox(ent->solid, bmins, bmaxs);
            cmodel = cgi.CM_TempBoxModel(
                bmins, bmaxs,
                CONTENTS_BODY); // CONTENTS_SOLID | CONTENTS_BODY );
            VectorCopy(vec3_origin, angles);
            VectorCopy(cent->lerpOrigin, origin);
        }

        cgi.CM_TransformedBoxTrace(&trace, start, end, mins, maxs, cmodel, mask,
                                   origin, angles, cylinder);

        if (trace.allsolid || trace.fraction < tr->fraction) {
            trace.entityNum = ent->number;
            *tr = trace;
        } else if (trace.startsolid) {
            tr->startsolid = qtrue;
        }
    }
}

void CG_ShowTrace(trace_t* trace, int passent, const char* reason)

{
    char text[1024];

    assert(reason);
    assert(trace);

    sprintf(text, "%0.2f : Pass (%d) Frac %f Hit (%d): '%s'\n",
            (float)cg.time / 1000.0f, passent, trace->fraction,
            trace->entityNum, reason ? reason : "");

    if (cg_traceinfo->integer == 3) {
        cgi.DebugPrintf(text);
    } else {
        cgi.DPrintf(text);
    }
}

/*
================
CG_Trace
================
*/
void CG_Trace(trace_t* result, const vec3_t start, const vec3_t mins,
              const vec3_t maxs, const vec3_t end, int skipNumber, int mask,
              qboolean cylinder, qboolean cliptoentities,
              const char* description)

{
    trace_t t;

    cgi.CM_BoxTrace(&t, start, end, mins, maxs, 0, mask, cylinder);
    t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;

    // If starting in a solid make sure the world is set as the entitynum

    if (t.startsolid) {
        t.entityNum = ENTITYNUM_WORLD;
    }

    if (cliptoentities) {
        // check all other solid models
        CG_ClipMoveToEntities(start, mins, maxs, end, skipNumber, mask, &t,
                              cylinder);
    }

    *result = t;

    if (cg_traceinfo->integer) {
        CG_ShowTrace(result, skipNumber, description);
    }
}

/*
================
CG_PlayerTrace
================
*/
void CG_PlayerTrace(trace_t* result, const vec3_t start, const vec3_t mins,
                    const vec3_t maxs, const vec3_t end, int skipNumber,
                    int mask, qboolean cylinder, qboolean tracedeep)

{
    CG_Trace(result, start, mins, maxs, end, skipNumber, mask, cylinder, qtrue,
             "PlayerTrace");
}

/*
================
CG_PointContents
================
*/
int CG_PointContents(const vec3_t point, int passEntityNum)
{
    int i;
    entityState_t* ent;
    centity_t* cent;
    clipHandle_t cmodel;
    int contents;

    contents = cgi.CM_PointContents(point, 0);

    for (i = 0; i < cg_numSolidEntities; i++) {
        cent = cg_solidEntities[i];

        ent = &cent->currentState;

        if (ent->number == passEntityNum) {
            continue;
        }

        if (ent->solid != SOLID_BMODEL) { // special value for bmodel
            continue;
        }

        cmodel = cgi.CM_InlineModel(ent->modelindex);
        if (!cmodel) {
            continue;
        }

        contents |= cgi.CM_TransformedPointContents(point, cmodel, ent->origin,
                                                    ent->angles);
    }

    return contents;
}

/*
========================
CG_InterpolatePlayerStateCamera

Generates cg.predicted_player_state by interpolating between
cg.snap->player_state and cg.nextFrame->player_state
========================
*/
static void CG_InterpolatePlayerStateCamera(void)
{
    float f;
    int i;
    snapshot_t *prev, *next;

    prev = cg.snap;
    next = cg.nextSnap;

    //
    // copy in the current ones if nothing else
    //
    VectorCopy(cg.predicted_player_state.camera_angles, cg.camera_angles);
    VectorCopy(cg.predicted_player_state.camera_origin, cg.camera_origin);
    cg.camera_fov = cg.predicted_player_state.fov;

    // if the next frame is a teleport, we can't lerp to it
    if (cg.nextFrameCameraCut) {
        return;
    }

    if (!next || next->serverTime <= prev->serverTime) {
        return;
    }

    f = (float)(cg.time - prev->serverTime) /
        (next->serverTime - prev->serverTime);

    // interpolate fov
    cg.camera_fov = prev->ps.fov + f * (next->ps.fov - prev->ps.fov);

    if (!(cg.snap->ps.pm_flags & PMF_CAMERA_VIEW)) {
        return;
    }

    for (i = 0; i < 3; i++) {
        cg.camera_origin[i] =
            prev->ps.camera_origin[i] +
            f * (next->ps.camera_origin[i] - prev->ps.camera_origin[i]);
        cg.camera_angles[i] =
            LerpAngle(prev->ps.camera_angles[i], next->ps.camera_angles[i], f);
    }
}

/*
========================
CG_InterpolatePlayerState

Generates cg.predicted_player_state by interpolating between
cg.snap->player_state and cg.nextFrame->player_state
========================
*/
static void CG_InterpolatePlayerState(qboolean grabAngles)
{
    float f;
    int i;
    playerState_t* out;
    snapshot_t *prev, *next;

    out = &cg.predicted_player_state;
    prev = cg.snap;
    next = cg.nextSnap;

    *out = cg.snap->ps;

    // interpolate the camera if necessary
    CG_InterpolatePlayerStateCamera();

    // if we are still allowing local input, short circuit the view angles
    if (grabAngles) {
        usercmd_t cmd;
        int cmdNum;

        cmdNum = cgi.GetCurrentCmdNumber();
        cgi.GetUserCmd(cmdNum, &cmd);

        PM_UpdateViewAngles(out, &cmd);
    }

    // if the next frame is a teleport, we can't lerp to it
    if (cg.nextFrameTeleport) {
        return;
    }

    if (!next || next->serverTime <= prev->serverTime) {
        return;
    }

    f = cg.frameInterpolation;

    for (i = 0; i < 3; i++) {
        out->origin[i] =
            prev->ps.origin[i] + f * (next->ps.origin[i] - prev->ps.origin[i]);
        if (!grabAngles) {
            out->viewangles[i] =
                LerpAngle(prev->ps.viewangles[i], next->ps.viewangles[i], f);
        }
        out->velocity[i] = prev->ps.velocity[i] +
                           f * (next->ps.velocity[i] - prev->ps.velocity[i]);
    }
}

/*
=================
CG_PredictPlayerState

Generates cg.predicted_player_state for the current cg.time
cg.predicted_player_state is guaranteed to be valid after exiting.

For demo playback, this will be an interpolation between two valid
playerState_t.

For normal gameplay, it will be the result of predicted usercmd_t on
top of the most recent playerState_t received from the server.

Each new snapshot will usually have one or more new usercmd over the last,
but we simulate all unacknowledged commands each time, not just the new ones.
This means that on an internet connection, quite a few pmoves may be issued
each frame.

OPTIMIZE: don't re-simulate unless the newly arrived snapshot playerState_t
differs from the predicted one.  Would require saving all intermediate
playerState_t during prediction.

We detect prediction errors and allow them to be decayed off over several frames
to ease the jerk.
=================
*/
void CG_PredictPlayerState(void)
{
    int cmdNum, current;
    playerState_t oldPlayerState;
    qboolean moved;
    usercmd_t latestCmd;

    cg.hyperspace = qfalse; // will be set if touching a trigger_teleport

    // if this is the first frame we must guarantee
    // predicted_player_state is valid even if there is some
    // other error condition
    if (!cg.validPPS) {
        cg.validPPS = qtrue;
        cg.predicted_player_state = cg.snap->ps;
    }

    // demo playback just copies the moves
    if (cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_NO_PREDICTION) ||
        (cg.snap->ps.pm_flags & PMF_FROZEN)) {
        CG_InterpolatePlayerState(qfalse);
        return;
    }

    // non-predicting local movement will grab the latest angles
    // FIXME
    //  Noclip is jittery for some reason, so I'm disabling prediction while
    //  noclipping
    if (cg_nopredict->integer || cg_synchronousClients->integer ||
        (cg.snap->ps.pm_type == PM_NOCLIP)) {
        CG_InterpolatePlayerState(qtrue);
        return;
    }

    // prepare for pmove
    cg_pmove.ps = &cg.predicted_player_state;
    cg_pmove.trace = CG_PlayerTrace;
    cg_pmove.pointcontents = CG_PointContents;

    if (cg_pmove.ps->pm_type == PM_DEAD) {
        cg_pmove.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
    } else {
        cg_pmove.tracemask = MASK_PLAYERSOLID;
    }

    cg_pmove.noFootsteps = (cgs.dmflags & DF_NO_FOOTSTEPS) > 0;

    // save the state before the pmove so we can detect transitions
    oldPlayerState = cg.predicted_player_state;

    current = cgi.GetCurrentCmdNumber();

    // get the latest command so we can know which commands are from previous
    // map_restarts
    cgi.GetUserCmd(current, &latestCmd);

    // get the most recent information we have, even if
    // the server time is beyond our current cg.time,
    // because predicted player positions are going to
    // be ahead of everything else anyway
    if (cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport) {
        cg.predicted_player_state = cg.nextSnap->ps;
        cg.physicsTime = cg.nextSnap->serverTime;
    } else {
        cg.predicted_player_state = cg.snap->ps;
        cg.physicsTime = cg.snap->serverTime;
    }

    if (pmove_msec->integer >= 8)
    {
        if (pmove_msec->integer > 33)
            cgi.Cvar_Set("pmove_msec", "33");
    }
    else
    {
        cgi.Cvar_Set("pmove_msec", "8");
    }

    cg_pmove.pmove_fixed = pmove_fixed->integer;
    cg_pmove.pmove_msec = pmove_msec->integer;

    // run cmds
    moved = qfalse;
    for (cmdNum = current - CMD_BACKUP + 1; cmdNum <= current; cmdNum++) {
        // get the command
        cgi.GetUserCmd(cmdNum, &cg_pmove.cmd);

        if (cg_pmove.pmove_fixed) {
            PM_UpdateViewAngles(cg_pmove.ps, &cg_pmove.cmd);
        }

        // don't do anything if the time is before the snapshot player time
        if (cg_pmove.cmd.serverTime <= cg.predicted_player_state.commandTime) {
            continue;
        }

        // don't do anything if the command was from a previous map_restart
        if (cg_pmove.cmd.serverTime > latestCmd.serverTime) {
            continue;
        }

        // check for a prediction error from last frame
        // on a lan, this will often be the exact value
        // from the snapshot, but on a wan we will have
        // to predict several commands to get to the point
        // we want to compare
        if (cg.predicted_player_state.commandTime ==
            oldPlayerState.commandTime) {

            if (cg.thisFrameTeleport) {
                // a teleport will not cause an error decay
                VectorClear(cg.predictedError);
                cg.thisFrameTeleport = qfalse;
                if (cg_showmiss->integer) {
                    cgi.Printf("PredictionTeleport\n");
                }
            }
        }

        // if our feet are falling, don't try to move
        if (cg_pmove.ps->feetfalling && (cg_pmove.waterlevel < 2)) {
            cg_pmove.cmd.forwardmove = 0;
            cg_pmove.cmd.rightmove = 0;
        }

        if (cg_pmove.pmove_fixed) {
            cg_pmove.cmd.serverTime = pmove_msec->integer
                * ((cg_pmove.cmd.serverTime + pmove_msec->integer - 1)
                    / pmove_msec->integer);
        }

        Pmove(&cg_pmove);

        moved = qtrue;

        // add push trigger movement effects
        // CG_TouchTriggerPrediction();
    }

    if (cg_showmiss->integer > 1) {
        cgi.Printf("[%i : %i] ", cg_pmove.cmd.serverTime, cg.time);
    }

    // interpolate the camera if necessary
    CG_InterpolatePlayerStateCamera();

    // Interpolate the player position
    if (cg.predicted_player_state.groundEntityNum != ENTITYNUM_WORLD) {
        centity_t* cent;
        vec3_t offset;

        cent = &cg_entities[cg.predicted_player_state.groundEntityNum];

        if (cent->interpolate) {
            float f;

            f = cg.frameInterpolation - 1.0;

            offset[0] = (cent->nextState.origin[0] - cg_entities[cg.predicted_player_state.groundEntityNum].currentState.origin[0]) * f;
            offset[1] = (cent->nextState.origin[1] - cg_entities[cg.predicted_player_state.groundEntityNum].currentState.origin[1]) * f;
            offset[2] = (cent->nextState.origin[2] - cg_entities[cg.predicted_player_state.groundEntityNum].currentState.origin[2]) * f;

            cg.predicted_player_state.origin[0] += offset[0];
            cg.predicted_player_state.origin[1] += offset[1];
            cg.predicted_player_state.origin[2] += offset[2];
        }
    }

    // adjust for the movement of the groundentity

    if (!moved) {
        if (cg_showmiss->integer) {
            cgi.Printf("not moved\n");
        }
        return;
    }

    // fire events and other transition triggered things
    CG_TransitionPlayerState(&cg.predicted_player_state, &oldPlayerState);
}
