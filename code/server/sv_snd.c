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

// sv_snd.c: Server sound

#include "server.h"

/*
===============
SV_Sound
===============
*/
void SV_Sound( vec3_t *org, int entnum, int channel, const char *sound_name, float volume, float mindist, float pitch, float maxdist, qboolean streamed )
{
	int i;

	for( i = 0; i < sv_maxclients->integer; i++ )
	{
		client_t *client = &svs.clients[ i ];
		server_sound_t *sound;

		if( client->state != CS_ACTIVE )
			continue;

		if( client->number_of_server_sounds >= MAX_SERVER_SOUNDS )
			continue;

		sound = &client->server_sounds[ client->number_of_server_sounds ];
		sound->stop_flag = 0;
		sound->entity_number = entnum;

		if( org )
		{
			VectorCopy( *org, sound->origin );
		}
		else
		{
			VectorClear( sound->origin );
		}

		sound->channel = channel;
		sound->volume = volume;
		sound->min_dist = mindist;
		sound->pitch = pitch;
		sound->maxDist = maxdist;
		sound->sound_index = SV_SoundIndex( sound_name, streamed );
		sound->streamed = streamed;
		client->number_of_server_sounds++;
	}
}

/*
===============
SV_ClearSounds
===============
*/
void SV_ClearSounds( client_t *client )
{
	client->number_of_server_sounds = 0;
}

/*
===============
SV_StopSound
===============
*/
void SV_StopSound( int entnum, int channel )
{
	server_sound_t *server_sound;
	int i;
	client_t *client;

	for( client = svs.clients, i = 0; i < svs.iNumClients; client++, i++ )
	{
		if( client->state == CS_ACTIVE && client->number_of_server_sounds < MAX_SERVER_SOUNDS )
		{
			server_sound = &client->server_sounds[ client->number_of_server_sounds ];
			server_sound->stop_flag = qtrue;
			server_sound->entity_number = entnum;
			server_sound->channel = channel;
			client->number_of_server_sounds++;
		}
	}
}
