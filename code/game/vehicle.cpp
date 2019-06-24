/*
===========================================================================
Copyright (C) 2018 the OpenMoHAA team

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

// vehicle.cpp: Script controlled Vehicles.
//

#include "g_local.h"
#include "scriptslave.h"
#include "vehicle.h"
#include "player.h"
#include "specialfx.h"
#include "explosion.h"
#include "earthquake.h"
#include "gibs.h"
#include "vehicleturret.h"

Event EV_Vehicle_Start
	(
	"start",
	EV_DEFAULT,
	NULL,
	NULL,
	"Initialize the vehicle."
	);

Event EV_Vehicle_Enter
	(
	"enter",
	EV_DEFAULT,
	"eS",
	"vehicle driver_anim",
	"Called when someone gets into a vehicle."
	);

Event EV_Vehicle_Exit
	(
	"exit",
	EV_DEFAULT,
	"e",
	"entity",
	"Called when driver gets out of the vehicle."
	);

Event EV_Vehicle_Drivable
	(
	"drivable",
	EV_DEFAULT,
	NULL,
	NULL,
	"Make the vehicle drivable"
	);

Event EV_Vehicle_UnDrivable
	(
	"undrivable",
	EV_DEFAULT,
	NULL,
	NULL,
	"Make the vehicle undrivable"
	);

Event EV_Vehicle_Jumpable
	(
	"canjump",
	EV_DEFAULT,
	"b",
	"jumpable",
	"Sets whether or not the vehicle can jump"
	);

Event EV_Vehicle_Lock
	(
	"lock",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the vehicle to be locked"
	);

Event EV_Vehicle_UnLock
	(
	"unlock",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the vehicle to be unlocked"
	);

Event EV_Vehicle_SeatAnglesOffset
	(
	"seatangles",
	EV_DEFAULT,
	"v",
	"angles",
	"Set the angles offset of the seat"
	);

Event EV_Vehicle_SeatOffset
	(
	"seatoffset",
	EV_DEFAULT,
	"v",
	"offset",
	"Set the offset of the seat"
	);


Event EV_Vehicle_SetWeapon
	(
	"setweapon",
	EV_DEFAULT,
	"s",
	"weaponname",
	"Set the weapon for the vehicle"
	);
	
Event EV_Vehicle_ShowWeapon
	(
	"showweapon",
	EV_DEFAULT,
	NULL,
	NULL,
	"Set the weapon to be show in the view"
	);
	
Event EV_Vehicle_SetSpeed
	(
	"vehiclespeed",
	EV_DEFAULT,
	"f",
	"speed",
	"Set the speed of the vehicle"
	);
	
Event EV_Vehicle_SetTurnRate
	(
	"turnrate",
	EV_DEFAULT,
	"f",
	"rate",
	"Set the turning rate of the vehicle"
	);
	
Event EV_Vehicle_SteerInPlace
	(
	"steerinplace",
	EV_DEFAULT,
	NULL,
	NULL,
	"Set the vehicle to turn in place"
	);
	
Event EV_Vehicle_Destroyed
	(
	"vehicledestroyed",
	EV_DEFAULT,
	NULL,
	NULL,
	"Driver is dead"
	);
	
Event EV_Vehicle_Mass
	(
	"vehiclemass",
	EV_DEFAULT,
	"f",
	"weight",
	"Sets the mass of the vehicle (backmass = frontmass = mass/2)"
	);
	
Event EV_Vehicle_Front_Mass
	(
	"front_mass",
	EV_DEFAULT,
	"f",
	"weight",
	"Sets the mass of the front of the vehicle"
	);
	
Event EV_Vehicle_Back_Mass
	(
	"back_mass",
	EV_DEFAULT,
	"f",
	"weight",
	"Sets the mass of the back of the vehicle"
	);
	
Event EV_Vehicle_Tread
	(
	"vehicletread",
	EV_DEFAULT,
	"f",
	"size",
	"Sets the size of the wheels"
	);

Event EV_Vehicle_Radius
	(
	"vehicleradius",
	EV_DEFAULT,
	"f",
	"size",
	"Sets the radius of the wheels"
	);
	
Event EV_Vehicle_RollingResistance
	(
	"vehiclerollingresistance",
	EV_DEFAULT,
	"f",
	"size",
	"Sets the radius of the wheels"
	);
	
Event EV_Vehicle_Drag
	(
	"vehicledrag",
	EV_DEFAULT,
	"f",
	"size",
	"Sets the Drag Factor"
	);
	
Event EV_Vehicle_Drive
	(
	"drive",
	EV_DEFAULT,
	"vffffV",
	"position speed acceleration reach_distance look_ahead alternate_position",
	"Makes the vehicle drive to position with speed and acceleration until reached_distance close to position"
	);
	
Event EV_Vehicle_DriveNoWait
	(
	"driveNo",
	EV_DEFAULT,
	"vfff",
	"position speed acceleration reach_distance",
	"Makes the vehicle drive to position with speed and acceleration until reached_distance close to position, thread doesn't wait"
	);
	
Event EV_Vehicle_Stop
	(
	"stop",
	EV_DEFAULT,
	NULL,
	NULL,
	"Make the Vehicle Stop Moving... FULL BREAKS!"
	);
	
Event EV_Vehicle_FullStop
	(
	"fullstop",
	EV_DEFAULT,
	NULL,
	NULL,
	"Make the Vehicle Stop Moving... Completely!"
	);
	
Event EV_Vehicle_Init
	(
	"vehicleinit",
	EV_DEFAULT,
	NULL,
	NULL,
	"Initialized the Vehicle as the specified file"
	);
	
Event EV_Vehicle_BouncyCoef
	(
	"vehiclebouncy",
	EV_DEFAULT,
	"f",
	"bouncycoef",
	"Sets the Bouncy Coefficient for the shocks."
	);
	
Event EV_Vehicle_SpringyCoef
	(
	"vehiclespringy",
	EV_DEFAULT,
	"f",
	"springycoef",
	"Sets the Springy Coefficient for the shocks."
	);
	
Event EV_Vehicle_Yaw
	(
	"vehicleYaw",
	EV_DEFAULT,
	"fff",
	"min max coef",
	"Sets the Yaw min and max and the acceleration coefficient for the shocks."
	);
	
Event EV_Vehicle_Roll
	(
	"vehicleRoll",
	EV_DEFAULT,
	"fff",
	"min max coef",
	"Sets the Roll min and max and the acceleration coefficient for the shocks."
	);
	
Event EV_Vehicle_Z
	(
	"vehicleZ",
	EV_DEFAULT,
	"fff",
	"min max coef",
	"Sets the Z min and max and the acceleration coefficient for the shocks."
	);
	
Event EV_Vehicle_QueryFreePassengerSlot
	(
	"QueryFreePassengerSlot",
	EV_DEFAULT,
	NULL,
	NULL,
	"Returns a number that represents the first free passenger slot on the vehicle.",
	EV_RETURN );
	
Event EV_Vehicle_QueryPassengerSlotPosition
	(
	"QueryPassengerSlotPosition",
	EV_DEFAULT,
	"i",
	"slot",
	"Returns the position of the specified slot on the vehicle.",
	EV_RETURN );
	
Event EV_Vehicle_QueryPassengerSlotStatus
	(
	"QueryPassengerSlotStatus",
	EV_DEFAULT,
	"i",
	"slot",
	"Returns the status of the specified slot on the vehicle.",
	EV_RETURN );
	
Event EV_Vehicle_AttachPassengerSlot
	(
	"AttachPassengerSlot",
	EV_DEFAULT,
	"ie",
	"slot entity",
	"Attaches an entity to the specified slot."
	);
	
Event EV_Vehicle_QueryPassengerSlotEntity
	(
	"QueryPassengerSlotEntity",
	EV_DEFAULT,
	"i",
	"slot",
	"Returns an entity at the specified slot.",
	EV_RETURN );
	
Event EV_Vehicle_DetachPassengerSlot
	(
	"DetachPassengerSlot",
	EV_DEFAULT,
	"iV",
	"slot exit_",
	"Detaches an entity to the specified slot."
	);
	
Event EV_Vehicle_QueryFreeDriverSlot
	(
	"QueryFreeDriverSlot",
	EV_DEFAULT,
	NULL,
	NULL,
	"Returns a number that represents the first free driver slot on the vehicle.",
	EV_RETURN );
	
Event EV_Vehicle_QueryDriverSlotPosition
	(
	"QueryDriverSlotPosition",
	EV_DEFAULT,
	"i",
	"slot",
	"Returns the position of the specified slot on the vehicle.",
	EV_RETURN );
	
Event EV_Vehicle_QueryDriverSlotStatus
	(
	"QueryDriverSlotStatus",
	EV_DEFAULT,
	"i",
	"slot",
	"Returns the status of the specified slot on the vehicle.",
	EV_RETURN );
	
Event EV_Vehicle_AttachDriverSlot
	(
	"AttachDriverSlot",
	EV_DEFAULT,
	"ie",
	"slot entity",
	"Attaches an entity to the specified slot."
	);
	
Event EV_Vehicle_QueryDriverSlotEntity
	(
	"QueryDriverSlotEntity",
	EV_DEFAULT,
	"i",
	"slot",
	"Returns an entity at the specified slot.",
	EV_RETURN );
	
Event EV_Vehicle_DetachDriverSlot
	(
	"DetachDriverSlot",
	EV_DEFAULT,
	"iV",
	"slot exit_",
	"Detaches an entity to the specified slot."
	);
	
Event EV_Vehicle_QueryFreeTurretSlot
	(
	"QueryFreeTurretSlot",
	EV_DEFAULT,
	NULL,
	NULL,
	"Returns a number that represents the first free turret slot on the vehicle.",
	EV_RETURN );
	
Event EV_Vehicle_QueryTurretSlotPosition
	(
	"QueryTurretSlotPosition",
	EV_DEFAULT,
	"i",
	"slot",
	"Returns the position of the specified slot on the vehicle.",
	EV_RETURN );
	
Event EV_Vehicle_QueryTurretSlotStatus
	(
	"QueryTurretSlotStatus",
	EV_DEFAULT,
	"i",
	"slot",
	"Returns the status of the specified slot on the vehicle.",
	EV_RETURN );
	
Event EV_Vehicle_AttachTurretSlot
	(
	"AttachTurretSlot",
	EV_DEFAULT,
	"ie",
	"slot entity",
	"Attaches an entity to the specified slot."
	);
	
Event EV_Vehicle_QueryTurretSlotEntity
	(
	"QueryTurretSlotEntity",
	EV_DEFAULT,
	"i",
	"slot",
	"Returns an entity at the specified slot.",
	EV_RETURN );
	
Event EV_Vehicle_DetachTurretSlot
	(
	"DetachTurretSlot",
	EV_DEFAULT,
	"iV",
	"slot exit_",
	"Detaches an entity to the specified slot."
	);
	
Event EV_Vehicle_WheelCorners
	(
	"VehicleWheelCorners",
	EV_DEFAULT,
	"vv",
	"size offset",
	"Sets the wheel trace corners."
	);
	
Event EV_Vehicle_QueryDriverSlotAngles
	(
	"QueryDriverSlotAngles",
	EV_DEFAULT,
	"i",
	"slot",
	"Returns the angles of the specified slot on the vehicle.",
	EV_RETURN );
	
Event EV_Vehicle_QueryPassengerSlotAngles
	(
	"QueryDriverSlotAngles",
	EV_DEFAULT,
	"i",
	"slot",
	"Returns the angles of the specified slot on the vehicle.",
	EV_RETURN );
	
Event EV_Vehicle_QueryTurretSlotAngles
	(
	"QueryDriverSlotAngles",
	EV_DEFAULT,
	"i",
	"slot",
	"Returns the angles of the specified slot on the vehicle.",
	EV_RETURN );
	
Event EV_Vehicle_AnimationSet
	(
	"AnimationSet",
	EV_DEFAULT,
	"s",
	"animset",
	"Sets the Animation Set to use."
	);
	
Event EV_Vehicle_SoundSet
	(
	"SoundSet",
	EV_DEFAULT,
	"s",
	"soundset",
	"Sets the Sound Set to use."
	);
	
Event EV_Vehicle_SpawnTurret
	(
	"spawnturret",
	EV_DEFAULT,
	"is",
	"slot tikifile",
	"Spawns a turret with the specified model and connects it to the specified slot"
	);
	
Event EV_Vehicle_ModifyDrive
	(
	"modifydrive",
	EV_DEFAULT,
	"fff",
	"desired_speed acceleration look_ahead",
	"Modifys the parameters of the current drive."
	);
	
Event EV_Vehicle_NextDrive
	(
	"nextdrive",
	EV_DEFAULT,
	"e",
	"next_path",
	"appends the specified path to the current path"
	);
	
Event EV_Vehicle_StopAtEnd
	(
	"stopatend",
	EV_DEFAULT,
	NULL,
	NULL,
	"Makes the vehicle slow down to a complete stop at the end of the path."
	);
	
Event EV_Vehicle_LockMovement
	(
	"lockmovement",
	EV_DEFAULT,
	NULL,
	NULL,
	"The Vehicle cannot move."
	);
	
Event EV_Vehicle_UnlockMovement
	(
	"unlockmovement",
	EV_DEFAULT,
	NULL,
	NULL,
	"The Vehicle can move again."
	);
	
Event EV_Vehicle_RemoveOnDeath
	(
	"removeondeath",
	EV_DEFAULT,
	"i",
	"removeondeath",
	"If set to a non-zero value, vehicles will not be removed when they die"
	);
	
Event EV_Vehicle_SetExplosionModel
	(
	"explosionmodel",
	EV_DEFAULT,
	"s",
	"model",
	"Sets the TIKI to call when the vehicle dies."
	);
	
Event EV_Vehicle_SetCollisionEntity
	(
	"setcollisionentity",
	EV_DEFAULT,
	"e",
	"entity",
	"Sets the Collision Entity."
	);
	
Event EV_Vehicle_CollisionEntitySetter
	(
	"collisionent",
	EV_DEFAULT,
	"e",
	"entity",
	"Gets the Collision Entity",
	EV_SETTER );
	
Event EV_Vehicle_CollisionEntityGetter
	(
	"collisionent",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the Collision Entity",
	EV_GETTER );
	
Event EV_Vehicle_SetSoundParameters
	(
	"setsoundparameters",
	EV_DEFAULT,
	"ffff",
	"min_speed min_pitch max_speed max_pitch",
	"Sets the Sound parameters for this vehicle"
	);
	
Event EV_Vehicle_SetVolumeParameters
	(
	"setvolumeparameters",
	EV_DEFAULT,
	"ffff",
	"min_speed min_volume max_speed max_volume",
	"Sets the Volume parameters for this vehicle"
	);
	
Event EV_Vehicle_Skidding
	(
	"skidding",
	EV_DEFAULT,
	"i",
	"on_off",
	"Makes the vehicle skid around corners."
	);
	
Event EV_Vehicle_ContinueSkidding
	(
	"_continue",
	EV_DEFAULT,
	NULL,
	NULL,
	"Continues the skidding animation of a vehicle."
	);
	
Event EV_Vehicle_VehicleAnim
	(
	"vehicleanim",
	EV_DEFAULT,
	"sF",
	"anim_name weight",
	"Sets an animation to use in the LD Animation slot. \nWeight defaults to 1.0"
	);
	
Event EV_Vehicle_VehicleAnimDone
	(
	"_vehicleanimdone",
	EV_DEFAULT,
	NULL,
	NULL,
	"For Internal Use Only"
	);

cvar_t *g_showvehiclemovedebug;
cvar_t *g_showvehicleentrypoints;
cvar_t *g_showvehicleslotpoints;
cvar_t *g_showvehiclepath;

CLASS_DECLARATION( Animate, VehicleBase, NULL )
{
	{ NULL, NULL }
};

VehicleBase::VehicleBase()
{
	offset = "0 0 0";

	if( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}

	takedamage = DAMAGE_NO;
	edict->s.renderfx &= ~RF_DONTDRAW;
	edict->r.svFlags &= ~SVF_NOCLIENT;

	//
	// rotate the mins and maxs for the model
	//
	setSize( mins, maxs );

	vlink = NULL;
	offset = vec_zero;

	PostEvent( EV_BecomeNonSolid, EV_POSTSPAWN );
}

CLASS_DECLARATION( VehicleBase, Vehicle, "script_vehicle" )
{
		{ &EV_Blocked,									&Vehicle::VehicleBlocked },
		{ &EV_Touch,									&Vehicle::VehicleTouched },
		{ &EV_Use,										&Vehicle::DriverUse },
		{ &EV_Vehicle_Start,							&Vehicle::VehicleStart },
		{ &EV_Vehicle_Drivable,							&Vehicle::Drivable },
		{ &EV_Vehicle_UnDrivable,						&Vehicle::UnDrivable },
		{ &EV_Vehicle_Jumpable,							&Vehicle::Jumpable },
		{ &EV_Vehicle_SeatAnglesOffset,					&Vehicle::SeatAnglesOffset },
		{ &EV_Vehicle_SeatOffset,						&Vehicle::SeatOffset },
		{ &EV_Vehicle_Lock,								&Vehicle::Lock },
		{ &EV_Vehicle_UnLock,							&Vehicle::UnLock },
		{ &EV_Vehicle_SetWeapon,						&Vehicle::SetWeapon },
		{ &EV_Vehicle_SetSpeed,							&Vehicle::SetSpeed },
		{ &EV_Vehicle_SetTurnRate,						&Vehicle::SetTurnRate },
		{ &EV_Vehicle_SteerInPlace,						&Vehicle::SteerInPlace },
		{ &EV_Vehicle_ShowWeapon,						&Vehicle::ShowWeaponEvent },
		{ &EV_Damage,									&Vehicle::EventDamage },
		{ &EV_Vehicle_Destroyed,						&Vehicle::VehicleDestroyed },
		{ &EV_Vehicle_Mass,								&Vehicle::SetMass },
		{ &EV_Vehicle_Front_Mass,						&Vehicle::SetFrontMass },
		{ &EV_Vehicle_Back_Mass,						&Vehicle::SetBackMass },
		{ &EV_Vehicle_Tread,							&Vehicle::SetTread },
		{ &EV_Vehicle_RollingResistance,				&Vehicle::SetRollingResistance },
		{ &EV_Vehicle_Drag,								&Vehicle::SetDrag },
		{ &EV_Vehicle_Drive,							&Vehicle::EventDrive },
		{ &EV_Vehicle_DriveNoWait,						&Vehicle::EventDriveNoWait },
		{ &EV_Vehicle_Stop,								&Vehicle::EventStop },
		{ &EV_Vehicle_FullStop,							&Vehicle::EventFullStop },
		{ &EV_Vehicle_BouncyCoef,						&Vehicle::BouncyCoef },
		{ &EV_Vehicle_SpringyCoef,						&Vehicle::SpringyCoef },
		{ &EV_Vehicle_Yaw,								&Vehicle::GetYaw },
		{ &EV_Vehicle_Roll,								&Vehicle::RollMinMax },
		{ &EV_Vehicle_Z,								&Vehicle::ZMinMax },
		{ &EV_Vehicle_QueryFreePassengerSlot,			&Vehicle::QueryFreePassengerSlot },
		{ &EV_Vehicle_QueryFreeDriverSlot,				&Vehicle::QueryFreeDriverSlot },
		{ &EV_Vehicle_QueryFreeTurretSlot,				&Vehicle::QueryFreeTurretSlot },
		{ &EV_Vehicle_QueryPassengerSlotPosition,		&Vehicle::QueryPassengerSlotPosition },
		{ &EV_Vehicle_QueryDriverSlotPosition,			&Vehicle::QueryDriverSlotPosition },
		{ &EV_Vehicle_QueryTurretSlotPosition,			&Vehicle::QueryTurretSlotPosition },
		{ &EV_Vehicle_QueryPassengerSlotStatus,			&Vehicle::QueryPassengerSlotStatus },
		{ &EV_Vehicle_QueryDriverSlotStatus,			&Vehicle::QueryDriverSlotStatus },
		{ &EV_Vehicle_QueryTurretSlotStatus,			&Vehicle::QueryTurretSlotStatus },
		{ &EV_Vehicle_QueryPassengerSlotEntity,			&Vehicle::QueryPassengerSlotEntity },
		{ &EV_Vehicle_QueryDriverSlotEntity,			&Vehicle::QueryDriverSlotEntity },
		{ &EV_Vehicle_QueryTurretSlotEntity,			&Vehicle::QueryTurretSlotEntity },
		{ &EV_Vehicle_QueryDriverSlotAngles,			&Vehicle::QueryDriverSlotAngles },
		{ &EV_Vehicle_QueryPassengerSlotAngles,			&Vehicle::QueryPassengerSlotAngles },
		{ &EV_Vehicle_QueryTurretSlotAngles,			&Vehicle::QueryTurretSlotAngles },
		{ &EV_Vehicle_AttachPassengerSlot,				&Vehicle::AttachPassengerSlot },
		{ &EV_Vehicle_AttachDriverSlot,					&Vehicle::AttachDriverSlot },
		{ &EV_Vehicle_AttachTurretSlot,					&Vehicle::AttachTurretSlot },
		{ &EV_Vehicle_DetachPassengerSlot,				&Vehicle::DetachPassengerSlot },
		{ &EV_Vehicle_DetachDriverSlot,					&Vehicle::DetachDriverSlot },
		{ &EV_Vehicle_DetachTurretSlot,					&Vehicle::DetachTurretSlot },
		{ &EV_Vehicle_WheelCorners,						&Vehicle::SetWheelCorners },
		{ &EV_Vehicle_AnimationSet,						&Vehicle::SetAnimationSet },
		{ &EV_Vehicle_SoundSet,							&Vehicle::SetSoundSet },
		{ &EV_Vehicle_ModifyDrive,						&Vehicle::EventModifyDrive },
		{ &EV_Model,									&Vehicle::EventModel },
		{ &EV_Vehicle_NextDrive,						&Vehicle::EventNextDrive },
		{ &EV_Vehicle_LockMovement,						&Vehicle::EventLockMovement },
		{ &EV_Vehicle_UnlockMovement,					&Vehicle::EventUnlockMovement },
		{ &EV_Vehicle_RemoveOnDeath,					&Vehicle::EventRemoveOnDeath },
		{ &EV_Vehicle_SetExplosionModel,				&Vehicle::EventSetExplosionModel },
		{ &EV_Vehicle_SetCollisionEntity,				&Vehicle::EventSetCollisionModel },
		{ &EV_Vehicle_SetSoundParameters,				&Vehicle::EventSetSoundParameters },
		{ &EV_Vehicle_SetVolumeParameters,				&Vehicle::EventSetVolumeParameters },
		{ &EV_Vehicle_StopAtEnd,						&Vehicle::EventStopAtEnd },
		{ &EV_Vehicle_Skidding,							&Vehicle::EventSkidding },
		{ &EV_Vehicle_ContinueSkidding,					&Vehicle::EventContinueSkidding },
		{ &EV_Vehicle_CollisionEntitySetter,			&Vehicle::EventSetCollisionModel },
		{ &EV_Vehicle_CollisionEntityGetter,			&Vehicle::EventGetCollisionModel },
		{ &EV_Vehicle_VehicleAnim,						&Vehicle::EventVehicleAnim },
		{ &EV_Vehicle_VehicleAnimDone,					&Vehicle::EventVehicleAnimDone },
		{ NULL, NULL }
};

/*
====================
Vehicle::Vehicle
====================
*/
Vehicle::Vehicle()
{
	entflags |= EF_VEHICLE;

	g_showvehiclemovedebug = gi.Cvar_Get( "g_showvehiclemovedebug", "0", 0 );
	g_showvehicleentrypoints = gi.Cvar_Get( "g_showvehicleentrypoints", "0", 0 );
	g_showvehicleslotpoints = gi.Cvar_Get( "g_showvehicleslotpoints", "0", 0 );
	g_showvehiclepath = gi.Cvar_Get( "g_showvehiclepath", "0", 0 );

	if( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}

	edict->s.eType = ET_VEHICLE;

	takedamage = DAMAGE_YES;
	seatangles = vec_zero;
	driveroffset = Vector( 0, 0, 50 );
	seatoffset = vec_zero;
	currentspeed = 0;
	turnangle = 0;
	turnimpulse = 0;
	moveimpulse = 0;
	jumpimpulse = 0;
	conesize = 75;
	hasweapon = false;
	locked = true;
	steerinplace = false;
	drivable = false;
	jumpable = false;
	showweapon = false;
	hasweapon = false;
	flags |= FL_TOUCH_TRIGGERS | FL_POSTTHINK | FL_THINK | FL_DIE_EXPLODE;
	// touch triggers by default
	gravity = 1;
	mass = size.length() * 10;

	health = 1000;
	speed = 1200;
	maxturnrate = 60.0f;

	m_vAngles = angles;
	prev_acceleration = vec_zero;
	real_acceleration = vec_zero;
	prev_origin = origin;
	real_velocity = vec_zero;
	prev_velocity = velocity;
	m_vPrevNormal = vec_zero;
	prev_moveimpulse = 0;
	m_iFrameCtr = 0;
	m_bFrontSlipping = qfalse;
	m_bBackSlipping = qfalse;
	m_vAngularVelocity = vec_zero;
	m_vAngularAcceleration = vec_zero;
	m_sMoveGrid = new cMoveGrid( 3, 3, 1 );
	m_bAutoPilot = qfalse;
	m_fIdealSpeed = 0;
	m_fIdealAccel = 0;
	m_fIdealDistance = 100;
	m_vOriginOffset = vec_zero;
	m_vOriginOffset2 = vec_zero;
	m_vAnglesOffset = vec_zero;
	m_fBouncyCoef = 0.2f;
	m_fSpringyCoef = 0.8f;
	m_fYawMin = -10.0f;
	m_fYawMax = 10.0f;
	m_fYawCoef = 0.1f;
	m_fRollMin = -10.0f;
	m_fRollMax = 10.0f;
	m_fRollCoef = 0.1f;
	m_fZMin = -10.0f;
	m_fZMax = 10.0f;
	m_fZCoef = 0.1f;
	v_angle = vec_zero;
	m_pCurPath = NULL;
	m_bThinkCalled = qfalse;
	m_iCurNode = NULL;
	m_pAlternatePath = NULL;
	m_pNextPath = NULL;
	m_iNextPathStartNode = NULL;
	vs.hit_obstacle = qfalse;
	m_iAlternateNode = 0;
	m_fLookAhead = 0;
	m_fShaderOffset = 0;
	vs.groundTrace.fraction = 1.0f;
	edict->clipmask = MASK_VEHICLE;
	m_fLeftForce = 0;
	m_fRightForce = 0;
	m_fForwardForce = 0;
	m_fBackForce = 0;
	m_fUpForce = 0;
	m_fDownForce = 0;
	edict->s.eFlags |= EF_LINKANGLES;
	m_bMovementLocked = qfalse;
	m_bRemoveOnDeath = qfalse;
	m_sExplosionModel = "fx/fx_explosion.tik";
	m_pCollisionEntity = NULL;
	m_fSoundMinPitch = 0.95f;
	m_fSoundMinSpeed = 0;
	m_fSoundMaxPitch = 1.0f;
	m_fSoundMaxSpeed = 200.0f;
	m_fVolumeMinPitch = 1.0f;
	m_fVolumeMinSpeed = 0;
	m_fVolumeMaxPitch = 1.5f;
	m_fVolumeMaxSpeed = 200.0f;
	m_eSoundState = ST_OFF;
	m_fNextSoundState = level.time;
	m_pVehicleSoundEntities[ 0 ] = NULL;
	m_pVehicleSoundEntities[ 1 ] = NULL;
	m_pVehicleSoundEntities[ 2 ] = NULL;
	m_pVehicleSoundEntities[ 3 ] = NULL;
	m_bStopEnabled = qfalse;
	m_bEnableSkidding = qfalse;
	m_vSkidOrigin = vec_zero;
	m_fSkidLeftForce = 0;
	m_fSkidRightForce = 0;
	m_sAnimationSet = "";
	m_sSoundSet = "";
	m_iLastTiresUpdate = -1;
	m_vTireEnd[ 0 ] = origin;
	m_bTireHit[ 0 ] = false;
	m_vTireEnd[ 1 ] = origin;
	m_bTireHit[ 1 ] = false;
	m_vTireEnd[ 2 ] = origin;
	m_bTireHit[ 2 ] = false;
	m_vTireEnd[ 3 ] = origin;
	m_bTireHit[ 3 ] = false;

	ResetSlots();

	PostEvent( EV_Vehicle_Start, EV_POSTSPAWN );
}

