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

void CG_PrepScoreBoardInfo()
{
	switch (cgs.gametype) {
	case GT_TEAM_ROUNDS:
		strcpy(cg.scoresMenuName, "DM_Round_Scoreboard");
		break;
    case GT_OBJECTIVE:
        strcpy(cg.scoresMenuName, "Obj_Scoreboard");
		break;
	case GT_TOW:
		strcpy(cg.scoresMenuName, "Tow_Scoreboard");
		break;
	case GT_LIBERATION:
		strcpy(cg.scoresMenuName, "Lib_Scoreboard");
		break;
    default:
        strcpy(cg.scoresMenuName, "DM_Scoreboard");
        break;
    }
}

const char* CG_GetColumnName_ver_15(int iColumnNum, int* iColumnWidth)
{
	int iReturnWidth;
	const char* pszReturnString;

	switch (iColumnNum) {
	case 0:
		iReturnWidth = 24;
		pszReturnString = "#";
		break;
	case 1:
		iReturnWidth = 128;
		pszReturnString = "Name";
		break;
	case 2:
		iReturnWidth = 64;
		pszReturnString = "Kills";
		break;
	case 3:
		iReturnWidth = 64;
		pszReturnString = "Deaths";
		if (cgs.gametype > GT_TEAM) {
			pszReturnString = "Total";
		}
		break;
	case 4:
		iReturnWidth = 64;
		pszReturnString = "Time";
		break;
	case 5:
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

const char* CG_GetColumnName_ver_6(int iColumnNum, int* iColumnWidth)
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

void CG_ParseScores_ver_15()
{
	int i;
	int iEntryCount;
	int iClientTeam, iClientNum;
	int iDatumCount;
	int iMatchTeam;
	int iCurrentEntry;
	qboolean bIsDead, bIsHeader;
	char szString2[MAX_STRING_TOKENS];
	char szString3[MAX_STRING_TOKENS];
	char szString4[MAX_STRING_TOKENS];
	char szString5[MAX_STRING_TOKENS];
	char szString6[MAX_STRING_TOKENS];
	char szString7[MAX_STRING_TOKENS];
	float vSameTeamTextColor[4];
	float vSameTeamBackColor[4];
	float vOtherTeamTextColor[4];
	float vOtherTeamBackColor[4];
	float vNoTeamTextColor[4];
	float vNoTeamBackColor[4];
	float vThisClientTextColor[4];
	float vThisClientBackColor[4];
	float vDeadTextColorDead[4];
	float* pItemTextColor;
	float* pItemBackColor;

	iMatchTeam = -1;
	vSameTeamTextColor[0] = 1.0f;
	vSameTeamTextColor[1] = 1.0f;
	vSameTeamTextColor[2] = 1.0f;
	vSameTeamTextColor[3] = 1.0f;
	vSameTeamBackColor[0] = 0.1f;
	vSameTeamBackColor[1] = 0.5f;
	vSameTeamBackColor[2] = 0.1f;
	vSameTeamBackColor[3] = 0.4f;
	vOtherTeamTextColor[0] = 1.0f;
	vOtherTeamTextColor[1] = 1.0f;
	vOtherTeamTextColor[2] = 1.0f;
	vOtherTeamTextColor[3] = 1.0f;
	vOtherTeamBackColor[0] = 0.5f;
	vOtherTeamBackColor[1] = 0.1f;
	vOtherTeamBackColor[2] = 0.1f;
	vOtherTeamBackColor[3] = 0.4f;
	vNoTeamTextColor[0] = 1.0f;
	vNoTeamTextColor[1] = 1.0f;
	vNoTeamTextColor[2] = 1.0f;
	vNoTeamTextColor[3] = 1.0f;
	vNoTeamBackColor[0] = 0.1f;
	vNoTeamBackColor[1] = 0.1f;
	vNoTeamBackColor[2] = 0.1f;
	vNoTeamBackColor[3] = 0.4f;
	vDeadTextColorDead[0] = 1.0f;
	vDeadTextColorDead[1] = 0.1f;
	vDeadTextColorDead[2] = 0.1f;
	vDeadTextColorDead[3] = 1.0f;

	vThisClientTextColor[0] = 0.0f;
	vThisClientTextColor[1] = 0.0f;
	vThisClientTextColor[2] = 0.0f;
	vThisClientTextColor[3] = 1.0f;

	if (cgs.gametype > GT_FFA)
	{
		vThisClientBackColor[0] = 0.5f;
		vThisClientBackColor[1] = 0.75f;
		vThisClientBackColor[2] = 0.5f;
	}
	else
	{
		vThisClientBackColor[0] = 0.75f;
		vThisClientBackColor[1] = 0.75f;
		vThisClientBackColor[2] = 0.75f;
	}

	vThisClientBackColor[3] = 0.8f;

	iCurrentEntry = 1;
	if (cgs.gametype > GT_FFA)
	{
		iDatumCount = 6;
		iMatchTeam = cg.snap->ps.stats[STAT_TEAM];
		if (iMatchTeam != TEAM_ALLIES && iMatchTeam != TEAM_AXIS)
		{
			iMatchTeam = TEAM_ALLIES;
			vThisClientTextColor[0] = 0.0f;
			vThisClientTextColor[1] = 0.0f;
			vThisClientTextColor[2] = 0.0f;
			vThisClientBackColor[0] = 0.75f;
			vThisClientBackColor[1] = 0.75f;
			vThisClientBackColor[2] = 0.75f;
		}
	}
	else
	{
		// free-for-all
		iDatumCount = 5;
	}

	iEntryCount = atoi(cgi.Argv(iCurrentEntry++));
	if (iEntryCount > MAX_CLIENTS) {
		iEntryCount = MAX_CLIENTS;
	}

	if (cgs.gametype == GT_TOW)
	{
		cgi.Cvar_Set("tow_allied_obj1", va("%i", (int)atof(cgi.Argv(iCurrentEntry++))));
		cgi.Cvar_Set("tow_allied_obj2", va("%i", (int)atof(cgi.Argv(iCurrentEntry++))));
		cgi.Cvar_Set("tow_allied_obj3", va("%i", (int)atof(cgi.Argv(iCurrentEntry++))));
		cgi.Cvar_Set("tow_allied_obj4", va("%i", (int)atof(cgi.Argv(iCurrentEntry++))));
		cgi.Cvar_Set("tow_allied_obj5", va("%i", (int)atof(cgi.Argv(iCurrentEntry++))));
		cgi.Cvar_Set("tow_axis_obj1", va("%i", (int)atof(cgi.Argv(iCurrentEntry++))));
		cgi.Cvar_Set("tow_axis_obj2", va("%i", (int)atof(cgi.Argv(iCurrentEntry++))));
		cgi.Cvar_Set("tow_axis_obj3", va("%i", (int)atof(cgi.Argv(iCurrentEntry++))));
		cgi.Cvar_Set("tow_axis_obj4", va("%i", (int)atof(cgi.Argv(iCurrentEntry++))));
		cgi.Cvar_Set("tow_axis_obj5", va("%i", (int)atof(cgi.Argv(iCurrentEntry++))));
	}

	if (cgs.gametype == GT_LIBERATION)
	{
		cgi.Cvar_Set("scoreboard_toggle1", va("%i", (int)atof(cgi.Argv(iCurrentEntry++))));
		cgi.Cvar_Set("scoreboard_toggle2", va("%i", (int)atof(cgi.Argv(iCurrentEntry++))));
	}

	for (i = 0; i < iEntryCount; ++i)
	{
		bIsHeader = qfalse;
		if (cgs.gametype > GT_FFA)
		{
			iClientNum = atoi(cgi.Argv(iCurrentEntry + iDatumCount * i));
			iClientTeam = atoi(cgi.Argv(1 + iCurrentEntry + iDatumCount * i));
			if (iClientTeam >= 0) {
				bIsDead = qfalse;
			}
			else {
				bIsDead = qtrue;
				iClientTeam = -iClientTeam;
			}

			if (iClientNum == -1)
			{
				szString2[0] = 0;
				bIsHeader = qtrue;

				switch (iClientTeam)
				{
				case 1:
					strcpy(szString3, cgi.LV_ConvertString("Spectators"));
					break;
				case 2:
					strcpy(szString3, cgi.LV_ConvertString("Free-For-Allers"));
					break;
				case 3:
                    sprintf(
                        szString3,
                        "%s - %d %s",
						cgi.LV_ConvertString("Allies"),
						atoi(cgi.Argv(2 + iCurrentEntry + iDatumCount * i)),
                        cgi.LV_ConvertString("Players")
                    );
                    iCurrentEntry++;
					break;
				case 4:
					sprintf(
                        szString3,
                        "%s - %d %s",
						cgi.LV_ConvertString("Axis"),
						atoi(cgi.Argv(2 + iCurrentEntry + iDatumCount * i)),
                        cgi.LV_ConvertString("Players")
					);
					iCurrentEntry++;
					break;
				default:
					strcpy(szString3, cgi.LV_ConvertString("No Team"));
					break;
				}
			}
			else if (iClientNum == -2)
			{
				// spectating
				szString2[0] = 0;
				szString3[0] = 0;
			}
			else
			{
				strcpy(szString2, va("%i", iClientNum));
				strcpy(szString3, cg.clientinfo[iClientNum].name);
			}

			strcpy(szString4, cgi.Argv(2 + iCurrentEntry + iDatumCount * i));
			strcpy(szString5, cgi.Argv(3 + iCurrentEntry + iDatumCount * i));
			strcpy(szString6, cgi.Argv(4 + iCurrentEntry + iDatumCount * i));
			strcpy(szString7, cgi.Argv(5 + iCurrentEntry + iDatumCount * i));

			if (cgs.gametype >= GT_TEAM_ROUNDS && iClientNum == -1 && (iClientTeam == TEAM_ALLIES || iClientTeam == TEAM_AXIS))
			{
				strcat(szString4, va(" %s", cgi.LV_ConvertString("Wins")));
				szString5[0] = 0;
			}

			if (iClientNum == cg.snap->ps.clientNum)
			{
				pItemTextColor = vThisClientTextColor;
				pItemBackColor = vThisClientBackColor;
			}
			else if (iClientNum == -2)
			{
				pItemTextColor = vNoTeamTextColor;
				pItemBackColor = vNoTeamBackColor;
			}
			else if (iClientTeam == TEAM_ALLIES || iClientTeam == TEAM_AXIS)
			{
				if (iClientTeam == iMatchTeam)
				{
					pItemTextColor = vSameTeamTextColor;
					pItemBackColor = vSameTeamBackColor;
				}
				else
				{
					pItemTextColor = vOtherTeamTextColor;
					pItemBackColor = vOtherTeamBackColor;
				}
			}
			else
			{
				pItemTextColor = vNoTeamTextColor;
				pItemBackColor = vNoTeamBackColor;
			}

			if (bIsDead) {
				pItemTextColor = vDeadTextColorDead;
			}
		}
		else
		{
			iClientNum = atoi(cgi.Argv(iCurrentEntry + iDatumCount * i));
			if (iClientNum >= 0)
			{
                strcpy(szString2, va("%i", iClientNum));
				strcpy(szString3, cg.clientinfo[iClientNum].name);
				strcpy(szString4, cgi.Argv(1 + iCurrentEntry + iDatumCount * i));
				strcpy(szString5, cgi.Argv(2 + iCurrentEntry + iDatumCount * i));
				strcpy(szString6, cgi.Argv(3 + iCurrentEntry + iDatumCount * i));
				strcpy(szString7, cgi.Argv(4 + iCurrentEntry + iDatumCount * i));
			}
			else
			{
				szString2[0] = 0;
				if (iClientNum == -3)
				{
					strcpy(szString2, cgi.LV_ConvertString("Players"));
					bIsHeader = qtrue;
				}
				else if (iClientNum == -2)
				{
					strcpy(szString2, cgi.LV_ConvertString("Spectators"));
					bIsHeader = qtrue;
				}
				else
				{
					// unknown
					szString3[0] = 0;
				}
				szString4[0] = 0;
				szString5[0] = 0;
				szString6[0] = 0;
				szString7[0] = 0;
			}

			if (iClientNum == cg.snap->ps.clientNum)
			{
				pItemTextColor = vThisClientTextColor;
				pItemBackColor = vThisClientBackColor;
			}
			else
			{
				pItemTextColor = vNoTeamTextColor;
				pItemBackColor = vNoTeamBackColor;
			}
		}

		cgi.UI_SetScoreBoardItem(
			i,
			szString2,
			szString3,
			szString4,
			szString5,
			szString6,
			szString7,
			NULL,
			NULL,
			pItemTextColor,
			pItemBackColor,
			bIsHeader
		);
	}

	cgi.UI_DeleteScoreBoardItems(iEntryCount);
}

void CG_ParseScores_ver_6()
{
    int i;
    int iEntryCount;
    int iClientTeam, iClientNum;
    int iDatumCount;
    int iMatchTeam;
    int iCurrentEntry;
    qboolean bIsDead, bIsHeader;
    char szString2[MAX_STRING_TOKENS];
    char szString3[MAX_STRING_TOKENS];
    char szString4[MAX_STRING_TOKENS];
    char szString5[MAX_STRING_TOKENS];
    char szString6[MAX_STRING_TOKENS];
    float vSameTeamTextColor[4];
    float vSameTeamBackColor[4];
    float vOtherTeamTextColor[4];
    float vOtherTeamBackColor[4];
    float vNoTeamTextColor[4];
    float vNoTeamBackColor[4];
    float vThisClientTextColor[4];
    float vThisClientBackColor[4];
    float vDeadTextColorDead[4];
    float* pItemTextColor;
    float* pItemBackColor;

    iMatchTeam = -1;
    vSameTeamTextColor[0] = 1.0f;
    vSameTeamTextColor[1] = 1.0f;
    vSameTeamTextColor[2] = 1.0f;
    vSameTeamTextColor[3] = 1.0f;
    vSameTeamBackColor[0] = 0.1f;
    vSameTeamBackColor[1] = 0.5f;
    vSameTeamBackColor[2] = 0.1f;
    vSameTeamBackColor[3] = 0.4f;
    vOtherTeamTextColor[0] = 1.0f;
    vOtherTeamTextColor[1] = 1.0f;
    vOtherTeamTextColor[2] = 1.0f;
    vOtherTeamTextColor[3] = 1.0f;
    vOtherTeamBackColor[0] = 0.5f;
    vOtherTeamBackColor[1] = 0.1f;
    vOtherTeamBackColor[2] = 0.1f;
    vOtherTeamBackColor[3] = 0.4f;
    vNoTeamTextColor[0] = 1.0f;
    vNoTeamTextColor[1] = 1.0f;
    vNoTeamTextColor[2] = 1.0f;
    vNoTeamTextColor[3] = 1.0f;
    vNoTeamBackColor[0] = 0.1f;
    vNoTeamBackColor[1] = 0.1f;
    vNoTeamBackColor[2] = 0.1f;
    vNoTeamBackColor[3] = 0.4f;
    vDeadTextColorDead[0] = 1.0f;
    vDeadTextColorDead[1] = 0.1f;
    vDeadTextColorDead[2] = 0.1f;
    vDeadTextColorDead[3] = 1.0f;

    vThisClientTextColor[0] = 0.0f;
    vThisClientTextColor[1] = 0.0f;
    vThisClientTextColor[2] = 0.0f;
    vThisClientTextColor[3] = 1.0f;

    if (cgs.gametype > GT_FFA)
    {
        vThisClientBackColor[0] = 0.5f;
        vThisClientBackColor[1] = 0.75f;
        vThisClientBackColor[2] = 0.5f;
    }
    else
    {
        vThisClientBackColor[0] = 0.75f;
        vThisClientBackColor[1] = 0.75f;
        vThisClientBackColor[2] = 0.75f;
    }

    vThisClientBackColor[3] = 0.8f;

    iCurrentEntry = 1;
    if (cgs.gametype > GT_FFA)
    {
        iDatumCount = 6;
        iMatchTeam = cg.snap->ps.stats[STAT_TEAM];
        if (iMatchTeam != TEAM_ALLIES && iMatchTeam != TEAM_AXIS)
        {
            iMatchTeam = TEAM_ALLIES;
            vThisClientTextColor[0] = 0.0f;
            vThisClientTextColor[1] = 0.0f;
            vThisClientTextColor[2] = 0.0f;
            vThisClientBackColor[0] = 0.75f;
            vThisClientBackColor[1] = 0.75f;
            vThisClientBackColor[2] = 0.75f;
        }
    }
    else
    {
        // free-for-all
        iDatumCount = 5;
    }

    iEntryCount = atoi(cgi.Argv(iCurrentEntry++));
    if (iEntryCount > MAX_CLIENTS) {
        iEntryCount = MAX_CLIENTS;
    }

    for (i = 0; i < iEntryCount; ++i)
    {
        bIsHeader = qfalse;
        if (cgs.gametype > GT_FFA)
        {
            iClientNum = atoi(cgi.Argv(iCurrentEntry + iDatumCount * i));
            iClientTeam = atoi(cgi.Argv(1 + iCurrentEntry + iDatumCount * i));
            if (iClientTeam >= 0) {
                bIsDead = qfalse;
            }
            else {
                bIsDead = qtrue;
                iClientTeam = -iClientTeam;
            }

            if (iClientNum == -1)
            {
                bIsHeader = qtrue;

                switch (iClientTeam)
                {
                case 1:
                    strcpy(szString2, cgi.LV_ConvertString("Spectators"));
                    break;
                case 2:
                    strcpy(szString2, cgi.LV_ConvertString("Free-For-Allers"));
                    break;
                case 3:
                    strcpy(szString2, cgi.LV_ConvertString("Allies"));
                    break;
                case 4:
                    strcpy(szString2, cgi.LV_ConvertString("Axis"));
                    break;
                default:
                    strcpy(szString2, cgi.LV_ConvertString("No Team"));
                    break;
                }
            }
            else if (iClientNum == -2)
            {
                // spectating
                szString2[0] = 0;
            }
            else
			{
                strcpy(szString2, cg.clientinfo[iClientNum].name);
            }

            strcpy(szString3, cgi.Argv(2 + iCurrentEntry + iDatumCount * i));
            strcpy(szString4, cgi.Argv(3 + iCurrentEntry + iDatumCount * i));
            strcpy(szString5, cgi.Argv(4 + iCurrentEntry + iDatumCount * i));
            strcpy(szString6, cgi.Argv(5 + iCurrentEntry + iDatumCount * i));

            if (iClientNum == cg.snap->ps.clientNum)
            {
                pItemTextColor = vThisClientTextColor;
                pItemBackColor = vThisClientBackColor;
            }
            else if (iClientNum == -2)
            {
                pItemTextColor = vNoTeamTextColor;
                pItemBackColor = vNoTeamBackColor;
            }
            else if (iClientTeam == TEAM_ALLIES || iClientTeam == TEAM_AXIS)
            {
                if (iClientTeam == iMatchTeam)
                {
                    pItemTextColor = vSameTeamTextColor;
                    pItemBackColor = vSameTeamBackColor;
                }
                else
                {
                    pItemTextColor = vOtherTeamTextColor;
                    pItemBackColor = vOtherTeamBackColor;
                }
            }
            else
            {
                pItemTextColor = vNoTeamTextColor;
                pItemBackColor = vNoTeamBackColor;
            }

            if (bIsDead) {
                pItemTextColor = vDeadTextColorDead;
            }
        }
        else
        {
            iClientNum = atoi(cgi.Argv(iCurrentEntry + iDatumCount * i));
            if (iClientNum >= 0)
            {
                strcpy(szString2, cg.clientinfo[iClientNum].name);
                strcpy(szString3, cgi.Argv(1 + iCurrentEntry + iDatumCount * i));
                strcpy(szString4, cgi.Argv(2 + iCurrentEntry + iDatumCount * i));
                strcpy(szString5, cgi.Argv(3 + iCurrentEntry + iDatumCount * i));
                strcpy(szString6, cgi.Argv(4 + iCurrentEntry + iDatumCount * i));
            }
            else
            {
                if (iClientNum == -3)
                {
                    strcpy(szString2, cgi.LV_ConvertString("Players"));
                    bIsHeader = qtrue;
                }
                else if (iClientNum == -2)
                {
                    strcpy(szString2, cgi.LV_ConvertString("Spectators"));
                    bIsHeader = qtrue;
                }
                else
                {
                    // unknown
                    szString2[0] = 0;
                }
                szString3[0] = 0;
                szString4[0] = 0;
                szString5[0] = 0;
                szString6[0] = 0;
            }

            if (iClientNum == cg.snap->ps.clientNum)
            {
                pItemTextColor = vThisClientTextColor;
                pItemBackColor = vThisClientBackColor;
            }
            else
            {
                pItemTextColor = vNoTeamTextColor;
                pItemBackColor = vNoTeamBackColor;
            }
        }

        cgi.UI_SetScoreBoardItem(
            i,
            szString2,
            szString3,
            szString4,
            szString5,
            szString6,
            NULL,
            NULL,
            NULL,
            pItemTextColor,
            pItemBackColor,
            bIsHeader
        );
    }

    cgi.UI_DeleteScoreBoardItems(iEntryCount);
}

void CG_ParseScores()
{
    if (cgi.protocol >= PROTOCOL_MOHTA_MIN) {
        CG_ParseScores_ver_15();
	} else {
		CG_ParseScores_ver_6();
	}
}

void CG_InitScoresAPI(clientGameExport_t* cge)
{
	cge->CG_GetScoreBoardColor = CG_GetScoreBoardColor;
	cge->CG_GetScoreBoardFontColor = CG_GetScoreBoardFontColor;
	cge->CG_GetScoreBoardPosition = CG_GetScoreBoardPosition;
	cge->CG_GetScoreBoardDrawHeader = CG_GetScoreBoardDrawHeader;

	if (cgi.protocol >= PROTOCOL_MOHTA_MIN) {
		cge->CG_GetColumnName = &CG_GetColumnName_ver_15;
    } else {
		cge->CG_GetColumnName = &CG_GetColumnName_ver_6;
	}
}
