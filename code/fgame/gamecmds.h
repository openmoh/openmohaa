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

// gamecmds.h: Game Commands.

#pragma once

#include "g_local.h"

void G_InitConsoleCommands(void);

qboolean G_ProcessClientCommand(gentity_t *ent);

void     G_Say(gentity_t *ent, qboolean team, qboolean arg0);
qboolean G_CameraCmd(gentity_t *ent);
qboolean G_SoundCmd(gentity_t *ent);
qboolean G_SayCmd(gentity_t *ent);
qboolean G_EventListCmd(gentity_t *ent);
qboolean G_PendingEventsCmd(gentity_t *ent);
qboolean G_EventHelpCmd(gentity_t *ent);
qboolean G_DumpEventsCmd(gentity_t *ent);
qboolean G_ClassEventsCmd(gentity_t *ent);
qboolean G_DumpClassEventsCmd(gentity_t *ent);
qboolean G_DumpAllClassesCmd(gentity_t *ent);
qboolean G_ClassListCmd(gentity_t *ent);
qboolean G_ClassTreeCmd(gentity_t *ent);
qboolean G_ShowVarCmd(gentity_t *ent);
qboolean G_RestartCmd(gentity_t *ent);
qboolean G_LevelVarsCmd(gentity_t *ent);
qboolean G_GameVarsCmd(gentity_t* ent);
qboolean G_ScriptCmd(gentity_t* ent);
qboolean G_ReloadMap(gentity_t* ent);
qboolean G_CompileScript(gentity_t *ent);
qboolean G_AddBotCommand(gentity_t *ent);
qboolean G_RemoveBotCommand(gentity_t *ent);
#ifdef _DEBUG
qboolean G_BotCommand(gentity_t *ent);
#endif