/*
====================
Vehicle::~Vehicle
====================
*/
Vehicle::~Vehicle()
{
	for( int i = 0; i < MAX_SOUND_ENTITIES; i++ )
	{
		if( m_pVehicleSoundEntities[ i ] )
			m_pVehicleSoundEntities[ i ]->PostEvent( EV_Remove, EV_LINKDOORS );
	}

	if( m_pCollisionEntity )
		m_pCollisionEntity->ProcessEvent( EV_Remove );

	entflags &= ~EF_VEHICLE;
}

/*
====================
Vehicle::VehicleStart
====================
*/
void Vehicle::VehicleStart
	(
	Event *ev
	)

{
	Entity			*ent;
	VehicleBase		*last;
	Vector			drivemins, drivemaxs;
	float			max;
	orientation_t	orient;

	// become solid
	setSolidType( SOLID_BBOX );
	edict->r.contents = CONTENTS_BBOX;

	last = this;

	setLocalOrigin( localorigin + Vector( 0.0f, 0.0f, 30.0f ) );
	angles = m_vAngles;

	for( ent = G_NextEntity( NULL ); ent != NULL; ent = G_NextEntity( ent ) )
	{
		if( ( ent != this ) && ( ent->isSubclassOf( VehicleBase ) ) )
		{
			if( ( ent->absmax.x >= absmin.x ) && ( ent->absmax.y >= absmin.y ) && ( ent->absmax.z >= absmin.z ) &&
				( ent->absmin.x <= absmax.x ) && ( ent->absmin.y <= absmax.y ) && ( ent->absmin.z <= absmax.z ) )
			{
				last->vlink = ( VehicleBase * )ent;
				last = ( VehicleBase * )ent;
				last->offset = last->origin - origin;
				last->offset = getLocalVector( last->offset );
				last->edict->s.scale *= edict->s.scale;
			}
		}
	}

	last->vlink = NULL;

	OpenSlotsByModel();

	//
	// get the seat offset
	//
	if( GetRawTag( "seat", &orient ) )
	{
		driveroffset = Vector( orient.origin );
	}
	driveroffset += seatoffset * edict->s.scale;

	SetDriverAngles( angles + seatangles );

	max_health = health;

	//
	// calculate drive mins and maxs
	//
	max = 0;
	if( fabs( mins[ 0 ] ) > max )
		max = fabs( mins[ 0 ] );
	if( fabs( maxs[ 0 ] ) > max )
		max = fabs( maxs[ 0 ] );
	if( fabs( mins[ 1 ] ) > max )
		max = fabs( mins[ 1 ] );
	if( fabs( maxs[ 1 ] ) > max )
		max = fabs( maxs[ 1 ] );
	drivemins = Vector( -max, -max, mins[ 2 ] ) * edict->s.scale;
	drivemaxs = Vector( max, max, maxs[ 2 ] ) * edict->s.scale;

	last_origin = origin;
	link();
}

/*
====================
Vehicle::Drivable
====================
*/
void Vehicle::Drivable
	(
	Event *ev
	)

{
	setMoveType( MOVETYPE_VEHICLE );
	drivable = true;
}

/*
====================
Vehicle::UnDrivable
====================
*/
void Vehicle::UnDrivable
	(
	Event *ev
	)

{
	setMoveType( MOVETYPE_PUSH );
	drivable = false;
}

/*
====================
Vehicle::Jumpable
====================
*/
void Vehicle::Jumpable
	(
	Event *ev
	)

{
	jumpable = true;
}

/*
====================
Vehicle::Lock
====================
*/
void Vehicle::Lock
	(
	Event *ev
	)

{
	Lock();
}

/*
====================
Vehicle::UnLock
====================
*/
void Vehicle::UnLock
	(
	Event *ev
	)

{
	UnLock();
}

/*
====================
Vehicle::SteerInPlace
====================
*/
void Vehicle::SteerInPlace
	(
	Event *ev
	)

{
	steerinplace = true;
}

/*
====================
Vehicle::SeatAnglesOffset
====================
*/
void Vehicle::SeatAnglesOffset
	(
	Event *ev
	)

{
	seatangles = ev->GetVector( 1 );
}

/*
====================
Vehicle::SeatOffset
====================
*/
void Vehicle::SeatOffset
	(
	Event *ev
	)

{
	seatoffset = ev->GetVector( 1 );
}

/*
====================
Vehicle::SetWeapon
====================
*/
void Vehicle::SetWeapon
	(
	Event *ev
	)

{
	showweapon = true;
	hasweapon = true;
	weaponName = ev->GetString( 1 );
}

/*
====================
Vehicle::ShowWeaponEvent
====================
*/
void Vehicle::ShowWeaponEvent
	(
	Event *ev
	)

{
	showweapon = true;
}

/*
====================
Vehicle::HasWeapon
====================
*/
qboolean Vehicle::HasWeapon
	(
	void
	)

{
	return hasweapon;
}

/*
====================
Vehicle::ShowWeapon
====================
*/
qboolean Vehicle::ShowWeapon
	(
	void
	)

{
	return showweapon;
}

/*
====================
Vehicle::SetDriverAngles
====================
*/
void Vehicle::SetDriverAngles
	(
	Vector angles
	)

{
	int i;

	if( !driver.ent )
		return;

	for( i = 0; i < 3; i++ )
	{
		driver.ent->client->ps.delta_angles[ i ] = ANGLE2SHORT( angles[ i ] - driver.ent->client->cmd_angles[ i ] );
	}
}

/*
====================
Vehicle::CheckWater
====================
*/
void Vehicle::CheckWater
	(
	void
	)

{
	Vector  point;
	int	  cont;
	int	  sample1;
	int	  sample2;
	VehicleBase *v;

	unlink();
	v = this;
	while( v->vlink )
	{
		v = v->vlink;
		v->unlink();
	}

	if( driver.ent )
	{
		driver.ent->unlink();
	}

	//
	// get waterlevel
	//
	waterlevel = 0;
	watertype = 0;

	sample2 = maxs[ 2 ] - mins[ 2 ];
	sample1 = sample2 / 2;

	point = origin;
	point[ 2 ] += mins[ 2 ];
	cont = gi.PointContents( point, 0 );

	if( cont & MASK_WATER )
	{
		watertype = cont;
		waterlevel = 1;
		point[ 2 ] = origin[ 2 ] + mins[ 2 ] + sample1;
		cont = gi.PointContents( point, 0 );
		if( cont & MASK_WATER )
		{
			waterlevel = 2;
			point[ 2 ] = origin[ 2 ] + mins[ 2 ] + sample2;
			cont = gi.PointContents( point, 0 );
			if( cont & MASK_WATER )
			{
				waterlevel = 3;
			}
		}
	}

	link();
	v = this;
	while( v->vlink )
	{
		v = v->vlink;
		v->link();
	}

	if( driver.ent )
	{
		driver.ent->link();
		driver.ent->waterlevel = waterlevel;
		driver.ent->watertype = watertype;
	}
}

/*
====================
Vehicle::WorldEffects
====================
*/
void Vehicle::WorldEffects
	(
	void
	)

{
	//
	// Check for earthquakes
	//
	if( groundentity && ( level.earthquake_magnitude > 0.0f ) )
	{
		velocity += Vector
			(
			level.earthquake_magnitude * EARTHQUAKE_STRENGTH * G_CRandom(),
			level.earthquake_magnitude * EARTHQUAKE_STRENGTH * G_CRandom(),
			level.earthquake_magnitude * 1.5f * G_Random()
			);
	}

	//
	// check for lava
	//
	if( watertype & CONTENTS_LAVA )
	{
		Damage( world, world, 20 * waterlevel, origin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_LAVA );
	}
}

/*
====================
Vehicle::DriverUse
====================
*/
void Vehicle::DriverUse
	(
	Event *ev
	)

{
	int slot;
	Vector pos;
	Vector dist;
	float min_length = 1e30f;
	int min_slot = 0;
	int min_type = -1;
	Entity *ent;
	Vector vExitAngles;
	bool bHasExitAngles;
	Vector vExitPosition;

	ent = ev->GetEntity( 1 );

	if( locked )
	{
		return;
	}

	if( ev->NumArgs() == 2 )
	{
		if( ev->IsVectorAt( 2 ) )
		{
			vExitPosition = ev->GetVector( 2 );
		}
		else if( ev->IsEntityAt( 2 ) )
		{
			Entity *pEnt = ev->GetEntity( 2 );

			bHasExitAngles = true;
			vExitAngles = pEnt->angles;
			vExitPosition = pEnt->origin;
		}
		else if( ev->IsSimpleEntityAt( 2 ) )
		{
			SimpleEntity *pEnt = ev->GetSimpleEntity( 2 );

			bHasExitAngles = true;
			vExitAngles = pEnt->angles;
			vExitPosition = pEnt->origin;
		}
	}

	slot = FindDriverSlotByEntity( ent );

	if( slot >= 0 )
	{
		DetachDriverSlot( slot, vec_zero, NULL );

		if( ent->IsSubclassOfVehicleTank() && Turrets[ 0 ].ent->IsSubclassOfVehicleTurretGun() )
		{
			VehicleTurretGun *pTurret = ( VehicleTurretGun * )Turrets[ 0 ].ent.Pointer();

			pTurret->m_bUseRemoteControl = false;
			pTurret->m_pRemoteOwner = NULL;
		}

		return;
	}

	slot = FindPassengerSlotByEntity( ent );

	if( slot >= 0 )
	{
		DetachPassengerSlot( slot, vec_zero, NULL );
		return;
	}

	if( ent->IsSubclassOfWeapon() )
	{
		slot = FindTurretSlotByEntity( ent );

		if( slot >= 0 )
		{
			DetachTurretSlot( slot, vec_zero, NULL );
		}
	}
	else if( ent->IsSubclassOfPlayer() )
	{
		Player *player = ( Player * )ent;

		if( player->m_pTurret )
		{
			slot = FindTurretSlotByEntity( player->m_pTurret );
 
			if( slot >= 0 )
			{
				if( bHasExitAngles )
				{
					AttachTurretSlot( slot, player->m_pTurret, vExitPosition, &vExitAngles );
				}
				else
				{
					AttachTurretSlot( slot, player->m_pTurret, vExitPosition, NULL );
				}

				player->m_pVehicle = NULL;
				return;
			}
		}
	}

	// Check for passengers slots
	for( slot = 0; slot < MAX_PASSENGERS; slot++ )
	{
		if( !( Passengers[ 0 ].flags & SLOT_FREE ) )
		{
			continue;
		}

		QueryPassengerSlotPosition( slot, ( float * )&pos );

		dist = pos - ent->origin;

		if( dist.length() < min_length )
		{
			min_length = dist.length();
			min_type = 0;
			min_slot = slot;
		}
	}

	// Check for turrets slots
	if( ent->IsSubclassOfWeapon() )
	{
		for( slot = 0; slot < MAX_TURRETS; slot++ )
		{
			if( !( Turrets[ 0 ].flags & SLOT_FREE ) )
			{
				continue;
			}

			QueryTurretSlotPosition( slot, ( float * )&pos );

			dist = pos - ent->origin;

			if( dist.length() < min_length )
			{
				min_length = dist.length();
				min_type = 1;
				min_slot = slot;
			}
		}
	}
	else
	{
		for( slot = 0; slot < MAX_TURRETS; slot++ )
		{
			if( !( Turrets[ 0 ].flags & SLOT_BUSY ) )
			{
				continue;
			}

			QueryTurretSlotPosition( slot, ( float * )&pos );

			dist = pos - ent->origin;

			if( dist.length() < min_length )
			{
				min_length = dist.length();
				min_type = 1;
				min_slot = slot;
			}
		}
	}

	slot = 0;

	// Check for driver(s) slot(s)
	if( driver.flags & SLOT_FREE )
	{
		QueryDriverSlotPosition( slot, ( float * )&pos );

		dist = pos - ent->origin;

		if( dist.length() < min_length )
		{
			min_length = dist.length();
			min_type = 2;
			min_slot = slot;
		}
	}

	switch( min_type )
	{
	case 0:
		AttachPassengerSlot( min_slot, ent, vec_zero );
		break;
	case 1:
		AttachTurretSlot( min_slot, ent, vec_zero, NULL );
		break;
	case 2:
		AttachDriverSlot( min_slot, ent, vec_zero );
		break;
	}
}

