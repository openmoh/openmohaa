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
// Player functions for the cgame

#include "cg_local.h"
#include "cg_commands.h"

/*

player entities generate a great deal of information from implicit ques
taken from the entityState_t

*/

//=====================================================================

/*
===============
CG_ResetPlayerEntity

A player just came into view or teleported, so reset all animation info
===============
*/
void CG_ResetPlayerEntity(centity_t *cent)
{
    cent->errorTime    = -99999; // guarantee no error decay added
    cent->extrapolated = qfalse;

    VectorCopy(cent->currentState.origin, cent->lerpOrigin);
    VectorCopy(cent->lerpOrigin, cent->rawOrigin);

    VectorCopy(cent->currentState.angles, cent->lerpAngles);
    VectorCopy(cent->lerpAngles, cent->rawAngles);

    if (cent->currentState.number == cg.snap->ps.clientNum) {
        // initialize the camera position
        VectorCopy(cent->lerpOrigin, cg.currentViewPos);
        // initialize the camera angles
        VectorCopy(cent->lerpAngles, cg.currentViewAngles);
    }

    // Reset splash info
    cent->splash_still_count = 0;
}

/*
===============
CG_Player
===============
*/
void CG_Player(centity_t *cent)
{
    //CG_PlayerSplash( cent );
}
