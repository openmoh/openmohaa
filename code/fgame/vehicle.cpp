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
#include "g_phys.h"
#include "scriptslave.h"
#include "vehicle.h"
#include "player.h"
#include "specialfx.h"
#include "explosion.h"
#include "earthquake.h"
#include "gibs.h"
#include "vehicleturret.h"
#include "scriptexception.h"
#include "debuglines.h"

Event EV_Vehicle_Start
(
    "start",
    EV_DEFAULT,
    NULL,
    NULL,
    "Initialize the vehicle.",
    EV_NORMAL
);
Event EV_Vehicle_Enter
(
    "enter",
    EV_DEFAULT,
    "eS",
    "vehicle driver_anim",
    "Called when someone gets into a vehicle.",
    EV_NORMAL
);
Event EV_Vehicle_Exit
(
    "exit",
    EV_DEFAULT,
    "e",
    "vehicle",
    "Called when driver gets out of the vehicle.",
    EV_NORMAL
);
Event EV_Vehicle_Drivable
(
    "drivable",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make the vehicle drivable",
    EV_NORMAL
);
Event EV_Vehicle_UnDrivable
(
    "undrivable",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make the vehicle undrivable",
    EV_NORMAL
);
Event EV_Vehicle_PathDrivable
(
    "pathDrivable",
    EV_DEFAULT,
    "b",
    "pathDrivable",
    "Make the vehicle drivable along a path",
    EV_NORMAL
);
Event EV_Vehicle_Jumpable
(
    "canjump",
    EV_DEFAULT,
    "b",
    "jumpable",
    "Sets whether or not the vehicle can jump",
    EV_NORMAL
);
Event EV_Vehicle_Lock
(
    "lock",
    EV_DEFAULT,
    NULL,
    NULL,
    "Sets the vehicle to be locked",
    EV_NORMAL
);
Event EV_Vehicle_UnLock
(
    "unlock",
    EV_DEFAULT,
    NULL,
    NULL,
    "Sets the vehicle to be unlocked",
    EV_NORMAL
);
Event EV_Vehicle_SeatAnglesOffset
(
    "seatanglesoffset",
    EV_DEFAULT,
    "v",
    "angles",
    "Set the angles offset of the seat",
    EV_NORMAL
);
Event EV_Vehicle_SeatOffset
(
    "seatoffset",
    EV_DEFAULT,
    "v",
    "offset",
    "Set the offset of the seat",
    EV_NORMAL
);
Event EV_Vehicle_SetWeapon
(
    "setweapon",
    EV_DEFAULT,
    "s",
    "weaponname",
    "Set the weapon for the vehicle",
    EV_NORMAL
);
Event EV_Vehicle_SetName
(
    "name",
    EV_DEFAULT,
    "s",
    "vehicleName",
    "Set the name for the vehicle",
    EV_NORMAL
);
Event EV_Vehicle_ShowWeapon
(
    "showweapon",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set the weapon to be show in the view",
    EV_NORMAL
);
Event EV_Vehicle_SetSpeed
(
    "vehiclespeed",
    EV_DEFAULT,
    "f",
    "speed",
    "Set the speed of the vehicle",
    EV_NORMAL
);
Event EV_Vehicle_SetTurnRate
(
    "turnrate",
    EV_DEFAULT,
    "f",
    "rate",
    "Set the turning rate of the vehicle",
    EV_NORMAL
);
Event EV_Vehicle_SteerInPlace
(
    "steerinplace",
    EV_DEFAULT,
    NULL,
    NULL,
    "Set the vehicle to turn in place",
    EV_NORMAL
);
Event EV_Vehicle_Destroyed
(
    "vehicledestroyed",
    EV_DEFAULT,
    NULL,
    NULL,
    "Driver is dead",
    EV_NORMAL
);
Event EV_Vehicle_Mass
(
    "vehiclemass",
    EV_DEFAULT,
    "f",
    "weight",
    "Sets the mass of the vehicle (backmass = frontmass = mass/2)",
    EV_NORMAL
);
Event EV_Vehicle_Front_Mass
(
    "front_mass",
    EV_DEFAULT,
    "f",
    "weight",
    "Sets the mass of the front of the vehicle",
    EV_NORMAL
);
Event EV_Vehicle_Back_Mass
(
    "back_mass",
    EV_DEFAULT,
    "f",
    "weight",
    "Sets the mass of the back of the vehicle",
    EV_NORMAL
);
Event EV_Vehicle_Tread
(
    "vehicletread",
    EV_DEFAULT,
    "f",
    "size",
    "Sets the size of the wheels",
    EV_NORMAL
);
Event EV_Vehicle_Radius
(
    "vehicleradius",
    EV_DEFAULT,
    "f",
    "size",
    "Sets the radius of the wheels",
    EV_NORMAL
);
Event EV_Vehicle_RollingResistance
(
    "vehiclerollingresistance",
    EV_DEFAULT,
    "f",
    "size",
    "Sets the radius of the wheels",
    EV_NORMAL
);
Event EV_Vehicle_Drag
(
    "vehicledrag",
    EV_DEFAULT,
    "f",
    "size",
    "Sets the Drag Factor",
    EV_NORMAL
);
Event EV_Vehicle_Drive
(
    "drive",
    EV_DEFAULT,
    "vffffV",
    "position speed acceleration reach_distance look_ahead alternate_position",
    "Makes the vehicle drive to position with speed and acceleration until reached_distance close to position",
    EV_NORMAL
);
Event EV_Vehicle_DriveNoWait
(
    "driveNoWait",
    EV_DEFAULT,
    "vfff",
    "position speed acceleration reach_distance",
    "Makes the vehicle drive to position with speed and acceleration until reached_distance close to position, thread "
    "doesn't wait",
    EV_NORMAL
);
Event EV_Vehicle_Stop
(
    "stop",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make the Vehicle Stop Moving... FULL BREAKS!",
    EV_NORMAL
);
Event EV_Vehicle_FullStop
(
    "fullstop",
    EV_DEFAULT,
    NULL,
    NULL,
    "Make the Vehicle Stop Moving... Completely!",
    EV_NORMAL
);
Event EV_Vehicle_Init
(
    "vehicleinit",
    EV_DEFAULT,
    NULL,
    NULL,
    "Initialized the Vehicle as the specified file",
    EV_NORMAL
);
Event EV_Vehicle_BouncyCoef
(
    "vehiclebouncy",
    EV_DEFAULT,
    "f",
    "bouncycoef",
    "Sets the Bouncy Coefficient for the shocks.",
    EV_NORMAL
);
Event EV_Vehicle_SpringyCoef
(
    "vehiclespringy",
    EV_DEFAULT,
    "f",
    "springycoef",
    "Sets the Springy Coefficient for the shocks.",
    EV_NORMAL
);
Event EV_Vehicle_Yaw
(
    "vehicleYaw",
    EV_DEFAULT,
    "fff",
    "min max coef",
    "Sets the Yaw min and max and the acceleration coefficient for the shocks.",
    EV_NORMAL
);
Event EV_Vehicle_Roll
(
    "vehicleRoll",
    EV_DEFAULT,
    "fff",
    "min max coef",
    "Sets the Roll min and max and the acceleration coefficient for the shocks.",
    EV_NORMAL
);
Event EV_Vehicle_Z
(
    "vehicleZ",
    EV_DEFAULT,
    "fff",
    "min max coef",
    "Sets the Z min and max and the acceleration coefficient for the shocks.",
    EV_NORMAL
);
Event EV_Vehicle_QuerySpeed
(
    "QuerySpeed",
    EV_DEFAULT,
    "f",
    "speed",
    "Returns the current speed of the vehicle.",
    EV_RETURN
);
Event EV_Vehicle_QueryFreePassengerSlot
(
    "QueryFreePassengerSlot",
    EV_DEFAULT,
    NULL,
    NULL,
    "Returns a number that represents the first free passenger slot on the vehicle.",
    EV_RETURN
);
Event EV_Vehicle_QueryPassengerSlotPosition
(
    "QueryPassengerSlotPosition",
    EV_DEFAULT,
    "i",
    "slot",
    "Returns the position of the specified slot on the vehicle.",
    EV_RETURN
);
Event EV_Vehicle_QueryPassengerSlotStatus
(
    "QueryPassengerSlotStatus",
    EV_DEFAULT,
    "i",
    "slot",
    "Returns the status of the specified slot on the vehicle.",
    EV_RETURN
);
Event EV_Vehicle_BounceForwardsVelocity
(
    "BounceForwardsVelocity",
    EV_DEFAULT,
    "NULL",
    "NULL",
    "For vehicles on rails that are moving forwards, reverse our velocity.",
    EV_NORMAL
);
Event EV_Vehicle_BounceBackwardsVelocity
(
    "BounceBackwardsVelocity",
    EV_DEFAULT,
    "i",
    "bStayFullSpeed",
    "For vehicles on rails that are moving backwards, reverse our velocity. (Optionally pass bStayFullSpeed to keep "
    "vehic"
    "le at full speed after the bounce)",
    EV_NORMAL
);
Event EV_Vehicle_StopForwardsVelocity
(
    "StopForwardsVelocity",
    EV_DEFAULT,
    "NULL",
    "NULL",
    "Stops any forward motion for vehicles on rails.",
    EV_NORMAL
);
Event EV_Vehicle_StopBackwardsVelocity
(
    "StopBackwardsVelocity",
    EV_DEFAULT,
    "NULL",
    "NULL",
    "Stops any backwards motion for vehicles on rails.",
    EV_NORMAL
);
Event EV_Vehicle_AttachPassengerSlot
(
    "AttachPassengerSlot",
    EV_DEFAULT,
    "ie",
    "slot entity",
    "Attaches an entity to the specified slot.",
    EV_NORMAL
);
Event EV_Vehicle_QueryPassengerSlotEntity
(
    "QueryPassengerSlotEntity",
    EV_DEFAULT,
    "i",
    "slot",
    "Returns an entity at the specified slot.",
    EV_RETURN
);
Event EV_Vehicle_DetachPassengerSlot
(
    "DetachPassengerSlot",
    EV_DEFAULT,
    "iV",
    "slot exit_position",
    "Detaches an entity to the specified slot.",
    EV_NORMAL
);
Event EV_Vehicle_QueryFreeDriverSlot
(
    "QueryFreeDriverSlot",
    EV_DEFAULT,
    NULL,
    NULL,
    "Returns a number that represents the first free driver slot on the vehicle.",
    EV_RETURN
);
Event EV_Vehicle_QueryDriverSlotPosition
(
    "QueryDriverSlotPosition",
    EV_DEFAULT,
    "i",
    "slot",
    "Returns the position of the specified slot on the vehicle.",
    EV_RETURN
);
Event EV_Vehicle_QueryDriverSlotStatus
(
    "QueryDriverSlotStatus",
    EV_DEFAULT,
    "i",
    "slot",
    "Returns the status of the specified slot on the vehicle.",
    EV_RETURN
);
Event EV_Vehicle_AttachDriverSlot
(
    "AttachDriverSlot",
    EV_DEFAULT,
    "ie",
    "slot entity",
    "Attaches an entity to the specified slot.",
    EV_NORMAL
);
Event EV_Vehicle_QueryDriverSlotEntity
(
    "QueryDriverSlotEntity",
    EV_DEFAULT,
    "i",
    "slot",
    "Returns an entity at the specified slot.",
    EV_RETURN
);
Event EV_Vehicle_DetachDriverSlot
(
    "DetachDriverSlot",
    EV_DEFAULT,
    "iV",
    "slot exit_position",
    "Detaches an entity to the specified slot.",
    EV_NORMAL
);
Event EV_Vehicle_QueryFreeTurretSlot
(
    "QueryFreeTurretSlot",
    EV_DEFAULT,
    NULL,
    NULL,
    "Returns a number that represents the first free turret slot on the vehicle.",
    EV_RETURN
);
Event EV_Vehicle_QueryTurretSlotPosition
(
    "QueryTurretSlotPosition",
    EV_DEFAULT,
    "i",
    "slot",
    "Returns the position of the specified slot on the vehicle.",
    EV_RETURN
);
Event EV_Vehicle_QueryTurretSlotStatus
(
    "QueryTurretSlotStatus",
    EV_DEFAULT,
    "i",
    "slot",
    "Returns the status of the specified slot on the vehicle.",
    EV_RETURN
);
Event EV_Vehicle_AttachTurretSlot
(
    "AttachTurretSlot",
    EV_DEFAULT,
    "ie",
    "slot entity",
    "Attaches an entity to the specified slot.",
    EV_NORMAL
);
Event EV_Vehicle_QueryTurretSlotEntity
(
    "QueryTurretSlotEntity",
    EV_DEFAULT,
    "i",
    "slot",
    "Returns an entity at the specified slot.",
    EV_RETURN
);
Event EV_Vehicle_DetachTurretSlot
(
    "DetachTurretSlot",
    EV_DEFAULT,
    "iV",
    "slot exit_position",
    "Detaches an entity to the specified slot.",
    EV_NORMAL
);
Event EV_Vehicle_WheelCorners
(
    "VehicleWheelCorners",
    EV_DEFAULT,
    "vv",
    "size offset",
    "Sets the wheel trace corners.",
    EV_NORMAL
);
Event EV_Vehicle_QueryDriverSlotAngles
(
    "QueryDriverSlotAngles",
    EV_DEFAULT,
    "i",
    "slot",
    "Returns the angles of the specified slot on the vehicle.",
    EV_RETURN
);
Event EV_Vehicle_QueryPassengerSlotAngles
(
    "QueryDriverSlotAngles",
    EV_DEFAULT,
    "i",
    "slot",
    "Returns the angles of the specified slot on the vehicle.",
    EV_RETURN
);
Event EV_Vehicle_QueryTurretSlotAngles
(
    "QueryDriverSlotAngles",
    EV_DEFAULT,
    "i",
    "slot",
    "Returns the angles of the specified slot on the vehicle.",
    EV_RETURN
);
Event EV_Vehicle_AnimationSet
(
    "AnimationSet",
    EV_DEFAULT,
    "s",
    "animset",
    "Sets the Animation Set to use.",
    EV_NORMAL
);
Event EV_Vehicle_SoundSet
(
    "SoundSet",
    EV_DEFAULT,
    "s",
    "soundset",
    "Sets the Sound Set to use.",
    EV_NORMAL
);
Event EV_Vehicle_SpawnTurret
(
    "spawnturret",
    EV_DEFAULT,
    "is",
    "slot tikifile",
    "Spawns a turret with the specified model and connects it to the specified slot",
    EV_NORMAL
);
Event EV_Vehicle_ModifyDrive
(
    "modifydrive",
    EV_DEFAULT,
    "fff",
    "desired_speed acceleration look_ahead",
    "Modifys the parameters of the current drive.",
    EV_NORMAL
);
Event EV_Vehicle_NextDrive
(
    "nextdrive",
    EV_DEFAULT,
    "e",
    "next_path",
    "appends the specified path to the current path",
    EV_NORMAL
);
Event EV_Vehicle_StopAtEnd
(
    "stopatend",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes the vehicle slow down to a complete stop at the end of the path.",
    EV_NORMAL
);
Event EV_Vehicle_LockMovement
(
    "lockmovement",
    EV_DEFAULT,
    NULL,
    NULL,
    "The Vehicle cannot move.",
    EV_NORMAL
);
Event EV_Vehicle_UnlockMovement
(
    "unlockmovement",
    EV_DEFAULT,
    NULL,
    NULL,
    "The Vehicle can move again.",
    EV_NORMAL
);
Event EV_Vehicle_RemoveOnDeath
(
    "removeondeath",
    EV_DEFAULT,
    "i",
    "removeondeath",
    "If set to a non-zero value, vehicles will not be removed when they die",
    EV_NORMAL
);
Event EV_Vehicle_SetExplosionModel
(
    "explosionmodel",
    EV_DEFAULT,
    "s",
    "model",
    "Sets the TIKI to call when the vehicle dies.",
    EV_NORMAL
);
Event EV_Vehicle_SetCollisionEntity
(
    "setcollisionentity",
    EV_DEFAULT,
    "e",
    "entity",
    "Sets the Collision Entity.",
    EV_NORMAL
);
Event EV_Vehicle_CollisionEntitySetter
(
    "collisionent",
    EV_DEFAULT,
    "e",
    "entity",
    "Gets the Collision Entity",
    EV_SETTER
);
Event EV_Vehicle_CollisionEntityGetter
(
    "collisionent",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the Collision Entity",
    EV_GETTER
);
Event EV_Vehicle_SetSoundParameters
(
    "setsoundparameters",
    EV_DEFAULT,
    "ffff",
    "min_speed min_pitch max_speed max_pitch",
    "Sets the Sound parameters for this vehicle",
    EV_NORMAL
);
Event EV_Vehicle_SetVolumeParameters
(
    "setvolumeparameters",
    EV_DEFAULT,
    "ffff",
    "min_speed min_volume max_speed max_volume",
    "Sets the Volume parameters for this vehicle",
    EV_NORMAL
);
Event EV_Vehicle_Skidding
(
    "skidding",
    EV_DEFAULT,
    "i",
    "on_off",
    "Makes the vehicle skid around corners.",
    EV_NORMAL
);
Event EV_Vehicle_ContinueSkidding
(
    "_continueskidding",
    EV_DEFAULT,
    NULL,
    NULL,
    "Continues the skidding animation of a vehicle.",
    EV_NORMAL
);
Event EV_Vehicle_VehicleAnim
(
    "vehicleanim",
    EV_DEFAULT,
    "sFI",
    "anim_name weight",
    "Sets an animation to use in the LD Animation slot. \nWeight defaults to 1.0",
    EV_NORMAL
);
Event EV_Vehicle_VehicleAnimDone
(
    "_vehicleanimdone",
    EV_DEFAULT,
    NULL,
    NULL,
    "For Internal Use Only",
    EV_NORMAL
);
Event EV_Vehicle_VehicleMoveAnim
(
    "moveanim",
    EV_DEFAULT,
    "s",
    "anim_name",
    "move the vehicle with an animation",
    EV_NORMAL
);
Event EV_Vehicle_VehicleMoveAnimDone
(
    "_moveanimdone",
    EV_DEFAULT,
    NULL,
    NULL,
    "For Internal Use Only",
    EV_NORMAL
);
Event EV_Vehicle_DamageSounds
(
    "damagesounds",
    EV_DEFAULT,
    "i",
    "on_off",
    "Makes the vehicle play damaged sounds.",
    EV_NORMAL
);
Event EV_Vehicle_RunSounds
(
    "runsounds",
    EV_DEFAULT,
    "i",
    "on_off",
    "Makes the vehicle play running sounds.",
    EV_NORMAL
);
Event EV_Vehicle_ProjectileVulnerable
(
    "projectilevulnerable",
    EV_DEFAULT,
    "I",
    "number_of_hits",
    "Make vehicle vulnerable to being one-shot by projectiles. If number_of_hits is given, it will take this many "
    "shots.",
    EV_NORMAL
);
Event EV_Vehicle_CanUse
(
    "canuse",
    EV_DEFAULT,
    "e",
    "entity",
    "Returns 1 if passed entity can 'use' this vehicle.",
    EV_RETURN
);

cvar_t *g_showvehiclemovedebug;
cvar_t *g_showvehicleentrypoints;
cvar_t *g_showvehicleslotpoints;
cvar_t *g_showvehicletags;
cvar_t *g_showvehiclepath;

CLASS_DECLARATION(Animate, VehicleBase, NULL) {
    {NULL, NULL}
};

VehicleBase::VehicleBase()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }

    takedamage = DAMAGE_NO;
    showModel();

    //
    // rotate the mins and maxs for the model
    //
    setSize(mins, maxs);

    vlink  = NULL;
    offset = vec_zero;

    PostEvent(EV_BecomeNonSolid, EV_POSTSPAWN);
}

CLASS_DECLARATION(VehicleBase, Vehicle, "script_vehicle") {
    {&EV_Blocked,                            &Vehicle::VehicleBlocked            },
    {&EV_Touch,                              &Vehicle::VehicleTouched            },
    {&EV_Use,                                &Vehicle::DriverUse                 },
    {&EV_Vehicle_Start,                      &Vehicle::VehicleStart              },
    {&EV_Vehicle_Drivable,                   &Vehicle::Drivable                  },
    {&EV_Vehicle_PathDrivable,               &Vehicle::PathDrivable              },
    {&EV_Vehicle_ProjectileVulnerable,       &Vehicle::SetProjectileVulnerable   },
    {&EV_Vehicle_UnDrivable,                 &Vehicle::UnDrivable                },
    {&EV_Vehicle_Jumpable,                   &Vehicle::Jumpable                  },
    {&EV_Vehicle_SeatAnglesOffset,           &Vehicle::SeatAnglesOffset          },
    {&EV_Vehicle_SeatOffset,                 &Vehicle::SeatOffset                },
    {&EV_Vehicle_Lock,                       &Vehicle::Lock                      },
    {&EV_Vehicle_UnLock,                     &Vehicle::UnLock                    },
    {&EV_Vehicle_SetWeapon,                  &Vehicle::SetWeapon                 },
    {&EV_Vehicle_SetName,                    &Vehicle::SetName                   },
    {&EV_Vehicle_SetSpeed,                   &Vehicle::SetSpeed                  },
    {&EV_Vehicle_SetTurnRate,                &Vehicle::SetTurnRate               },
    {&EV_Vehicle_SteerInPlace,               &Vehicle::SteerInPlace              },
    {&EV_Vehicle_ShowWeapon,                 &Vehicle::ShowWeaponEvent           },
    {&EV_Damage,                             &Vehicle::EventDamage               },
    {&EV_Vehicle_Destroyed,                  &Vehicle::VehicleDestroyed          },
    {&EV_Vehicle_Mass,                       &Vehicle::SetMass                   },
    {&EV_Vehicle_Front_Mass,                 &Vehicle::SetFrontMass              },
    {&EV_Vehicle_Back_Mass,                  &Vehicle::SetBackMass               },
    {&EV_Vehicle_Tread,                      &Vehicle::SetTread                  },
    {&EV_Vehicle_Radius,                     &Vehicle::SetTireRadius             },
    {&EV_Vehicle_RollingResistance,          &Vehicle::SetRollingResistance      },
    {&EV_Vehicle_Drag,                       &Vehicle::SetDrag                   },
    {&EV_Vehicle_Drive,                      &Vehicle::EventDrive                },
    {&EV_Vehicle_DriveNoWait,                &Vehicle::EventDriveNoWait          },
    {&EV_Vehicle_Stop,                       &Vehicle::EventStop                 },
    {&EV_Vehicle_FullStop,                   &Vehicle::EventFullStop             },
    {&EV_Vehicle_Init,                       &Vehicle::ModelInit                 },
    {&EV_Vehicle_BouncyCoef,                 &Vehicle::BouncyCoef                },
    {&EV_Vehicle_SpringyCoef,                &Vehicle::SpringyCoef               },
    {&EV_Vehicle_Yaw,                        &Vehicle::GetYaw                    },
    {&EV_Vehicle_Roll,                       &Vehicle::RollMinMax                },
    {&EV_Vehicle_Z,                          &Vehicle::ZMinMax                   },
    {&EV_Vehicle_QuerySpeed,                 &Vehicle::QuerySpeed                },
    {&EV_Vehicle_QueryFreePassengerSlot,     &Vehicle::QueryFreePassengerSlot    },
    {&EV_Vehicle_QueryFreeDriverSlot,        &Vehicle::QueryFreeDriverSlot       },
    {&EV_Vehicle_QueryFreeTurretSlot,        &Vehicle::QueryFreeTurretSlot       },
    {&EV_Vehicle_QueryPassengerSlotPosition, &Vehicle::QueryPassengerSlotPosition},
    {&EV_Vehicle_QueryDriverSlotPosition,    &Vehicle::QueryDriverSlotPosition   },
    {&EV_Vehicle_QueryTurretSlotPosition,    &Vehicle::QueryTurretSlotPosition   },
    {&EV_Vehicle_QueryPassengerSlotStatus,   &Vehicle::QueryPassengerSlotStatus  },
    {&EV_Vehicle_QueryDriverSlotStatus,      &Vehicle::QueryDriverSlotStatus     },
    {&EV_Vehicle_QueryTurretSlotStatus,      &Vehicle::QueryTurretSlotStatus     },
    {&EV_Vehicle_QueryPassengerSlotEntity,   &Vehicle::QueryPassengerSlotEntity  },
    {&EV_Vehicle_QueryDriverSlotEntity,      &Vehicle::QueryDriverSlotEntity     },
    {&EV_Vehicle_QueryTurretSlotEntity,      &Vehicle::QueryTurretSlotEntity     },
    {&EV_Vehicle_QueryDriverSlotAngles,      &Vehicle::QueryDriverSlotAngles     },
    {&EV_Vehicle_QueryPassengerSlotAngles,   &Vehicle::QueryPassengerSlotAngles  },
    {&EV_Vehicle_BounceForwardsVelocity,     &Vehicle::BounceForwardsVelocity    },
    {&EV_Vehicle_BounceBackwardsVelocity,    &Vehicle::BounceBackwardsVelocity   },
    {&EV_Vehicle_StopBackwardsVelocity,      &Vehicle::StopBackwardsVelocity     },
    {&EV_Vehicle_StopBackwardsVelocity,      &Vehicle::StopBackwardsVelocity     },
    {&EV_Vehicle_QueryTurretSlotAngles,      &Vehicle::QueryTurretSlotAngles     },
    {&EV_Vehicle_AttachPassengerSlot,        &Vehicle::AttachPassengerSlot       },
    {&EV_Vehicle_AttachDriverSlot,           &Vehicle::AttachDriverSlot          },
    {&EV_Vehicle_AttachTurretSlot,           &Vehicle::AttachTurretSlot          },
    {&EV_Vehicle_DetachPassengerSlot,        &Vehicle::DetachPassengerSlot       },
    {&EV_Vehicle_DetachDriverSlot,           &Vehicle::DetachDriverSlot          },
    {&EV_Vehicle_DetachTurretSlot,           &Vehicle::DetachTurretSlot          },
    {&EV_Vehicle_WheelCorners,               &Vehicle::SetWheelCorners           },
    {&EV_Vehicle_AnimationSet,               &Vehicle::SetAnimationSet           },
    {&EV_Vehicle_SoundSet,                   &Vehicle::SetSoundSet               },
    {&EV_Vehicle_SpawnTurret,                &Vehicle::SpawnTurret               },
    {&EV_Vehicle_ModifyDrive,                &Vehicle::EventModifyDrive          },
    {&EV_Model,                              &Vehicle::EventModel                },
    {&EV_Vehicle_NextDrive,                  &Vehicle::EventNextDrive            },
    {&EV_Vehicle_LockMovement,               &Vehicle::EventLockMovement         },
    {&EV_Vehicle_UnlockMovement,             &Vehicle::EventUnlockMovement       },
    {&EV_Vehicle_RemoveOnDeath,              &Vehicle::EventRemoveOnDeath        },
    {&EV_Vehicle_SetExplosionModel,          &Vehicle::EventSetExplosionModel    },
    {&EV_Vehicle_SetCollisionEntity,         &Vehicle::EventSetCollisionModel    },
    {&EV_Vehicle_SetSoundParameters,         &Vehicle::EventSetSoundParameters   },
    {&EV_Vehicle_SetVolumeParameters,        &Vehicle::EventSetVolumeParameters  },
    {&EV_Vehicle_StopAtEnd,                  &Vehicle::EventStopAtEnd            },
    {&EV_Vehicle_Skidding,                   &Vehicle::EventSkidding             },
    {&EV_Vehicle_ContinueSkidding,           &Vehicle::EventContinueSkidding     },
    {&EV_Vehicle_CollisionEntitySetter,      &Vehicle::EventSetCollisionModel    },
    {&EV_Vehicle_CollisionEntityGetter,      &Vehicle::EventGetCollisionModel    },
    {&EV_Vehicle_VehicleAnim,                &Vehicle::EventVehicleAnim          },
    {&EV_Vehicle_VehicleAnimDone,            &Vehicle::EventVehicleAnimDone      },
    {&EV_Vehicle_VehicleMoveAnim,            &Vehicle::EventVehicleMoveAnim      },
    {&EV_Vehicle_VehicleMoveAnimDone,        &Vehicle::EventVehicleMoveAnimDone  },
    {&EV_Vehicle_DamageSounds,               &Vehicle::EventDamageSounds         },
    {&EV_Vehicle_RunSounds,                  &Vehicle::EventRunSounds            },
    {&EV_Remove,                             &Vehicle::Remove                    },
    {&EV_Turret_SetMaxUseAngle,              &Vehicle::EventSetMaxUseAngle       },
    {&EV_Vehicle_CanUse,                     &Vehicle::EventCanUse               },
    {NULL,                                   NULL                                }
};