/*
====================
Vehicle::Drive
====================
*/
qboolean Vehicle::Drive
	(
	usercmd_t *ucmd
	)

{
	Vector i, j, k;

	i = velocity;
	VectorNormalize( i );

	if( !driver.ent || !driver.ent->isClient() )
	{
		return false;
	}

	if( !drivable )
	{
		driver.ent->client->ps.pm_flags |= PMF_FROZEN;
		ucmd->forwardmove = 0;
		ucmd->rightmove = 0;
		ucmd->upmove = 0;
		return false;
	}

	driver.ent->client->ps.pm_flags |= PMF_NO_PREDICTION;

	moveimpulse = ( ( float )ucmd->forwardmove ) * ( VectorLength( i ) + 1.0 );
	m_bIsBraking = ucmd->forwardmove >> 31;
	m_fAccelerator += ( ( float )ucmd->forwardmove ) * 0.005;
	if( m_fAccelerator < 0.0 )
		m_fAccelerator = 0.0;
	else if( m_fAccelerator > 1.0 )
		m_fAccelerator = 1.0;

	turnimpulse = ( ( float )-ucmd->rightmove );
	jumpimpulse = ( ( float )ucmd->upmove * gravity ) / 350;
	if( ( jumpimpulse < 0 ) || ( !jumpable ) )
		jumpimpulse = 0;

	turnimpulse += 8 * angledist( SHORT2ANGLE( ucmd->angles[ 1 ] ) - driver.ent->client->cmd_angles[ 1 ] );

	return true;
}

/*
====================
Vehicle::Think
====================
*/
void Vehicle::Think
	(
	void
	)

{
	flags |= FL_POSTTHINK;
}

/*
====================
Vehicle::Postthink
====================
*/
void Vehicle::Postthink
	(
	void
	)

{
	float	turn;
	Vector	i;
	Vector	j;
	Vector	k;
	Vector	temp;
	Vector	roll;
	Vector	acceleration;
	Vector	atmp;
	Vector	atmp2;
	Vector	aup;
	VehicleBase *v;
	VehicleBase *last;
	float	drivespeed;
	Vector	primal_angles = angles;
	Vector	primal_origin = origin;
	Vector	vTmp;
	Vector	vAddedAngles;
	Vector	n_angles;
	orientation_t orient;

	if( !g_vehicle->integer )
	{
		return;
	}

	if( m_pCollisionEntity )
	{
		setSolidType( SOLID_NOT );
		m_pCollisionEntity->Solid();
	}

	prev_velocity = velocity;

	FactorOutAnglesOffset();
	FactorOutOriginOffset();

	if( m_vOldMins != vec_zero && m_vOldMaxs != vec_zero )
	{
		mins = m_vOldMins;
		maxs = m_vOldMaxs;
	}
	else
	{
		if( mins != vec_zero || maxs != vec_zero )
		{
			m_vOldMins = mins;
			m_vOldMaxs = maxs;
		}
	}

	SetSlotsNonSolid();
	MoveVehicle();
	SetSlotsSolid();

	m_bThinkCalled = true;

	if( !m_bAutoPilot )
	{
		if( !driver.ent || !driver.ent->IsSubclassOfPlayer() )
		{
			acceleration = velocity;
			acceleration.z = 0;

			if( acceleration.length() < 0.1f )
			{
				velocity = vec_zero;
			}
		}

		moveimpulse *= 0.825f;
		turnimpulse *= 0.825f;
	}
	else
	{
		AutoPilot();
	}

	currentspeed = moveimpulse;
	turnangle = turnangle * 0.25f + turnimpulse;
	if( turnangle > maxturnrate )
	{
		turnangle = maxturnrate;
	}
	else if( turnangle < -maxturnrate )
	{
		turnangle = -maxturnrate;
	}

	if( level.inttime <= 1200 )
	{
		prev_origin = origin;
	}
	else
	{
		real_velocity = origin - prev_origin;
		prev_origin = origin;
		prev_acceleration = real_acceleration;
		real_acceleration = prev_velocity - real_velocity;
		prev_velocity = real_velocity;

		n_angles = real_acceleration - prev_acceleration;
	}

	UpdateSkidAngle();
	UpdateBones();
	UpdateShaderOffset();
	UpdateTires();
	UpdateNormals();

	angles.AngleVectorsLeft( &i, &j, &k );

	turn = level.frametime * turnangle;

	velocity[ 0 ] *= 0.925f;
	velocity[ 1 ] *= 0.925f;

	velocity = Vector( orientation[ 0 ] ) * currentspeed;

	drivespeed = velocity * Vector( orientation[ 0 ] );

	if( drivespeed > speed )
	{
		drivespeed = speed;
	}

	velocity = Vector( orientation[ 0 ] ) * drivespeed;
	velocity.z = drivespeed * jumpimpulse;
	avelocity *= 0.05f;

	if( steerinplace && drivespeed < 350.0f )
	{
		drivespeed = 350.0f;
	}

	avelocity[ 1 ] += turn * drivespeed;
	angles += avelocity * level.frametime;

	UpdateSound();

	CalculateAnglesOffset( n_angles );
	CalculateOriginOffset();

	last_origin = origin;

	vAddedAngles = angles - primal_angles;

	if( vAddedAngles[ 0 ] * level.frametime > -1.0f || vAddedAngles[ 0 ] * level.frametime < 1.0f )
	{
		vAddedAngles[ 0 ] = 0.0f;
	}

	if( vAddedAngles[ 1 ] * level.frametime > -1.0f || vAddedAngles[ 1 ] * level.frametime < 1.0f )
	{
		vAddedAngles[ 1 ] = 0.0f;
	}

	if( vAddedAngles[ 2 ] * level.frametime > -1.0f || vAddedAngles[ 2 ] * level.frametime < 1.0f )
	{
		vAddedAngles[ 2 ] = 0.0f;
	}

	avelocity = vAddedAngles;

	FactorInOriginOffset();
	FactorInAnglesOffset( &vAddedAngles );

	CalculateAnimationData( vAddedAngles, m_vOriginOffset + m_vOriginOffset2 );

	if( m_pCollisionEntity )
	{
		n_angles = angles - m_pCollisionEntity->angles;

		n_angles[ 0 ] = angledist( n_angles[ 0 ] );
		n_angles[ 1 ] = angledist( n_angles[ 1 ] );
		n_angles[ 2 ] = angledist( n_angles[ 2 ] );

		G_PushMove( m_pCollisionEntity, origin - primal_origin, n_angles );
		G_TouchTriggers( m_pCollisionEntity );

		m_pCollisionEntity->setOrigin( origin );
		m_pCollisionEntity->setAngles( angles );
	}

	SetupVehicleSoundEntities();
	UpdateDriverSlot( 0 );

	for( int slot = 0; slot < MAX_PASSENGERS; slot++ )
	{
		UpdatePassengerSlot( slot );
	}

	for( int slot = 0; slot < MAX_TURRETS; slot++ )
	{
		UpdateTurretSlot( slot );
	}

	atmp = angles - primal_angles;

	if( g_showvehicleentrypoints->integer )
	{
		for( int slot = 0; slot < MAX_PASSENGERS; slot++ )
		{
			if( Passengers[ slot ].enter_boneindex >= 0 )
			{
				QueryPassengerSlotPosition( slot, ( float * )&temp );

				G_DebugCircle( temp, 10.0f, 1.0f, 0.0f, 0.0f, 1.0f, true );
			}
		}

		for( int slot = 0; slot < MAX_TURRETS; slot++ )
		{
			if( Turrets[ slot ].enter_boneindex >= 0 )
			{
				QueryTurretSlotPosition( slot, ( float * )&temp );

				G_DebugCircle( temp, 10.0f, 0.0f, 1.0f, 0.0f, 1.0f, true );
			}
		}

		if( driver.enter_boneindex >= 0 )
		{
			QueryTurretSlotPosition( 0, ( float * )&temp );

			G_DebugCircle( temp, 10.0f, 0.0f, 0.0f, 1.0f, 1.0f, true );
		}
	}

	if( g_showvehicleslotpoints->integer )
	{
		for( int slot = 0; slot < MAX_PASSENGERS; slot++ )
		{
			if( Passengers[ slot ].boneindex < 0 )
			{
				continue;
			}

			GetTagPositionAndOrientation( Passengers[ slot ].boneindex, &orient );
			G_DebugCircle( orient.origin, 10.0, 1.0f, 0.5f, 0.5f, 1.0f, true );
		}

		for( int slot = 0; slot < MAX_TURRETS; slot++ )
		{
			if( Turrets[ slot ].boneindex < 0 )
			{
				continue;
			}

			GetTagPositionAndOrientation( Turrets[ slot ].boneindex, &orient );
			G_DebugCircle( orient.origin, 10.0, 0.5f, 1.0f, 1.0f, 1.0f, true );
		}

		if( driver.boneindex >= 0 )
		{
			GetTagPositionAndOrientation( driver.boneindex, &orient );
			G_DebugCircle( orient.origin, 10.0, 0.5f, 0.5f, 1.0f, 1.0f, true );
		}
	}

	last = this;
	while( last->vlink )
	{
		v = last->vlink;
		v->setOrigin( origin + ( i * v->offset.x ) + ( j * v->offset.y ) + ( k * v->offset.z ) );
		v->avelocity = avelocity;
		v->velocity = velocity;
		v->angles[ ROLL ] = angles[ ROLL ];
		v->angles[ YAW ] = angles[ YAW ];
		v->angles[ PITCH ] = ( float )( ( int )( v->angles[ PITCH ] + ( drivespeed / 4 ) ) % 360 );

		v->setAngles( v->angles );

		last = v;
	}

	CheckWater();
	WorldEffects();

	if( m_pCollisionEntity )
	{
		setSolidType( SOLID_NOT );
		m_pCollisionEntity->Solid();
	}
	else
	{
		setSolidType( SOLID_BBOX );
		edict->r.contents = CONTENTS_UNKNOWN2;
	}
}

/*
====================
Vehicle::VehicleTouched
====================
*/
void Vehicle::VehicleTouched
	(
	Event *ev
	)

	{
	Entity	*other;
	float		speed;
   Vector   delta;
	Vector	dir;

 	other = ev->GetEntity( 1 );
	if ( other == driver.ent )
		{
		return;
		}

   if ( other == world )
      {
      return;
      }

   if ( drivable && !driver.ent )
      {
      return;
      }

   delta = origin - last_origin;
	speed = delta.length();
	if ( speed > 2 )
		{
		Sound( "vehicle_crash", qtrue );
		dir = delta * ( 1 / speed );
      other->Damage( this, lastdriver.ent, speed * 8, origin, dir, vec_zero, speed*15, 0, MOD_VEHICLE );
		}

	}

/*
====================
Vehicle::VehicleBlocked
====================
*/
void Vehicle::VehicleBlocked
	(
	Event *ev
	)

	{
   return;
/*
	Entity	*other;
	float		speed;
   float    damage;
   Vector   delta;
   Vector   newvel;
	Vector	dir;

   if ( !velocity[0] && !velocity[1] )
      return;

	other = ev->GetEntity( 1 );
	if ( other == driver.ent )
		{
		return;
		}
   if ( other->isSubclassOf( VehicleBase ) )
      {
      delta = other->origin - origin;
      delta.normalize();

      newvel = vec_zero - ( velocity) + ( other->velocity * 0.25 );
      if ( newvel * delta < 0 )
         {
         velocity = newvel;
         delta = velocity - other->velocity;
         damage = delta.length()/4;
         }
      else
         {
         return;
         }
      }
   else if ( ( velocity.length() < 350 ) )
      {
      other->velocity += velocity*1.25f;
      other->velocity[ 2 ] += 100;
      damage = velocity.length()/4;
      }
   else
      {
      damage = other->health + 1000;
      }

	// Gib 'em outright
	speed = fabs( velocity.length() );
	dir = velocity * ( 1 / speed );
   other->Damage( this, lastdriver.ent, damage, origin, dir, vec_zero, speed, 0, MOD_VEHICLE, -1, -1, 1.0f );
*/
   }

/*
====================
Vehicle::Driver
====================
*/
Entity *Vehicle::Driver
	(
	void
	)

	{
	return driver.ent;
	}

/*
====================
Vehicle::IsDrivable
====================
*/
qboolean Vehicle::IsDrivable
	(
	void
	)

	{
	return drivable;
	}

/*
====================
Vehicle::EventSetSpeed
====================
*/
void Vehicle::SetSpeed
	(
	Event *ev
	)
   {
   speed = ev->GetFloat( 1 );
   }

/*
====================
Vehicle::EventSetTurnRate
====================
*/
void Vehicle::SetTurnRate
	(
	Event *ev
	)
   {
   maxturnrate = ev->GetFloat( 1 );
   }

/*
====================
Vehicle::VehicleDestroyed
====================
*/
void Vehicle::VehicleDestroyed
	(
	Event *ev
	)
	
{
}

/*
====================
Vehicle::SetMoveInfo
====================
*/
void Vehicle::SetMoveInfo
	(
	vmove_t *vm
	)

{
	memset( vm, 0, sizeof( vmove_t ) );

	VectorCopy( origin, vs.origin );
	vs.useGravity = 0;
	vs.entityNum = entnum;

	vm->vs = &vs;
	vm->frametime = level.frametime;
	vm->tracemask = edict->clipmask;
	VectorCopy( mins, vm->mins );
	VectorCopy( maxs, vm->maxs );

	vs.entityNum = edict->s.number;
	vs.desired_dir[ 0 ] = velocity[ 0 ];
	vs.desired_dir[ 1 ] = velocity[ 1 ];

	vm->desired_speed = VectorNormalize2D( this->vs.desired_dir );
}

/*
====================
Vehicle::GetMoveInfo
====================
*/
void Vehicle::GetMoveInfo
	(
	vmove_t *vm
	)

{
	Vector newOrigin = vm->vs->origin;

	if( bindmaster )
	{
		newOrigin = vm->vs->origin - origin;
	}

	setLocalOrigin( newOrigin );

	groundentity = NULL;
	if( vm->vs->groundEntityNum != ENTITYNUM_NONE )
	{
		groundentity = &g_entities[ vm->vs->groundEntityNum ];
	}
}

/*
====================
Vehicle::SetCEMoveInfo
====================
*/
void Vehicle::SetCEMoveInfo
	(
	vmove_t *vm
	)

{
	SetMoveInfo( vm );

	vm->mins[ 0 ] = m_pCollisionEntity->mins[ 0 ] - 24.0f;
	vm->mins[ 1 ] = m_pCollisionEntity->mins[ 1 ] - 24.0f;
	vm->mins[ 2 ] = m_pCollisionEntity->mins[ 2 ];
	vm->maxs[ 0 ] = m_pCollisionEntity->maxs[ 0 ] + 24.0f;
	vm->maxs[ 1 ] = m_pCollisionEntity->maxs[ 1 ] + 24.0f;
	vm->maxs[ 2 ] = m_pCollisionEntity->maxs[ 2 ];
}

/*
====================
Vehicle::GetCEMoveInfo
====================
*/
void Vehicle::GetCEMoveInfo
	(
	vmove_t *vm
	)

{
	GetMoveInfo( vm );
}

/*
====================
Vehicle::SetViewAngles
====================
*/
void Vehicle::SetViewAngles
	(
	Vector newViewangles
	)

{
	client->ps.delta_angles[ 0 ] = ANGLE2SHORT( newViewangles.x );
	client->ps.delta_angles[ 1 ] = ANGLE2SHORT( newViewangles.y );
	client->ps.delta_angles[ 2 ] = ANGLE2SHORT( newViewangles.z );

	AnglesToAxis( newViewangles, orientation );

	yaw_forward = orientation[ 0 ];
	yaw_left = orientation[ 1 ];
}

/*
====================
Vehicle::EventSetMass
====================
*/
void Vehicle::SetMass
	(
	Event *ev
	)
	
{
	m_fMass = ev->GetFloat( 1 );
	m_fFrontMass = m_fMass * 0.5;
	m_fBackMass = m_fMass * 0.5;
}

void Vehicle::SetFrontMass
	(
	Event *ev
	)
	
{
	m_fFrontMass = ev->GetFloat( 1 );
	m_fMass = m_fFrontMass + m_fBackMass;
}

/*
====================
Vehicle::EventSetBackMass
====================
*/
void Vehicle::SetBackMass
	(
	Event *ev
	)
	
{
	m_fBackMass = ev->GetFloat( 1 );
	m_fMass = m_fFrontMass + m_fBackMass;
}

/*
====================
Vehicle::EventSetTread
====================
*/
void Vehicle::SetTread
	(
	Event *ev
	)
	
{
	m_fTread = ev->GetFloat( 1 );
}

/*
====================
Vehicle::EventSetTireRadius
====================
*/
void Vehicle::SetTireRadius
	(
	Event *ev
	)
	
{
	m_fTireRadius = ev->GetFloat( 1 );
}

/*
====================
Vehicle::EventSetRollingResistance
====================
*/
void Vehicle::SetRollingResistance
	(
	Event *ev
	)
	
{
	m_fRollingResistance = ev->GetFloat( 1 );
}

/*
====================
Vehicle::EventSetDrag
====================
*/
void Vehicle::SetDrag
	(
	Event *ev
	)
	
{
	m_fDrag = ev->GetFloat( 1 );
}

/*
====================
Vehicle::TorqueLookup
====================
*/
float Vehicle::TorqueLookup
	(
	int rpm
	)

{
	if( rpm > 4999 )
	{
		if( rpm > 5999 )
		{
			return 0.0;
		}
		else
		{
			return ( float )( 190 * ( 6000 - rpm ) ) * 0.001;
		}
	}
	else
	{
		return 190.0;
	}
}

/*
====================
Vehicle::UpdateVariables
====================
*/
void Vehicle::UpdateVariables
	(
	Vector *acceleration,
	Vector *vpn,
	Vector *vup,
	Vector *vright,
	Vector *t_vpn,
	Vector *t_vup,
	Vector *t_vright
	)

{

}

/*
====================
Vehicle::EventModelInit
====================
*/
void Vehicle::ModelInit
	(
	Event *ev
	)
	
{
	SetControllerTag( 0, gi.Tag_NumForName( edict->tiki, "tire_rotate_front_left" ) );
	SetControllerTag( 1, gi.Tag_NumForName( edict->tiki, "tire_rotate_front_right" ) );
	SetControllerTag( 2, gi.Tag_NumForName( edict->tiki, "steeringwheel_center" ) );
}

/*
====================
Vehicle::EventBouncyCoef
====================
*/
void Vehicle::BouncyCoef
	(
	Event *ev
	)
	
{
	m_fBouncyCoef = ev->GetFloat( 1 );
}

/*
====================
Vehicle::EventSpringyCoef
====================
*/
void Vehicle::SpringyCoef
	(
	Event *ev
	)
	
{
	m_fSpringyCoef = ev->GetFloat( 1 );
}

/*
====================
Vehicle::EventYawMinMax
====================
*/
void Vehicle::YawMinMax
	(
	Event *ev
	)
	
{
	if( ev->NumArgs() != 3 )
	{
		ScriptError( "No Parameter for YawMinMax" );
	}

	m_fYawMin = ev->GetFloat( 1 );
	m_fYawMax = ev->GetFloat( 2 );
	m_fYawCoef = ev->GetFloat( 3 );

	if( m_fYawMin > m_fYawMax )
	{
		ScriptError( "Mismatched mins and maxs for YawMinMax" );
	}
}

/*
====================
Vehicle::EventRollMinMax
====================
*/
void Vehicle::RollMinMax
	(
	Event *ev
	)
	
{
	if( ev->NumArgs() != 3 )
	{
		ScriptError( "No Parameter for RollMinMax" );
	}

	m_fRollMin = ev->GetFloat( 1 );
	m_fRollMax = ev->GetFloat( 2 );
	m_fRollCoef = ev->GetFloat( 3 );

	if( m_fRollMin > m_fRollMax )
	{
		ScriptError( "Mismatched mins and maxs for RollMinMax" );
	}
}

