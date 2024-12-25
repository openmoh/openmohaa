/*
===========================================================================
Copyright (C) 2024 the OpenMoHAA team

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

#pragma once

struct saved_bot_t {
    char userinfo[MAX_INFO_STRING];
};

void       G_BotBegin(gentity_t *ent);
void       G_BotThink(gentity_t *ent, int msec);
void       G_BotShift(int clientNum);
gentity_t *G_GetFirstBot();
void       G_AddBot(const saved_bot_t *saved = NULL);
void       G_AddBots(unsigned int num);
void       G_RemoveBot(gentity_t *ent);
void       G_RemoveBots(unsigned int num);
bool       G_IsBot(gentity_t *ent);
bool       G_IsPlayer(gentity_t *ent);
void       G_ResetBots();
void       G_BotInit();
void       G_BotFrame();
void       G_BotPostInit();
void       G_SpawnBots();
const char* G_GetRandomAlliedPlayerModel();
const char* G_GetRandomGermanPlayerModel();