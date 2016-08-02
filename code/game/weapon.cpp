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

// weapon.cpp: Source file for Weapon class.  The weapon class is the base class for
// all weapons in Sin.  Any entity created from a class derived from the weapon
// class will be usable by any Sentient (players and monsters) as a weapon.
//

#include "g_local.h"
#include "entity.h"
#include "item.h"
#include "weapon.h"
#include "scriptmaster.h"
#include "sentient.h"
#include "misc.h"
#include "specialfx.h"
#include "actor.h"
#include "weaputils.h"
#include "player.h"
#include "vehicleturret.h"

Event EV_Weapon_Shoot
   (
   "shoot", 
   EV_DEFAULT,
   "S",
   "mode",
   "Shoot the weapon",
	EV_NORMAL
   );
Event EV_Weapon_DoneRaising
   (
   "ready",
   EV_DEFAULT,
   NULL,
   NULL,
   "Signals the end of the ready animation so the weapon can be used",
	EV_NORMAL
   );
Event EV_Weapon_DoneFiring
   (
   "donefiring",
   EV_DEFAULT,
   NULL,
   NULL,
   "Signals the end of the fire animation",
	EV_NORMAL
   );
Event EV_Weapon_Idle
	(
	"idle",
	EV_DEFAULT,
	NULL,
	NULL,
	"Puts the weapon into an idle state",
	EV_NORMAL
	);
Event EV_Weapon_IdleInit
	(
	"idleinit",
	EV_DEFAULT,
	NULL,
	NULL,
	"Puts the weapon into an idle state and clears all the anim slots",
	EV_NORMAL
	);
Event EV_Weapon_SecondaryUse
   (
   "secondaryuse",
   EV_DEFAULT,
   NULL,
   NULL,
   "Puts the weapon into its secondary mode of operation",
	EV_NORMAL
   );
Event EV_Weapon_DoneReloading
   (
   "donereloading",
   EV_DEFAULT,
   NULL,
   NULL,
   "Signals the end of the reload animation",
	EV_NORMAL
   );
Event EV_Weapon_SetAmmoClipSize
   (
   "clipsize",
   EV_DEFAULT,
   "i",
   "ammoClipSize",
   "Set the amount of rounds a clip of the weapon holds",
	EV_NORMAL
   );
Event EV_Weapon_SetAmmoInClip
   (
   "ammo_in_clip",
   EV_DEFAULT,
   "i",
   "ammoInClip",
   "Set the amount of ammo in the clip",
	EV_NORMAL
   );
Event EV_Weapon_SetShareClip
   (
   "shareclip", 
   EV_DEFAULT,
   "i",
   "shareClip",
   "Sets the weapon to share the same clip between all fire modes",
	EV_NORMAL
   );
Event EV_Weapon_FillClip
	(
	"clip_fill",
	EV_DEFAULT,
	NULL,
	NULL,
	"Fills the weapons ammo clip with ammo from its owner",
	EV_NORMAL
	);
Event EV_Weapon_EmptyClip
	(
	"clip_empty",
	EV_DEFAULT,
	NULL,
	NULL,
	"Empties the weapon's clip of ammo, returning it to the owner",
	EV_NORMAL
	);
Event EV_Weapon_AddClip
	(
	"clip_add",
	EV_DEFAULT,
	"i",
	"ammoAmount",
	"Add to the weapons ammo clip with ammo from its owner",
	EV_NORMAL
	);
Event EV_Weapon_SetMaxRange
   (
   "maxrange",
   EV_DEFAULT,
   "f",
   "maxRange",
   "Set the maximum range of a weapon so the AI knows how to use it",
	EV_NORMAL
   );
Event EV_Weapon_SetMinRange
   (
   "minrange",
   EV_DEFAULT,
   "f",
   "minRange",
   "Set the minimum range of a weapon so the AI knows how to use it",
	EV_NORMAL
   );
Event EV_Weapon_SetFireDelay
	(
	"firedelay",
	EV_DEFAULT,
	"f",
	"fFireDelay",
	"Set the minimum time between shots from the weapon",
	EV_NORMAL
	);
Event EV_Weapon_DMSetFireDelay
	(
	"dmfiredelay",
	EV_DEFAULT,
	"f",
	"fFireDelay",
	"Set the minimum time between shots from the weapon",
	EV_NORMAL
	);
Event EV_Weapon_NotDroppable
(
	"notdroppable",
	EV_DEFAULT,
	NULL,
	NULL,
	"Makes a weapon not droppable"
);
Event EV_Weapon_SetAimAnim
   (
   "setaimanim",
   EV_DEFAULT,
   "si",
   "aimAnimation aimFrame",
   "Set the aim animation and frame for when a weapon fires",
	EV_NORMAL
   );
Event EV_Weapon_SetRank
(
	"rank",
	EV_DEFAULT,
	"ii",
	"iOrder iRank",
	"Set the order value and power ranking for the weapon"
);
Event EV_Weapon_SetFireType
   (
   "firetype",
   EV_DEFAULT,
   "s",
   "firingType",
   "Set the firing type of the weapon (projectile or bullet)",
	EV_NORMAL
   );
Event EV_Weapon_SetAIRange
	(
	"airange",
	EV_DEFAULT,
	"s",
	"airange",
	"Set the range of this gun for the ai: short, medium, long, sniper",
	EV_NORMAL
	);
Event EV_Weapon_SetProjectile
   (
   "projectile",
   EV_DEFAULT,
   "s",
   "projectileModel",
   "Set the model of the projectile that this weapon fires",
	EV_NORMAL
   );
Event EV_Weapon_SetDMProjectile
   (
   "dmprojectile",
   EV_DEFAULT,
   "s",
   "projectileModel",
   "Set the model of the projectile that this weapon fires",
	EV_NORMAL
   );
Event EV_Weapon_SetBulletDamage
   (
   "bulletdamage",
   EV_DEFAULT,
   "f",
   "bulletDamage",
   "Set the damage that the bullet causes",
	EV_NORMAL
   );
Event EV_Weapon_SetDMBulletDamage
   (
   "dmbulletdamage",
   EV_DEFAULT,
   "f",
   "bulletDamage",
   "Set the damage that the bullet causes",
	EV_NORMAL
   );
Event EV_Weapon_SetBulletKnockback
   (
   "bulletknockback",
   EV_DEFAULT,
   "f",
   "bulletKnockback",
   "Set the knockback that the bullet causes",
	EV_NORMAL
   );
Event EV_Weapon_SetDMBulletKnockback
   (
   "dmbulletknockback",
   EV_DEFAULT,
   "f",
   "bulletKnockback",
   "Set the knockback that the bullet causes",
	EV_NORMAL
   );
Event EV_Weapon_SetBulletCount
   (
   "bulletcount",
   EV_DEFAULT,
   "f",
   "bulletCount",
   "Set the number of bullets this weapon shoots when fired",
	EV_NORMAL
   );
Event EV_Weapon_SetDMBulletCount
   (
   "dmbulletcount",
   EV_DEFAULT,
   "f",
   "bulletCount",
   "Set the number of bullets this weapon shoots when fired",
	EV_NORMAL
   );
Event EV_Weapon_SetBulletRange
   (
   "bulletrange",
   EV_DEFAULT,
   "f",
   "bulletRange",
   "Set the range of the bullets",
	EV_NORMAL
   );
Event EV_Weapon_SetDMBulletRange
   (
   "dmbulletrange",
   EV_DEFAULT,
   "f",
   "bulletRange",
   "Set the range of the bullets",
	EV_NORMAL
   );
Event EV_Weapon_SetBulletSpread
   (
   "bulletspread",
   EV_DEFAULT,
   "ffFF",
   "bulletSpreadX bulletSpreadY bulletSpreadXmax bulletSpreadYmax",
   "Set the min & optional max spread of the bullet in the x and y axis",
	EV_NORMAL
   );
Event EV_Weapon_SetDMBulletSpread
   (
   "dmbulletspread",
   EV_DEFAULT,
   "ffFF",
   "bulletSpreadX bulletSpreadY bulletSpreadXmax bulletSpreadYmax",
   "Set the min & optional max spread of the bullet in the x and y axis",
	EV_NORMAL
   );
Event EV_Weapon_SetZoomSpreadMult
	(
	"zoomspreadmult",
	EV_DEFAULT,
	"f",
	"scale",
	"Sets the spread multiplier for when using the zoom on a zooming weapon",
	EV_NORMAL
	);
Event EV_Weapon_SetDMZoomSpreadMult
	(
	"dmzoomspreadmult",
	EV_DEFAULT,
	"f",
	"scale",
	"Sets the spread multiplier for when using the zoom on a zooming weapon",
	EV_NORMAL
	);
Event EV_Weapon_SetFireSpreadMult
	(
	"firespreadmult",
	EV_DEFAULT,
	"ffff",
	"scaleadd falloff cap maxtime",
	"Sets a time decayed multiplyer to spread when the weapon is fired",
	EV_NORMAL
	);
Event EV_Weapon_SetDMFireSpreadMult
	(
	"dmfirespreadmult",
	EV_DEFAULT,
	"ffff",
	"scaleadd falloff cap maxtime",
	"Sets a time decayed multiplyer to spread when the weapon is fired",
	EV_NORMAL
	);
Event EV_Weapon_SetTracerFrequency
(
	"tracerfrequency",
	EV_DEFAULT,
	"f",
	"frequenct",
	"Set the frequency of making tracers"
);
Event EV_Weapon_SetRange
   (
   "range",
   EV_DEFAULT,
   "f",
   "range",
   "Set the range of the weapon",
	EV_NORMAL
   );
Event EV_Weapon_Secondary
   (
   "secondary",
   EV_DEFAULT,
   "SSSSSSSS",
   "arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8",
   "Set the secondary mode of the weapon, by passing commands through",
	EV_NORMAL
   );
Event EV_Weapon_AmmoType
   (
   "ammotype",
   EV_DEFAULT,
   "s",
   "name",
   "Set the type of ammo this weapon uses",
	EV_NORMAL
   );
Event EV_Weapon_StartAmmo
   (
   "startammo",
   EV_DEFAULT,
   "i",
   "amount",
   "Set the starting ammo of this weapon",
	EV_NORMAL
   );
Event EV_Weapon_DMStartAmmo
   (
   "dmstartammo",
   EV_DEFAULT,
   "i",
   "amount",
   "Set the starting ammo of this weapon",
	EV_NORMAL
   );
Event EV_Weapon_AmmoRequired
   (
   "ammorequired",
   EV_DEFAULT,
   "i",
   "amount",
   "Set the amount of ammo this weapon requires to fire",
	EV_NORMAL
   );
Event EV_Weapon_DMAmmoRequired
   (
   "dmammorequired",
   EV_DEFAULT,
   "i",
   "amount",
   "Set the amount of ammo this weapon requires to fire",
	EV_NORMAL
   );
Event EV_Weapon_MaxChargeTime
   (
   "maxchargetime",
   EV_DEFAULT,
   "i",
   "time",
   "Set the maximum time the weapon may be charged up",
	EV_NORMAL
   );
Event EV_Weapon_MinChargeTime
   (
   "minchargetime",
   EV_DEFAULT,
   "i",
   "time",
   "Set the minimum time the weapon must be charged up",
	EV_NORMAL
   );
Event EV_Weapon_GiveStartingAmmo
   (
   "startingammotoowner",
   EV_DEFAULT,
   NULL,
   NULL,
   "Internal event used to give ammo to the owner of the weapon",
	EV_NORMAL
   );
Event EV_Weapon_AutoAim
   (
   "autoaim",
   EV_DEFAULT,
   NULL,
   NULL,
   "Turn on auto aiming for the weapon",
	EV_NORMAL
   );
Event EV_Weapon_Crosshair
   (
   "crosshair",
   EV_DEFAULT,
   "b",
   "bool",
   "Turn on/off the crosshair for this weapon",
	EV_NORMAL
   );
Event EV_Weapon_DMCrosshair
   (
   "dmcrosshair",
   EV_DEFAULT,
   "b",
   "bool",
   "Turn on/off the crosshair for this weapon",
	EV_NORMAL
   );
Event EV_Weapon_SetQuiet
   (
   "quiet",
   EV_DEFAULT,
   NULL,
   NULL,
   "Makes the weapon make no noise.",
	EV_NORMAL
   );
Event EV_Weapon_SetLoopFire
   (
   "loopfire",
   EV_DEFAULT,
   NULL,
   NULL,
   "Makes the weapon fire by looping the fire animation.",
	EV_NORMAL
   );
Event EV_Weapon_OffHandAttachToTag
   (
   "offhandattachtotag",
   EV_DEFAULT,
   "s",
   "tagname",
   "Set the name of the tag to attach this to it's owner's off hand.",
	EV_NORMAL
   );
Event EV_Weapon_MainAttachToTag
   (
   "mainattachtotag",
   EV_DEFAULT,
   "s",
   "tagname",
   "Set the name of the tag to attach this to it's owner when being used.",
	EV_NORMAL
   );
Event EV_Weapon_HolsterTag
   (
   "holstertag",
   EV_DEFAULT,
   "s",
   "tagname",
   "Set the name of the tag to attach this to when the weapon is holstered.",
	EV_NORMAL
   );
Event EV_Weapon_HolsterOffset
   (
   "holsteroffset",
   EV_DEFAULT,
   "v",
   "offset",
   "Set the positional offset when it is holstered",
	EV_NORMAL
   );
Event EV_Weapon_HolsterAngles
   (
   "holsterangles",
   EV_DEFAULT,
   "v",
   "angles",
   "Set the angles of this weapon when it is holstered",
	EV_NORMAL
   );
Event EV_Weapon_HolsterScale
   (
   "holsterscale",
   EV_DEFAULT,
   "f",
   "scale",
   "Set the scale of this weapon when it is holstered",
	EV_NORMAL
   );
Event EV_Weapon_AutoPutaway
   (
   "autoputaway",
   EV_DEFAULT,
   "b",
   "bool",
   "Set the weapon to be automatically put away when out of ammo",
	EV_NORMAL
   );