/*
====================
Vehicle::Vehicle
====================
*/
Vehicle::Vehicle()
{
    int i;

    entflags |= ECF_VEHICLE;

    AddWaitTill(STRING_DRIVE);
    AddWaitTill(STRING_VEHICLEANIMDONE);
    AddWaitTill(STRING_ANIMDONE);

    g_showvehiclemovedebug   = gi.Cvar_Get("g_showvehiclemovedebug", "0", 0);
    g_showvehicleentrypoints = gi.Cvar_Get("g_showvehicleentrypoints", "0", 0);
    g_showvehicleslotpoints  = gi.Cvar_Get("g_showvehicleslotpoints", "0", 0);
    g_showvehicletags        = gi.Cvar_Get("g_showvehicletags", "0", 0);
    g_showvehiclepath        = gi.Cvar_Get("g_showvehiclepath", "0", 0);

    for (i = 0; i < NUM_VEHICLE_TIRES; i++) {
        VectorClear(m_vTireEnd[i]);
    }
    m_bTireHit[0] = false;
    VectorClear(m_vNormalSum);
    m_iNumNormals = 0;

    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }

    edict->s.eType = ET_VEHICLE;

    moveresult   = 0;
    isBlocked    = false;
    m_iGear      = 1;
    m_iRPM       = 0;
    maxtracedist = 0;
    airspeed     = 0;
    for (i = 0; i < 6; i++) {
        m_fGearRatio[i] = 0;
    }

    m_fMass      = 0;
    m_fFrontMass = 0;
    m_fBackMass  = 0;

    m_fWheelBase            = 0.0;
    m_fWheelFrontLoad       = 0.0;
    m_fWheelFrontInnerLoad  = 0.0;
    m_fWheelFrontOutterLoad = 0.0;
    m_fWheelFrontDist       = 0.0;
    m_fWheelFrontSuspension = 0.0;
    m_fWheelBackLoad        = 0.0;
    m_fWheelBackInnerLoad   = 0.0;
    m_fWheelBackOutterLoad  = 0.0;
    m_fWheelBackDist        = 0.0;
    m_fWheelBackSuspension  = 0.0;

    m_fCGHeight     = 0.0;
    m_fBankAngle    = 0.0;
    m_fTread        = 0.0;
    m_fTrackWidth   = 0.0;
    m_fTireFriction = 0.0;
    m_fDrag         = 0.0;
    m_fTireRadius   = 0.0;
    m_fFrontBrakes  = 0.0;
    m_fBackBrakes   = 0.0;

    m_fRollingResistance   = 0.0;
    m_fTireRotationalSpeed = 0.0;
    m_fFrontBrakingForce   = 0.0;
    m_fBackBrakingForce    = 0.0;
    m_fBrakingPerformance  = 0.0;

    m_fLastTurn          = 0.0;
    m_fTangForce         = 0.0;
    m_fInertia           = 0.0;
    m_fDifferentialRatio = 0.0;
    m_fGearEfficiency    = 0.0;
    m_fMaxTraction       = 0.0;
    m_fTractionForce     = 0.0;
    m_fAccelerator       = 0.0;
    m_fTorque            = 0.0;
    m_fStopStartDistance = 0.0;
    m_fStopStartSpeed    = 0.0;
    m_fStopEndDistance   = 0.0;
    m_bWheelSpinning     = 0;
    m_bIsSkidding        = 0;
    m_bIsBraking         = 0;
    m_bAutomatic         = 0;
    m_iNumNormals        = 0;

    memset(&vs, 0, sizeof(vs));
    takedamage                 = DAMAGE_YES;
    seatangles                 = vec_zero;
    driveroffset               = Vector(0, 0, 50);
    seatoffset                 = vec_zero;
    currentspeed               = 0;
    turnangle                  = 0;
    turnimpulse                = 0;
    moveimpulse                = 0;
    jumpimpulse                = 0;
    conesize                   = 75;
    hasweapon                  = false;
    locked                     = true;
    steerinplace               = false;
    drivable                   = false;
    pathDrivable               = false;
    jumpable                   = false;
    showweapon                 = false;
    m_iProjectileHitsRemaining = 0;

    flags |= FL_TOUCH_TRIGGERS | FL_POSTTHINK | FL_THINK | FL_DIE_EXPLODE;
    // touch triggers by default
    gravity = 1;
    mass    = size.length() * 10;

    health      = 1000;
    speed       = 1200;
    maxturnrate = 60.0f;
    m_vAngles   = angles;

    prev_acceleration = vec_zero;
    real_acceleration = vec_zero;
    prev_origin       = origin;
    real_velocity     = vec_zero;
    prev_velocity     = velocity;

    m_vPrevNormal    = vec_zero;
    prev_moveimpulse = 0;

    m_iFrameCtr      = 0;
    m_bFrontSlipping = qfalse;
    m_bBackSlipping  = qfalse;

    m_vAngularVelocity     = vec_zero;
    m_vAngularAcceleration = vec_zero;

    m_sMoveGrid = new cMoveGrid(3, 3, 1);

    m_bAutoPilot       = qfalse;
    m_fIdealSpeed      = 0;
    m_fIdealAccel      = 0;
    m_fIdealDistance   = 100;
    m_bBounceBackwards = false;

    m_vOriginOffset  = vec_zero;
    m_vOriginOffset2 = vec_zero;
    m_vAnglesOffset  = vec_zero;

    m_fBouncyCoef  = 0.2f;
    m_fSpringyCoef = 0.8f;
    m_fYawMin      = -10.0f;
    m_fYawMax      = 10.0f;
    m_fYawCoef     = 0.1f;
    m_fRollMin     = -10.0f;
    m_fRollMax     = 10.0f;
    m_fRollCoef    = 0.1f;
    m_fZMin        = -10.0f;
    m_fZMax        = 10.0f;
    m_fZCoef       = 0.1f;

    m_vAngularVelocity     = vec_zero;
    m_vAngularAcceleration = vec_zero;
    v_angle                = vec_zero;
    m_bThinkCalled         = qfalse;
    m_pCurPath             = NULL;
    m_iCurNode             = 0;
    m_pAlternatePath       = NULL;
    m_iAlternateNode       = 0;
    m_pNextPath            = NULL;
    m_iNextPathStartNode   = -1;

    m_fLookAhead            = 0;
    m_fShaderOffset         = 0;
    vs.hit_obstacle         = false;
    vs.groundTrace.fraction = 1.0f;
    edict->clipmask         = MASK_VEHICLE;

    m_fLeftForce    = 0;
    m_fRightForce   = 0;
    m_fForwardForce = 0;
    m_fBackForce    = 0;
    m_fUpForce      = 0;
    m_fDownForce    = 0;

    edict->s.eFlags |= EF_LINKANGLES;

    m_bMovementLocked  = qfalse;
    m_bRemoveOnDeath   = qtrue;
    m_sExplosionModel  = "fx/fx_explosion.tik";
    m_pCollisionEntity = NULL;

    m_fSoundMinPitch  = 0.95f;
    m_fSoundMinSpeed  = 0;
    m_fSoundMaxPitch  = 1.0f;
    m_fSoundMaxSpeed  = 200.0f;
    m_fVolumeMinPitch = 1.0f;
    m_fVolumeMinSpeed = 0;
    m_fVolumeMaxPitch = 1.5f;
    m_fVolumeMaxSpeed = 200.0f;

    m_eSoundState              = ST_OFF;
    m_fNextSoundState          = level.time;
    m_pVehicleSoundEntities[0] = NULL;
    m_pVehicleSoundEntities[1] = NULL;
    m_pVehicleSoundEntities[2] = NULL;
    m_pVehicleSoundEntities[3] = NULL;

    m_bStopEnabled    = qfalse;
    m_bEnableSkidding = qfalse;
    m_fSkidAngle      = 0;
    m_vSkidOrigin     = vec_zero;
    m_fSkidLeftForce  = 0;
    m_fSkidRightForce = 0;

    m_sAnimationSet = "";
    m_sSoundSet     = "";

    for (i = 0; i < NUM_VEHICLE_TIRES; i++) {
        m_vTireEnd[i] = origin;
        m_bTireHit[i] = false;
    }

    m_bAnimMove            = false;
    m_fMaxUseAngle         = 0;
    m_bBounceStayFullSpeed = false;

    ResetSlots();

    PostEvent(EV_Vehicle_Start, EV_POSTSPAWN);
}

/*
====================
Vehicle::~Vehicle
====================
*/
Vehicle::~Vehicle()
{
    for (int i = 0; i < MAX_CORNERS; i++) {
        if (m_pVehicleSoundEntities[i]) {
            m_pVehicleSoundEntities[i]->PostEvent(EV_Remove, EV_LINKDOORS);
        }
    }

    if (m_pCollisionEntity) {
        m_pCollisionEntity->ProcessEvent(EV_Remove);
    }

    entflags &= ~ECF_VEHICLE;
}

/*
====================
Vehicle::ResetSlots
====================
*/
void Vehicle::ResetSlots(void)
{
    driver.ent                 = NULL;
    driver.boneindex           = -1;
    driver.enter_boneindex     = -1;
    driver.flags               = SLOT_UNUSED;
    lastdriver.ent             = NULL;
    lastdriver.boneindex       = -1;
    lastdriver.enter_boneindex = -1;
    lastdriver.flags           = SLOT_UNUSED;

    for (int i = 0; i < MAX_PASSENGERS; i++) {
        Passengers[i].ent             = NULL;
        Passengers[i].boneindex       = -1;
        Passengers[i].enter_boneindex = -1;
        Passengers[i].flags           = SLOT_UNUSED;
    }

    for (int i = 0; i < MAX_TURRETS; i++) {
        Turrets[i].ent             = NULL;
        Turrets[i].boneindex       = -1;
        Turrets[i].enter_boneindex = -1;
        Turrets[i].flags           = SLOT_UNUSED;
    }
}

/*
====================
Vehicle::OpenSlotsByModel
====================
*/
void Vehicle::OpenSlotsByModel(void)
{
    str bonename;
    int bonenum;
    int boneindex;

    driver.boneindex       = gi.Tag_NumForName(edict->tiki, "driver");
    driver.enter_boneindex = gi.Tag_NumForName(edict->tiki, "driver_enter");

    if (driver.flags & SLOT_UNUSED) {
        driver.ent   = NULL;
        driver.flags = SLOT_FREE;
    }

    numPassengers = 0;

    for (bonenum = 0; bonenum < MAX_PASSENGERS; bonenum++) {
        str bonenumstr = bonenum;
        bonename       = "passenger" + bonenumstr;
        boneindex      = gi.Tag_NumForName(edict->tiki, bonename.c_str());

        if (boneindex >= 0) {
            numPassengers++;

            Passengers[bonenum].boneindex       = boneindex;
            Passengers[bonenum].enter_boneindex = gi.Tag_NumForName(edict->tiki, "passenger_enter" + bonenumstr);

            if (Passengers[bonenum].flags & SLOT_UNUSED) {
                Passengers[bonenum].ent   = NULL;
                Passengers[bonenum].flags = SLOT_FREE;
            }
        }
    }

    numTurrets = 0;

    for (bonenum = 0; bonenum < MAX_TURRETS; bonenum++) {
        str bonenumstr = bonenum;
        bonename       = "turret" + bonenumstr;
        boneindex      = gi.Tag_NumForName(edict->tiki, bonename.c_str());

        if (boneindex >= 0) {
            numTurrets++;

            Turrets[bonenum].boneindex       = boneindex;
            Turrets[bonenum].enter_boneindex = gi.Tag_NumForName(edict->tiki, "turret_enter" + bonenumstr);

            if (Turrets[bonenum].flags & SLOT_UNUSED) {
                Turrets[bonenum].ent   = NULL;
                Turrets[bonenum].flags = SLOT_FREE;
            }
        }
    }
}

/*
====================
Vehicle::EventModelInit
====================
*/
void Vehicle::ModelInit(Event *ev)
{
    SetControllerTag(0, gi.Tag_NumForName(edict->tiki, "Tire_rotate_front_left"));
    SetControllerTag(1, gi.Tag_NumForName(edict->tiki, "Tire_rotate_front_right"));
    SetControllerTag(2, gi.Tag_NumForName(edict->tiki, "steeringwheel_center"));
}

/*
====================
Vehicle::VehicleStart
====================
*/
void Vehicle::VehicleStart(Event *ev)
{
    Entity       *ent;
    VehicleBase  *last;
    orientation_t orient;

    // become solid
    setSolidType(SOLID_BBOX);
    edict->r.contents = CONTENTS_BBOX;

    last = this;

    setLocalOrigin(localorigin + Vector(0.0f, 0.0f, 30.0f));
    angles.x = m_vAngles.x;
    angles.z = m_vAngles.z;

    for (ent = G_NextEntity(NULL); ent != NULL; ent = G_NextEntity(ent)) {
        if ((ent != this) && (ent->isSubclassOf(VehicleBase))) {
            if ((ent->absmax.x >= absmin.x) && (ent->absmax.y >= absmin.y) && (ent->absmax.z >= absmin.z)
                && (ent->absmin.x <= absmax.x) && (ent->absmin.y <= absmax.y) && (ent->absmin.z <= absmax.z)) {
                last->vlink  = (VehicleBase *)ent;
                last         = (VehicleBase *)ent;
                last->offset = last->origin - origin;
                last->offset = getLocalVector(last->offset);
                last->edict->s.scale *= edict->s.scale;
            }
        }
    }

    last->vlink = NULL;

    OpenSlotsByModel();

    //
    // get the seat offset
    //
    if (GetRawTag("seat", &orient)) {
        driveroffset = Vector(orient.origin);
    }
    driveroffset += seatoffset * edict->s.scale;

    SetDriverAngles(angles + seatangles);

    max_health = health;

    //
    // since 2.0: set targetname for all busy turrets
    //
    for (int i = 0; i < numTurrets; i++) {
        if (Turrets[i].flags & SLOT_BUSY) {
            Turrets[i].ent->SetTargetName(TargetName() + "_turret" + str(i));
        }
    }

    last_origin = origin;
    link();
}

/*
====================
Vehicle::SetWheelCorners
====================
*/
void Vehicle::SetWheelCorners(Event *ev)
{
    Vector size;
    Vector offset;

    size   = ev->GetVector(1);
    offset = ev->GetVector(2);

    m_vOriginCornerOffset = offset;
    maxtracedist          = size[2];

    Corners[0][0] = -(size[0] / 2);
    Corners[0][1] = (size[1] / 2);
    Corners[0][2] = size[2];

    Corners[1][0] = (size[0] / 2);
    Corners[1][1] = (size[1] / 2);
    Corners[1][2] = size[2];

    Corners[2][0] = -(size[0] / 2);
    Corners[2][1] = -(size[1] / 2);
    Corners[2][2] = size[2];

    Corners[3][0] = (size[0] / 2);
    Corners[3][1] = -(size[1] / 2);
    Corners[3][2] = size[2];

    SetupVehicleSoundEntities();
}

/*
====================
Vehicle::QuerySpeed
====================
*/
float Vehicle::QuerySpeed(void)
{
    return currentspeed;
}