/*
====================
Vehicle::EventZMinMax
====================
*/
void Vehicle::ZMinMax
	(
	Event *ev
	)
	
{
	if( ev->NumArgs() != 3 )
	{
		ScriptError( "No Parameter for ZMinMax" );
	}

	m_fZMin = ev->GetFloat( 1 );
	m_fZMax = ev->GetFloat( 2 );
	m_fZCoef = ev->GetFloat( 3 );

	if( m_fZMin > m_fZMax )
	{
		ScriptError( "Mismatched mins and maxs for ZMinMax" );
	}
}

/*
====================
Vehicle::EventSetAnimationSet
====================
*/
void Vehicle::SetAnimationSet
	(
	Event *ev
	)
	
{
	m_sAnimationSet = ev->GetString( 1 );
}

/*
====================
Vehicle::EventSetSoundSet
====================
*/
void Vehicle::SetSoundSet
	(
	Event *ev
	)
	
{
	m_sSoundSet = ev->GetString( 1 );
}

/*
====================
Vehicle::EventSpawnTurret
====================
*/
void Vehicle::SpawnTurret
	(
	Event *ev
	)
	
{
	VehicleTurretGun *pTurret;
	int slot;

	pTurret = new VehicleTurretGun;
	pTurret->SetBaseOrientation( orientation, NULL );
	pTurret->setModel( ev->GetString( 2 ) );

	slot = ev->GetInteger( 1 );
	AttachTurretSlot( slot, pTurret, vec_zero, NULL );

	pTurret->SetVehicleOwner( this );
	Event *event = new Event( EV_TakeDamage );
	pTurret->PostEvent( event, EV_POSTSPAWN );
	UpdateTurretSlot( slot );

	pTurret->ProcessPendingEvents();
}

/*
====================
Vehicle::EventLockMovement
====================
*/
void Vehicle::EventLockMovement
	(
	Event *ev
	)
	
{
	m_bMovementLocked = true;
}

/*
====================
Vehicle::EventUnlockMovement
====================
*/
void Vehicle::EventUnlockMovement
	(
	Event *ev
	)
	
{
	m_bMovementLocked = false;
}

/*
====================
Vehicle::EventQueryFreePassengerSlot
====================
*/
void Vehicle::QueryFreePassengerSlot
	(
	Event *ev
	)
	
{
	ev->AddInteger( QueryFreePassengerSlot() );
}

/*
====================
Vehicle::EventQueryFreeDriverSlot
====================
*/
void Vehicle::QueryFreeDriverSlot
	(
	Event *ev
	)
	
{
	ev->AddInteger( QueryFreeDriverSlot() );
}

/*
====================
Vehicle::EventQueryFreeTurretSlot
====================
*/
void Vehicle::QueryFreeTurretSlot
	(
	Event *ev
	)
	
{
	ev->AddInteger( QueryFreeTurretSlot() );
}

/*
====================
Vehicle::EventQueryPassengerSlotPosition
====================
*/
void Vehicle::QueryPassengerSlotPosition
	(
	Event *ev
	)
	
{
	Vector vPos;
	int iSlot;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_PASSENGERS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	QueryPassengerSlotPosition( iSlot, vPos );
	ev->AddVector( vPos );
}

/*
====================
Vehicle::EventQueryDriverSlotPosition
====================
*/
void Vehicle::QueryDriverSlotPosition
	(
	Event *ev
	)
	
{
	Vector vPos;
	int iSlot;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_DRIVERS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	QueryDriverSlotPosition( iSlot, vPos );
	ev->AddVector( vPos );
}

/*
====================
Vehicle::EventQueryTurretSlotPosition
====================
*/
void Vehicle::QueryTurretSlotPosition
	(
	Event *ev
	)
	
{
	Vector vPos;
	int iSlot;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_TURRETS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	QueryTurretSlotPosition( iSlot, vPos );
	ev->AddVector( vPos );
}

/*
====================
Vehicle::EventQueryPassengerSlotAngles
====================
*/
void Vehicle::QueryPassengerSlotAngles
	(
	Event *ev
	)
	
{
	Vector vAngles;
	int iSlot;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_PASSENGERS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	QueryPassengerSlotAngles( iSlot, vAngles );
	ev->AddVector( vAngles );
}

/*
====================
Vehicle::EventQueryDriverSlotAngles
====================
*/
void Vehicle::QueryDriverSlotAngles
	(
	Event *ev
	)
	
{
	Vector vAngles;
	int iSlot;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_DRIVERS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	QueryDriverSlotAngles( iSlot, vAngles );
	ev->AddVector( vAngles );
}

/*
====================
Vehicle::EventQueryTurretSlotAngles
====================
*/
void Vehicle::QueryTurretSlotAngles
	(
	Event *ev
	)
	
{
	Vector vAngles;
	int iSlot;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_TURRETS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	QueryTurretSlotAngles( iSlot, vAngles );
	ev->AddVector( vAngles );
}

/*
====================
Vehicle::EventQueryPassengerSlotStatus
====================
*/
void Vehicle::QueryPassengerSlotStatus
	(
	Event *ev
	)
	
{
	int iSlot;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_PASSENGERS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	ev->AddInteger( QueryPassengerSlotStatus( iSlot ) );
}

/*
====================
Vehicle::EventQueryDriverSlotStatus
====================
*/
void Vehicle::QueryDriverSlotStatus
	(
	Event *ev
	)
	
{
	int iSlot;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_DRIVERS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	ev->AddInteger( QueryDriverSlotStatus( iSlot ) );
}

/*
====================
Vehicle::EventQueryTurretSlotStatus
====================
*/
void Vehicle::QueryTurretSlotStatus
	(
	Event *ev
	)
	
{
	int iSlot;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_TURRETS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	ev->AddInteger( QueryTurretSlotStatus( iSlot ) );
}

/*
====================
Vehicle::EventQueryPassengerSlotEntity
====================
*/
void Vehicle::QueryPassengerSlotEntity
	(
	Event *ev
	)
	
{
	int iSlot;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_PASSENGERS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	ev->AddEntity( QueryPassengerSlotEntity( iSlot ) );
}

/*
====================
Vehicle::EventQueryDriverSlotEntity
====================
*/
void Vehicle::QueryDriverSlotEntity
	(
	Event *ev
	)
	
{
	int iSlot;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_DRIVERS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	ev->AddEntity( QueryDriverSlotEntity( iSlot ) );
}

/*
====================
Vehicle::EventQueryTurretSlotEntity
====================
*/
void Vehicle::QueryTurretSlotEntity
	(
	Event *ev
	)
	
{
	int iSlot;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_TURRETS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	ev->AddEntity( QueryTurretSlotEntity( iSlot ) );
}

/*
====================
Vehicle::EventAttachPassengerSlot
====================
*/
void Vehicle::AttachPassengerSlot
	(
	Event *ev
	)
	
{
	int iSlot;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_PASSENGERS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	AttachPassengerSlot( iSlot, ev->GetEntity( 2 ), vec_zero );
	UpdatePassengerSlot( iSlot );
}

/*
====================
Vehicle::EventAttachDriverSlot
====================
*/
void Vehicle::AttachDriverSlot
	(
	Event *ev
	)
	
{
	int iSlot;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_DRIVERS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	AttachDriverSlot( 0, ev->GetEntity( 2 ), vec_zero );
	UpdateDriverSlot( 0 );
}

/*
====================
Vehicle::EventAttachTurretSlot
====================
*/
void Vehicle::AttachTurretSlot
	(
	Event *ev
	)
	
{
	int iSlot;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_TURRETS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	AttachTurretSlot( iSlot, ev->GetEntity( 2 ), vec_zero, NULL );
	UpdateTurretSlot( iSlot );
}

/*
====================
Vehicle::EventDetachPassengerSlot
====================
*/
void Vehicle::DetachPassengerSlot
	(
	Event *ev
	)
	
{
	int iSlot;
	Vector vExitPosition;
	Vector vExitAngles;
	bool bHasExitAngles = false;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_PASSENGERS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	if( ev->NumArgs() == 2 )
	{
		if( ev->IsVectorAt( 2 ) )
		{
			vExitPosition = ev->GetVector( 2 );
		}
		else if( ev->IsEntityAt( 2 ) )
		{
			vExitPosition = ev->GetEntity( 2 )->origin;
			bHasExitAngles = true;
		}
		else if( ev->IsSimpleEntityAt( 2 ) )
		{
			vExitPosition = ev->GetSimpleEntity( 2 )->origin;
		}

		DetachPassengerSlot( iSlot, vExitPosition, bHasExitAngles ? &vExitAngles : NULL );
	}
	else
	{
		DetachPassengerSlot( iSlot, vec_zero, NULL );
	}
}

/*
====================
Vehicle::EventDetachDriverSlot
====================
*/
void Vehicle::DetachDriverSlot
	(
	Event *ev
	)
	
{
	int iSlot;
	Vector vExitPosition;
	Vector vExitAngles;
	bool bHasExitAngles = false;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_DRIVERS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	if( ev->NumArgs() == 2 )
	{
		if( ev->IsVectorAt( 2 ) )
		{
			vExitPosition = ev->GetVector( 2 );
		}
		else if( ev->IsEntityAt( 2 ) )
		{
			vExitPosition = ev->GetEntity( 2 )->origin;
			bHasExitAngles = true;
		}
		else if( ev->IsSimpleEntityAt( 2 ) )
		{
			vExitPosition = ev->GetSimpleEntity( 2 )->origin;
		}

		DetachDriverSlot( iSlot, vExitPosition, bHasExitAngles ? &vExitAngles : NULL );
	}
	else
	{
		DetachDriverSlot( iSlot, vec_zero, NULL );
	}
}

/*
====================
Vehicle::EventDetachTurretSlot
====================
*/
void Vehicle::DetachTurretSlot
	(
	Event *ev
	)
	
{
	int iSlot;
	Vector vExitPosition;
	Vector vExitAngles;
	bool bHasExitAngles = false;

	iSlot = ev->GetInteger( 1 );
	if( iSlot >= MAX_TURRETS )
	{
		ScriptError( "Slot Specified is greater than maximum allowed for that parameter\n" );
	}

	if( ev->NumArgs() == 2 )
	{
		if( ev->IsVectorAt( 2 ) )
		{
			vExitPosition = ev->GetVector( 2 );
		}
		else if( ev->IsEntityAt( 2 ) )
		{
			vExitPosition = ev->GetEntity( 2 )->origin;
			bHasExitAngles = true;
		}
		else if( ev->IsSimpleEntityAt( 2 ) )
		{
			vExitPosition = ev->GetSimpleEntity( 2 )->origin;
		}

		DetachTurretSlot( iSlot, vExitPosition, bHasExitAngles ? &vExitAngles : NULL );
	}
	else
	{
		DetachTurretSlot( iSlot, vec_zero, NULL );
	}
}

/*
====================
Vehicle::SetWheelCorners
====================
*/
void Vehicle::SetWheelCorners
	(
	Event *ev
	)
	
{
	Vector size;

	size = ev->GetVector( 1 );
	m_vOriginCornerOffset = ev->GetVector( 2 );

	maxtracedist = size[ 2 ];

	Corners[ 0 ][ 0 ] = -( size[ 0 ] * 0.5 );
	Corners[ 0 ][ 1 ] = ( size[ 1 ] * 0.5 );
	Corners[ 0 ][ 2 ] = size[ 2 ];

	Corners[ 1 ][ 0 ] = ( size[ 0 ] * 0.5 );
	Corners[ 1 ][ 1 ] = ( size[ 1 ] * 0.5 );
	Corners[ 1 ][ 2 ] = size[ 2 ];

	Corners[ 2 ][ 0 ] = -( size[ 0 ] * 0.5 );
	Corners[ 2 ][ 1 ] = -( size[ 1 ] * 0.5 );
	Corners[ 2 ][ 2 ] = size[ 2 ];

	Corners[ 3 ][ 0 ] = ( size[ 0 ] * 0.5 );
	Corners[ 3 ][ 1 ] = -( size[ 1 ] * 0.5 );
	Corners[ 3 ][ 2 ] = size[ 2 ];

	SetupVehicleSoundEntities();
}

/*
====================
Vehicle::EventDriveInternal
====================
*/
void Vehicle::EventDriveInternal
	(
	Event *ev,
	bool wait
	)

{
	SimpleEntity *path;
	SimpleEntity *alternate_path = NULL;

	m_fIdealDistance = 100.0f;
	m_fLookAhead = 256.0f;
	m_fIdealAccel = 35.0f;
	m_fIdealSpeed = 250.0f;

	switch( ev->NumArgs() )
	{
	case 6:
		alternate_path = ev->GetSimpleEntity( 6 );
	case 5:
		m_fLookAhead = ev->GetFloat( 5 );
	case 4:
		m_fIdealDistance = ev->GetFloat( 4 );
	case 3:
		m_fIdealAccel = ev->GetFloat( 3 );
	case 2:
		m_fIdealSpeed = ev->GetFloat( 2 );
		break;
	case 1:
		break;
	default:
		ScriptError( "wrong number of arguments" );
	}

	path = ev->GetSimpleEntity( 1 );

	if( path )
	{
		ScriptError( "Vehicle Given Drive Command with NULL path." );
	}

	if( !m_pCurPath )
	{
		m_pCurPath = new cSpline < 4, 512 > ;
	}

	if( !m_pAlternatePath )
	{
		m_pAlternatePath = new cSpline < 4, 512 > ;
	}

	SetupPath( m_pCurPath, path );

	// Setup the alternate path
	if( alternate_path )
	{
		SetupPath( m_pAlternatePath, alternate_path );
	}

	m_bAutoPilot = true;
	m_iCurNode = 0;
	m_iAlternateNode = 0;

	Sound( m_sSoundSet + "snd_start" );
}

/*
====================
Vehicle::EventDrive
====================
*/
void Vehicle::EventDrive
	(
	Event *ev
	)
	
{
	EventDriveInternal( ev, true );
}

/*
====================
Vehicle::EventDriveNoWait
====================
*/
void Vehicle::EventDriveNoWait
	(
	Event *ev
	)
	
{
	EventDriveInternal( ev, false );
}

/*
====================
Vehicle::EventStop
====================
*/
void Vehicle::EventStop
	(
	Event *ev
	)
	
{
	m_bStopEnabled = false;
	m_bAutoPilot = false;
	m_bIsSkidding = false;
	moveimpulse = 0;
	turnimpulse = 0;
	m_iCurNode = 0;
	Unregister( STRING_DRIVE );
}

/*
====================
Vehicle::EventFullStop
====================
*/
void Vehicle::EventFullStop
	(
	Event *ev
	)
	
{
	m_bStopEnabled = 0;
	m_bIsSkidding = 0;
	m_bAutoPilot = 0;
	moveimpulse = 0;
	turnimpulse = 0;
	velocity = vec_zero;
	m_iCurNode = 0;
	Unregister( STRING_DRIVE );
}

/*
====================
Vehicle::EventModifyDrive
====================
*/
void Vehicle::EventModifyDrive
	(
	Event *ev
	)
	
{
	if( !level.Spawned() )
	{
		ScriptError(  "ModifyDrive used improperly... (used before the level is spawned)" );
	}

	if( !m_bAutoPilot || !this->m_pCurPath )
	{
		ScriptError( "ModifyDrive used when not driving!" );
	}

	if( ev->NumArgs() < 1 || ev->NumArgs() > 3 )
	{
		ScriptError( "wrong number of arguments" );
	}

	m_fIdealSpeed = ev->GetFloat( 1 );

	if( ev->NumArgs() >= 2 )
	{
		m_fIdealAccel = ev->GetFloat( 2 );
	}

	if( ev->NumArgs() >= 3 )
	{
		m_fLookAhead = ev->GetFloat( 3 );
	}
}

/*
====================
Vehicle::EventNextDrive
====================
*/
void Vehicle::EventNextDrive
	(
	Event *ev
	)
	
{
	SimpleEntity *path;
	float *i_fTmp;
	float o_fTmp[ 4 ];
	Vector org1;
	Vector org2;

	path = ev->GetSimpleEntity( 1 );

	if( !m_bAutoPilot )
	{
		ScriptError( "Cannot Set Next Path because Not Currently Driving a Path." );
	}

	if( !m_pCurPath || m_pCurPath->m_iPoints == 0 )
	{
		ScriptError( "Cannot Set Next Path because Current Path is Empty." );
	}

	if( !m_pNextPath )
	{
		m_pNextPath = new cSpline < 4, 512 > ;
	}

	SetupPath( m_pNextPath, path );

	i_fTmp = m_pCurPath->GetByNode( m_pCurPath->m_iPoints, NULL );
	org1 = ( i_fTmp + 1 );
	i_fTmp = m_pNextPath->GetByNode( 0.0f, NULL );
	org2 = ( i_fTmp + 1 );

	o_fTmp[ 0 ] = ( org2 - org1 ).length();
	VectorClear( o_fTmp + 1 );

	m_pNextPath->UniformAdd( o_fTmp );
	m_iNextPathStartNode = m_pCurPath->Append( m_pNextPath );
}

/*
====================
Vehicle::EventModel
====================
*/
void Vehicle::EventModel
	(
	Event *ev
	)
	
{
	SetModelEvent( ev );
}

/*
====================
Vehicle::EventRemoveOnDeath
====================
*/
void Vehicle::EventRemoveOnDeath
	(
	Event *ev
	)
	
{
	m_bRemoveOnDeath = ev->GetBoolean( 1 );
}

/*
====================
Vehicle::EventSetExplosionModel
====================
*/
void Vehicle::EventSetExplosionModel
	(
	Event *ev
	)
	
{
	m_sExplosionModel = ev->GetString( 1 );
}

/*
====================
Vehicle::EventSetCollisionModel
====================
*/
void Vehicle::EventSetCollisionModel
	(
	Event *ev
	)
	
{
	Entity *pColEnt = ev->GetEntity( 1 );

	if( !pColEnt )
	{
		ScriptError( "Trying to set a collision model with a NULL entity." );
	}

	if( m_pCollisionEntity )
	{
		m_pCollisionEntity->PostEvent( EV_Remove, EV_VEHICLE );
	}

	m_pCollisionEntity = new VehicleCollisionEntity( this );
	m_pCollisionEntity->setModel( pColEnt->model );

	if( !m_pCollisionEntity->model.length() || *m_pCollisionEntity->model != '*' )
	{
		// Re-post the event with the correct time
		m_pCollisionEntity->CancelEventsOfType( EV_Remove );
		m_pCollisionEntity->PostEvent( EV_Remove, EV_VEHICLE );
		m_pCollisionEntity = NULL;

		ScriptError( "Model for Entity not of a valid type. Must be B-Model." );
	}

	m_pCollisionEntity->setOrigin( origin );
	m_pCollisionEntity->setAngles( angles );
}

/*
====================
Vehicle::EventGetCollisionModel
====================
*/
void Vehicle::EventGetCollisionModel
	(
	Event *ev
	)
	
{
	ev->AddEntity( m_pCollisionEntity );
}

/*
====================
Vehicle::EventSetSoundParameters
====================
*/
void Vehicle::EventSetSoundParameters
	(
	Event *ev
	)
	
{
	m_fSoundMinSpeed = ev->GetFloat( 1 );
	m_fSoundMinPitch = ev->GetFloat( 2 );
	m_fSoundMaxSpeed = ev->GetFloat( 3 );
	m_fSoundMaxPitch = ev->GetFloat( 4 );
}

/*
====================
Vehicle::EventSetVolumeParameters
====================
*/
void Vehicle::EventSetVolumeParameters
	(
	Event *ev
	)
	
{
	m_fVolumeMinSpeed = ev->GetFloat( 1 );
	m_fVolumeMinPitch = ev->GetFloat( 2 );
	m_fVolumeMaxSpeed = ev->GetFloat( 3 );
	m_fVolumeMaxPitch = ev->GetFloat( 4 );
}

/*
====================
Vehicle::EventDamage
====================
*/
void Vehicle::EventDamage
	(
	Event *ev
	)
	
