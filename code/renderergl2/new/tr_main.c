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

#include "../tr_local.h"

#define CIRCLE_LENGTH		25

/*
================
R_DebugCircle
================
*/
void R_DebugCircle(const vec3_t org, float radius, float r, float g, float b, float alpha, qboolean horizontal) {
    int				i;
    float			ang;
    debugline_t* line;
    vec3_t			forward, right;
    vec3_t			pos, lastpos;

    if (!ri.DebugLines || !ri.numDebugLines) {
        return;
    }

    if (horizontal)
    {
        VectorSet(forward, 1, 0, 0);
        VectorSet(right, 0, 1, 0);
    }
    else
    {
        VectorCopy(tr.refdef.viewaxis[1], right);
        VectorCopy(tr.refdef.viewaxis[2], forward);
    }

    VectorClear(pos);
    VectorClear(lastpos);

    for (i = 0; i < CIRCLE_LENGTH; i++) {
        VectorCopy(pos, lastpos);

        ang = (float)i * 0.0174532925199433f;
        pos[0] = (org[0] + sin(ang) * radius * forward[0]) +
            cos(ang) * radius * right[0];
        pos[1] = (org[1] + sin(ang) * radius * forward[1]) +
            cos(ang) * radius * right[1];
        pos[2] = (org[2] + sin(ang) * radius * forward[2]) +
            cos(ang) * radius * right[2];

        if (i > 0)
        {
            if (*ri.numDebugLines >= r_numdebuglines->integer) {
                ri.Printf(PRINT_ALL, "R_DebugCircle: Exceeded MAX_DEBUG_LINES\n");
                return;
            }

            line = &(*ri.DebugLines)[*ri.numDebugLines];
            (*ri.numDebugLines)++;
            VectorCopy(lastpos, line->start);
            VectorCopy(pos, line->end);
            VectorSet(line->color, r, g, b);
            line->alpha = alpha;
            line->width = 1.0;
            line->factor = 1;
            line->pattern = -1;
        }
    }
}

/*
================
R_DebugLine
================
*/
void R_DebugLine(const vec3_t start, const vec3_t end, float r, float g, float b, float alpha) {
    debugline_t* line;

    if (!ri.DebugLines || !ri.numDebugLines) {
        return;
    }

    if (*ri.numDebugLines >= r_numdebuglines->integer) {
        ri.Printf(PRINT_ALL, "R_DebugLine: Exceeded MAX_DEBUG_LINES\n");
    }

    line = &(*ri.DebugLines)[*ri.numDebugLines];
    (*ri.numDebugLines)++;
    VectorCopy(start, line->start);
    VectorCopy(end, line->end);
    VectorSet(line->color, r, g, b);
    line->alpha = alpha;
    line->width = 1.0;
    line->factor = 1;
    line->pattern = -1;
}

/*
================
R_DrawDebugLines
================
*/
void R_DrawDebugLines(void) {
    // FIXME: stub
}