Event EV_Weapon_UseNoAmmo
   (
   "usenoammo",
   EV_DEFAULT,
   "b",
   "bool",
   "Set the weapon to be able to be used when it's out of ammo",
	EV_NORMAL
   );
Event EV_Weapon_SetMeansOfDeath
   (
   "meansofdeath",
   EV_DEFAULT,
   "s",
   "meansOfDeath",
   "Set the meansOfDeath of the weapon.",
	EV_NORMAL
   );
Event EV_Weapon_SetWorldHitSpawn
   (
   "worldhitspawn",
   EV_DEFAULT,
   "s",
   "modelname",
   "Set a model to be spawned when the weapon strikes the world->",
	EV_NORMAL
   );
Event EV_Weapon_MakeNoise
   (
   "makenoise",
   EV_DEFAULT,
   "FB",
   "noise_radius force",
   "Makes the weapon make noise that actors can hear.",
	EV_NORMAL
   );
Event EV_Weapon_SetWeaponType
	(
	"weapontype",
	EV_DEFAULT,
	"s",
	"weapon_type",
	"Sets the weapon type",
	EV_NORMAL
	);
Event EV_Weapon_SetWeaponGroup
	(
	"weapongroup",
	EV_DEFAULT,
	"s",
	"weapon_group",
	"Sets the weapon group, a set of animations for actor animations scripts to use",
	EV_NORMAL
	);
Event EV_Weapon_SetZoom
(
	"zoom",
	EV_DEFAULT,
	"iI",
	"zoomfov autozoom",
	"Sets fov to zoom to on a secondary fire"
);
Event EV_Weapon_SetSemiAuto
(
	"semiauto",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets the weapon to fire semi-auto"
);
Event EV_Weapon_AttachToHand
(
	"attachtohand",
	EV_DEFAULT,
	"s",
	"weapon_hand",
	"Attaches an active weapon to the specified hand"
);
Event EV_Weapon_CantPartialReload
(
	"cantpartialreload",
	EV_DEFAULT,
	NULL,
	NULL,
	"Prevents the weapon from being reloaded part way through a clip"
);
Event EV_Weapon_DMCantPartialReload
(
	"dmcantpartialreload",
	EV_DEFAULT,
	NULL,
	NULL,
	"Prevents the weapon from being reloaded part way through a clip for DM"
);
Event EV_Weapon_FallingAngleAdjust
(
	"fallingangleadjust",
	EV_DEFAULT,
	0,
	0,
	"Adjusts the weapons angles as it falls to the ground"
);
Event EV_Weapon_SetViewKick
(
	"viewkick",
	EV_DEFAULT,
	"ffFF",
	"pitchmin pitchmax yawmin yawmax",
	"Adds kick to the view of the owner when fired."
);
Event EV_Weapon_MovementSpeed
(
	"movementspeed",
	EV_DEFAULT,
	"f",
	"speedmult",
	"Alters the movement speed of the player when he has the weapon out"
);
Event EV_Weapon_DMMovementSpeed
(
	"dmmovementspeed",
	EV_DEFAULT,
	"f",
	"speedmult",
	"Alters the movement speed of the player when he has the weapon out"
);
Event EV_Weapon_AmmoPickupSound
(
	"ammopickupsound",
	EV_DEFAULT,
	"s",
	"name",
	"sets the weapon's ammo pickup sound alias"
);
Event EV_Weapon_NoAmmoSound
(
	"noammosound",
	EV_DEFAULT,
	"s",
	"name",
	"sets the weapon's dry fire sound alias"
);

CLASS_DECLARATION( Item, Weapon, NULL )
{
	{ &EV_SetAnim,								&Weapon::SetWeaponAnimEvent },
	{ &EV_Item_Pickup,							&Weapon::PickupWeapon },
	{ &EV_Weapon_DoneRaising,					&Weapon::DoneRaising },
	{ &EV_Weapon_DoneFiring,					&Weapon::DoneFiring },
	{ &EV_Weapon_Idle,							&Weapon::Idle },
	{ &EV_Weapon_IdleInit,						&Weapon::IdleInit },
	{ &EV_BroadcastAIEvent,		         		&Weapon::WeaponSound },
	{ &EV_Weapon_DoneReloading,					&Weapon::DoneReloading },
	{ &EV_Weapon_SetAmmoClipSize,				&Weapon::SetAmmoClipSize },
	{ &EV_Weapon_SetAmmoInClip,					&Weapon::SetAmmoInClip },
	{ &EV_Weapon_SetShareClip,					&Weapon::SetShareClip },
	{ &EV_Weapon_FillClip,						&Weapon::EventClipFill },
	{ &EV_Weapon_EmptyClip,						&Weapon::EventClipEmpty },
	{ &EV_Weapon_AddClip,						&Weapon::EventClipAdd },
	{ &EV_Weapon_SetMaxRange,					&Weapon::SetMaxRangeEvent },
	{ &EV_Weapon_SetMinRange,					&Weapon::SetMinRangeEvent },
	{ &EV_Weapon_SetFireDelay,					&Weapon::EventSetFireDelay },
	{ &EV_Weapon_NotDroppable,    				&Weapon::NotDroppableEvent },
	{ &EV_Weapon_SetAimAnim,					&Weapon::SetAimAnim },
	{ &EV_Weapon_Shoot,							&Weapon::Shoot },
	{ &EV_Weapon_SetRank,						&Weapon::SetRankEvent },
	{ &EV_Weapon_SetFireType,					&Weapon::SetFireType },
	{ &EV_Weapon_SetAIRange,					&Weapon::SetAIRange },
	{ &EV_Weapon_SetProjectile,					&Weapon::SetProjectile },
	{ &EV_Weapon_SetDMProjectile,				&Weapon::SetDMProjectile },
	{ &EV_Weapon_SetBulletDamage,				&Weapon::SetBulletDamage },
	{ &EV_Weapon_SetBulletCount,				&Weapon::SetBulletCount },
	{ &EV_Weapon_SetBulletKnockback,			&Weapon::SetBulletKnockback },
	{ &EV_Weapon_SetBulletRange,				&Weapon::SetBulletRange },
	{ &EV_Weapon_SetRange,						&Weapon::SetRange },
	{ &EV_Weapon_SetBulletSpread,				&Weapon::SetBulletSpread },
	{ &EV_Weapon_SetTracerFrequency,			&Weapon::SetTracerFrequency },
	{ &EV_Weapon_Secondary,						&Weapon::Secondary },
	{ &EV_Weapon_AmmoType,						&Weapon::SetAmmoType },
	{ &EV_Weapon_StartAmmo,						&Weapon::SetStartAmmo },
	{ &EV_Weapon_AmmoRequired,					&Weapon::SetAmmoRequired },
	{ &EV_Weapon_MaxChargeTime,					&Weapon::SetMaxChargeTime },
	{ &EV_Weapon_MinChargeTime,					&Weapon::SetMinChargeTime },
	{ &EV_Weapon_GiveStartingAmmo,				&Weapon::GiveStartingAmmoToOwner },
	{ &EV_Weapon_AutoAim,						&Weapon::AutoAim },
	{ &EV_Weapon_Crosshair,						&Weapon::Crosshair },
	{ &EV_Weapon_MainAttachToTag,				&Weapon::MainAttachToTag },
	{ &EV_Weapon_OffHandAttachToTag,			&Weapon::OffHandAttachToTag },
	{ &EV_Weapon_HolsterTag,					&Weapon::HolsterAttachToTag },
	{ &EV_Weapon_HolsterOffset,					&Weapon::SetHolsterOffset },
	{ &EV_Weapon_HolsterAngles,					&Weapon::SetHolsterAngles },
	{ &EV_Weapon_HolsterScale,					&Weapon::SetHolsterScale },
	{ &EV_Weapon_SetQuiet,						&Weapon::SetQuiet },
	{ &EV_Weapon_SetLoopFire,					&Weapon::SetLoopFire },
	{ &EV_Weapon_AutoPutaway,					&Weapon::SetAutoPutaway },
	{ &EV_Weapon_UseNoAmmo,						&Weapon::SetUseNoAmmo },
	{ &EV_Weapon_SetMeansOfDeath,				&Weapon::SetMeansOfDeath },
	{ &EV_Weapon_SetWorldHitSpawn,				&Weapon::SetWorldHitSpawn },
	{ &EV_Weapon_MakeNoise,						&Weapon::MakeNoise },
	{ &EV_Weapon_SetWeaponType,					&Weapon::SetWeaponType },
	{ &EV_Weapon_SetWeaponGroup,				&Weapon::SetWeaponGroup },
	{ &EV_Weapon_SetZoom,						&Weapon::SetZoom },
	{ &EV_Weapon_SetSemiAuto,					&Weapon::SetSemiAuto },
	{ &EV_Weapon_AttachToHand,					&Weapon::AttachToHand },
	{ &EV_Weapon_CantPartialReload,				&Weapon::SetCantPartialReload },
	{ &EV_Weapon_FallingAngleAdjust,			&Weapon::FallingAngleAdjust },
	{ &EV_Weapon_SetViewKick, 					&Weapon::SetViewKick },
	{ &EV_Weapon_DMSetFireDelay,				&Weapon::EventSetDMFireDelay },
	{ &EV_Weapon_SetDMBulletDamage,				&Weapon::SetDMBulletDamage },
	{ &EV_Weapon_SetDMBulletCount,				&Weapon::SetDMBulletCount },
	{ &EV_Weapon_SetDMBulletSpread,				&Weapon::SetDMBulletSpread },
	{ &EV_Weapon_SetZoomSpreadMult,				&Weapon::SetZoomSpreadMult },
	{ &EV_Weapon_SetDMZoomSpreadMult,			&Weapon::SetDMZoomSpreadMult },
	{ &EV_Weapon_SetFireSpreadMult,				&Weapon::SetFireSpreadMult },
	{ &EV_Weapon_SetDMFireSpreadMult,			&Weapon::SetDMFireSpreadMult },
	{ &EV_Weapon_DMAmmoRequired, 				&Weapon::SetDMAmmoRequired },
	{ &EV_Weapon_DMCantPartialReload,			&Weapon::SetDMCantPartialReload },
	{ &EV_Weapon_DMStartAmmo, 					&Weapon::SetDMStartAmmo },
	{ &EV_Weapon_SetDMBulletRange,				&Weapon::SetDMBulletRange },
	{ &EV_Weapon_DMCrosshair,					&Weapon::DMCrosshair },
	{ &EV_Weapon_MovementSpeed,					&Weapon::SetMovementSpeed },
	{ &EV_Weapon_DMMovementSpeed,				&Weapon::SetDMMovementSpeed },
	{ &EV_Weapon_AmmoPickupSound,				&Weapon::EventAmmoPickupSound },
	{ &EV_Weapon_NoAmmoSound,					&Weapon::EventNoAmmoSound },
	{ NULL, NULL }
};

//======================
//Weapon::Weapon
//======================
Weapon::Weapon()
{
	entflags |= EF_WEAPON;

	mAIRange = RANGE_SHORT;

	if( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}

	// Owner of the weapon
	owner = NULL;

	// Starting rank of the weapon
	rank = 0;

	// Amount of ammo required for weapon
	INITIALIZE_WEAPONMODE_VAR( ammorequired, 0 );

	// Starting ammo of the weapon
	INITIALIZE_WEAPONMODE_VAR( startammo, 0 );

	// Amount of ammo the clip can hold
	INITIALIZE_WEAPONMODE_VAR( ammo_clip_size, 0 );

	// Amount of ammo in clip
	INITIALIZE_WEAPONMODE_VAR( ammo_in_clip, 0 );

	// Amount of time to pass before broadcasting a weapon sound again
	nextweaponsoundtime = 0;

	// The initial state of the weapon
	weaponstate = WEAPON_HOLSTERED;

	// Is the weapon droppable when the owner is killed
	notdroppable = false;

	// Aim animation for behavior of monsters
	aimanim = -1;
	aimframe = 0;

	m_iAnimSlot = 0;

	// start off unattached
	attached = false;

	// maximum effective firing distance (for autoaim)
	maxrange = 1000;

	// minimum safe firing distance (for AI)
	minrange = 0;

	// speed of the projectile (0 == infinite speed)
	memset( projectilespeed, 0, sizeof( projectilespeed ) );

	// default action_level_increment
	INITIALIZE_WEAPONMODE_VAR( action_level_increment, 2 );

	// Weapons don't move
	setMoveType( MOVETYPE_NONE );

	m_fLastFireTime = 0;

	// What type of ammo this weapon fires
	INITIALIZE_WEAPONMODE_VAR( firetype, ( firetype_t )0 );

	INITIALIZE_WEAPONMODE_VAR( firedelay, 0.1f );

	// Init the bullet specs
	INITIALIZE_WEAPONMODE_VAR( projectilespeed, 0 );
	INITIALIZE_WEAPONMODE_VAR( bulletdamage, 0 );
	INITIALIZE_WEAPONMODE_VAR( bulletcount, 1 );
	INITIALIZE_WEAPONMODE_VAR( bulletrange, 1024 );
	INITIALIZE_WEAPONMODE_VAR( bulletknockback, 0 );
	INITIALIZE_WEAPONMODE_VAR( ammo_type, "" );
	INITIALIZE_WEAPONMODE_VAR( loopfire, false );
	INITIALIZE_WEAPONMODE_VAR( quiet, qfalse );
	INITIALIZE_WEAPONMODE_VAR( loopfire, qfalse );
	INITIALIZE_WEAPONMODE_VAR( tracercount, 0 );
	INITIALIZE_WEAPONMODE_VAR( tracerfrequency, 0 );

	for( int i = 0; i < MAX_FIREMODES; i++ )
	{
		INITIALIZE_WEAPONMODE_VAR( viewkickmin[ i ], 0 );
		INITIALIZE_WEAPONMODE_VAR( viewkickmax[ i ], 0 );
	}

	INITIALIZE_WEAPONMODE_VAR( m_fFireSpreadMultAmount, 0 );
	INITIALIZE_WEAPONMODE_VAR( m_fFireSpreadMultFalloff, 0 );
	INITIALIZE_WEAPONMODE_VAR( m_fFireSpreadMultCap, 0 );
	INITIALIZE_WEAPONMODE_VAR( m_fFireSpreadMultTime, 0 );
	INITIALIZE_WEAPONMODE_VAR( m_fFireSpreadMult, 0 );

	// Init the max amount of time a weapon may be charged (5 seconds)
	INITIALIZE_WEAPONMODE_VAR( min_charge_time, 0 );
	INITIALIZE_WEAPONMODE_VAR( max_charge_time, 5 );
	charge_fraction = 1.0f;

	// Tag to attach this weapon to on its owner when used in the left hand and in the right hand
	attachToTag_offhand = "tag_weapon_left";
	attachToTag_main = "tag_weapon_right";

	// putaway is flagged true when the weapon should be put away by state machine
	putaway = false;

	// This is used for setting alternate fire functionality when initializing stuff
	firemodeindex = FIRE_PRIMARY;

	// Name and index
	setName( "Unnamed Weapon" );

	m_fZoomSpreadMult = 1.0f;
	m_bCanPartialReload = qtrue;
	m_bShareClip = qfalse;

	// do better lighting on all weapons
	edict->s.renderfx |= RF_EXTRALIGHT;

	// Weapons do not auto aim automatically
	autoaim = false;

	// No crosshair visible
	crosshair = false;

	m_bAutoZoom = false;
	m_iZoom = 0;
	m_bSemiAuto = false;
	m_bShouldReload = false;

	// Set the stats
	m_iNumHits = 0;
	m_iNumGroinShots = 0;
	m_iNumHeadShots = 0;
	m_iNumLeftArmShots = 0;
	m_iNumRightArmShots = 0;
	m_iNumLeftLegShots = 0;
	m_iNumRightLegShots = 0;
	m_iNumTorsoShots = 0;

	m_fMovementSpeed = 1.0f;

	m_sAmmoPickupSound = "snd_pickup_";
	m_NoAmmoSound = "snd_noammo";

	// Set the default weapon group
	m_csWeaponGroup = STRING_EMPTY;

	// Weapons default to making noise
	next_noise_time = 0;
	next_noammo_time = 0;

	// Used to keep track of last angles and scale before holstering
	lastValid = qfalse;
	lastScale = 1.0f;
	holsterScale = 1.0f;

	// Weapon will not be putaway by default when out of ammo
	auto_putaway = qfalse;

	// Weapon will be able to be used when it has no ammo
	use_no_ammo = qtrue;

	INITIALIZE_WEAPONMODE_VAR( meansofdeath, MOD_NONE );

	PostEvent( EV_Weapon_IdleInit, 0 );
}


