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
// VehicleSoundENtity.cpp : Make sounds for vehicles

#include "VehicleSoundEntity.h"
#include "weapon.h"
#include "vehicle.h"

Event EV_VehicleSoundEntity_PostSpawn
	(
	"vehiclesoundentity_post",
	EV_DEFAULT,
	NULL,
	NULL,
	"PostSpawn of a Vehicle Sound Entity"
	);

Event EV_VehicleSoundEntity_UpdateTraces
	(
	"vehiclesoudnentity_updatetraces",
	EV_DEFAULT,
	NULL,
	NULL,
	"Updates the traces of a Vehicle Sound Entity"
	);

CLASS_DECLARATION( Entity, VehicleSoundEntity, NULL )
{
	{ &EV_VehicleSoundEntity_PostSpawn,							&VehicleSoundEntity::EventPostSpawn },
	{ &EV_VehicleSoundEntity_UpdateTraces,						&VehicleSoundEntity::EventUpdateTraces },
	{ NULL, NULL }
};

VehicleSoundEntity::VehicleSoundEntity()
{
	if( LoadingSavegame )
	{
		return;
	}

	gi.Error( ERR_DROP, "VehicleSoundEntity Created with no parameters!\n" );
}

VehicleSoundEntity::VehicleSoundEntity( Vehicle *owner )
{
	m_pVehicle = owner;
	m_bDoSoundStuff = false;
	m_iTraceSurfaceFlags = 0;

	PostEvent( EV_VehicleSoundEntity_PostSpawn, EV_POSTSPAWN );
}

void VehicleSoundEntity::Start
	(
	void
	)
{
	m_bDoSoundStuff = true;
}

void VehicleSoundEntity::Stop
	(
	void
	)
{
	m_bDoSoundStuff = false;
}

void VehicleSoundEntity::Think
	(
	void
	)
{
	DoSoundStuff();
}

void VehicleSoundEntity::EventPostSpawn
	(
	Event *ev
	)
{
	setModel( "models/vehicles/vehiclesoundentity.tik" );

	PostEvent( EV_VehicleSoundEntity_UpdateTraces, 1.0f );
	flags |= FL_THINK;
}

void VehicleSoundEntity::EventUpdateTraces
	(
	Event *ev
	)
{
	if( m_bDoSoundStuff )
	{
		m_pVehicle->SetSlotsNonSolid();

		trace_t trace = G_Trace(
			origin,
			vec3_origin,
			vec3_origin,
			Vector( origin[ 0 ], origin[ 1 ], origin[ 2 ] - 256.0f ),
			m_pVehicle->edict,
			MASK_SOUND,
			qfalse,
			"VehicleSoundEntity::DoSoundStuff"
			);

		if( trace.fraction >= 1.0f )
			m_iTraceSurfaceFlags = 0;
		else
			m_iTraceSurfaceFlags = trace.surfaceFlags;

		m_pVehicle->SetSlotsSolid();
	}

	PostEvent( EV_VehicleSoundEntity_UpdateTraces, 1.0f );
}

void VehicleSoundEntity::DoSoundStuff
	(
	void
	)
{
	float pitch;

	if( !m_bDoSoundStuff )
	{
		StopLoopSound();
		return;
	}

	pitch = ( velocity.length() - m_pVehicle->m_fSoundMinSpeed ) / ( m_pVehicle->m_fSoundMaxSpeed - m_pVehicle->m_fSoundMinSpeed );
	if( pitch > 1.0f ) {
		pitch = 1.0f;
	} else if( pitch < 0.0f ) {
		pitch = 0.0f;
	}

	pitch *= m_pVehicle->m_fSoundMinPitch + ( m_pVehicle->m_fSoundMaxPitch - m_pVehicle->m_fSoundMinPitch );

	if( m_iTraceSurfaceFlags & SURF_DIRT )
	{
		LoopSound( m_pVehicle->m_sSoundSet + "treat_snd_dirt", -1.0f, -1.0f, -1.0f, pitch );
	}
	else if( m_iTraceSurfaceFlags & SURF_GRASS )
	{
		LoopSound( m_pVehicle->m_sSoundSet + "treat_snd_grass", -1.0f, -1.0f, -1.0f, pitch );
	}
	else if( m_iTraceSurfaceFlags & SURF_WOOD )
	{
		LoopSound( m_pVehicle->m_sSoundSet + "treat_snd_wood", -1.0f, -1.0f, -1.0f, pitch );
	}
	else if( m_iTraceSurfaceFlags & SURF_MUD )
	{
		LoopSound( m_pVehicle->m_sSoundSet + "treat_snd_mud", -1.0f, -1.0f, -1.0f, pitch );
	}
	else if( m_iTraceSurfaceFlags & ( SURF_GRAVEL | SURF_ROCK ) )
	{
		LoopSound( m_pVehicle->m_sSoundSet + "treat_snd_stone", -1.0f, -1.0f, -1.0f, pitch );
	}
	else
	{
		StopLoopSound();
	}
}
