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

// simpleentity.cpp : Simple entity

#include "glb_local.h"
#include "simpleentity.h"
#include "world.h"
#include "level.h"

Event EV_SimpleEntity_GetAngle
(
	"angle",
	EV_DEFAULT,
	NULL,
	NULL,
	"get the angles of the entity using just one value.\n"
	"Gets the yaw of the entity or an up and down\n"
	"direction if newAngle is[ 0 - 359 ] or - 1 or - 2",
	EV_GETTER
);

Event EV_SetAngle
(
	"angle",
	EV_DEFAULT,
	"f",
	"newAngle",
	"set the angles of the entity using just one value.\n"
	"Sets the yaw of the entity or an up and down\n"
	"direction if newAngle is[ 0 - 359 ] or - 1 or - 2",
	EV_NORMAL
);

Event EV_SimpleEntity_SetterAngle
(
	"angle",
	EV_DEFAULT,
	"f",
	"newAngle",
	"set the angles of the entity using just one value.\n"
	"Sets the yaw of the entity or an up and down\n"
	"direction if newAngle is[ 0 - 359 ] or - 1 or - 2",
	EV_SETTER
);

Event EV_SimpleEntity_GetAngles
(
	"angles",
	EV_DEFAULT,
	NULL,
	NULL,
	"get the angles of the entity.",
	EV_GETTER
);

Event EV_SetAngles
(
	"angles",
	EV_DEFAULT,
	"v[0,360][0,360][0,360]",
	"newAngles",
	"Set the angles of the entity to newAngles.",
	EV_NORMAL
);

Event EV_SimpleEntity_SetterAngles
(
	"angles",
	EV_DEFAULT,
	"v[0,360][0,360][0,360]",
	"newAngles",
	"Set the angles of the entity to newAngles.",
	EV_SETTER
);

Event EV_SimpleEntity_GetOrigin
(
	"origin",
	EV_DEFAULT,
	NULL,
	NULL,
	"entity's origin",
	EV_GETTER
);

Event EV_SetOrigin
(
	"origin",
	EV_DEFAULT,
	"v",
	"newOrigin",
	"Set the origin of the entity to newOrigin.",
	EV_NORMAL
);

Event EV_SimpleEntity_SetterOrigin
(
	"origin",
	EV_DEFAULT,
	"v",
	"newOrigin",
	"Set the origin of the entity to newOrigin.",
	EV_SETTER
);

Event EV_SimpleEntity_GetTargetname
(
	"targetname",
	EV_DEFAULT,
	NULL,
	NULL,
	"entity's targetname",
	EV_GETTER
);

Event EV_SimpleEntity_SetTargetname
(
	"targetname",
	EV_DEFAULT,
	"s",
	"targetName",
	"set the targetname of the entity to targetName.",
	EV_NORMAL
);

Event EV_SimpleEntity_SetterTargetname
(
	"targetname",
	EV_DEFAULT,
	"s",
	"targetName",
	"set the targetname of the entity to targetName.",
	EV_SETTER
);

Event EV_SimpleEntity_GetTarget
(
	"target",
	EV_DEFAULT,
	NULL,
	NULL,
	"entity's target",
	EV_GETTER
);

Event EV_SimpleEntity_SetTarget
(
	"target",
	EV_DEFAULT,
	"s",
	"targetname_to_target",
	"target another entity with targetname_to_target.",
	EV_NORMAL
);

Event EV_SimpleEntity_SetterTarget
(
	"target",
	EV_DEFAULT,
	"s",
	"targetname_to_target",
	"target another entity with targetname_to_target.",
	EV_SETTER
);

Event EV_SimpleEntity_Centroid
(
	"centroid",
	EV_DEFAULT,
	NULL,
	NULL,
	"entity's centroid",
	EV_GETTER
);

Event EV_SimpleEntity_ForwardVector
(
	"forwardvector",
	EV_DEFAULT,
	NULL,
	NULL,
	"get the forward vector of angles",
	EV_GETTER
);

Event EV_SimpleEntity_LeftVector
(
	"leftvector",
	EV_DEFAULT,
	NULL,
	NULL,
	"get the left vector of angles",
	EV_GETTER
);

Event EV_SimpleEntity_RightVector
(
	"rightvector",
	EV_DEFAULT,
	NULL,
	NULL,
	"get the right vector of angles",
	EV_GETTER
);

Event EV_SimpleEntity_UpVector
(
	"upvector",
	EV_DEFAULT,
	NULL,
	NULL,
	"get the up vector of angles",
	EV_GETTER
);

void SimpleEntity::SimpleArchive( Archiver& arc )
{
	int index;

	Listener::Archive( arc );

	arc.ArchiveVector( &angles );

	arc.ArchiveString( &target );
	arc.ArchiveString( &targetname );

	if( targetname.length() )
	{
		if( arc.Loading() )
		{
			arc.ArchiveInteger( &index );
			world->AddTargetEntityAt( this, index );
		}
		else
		{
			index = world->GetTargetnameIndex( this );
			arc.ArchiveInteger( &index );
		}
	}
}

