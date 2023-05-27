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

/*
** RB_CalcColorFromConstant
*/
void RB_CalcColorFromConstant(unsigned char* dstColors, unsigned char* constantColor)
{
    int i;

    for (i = 0; i < tess.numVertexes; i++) {
        dstColors[i * 4] = constantColor[0];
        dstColors[i * 4 + 1] = constantColor[1];
        dstColors[i * 4 + 2] = constantColor[2];
        dstColors[i * 4 + 3] = constantColor[3];
    }
}

void RB_CalcRGBFromDot(unsigned char* colors, float alphaMin, float alphaMax)
{
    // FIXME: unimplemented
}

void RB_CalcRGBFromOneMinusDot(unsigned char* colors, float alphaMin, float alphaMax)
{
    // FIXME: unimplemented
}

void RB_CalcAlphaFromConstant(unsigned char* dstColors, int constantAlpha)
{
    int i;

    for (i = 0; i < tess.numVertexes; i++) {
        dstColors[i * 4 + 3] = constantAlpha;
    }
}

void RB_CalcAlphaFromDot(unsigned char* colors, float alphaMin, float alphaMax)
{
    // FIXME: unimplemented
}

void RB_CalcAlphaFromOneMinusDot(unsigned char* colors, float alphaMin, float alphaMax)
{
    // FIXME: unimplemented
}

void RB_CalcAlphaFromTexCoords(unsigned char* colors, float alphaMin, float alphaMax, int alphaMinCap, int alphaCap, float sWeight, float tWeight, float* st)
{
    // FIXME: unimplemented
}

void RB_CalcRGBFromTexCoords(unsigned char* colors, float alphaMin, float alphaMax, int alphaMinCap, int alphaCap, float sWeight, float tWeight, float* st)
{
    // FIXME: unimplemented
}