{
	Vector vDirection;
	float fForce;
	int i;

	if( !IsDamagedBy( ev->GetEntity( 3 ) ) )
	{
		return;
	}

	Event *event = new Event( EV_Damage );

	vDirection = ev->GetVector( 5 );
	fForce = ev->GetFloat( 7 );
	VectorNormalizeFast( vDirection );

	m_fForwardForce += DotProduct( orientation[ 1 ], vDirection ) * fForce;
	m_fLeftForce += DotProduct( orientation[ 0 ], vDirection ) * fForce;

	for( i = 1; i <= ev->NumArgs(); i++ )
	{
		if( i == 7 )
		{
			event->AddFloat( 0 );
		}
		else
		{
			event->AddValue( ev->GetValue( i ) );
		}
	}

	if( driver.ent )
	{
		if( driver.ent->IsSubclassOfPlayer() )
		{
			Player *player = ( Player * )driver.ent.Pointer();
			Vector dir = ev->GetVector( 5 );

			if( player->camera )
			{
				player->damage_yaw = AngleSubtract( player->camera->angles[ 1 ], dir.toYaw() ) + 180.5f;
			}
			else
			{
				player->damage_yaw = AngleSubtract( player->GetVAngles()[ 1 ], dir.toYaw() ) + 180.5f;
			}
		}
	}

	DamageEvent( event );
	delete event;
}

/*
====================
Vehicle::EventStopAtEnd
====================
*/
void Vehicle::EventStopAtEnd
	(
	Event *ev
	)
	
{
	if( !m_pCurPath )
	{
		ScriptError( "Tried to Stop at end of path on a vehicle who is not driving a path!" );
	}

	m_fStopStartDistance = GetPathPosition( m_pCurPath, m_iCurNode );
	m_fStopStartSpeed = moveimpulse;
	m_fStopEndDistance = *m_pCurPath->GetByNode( m_pCurPath->m_vPoints[ 0 ][ 0 ], NULL );
	m_bStopEnabled = 1;
}

/*
====================
Vehicle::EventSkidding
====================
*/
void Vehicle::EventSkidding
	(
	Event *ev
	)
	
{
	if( ev->NumArgs() == 1 )
	{
		m_bEnableSkidding = ev->GetInteger( 1 );
	}
	else
	{
		m_bEnableSkidding = true;
	}

	ProcessEvent( EV_Vehicle_ContinueSkidding );
}

/*
====================
Vehicle::EventContinueSkidding
====================
*/
void Vehicle::EventContinueSkidding
	(
	Event *ev
	)
	
{
	if( m_bEnableSkidding )
	{
		if( HasAnim( "skidding" ) )
		{
			NewAnim( "skidding", EV_Vehicle_ContinueSkidding, 7, 0.000001f );
			return;
		}
		else
		{
			assert( !"Vehicle without skidding animation." );
		}
	}
	else
	{
		if( HasAnim( "idle" ) )
		{
			NewAnim( "idle", 0, 7, 0.000001f );
			return;
		}
		else
		{
			assert( !"Vehicle without idle animation." );
		}
	}
}

/*
====================
Vehicle::EventVehicleAnim
====================
*/
void Vehicle::EventVehicleAnim
	(
	Event *ev
	)
	
{
	float weight;

	if( ev->NumArgs() > 1 )
	{
		weight = ev->GetFloat( 2 );
	}
	else
	{
		weight = 1.0f;
	}

	NewAnim( ev->GetString( 1 ), EV_Vehicle_VehicleAnimDone, 8, weight );
}

/*
====================
Vehicle::EventVehicleAnimDone
====================
*/
void Vehicle::EventVehicleAnimDone
	(
	Event *ev
	)
	
{
	Unregister( STRING_VEHICLEANIMDONE );
}

/*
====================
Vehicle::TouchStuff
====================
*/
void Vehicle::TouchStuff
	(
	vmove_t *vm
	)

{
	int i, j;
	gentity_t *other;
	Event		*event;

	if (driver.ent)
		G_TouchTriggers(driver.ent);

	for (int i = 0; i < MAX_PASSENGERS; i++)
	{
		if (Passengers[i].ent)
		{
			G_TouchTriggers(Passengers[i].ent);
		}
	}

	for (int i = 0; i < MAX_TURRETS; i++)
	{
		if (Turrets[i].ent)
		{
			G_TouchTriggers(Turrets[i].ent);
		}
	}

	if (getMoveType() != MOVETYPE_NOCLIP)
	{
		G_TouchTriggers(this);
	}

	for (i = 0; i < vm->numtouch; i++)
	{
		other = &g_entities[vm->touchents[i]];

		for (j = 0; j < i; j++)
		{
			gentity_t *ge = &g_entities[j];

			if (ge == other)
				break;
		}

		if (j != i)
		{
			// duplicated
			continue;
		}

		// Don't bother touching the world
		if ((!other->entity) || (other->entity == world))
		{
			continue;
		}

		event = new Event(EV_Touch);
		event->AddEntity(this);
		other->entity->ProcessEvent(event);

		event = new Event(EV_Touch);
		event->AddEntity(other->entity);
		ProcessEvent(event);
	}
}

/*
====================
Vehicle::ResetSlots
====================
*/
void Vehicle::ResetSlots
	(
	void
	)
	
{
	driver.ent = NULL;
	driver.boneindex = -1;
	driver.enter_boneindex = -1;
	driver.flags = SLOT_UNUSED;
	lastdriver.ent = NULL;
	lastdriver.boneindex = -1;
	lastdriver.enter_boneindex = -1;
	lastdriver.flags = SLOT_UNUSED;

	for( int i = 0; i < MAX_PASSENGERS; i++ )
	{
		Passengers[ i ].ent = NULL;
		Passengers[ i ].boneindex = -1;
		Passengers[ i ].enter_boneindex = -1;
		Passengers[ i ].flags = SLOT_UNUSED;
	}

	for( int i = 0; i < MAX_TURRETS; i++ )
	{
		Turrets[ i ].ent = NULL;
		Turrets[ i ].boneindex = -1;
		Turrets[ i ].enter_boneindex = -1;
		Turrets[ i ].flags = SLOT_UNUSED;
	}
}

/*
====================
Vehicle::OpenSlotsByModel
====================
*/
void Vehicle::OpenSlotsByModel
	(
	void
	)
	
{
	str bonename;
	int bonenum;
	int boneindex;

	driver.boneindex = gi.Tag_NumForName( edict->tiki, "driver" );
	driver.enter_boneindex = gi.Tag_NumForName( edict->tiki, "driver_enter" );

	if( driver.flags & SLOT_UNUSED )
	{
		driver.ent = NULL;
		driver.flags = SLOT_FREE;
	}

	numPassengers = 0;

	for( bonenum = 0; bonenum < MAX_PASSENGERS; bonenum++ )
	{
		bonename = "passenger" + bonenum;
		boneindex = gi.Tag_NumForName( edict->tiki, bonename.c_str() );

		if( boneindex >= 0 )
		{
			numPassengers++;

			Passengers[ bonenum ].boneindex = boneindex;
			Passengers[ bonenum ].enter_boneindex = gi.Tag_NumForName( edict->tiki, "passenger_enter" + bonenum );

			if( Passengers[ bonenum ].flags & SLOT_UNUSED )
			{
				Passengers[ bonenum ].ent = NULL;
				Passengers[ bonenum ].flags = SLOT_FREE;
			}
		}
	}

	numTurrets = 0;

	for( bonenum = 0; bonenum < MAX_TURRETS; bonenum++ )
	{
		bonename = "turret" + bonenum;
		boneindex = gi.Tag_NumForName( edict->tiki, bonename.c_str() );

		if( boneindex >= 0 )
		{
			numTurrets++;

			Turrets[ bonenum ].boneindex = boneindex;
			Turrets[ bonenum ].enter_boneindex = gi.Tag_NumForName( edict->tiki, "turret_enter" + bonenum );

			if( Turrets[ bonenum ].flags & SLOT_UNUSED )
			{
				Turrets[ bonenum ].ent = NULL;
				Turrets[ bonenum ].flags = SLOT_FREE;
			}
		}
	}
}

/*
====================
Vehicle::MoveVehicle
====================
*/
void Vehicle::MoveVehicle
	(
	void
	)
	
{
	//trace_t tr;
	Vector vecStart;
	Vector vecStart2;
	Vector vecEnd;
	Vector vecDelta;
	Vector vecAng;
	//int i;
	//gridpoint_t *gp;
	vmove_t vm;
	float flMoveFrac = 1.0f;
	//float fSpeed;
	bool bDoGravity = true;
	bool bHitPerson = false;
	//Entity *pSkippedEntities[ MAX_SKIPPED_ENTITIES ];
	//int iContentsEntities[ MAX_SKIPPED_ENTITIES ];
	//solid_t solidEntities[ MAX_SKIPPED_ENTITIES ];
	int iNumSkippedEntities = 0;

	if( m_bMovementLocked )
	{
		return;
	}

	setAngles();
	if( m_pCollisionEntity )
	{
		SetCEMoveInfo( &vm );
	}
	else
	{
		SetMoveInfo( &vm );
	}
	m_sMoveGrid->SetMoveInfo( &vm );
	m_sMoveGrid->SetOrientation( orientation );
	m_sMoveGrid->CalculateBoxPoints();
	CheckGround();

	VmoveSingle( &vm );

	setOrigin( vm.vs->origin );

	// FIXME: stub

	/*if( velocity.length() > 0.5f )
	{
		fSpeed = DotProduct( velocity, orientation[ 0 ] );
		vecDelta = velocity * level.frametime;

		for( i = 0; i < 3; i++ )
		{
			if( fSpeed > 0.0f )
			{
				gp = m_sMoveGrid->GetGridPoint( 0, i, 0 );
				vecStart = gp->origin + origin;
			}
			else
			{
				gp = m_sMoveGrid->GetGridPoint( 2, i, 0 );
				vecStart = gp->origin + origin;
			}

			if( real_velocity.length() > 0.5f )
			{
				vecStart2 = vecDelta + vecStart;

				for( ;; )
				{
					tr = G_Trace( vecStart2, Vector( vm.mins ) - Vector( -32, -32, -32 ), Vector( vm.maxs ) + Vector( 32, 32, 32 ), vecStart2, this, 0x6001382, false, "Vehicle::Move" );

					if( !tr.ent || !tr.ent->entity || tr.ent->entity == world )
					{
						break;
					}

					tr.ent->entity->CheckGround();

					if( !tr.ent->entity->groundentity || tr.ent->entity->groundentity == edict &&
						( !m_pCollisionEntity || tr.ent->entity != m_pCollisionEntity ) )
					{
						Event *event = new Event( EV_Touch );
						event->AddEntity( this );
						tr.ent->entity->ProcessEvent( event );

						event = new Event( EV_Touch );
						event->AddEntity( tr.ent->entity );
						ProcessEvent( event );

						if( tr.ent->entity->IsSubclassOfSentient() )
							bHitPerson = true;

						if( g_showvehiclemovedebug->integer )
						{
							Com_Printf( "Vehicle Hit(MV0): %s : %s\n", tr.ent->entity->getClassname(), tr.ent->entity->targetname.c_str() );
						}

						break;
					}

					pSkippedEntities[ iNumSkippedEntities ] = tr.ent->entity;
					iContentsEntities[ iNumSkippedEntities ] = tr.ent->r.contents;
					solidEntities[ iNumSkippedEntities ] = tr.ent->solid;
					iNumSkippedEntities++;

					if( iNumSkippedEntities >= MAX_SKIPPED_ENTITIES )
					{
						gi.Error( ERR_DROP, "MAX_SKIPPED_ENTITIES hit in VehicleMove.\n" );
						return;
					}

					tr.ent->entity->setSolidType( SOLID_NOT );

					if( g_showvehiclemovedebug->integer )
					{
						Com_Printf( "Vehicle Skipped(MV0): %s : %s\n", tr.ent->entity->getClassname(), tr.ent->entity->targetname.c_str() );
					}
				}
			}

			if( g_showvehiclemovedebug->integer )
			{
				G_DebugBBox( vecStart, vm.mins, vm.maxs, 1.0f, 0.0f, 0.0f, 1.0f );
				G_DebugBBox( vecStart + Vector( 0, 0, 64.0f ), vm.mins, vm.maxs, 1.0f, 0.0f, 0.0f, 1.0f );
			}

			vecEnd = vecStart + vecDelta;

			for( ;; )
			{
				tr = G_Trace( vecStart, Vector( vm.mins ), Vector( vm.maxs ), vecEnd, this, edict->clipmask, false, "Vehicle::Move" );

				if( tr.fraction == 1.0f && !tr.allsolid && !tr.startsolid )
				{
					break;
				}

				if( !tr.ent || !tr.ent->entity || tr.ent->entity == world )
				{
					if( g_showvehiclemovedebug->integer )
					{
						if( flMoveFrac > tr.fraction )
						{
							flMoveFrac = tr.fraction - 0.1f;
						}

						G_DebugBBox( tr.endpos, vm.mins, vm.maxs, 0.0f, 1.0f, 0.0f, 1.0f );
					}

					vecStart = tr.endpos;
					vecEnd = tr.endpos;

					for( ;; )
					{
						tr = G_Trace( vecStart, Vector( vm.mins ), Vector( vm.maxs ), vecEnd, this, edict->clipmask, false, "Vehicle::Move" );

						if( tr.fraction != 1.0f || tr.plane.normal[ 2 ] < 0.7f || tr.allsolid )
						{
							flMoveFrac = 0.0f;
						}

						if( !tr.ent || !tr.ent->entity || tr.ent->entity == world )
						{
							// FIXME
							break;
						}

						tr.ent->entity->CheckGround();

						if( !tr.ent->entity->groundentity )
						{
							break;
						}

						if( tr.ent != edict )
						{
							if( !m_pCollisionEntity )
							{
								break;
							}

							if( tr.ent->entity != m_pCollisionEntity )
							{
								break;
							}
						}

						pSkippedEntities[ iNumSkippedEntities ] = tr.ent->entity;
						iContentsEntities[ iNumSkippedEntities ] = tr.ent->r.contents;
						solidEntities[ iNumSkippedEntities ] = tr.ent->solid;
						iNumSkippedEntities++;

						if( iNumSkippedEntities >= MAX_SKIPPED_ENTITIES )
						{
							gi.Error( ERR_DROP, "MAX_SKIPPED_ENTITIES hit in VehicleMove.\n" );
							return;
						}

						tr.ent->entity->setSolidType( SOLID_NOT );
						if( g_showvehiclemovedebug->integer )
						{
							Com_Printf( "Vehicle Skipped(MV2): %s : %s\n", tr.ent->entity->getClassname(), tr.ent->entity->targetname.c_str() );
						}
					}

					Event *event = new Event( EV_Touch );
					event->AddEntity( this );
					tr.ent->entity->ProcessEvent( event );

					event = new Event( EV_Touch );
					event->AddEntity( tr.ent->entity );
					ProcessEvent( event );

					if( tr.ent->entity->IsSubclassOfPlayer() )
					{
						bHitPerson = true;
					}

					if( g_showvehiclemovedebug->integer )
					{
						Com_Printf( "Vehicle Hit(MV2): %s : %s\n", tr.ent->entity->getClassname(), tr.ent->entity->targetname.c_str() );
						G_DebugBBox( vecStart, gp->vm.mins, gp->vm.maxs, 0.0f, 0.0f, 1.0f, 1.0f );
					}

					if( ( !tr.ent || !tr.ent->entity->IsSubclassOfProjectile() ) && driver.ent && driver.ent->IsSubclassOfPlayer() )
					{
						if( fSpeed > 0.0f )
						{
							if( i )
							{
								if( i == 2 && turnimpulse >= 0.0f )
								{
									turnimpulse -= 800.0f * level.frametime;
								}

								continue;
							}

							if( turnimpulse > 0.0f )
							{
								continue;
							}

							turnimpulse += 800 * level.frametime;
							continue;
						}

						if( i == 0 )
						{
							if( turnimpulse < 0.0f )
								continue;

							turnimpulse -= 800 * level.frametime;
							continue;
						}

						if( i == 2 && turnimpulse <= 0.0f )
						{
							turnimpulse += 800 * level.frametime;
							continue;
						}
					}

					if( flMoveFrac < 0.1f )
					{
						bDoGravity = false;
					}

					break;
				}

				tr.ent->entity->CheckGround();

				if( !tr.ent->entity->groundentity || tr.ent->entity->groundentity == edict &&
					( !m_pCollisionEntity || tr.ent->entity != m_pCollisionEntity ) )
				{
					Event *event = new Event( EV_Touch );
					event->AddEntity( this );
					tr.ent->entity->ProcessEvent( event );

					event = new Event( EV_Touch );
					event->AddEntity( tr.ent->entity );
					ProcessEvent( event );

					if( tr.ent->entity->IsSubclassOfSentient() )
						bHitPerson = true;

					if( g_showvehiclemovedebug->integer )
					{
						Com_Printf( "Vehicle Hit(MV): %s : %s\n", tr.ent->entity->getClassname(), tr.ent->entity->targetname.c_str() );
					}

					break;
				}

				pSkippedEntities[ iNumSkippedEntities ] = tr.ent->entity;
				iContentsEntities[ iNumSkippedEntities ] = tr.ent->r.contents;
				solidEntities[ iNumSkippedEntities ] = tr.ent->solid;
				iNumSkippedEntities++;

				if( iNumSkippedEntities >= MAX_SKIPPED_ENTITIES )
				{
					gi.Error( ERR_DROP, "MAX_SKIPPED_ENTITIES hit in VehicleMove.\n" );
					return;
				}

				tr.ent->entity->setSolidType( SOLID_NOT );

				if( g_showvehiclemovedebug->integer )
				{
					Com_Printf( "Vehicle Skipped(MV): %s : %s\n", tr.ent->entity->getClassname(), tr.ent->entity->targetname.c_str() );
				}
			}
		}
	}*/
}

/*
====================
Vehicle::AssertMove
====================
*/
bool Vehicle::AssertMove
	(
	Vector vNewOrigin,
	Vector vOldOrigin
	)

{
	Entity *check;
	gentity_t *edict;
	int touch[ MAX_GENTITIES ];
	int i;
	int num;

	if( m_pCollisionEntity )
	{
		num = gi.AreaEntities( m_pCollisionEntity->mins, m_pCollisionEntity->maxs, touch, MAX_GENTITIES );
	}
	else
	{
		num = gi.AreaEntities( mins, maxs, touch, MAX_GENTITIES );
	}

	if( num <= 0 )
	{
		return true;
	}

	for( i = 0; i < num; i++ )
	{
		edict = &g_entities[ touch[ i ] ];
		check = edict->entity;

		if( check->edict->s.number != edict->s.number && edict->solid && check->movetype != MOVETYPE_STOP )
		{
			if( check->movetype != MOVETYPE_NONE
				&& check->movetype != MOVETYPE_NOCLIP
				&& edict->r.contents != CONTENTS_PLAYERCLIP
				&& IsTouching( check )
				&& G_TestEntityPosition( check, check->origin ) )
			{
				return false;
			}
		}
	}

	return true;
}

/*
====================
Vehicle::AssertRotation
====================
*/
bool Vehicle::AssertRotation
	(
	Vector vNewAngles,
	Vector vOldAngles
	)

{
	Vector i;
	Vector j;
	Vector k;
	Vector i2;
	Vector j2;
	Vector k2;
	Vector vAngleDiff;
	float mAngleDiff[ 3 ][ 3 ];

	AngleVectorsLeft( vOldAngles, i, j, k );
	AngleVectorsLeft( vNewAngles, i2, j2, k2 );
	AnglesSubtract( vOldAngles, vNewAngles, vAngleDiff );
	AngleVectorsLeft( vAngleDiff, mAngleDiff[ 0 ], mAngleDiff[ 1 ], mAngleDiff[ 2 ] );

	// FIXME: not sure what it is supposed to do. Should we put an assert there ?

	return true;
}

/*
====================
Vehicle::NoMove
====================
*/
void Vehicle::NoMove
	(
	void
	)
	
{
	vmove_t vm;

	SetMoveInfo( &vm );
	VectorClear2D( vs.desired_dir );
	VmoveSingle( &vm );
	GetMoveInfo( &vm );
}

/*
====================
Vehicle::SlidePush
====================
*/
void Vehicle::SlidePush
	(
	Vector vPush
	)

