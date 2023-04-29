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
// Sound function

#include "cg_local.h"

void CG_ProcessSound( server_sound_t *sound )
	{
	if ( sound->stop_flag )
		{
		cgi.S_StopSound( sound->entity_number, sound->channel );
		}
	else
		{
		cgi.S_StartSound( sound->origin, sound->entity_number, sound->channel, cgs.sound_precache[sound->sound_index], 
				sound->volume, sound->min_dist, sound->pitch, sound->maxDist, sound->streamed );
		}
	}
