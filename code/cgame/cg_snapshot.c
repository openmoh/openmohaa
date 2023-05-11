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
// things that happen on snapshot transition, not necessarily every
// single frame

#include "cg_local.h"
#include "tiki.h"

/*
==================
CG_ResetEntity
==================
*/
static void CG_ResetEntity(centity_t* cent)
{
    dtiki_t* tiki;
    int i;

    VectorCopy(cent->currentState.origin, cent->lerpOrigin);

    VectorCopy(cent->currentState.angles, cent->lerpAngles);

    // if we just teleported, all we care about is the position and orientation
    // information
    if (cent->teleported) {
        cent->teleported = qfalse;
        return;
    }

    // reset local color
    cent->client_color[0] = 1;
    cent->client_color[1] = 1;
    cent->client_color[2] = 1;
    cent->client_color[3] = 1;

    // Make sure entity starts with no loop sound
    cent->tikiLoopSound = 0;

    // Reset client flags
    cent->clientFlags = 0;

    // Reset splash info
    cent->splash_last_spawn_time = 0;
    cent->splash_still_count = -1;

    tiki = cgi.R_Model_GetHandle(cgs.model_draw[cent->currentState.modelindex]);
    // reset client command
    CG_RemoveClientEntity(cent->currentState.number, tiki, cent);

    // reset the animation for the entities
    if (tiki && tiki->a->bIsCharacter) {
        for (i = 0; i < MAX_FRAMEINFOS; i++) {
            cent->animLast[i] = cent->currentState.frameInfo[i].index;
            cent->animLastTimes[i] = cent->currentState.frameInfo[i].time;
            if (cent->currentState.frameInfo[i].weight) {
                cent->animLastWeight |= 1 << i;
            } else {
                // zero-weight shouldn't count
                cent->animLastWeight &= ~(1 << i);
            }
        }
    } else {
        // clear all the last animations
        for (i = 0; i < MAX_FRAMEINFOS; i++) {
            cent->animLast[i] = -1;
        }

        cent->animLastWeight = 0;
    }

    cent->usageIndexLast = 0;
    cent->bFootOnGround_Left = qtrue;
    cent->bFootOnGround_Right = qtrue;

    if (cent->currentState.eType == ET_PLAYER) {
        CG_ResetPlayerEntity(cent);
    }
}

/*
===============
CG_TransitionEntity

cent->nextState is moved to cent->currentState and events are fired
===============
*/
static void CG_TransitionEntity(centity_t* cent)
{
    cent->currentState = cent->nextState;
    cent->currentValid = qtrue;

    // reset if the entity wasn't in the last frame or was teleported
    if (!cent->interpolate) {
        CG_ResetEntity(cent);
    }

    // clear the next state.  if will be set by the next CG_SetNextSnap
    cent->interpolate = qfalse;
    // reset the teleported state
    cent->teleported = qfalse;

    if (cent->currentState.eType == ET_EVENTS) {
        CG_Event(cent);
    }
}

/*
==================
CG_SetInitialSnapshot

This will only happen on the very first snapshot, or
on tourney restarts.  All other times will use
CG_TransitionSnapshot instead.
==================
*/
void CG_SetInitialSnapshot(snapshot_t* snap)
{
    int i;
    centity_t* cent;
    entityState_t* state;

    cg.snap = snap;

    // sort out solid entities
    CG_BuildSolidList();

    CG_ExecuteNewServerCommands(snap->serverCommandSequence, qfalse);

    for (i = 0; i < cg.snap->numEntities; i++) {
        state = &cg.snap->entities[i];
        cent = &cg_entities[state->number];

        cent->currentState = *state;
        cent->interpolate = qfalse;
        cent->currentValid = qtrue;

        CG_ResetEntity(cent);
    }

    cgi.MUSIC_UpdateMood(snap->ps.current_music_mood,
                         snap->ps.fallback_music_mood);
    cgi.MUSIC_UpdateVolume(snap->ps.music_volume,
                           snap->ps.music_volume_fade_time);
    cgi.S_SetReverb(snap->ps.reverb_type, snap->ps.reverb_level);
}

