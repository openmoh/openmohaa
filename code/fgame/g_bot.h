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

#pragma once

struct saved_bot_t {
    client_persistant_t pers;
    bool bValid;

    saved_bot_t()
        : bValid(false)
    {}
};

void G_BotBegin(gentity_t* ent);
void G_BotThink(gentity_t* ent, int msec);
gentity_t* G_GetFirstBot();
void G_AddBot(unsigned int num, saved_bot_t* saved = NULL);
void G_RemoveBot(unsigned int num);
void G_ResetBots();
void G_SpawnBots();