//======================
//Weapon::Weapon
//======================
Weapon::Weapon
   (
   const char *file
   )

   {
   // The tik file holds all the information available for a weapon   
   Weapon();
   }

//======================
//Weapon::~Weapon
//======================
Weapon::~Weapon()
{
	DetachGun();
	entflags &= ~EF_WEAPON;
}

//======================
//Weapon::Delete
//======================
void Weapon::Delete
	(
	void
	)

{
	if( g_iInThinks )
	{
		DetachGun();

		if( owner )
			RemoveFromOwner();

		PostEvent( EV_Remove, 0 );
	}
	else
	{
		delete this;
	}
}

//======================
//Weapon::GetScriptOwner
//======================
Listener *Weapon::GetScriptOwner
	(
	void
	)

{
	return owner;
}

//======================
//Weapon::GetRank
//======================
int Weapon::GetRank
	(
	void
	)

	{
	return rank;
	}

//======================
//Weapon::GetOrder
//======================
int Weapon::GetOrder
	(
	void
	)

	{
	return order;
	}

//======================
//Weapon::SetRank
//======================
void Weapon::SetRank
	(
	int order,
	int rank
	)

{
	this->order = order;
	this->rank = rank;
}

//======================
//Weapon::SetRankEvent
//======================
void Weapon::SetRankEvent
	(
	Event *ev
	)

{
	SetRank( ev->GetInteger( 1 ), ev->GetInteger( 2 ) );
}

//======================
//Weapon::SetAutoPutaway
//======================
void Weapon::SetAutoPutaway
	(
	Event *ev
	)

{
	auto_putaway = ev->GetBoolean( 1 );
}

//======================
//Weapon::SetUseNoAmmo
//======================
void Weapon::SetUseNoAmmo
	(
	Event *ev
	)

{
	use_no_ammo = ev->GetBoolean( 1 );
}

//======================
//Weapon::SetStartAmmo
//======================
void Weapon::SetStartAmmo
   (
   Event *ev
   )

{
	if( g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	startammo[firemodeindex] = ev->GetInteger( 1 );
}

//======================
//Weapon::SetDMStartAmmo
//======================
void Weapon::SetDMStartAmmo
   (
   Event *ev
   )

{
	if( !g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	startammo[firemodeindex] = ev->GetInteger( 1 );
}

//======================
//Weapon::SetMaxChargeTime
//======================
void Weapon::SetMaxChargeTime
   (
   Event *ev
   )

   {
   assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
   max_charge_time[firemodeindex] = ev->GetFloat( 1 );
}

//======================
//Weapon::SetMaxChargeTime
//======================
void Weapon::SetMinChargeTime
(
Event *ev
)

{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	min_charge_time[ firemodeindex ] = ev->GetFloat( 1 );
}

//======================
//Weapon::GetMinChargeTime
//======================
float Weapon::GetMinChargeTime
	(
	firemode_t mode
	)

{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	return min_charge_time[ mode ];
}

//======================
//Weapon::GetMinChargeTime
//======================
float Weapon::GetMaxChargeTime
	(
	firemode_t mode
	)

{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	return max_charge_time[ mode ];
}

//======================
//Weapon::SetAmmoRequired
//======================
void Weapon::SetAmmoRequired
   (
   Event *ev
   )

{
	if( g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	ammorequired[firemodeindex] = ev->GetInteger( 1 );
}

//======================
//Weapon::SetDMAmmoRequired
//======================
void Weapon::SetDMAmmoRequired
(
Event *ev
)

{
	if( !g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	ammorequired[ firemodeindex ] = ev->GetInteger( 1 );
}

//======================
//Weapon::GetStartAmmo
//======================
int Weapon::GetStartAmmo
	(
	firemode_t mode
	)

{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );
	return startammo[ mode ];
}

//======================
//Weapon::GetAmmoType
//======================
str Weapon::GetAmmoType
(
firemode_t mode
)

{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );

	mode = m_bShareClip ? FIRE_PRIMARY : mode;
	return ammo_type[ mode ];
}

//======================
//Weapon::SetAmmoType
//======================
void Weapon::SetAmmoType
   (
   Event *ev
   )

   {
   assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );

   if ( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) )
      ammo_type[firemodeindex] = ev->GetString( 1 );
   else
      {
      warning( "Weapon::SetAmmoType", "Invalid mode %d\n", firemodeindex );
      return;
      }
   }

//======================
//Weapon::SetAmmoAmount
//======================
void Weapon::SetAmmoAmount
	(
	int amount,
	firemode_t mode
	)

{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );

	mode = m_bShareClip ? FIRE_PRIMARY : mode;

	// If the clip can hold ammo, then set the amount in the clip to the specified amount
	if( ammo_clip_size[ mode ] )
		ammo_in_clip[ mode ] = amount;
}

//======================
//Weapon::GetClipSize
//======================
int Weapon::GetClipSize
	(
	firemode_t mode
	)

{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );

	mode = m_bShareClip ? FIRE_PRIMARY : mode;

	if( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) )
		return ammo_clip_size[ mode ];
	else
	{
		warning( "Weapon::GetClipSize", "Invalid mode %d\n", mode );
		return 0;
	}
}

//======================
//Weapon::UseAmmo
//======================
void Weapon::UseAmmo
	(
	int amount,
	firemode_t mode
	)

{
	mode = m_bShareClip ? FIRE_PRIMARY : mode;

	if( UnlimitedAmmo( mode ) )
		return;

	// Remove ammo from the clip if it's available
	if( ammo_clip_size[ mode ] )
	{
		ammo_in_clip[ mode ] -= amount;
		if( ammo_in_clip[ mode ] < 0 )
		{
			warning( "UseAmmo", "Used more ammo than in clip.\n" );
			ammo_in_clip[ mode ] = 0;
		}
		if( !ammo_in_clip[ mode ] )
			SetShouldReload( qtrue );
		owner->AmmoAmountInClipChanged( ammo_type[ mode ], ammo_in_clip[ mode ] );
	}
	else
	{
		assert( owner );
		if( owner && owner->isClient() && !UnlimitedAmmo( mode ) )
		{
			// Remove ammo from the player's inventory
			owner->UseAmmo( ammo_type[ mode ], ammorequired[ mode ] );
		}
	}
}

/*
//======================
//Weapon::GetMuzzlePosition
//======================
void Weapon::GetMuzzlePosition
	(
   Vector *position,
	Vector *forward,
	Vector *right,
	Vector *up
	)

	{
   orientation_t  weap_or, barrel_or, orient;
   Vector         pos;
   vec3_t         mat[3]={0,0,0,0,0,0,0,0,0};
   vec3_t         orient[3];
   int            i, mi, tagnum;
   
	assert( owner );

	// We should always have an owner
	if ( !owner )
		{
		return;
		}

   // Get the owner's weapon orientation ( this is custom code and doesn't use the GetTag function
   // because we need to use the saved off fire_frame and fire_animation indexes from the owner
   mi = owner->edict->s.modelindex;
   
   tagnum = gi.Tag_NumForName( mi, current_attachToTag.c_str() );

   if ( tagnum < 0 )
      {
      warning( "Weapon::GetMuzzlePosition", "Could not find tag \"%s\"", current_attachToTag.c_str() );
      pos = owner->centroid;
      AnglesToAxis( owner->angles, mat );
      goto out;
      }

   // Get the orientation based on the frame and anim stored off in the owner.
   // This is to prevent weird timing with getting orientations on different frames of firing
   // animations and the orientations will not be consistent.
   
   AnglesToAxis( owner->angles, owner->orientation );

   orient = gi.Tag_OrientationEx( mi,
                              owner->CurrentAnim( legs ),
                              owner->CurrentFrame( legs ),
                              tagnum & TAG_MASK, 
                              owner->edict->s.scale,
                              owner->edict->s.bone_tag,
                              owner->edict->s.bone_quat,
                              0,
                              0, 
                              1.0f,
                              ( owner->edict->s.anim & ANIM_BLEND ) != 0, 
                              ( owner->edict->s.torso_anim & ANIM_BLEND ) != 0, 
                              owner->GetFiringAnim(),
                              owner->GetFiringFrame(),
                              0,
                              0,
                              1.0f
                            );
   
   // Transform the weapon's orientation through the owner's orientation
   // Player orientation is normally based on the player's view, but we need
   // it to be based on the model's orientation, so we calculate it here.
   AnglesToAxis( owner->angles, orient );
   VectorCopy( owner->origin, weap_or.origin );
   for ( i=0;  i<3; i++ ) 
      {
      VectorMA( weap_or.origin, orient.origin[i], orient[i], weap_or.origin );
      }

   MatrixMultiply( orient.axis, orient, weap_or.axis );

   // For debugging
   G_DrawCoordSystem( weap_or.origin, weap_or.axis[0], weap_or.axis[1], weap_or.axis[2], 50 );
   
   // Get the tag_barrel orientation from the weapon
   if ( !this->GetRawTag( "tag_barrel", &barrel_or ) )
      {
      //warning( "Weapon::GetMuzzlePosition", "Could not find tag_barrel\n" );
      pos = owner->centroid;
      AnglesToAxis( owner->angles, mat );
      goto out;      
      }
   
   //gi.DPrintf( "anim:%d frame:%d\n", this->CurrentAnim(), this->CurrentFrame() );

   // Translate the barrel's orientation through the weapon's orientation
   VectorCopy( weap_or.origin, pos );

   for ( i = 0 ; i < 3 ; i++ ) 
      {
		VectorMA( pos, barrel_or.origin[i], weap_or.axis[i], pos );
	   }

   MatrixMultiply( barrel_or.axis, weap_or.axis, mat );

#if 0
   gi.DPrintf( "own_angles: %0.2f %0.2f %0.2f\n", owner->angles[0], owner->angles[1], owner->angles[2] );
   gi.DPrintf( "own_orient: %0.2f %0.2f %0.2f\n", owner->orientation[0][0], owner->orientation[0][1], owner->orientation[0][2] );
   gi.DPrintf( "bone forward:  %0.2f %0.2f %0.2f\n", orient.axis[0][0], orient.axis[0][1], orient.axis[0][2] );
   gi.DPrintf( "barrel forward: %0.2f %0.2f %0.2f\n", barrel_or.axis[0][0], barrel_or.axis[0][1], barrel_or.axis[0][2] );
   gi.DPrintf( "weapon forward: %0.2f %0.2f %0.2f\n", weap_or.axis[0][0], weap_or.axis[0][1], weap_or.axis[0][2] );
   gi.DPrintf( "mat forward: %0.2f %0.2f %0.2f\n \n", mat[0][0], mat[0][1], mat[0][2] );
#endif

   // For debugging
   G_DrawCoordSystem( pos, mat[0], mat[1], mat[2], 30 );

   // Ok - we now have a position, forward, right, and up
out:
   if ( position )
		{
		*position = pos;
		}

	if ( forward )
		{
		*forward = mat[0];
		}

	if ( right )
		{
		*right = mat[1];
		}

	if ( up )
		{
		*up = mat[2];
		}
   }
*/

//======================
//Weapon::GetMuzzlePosition
//======================
void Weapon::GetMuzzlePosition
	(
	Vector *position,
	Vector *forward,
	Vector *right,
	Vector *up,
	Vector *vBarrelPos
	)