/*
===================
CG_TransitionSnapshot

The transition point from snap to nextSnap has passed
===================
*/
static void CG_TransitionSnapshot(qboolean differentServer)
{
    centity_t* cent;
    snapshot_t* oldFrame;
    int i;

    if (differentServer) {
        CG_ServerRestarted();
    }

    // execute any server string commands before transitioning entities
    CG_ExecuteNewServerCommands(cg.nextSnap->serverCommandSequence, differentServer);

    // clear the currentValid flag for all entities in the existing snapshot
    for (i = 0; i < cg.snap->numEntities; i++) {
        cent = &cg_entities[cg.snap->entities[i].number];
        cent->currentValid = qfalse;
    }

    // move nextSnap to snap and do the transitions
    oldFrame = cg.snap;
    cg.snap = cg.nextSnap;

    // FAKK: Commented out to make our stuff work
    // cg_entities[ cg.snap->ps.clientNum ].interpolate = qfalse;

    for (i = 0; i < cg.snap->numEntities; i++) {
        cent = &cg_entities[cg.snap->entities[i].number];
        if (differentServer) {
            cent->interpolate = qfalse;
            cent->teleported = qfalse;
        }
        CG_TransitionEntity(cent);
    }

    for (i = 0; i < cg.snap->number_of_sounds; i++) {
        CG_ProcessSound(&cg.snap->sounds[i]);
    }

    cgi.MUSIC_UpdateMood(cg.snap->ps.current_music_mood, cg.snap->ps.fallback_music_mood);
    cg.nextSnap = NULL;

    // check for playerstate transition events

    if (oldFrame) {
        playerState_t *ops, *ps;

        ops = &oldFrame->ps;
        ps = &cg.snap->ps;

        // teleporting checks are irrespective of prediction
        if (ps->pm_flags & PMF_RESPAWNED) {
            cg.thisFrameTeleport = qtrue;
        }

        if ((ops->music_volume != ps->music_volume) ||
            (ops->music_volume_fade_time != ps->music_volume_fade_time)) {
            cgi.MUSIC_UpdateVolume(ps->music_volume,
                                   ps->music_volume_fade_time);
        }

        if ((ops->reverb_type != ps->reverb_type) ||
            (ops->reverb_level != ps->reverb_level)) {
            cgi.S_SetReverb(ps->reverb_type, ps->reverb_level);
        }

        // if we are not doing client side movement prediction for any
        // reason, then the client events and view changes will be issued now
        if (cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_NO_PREDICTION) ||
            cg_nopredict->integer || cg_synchronousClients->integer) {
            CG_TransitionPlayerState(ps, ops);
        }
    }
}

/*
===================
CG_SetNextSnap

A new snapshot has just been read in from the client system.
===================
*/
static void CG_SetNextSnap(snapshot_t* snap)
{
    int num;
    entityState_t* es;
    centity_t* cent;

    cg.nextSnap = snap;

    cg_entities[cg.snap->ps.clientNum].interpolate = qtrue;

    // check for extrapolation errors
    for (num = 0; num < snap->numEntities; num++) {
        es = &snap->entities[num];
        cent = &cg_entities[es->number];

        cent->nextState = *es;

        // if this frame is a teleport, or the entity wasn't in the
        // previous frame, don't interpolate
        if (!cent->currentValid ||
            ((cent->currentState.eFlags ^ es->eFlags) & EF_TELEPORT_BIT) ||
            (cent->currentState.parent != es->parent) ||
            (cent->currentState.modelindex != es->modelindex)) {
            cent->interpolate = qfalse;
            // if this isn't the first frame and we have valid data, set the
            // teleport flag
            if (cent->currentValid) {
                cent->teleported = qtrue;
            }
        } else {
            cent->interpolate = qtrue;
        }
    }

    // if the next frame is a teleport for the playerstate, we
    // can't interpolate during demos
    if (cg.snap && (snap->ps.pm_flags & PMF_RESPAWNED)) {
        cg.nextFrameTeleport = qtrue;
    } else {
        cg.nextFrameTeleport = qfalse;
    }

    // if changing follow mode, don't interpolate
    if (cg.nextSnap->ps.clientNum != cg.snap->ps.clientNum) {
        cg.nextFrameTeleport = qtrue;
    }

    // if the camera cut bit changed, than the next frame is a camera cut
    if ((cg.nextSnap->ps.camera_flags & CF_CAMERA_CUT_BIT) !=
        (cg.snap->ps.camera_flags & CF_CAMERA_CUT_BIT)) {
        cg.nextFrameCameraCut = qtrue;
    } else {
        cg.nextFrameCameraCut = qfalse;
    }

    // if changing server restarts, don't interpolate
    if (snap->serverTime < cgi.GetServerStartTime()) {
        // reset the camera
        cg.nextFrameTeleport = qtrue;
    }

    // sort out solid entities
    CG_BuildSolidList();
}