{
	vmove_t vm;
	int i;
	int j;
	Entity *pSkippedEntities[ MAX_SKIPPED_ENTITIES ];
	int iContentsEntities[ MAX_SKIPPED_ENTITIES ];
	solid_t solidEntities[ MAX_SKIPPED_ENTITIES ];
	int iNumSkippedEntities = 0;
	int iNumSkipped = 0;
	gentity_t *other;
	Vector newOrigin;

	do
	{
		SetMoveInfo( &vm );

		VectorCopy( vPush, vs.velocity );
		vs.desired_dir[ 0 ] = vPush[ 0 ];
		vs.desired_dir[ 1 ] = vPush[ 1 ];
		VectorNormalize2D( vs.desired_dir );

		if( g_showvehiclemovedebug->integer )
		{
			G_DebugBBox( origin, vm.mins, vm.maxs, 1.0f, 0.0f, 0.0f, 1.0f );
			G_DebugBBox( origin, vm.mins, vm.maxs, 0.0f, 1.0f, 0.0f, 1.0f );
		}

		VmoveSingle( &vm );

		iNumSkippedEntities = 0;

		for( i = 0; i < vm.numtouch; i++ )
		{
			other = &g_entities[ vm.touchents[ i ] ];

			for( j = 0; j < i; j++ )
			{
				if( &g_entities[ j ] == other )
				{
					break;
				}
			}

			if( j == i && other->entity )
			{
				other->entity->CheckGround();

				if( other->entity->groundentity && ( other->entity->groundentity == edict || other->entity == m_pCollisionEntity ) )
				{
					// save the entity
					pSkippedEntities[ iNumSkipped ] = other->entity;
					iContentsEntities[ iNumSkipped ] = other->r.contents;
					solidEntities[ iNumSkipped ] = other->solid;
					iNumSkipped++;

					if( iNumSkipped >= MAX_SKIPPED_ENTITIES )
					{
						gi.Error( ERR_DROP, "MAX_SKIPPED_ENTITIES hit in VehicleMove.\n" );
						return;
					}

					other->entity->setSolidType( SOLID_NOT );
					iNumSkippedEntities++;
				}

				if( g_showvehiclemovedebug->integer )
				{
					Com_Printf( "Vehicle Hit(SP): %s : %s\n", other->entity->getClassname(), other->entity->targetname.c_str() );
				}
			}
		}
	} while( iNumSkippedEntities != 0 );

	if( bindmaster )
	{
		newOrigin = vm.vs->origin - bindmaster->origin;
	}
	else
	{
		newOrigin = vm.vs->origin;
	}

	setLocalOrigin( newOrigin );

	if( g_showvehiclemovedebug->integer )
	{
		G_DebugBBox( origin, vm.mins, vm.maxs, 0, 0, 1.0f, 1.0f );
	}

	TouchStuff( &vm );

	for( i = 0; i < iNumSkipped; i++ )
	{
		pSkippedEntities[ i ]->setSolidType( solidEntities[ i ] );
		pSkippedEntities[ i ]->edict->r.contents = iContentsEntities[ i ];
	}
}

/*
====================
Vehicle::SetupPath
====================
*/
void Vehicle::SetupPath
	(
	cSpline<4, 512> *pPath,
	SimpleEntity *se
	)

{
	Vector vLastOrigin;
	SimpleEntity *ent;
	float fCurLength = 0.0f;
	int i = 1;

	if( !pPath )
	{
		return;
	}

	pPath->Reset();

	if( !se->target.length() || !*se->target )
	{
		return;
	}

	vLastOrigin = se->origin;

	for( ent = se; ent != NULL; ent = ent->Next(), i++ )
	{
		Vector vDelta = vLastOrigin - ent->origin;
		float vTmp[ 4 ];

		if( vDelta.length() == 0.0f && i > 1 )
		{
			Com_Printf( "^~^~^Warning: Vehicle Driving with a Path that contains 2 equal points\n" );
		}
		else
		{
			vLastOrigin = ent->origin;
			fCurLength += vDelta.length();

			vTmp[ 0 ] = fCurLength;
			VectorCopy( vTmp + 1, ent->origin );

			if( ent->IsSubclassOfVehiclePoint() )
			{
				pPath->Add( vTmp, ( ( VehiclePoint * )ent )->spawnflags );
			}
			else
			{
				pPath->Add( vTmp, 0 );
			}
		}

		if( ent == se && i > 1 )
		{
			break;
		}
	}
}

/*
====================
Vehicle::UpdateSound
====================
*/
void Vehicle::UpdateSound
	(
	void
	)
	
{
	float pitch;
	float volume;

	if( level.time < m_fNextSoundState )
	{
		return;
	}

	// Calculate the pitch based on the vehicle's speed
	pitch = ( velocity.length() - m_fSoundMinSpeed ) / ( m_fSoundMaxSpeed - m_fSoundMinSpeed );

	if( pitch > 1.0f )
	{
		pitch = 1.0f;
	}
	else if( pitch < 0.0f )
	{
		pitch = 0.0f;
	}

	pitch = m_fSoundMinPitch + ( m_fSoundMaxPitch - m_fSoundMinPitch ) * pitch;

	volume = ( velocity.length() - m_fVolumeMinSpeed ) / ( m_fVolumeMaxSpeed - m_fVolumeMinSpeed );

	if( volume > 1.0f )
	{
		volume = 1.0f;
	}
	else if( volume < 0.0f )
	{
		volume = 0.0f;
	}

	volume = this->m_fVolumeMinPitch + ( this->m_fVolumeMaxPitch - this->m_fVolumeMinPitch ) * volume;

	switch( m_eSoundState )
	{
	case ST_OFF:
		StopLoopSound();
		TurnOffVehicleSoundEntities();
		m_fNextSoundState = level.time;

		if( driver.ent || m_bAutoPilot )
		{
			m_eSoundState = ST_OFF_TRANS_IDLE;
		}
		break;

	case ST_OFF_TRANS_IDLE:
		m_fNextSoundState = level.time;
		m_eSoundState = ST_IDLE;
		Sound( m_sSoundSet + "snd_on" );
		LoopSound( m_sSoundSet + "snd_idle" );
		break;

	case ST_IDLE_TRANS_OFF:
		m_fNextSoundState = level.time;
		m_eSoundState = ST_OFF;
		Sound( m_sSoundSet + "snd_off" );
		StopLoopSound();
		break;

	case ST_IDLE:
		m_fNextSoundState = level.time;

		if( driver.ent || m_bAutoPilot )
		{
			if( fabs( DotProduct( orientation[ 0 ], velocity ) ) > 10.0f )
			{
				m_eSoundState = ST_IDLE_TRANS_RUN;
			}
		}
		else
		{
			m_eSoundState = ST_IDLE_TRANS_OFF;
		}

		LoopSound( m_sSoundSet + "snd_idle" );

		TurnOffVehicleSoundEntities();
		break;

	case ST_IDLE_TRANS_RUN:
		m_fNextSoundState = level.time;
		m_eSoundState = ST_RUN;

		Sound( m_sSoundSet + "snd_revup" );
		LoopSound( m_sSoundSet + "snd_run", -1.0f, -1.0f, -1.0f, pitch );
		break;

	case ST_RUN:
		m_fNextSoundState = level.time;

		if( fabs( DotProduct( orientation[ 0 ], velocity ) ) < 10.0f )
		{
			m_eSoundState = ST_RUN_TRANS_IDLE;
		}

		TurnOnVehicleSoundEntities();
		LoopSound( m_sSoundSet + "snd_run", volume, -1.0f, -1.0f, pitch );
		break;

	case ST_RUN_TRANS_IDLE:
		m_fNextSoundState = level.time;
		m_eSoundState = ST_IDLE;

		Sound( m_sSoundSet + "snd_revdown" );
		LoopSound( m_sSoundSet + "snd_idle" );
		break;

	default:
		m_fNextSoundState = level.time;
		m_eSoundState = ST_OFF;
		break;
	}
}

/*
====================
Vehicle::SetupVehicleSoundEntities
====================
*/
void Vehicle::SetupVehicleSoundEntities
	(
	void
	)
	
{
	int i;
	Vector a;
	Vector b;
	Vector c;
	Vector start;

	angles.AngleVectorsLeft( &a, &b, &c );

	// place the sound entities in the vehicle wheels
	for( i = 0; i < MAX_SOUND_ENTITIES; i++ )
	{
		if( !m_pVehicleSoundEntities[ i ] )
		{
			m_pVehicleSoundEntities[ i ] = new VehicleSoundEntity( this );
		}

		start = origin + a * Corners[ i ][ 0 ] + b * Corners[ i ][ 1 ] + c * Corners[ i ][ 2 ];
		m_pVehicleSoundEntities[ i ]->setOrigin( start );
	}
}

/*
====================
Vehicle::RemoveVehicleSoundEntities
====================
*/
void Vehicle::RemoveVehicleSoundEntities
	(
	void
	)
	
{
	for( int i = 0; i < MAX_SOUND_ENTITIES; i++ )
	{
		if( !m_pVehicleSoundEntities[ i ] )
		{
			continue;
		}

		m_pVehicleSoundEntities[ i ]->PostEvent( EV_Remove, EV_VEHICLE );
	}
}

/*
====================
Vehicle::TurnOnVehicleSoundEntities
====================
*/
void Vehicle::TurnOnVehicleSoundEntities
	(
	void
	)
	
{
	for( int i = 0; i < MAX_SOUND_ENTITIES; i++ )
	{
		if( !m_pVehicleSoundEntities[ i ] )
		{
			m_pVehicleSoundEntities[ i ] = new VehicleSoundEntity( this );
		}

		m_pVehicleSoundEntities[ i ]->Start();
	}
}

/*
====================
Vehicle::TurnOffVehicleSoundEntities
====================
*/
void Vehicle::TurnOffVehicleSoundEntities
	(
	void
	)
	
{
	for( int i = 0; i < MAX_SOUND_ENTITIES; i++ )
	{
		if( !m_pVehicleSoundEntities[ i ] )
		{
			m_pVehicleSoundEntities[ i ] = new VehicleSoundEntity( this );
		}

		m_pVehicleSoundEntities[ i ]->Stop();
	}
}

/*
====================
Vehicle::UpdateTires
====================
*/
void Vehicle::UpdateTires
	(
	void
	)
	
{
	int index;
	trace_t trace;
	Vector a;
	Vector b;
	Vector c;
	Vector vTmp;
	Vector t_mins;
	Vector t_maxs;
	Vector start;
	Vector end;
	Vector boxoffset;
	Entity *pSkippedEntities[ MAX_SKIPPED_ENTITIES ];
	int iContentsEntities[ MAX_SKIPPED_ENTITIES ];
	solid_t solidEntities[ MAX_SKIPPED_ENTITIES ];
	int iNumSkippedEntities;
	int iNumSkipped = 0;

	t_mins = mins * 0.25f;
	t_maxs = maxs * 0.25f;

	if( real_velocity.length() <= 0.5f && m_iLastTiresUpdate != -1 )
	{
		if( m_iLastTiresUpdate + 1000 > level.inttime )
			return;
	}

	m_iLastTiresUpdate = level.inttime;

	vTmp[ 1 ] = angles[ 1 ] + m_fSkidAngle;
	AngleVectors( vTmp, a, b, c );

	// Temporary make slots non-solid for G_Trace
	SetSlotsNonSolid();

	do
	{
		iNumSkippedEntities = 0;

		for( index = 0; index < MAX_CORNERS; index++ )
		{
			boxoffset = Corners[ index ];
			start = origin + a * boxoffset[ 0 ] + b * boxoffset[ 1 ] + c * boxoffset[ 2 ];
			end = start;
			end[ 2 ] -= 400.0f;

			trace = G_Trace( start, t_mins, t_maxs, end, this, MASK_VEHICLE, false, "Vehicle::PostThink Corners" );

			if( g_showvehiclemovedebug->integer )
			{
				G_DebugBBox( origin, start, end, 1.0f, 1.0f, 1.0f, 1.0f );
				G_DebugBBox( origin, start, trace.endpos, 1.0f, 0.0f, 0.0f, 1.0f );
			}

			if( trace.ent && trace.ent->entity && trace.ent->entity->isSubclassOf( VehicleCollisionEntity ) )
			{
				// save the entity
				pSkippedEntities[ iNumSkipped ] = trace.ent->entity;
				iContentsEntities[ iNumSkipped ] = trace.ent->r.contents;
				solidEntities[ iNumSkipped ] = trace.ent->solid;
				iNumSkipped++;

				if( iNumSkipped >= MAX_SKIPPED_ENTITIES )
				{
					gi.Error( ERR_DROP, "MAX_SKIPPED_ENTITIES hit in VehicleMove.\n" );
					return;
				}

				trace.ent->entity->setSolidType( SOLID_NOT );
				iNumSkippedEntities++;
			}

			if( trace.fraction == 1.0 )
			{
				m_bTireHit[ index ] = false;
			}
			else
			{
				m_vTireEnd[ index ] = trace.endpos;
				m_bTireHit[ index ] = true;
			}
		}
	} while( iNumSkippedEntities != 0 );

	for( index = 0; index < iNumSkipped; index++ )
	{
		pSkippedEntities[ index ]->setSolidType( solidEntities[ index ] );
		pSkippedEntities[ index ]->edict->r.contents = iContentsEntities[ index ];
	}

	// Turn slots back into a solid state
	SetSlotsSolid();
}

/*
====================
Vehicle::UpdateNormals
====================
*/
void Vehicle::UpdateNormals
	(
	void
	)
	
{
	Vector vDist1;
	Vector vDist2;
	Vector vCross;
	Vector temp;
	Vector pitch;
	Vector i;
	Vector j;

	if (real_velocity.length() <= 0.5)
	{
		if (m_iLastTiresUpdate != -1 && m_iLastTiresUpdate + 1000 > level.inttime)
			return;
	}

	AngleVectorsLeft(angles, NULL, pitch, NULL);

	m_vNormalSum = vec_zero;

	pitch = -pitch;

	m_iNumNormals = 0;

	if (m_bTireHit[0] && m_bTireHit[1] && m_bTireHit[2])
	{
		vDist1 = m_vTireEnd[1] - m_vTireEnd[0];
		vDist2 = m_vTireEnd[1] - m_vTireEnd[2];
		
		vCross.CrossProduct(vDist1, vDist2);
		VectorNormalize(vCross);

		m_vNormalSum += vCross;

		m_iNumNormals++;
	}

	if (m_bTireHit[1] && m_bTireHit[2] && m_bTireHit[3])
	{
		vDist1 = m_vTireEnd[2] - m_vTireEnd[1];
		vDist2 = m_vTireEnd[2] - m_vTireEnd[3];
		
		vCross.CrossProduct(vDist1, vDist2);
		VectorNormalize(vCross);

		m_vNormalSum += vCross;

		m_iNumNormals++;
	}

	if (m_bTireHit[2] && m_bTireHit[3] && m_bTireHit[0])
	{
		vDist1 = m_vTireEnd[3] - m_vTireEnd[0];
		vDist2 = m_vTireEnd[3] - m_vTireEnd[2];

		vCross.CrossProduct(vDist1, vDist2);
		VectorNormalize(vCross);

		m_vNormalSum += vCross;

		m_iNumNormals++;
	}

	if (m_bTireHit[3] && m_bTireHit[0] && m_bTireHit[1])
	{
		vDist1 = m_vTireEnd[0] - m_vTireEnd[3];
		vDist2 = m_vTireEnd[0] - m_vTireEnd[1];

		vCross.CrossProduct(vDist1, vDist2);
		VectorNormalize(vCross);

		m_vNormalSum += vCross;

		m_iNumNormals++;
	}

	if (m_iNumNormals > 1)
	{
		temp = m_vNormalSum / m_iNumNormals;

		i.CrossProduct(temp, pitch);

		angles[0] = i.toPitch();

		j.CrossProduct(temp, i);

		angles[2] = j.toPitch();
	}
}

/*
====================
Vehicle::UpdateBones
====================
*/
void Vehicle::UpdateBones
	(
	void
	)
	
{
	float fNewTurnAngle = AngleNormalize180( turnangle - m_fSkidAngle );

	if( fabs( fNewTurnAngle ) > maxturnrate )
	{
		fNewTurnAngle = maxturnrate;
	}

	SetControllerAngles( 0, Vector( 0, fNewTurnAngle, 0 ) );
	SetControllerAngles( 1, Vector( 0, fNewTurnAngle, 0 ) );
}

/*
====================
Vehicle::UpdateShaderOffset
====================
*/
void Vehicle::UpdateShaderOffset
	(
	void
	)
	
{
	m_fShaderOffset -= orientation[ 0 ] * real_velocity * 0.25 * level.frametime;
	edict->s.shader_time = m_fShaderOffset;
}

/*
====================
Vehicle::UpdateTurretSlot
====================
*/
void Vehicle::UpdateTurretSlot
	(
	int iSlot
	)

{
	orientation_t orient;

	if( !( Turrets[ iSlot ].flags & SLOT_BUSY ) || !Turrets[ iSlot ].ent )
	{
		return;
	}

	if( Turrets[ iSlot ].boneindex != -1 )
	{
		GetTag( Turrets[ iSlot ].boneindex, &orient );
		if( Turrets[ iSlot ].ent->IsSubclassOfActor() )
		{
			Turrets[ iSlot ].ent->setOriginEvent( orient.origin );
		}
		else
		{
			Turrets[ iSlot ].ent->setOrigin( orient.origin );
		}
	}
	else
	{
		Vector forward = orientation[ 0 ];
		Vector left = orientation[ 1 ];
		Vector up = orientation[ 2 ];

		if( Turrets[ iSlot ].ent->IsSubclassOfActor() )
		{
			Turrets[ iSlot ].ent->setOriginEvent( origin );
		}
		else
		{
			Turrets[ iSlot ].ent->setOrigin( origin );
		}
	}

	Turrets[ iSlot ].ent->avelocity = avelocity;
	Turrets[ iSlot ].ent->velocity = velocity;

	if( !Turrets[ iSlot ].ent->IsSubclassOfActor() || ( ( Actor * )Turrets[ iSlot ].ent.Pointer() )->m_Enemy )
	{
		Turrets[ iSlot ].ent->setAngles( angles );
	}
}

/*
====================
Vehicle::UpdatePassengerSlot
====================
*/
void Vehicle::UpdatePassengerSlot
	(
	int iSlot
	)

{
	orientation_t orient;

	if( !( Passengers[ iSlot ].flags & SLOT_BUSY ) || !Passengers[ iSlot ].ent )
	{
		return;
	}

	if( Passengers[ iSlot ].boneindex != -1 )
	{
		GetTag( Passengers[ iSlot ].boneindex, &orient );
		if( Passengers[ iSlot ].ent->IsSubclassOfActor() )
		{
			Passengers[ iSlot ].ent->setOriginEvent( orient.origin );
		}
		else
		{
			Passengers[ iSlot ].ent->setOrigin( orient.origin );
		}
	}
	else
	{
		if( Passengers[ iSlot ].ent->IsSubclassOfActor() )
		{
			Passengers[ iSlot ].ent->setOriginEvent( origin );
		}
		else
		{
			Passengers[ iSlot ].ent->setOrigin( origin );
		}
	}

	Passengers[ iSlot ].ent->avelocity = avelocity;
	Passengers[ iSlot ].ent->velocity = velocity;

	if( !Passengers[ iSlot ].ent->IsSubclassOfActor() || ( ( Actor * )Passengers[ iSlot ].ent.Pointer() )->m_Enemy )
	{
		Passengers[ iSlot ].ent->setAngles( angles );
	}
}

/*
====================
Vehicle::UpdateDriverSlot
====================
*/
void Vehicle::UpdateDriverSlot
	(
	int iSlot
	)

{
	orientation_t orient;

	if( !( driver.flags & SLOT_BUSY ) || !driver.ent )
	{
		return;
	}

	if( driver.boneindex != -1 )
	{
		GetTag( driver.boneindex, &orient );
		if( driver.ent->IsSubclassOfActor() )
		{
			driver.ent->setOriginEvent( orient.origin );
		}
		else
		{
			driver.ent->setOrigin( orient.origin );
		}
	}
	else
	{
		Vector forward = orientation[ 0 ];
		Vector left = orientation[ 1 ];
		Vector up = orientation[ 2 ];

		if( driver.ent->IsSubclassOfActor() )
		{
			driver.ent->setOriginEvent( origin + forward * driveroffset[ 0 ] + left * driveroffset[ 1 ] + up * driveroffset[ 2 ] );
		}
		else
		{
			driver.ent->setOrigin( origin + forward * driveroffset[ 0 ] + left * driveroffset[ 1 ] + up * driveroffset[ 2 ] );
		}
	}

	if( drivable )
	{
		driver.ent->avelocity = avelocity;
		driver.ent->velocity = velocity;
		driver.ent->setAngles( angles );
	}
}