{
	orientation_t	weap_or, barrel_or, orient;
	//orientation_t	view_or;
	Vector			pos, f, r, u, aim_dir;
	Vector			vAng;
	vec3_t			mat[ 3 ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int				i, tagnum;
	Player			*owner;
	//qboolean		bViewShot;

	owner = ( Player * )this->owner.Pointer();
	assert( owner );

	// We should always have an owner
	if( !owner )
	{
		if( IsSubclassOfVehicleTurretGun() )
		{
			VehicleTurretGun *t = ( VehicleTurretGun * )this;
			owner = ( Player * )t->GetRemoteOwner();
		}
	}

	// If this is a crosshair or torsoaim weapon, then the dir is specified by the player's torso angles
	if( crosshair )
	{
		Player *player;

		// Use the player's torso angles to determine direction, but use the actual barrel to determine position

		player = ( Player * )owner;
		AngleVectors( player->GetViewAngles(), f, r, u );

		if( forward )
			*forward = f;
		if( right )
			*right = r;
		if( up )
			*up = u;
	}

	if( current_attachToTag.length() )
	{
		tagnum = gi.Tag_NumForName( owner->edict->tiki, current_attachToTag.c_str() );

		if( tagnum < 0 )
		{
			warning( "Weapon::GetMuzzlePosition", "Could not find tag \"%s\"", current_attachToTag.c_str() );
			pos = owner->origin;
			AnglesToAxis( owner->angles, mat );
			goto out;
		}

		// Get the orientation based on the frame and anim stored off in the owner.
		// This is to prevent weird timing with getting orientations on different frames of firing
		// animations and the orientations will not be consistent.   
		AnglesToAxis( owner->angles, owner->orientation );

		orient = G_TIKI_Orientation( owner->edict, tagnum );

		for( i = 0; i < 3; i++ )
		{
			VectorMA( weap_or.origin, orient.origin[ i ], owner->orientation[ i ], weap_or.origin );
		}

		MatrixMultiply( orient.axis, owner->orientation, weap_or.axis );
	}
	else
	{
		VectorCopy( origin, weap_or.origin );
		AnglesToAxis( angles, weap_or.axis );
	}

	if( !IsSubclassOfInventoryItem() )
	{
		if( ( weapon_class & WEAPON_CLASS_THROWABLE ) )
		{
			AngleVectors( owner->GetViewAngles(), mat[ 0 ], mat[ 1 ], mat[ 2 ] );
		}

		pos = owner->EyePosition();
		VectorCopy( pos, weap_or.origin );
	}
	else
	{
		// using the weapon's current origin
		VectorCopy( origin, weap_or.origin );
		AnglesToAxis( angles, weap_or.axis );
	}

	// For debugging
	G_DrawCoordSystem( weap_or.origin, weap_or.axis[ 0 ], weap_or.axis[ 1 ], weap_or.axis[ 2 ], 50 );
	pos = weap_or.origin;

	// Get the tag_barrel orientation from the weapon
	if( !vBarrelPos || !this->GetRawTag( "tag_barrel", &barrel_or ) )
	{
		//warning( "Weapon::GetMuzzlePosition", "Could not find tag_barrel\n" );

		if( owner->IsSubclassOfPlayer() )
		{
			Player *player = ( Player * )owner;
			AnglesToAxis( player->GetViewAngles(), weap_or.axis );
		}
		else
			AnglesToAxis( owner->angles, weap_or.axis );

		AxisCopy( weap_or.axis, mat );

		if( vBarrelPos )
			*vBarrelPos = weap_or.origin;

		goto out;
	}

	if( owner->IsSubclassOfPlayer() && !IsSubclassOfTurretGun() )
	{
		if( vBarrelPos )
			*vBarrelPos = origin;
	}
	else
	{
		// Translate the barrel's orientation through the weapon's orientation
		VectorCopy( weap_or.origin, pos );

		for( i = 0; i < 3; i++ )
		{
			VectorMA( pos, barrel_or.origin[ i ], weap_or.axis[ i ], pos );
		}

		MatrixMultiply( barrel_or.axis, weap_or.axis, mat );

		if( vBarrelPos )
			*vBarrelPos = pos;
	}

#if 0
	gi.DPrintf( "own_angles: %0.2f %0.2f %0.2f\n", owner->angles[0], owner->angles[1], owner->angles[2] );
	gi.DPrintf( "own_orient: %0.2f %0.2f %0.2f\n", owner->orientation[0][0], owner->orientation[0][1], owner->orientation[0][2] );
	gi.DPrintf( "bone forward:  %0.2f %0.2f %0.2f\n", orient.axis[0][0], orient.axis[0][1], orient.axis[0][2] );
	gi.DPrintf( "barrel forward: %0.2f %0.2f %0.2f\n", barrel_or.axis[0][0], barrel_or.axis[0][1], barrel_or.axis[0][2] );
	gi.DPrintf( "weapon forward: %0.2f %0.2f %0.2f\n", weap_or.axis[0][0], weap_or.axis[0][1], weap_or.axis[0][2] );
	gi.DPrintf( "mat forward: %0.2f %0.2f %0.2f\n \n", mat[0][0], mat[0][1], mat[0][2] );
#endif

	// Ok - we now have a position, forward, right, and up
out:

	// For debugging
	G_DrawCoordSystem( pos, mat[ 0 ], mat[ 1 ], mat[ 2 ], 30 );

	if( position )
	{
		*position = pos;
	}

	if( !forward && !right && !up )
	{
		return;
	}

	if( !IsSubclassOfTurretGun() && owner && owner->IsSubclassOfPlayer() )
	{
		// ...
		// fixme?
	}

	if( pos != vec_zero )
	{
		// ffs, wondering how they fucking done it in mohaaa...
		//aim_dir = barrel_or.origin - pos;
		//aim_dir.normalize();

		AngleVectors( owner->GetViewAngles(), aim_dir, NULL, NULL );

		if( !IsSubclassOfVehicleTurretGun() )
		{
			goto out2;
		}

		Vector ang;

		vectoangles( mat[ 0 ], ang );

		// this is a part of the most annoying function to reverse ever
		// fixme
	}

	// If there is a target, then use the dir based on that
	if( aim_target )
	{
		aim_dir = aim_target->centroid - pos;
		aim_dir.normalize();

		if( !IsSubclassOfTurretGun() )
		{
out2:
			AngleVectors( aim_dir.toAngles(), mat[ 0 ], mat[ 1 ], mat[ 2 ] );

			if( forward )
				*forward = mat[ 0 ];
			if( right )
				*right = mat[ 1 ];
			if( up )
				*up = mat[ 2 ];

			return;
		}

		Vector ang;

		vectoangles( mat[ 0 ], ang );

		// again a very annoying part to reverse
		// fixme
	}

	// If this weapon doesn't have a crosshair specified, then use the mat from the barrel for the directions
	if( !crosshair )
	{
		if( forward )
			*forward = mat[ 0 ];
		if( right )
			*right = mat[ 1 ];
		if( up )
			*up = mat[ 2 ];
	}
}

//======================
//Weapon::SetAmmoClipSize
//======================
void Weapon::SetAmmoClipSize
	(
   Event * ev
	)

	{
   assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	ammo_clip_size[firemodeindex] = ev->GetInteger( 1 );
	}

//======================
//Weapon::SetAmmoInClip
//======================
void Weapon::SetAmmoInClip
	(
   Event * ev
	)

	{
   assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	ammo_in_clip[firemodeindex] = ev->GetInteger( 1 );
}

//======================
//Weapon::SetShareClip
//======================
void Weapon::SetShareClip
	(
	Event * ev
	)

{
	m_bShareClip = qtrue;
}

//======================
//Weapon::Shoot
//======================
void Weapon::Shoot
	(
	Event *ev
	)

{
	Vector			pos, forward, right, up, vBarrel, delta;
	firemode_t		mode = FIRE_PRIMARY;
	qboolean		mc;

	if( ev->NumArgs() > 0 )
	{
		mode = WeaponModeNameToNum( ev->GetString( 1 ) );

		if( mode == FIRE_ERROR )
			return;
	}

	mc = MuzzleClear();

	// If we are in loopfire, we need to keep checking ammo and using it up
	if( loopfire[ mode ] )
	{
		if( HasAmmo( mode ) && mc )
		{
			// Use up the appropriate amount of ammo, it's already been checked that we have enough
			UseAmmo( ammorequired[ mode ], mode );
		}
		else
		{
			ForceIdle();
		}
	}

	// If the muzzle is not clear, then change to a clear animation, otherwise punt out.
	if( !mc )
	{
		ForceIdle();
		return;
	}

	GetMuzzlePosition( &pos, &forward, &right, &up, &vBarrel );

	if( m_fFireSpreadMultAmount[ mode ] != 0.0f )
	{
		float fTime = level.time - m_fFireSpreadMultTime[ mode ];

		if( fTime <= m_fFireSpreadMultTimeCap[ mode ] )
		{
			float fDecay = fTime * m_fFireSpreadMultFalloff[ mode ];

			if( m_fFireSpreadMult[ mode ] <= 0.0f )
			{
				m_fFireSpreadMult[ mode ] -= fDecay;

				if( m_fFireSpreadMult[ mode ] > 0.0f )
				{
					m_fFireSpreadMult[ mode ] = 0.0f;
				}
			}
			else
			{
				m_fFireSpreadMult[ mode ] -= fDecay;

				if( m_fFireSpreadMult[ mode ] < 0.0f )
				{
					m_fFireSpreadMult[ mode ] = 0.0f;
				}
			}
		}
		else
		{
			m_fFireSpreadMult[ mode ] = 0.0f;
		}

		m_fFireSpreadMultTime[ mode ] = level.time;
	}

	if( firetype[ mode ] == FT_PROJECTILE )
	{
		ProjectileAttack( pos,
			forward,
			owner,
			projectileModel[ mode ],
			charge_fraction
			);
	}
	else if( firetype[ mode ] == FT_BULLET || firetype[ mode ] == FT_FAKEBULLET )
	{
		Vector vSpread;
		float fSpreadFactor;
		int tracerFrequency;
		float bulletDamage;

		if( owner )
		{
			if( owner->client )
			{
				Player *player = ( Player * )owner.Pointer();

				fSpreadFactor = player->velocity.length() / sv_runspeed->integer;

				if( fSpreadFactor > 1.0f )
				{
					fSpreadFactor = 1.0f;
				}

				vSpread = bulletspreadmax[ mode ] * fSpreadFactor;
				fSpreadFactor = 1.0f - fSpreadFactor;
				vSpread += bulletspread[ mode ] * fSpreadFactor;
				vSpread *= m_fFireSpreadMult[ mode ] + 1.0f;

				if( m_iZoom )
				{
					if( player->IsSubclassOfPlayer() && player->IsZoomed() )
					{
						vSpread *= 1.0f + fSpreadFactor * ( m_fZoomSpreadMult - 1.0f );
					}
				}
			}
		}
		else
		{
			vSpread = ( bulletspreadmax[ mode ] + bulletspread[ mode ] ) * 0.5f;
		}

		if( owner && owner->IsSubclassOfPlayer() )
		{
			if( IsSubclassOfTurretGun() )
			{
				tracerFrequency = 3;
				bulletDamage = 45.0f;
			}
			else
			{
				tracerFrequency = 0;
				bulletDamage = bulletdamage[ mode ];
			}
		}
		else
		{
			tracerFrequency = tracerfrequency[ mode ];
			bulletDamage = bulletdamage[ mode ];
		}

		if( firetype[ mode ] == FT_BULLET )
		{
			BulletAttack( pos,
				vBarrel,
				forward,
				right,
				up,
				bulletrange[ mode ],
				bulletDamage,
				bulletknockback[ mode ],
				0,
				GetMeansOfDeath( mode ),
				vSpread,
				bulletcount[ mode ],
				owner,
				tracerFrequency,
				&tracercount[ mode ],
				this
				);
		}
		else
		{
			FakeBulletAttack( pos,
				vBarrel,
				forward,
				right,
				up,
				bulletrange[ mode ],
				bulletDamage,
				vSpread,
				bulletcount[ mode ],
				owner,
				tracerFrequency,
				&tracercount[ mode ] );
		}
	}
	else if( firetype[ mode ] == FT_SPECIAL_PROJECTILE )
	{
		this->SpecialFireProjectile( pos,
			forward,
			right,
			up,
			owner,
			projectileModel[ mode ],
			charge_fraction
			);
	}
	else if( firetype[ mode ] == FT_CLICKITEM )
	{
		ClickItemAttack( pos,
			forward,
			bulletrange[ mode ],
			owner );
	}
	else if( firetype[ mode ] == FT_HEAVY )
	{
		if( owner || !IsSubclassOfVehicleTurretGun() )
		{
			HeavyAttack(
				pos,
				forward,
				projectileModel[ mode ],
				0,
				owner,
				this );
		}
		else
		{
			VehicleTurretGun *turret = ( VehicleTurretGun * )this;

			if( turret->UseRemoteControl() && turret->GetRemoteOwner() )
			{
				HeavyAttack(
					pos,
					forward,
					projectileModel[ mode ],
					0,
					turret->GetRemoteOwner(),
					this );
			}
			else
			{
				HeavyAttack(
					pos,
					forward,
					projectileModel[ mode ],
					0,
					this,
					this );
			}
		}
	}
	else if( firetype[ mode ] == FT_MELEE ) // this is a weapon that fires like a sword
	{
		Vector melee_pos, melee_end;
		Vector dir;
		float damage;
		meansOfDeath_t meansofdeath;
		float knockback;

		if( owner )
		{
			dir = owner->centroid - pos;
		}
		else
		{
			dir = centroid - pos;
		}

		dir.z = 0;

		melee_pos = pos - forward * dir.length();
		melee_end = melee_pos + forward * bulletrange[ mode ];

		damage = bulletdamage[ mode ];
		knockback = 0;

		meansofdeath = GetMeansOfDeath( mode );

		Container<Entity *>victimlist;

		m_iNumShotsFired++;
		if( MeleeAttack( melee_pos, melee_end, damage, owner, meansofdeath, 8, -8, 8, knockback, true, &victimlist ) )
		{
			m_iNumHits++;
			m_iNumTorsoShots++;
		}
	}

	if( !quiet[ firemodeindex ] )
	{
		if( next_noise_time <= level.time )
		{
			BroadcastAIEvent( AI_EVENT_WEAPON_FIRE );
			next_noise_time = level.time + 1;
		}
	}

	if( owner && owner->client )
	{
		Vector vAngles = owner->GetViewAngles();

		if( viewkickmin[ mode ][ 0 ] != 0.0f || viewkickmax[ mode ][ 0 ] != 0.0f )
		{
			vAngles[ 0 ] += random() * ( viewkickmax[ mode ][ 0 ] - viewkickmin[ mode ][ 0 ] ) + viewkickmin[ mode ][ 0 ];
		}

		if( viewkickmin[ 1 ][ 0 ] != 0.0f || viewkickmax[ 1 ][ 0 ] != 0.0f )
		{
			vAngles[ 1 ] += random() * ( viewkickmax[ mode ][ 1 ] - viewkickmin[ mode ][ 1 ] ) + viewkickmin[ mode ][ 1 ];
		}

		owner->SetViewAngles( vAngles );
	}

	if( m_fFireSpreadMultAmount[ mode ] )
	{
		m_fFireSpreadMult[ mode ] += m_fFireSpreadMultAmount[ mode ];

		if( m_fFireSpreadMultCap[ mode ] < 0.0f )
		{
			if( m_fFireSpreadMultCap[ mode ] > m_fFireSpreadMult[ mode ] )
			{
				m_fFireSpreadMult[ mode ] = m_fFireSpreadMultCap[ mode ];
			}
			else if( m_fFireSpreadMult[ mode ] > 0.0f )
			{
				m_fFireSpreadMult[ mode ] = 0.0f;
			}
		}
		else if( m_fFireSpreadMult[ mode ] <= m_fFireSpreadMultCap[ mode ] &&
			m_fFireSpreadMult[ mode ] < 0.0f )
		{
			m_fFireSpreadMult[ mode ] = 0.0f;
		}
	}

	m_fLastFireTime = level.time;
}

//======================
//Weapon::SetAimAnim
//======================
void Weapon::SetAimAnim
   (
   Event *ev
   )

   {
   str anim;

   anim     = ev->GetString( 1 );
   aimanim = gi.Anim_NumForName( edict->tiki, anim.c_str() );
   aimframe = ev->GetInteger( 2 );
   }

//======================
//Weapon::SetOwner
//======================
void Weapon::SetOwner
	(
	Sentient *ent
	)

	{
	assert( ent );
	if ( !ent )
		{
		// return to avoid any buggy behaviour
		return;
		}

	Item::SetOwner( ent );

	setOrigin( vec_zero );
   setAngles( vec_zero );
	}

//======================
//Weapon::AttachToHand
//======================
void Weapon::AttachToHand
	(
	Event *ev
	)
{
	str tag;

	if( !owner || !attached ) {
		return;
	}

	weaponhand_t hand = WeaponHandNameToNum( ev->GetString( 1 ) );

	if( hand == WEAPON_ERROR ) {
		return;
	}

	if( hand == WEAPON_OFFHAND )
	{
		tag = attachToTag_offhand;
	}
	else
	{
		tag = attachToTag_main;
	}

	current_attachToTag = tag;

	int tagnum = gi.Tag_NumForName( owner->edict->tiki, tag );
	if( tagnum < 0 )
	{
		warning( "Weapon::AttachToHand", "Attachment of weapon '%s' to tag \"%s\": Tag Not Found\n", getName().c_str(), tag.c_str() );
	}
	else
	{
		attach( owner->entnum, tagnum );
		setOrigin();
	}
}

//======================
//Weapon::SetCantPartialReload
//======================
void Weapon::SetCantPartialReload
	(
	Event *ev
	)

{
	if( g_gametype->integer )
		return;

	m_bCanPartialReload = qfalse;
}

//======================
//Weapon::SetDMCantPartialReload
//======================
void Weapon::SetDMCantPartialReload
	(
	Event *ev
	)

{
	if( !g_gametype->integer )
		return;

	m_bCanPartialReload = qfalse;
}

//======================
//Weapon::AmmoAvailable
//======================
int Weapon::AmmoAvailable
	(
	firemode_t mode
	)

{
	// Returns the amount of ammo the owner has that is available for use
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );

	mode = m_bShareClip ? FIRE_PRIMARY : mode;


	// Make sure there is an owner before querying the amount of ammo
	if( owner )
	{
		return owner->AmmoCount( ammo_type[ mode ] );
	}
	else
	{
		if( ammo_clip_size[ mode ] )
		{
			return ammo_clip_size[ mode ];
		}
		else
		{
			return ammorequired[ mode ];
		}
	}
}