/*
====================
Vehicle::QueryFreePassengerSlot
====================
*/
int Vehicle::QueryFreePassengerSlot(void)
{
    for (int i = 0; i < numPassengers; i++) {
        if (Passengers[i].flags & SLOT_FREE) {
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
int Vehicle::QueryFreeDriverSlot(void)
{
    return (driver.flags & SLOT_FREE) ? 0 : -1;
}

/*
====================
Vehicle::QueryFreeTurretSlot
====================
*/
int Vehicle::QueryFreeTurretSlot(void)
{
    for (int i = 0; i < numTurrets; i++) {
        if (Turrets[i].flags & SLOT_FREE) {
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
void Vehicle::QueryPassengerSlotPosition(int slot, float *pos)
{
    orientation_t orient;

    if (Passengers[slot].enter_boneindex >= 0) {
        GetTagPositionAndOrientation(Passengers[slot].enter_boneindex, &orient);
        VectorCopy(orient.origin, pos);
    } else {
        VectorCopy(origin, pos);
    }
}

/*
====================
Vehicle::QueryDriverSlotPosition
====================
*/
void Vehicle::QueryDriverSlotPosition(int slot, float *pos)
{
    orientation_t orient;

    if (driver.enter_boneindex >= 0) {
        GetTagPositionAndOrientation(driver.enter_boneindex, &orient);
        VectorCopy(orient.origin, pos);
    } else {
        VectorCopy(origin, pos);
    }
}

/*
====================
Vehicle::QueryTurretSlotPosition
====================
*/
void Vehicle::QueryTurretSlotPosition(int slot, float *pos)
{
    orientation_t orient;

    if (Turrets[slot].enter_boneindex >= 0) {
        GetTagPositionAndOrientation(Turrets[slot].enter_boneindex, &orient);
        VectorCopy(orient.origin, pos);
    } else {
        VectorCopy(origin, pos);
    }
}

/*
====================
Vehicle::QueryPassengerSlotAngles
====================
*/
void Vehicle::QueryPassengerSlotAngles(int slot, float *ang)
{
    orientation_t orient;

    GetTagPositionAndOrientation(Passengers[slot].enter_boneindex, &orient);
    MatrixToEulerAngles(orient.axis, ang);
}

/*
====================
Vehicle::QueryDriverSlotAngles
====================
*/
void Vehicle::QueryDriverSlotAngles(int slot, float *ang)
{
    orientation_t orient;

    GetTagPositionAndOrientation(driver.enter_boneindex, &orient);
    MatrixToEulerAngles(orient.axis, ang);
}

/*
====================
Vehicle::QueryTurretSlotAngles
====================
*/
void Vehicle::QueryTurretSlotAngles(int slot, float *ang)
{
    orientation_t orient;

    GetTagPositionAndOrientation(Turrets[slot].enter_boneindex, &orient);
    MatrixToEulerAngles(orient.axis, ang);
}

/*
====================
Vehicle::QueryPassengerSlotStatus
====================
*/
int Vehicle::QueryPassengerSlotStatus(int slot)
{
    return Passengers[slot].flags;
}

/*
====================
Vehicle::QueryDriverSlotStatus
====================
*/
int Vehicle::QueryDriverSlotStatus(int slot)
{
    return driver.flags;
}

/*
====================
Vehicle::QueryTurretSlotStatus
====================
*/
int Vehicle::QueryTurretSlotStatus(int slot)
{
    return Turrets[slot].flags;
}

/*
====================
Vehicle::QuerySpeed
====================
*/
void Vehicle::QuerySpeed(Event *ev)
{
    ev->AddFloat(QuerySpeed());
}

/*
====================
Vehicle::EventQueryFreePassengerSlot
====================
*/
void Vehicle::QueryFreePassengerSlot(Event *ev)
{
    ev->AddInteger(QueryFreePassengerSlot());
}

/*
====================
Vehicle::EventQueryFreeDriverSlot
====================
*/
void Vehicle::QueryFreeDriverSlot(Event *ev)
{
    ev->AddInteger(QueryFreeDriverSlot());
}

/*
====================
Vehicle::EventQueryFreeTurretSlot
====================
*/
void Vehicle::QueryFreeTurretSlot(Event *ev)
{
    ev->AddInteger(QueryFreeTurretSlot());
}

/*
====================
Vehicle::EventQueryPassengerSlotPosition
====================
*/
void Vehicle::QueryPassengerSlotPosition(Event *ev)
{
    Vector vPos;
    int    iSlot;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_PASSENGERS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    QueryPassengerSlotPosition(iSlot, vPos);
    ev->AddVector(vPos);
}

/*
====================
Vehicle::EventQueryDriverSlotPosition
====================
*/
void Vehicle::QueryDriverSlotPosition(Event *ev)
{
    Vector vPos;
    int    iSlot;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_DRIVERS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    QueryDriverSlotPosition(iSlot, vPos);
    ev->AddVector(vPos);
}

/*
====================
Vehicle::EventQueryTurretSlotPosition
====================
*/
void Vehicle::QueryTurretSlotPosition(Event *ev)
{
    Vector vPos;
    int    iSlot;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_TURRETS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    QueryTurretSlotPosition(iSlot, vPos);
    ev->AddVector(vPos);
}

/*
====================
Vehicle::EventQueryPassengerSlotAngles
====================
*/
void Vehicle::QueryPassengerSlotAngles(Event *ev)
{
    Vector vAngles;
    int    iSlot;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_PASSENGERS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    QueryPassengerSlotAngles(iSlot, vAngles);
    ev->AddVector(vAngles);
}

/*
====================
Vehicle::EventQueryDriverSlotAngles
====================
*/
void Vehicle::QueryDriverSlotAngles(Event *ev)
{
    Vector vAngles;
    int    iSlot;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_DRIVERS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    QueryDriverSlotAngles(iSlot, vAngles);
    ev->AddVector(vAngles);
}

/*
====================
Vehicle::EventQueryTurretSlotAngles
====================
*/
void Vehicle::QueryTurretSlotAngles(Event *ev)
{
    Vector vAngles;
    int    iSlot;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_TURRETS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    QueryTurretSlotAngles(iSlot, vAngles);
    ev->AddVector(vAngles);
}

/*
====================
Vehicle::EventQueryPassengerSlotStatus
====================
*/
void Vehicle::QueryPassengerSlotStatus(Event *ev)
{
    int iSlot;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_PASSENGERS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    ev->AddInteger(QueryPassengerSlotStatus(iSlot));
}

/*
====================
Vehicle::EventQueryDriverSlotStatus
====================
*/
void Vehicle::QueryDriverSlotStatus(Event *ev)
{
    int iSlot;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_DRIVERS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    ev->AddInteger(QueryDriverSlotStatus(iSlot));
}

/*
====================
Vehicle::EventQueryTurretSlotStatus
====================
*/
void Vehicle::QueryTurretSlotStatus(Event *ev)
{
    int iSlot;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_TURRETS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    ev->AddInteger(QueryTurretSlotStatus(iSlot));
}

/*
====================
Vehicle::EventQueryPassengerSlotEntity
====================
*/
void Vehicle::QueryPassengerSlotEntity(Event *ev)
{
    int iSlot;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_PASSENGERS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    ev->AddEntity(QueryPassengerSlotEntity(iSlot));
}

/*
====================
Vehicle::EventQueryDriverSlotEntity
====================
*/
void Vehicle::QueryDriverSlotEntity(Event *ev)
{
    int iSlot;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_DRIVERS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    ev->AddEntity(QueryDriverSlotEntity(iSlot));
}

/*
====================
Vehicle::EventQueryTurretSlotEntity
====================
*/
void Vehicle::QueryTurretSlotEntity(Event *ev)
{
    int iSlot;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_TURRETS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    ev->AddEntity(QueryTurretSlotEntity(iSlot));
}

/*
====================
Vehicle::EventAttachPassengerSlot
====================
*/
void Vehicle::AttachPassengerSlot(Event *ev)
{
    int iSlot;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_PASSENGERS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    AttachPassengerSlot(iSlot, ev->GetEntity(2), vec_zero);
    UpdatePassengerSlot(iSlot);
}

/*
====================
Vehicle::EventAttachDriverSlot
====================
*/
void Vehicle::AttachDriverSlot(Event *ev)
{
    int iSlot;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_DRIVERS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    AttachDriverSlot(0, ev->GetEntity(2), vec_zero);
    UpdateDriverSlot(0);
}

/*
====================
Vehicle::EventAttachTurretSlot
====================
*/
void Vehicle::AttachTurretSlot(Event *ev)
{
    int iSlot;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_TURRETS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    AttachTurretSlot(iSlot, ev->GetEntity(2), vec_zero, NULL);
    UpdateTurretSlot(iSlot);
}

/*
====================
Vehicle::EventDetachPassengerSlot
====================
*/
void Vehicle::DetachPassengerSlot(Event *ev)
{
    int    iSlot;
    Vector vExitPosition;
    Vector vExitAngles;
    bool   bHasExitAngles = false;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_PASSENGERS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    if (ev->NumArgs() == 2) {
        if (ev->IsVectorAt(2)) {
            vExitPosition = ev->GetVector(2);
        } else if (ev->IsEntityAt(2)) {
            vExitPosition  = ev->GetEntity(2)->origin;
            bHasExitAngles = true;
        } else if (ev->IsSimpleEntityAt(2)) {
            vExitPosition = ev->GetSimpleEntity(2)->origin;
        }

        DetachPassengerSlot(iSlot, vExitPosition, bHasExitAngles ? &vExitAngles : NULL);
    } else {
        DetachPassengerSlot(iSlot, vec_zero, NULL);
    }
}

/*
====================
Vehicle::EventDetachDriverSlot
====================
*/
void Vehicle::DetachDriverSlot(Event *ev)
{
    int    iSlot;
    Vector vExitPosition;
    Vector vExitAngles;
    bool   bHasExitAngles = false;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_DRIVERS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    if (ev->NumArgs() == 2) {
        if (ev->IsVectorAt(2)) {
            vExitPosition = ev->GetVector(2);
        } else if (ev->IsEntityAt(2)) {
            vExitPosition  = ev->GetEntity(2)->origin;
            bHasExitAngles = true;
        } else if (ev->IsSimpleEntityAt(2)) {
            vExitPosition = ev->GetSimpleEntity(2)->origin;
        }

        DetachDriverSlot(iSlot, vExitPosition, bHasExitAngles ? &vExitAngles : NULL);
    } else {
        DetachDriverSlot(iSlot, vec_zero, NULL);
    }
}

/*
====================
Vehicle::EventDetachTurretSlot
====================
*/
void Vehicle::DetachTurretSlot(Event *ev)
{
    int    iSlot;
    Vector vExitPosition;
    Vector vExitAngles;
    bool   bHasExitAngles = false;

    iSlot = ev->GetInteger(1);
    if (iSlot >= MAX_TURRETS) {
        ScriptError("Slot Specified is greater than maximum allowed for that parameter\n");
    }

    if (ev->NumArgs() == 2) {
        if (ev->IsVectorAt(2)) {
            vExitPosition = ev->GetVector(2);
        } else if (ev->IsEntityAt(2)) {
            vExitAngles    = ev->GetEntity(2)->angles;
            bHasExitAngles = true;
            vExitPosition  = ev->GetEntity(2)->origin;
        } else if (ev->IsSimpleEntityAt(2)) {
            vExitPosition = ev->GetSimpleEntity(2)->origin;
        }
    }

    DetachTurretSlot(iSlot, vExitPosition, bHasExitAngles ? &vExitAngles : NULL);
}

/*
====================
Vehicle::QueryPassengerSlotEntity
====================
*/
Entity *Vehicle::QueryPassengerSlotEntity(int slot)
{
    return Passengers[slot].ent;
}

/*
====================
Vehicle::QueryDriverSlotEntity
====================
*/
Entity *Vehicle::QueryDriverSlotEntity(int slot)
{
    return driver.ent;
}

/*
====================
Vehicle::QueryTurretSlotEntity
====================
*/
Entity *Vehicle::QueryTurretSlotEntity(int slot)
{
    return Turrets[slot].ent;
}

/*
====================
Vehicle::FindExitPosition
====================
*/
bool Vehicle::FindExitPosition(Entity *pEnt, const Vector& vOrigin, const Vector *vAngles)
{
    Event  *ev;
    Vector  yawAngles;
    Vector  forward;
    Vector  offset;
    float   radius;
    int     i, j;
    trace_t trace;

    if (!pEnt) {
        return false;
    }

    if (!pEnt->IsSubclassOfPlayer() && vOrigin != vec_zero) {
        if (vAngles) {
            pEnt->setAngles(*vAngles);
        }

        pEnt->setOrigin(vOrigin);
        pEnt->velocity = vec_zero;

        ev = new Event(EV_Vehicle_Exit);
        ev->AddEntity(this);
        pEnt->ProcessEvent(ev);

        return true;
    }

    radius = (pEnt->size.length() + size.length()) * 0.5;

    for (i = 0; i < 128; i += 32) {
        for (j = 0; j < 360; j += 30) {
            yawAngles    = vec_zero;
            yawAngles[1] = j + angles[1] + 180;
            yawAngles.AngleVectorsLeft(&forward);

            offset = origin + forward * radius;
            offset[2] += i;

            trace = G_Trace(
                offset,
                pEnt->mins,
                pEnt->maxs,
                offset,
                NULL,
                pEnt->edict->clipmask,
                pEnt->IsSubclassOfPlayer() ? qtrue : qfalse,
                " Vehicle::FindExitPosition"
            );

            if (!trace.startsolid && !trace.allsolid) {
                Vector end;

                offset = trace.endpos;
                end    = offset;
                end[2] -= 128;

                trace = G_Trace(
                    offset,
                    pEnt->mins,
                    pEnt->maxs,
                    end,
                    NULL,
                    pEnt->edict->clipmask,
                    pEnt->IsSubclassOfPlayer() ? qtrue : qfalse,
                    " Vehicle::FindExitPosition"
                );

                if (!trace.startsolid && !trace.allsolid && trace.fraction < 1) {
                    pEnt->setOrigin(trace.endpos);
                    pEnt->velocity = vec_zero;

                    if (vAngles) {
                        pEnt->setAngles(*vAngles);
                    }

                    ev = new Event(EV_Vehicle_Exit);
                    ev->AddEntity(this);
                    pEnt->ProcessEvent(ev);
                    return true;
                }
            }
        }
    }

    return false;
}

/*
====================
Vehicle::AttachPassengerSlot
====================
*/
void Vehicle::AttachPassengerSlot(int slot, Entity *ent, Vector vExitPosition)
{
    Entity *passenger;
    str     sName;

    if (!ent) {
        return;
    }

    passenger = Passengers[slot].ent;
    if (!passenger) {
        Passengers[slot].ent   = ent;
        Passengers[slot].flags = SLOT_BUSY;

        sName = m_sSoundSet + "snd_doorclose";
        Sound(sName);

        Event *event = new Event(EV_Vehicle_Enter);
        event->AddEntity(this);
        Passengers[slot].ent->ProcessEvent(event);

        offset = ent->origin - origin;

        flags |= FL_POSTTHINK;
        SetDriverAngles(seatangles + angles);
    } else if (!isLocked() && ent == passenger) {
        DetachPassengerSlot(slot, vec_zero, NULL);
    }
}

/*
====================
Vehicle::AttachDriverSlot
====================
*/
void Vehicle::AttachDriverSlot(int slot, Entity *ent, Vector vExitPosition)
{
    Entity *d;
    str     sName;

    if (!ent || !ent->IsSubclassOfSentient()) {
        return;
    }

    d = Driver();
    if (!d) {
        driver.ent     = ent;
        driver.flags   = SLOT_BUSY;
        lastdriver.ent = driver.ent;

        sName = m_sSoundSet + "snd_doorclose";
        Sound(sName);

        sName = m_sSoundSet + "snd_start";
        Sound(sName);

        Event *event = new Event(EV_Vehicle_Enter);
        event->AddEntity(this);
        driver.ent->ProcessEvent(event);

        offset = ent->origin - origin;

        flags |= FL_POSTTHINK;
        SetDriverAngles(seatangles + angles);
        DriverAdded();
    } else if (ent == d && !isLocked()) {
        DetachDriverSlot(slot, vec_zero, NULL);
    }
}

/*
====================
Vehicle::AttachTurretSlot

Attach a turret or a sentient that will use the turret to the vehicle.
====================
*/
void Vehicle::AttachTurretSlot(int slot, Entity *ent, Vector vExitPosition, Vector *vExitAngles)
{
    TurretGun        *pTurret;
    VehicleTurretGun *pVehicleTurret;
    str               sName;

    if (!ent) {
        return;
    }

    pTurret = (TurretGun *)Turrets[slot].ent.Pointer();
    if (pTurret && ent->IsSubclassOfWeapon()) {
        if (ent == pTurret && !isLocked()) {
            DetachTurretSlot(slot, vec_zero, NULL);
        }
    } else if (ent->IsSubclassOfWeapon()) {
        Turrets[slot].ent   = ent;
        Turrets[slot].flags = SLOT_BUSY;

        pTurret = (TurretGun *)ent;

        ent->takedamage = DAMAGE_NO;
        ent->setSolidType(SOLID_NOT);

        Event *event = new Event(EV_Vehicle_Enter);
        event->AddEntity(this);
        Turrets[slot].ent->ProcessEvent(event);

        offset = ent->origin - origin;

        flags |= FL_POSTTHINK;
        Turrets[slot].ent->setAngles(angles);

        if (pTurret->IsSubclassOfTurretGun()) {
            pTurret->m_bUsable   = false;
            pTurret->m_bRestable = false;
        }
    } else if (pTurret) {
        Entity *pTurretOwner       = NULL;
        Entity *pRemoteTurretOwner = NULL;

        if (pTurret->IsSubclassOfTurretGun()) {
            pTurretOwner = pTurret->GetOwner();
        }

        if (pTurret->IsSubclassOfVehicleTurretGun()) {
            pVehicleTurret     = (VehicleTurretGun *)pTurret;
            pRemoteTurretOwner = pVehicleTurret->GetRawRemoteOwner();
        }

        if (pTurret->IsSubclassOfTurretGun()) {
            if (pTurret->IsSubclassOfVehicleTurretGun() && pVehicleTurret->isLocked()) {
                ScriptError("Turret is locked, cannot attach to turret slot.");
            }

            pTurret->m_bUsable = true;
        }

        Event *event = new Event(EV_Use);
        event->AddEntity(ent);
        pTurret->ProcessEvent(event);

        if (ent->IsSubclassOfPlayer()) {
            Player *pPlayer     = (Player *)ent;
            pPlayer->m_pVehicle = this;
        }

        if (pTurret->IsSubclassOfTurretGun()) {
            pTurret->m_bUsable = false;
        }

        if (pTurretOwner == ent || pRemoteTurretOwner == ent) {
            if (pRemoteTurretOwner) {
                pVehicleTurret->SetRemoteOwner(NULL);
            }

            FindExitPosition(ent, vExitPosition, vExitAngles);
        }
    }
}

/*
====================
Vehicle::DetachPassengerSlot
====================
*/
void Vehicle::DetachPassengerSlot(int slot, Vector vExitPosition, Vector *vExitAngles)
{
    Entity *passenger = Passengers[slot].ent;

    if (!passenger) {
        return;
    }

    if (!FindExitPosition(passenger, vExitPosition, vExitAngles)) {
        // cannot exit
        return;
    }

    Sound(m_sSoundSet + "snd_dooropen");
    Passengers[slot].ent   = NULL;
    Passengers[slot].flags = SLOT_FREE;
}

/*
====================
Vehicle::DetachDriverSlot
====================
*/
void Vehicle::DetachDriverSlot(int slot, Vector vExitPosition, Vector *vExitAngles)
{
    Entity *other = driver.ent;

    if (!other) {
        return;
    }

    if (!FindExitPosition(other, vExitPosition, vExitAngles)) {
        // cannot exit
        return;
    }

    turnimpulse = 0;
    moveimpulse = 0;
    jumpimpulse = 0;

    if (drivable) {
        StopLoopSound();
        Sound("snd_dooropen", CHAN_BODY);
        Sound("snd_stop", CHAN_VOICE);
        driver.ent->setSolidType(SOLID_BBOX);
    }

    driver.ent   = NULL;
    driver.flags = SLOT_FREE;
}

/*
====================
Vehicle::DetachTurretSlot

Detach a turret or a sentient.
====================
*/
void Vehicle::DetachTurretSlot(int slot, Vector vExitPosition, Vector *vExitAngles)
{
    Entity *passenger = Turrets[slot].ent;

    if (!passenger) {
        return;
    }

    if (!FindExitPosition(passenger, vExitPosition, vExitAngles)) {
        // cannot exit
        return;
    }

    turnimpulse = 0;
    moveimpulse = 0;
    jumpimpulse = 0;
    passenger->setSolidType(SOLID_BBOX);

    Sound(m_sSoundSet + "snd_dooropen");

    if (passenger->IsSubclassOfTurretGun()) {
        TurretGun *pTurret   = static_cast<TurretGun *>(passenger);
        pTurret->m_bUsable   = true;
        pTurret->m_bRestable = true;
    }

    Turrets[slot].ent   = NULL;
    Turrets[slot].flags = SLOT_FREE;
}

/*
====================
Vehicle::FindPassengerSlotByEntity
====================
*/
int Vehicle::FindPassengerSlotByEntity(Entity *ent)
{
    for (int i = 0; i < MAX_PASSENGERS; i++) {
        if (Passengers[i].ent == ent) {
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
int Vehicle::FindDriverSlotByEntity(Entity *ent)
{
    return driver.ent == ent ? 0 : -1;
}

/*
====================
Vehicle::FindTurretSlotByEntity
====================
*/
int Vehicle::FindTurretSlotByEntity(Entity *ent)
{
    for (int i = 0; i < MAX_TURRETS; i++) {
        if (Turrets[i].ent == ent) {
            return i;
        }
    }

    return -1;
}

/*
====================
Vehicle::Drivable
====================
*/
void Vehicle::Drivable(Event *ev)
{
    setMoveType(MOVETYPE_VEHICLE);
    drivable = true;
}

/*
====================
Vehicle::PathDrivable
====================
*/
void Vehicle::PathDrivable(Event *ev)
{
    pathDrivable = ev->GetBoolean(1);
}

/*
====================
Vehicle::UnDrivable
====================
*/
void Vehicle::UnDrivable(Event *ev)
{
    setMoveType(MOVETYPE_PUSH);
    drivable = false;
}

/*
====================
Vehicle::Jumpable
====================
*/
void Vehicle::Jumpable(Event *ev)
{
    jumpable = true;
}

/*
====================
Vehicle::Lock
====================
*/
void Vehicle::Lock(Event *ev)
{
    Lock();
}

/*
====================
Vehicle::UnLock
====================
*/
void Vehicle::UnLock(Event *ev)
{
    UnLock();
}

/*
====================
Vehicle::SteerInPlace
====================
*/
void Vehicle::SteerInPlace(Event *ev)
{
    steerinplace = true;
}

/*
====================
Vehicle::SeatAnglesOffset
====================
*/
void Vehicle::SeatAnglesOffset(Event *ev)
{
    seatangles = ev->GetVector(1);
}

/*
====================
Vehicle::SeatOffset
====================
*/
void Vehicle::SeatOffset(Event *ev)
{
    seatoffset = ev->GetVector(1);
}

/*
====================
Vehicle::SetWeapon
====================
*/
void Vehicle::SetWeapon(Event *ev)
{
    showweapon = true;
    hasweapon  = true;
    weaponName = ev->GetString(1);
}

/*
====================
Vehicle::SetName
====================
*/
void Vehicle::SetName(Event *ev)
{
    vehicleName = ev->GetString(1);
}

/*
====================
Vehicle::ShowWeaponEvent
====================
*/
void Vehicle::ShowWeaponEvent(Event *ev)
{
    showweapon = true;
}

/*
====================
Vehicle::HasWeapon
====================
*/
qboolean Vehicle::HasWeapon(void)
{
    return hasweapon;
}

/*
====================
Vehicle::ShowWeapon
====================
*/
qboolean Vehicle::ShowWeapon(void)
{
    return showweapon;
}

/*
====================
Vehicle::SetDriverAngles
====================
*/
void Vehicle::SetDriverAngles(Vector angles)
{
    int i;

    if (!driver.ent || !driver.ent->client) {
        return;
    }

    for (i = 0; i < 3; i++) {
        driver.ent->client->ps.delta_angles[i] = ANGLE2SHORT(angles[i] - driver.ent->client->cmd_angles[i]);
    }
}

/*
====================
Vehicle::Driver
====================
*/
Entity *Vehicle::Driver(void)
{
    return driver.ent;
}

/*
====================
Vehicle::IsDrivable
====================
*/
qboolean Vehicle::IsDrivable(void)
{
    return drivable;
}

/*
====================
Vehicle::BounceForwardsVelocity
====================
*/
void Vehicle::BounceForwardsVelocity(Event *ev)
{
    if (m_bBounceBackwards) {
        m_bBounceStayFullSpeed = false;
        m_bBounceBackwards     = false;
        currentspeed           = -currentspeed;
        moveimpulse            = -moveimpulse;
    }
}

/*
====================
Vehicle::BounceBackwardsVelocity
====================
*/
void Vehicle::BounceBackwardsVelocity(Event *ev)
{
    if (!m_bBounceBackwards) {
        m_bBounceBackwards = true;
        currentspeed       = -currentspeed;
        moveimpulse        = -moveimpulse;
        if (ev->NumArgs() > 0) {
            m_bBounceStayFullSpeed = ev->GetBoolean(1);
        }
    }
}

/*
====================
Vehicle::StopForwardsVelocity
====================
*/
void Vehicle::StopForwardsVelocity(Event *ev)
{
    if (!m_bBounceBackwards) {
        moveimpulse = 0;
    }
}

/*
====================
Vehicle::StopBackwardsVelocity
====================
*/
void Vehicle::StopBackwardsVelocity(Event *ev)
{
    if (m_bBounceBackwards) {
        moveimpulse = 0;
    }
}

/*
====================
Vehicle::EventSetSpeed
====================
*/
void Vehicle::SetSpeed(Event *ev)
{
    speed = ev->GetFloat(1);
}

/*
====================
Vehicle::EventSetTurnRate
====================
*/
void Vehicle::SetTurnRate(Event *ev)
{
    maxturnrate = ev->GetFloat(1);
}

/*
====================
Vehicle::SetMass
====================
*/
void Vehicle::SetMass(Event *ev)
{
    m_fMass      = ev->GetFloat(1);
    m_fFrontMass = m_fMass * 0.5;
    m_fBackMass  = m_fMass * 0.5;
}

/*
====================
Vehicle::SetFrontMass
====================
*/
void Vehicle::SetFrontMass(Event *ev)
{
    m_fFrontMass = ev->GetFloat(1);
    m_fMass      = m_fFrontMass + m_fBackMass;
}

/*
====================
Vehicle::SetBackMass
====================
*/
void Vehicle::SetBackMass(Event *ev)
{
    m_fBackMass = ev->GetFloat(1);
    m_fMass     = m_fFrontMass + m_fBackMass;
}

/*
====================
Vehicle::SetTread
====================
*/
void Vehicle::SetTread(Event *ev)
{
    m_fTread = ev->GetFloat(1);
}

/*
====================
Vehicle::SetTireRadius
====================
*/
void Vehicle::SetTireRadius(Event *ev)
{
    m_fTireRadius = ev->GetFloat(1);
}

/*
====================
Vehicle::SetRollingResistance
====================
*/
void Vehicle::SetRollingResistance(Event *ev)
{
    m_fRollingResistance = ev->GetFloat(1);
}

/*
====================
Vehicle::SetDrag
====================
*/
void Vehicle::SetDrag(Event *ev)
{
    m_fDrag = ev->GetFloat(1);
}

/*
====================
Vehicle::BouncyCoef
====================
*/
void Vehicle::BouncyCoef(Event *ev)
{
    m_fBouncyCoef = ev->GetFloat(1);
}

/*
====================
Vehicle::SpringyCoef
====================
*/
void Vehicle::SpringyCoef(Event *ev)
{
    m_fSpringyCoef = ev->GetFloat(1);
}

/*
====================
Vehicle::YawMinMax
====================
*/
void Vehicle::YawMinMax(Event *ev)
{
    if (ev->NumArgs() != 3) {
        ScriptError("No Parameter for YawMinMax");
    }

    m_fYawMin  = ev->GetFloat(1);
    m_fYawMax  = ev->GetFloat(2);
    m_fYawCoef = ev->GetFloat(3);

    if (m_fYawMin > m_fYawMax) {
        ScriptError("Mismatched mins and maxs for YawMinMax");
    }
}

/*
====================
Vehicle::RollMinMax
====================
*/
void Vehicle::RollMinMax(Event *ev)
{
    if (ev->NumArgs() != 3) {
        ScriptError("No Parameter for RollMinMax");
    }

    m_fRollMin  = ev->GetFloat(1);
    m_fRollMax  = ev->GetFloat(2);
    m_fRollCoef = ev->GetFloat(3);

    if (m_fRollMin > m_fRollMax) {
        ScriptError("Mismatched mins and maxs for RollMinMax");
    }
}

/*
====================
Vehicle::ZMinMax
====================
*/
void Vehicle::ZMinMax(Event *ev)
{
    if (ev->NumArgs() != 3) {
        ScriptError("No Parameter for ZMinMax");
    }

    m_fZMin  = ev->GetFloat(1);
    m_fZMax  = ev->GetFloat(2);
    m_fZCoef = ev->GetFloat(3);

    if (m_fZMin > m_fZMax) {
        ScriptError("Mismatched mins and maxs for ZMinMax");
    }
}

/*
====================
Vehicle::SetAnimationSet
====================
*/
void Vehicle::SetAnimationSet(Event *ev)
{
    m_sAnimationSet = ev->GetString(1);
}

/*
====================
Vehicle::SetSoundSet
====================
*/
void Vehicle::SetSoundSet(Event *ev)
{
    m_sSoundSet = ev->GetString(1);
}

/*
====================
Vehicle::GetTagPositionAndOrientation
====================
*/
qboolean Vehicle::GetTagPositionAndOrientation(int tagnum, orientation_t *new_or)
{
    int           i;
    orientation_t tag_or;
    float         axis[3][3];

    GetRawTag(tagnum, &tag_or);

    AnglesToAxis(angles, axis);

    VectorCopy(origin, new_or->origin);

    for (i = 0; i < 3; i++) {
        VectorMA(new_or->origin, tag_or.origin[i], tag_or.axis[i], new_or->origin);
    }

    MatrixMultiply(tag_or.axis, axis, new_or->axis);
    return true;
}

/*
====================
Vehicle::GetTagPositionAndOrientation
====================
*/
qboolean Vehicle::GetTagPositionAndOrientation(str tagname, orientation_t *new_or)
{
    int tagnum = gi.Tag_NumForName(edict->tiki, tagname.c_str());

    if (tagnum < 0) {
        warning("Vehicle::GetTagPositionAndOrientation", "Could not find tag \"%s\"", tagname.c_str());
        return false;
    } else {
        return GetTagPositionAndOrientation(tagnum, new_or);
    }
}

/*
====================
Vehicle::CheckWater
====================
*/
void Vehicle::CheckWater(void)
{
    Vector       point;
    int          cont;
    int          sample1;
    int          sample2;
    VehicleBase *v;

    unlink();
    v = this;
    while (v->vlink) {
        v = v->vlink;
        v->unlink();
    }

    if (driver.ent) {
        driver.ent->unlink();
    }

    //
    // get waterlevel
    //
    waterlevel = 0;
    watertype  = 0;

    sample2 = maxs[2] - mins[2];
    sample1 = sample2 / 2;

    point = origin;
    point[2] += mins[2];
    cont = gi.pointcontents(point, 0);

    if (cont & MASK_WATER) {
        watertype  = cont;
        waterlevel = 1;
        point[2]   = origin[2] + mins[2] + sample1;
        cont       = gi.pointcontents(point, 0);
        if (cont & MASK_WATER) {
            waterlevel = 2;
            point[2]   = origin[2] + mins[2] + sample2;
            cont       = gi.pointcontents(point, 0);
            if (cont & MASK_WATER) {
                waterlevel = 3;
            }
        }
    }

    link();
    v = this;
    while (v->vlink) {
        v = v->vlink;
        v->link();
    }

    if (driver.ent) {
        driver.ent->link();
        driver.ent->waterlevel = waterlevel;
        driver.ent->watertype  = watertype;
    }
}

/*
====================
Vehicle::Drive
====================
*/
qboolean Vehicle::Drive(usercmd_t *ucmd)
{
    Vector i, j, k;

    i = velocity;
    VectorNormalize(i);

    if (!driver.ent || !driver.ent->isClient()) {
        return false;
    }

    if (!drivable) {
        driver.ent->client->ps.pm_flags |= PMF_FROZEN;
        ucmd->forwardmove = 0;
        ucmd->rightmove   = 0;
        ucmd->upmove      = 0;
        return false;
    }

    driver.ent->client->ps.pm_flags |= PMF_NO_PREDICTION;

    moveimpulse  = ((float)ucmd->forwardmove) * (VectorLength(i) + 1.0);
    m_bIsBraking = ucmd->forwardmove < 0;
    m_fAccelerator += ((float)ucmd->forwardmove) * 0.005;
    if (m_fAccelerator < 0.0) {
        m_fAccelerator = 0.0;
    } else if (m_fAccelerator > 1.0) {
        m_fAccelerator = 1.0;
    }

    turnimpulse = ((float)-ucmd->rightmove);
    jumpimpulse = ((float)ucmd->upmove * gravity) / 350;
    if ((jumpimpulse < 0) || (!jumpable)) {
        jumpimpulse = 0;
    }

    turnimpulse += angledist(SHORT2ANGLE(ucmd->angles[1]) - driver.ent->client->cmd_angles[1]) * 8;

    return true;
}

/*
====================
Vehicle::PathDrive
====================
*/
qboolean Vehicle::PathDrive(usercmd_t *ucmd)
{
    if (!pathDrivable) {
        return false;
    }

    if (m_bAutoPilot) {
        if (ucmd->forwardmove > 0) {
            //
            // forward speed
            //
            if (m_bBounceBackwards) {
                m_fIdealSpeed = 0;
                if (!currentspeed) {
                    m_bBounceBackwards = false;
                }
            } else {
                m_fIdealSpeed = m_fMaxSpeed;
            }
        } else if (ucmd->forwardmove < 0) {
            //
            // backward speed
            //
            if (m_bBounceBackwards) {
                m_fIdealSpeed = -m_fMaxSpeed;
            } else {
                m_fIdealSpeed = 0;
                if (!currentspeed) {
                    m_bBounceBackwards = true;
                }
            }
        } else {
            //
            // stopped
            //
            m_fIdealSpeed = 0;
        }
    }

    return true;
}

/*
====================
Vehicle::AutoPilot
====================
*/
void Vehicle::AutoPilot(void)
{
    Vector vAngles;
    Vector vDeltaAngles, vDelta, vDeltaSave;
    float *vTmp;
    vec3_t vPrev, vCur, vTotal;
    float  fTotal, fCoef;
    Vector vWishPosition, vAlternateWishPosition;
    float  fDistToCurPos, fCurPathPosition;

    if (!m_pCurPath || m_pCurPath->m_iPoints == 0) {
        m_bAutoPilot = false;
        return;
    }

    if (g_showvehiclepath && g_showvehiclepath->integer) {
        int    iFlags;
        float  fBS;
        Vector vTmp1;
        Vector vTmp2;
        Vector vBS;

        iFlags = 0;
        fBS    = sin(level.time) * 16;
        vBS    = Vector(1, 1, 1) * fBS;

        for (int i = 0; i < m_pCurPath->m_iPoints; i++) {
            float  fZ = 0;
            Vector vColor(0, 1, 1);

            vTmp  = m_pCurPath->GetByNode(i, &iFlags);
            vTmp1 = vTmp + 1;
            fZ    = 0;

            if (iFlags & 1) {
                vColor = Vector(1, 1, 1);
                fZ += 1;
                G_DebugString(vTmp1 + Vector(0, 0, fZ * 32), sin(level.time) + 3, 1, 1, 0, "START_STOPPING");
            }
            if (iFlags & 2) {
                vColor = Vector(0, 1, 1);
                fZ += 1;
                G_DebugString(vTmp1 + Vector(0, 0, fZ * 32), sin(level.time) + 3, 0, 1, 0, "START_SKIDDING");
            }
            if (iFlags & 4) {
                fZ += 1;
                G_DebugString(vTmp1 + Vector(0, 0, fZ * 32), sin(level.time) + 3, 0, 0, 1, "STOP_SKIDDING");
            }

            if (i == m_iCurNode) {
                G_DebugBBox(vTmp1, vBS * -1, vBS, 1, 0, 0, 1);
            } else {
                G_DebugBBox(vTmp1, vBS * -1, vBS, vColor[0], vColor[1], vColor[2], 1);
            }

            vTmp  = m_pCurPath->GetByNode(i + 1, NULL);
            vTmp2 = vTmp + 1;

            G_DebugLine(vTmp1, vTmp2, 0, 1, 0, 1);
        }
    }

    do {
        if (m_iCurNode > 0) {
            int iFlags;

            vTmp = m_pCurPath->GetByNode(m_iCurNode, NULL);
            VectorCopy(vTmp + 1, vCur);

            if (g_showvehiclemovedebug->integer) {
                G_DebugString(vCur, 3, 1, 1, 1, "%f", vTmp[0]);
            }

            vTmp = m_pCurPath->GetByNode(m_iCurNode - 1, NULL);
            VectorCopy(vTmp + 1, vPrev);

            if (g_showvehiclemovedebug->integer) {
                G_DebugString(vPrev, 3, 1, 1, 1, "%f", vTmp[0]);
            }

            VectorSubtract(vCur, vPrev, vTotal);
            fTotal      = VectorLength(vTotal);
            m_vIdealDir = vTotal;

            VectorNormalize(m_vIdealDir);
            angles.AngleVectorsLeft(&vDelta, NULL, NULL);
            fCoef = ProjectLineOnPlane(vDelta, DotProduct(origin, vDelta), vPrev, vCur, NULL);

            if (g_showvehiclemovedebug->integer) {
                Vector vColor;
                G_DebugBBox(vColor, Vector(-32, -32, -32), Vector(32, 32, 32), 0, 1, 1, 1);
                G_DebugBBox(vColor, Vector(-32, -32, -32), Vector(32, 32, 32), 1, 1, 0, 1);
                G_DebugArrow(vColor, m_vIdealDir * -1, (1 - fCoef) * fTotal, 0, 1, 0, 1);
                G_DebugArrow(vColor, m_vIdealDir * 1, fCoef * fTotal, 0, 0, 1, 1);
            }

            vTmp             = m_pCurPath->GetByNode(m_iCurNode - (1.0 - fCoef), NULL);
            fCurPathPosition = vTmp[0];
            vTmp             = m_pCurPath->Get(fCurPathPosition + m_fLookAhead, NULL);
            vWishPosition    = vTmp + 1;
            fDistToCurPos    = Vector(origin[0] - vWishPosition[0], origin[1] - vWishPosition[1], 0).length();

            if (fCoef > 1 && !m_bBounceBackwards) {
                m_iCurNode++;
                if (m_iCurNode >= m_pCurPath->m_iPoints) {
                    break;
                }

                m_pCurPath->GetByNode(m_iCurNode, &iFlags);
                if (iFlags & 1) {
                    ProcessEvent(EV_Vehicle_StopAtEnd);
                }

                if (iFlags & 2) {
                    Event *event = new Event(EV_Vehicle_Skidding);
                    event->AddInteger(1);
                    ProcessEvent(event);
                }

                if (iFlags & 4) {
                    Event *event = new Event(EV_Vehicle_Skidding);
                    event->AddInteger(0);
                    ProcessEvent(event);
                }
            } else if (fCoef < 0 && m_bBounceBackwards) {
                m_iCurNode--;
                if (m_iCurNode < 1) {
                    m_iCurNode         = 1;
                    m_bBounceBackwards = false;
                    currentspeed       = -currentspeed;
                    moveimpulse        = -moveimpulse;
                }
            }

        cont:
            if (m_pAlternatePath && m_pAlternatePath->m_iPoints) {
                if (m_iAlternateNode > 1) {
                    vTmp = m_pAlternatePath->GetByNode(m_iAlternateNode, NULL);
                    VectorCopy(vTmp + 1, vCur);

                    vTmp = m_pAlternatePath->GetByNode(m_iAlternateNode - 1, NULL);
                    VectorCopy(vTmp + 1, vPrev);

                    VectorSubtract(vCur, vPrev, vTotal);
                    fTotal      = VectorLength(vTotal);
                    m_vIdealDir = vTotal;

                    VectorNormalize(m_vIdealDir);
                    angles.AngleVectorsLeft(&vDelta, NULL, NULL);
                    fCoef = ProjectLineOnPlane(vDelta, DotProduct(origin, vDelta), vPrev, vCur, NULL);

                    vTmp = m_pCurPath->GetByNode(m_iAlternateNode - (1.0 - fCoef), NULL);
                    // 2.30: bounce backward
                    if (m_bBounceBackwards) {
                        vTmp = m_pCurPath->Get(vTmp[0] - m_fLookAhead, NULL);
                    } else {
                        vTmp = m_pCurPath->Get(vTmp[0] + m_fLookAhead, NULL);
                    }
                    vAlternateWishPosition = vTmp + 1;

                    if (fCoef > 1) {
                        m_iAlternateNode++;
                    } else if (fCoef < 0) {
                        // 2.30: backward
                        m_iAlternateNode--;
                    }
                } else {
                    vTmp                   = m_pAlternatePath->GetByNode(m_iAlternateNode, NULL);
                    vAlternateWishPosition = vTmp + 1;
                    m_vIdealDir            = origin - vWishPosition;
                    m_vIdealDir[2]         = 0;
                    m_vIdealDir.normalize();

                    vDelta    = vAlternateWishPosition - origin;
                    vDelta[2] = 0;

                    if (vDelta.length() <= m_fIdealDistance) {
                        m_iAlternateNode++;
                    }
                }
            }

            vDelta = vWishPosition - origin;

            if (g_showvehiclemovedebug->integer) {
                G_DebugLine(vWishPosition, origin, 1, 0, 0, 1);
            }

            vDeltaSave = vDelta;
            vDelta.normalize();

            if (m_pAlternatePath && m_pAlternatePath->m_iPoints) {
                trace_t trace;

                trace = G_Trace(
                    origin, vec_zero, vec_zero, vWishPosition, this, edict->clipmask, qfalse, "Vehicle::AutoPilot"
                );
                if ((trace.fraction < 1 || trace.startsolid || trace.allsolid) && trace.ent
                    && trace.entityNum != ENTITYNUM_WORLD) {
                    vDelta     = vAlternateWishPosition - origin;
                    vDeltaSave = vDelta;
                    vDelta.normalize();
                }
            }

            vDelta[2]     = 0;
            vDeltaSave[2] = 0;

            if (fDistToCurPos > 1) {
                if (moveimpulse >= 0 && !m_bBounceBackwards) { // 2.30: checks if it doesn't bounce backwards
                    vectoangles(vDelta, vDeltaAngles);
                    turnimpulse = angledist(vDeltaAngles.y - angles.y);
                } else {
                    Vector vVec = vDelta * -1;

                    vectoangles(vVec, vDeltaAngles);
                    if (IsSubclassOfVehicleTank()) {
                        turnimpulse = angledist(vDeltaAngles.y - angles.y);
                    } else {
                        turnimpulse = -angledist(vDeltaAngles.y - angles.y);
                    }
                }
            }

            if (m_bStopEnabled) {
                moveimpulse = (fCurPathPosition - m_fStopStartDistance) / (m_fStopEndDistance - m_fStopStartDistance);
                moveimpulse *= moveimpulse;
                moveimpulse *= moveimpulse;
                moveimpulse *= moveimpulse;
                moveimpulse *= moveimpulse;
                moveimpulse = 1 - moveimpulse;
                moveimpulse *= m_fStopStartSpeed;

                if (fabs(moveimpulse) < 2) {
                    if (m_fIdealSpeed >= 0) {
                        moveimpulse = 2;
                    } else {
                        moveimpulse = -2;
                    }
                }
                // 2.30: check if it stays full speed when bouncing
            } else if (m_bBounceStayFullSpeed) {
                if (m_fIdealSpeed > fabs(moveimpulse)) {
                    moveimpulse -= m_fIdealAccel * level.frametime;
                    if (fabs(moveimpulse) > m_fIdealSpeed) {
                        moveimpulse = -m_fIdealSpeed;
                    }
                }
            } else if (m_fIdealSpeed > moveimpulse) {
                moveimpulse += m_fIdealAccel * level.frametime;
                if (moveimpulse > m_fIdealSpeed) {
                    moveimpulse = m_fIdealSpeed;
                }
            } else if (m_fIdealSpeed < moveimpulse) {
                moveimpulse -= m_fIdealAccel * level.frametime;
                if (moveimpulse < m_fIdealSpeed) {
                    moveimpulse = m_fIdealSpeed;
                }
            }

            if (m_iNextPathStartNode >= 0 && m_pNextPath && m_pNextPath->m_iPoints
                && m_iCurNode > m_iNextPathStartNode + 2) {
                cVehicleSpline *spline = m_pCurPath;
                m_pCurPath             = m_pNextPath;
                m_pNextPath            = spline;
                m_iCurNode             = 2;
                m_pNextPath->Reset();
                m_iNextPathStartNode = -1;
                Unregister(STRING_DRIVE);
                m_bStopEnabled    = false;
                m_bEnableSkidding = false;
            }
            return;
        }

        vTmp           = m_pCurPath->GetByNode(m_iCurNode, NULL);
        vWishPosition  = vTmp + 1;
        m_vIdealDir    = origin - vWishPosition;
        m_vIdealDir[2] = 0;
        m_vIdealDir.normalize();

        vDelta    = vWishPosition - origin;
        vDelta[2] = 0;

        fDistToCurPos = 2;
        if (vDelta.length() > m_fIdealDistance) {
            goto cont;
        }

        fDistToCurPos = 0;

        m_iCurNode++;
    } while (m_iCurNode < m_pCurPath->m_iPoints);

    if (m_bStopEnabled) {
        moveimpulse = 0;
    }

    m_bAutoPilot      = false;
    m_bStopEnabled    = false;
    m_bEnableSkidding = false;

    StopLoopSound();
    // play the stop sound
    Sound(m_sSoundSet + "snd_stop", CHAN_VOICE);

    delete m_pCurPath;
    m_pCurPath  = NULL;
    m_iCurNode  = 0;
    turnimpulse = 0;

    //
    // notify scripts that driving has stopped
    //
    Unregister(STRING_DRIVE);
}

/*
====================
Vehicle::EventDriveInternal
====================
*/
void Vehicle::EventDriveInternal(Event *ev, bool wait)
{
    SimpleEntity *path;
    SimpleEntity *alternate_path = NULL;

    m_fIdealDistance = 100;
    m_fLookAhead     = 256;
    m_fIdealAccel    = 35;
    m_fIdealSpeed    = 250;
    m_fMaxSpeed      = 250;

    switch (ev->NumArgs()) {
    case 6:
        alternate_path = ev->GetSimpleEntity(6);
    case 5:
        m_fLookAhead = ev->GetFloat(5);
    case 4:
        m_fIdealDistance = ev->GetFloat(4);
    case 3:
        m_fIdealAccel = ev->GetFloat(3);
    case 2:
        m_fIdealSpeed = ev->GetFloat(2);
    case 1:
        path = ev->GetSimpleEntity(1);
        if (!path) {
            ScriptError("Vehicle Given Drive Command with NULL path.");
            return;
        }
        break;
    default:
        ScriptError("wrong number of arguments");
    }

    if (!m_pCurPath) {
        m_pCurPath = new cVehicleSpline;
    }

    if (!m_pAlternatePath) {
        m_pAlternatePath = new cVehicleSpline;
    }

    SetupPath(m_pCurPath, path);

    // Setup the alternate path
    if (alternate_path) {
        SetupPath(m_pAlternatePath, alternate_path);
    }

    m_bAutoPilot     = true;
    m_iCurNode       = 0;
    m_iAlternateNode = 0;

    Sound(m_sSoundSet + "snd_start");
}

/*
====================
Vehicle::SetupPath
====================
*/
void Vehicle::SetupPath(cVehicleSpline *pPath, SimpleEntity *se)
{
    Vector        vLastOrigin;
    SimpleEntity *ent;
    float         fCurLength = 0.0f;
    int           i          = 1;

    if (!pPath) {
        return;
    }

    pPath->Reset();

    if (!se->Target().length() || !se->Target()[0]) {
        return;
    }

    vLastOrigin = se->origin;

    for (ent = se; ent != NULL; ent = ent->Next(), i++) {
        Vector vDelta = vLastOrigin - ent->origin;
        float  vTmp[4];

        if (vDelta.length() == 0.0f && i > 1) {
            Com_Printf("^~^~^Warning: Vehicle Driving with a Path that contains 2 equal points\n");
        } else {
            fCurLength += vDelta.length();

            vTmp[0] = fCurLength;
            VectorCopy(ent->origin, vTmp + 1);

            if (ent->IsSubclassOfVehiclePoint()) {
                pPath->Add(vTmp, static_cast<VehiclePoint *>(ent)->spawnflags);
            } else {
                pPath->Add(vTmp, 0);
            }

            vLastOrigin = ent->origin;
        }

        if (ent == se && i > 1) {
            break;
        }
    }
}

/*
====================
Vehicle::EventDrive
====================
*/
void Vehicle::EventDrive(Event *ev)
{
    EventDriveInternal(ev, true);
}

/*
====================
Vehicle::EventDriveNoWait
====================
*/
void Vehicle::EventDriveNoWait(Event *ev)
{
    EventDriveInternal(ev, false);
}

/*
====================
Vehicle::EventStop
====================
*/
void Vehicle::EventStop(Event *ev)
{
    m_bStopEnabled = false;
    m_bAutoPilot   = false;
    m_bIsSkidding  = false;
    moveimpulse    = 0;
    turnimpulse    = 0;
    m_iCurNode     = 0;
    Unregister(STRING_DRIVE);
}

/*
====================
Vehicle::EventFullStop
====================
*/
void Vehicle::EventFullStop(Event *ev)
{
    m_bAutoPilot   = false;
    m_bStopEnabled = false;
    m_bIsSkidding  = false;
    moveimpulse    = 0;
    turnimpulse    = 0;
    velocity       = vec_zero;
    m_iCurNode     = 0;
    Unregister(STRING_DRIVE);
}

/*
====================
Vehicle::VehicleDestroyed
====================
*/
void Vehicle::VehicleDestroyed(Event *ev)
{
    Entity      *ent;
    Entity      *targetEnt;
    Event       *event;
    const char  *name;
    VehicleBase *v;
    Vector       vDelta;

    takedamage = DAMAGE_NO;
    setSolidType(SOLID_NOT);
    hideModel();

    ent = ev->GetEntity(1);
    if (driver.ent) {
        EntityPtr driverPtr;

        velocity  = vec_zero;
        driverPtr = driver.ent;

        event = new Event(EV_Use);
        event->AddEntity(driverPtr);
        ProcessEvent(event);

        vDelta = driverPtr->origin - origin;
        vDelta[2] += 64;
        vDelta.normalize();

        // kill the driver
        driverPtr->Damage(this, this, driverPtr->health * 2, origin, vDelta, vec_zero, 50, 0, MOD_VEHICLE);
    }

    if (flags & FL_DIE_EXPLODE) {
        setSolidType(SOLID_NOT);
        hideModel();
        CreateExplosion(origin, edict->s.scale * 150, this, this, this, m_sExplosionModel);
    }

    if (flags & FL_DIE_GIBS) {
        setSolidType(SOLID_NOT);
        hideModel();
        CreateGibs(this, -150, edict->s.scale, 3, NULL);
    }

    //
    // remove all links
    //
    for (v = this; v->vlink; v = vlink) {
        v->vlink->PostEvent(EV_Remove, EV_VEHICLE);
    }

    //
    // remove all kill targets
    //
    name = KillTarget();
    if (name && strcmp(name, "")) {
        for (targetEnt = G_FindTarget(NULL, name); targetEnt; targetEnt = G_FindTarget(targetEnt, name)) {
            targetEnt->PostEvent(EV_Remove, EV_VEHICLE);
        }
    }

    //
    // activate targets
    //
    name = Target();
    if (name && strcmp(name, "")) {
        for (targetEnt = G_FindTarget(NULL, name); targetEnt; targetEnt = G_FindTarget(targetEnt, name)) {
            event = new Event(EV_Activate);
            event->AddEntity(ent);
            targetEnt->ProcessEvent(event);
        }
    }

    PostEvent(EV_Remove, EV_VEHICLE);
}

/*
====================
Vehicle::DetachRemoteOwner
====================
*/
void Vehicle::DetachRemoteOwner()
{
    VehicleTurretGun *vtg = static_cast<VehicleTurretGun *>(Turrets[0].ent.Pointer());
    vtg->EndRemoteControl();
}

static float GetAngleBetweenVectors2D(const Vector& a, const Vector& b)
{
    float value;
    float angle;

    value = atan2f(a.x, a.y);
    angle = RAD2DEG(value - atan2f(b.x, b.y));
    if (fabs(angle) <= 180) {
        return angle;
    } else if (angle < 0) {
        return angle + 360;
    } else {
        return angle - 360;
    }
}

/*
====================
Vehicle::DriverUse
====================
*/
void Vehicle::DriverUse(Event *ev)
{
    int     slot;
    Vector  pos;
    Vector  dist;
    float   min_length = 1e30f;
    int     min_slot   = 0;
    int     min_type   = -1;
    Entity *ent;
    Vector  vExitAngles;
    bool    bHasExitAngles;
    Vector  vExitPosition;
    bool    bHasExitPosition;

    ent = ev->GetEntity(1);

    bHasExitAngles   = false;
    bHasExitPosition = false;

    if (locked) {
        return;
    }

    if (ev->NumArgs() == 2) {
        if (ev->IsVectorAt(2)) {
            vExitPosition    = ev->GetVector(2);
            bHasExitPosition = true;
        } else if (ev->IsEntityAt(2)) {
            Entity *pEnt = ev->GetEntity(2);

            bHasExitAngles   = true;
            vExitAngles      = pEnt->angles;
            vExitPosition    = pEnt->origin;
            bHasExitPosition = true;
        } else if (ev->IsSimpleEntityAt(2)) {
            SimpleEntity *pEnt = ev->GetSimpleEntity(2);

            bHasExitAngles   = true;
            vExitAngles      = pEnt->angles;
            vExitPosition    = pEnt->origin;
            bHasExitPosition = true;
        }
    }

    slot = FindDriverSlotByEntity(ent);

    if (slot >= 0) {
        DetachDriverSlot(slot, vec_zero, NULL);

        if (IsSubclassOfVehicleTank()) {
            // Added check to see if the turret is valid in OPM
            if (Turrets[0].ent && Turrets[0].ent->IsSubclassOfVehicleTurretGun()) {
                DetachRemoteOwner();
            }
        }

        return;
    }

    slot = FindPassengerSlotByEntity(ent);

    if (slot >= 0) {
        DetachPassengerSlot(slot, vec_zero, NULL);
        return;
    }

    if (ent->IsSubclassOfWeapon()) {
        slot = FindTurretSlotByEntity(ent);

        if (slot >= 0) {
            DetachTurretSlot(slot, vec_zero, NULL);
        }
    } else if (ent->IsSubclassOfSentient()) {
        Sentient  *sent       = static_cast<Sentient *>(ent);
        TurretGun *sentTurret = sent->GetTurret();

        if (sentTurret) {
            slot = FindTurretSlotByEntity(sent->GetTurret());

            if (slot >= 0) {
                if (bHasExitPosition) {
                    if (bHasExitAngles) {
                        AttachTurretSlot(slot, sentTurret, vExitPosition, &vExitAngles);
                    } else {
                        AttachTurretSlot(slot, sentTurret, vExitPosition, NULL);
                    }
                } else {
                    AttachTurretSlot(slot, sentTurret, vec_zero, NULL);
                }

                sent->SetVehicle(NULL);
                return;
            }
        }
    }

    // Check for passengers slots
    for (slot = 0; slot < MAX_PASSENGERS; slot++) {
        if (!(Passengers[0].flags & SLOT_FREE)) {
            continue;
        }

        QueryPassengerSlotPosition(slot, (float *)&pos);

        dist = pos - ent->origin;

        if (dist.length() < min_length) {
            min_length = dist.length();
            min_type   = 0;
            min_slot   = slot;
        }
    }

    // Check for turrets slots
    if (ent->IsSubclassOfWeapon()) {
        for (slot = 0; slot < MAX_TURRETS; slot++) {
            if (!(Turrets[0].flags & SLOT_FREE)) {
                continue;
            }

            QueryTurretSlotPosition(slot, (float *)&pos);

            dist = pos - ent->origin;

            if (dist.length() < min_length) {
                min_length = dist.length();
                min_type   = 1;
                min_slot   = slot;
            }
        }
    } else {
        for (slot = 0; slot < MAX_TURRETS; slot++) {
            if (!(Turrets[0].flags & SLOT_BUSY)) {
                continue;
            }

            QueryTurretSlotPosition(slot, (float *)&pos);

            dist = pos - ent->origin;

            if (dist.length() < min_length) {
                min_length = dist.length();
                min_type   = 1;
                min_slot   = slot;
            }
        }
    }

    slot = 0;

    // Check for driver(s) slot(s)
    if (driver.flags & SLOT_FREE) {
        float length;

        QueryDriverSlotPosition(slot, (float *)&pos);

        dist = pos - ent->origin;

        length = dist.length();
        if (ent->IsSubclassOfPlayer()) {
            length /= 2.0;
        }

        if (length < min_length) {
            min_length = dist.length();
            min_type   = 2;
            min_slot   = slot;
        }
    }

    if (g_gametype->integer == GT_SINGLE_PLAYER || min_type == 2) {
        switch (min_type) {
        case 0:
            AttachPassengerSlot(min_slot, ent, vec_zero);
            break;
        case 1:
            AttachTurretSlot(min_slot, ent, vec_zero, NULL);
            break;
        case 2:
            AttachDriverSlot(min_slot, ent, vec_zero);
            break;
        }
    }
}

/*
====================
Vehicle::SetMoveInfo
====================
*/
void Vehicle::SetMoveInfo(vmove_t *vm)
{
    memset(vm, 0, sizeof(vmove_t));

    VectorCopy(origin, vs.origin);
    vs.useGravity = false;
    vs.entityNum  = entnum;

    vm->vs        = &vs;
    vm->frametime = level.frametime;
    vm->tracemask = edict->clipmask;
    VectorCopy(mins, vm->mins);
    VectorCopy(maxs, vm->maxs);

    vs.entityNum      = edict->s.number;
    vs.desired_dir[0] = velocity[0];
    vs.desired_dir[1] = velocity[1];

    vm->desired_speed = VectorNormalize2D(vs.desired_dir);
}

/*
====================
Vehicle::GetMoveInfo
====================
*/
void Vehicle::GetMoveInfo(vmove_t *vm)
{
    Vector newOrigin = vm->vs->origin;

    if (bindmaster) {
        newOrigin = vm->vs->origin - origin;
    }

    setLocalOrigin(newOrigin);

    groundentity = NULL;
    if (vm->vs->groundEntityNum != ENTITYNUM_NONE) {
        groundentity = &g_entities[vm->vs->groundEntityNum];
    }
}

/*
====================
Vehicle::SetCEMoveInfo
====================
*/
void Vehicle::SetCEMoveInfo(vmove_t *vm)
{
    SetMoveInfo(vm);

    vm->mins[0] = m_pCollisionEntity->mins[0] - 24.0f;
    vm->mins[1] = m_pCollisionEntity->mins[1] - 24.0f;
    vm->mins[2] = m_pCollisionEntity->mins[2];
    vm->maxs[0] = m_pCollisionEntity->maxs[0] + 24.0f;
    vm->maxs[1] = m_pCollisionEntity->maxs[1] + 24.0f;
    vm->maxs[2] = m_pCollisionEntity->maxs[2];
}

/*
====================
Vehicle::GetCEMoveInfo
====================
*/
void Vehicle::GetCEMoveInfo(vmove_t *vm)
{
    GetMoveInfo(vm);
}

/*
====================
Vehicle::SetViewAngles
====================
*/
void Vehicle::SetViewAngles(Vector newViewangles)
{
    client->ps.delta_angles[0] = floor(ANGLE2SHORT(newViewangles.x));
    client->ps.delta_angles[1] = floor(ANGLE2SHORT(newViewangles.y));
    client->ps.delta_angles[2] = floor(ANGLE2SHORT(newViewangles.z));

    v_angle = newViewangles;
    AnglesToAxis(v_angle, orientation);

    yaw_forward = orientation[0];
    yaw_left    = orientation[1];
}

/*
====================
Vehicle::SetSlotsNonSolid
====================
*/
void Vehicle::SetSlotsNonSolid(void)
{
    for (int i = 0; i < MAX_PASSENGERS; i++) {
        Passengers[i].NotSolid();
    }

    for (int i = 0; i < MAX_TURRETS; i++) {
        Turrets[i].NotSolid();
    }

    driver.NotSolid();

    if (m_pCollisionEntity) {
        m_pCollisionEntity->NotSolid();
    }
}

/*
====================
Vehicle::SetSlotsSolid
====================
*/
void Vehicle::SetSlotsSolid(void)
{
    for (int i = 0; i < MAX_PASSENGERS; i++) {
        Passengers[i].Solid();
    }

    for (int i = 0; i < MAX_TURRETS; i++) {
        Turrets[i].Solid();
    }

    driver.Solid();

    if (m_pCollisionEntity) {
        m_pCollisionEntity->Solid();
    }
}

/*
====================
Vehicle::AssertMove
====================
*/
bool Vehicle::AssertMove(Vector vNewOrigin, Vector vOldOrigin)
{
    Entity    *check;
    gentity_t *edict;
    int        touch[MAX_GENTITIES];
    int        i;
    int        num;

    if (m_pCollisionEntity) {
        num = gi.AreaEntities(m_pCollisionEntity->mins, m_pCollisionEntity->maxs, touch, MAX_GENTITIES);
    } else {
        num = gi.AreaEntities(mins, maxs, touch, MAX_GENTITIES);
    }

    if (num <= 0) {
        return true;
    }

    for (i = 0; i < num; i++) {
        edict = &g_entities[touch[i]];
        check = edict->entity;

        if (check->edict->s.number != edict->s.number && edict->solid && check->movetype != MOVETYPE_STOP) {
            if (check->movetype != MOVETYPE_NONE && check->movetype != MOVETYPE_NOCLIP
                && edict->r.contents != CONTENTS_PLAYERCLIP && IsTouching(check)
                && G_TestEntityPosition(check, check->origin)) {
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
bool Vehicle::AssertRotation(Vector vNewAngles, Vector vOldAngles)
{
    Vector i;
    Vector j;
    Vector k;
    Vector i2;
    Vector j2;
    Vector k2;
    Vector vAngleDiff;
    float  mAngleDiff[3][3];

    AngleVectorsLeft(vOldAngles, i, j, k);
    AngleVectorsLeft(vNewAngles, i2, j2, k2);
    AnglesSubtract(vOldAngles, vNewAngles, vAngleDiff);
    AngleVectorsLeft(vAngleDiff, mAngleDiff[0], mAngleDiff[1], mAngleDiff[2]);

    // FIXME: shrug

    return true;
}

/*
====================
Vehicle::NoMove
====================
*/
void Vehicle::NoMove(void)
{
    vmove_t vm;

    SetMoveInfo(&vm);
    VectorClear2D(vs.desired_dir);
    VmoveSingle(&vm);
    GetMoveInfo(&vm);
}

/*
====================
Vehicle::SlidePush
====================
*/
void Vehicle::SlidePush(Vector vPush)
{
    vmove_t    vm;
    int        i;
    int        j;
    Entity    *pSkippedEntities[MAX_SKIPPED_ENTITIES];
    int        iContentsEntities[MAX_SKIPPED_ENTITIES];
    solid_t    solidEntities[MAX_SKIPPED_ENTITIES];
    int        iNumSkippedEntities = 0;
    int        iNumSkipped         = 0;
    gentity_t *other;
    Vector     newOrigin;

    do {
        SetMoveInfo(&vm);

        vPush.copyTo(vm.vs->velocity);
        vm.frametime = 1;

        vs.desired_dir[0] = vPush[0];
        vs.desired_dir[1] = vPush[1];
        vm.desired_speed  = VectorNormalize2D(vs.desired_dir);

        if (g_showvehiclemovedebug->integer) {
            G_DebugBBox(origin, vm.mins, vm.maxs, 1, 0, 0, 1);
            G_DebugBBox(origin, vm.mins, vm.maxs, 0, 1, 0, 1);
        }

        VmoveSingle(&vm);

        iNumSkippedEntities = 0;

        for (i = 0; i < vm.numtouch; i++) {
            other = &g_entities[vm.touchents[i]];

            for (j = 0; j < i; j++) {
                if (&g_entities[j] == other) {
                    break;
                }
            }

            if (j == i && other->entity) {
                other->entity->CheckGround();

                if (other->entity->groundentity
                    && (other->entity->groundentity == edict || other->entity == m_pCollisionEntity)) {
                    // save the entity
                    pSkippedEntities[iNumSkipped]  = other->entity;
                    iContentsEntities[iNumSkipped] = other->r.contents;
                    solidEntities[iNumSkipped]     = other->solid;
                    iNumSkipped++;

                    if (iNumSkipped >= MAX_SKIPPED_ENTITIES) {
                        gi.Error(ERR_DROP, "MAX_SKIPPED_ENTITIES hit in VehicleMove.\n");
                        return;
                    }

                    other->entity->setSolidType(SOLID_NOT);
                    iNumSkippedEntities++;
                }

                if (g_showvehiclemovedebug->integer) {
                    Com_Printf(
                        "Vehicle Hit(SP): %s : %s\n", other->entity->getClassname(), other->entity->targetname.c_str()
                    );
                }
            }
        }
    } while (iNumSkippedEntities != 0);

    newOrigin = vm.vs->origin;
    if (bindmaster) {
        newOrigin -= bindmaster->origin;
    }

    setLocalOrigin(newOrigin);

    if (g_showvehiclemovedebug->integer) {
        G_DebugBBox(origin, vm.mins, vm.maxs, 0, 0, 1, 1);
    }

    TouchStuff(&vm);

    for (i = 0; i < iNumSkipped; i++) {
        pSkippedEntities[i]->setSolidType(solidEntities[i]);
        pSkippedEntities[i]->setContents(iContentsEntities[i]);
    }
}

/*
====================
Vehicle::AnimMoveVehicle
====================
*/
void Vehicle::AnimMoveVehicle(void)
{
    Vector  vPosition;
    Vector  vAngles;
    Entity *ent;

    //
    // velocity
    //
    vPosition = origin + frame_delta;
    velocity  = (vPosition - origin) * (1.0 / level.frametime);

    setOrigin(vPosition);

    //
    // angular velocity
    //
    vAngles = angles;
    vAngles[1] += angular_delta;
    avelocity = (vAngles - angles) * (1.0 / level.frametime);

    setAngles(vAngles);

    for (ent = teamchain; ent; ent = ent->teamchain) {
        ent->setLocalOrigin(ent->localorigin);
        ent->setAngles(ent->localangles);
    }
}

/*
====================
Vehicle::MoveVehicle
====================
*/
void Vehicle::MoveVehicle(void)
{
    trace_t      tr;
    Vector       vecStart;
    Vector       vecStart2;
    Vector       vecEnd;
    Vector       vecDelta;
    Vector       vecAng;
    int          i;
    gridpoint_t *gp;
    vmove_t      vm;
    float        flMoveFrac = 1;
    float        fSpeed;
    bool         bDoPush    = true;
    bool         bDoGravity = false;
    bool         bHitPerson = false;
    Entity      *pSkippedEntities[MAX_SKIPPED_ENTITIES];
    int          iContentsEntities[MAX_SKIPPED_ENTITIES];
    solid_t      solidEntities[MAX_SKIPPED_ENTITIES];
    int          iNumSkippedEntities = 0;
    Event       *event               = nullptr;
    Entity      *chain;

    if (m_bMovementLocked) {
        return;
    }

    setAngles();
    if (m_pCollisionEntity) {
        SetCEMoveInfo(&vm);
    } else {
        SetMoveInfo(&vm);
    }
    m_sMoveGrid->SetMoveInfo(&vm);
    m_sMoveGrid->SetOrientation(orientation);
    m_sMoveGrid->CalculateBoxPoints();
    CheckGround();

    if (velocity.length() > 0.5f) {
        Vector vVel;

        fSpeed   = orientation[0] * velocity;
        vecDelta = velocity * level.frametime;

        vVel        = velocity * level.frametime * 8;
        vecDelta[2] = 0;

        for (i = 0; i < 3 && bDoPush; i++) {
            bool bContinue = false;

            if (fSpeed > 0.0f) {
                gp = m_sMoveGrid->GetGridPoint(2, i, 0);
            } else {
                gp = m_sMoveGrid->GetGridPoint(0, i, 0);
            }
            vecStart = origin + gp->origin;

            if (real_velocity.length() > 0.5f) {
                vecEnd = vecStart + vVel;

                for (;;) {
                    tr = G_Trace(
                        vecEnd,
                        Vector(gp->vm.mins) - Vector(-32, -32, -32),
                        Vector(gp->vm.maxs) + Vector(32, 32, 32),
                        vecEnd,
                        this,
                        MASK_MOVEVEHICLE,
                        false,
                        "Vehicle::MoveVehicle"
                    );

                    if (!tr.ent || !tr.ent->entity || tr.ent->entity == world) {
                        break;
                    }

                    tr.ent->entity->CheckGround();

                    if (!tr.ent->entity->groundentity
                        || (tr.ent->entity->groundentity == edict
                            && (!m_pCollisionEntity || tr.ent->entity != m_pCollisionEntity))) {
                        Event *event = new Event(EV_Touch);
                        event->AddEntity(this);
                        tr.ent->entity->ProcessEvent(event);

                        event = new Event(EV_Touch);
                        event->AddEntity(tr.ent->entity);
                        ProcessEvent(event);

                        if (tr.ent->entity->IsSubclassOfSentient()) {
                            bHitPerson = true;
                        }

                        if (g_showvehiclemovedebug->integer) {
                            Com_Printf(
                                "Vehicle Hit(MV0): %s : %s\n",
                                tr.ent->entity->getClassname(),
                                tr.ent->entity->targetname.c_str()
                            );
                        }

                        break;
                    }

                    pSkippedEntities[iNumSkippedEntities]  = tr.ent->entity;
                    iContentsEntities[iNumSkippedEntities] = tr.ent->r.contents;
                    solidEntities[iNumSkippedEntities]     = tr.ent->solid;
                    iNumSkippedEntities++;

                    if (iNumSkippedEntities >= MAX_SKIPPED_ENTITIES) {
                        gi.Error(ERR_DROP, "MAX_SKIPPED_ENTITIES hit in VehicleMove.\n");
                        return;
                    }

                    tr.ent->entity->setSolidType(SOLID_NOT);

                    if (g_showvehiclemovedebug->integer) {
                        Com_Printf(
                            "Vehicle Skipped(MV0): %s : %s\n",
                            tr.ent->entity->getClassname(),
                            tr.ent->entity->targetname.c_str()
                        );
                    }
                }
            }

            if (g_showvehiclemovedebug->integer) {
                G_DebugBBox(vecStart, gp->vm.mins, gp->vm.maxs, 1, 0, 0, 1);
            }

            vecStart.z += 64;
            if (g_showvehiclemovedebug->integer) {
                G_DebugBBox(vecStart, gp->vm.mins, gp->vm.maxs, 1, 0, 0, 1);
            }

            vecEnd = vecStart + vecDelta;

            for (;;) {
                tr = G_Trace(
                    vecStart,
                    Vector(gp->vm.mins),
                    Vector(gp->vm.maxs),
                    vecEnd,
                    this,
                    edict->clipmask,
                    false,
                    "Vehicle::MoveVehicle"
                );

                if (tr.fraction == 1.0f && !tr.allsolid && !tr.startsolid) {
                    bContinue = true;
                    break;
                }

                if (!tr.ent || !tr.ent->entity || tr.ent->entity == world) {
                    break;
                }

                tr.ent->entity->CheckGround();

                if (!tr.ent->entity->groundentity
                    || (tr.ent->entity->groundentity == edict
                        && (!m_pCollisionEntity || tr.ent->entity != m_pCollisionEntity))) {
                    Event *event = new Event(EV_Touch);
                    event->AddEntity(this);
                    tr.ent->entity->ProcessEvent(event);

                    event = new Event(EV_Touch);
                    event->AddEntity(tr.ent->entity);
                    ProcessEvent(event);

                    if (tr.ent->entity->IsSubclassOfSentient()) {
                        bHitPerson = true;
                    }

                    if (g_showvehiclemovedebug->integer) {
                        Com_Printf(
                            "Vehicle Hit(MV): %s : %s\n",
                            tr.ent->entity->getClassname(),
                            tr.ent->entity->TargetName().c_str()
                        );
                    }

                    break;
                }

                pSkippedEntities[iNumSkippedEntities]  = tr.ent->entity;
                iContentsEntities[iNumSkippedEntities] = tr.ent->r.contents;
                solidEntities[iNumSkippedEntities]     = tr.ent->solid;
                iNumSkippedEntities++;

                if (iNumSkippedEntities >= MAX_SKIPPED_ENTITIES) {
                    gi.Error(ERR_DROP, "MAX_SKIPPED_ENTITIES hit in VehicleMove.\n");
                    return;
                }

                tr.ent->entity->setSolidType(SOLID_NOT);

                if (g_showvehiclemovedebug->integer) {
                    Com_Printf(
                        "Vehicle Skipped(MV): %s : %s\n",
                        tr.ent->entity->getClassname(),
                        tr.ent->entity->TargetName().c_str()
                    );
                }
            }

            if (bContinue) {
                continue;
            }

            if (flMoveFrac > tr.fraction) {
                flMoveFrac = tr.fraction - 0.1f;
            }

            vecStart = tr.endpos;
            vecEnd   = vecStart;
            vecEnd.z -= 64;

            if (g_showvehiclemovedebug->integer) {
                G_DebugBBox(tr.endpos, gp->vm.mins, gp->vm.maxs, 0, 1, 0, 1);
            }

            for (;;) {
                tr = G_Trace(
                    vecStart,
                    Vector(gp->vm.mins),
                    Vector(gp->vm.maxs),
                    vecEnd,
                    this,
                    edict->clipmask,
                    false,
                    "Vehicle::MoveVehicle"
                );

                if (tr.fraction != 1.0f || tr.plane.normal[2] < 0.7f || tr.allsolid) {
                    flMoveFrac = 0.0f;
                }

                if (tr.fraction == 1.0f && !tr.allsolid && !tr.startsolid) {
                    break;
                }

                if (!tr.ent || !tr.ent->entity || tr.ent->entity == world) {
                    break;
                }

                tr.ent->entity->CheckGround();

                if (!tr.ent->entity->groundentity
                    || (tr.ent->entity->groundentity == edict
                        && (!m_pCollisionEntity || tr.ent->entity != m_pCollisionEntity))) {
                    Event *event = new Event(EV_Touch);
                    event->AddEntity(this);
                    tr.ent->entity->ProcessEvent(event);

                    event = new Event(EV_Touch);
                    event->AddEntity(tr.ent->entity);
                    ProcessEvent(event);

                    if (tr.ent->entity->IsSubclassOfSentient()) {
                        bHitPerson = true;
                    }

                    if (g_showvehiclemovedebug->integer) {
                        Com_Printf(
                            "Vehicle Hit(MV2): %s : %s\n",
                            tr.ent->entity->getClassname(),
                            tr.ent->entity->TargetName().c_str()
                        );
                    }

                    break;
                }

                pSkippedEntities[iNumSkippedEntities]  = tr.ent->entity;
                iContentsEntities[iNumSkippedEntities] = tr.ent->r.contents;
                solidEntities[iNumSkippedEntities]     = tr.ent->solid;
                iNumSkippedEntities++;

                if (iNumSkippedEntities >= MAX_SKIPPED_ENTITIES) {
                    gi.Error(ERR_DROP, "MAX_SKIPPED_ENTITIES hit in VehicleMove.\n");
                    return;
                }

                tr.ent->entity->setSolidType(SOLID_NOT);
                if (g_showvehiclemovedebug->integer) {
                    Com_Printf(
                        "Vehicle Skipped(MV2): %s : %s\n",
                        tr.ent->entity->getClassname(),
                        tr.ent->entity->targetname.c_str()
                    );
                }
            }

            if (g_showvehiclemovedebug->integer) {
                G_DebugBBox(tr.endpos, gp->vm.mins, gp->vm.maxs, 0, 0, 1, 1);
            }

            if ((!tr.ent || !tr.ent->entity->IsSubclassOfProjectile()) && driver.ent
                && driver.ent->IsSubclassOfPlayer()) {
                if (fSpeed > 0) {
                    if (i) {
                        if (i == 2 && turnimpulse >= 0.0f) {
                            turnimpulse -= level.frametime * 800;
                        }
                    } else if (turnimpulse <= 0) {
                        turnimpulse += level.frametime * 800;
                    }
                } else {
                    if (i) {
                        if (i == 2 && turnimpulse <= 0.0f) {
                            turnimpulse += level.frametime * 800;
                        }
                    } else if (turnimpulse >= 0) {
                        turnimpulse -= level.frametime * 800;
                    }
                }
            }

            if (flMoveFrac < 0.1f) {
                bDoPush = false;
            }
        }

        if (bDoPush) {
            SlidePush(Vector(0, 0, 64));
            SlidePush(vecDelta * flMoveFrac);
            SlidePush(Vector(0, 0, -128));

            bDoGravity = true;
            if (!bHitPerson) {
                velocity *= flMoveFrac;
            }
        } else if (!bHitPerson) {
            velocity *= 0.5;
        }
    }

    if (bDoGravity) {
        tr = G_Trace(
            origin,
            mins,
            maxs,
            origin + Vector(0, 0, velocity[2] * level.frametime),
            this,
            edict->clipmask,
            qtrue,
            "Vehicle::MoveVehicle"
        );

        if (tr.fraction < 1) {
            velocity[2] = 0;
        }

        setOrigin(tr.endpos);
    }

    for (i = 0; i < iNumSkippedEntities; i++) {
        pSkippedEntities[i]->setSolidType(solidEntities[i]);
        pSkippedEntities[i]->setContents(iContentsEntities[i]);
    }

    for (chain = teamchain; chain; chain = chain->teamchain) {
        chain->setLocalOrigin(chain->localorigin);
        chain->setAngles(chain->localangles);
    }
}

/*
====================
Vehicle::Think
====================
*/
void Vehicle::Think(void)
{
    flags |= FL_POSTTHINK;
}

/*
====================
Vehicle::Postthink
====================
*/
void Vehicle::Postthink(void)
{
    float         turn;
    Vector        i;
    Vector        j;
    Vector        k;
    Vector        temp;
    Vector        roll;
    Vector        acceleration;
    Vector        atmp;
    Vector        atmp2;
    Vector        aup;
    VehicleBase  *v;
    VehicleBase  *last;
    float         drivespeed;
    float         dot;
    Vector        primal_angles = angles;
    Vector        primal_origin = origin;
    Vector        vTmp;
    Vector        vAddedAngles;
    Vector        n_angles;
    orientation_t orient;

    if (!g_vehicle->integer || (edict->s.renderfx & RF_DONTDRAW)) {
        return;
    }

    if (m_pCollisionEntity) {
        setSolidType(SOLID_NOT);
        m_pCollisionEntity->Solid();
    }

    prev_velocity = velocity;

    if (m_vOldMins != vec_zero && m_vOldMaxs != vec_zero) {
        mins = m_vOldMins;
        maxs = m_vOldMaxs;
    } else if (mins != vec_zero || maxs != vec_zero) {
        m_vOldMins = mins;
        m_vOldMaxs = maxs;
    }

    SetSlotsNonSolid();
    if (m_bAnimMove) { // 2.0: use vehicle's animation to move
        AnimMoveVehicle();
    } else {
        if (!m_bMovementLocked) {
            FactorOutAnglesOffset();
            FactorOutOriginOffset();
        }
        MoveVehicle();
    }

    SetSlotsSolid();

    m_bThinkCalled = true;

    if (m_bAnimMove) {
        moveimpulse = velocity.length() * level.frametime;
        turnimpulse = avelocity[1] * level.frametime;
    } else if (m_bAutoPilot) {
        AutoPilot();
    } else {
        if (!driver.ent || !driver.ent->IsSubclassOfPlayer()) {
            acceleration = velocity;

            if (acceleration.length() < 0.1f) {
                velocity = vec_zero;
            }
        }

        moveimpulse *= 0.825f;
        turnimpulse *= 0.825f;
    }

    currentspeed = moveimpulse;
    turnangle    = turnangle * 0.25f + turnimpulse;
    turnangle    = Q_clamp_float(turnangle, -maxturnrate, maxturnrate);

    if (level.inttime <= 1200) {
        prev_origin = origin;
    } else {
        real_velocity     = origin - prev_origin;
        prev_origin       = origin;
        prev_acceleration = real_acceleration;
        real_acceleration = real_velocity - prev_velocity;
        prev_velocity     = real_velocity;
        acceleration      = real_acceleration - prev_acceleration;
    }

    UpdateSkidAngle();
    UpdateBones();
    UpdateShaderOffset();
    UpdateTires();
    UpdateNormals();

    angles.AngleVectorsLeft(&i, &j, &k);

    turn = level.frametime * turnangle;

    velocity[0] *= 0.925f;
    velocity[1] *= 0.925f;

    velocity = Vector(orientation[0]) * currentspeed;
    temp     = orientation[0];
    temp.z   = 0;

    drivespeed = velocity * temp;
    drivespeed = Q_clamp_float(drivespeed, -speed, speed);
    n_angles   = temp * drivespeed;

    velocity   = n_angles;
    velocity.z = drivespeed * jumpimpulse;
    avelocity *= 0.05f;

    if (steerinplace && drivespeed < 350.0f) {
        drivespeed = 350.0f;
    }

    avelocity.y += turn * drivespeed;
    angles += avelocity * level.frametime;

    dot = acceleration * orientation[0];

    UpdateSound();

    i = orientation[0];
    j = orientation[1];
    k = orientation[2];

    CalculateAnglesOffset(acceleration);
    CalculateOriginOffset();

    last_origin = origin;
    vTmp        = (angles - primal_angles) * level.frametime;

    if (vTmp.x > 180 || vTmp.x < -180) {
        vTmp.x = 0;
    }
    if (vTmp.y > 180 || vTmp.y < -180) {
        vTmp.y = 0;
    }
    if (vTmp.z > 180 || vTmp.z < -180) {
        vTmp.z = 0;
    }

    if (vTmp.x > -1 || vTmp.x < 1) {
        vTmp.x = 0;
    }
    if (vTmp.y > -1 || vTmp.y < 1) {
        vTmp.y = 0;
    }
    if (vTmp.z > -1 || vTmp.z < 1) {
        vTmp.z = 0;
    }

    avelocity = vAddedAngles;

    if (!m_bAnimMove && !m_bMovementLocked) {
        FactorInOriginOffset();
        FactorInAnglesOffset(&vAddedAngles);
    }

    n_angles = m_vOriginOffset + m_vOriginOffset2;
    CalculateAnimationData(vAddedAngles, m_vOriginOffset + m_vOriginOffset2);

    if (m_pCollisionEntity) {
        Vector vaDelta;

        SetSlotsNonSolid();

        vaDelta[0] = angledist(angles[0] - m_pCollisionEntity->angles[0]);
        vaDelta[1] = angledist(angles[1] - m_pCollisionEntity->angles[1]);
        vaDelta[2] = angledist(angles[2] - m_pCollisionEntity->angles[2]);

        m_pCollisionEntity->Solid();

        G_PushMove(m_pCollisionEntity, origin - primal_origin, vaDelta);
        G_TouchTriggers(m_pCollisionEntity);

        SetSlotsSolid();

        m_pCollisionEntity->setOrigin(origin);
        m_pCollisionEntity->setAngles(angles);
    }

    SetupVehicleSoundEntities();
    UpdateDriverSlot(0);

    for (int slot = 0; slot < MAX_PASSENGERS; slot++) {
        UpdatePassengerSlot(slot);
    }

    atmp = angles - primal_angles;

    for (int slot = 0; slot < MAX_TURRETS; slot++) {
        UpdateTurretSlot(slot);
    }

    if (g_showvehicleentrypoints->integer) {
        for (int slot = 0; slot < MAX_PASSENGERS; slot++) {
            if (Passengers[slot].enter_boneindex >= 0) {
                QueryPassengerSlotPosition(slot, (float *)&temp);

                G_DebugCircle(temp, 10, 1, 0, 0, 1, true);
            }
        }

        for (int slot = 0; slot < MAX_TURRETS; slot++) {
            if (Turrets[slot].enter_boneindex >= 0) {
                QueryTurretSlotPosition(slot, (float *)&temp);

                G_DebugCircle(temp, 10, 0, 1, 0, 1, true);
            }
        }

        if (driver.enter_boneindex >= 0) {
            QueryDriverSlotPosition(0, (float *)&temp);

            G_DebugCircle(temp, 10, 0, 0, 1, 1, true);
        }
    }

    if (g_showvehicleslotpoints->integer) {
        for (int slot = 0; slot < MAX_PASSENGERS; slot++) {
            if (Passengers[slot].boneindex >= 0) {
                GetTagPositionAndOrientation(Passengers[slot].boneindex, &orient);
                G_DebugCircle(orient.origin, 10.0, 1.0f, 0.5f, 0.5f, 1.0f, true);

                G_DebugLine(orient.origin, orient.origin + Vector(orient.axis[0]) * 32, 1, 0, 0, 1);
                G_DebugLine(orient.origin, orient.origin + Vector(orient.axis[1]) * 32, 0, 1, 0, 1);
                G_DebugLine(orient.origin, orient.origin + Vector(orient.axis[2]) * 32, 0, 0, 1, 1);
            }
        }

        for (int slot = 0; slot < MAX_TURRETS; slot++) {
            if (Turrets[slot].boneindex >= 0) {
                GetTagPositionAndOrientation(Turrets[slot].boneindex, &orient);
                G_DebugCircle(orient.origin, 10.0, 0.5f, 1.0f, 1.0f, 1.0f, true);

                G_DebugLine(orient.origin, orient.origin + Vector(orient.axis[0]) * 32, 1, 0, 0, 1);
                G_DebugLine(orient.origin, orient.origin + Vector(orient.axis[1]) * 32, 0, 1, 0, 1);
                G_DebugLine(orient.origin, orient.origin + Vector(orient.axis[2]) * 32, 0, 0, 1, 1);
            }
        }

        if (driver.boneindex >= 0) {
            GetTagPositionAndOrientation(driver.boneindex, &orient);
            G_DebugCircle(orient.origin, 10.0, 0.5f, 0.5f, 1.0f, 1.0f, true);

            G_DebugLine(orient.origin, orient.origin + Vector(orient.axis[0]) * 32, 1, 0, 0, 1);
            G_DebugLine(orient.origin, orient.origin + Vector(orient.axis[1]) * 32, 0, 1, 0, 1);
            G_DebugLine(orient.origin, orient.origin + Vector(orient.axis[2]) * 32, 0, 0, 1, 1);
        }
    }

    if (g_showvehicletags->integer) {
        int numtags;
        int tagnum;

        numtags = gi.TIKI_NumTags(edict->tiki);
        for (tagnum = 0; tagnum < numtags; tagnum++) {
            const char *name = gi.Tag_NameForNum(edict->tiki, tagnum);
            if (!strncmp(name, "tag", 3)) {
                GetTagPositionAndOrientation(tagnum, &orient);
                G_DebugCircle(orient.origin, 10.0, 1, 1, 1, 1, true);

                G_DebugLine(orient.origin, orient.origin + Vector(orient.axis[0]) * 32, 1, 0, 0, 1);
                G_DebugLine(orient.origin, orient.origin + Vector(orient.axis[1]) * 32, 0, 1, 0, 1);
                G_DebugLine(orient.origin, orient.origin + Vector(orient.axis[2]) * 32, 0, 0, 1, 1);
            }
        }
    }

    for (last = this; last->vlink; last = v) {
        v = last->vlink;

        v->setOrigin(origin + i * v->offset.x + j * v->offset.y + k * v->offset.z);
        v->avelocity = avelocity;
        v->velocity  = velocity;

        v->angles[ROLL]  = angles[ROLL];
        v->angles[YAW]   = angles[YAW];
        v->angles[PITCH] = (float)((int)(v->angles[PITCH] + drivespeed / 4.f) % 360);
        v->setAngles(v->angles);
    }

    CheckWater();
    WorldEffects();

    if (m_pCollisionEntity) {
        setSolidType(SOLID_NOT);
        m_pCollisionEntity->Solid();
    } else {
        setSolidType(SOLID_BBOX);
        setContents(CONTENTS_UNKNOWN2);
    }
}

/*
====================
Vehicle::VehicleTouched
====================
*/
void Vehicle::VehicleTouched(Event *ev)
{
    Entity *other;
    float   dot;
    float   speed;
    Vector  delta;
    Vector  dir;
    Event  *event;

    other = ev->GetEntity(1);
    if (other == driver.ent) {
        return;
    }

    if (other == world) {
        return;
    }

    if (drivable && !driver.ent) {
        return;
    }

    delta = other->origin - last_origin;
    delta.normalize();

    dot = velocity * orientation[0];
    if (dot > 0) {
        if (delta * orientation[0] < 0) {
            return;
        }
    } else {
        if (delta * orientation[0] > 0) {
            return;
        }
    }

    if (!other->IsSubclassOfVehicleTurretGun()) {
        speed = velocity.length();
        if (speed > 10) {
            Sound(m_sSoundSet + "vehicle_crash");
            dir = delta * (1 / speed);

            event = new Event(EV_Damage);
            if (lastdriver.ent && lastdriver.ent->IsSubclassOfPlayer()) {
                event->AddEntity(lastdriver.ent);
            } else {
                event->AddEntity(world);
            }

            event->AddFloat(speed);
            event->AddEntity(this);
            event->AddVector(origin);
            event->AddVector(dir);
            event->AddVector(vec_zero);
            event->AddFloat(speed);
            event->AddInteger(0);
            event->AddInteger(MOD_VEHICLE);
            event->AddInteger(-1);
            other->PostEvent(event, 0);
        }
    }
}

void Vehicle::SetProjectileVulnerable(Event *ev)
{
    if (ev->NumArgs() > 1) {
        ScriptError("Too many arguments");
    }

    if (ev->NumArgs() > 0) {
        m_iProjectileHitsRemaining = ev->GetInteger(1);
        if (m_iProjectileHitsRemaining < 0) {
            ScriptError("Negative arguments illegal");
        }
    } else {
        m_iProjectileHitsRemaining = 0;
    }
}

void Vehicle::DoProjectileVulnerability(Entity *pProjectile, Entity *pOwner, meansOfDeath_t meansOfDeath)
{
    Event *event;
    Vector delta;

    if (m_iProjectileHitsRemaining > 1) {
        m_iProjectileHitsRemaining--;
        return;
    }

    event = new Event(EV_Damage);
    delta = origin - pProjectile->origin;

    event->AddEntity(pProjectile);
    event->AddFloat(health * 2);
    event->AddEntity(pOwner);
    event->AddVector(origin);
    event->AddVector(delta);
    event->AddVector(vec_zero);
    event->AddFloat(0);
    event->AddInteger(0);
    event->AddInteger(meansOfDeath);
    event->AddInteger(-1);
    PostEvent(event, 0);
}

/*
====================
Vehicle::VehicleBlocked
====================
*/
void Vehicle::VehicleBlocked(Event *ev)
{
    return;
    /*
    Entity* other;
    float		speed;
    float    damage;
    Vector   delta;
    Vector   newvel;
    Vector	dir;

    if (!velocity[0] && !velocity[1])
        return;

    other = ev->GetEntity(1);
    if (other == driver.ent)
    {
        return;
    }
    if (other->isSubclassOf(VehicleBase))
    {
        delta = other->origin - origin;
        delta.normalize();

        newvel = vec_zero - (velocity)+(other->velocity * 0.25);
        if (newvel * delta < 0)
        {
            velocity = newvel;
            delta = velocity - other->velocity;
            damage = delta.length() / 4;
        }
        else
        {
            return;
        }
    }
    else if ((velocity.length() < 350))
    {
        other->velocity += velocity * 1.25f;
        other->velocity[2] += 100;
        damage = velocity.length() / 4;
    }
    else
    {
        damage = other->health + 1000;
    }

    // Gib 'em outright
    speed = fabs(velocity.length());
    dir = velocity * (1 / speed);
    other->Damage(this, lastdriver.ent, damage, origin, dir, vec_zero, speed, 0, MOD_VEHICLE, -1, -1, 1.0f);
*/
}

/*
====================
Vehicle::WorldEffects
====================
*/
void Vehicle::WorldEffects(void)
{
    /*
    //
    // Check for earthquakes
    //
    if (groundentity && (level.earthquake_magnitude > 0.0f)) {
        velocity += Vector(
            level.earthquake_magnitude * EARTHQUAKE_STRENGTH * G_CRandom(),
            level.earthquake_magnitude * EARTHQUAKE_STRENGTH * G_CRandom(),
            level.earthquake_magnitude * 1.5f * G_Random()
        );
    }

    //
    // check for lava
    //
    if (watertype & CONTENTS_LAVA) {
        Damage(world, world, 20 * waterlevel, origin, vec_zero, vec_zero, 0, DAMAGE_NO_ARMOR, MOD_LAVA);
    }
*/
}

/*
====================
Vehicle::UpdateVariables
====================
*/
void Vehicle::UpdateVariables(
    Vector *acceleration, Vector *vpn, Vector *vup, Vector *vright, Vector *t_vpn, Vector *t_vup, Vector *t_vright
)
{}

/*
====================
Vehicle::TorqueLookup
====================
*/
float Vehicle::TorqueLookup(int rpm)
{
    if (rpm < 5000) {
        return 190;
    } else if (rpm < 6000) {
        return (float)(190 * (6000 - rpm)) / 1000.f;
    } else {
        return 0;
    }
}

/*
====================
Vehicle::SpawnTurret
====================
*/
void Vehicle::SpawnTurret(Event *ev)
{
    VehicleTurretGun *pTurret;
    int               slot;

    pTurret = new VehicleTurretGun();
    pTurret->SetBaseOrientation(orientation, NULL);
    pTurret->setModel(ev->GetString(2));

    slot = ev->GetInteger(1);
    AttachTurretSlot(slot, pTurret, vec_zero, NULL);

    pTurret->SetVehicleOwner(this);
    pTurret->PostEvent(EV_TakeDamage, EV_POSTSPAWN);
    UpdateTurretSlot(slot);

    pTurret->ProcessPendingEvents();
}

/*
====================
Vehicle::EventModifyDrive
====================
*/
void Vehicle::EventModifyDrive(Event *ev)
{
    if (!level.Spawned()) {
        ScriptError("ModifyDrive used improperly... (used before the level is spawned)");
    }

    if (!m_bAutoPilot || !this->m_pCurPath) {
        ScriptError("ModifyDrive used when not driving!");
    }

    if (ev->NumArgs() < 1 || ev->NumArgs() > 3) {
        ScriptError("wrong number of arguments");
    }

    switch (ev->NumArgs()) {
    case 3:
        m_fLookAhead = ev->GetFloat(3);
    case 2:
        m_fIdealAccel = ev->GetFloat(2);
    case 1:
        m_fIdealSpeed = ev->GetFloat(1);
        m_fMaxSpeed   = m_fIdealSpeed;
        break;
    }
}

/*
====================
Vehicle::EventModel
====================
*/
void Vehicle::EventModel(Event *ev)
{
    SetModelEvent(ev);
}

/*
====================
Vehicle::TouchStuff
====================
*/
void Vehicle::TouchStuff(vmove_t *vm)
{
    int        i, j;
    gentity_t *other;
    Event     *event;

    if (driver.ent) {
        G_TouchTriggers(driver.ent);
    }

    for (int i = 0; i < MAX_PASSENGERS; i++) {
        if (Passengers[i].ent) {
            G_TouchTriggers(Passengers[i].ent);
        }
    }

    for (int i = 0; i < MAX_TURRETS; i++) {
        if (Turrets[i].ent) {
            G_TouchTriggers(Turrets[i].ent);
        }
    }

    if (getMoveType() != MOVETYPE_NOCLIP) {
        G_TouchTriggers(this);
    }

    for (i = 0; i < vm->numtouch; i++) {
        other = &g_entities[vm->touchents[i]];

        for (j = 0; j < i; j++) {
            gentity_t *ge = &g_entities[j];

            if (ge == other) {
                break;
            }
        }

        if (j != i) {
            // duplicated
            continue;
        }

        // Don't bother touching the world
        if ((!other->entity) || (other->entity == world)) {
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
Vehicle::EventNextDrive
====================
*/
void Vehicle::EventNextDrive(Event *ev)
{
    SimpleEntity *path;
    float        *i_fTmp;
    float         o_fTmp[4];
    Vector        org1;
    Vector        org2;

    path = ev->GetSimpleEntity(1);

    if (!m_bAutoPilot) {
        ScriptError("Cannot Set Next Path because Not Currently Driving a Path.");
    }

    if (!m_pCurPath || m_pCurPath->m_iPoints == 0) {
        ScriptError("Cannot Set Next Path because Current Path is Empty.");
    }

    if (!m_pNextPath) {
        m_pNextPath = new cVehicleSpline;
    }

    SetupPath(m_pNextPath, path);

    i_fTmp = m_pCurPath->GetByNode(m_pCurPath->m_iPoints, NULL);
    org1   = (i_fTmp + 1);
    i_fTmp = m_pNextPath->GetByNode(0.0f, NULL);
    org2   = (i_fTmp + 1);

    o_fTmp[0] = (org2 - org1).length();
    VectorClear(o_fTmp + 1);

    m_pNextPath->UniformAdd(o_fTmp);
    m_iNextPathStartNode = m_pCurPath->Append(m_pNextPath);
}

/*
====================
Vehicle::EventLockMovement
====================
*/
void Vehicle::EventLockMovement(Event *ev)
{
    m_bMovementLocked = true;
}

/*
====================
Vehicle::EventUnlockMovement
====================
*/
void Vehicle::EventUnlockMovement(Event *ev)
{
    m_bMovementLocked = false;
}

/*
====================
Vehicle::EventRemoveOnDeath
====================
*/
void Vehicle::EventRemoveOnDeath(Event *ev)
{
    m_bRemoveOnDeath = ev->GetBoolean(1);
}

/*
====================
Vehicle::EventSetExplosionModel
====================
*/
void Vehicle::EventSetExplosionModel(Event *ev)
{
    m_sExplosionModel = ev->GetString(1);
}

/*
====================
Vehicle::EventSetCollisionModel
====================
*/
void Vehicle::EventSetCollisionModel(Event *ev)
{
    Entity *pColEnt = ev->GetEntity(1);

    if (!pColEnt) {
        ScriptError("Trying to set a collision model with a NULL entity.");
    }

    if (m_pCollisionEntity) {
        m_pCollisionEntity->PostEvent(EV_Remove, EV_VEHICLE);
    }

    m_pCollisionEntity = new VehicleCollisionEntity(this);
    m_pCollisionEntity->setModel(pColEnt->model);

    if (!m_pCollisionEntity->model.length() || *m_pCollisionEntity->model != '*') {
        // Re-post the event with the correct time
        m_pCollisionEntity->CancelEventsOfType(EV_Remove);
        m_pCollisionEntity->PostEvent(EV_Remove, EV_VEHICLE);
        m_pCollisionEntity = NULL;

        ScriptError("Model for Entity not of a valid type. Must be B-Model.");
    }

    m_pCollisionEntity->setOrigin(origin);
    m_pCollisionEntity->setAngles(angles);
}

/*
====================
Vehicle::EventGetCollisionModel
====================
*/
void Vehicle::EventGetCollisionModel(Event *ev)
{
    ev->AddEntity(m_pCollisionEntity);
}

/*
====================
Vehicle::EventSetSoundParameters
====================
*/
void Vehicle::EventSetSoundParameters(Event *ev)
{
    m_fSoundMinSpeed = ev->GetFloat(1);
    m_fSoundMinPitch = ev->GetFloat(2);
    m_fSoundMaxSpeed = ev->GetFloat(3);
    m_fSoundMaxPitch = ev->GetFloat(4);
}

/*
====================
Vehicle::EventSetVolumeParameters
====================
*/
void Vehicle::EventSetVolumeParameters(Event *ev)
{
    m_fVolumeMinSpeed = ev->GetFloat(1);
    m_fVolumeMinPitch = ev->GetFloat(2);
    m_fVolumeMaxSpeed = ev->GetFloat(3);
    m_fVolumeMaxPitch = ev->GetFloat(4);
}

/*
====================
Vehicle::UpdateSound
====================
*/
void Vehicle::UpdateSound(void)
{
    float pitch;
    float volume;

    if (level.time < m_fNextSoundState) {
        return;
    }

    // Calculate the pitch based on the vehicle's speed
    pitch = (velocity.length() - m_fSoundMinSpeed) / (m_fSoundMaxSpeed - m_fSoundMinSpeed);

    if (pitch > 1.0f) {
        pitch = 1.0f;
    } else if (pitch < 0.0f) {
        pitch = 0.0f;
    }

    pitch = m_fSoundMinPitch + (m_fSoundMaxPitch - m_fSoundMinPitch) * pitch;

    volume = (velocity.length() - m_fVolumeMinSpeed) / (m_fVolumeMaxSpeed - m_fVolumeMinSpeed);

    if (volume > 1.0f) {
        volume = 1.0f;
    } else if (volume < 0.0f) {
        volume = 0.0f;
    }

    volume = this->m_fVolumeMinPitch + (this->m_fVolumeMaxPitch - this->m_fVolumeMinPitch) * volume;

    switch (m_eSoundState) {
    case ST_OFF:
        StopLoopSound();
        TurnOffVehicleSoundEntities();
        m_fNextSoundState = level.time;

        if (driver.ent || m_bAutoPilot) {
            m_eSoundState = ST_OFF_TRANS_IDLE;
        }
        break;

    case ST_OFF_TRANS_IDLE:
        m_fNextSoundState = level.time;
        m_eSoundState     = ST_IDLE;
        Sound(m_sSoundSet + "snd_on");
        LoopSound(m_sSoundSet + "snd_idle");
        break;

    case ST_IDLE_TRANS_OFF:
        m_fNextSoundState = level.time;
        m_eSoundState     = ST_OFF;
        Sound(m_sSoundSet + "snd_off");
        StopLoopSound();
        break;

    case ST_IDLE:
        m_fNextSoundState = level.time;

        if (driver.ent || m_bAutoPilot) {
            if (fabs(DotProduct(orientation[0], velocity)) > 10.0f) {
                m_eSoundState = ST_IDLE_TRANS_RUN;
            }
        } else {
            m_eSoundState = ST_IDLE_TRANS_OFF;
        }

        LoopSound(m_sSoundSet + "snd_idle");

        TurnOffVehicleSoundEntities();
        break;

    case ST_IDLE_TRANS_RUN:
        m_fNextSoundState = level.time;
        m_eSoundState     = ST_RUN;

        Sound(m_sSoundSet + "snd_revup");
        LoopSound(m_sSoundSet + "snd_run", -1.0f, -1.0f, -1.0f, pitch);
        break;

    case ST_RUN:
        m_fNextSoundState = level.time;

        if (fabs(DotProduct(orientation[0], velocity)) < 10.0f) {
            m_eSoundState = ST_RUN_TRANS_IDLE;
        }

        TurnOnVehicleSoundEntities();
        LoopSound(m_sSoundSet + "snd_run", volume, -1.0f, -1.0f, pitch);
        break;

    case ST_RUN_TRANS_IDLE:
        m_fNextSoundState = level.time;
        m_eSoundState     = ST_IDLE;

        Sound(m_sSoundSet + "snd_revdown");
        LoopSound(m_sSoundSet + "snd_idle");
        break;

    default:
        m_fNextSoundState = level.time;
        m_eSoundState     = ST_OFF;
        break;
    }
}

/*
====================
Vehicle::SetupVehicleSoundEntities
====================
*/
void Vehicle::SetupVehicleSoundEntities(void)
{
    int    i;
    Vector a;
    Vector b;
    Vector c;
    Vector start;

    angles.AngleVectorsLeft(&a, &b, &c);

    // place the sound entities in the vehicle wheels
    for (i = 0; i < MAX_CORNERS; i++) {
        if (!m_pVehicleSoundEntities[i]) {
            m_pVehicleSoundEntities[i] = new VehicleSoundEntity(this);
        }

        start = origin + a * Corners[i][0] + b * Corners[i][1] + c * Corners[i][2];
        m_pVehicleSoundEntities[i]->setOrigin(start);
    }
}

/*
====================
Vehicle::TurnOnVehicleSoundEntities
====================
*/
void Vehicle::TurnOnVehicleSoundEntities(void)
{
    for (int i = 0; i < MAX_CORNERS; i++) {
        if (!m_pVehicleSoundEntities[i]) {
            m_pVehicleSoundEntities[i] = new VehicleSoundEntity(this);
        }

        m_pVehicleSoundEntities[i]->Start();
    }
}

/*
====================
Vehicle::TurnOffVehicleSoundEntities
====================
*/
void Vehicle::TurnOffVehicleSoundEntities(void)
{
    for (int i = 0; i < MAX_CORNERS; i++) {
        if (!m_pVehicleSoundEntities[i]) {
            m_pVehicleSoundEntities[i] = new VehicleSoundEntity(this);
        }

        m_pVehicleSoundEntities[i]->Stop();
    }
}

/*
====================
Vehicle::RemoveVehicleSoundEntities
====================
*/
void Vehicle::RemoveVehicleSoundEntities(void)
{
    for (int i = 0; i < MAX_CORNERS; i++) {
        if (!m_pVehicleSoundEntities[i]) {
            continue;
        }

        m_pVehicleSoundEntities[i]->PostEvent(EV_Remove, EV_VEHICLE);
    }
}

/*
====================
Vehicle::KickSuspension
====================
*/
void Vehicle::KickSuspension(Vector vDirection, float fForce)
{
    VectorNormalizeFast(vDirection);

    m_fForwardForce += vDirection * orientation[1] * fForce;
    m_fLeftForce += vDirection * orientation[0] * fForce;
}

/*
====================
Vehicle::EventDamage
====================
*/
void Vehicle::EventDamage(Event *ev)
{
    Entity *pEnt;
    Vector  vDirection;
    float   fForce;
    int     i;

    if (!IsDamagedBy(ev->GetEntity(3))) {
        return;
    }

    pEnt = ev->GetEntity(1);
    if (pEnt && pEnt == driver.ent) {
        return;
    }

    Event *event = new Event(EV_Damage, ev->NumArgs());

    vDirection = ev->GetVector(5);
    fForce     = ev->GetFloat(7);
    KickSuspension(vDirection, fForce);

    for (i = 1; i <= ev->NumArgs(); i++) {
        if (i == 7) {
            event->AddFloat(0);
        } else {
            event->AddValue(ev->GetValue(i));
        }
    }

    if (driver.ent && driver.ent->IsSubclassOfPlayer()) {
        Player *player = static_cast<Player *>(driver.ent.Pointer());
        Vector  dir    = ev->GetVector(1);
        float   dir_yaw;
        float   camera_yaw;

        if (player->camera) {
            dir_yaw    = dir.toYaw();
            camera_yaw = player->camera->angles[1];
        } else {
            dir_yaw    = dir.toYaw();
            camera_yaw = player->GetVAngles()[1];
        }

        player->damage_yaw = AngleSubtract(camera_yaw, dir_yaw) + 180.5;
    }

    DamageEvent(event);
    delete event;
}

/*
====================
Vehicle::FactorInOriginOffset
====================
*/
void Vehicle::FactorInOriginOffset(void)
{
    origin += m_vOriginOffset;
    setOrigin(origin);
}

/*
====================
Vehicle::FactorOutOriginOffset
====================
*/
void Vehicle::FactorOutOriginOffset(void)
{
    origin -= m_vOriginOffset;
    setOrigin(origin);
}

/*
====================
Vehicle::CalculateOriginOffset
====================
*/
void Vehicle::CalculateOriginOffset(void)
{
    int    index;
    Vector vTireAvg;
    Vector vMissHit;
    Vector temp;
    int    iNum = 0;
    Vector acceleration;
    Vector oldoffset;

    oldoffset = m_vOriginOffset;
    m_vOriginOffset += m_vOriginOffset2;
    m_vOriginOffset2 = vec_zero;

    for (index = 0; index < MAX_CORNERS; index++) {
        if (m_bTireHit[index]) {
            temp = m_vTireEnd[index];
            vTireAvg += origin - temp;
            iNum++;
        } else {
            temp     = Corners[index];
            vMissHit = temp;
        }
    }

    if (iNum == 3) {
        temp = m_vNormalSum * (1.0f / m_iNumNormals);
        ProjectPointOnPlane(acceleration, vMissHit, temp);
        vTireAvg += acceleration;
    } else if (iNum == 4) {
        vTireAvg *= 0.25f;
        MatrixTransformVector(m_vOriginCornerOffset, orientation, acceleration);
        vTireAvg -= acceleration;
        m_vOriginOffset2 += vTireAvg;
    }

    m_vOriginOffset2 += vec_zero;
    FactorInSkidOrigin();

    Vector vTmp = real_acceleration - prev_acceleration;

    m_fDownForce = vTmp[2] * m_fZCoef;
    m_fDownForce = Q_clamp_float(m_fDownForce, m_fZMin, m_fZMax);

    m_fUpForce = -m_vOriginOffset[2] * m_fBouncyCoef + m_fUpForce;
    m_fUpForce *= m_fSpringyCoef;
    m_vOriginOffset[2] += (m_fDownForce + m_fUpForce) * level.frametime * 12;

    if (m_vOriginOffset[2] < m_fZMin) {
        m_vOriginOffset[2] = m_fZMin;
    }

    m_vOriginOffset -= m_vOriginOffset2;

    if (!isfinite(m_vOriginOffset[0]) || !isfinite(m_vOriginOffset[1]) || !isfinite(m_vOriginOffset[2])) {
        m_vOriginOffset = oldoffset;
    }
}

/*
====================
Vehicle::UpdateTires
====================
*/
void Vehicle::UpdateTires(void)
{
    int     index;
    trace_t trace;
    Vector  a;
    Vector  b;
    Vector  c;
    Vector  vTmp;
    Vector  t_mins;
    Vector  t_maxs;
    Vector  start;
    Vector  end;
    Vector  boxoffset;
    Entity *pSkippedEntities[MAX_SKIPPED_ENTITIES];
    int     iContentsEntities[MAX_SKIPPED_ENTITIES];
    solid_t solidEntities[MAX_SKIPPED_ENTITIES];
    int     iNumSkippedEntities;
    int     iNumSkipped = 0;

    t_mins = mins * 0.25f;
    t_maxs = maxs * 0.25f;

    if (real_velocity.length() <= 0.5f && m_iLastTiresUpdate != -1 && m_iLastTiresUpdate + 1000 > level.inttime) {
        return;
    }

    m_iLastTiresUpdate = level.inttime;

    vTmp.y = angles.y + m_fSkidAngle;
    AngleVectors(vTmp, a, b, c);

    // Temporary make slots non-solid for G_Trace
    SetSlotsNonSolid();

    do {
        iNumSkippedEntities = 0;

        for (index = 0; index < MAX_CORNERS; index++) {
            boxoffset = Corners[index];
            start     = origin + a * boxoffset[0] + b * boxoffset[1] + c * boxoffset[2];
            end       = start + Vector(0, 0, -400);

            trace = G_Trace(start, t_mins, t_maxs, end, this, MASK_VEHICLE_TIRES, false, "Vehicle::PostThink Corners");

            if (g_showvehiclemovedebug->integer) {
                G_DebugLine(start, end, 1, 1, 1, 1);
                G_DebugLine(start, trace.endpos, 1, 0, 0, 1);
            }

            if (trace.ent && trace.ent->entity && trace.ent->entity->isSubclassOf(VehicleCollisionEntity)) {
                // save the entity
                pSkippedEntities[iNumSkipped]  = trace.ent->entity;
                iContentsEntities[iNumSkipped] = trace.ent->r.contents;
                solidEntities[iNumSkipped]     = trace.ent->solid;
                iNumSkipped++;

                if (iNumSkipped >= MAX_SKIPPED_ENTITIES) {
                    gi.Error(ERR_DROP, "MAX_SKIPPED_ENTITIES hit in VehicleMove.\n");
                    return;
                }

                trace.ent->entity->setSolidType(SOLID_NOT);
                iNumSkippedEntities++;
            }

            if (trace.fraction == 1.0) {
                m_bTireHit[index] = false;
            } else {
                m_vTireEnd[index] = trace.endpos;
                m_bTireHit[index] = true;
            }
        }
    } while (iNumSkippedEntities != 0);

    for (index = 0; index < iNumSkipped; index++) {
        pSkippedEntities[index]->setSolidType(solidEntities[index]);
        pSkippedEntities[index]->setContents(iContentsEntities[index]);
    }

    // Turn slots back into a solid state
    SetSlotsSolid();
}

/*
====================
Vehicle::UpdateNormals
====================
*/
void Vehicle::UpdateNormals(void)
{
    Vector vDist1;
    Vector vDist2;
    Vector vCross;
    Vector temp;
    Vector pitch;
    Vector i;
    Vector j;

    if (real_velocity.length() <= 0.5 && m_iLastTiresUpdate != -1 && m_iLastTiresUpdate + 1000 > level.inttime) {
        return;
    }

    angles.AngleVectorsLeft(&i, &j, NULL);

    j             = vec_zero - j;
    m_vNormalSum  = vec_zero;
    m_iNumNormals = 0;

    if (m_bTireHit[0] && m_bTireHit[1] && m_bTireHit[2]) {
        vDist1 = m_vTireEnd[1] - m_vTireEnd[0];
        vDist2 = m_vTireEnd[1] - m_vTireEnd[2];

        CrossProduct(vDist1, vDist2, vCross);
        vCross.normalize();
        m_vNormalSum += vCross;

        m_iNumNormals++;
    }

    if (m_bTireHit[1] && m_bTireHit[2] && m_bTireHit[3]) {
        vDist1 = m_vTireEnd[2] - m_vTireEnd[1];
        vDist2 = m_vTireEnd[2] - m_vTireEnd[3];

        CrossProduct(vDist1, vDist2, vCross);
        vCross.normalize();
        m_vNormalSum += vCross;

        m_iNumNormals++;
    }

    if (m_bTireHit[2] && m_bTireHit[3] && m_bTireHit[0]) {
        vDist1 = m_vTireEnd[3] - m_vTireEnd[0];
        vDist2 = m_vTireEnd[3] - m_vTireEnd[2];

        CrossProduct(vDist1, vDist2, vCross);
        vCross.normalize();
        m_vNormalSum += vCross;

        m_iNumNormals++;
    }

    if (m_bTireHit[3] && m_bTireHit[0] && m_bTireHit[1]) {
        vDist1 = m_vTireEnd[0] - m_vTireEnd[3];
        vDist2 = m_vTireEnd[0] - m_vTireEnd[1];

        CrossProduct(vDist1, vDist2, vCross);
        vCross.normalize();
        m_vNormalSum += vCross;

        m_iNumNormals++;
    }

    if (m_iNumNormals > 1) {
        temp = m_vNormalSum / m_iNumNormals;
        i    = temp.CrossProduct(temp, j);

        pitch     = i;
        angles[0] = pitch.toPitch();

        temp      = m_vNormalSum / m_iNumNormals;
        pitch     = temp.CrossProduct(temp, i);
        angles[2] = pitch.toPitch();
    }
}

/*
====================
Vehicle::UpdateBones
====================
*/
void Vehicle::UpdateBones(void)
{
    float fNewTurnAngle = AngleNormalize180(turnangle - m_fSkidAngle);
    fNewTurnAngle       = Q_clamp_float(fNewTurnAngle, -maxturnrate, maxturnrate);

    SetControllerAngles(0, Vector(0, fNewTurnAngle, 0));
    SetControllerAngles(1, Vector(0, fNewTurnAngle, 0));
}

/*
====================
Vehicle::UpdateShaderOffset
====================
*/
void Vehicle::UpdateShaderOffset(void)
{
    m_fShaderOffset -= real_velocity * orientation[0] / 4 * level.frametime;
    edict->s.shader_time = m_fShaderOffset;
}

/*
====================
Vehicle::UpdateDriverSlot
====================
*/
void Vehicle::UpdateDriverSlot(int iSlot)
{
    orientation_t orient;

    if (!(driver.flags & SLOT_BUSY) || !driver.ent) {
        return;
    }

    if (driver.boneindex != -1) {
        GetTag(driver.boneindex, &orient);
        if (driver.ent->IsSubclassOfActor()) {
            driver.ent->setOriginEvent(orient.origin);
        } else {
            driver.ent->setOrigin(orient.origin);
        }

        if (drivable) {
            driver.ent->avelocity = avelocity;
            driver.ent->velocity  = velocity;
            driver.ent->setAngles(angles);
        }
    } else {
        Vector forward = orientation[0];
        Vector left    = orientation[1];
        Vector up      = orientation[2];

        if (driver.ent->IsSubclassOfActor()) {
            driver.ent->setOriginEvent(
                origin + forward * driveroffset[0] + left * driveroffset[1] + up * driveroffset[2]
            );
        } else {
            driver.ent->setOrigin(origin + forward * driveroffset[0] + left * driveroffset[1] + up * driveroffset[2]);
        }

        if (drivable) {
            driver.ent->avelocity = avelocity;
            driver.ent->velocity  = velocity;
            driver.ent->setAngles(angles);
        }
    }
}

/*
====================
Vehicle::UpdatePassengerSlot
====================
*/
void Vehicle::UpdatePassengerSlot(int iSlot)
{
    orientation_t orient;

    if (!(Passengers[iSlot].flags & SLOT_BUSY) || !Passengers[iSlot].ent) {
        return;
    }

    if (Passengers[iSlot].boneindex != -1) {
        GetTag(Passengers[iSlot].boneindex, &orient);
        if (Passengers[iSlot].ent->IsSubclassOfActor()) {
            Passengers[iSlot].ent->setOriginEvent(orient.origin);
        } else {
            Passengers[iSlot].ent->setOrigin(orient.origin);
        }

        Passengers[iSlot].ent->avelocity = avelocity;
        Passengers[iSlot].ent->velocity  = velocity;

        if (!Passengers[iSlot].ent->IsSubclassOfActor() || ((Actor *)Passengers[iSlot].ent.Pointer())->m_Enemy) {
            Vector newAngles;

            MatrixToEulerAngles(orient.axis, newAngles);
            Passengers[iSlot].ent->setAngles(newAngles);
        }
    } else {
        if (Passengers[iSlot].ent->IsSubclassOfActor()) {
            Passengers[iSlot].ent->setOriginEvent(origin);
        } else {
            Passengers[iSlot].ent->setOrigin(origin);
        }

        Passengers[iSlot].ent->avelocity = avelocity;
        Passengers[iSlot].ent->velocity  = velocity;

        if (!Passengers[iSlot].ent->IsSubclassOfActor() || ((Actor *)Passengers[iSlot].ent.Pointer())->m_Enemy) {
            Passengers[iSlot].ent->setAngles(angles);
        }
    }
}

/*
====================
Vehicle::UpdateTurretSlot
====================
*/
void Vehicle::UpdateTurretSlot(int iSlot)
{
    orientation_t orient;

    if (!(Turrets[iSlot].flags & SLOT_BUSY) || !Turrets[iSlot].ent) {
        return;
    }

    if (Turrets[iSlot].boneindex != -1) {
        GetTag(Turrets[iSlot].boneindex, &orient);
        if (Turrets[iSlot].ent->IsSubclassOfActor()) {
            Turrets[iSlot].ent->setOriginEvent(orient.origin);
        } else {
            Turrets[iSlot].ent->setOrigin(orient.origin);
        }

        Turrets[iSlot].ent->avelocity = avelocity;
        Turrets[iSlot].ent->velocity  = velocity;

        if (Turrets[iSlot].ent->IsSubclassOfVehicleTurretGun()) {
            VehicleTurretGun *vtg = static_cast<VehicleTurretGun *>(Turrets[iSlot].ent.Pointer());
            vtg->SetBaseOrientation(orient.axis, NULL);
        }
    } else {
        Vector forward = orientation[0];
        Vector left    = orientation[1];
        Vector up      = orientation[2];

        if (Turrets[iSlot].ent->IsSubclassOfActor()) {
            Turrets[iSlot].ent->setOriginEvent(origin);
        } else {
            Turrets[iSlot].ent->setOrigin(origin);
        }

        Turrets[iSlot].ent->avelocity = avelocity;
        Turrets[iSlot].ent->velocity  = velocity;

        if (Turrets[iSlot].ent->IsSubclassOfVehicleTurretGun()) {
            VehicleTurretGun *vtg = static_cast<VehicleTurretGun *>(Turrets[iSlot].ent.Pointer());
            vtg->SetBaseOrientation(this->orientation, NULL);
        }
    }
}

/*
====================
Vehicle::EventStopAtEnd
====================
*/
void Vehicle::EventStopAtEnd(Event *ev)
{
    if (!m_pCurPath) {
        ScriptError("Tried to Stop at end of path on a vehicle who is not driving a path!");
    }

    m_fStopStartDistance = GetPathPosition(m_pCurPath, m_iCurNode);
    m_fStopStartSpeed    = moveimpulse;
    m_fStopEndDistance   = *m_pCurPath->GetByNode(m_pCurPath->m_iPoints, NULL);
    m_bStopEnabled       = true;
}

/*
====================
Vehicle::GetPathPosition
====================
*/
float Vehicle::GetPathPosition(cVehicleSpline *pPath, int iNode)
{
    float *vTmp;
    vec3_t vPrev;
    vec3_t vCur;
    vec3_t vTotal;
    Vector vDelta;
    float  fTotal;
    float  fCoef;

    vTmp = pPath->GetByNode(iNode, NULL);
    VectorCopy(vTmp + 1, vCur);

    if (g_showvehiclemovedebug->integer) {
        G_DebugString(vCur, 3.0f, 1.0f, 1.0f, 1.0f, "%f", vTmp[0]);
    }

    vTmp = pPath->GetByNode(iNode - 1, NULL);
    VectorCopy(vTmp + 1, vPrev);

    if (g_showvehiclemovedebug->integer) {
        G_DebugString(vPrev, 3.0f, 1.0f, 1.0f, 1.0f, "%f", vTmp[0]);
    }

    VectorSubtract(vCur, vPrev, vTotal);
    fTotal      = VectorLength(vTotal);
    m_vIdealDir = vTotal;
    VectorNormalize(m_vIdealDir);
    angles.AngleVectorsLeft(&vDelta, NULL, NULL);

    fCoef = ProjectLineOnPlane(vDelta, DotProduct(vDelta, origin), vPrev, vCur, NULL);

    if (g_showvehiclemovedebug->integer) {
        G_DebugBBox(vPrev, Vector(-32, -32, -32), Vector(32, 32, 32), 0, 1, 1, 1);
        G_DebugBBox(vCur, Vector(-32, -32, -32), Vector(32, 32, 32), 1, 1, 0, 1);
        G_DebugArrow(vCur, m_vIdealDir * -1.0, (1.0 - fCoef) * fTotal, 0, 1, 0, 1);
        G_DebugArrow(vPrev, m_vIdealDir, fCoef * fTotal, 0, 0, 1, 1);
    }

    return *pPath->GetByNode(iNode - (1.0 - fCoef), NULL);
}

/*
====================
Vehicle::EventSkidding
====================
*/
void Vehicle::EventSkidding(Event *ev)
{
    if (ev->NumArgs() == 1) {
        m_bEnableSkidding = ev->GetInteger(1);
    } else {
        m_bEnableSkidding = true;
    }

    ProcessEvent(EV_Vehicle_ContinueSkidding);
}

/*
====================
Vehicle::UpdateSkidAngle
====================
*/
void Vehicle::UpdateSkidAngle(void)
{
    if (m_bEnableSkidding) {
        if (g_showvehiclemovedebug && g_showvehiclemovedebug->integer) {
            Com_Printf("Skidding!\n");
        }

        m_fSkidLeftForce += velocity.length() / 150.0f * turnangle;
        m_fSkidRightForce += -m_fSkidAngle * 0.2;
        m_fSkidRightForce *= 0.3f;
        m_fSkidAngle     = m_fSkidAngle + (m_fSkidLeftForce + m_fSkidRightForce) * 22.0f * level.frametime;
        m_vSkidOrigin[0] = -fabs(m_fSkidAngle);
    } else {
        m_fSkidAngle = 0;
    }
}

/*
====================
Vehicle::FactorInSkidOrigin
====================
*/
void Vehicle::FactorInSkidOrigin(void)
{
    Vector vNewOrigin;

    vNewOrigin[0] = orientation[0][0] * m_vSkidOrigin[0] + orientation[1][0] * m_vSkidOrigin[1]
                  + orientation[2][0] * m_vSkidOrigin[2];
    vNewOrigin[1] = orientation[0][1] * m_vSkidOrigin[0] + orientation[1][1] * m_vSkidOrigin[1]
                  + orientation[2][1] * m_vSkidOrigin[2];
    vNewOrigin[2] = orientation[0][2] * m_vSkidOrigin[0] + orientation[1][2] * m_vSkidOrigin[1]
                  + orientation[2][2] * m_vSkidOrigin[2];

    m_vOriginOffset2 += vNewOrigin;
}

/*
====================
Vehicle::EventContinueSkidding
====================
*/
void Vehicle::EventContinueSkidding(Event *ev)
{
    if (m_bEnableSkidding) {
        if (HasAnim("skidding")) {
            NewAnim("skidding", EV_Vehicle_ContinueSkidding, 7, 0.000001f);
        } else {
            assert(!"Vehicle without skidding animation.");
        }
    } else {
        if (HasAnim("idle")) {
            NewAnim("idle", 0, 7, 0.000001f);
        } else {
            assert(!"Vehicle without idle animation.");
        }
    }
}

/*
====================
Vehicle::FactorInAnglesOffset
====================
*/
void Vehicle::FactorInAnglesOffset(Vector *vAddedAngles)
{
    (*vAddedAngles) = m_vAnglesOffset;
    vAddedAngles->y += m_fSkidAngle;
}

/*
====================
Vehicle::FactorOutAnglesOffset
====================
*/
void Vehicle::FactorOutAnglesOffset(void) {}

/*
====================
Vehicle::CalculateAnglesOffset
====================
*/
void Vehicle::CalculateAnglesOffset(Vector acceleration)
{
    if (level.inttime <= 1200) {
        // leave some time before allowing to shake
        // so all tanks spawn peacefully in the level
        return;
    }

    m_fForwardForce += DotProduct(orientation[0], acceleration) * m_fYawCoef;
    m_fBackForce = -m_vAnglesOffset[0] * m_fBouncyCoef + m_fBackForce;
    m_fBackForce *= m_fSpringyCoef;

    m_vAnglesOffset[0] += m_fForwardForce + m_fBackForce * 12.0 * level.frametime;
    m_vAnglesOffset[0] = Q_clamp_float(m_vAnglesOffset[0], m_fYawMin, m_fYawMax);

    m_fForwardForce = 0;
    m_fLeftForce += acceleration * orientation[1] * m_fRollCoef;
    m_fRightForce = -m_vAnglesOffset[2] * m_fBouncyCoef + m_fRightForce;
    m_fRightForce *= m_fSpringyCoef;

    m_vAnglesOffset[2] += 12.0 * (m_fLeftForce + m_fRightForce) * level.frametime;
    m_vAnglesOffset[2] = Q_clamp_float(m_vAnglesOffset[2], m_fRollMin, m_fRollMax);

    m_fLeftForce = 0;
}

/*
====================
Vehicle::CalculateAnimationData
====================
*/
void Vehicle::CalculateAnimationData(Vector vAngles, Vector vOrigin)
{
    float fLeft    = fEpsilon();
    float fRight   = fEpsilon();
    float fForward = fEpsilon();
    float fBack    = fEpsilon();
    float fLow     = fEpsilon();
    float fHigh    = fEpsilon();

    if (vAngles[0] < 0.0) {
        fBack = vAngles[0] / m_fYawMin;
    } else if (vAngles[0] > 0.0) {
        fForward = vAngles[0] / m_fYawMax;
    }

    if (vAngles[2] < 0.0) {
        fRight = vAngles[2] / m_fRollMin;
    } else if (vAngles[2] > 0.0) {
        fLeft = vAngles[2] / m_fRollMax;
    }

    if (vOrigin[2] < 0.0) {
        fBack = vOrigin[2] / m_fZMin;
    } else if (vOrigin[2] > 0.0) {
        fForward = vOrigin[2] / m_fZMax;
    }

    if (!m_bAnimMove) {
        NewAnim("idle", NULL, 0, 1.0);
        NewAnim("lean_left", NULL, 3, fLeft);
        NewAnim("lean_right", NULL, 4, fRight);
        NewAnim("lean_forward", NULL, 1, fForward);
        NewAnim("lean_back", NULL, 2, fBack);
        NewAnim("high", NULL, 6, fLow);
        NewAnim("low", NULL, 5, fHigh);
    }
}

/*
====================
Vehicle::EventVehicleAnim
====================
*/
void Vehicle::EventVehicleAnim(Event *ev)
{
    float weight;

    if (ev->NumArgs() > 1) {
        weight = ev->GetFloat(2);
    } else {
        weight = 1.0f;
    }

    NewAnim(ev->GetString(1), EV_Vehicle_VehicleAnimDone, 8, weight);
}

/*
====================
Vehicle::EventVehicleAnimDone
====================
*/
void Vehicle::EventVehicleAnimDone(Event *ev)
{
    Unregister(STRING_VEHICLEANIMDONE);
}

void Vehicle::EventVehicleMoveAnim(Event *ev)
{
    str anim_name;

    anim_name = ev->GetString(1);
    if (!HasAnim(anim_name)) {
        return;
    }

    m_bAnimMove = true;

    StopAnimating(0);
    StopAnimating(3);
    StopAnimating(4);
    StopAnimating(1);
    StopAnimating(2);
    StopAnimating(6);
    StopAnimating(5);
    StopAnimating(8);

    NewAnim(anim_name, EV_Vehicle_VehicleMoveAnimDone);
}

void Vehicle::EventVehicleMoveAnimDone(Event *ev)
{
    Unregister(STRING_ANIMDONE);
    moveimpulse = 0;
    turnimpulse = 0;
    m_bAnimMove = false;
}

void Vehicle::EventDamageSounds(Event *ev)
{
    if (ev->NumArgs() == 1) {
        m_bDamageSounds = ev->GetInteger(1);
    } else {
        m_bDamageSounds = true;
    }
}

void Vehicle::EventRunSounds(Event *ev)
{
    if (ev->NumArgs() == 1) {
        m_bRunSounds = ev->GetInteger(1);
    } else {
        m_bRunSounds = true;
    }
}

void Vehicle::Remove(Event *ev)
{
    int i;

    for (i = 0; i < MAX_TURRETS; i++) {
        Entity *pTurret = Turrets[i].ent;
        if (!pTurret) {
            continue;
        }

        pTurret->PostEvent(EV_Remove, 0);
        Turrets[i].ent = NULL;
    }

    Entity::Remove(ev);
}

/*
====================
Vehicle::IsDamagedBy

Returns whether or not the vehicle is damaged by the specified entity.
====================
*/
bool Vehicle::IsDamagedBy(Entity *ent)
{
    int i;

    if (FindDriverSlotByEntity(ent) != -1) {
        return false;
    }

    if (FindPassengerSlotByEntity(ent) != -1) {
        return false;
    }

    if (FindTurretSlotByEntity(ent) != -1) {
        return false;
    }

    for (i = 0; i < MAX_TURRETS; i++) {
        TurretGun *pTurret;

        if (!Turrets[i].ent) {
            continue;
        }

        pTurret = static_cast<TurretGun *>(Turrets[i].ent.Pointer());
        if (pTurret->IsSubclassOfTurretGun() && pTurret->owner == ent) {
            return false;
        }
    }

    return true;
}

/*
====================
Vehicle::DriverAdded
====================
*/
void Vehicle::DriverAdded() {}

void Vehicle::EventSetMaxUseAngle(Event *ev)
{
    m_fMaxUseAngle = ev->GetFloat(1);
}

void Vehicle::EventCanUse(Event *ev)
{
    Entity *entity = ev->GetEntity(1);

    if (driver.ent || !m_fMaxUseAngle || !Turrets[0].ent) {
        ev->AddInteger(1);
        return;
    }

    if (!entity) {
        ev->AddInteger(0);
        return;
    }

    Vector vForward;
    Vector vDir;

    AngleVectors(Turrets[0].ent->angles, vForward, NULL, NULL);

    vDir = Turrets[0].ent->origin - entity->origin;
    VectorNormalize(vDir);

    if (fabs(GetAngleBetweenVectors2D(vForward, vDir)) > m_fMaxUseAngle) {
        ev->AddInteger(1);
    } else {
        ev->AddInteger(0);
    }
}

/*
====================
Vehicle::Archive
====================
*/
void Vehicle::Archive(Archiver& arc)
{
    VehicleBase::Archive(arc);

    arc.ArchiveFloat(&maxturnrate);
    arc.ArchiveFloat(&currentspeed);
    arc.ArchiveFloat(&turnangle);
    arc.ArchiveFloat(&turnimpulse);
    arc.ArchiveFloat(&moveimpulse);
    arc.ArchiveFloat(&jumpimpulse);
    arc.ArchiveFloat(&speed);
    arc.ArchiveFloat(&conesize);
    arc.ArchiveFloat(&maxtracedist);
    arc.ArchiveString(&weaponName);
    arc.ArchiveVector(&last_origin);
    arc.ArchiveVector(&seatangles);
    arc.ArchiveVector(&seatoffset);
    arc.ArchiveVector(&driveroffset);

    arc.ArchiveVector(&Corners[0]);
    arc.ArchiveVector(&Corners[1]);
    arc.ArchiveVector(&Corners[2]);
    arc.ArchiveVector(&Corners[3]);

    arc.ArchiveBoolean(&drivable);
    arc.ArchiveBoolean(&locked);
    arc.ArchiveBoolean(&hasweapon);
    arc.ArchiveBoolean(&showweapon);
    arc.ArchiveBoolean(&steerinplace);
    arc.ArchiveBoolean(&jumpable);

    arc.ArchiveBoolean(&m_bMovementLocked);
    driver.Archive(arc);
    lastdriver.Archive(arc);

    for (int i = MAX_PASSENGERS - 1; i >= 0; i--) {
        Passengers[i].Archive(arc);
    }

    for (int i = MAX_TURRETS - 1; i >= 0; i--) {
        Turrets[i].Archive(arc);
    }

    arc.ArchiveInteger(&numPassengers);
    arc.ArchiveInteger(&numTurrets);
    arc.ArchiveInteger(&moveresult);
    arc.ArchiveInteger(&isBlocked);
    arc.ArchiveInteger(&m_iFrameCtr);
    arc.ArchiveInteger(&m_iGear);
    arc.ArchiveInteger(&m_iRPM);
    arc.ArchiveInteger(&m_iNextPathStartNode);

    if (!arc.Saving()) {
        m_iLastTiresUpdate = -1;

        int tempInt;
        arc.ArchiveInteger(&tempInt);

        if (tempInt) {
            m_pAlternatePath = new cVehicleSpline;
        } else {
            m_pAlternatePath = NULL;
        }
    } else {
        int tempInt = m_pAlternatePath != NULL;
        arc.ArchiveInteger(&tempInt);
    }

    if (m_pAlternatePath) {
        m_pAlternatePath->Archive(arc);
    }

    arc.ArchiveInteger(&m_iAlternateNode);

    if (!arc.Saving()) {
        int tempInt;
        arc.ArchiveInteger(&tempInt);

        if (tempInt) {
            m_pCurPath = new cVehicleSpline;
        } else {
            m_pCurPath = NULL;
        }
    } else {
        int tempInt = m_pCurPath != NULL;
        arc.ArchiveInteger(&tempInt);
    }

    if (m_pCurPath) {
        m_pCurPath->Archive(arc);
    }

    arc.ArchiveInteger(&m_iCurNode);

    if (!arc.Saving()) {
        int tempInt;
        arc.ArchiveInteger(&tempInt);

        if (tempInt) {
            m_pNextPath = new cVehicleSpline;
        } else {
            m_pNextPath = NULL;
        }
    } else {
        int tempInt = m_pNextPath != NULL;
        arc.ArchiveInteger(&tempInt);
    }

    if (m_pNextPath) {
        m_pNextPath->Archive(arc);
    }

    arc.ArchiveFloat(&maxturnrate);
    arc.ArchiveFloat(&currentspeed);
    arc.ArchiveFloat(&turnangle);
    arc.ArchiveFloat(&turnimpulse);
    arc.ArchiveFloat(&moveimpulse);
    arc.ArchiveFloat(&prev_moveimpulse);
    arc.ArchiveFloat(&jumpimpulse);
    arc.ArchiveFloat(&speed);
    arc.ArchiveFloat(&conesize);
    arc.ArchiveFloat(&maxtracedist);
    arc.ArchiveFloat(&airspeed);
    arc.ArchiveFloat(&m_fGearRatio[0]);
    arc.ArchiveFloat(&m_fGearRatio[1]);
    arc.ArchiveFloat(&m_fGearRatio[2]);
    arc.ArchiveFloat(&m_fGearRatio[3]);
    arc.ArchiveFloat(&m_fGearRatio[4]);
    arc.ArchiveFloat(&m_fGearRatio[5]);
    arc.ArchiveFloat(&m_fMass);
    arc.ArchiveFloat(&m_fFrontMass);
    arc.ArchiveFloat(&m_fBackMass);
    arc.ArchiveFloat(&m_fWheelBase);
    arc.ArchiveFloat(&m_fWheelFrontLoad);
    arc.ArchiveFloat(&m_fWheelFrontInnerLoad);
    arc.ArchiveFloat(&m_fWheelFrontOutterLoad);
    arc.ArchiveFloat(&m_fWheelFrontDist);
    arc.ArchiveFloat(&m_fWheelFrontSuspension);
    arc.ArchiveFloat(&m_fWheelBackLoad);
    arc.ArchiveFloat(&m_fWheelBackInnerLoad);
    arc.ArchiveFloat(&m_fWheelBackOutterLoad);
    arc.ArchiveFloat(&m_fWheelBackDist);
    arc.ArchiveFloat(&m_fWheelBackSuspension);
    arc.ArchiveFloat(&m_fCGHeight);
    arc.ArchiveFloat(&m_fBankAngle);
    arc.ArchiveFloat(&m_fTread);
    arc.ArchiveFloat(&m_fTrackWidth);
    arc.ArchiveFloat(&m_fTireFriction);
    arc.ArchiveFloat(&m_fDrag);
    arc.ArchiveFloat(&m_fTireRadius);
    arc.ArchiveFloat(&m_fFrontBrakes);
    arc.ArchiveFloat(&m_fBackBrakes);
    arc.ArchiveFloat(&m_fRollingResistance);
    arc.ArchiveFloat(&m_fTireRotationalSpeed);
    arc.ArchiveFloat(&m_fFrontBrakingForce);
    arc.ArchiveFloat(&m_fBackBrakingForce);
    arc.ArchiveFloat(&m_fBrakingPerformance);
    arc.ArchiveFloat(&m_fLastTurn);
    arc.ArchiveFloat(&m_fTangForce);
    arc.ArchiveFloat(&m_fInertia);
    arc.ArchiveFloat(&m_fDifferentialRatio);
    arc.ArchiveFloat(&m_fGearEfficiency);
    arc.ArchiveFloat(&m_fMaxTraction);
    arc.ArchiveFloat(&m_fTractionForce);
    arc.ArchiveFloat(&m_fAccelerator);
    arc.ArchiveFloat(&m_fTorque);
    arc.ArchiveFloat(&m_fDownForce);
    arc.ArchiveFloat(&m_fUpForce);
    arc.ArchiveFloat(&m_fLeftForce);
    arc.ArchiveFloat(&m_fRightForce);
    arc.ArchiveFloat(&m_fForwardForce);
    arc.ArchiveFloat(&m_fBackForce);
    arc.ArchiveFloat(&m_fBouncyCoef);
    arc.ArchiveFloat(&m_fSpringyCoef);
    arc.ArchiveFloat(&m_fYawMin);
    arc.ArchiveFloat(&m_fYawMax);
    arc.ArchiveFloat(&m_fRollMin);
    arc.ArchiveFloat(&m_fRollMax);
    arc.ArchiveFloat(&m_fZMin);
    arc.ArchiveFloat(&m_fZMax);
    arc.ArchiveFloat(&m_fYawCoef);
    arc.ArchiveFloat(&m_fRollCoef);
    arc.ArchiveFloat(&m_fZCoef);
    arc.ArchiveFloat(&m_fShaderOffset);
    arc.ArchiveFloat(&m_fSoundMinSpeed);
    arc.ArchiveFloat(&m_fSoundMinPitch);
    arc.ArchiveFloat(&m_fSoundMaxSpeed);
    arc.ArchiveFloat(&m_fSoundMaxPitch);
    arc.ArchiveFloat(&m_fVolumeMinSpeed);
    arc.ArchiveFloat(&m_fVolumeMinPitch);
    arc.ArchiveFloat(&m_fVolumeMaxSpeed);
    arc.ArchiveFloat(&m_fVolumeMaxPitch);
    arc.ArchiveFloat(&m_fStopStartDistance);
    arc.ArchiveFloat(&m_fStopStartSpeed);
    arc.ArchiveFloat(&m_fStopEndDistance);
    arc.ArchiveFloat(&m_fSkidAngle);
    arc.ArchiveFloat(&m_fSkidLeftForce);
    arc.ArchiveFloat(&m_fSkidRightForce);
    arc.ArchiveString(&weaponName);
    arc.ArchiveString(&m_sAnimationSet);
    arc.ArchiveString(&m_sSoundSet);
    arc.ArchiveString(&m_sExplosionModel);
    arc.ArchiveVector(&last_origin);
    arc.ArchiveVector(&seatangles);
    arc.ArchiveVector(&driveroffset);
    arc.ArchiveVector(&Corners[0]);
    arc.ArchiveVector(&Corners[1]);
    arc.ArchiveVector(&Corners[2]);
    arc.ArchiveVector(&Corners[3]);
    arc.ArchiveVector(&v_angle);
    arc.ArchiveVector(&yaw_forward);
    arc.ArchiveVector(&yaw_left);
    arc.ArchiveVector(&ground_normal);
    arc.ArchiveVector(&base_angles);
    arc.ArchiveVector(&prev_velocity);
    arc.ArchiveVector(&real_velocity);
    arc.ArchiveVector(&prev_origin);
    arc.ArchiveVector(&real_acceleration);
    arc.ArchiveVector(&prev_acceleration);
    arc.ArchiveVector(&m_vOldMins);
    arc.ArchiveVector(&m_vOldMaxs);
    arc.ArchiveVector(&m_vCG);
    arc.ArchiveVector(&m_vAngularAcceleration);
    arc.ArchiveVector(&m_vAngularVelocity);
    arc.ArchiveVector(&m_vAngles);
    arc.ArchiveVector(&m_vFrontNormal);
    arc.ArchiveVector(&m_vBackNormal);
    arc.ArchiveVector(&m_vFrontAngles);
    arc.ArchiveVector(&m_vBackAngles);
    arc.ArchiveVector(&m_vBaseNormal);
    arc.ArchiveVector(&m_vBaseAngles);
    arc.ArchiveVector(&m_vPrevNormal);
    arc.ArchiveVector(&m_vResistance);
    arc.ArchiveVector(&m_vWheelForce);
    arc.ArchiveVector(&m_vTangForce);
    arc.ArchiveVector(&m_vForce);
    arc.ArchiveVector(&m_vAcceleration);
    arc.ArchiveVector(&m_vOriginOffset);
    arc.ArchiveVector(&m_vOriginOffset2);
    arc.ArchiveVector(&m_vOriginCornerOffset);
    arc.ArchiveVector(&m_vAnglesOffset);
    arc.ArchiveVector(&m_vSaveAngles);
    arc.ArchiveVector(&m_vSkidOrigin);
    arc.ArchiveBoolean(&drivable);
    arc.ArchiveBoolean(&locked);
    arc.ArchiveBoolean(&hasweapon);
    arc.ArchiveBoolean(&showweapon);
    arc.ArchiveBoolean(&steerinplace);
    arc.ArchiveBoolean(&jumpable);
    arc.ArchiveBoolean(&m_bWheelSpinning);
    arc.ArchiveBoolean(&m_bIsSkidding);
    arc.ArchiveBoolean(&m_bIsBraking);
    arc.ArchiveBoolean(&m_bBackSlipping);
    arc.ArchiveBoolean(&m_bFrontSlipping);
    arc.ArchiveBoolean(&m_bAutomatic);
    arc.ArchiveBoolean(&m_bThinkCalled);
    arc.ArchiveBoolean(&m_bRemoveOnDeath);
    arc.ArchiveBoolean(&m_bStopEnabled);
    arc.ArchiveBoolean(&m_bEnableSkidding);
    arc.ArchiveRaw(&vs, sizeof(vehicleState_t));

    if (!arc.Saving()) {
        m_sMoveGrid = new cMoveGrid(3, 3, 1);
    }

    m_sMoveGrid->Archive(arc);

    arc.ArchiveFloat(&m_fIdealSpeed);
    arc.ArchiveVector(&m_vIdealPosition);
    arc.ArchiveVector(&m_vIdealDir);
    arc.ArchiveFloat(&m_fIdealAccel);
    arc.ArchiveFloat(&m_fIdealDistance);
    arc.ArchiveFloat(&m_fLookAhead);
    arc.ArchiveBool(&m_bAutoPilot);
    arc.ArchiveSafePointer(&m_pCollisionEntity);
    ArchiveEnum(m_eSoundState, SOUND_STATE);
    arc.ArchiveFloat(&m_fNextSoundState);
    arc.ArchiveSafePointer(&m_pVehicleSoundEntities[0]);
    arc.ArchiveSafePointer(&m_pVehicleSoundEntities[1]);
    arc.ArchiveSafePointer(&m_pVehicleSoundEntities[2]);
    arc.ArchiveSafePointer(&m_pVehicleSoundEntities[3]);
}

/*
====================
Vehicle::GetSoundSet
====================
*/
str Vehicle::GetSoundSet(void)
{
    return m_sSoundSet;
}

/*
====================
Vehicle::getName
====================
*/
const str& Vehicle::getName() const
{
    return vehicleName;
}

/*
====================
Vehicle::isLocked
====================
*/
qboolean Vehicle::isLocked(void)
{
    return locked;
}

/*
====================
Vehicle::Lock
====================
*/
void Vehicle::Lock(void)
{
    locked = true;
}

/*
====================
Vehicle::UnLock
====================
*/
void Vehicle::UnLock(void)
{
    locked = false;
}

/*
====================
Vehicle::GetCollisionEntity
====================
*/
VehicleCollisionEntity *Vehicle::GetCollisionEntity(void)
{
    return m_pCollisionEntity;
}

CLASS_DECLARATION(Vehicle, DrivableVehicle, "script_drivablevehicle") {
    {&EV_Damage, &Entity::DamageEvent    },
    {&EV_Killed, &DrivableVehicle::Killed},
    {NULL,       NULL                    }
};

/*
====================
DrivableVehicle::DrivableVehicle
====================
*/
DrivableVehicle::DrivableVehicle()
{
    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }

    AddWaitTill(STRING_DEATH);

    drivable = true;
    setMoveType(MOVETYPE_VEHICLE);
    flags |= FL_POSTTHINK | FL_THINK;
}

/*
====================
DrivableVehicle::Killed
====================
*/
void DrivableVehicle::Killed(Event *ev)
{
    Entity      *ent;
    Entity      *attacker;
    Vector       dir;
    Event       *event;
    const char  *name;
    VehicleBase *last;
    int          i;

    deadflag = DEAD_DEAD;
    if (!m_bRemoveOnDeath) {
        Unregister(STRING_DEATH);
        return;
    }

    takedamage = DAMAGE_NO;
    setSolidType(SOLID_NOT);
    hideModel();

    attacker = ev->GetEntity(1);

    //
    // kill the driver.ent
    //
    if (driver.ent) {
        Vector      dir;
        SentientPtr sent;
        Event      *event;

        velocity = vec_zero;
        sent     = static_cast<Sentient *>(driver.ent.Pointer());
        event    = new Event(EV_Use);
        event->AddEntity(sent);
        ProcessEvent(event);
        dir = sent->origin - origin;
        dir[2] += 64;
        dir.normalize();
        sent->Damage(this, this, sent->health * 2, origin, dir, vec_zero, 50, 0, MOD_VEHICLE);
    }

    //
    // kill all passengers
    //
    for (i = 0; i < MAX_PASSENGERS; i++) {
        if (Passengers[i].ent) {
            Vector      dir;
            SentientPtr sent;
            Event      *event;

            velocity = vec_zero;
            sent     = static_cast<Sentient *>(Passengers[i].ent.Pointer());
            event    = new Event(EV_Use);
            event->AddEntity(sent);
            ProcessEvent(event);
            dir = sent->origin - origin;
            dir[2] += 64;
            dir.normalize();
            sent->Damage(this, this, sent->health * 2, origin, dir, vec_zero, 50, 0, MOD_VEHICLE);
        }
    }

    //
    // remove all turrets
    //
    for (i = 0; i < MAX_TURRETS; i++) {
        if (Turrets[i].ent) {
            Turrets[i].ent->PostEvent(EV_Remove, EV_VEHICLE);
        }
    }

    if (flags & FL_DIE_EXPLODE) {
        CreateExplosion(origin, 150 * edict->s.scale, this, this, this);
    }

    if (flags & FL_DIE_GIBS) {
        setSolidType(SOLID_NOT);
        hideModel();

        CreateGibs(this, -150, edict->s.scale, 3);
    }
    //
    // kill all my wheels
    //
    last = this;
    while (last->vlink) {
        last->vlink->PostEvent(EV_Remove, EV_VEHICLE);
        last = last->vlink;
    }

    //
    // kill the killtargets
    //
    name = KillTarget();
    if (name && strcmp(name, "")) {
        for (ent = G_FindTarget(NULL, name); ent; ent = G_FindTarget(ent, name)) {
            ent->PostEvent(EV_Remove, EV_VEHICLE);
        }
    }

    //
    // fire targets
    //
    name = Target();
    if (name && strcmp(name, "")) {
        for (ent = G_FindTarget(NULL, name); ent; ent = G_FindTarget(ent, name)) {
            event = new Event(EV_Activate);
            event->AddEntity(ent);
            ent->ProcessEvent(event);
        }
    }

    PostEvent(EV_Remove, EV_VEHICLE);
    Unregister(STRING_DEATH);
}
