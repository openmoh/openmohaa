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
// VehicleSoundEntity.h : Sound helper for vehicles

#ifndef __VEHICLESOUNDENTITY_H__
#define __VEHICLESOUNDENTITY_H__

#include "entity.h"

class Vehicle;

class VehicleSoundEntity : public Entity {
private:
	SafePtr<Vehicle> m_pVehicle;
	bool m_bDoSoundStuff;
	int m_iTraceSurfaceFlags;

public:
	CLASS_PROTOTYPE( VehicleSoundEntity );

	VehicleSoundEntity();
	VehicleSoundEntity( Vehicle *owner );

	void			Start( void );
	void			Stop( void );
	void Think( void ) override;
	void Archive( Archiver& arc ) override;

private:
	void			EventPostSpawn( Event *ev );
	void			EventUpdateTraces( Event *ev );
	void			DoSoundStuff( void );
};

inline
void VehicleSoundEntity::Archive
	(
	Archiver& arc
	)
{
	Entity::Archive( arc );

	arc.ArchiveSafePointer( &m_pVehicle );
	arc.ArchiveBool( &m_bDoSoundStuff );
	arc.ArchiveInteger( &m_iTraceSurfaceFlags );
}

#endif // __VEHICLECOLLISIONENTITY_H__