//======================
//Weapon::UnlimitedAmmo
//======================
qboolean Weapon::UnlimitedAmmo
	(
	firemode_t mode
	)

{
	if( !owner )
	{
		return false;
	}

	if( !owner->isClient() || DM_FLAG( DF_INFINITE_AMMO ) )
	{
		return true;
	}
	else if( !Q_stricmp( ammo_type[ mode ], "None" ) )
	{
		return true;
	}

	return false;
}

//======================
//Weapon::HasAmmo
//======================
qboolean Weapon::HasAmmo
	(
	firemode_t mode
	)

{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );

	if( !( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) ) )
	{
		warning( "Weapon::HasAmmo", "Invalid mode %d\n", mode );
		return false;
	}

	if( UnlimitedAmmo( mode ) )
	{
		return true;
	}

	// If the weapon uses a clip, check for ammo in the right clip
	if( ammo_clip_size[ mode ] && HasAmmoInClip( mode ) )
	{
		return true;
	}
	else // Otherwise check if ammo is available in general
	{
		if( !ammorequired[ mode ] )
			return true;

		return ( AmmoAvailable( mode ) >= ammorequired[ mode ] );
	}
}

//======================
//Weapon::HasAmmoInClip
//======================
qboolean Weapon::HasAmmoInClip
	(
	firemode_t mode
	)

{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );

	if( !( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) ) )
		warning( "Weapon::HasAmmoInClip", "Invalid mode %d\n", mode );

	mode = m_bShareClip ? FIRE_PRIMARY : mode;

	if( ammo_clip_size[ mode ] )
	{
		if( ammo_in_clip[ mode ] >= ammorequired[ mode ] )
			return true;
	}
	else
	{
		if( !ammorequired[ mode ] )
			return true;

		return ( AmmoAvailable( mode ) >= ammorequired[ mode ] );
	}

	return false;
}

//======================
//Weapon::IsSemiAuto
//======================
qboolean Weapon::IsSemiAuto
	(
	void
	)

{
	return m_bSemiAuto;
}

//======================
//Weapon::GetState
//======================
weaponstate_t Weapon::GetState
	(
	void
	)

{
	return weaponstate;
}

//======================
//Weapon::ForceState
//======================
void Weapon::ForceState
	(
	weaponstate_t state
	)

{
	weaponstate = state;
}

//======================
//Weapon::MuzzleClear
//======================
qboolean Weapon::MuzzleClear
	(
	void
	)

{
	return qtrue;
}

//======================
//Weapon::ReadyToFire
//======================
qboolean Weapon::ReadyToFire
	(
	firemode_t mode,
	qboolean   playsound
	)

{
	if( m_iZoom && mode == FIRE_SECONDARY ) {
		return qtrue;
	}

	// Make sure the weapon is in the ready state and the weapon has ammo
	if( level.time > ( m_fLastFireTime + FireDelay( mode ) ) )
	{
		if( HasAmmoInClip( mode ) )
		{
			return qtrue;
		}
		if( playsound && ( level.time >	next_noammo_time ) )
		{
			Sound( "snd_noammo" );
			next_noammo_time = level.time + level.frametime + G_Random( 0.1f ) + 0.95f;
		}
	}
	return qfalse;
}

//======================
//Weapon::PutAway
//======================
void Weapon::PutAway
	(
	void
	)

{
	// set the putaway flag to true, so the state machine know to put this weapon away
	putaway = true;
}

//======================
//Weapon::DetachFromOwner
//======================
void Weapon::DetachFromOwner
	(
	void
	)

{
	DetachGun();
	weaponstate = WEAPON_HOLSTERED;
}

//======================
//Weapon::AttachToOwner
//======================
void Weapon::AttachToOwner
	(
	weaponhand_t hand
	)

{
	AttachGun( hand );
	ForceIdle();
}

//======================
//Weapon::AttachToHolster
//======================
void Weapon::AttachToHolster
   (
   weaponhand_t hand
   )

{
	AttachGun( hand, qtrue );
	SetWeaponAnim( "holster", EV_Weapon_Idle );
}

//======================
//Weapon::Drop
//======================
qboolean Weapon::Drop
	(
	void
	)

{
	Vector temp;

	if( !owner )
	{
		return false;
	}

	if( !IsDroppable() )
	{
		return false;
	}

	if( attached )
	{
		Vector vAng;
		orientation_t oTag;
		Entity *pParent;

		if( edict->s.parent == ENTITYNUM_NONE || edict->s.tag_num < 0 )
		{
			pParent = owner;
			vAng = pParent->angles;
		}
		else
		{
			vAng = vec_zero;
			pParent = G_GetEntity( edict->s.parent );

			AnglesToAxis( pParent->angles, pParent->orientation );
			pParent->GetTag( edict->s.tag_num, &oTag );
			MatrixToEulerAngles( oTag.axis, vAng );
		}

		setAngles( vAng );
		DetachGun();
	}
	else
	{
		temp[ 2 ] = 40;

		setOrigin( owner->origin + temp );
		setAngles( owner->angles );
	}

	setSize( Vector( -12, -12, -2 ), Vector( 12, 12, 12 ) );

	// stop animating
	StopWeaponAnim();

	// drop the weapon
	PlaceItem();

	temp = centroid - owner->centroid;
	temp[ 2 ] = 0;

	VectorNormalize( temp );

	temp *= 75.0f;

	temp[ 0 ] += G_CRandom( 25 );
	temp[ 1 ] += G_CRandom( 25 );
	temp[ 2 ] = G_CRandom( 50 ) + 150;
	velocity = owner->velocity * 0.5 + temp;

	avelocity = Vector( 0, G_CRandom( 120 ), 0 );
	spawnflags |= DROPPED_PLAYER_ITEM;

	if( owner && owner->isClient() )
	{
		if( owner->deadflag && g_dropclips->integer > 0 )
		{
			int ammo;

			if( ammo_clip_size[ FIRE_PRIMARY ] )
				startammo[ FIRE_PRIMARY ] = g_dropclips->integer * ammo_in_clip[ FIRE_PRIMARY ];
			else
				startammo[ FIRE_PRIMARY ] = g_dropclips->integer;

			ammo = AmmoAvailable( FIRE_PRIMARY );
			if( startammo[ FIRE_PRIMARY ] > ammo )
			{
				startammo[ FIRE_PRIMARY ] = ammo;
			}

			if( ammo_clip_size[ FIRE_SECONDARY ] )
				startammo[ FIRE_SECONDARY ] = g_dropclips->integer * ammo_in_clip[ FIRE_SECONDARY ];
			else
				startammo[ FIRE_SECONDARY ] = g_dropclips->integer;

			ammo = AmmoAvailable( FIRE_SECONDARY );
			if( startammo[ FIRE_SECONDARY ] > ammo )
			{
				startammo[ FIRE_SECONDARY ] = ammo;
			}

			ammo_in_clip[ FIRE_PRIMARY ] = 0;
			ammo_in_clip[ FIRE_SECONDARY ] = 0;
		}
		else
		{
			startammo[ FIRE_PRIMARY ] = AmmoAvailable( FIRE_PRIMARY );
			owner->takeAmmoType( ammo_type[ FIRE_PRIMARY ] );
		}
	}
	else
	{
		if( ammo_clip_size[ FIRE_PRIMARY ] && ammo_in_clip[ FIRE_PRIMARY ] )
			startammo[ FIRE_PRIMARY ] = ammo_in_clip[ FIRE_PRIMARY ];
		else
			startammo[ FIRE_PRIMARY ] >>= 2;

		if( ammo_clip_size[ FIRE_SECONDARY ] && ammo_in_clip[ FIRE_SECONDARY ] )
			startammo[ FIRE_SECONDARY ] = ammo_in_clip[ FIRE_SECONDARY ];
		else
			startammo[ FIRE_SECONDARY ] >>= 2;

		if( startammo[ FIRE_PRIMARY ] == 0 )
		{
			startammo[ FIRE_PRIMARY ] = 1;
		}

		if( startammo[ FIRE_SECONDARY ] == 0 )
		{
			startammo[ FIRE_SECONDARY ] = 1;
		}
	}

	// Wait some time before the last owner can pickup this weapon
	last_owner = owner;
	last_owner_trigger_time = level.time + 2.0f;

	// Cancel reloading events
	CancelEventsOfType( EV_Weapon_DoneReloading );

	// Remove this from the owner's item list
	RemoveFromOwner();

	PostEvent( EV_Remove, g_droppeditemlife->value );
	PostEvent( EV_Weapon_FallingAngleAdjust, level.frametime );
	return true;
}

//======================
//Weapon::Charge
//======================
void Weapon::Charge
	(
	firemode_t mode
	)

{
}

//======================
//Weapon::ReleaseFire
//======================
void Weapon::ReleaseFire
	(
	firemode_t mode,
	float      charge_time
	)

{
	// Calculate and store off the charge fraction to use when the weapon actually shoots

	// Clamp to max_charge_time
	if( charge_time - min_charge_time[ mode ] >= 0.0f )
	{
		if( charge_time <= max_charge_time[ mode ] )
			charge_fraction = charge_time / max_charge_time[ mode ];
		else
			charge_fraction = 1.0f;
	}
	else
	{
		charge_fraction = 0.0f;
	}

	// Call regular fire function
	Fire( mode );
}