/*
====================
Vehicle::UpdateSkidAngle
====================
*/
void Vehicle::UpdateSkidAngle
	(
	void
	)

{
	if( m_bEnableSkidding )
	{
		if( g_showvehiclemovedebug && g_showvehiclemovedebug->integer )
		{
			Com_Printf( "Skidding!\n" );
		}

		m_fSkidLeftForce += velocity.length() / 150.0f * turnangle;
		m_fSkidRightForce += -m_fSkidAngle * 0.2;
		m_fSkidRightForce *= 0.3f;
		m_fSkidAngle = m_fSkidAngle + ( m_fSkidLeftForce + m_fSkidRightForce ) * 22.0f * level.frametime;
		m_vSkidOrigin[ 0 ] = -fabs( m_fSkidAngle );
	}
	else
	{
		m_fSkidAngle = 0;
	}
}

/*
====================
Vehicle::GetPathPosition
====================
*/
float Vehicle::GetPathPosition
	(
	cSpline<4, 512> *pPath,
	int iNode
	)

{
	float *vTmp;
	float vPrev[ 3 ];
	float vCur[ 3 ];
	float vTotal[ 3 ];
	Vector vDelta;
	float fTotal;
	float fCoef;

	vTmp = pPath->GetByNode( iNode, NULL );
	VectorCopy( vCur, vTmp + 1 );

	if( g_showvehiclemovedebug->integer )
	{
		G_DebugString( Vector( vTmp[ 1 ], vTmp[ 2 ], vTmp[ 3 ] ), 3.0f, 1.0f, 1.0f, 1.0f, "%f", vTmp[ 0 ] );
	}

	vTmp = pPath->GetByNode( iNode - 1, NULL );
	VectorCopy( vPrev, vTmp + 1 );

	if( g_showvehiclemovedebug->integer )
	{
		G_DebugString( Vector( vTmp[ 1 ], vTmp[ 2 ], vTmp[ 3 ] ), 3.0f, 1.0f, 1.0f, 1.0f, "%f", vTmp[ 0 ] );
	}

	VectorCopy( Vector( vCur ) - Vector( vPrev ), vTotal );
	m_vIdealDir = vTotal;
	fTotal = m_vIdealDir.length();
	VectorNormalize( m_vIdealDir );
	angles.AngleVectorsLeft( &vDelta, NULL, NULL );

	fCoef = ProjectLineOnPlane( vDelta, DotProduct( vDelta, origin ), vPrev, vCur, NULL );

	if( g_showvehiclemovedebug->integer )
	{
		G_DebugBBox( vPrev, Vector( -32.0f, -32.0f, -32.0f ), Vector( 32.0f, 32.0f, 32.0f ), 0, 1.0f, 1.0f, 1.0f );
		G_DebugBBox( vCur, Vector( -32.0f, -32.0f, -32.0f ), Vector( 32.0f, 32.0f, 32.0f ), 1.0f, 1.0f, 0, 1.0f );
		G_DebugArrow( vCur, m_vIdealDir * -1.0f, ( 1.0 - fCoef ) * fTotal, 0, 1.0f, 0, 1.0f );
		G_DebugArrow( vPrev, m_vIdealDir, fCoef * fTotal, 0, 0, 1.0f, 1.0f );
	}

	return *pPath->GetByNode( iNode - ( 1.0 - fCoef ), NULL );
}

/*
====================
Vehicle::FactorInSkidOrigin
====================
*/
void Vehicle::FactorInSkidOrigin
	(
	void
	)
	
{
	Vector vNewOrigin;

	vNewOrigin[ 0 ] = orientation[ 0 ][ 0 ] * m_vSkidOrigin[ 0 ] + orientation[ 1 ][ 0 ] * m_vSkidOrigin[ 1 ] + orientation[ 2 ][ 0 ] * m_vSkidOrigin[ 2 ];
	vNewOrigin[ 1 ] = orientation[ 0 ][ 1 ] * m_vSkidOrigin[ 0 ] + orientation[ 1 ][ 1 ] * m_vSkidOrigin[ 1 ] + orientation[ 2 ][ 1 ] * m_vSkidOrigin[ 2 ];
	vNewOrigin[ 2 ] = orientation[ 0 ][ 2 ] * m_vSkidOrigin[ 0 ] + orientation[ 1 ][ 2 ] * m_vSkidOrigin[ 1 ] + orientation[ 2 ][ 2 ] * m_vSkidOrigin[ 2 ];

	m_vOriginOffset2 += vNewOrigin;
}

/*
====================
Vehicle::FactorInOriginOffset
====================
*/
void Vehicle::FactorInOriginOffset
	(
	void
	)
	
{
	origin += m_vOriginOffset;
	setOrigin( origin );
}

/*
====================
Vehicle::CalculateOriginOffset
====================
*/
void Vehicle::CalculateOriginOffset
	(
	void
	)
	
{
	int index;
	Vector vTireAvg;
	Vector vMissHit;
	Vector temp;
	int iNum = 0;
	Vector acceleration;

	m_vOriginOffset += m_vOriginOffset2;
	m_vOriginOffset2 = vec_zero;

	for( index = 0; index < MAX_CORNERS; index++ )
	{
		if( m_bTireHit[ index ] )
		{
			iNum++;
			temp = m_vTireEnd[ index ];
			vTireAvg += origin - temp;
		}
		else
		{
			temp = Corners[ index ];
			vMissHit = temp;
		}
	}

	if( iNum == 3 )
	{
		temp = m_vNormalSum * ( 1.0f / m_iNumNormals );
		ProjectPointOnPlane( acceleration, vMissHit, temp );
		vTireAvg += acceleration;
	}
	else if( iNum == 4 )
	{
		vTireAvg *= 0.25f;
		MatrixTransformVector( m_vOriginCornerOffset, orientation, acceleration );
		vTireAvg -= acceleration;
		m_vOriginOffset2 += vTireAvg;
	}

	m_vOriginOffset2[ 0 ] += orientation[ 0 ][ 0 ] * m_vSkidOrigin[ 0 ] + orientation[ 1 ][ 0 ] * m_vSkidOrigin[ 1 ] + orientation[ 2 ][ 0 ] * m_vSkidOrigin[ 2 ];
	m_vOriginOffset2[ 1 ] += orientation[ 0 ][ 1 ] * m_vSkidOrigin[ 0 ] + orientation[ 1 ][ 1 ] * m_vSkidOrigin[ 1 ] + orientation[ 2 ][ 1 ] * m_vSkidOrigin[ 2 ];
	m_vOriginOffset2[ 2 ] += orientation[ 0 ][ 2 ] * m_vSkidOrigin[ 0 ] + orientation[ 1 ][ 2 ] * m_vSkidOrigin[ 1 ] + orientation[ 2 ][ 2 ] * m_vSkidOrigin[ 2 ];

	Vector vTmp = real_acceleration - prev_acceleration;

	m_fDownForce = vTmp[ 2 ] * m_fZCoef;

	if( m_fDownForce > m_fZMax )
	{
		m_fDownForce = m_fZMax;
	}
	else if( m_fDownForce < m_fZMin )
	{
		m_fDownForce = m_fZMin;
	}

	m_fUpForce = ( -m_vOriginOffset[ 2 ] * m_fBouncyCoef + m_fUpForce ) * m_fSpringyCoef;
	m_vOriginOffset2[ 2 ] += ( m_fDownForce + m_fUpForce ) * 12.0 * level.frametime;

	if( m_vOriginOffset2[ 2 ] > m_fZMax )
	{
		m_vOriginOffset2[ 2 ] = m_fZMax;
	}
	else if( m_vOriginOffset2[ 2 ] < m_fZMin )
	{
		m_vOriginOffset2[ 2 ] = m_fZMin;
	}

	m_vOriginOffset -= m_vOriginOffset2;
}

/*
====================
Vehicle::FactorOutOriginOffset
====================
*/
void Vehicle::FactorOutOriginOffset
	(
	void
	)
	
{
	origin -= m_vOriginOffset;
	setOrigin( origin );
}

/*
====================
Vehicle::FactorInAnglesOffset
====================
*/
void Vehicle::FactorInAnglesOffset
	(
	Vector *vAddedAngles
	)

{
	( *vAddedAngles ) += m_vAnglesOffset;
	( *vAddedAngles )[ 1 ] += m_fSkidAngle;
}

/*
====================
Vehicle::CalculateAnglesOffset
====================
*/
void Vehicle::CalculateAnglesOffset
	(
	Vector acceleration
	)

{
	if( level.time <= 1200 )
	{
		return;
	}

	m_fForwardForce += DotProduct( orientation[ 0 ], acceleration ) * m_fYawCoef;
	m_fBackForce = ( -m_vAnglesOffset[ 0 ] * m_fBouncyCoef + m_fBackForce ) * m_fSpringyCoef;

	m_vAnglesOffset[ 0 ] += m_fForwardForce + m_fBackForce * 12.0 * level.frametime;

	if( m_vAnglesOffset[ 0 ] > m_fYawMax )
	{
		m_vAnglesOffset[ 0 ] = m_fYawMax;
	}
	else if( m_vAnglesOffset[ 0 ] < m_fYawMin )
	{
		m_vAnglesOffset[ 0 ] = m_fYawMin;
	}

	m_fForwardForce = 0;
	m_fLeftForce += DotProduct( orientation[ 1 ], acceleration ) * m_fRollCoef;
	m_fRightForce = ( -m_vAnglesOffset[ 2 ] * m_fBouncyCoef + m_fRightForce ) * m_fSpringyCoef;

	m_vAnglesOffset[ 2 ] += 12.0 * ( m_fLeftForce + m_fRightForce ) *level.frametime;

	if( m_vAnglesOffset[ 2 ] > m_fRollMax )
	{
		m_vAnglesOffset[ 2 ] = m_fRollMax;
	}
	else if( m_vAnglesOffset[ 2 ] < m_fRollMin )
	{
		m_vAnglesOffset[ 2 ] = m_fRollMin;
	}

	m_fLeftForce = 0;
}

/*
====================
Vehicle::FactorOutAnglesOffset
====================
*/
void Vehicle::FactorOutAnglesOffset
	(
	void
	)
	
{

}

/*
====================
Vehicle::GetTagPositionAndOrientation
====================
*/
qboolean Vehicle::GetTagPositionAndOrientation
	(
	str tagname,
	orientation_t *new_or
	)

{
	int tagnum = gi.Tag_NumForName( edict->tiki, tagname.c_str() );

	if( tagnum < 0 )
	{
		warning( "Vehicle::GetTagPositionAndOrientation", "Could not find tag \"%s\"", tagname.c_str() );
		return false;
	}
	else
	{
		return GetTagPositionAndOrientation( tagnum, new_or );
	}
}

/*
====================
Vehicle::GetTagPositionAndOrientation
====================
*/
qboolean Vehicle::GetTagPositionAndOrientation
	(
	int tagnum,
	orientation_t *new_or
	)

{
	int i;
	orientation_t tag_or;
	float axis[ 3 ][ 3 ];

	GetRawTag( tagnum, &tag_or );

	AnglesToAxis( angles, axis );

	VectorCopy( origin, new_or->origin );

	for( i = 0; i < 3; i++ )
	{
		VectorMA( new_or->origin, tag_or.origin[ i ], tag_or.axis[ i ], new_or->origin );
	}

	MatrixMultiply( tag_or.axis, axis, new_or->axis );
	return true;
}

/*
====================
Vehicle::CalculateAnimationData
====================
*/
void Vehicle::CalculateAnimationData
	(
	Vector vAngles,
	Vector vOrigin
	)

{
	float fLeft = fEpsilon();
	float fRight = fEpsilon();
	float fForward = 0;
	float fBack = fEpsilon();
	float fLow = fEpsilon();

	if( vAngles[ 1 ] < 0.0 )
	{
		fBack = vAngles[ 1 ] / m_fYawMin;
	}
	else if( vAngles[ 1 ] > 0.0 )
	{
		fForward = vAngles[ 1 ] / m_fYawMax;
	}

	if( vAngles[ 2 ] > 0.0 )
	{
		fRight = vAngles[ 2 ] / m_fRollMin;
	}
	else if( vAngles[ 2 ] < 0.0 )
	{
		fLeft = vAngles[ 2 ] / m_fRollMax;
	}

	if( vOrigin[ 2 ] < 0.0 )
	{
		fBack = vOrigin[ 2 ] / m_fZMin;
	}
	else if( vOrigin[ 2 ] > 0.0 )
	{
		fForward = vOrigin[ 2 ] / m_fZMax;
	}

	NewAnim( "idle", 0 );
	NewAnim( "lean_left", 0, 3, fLeft );
	NewAnim( "lean_right", 0, 4, fRight );
	NewAnim( "lean_forward", 0, 1, fForward );
	NewAnim( "lean_back", 0, 2, fBack );
	NewAnim( "high", 0, 6, fEpsilon() );
	NewAnim( "low", 0, 5, fEpsilon() );
}

/*
====================
Vehicle::IsDamagedBy

Returns whether or not the vehicle is damaged by the specified entity.
====================
*/
bool Vehicle::IsDamagedBy
	(
	Entity *ent
	)

{
	int i = FindDriverSlotByEntity( ent );

	if( i == -1 )
	{
		i = FindPassengerSlotByEntity( ent );

		if( i == -1 )
		{
			i = FindTurretSlotByEntity( ent );

			if( i == -1 )
			{
				for( i = 0; i < MAX_TURRETS; i++ )
				{
					TurretGun *pTurret = ( TurretGun * )Turrets[ i ].ent.Pointer();

					if( !pTurret )
					{
						continue;
					}

					if( pTurret->IsSubclassOfTurretGun() && pTurret->GetOwner() == ent )
					{
						return false;
					}
				}

				return true;
			}
		}
	}

	return false;
}

/*
====================
Vehicle::AutoPilot
====================
*/
void Vehicle::AutoPilot
	(
	void
	)

{
	float *vTmp;

	if (!m_pCurPath || m_pCurPath->m_iPoints == 0)
	{
		m_bAutoPilot = false;
		return;
	}

	if (g_showvehiclepath && g_showvehiclepath->integer)
	{
		int iFlags = 0;
		float fZ;
		Vector vTmp1;
		Vector vTmp2;
		for (int i = 0; i < m_pCurPath->m_iPoints; i++)
		{
			vTmp = m_pCurPath->GetByNode(i, &iFlags);
			vTmp1 = vTmp + 1;
			fZ = 0;
			//FIXME: macros
			if (iFlags & 1)
			{
				fZ = 1;
				G_DebugString(vTmp1 + Vector(0, 0, 32), sin(level.time) + 3, 1, 1, 0, "START_STOPPING");
			}
			if (iFlags & 2)
			{
				G_DebugString(vTmp1 + Vector(0, 0, (fZ + 1) * 32), sin(level.time) + 3, 0, 1, 0, "START_SKIDDING");
				fZ++;
			}
			if (iFlags & 4)
			{
				G_DebugString(vTmp1 + Vector(0, 0, (fZ + 1) * 32), sin(level.time) + 3, 0, 0, 1, "STOP_SKIDDING");
			}
			Vector vMaxs = Vector(sin(level.time), sin(level.time), sin(level.time)) * 16;
			Vector vMins = vMaxs * -1;
			vMaxs = Vector(sin(level.time), sin(level.time), sin(level.time)) * 16;
			G_DebugBBox(vTmp1, vMins, vMaxs, 0, 0, 1, 1);


			vTmp = m_pCurPath->GetByNode(i + 1, NULL);
			vTmp2 = vTmp + 1;

			G_DebugLine(vTmp1, vTmp2, 0, 1, 0, 1);
		}
	}
	// FIXME: stub
}

/*
====================
Vehicle::GetSoundSet
====================
*/
str Vehicle::GetSoundSet
	(
	void
	)

{
	return m_sSoundSet;
}

/*
====================
Vehicle::QueryFreePassengerSlot
====================
*/
int Vehicle::QueryFreePassengerSlot
	(
	void
	)

{
	for( int i = 0; i < numPassengers; i++ )
	{
		if( Passengers[ i ].flags & SLOT_FREE )
		{
			return i;
		}
	}

	return -1;
}

/*
====================
Vehicle::QueryFreeDriverSlot
====================
*/
int Vehicle::QueryFreeDriverSlot
	(
	void
	)

{
	return ( driver.flags & SLOT_FREE ) ? 0 : -1;
}

/*
====================
Vehicle::QueryFreeTurretSlot
====================
*/
int Vehicle::QueryFreeTurretSlot
	(
	void
	)

{
	for( int i = 0; i < numTurrets; i++ )
	{
		if( Turrets[ i ].flags & SLOT_FREE )
		{
			return i;
		}
	}

	return -1;
}

/*
====================
Vehicle::QueryPassengerSlotPosition
====================
*/
void Vehicle::QueryPassengerSlotPosition
	(
	int slot,
	float *pos
	)

{
	orientation_t orient;

	if( Passengers[ slot ].enter_boneindex >= 0 )
	{
		GetTagPositionAndOrientation( Passengers[ slot ].enter_boneindex, &orient );
		VectorCopy( orient.origin, pos );
	}
	else
	{
		VectorCopy( origin, pos );
	}
}

/*
====================
Vehicle::QueryDriverSlotPosition
====================
*/
void Vehicle::QueryDriverSlotPosition
	(
	int slot,
	float *pos
	)

{
	orientation_t orient;

	if( driver.enter_boneindex >= 0 )
	{
		GetTagPositionAndOrientation( driver.enter_boneindex, &orient );
		VectorCopy( orient.origin, pos );
	}
	else
	{
		VectorCopy( origin, pos );
	}
}

/*
====================
Vehicle::QueryTurretSlotPosition
====================
*/
void Vehicle::QueryTurretSlotPosition
	(
	int slot,
	float *pos
	)

{
	orientation_t orient;

	if( Turrets[ slot ].enter_boneindex >= 0 )
	{
		GetTagPositionAndOrientation( Turrets[ slot ].enter_boneindex, &orient );
		VectorCopy( orient.origin, pos );
	}
	else
	{
		VectorCopy( origin, pos );
	}
}

/*
====================
Vehicle::QueryPassengerSlotAngles
====================
*/
void Vehicle::QueryPassengerSlotAngles
	(
	int slot,
	float *ang
	)

{
	orientation_t orient;

	GetTagPositionAndOrientation( Passengers[ slot ].enter_boneindex, &orient );
	MatrixToEulerAngles( orient.axis, ang );
}

/*
====================
Vehicle::QueryDriverSlotAngles
====================
*/
void Vehicle::QueryDriverSlotAngles
	(
	int slot,
	float *ang
	)

{
	orientation_t orient;

	GetTagPositionAndOrientation( driver.enter_boneindex, &orient );
	MatrixToEulerAngles( orient.axis, ang );
}

/*
====================
Vehicle::QueryTurretSlotAngles
====================
*/
void Vehicle::QueryTurretSlotAngles
	(
	int slot,
	float *ang
	)

{
	orientation_t orient;

	GetTagPositionAndOrientation( Turrets[ slot ].enter_boneindex, &orient );
	MatrixToEulerAngles( orient.axis, ang );
}

/*
====================
Vehicle::QueryPassengerSlotStatus
====================
*/
int Vehicle::QueryPassengerSlotStatus
	(
	int slot
	)

{
	return Passengers[ slot ].flags;
}

/*
====================
Vehicle::QueryDriverSlotStatus
====================
*/
int Vehicle::QueryDriverSlotStatus
	(
	int slot
	)

{
	return driver.flags;
}

/*
====================
Vehicle::QueryTurretSlotStatus
====================
*/
int Vehicle::QueryTurretSlotStatus
	(
	int slot
	)

{
	return Turrets[ slot ].flags;
}

/*
====================
Vehicle::QueryPassengerSlotEntity
====================
*/
Entity *Vehicle::QueryPassengerSlotEntity
	(
	int slot
	)

{
	return Passengers[ slot ].ent;
}

/*
====================
Vehicle::QueryDriverSlotEntity
====================
*/
Entity *Vehicle::QueryDriverSlotEntity
	(
	int slot
	)