void SimpleEntity::Archive( Archiver& arc )
{
	SimpleEntity::SimpleArchive( arc );

	arc.ArchiveVector( &origin );
	arc.ArchiveVector( &centroid );
}

void SimpleEntity::setOrigin( Vector origin )
{
	this->origin = origin;
	this->centroid = origin;
}

void SimpleEntity::setOriginEvent( Vector origin )
{
	setOrigin( origin );
}

void SimpleEntity::setAngles( Vector angles )
{
	this->angles = angles.AnglesMod();
}

SimpleEntity::SimpleEntity()
{
	entflags = 0;
}

SimpleEntity::~SimpleEntity()
{
	if( world ) {
		world->RemoveTargetEntity( this );
	}
}

void SimpleEntity::SetTarget( str target )
{
	this->target = target;
}

void SimpleEntity::SetTargetName( str targetname )
{
	world->RemoveTargetEntity( this );

	this->targetname = targetname;

	world->AddTargetEntity( this );
}

str& SimpleEntity::Target()
{
	return target;
}

str& SimpleEntity::TargetName()
{
	return targetname;
}

SimpleEntity *SimpleEntity::Next( void )
{
	SimpleEntity *ent = world->GetTarget( target, true );

	if( !ent || !ent->isSubclassOf( SimpleEntity ) )
	{
		return NULL;
	}
	else
	{
		return ent;
	}
}

void SimpleEntity::EventGetAngle( Event *ev )
{
	ev->AddFloat( angles[ 1 ] );
}

void SimpleEntity::EventGetAngles( Event *ev )
{
	ev->AddVector( angles );
}

void SimpleEntity::EventGetOrigin( Event *ev )
{
	ev->AddVector( origin );
}

void SimpleEntity::EventGetTargetname( Event *ev )
{
	ev->AddString( TargetName() );
}

void SimpleEntity::EventGetTarget( Event *ev )
{
	ev->AddString( Target() );
}

void SimpleEntity::EventSetAngle( Event *ev )
{
	Vector dir;
	float angle = ev->GetFloat( 1 );

	dir = G_GetMovedir( angle );
	dir.toAngles();

	setAngles( dir );
}

void SimpleEntity::EventSetAngles( Event *ev )
{
	Vector angles;

	if( ev->NumArgs() == 1 )
	{
		angles = ev->GetVector( 1 );
	}
	else
	{
		angles = Vector( ev->GetFloat( 1 ), ev->GetFloat( 2 ), ev->GetFloat( 3 ) );
	}

	setAngles( angles );
}

void SimpleEntity::EventSetOrigin( Event *ev )
{
	setOriginEvent( ev->GetVector( 1 ) );
}

void SimpleEntity::EventSetTargetname( Event *ev )
{
	SetTargetName( ev->GetString( 1 ) );
}

void SimpleEntity::EventSetTarget( Event *ev )
{
	SetTarget( ev->GetString( 1 ) );
}

void SimpleEntity::GetCentroid( Event *ev )
{
	ev->AddVector( centroid );
}

void SimpleEntity::GetForwardVector( Event *ev )
{
	Vector fwd;

	AngleVectorsLeft( angles, fwd, NULL, NULL );
	ev->AddVector( fwd );
}

void SimpleEntity::GetLeftVector( Event *ev )
{
	Vector left;

	AngleVectorsLeft( angles, NULL, left, NULL );
	ev->AddVector( left );
}

void SimpleEntity::GetRightVector( Event *ev )
{
	Vector right;

	AngleVectors( angles, NULL, right, NULL );
	ev->AddVector( right );
}

void SimpleEntity::GetUpVector( Event *ev )
{
	Vector up;

	AngleVectorsLeft( angles, NULL, NULL, up );
	ev->AddVector( up );
}

void SimpleEntity::MPrintf( const char *fmt, ... )
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	if (!*g_monitor->string)
	{
		return;
	}

	if (targetname == g_monitor->string)
	{
		va_start(argptr, fmt);
		Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
		va_end(argptr);
		Com_Printf("%s", msg);
	}
}

int SimpleEntity::IsSubclassOfEntity( void ) const
{
	return ( entflags & EF_ENTITY );
}

int SimpleEntity::IsSubclassOfAnimate( void ) const
{
	return ( entflags & EF_ANIMATE );
}

int SimpleEntity::IsSubclassOfSentient( void ) const
{
	return ( entflags & EF_SENTIENT );
}

int SimpleEntity::IsSubclassOfPlayer( void ) const
{
	return ( entflags & EF_PLAYER );
}