//======================
//Weapon::Fire
//======================
void Weapon::Fire
	(
	firemode_t mode
	)

{
	Event *done_event = NULL;
	Vector pos;

	// Sanity check the mode
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );

	// If we are in loopfire mode, then we don't pass a DoneFiring event
	if( !loopfire[ mode ] )
	{
		// The DoneFiring event requires to know the firing mode so save that off in the event
		done_event = new Event( EV_Weapon_DoneFiring );
		done_event->AddInteger( mode );
	}

	if( !MuzzleClear() )
	{
		SetWeaponAnim( "clear", done_event );
		weaponstate = WEAPON_READY;
		return;
	}

	// Use up the appropriate amount of ammo, it's already been checked that we have enough
	UseAmmo( ammorequired[ mode ], mode );

	// Set the state of the weapon to FIRING
	weaponstate = WEAPON_FIRING;

	// Cancel any old done firing events
	CancelEventsOfType( EV_Weapon_DoneFiring );

	// Play the correct animation
	if( mode == FIRE_PRIMARY )
	{
		if( ammo_clip_size[ FIRE_PRIMARY ] && !ammo_in_clip[ FIRE_PRIMARY ] && HasAnim( "fire_empty" ) )
		{
			SetWeaponAnim( "fire_empty", done_event );
		}
		else
		{
			SetWeaponAnim( "fire", done_event );
		}
	}
	else if( mode == FIRE_SECONDARY )
	{
		if( ( ( !m_bShareClip && ammo_clip_size[ FIRE_SECONDARY ] && !ammo_in_clip[ FIRE_SECONDARY ] ) ||
			( m_bShareClip && ammo_clip_size[ FIRE_PRIMARY ] && !ammo_in_clip[ FIRE_PRIMARY ] ) ) && HasAnim( "fire_empty" ) )
		{
			SetWeaponAnim( "secondaryfire_empty", done_event );
		}
		else
		{
			SetWeaponAnim( "secondaryfire", done_event );
		}
	}
}

//======================
//Weapon::DetachGun
//======================
void Weapon::DetachGun
	(
	void
	)

{
	if( attached )
	{
		if( m_iZoom && owner && owner->IsSubclassOfPlayer() )
		{
			Player *p = ( Player * )owner.Pointer();
			p->ZoomOff();
		}
		StopSound( CHAN_WEAPONIDLE );
		attached = false;
		detach();
		hideModel();
	}
}

//======================
//Weapon::AttachGun
//======================
void Weapon::AttachGun
	(
	weaponhand_t   hand,
	qboolean       holstering
	)

{
	int tag_num;

	if( !owner )
	{
		current_attachToTag = "";
		return;
	}

	if( attached )
	{
		DetachGun();
	}

	if( holstering )
	{
		// Save off these values if we are holstering the weapon.  We will restore them when
		// the users raises the weapons again.
		lastAngles = this->angles;
		lastScale = this->edict->s.scale;
		lastValid = qtrue;
	}
	else if( lastValid )
	{
		// Restore the last 
		setScale( lastScale );
		setAngles( lastAngles );
		lastValid = qfalse;
	}

	switch( hand )
	{
	case WEAPON_MAIN:
		if( holstering )
		{
			current_attachToTag = holster_attachToTag;
			setAngles( holsterAngles );
			setScale( holsterScale );
		}
		else
		{
			current_attachToTag = attachToTag_main;
		}
		break;
	case WEAPON_OFFHAND:
		if( holstering )
		{
			current_attachToTag = holster_attachToTag;
			setAngles( holsterAngles );
			setScale( holsterScale );
		}
		else
		{
			current_attachToTag = attachToTag_offhand;
		}
		break;
	default:
		warning( "Weapon::AttachGun", "Invalid hand for attachment of weapon specified" );
		break;
	}

	if( !current_attachToTag.length() )
		return;

	if( owner->edict->tiki )
	{
		tag_num = gi.Tag_NumForName( owner->edict->tiki, this->current_attachToTag.c_str() );

		NoLerpThisFrame();
		if( tag_num >= 0 )
		{
			attached = true;
			attach( owner->entnum, tag_num );
			showModel();
			setOrigin();
		}
		else
		{
			warning( "Weapon::AttachGun", "Attachment of weapon to tag \"%s\": Tag Not Found\n", this->current_attachToTag.c_str() );
		}
	}

	if( m_bAutoZoom && owner->IsSubclassOfPlayer() )
	{
		Player *p = ( Player * )owner.Pointer();
		p->ToggleZoom( m_iZoom );
	}
}

//======================
//Weapon::GiveStartingAmmoToOwner
//======================
void Weapon::GiveStartingAmmoToOwner
	(
	Event *ev
	)

{
	str   ammotype;
	int   mode;

	assert( owner );

	if( !owner )
	{
		warning( "Weapon::GiveStartingAmmoToOwner", "Owner not found\n" );
		return;
	}

	// Give the player the starting ammo
	for( mode = FIRE_PRIMARY; mode<MAX_FIREMODES; mode++ )
	{
		ammotype = GetAmmoType( ( firemode_t )( mode ) );
		if( ammotype.length() )
		{
			int start_ammo = this->GetStartAmmo( ( firemode_t )mode );

			if( ammo_clip_size[ mode ] )
			{
				int ammo = ammo_clip_size[ mode ] - ammo_in_clip[ mode ];

				if( ammo > 0 )
				{
					if( ammo < start_ammo )
					{
						start_ammo -= ammo;
						ammo_in_clip[ mode ] = ammo + ammo_in_clip[ mode ];
					}
					else
					{
						ammo_in_clip[ mode ] = start_ammo + ammo_in_clip[ mode ];
						start_ammo = 0;
					}
				}
			}

			if( start_ammo )
			{
				owner->GiveAmmo( ammotype, start_ammo );
			}
		}
	}
}
//======================
//Weapon::PickupWeapon
//======================
void Weapon::PickupWeapon
	(
	Event *ev
	)

{
	Sentient       *sen;
	Entity         *other;
	qboolean       hasweapon;

	other = ev->GetEntity( 1 );
	assert( other );

	if( !other->IsSubclassOfSentient() )
	{
		return;
	}

	sen = ( Sentient * )other;

	// If this is the last owner, check to see if he can pick it up
	if( ( sen == last_owner ) && ( level.time < last_owner_trigger_time ) )
	{
		return;
	}

	hasweapon = sen->HasItem( item_name ) || sen->HasWeaponClass( weapon_class );

	if( g_gametype->integer && !hasweapon && !IsSecondaryWeapon() )
	{
		// Make sure the sentient doesn't have a primary weapon on DM modes
		hasweapon = sen->HasPrimaryWeapon();
	}

	if( !hasweapon )
	{
		if( other->IsSubclassOfPlayer() )
		{
			gi.SendServerCommand( other->edict - g_entities, "print \"" HUD_MESSAGE_YELLOW "%s\n\"", gi.LV_ConvertString( va( "Picked Up %s", item_name.c_str() ) ) );

			if( !( spawnflags & DROPPED_PLAYER_ITEM ) && !( spawnflags & DROPPED_ITEM ) )
			{
				ItemPickup( other );
				return;
			}

			if( Pickupable( other ) )
			{
				setMoveType( MOVETYPE_NONE );
				setSolidType( SOLID_NOT );

				hideModel();

				velocity = vec_zero;
				avelocity = vec_zero;

				CancelEventsOfType( EV_Remove );
				CancelEventsOfType( EV_Weapon_FallingAngleAdjust );

				StopWeaponAnim();
				DetachGun();

				ForceState( WEAPON_CHANGING );
				current_attachToTag = "";
				lastValid = qfalse;
				edict->s.tag_num = -1;
				edict->s.attach_use_angles = qfalse;
				VectorClear( edict->s.attach_offset );
				setOrigin( vec_zero );
				setAngles( vec_zero );

				SetOwner( sen );
				sen->AddItem( this );
				sen->ReceivedItem( this );

				Sound( sPickupSound );
			}
		}
	}
	else
	{
		str sAmmoName = ammo_type[ FIRE_PRIMARY ];

		if( sen->AmmoCount( sAmmoName ) != sen->MaxAmmoCount( sAmmoName ) )
		{
			setSolidType( SOLID_NOT );
			hideModel();

			CancelEventsOfType( EV_Item_DropToFloor );
			CancelEventsOfType( EV_Item_Respawn );
			CancelEventsOfType( EV_FadeOut );
			CancelEventsOfType( EV_Remove );
			CancelEventsOfType( EV_Weapon_FallingAngleAdjust );

			if( Respawnable() )
			{
				PostEvent( EV_Item_Respawn, 0 );
			}
			else
			{
				PostEvent( EV_Remove, 0 );
			}

			Sound( m_sAmmoPickupSound );
		}
	}

	if( sen->isClient() && startammo[ FIRE_PRIMARY ] && ammo_type[ FIRE_PRIMARY ].length() )
	{
		str sMessage;
		str sAmmoType = ammo_type[ FIRE_PRIMARY ];

		sen->GiveAmmo( sAmmoType, startammo[ FIRE_PRIMARY ] );

		if( !sAmmoType.icmp( "agrenade" ) )
		{
			if( startammo[ FIRE_PRIMARY ] == 1 )
				sMessage = gi.LV_ConvertString( "Got 1 Grenade" );
			else
				sMessage = gi.LV_ConvertString( va( "Got %i Grenades", startammo[ FIRE_PRIMARY ] ) );
		}
		else
		{
			sMessage = gi.LV_ConvertString( va( "Got %i %s Rounds", startammo[ FIRE_PRIMARY ], sAmmoType.c_str() ) );
		}

		gi.SendServerCommand( other->edict - g_entities, "print \"" HUD_MESSAGE_YELLOW "%s\n\"", sMessage.c_str() );
	}

	Unregister( STRING_PICKUP );
}

//======================
//Weapon::IsSecondaryWeapon
//======================
qboolean Weapon::IsSecondaryWeapon
	(
	void
	)

{
	return ( weapon_class & WEAPON_CLASS_SECONDARY );
}

//======================
//Weapon::ForceIdle
//======================
void Weapon::ForceIdle
	(
	void
	)

{
	SetWeaponIdleAnim();

	// Force the weapon to the idle animation
	weaponstate = WEAPON_READY;
}

//======================
//Weapon::SetWeaponAnim
//======================
qboolean Weapon::SetWeaponAnim
	(
	const char *anim,
	Event *ev
	)

{
	int animnum = gi.Anim_NumForName( edict->tiki, anim );

	if( animnum == -1 )
	{
		if( ev ) {
			delete ev;
		}

		return qfalse;
	}

	StopAnimating( m_iAnimSlot );
	SetTime( m_iAnimSlot );

	int idleanim = gi.Anim_NumForName( edict->tiki, "idle" );

	edict->s.frameInfo[ m_iAnimSlot ].index = idleanim;
	m_iAnimSlot = ( m_iAnimSlot + 1 ) & 3;
	edict->s.frameInfo[ m_iAnimSlot ].index = idleanim;

	if( ev )
	{
		NewAnim( animnum, ev, m_iAnimSlot );
	}
	else
	{
		NewAnim( animnum, m_iAnimSlot );
	}

	SetOnceType( m_iAnimSlot );
	SetTime( m_iAnimSlot );

	return qtrue;
}

//======================
//Weapon::SetWeaponAnim
//======================
qboolean Weapon::SetWeaponAnim
	(
	const char *anim,
	Event& ev
	)

{
	Event *event = new Event( ev );

	return SetWeaponAnim( anim, event );
}

//======================
//Weapon::SetWeaponAnimEvent
//======================
void Weapon::SetWeaponAnimEvent
	(
	Event *ev
	)

{
	SetWeaponAnim( ev->GetString( 1 ) );
}

//======================
//Weapon::SetWeaponIdleAnim
//======================
void Weapon::SetWeaponIdleAnim
	(
	void
	)

{
	if( ammo_clip_size[ FIRE_PRIMARY ] && !ammo_in_clip[ FIRE_PRIMARY ] )
	{
		if( SetWeaponAnim( "idle_empty" ) )
		{
			return;
		}
	}

	SetWeaponAnim( "idle" );
}

//======================
//Weapon::SetWeaponIdleAnimEvent
//======================
void Weapon::SetWeaponIdleAnimEvent
	(
	Event *ev
	)

{
	SetWeaponIdleAnim();
}

//======================
//Weapon::StopWeaponAnim
//======================
void Weapon::StopWeaponAnim
	(
	void
	)

{
	SetTime( m_iAnimSlot );
	StopAnimating( m_iAnimSlot );

	int animnum = gi.Anim_NumForName( edict->tiki, "idle" );

	edict->s.frameInfo[ m_iAnimSlot ].index = animnum;
	edict->s.frameInfo[ m_iAnimSlot ].weight = 1.0f;
	edict->s.frameInfo[ m_iAnimSlot ].time = 0;

	m_iAnimSlot = ( m_iAnimSlot + 1 ) & 3;
}

//======================
//Weapon::DoneRaising
//======================
void Weapon::DoneRaising
	(
	Event *ev
	)

{
	weaponstate = WEAPON_READY;
	ForceIdle();

	if( !owner )
	{
		PostEvent( EV_Remove, 0 );
		return;
	}
}

//======================
//Weapon::ClientFireDone
//======================
void Weapon::ClientFireDone
   (
   void
   )

   {
   // This is called when the client's firing animation is done
   }

//======================
//Weapon::DoneFiring
//======================
void Weapon::DoneFiring
	(
	Event *ev
	)

{
	// This is called when the weapon's firing animation is done
	ForceIdle();

	// Check to see if the auto_putaway flag is set, and the weapon is out of ammo.  If so, then putaway the
	// weapon.
	if( !HasAmmo( FIRE_PRIMARY ) && auto_putaway )
	{
		PutAway();
	}
}

//======================
//Weapon::DoneReloading
//======================
void Weapon::DoneReloading
	(
	Event *ev
	)

{
   SetShouldReload( qfalse );
   weaponstate = WEAPON_READY;
}