/*
========================
CG_ReadNextSnapshot

This is the only place new snapshots are requested
This may increment cgs.processedSnapshotNum multiple
times if the client system fails to return a
valid snapshot.
========================
*/
static snapshot_t* CG_ReadNextSnapshot(void)
{
    qboolean r;
    snapshot_t* dest;

    if (cg.latestSnapshotNum > cgs.processedSnapshotNum + 1000) {
        cgi.Error(ERR_DROP, "CG_ReadNextSnapshot: way out of range, %i > %i",
                  cg.latestSnapshotNum, cgs.processedSnapshotNum);
    }

    while (cgs.processedSnapshotNum < cg.latestSnapshotNum) {
        // decide which of the two slots to load it into
        if (cg.snap == &cg.activeSnapshots[0]) {
            dest = &cg.activeSnapshots[1];
        } else {
            dest = &cg.activeSnapshots[0];
        }

        // try to read the snapshot from the client system
        cgs.processedSnapshotNum++;
        r = cgi.GetSnapshot(cgs.processedSnapshotNum, dest);

        // if it succeeded, return
        if (r) {
            CG_AddLagometerSnapshotInfo(dest);
            return dest;
        }

        // a GetSnapshot will return failure if the snapshot
        // never arrived, or  is so old that its entities
        // have been shoved off the end of the circular
        // buffer in the client system.

        // record as a dropped packet
        CG_AddLagometerSnapshotInfo(NULL);

        // If there are additional snapshots, continue trying to
        // read them.
    }

    // nothing left to read
    return NULL;
}

/*
============
CG_ProcessSnapshots

We are trying to set up a renderable view, so determine
what the simulated time is, and try to get snapshots
both before and after that time if available.

If we don't have a valid cg.snap after exiting this function,
then a 3D game view cannot be rendered.  This should only happen
right after the initial connection.  After cg.snap has been valid
once, it will never turn invalid.

Even if cg.snap is valid, cg.nextSnap may not be, if the snapshot
hasn't arrived yet (it becomes an extrapolating situation instead
of an interpolating one)

============
*/
void CG_ProcessSnapshots(void)
{
    snapshot_t* snap;
    int n;
    qboolean differentServer;

    // see what the latest snapshot the client system has is
    cgi.GetCurrentSnapshotNumber(&n, &cg.latestSnapshotTime);
    if (n != cg.latestSnapshotNum) {
        if (n < cg.latestSnapshotNum) {
            // this should never happen
            cgi.Error(ERR_DROP,
                      "CG_ProcessSnapshots: n < cg.latestSnapshotNum");
        }
        cg.latestSnapshotNum = n;
    }

    // If we have yet to receive a snapshot, check for it.
    // Once we have gotten the first snapshot, cg.snap will
    // always have valid data for the rest of the game
    while (!cg.snap) {
        snap = CG_ReadNextSnapshot();
        if (!snap) {
            // we can't continue until we get a snapshot
            return;
        }

        // set our weapon selection to what
        // the playerstate is currently using
        if (!(snap->snapFlags & SNAPFLAG_NOT_ACTIVE)) {
            CG_SetInitialSnapshot(snap);
        }
    }

    // loop until we either have a valid nextSnap with a serverTime
    // greater than cg.time to interpolate towards, or we run
    // out of available snapshots
    do {
        // if we don't have a nextframe, try and read a new one in
        if (!cg.nextSnap) {
            snap = CG_ReadNextSnapshot();

            // if we still don't have a nextframe, we will just have to
            // extrapolate
            if (!snap) {
                break;
            }

            CG_SetNextSnap(snap);

            // if time went backwards, we have a level restart
            if (cg.nextSnap->serverTime < cg.snap->serverTime) {
                // only drop if this is not a restart or loadgame
                if (!((cg.nextSnap->snapFlags ^ cg.snap->snapFlags) &
                      SNAPFLAG_SERVERCOUNT)) {
                    cgi.Error(
                        ERR_DROP,
                        "CG_ProcessSnapshots: Server time went backwards");
                }
            }
        }

        differentServer = cg.snap->serverTime <= cgi.GetServerStartTime();

        // if our time is < nextFrame's, we have a nice interpolating state
        if (cg.time >= cg.snap->serverTime &&
            cg.time < cg.nextSnap->serverTime &&
            !differentServer) {
            break;
        }

        // we have passed the transition from nextFrame to frame
        CG_TransitionSnapshot(differentServer);
    } while (1);

    // assert our valid conditions upon exiting
    if (cg.snap == NULL) {
        cgi.Error(ERR_DROP, "CG_ProcessSnapshots: cg.snap == NULL");
    }
    if (cg.time < cg.snap->serverTime) {
        // this can happen right after a vid_restart
        cg.time = cg.snap->serverTime;
        cgi.SetTime(cg.time);
    }
    if (cg.nextSnap != NULL && cg.nextSnap->serverTime <= cg.time) {
        cgi.Error(ERR_DROP,
                  "CG_ProcessSnapshots: cg.nextSnap->serverTime <= cg.time");
    }
}
