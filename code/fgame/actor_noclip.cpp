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

// actor_noclip.cpp

#include "actor.h"

void Actor::InitNoClip(GlobalFuncs_t *func)
{
    func->ThinkState = &Actor::Think_NoClip;
    func->IsState    = &Actor::IsIdleState;
}

void Actor::Think_NoClip(void)
{
    float  frame_dist;
    vec3_t frame_offset;
    float  total_dist;
    vec3_t total_offset;
    Vector newOrigin;
    bool   done = false;

    m_pszDebugState = "";

    ContinueAnimationAllowNoPath();

    CheckUnregister();
    UpdateAngles();
    UpdateAnim();

    VectorSubtract(m_NoClipDest, origin, total_offset);
    total_dist = VectorNormalize2(total_offset, frame_offset);
    frame_dist = VectorLength(frame_delta);

    if (frame_dist > m_maxspeed * level.frametime) {
        frame_dist = m_maxspeed * level.frametime;
    }

    if (frame_dist < total_dist) {
        VectorScale(frame_offset, frame_dist, frame_offset);
        VectorAdd(origin, frame_offset, newOrigin);
    } else {
        VectorCopy(m_NoClipDest, newOrigin);
        done = true;
    }

    SafeSetOrigin(newOrigin);

    velocity[0] = frame_offset[0] / level.frametime;
    velocity[1] = frame_offset[1] / level.frametime;
    velocity[2] = frame_offset[2] / level.frametime;

    if (VectorLengthSquared(velocity) < 1) {
        VectorClear(velocity);
        done = true;
    }
    groundentity = NULL;

    if (done) {
        Com_Printf("(entnum %d, radnum %d) failsafe finished\n", entnum, radnum);
        EndCurrentThinkState();
    }

    UpdateBoneControllers();
    UpdateFootsteps();
}
