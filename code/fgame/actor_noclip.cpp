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

bool Actor::IsNoClipState(int state)
{
    return state == THINKSTATE_NOCLIP;
}

void Actor::Think_NoClip(void)
{
    //FIXME: not sure of naming
    bool   done      = false;
    Vector newOrigin = vec_zero;
    Vector total_offset;
    Vector total_offset_unit;
    float  total_dist;
    //vec3_t frame_offset;
    float frame_dist;

    m_pszDebugState = "";

    ContinueAnimationAllowNoPath();

    CheckUnregister();
    UpdateAngles();
    UpdateAnim();

    total_offset = m_NoClipDest - origin;
    total_dist   = VectorNormalize2(total_offset, total_offset_unit);

    frame_dist = level.frametime * m_maxspeed;

    if (frame_dist >= frame_delta.lengthSquared()) {
        frame_dist = frame_delta.lengthSquared();
    }

    if (frame_dist < total_dist) {
        newOrigin = total_offset_unit * frame_dist + origin;

    } else {
        done      = true;
        newOrigin = m_NoClipDest;
    }

    SafeSetOrigin(newOrigin);

    velocity = total_offset_unit / level.frametime;

    if (velocity.lengthSquared() < 1) {
        done     = true;
        velocity = vec_zero;
    }
    groundentity = NULL;

    if (done) {
        Com_Printf("(entnum %d, radnum %d) failsafe finished\n", entnum, radnum);
        EndCurrentThinkState();
    }

    UpdateBoneControllers();
    UpdateFootsteps();
}