{
	return driver.ent;
}

/*
====================
Vehicle::QueryTurretSlotEntity
====================
*/
Entity *Vehicle::QueryTurretSlotEntity
	(
	int slot
	)

{
	return Turrets[ slot ].ent;
}

/*
====================
Vehicle::AttachPassengerSlot
====================
*/
void Vehicle::AttachPassengerSlot
	(
	int slot,
	Entity *ent,
	Vector vExitPosition
	)

{
	Entity *passenger;
	str sName;

	if( !ent )
	{
		return;
	}

	passenger = Passengers[ slot ].ent;
	if( !passenger )
	{
		Passengers[ slot ].ent = ent;
		Passengers[ slot ].flags = SLOT_BUSY;

		sName = m_sSoundSet + "snd_doorclose";
		Sound( sName );

		Event *event = new Event( EV_Vehicle_Enter );
		event->AddEntity( this );
		driver.ent->ProcessEvent( event );

		offset = ent->origin - origin;

		flags |= FL_POSTTHINK;
		SetDriverAngles( seatangles + angles );
	}
	else if( !isLocked() && ent == passenger )
	{
		DetachPassengerSlot( slot, vec_zero, NULL );
	}
}

/*
====================
Vehicle::AttachDriverSlot
====================
*/
void Vehicle::AttachDriverSlot
	(
	int slot,
	Entity *ent,
	Vector vExitPosition
	)

{
	Entity *d;
	str sName;

	if( !ent || !ent->IsSubclassOfSentient() )
	{
		return;
	}

	d = Driver();
	if( !d )
	{
		driver.ent = ent;
		driver.flags = SLOT_BUSY;
		lastdriver.ent = driver.ent;

		sName = m_sSoundSet + "snd_doorclose";
		Sound( sName );

		sName = m_sSoundSet + "snd_start";
		Sound( sName );

		Event *event = new Event( EV_Vehicle_Enter );
		event->AddEntity( this );
		driver.ent->ProcessEvent( event );

		offset = ent->origin - origin;

		flags |= FL_POSTTHINK;
		SetDriverAngles( seatangles + angles );
	}
	else if( !isLocked() && ent == d )
	{
		DetachDriverSlot( slot, vec_zero, NULL );
	}
}

/*
====================
Vehicle::AttachTurretSlot

Attach a turret or a sentient that will use the turret to the vehicle.
====================
*/
void Vehicle::AttachTurretSlot
	(
	int slot,
	Entity *ent,
	Vector vExitPosition,
	Vector *vExitAngles
	)

{
	TurretGun *pTurret;
	VehicleTurretGun *pVehicleTurret;
	str sName;

	if( !ent )
	{
		return;
	}

	pTurret = ( TurretGun * )Turrets[ slot ].ent.Pointer();
	if( pTurret && ent->IsSubclassOfWeapon() )
	{
		if( !isLocked() )
			DetachTurretSlot( slot, vec_zero, NULL );
	}
	else
	{
		if( ent->IsSubclassOfWeapon() )
		{
			Turrets[ slot ].ent = ent;
			Turrets[ slot ].flags = SLOT_BUSY;

			pTurret = ( TurretGun * )ent;

			ent->takedamage = DAMAGE_NO;
			ent->setSolidType( SOLID_NOT );

			Event *event = new Event( EV_Vehicle_Enter );
			event->AddEntity( this );
			Turrets[ slot ].ent->ProcessEvent( event );

			offset = ent->origin - origin;

			flags |= FL_POSTTHINK;
			Turrets[ slot ].ent->setAngles( angles );

			if( pTurret->IsSubclassOfTurretGun() )
			{
				pTurret->m_bUsable = false;
				pTurret->m_bRestable = false;
			}
		}
		else if( pTurret )
		{
			Entity *pTurretOwner = NULL;
			Entity *pRemoteTurretOwner = NULL;

			if( pTurret->IsSubclassOfTurretGun() )
			{
				pTurretOwner = pTurret->GetOwner();
			}

			if( pTurret->IsSubclassOfVehicleTurretGun() )
			{
				pVehicleTurret = ( VehicleTurretGun * )pTurret;
				pRemoteTurretOwner = pVehicleTurret->GetRemoteOwner();
			}

			if( pTurret->IsSubclassOfTurretGun() )
			{
				if( pTurret->IsSubclassOfVehicleTurretGun() && pVehicleTurret->isLocked() )
				{
					ScriptError( "Turret is locked, cannot attach to turret slot." );
				}

				pTurret->m_bUsable = true;
			}

			Event *event = new Event( EV_Use );
			event->AddEntity( ent );
			pTurret->ProcessEvent( event );

			if( ent->IsSubclassOfPlayer() )
			{
				Player *pPlayer = ( Player * )ent;
				pPlayer->m_pVehicle = this;
			}

			if( pTurret->IsSubclassOfTurretGun() )
			{
				pTurret->m_bUsable = false;
			}

			if( pTurretOwner == ent || pRemoteTurretOwner == ent )
			{
				if( pRemoteTurretOwner )
				{
					pVehicleTurret->SetRemoteOwner( NULL );
				}

				if( vExitPosition != vec_zero )
				{
					Vector pos;
					trace_t trace;

					pos = vExitPosition;

					trace = G_Trace(
						pos,
						ent->mins,
						ent->maxs,
						pos,
						NULL,
						edict->clipmask,
						false,
						"Vehicle::AttachTurretSlot" );

					if( !trace.allsolid && !trace.startsolid )
					{
						trace = G_Trace(
							pos,
							ent->mins,
							ent->maxs,
							pos - Vector( 0, 0, 128 ),
							NULL,
							edict->clipmask,
							false,
							"Vehicle::AttachTurretSlot" );

						if( trace.fraction < 1.0f )
						{
							if( vExitAngles )
							{
								ent->setAngles( *vExitAngles );
							}

							ent->setOrigin( trace.endpos );
							velocity = vec_zero;

							Event *event = new Event( EV_Vehicle_Exit );
							event->AddEntity( this );
							ent->ProcessEvent( event );
						}
					}
				}
				else
				{
					int height;
					int ang;
					Vector angles;
					Vector forward;
					Vector pos;
					float ofs;
					trace_t trace;

					if( locked )
						return;

					//
					// place the turret on the ground
					//
					ofs = size.length() * 0.5f;
					for( height = 0; height < 100; height += 16 )
					{
						for( ang = 0; ang < 360; ang += 30 )
						{
							angles[ 1 ] = ent->angles[ 1 ] + ang + 90;
							angles.AngleVectors( &forward, NULL, NULL );
							pos = origin + ( forward * ofs );
							pos[ 2 ] += height;
							trace = G_Trace( pos, ent->mins, ent->maxs, pos, NULL, MASK_PLAYERSOLID, false, "Vehicle::AttachTurretSlot" );
							if( !trace.startsolid && !trace.allsolid )
							{
								Vector end;

								end = pos;
								end[ 2 ] -= 128;
								trace = G_Trace( pos, ent->mins, ent->maxs, end, NULL, MASK_PLAYERSOLID, false, "Vehicle::AttachTurretSlot" );
								if( trace.fraction < 1.0f )
								{
									ent->setOrigin( vExitPosition );
									ent->velocity = vec_zero;

									Event *ev = new Event( EV_Vehicle_Exit );
									ev->AddEntity( this );
									ent->ProcessEvent( ev );
								}
							}
						}
					}
				}
			}
		}
	}
}

/*
====================
Vehicle::DetachPassengerSlot
====================
*/
void Vehicle::DetachPassengerSlot
	(
	int slot,
	Vector vExitPosition,
	Vector *vExitAngles
	)

{
	Entity *passenger = Passengers[ slot ].ent;

	if( !passenger )
	{
		return;
	}

	if( vExitPosition == vec_zero )
	{
		int height;
		int ang;
		Vector angles;
		Vector forward;
		Vector pos;
		float ofs;
		trace_t trace;

		if( locked )
			return;

		//
		// place the passenger on the ground
		//
		ofs = size.length() * 0.5f;
		for( height = 0; height < 100; height += 16 )
		{
			for( ang = 0; ang < 360; ang += 30 )
			{
				angles[ 1 ] = passenger->angles[ 1 ] + ang + 90;
				angles.AngleVectors( &forward, NULL, NULL );
				pos = origin + ( forward * ofs );
				pos[ 2 ] += height;
				trace = G_Trace( pos, passenger->mins, passenger->maxs, pos, NULL, MASK_PLAYERSOLID, false, "Vehicle::DetachPassengerSlot" );
				if( !trace.startsolid && !trace.allsolid )
				{
					Vector end;

					end = pos;
					end[ 2 ] -= 128;
					trace = G_Trace( pos, passenger->mins, passenger->maxs, end, NULL, MASK_PLAYERSOLID, false, "Vehicle::DetachPassengerSlot" );
					if( trace.fraction < 1.0f )
					{
						passenger->setOrigin( pos );
						passenger->velocity = vec_zero;

						Event *ev = new Event( EV_Vehicle_Exit );
						ev->AddEntity( this );
						passenger->ProcessEvent( ev );

						Sound( m_sSoundSet + "snd_dooropen" );
					}
				}
			}
		}
	}
	else
	{
		if( vExitAngles )
		{
			passenger->setAngles( *vExitAngles );
		}

		passenger->setOrigin( vExitPosition );
		passenger->velocity = vec_zero;

		Event *ev = new Event( EV_Vehicle_Exit );
		ev->AddEntity( this );
		passenger->ProcessEvent( ev );
	}

	Passengers[ slot ].ent = NULL;
	Passengers[ slot ].flags = SLOT_FREE;
}

/*
====================
Vehicle::DetachDriverSlot
====================
*/
void Vehicle::DetachDriverSlot
	(
	int slot,
	Vector vExitPosition,
	Vector *vExitAngles
	)

{
	Entity *other = driver.ent;

	if( !other )
	{
		return;
	}

	if( vExitPosition == vec_zero )
	{
		int height;
		int ang;
		Vector angles;
		Vector forward;
		Vector pos;
		float ofs;
		trace_t trace;

		if( other != driver.ent )
		{
			return;
		}

		if( locked )
			return;

		//
		// place the driver.ent on the ground
		//
		ofs = size.length() * 0.5f;
		for( height = 0; height < 100; height += 16 )
		{
			for( ang = 0; ang < 360; ang += 30 )
			{
				angles[ 1 ] = driver.ent->angles[ 1 ] + ang + 90;
				angles.AngleVectors( &forward, NULL, NULL );
				pos = origin + ( forward * ofs );
				pos[ 2 ] += height;
				trace = G_Trace( pos, driver.ent->mins, driver.ent->maxs, pos, NULL, MASK_PLAYERSOLID, false, "Vehicle::DetachDriverSlot" );
				if( !trace.startsolid && !trace.allsolid )
				{
					Vector end;

					end = pos;
					end[ 2 ] -= 128;
					trace = G_Trace( pos, driver.ent->mins, driver.ent->maxs, end, NULL, MASK_PLAYERSOLID, false, "Vehicle::DetachDriverSlot" );
					if( trace.fraction < 1.0f )
					{
						driver.ent->setOrigin( pos );

						turnimpulse = 0;
						moveimpulse = 0;
						jumpimpulse = 0;

						Event *event = new Event( EV_Vehicle_Exit );
						event->AddEntity( this );
						driver.ent->ProcessEvent( event );
						if( hasweapon )
						{
							Player *player = ( Player * )driver.ent.Pointer();
							player->takeItem( weaponName.c_str() );
						}
						if( drivable )
						{
							StopLoopSound();
							Sound( "snd_dooropen", CHAN_BODY );
							Sound( "snd_stop", CHAN_VOICE );
							driver.ent->setSolidType( SOLID_BBOX );
						}
					}
				}
			}
		}
	}
	else
	{
		if( vExitAngles )
		{
			other->setAngles( *vExitAngles );
		}

		other->setOrigin( vExitPosition );
		other->velocity = vec_zero;

		Event *ev = new Event( EV_Vehicle_Exit );
		ev->AddEntity( this );
		other->ProcessEvent( ev );
	}

	driver.ent = NULL;
	driver.flags = SLOT_FREE;
}

/*
====================
Vehicle::DetachTurretSlot

Detach a turret or a sentient.
====================
*/
void Vehicle::DetachTurretSlot
	(
	int slot,
	Vector vExitPosition,
	Vector *vExitAngles
	)

{
	Entity *passenger = Turrets[ slot ].ent;

	if( !passenger )
	{
		return;
	}

	if( vExitPosition == vec_zero )
	{
		int height;
		int ang;
		Vector angles;
		Vector forward;
		Vector pos;
		float ofs;
		trace_t trace;

		if( locked )
			return;

		//
		// place the turret on the ground
		//
		ofs = size.length() * 0.5f;
		for( height = 0; height < 100; height += 16 )
		{
			for( ang = 0; ang < 360; ang += 30 )
			{
				angles[ 1 ] = passenger->angles[ 1 ] + ang + 90;
				angles.AngleVectors( &forward, NULL, NULL );
				pos = origin + ( forward * ofs );
				pos[ 2 ] += height;
				trace = G_Trace( pos, passenger->mins, passenger->maxs, pos, NULL, MASK_PLAYERSOLID, false, "Vehicle::DetachTurretSlot" );
				if( !trace.startsolid && !trace.allsolid )
				{
					Vector end;

					end = pos;
					end[ 2 ] -= 128;
					trace = G_Trace( pos, passenger->mins, passenger->maxs, end, NULL, MASK_PLAYERSOLID, false, "Vehicle::DetachTurretSlot" );
					if( trace.fraction < 1.0f )
					{
						passenger->setOrigin( pos );
						passenger->velocity = vec_zero;

						turnimpulse = 0;
						moveimpulse = 0;
						jumpimpulse = 0;

						Event *ev = new Event( EV_Vehicle_Exit );
						ev->AddEntity( this );
						passenger->ProcessEvent( ev );

						Sound( m_sSoundSet + "snd_dooropen" );

						TurretGun *pTurret = ( TurretGun * )passenger;
						if( pTurret->IsSubclassOfTurretGun() )
						{
							pTurret->m_bUsable = true;
							pTurret->m_bRestable = true;
						}
					}
				}
			}
		}
	}
	else
	{
		if( vExitAngles )
		{
			passenger->setAngles( *vExitAngles );
		}

		passenger->setOrigin( vExitPosition );
		passenger->velocity = vec_zero;

		Event *ev = new Event( EV_Vehicle_Exit );
		ev->AddEntity( this );
		passenger->ProcessEvent( ev );
	}

	Turrets[ slot ].ent = NULL;
	Turrets[ slot ].flags = SLOT_FREE;
}

/*
====================
Vehicle::FindPassengerSlotByEntity
====================
*/
int Vehicle::FindPassengerSlotByEntity
	(
	Entity *ent
	)

{
	for( int i = 0; i < MAX_PASSENGERS; i++ )
	{
		if( Passengers[ i ].ent == ent )
		{
			return i;
		}
	}

	return -1;
}

/*
====================
Vehicle::FindDriverSlotByEntity
====================
*/
int Vehicle::FindDriverSlotByEntity
	(
	Entity *ent
	)

{
	return driver.ent == ent ? 0 : -1;
}

/*
====================
Vehicle::FindTurretSlotByEntity
====================
*/
int Vehicle::FindTurretSlotByEntity
	(
	Entity *ent
	)

{
	for( int i = 0; i < MAX_TURRETS; i++ )
	{
		if( Turrets[ i ].ent == ent )
		{
			return i;
		}
	}

	return -1;
}

/*
====================
Vehicle::SetSlotsNonSolid
====================
*/
void Vehicle::SetSlotsNonSolid
	(
	void
	)

{
	for( int i = 0; i < MAX_PASSENGERS; i++ )
	{
		Passengers[ i ].NotSolid();
	}

	for( int i = 0; i < MAX_TURRETS; i++ )
	{
		Turrets[ i ].NotSolid();
	}

	driver.NotSolid();

	if( m_pCollisionEntity )
	{
		m_pCollisionEntity->NotSolid();
	}
}

/*
====================
Vehicle::SetSlotsSolid
====================
*/
void Vehicle::SetSlotsSolid
	(
	void
	)

{
	for( int i = 0; i < MAX_PASSENGERS; i++ )
	{
		Passengers[ i ].Solid();
	}

	for( int i = 0; i < MAX_TURRETS; i++ )
	{
		Turrets[ i ].Solid();
	}

	driver.Solid();

	if( m_pCollisionEntity )
	{
		m_pCollisionEntity->Solid();
	}
}

/*
====================
Vehicle::KickSuspension
====================
*/
void Vehicle::KickSuspension
	(
	Vector vDirection,
	float fForce
	)

{
	VectorNormalizeFast( vDirection );

	m_fForwardForce += DotProduct( vDirection, orientation[ 1 ] ) * fForce;
	m_fLeftForce += DotProduct( vDirection, orientation[ 0 ] ) * fForce;
}

/*
====================
Vehicle::isLocked
====================
*/
qboolean Vehicle::isLocked
	(
	void
	)

{
	return locked;
}

/*
====================
Vehicle::Lock
====================
*/
void Vehicle::Lock
	(
	void
	)

{
	locked = true;
}

/*
====================
Vehicle::UnLock
====================
*/
void Vehicle::UnLock
	(
	void
	)

{
	locked = false;
}

/*
====================
Vehicle::GetCollisionEntity
====================
*/
VehicleCollisionEntity *Vehicle::GetCollisionEntity
	(
	void
	)

{
	return m_pCollisionEntity;
}

CLASS_DECLARATION( Vehicle, DrivableVehicle, "script_drivablevehicle" )
{
	{ &EV_Damage,					&Entity::DamageEvent },
	{ &EV_Killed,					&DrivableVehicle::Killed },
	{ NULL, NULL }
};

/*
====================
DrivableVehicle::DrivableVehicle
====================
*/
DrivableVehicle::DrivableVehicle()
{
	if( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}

	drivable = true;
	flags |= FL_POSTTHINK | FL_THINK;
	setMoveType( MOVETYPE_VEHICLE );
}

/*
====================
DrivableVehicle::Killed
====================
*/
void DrivableVehicle::Killed
	(
	Event *ev
	)

{
	Entity * ent;
	Entity * attacker;
	Vector dir;
	Event * event;
	const char * name;
	VehicleBase *last;

	takedamage = DAMAGE_NO;
	setSolidType( SOLID_NOT );
	hideModel();

	attacker = ev->GetEntity( 1 );

	//
	// kill the driver.ent
	//
	if( driver.ent )
	{
		Vector dir;
		SentientPtr sent;
		Event * event;

		velocity = vec_zero;
		sent = ( Sentient * )driver.ent.Pointer();
		event = new Event( EV_Use );
		event->AddEntity( sent );
		ProcessEvent( event );
		dir = sent->origin - origin;
		dir[ 2 ] += 64;
		dir.normalize();
		sent->Damage( this, this, sent->health * 2, origin, dir, vec_zero, 50, 0, MOD_VEHICLE );
	}

	if( flags & FL_DIE_EXPLODE )
	{
		CreateExplosion( origin, 150 * edict->s.scale, this, this, this );
	}

	if( flags & FL_DIE_GIBS )
	{
		setSolidType( SOLID_NOT );
		hideModel();

		CreateGibs( this, -150, edict->s.scale, 3 );
	}
	//
	// kill all my wheels
	//
	last = this;
	while( last->vlink )
	{
		last->vlink->PostEvent( EV_Remove, 0 );
		last = last->vlink;
	}


	//
	// kill the killtargets
	//
	name = KillTarget();
	if( name && strcmp( name, "" ) )
	{
		ent = NULL;
		do
		{
			ent = ( Entity * )G_FindTarget( ent, name );
			if( !ent )
			{
				break;
			}
			ent->PostEvent( EV_Remove, 0 );
		} while( 1 );
	}

	//
	// fire targets
	//
	name = Target();
	if( name && strcmp( name, "" ) )
	{
		ent = NULL;
		do
		{
			ent = ( Entity * )G_FindTarget( ent, name );
			if( !ent )
			{
				break;
			}

			event = new Event( EV_Activate );
			event->AddEntity( attacker );
			ent->ProcessEvent( event );
		} while( 1 );
	}

	PostEvent( EV_Remove, 0 );
}