//======================
//Weapon::CheckReload
//======================
qboolean Weapon::CheckReload
	(
	firemode_t mode
	)

{
	// Check to see if the weapon needs to be reloaded
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );

	mode = m_bShareClip ? FIRE_PRIMARY : mode;

	if( putaway )
		return false;

	if( ammo_in_clip[ mode ] < ammo_clip_size[ mode ] && AmmoAvailable( mode ) && ( m_bCanPartialReload || ammo_in_clip[ mode ] <= 0 ) )
		return true;

	return false;
}

//======================
//Weapon::Idle
//======================
void Weapon::Idle
	(
	Event *ev
	)

{
	ForceIdle();
}

//======================
//Weapon::IdleInit
//======================
void Weapon::IdleInit
	(
	Event *ev
	)

{
	for( int i = 7; i >= 0; i-- )
	{
		SetWeaponAnim( "idle" );
	}

	weaponstate = WEAPON_READY;
}

//======================
//Weapon::GetMaxRange
//======================
float	Weapon::GetMaxRange
	(
	void
	)

	{
	return maxrange;
	}

//======================
//Weapon::GetMinRange
//======================
float	Weapon::GetMinRange
	(
	void
	)

	{
	return minrange;
	}

//======================
//Weapon::SetMaxRangeEvent
//======================
void Weapon::SetMaxRangeEvent
	(
	Event *ev
	)

	{
	maxrange = ev->GetFloat( 1 );
	}

//======================
//Weapon::SetMinRangeEvent
//======================
void Weapon::SetMinRangeEvent
	(
	Event *ev
	)

	{
	minrange = ev->GetFloat( 1 );
	}

//======================
//Weapon::NotDroppableEvent
//======================
void Weapon::NotDroppableEvent
	(
	Event *ev
	)

	{
   notdroppable = true;
	}

//======================
//Weapon::SetMaxRange
//======================
void Weapon::SetMaxRange
	(
	float val
	)

	{
	maxrange = val;
	}

//======================
//Weapon::SetMinRange
//======================
void Weapon::SetMinRange
	(
	float val
	)

	{
	minrange = val;
	}

//======================
//Weapon::WeaponSound
//======================
void Weapon::WeaponSound
	(
	Event *ev
	)

{
	Event *e;

	// Broadcasting a sound can be time consuming.  Only do it once in a while on really fast guns.
	if( nextweaponsoundtime > level.time )
	{
		if( owner )
		{
			owner->BroadcastAIEvent( AI_EVENT_WEAPON_FIRE );
		}
		else
		{
			BroadcastAIEvent( AI_EVENT_WEAPON_FIRE );
		}
		return;
	}

	if( owner )
	{
		e = new Event( ev );
		owner->ProcessEvent( e );
	}
	else
	{
		Item::BroadcastAIEvent( AI_EVENT_WEAPON_FIRE );
	}

	// give us some breathing room
	nextweaponsoundtime = level.time + 0.4;
}

//======================
//Weapon::Removable
//======================
qboolean Weapon::Removable
   (
   void
   )

   {
   if (
         ( ( int )( dmflags->integer ) & DF_WEAPONS_STAY ) &&
         !( spawnflags & ( DROPPED_ITEM | DROPPED_PLAYER_ITEM ) )
      )
      return false;
   else
      return true;
   }

//======================
//Weapon::Pickupable
//======================
qboolean Weapon::Pickupable
   (
   Entity *other
   )

   {
   Sentient *sen;

   if ( !other->IsSubclassOfSentient() )
		{
		return false;
		}
   else if ( !other->isClient() )
      {
      return false;
      }

   sen = ( Sentient * )other;

   //FIXME
   // This should be in player

   // If we have the weapon and weapons stay, then don't pick it up
   if ( ( ( int )( dmflags->integer ) & DF_WEAPONS_STAY ) && !( spawnflags & ( DROPPED_ITEM | DROPPED_PLAYER_ITEM ) ) )
      {
      Weapon   *weapon;

      weapon = ( Weapon * )sen->FindItem( getName() );

      if ( weapon )
         return false;
      }

   return true;
   }

//======================
//Weapon::AutoChange
//======================
qboolean Weapon::AutoChange
   (
   void
   )

   {
   return true;
   }

//======================
//Weapon::ClipAmmo
//======================
int Weapon::ClipAmmo
	(
	firemode_t mode
	)

{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );

	mode = m_bShareClip ? FIRE_PRIMARY : mode;

	if( ammo_clip_size[ mode ] )
		return ammo_in_clip[ mode ];
	else
		return -1;
}

//======================
//Weapon::ProcessWeaponCommandsEvent
//======================
void Weapon::ProcessWeaponCommandsEvent
	(
	Event *ev
	)

	{
   int index;

   index = ev->GetInteger( 1 );
   ProcessInitCommands();
	}

//======================
//Weapon::SetActionLevelIncrement
//======================
void Weapon::SetActionLevelIncrement
   (
   Event *ev
   )
   {
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
   action_level_increment[firemodeindex] = ev->GetInteger( 1 );
   }

//======================
//Weapon::ActionLevelIncrement
//======================
int Weapon::ActionLevelIncrement
   (
   firemode_t mode
   )

   {
   assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );

   if ( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) )
      return action_level_increment[mode];
   else 
      {
      warning( "Weapon::ActionLevelIncrement", "Invalid mode %d\n", mode );
      return 0;
      }
   }

//======================
//Weapon::IsDroppable
//======================
qboolean Weapon::IsDroppable
	(
	void
	)
{
	if( notdroppable )
	{
		return false;
	}
	else
	{
		return true;
	}
}

//======================
//Weapon::SetFireType
//======================
void Weapon::SetFireType
	(
	Event *ev
	)

{
	str ftype;

	ftype = ev->GetString( 1 );

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );

	if( !ftype.icmp( "projectile" ) )
		firetype[ firemodeindex ] = FT_PROJECTILE;
	else if( !ftype.icmp( "bullet" ) )
		firetype[ firemodeindex ] = FT_BULLET;
	else if( !ftype.icmp( "fakebullet" ) )
		firetype[ firemodeindex ] = FT_FAKEBULLET;
	else if( !ftype.icmp( "melee" ) )
		firetype[ firemodeindex ] = FT_MELEE;
	else if( !ftype.icmp( "special_projectile" ) )
		firetype[ firemodeindex ] = FT_SPECIAL_PROJECTILE;
	else if( !ftype.icmp( "clickitem" ) )
		firetype[ firemodeindex ] = FT_CLICKITEM;
	else if( !ftype.icmp( "heavy" ) )
		firetype[ firemodeindex ] = FT_HEAVY;
	else if( !ftype.icmp( "none" ) )
		firetype[ firemodeindex ] = FT_NONE;
	else
		warning( "Weapon::SetFireType", "unknown firetype: %s\n", ftype.c_str() );
}

//======================
//Weapon::GetFireType
//======================
firetype_t Weapon::GetFireType
   (
   firemode_t mode
   )

   {
   return firetype[mode];
   }

firemode_t Weapon::GetFireMode()
{
	return firemodeindex;
}

//======================
//Weapon::SetProjectile
//======================
void Weapon::SetProjectile
	(
	Event *ev
	)

