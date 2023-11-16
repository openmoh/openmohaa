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
#include "g_phys.h"
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
#include "debuglines.h"
#include "g_spawn.h"

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
    "Empties the weapon's clip of ammo,"
    "returning it to the owner",
    EV_NORMAL
);
Event EV_Weapon_AddToClip
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
Event EV_Weapon_FireDelay
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
Event EV_Weapon_NotDroppable("notdroppable", EV_DEFAULT, NULL, NULL, "Makes a weapon not droppable");
Event EV_Weapon_SetAimAnim
(
    "setaimanim",
    EV_DEFAULT,
    "si",
    "aimAnimation aimFrame",
    "Set the aim animation and frame for when a weapon fires",
    EV_NORMAL
);
Event
    EV_Weapon_SetRank("rank", EV_DEFAULT, "ii", "iOrder iRank", "Set the order value and power ranking for the weapon");
Event EV_Weapon_SetFireType
(
    "firetype",
    EV_DEFAULT,
    "s",
    "firingType",
    "Set the firing type of the weapon (projectile or bullet)",
    EV_NORMAL
);
Event EV_Weapon_SetAIRange(
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
Event EV_Weapon_SetBulletLarge
(
    "bulletlarge",
    EV_DEFAULT,
    "i",
    "bulletType",
    "Set if the bullets fired are rifle bullets(1), artillery(2) or larger tracers(3)",
    EV_NORMAL
);
Event EV_Weapon_SetTracerSpeed
(
    "tracerspeed",
    EV_DEFAULT,
    "f",
    "speed",
    "Scale factor of how fast a tracer should travel (valid ranges 0-2)",
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
Event EV_Weapon_SetBulletThroughWood
(
    "throughwood",
    EV_DEFAULT,
    "f",
    "dist",
    "Sets how far the bullets can go through wood",
    EV_NORMAL
);
Event EV_Weapon_SetBulletThroughMetal
(
    "throughmetal",
    EV_DEFAULT,
    "f",
    "dist",
    "Sets how far the bullets can go through metal",
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
Event EV_Weapon_SetBulletSpread(
    "bulletspread",
    EV_DEFAULT,
    "ffFF",
    "bulletSpreadX bulletSpreadY bulletSpreadXmax bulletSpreadYmax",
    "Set the min & optional max spread of the bullet in the x and y axis",
    EV_NORMAL
);
Event EV_Weapon_SetDMBulletSpread(
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
    "Set the frequency of making tracers",
    EV_NORMAL
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
Event EV_Weapon_AddAdditionalStartAmmo
(
    "additionalstartammo",
    EV_DEFAULT,
    "si",
    "ammotype amount",
    "Gives some additional start ammo of the specified type",
    EV_NORMAL
);
Event EV_Weapon_AddStartItem
(
    "startitem",
    EV_DEFAULT,
    "s",
    "itemname",
    "Adds an item to the starting loadout of the weapon",
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
Event EV_Weapon_SetSecondaryAmmoInHud
(
    "secondaryammoinhud",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes the weapon show its secondary ammo to the hud.",
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
Event EV_Weapon_SetWorldHitSpawn(
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
Event EV_Weapon_SetType
(
    "weapontype",
    EV_DEFAULT,
    "s",
    "weapon_type",
    "Sets the weapon type",
    EV_NORMAL
);
Event EV_Weapon_SetGroup
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
    "Sets fov to zoom to on a secondary fire",
    EV_NORMAL
);
Event EV_Weapon_SetSemiAuto
(
    "semiauto",
    EV_DEFAULT,
    NULL,
    NULL,
    "Sets the weapon to fire semi-auto",
    EV_NORMAL
);
Event EV_Weapon_AttachToHand
(
    "attachtohand",
    EV_DEFAULT,
    "s",
    "weapon_hand",
    "Attaches an active weapon to the specified hand",
    EV_NORMAL
);
Event EV_Weapon_CantPartialReload
(
    "cantpartialreload",
    EV_DEFAULT,
    NULL,
    NULL,
    "Prevents the weapon from being reloaded part way through a clip",
    EV_NORMAL
);
Event EV_Weapon_DMCantPartialReload
(
    "dmcantpartialreload",
    EV_DEFAULT,
    NULL,
    NULL,
    "Prevents the weapon from being reloaded part way through a clip for DM",
    EV_NORMAL
);
Event EV_Weapon_FallingAngleAdjust
(
    "fallingangleadjust",
    EV_DEFAULT,
    0,
    0,
    "Adjusts the weapons angles as it falls to the ground",
    EV_NORMAL
);
Event EV_Weapon_SetViewKick
(
    "viewkick",
    EV_DEFAULT,
    "ffFF",
    "pitchmin pitchmax yawmin yawmax",
    "Adds kick to the view of the owner when fired.",
    EV_NORMAL
);
Event EV_Weapon_MovementSpeed
(
    "movementspeed",
    EV_DEFAULT,
    "f",
    "speedmult",
    "Alters the movement speed of the player when he has the weapon out",
    EV_NORMAL
);
Event EV_Weapon_DMMovementSpeed
(
    "dmmovementspeed",
    EV_DEFAULT,
    "f",
    "speedmult",
    "Alters the movement speed of the player when he has the weapon out",
    EV_NORMAL
);
Event EV_Weapon_MaxFireMovement
(
    "maxfiremovement",
    EV_DEFAULT,
    "f",
    "speedmult",
    "Sets the max speed the player can be moving to fire the weapon (fraction of weapon's running speed)",
    EV_NORMAL
);
Event EV_Weapon_ZoomMovement
(
    "zoommovement",
    EV_DEFAULT,
    "f",
    "speedmult",
    "Sets the max speed the player can move while zoomed (fraction of weapon's running speed)",
    EV_NORMAL
);
Event EV_Weapon_AmmoPickupSound
(
    "ammopickupsound",
    EV_DEFAULT,
    "s",
    "name",
    "sets the weapon's ammo pickup sound alias",
    EV_NORMAL
);
Event EV_Weapon_NoAmmoSound
(
    "noammosound",
    EV_DEFAULT,
    "s",
    "name",
    "sets the weapon's dry fire sound alias",
    EV_NORMAL
);
Event EV_Weapon_MaxMovementSound
(
    "maxmovementsound",
    EV_DEFAULT,
    "s",
    "name",
    "sets the weapon's movement fire prevention sound alias",
    EV_NORMAL
);
Event EV_Weapon_NumFireAnims
(
    "numfireanims",
    EV_DEFAULT,
    "i",
    "value",
    "Sets the number of fire animations this weapon uses.",
    EV_NORMAL
);
Event EV_Weapon_SetCurrentFireAnim
(
    "setcurrentfireanim",
    EV_DEFAULT,
    "i",
    "value",
    "Sets the current firing animation.",
    EV_NORMAL
);
Event EV_Weapon_SubType
(
    "weaponsubtype",
    EV_DEFAULT,
    "i",
    "subtype",
    "sets the weapon's sub-type. Used by smoke grenades.",
    EV_NORMAL
);
Event EV_SetCookTime
(
    "cooktime",
    EV_DEFAULT,
    "f",
    "cooktime",
    "sets weapons cook time.",
    EV_NORMAL
);
Event EV_OverCooked
(
    "overcooked",
    EV_DEFAULT,
    NULL,
    NULL,
    "used when the cookable weapon has been over cooked.",
    EV_NORMAL
);
Event EV_OverCooked_Warning
(
    "overcooked_warning",
    EV_DEFAULT,
    NULL,
    NULL,
    "causes a warning sound to play that the grenade is about to be overcooked.",
    EV_NORMAL
);

CLASS_DECLARATION(Item, Weapon, NULL) {
    {&EV_SetAnim,                       &Weapon::SetWeaponAnimEvent     },
    {&EV_Item_Pickup,                   &Weapon::PickupWeapon           },
    {&EV_Weapon_DoneRaising,            &Weapon::DoneRaising            },
    {&EV_Weapon_DoneFiring,             &Weapon::DoneFiring             },
    {&EV_Weapon_Idle,                   &Weapon::Idle                   },
    {&EV_Weapon_IdleInit,               &Weapon::IdleInit               },
    {&EV_BroadcastAIEvent,              &Weapon::WeaponSound            },
    {&EV_Weapon_DoneReloading,          &Weapon::DoneReloading          },
    {&EV_Weapon_SetAmmoClipSize,        &Weapon::SetAmmoClipSize        },
    {&EV_Weapon_SetAmmoInClip,          &Weapon::SetAmmoInClip          },
    {&EV_Weapon_SetShareClip,           &Weapon::SetShareClip           },
    {&EV_Weapon_FillClip,               &Weapon::FillAmmoClip           },
    {&EV_Weapon_EmptyClip,              &Weapon::EmptyAmmoClip          },
    {&EV_Weapon_AddToClip,              &Weapon::AddToAmmoClip          },
    {&EV_Weapon_SetMaxRange,            &Weapon::SetMaxRangeEvent       },
    {&EV_Weapon_SetMinRange,            &Weapon::SetMinRangeEvent       },
    {&EV_Weapon_FireDelay,              &Weapon::SetFireDelay           },
    {&EV_Weapon_NotDroppable,           &Weapon::NotDroppableEvent      },
    {&EV_Weapon_SetAimAnim,             &Weapon::SetAimAnim             },
    {&EV_Weapon_Shoot,                  &Weapon::Shoot                  },
    {&EV_Weapon_SetRank,                &Weapon::SetRankEvent           },
    {&EV_Weapon_SetFireType,            &Weapon::SetFireType            },
    {&EV_Weapon_SetAIRange,             &Weapon::SetAIRange             },
    {&EV_Weapon_SetProjectile,          &Weapon::SetProjectile          },
    {&EV_Weapon_SetDMProjectile,        &Weapon::SetDMProjectile        },
    {&EV_Weapon_SetBulletDamage,        &Weapon::SetBulletDamage        },
    {&EV_Weapon_SetBulletLarge,         &Weapon::SetBulletLarge         },
    {&EV_Weapon_SetTracerSpeed,         &Weapon::SetTracerSpeed         },
    {&EV_Weapon_SetBulletCount,         &Weapon::SetBulletCount         },
    {&EV_Weapon_SetBulletKnockback,     &Weapon::SetBulletKnockback     },
    {&EV_Weapon_SetBulletThroughWood,   &Weapon::SetBulletThroughWood   },
    {&EV_Weapon_SetBulletThroughMetal,  &Weapon::SetBulletThroughMetal  },
    {&EV_Weapon_SetBulletRange,         &Weapon::SetBulletRange         },
    {&EV_Weapon_SetRange,               &Weapon::SetRange               },
    {&EV_Weapon_SetBulletSpread,        &Weapon::SetBulletSpread        },
    {&EV_Weapon_SetTracerFrequency,     &Weapon::SetTracerFrequency     },
    {&EV_Weapon_Secondary,              &Weapon::Secondary              },
    {&EV_Weapon_AmmoType,               &Weapon::SetAmmoType            },
    {&EV_Weapon_StartAmmo,              &Weapon::SetStartAmmo           },
    {&EV_Weapon_AmmoRequired,           &Weapon::SetAmmoRequired        },
    {&EV_Weapon_MaxChargeTime,          &Weapon::SetMaxChargeTime       },
    {&EV_Weapon_MinChargeTime,          &Weapon::SetMinChargeTime       },
    {&EV_Weapon_AddAdditionalStartAmmo, &Weapon::AddAdditionalStartAmmo },
    {&EV_Weapon_AddStartItem,           &Weapon::AddStartItem           },
    {&EV_Weapon_GiveStartingAmmo,       &Weapon::GiveStartingAmmoToOwner},
    {&EV_Weapon_AutoAim,                &Weapon::AutoAim                },
    {&EV_Weapon_Crosshair,              &Weapon::Crosshair              },
    {&EV_Weapon_MainAttachToTag,        &Weapon::MainAttachToTag        },
    {&EV_Weapon_OffHandAttachToTag,     &Weapon::OffHandAttachToTag     },
    {&EV_Weapon_HolsterTag,             &Weapon::HolsterAttachToTag     },
    {&EV_Weapon_HolsterOffset,          &Weapon::SetHolsterOffset       },
    {&EV_Weapon_HolsterAngles,          &Weapon::SetHolsterAngles       },
    {&EV_Weapon_HolsterScale,           &Weapon::SetHolsterScale        },
    {&EV_Weapon_SetQuiet,               &Weapon::SetQuiet               },
    {&EV_Weapon_SetLoopFire,            &Weapon::SetLoopFire            },
    {&EV_Weapon_AutoPutaway,            &Weapon::SetAutoPutaway         },
    {&EV_Weapon_UseNoAmmo,              &Weapon::SetUseNoAmmo           },
    {&EV_Weapon_SetMeansOfDeath,        &Weapon::SetMeansOfDeath        },
    {&EV_Weapon_SetWorldHitSpawn,       &Weapon::SetWorldHitSpawn       },
    {&EV_Weapon_MakeNoise,              &Weapon::MakeNoise              },
    {&EV_Weapon_SetType,                &Weapon::SetWeaponType          },
    {&EV_Weapon_SetGroup,               &Weapon::SetWeaponGroup         },
    {&EV_Weapon_SetZoom,                &Weapon::SetZoom                },
    {&EV_Weapon_SetSemiAuto,            &Weapon::SetSemiAuto            },
    {&EV_Weapon_AttachToHand,           &Weapon::AttachToHand           },
    {&EV_Weapon_CantPartialReload,      &Weapon::SetCantPartialReload   },
    {&EV_Weapon_FallingAngleAdjust,     &Weapon::FallingAngleAdjust     },
    {&EV_Weapon_SetViewKick,            &Weapon::SetViewKick            },
    {&EV_Weapon_DMSetFireDelay,         &Weapon::SetDMFireDelay         },
    {&EV_Weapon_SetDMBulletDamage,      &Weapon::SetDMBulletDamage      },
    {&EV_Weapon_SetDMBulletCount,       &Weapon::SetDMBulletCount       },
    {&EV_Weapon_SetDMBulletSpread,      &Weapon::SetDMBulletSpread      },
    {&EV_Weapon_SetZoomSpreadMult,      &Weapon::SetZoomSpreadMult      },
    {&EV_Weapon_SetDMZoomSpreadMult,    &Weapon::SetDMZoomSpreadMult    },
    {&EV_Weapon_SetFireSpreadMult,      &Weapon::SetFireSpreadMult      },
    {&EV_Weapon_SetDMFireSpreadMult,    &Weapon::SetDMFireSpreadMult    },
    {&EV_Weapon_DMAmmoRequired,         &Weapon::SetDMAmmoRequired      },
    {&EV_Weapon_DMCantPartialReload,    &Weapon::SetDMCantPartialReload },
    {&EV_Weapon_DMStartAmmo,            &Weapon::SetDMStartAmmo         },
    {&EV_Weapon_SetDMBulletRange,       &Weapon::SetDMBulletRange       },
    {&EV_Weapon_DMCrosshair,            &Weapon::DMCrosshair            },
    {&EV_Weapon_MovementSpeed,          &Weapon::SetMovementSpeed       },
    {&EV_Weapon_DMMovementSpeed,        &Weapon::SetDMMovementSpeed     },
    {&EV_Weapon_MaxFireMovement,        &Weapon::SetMaxFireMovement     },
    {&EV_Weapon_ZoomMovement,           &Weapon::SetZoomMovement        },
    {&EV_Weapon_AmmoPickupSound,        &Weapon::EventAmmoPickupSound   },
    {&EV_Weapon_NoAmmoSound,            &Weapon::EventNoAmmoSound       },
    {&EV_Weapon_MaxMovementSound,       &Weapon::EventMaxMovementSound  },
    {&EV_Weapon_NumFireAnims,           &Weapon::SetNumFireAnims        },
    {&EV_Weapon_NumFireAnims,           &Weapon::SetWeaponSubtype       },
    {&EV_SetCookTime,                   &Weapon::SetCookTime            },
    {&EV_OverCooked,                    &Weapon::OnOverCooked           },
    {&EV_OverCooked_Warning,            &Weapon::OnOverCookedWarning    },
    {&EV_Weapon_SetCurrentFireAnim,     &Weapon::SetCurrentFireAnim     },
    {&EV_Weapon_SetSecondaryAmmoInHud,  &Weapon::SetSecondaryAmmoInHud  },
    {NULL,                              NULL                            }
};

//======================
//Weapon::Weapon
//======================
Weapon::Weapon()
{
    entflags |= ECF_WEAPON;

    mAIRange = RANGE_SHORT;

    if (LoadingSavegame) {
        // Archive function will setup all necessary data
        return;
    }

    // Set the weapon class to item by default
    weapon_class = WEAPON_CLASS_ITEM;
    order        = 0;

    // Owner of the weapon
    owner = NULL;

    // Maximum spread multiplier while firing
    m_fFireSpreadMultCap[0] = 0;
    m_fFireSpreadMultCap[1] = 0;

    // Starting rank of the weapon
    rank = 0;

    // Amount of ammo required for weapon
    INITIALIZE_WEAPONMODE_VAR(ammorequired, 0);

    // Starting ammo of the weapon
    INITIALIZE_WEAPONMODE_VAR(startammo, 0);

    // Amount of ammo the clip can hold
    INITIALIZE_WEAPONMODE_VAR(ammo_clip_size, 0);

    // Amount of ammo in clip
    INITIALIZE_WEAPONMODE_VAR(ammo_in_clip, 0);

    // Amount of time to pass before broadcasting a weapon sound again
    nextweaponsoundtime = 0;

    // Last fire state
    m_fLastFireTime = 0;
    m_eLastFireMode = (firemode_t)-11;

    // The initial state of the weapon
    weaponstate = WEAPON_HOLSTERED;

    // Is the weapon droppable when the owner is killed
    notdroppable = false;

    // Aim animation for behavior of monsters
    aimanim  = -1;
    aimframe = 0;

    m_iAnimSlot = 0;

    // start off unattached
    attached = false;

    // maximum effective firing distance (for autoaim)
    maxrange = 1000;

    // minimum safe firing distance (for AI)
    minrange = 0;

    // speed of the projectile (0 == infinite speed)
    memset(projectilespeed, 0, sizeof(projectilespeed));

    // Weapons don't move
    setMoveType(MOVETYPE_NONE);

    // What type of ammo this weapon fires
    INITIALIZE_WEAPONMODE_VAR(firetype, FT_NONE);

    INITIALIZE_WEAPONMODE_VAR(fire_delay, 0.1f);

    // Init the bullet specs
    INITIALIZE_WEAPONMODE_VAR(projectilespeed, 0);
    INITIALIZE_WEAPONMODE_VAR(bulletdamage, 0);
    INITIALIZE_WEAPONMODE_VAR(bulletlarge, 0);
    INITIALIZE_WEAPONMODE_VAR(bulletcount, 1);
    INITIALIZE_WEAPONMODE_VAR(bulletrange, 4096);
    INITIALIZE_WEAPONMODE_VAR(bulletknockback, 0);
    INITIALIZE_WEAPONMODE_VAR(bulletthroughwood, 0);
    INITIALIZE_WEAPONMODE_VAR(bulletthroughmetal, 0);
    INITIALIZE_WEAPONMODE_VAR(ammo_type, "");
    INITIALIZE_WEAPONMODE_VAR(loopfire, false);
    INITIALIZE_WEAPONMODE_VAR(quiet, qfalse);
    INITIALIZE_WEAPONMODE_VAR(loopfire, qfalse);
    INITIALIZE_WEAPONMODE_VAR(tracercount, 0);
    INITIALIZE_WEAPONMODE_VAR(tracerfrequency, 0);
    INITIALIZE_WEAPONMODE_VAR(tracerspeed, 1);

    for (int i = 0; i < MAX_FIREMODES; i++) {
        INITIALIZE_WEAPONMODE_VAR(viewkickmin[i], 0);
        INITIALIZE_WEAPONMODE_VAR(viewkickmax[i], 0);
    }

    INITIALIZE_WEAPONMODE_VAR(m_fFireSpreadMultAmount, 0);
    INITIALIZE_WEAPONMODE_VAR(m_fFireSpreadMultFalloff, 0);
    INITIALIZE_WEAPONMODE_VAR(m_fFireSpreadMultCap, 0);
    INITIALIZE_WEAPONMODE_VAR(m_fFireSpreadMultTime, 0);
    INITIALIZE_WEAPONMODE_VAR(m_fFireSpreadMult, 0);

    // Init the max amount of time a weapon may be charged (5 seconds)
    INITIALIZE_WEAPONMODE_VAR(min_charge_time, 0);
    INITIALIZE_WEAPONMODE_VAR(max_charge_time, 5);
    charge_fraction = 1.0f;

    // Tag to attach this weapon to on its owner when used in the left hand and in the right hand
    attachToTag_offhand = "tag_weapon_left";
    attachToTag_main    = "tag_weapon_right";

    // putaway is flagged true when the weapon should be put away by state machine
    putaway = false;

    // This is used for setting alternate fire functionality when initializing stuff
    firemodeindex = FIRE_PRIMARY;

    // Name and index
    setName("Unnamed Weapon");

    m_bCanPartialReload = qtrue;
    m_bShareClip        = qfalse;

    // do better lighting on all weapons
    edict->s.renderfx |= RF_EXTRALIGHT;

    // Weapons do not auto aim automatically
    autoaim = false;

    // No crosshair visible
    crosshair = false;

    m_iZoom           = 0;
    m_bAutoZoom       = false;
    m_fZoomSpreadMult = 1.0f;
    m_bSemiAuto       = false;

    // Weapons default to making noise
    next_noise_time       = 0;
    next_noammo_time      = 0;
    next_maxmovement_time = 0;

    // Obviously mustn't reload at first
    m_bShouldReload = false;

    // Used to keep track of last angles and scale before holstering
    lastValid    = qfalse;
    lastScale    = 1.0f;
    holsterScale = 1.0f;

    // Weapon will not be putaway by default when out of ammo
    auto_putaway = qfalse;

    // Weapon will be able to be used when it has no ammo
    use_no_ammo = qtrue;

    INITIALIZE_WEAPONMODE_VAR(meansofdeath, MOD_NONE);

    // Set the stats
    m_iNumHits          = 0;
    m_iNumGroinShots    = 0;
    m_iNumHeadShots     = 0;
    m_iNumLeftArmShots  = 0;
    m_iNumRightArmShots = 0;
    m_iNumLeftLegShots  = 0;
    m_iNumRightLegShots = 0;
    m_iNumTorsoShots    = 0;

    // Set the default weapon group
    m_csWeaponGroup    = STRING_EMPTY;
    m_fMovementSpeed   = 1.0f;
    m_fMaxFireMovement = 1.0f;
    m_fZoomMovement    = 1.0f;

    m_sAmmoPickupSound  = "snd_pickup_";
    m_NoAmmoSound       = "snd_noammo";
    m_sMaxMovementSound = "snd_maxmovement";

    // Always has a fire animation
    m_iNumFireAnims    = 1;
    m_iCurrentFireAnim = 0;

    // Default tag to use for muzzle and special effects
    m_sTagBarrel = "tag_barrel";

    m_iWeaponSubtype = 0;

    // Not cooking by default
    m_fCookTime      = 0;
    m_eCookModeIndex = FIRE_PRIMARY;
    // Defaults to no secondary HUD
    m_bSecondaryAmmoInHud = false;

    PostEvent(EV_Weapon_IdleInit, 0);

    last_owner_trigger_time = 0;
}

//======================
//Weapon::Weapon
//======================
Weapon::Weapon(const char *file)
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

    if (owner) {
        RemoveFromOwner();
    }

    entflags &= ~ECF_WEAPON;
}

//======================
//Weapon::Delete
//======================
void Weapon::Delete(void)
{
    if (g_iInThinks) {
        DetachGun();

        if (owner) {
            RemoveFromOwner();
        }

        PostEvent(EV_Remove, 0);
    } else {
        delete this;
    }
}

//======================
//Weapon::SetRankEvent
//======================
void Weapon::SetRankEvent(Event *ev)
{
    SetRank(ev->GetInteger(1), ev->GetInteger(2));
}

//======================
//Weapon::SetWeaponType
//======================
void Weapon::SetWeaponType(Event *ev)
{
    weapon_class = G_WeaponClassNameToNum(ev->GetString(1));
}

//======================
//Weapon::SetWeaponGroup
//======================
void Weapon::SetWeaponGroup(Event *ev)
{
    m_csWeaponGroup = ev->GetConstString(1);
}

//======================
//Weapon::SetZoom
//======================
void Weapon::SetZoom(Event *ev)
{
    m_iZoom = ev->GetInteger(1);

    if (ev->NumArgs() > 1) {
        int autozoom = ev->GetInteger(2);

        if (autozoom) {
            m_bAutoZoom = qtrue;
        }
    }
}

//======================
//Weapon::SetSemiAuto
//======================
void Weapon::SetSemiAuto(Event *ev)
{
    m_bSemiAuto = qtrue;
}

//======================
//Weapon::SetAutoPutaway
//======================
void Weapon::SetAutoPutaway(Event *ev)
{
    auto_putaway = ev->GetBoolean(1);
}

//======================
//Weapon::SetUseNoAmmo
//======================
void Weapon::SetUseNoAmmo(Event *ev)
{
    use_no_ammo = ev->GetBoolean(1);
}

//======================
//Weapon::SetStartAmmo
//======================
void Weapon::SetStartAmmo(Event *ev)
{
    if (g_protocol <= protocol_e::PROTOCOL_MOH && g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    startammo[firemodeindex] = ev->GetInteger(1);
}

//======================
//Weapon::SetDMStartAmmo
//======================
void Weapon::SetDMStartAmmo(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    startammo[firemodeindex] = ev->GetInteger(1);
}

//======================
//Weapon::SetMaxChargeTime
//======================
void Weapon::SetMaxChargeTime(Event *ev)
{
    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    max_charge_time[firemodeindex] = ev->GetFloat(1);
}

//======================
//Weapon::SetMaxChargeTime
//======================
void Weapon::SetMinChargeTime(Event *ev)
{
    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    min_charge_time[firemodeindex] = ev->GetFloat(1);
}

//======================
//Weapon::GetMinChargeTime
//======================
float Weapon::GetMinChargeTime(firemode_t mode)
{
    assert((mode >= 0) && (mode < MAX_FIREMODES));
    return min_charge_time[mode];
}

//======================
//Weapon::GetMinChargeTime
//======================
float Weapon::GetMaxChargeTime(firemode_t mode)
{
    assert((mode >= 0) && (mode < MAX_FIREMODES));
    return max_charge_time[mode];
}

//======================
//Weapon::SetAmmoRequired
//======================
void Weapon::SetAmmoRequired(Event *ev)
{
    if (g_protocol <= protocol_e::PROTOCOL_MOH && g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    ammorequired[firemodeindex] = ev->GetInteger(1);
}

//======================
//Weapon::SetDMAmmoRequired
//======================
void Weapon::SetDMAmmoRequired(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    ammorequired[firemodeindex] = ev->GetInteger(1);
}

//======================
//Weapon::GetAmmoType
//======================
str Weapon::GetAmmoType(firemode_t mode)
{
    assert((mode >= 0) && (mode < MAX_FIREMODES));

    mode = m_bShareClip ? FIRE_PRIMARY : mode;
    return ammo_type[mode];
}

//======================
//Weapon::SetAmmoType
//======================
void Weapon::SetAmmoType(Event *ev)
{
    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));

    if ((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES)) {
        ammo_type[firemodeindex] = ev->GetString(1);
    } else {
        warning("Weapon::SetAmmoType", "Invalid mode %d\n", firemodeindex);
        return;
    }
}

//======================
//Weapon::SetAmmoAmount
//======================
void Weapon::SetAmmoAmount(int amount, firemode_t mode)
{
    assert((mode >= 0) && (mode < MAX_FIREMODES));

    mode = m_bShareClip ? FIRE_PRIMARY : mode;

    // If the clip can hold ammo, then set the amount in the clip to the specified amount
    if (ammo_clip_size[mode]) {
        ammo_in_clip[mode] = amount;
    }
}

//======================
//Weapon::GetClipSize
//======================
int Weapon::GetClipSize(firemode_t mode)
{
    assert((mode >= 0) && (mode < MAX_FIREMODES));

    mode = m_bShareClip ? FIRE_PRIMARY : mode;

    if ((mode >= 0) && (mode < MAX_FIREMODES)) {
        return ammo_clip_size[mode];
    } else {
        warning("Weapon::GetClipSize", "Invalid mode %d\n", mode);
        return 0;
    }
}

//======================
//Weapon::UseAmmo
//======================
void Weapon::UseAmmo(int amount, firemode_t mode)
{
    mode = m_bShareClip ? FIRE_PRIMARY : mode;

    if (UnlimitedAmmo(mode) && (!owner || !owner->isClient())) {
        return;
    }

    // Remove ammo from the clip if it's available
    if (ammo_clip_size[mode]) {
        ammo_in_clip[mode] -= amount;
        if (ammo_in_clip[mode] < 0) {
            warning("UseAmmo", "Used more ammo than in clip.\n");
            ammo_in_clip[mode] = 0;
        }
        if (!ammo_in_clip[mode]) {
            SetShouldReload(qtrue);
        }
        owner->AmmoAmountInClipChanged(ammo_type[mode], ammo_in_clip[mode]);
    } else {
        assert(owner);
        if (owner && owner->isClient() && !UnlimitedAmmo(mode)) {
            // Remove ammo from the player's inventory
            owner->UseAmmo(ammo_type[mode], ammorequired[mode]);
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
void Weapon::GetMuzzlePosition(vec3_t position, vec3_t vBarrelPos, vec3_t forward, vec3_t right, vec3_t up)
{
    orientation_t weap_or, barrel_or, orient;
    Vector        delta;
    Vector        aim_angles;
    int           tagnum;
    int           i;

    owner = (Player *)this->owner.Pointer();
    assert(owner);

    // We should always have an owner
    if (!owner) {
        if (IsSubclassOfVehicleTurretGun()) {
            VehicleTurretGun *t = (VehicleTurretGun *)this;
            owner               = (Player *)t->GetRemoteOwner().Pointer();
        }
    }

    if (owner) {
        tagnum = gi.Tag_NumForName(owner->edict->tiki, current_attachToTag.c_str());

        // Get the orientation based on the frame and anim stored off in the owner.
        // This is to prevent weird timing with getting orientations on different frames of firing
        // animations and the orientations will not be consistent.
        AnglesToAxis(owner->angles, owner->orientation);

        orient = G_TIKI_Orientation(owner->edict, tagnum);

        if (owner->IsSubclassOfPlayer()) {
            Player *player;

            // Use the player's torso angles to determine direction, but use the actual barrel to determine position

            player = static_cast<Player *>(owner.Pointer());

            if (forward || right || up) {
                AngleVectors(player->m_vViewAng, forward, right, up);
            }

            VectorCopy(player->m_vViewPos, position);

            if (this->GetRawTag(GetTagBarrel(), &barrel_or) && vBarrelPos) {
                VectorCopy(owner->origin, vBarrelPos);
                for (i = 0; i < 3; i++) {
                    VectorMA(vBarrelPos, orient.origin[i], owner->orientation[i], vBarrelPos);
                }

                MatrixMultiply(orient.axis, owner->orientation, weap_or.axis);
                for (i = 0; i < 3; i++) {
                    VectorMA(vBarrelPos, barrel_or.origin[i], weap_or.axis[i], vBarrelPos);
                }
            } else if (vBarrelPos) {
                VectorCopy(position, vBarrelPos);
            }
        } else {
            VectorCopy(owner->origin, position);

            for (i = 0; i < 3; i++) {
                VectorMA(position, orient.origin[i], owner->orientation[i], position);
            }

            MatrixMultiply(orient.axis, owner->orientation, weap_or.axis);

            if (this->GetRawTag(GetTagBarrel(), &barrel_or)) {
                for (i = 0; i < 3; i++) {
                    VectorMA(position, barrel_or.origin[i], weap_or.axis[i], position);
                }
            }

            if (vBarrelPos) {
                VectorCopy(position, vBarrelPos);
            }

            delta      = owner->GunTarget(false, position, weap_or.axis[0]) - position;
            aim_angles = delta.toAngles();

            if (forward || right || up) {
                AngleVectors(aim_angles, forward, right, up);
            }
        }
    } else {
        if (forward || right || up) {
            AngleVectors(angles, forward, right, up);
        }

        VectorCopy(origin, position);

        if (this->GetRawTag(GetTagBarrel(), &barrel_or)) {
            AnglesToAxis(angles, weap_or.axis);

            for (i = 0; i < 3; i++) {
                VectorMA(position, barrel_or.origin[i], weap_or.axis[i], position);
            }
        }

        if (vBarrelPos) {
            VectorCopy(position, vBarrelPos);
        }
    }
}

//======================
//Weapon::SetAmmoClipSize
//======================
void Weapon::SetAmmoClipSize(Event *ev)
{
    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    ammo_clip_size[firemodeindex] = ev->GetInteger(1);
}

//======================
//Weapon::SetAmmoInClip
//======================
void Weapon::SetAmmoInClip(Event *ev)
{
    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    ammo_in_clip[firemodeindex] = ev->GetInteger(1);
}

//======================
//Weapon::SetShareClip
//======================
void Weapon::SetShareClip(Event *ev)
{
    m_bShareClip = qtrue;
}

//======================
//Weapon::SetTagBarrel
//======================
void Weapon::SetTagBarrel(const char *tagBarrel)
{
    m_sTagBarrel = tagBarrel;
}

//======================
//Weapon::GetTagBarrel
//======================
str Weapon::GetTagBarrel() const
{
    return m_sTagBarrel;
}

//======================
//Weapon::Shoot
//======================
void Weapon::Shoot(Event *ev)
{
    Vector     pos, forward, right, up, vBarrel, delta;
    firemode_t mode = FIRE_PRIMARY;
    qboolean   mc;

    if (ev->NumArgs() > 0) {
        mode = WeaponModeNameToNum(ev->GetString(1));

        if (mode == FIRE_ERROR) {
            return;
        }

        if (ev->NumArgs() > 1) {
            SetTagBarrel(ev->GetString(2));
        }
    }

    if (owner && owner->IsSubclassOfSentient() && owner->m_bOvercookDied) {
        owner->m_bOvercookDied = false;
        return;
    }

    mc = MuzzleClear();

    // If we are in loopfire, we need to keep checking ammo and using it up
    if (loopfire[mode]) {
        if (HasAmmo(mode) && mc) {
            // Use up the appropriate amount of ammo, it's already been checked that we have enough
            UseAmmo(ammorequired[mode], mode);
        } else {
            ForceIdle();
        }
    }

    // If the muzzle is not clear, then change to a clear animation, otherwise punt out.
    if (!mc) {
        ForceIdle();
        return;
    }

    GetMuzzlePosition(pos, vBarrel, forward, right, up);
    ApplyFireKickback(forward, 1000.0);

    if (firetype[mode] != FT_LANDMINE || CanPlaceLandmine(pos, owner)) {
        if (m_fFireSpreadMultAmount[mode] != 0.0f) {
            float fTime = level.time - m_fFireSpreadMultTime[mode];

            if (fTime <= m_fFireSpreadMultTimeCap[mode]) {
                float fDecay = fTime * m_fFireSpreadMultFalloff[mode];

                if (m_fFireSpreadMult[mode] <= 0.0f) {
                    m_fFireSpreadMult[mode] -= fDecay;

                    if (m_fFireSpreadMult[mode] > 0.0f) {
                        m_fFireSpreadMult[mode] = 0.0f;
                    }
                } else {
                    m_fFireSpreadMult[mode] -= fDecay;

                    if (m_fFireSpreadMult[mode] < 0.0f) {
                        m_fFireSpreadMult[mode] = 0.0f;
                    }
                }
            } else {
                m_fFireSpreadMult[mode] = 0.0f;
            }

            m_fFireSpreadMultTime[mode] = level.time;
        }

        switch (firetype[mode]) {
        case FT_PROJECTILE:
            ProjectileAttack(pos, forward, owner, projectileModel[mode], charge_fraction);
            break;
        case FT_LANDMINE:
            PlaceLandmine(pos, owner, projectileModel[mode], this);
            break;
        case FT_DEFUSE:
            DefuseObject(right, owner, bulletrange[mode]);
            break;
        case FT_BULLET:
            {
                Vector            vSpread;
                float             fSpreadFactor;
                int               tracerFrequency;
                SafePtr<Sentient> ownerPtr;

                if (owner) {
                    if (owner->client) {
                        Player *player = (Player *)owner.Pointer();

                        fSpreadFactor = player->velocity.length() / sv_runspeed->integer;

                        if (fSpreadFactor > 1.0f) {
                            fSpreadFactor = 1.0f;
                        }

                        vSpread       = bulletspreadmax[mode] * fSpreadFactor;
                        fSpreadFactor = 1.0f - fSpreadFactor;
                        vSpread += bulletspread[mode] * fSpreadFactor;
                        vSpread *= m_fFireSpreadMult[mode] + 1.0f;

                        if (m_iZoom) {
                            if (player->IsSubclassOfPlayer() && player->IsZoomed()) {
                                vSpread *= 1.0f + fSpreadFactor * (m_fZoomSpreadMult - 1.0f);
                            }
                        }
                    }
                } else {
                    vSpread = (bulletspreadmax[mode] + bulletspread[mode]) * 0.5f;
                }

                if (!g_gametype->integer && owner && owner->IsSubclassOfPlayer()) {
                    if (IsSubclassOfTurretGun()) {
                        tracerFrequency = 3;
                    } else {
                        tracerFrequency = 0;
                    }
                } else {
                    tracerFrequency = tracerfrequency[mode];
                }

                ownerPtr = owner;

                if (!owner && IsSubclassOfVehicleTurretGun()) {
                    VehicleTurretGun *turretGun = static_cast<VehicleTurretGun *>(this);
                    ownerPtr                    = turretGun->GetRemoteOwner();
                }

                BulletAttack(
                    pos,
                    vBarrel,
                    forward,
                    right,
                    up,
                    bulletrange[mode],
                    bulletdamage[mode],
                    bulletlarge[mode],
                    bulletknockback[mode],
                    0,
                    GetMeansOfDeath(mode),
                    vSpread,
                    bulletcount[mode],
                    ownerPtr,
                    tracerFrequency,
                    &tracercount[mode],
                    bulletthroughwood[mode],
                    bulletthroughmetal[mode],
                    this,
                    tracerspeed[mode]
                );
            }
            break;
        case FT_FAKEBULLET:
            {
                Vector vSpread;
                float  fSpreadFactor;

                if (owner) {
                    if (owner->client) {
                        Player *player = (Player *)owner.Pointer();

                        fSpreadFactor = player->velocity.length() / sv_runspeed->integer;

                        if (fSpreadFactor > 1.0f) {
                            fSpreadFactor = 1.0f;
                        }

                        vSpread       = bulletspreadmax[mode] * fSpreadFactor;
                        fSpreadFactor = 1.0f - fSpreadFactor;
                        vSpread += bulletspread[mode] * fSpreadFactor;
                        vSpread *= m_fFireSpreadMult[mode] + 1.0f;

                        if (m_iZoom) {
                            if (player->IsSubclassOfPlayer() && player->IsZoomed()) {
                                vSpread *= 1.0f + fSpreadFactor * (m_fZoomSpreadMult - 1.0f);
                            }
                        }
                    }
                } else {
                    vSpread = (bulletspreadmax[mode] + bulletspread[mode]) * 0.5f;
                }

                FakeBulletAttack(
                    pos,
                    vBarrel,
                    forward,
                    right,
                    up,
                    bulletrange[mode],
                    bulletdamage[mode],
                    bulletlarge[mode],
                    vSpread,
                    bulletcount[mode],
                    owner,
                    tracerfrequency[mode],
                    &tracercount[mode],
                    tracerspeed[mode]
                );
            }
            break;
        case FT_SPECIAL_PROJECTILE:
            SpecialFireProjectile(pos, forward, right, up, owner, projectileModel[mode], charge_fraction);
            break;
        case FT_CLICKITEM:
            ClickItemAttack(pos, forward, bulletrange[mode], owner);
            break;
        case FT_HEAVY:
            if (owner || !IsSubclassOfVehicleTurretGun()) {
                HeavyAttack(pos, forward, projectileModel[mode], 0, owner, this);
            } else {
                VehicleTurretGun *turret = (VehicleTurretGun *)this;

                if (turret->UseRemoteControl() && turret->GetRemoteOwner()) {
                    HeavyAttack(pos, forward, projectileModel[mode], 0, turret->GetRemoteOwner(), this);
                } else {
                    HeavyAttack(pos, forward, projectileModel[mode], 0, this, this);
                }
            }
            break;
        case FT_MELEE:
            {
                Vector         melee_pos, melee_end;
                Vector         dir;
                float          damage;
                meansOfDeath_t meansofdeath;
                float          knockback;

                if (owner) {
                    dir = owner->centroid - pos;
                } else {
                    dir = centroid - pos;
                }

                dir.z = 0;

                melee_pos = pos - forward * dir.length();
                melee_end = melee_pos + forward * bulletrange[mode];

                damage    = bulletdamage[mode];
                knockback = 0;

                meansofdeath = GetMeansOfDeath(mode);

                Container<Entity *> victimlist;

                m_iNumShotsFired++;
                if (MeleeAttack(
                        melee_pos, melee_end, damage, owner, meansofdeath, 8, -8, 8, knockback, true, &victimlist
                    )) {
                    m_iNumHits++;
                    m_iNumTorsoShots++;
                }
            }
            break;
        default:
            // none or unsupported
            break;
        }

        if (!quiet[firemodeindex]) {
            if (next_noise_time <= level.time) {
                BroadcastAIEvent(AI_EVENT_WEAPON_FIRE);
                next_noise_time = level.time + 1;
            }
        }

        if (owner && owner->client) {
            Vector vAngles = owner->GetViewAngles();

            if (viewkickmin[mode][0] != 0.0f || viewkickmax[mode][0] != 0.0f) {
                vAngles[0] += random() * (viewkickmax[mode][0] - viewkickmin[mode][0]) + viewkickmin[mode][0];
            }

            if (viewkickmin[1][0] != 0.0f || viewkickmax[1][0] != 0.0f) {
                vAngles[1] += random() * (viewkickmax[mode][1] - viewkickmin[mode][1]) + viewkickmin[mode][1];
            }

            owner->SetViewAngles(vAngles);
        }

        if (m_fFireSpreadMultAmount[mode]) {
            m_fFireSpreadMult[mode] += m_fFireSpreadMultAmount[mode];

            if (m_fFireSpreadMultCap[mode] < 0.0f) {
                if (m_fFireSpreadMultCap[mode] > m_fFireSpreadMult[mode]) {
                    m_fFireSpreadMult[mode] = m_fFireSpreadMultCap[mode];
                } else if (m_fFireSpreadMult[mode] > 0.0f) {
                    m_fFireSpreadMult[mode] = 0.0f;
                }
            } else if (m_fFireSpreadMult[mode] <= m_fFireSpreadMultCap[mode] && m_fFireSpreadMult[mode] < 0.0f) {
                m_fFireSpreadMult[mode] = 0.0f;
            }
        }

        m_fLastFireTime = level.time;
        m_eLastFireMode = mode;
    } else {
        // Landmine weapon
        if (ammo_clip_size[mode]) {
            UseAmmo(-ammorequired[mode], mode);
        } else if (owner && owner->isClient() && !UnlimitedAmmo(mode)) {
            owner->UseAmmo(ammo_type[mode], -ammorequired[mode]);
        }
    }
}

//======================
//Weapon::ApplyFireKickback
//======================
void Weapon::ApplyFireKickback(const Vector& org, float kickback) {}

//======================
//Weapon::SetAimAnim
//======================
void Weapon::SetAimAnim(Event *ev)
{
    str anim;

    anim     = ev->GetString(1);
    aimanim  = gi.Anim_NumForName(edict->tiki, anim.c_str());
    aimframe = ev->GetInteger(2);
}

//======================
//Weapon::SetOwner
//======================
void Weapon::SetOwner(Sentient *ent)
{
    assert(ent);
    if (!ent) {
        // return to avoid any buggy behaviour
        return;
    }

    Item::SetOwner(ent);

    setOrigin(vec_zero);
    setAngles(vec_zero);
}

//======================
//Weapon::AmmoAvailable
//======================
int Weapon::AmmoAvailable(firemode_t mode)
{
    // Returns the amount of ammo the owner has that is available for use
    assert((mode >= 0) && (mode < MAX_FIREMODES));

    mode = m_bShareClip ? FIRE_PRIMARY : mode;

    // Make sure there is an owner before querying the amount of ammo
    if (owner) {
        return owner->AmmoCount(ammo_type[mode]);
    } else {
        if (ammo_clip_size[mode]) {
            return ammo_clip_size[mode];
        } else {
            return ammorequired[mode];
        }
    }
}

//======================
//Weapon::UnlimitedAmmo
//======================
qboolean Weapon::UnlimitedAmmo(firemode_t mode)
{
    if (!owner) {
        return true;
    }

    if (!owner->isClient() || DM_FLAG(DF_INFINITE_AMMO)) {
        return true;
    } else if (!Q_stricmp(ammo_type[mode], "None")) {
        return true;
    }

    return false;
}

//======================
//Weapon::HasAmmo
//======================
qboolean Weapon::HasAmmo(firemode_t mode)
{
    assert((mode >= 0) && (mode < MAX_FIREMODES));

    if (!((mode >= 0) && (mode < MAX_FIREMODES))) {
        warning("Weapon::HasAmmo", "Invalid mode %d\n", mode);
        return false;
    }

    if (m_bShareClip) {
        // share the clip
        mode = FIRE_PRIMARY;
    }

    if (UnlimitedAmmo(mode)) {
        return true;
    }

    // If the weapon uses a clip, check for ammo in the right clip
    if (ammo_clip_size[mode] && HasAmmoInClip(mode)) {
        return true;
    } else // Otherwise check if ammo is available in general
    {
        if (!ammorequired[mode]) {
            return true;
        }

        return (AmmoAvailable(mode) >= ammorequired[mode]);
    }
}

//======================
//Weapon::HasAmmoInClip
//======================
qboolean Weapon::HasAmmoInClip(firemode_t mode)
{
    assert((mode >= 0) && (mode < MAX_FIREMODES));

    if (!((mode >= 0) && (mode < MAX_FIREMODES))) {
        warning("Weapon::HasAmmoInClip", "Invalid mode %d\n", mode);
    }

    mode = m_bShareClip ? FIRE_PRIMARY : mode;

    if (ammo_clip_size[mode]) {
        if (ammo_in_clip[mode] >= ammorequired[mode]) {
            return true;
        }
    } else {
        if (!ammorequired[mode]) {
            return true;
        }

        return (AmmoAvailable(mode) >= ammorequired[mode]);
    }

    return false;
}

//======================
//Weapon::ReadyToFire
//======================
qboolean Weapon::ReadyToFire(firemode_t mode, qboolean playsound)
{
    if (owner && owner->IsSubclassOfSentient()) {
        // Clear the cook flag
        owner->m_bOvercookDied = false;
    }

    if (!level.playerfrozen && m_iZoom && mode == FIRE_SECONDARY) {
        return qtrue;
    }

    // Make sure the weapon is in the ready state and the weapon has ammo
    if (m_eLastFireMode != mode || level.time > (m_fLastFireTime + FireDelay(mode))) {
        if (HasAmmoInClip(mode)) {
            if (m_fMaxFireMovement >= 1.f) {
                return qtrue;
            }

            if (!owner) {
                return qtrue;
            }

            if ((owner->velocity.lengthXY() / sv_runspeed->value) <= (m_fMovementSpeed * m_fMaxFireMovement)) {
                return qtrue;
            }

            if (playsound && (level.time > next_maxmovement_time)) {
                Sound(m_sMaxMovementSound);
                next_maxmovement_time = level.time + level.frametime + G_Random(0.1f) + 0.95f;
            }
        }

        if (playsound && (level.time > next_noammo_time)) {
            Sound(m_NoAmmoSound);
            next_noammo_time = level.time + level.frametime + G_Random(0.1f) + 0.95f;
        }
    }
    return qfalse;
}

//======================
//Weapon::PutAway
//======================
void Weapon::PutAway(void)
{
    // set the putaway flag to true, so the state machine know to put this weapon away
    putaway = true;
}

//======================
//Weapon::DetachFromOwner
//======================
void Weapon::DetachFromOwner(void)
{
    DetachGun();
    weaponstate = WEAPON_HOLSTERED;
}

//======================
//Weapon::AttachToOwner
//======================
void Weapon::AttachToOwner(weaponhand_t hand)
{
    AttachGun(hand);
    ForceIdle();
}

//======================
//Weapon::AttachToHolster
//======================
void Weapon::AttachToHolster(weaponhand_t hand)
{
    AttachGun(hand, qtrue);
    SetWeaponAnim("holster", EV_Weapon_Idle);
}

//======================
//Weapon::Drop
//======================
qboolean Weapon::Drop(void)
{
    Vector temp;

    if (!owner) {
        return false;
    }

    if (!IsDroppable()) {
        return false;
    }

    if (attached) {
        Vector        vAng;
        orientation_t oTag;
        Entity       *pParent;

        if (edict->s.parent == ENTITYNUM_NONE || edict->s.tag_num < 0) {
            pParent = owner;
            vAng    = pParent->angles;
        } else {
            vAng    = vec_zero;
            pParent = G_GetEntity(edict->s.parent);

            AnglesToAxis(pParent->angles, pParent->orientation);
            pParent->GetTag(edict->s.tag_num, &oTag);
            MatrixToEulerAngles(oTag.axis, vAng);
        }

        setAngles(vAng);
        DetachGun();
    } else {
        temp[2] = 40;

        setOrigin(owner->origin + temp);
        setAngles(owner->angles);
    }

    setSize(Vector(-12, -12, -2), Vector(12, 12, 12));

    // stop animating
    StopWeaponAnim();

    // drop the weapon
    PlaceItem();

    temp    = centroid - owner->centroid;
    temp[2] = 0;

    VectorNormalize(temp);

    temp *= 75.0f;

    temp[0] += G_CRandom(25);
    temp[1] += G_CRandom(25);
    temp[2]  = G_CRandom(50) + 150;
    velocity = owner->velocity * 0.5 + temp;

    avelocity = Vector(0, G_CRandom(120), 0);
    spawnflags |= DROPPED_PLAYER_ITEM;

    if (owner && owner->isClient()) {
        if (owner->deadflag && g_dropclips->integer > 0) {
            int ammo;

            if (ammo_clip_size[FIRE_PRIMARY]) {
                startammo[FIRE_PRIMARY] = g_dropclips->integer * ammo_in_clip[FIRE_PRIMARY];
            } else {
                startammo[FIRE_PRIMARY] = g_dropclips->integer;
            }

            ammo = AmmoAvailable(FIRE_PRIMARY);
            if (startammo[FIRE_PRIMARY] > ammo) {
                startammo[FIRE_PRIMARY] = ammo;
            }

            if (ammo_clip_size[FIRE_SECONDARY]) {
                startammo[FIRE_SECONDARY] = g_dropclips->integer * ammo_in_clip[FIRE_SECONDARY];
            } else {
                startammo[FIRE_SECONDARY] = g_dropclips->integer;
            }

            ammo = AmmoAvailable(FIRE_SECONDARY);
            if (startammo[FIRE_SECONDARY] > ammo) {
                startammo[FIRE_SECONDARY] = ammo;
            }

            ammo_in_clip[FIRE_PRIMARY]   = 0;
            ammo_in_clip[FIRE_SECONDARY] = 0;
        } else {
            startammo[FIRE_PRIMARY] = AmmoAvailable(FIRE_PRIMARY);
            owner->takeAmmoType(ammo_type[FIRE_PRIMARY]);
        }
    } else {
        if (ammo_clip_size[FIRE_PRIMARY] && ammo_in_clip[FIRE_PRIMARY]) {
            startammo[FIRE_PRIMARY] = ammo_in_clip[FIRE_PRIMARY];
        } else {
            startammo[FIRE_PRIMARY] >>= 2;
        }

        if (ammo_clip_size[FIRE_SECONDARY] && ammo_in_clip[FIRE_SECONDARY]) {
            startammo[FIRE_SECONDARY] = ammo_in_clip[FIRE_SECONDARY];
        } else {
            startammo[FIRE_SECONDARY] >>= 2;
        }

        if (startammo[FIRE_PRIMARY] == 0) {
            startammo[FIRE_PRIMARY] = 1;
        }

        if (startammo[FIRE_SECONDARY] == 0) {
            startammo[FIRE_SECONDARY] = 1;
        }
    }

    // Wait some time before the last owner can pickup this weapon
    last_owner              = owner;
    last_owner_trigger_time = level.time + 2.0f;

    // Cancel reloading events
    CancelEventsOfType(EV_Weapon_DoneReloading);

    // Remove this from the owner's item list
    RemoveFromOwner();

    PostEvent(EV_Remove, g_droppeditemlife->value);
    PostEvent(EV_Weapon_FallingAngleAdjust, level.frametime);
    return true;
}

//======================
//Weapon::Charge
//======================
void Weapon::Charge(firemode_t mode)
{
    if (mode == FIRE_PRIMARY) {
        if (m_fCookTime > 0) {
            m_eCookModeIndex = mode;
            //
            // Post cook event
            //
            PostEvent(EV_OverCooked, m_fCookTime);
            PostEvent(EV_OverCooked_Warning, m_fCookTime - 1);
        }
        SetWeaponAnim("charge");
    } else if (mode == FIRE_SECONDARY) {
        if (m_fCookTime > 0) {
            m_eCookModeIndex = mode;
            //
            // Post cook event
            //
            PostEvent(EV_OverCooked, m_fCookTime);
            PostEvent(EV_OverCooked_Warning, m_fCookTime - 1);
        }
        SetWeaponAnim("secondarycharge");
    }
}

//======================
//Weapon::OnOverCookedWarning
//======================
void Weapon::OnOverCookedWarning(Event *ev) {}

//======================
//Weapon::OnOverCooked
//======================
void Weapon::OnOverCooked(Event *ev)
{
    if (!owner) {
        return;
    }

    if (projectileModel[m_eCookModeIndex].length()) {
        Entity *spawnedEnt;

        SpawnArgs sp;
        sp.setArg("model", projectileModel[m_eCookModeIndex]);

        spawnedEnt = static_cast<Entity *>(sp.Spawn());
        if (spawnedEnt && spawnedEnt->IsSubclassOfProjectile()) {
            Event      *newev;
            Projectile *proj = static_cast<Projectile *>(spawnedEnt);
            trace_t     trace;

            proj->origin            = owner->origin;
            proj->angles            = owner->angles;
            proj->owner             = owner->entnum;
            proj->edict->r.ownerNum = owner->entnum;

            trace = G_Trace(
                proj->origin,
                vec_zero,
                vec_zero,
                owner->origin,
                static_cast<Entity *>(owner.Pointer()),
                owner->edict->clipmask,
                qfalse,
                "Weapon::OnOverCooked"
            );

            if (trace.ent && trace.ent->entity->entnum == world->entnum) {
                proj->origin = owner->origin;
            }

            newev = new Event(EV_Projectile_Explode);
            newev->AddEntity(NULL);
            if (g_gametype->integer == GT_SINGLE_PLAYER) {
                newev->AddFloat(1000.0);
            }

            if (owner->IsSubclassOfSentient()) {
                owner->m_bOvercookDied = true;
            }

            proj->ProcessEvent(newev);
        }
    }
}

//======================
//Weapon::ReleaseFire
//======================
void Weapon::ReleaseFire(firemode_t mode, float charge_time)
{
    // Make sure to stop the wepaon from cooking
    CancelEventsOfType(EV_OverCooked);
    CancelEventsOfType(EV_OverCooked_Warning);

    // Calculate and store off the charge fraction to use when the weapon actually shoots

    // Clamp to max_charge_time
    if (charge_time - min_charge_time[mode] >= 0.0f) {
        if (charge_time <= max_charge_time[mode]) {
            charge_fraction = charge_time / max_charge_time[mode];
        } else {
            charge_fraction = 1.0f;
        }
    } else {
        charge_fraction = 0.0f;
    }

    // Call regular fire function
    Fire(mode);
}

//======================
//Weapon::GetFireAnim
//======================
const char *Weapon::GetFireAnim() const
{
    if (m_iNumFireAnims > 1) {
        static char tagname[256];

        Com_sprintf(tagname, sizeof(tagname), "fire_%d", m_iCurrentFireAnim + 1);
        return tagname;
    } else {
        return "fire";
    }
}

//======================
//Weapon::Fire
//======================
void Weapon::Fire(firemode_t mode)
{
    Event *done_event = NULL;
    Vector pos;

    // Sanity check the mode
    assert((mode >= 0) && (mode < MAX_FIREMODES));

    // If we are in loopfire mode, then we don't pass a DoneFiring event
    if (!loopfire[mode]) {
        // The DoneFiring event requires to know the firing mode so save that off in the event
        done_event = new Event(EV_Weapon_DoneFiring);
        done_event->AddInteger(mode);
    }

    if (!MuzzleClear()) {
        SetWeaponAnim("clear", done_event);
        weaponstate = WEAPON_READY;
        return;
    }

    // Use up the appropriate amount of ammo, it's already been checked that we have enough
    UseAmmo(ammorequired[mode], mode);

    // Set the state of the weapon to FIRING
    weaponstate = WEAPON_FIRING;

    // Cancel any old done firing events
    CancelEventsOfType(EV_Weapon_DoneFiring);

    // Play the correct animation
    if (mode == FIRE_PRIMARY) {
        if (m_iNumFireAnims > 1) {
            m_iCurrentFireAnim++;

            if (m_iCurrentFireAnim >= m_iNumFireAnims) {
                m_iCurrentFireAnim = 0;
            }
        }
        if (ammo_clip_size[FIRE_PRIMARY] && !ammo_in_clip[FIRE_PRIMARY] && HasAnim("fire_empty")) {
            SetWeaponAnim("fire_empty", done_event);
        } else {
            SetWeaponAnim(GetFireAnim(), done_event);
        }
    } else if (mode == FIRE_SECONDARY) {
        if (((!m_bShareClip && ammo_clip_size[FIRE_SECONDARY] && !ammo_in_clip[FIRE_SECONDARY])
             || (m_bShareClip && ammo_clip_size[FIRE_PRIMARY] && !ammo_in_clip[FIRE_PRIMARY]))
            && HasAnim("secondaryfire_empty")) {
            SetWeaponAnim("secondaryfire_empty", done_event);
        } else {
            SetWeaponAnim("secondaryfire", done_event);
        }
    }
}

//======================
//Weapon::DetachGun
//======================
void Weapon::DetachGun(void)
{
    if (attached) {
        if (m_iZoom && owner && owner->IsSubclassOfPlayer()) {
            Player *p = (Player *)owner.Pointer();
            p->ZoomOff();
        }
        StopSound(CHAN_WEAPONIDLE);
        attached = false;
        detach();
        hideModel();
    }
}

//======================
//Weapon::AttachGun
//======================
void Weapon::AttachGun(weaponhand_t hand, qboolean holstering)
{
    int tag_num;

    if (!owner) {
        current_attachToTag = "";
        return;
    }

    if (attached) {
        DetachGun();
    }

    if (holstering) {
        // Save off these values if we are holstering the weapon.  We will restore them when
        // the users raises the weapons again.
        lastAngles = this->angles;
        lastScale  = this->edict->s.scale;
        lastValid  = qtrue;
    } else if (lastValid) {
        // Restore the last
        setScale(lastScale);
        setAngles(lastAngles);
        lastValid = qfalse;
    }

    switch (hand) {
    case WEAPON_MAIN:
        if (holstering) {
            current_attachToTag = holster_attachToTag;
            setAngles(holsterAngles);
            setScale(holsterScale);
        } else {
            current_attachToTag = attachToTag_main;
        }
        break;
    case WEAPON_OFFHAND:
        if (holstering) {
            current_attachToTag = holster_attachToTag;
            setAngles(holsterAngles);
            setScale(holsterScale);
        } else {
            current_attachToTag = attachToTag_offhand;
        }
        break;
    default:
        warning("Weapon::AttachGun", "Invalid hand for attachment of weapon specified");
        break;
    }

    if (!current_attachToTag.length()) {
        return;
    }

    if (owner->edict->tiki) {
        tag_num = gi.Tag_NumForName(owner->edict->tiki, this->current_attachToTag.c_str());

        NoLerpThisFrame();
        if (tag_num >= 0) {
            attached = true;
            attach(owner->entnum, tag_num);
            showModel();
            setOrigin();
        } else {
            warning(
                "Weapon::AttachGun",
                "Attachment of weapon to tag \"%s\": Tag Not Found\n",
                this->current_attachToTag.c_str()
            );
        }
    }

    if (m_bAutoZoom && owner->IsSubclassOfPlayer()) {
        Player *p = (Player *)owner.Pointer();
        p->ToggleZoom(m_iZoom);
    }
}

//======================
//Weapon::AttachToHand
//======================
void Weapon::AttachToHand(Event *ev)
{
    str tag;

    if (!owner || !attached) {
        return;
    }

    weaponhand_t hand = WeaponHandNameToNum(ev->GetString(1));

    if (hand == WEAPON_ERROR) {
        return;
    }

    if (hand == WEAPON_OFFHAND) {
        tag = attachToTag_offhand;
    } else {
        tag = attachToTag_main;
    }

    current_attachToTag = tag;

    int tagnum = gi.Tag_NumForName(owner->edict->tiki, tag);
    if (tagnum < 0) {
        warning(
            "Weapon::AttachToHand",
            "Attachment of weapon '%s' to tag \"%s\": Tag Not Found\n",
            getName().c_str(),
            tag.c_str()
        );
    } else {
        attach(owner->entnum, tagnum);
        setOrigin();
    }
}

//======================
//Weapon::SetCantPartialReload
//======================
void Weapon::SetCantPartialReload(Event *ev)
{
    if (g_protocol <= protocol_e::PROTOCOL_MOH && g_gametype->integer) {
        return;
    }

    m_bCanPartialReload = qfalse;
}

//======================
//Weapon::SetDMCantPartialReload
//======================
void Weapon::SetDMCantPartialReload(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }

    m_bCanPartialReload = qfalse;
}

//======================
//Weapon::AddAdditionalStartAmmo
//======================
void Weapon::AddAdditionalStartAmmo(Event *ev)
{
    m_additionalStartAmmoTypes.AddObject(ev->GetString(1));
    m_additionalStartAmmoAmounts.AddObject(ev->GetInteger(2));
}

//======================
//Weapon::AddStartItem
//======================
void Weapon::AddStartItem(Event *ev)
{
    m_startItems.AddObject(ev->GetString(1));
}

//======================
//Weapon::GiveStartingAmmoToOwner
//======================
void Weapon::GiveStartingAmmoToOwner(Event *ev)
{
    str ammotype;
    int mode;
    int i;

    assert(owner);

    if (!owner) {
        warning("Weapon::GiveStartingAmmoToOwner", "Owner not found\n");
        return;
    }

    // Give the player the starting ammo
    for (mode = FIRE_PRIMARY; mode < MAX_FIREMODES; mode++) {
        ammotype = GetAmmoType((firemode_t)(mode));
        if (ammotype.length()) {
            int start_ammo = this->GetStartAmmo((firemode_t)mode);

            if (ammo_clip_size[mode]) {
                int ammo = ammo_clip_size[mode] - ammo_in_clip[mode];

                if (ammo > 0) {
                    if (ammo < start_ammo) {
                        start_ammo -= ammo;
                        ammo_in_clip[mode] = ammo + ammo_in_clip[mode];
                    } else {
                        ammo_in_clip[mode] = start_ammo + ammo_in_clip[mode];
                        start_ammo         = 0;
                    }
                }
            }

            if (start_ammo) {
                owner->GiveAmmo(ammotype, start_ammo);
            }
        }
    }

    if (m_additionalStartAmmoTypes.NumObjects()) {
        for (i = 1; i <= m_additionalStartAmmoTypes.NumObjects(); i++) {
            const str& type            = m_additionalStartAmmoTypes.ObjectAt(i);
            int        startAmmoAmount = m_additionalStartAmmoAmounts.ObjectAt(i);

            if (type.length() && startAmmoAmount) {
                owner->GiveAmmo(type, startAmmoAmount);
            }
        }

        m_additionalStartAmmoTypes.ClearObjectList();
        m_additionalStartAmmoAmounts.ClearObjectList();
    }

    if (m_startItems.NumObjects()) {
        for (i = 1; i <= m_startItems.NumObjects(); i++) {
            const str& itemName = m_startItems.ObjectAt(i);

            if (itemName.length()) {
                owner->giveItem(itemName);
            }
        }

        m_startItems.ClearObjectList();
    }
}

//======================
//Weapon::IsSecondaryWeapon
//======================
qboolean Weapon::IsSecondaryWeapon(void)
{
    return (weapon_class & WEAPON_CLASS_SECONDARY);
}

//======================
//Weapon::PickupWeapon
//======================
void Weapon::PickupWeapon(Event *ev)
{
    Sentient *sen;
    Entity   *other;
    qboolean  hasweapon;
    qboolean  hasclass;
    int       iGiveAmmo;
    str       realname;

    other = ev->GetEntity(1);
    assert(other);

    if (!other->IsSubclassOfSentient()) {
        return;
    }

    sen = (Sentient *)other;

    // If this is the last owner, check to see if he can pick it up
    if ((sen == last_owner) && (level.time < last_owner_trigger_time)) {
        return;
    }

    hasweapon = sen->HasItem(item_name);
    hasclass  = sen->HasWeaponClass(weapon_class);

    if ((g_gametype->integer || g_realismmode->integer) && !hasclass && !IsSecondaryWeapon()
        && sen->HasPrimaryWeapon()) {
        // Make sure the sentient doesn't have a primary weapon on DM modes
        return;
    }

    if ((g_gametype->integer || g_realismmode->integer) && other->IsSubclassOfPlayer()
        && (weapon_class & WEAPON_CLASS_GRENADE) && !hasweapon && hasclass) {
        hasclass = false;
    }

    if (!hasweapon && !hasclass) {
        if (other->IsSubclassOfPlayer()) {
            gi.SendServerCommand(
                other->edict - g_entities,
                "print \"" HUD_MESSAGE_YELLOW "%s\n\"",
                gi.LV_ConvertString(va("Picked Up %s", item_name.c_str()))
            );

            if (!(spawnflags & DROPPED_PLAYER_ITEM) && !(spawnflags & DROPPED_ITEM)) {
                ItemPickup(other);
                return;
            }

            if (Pickupable(other)) {
                setMoveType(MOVETYPE_NONE);
                setSolidType(SOLID_NOT);

                hideModel();

                velocity  = vec_zero;
                avelocity = vec_zero;

                CancelEventsOfType(EV_Remove);
                CancelEventsOfType(EV_Weapon_FallingAngleAdjust);

                DetachFromOwner();
                current_attachToTag        = "";
                lastValid                  = qfalse;
                edict->s.tag_num           = -1;
                edict->s.attach_use_angles = qfalse;
                VectorClear(edict->s.attach_offset);

                setOrigin(vec_zero);
                setAngles(vec_zero);
                SetOwner(sen);

                sen->AddItem(this);
                sen->ReceivedItem(this);

                Sound(sPickupSound);
            }
        }
    } else {
        str sAmmoName = ammo_type[FIRE_PRIMARY];

        if (sen->AmmoCount(sAmmoName) != sen->MaxAmmoCount(sAmmoName)) {
            setSolidType(SOLID_NOT);
            hideModel();

            CancelEventsOfType(EV_Item_DropToFloor);
            CancelEventsOfType(EV_Item_Respawn);
            CancelEventsOfType(EV_FadeOut);
            CancelEventsOfType(EV_Remove);
            CancelEventsOfType(EV_Weapon_FallingAngleAdjust);

            if (Respawnable()) {
                PostEvent(EV_Item_Respawn, 0);
            } else {
                PostEvent(EV_Remove, 0);
            }

            Sound(m_sAmmoPickupSound);
        }
    }

    if (startammo[FIRE_PRIMARY] && ammo_type[FIRE_PRIMARY].length() && other->isClient()) {
        str        sMessage;
        const str& sAmmoType = ammo_type[FIRE_PRIMARY];

        iGiveAmmo = startammo[FIRE_PRIMARY];

        sen->GiveAmmo(sAmmoType, iGiveAmmo);

        if (!g_gametype->integer && other->IsSubclassOfPlayer()) {
            if (!sAmmoType.icmp("agrenade")) {
                if (iGiveAmmo == 1) {
                    sMessage = gi.LV_ConvertString("Got 1 Grenade");
                } else {
                    sMessage = gi.LV_ConvertString(va("Got %i Grenades", iGiveAmmo));
                }
            }
        }

        if (!sAmmoType.icmp("grenade")) {
            if (iGiveAmmo == 1) {
                sMessage = gi.LV_ConvertString("Got 1 Grenade");
            } else {
                sMessage = gi.LV_ConvertString(va("Got %i Grenades", iGiveAmmo));
            }
        } else {
            sMessage = gi.LV_ConvertString(va("Got %i %s Rounds", iGiveAmmo, sAmmoType.c_str()));
        }

        gi.SendServerCommand(other->edict - g_entities, "print \"" HUD_MESSAGE_YELLOW "%s\n\"", sMessage.c_str());
    }

    if (ammo_type[FIRE_SECONDARY] != ammo_type[FIRE_PRIMARY]) {
        if (startammo[FIRE_SECONDARY] && ammo_type[FIRE_SECONDARY].length() && other->isClient()) {
            str        sMessage;
            const str& sAmmoType = ammo_type[FIRE_PRIMARY];

            iGiveAmmo = startammo[FIRE_SECONDARY];

            sen->GiveAmmo(sAmmoType, iGiveAmmo);

            if (!g_gametype->integer && other->IsSubclassOfPlayer()) {
                if (!sAmmoType.icmp("agrenade")) {
                    if (iGiveAmmo == 1) {
                        sMessage = gi.LV_ConvertString("Got 1 Grenade");
                    } else {
                        sMessage = gi.LV_ConvertString(va("Got %i Grenades", iGiveAmmo));
                    }
                }
            }

            if (!sAmmoType.icmp("grenade")) {
                if (iGiveAmmo == 1) {
                    sMessage = gi.LV_ConvertString("Got 1 Grenade");
                } else {
                    sMessage = gi.LV_ConvertString(va("Got %i Grenades", iGiveAmmo));
                }
            } else if (!sAmmoType.icmp("riflegrenade")) {
                if (iGiveAmmo == 1) {
                    sMessage = gi.LV_ConvertString("Got 1 Rifle Grenade");
                } else {
                    sMessage = gi.LV_ConvertString(va("Got %i Rifle Grenades", iGiveAmmo));
                }
            } else {
                sMessage = gi.LV_ConvertString(va("Got %i %s Rounds", startammo[FIRE_PRIMARY], sAmmoType.c_str()));
            }

            gi.SendServerCommand(other->edict - g_entities, "print \"" HUD_MESSAGE_YELLOW "%s\n\"", sMessage.c_str());
        }
    }

    Unregister(STRING_PICKUP);
}

//======================
//Weapon::ForceIdle
//======================
void Weapon::ForceIdle(void)
{
    SetWeaponIdleAnim();

    // Force the weapon to the idle animation
    weaponstate = WEAPON_READY;
}

//======================
//Weapon::SetWeaponIdleAnim
//======================
void Weapon::SetWeaponIdleAnim(void)
{
    if (m_bShareClip) {
        if (ammo_clip_size[FIRE_PRIMARY] && !ammo_in_clip[FIRE_PRIMARY]) {
            if (SetWeaponAnim("idle_empty")) {
                return;
            }
        }

        SetWeaponAnim("idle");
    } else {
        if (ammo_clip_size[FIRE_PRIMARY] && !ammo_in_clip[FIRE_PRIMARY]) {
            if (SetWeaponAnim("idle_empty")) {
                return;
            }
        }
    }

    SetWeaponAnim("idle");
}

//======================
//Weapon::SetWeaponIdleAnimEvent
//======================
void Weapon::SetWeaponIdleAnimEvent(Event *ev)
{
    SetWeaponIdleAnim();
}

//======================
//Weapon::SetWeaponAnimEvent
//======================
void Weapon::SetWeaponAnimEvent(Event *ev)
{
    SetWeaponAnim(ev->GetString(1));
}

//======================
//Weapon::SetWeaponAnim
//======================
qboolean Weapon::SetWeaponAnim(const char *anim, Event *ev)
{
    int animnum = gi.Anim_NumForName(edict->tiki, anim);

    if (animnum == -1) {
        if (ev) {
            delete ev;
        }

        return qfalse;
    }

    StopAnimating(m_iAnimSlot);
    RestartAnimSlot(m_iAnimSlot);

    int idleanim = gi.Anim_NumForName(edict->tiki, "idle");

    edict->s.frameInfo[m_iAnimSlot].index = idleanim;
    m_iAnimSlot                           = (m_iAnimSlot + 1) & 3;
    edict->s.frameInfo[m_iAnimSlot].index = idleanim;

    if (ev) {
        NewAnim(animnum, ev, m_iAnimSlot);
    } else {
        NewAnim(animnum, m_iAnimSlot);
    }

    SetOnceType(m_iAnimSlot);
    RestartAnimSlot(m_iAnimSlot);

    return qtrue;
}

//======================
//Weapon::SetWeaponAnim
//======================
qboolean Weapon::SetWeaponAnim(const char *anim, Event& ev)
{
    Event *event = new Event(ev);

    return SetWeaponAnim(anim, event);
}

//======================
//Weapon::StopWeaponAnim
//======================
void Weapon::StopWeaponAnim(void)
{
    int animnum;

    RestartAnimSlot(m_iAnimSlot);
    StopAnimating(m_iAnimSlot);

    animnum = gi.Anim_NumForName(edict->tiki, "idle");
    StartAnimSlot(m_iAnimSlot, animnum, 1.f);

    m_iAnimSlot = (m_iAnimSlot + 1) & 3;
}

//======================
//Weapon::DoneRaising
//======================
void Weapon::DoneRaising(Event *ev)
{
    weaponstate = WEAPON_READY;
    ForceIdle();

    if (!owner) {
        PostEvent(EV_Remove, 0);
        return;
    }
}

//======================
//Weapon::ClientFireDone
//======================
void Weapon::ClientFireDone(void)
{
    // This is called when the client's firing animation is done
}

//======================
//Weapon::DoneFiring
//======================
void Weapon::DoneFiring(Event *ev)
{
    // This is called when the weapon's firing animation is done
    ForceIdle();

    // Check to see if the auto_putaway flag is set, and the weapon is out of ammo.  If so, then putaway the
    // weapon.
    if (!HasAmmo(FIRE_PRIMARY) && auto_putaway) {
        PutAway();
    }
}

//======================
//Weapon::FillAmmoClip
//======================
void Weapon::FillAmmoClip(Event *ev)
{
    int amount;
    int amount_used;

    if (!ammo_clip_size[0] || !owner) {
        return;
    }

    if (UnlimitedAmmo(FIRE_PRIMARY)) {
        // reload directly
        ammo_in_clip[FIRE_PRIMARY] = ammo_clip_size[0];
    } else {
        // Calc the amount the clip should get
        amount = ammo_clip_size[FIRE_PRIMARY] - ammo_in_clip[FIRE_PRIMARY];

        // use up the ammo from the player
        amount_used = owner->UseAmmo(ammo_type[FIRE_PRIMARY], amount);

        // Stick it in the clip
        ammo_in_clip[FIRE_PRIMARY] = amount_used + ammo_in_clip[FIRE_PRIMARY];
    }

    owner->AmmoAmountInClipChanged(ammo_type[FIRE_PRIMARY], ammo_in_clip[FIRE_PRIMARY]);

    SetShouldReload(qfalse);
}

//======================
//Weapon::EmptyAmmoClip
//======================
void Weapon::EmptyAmmoClip(Event *ev)
{
    if (!ammo_clip_size[FIRE_PRIMARY]) {
        return;
    }

    if (!owner) {
        return;
    }

    owner->GiveAmmo(ammo_type[FIRE_PRIMARY], ammo_in_clip[FIRE_PRIMARY]);
    ammo_in_clip[FIRE_PRIMARY] = 0;

    SetShouldReload(qtrue);

    owner->AmmoAmountInClipChanged(ammo_type[FIRE_PRIMARY], ammo_in_clip[FIRE_PRIMARY]);
}

//======================
//Weapon::AddToAmmoClip
//======================
void Weapon::AddToAmmoClip(Event *ev)
{
    int amount;
    int amount_used;

    if (!ammo_clip_size[FIRE_PRIMARY] || !owner) {
        return;
    }

    // Calc the amount the clip should get
    amount = ev->GetInteger(1);

    if (amount > ammo_clip_size[FIRE_PRIMARY] - ammo_in_clip[FIRE_PRIMARY]) {
        amount = ammo_clip_size[FIRE_PRIMARY] - ammo_in_clip[FIRE_PRIMARY];
    }

    if (UnlimitedAmmo(FIRE_PRIMARY)) {
        // Stick it in the clip
        ammo_in_clip[FIRE_PRIMARY] = amount + ammo_in_clip[FIRE_PRIMARY];
    } else {
        // use up the ammo from the player
        amount_used = owner->UseAmmo(ammo_type[FIRE_PRIMARY], amount);

        // Stick it in the clip
        ammo_in_clip[FIRE_PRIMARY] = amount_used + ammo_in_clip[FIRE_PRIMARY];
    }

    owner->AmmoAmountInClipChanged(ammo_type[FIRE_PRIMARY], ammo_in_clip[FIRE_PRIMARY]);

    SetShouldReload(qfalse);
}

//======================
//Weapon::SetIdleState
//======================
void Weapon::SetIdleState(int state)
{
    if (!owner) {
        return;
    }

    switch (state) {
    case 0:
    default:
        SetWeaponAnim("idle0");
        break;
    case 1:
        SetWeaponAnim("idle1");
        break;
    case 2:
        SetWeaponAnim("idle2");
        break;
    }
}

//======================
//Weapon::StartReloading
//======================
void Weapon::StartReloading(void)
{
    if (!ammo_clip_size[0] || !owner) {
        return;
    }

    if (SetWeaponAnim("reload", EV_Weapon_DoneReloading)) {
        weaponstate = WEAPON_RELOADING;
    } else {
        ProcessEvent(EV_Weapon_FillClip);
        ProcessEvent(EV_Weapon_DoneReloading);
    }

    m_fFireSpreadMult[FIRE_PRIMARY] = 0;
}

//======================
//Weapon::DoneReloading
//======================
void Weapon::DoneReloading(Event *ev)
{
    SetShouldReload(qfalse);
    weaponstate = WEAPON_READY;
}

//======================
//Weapon::CheckReload
//======================
qboolean Weapon::CheckReload(firemode_t mode)
{
    // Check to see if the weapon needs to be reloaded
    assert((mode >= 0) && (mode < MAX_FIREMODES));

    mode = m_bShareClip ? FIRE_PRIMARY : mode;

    if (putaway) {
        return false;
    }

    if (ammo_in_clip[mode] < ammo_clip_size[mode] && AmmoAvailable(mode)
        && (m_bCanPartialReload || ammo_in_clip[mode] <= 0)) {
        return true;
    }

    return false;
}

//======================
//Weapon::SetShouldReload
//======================
void Weapon::SetShouldReload(qboolean should_reload)
{
    m_bShouldReload = should_reload;
}

//======================
//Weapon::ShouldReload
//======================
qboolean Weapon::ShouldReload(void)
{
    if (m_bShouldReload) {
        return qtrue;
    } else {
        if (ammo_clip_size[FIRE_PRIMARY] && !ammo_in_clip[FIRE_PRIMARY] && AmmoAvailable(FIRE_PRIMARY)) {
            return qtrue;
        }
    }

    return qfalse;
}

//======================
//Weapon::Idle
//======================
void Weapon::Idle(Event *ev)
{
    ForceIdle();
}

//======================
//Weapon::IdleInit
//======================
void Weapon::IdleInit(Event *ev)
{
    for (int i = 7; i >= 0; i--) {
        SetWeaponAnim("idle");
    }

    weaponstate = WEAPON_READY;
}

//======================
//Weapon::GetMaxRange
//======================
float Weapon::GetMaxRange(void)
{
    return maxrange;
}

//======================
//Weapon::GetMinRange
//======================
float Weapon::GetMinRange(void)
{
    return minrange;
}

//======================
//Weapon::SetMaxRangeEvent
//======================
void Weapon::SetMaxRangeEvent(Event *ev)
{
    maxrange = ev->GetFloat(1);
}

//======================
//Weapon::SetMinRangeEvent
//======================
void Weapon::SetMinRangeEvent(Event *ev)
{
    minrange = ev->GetFloat(1);
}

//======================
//Weapon::NotDroppableEvent
//======================
void Weapon::NotDroppableEvent(Event *ev)
{
    notdroppable = true;
}

//======================
//Weapon::SetMaxRange
//======================
void Weapon::SetMaxRange(float val)
{
    maxrange = val;
}

//======================
//Weapon::SetMinRange
//======================
void Weapon::SetMinRange(float val)
{
    minrange = val;
}

//======================
//Weapon::WeaponSound
//======================
void Weapon::WeaponSound(Event *ev)
{
    // Broadcasting a sound can be time consuming.  Only do it once in a while on really fast guns.
    if (nextweaponsoundtime > level.time) {
        if (owner) {
            owner->BroadcastAIEvent(AI_EVENT_WEAPON_FIRE);
        } else {
            BroadcastAIEvent(AI_EVENT_WEAPON_FIRE);
        }
        return;
    }

    if (owner) {
        owner->ProcessEvent(*ev);
    } else {
        Item::BroadcastAIEvent(AI_EVENT_WEAPON_FIRE);
    }

    // give us some breathing room
    nextweaponsoundtime = level.time + 0.4;
}

//======================
//Weapon::Removable
//======================
qboolean Weapon::Removable(void)
{
    if (((int)(dmflags->integer) & DF_WEAPONS_STAY) && !(spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))) {
        return false;
    } else {
        return true;
    }
}

//======================
//Weapon::Pickupable
//======================
qboolean Weapon::Pickupable(Entity *other)
{
    Sentient *sen;

    if (!other->IsSubclassOfSentient()) {
        return false;
    } else if (!other->isClient()) {
        return false;
    }

    sen = (Sentient *)other;

    // If we have the weapon and weapons stay, then don't pick it up
    if (((int)(dmflags->integer) & DF_WEAPONS_STAY) && !(spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))) {
        Weapon *weapon;

        weapon = (Weapon *)sen->FindItem(getName());

        if (weapon) {
            return false;
        }
    }

    return true;
}

//======================
//Weapon::AutoChange
//======================
qboolean Weapon::AutoChange(void)
{
    return true;
}

//======================
//Weapon::ClipAmmo
//======================
int Weapon::ClipAmmo(firemode_t mode)
{
    assert((mode >= 0) && (mode < MAX_FIREMODES));

    mode = m_bShareClip ? FIRE_PRIMARY : mode;

    if (ammo_clip_size[mode]) {
        return ammo_in_clip[mode];
    } else {
        return -1;
    }
}

//======================
//Weapon::SetFireDelay
//======================
void Weapon::SetFireDelay(Event *ev)
{
    if (g_protocol <= protocol_e::PROTOCOL_MOH && g_gametype->integer) {
        return;
    }

    fire_delay[firemodeindex] = ev->GetFloat(1);
}

//======================
//Weapon::SetDMFireDelay
//======================
void Weapon::SetDMFireDelay(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }

    fire_delay[firemodeindex] = ev->GetFloat(1);
}

//======================
//Weapon::FireDelay
//======================
float Weapon::FireDelay(firemode_t mode)
{
    return fire_delay[mode];
}

//======================
//Weapon::IsDroppable
//======================
qboolean Weapon::IsDroppable(void)
{
    if (notdroppable) {
        return false;
    } else {
        return true;
    }
}

//======================
//Weapon::SetFireType
//======================
void Weapon::SetFireType(Event *ev)
{
    str ftype;

    ftype = ev->GetString(1);

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));

    if (!ftype.icmp("projectile")) {
        firetype[firemodeindex] = FT_PROJECTILE;
    } else if (!ftype.icmp("bullet")) {
        firetype[firemodeindex] = FT_BULLET;
    } else if (!ftype.icmp("fakebullet")) {
        firetype[firemodeindex] = FT_FAKEBULLET;
    } else if (!ftype.icmp("melee")) {
        firetype[firemodeindex] = FT_MELEE;
    } else if (!ftype.icmp("special_projectile")) {
        firetype[firemodeindex] = FT_SPECIAL_PROJECTILE;
    } else if (!ftype.icmp("clickitem")) {
        firetype[firemodeindex] = FT_CLICKITEM;
    } else if (!ftype.icmp("heavy")) {
        firetype[firemodeindex] = FT_HEAVY;
    } else if (!ftype.icmp("landmine")) {
        firetype[firemodeindex] = FT_LANDMINE;
    } else if (!ftype.icmp("defuse")) {
        firetype[firemodeindex] = FT_DEFUSE;
    } else if (!ftype.icmp("none")) {
        firetype[firemodeindex] = FT_NONE;
    } else {
        warning("Weapon::SetFireType", "unknown firetype: %s\n", ftype.c_str());
    }
}

//======================
//Weapon::GetFireType
//======================
firetype_t Weapon::GetFireType(firemode_t mode)
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
void Weapon::SetProjectile(Event *ev)
{
    if (g_protocol <= protocol_e::PROTOCOL_MOH && g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    projectileModel[firemodeindex] = ev->GetString(1);
    CacheResource(projectileModel[firemodeindex].c_str());
}

//======================
//Weapon::SetDMProjectile
//======================
void Weapon::SetDMProjectile(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    projectileModel[firemodeindex] = ev->GetString(1);
    CacheResource(projectileModel[firemodeindex].c_str());
}

//======================
//Weapon::SetBulletDamage
//======================
void Weapon::SetBulletDamage(Event *ev)
{
    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    bulletdamage[firemodeindex] = ev->GetFloat(1);
}

//======================
//Weapon::SetBulletLarge
//======================
void Weapon::SetBulletLarge(Event *ev)
{
    bulletlarge[firemodeindex] = ev->GetInteger(1);
}

//======================
//Weapon::SetBulletLarge
//======================
void Weapon::SetTracerSpeed(Event *ev)
{
    tracerspeed[firemodeindex] = ev->GetFloat(1);
}

//======================
//Weapon::SetDMBulletDamage
//======================
void Weapon::SetDMBulletDamage(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    bulletdamage[firemodeindex] = ev->GetFloat(1);
}

//======================
//Weapon::SetBulletKnockback
//======================
void Weapon::SetBulletKnockback(Event *ev)
{
    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    bulletknockback[firemodeindex] = ev->GetFloat(1);
}

//======================
//Weapon::SetDMBulletKnockback
//======================
void Weapon::SetDMBulletKnockback(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    bulletknockback[firemodeindex] = ev->GetFloat(1);
}

//======================
//Weapon::SetBulletThroughWood
//======================
void Weapon::SetBulletThroughWood(Event *ev)
{
    bulletthroughwood[firemodeindex] = ev->GetFloat(1);
}

//======================
//Weapon::SetBulletThroughMetal
//======================
void Weapon::SetBulletThroughMetal(Event *ev)
{
    bulletthroughmetal[firemodeindex] = ev->GetFloat(1);
}

//======================
//Weapon::SetBulletRange
//======================
void Weapon::SetBulletRange(Event *ev)
{
    if (g_protocol <= protocol_e::PROTOCOL_MOH && g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    bulletrange[firemodeindex] = ev->GetFloat(1);
}

//======================
//Weapon::SetDMBulletRange
//======================
void Weapon::SetDMBulletRange(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    bulletrange[firemodeindex] = ev->GetFloat(1);
}

//======================
//Weapon::SetRange
//======================
void Weapon::SetRange(Event *ev)
{
    SetBulletRange(ev);
}

//======================
//Weapon::SetBulletCount
//======================
void Weapon::SetBulletCount(Event *ev)
{
    if (g_protocol <= protocol_e::PROTOCOL_MOH && g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    bulletcount[firemodeindex] = ev->GetFloat(1);
}

//======================
//Weapon::SetDMBulletCount
//======================
void Weapon::SetDMBulletCount(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    bulletcount[firemodeindex] = ev->GetFloat(1);
}

//======================
//Weapon::SetBulletSpread
//======================
void Weapon::SetBulletSpread(Event *ev)
{
    if (g_protocol <= protocol_e::PROTOCOL_MOH && g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    bulletspread[firemodeindex].x = ev->GetFloat(1);
    bulletspread[firemodeindex].y = ev->GetFloat(2);

    if (ev->NumArgs() > 2) {
        bulletspreadmax[firemodeindex].x = ev->GetFloat(3);
        bulletspreadmax[firemodeindex].y = ev->GetFloat(4);
    }
}

//======================
//Weapon::SetDMBulletSpread
//======================
void Weapon::SetDMBulletSpread(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    bulletspread[firemodeindex].x = ev->GetFloat(1);
    bulletspread[firemodeindex].y = ev->GetFloat(2);

    if (ev->NumArgs() > 2) {
        bulletspreadmax[firemodeindex].x = ev->GetFloat(3);
        bulletspreadmax[firemodeindex].y = ev->GetFloat(4);
    }
}

//======================
//Weapon::SetZoomSpreadMult
//======================
void Weapon::SetZoomSpreadMult(Event *ev)
{
    if (g_protocol <= protocol_e::PROTOCOL_MOH && g_gametype->integer) {
        return;
    }

    m_fZoomSpreadMult = ev->GetFloat(1);
}

//======================
//Weapon::SetDMZoomSpreadMult
//======================
void Weapon::SetDMZoomSpreadMult(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    m_fZoomSpreadMult = ev->GetFloat(1);
}

//======================
//Weapon::SetZoomSpreadMult
//======================
void Weapon::SetFireSpreadMult(Event *ev)
{
    if (g_protocol <= protocol_e::PROTOCOL_MOH && g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    m_fFireSpreadMultAmount[firemodeindex]  = ev->GetFloat(1);
    m_fFireSpreadMultFalloff[firemodeindex] = ev->GetFloat(2);
    m_fFireSpreadMultCap[firemodeindex]     = ev->GetFloat(3);
    m_fFireSpreadMultTimeCap[firemodeindex] = ev->GetFloat(4);
}

//======================
//Weapon::SetDMZoomSpreadMult
//======================
void Weapon::SetDMFireSpreadMult(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    m_fFireSpreadMultAmount[firemodeindex]  = ev->GetFloat(1);
    m_fFireSpreadMultFalloff[firemodeindex] = ev->GetFloat(2);
    m_fFireSpreadMultCap[firemodeindex]     = ev->GetFloat(3);
    m_fFireSpreadMultTimeCap[firemodeindex] = ev->GetFloat(4);
}

//======================
//Weapon::SetTracerFrequency
//======================
void Weapon::SetTracerFrequency(Event *ev)
{
    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    tracerfrequency[firemodeindex] = ev->GetInteger(1);
}

//======================
//Weapon::Secondary
//======================
void Weapon::Secondary(Event *ev)
{
    int i;

    Event *altev = new Event(ev->GetToken(1));

    firemodeindex = FIRE_SECONDARY;

    for (i = 2; i <= ev->NumArgs(); i++) {
        altev->AddToken(ev->GetToken(i));
    }

    ProcessEvent(altev);
    firemodeindex = FIRE_PRIMARY;
}

//====================
//Weapon::AutoAim
//====================
void Weapon::AutoAim(Event *ev)
{
    autoaim = true;
}

//====================
//Weapon::Crosshair
//====================
void Weapon::Crosshair(Event *ev)
{
    crosshair = ev->GetBoolean(1);
}

//====================
//Weapon::DMCrosshair
//====================
void Weapon::DMCrosshair(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }

    crosshair = ev->GetBoolean(1);
}

//====================
//Weapon::MainAttachToTag
//====================
void Weapon::MainAttachToTag(Event *ev)
{
    attachToTag_main = ev->GetString(1);
}

//====================
//Weapon::OffHandAttachToTag
//====================
void Weapon::OffHandAttachToTag(Event *ev)
{
    attachToTag_offhand = ev->GetString(1);
}

//====================
//Weapon::HolsterAttachToTag
//====================
void Weapon::HolsterAttachToTag(Event *ev)
{
    holster_attachToTag = ev->GetString(1);
}

//====================
//Weapon::SetHolsterOffset
//====================
void Weapon::SetHolsterOffset(Event *ev)
{
    holsterOffset = ev->GetVector(1);
}

//====================
//Weapon::SetHolsterAngles
//====================
void Weapon::SetHolsterAngles(Event *ev)
{
    holsterAngles = ev->GetVector(1);
}

//====================
//Weapon::SetHolsterScale
//====================
void Weapon::SetHolsterScale(Event *ev)
{
    holsterScale = ev->GetFloat(1);
}

//====================
//Weapon::SetQuiet
//====================
void Weapon::SetQuiet(Event *ev)
{
    quiet[firemodeindex] = true;
}

//====================
//Weapon::SetLoopFire
//====================
void Weapon::SetLoopFire(Event *ev)
{
    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    loopfire[firemodeindex] = true;
}

//======================
//Weapon::SetMeansOfDeath
//======================
void Weapon::SetMeansOfDeath(Event *ev)
{
    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    meansofdeath[firemodeindex] = (meansOfDeath_t)MOD_string_to_int(ev->GetString(1));
}

//======================
//Weapon::GetMeansOfDeath
//======================
meansOfDeath_t Weapon::GetMeansOfDeath(firemode_t mode)
{
    assert((mode >= 0) && (mode < MAX_FIREMODES));

    if ((mode >= 0) && (mode < MAX_FIREMODES)) {
        return meansofdeath[mode];
    } else {
        warning("Weapon::GetMeansOfDeath", "Invalid mode %d\n", mode);
        return MOD_NONE;
    }
}

//======================
//Weapon::SetAimTarget
//======================
void Weapon::SetAimTarget(Entity *ent)
{
    aim_target = ent;
}

//======================
//Weapon::WorldHitSpawn
//======================
void Weapon::WorldHitSpawn(firemode_t mode, Vector origin, Vector angles, float life) {}

//======================
//Weapon::SetWorldHitSpawn
//======================
void Weapon::SetWorldHitSpawn(Event *ev)
{
    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));
    worldhitspawn[firemodeindex] = ev->GetString(1);
}

//======================
//Weapon::MakeNoise
//======================
void Weapon::MakeNoise(Event *ev)
{
    float    radius = 500;
    qboolean force  = false;

    if (ev->NumArgs() > 0) {
        radius = ev->GetFloat(1);
    }

    if (ev->NumArgs() > 1) {
        force = ev->GetBoolean(2);
    }

    if (attached && (next_noise_time <= level.time || force)) {
        BroadcastAIEvent(AI_EVENT_MISC, radius);
        next_noise_time = level.time + 1;
    }
}

//======================
//Weapon::ShowInfo
//======================
void Weapon::ShowInfo(float fDot, float fDist) {}

//======================
//Weapon::FallingAngleAdjust
//======================
void Weapon::FallingAngleAdjust(Event *ev)
{
    Vector  vTmp;
    Vector  vDir;
    trace_t trace;

    if (owner) {
        return;
    }

    if (groundentity && (groundentity->entity == world || groundentity->r.bmodel)) {
        Vector vEnd = origin - Vector(0, 0, 128);

        trace = G_Trace(
            origin, vec_zero, vec_zero, vEnd, last_owner, edict->clipmask, qfalse, "Weapon::FallingAngleAdjust"
        );

        if (trace.fraction < 1.0f) {
            Vector vAng = Vector(0, angles[1], 0);
            Vector vCross;

            /*vAng.AngleVectorsLeft( &vDir );

			CrossProduct( vAng, vDir, vCross );
			CrossProduct( vDir, vCross, vAng );
			vAng = vAng.toAngles();*/

            if (angles[2] <= 0.0f) {
                vAng[2] = anglemod(vAng[2] - 90.0f);
            } else {
                vAng[2] = anglemod(vAng[2] + 90.0f);
            }

            setAngles(vAng);
        }

        if (weapon_class >= 0) {
            Sound(G_WeaponClassNumToName(weapon_class) + "_drop");
        }

        return;
    }

    angles[0] = anglemod(angles[0]);
    //if( angles[ 0 ] > 180.0f )
    //	angles[ 0 ] -= 180.0f;

    if (angles[0] >= -90.0f && angles[0] <= 0.0f) {
        angles[0] -= level.frametime * 160.0f;
        if (angles[0] < -180.0f) {
            angles[0] = -180.0f;
        }
    } else if (angles[0] > 0.0f) {
        angles[0] -= level.frametime * 160.0f;
    } else {
        angles[0] += level.frametime * 160.0f;
    }

    angles[2] = anglemod(angles[2]);
    //if( angles[ 2 ] > 180.0f )
    //	angles[ 2 ] -= 180.0f;

    if (angles[2] > -90.0f && angles[2] < 0.0f) {
        angles[2] -= level.frametime * 160.0f;
    } else if (angles[2] > 90.0f) {
        angles[2] -= level.frametime * 160.0f;
    } else {
        angles[2] += level.frametime * 160.0f;
    }

    angles[0] = anglemod(angles[0]);
    angles[2] = anglemod(angles[2]);
    setAngles(angles);
    PostEvent(EV_Weapon_FallingAngleAdjust, level.frametime);
}

//======================
//Weapon::GetUseCrosshair
//======================
qboolean Weapon::GetUseCrosshair() const
{
    if (m_fMaxFireMovement >= 1.f) {
        return crosshair;
    }

    if (!owner || (owner->velocity.lengthXY() / sv_runspeed->value) <= (m_fMaxFireMovement * m_fMovementSpeed)) {
        return crosshair;
    }

    return qfalse;
}

//======================
//Weapon::SetAIRange
//======================
void Weapon::SetAIRange(Event *ev)
{
    str s = ev->GetString(1);

    if (!s.icmp("short")) {
        mAIRange = RANGE_SHORT;
    } else if (!s.icmp("medium")) {
        mAIRange = RANGE_MEDIUM;
    } else if (!s.icmp("long")) {
        mAIRange = RANGE_LONG;
    } else if (!s.icmp("sniper")) {
        mAIRange = RANGE_SNIPER;
    } else {
        warning("Weapon::SetAIRange", "unknown range: %s.  Should be short, medium, long, or sniper\n", s.c_str());
    }
}

//======================
//Weapon::SetViewKick
//======================
void Weapon::SetViewKick(Event *ev)
{
    float pitchmin, pitchmax;
    float yawmin, yawmax;

    assert((firemodeindex >= 0) && (firemodeindex < MAX_FIREMODES));

    pitchmin = ev->GetFloat(1);
    pitchmax = ev->GetFloat(2);

    if (pitchmin <= pitchmax) {
        viewkickmin[firemodeindex][0] = pitchmin;
        viewkickmax[firemodeindex][0] = pitchmax;
    } else {
        viewkickmin[firemodeindex][0] = pitchmax;
        viewkickmax[firemodeindex][0] = pitchmin;
    }

    if (ev->NumArgs() > 2) {
        yawmin = ev->GetFloat(3);
        yawmax = ev->GetFloat(4);

        if (pitchmin <= pitchmax) {
            viewkickmin[firemodeindex][1] = yawmin;
            viewkickmax[firemodeindex][1] = yawmax;
        } else {
            viewkickmin[firemodeindex][1] = yawmax;
            viewkickmax[firemodeindex][1] = yawmin;
        }
    }
}

//======================
//Weapon::SetMovementSpeed
//======================
void Weapon::SetMovementSpeed(Event *ev)
{
    if (g_protocol <= protocol_e::PROTOCOL_MOH && g_gametype->integer) {
        return;
    }

    m_fMovementSpeed = ev->GetFloat(1);
}

//======================
//Weapon::SetDMMovementSpeed
//======================
void Weapon::SetDMMovementSpeed(Event *ev)
{
    if (!g_gametype->integer) {
        return;
    }

    m_fMovementSpeed = ev->GetFloat(1);
}

//======================
//Weapon::SetMaxFireMovement
//======================
void Weapon::SetMaxFireMovement(Event *ev)
{
    m_fMaxFireMovement = ev->GetFloat(1);
    if (m_fMaxFireMovement > 1.0) {
        // Cap at 1.0
        m_fMaxFireMovement = 1.0;
    }
}

//======================
//Weapon::SetZoomMovement
//======================
void Weapon::SetZoomMovement(Event *ev)
{
    m_fZoomMovement = ev->GetFloat(1);
    if (m_fZoomMovement > 1.0) {
        // Cap at 1.0
        m_fZoomMovement = 1.0;
    }
}

//======================
//Weapon::EventAmmoPickupSound
//======================
void Weapon::EventAmmoPickupSound(Event *ev)
{
    m_sAmmoPickupSound = ev->GetString(1);
}

//======================
//Weapon::EventNoAmmoSound
//======================
void Weapon::EventNoAmmoSound(Event *ev)
{
    m_NoAmmoSound = ev->GetString(1);
}

//======================
//Weapon::EventMaxMovementSound
//======================
void Weapon::EventMaxMovementSound(Event *ev)
{
    m_sMaxMovementSound = ev->GetString(1);
}

//======================
//Weapon::SetNumFireAnims
//======================
void Weapon::SetNumFireAnims(Event *ev)
{
    m_iNumFireAnims = ev->GetInteger(1);
}

//======================
//Weapon::SetWeaponSubtype
//======================
void Weapon::SetWeaponSubtype(Event *ev)
{
    m_iWeaponSubtype = ev->GetInteger(1);
}

//======================
//Weapon::SetCookTime
//======================
void Weapon::SetCookTime(Event *ev)
{
    m_fCookTime = ev->GetFloat(1);
}

//======================
//Weapon::SetCurrentFireAnim
//======================
void Weapon::SetCurrentFireAnim(Event *ev)
{
    m_iCurrentFireAnim = ev->GetInteger(1);
}

//======================
//Weapon::SetSecondaryAmmoInHud
//======================
void Weapon::SetSecondaryAmmoInHud(Event *ev)
{
    m_bSecondaryAmmoInHud = qtrue;
}

//======================
//Weapon::SetRank
//======================
void Weapon::SetRank(int order, int rank)
{
    this->order = order;
    this->rank  = rank;
}

//======================
//Weapon::GetRank
//======================
int Weapon::GetRank(void)
{
    return rank;
}

//======================
//Weapon::GetOrder
//======================
int Weapon::GetOrder(void)
{
    return order;
}

//======================
//Weapon::GetWeaponClass
//======================
int Weapon::GetWeaponClass(void) const
{
    return weapon_class;
}

//======================
//Weapon::GetWeaponGroup
//======================
const_str Weapon::GetWeaponGroup(void) const
{
    return m_csWeaponGroup;
};

//======================
//Weapon::SetItemSlot
//======================
void Weapon::SetItemSlot(int slot)
{
    weapon_class &= ~WEAPON_CLASS_ITEM_SLOT_BITS;
    weapon_class |= slot & WEAPON_CLASS_ITEM_SLOT_BITS;
}

//======================
//Weapon::GetPutaway
//======================
qboolean Weapon::GetPutaway(void)
{
    return putaway;
}

//======================
//Weapon::SetPutAway
//======================
void Weapon::SetPutAway(qboolean p)
{
    putaway = p;
}

//======================
//Weapon::GetState
//======================
weaponstate_t Weapon::GetState(void)
{
    return weaponstate;
}

//======================
//Weapon::SpecialFireProjectile
//======================
void Weapon::SpecialFireProjectile(
    Vector pos, Vector forward, Vector right, Vector up, Entity *owner, str projectileModel, float charge_fraction
)
{
    // Does nothing
}

//======================
//Weapon::GetCurrentAttachToTag
//======================
str Weapon::GetCurrentAttachToTag(void)
{
    return current_attachToTag;
}

//======================
//Weapon::SetCurrentAttachToTag
//======================
void Weapon::SetCurrentAttachToTag(str s)
{
    current_attachToTag = s;
}

//======================
//Weapon::GetHolsterTag
//======================
str Weapon::GetHolsterTag(void)
{
    return holster_attachToTag;
}

//======================
//Weapon::GetUseNoAmmo
//======================
qboolean Weapon::GetUseNoAmmo(void)
{
    return use_no_ammo;
}

//====================
//Weapon::GetZoom
//====================
int Weapon::GetZoom(void)
{
    return m_iZoom;
}

//====================
//Weapon::GetAutoZoom
//====================
qboolean Weapon::GetAutoZoom(void)
{
    return m_bAutoZoom;
}

//======================
//Weapon::GetMovementSpeed
//======================
float Weapon::GetMovementSpeed() const
{
    return m_fMovementSpeed;
}

//======================
//Weapon::IsCarryableTurret
//======================
bool Weapon::IsCarryableTurret() const
{
    return false;
}

//======================
//Weapon::GetStartAmmo
//======================
int Weapon::GetStartAmmo(firemode_t mode)
{
    assert((mode >= 0) && (mode < MAX_FIREMODES));
    return startammo[mode];
}

//======================
//Weapon::IsSemiAuto
//======================
qboolean Weapon::IsSemiAuto(void)
{
    return m_bSemiAuto;
}

//======================
//Weapon::ForceState
//======================
void Weapon::ForceState(weaponstate_t state)
{
    weaponstate = state;
}

//======================
//Weapon::MuzzleClear
//======================
qboolean Weapon::MuzzleClear(void)
{
    return qtrue;
}

//======================
//Weapon::GetBulletRange
//======================
float Weapon::GetBulletRange(firemode_t mode)
{
    return bulletrange[mode];
}

//======================
//Weapon::GetScriptOwner
//======================
Listener *Weapon::GetScriptOwner(void)
{
    return owner;
}
