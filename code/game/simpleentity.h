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

// simpleentity.h: Simple entity.

#ifndef __SIMPLEENTITY_H__
#define __SIMPLEENTITY_H__

#include "listener.h"

extern Event EV_SetAngles;
extern Event EV_SetAngle;
extern Event EV_SetOrigin;

typedef int entflags_t;

class SimpleEntity;
typedef SafePtr< SimpleEntity > SimpleEntityPtr;

class SimpleEntity : public Listener
{
public:
	// Base coord variable
	Vector			origin;
	Vector			angles;

	// Flag (used to set a class flag)
	entflags_t		entflags;

	// Used by scripts
	str				target;
	str				targetname;

	// Centered origin based on mins/maxs
	Vector			centroid;

public:
	CLASS_PROTOTYPE( SimpleEntity );

	SimpleEntity();
	virtual ~SimpleEntity();

	void SimpleArchive( Archiver& arc );
	void Archive( Archiver& arc ) override;

	virtual void setOrigin(Vector origin);
	virtual void setOriginEvent(Vector origin);
	virtual void setAngles(Vector angles);

	void		SetTarget( str target );
	void		SetTargetName( str targetname );
	str&		Target();
	str&		TargetName();

	SimpleEntity	*Next( void );

	int			IsSubclassOfEntity( void ) const;
	int			IsSubclassOfAnimate( void ) const;
	int			IsSubclassOfSentient( void ) const;
	int			IsSubclassOfPlayer( void ) const;
	int			IsSubclassOfActor( void ) const;
	int			IsSubclassOfItem( void ) const;
	int			IsSubclassOfInventoryItem( void ) const;
	int			IsSubclassOfWeapon( void ) const;
	int			IsSubclassOfProjectile( void ) const;
	int			IsSubclassOfDoor( void ) const;
	int			IsSubclassOfCamera( void ) const;
	int			IsSubclassOfVehicle( void ) const;
	int			IsSubclassOfVehicleTank( void ) const;
	int			IsSubclassOfVehicleTurretGun( void ) const;
	int			IsSubclassOfTurretGun( void ) const;
	int			IsSubclassOfPathNode( void ) const;
	int			IsSubclassOfWaypoint( void ) const;
	int			IsSubclassOfTempWaypoint( void ) const;
	int			IsSubclassOfVehiclePoint( void ) const;
	int			IsSubclassOfSplinePath( void ) const;
	int			IsSubclassOfCrateObject( void ) const;
	int			IsSubclassOfBot( void ) const;

	void		EventGetAngle( Event *ev );
	void		EventGetAngles( Event *ev );
	void		EventGetOrigin( Event *ev );
	void		EventGetTargetname( Event *ev );
	void		EventGetTarget( Event *ev );

	void		EventSetAngle( Event *ev );
	void		EventSetAngles( Event *ev );
	void		EventSetOrigin( Event *ev );
	void		EventSetTargetname( Event *ev );
	void		EventSetTarget( Event *ev );

	void		GetCentroid( Event *ev );

	void		GetForwardVector( Event *ev );
	void		GetLeftVector( Event *ev );
	void		GetRightVector( Event *ev );
	void		GetUpVector( Event *ev );

	void		MPrintf( const char *msg, ... );
};

class SimpleArchivedEntity : public SimpleEntity
{
public:
	CLASS_PROTOTYPE( SimpleArchivedEntity );

	SimpleArchivedEntity();
	virtual ~SimpleArchivedEntity();
};

#endif