int SimpleEntity::IsSubclassOfActor( void ) const
{
	return ( entflags & EF_ACTOR );
}

int SimpleEntity::IsSubclassOfItem( void ) const
{
	return ( entflags & EF_ITEM );
}

int SimpleEntity::IsSubclassOfInventoryItem( void ) const
{
	return ( entflags & EF_INVENTORYITEM );
}

int SimpleEntity::IsSubclassOfWeapon( void ) const
{
	return ( entflags & EF_WEAPON );
}

int SimpleEntity::IsSubclassOfProjectile( void ) const
{
	return ( entflags & EF_PROJECTILE );
}

int SimpleEntity::IsSubclassOfDoor( void ) const
{
	return ( entflags & EF_DOOR );
}

int SimpleEntity::IsSubclassOfCamera( void ) const
{
	return ( entflags & EF_CAMERA );
}

int SimpleEntity::IsSubclassOfVehicle( void ) const
{
	return ( entflags & EF_VEHICLE );
}

int SimpleEntity::IsSubclassOfVehicleTank( void ) const
{
	return ( entflags & EF_VEHICLETANK );
}

int SimpleEntity::IsSubclassOfVehicleTurretGun( void ) const
{
	return ( entflags & EF_VEHICLETURRET );
}

int SimpleEntity::IsSubclassOfTurretGun( void ) const
{
	return ( entflags & EF_TURRET );
}

int SimpleEntity::IsSubclassOfPathNode( void ) const
{
	return ( entflags & EF_PATHNODE );
}

int SimpleEntity::IsSubclassOfWaypoint( void ) const
{
	return ( entflags & EF_WAYPOINT );
}

int SimpleEntity::IsSubclassOfTempWaypoint( void ) const
{
	return ( entflags & EF_TEMPWAYPOINT );
}

int SimpleEntity::IsSubclassOfVehiclePoint( void ) const
{
	return ( entflags & EF_VEHICLEPOINT );
}

int SimpleEntity::IsSubclassOfSplinePath( void ) const
{
	return ( entflags & EF_SPLINEPATH );
}

int SimpleEntity::IsSubclassOfCrateObject( void ) const
{
	return ( entflags & EF_CRATEOBJECT );
}

int SimpleEntity::IsSubclassOfBot( void ) const
{
	return ( entflags & EF_BOT );
}

CLASS_DECLARATION( Listener, SimpleEntity, NULL )
{
	{ &EV_SimpleEntity_GetAngle,				&SimpleEntity::EventGetAngle },
	{ &EV_SimpleEntity_GetAngles,				&SimpleEntity::EventGetAngles },
	{ &EV_SimpleEntity_GetOrigin,				&SimpleEntity::EventGetOrigin },
	{ &EV_SimpleEntity_GetTargetname,			&SimpleEntity::EventGetTargetname },
	{ &EV_SimpleEntity_GetTarget,				&SimpleEntity::EventGetTarget },
	{ &EV_SimpleEntity_SetterAngle,				&SimpleEntity::EventSetAngle },
	{ &EV_SimpleEntity_SetterAngles,			&SimpleEntity::EventSetAngles },
	{ &EV_SimpleEntity_SetterOrigin,			&SimpleEntity::EventSetOrigin },
	{ &EV_SimpleEntity_SetterTargetname,		&SimpleEntity::EventSetTargetname },
	{ &EV_SimpleEntity_SetterTarget,			&SimpleEntity::EventSetTarget },
	{ &EV_SetAngle,								&SimpleEntity::EventSetAngle },
	{ &EV_SetAngles,							&SimpleEntity::EventSetAngles },
	{ &EV_SetOrigin,							&SimpleEntity::EventSetOrigin },
	{ &EV_SimpleEntity_SetTargetname,			&SimpleEntity::EventSetTargetname },
	{ &EV_SimpleEntity_SetTarget,				&SimpleEntity::EventSetTarget },
	{ &EV_SimpleEntity_Centroid,				&SimpleEntity::GetCentroid },
	{ &EV_SimpleEntity_ForwardVector,			&SimpleEntity::GetForwardVector },
	{ &EV_SimpleEntity_LeftVector,				&SimpleEntity::GetLeftVector },
	{ &EV_SimpleEntity_RightVector,				&SimpleEntity::GetRightVector },
	{ &EV_SimpleEntity_UpVector,				&SimpleEntity::GetUpVector },
	{ NULL, NULL }
};

SimpleArchivedEntity::SimpleArchivedEntity()
{
	level.m_SimpleArchivedEntities.AddObject( this );
}

SimpleArchivedEntity::~SimpleArchivedEntity()
{
	level.m_SimpleArchivedEntities.RemoveObject( this );
}

CLASS_DECLARATION( SimpleEntity, SimpleArchivedEntity, NULL )
{
	{ NULL, NULL }
};
