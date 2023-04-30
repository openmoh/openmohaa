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
// Scoreboard

#include "cg_local.h"

void CG_GetScoreBoardColor(float* fR, float* fG, float* fB, float* fA)
{
    // FIXME: unimplemented
}

void CG_GetScoreBoardFontColor(float* fR, float* fG, float* fB, float* fA)
{
    // FIXME: unimplemented
}

void CG_GetScoreBoardPosition(float* fX, float* fY, float* fW, float* fH)
{
    // FIXME: unimplemented
}

int CG_GetScoreBoardDrawHeader()
{
    // FIXME: unimplemented
    return 0;
}

const char* CG_GetColumnName(int iColumnNum, int* iColumnWidth)
{
    int iReturnWidth;
    const char* pszReturnString;

    switch (iColumnNum) {
    case 0:
        iReturnWidth = 128;
        pszReturnString = "Name";
        break;
    case 1:
        iReturnWidth = 64;
        pszReturnString = "Kills";
        break;
    case 2:
        iReturnWidth = 64;
        pszReturnString = "Deaths";
        if (cgs.gametype > GT_TEAM) {
            pszReturnString = "Total";
        }
        break;
    case 3:
        iReturnWidth = 64;
        pszReturnString = "Time";
        break;
    case 4:
        iReturnWidth = 64;
        pszReturnString = "Ping";
        break;
    default:
        iReturnWidth = 0;
        pszReturnString = 0;
        break;
    }

    if (iColumnWidth) {
        *iColumnWidth = iReturnWidth;
    }

    return pszReturnString;
}
