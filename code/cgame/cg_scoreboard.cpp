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
    *fR = 0.0f;
    *fG = 0.0f;
    *fB = 0.0f;
    *fA = 0.7f;
}

void CG_GetScoreBoardFontColor(float* fR, float* fG, float* fB, float* fA)
{
    *fR = 1.0f;
    *fG = 1.0f;
    *fB = 1.0f;
    *fA = 1.0f;
}

void CG_GetScoreBoardPosition(float* fX, float* fY, float* fW, float* fH)
{
    *fX = 32.0;
    *fY = 56.0;
    *fW = 384.0;
    *fH = 392.0;
}

int CG_GetScoreBoardDrawHeader()
{
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

void CG_PrepScoreBoardInfo()
{
    switch (cgs.gametype) {
    case GT_OBJECTIVE:
        strcpy(cg.scoresMenuName, "Obj_Scoreboard");
        break;
    case GT_TEAM_ROUNDS:
        strcpy(cg.scoresMenuName, "DM_Round_Scoreboard");
        break;
    default:
        strcpy(cg.scoresMenuName, "DM_Scoreboard");
        break;
    }
}

void CG_ParseScores()
{
    // FIXME: stub
}
