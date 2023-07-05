/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

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
// g_bot.cpp

#include "g_local.h"
#include "entity.h"
#include "playerbot.h"

void G_BotBegin
	(
	gentity_t *ent
	)
{
	level.m_bSpawnBot = true;
	G_ClientBegin( ent, NULL );
}

void G_BotThink
	(
	gentity_t *ent,
	int msec
	)
{
	usercmd_t ucmd;
	usereyes_t eyeinfo;
	PlayerBot *bot;

	assert( ent );
	assert( ent->entity );
	assert( ent->entity->IsSubclassOfBot() );

	bot = ( PlayerBot * )ent->entity;

	bot->UpdateBotStates();
	bot->GetUsercmd( &ucmd );
	bot->GetEyeInfo( &eyeinfo );

	G_ClientThink( ent, &ucmd, &eyeinfo );
}
