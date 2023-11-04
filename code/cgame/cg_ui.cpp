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
// UI features

#include "cg_local.h"
#include "str.h"

void CG_MessageMode_f(void)
{
    if (!cgs.gametype) {
        return;
    }

    cgi.UI_ToggleDMMessageConsole(300);
}

void CG_MessageMode_All_f(void)
{
    if (!cgs.gametype) {
        return;
    }

    cgi.UI_ToggleDMMessageConsole(100);
}

void CG_MessageMode_Team_f(void)
{
    if (!cgs.gametype) {
        return;
    }

    cgi.UI_ToggleDMMessageConsole(200);
}

void CG_MessageMode_Private_f(void)
{
    int clientNum;

    if (!cgs.gametype) {
        return;
    }

    clientNum = atoi(cgi.Argv(1)) - 1;
    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
        cgi.Printf(HUD_MESSAGE_CHAT_WHITE "Message Error: %s is a bad client number\n", cgi.Argv(1));
        return;
    }

    cgi.UI_ToggleDMMessageConsole(clientNum);
}

void CG_MessageSingleAll_f(void)
{
    if (!cgs.gametype) {
        return;
    }

    if (cgi.Argc() > 1) {
        cgi.SendClientCommand(va("dmmessage 0 %s\n", cgi.Args()));
    } else {
        cgi.UI_ToggleDMMessageConsole(-100);
    }
}

void CG_MessageSingleTeam_f(void)
{
    if (!cgs.gametype) {
        return;
    }

    if (cgi.Argc() > 1) {
        cgi.SendClientCommand(va("dmmessage -1 %s\n", cgi.Args()));
    } else {
        cgi.UI_ToggleDMMessageConsole(-200);
    }
}

void CG_MessageSingleClient_f(void)
{
    int clientNum;

    if (!cgs.gametype) {
        return;
    }

    clientNum = atoi(cgi.Argv(1));
    if (clientNum < 1 || clientNum > MAX_CLIENTS) {
        cgi.Printf(HUD_MESSAGE_CHAT_WHITE "Message Error: %s is a bad client number\n", cgi.Argv(1));
        return;
    }

    if (cgi.Argc() > 2) {
        int i;
        str sString;

        sString = "dmmessage ";
        sString += va("%i", clientNum);

        // copy the rest
        for (i = 2; i < cgi.Argc(); i++) {
            sString += va("%s", cgi.Argv(i));
        }

        sString += "\n";
        cgi.SendClientCommand(sString.c_str());
    } else {
        cgi.UI_ToggleDMMessageConsole(-clientNum);
    }
}

void CG_InstaMessageMain_f(void)
{
    if (!voiceChat->integer) {
        return;
    }

    if (!cgs.gametype) {
        return;
    }

    cg.iInstaMessageMenu = -1;
}

void CG_InstaMessageGroupA_f(void)
{
    if (!cgs.gametype) {
        return;
    }

    cg.iInstaMessageMenu = 1;
}

void CG_InstaMessageGroupB_f(void)
{
    if (!cgs.gametype) {
        return;
    }

    cg.iInstaMessageMenu = 2;
}

void CG_InstaMessageGroupC_f(void)
{
    if (!cgs.gametype) {
        return;
    }

    cg.iInstaMessageMenu = 3;
}

void CG_InstaMessageGroupD_f(void)
{
    if (!cgs.gametype) {
        return;
    }

    cg.iInstaMessageMenu = 4;
}

void CG_InstaMessageGroupE_f(void)
{
    if (!cgs.gametype) {
        return;
    }

    cg.iInstaMessageMenu = 5;
}

void CG_HudPrint_f(void)
{
    cgi.Printf("\x1%s", cgi.Argv(1));
}

int CG_CheckCaptureKey(int key, qboolean down, unsigned int time)
{
    // FIXME: unimplemented
    return 0;
}
