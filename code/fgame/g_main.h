/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// g_main.h -- Global header file for g_main.cpp

#ifndef __G_MAIN_H__
#define __G_MAIN_H__

#include "g_local.h"
#include "gamecvars.h"

class Player;

extern qboolean      LoadingSavegame;
extern qboolean      LoadingServer;
extern game_import_t gi;
extern game_export_t globals;
extern int           g_protocol;

extern qboolean g_iInThinks;
extern qboolean g_bBeforeThinks;

extern gentity_t active_edicts;
extern gentity_t free_edicts;

extern int sv_numtraces;
extern int sv_numpmtraces;

extern usercmd_t  *current_ucmd;
extern usereyes_t *current_eyeinfo;
extern Player     *g_pPlayer;

void G_ExitWithError(const char *error);
void G_AllocGameData(void);
void G_DeAllocGameData(void);
void G_ClientDrawBoundingBoxes(void);
void G_ClientDrawTags(void);

#endif /* g_main.h */
