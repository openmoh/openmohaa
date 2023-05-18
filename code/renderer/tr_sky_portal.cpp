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

// tr_sky_portal.cpp: sky portal

#include "tr_local.h"

void R_Sky_Init()
{
	tr.portalsky.inUse = qfalse;
	tr.portalsky.numSurfs = 0;
}

void R_Sky_Reset()
{
    if (tr.portalsky.inUse) {
        return;
    }

	tr.portalsky.mins[0] = 8192.0;
	tr.portalsky.mins[1] = 8192.0;
	tr.portalsky.mins[2] = 8192.0;
	tr.portalsky.maxs[0] = -8192.0;
	tr.portalsky.maxs[1] = -8192.0;
	tr.portalsky.maxs[2] = -8192.0;
	tr.portalsky.cntNode = 0;
	tr.portalsky.numSurfs = 0;
}

void R_Sky_AddSurf(msurface_t* surf)
{
	if (tr.portalsky.inUse)
	{
		static int last_sky_warning = 0;

		if (tr.refdef.time - 1000 > last_sky_warning)
		{
			ri.Printf(3, "WARNING: sky being drawn in a sky portal!  Bad!  Bad!\n");
			last_sky_warning = tr.refdef.time;
		}

		return;
	}

	if (tr.portalsky.numSurfs < 32) {
		tr.portalsky.skySurfs[tr.portalsky.numSurfs++] = surf;
	}

	if (tr.portalsky.cntNode)
	{
		int i;

		for (i = 0; i < 3; i++) {
			if (tr.portalsky.mins[i] >= tr.portalsky.cntNode->mins[i]) {
				tr.portalsky.mins[i] = tr.portalsky.cntNode->mins[i];
			}

			if (tr.portalsky.maxs[i] >= tr.portalsky.cntNode->maxs[i]) {
				tr.portalsky.maxs[i] = tr.portalsky.cntNode->maxs[i];
			}
		}
	}
}

void R_Sky_Render() {
    int i;
    viewParms_t newParms, oldParms;
    mnode_t* leaf;

    if (!tr.portalsky.numSurfs) {
        return;
    }

    if (!tr.refdef.sky_portal) {
        return;
    }

    if (tr.skyRendered) {
        // already rendered
        return;
    }

	if (tr.portalsky.inUse)
	{
		ri.Printf(PRINT_WARNING, "WARNING: Recursive skies found!  Make that not happen!\n");
		return;
	}

    for (i = 0; i < tr.portalsky.numSurfs; i++) {
        if (!SurfIsOffscreen((const srfSurfaceFace_t*)tr.portalsky.skySurfs[i]->data, tr.portalsky.skySurfs[i]->shader, ENTITYNUM_WORLD)) {
            break;
        }
    }

    if (i == tr.portalsky.numSurfs) {
        return;
    }

    tr.skyRendered = qtrue;
    oldParms = tr.viewParms;
    newParms = tr.viewParms;

	if (r_skyportal->integer)
	{
		if (sscanf(r_skyportal_origin->string, "%f %f %f", &newParms.ori.origin[0], &newParms.ori.origin[1], &newParms.ori.origin[2]) != 3)
		{
			ri.Printf(PRINT_WARNING, "WARNING: Invalid sky portal origin: %s\n", r_skyportal_origin->string);
			return;
		}
	}
	else
	{
        VectorCopy(tr.refdef.sky_origin, newParms.ori.origin);
		MatrixMultiply(newParms.ori.axis, tr.refdef.sky_axis, newParms.ori.axis);
	}

    tr.portalsky.inUse = qtrue;
    VectorCopy(newParms.ori.origin, newParms.pvsOrigin);

    leaf = R_PointInLeaf(newParms.pvsOrigin);
    if (leaf) {
        R_RenderView(&newParms);
    }

    tr.viewParms = oldParms;

	tr.portalsky.inUse = qfalse;
	tr.portalsky.numSurfs = 0;
	R_RotateForViewer();
	R_SetupFrustum();
}

void R_Sky_ChangeFrustum() {
	// FIXME: unimplemented
}