{
	if( g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	projectileModel[ firemodeindex ] = ev->GetString( 1 );
	CacheResource( projectileModel[ firemodeindex ].c_str() );
}

//======================
//Weapon::SetDMProjectile
//======================
void Weapon::SetDMProjectile
	(
	Event *ev
	)

{
	if( !g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	projectileModel[ firemodeindex ] = ev->GetString( 1 );
	CacheResource( projectileModel[ firemodeindex ].c_str() );
}

//======================
//Weapon::SetBulletDamage
//======================
void Weapon::SetBulletDamage
   (
   Event *ev
   )

   {
   assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
   bulletdamage[firemodeindex] = ev->GetFloat( 1 );
}

//======================
//Weapon::SetDMBulletDamage
//======================
void Weapon::SetDMBulletDamage
	(
	Event *ev
	)

{
	if( !g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	bulletdamage[ firemodeindex ] = ev->GetFloat( 1 );
}

//======================
//Weapon::SetBulletKnockback
//======================
void Weapon::SetBulletKnockback
   (
   Event *ev
   )

   {
   assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
   bulletknockback[firemodeindex] = ev->GetFloat( 1 );
   }

//======================
//Weapon::SetDMBulletKnockback
//======================
void Weapon::SetDMBulletKnockback
(
Event *ev
)

{
	if( !g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	bulletknockback[ firemodeindex ] = ev->GetFloat( 1 );
}

//======================
//Weapon::SetBulletRange
//======================
void Weapon::SetBulletRange
	(
	Event *ev
	)

{
	if( g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	bulletrange[ firemodeindex ] = ev->GetFloat( 1 );
}

//======================
//Weapon::SetDMBulletRange
//======================
void Weapon::SetDMBulletRange
	(
	Event *ev
	)

{
	if( !g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	bulletrange[ firemodeindex ] = ev->GetFloat( 1 );
}

//======================
//Weapon::SetRange
//======================
void Weapon::SetRange
	(
	Event *ev
	)

{
	SetBulletRange( ev );
}

//======================
//Weapon::SetBulletCount
//======================
void Weapon::SetBulletCount
	(
	Event *ev
	)

{
	if( g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	bulletcount[ firemodeindex ] = ev->GetFloat( 1 );
}

//======================
//Weapon::SetDMBulletCount
//======================
void Weapon::SetDMBulletCount
	(
	Event *ev
	)

{
	if( !g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	bulletcount[ firemodeindex ] = ev->GetFloat( 1 );
}

//======================
//Weapon::SetBulletSpread
//======================
void Weapon::SetBulletSpread
	(
	Event *ev
	)

{
	if( g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	bulletspread[ firemodeindex ].x = ev->GetFloat( 1 );
	bulletspread[ firemodeindex ].y = ev->GetFloat( 2 );

	if( ev->NumArgs() > 2 )
	{
		bulletspreadmax[ firemodeindex ].x = ev->GetFloat( 3 );
		bulletspreadmax[ firemodeindex ].y = ev->GetFloat( 4 );
	}
}

//======================
//Weapon::SetDMBulletSpread
//======================
void Weapon::SetDMBulletSpread
	(
	Event *ev
	)

{
	if( !g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	bulletspread[ firemodeindex ].x = ev->GetFloat( 1 );
	bulletspread[ firemodeindex ].y = ev->GetFloat( 2 );

	if( ev->NumArgs() > 2 )
	{
		bulletspreadmax[ firemodeindex ].x = ev->GetFloat( 3 );
		bulletspreadmax[ firemodeindex ].y = ev->GetFloat( 4 );
	}
}

//======================
//Weapon::SetZoomSpreadMult
//======================
void Weapon::SetZoomSpreadMult
	(
	Event *ev
	)

{
	if( g_gametype->integer )
		return;

	m_fZoomSpreadMult = ev->GetFloat( 1 );
}

//======================
//Weapon::SetDMZoomSpreadMult
//======================
void Weapon::SetDMZoomSpreadMult
	(
	Event *ev
	)

{
	if( !g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	m_fZoomSpreadMult = ev->GetFloat( 1 );
}

//======================
//Weapon::SetZoomSpreadMult
//======================
void Weapon::SetFireSpreadMult
	(
	Event *ev
	)

{
	if( g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	m_fFireSpreadMultAmount[ firemodeindex ] = ev->GetFloat( 1 );
	m_fFireSpreadMultFalloff[ firemodeindex ] = ev->GetFloat( 2 );
	m_fFireSpreadMultCap[ firemodeindex ] = ev->GetFloat( 3 );
	m_fFireSpreadMultTimeCap[ firemodeindex ] = ev->GetFloat( 4 );
}

//======================
//Weapon::SetDMZoomSpreadMult
//======================
void Weapon::SetDMFireSpreadMult
	(
	Event *ev
	)

{
	if( !g_gametype->integer )
		return;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	m_fFireSpreadMultAmount[ firemodeindex ] = ev->GetFloat( 1 );
	m_fFireSpreadMultFalloff[ firemodeindex ] = ev->GetFloat( 2 );
	m_fFireSpreadMultCap[ firemodeindex ] = ev->GetFloat( 3 );
	m_fFireSpreadMultTimeCap[ firemodeindex ] = ev->GetFloat( 4 );
}

//======================
//Weapon::SetTracerFrequency
//======================
void Weapon::SetTracerFrequency
	(
	Event *ev
	)

{
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	tracerfrequency[ firemodeindex ] = ev->GetInteger( 1 );
}

//======================
//Weapon::Alternate
//======================
void Weapon::Secondary
	(
	Event *ev
	)

{
	int i;

	Event *altev = new Event( ev->GetToken( 1 ) );

	firemodeindex = FIRE_SECONDARY;

	for( i = 2; i <= ev->NumArgs(); i++ )
	{
		altev->AddToken( ev->GetToken( i ) );
	}

	ProcessEvent( altev );
	firemodeindex = FIRE_PRIMARY;
}

//====================
//Weapon::AutoAim
//====================
void Weapon::AutoAim
   (
   Event *ev
   )

   {
   autoaim = true;
   }

//====================
//Weapon::Crosshair
//====================
void Weapon::Crosshair
	(
	Event *ev
	)

{
	crosshair = ev->GetBoolean( 1 );
}

//====================
//Weapon::DMCrosshair
//====================
void Weapon::DMCrosshair
	(
	Event *ev
	)

{
	if( !g_gametype->integer )
		return;

	crosshair = ev->GetBoolean( 1 );
}

//====================
//Weapon::OffHandAttachToTag
//====================
void Weapon::OffHandAttachToTag
	(
	Event *ev
	)

{
	attachToTag_offhand = ev->GetString( 1 );
}

//====================
//Weapon::MainAttachToTag
//====================
void Weapon::MainAttachToTag
	(
	Event *ev
	)

{
	attachToTag_main = ev->GetString( 1 );
}

//====================
//Weapon::HolsterAttachToTag
//====================
void Weapon::HolsterAttachToTag
	(
	Event *ev
	)

{
	holster_attachToTag = ev->GetString( 1 );
}

//====================
//Weapon::SetHolsterOffset
//====================
void Weapon::SetHolsterOffset
	(
	Event *ev
	)

{
	holsterOffset = ev->GetVector( 1 );
}

//====================
//Weapon::SetHolsterAngles
//====================
void Weapon::SetHolsterAngles
	(
	Event *ev
	)

{
	holsterAngles = ev->GetVector( 1 );
}

//====================
//Weapon::SetHolsterScale
//====================
void Weapon::SetHolsterScale
   (
   Event *ev
   )

   {
   holsterScale = ev->GetFloat( 1 );
   }

//====================
//Weapon::SetQuiet
//====================
void Weapon::SetQuiet
	(
	Event *ev
	)

{
   quiet[ firemodeindex ] = true;
}

//====================
//Weapon::SetLoopFire
//====================
void Weapon::SetLoopFire
   (
   Event *ev
   )

   {
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
   loopfire[firemodeindex] = true;
   }

//====================
//Weapon::GetZoom
//====================
int Weapon::GetZoom( void )
{
	return m_iZoom;
}

//====================
//Weapon::GetAutoZoom
//====================
qboolean Weapon::GetAutoZoom( void )
{
	return m_bAutoZoom;
}

//======================
//Weapon::SetMeansOfDeath
//======================
void Weapon::SetMeansOfDeath
   (
   Event *ev
   )

   {
	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	meansofdeath[firemodeindex] = (meansOfDeath_t )MOD_NameToNum( ev->GetString( 1 ) );
   }

//======================
//Weapon::GetMeansOfDeath
//======================
meansOfDeath_t Weapon::GetMeansOfDeath
	(
	firemode_t mode
	)

{
	assert( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) );

	if( ( mode >= 0 ) && ( mode < MAX_FIREMODES ) )
		return meansofdeath[ mode ];
	else
	{
		warning( "Weapon::GetMeansOfDeath", "Invalid mode %d\n", mode );
		return MOD_NONE;
	}
}

//======================
//Weapon::SetAimTarget
//======================
void Weapon::SetAimTarget
   (
   Entity *ent
   )

   {
   aim_target = ent;
   }

qboolean Weapon::ShouldReload( void )
{
	if( m_bShouldReload )
	{
		return qtrue;
	}
	else
	{
		if( ammo_clip_size[ FIRE_PRIMARY ] && !ammo_in_clip[ FIRE_PRIMARY ] && AmmoAvailable( FIRE_PRIMARY ) )
			return qtrue;
	}

	return qfalse;
}

void Weapon::SetShouldReload( qboolean should_reload )
{
	m_bShouldReload = should_reload;
}

void Weapon::StartReloading( void )
{
	if( !ammo_clip_size[ 0 ] || !owner ) {
		return;
	}

	Event *ev = new Event( EV_Weapon_DoneReloading );

	if( SetWeaponAnim( "reload", ev ) )
	{
		weaponstate = WEAPON_RELOADING;
	}
	else
	{
		ProcessEvent( EV_Weapon_FillClip );
		ProcessEvent( EV_Weapon_DoneReloading );
	}

	m_fFireSpreadMult[ FIRE_PRIMARY ] = 0;
}

//======================
//Weapon::WorldHitSpawn
//======================
void Weapon::WorldHitSpawn
	(
	firemode_t  mode,
	Vector      origin,
	Vector      angles,
	float       life
	)

{

}

//======================
//Weapon::SetWorldHitSpawn
//======================
void Weapon::SetWorldHitSpawn
   (
   Event *ev
   )

   {
   assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );
	worldhitspawn[firemodeindex] = ev->GetString( 1 );
   }

void Weapon::SetWeaponGroup( Event *ev )
{
	m_csWeaponGroup = ev->GetConstString( 1 );
}

void Weapon::SetZoom( Event *ev )
{
	m_iZoom = ev->GetInteger( 1 );

	if( ev->NumArgs() > 1 )
	{
		int autozoom = ev->GetInteger( 2 );

		if( autozoom ) {
			m_bAutoZoom = qtrue;
		}
	}
}

void Weapon::SetSemiAuto( Event *ev )
{
	m_bSemiAuto = qtrue;
}

void Weapon::SetWeaponType( Event *ev )
{
	weapon_class = G_WeaponClassNameToNum( ev->GetString( 1 ) );
}

void Weapon::EventClipAdd( Event *ev )
{
	int         amount;
	int         amount_used;

	// Calc the amount the clip should get
	amount = ev->GetInteger( 1 );

	assert( owner );
	if( owner && owner->isClient() && !UnlimitedAmmo( FIRE_PRIMARY ) )
	{
		if( amount > ammo_clip_size[ FIRE_PRIMARY ] )
			amount = ammo_clip_size[ FIRE_PRIMARY ] - ammo_in_clip[ FIRE_PRIMARY ];

		// use up the ammo from the player
		amount_used = owner->UseAmmo( ammo_type[ FIRE_PRIMARY ], amount );

		// Stick it in the clip
		if( ammo_clip_size[ FIRE_PRIMARY ] )
			ammo_in_clip[ FIRE_PRIMARY ] = amount_used + ammo_in_clip[ FIRE_PRIMARY ];

		if( ammo_in_clip[ FIRE_PRIMARY ] > ammo_clip_size[ FIRE_PRIMARY ] )
			ammo_in_clip[ FIRE_PRIMARY ] = ammo_clip_size[ FIRE_PRIMARY ];

		owner->AmmoAmountInClipChanged( ammo_type[ FIRE_PRIMARY ], ammo_in_clip[ FIRE_PRIMARY ] );

		SetShouldReload( qfalse );
	}
}

void Weapon::EventClipEmpty( Event *ev )
{
	if( !ammo_clip_size[ FIRE_PRIMARY ] )
		return;

	if( !owner )
		return;

	owner->GiveAmmo( ammo_type[ FIRE_PRIMARY ], ammo_in_clip[ FIRE_PRIMARY ] );
	ammo_in_clip[ FIRE_PRIMARY ] = 0;

	SetShouldReload( qtrue );

	owner->AmmoAmountInClipChanged( ammo_type[ FIRE_PRIMARY ], ammo_in_clip[ FIRE_PRIMARY ] );
}

void Weapon::EventClipFill( Event *ev )
{

	int         amount;
	int         amount_used;

	// Calc the amount the clip should get
	amount = ammo_clip_size[ FIRE_PRIMARY ] - ammo_in_clip[ FIRE_PRIMARY ];

	assert( owner );
	if( owner && owner->isClient() && !UnlimitedAmmo( FIRE_PRIMARY ) )
	{
		// use up the ammo from the player
		amount_used = owner->UseAmmo( ammo_type[ FIRE_PRIMARY ], amount );

		// Stick it in the clip
		if( ammo_clip_size[ FIRE_PRIMARY ] )
			ammo_in_clip[ FIRE_PRIMARY ] = amount_used + ammo_in_clip[ FIRE_PRIMARY ];

		assert( ammo_in_clip[ FIRE_PRIMARY ] <= ammo_clip_size[ FIRE_PRIMARY ] );
		if( ammo_in_clip[ FIRE_PRIMARY ] > ammo_clip_size[ FIRE_PRIMARY ] )
			ammo_in_clip[ FIRE_PRIMARY ] = ammo_clip_size[ FIRE_PRIMARY ];
	}

	owner->AmmoAmountInClipChanged( ammo_type[ FIRE_PRIMARY ], ammo_in_clip[ FIRE_PRIMARY ] );

	SetShouldReload( qfalse );
}

float Weapon::FireDelay
	(
	firemode_t mode
	)

{
	return firedelay[ mode ];
}

void Weapon::EventSetFireDelay
	(
	Event *ev
	)

{
	if( g_gametype->integer )
		return;

	firedelay[ firemodeindex ] = ev->GetFloat( 1 );
}

void Weapon::EventSetDMFireDelay
	(
	Event *ev
	)

{
	if( !g_gametype->integer )
		return;

	firedelay[ firemodeindex ] = ev->GetFloat( 1 );
}

void Weapon::MakeNoise
	(
	Event *ev
	)

{
	float radius = 500;
	qboolean force = false;

	if( ev->NumArgs() > 0 )
		radius = ev->GetFloat( 1 );

	if( ev->NumArgs() > 1 )
		force = ev->GetBoolean( 2 );

	if( attached && ( next_noise_time <= level.time || force ) )
	{
		BroadcastAIEvent( 8, radius );
		next_noise_time = level.time + 1;
	}
}

void Weapon::FallingAngleAdjust
(
Event *ev
)

{
	Vector vTmp;
	Vector vDir;
	trace_t trace;

	if( owner )
		return;

	if( groundentity && ( groundentity->entity == world || groundentity->r.bmodel ) )
	{
		Vector vEnd = origin - Vector( 0, 0, 128 );

		trace = G_Trace( origin,
			vec_zero,
			vec_zero,
			vEnd,
			last_owner,
			edict->clipmask,
			qfalse,
			"Weapon::FallingAngleAdjust" );

		if( trace.fraction < 1.0f )
		{
			Vector vAng = Vector( 0, angles[ 1 ], 0 );
			Vector vCross;

			/*vAng.AngleVectorsLeft( &vDir );

			CrossProduct( vAng, vDir, vCross );
			CrossProduct( vDir, vCross, vAng );
			vAng = vAng.toAngles();*/

			if( angles[ 2 ] <= 0.0f )
				vAng[ 2 ] = anglemod( vAng[ 2 ] - 90.0f );
			else
				vAng[ 2 ] = anglemod( vAng[ 2 ] + 90.0f );

			setAngles( vAng );
		}

		if( weapon_class >= 0 )
		{
			Sound( G_WeaponClassNumToName( weapon_class ) + "_drop" );
		}

		return;
	}

	angles[ 0 ] = anglemod( angles[ 0 ] );
	//if( angles[ 0 ] > 180.0f )
	//	angles[ 0 ] -= 180.0f;

	if( angles[ 0 ] >= -90.0f && angles[ 0 ] <= 0.0f )
	{
		angles[ 0 ] -= level.frametime * 160.0f;
		if( angles[ 0 ] < -180.0f )
			angles[ 0 ] = -180.0f;
	}
	else if( angles[ 0 ] > 0.0f )
	{
		angles[ 0 ] -= level.frametime * 160.0f;
	}
	else
	{
		angles[ 0 ] += level.frametime * 160.0f;
	}

	angles[ 2 ] = anglemod( angles[ 2 ] );
	//if( angles[ 2 ] > 180.0f )
	//	angles[ 2 ] -= 180.0f;

	if( angles[ 2 ] > -90.0f && angles[ 2 ] < 0.0f )
	{
		angles[ 2 ] -= level.frametime * 160.0f;
	}
	else if( angles[ 2 ] > 90.0f )
	{
		angles[ 2 ] -= level.frametime * 160.0f;
	}
	else
	{
		angles[ 2 ] += level.frametime * 160.0f;
	}

	angles[ 0 ] = anglemod( angles[ 0 ] );
	angles[ 2 ] = anglemod( angles[ 2 ] );
	setAngles( angles );
	PostEvent( EV_Weapon_FallingAngleAdjust, level.frametime );
}

void Weapon::SetAIRange
	(
	Event *ev
	)

{
	str s = ev->GetString( 1 );

	if( !s.icmp( "short" ) )
	{
		mAIRange = RANGE_SHORT;
	}
	else if( !s.icmp( "medium" ) )
	{
		mAIRange = RANGE_MEDIUM;
	}
	else if( !s.icmp( "long" ) )
	{
		mAIRange = RANGE_LONG;
	}
	else if( !s.icmp( "sniper" ) )
	{
		mAIRange = RANGE_SNIPER;
	}
	else
	{
		warning( "Weapon::SetAIRange", "unknown range: %s.  Should be short, medium, long, or sniper\n", s.c_str() );
	}
}

void Weapon::SetViewKick
	(
	Event *ev
	)

{
	float pitchmin, pitchmax;
	float yawmin, yawmax;

	assert( ( firemodeindex >= 0 ) && ( firemodeindex < MAX_FIREMODES ) );

	pitchmin = ev->GetFloat( 1 );
	pitchmax = ev->GetFloat( 2 );

	if( pitchmin <= pitchmax )
	{
		viewkickmin[ firemodeindex ][ 0 ] = pitchmin;
		viewkickmax[ firemodeindex ][ 0 ] = pitchmax;
	}
	else
	{
		viewkickmin[ firemodeindex ][ 0 ] = pitchmax;
		viewkickmax[ firemodeindex ][ 0 ] = pitchmin;
	}

	if( ev->NumArgs() > 2 )
	{
		yawmin = ev->GetFloat( 3 );
		yawmax = ev->GetFloat( 4 );

		if( pitchmin <= pitchmax )
		{
			viewkickmin[ firemodeindex ][ 1 ] = yawmin;
			viewkickmax[ firemodeindex ][ 1 ] = yawmax;
		}
		else
		{
			viewkickmin[ firemodeindex ][ 1 ] = yawmax;
			viewkickmax[ firemodeindex ][ 1 ] = yawmin;
		}
	}
}

void Weapon::SetMovementSpeed
	(
	Event *ev
	)

{
	if( g_gametype->integer )
		return;

	m_fMovementSpeed = ev->GetFloat( 1 );
}

void Weapon::SetDMMovementSpeed
	(
	Event *ev
	)

{
	if( !g_gametype->integer )
		return;

	m_fMovementSpeed = ev->GetFloat( 1 );
}

void Weapon::EventAmmoPickupSound
	(
	Event *ev
	)

{
	m_sAmmoPickupSound = ev->GetString( 1 );
}

void Weapon::EventNoAmmoSound
	(
	Event *ev
	)

{
	m_NoAmmoSound = ev->GetString( 1 );
}

float Weapon::GetBulletRange
	(
	firemode_t mode
	)

{
	return bulletrange[ mode ];
}
