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

void CG_MessageMode_f(void)
{
    // FIXME: unimplemented
}

void CG_MessageMode_All_f(void)
{
    // FIXME: unimplemented
}

void CG_MessageMode_Team_f(void)
{
    // FIXME: unimplemented
}

void CG_MessageMode_Private_f(void)
{
    // FIXME: unimplemented
}

void CG_MessageSingleAll_f(void)
{
    // FIXME: unimplemented
}

void CG_MessageSingleTeam_f(void)
{
    // FIXME: unimplemented
}

void CG_MessageSingleClient_f(void)
{
    // FIXME: unimplemented
}

void CG_InstaMessageMain_f(void)
{
    // FIXME: unimplemented
}

void CG_InstaMessageGroupA_f(void)
{
    // FIXME: unimplemented
}

void CG_InstaMessageGroupB_f(void)
{
    // FIXME: unimplemented
}

void CG_InstaMessageGroupC_f(void)
{
    // FIXME: unimplemented
}

void CG_InstaMessageGroupD_f(void)
{
    // FIXME: unimplemented
}

void CG_InstaMessageGroupE_f(void)
{
    // FIXME: unimplemented
}

void CG_HudPrint_f(void)
{
    cgi.Printf("\x1%s", cgi.Argv(0));
}

int CG_CheckCaptureKey(int key, qboolean down, unsigned int time)
{
    // FIXME: unimplemented
    return 0;
}
