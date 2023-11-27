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

// sentient.cpp: Base class of entity that can carry other entities, and use weapons.
//

#include "g_local.h"
#include "g_phys.h"
#include "entity.h"
#include "sentient.h"
#include "weapon.h"
#include "weaputils.h"
#include "scriptmaster.h"
#include "scriptexception.h"
#include "ammo.h"
#include "armor.h"
#include "misc.h"
#include "inventoryitem.h"
#include "player.h"
#include "actor.h"
#include "decals.h"
#include "g_spawn.h"
#include "object.h"
#include "../qcommon/tiki.h"
#include "weapturret.h"

Event EV_Sentient_ReloadWeapon
(
    "reloadweapon",
    EV_DEFAULT,
    "s",
    "hand",
    "Reloads the weapon in the specified hand",
    EV_NORMAL
);
Event EV_Sentient_Attack
(
    "fire",
    EV_DEFAULT,
    "SS",
    "hand mode",
    "Fires the weapon in the specified hand.",
    EV_NORMAL
);
Event EV_Sentient_StopFire
(
    "stopfire",
    EV_DEFAULT,
    "s",
    "hand",
    "Stops the firing of the weapon in the specified hand.",
    EV_NORMAL
);
Event EV_Sentient_Charge
(
    "charge",
    EV_DEFAULT,
    "s",
    "hand",
    "Starts the charging of the weapon in the specified hand",
    EV_NORMAL
);
Event EV_Sentient_ReleaseAttack
(
    "releasefire",
    EV_DEFAULT,
    "f",
    "fireholdtime",
    "Releases the attack in the time specified.",
    EV_NORMAL
);
Event EV_Sentient_GiveWeapon
(
    "weapon",
    EV_DEFAULT,
    "s",
    "weapon_modelname",
    "Gives the sentient the weapon specified.",
    EV_NORMAL
);
Event EV_Sentient_SetWeaponIdleState
(
    "setweaponidlestate",
    EV_DEFAULT,
    "i",
    "state",
    "set the idle state of the given weapon.",
    EV_NORMAL
);
Event EV_Sentient_PingForMines
(
    "pingformines",
    EV_DEFAULT,
    NULL,
    NULL,
    "actively uncover mines nearby.",
    EV_NORMAL
);
Event EV_Sentient_ForceLandmineMeasure
(
    "forcelandminemeasure",
    EV_DEFAULT,
    NULL,
    NULL,
    "Force a remeasurement to all landmines",
    EV_NORMAL
);
Event EV_Sentient_Take
(
    "take",
    EV_DEFAULT,
    "s",
    "item_name",
    "Takes away the specified item from the sentient.",
    EV_NORMAL
);
Event EV_Sentient_TakeAll
(
    "takeall",
    EV_DEFAULT,
    NULL,
    NULL,
    "Clears out the sentient's entire inventory.",
    EV_NORMAL
);
Event EV_Sentient_GiveAmmo
(
    "ammo",
    EV_DEFAULT,
    "si",
    "type amount",
    "Gives the sentient some ammo.",
    EV_NORMAL
);
Event EV_Sentient_GiveArmor
(
    "armor",
    EV_DEFAULT,
    "si",
    "type amount",
    "Gives the sentient some armor.",
    EV_NORMAL
);
Event EV_Sentient_GiveItem
(
    "item",
    EV_DEFAULT,
    "si",
    "type amount",
    "Gives the sentient the specified amount of the specified item.",
    EV_NORMAL
);
Event EV_Sentient_GiveDynItem
(
    "givedynitem",
    EV_DEFAULT,
    "ss",
    "model bonename",
    "Pass the args to the item.",
    EV_NORMAL
);
Event EV_Sentient_GiveTargetname
(
    "give",
    EV_DEFAULT,
    "s",
    "name",
    "Gives the sentient the targeted item.",
    EV_NORMAL
);
Event EV_Sentient_UseItem
(
    "use",
    EV_CONSOLE,
    "si",
    "name weapon_hand",
    "Use the specified weapon or item in the hand choosen (optional).",
    EV_NORMAL
);
Event EV_Sentient_SetBloodModel
(
    "bloodmodel",
    EV_DEFAULT,
    "s",
    "bloodModel",
    "set the model to be used when showing blood",
    EV_NORMAL
);
Event EV_Sentient_TurnOffShadow
(
    "noshadow",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turns off the shadow for this sentient.",
    EV_NORMAL
);
Event EV_Sentient_TurnOnShadow
(
    "shadow",
    EV_DEFAULT,
    NULL,
    NULL,
    "Turns on the shadow for this sentient.",
    EV_NORMAL
);
Event EV_Sentient_JumpXY
(
    "jumpxy",
    EV_DEFAULT,
    "fff",
    "forwardmove sidemove speed",
    "Makes the sentient jump.",
    EV_NORMAL
);
Event EV_Sentient_MeleeAttackStart
(
    "meleeattackstart",
    EV_DEFAULT,
    NULL,
    NULL,
    "Is the start of the sentient's melee attack.",
    EV_NORMAL
);
Event EV_Sentient_MeleeAttackEnd
(
    "meleeattackend",
    EV_DEFAULT,
    NULL,
    NULL,
    "Is the end of the sentient's melee attack.",
    EV_NORMAL
);
Event EV_Sentient_BlockStart
(
    "blockstart",
    EV_DEFAULT,
    NULL,
    NULL,
    "Is the start of the sentient's block.",
    EV_NORMAL
);
Event EV_Sentient_BlockEnd
(
    "blockend",
    EV_DEFAULT,
    NULL,
    NULL,
    "Is the end of the sentient's block.",
    EV_NORMAL
);
Event EV_Sentient_StunStart
(
    "stunstart",
    EV_DEFAULT,
    NULL,
    NULL,
    "Is the start of the sentient's stun.",
    EV_NORMAL
);
Event EV_Sentient_StunEnd
(
    "stunend",
    EV_DEFAULT,
    NULL,
    NULL,
    "Is the end of the sentient's stun.",
    EV_NORMAL
);
Event EV_Sentient_SetMouthAngle
(
    "mouthangle",
    EV_DEFAULT,
    "f",
    "mouth_angle",
    "Sets the mouth angle of the sentient.",
    EV_NORMAL
);
Event EV_Sentient_SetMaxMouthAngle
(
    "maxmouthangle",
    EV_DEFAULT,
    "f",
    "max_mouth_angle",
    "Sets the max mouth angle.",
    EV_NORMAL
);
Event EV_Sentient_OnFire
(
    "onfire",
    EV_DEFAULT,
    NULL,
    NULL,
    "Called every frame when the sentient is on fire.",
    EV_NORMAL
);
Event EV_Sentient_StopOnFire
(
    "stoponfire",
    EV_DEFAULT,
    NULL,
    NULL,
    "Stops the sentient from being on fire.",
    EV_NORMAL
);
Event EV_Sentient_SpawnBloodyGibs
(
    "spawnbloodygibs",
    EV_DEFAULT,
    "IF",
    "number_of_gibs scale",
    "Spawns some bloody generic gibs.",
    EV_NORMAL
);
Event EV_Sentient_SetMaxGibs
(
    "maxgibs",
    EV_DEFAULT,
    "i",
    "max_number_of_gibs",
    "Sets the maximum amount of generic gibs this sentient will spawn when hit.",
    EV_NORMAL
);
Event EV_Sentient_CheckAnimations
(
    "checkanims",
    EV_DEFAULT,
    NULL,
    NULL,
    "Check the animations in the .tik file versus the statefile",
    EV_NORMAL
);
Event EV_Sentient_DeactivateWeapon
(
    "deactivateweapon",
    EV_DEFAULT,
    "s",
    "side",
    "Deactivate the weapon in the specified hand.",
    EV_NORMAL
);
Event EV_Sentient_ActivateNewWeapon
(
    "activatenewweapon",
    EV_DEFAULT,
    NULL,
    NULL,
    "Activate the new weapon specified by useWeapon. handsurf allows specifying which hand to use for the player",
    EV_NORMAL
);
Event EV_Sentient_PutawayWeapon
(
    "putawayweapon",
    EV_DEFAULT,
    "s",
    "whichHand",
    "Put away or deactivate the current weapon, whichHand can be left or right.",
    EV_NORMAL
);
Event EV_Sentient_Weapon
(
    "weaponcommand",
    EV_DEFAULT,
    "sSSSSSSS",
    "hand arg1 arg2 arg3 arg4 arg5 arg6 arg7",
    "Pass the args to the active weapon in the specified hand",
    EV_NORMAL
);
Event EV_Sentient_UseWeaponClass
(
    "useweaponclass",
    EV_CONSOLE,
    "sI",
    "name weapon_hand",
    "Use the weapon of the specified class in the hand choosen (optional).",
    EV_NORMAL
);
Event EV_Sentient_German
(
    "german",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes the sentient a German.",
    EV_NORMAL
);
Event EV_Sentient_American
(
    "american",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes the sentient an American.",
    EV_NORMAL
);
Event EV_Sentient_GetTeam
(
    "team",
    EV_DEFAULT,
    NULL,
    NULL,
    "returns 'german' or 'american'",
    EV_GETTER
);
Event EV_Sentient_SetDamageMult
(
    "damagemult",
    EV_DEFAULT,
    "if",
    "location multiplier",
    "Sets the damage multiplier for a particular body location",
    EV_NORMAL
);
Event EV_Sentient_UseLastWeapon
(
    "uselast",
    EV_DEFAULT,
    NULL,
    NULL,
    "Activates the last active weapon",
    EV_NORMAL
);
Event EV_Sentient_ToggleItemUse
(
    "toggleitem",
    EV_CONSOLE,
    NULL,
    NULL,
    "Toggles the use of the player's item (first item if he has multiple)",
    EV_NORMAL
);
Event EV_Sentient_GetThreatBias
(
    "threatbias",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the threat bias for this player / AI",
    EV_GETTER
);
Event EV_Sentient_SetThreatBias
(
    "threatbias",
    EV_DEFAULT,
    "i",
    "bias",
    "Sets the threat bias for this player / AI",
    EV_SETTER
);
Event EV_Sentient_SetThreatBias2
(
    "threatbias",
    EV_DEFAULT,
    "i",
    "bias",
    "Sets the threat bias for this player / AI",
    EV_NORMAL
);
Event EV_Sentient_SetupHelmet
(
    "sethelmet",
    EV_DEFAULT,
    "sffss",
    "tikifile popspeed dmgmult surfacename [optional_additional_surface_name]",
    "Gives the sentient a helmet and sets the needed info for it",
    EV_NORMAL
);
Event EV_Sentient_PopHelmet
(
    "pophelmet",
    EV_DEFAULT,
    NULL,
    NULL,
    "Pops a sentient's helmet off if he's got one",
    EV_NORMAL
);
Event EV_Sentient_DropItems
(
    "dropitems",
    EV_DEFAULT,
    NULL,
    NULL,
    "drops inventory items",
    EV_NORMAL
);
Event EV_Sentient_DontDropWeapons
(
    "dontdropweapons",
    EV_DEFAULT,
    "B",
    "dont_drop",
    "Make the sentient not drop weapons",
    EV_NORMAL
);
Event EV_Sentient_ForceDropWeapon
(
    "forcedropweapon",
    EV_DEFAULT,
    NULL,
    NULL,
    "Force the sentient to drop weapons no matter what level.nodropweapon is.",
    EV_NORMAL
);
Event EV_Sentient_ForceDropWeapon2
(
    "forcedropweapon",
    EV_DEFAULT,
    NULL,
    NULL,
    "Force the sentient to drop weapons no matter what level.nodropweapon is.",
    EV_SETTER
);
Event EV_Sentient_ForceDropHealth
(
    "forcedropweapon",
    EV_DEFAULT,
    NULL,
    NULL,
    "Force the sentient to drop health no matter what level.nodrophealth is.",
    EV_NORMAL
);
Event EV_Sentient_ForceDropHealth2
(
    "forcedropweapon",
    EV_DEFAULT,
    NULL,
    NULL,
    "Force the sentient to drop health no matter what level.nodrophealth is.",
    EV_SETTER
);
Event EV_Sentient_GetForceDropHealth
(
    "forcedrophealth",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get if the sentient is forced to drop health no matter what level.nodrophealth is.",
    EV_GETTER
);
Event EV_Sentient_GetForceDropWeapon
(
    "forcedropweapon",
    EV_DEFAULT,
    NULL,
    NULL,
    "Get if the sentient is forced to drop health no matter what level.nodrophealth is.",
    EV_GETTER
);

//
// Openmohaa additions
//
Event EV_Sentient_GetNewActiveWeap
(
    "getnewactiveweap",
    EV_DEFAULT,
    NULL,
    NULL,
    "gets new active weapon",
    EV_RETURN
);
Event EV_Sentient_GetActiveWeap
(
    "getactiveweap",
    EV_DEFAULT,
    "i",
    "weaponhand",
    "gets currently active weapon in a given hand",
    EV_RETURN
);
Event EV_Sentient_Client_Landing
(
    "_client_landing",
    EV_DEFAULT,
    "FI",
    "fVolume iEquipment",
    "Play a landing sound that is appropriate to the surface we are landing on\n"
);

CLASS_DECLARATION(Animate, Sentient, NULL) {
    {&EV_Sentient_ReloadWeapon,         &Sentient::ReloadWeapon                 },
    {&EV_Sentient_Attack,               &Sentient::FireWeapon                   },
    {&EV_Sentient_StopFire,             &Sentient::StopFireWeapon               },
    {&EV_Sentient_Charge,               &Sentient::ChargeWeapon                 },
    {&EV_Sentient_ReleaseAttack,        &Sentient::ReleaseFireWeapon            },
    {&EV_Sentient_GiveAmmo,             &Sentient::EventGiveAmmo                },
    {&EV_Sentient_GiveWeapon,           &Sentient::EventGiveItem                },
    {&EV_Sentient_GiveArmor,            &Sentient::EventGiveItem                },
    {&EV_Sentient_GiveItem,             &Sentient::EventGiveItem                },
    {&EV_Sentient_GiveDynItem,          &Sentient::EventGiveDynItem             },
    {&EV_Sentient_UseItem,              &Sentient::EventUseItem                 },
    {&EV_Sentient_Take,                 &Sentient::EventTake                    },
    {&EV_Sentient_TakeAll,              &Sentient::EventFreeInventory           },
    {&EV_Sentient_SetBloodModel,        &Sentient::SetBloodModel                },
    {&EV_Sentient_GiveTargetname,       &Sentient::EventGiveTargetname          },
    {&EV_Sentient_SetWeaponIdleState,   &Sentient::EventSetWeaponIdleState      },
    {&EV_Sentient_PingForMines,         &Sentient::EventPingForMines            },
    {&EV_Sentient_ForceLandmineMeasure, &Sentient::EventForceLandmineMeasure    },
    {&EV_Damage,                        &Sentient::ArmorDamage                  },
    {&EV_Sentient_TurnOffShadow,        &Sentient::TurnOffShadow                },
    {&EV_Sentient_TurnOnShadow,         &Sentient::TurnOnShadow                 },
    {&EV_Sentient_JumpXY,               &Sentient::JumpXY                       },
    {&EV_Sentient_MeleeAttackStart,     &Sentient::MeleeAttackStart             },
    {&EV_Sentient_MeleeAttackEnd,       &Sentient::MeleeAttackEnd               },
    {&EV_Sentient_BlockStart,           &Sentient::BlockStart                   },
    {&EV_Sentient_BlockEnd,             &Sentient::BlockEnd                     },
    {&EV_Sentient_StunStart,            &Sentient::StunStart                    },
    {&EV_Sentient_StunEnd,              &Sentient::StunEnd                      },
    {&EV_Sentient_SetMaxMouthAngle,     &Sentient::SetMaxMouthAngle             },
    {&EV_Sentient_OnFire,               &Sentient::OnFire                       },
    {&EV_Sentient_StopOnFire,           &Sentient::StopOnFire                   },
    {&EV_Sentient_SpawnBloodyGibs,      &Sentient::SpawnBloodyGibs              },
    {&EV_Sentient_SetMaxGibs,           &Sentient::SetMaxGibs                   },
    {&EV_Sentient_CheckAnimations,      &Sentient::CheckAnimations              },
    {&EV_Sentient_German,               &Sentient::EventGerman                  },
    {&EV_Sentient_American,             &Sentient::EventAmerican                },
    {&EV_Sentient_GetTeam,              &Sentient::EventGetTeam                 },
    {&EV_Sentient_SetDamageMult,        &Sentient::SetDamageMult                },
    {&EV_Sentient_SetupHelmet,          &Sentient::EventSetupHelmet             },
    {&EV_Sentient_PopHelmet,            &Sentient::EventPopHelmet               },
    {&EV_Sentient_GetThreatBias,        &Sentient::EventGetThreatBias           },
    {&EV_Sentient_SetThreatBias,        &Sentient::EventSetThreatBias           },
    {&EV_Sentient_SetThreatBias2,       &Sentient::EventSetThreatBias           },
    {&EV_Sentient_DeactivateWeapon,     &Sentient::EventDeactivateWeapon        },
    {&EV_Sentient_ActivateNewWeapon,    &Sentient::ActivateNewWeapon            },
    {&EV_Sentient_PutawayWeapon,        &Sentient::PutawayWeapon                },
    {&EV_Sentient_Weapon,               &Sentient::WeaponCommand                },
    {&EV_Sentient_UseWeaponClass,       &Sentient::EventUseWeaponClass          },
    {&EV_Sentient_UseLastWeapon,        &Sentient::EventActivateLastActiveWeapon},
    {&EV_Sentient_ToggleItemUse,        &Sentient::EventToggleItemUse           },
    {&EV_Sentient_DropItems,            &Sentient::EventDropItems               },
    {&EV_Sentient_DontDropWeapons,      &Sentient::EventDontDropWeapons         },
    {&EV_Sentient_ForceDropHealth,      &Sentient::EventForceDropHealth         },
    {&EV_Sentient_ForceDropHealth2,     &Sentient::EventForceDropHealth         },
    {&EV_Sentient_GetForceDropHealth,   &Sentient::EventGetForceDropHealth      },
    {&EV_Sentient_ForceDropWeapon,      &Sentient::EventForceDropWeapon         },
    {&EV_Sentient_ForceDropWeapon2,     &Sentient::EventForceDropWeapon         },
    {&EV_Sentient_GetForceDropWeapon,   &Sentient::EventGetForceDropWeapon      },

    {&EV_Sentient_GetActiveWeap,        &Sentient::GetActiveWeap                },
    {&EV_Sentient_GetNewActiveWeap,     &Sentient::GetNewActiveWeapon           },
    {&EV_Sentient_Client_Landing,       &Sentient::EventClientLanding           },
    {NULL,                              NULL                                    }
};

Container<Sentient *> SentientList;

void Sentient::EventGiveDynItem(Event *ev)
{
    str      tikiname;
    int      tagnum;
    Vector   offset;
    DynItem *item;

    item                = new DynItem();
    tikiname            = ev->GetString(1);
    item->m_attachPrime = ev->GetString(2);

    item->setModel(tikiname);
    tagnum = gi.Tag_NumForName(edict->tiki, item->m_attachPrime.c_str());
    if (tagnum >= 0 && !item->attach(entnum, tagnum, qtrue, offset)) {
        // invalid tagnum
        delete item;
        return;
    }

    item->setSolidType(SOLID_BBOX);
    item->setMoveType(MOVETYPE_BOUNCE);
    item->takedamage = DAMAGE_YES;
    item->ProcessPendingEvents();
}

Sentient::Sentient()
    : m_bIsAnimal(false)
{
    SentientList.AddObject((Sentient *)this);
    entflags |= ECF_SENTIENT;

    m_bOvercookDied = false;

    if (LoadingSavegame) {
        return;
    }

    viewheight              = 0;
    means_of_death          = MOD_NONE;
    LMRF                    = 0;
    in_melee_attack         = false;
    in_block                = false;
    in_stun                 = false;
    on_fire                 = 0;
    on_fire_stop_time       = 0;
    next_catch_on_fire_time = 0;
    on_fire_tagnums[0]      = -1;
    on_fire_tagnums[1]      = -1;
    on_fire_tagnums[2]      = -1;
    attack_blocked_time     = 0;
    m_fHelmetSpeed          = 0;

    inventory.ClearObjectList();

    m_pNextSquadMate = this;
    m_pPrevSquadMate = this;

    m_Enemy.Clear();

    m_fPlayerSightLevel = 0;
    newWeapon           = NULL;

    eyeposition       = Vector(0, 0, 64);
    charge_start_time = 0;
    poweruptype       = 0;
    poweruptimer      = 0;
    // do better lighting on all sentients
    edict->s.renderfx |= RF_EXTRALIGHT;
    edict->s.renderfx |= RF_SHADOW;
    // sentients have precise shadows
    edict->s.renderfx |= RF_SHADOW_PRECISE;

    m_vViewVariation = Vector(0, 0, 0);
    for (int i = 0; i < MAX_ACTIVE_WEAPONS; i++) {
        activeWeaponList[i] = NULL;
    }

    in_melee_attack = false;
    in_block        = false;
    in_stun         = false;
    attack_blocked  = qfalse;
    max_mouth_angle = 10;

    // touch triggers by default
    flags |= FL_TOUCH_TRIGGERS;

    on_fire         = false;
    max_gibs        = 0;
    next_bleed_time = 0;

    ClearNewActiveWeapon();
    newActiveWeapon.weapon    = NULL;
    holsteredWeapon           = NULL;
    weapons_holstered_by_code = false;
    lastActiveWeapon.weapon   = NULL;
    edict->s.eFlags |= EF_UNARMED;

    m_pVehicle.Clear();
    m_pTurret.Clear();
    m_pLadder.Clear();
    m_iAttackerCount = 0;
    m_pLastAttacker.Clear();

    m_bIsDisguised        = false;
    m_bHasDisguise        = false;
    m_ShowPapersTime      = 0;
    m_iLastHitTime        = 0;
    m_Team                = TEAM_AMERICAN;
    m_iThreatBias         = 0;
    m_bFootOnGround_Right = true;
    m_bFootOnGround_Left  = true;
    iNextLandTime         = 0;
    m_bDontDropWeapons    = false;

    if (g_realismmode->integer) {
        m_fDamageMultipliers[HITLOC_HEAD]        = 5.0f;
        m_fDamageMultipliers[HITLOC_HELMET]      = 5.0f;
        m_fDamageMultipliers[HITLOC_NECK]        = 5.0f;
        m_fDamageMultipliers[HITLOC_TORSO_UPPER] = 1.0f;
        m_fDamageMultipliers[HITLOC_TORSO_MID]   = 0.95f;
        m_fDamageMultipliers[HITLOC_TORSO_LOWER] = 0.9f;
        m_fDamageMultipliers[HITLOC_PELVIS]      = 0.85f;
    } else {
        m_fDamageMultipliers[HITLOC_HEAD]        = 4.0f;
        m_fDamageMultipliers[HITLOC_HELMET]      = 4.0f;
        m_fDamageMultipliers[HITLOC_NECK]        = 4.0f;
        m_fDamageMultipliers[HITLOC_TORSO_UPPER] = 1.0f;
        m_fDamageMultipliers[HITLOC_TORSO_MID]   = 1.0f;
        m_fDamageMultipliers[HITLOC_TORSO_LOWER] = 1.0f;
        m_fDamageMultipliers[HITLOC_PELVIS]      = 0.9f;
    }

    m_fDamageMultipliers[HITLOC_R_ARM_UPPER] = 0.8f;
    m_fDamageMultipliers[HITLOC_L_ARM_UPPER] = 0.8f;
    m_fDamageMultipliers[HITLOC_R_LEG_UPPER] = 0.8f;
    m_fDamageMultipliers[HITLOC_L_LEG_UPPER] = 0.8f;
    m_fDamageMultipliers[HITLOC_R_ARM_LOWER] = 0.6f;
    m_fDamageMultipliers[HITLOC_L_ARM_LOWER] = 0.6f;
    m_fDamageMultipliers[HITLOC_R_LEG_LOWER] = 0.6f;
    m_fDamageMultipliers[HITLOC_L_LEG_LOWER] = 0.6f;
    m_fDamageMultipliers[HITLOC_R_HAND]      = 0.5f;
    m_fDamageMultipliers[HITLOC_L_HAND]      = 0.5f;
    m_fDamageMultipliers[HITLOC_R_FOOT]      = 0.5f;
    m_fDamageMultipliers[HITLOC_L_FOOT]      = 0.5f;

    m_PrevSentient = m_NextSentient = NULL;
    m_bForceDropHealth              = false;
    m_bForceDropWeapon              = false;

    Link();
}

Sentient::~Sentient()
{
    Unlink();
    DisbandSquadMate(this);

    SentientList.RemoveObject((Sentient *)this);
    FreeInventory();

    entflags &= ~ECF_SENTIENT;
}

void Sentient::Link()
{
    m_PrevSentient = NULL;
    m_NextSentient = level.m_HeadSentient[m_Team];
    if (m_NextSentient) {
        m_NextSentient->m_PrevSentient = this;
    }
    level.m_HeadSentient[m_Team] = this;
}

void Sentient::Unlink()
{
    if (m_NextSentient) {
        m_NextSentient->m_PrevSentient = m_PrevSentient;
    }
    if (m_PrevSentient) {
        m_PrevSentient->m_NextSentient = m_NextSentient;
    } else {
        level.m_HeadSentient[this->m_Team] = m_NextSentient;
    }

    m_NextSentient = m_PrevSentient = NULL;
}

Vector Sentient::EyePosition(void)

{
    return origin + eyeposition;
}

void Sentient::SetBloodModel(Event *ev)

{
    str name;
    str cache_name;
    str models_dir = "models/";

    if (ev->NumArgs() < 1) {
        return;
    }

    blood_model = ev->GetString(1);
    cache_name  = models_dir + blood_model;
    CacheResource(cache_name.c_str());

    name = GetBloodSpurtName();
    if (name.length()) {
        cache_name = models_dir + name;
        CacheResource(cache_name.c_str());
    }

    name = GetBloodSplatName();
    if (name.length()) {
        CacheResource(name.c_str());
    }

    name = GetGibName();
    if (name.length()) {
        cache_name = models_dir + name;
        CacheResource(cache_name.c_str());
    }
}

void Sentient::AddItem(Item *object)
{
    inventory.AddObject(object->entnum);
}

void Sentient::RemoveItem(Item *object)
{
    if (!inventory.IndexOfObject(object->entnum)) {
        return;
    }

    inventory.RemoveObject(object->entnum);

    if (object->IsSubclassOfWeapon()) {
        DeactivateWeapon((Weapon *)object);
    }

    //
    // let the sent know about it
    //
    RemovedItem(object);
}

void Sentient::RemoveWeapons(void)
{
    for (int i = inventory.NumObjects(); i > 0; i--) {
        int     entnum = inventory.ObjectAt(i);
        Weapon *item   = (Weapon *)G_GetEntity(entnum);

        if (item->IsSubclassOfWeapon()) {
            item->Delete();
        }
    }
}

Weapon *Sentient::GetWeapon(int index)
{
    for (int i = inventory.NumObjects(); i > 0; i--) {
        int     entnum = inventory.ObjectAt(i);
        Weapon *item   = (Weapon *)G_GetEntity(entnum);

        if (item->IsSubclassOfWeapon()) {
            if (!index) {
                return item;
            }

            index--;
        }
    }

    return NULL;
}

Item *Sentient::FindItemByExternalName(const char *itemname)
{
    int   num;
    int   i;
    Item *item;

    num = inventory.NumObjects();
    for (i = 1; i <= num; i++) {
        item = (Item *)G_GetEntity(inventory.ObjectAt(i));
        assert(item);
        if (!Q_stricmp(item->getName(), itemname)) {
            return item;
        }
    }

    return NULL;
}

Item *Sentient::FindItemByModelname(const char *mdl)
{
    int   num;
    int   i;
    Item *item;
    str   tmpmdl;

    if (Q_stricmpn("models/", mdl, 7)) {
        tmpmdl = "models/";
    }
    tmpmdl += mdl;

    num = inventory.NumObjects();
    for (i = 1; i <= num; i++) {
        item = (Item *)G_GetEntity(inventory.ObjectAt(i));
        assert(item);
        if (!Q_stricmp(item->model, tmpmdl)) {
            return item;
        }
    }

    return NULL;
}

Item *Sentient::FindItemByClassName(const char *classname)
{
    int   num;
    int   i;
    Item *item;

    num = inventory.NumObjects();
    for (i = 1; i <= num; i++) {
        item = (Item *)G_GetEntity(inventory.ObjectAt(i));
        assert(item);
        if (!Q_stricmp(item->edict->entname, classname)) {
            return item;
        }
    }

    return NULL;
}

Item *Sentient::FindItem(const char *itemname)
{
    Item *item;

    item = FindItemByExternalName(itemname);
    if (!item) {
        item = FindItemByModelname(itemname);
        if (!item) {
            item = FindItemByClassName(itemname);
        }
    }
    return item;
}

void Sentient::FreeInventory(void)
{
    int   num;
    int   i;
    Item *item;
    Ammo *ammo;

    // Detach all Weapons
    DetachAllActiveWeapons();

    // Delete all inventory items ( this includes weapons )
    num = inventory.NumObjects();
    for (i = num; i > 0; i--) {
        item = (Item *)G_GetEntity(inventory.ObjectAt(i));
        delete item;
    }
    inventory.ClearObjectList();

    // Remove all ammo
    num = ammo_inventory.NumObjects();
    for (i = num; i > 0; i--) {
        ammo = (Ammo *)ammo_inventory.ObjectAt(i);
        delete ammo;
    }
    ammo_inventory.ClearObjectList();

    if (IsSubclassOfPlayer()) {
        ((Player *)this)->InitMaxAmmo();
    }
}

void Sentient::EventFreeInventory(Event *ev)
{
    FreeInventory();
}

qboolean Sentient::HasItem(const char *itemname)
{
    return (FindItem(itemname) != NULL);
}

qboolean Sentient::HasWeaponClass(int iWeaponClass)
{
    int     i;
    Weapon *weapon;

    // look up for a weapon class
    for (i = 1; i <= inventory.NumObjects(); i++) {
        weapon = (Weapon *)G_GetEntity(inventory.ObjectAt(i));

        if (weapon->IsSubclassOfWeapon()) {
            if (weapon->GetWeaponClass() & iWeaponClass) {
                // weapon class found
                return qtrue;
            }
        }
    }

    return qfalse;
}

qboolean Sentient::HasPrimaryWeapon(void)
{
    int     i;
    Weapon *weapon;

    // look up for a primary weapon
    for (i = 1; i <= inventory.NumObjects(); i++) {
        weapon = (Weapon *)G_GetEntity(inventory.ObjectAt(i));

        if (weapon->IsSubclassOfWeapon()) {
            if (!(weapon->GetWeaponClass() & WEAPON_CLASS_MISC) && !weapon->IsSecondaryWeapon()) {
                // Sentient has a primary weapon
                return qtrue;
            }
        }
    }

    return qfalse;
}

qboolean Sentient::HasSecondaryWeapon(void)
{
    int     i;
    Weapon *weapon;

    // look up for a secondary weapon
    for (i = 1; i <= inventory.NumObjects(); i++) {
        weapon = (Weapon *)G_GetEntity(inventory.ObjectAt(i));

        if (weapon->IsSubclassOfWeapon()) {
            if (weapon->IsSecondaryWeapon()) {
                // Sentient has a secondary weapon
                return qtrue;
            }
        }
    }

    return qfalse;
}

void Sentient::EventGiveTargetname(Event *ev)
{
    int         i;
    ConSimple  *tlist;
    str         name;
    const char *ptr;
    qboolean    found;

    name = ev->GetString(1);

    ptr = name.c_str();

    // skip over the $
    ptr++;

    found = qfalse;

    str sName = ptr;
    tlist     = world->GetTargetList(sName);
    for (i = 1; i <= tlist->NumObjects(); i++) {
        Entity *ent;

        ent = (Entity *)tlist->ObjectAt(i).Pointer();
        assert(ent);

        if (ent->isSubclassOf(Item)) {
            Item *item;

            item = (Item *)ent;
            item->SetOwner(this);
            item->ProcessPendingEvents();
            AddItem(item);
            found = qtrue;
        }
    }

    if (!found) {
        ScriptError("Could not give item with targetname %s to this sentient.\n", name.c_str());
    }
}

Item *Sentient::giveItem(str itemname, int amount)
{
    ClassDef *cls;
    Item     *item;

    item = FindItem(itemname);
    if (item) {
        item->Add(amount);
        return item;
    } else {
        qboolean set_the_model = qfalse;

        // we don't have it, so lets try to resolve the item name
        // first lets see if it is a registered class name
        cls = getClass(itemname);
        if (!cls) {
            SpawnArgs args;

            // if that didn't work lets try to resolve it as a model
            args.setArg("model", itemname);

            cls = args.getClassDef();
            if (!cls) {
                gi.DPrintf("No item called '%s'\n", itemname.c_str());
                return NULL;
            }
            set_the_model = qtrue;
        }
        assert(cls);
        item = (Item *)cls->newInstance();

        if (!item) {
            gi.DPrintf("Could not spawn an item called '%s'\n", itemname.c_str());
            return NULL;
        }

        if (!item->isSubclassOf(Item)) {
            gi.DPrintf("Could not spawn an item called '%s'\n", itemname.c_str());
            delete item;
            return NULL;
        }

        if (set_the_model) {
            // Set the model
            item->setModel(itemname);
        }

        item->SetOwner(this);
        item->ProcessPendingEvents();
        item->setAmount(amount);

        AddItem(item);

        if (item->isSubclassOf(Weapon)) {
            // Post an event to give the ammo to the sentient
            Event *ev1;

            ev1 = new Event(EV_Weapon_GiveStartingAmmo);
            ev1->AddEntity(this);
            item->PostEvent(ev1, 0);
        }

        return item;
    }
    return NULL;
}

void Sentient::takeItem(const char *name)
{
    Item *item;

    item = FindItem(name);
    if (item) {
        gi.DPrintf("Taking item %s away from player\n", item->getName().c_str());

        item->PostEvent(EV_Remove, 0);
        return;
    }

    Ammo *ammo;
    ammo = FindAmmoByName(name);
    if (ammo) {
        gi.DPrintf("Taking ammo %s away from player\n", name);

        ammo->setAmount(0);
    }
}

void Sentient::takeAmmoType(const char *name)
{
    Ammo *ammo;

    ammo = FindAmmoByName(name);
    if (ammo) {
        gi.DPrintf("Taking ammo %s away from player\n", name);

        ammo->setAmount(0);
    }
}

void Sentient::EventUseItem(Event *ev)
{
    str          name;
    weaponhand_t hand = WEAPON_MAIN;

    if (deadflag) {
        return;
    }

    name = ev->GetString(1);

    if (ev->NumArgs() > 1) {
        hand = WeaponHandNameToNum(ev->GetString(2));
    }

    useWeapon(name, hand);
}

void Sentient::EventTake(Event *ev)
{
    takeItem(ev->GetString(1));
}

void Sentient::EventGiveItem(Event *ev)
{
    str   type;
    float amount;

    type = ev->GetString(1);
    if (ev->NumArgs() > 1) {
        amount = ev->GetInteger(2);
    } else {
        amount = 1;
    }

    giveItem(type, amount);
}

qboolean Sentient::DoGib(int meansofdeath, Entity *inflictor)

{
    if (!com_blood->integer) {
        return false;
    }

    if ((meansofdeath == MOD_TELEFRAG) || (meansofdeath == MOD_LAVA)) {
        return true;
    }

    if (health > -75) {
        return false;
    }

    // Impact and Crush < -75 health
    if ((meansofdeath == MOD_IMPACT) || (meansofdeath == MOD_CRUSH)) {
        return true;
    }

    return false;
}

void Sentient::SpawnEffect(str modelname, Vector pos)

{
    Animate *block;

    block = new Animate;
    block->setModel(modelname);
    block->setOrigin(pos);
    block->setSolidType(SOLID_NOT);
    block->setMoveType(MOVETYPE_NONE);
    block->NewAnim("idle");
    block->PostEvent(EV_Remove, 1);
}

int Sentient::CheckHitLocation(int iLocation)
{
    if (iLocation == 1) {
        if (WearingHelmet()) {
            return iLocation;
        } else {
            return HITLOC_HEAD;
        }
    }

    return iLocation;
}

#define WATER_CONVERSION_FACTOR 1.0f

void Sentient::ArmorDamage(Event *ev)
{
    Entity   *inflictor;
    Sentient *attacker;
    float     damage;
    Vector    momentum;
    Vector    position;
    Vector    normal;
    Vector    direction;
    Event    *event;
    int       dflags;
    int       meansofdeath;
    int       knockback;
    int       location;

    /*
    qboolean	blocked;
    float		damage_red;
    float		damage_green;
    float		damage_time;
    qboolean	set_means_of_death;
*/

    static bool    tmp          = false;
    static cvar_t *AIDamageMult = NULL;

    if (!tmp) {
        tmp          = true;
        AIDamageMult = gi.Cvar_Get("g_aiDamageMult", "1.0", 0);
    }

    if (IsDead()) {
        return;
    }

    attacker     = (Sentient *)ev->GetEntity(1);
    damage       = ev->GetFloat(2);
    inflictor    = ev->GetEntity(3);
    position     = ev->GetVector(4);
    direction    = ev->GetVector(5);
    normal       = ev->GetVector(6);
    knockback    = ev->GetInteger(7);
    dflags       = ev->GetInteger(8);
    meansofdeath = ev->GetInteger(9);
    location     = CheckHitLocation(ev->GetInteger(10));

    if (location == -2) {
        return;
    }

    if ((takedamage == DAMAGE_NO) || (movetype == MOVETYPE_NOCLIP)) {
        return;
    }

    if ((client && !g_gametype->integer) || (location < 0 || location > 18)) {
        if (attacker && attacker->IsSubclassOfActor()) {
            damage *= AIDamageMult->value;
        }
    } else {
        damage *= m_fDamageMultipliers[location];
    }

    // See if sentient is immune to this type of damage
    if (Immune(meansofdeath)) {
        /*
        means_of_death = meansofdeath;

        // Send pain event
        event = new Event( EV_Pain );
        event->AddEntity( attacker );
        event->AddFloat( 0 );
        event->AddVector( position );
        event->AddVector( direction );
        event->AddVector( normal );
        event->AddInteger( knockback );
        event->AddInteger( dflags );
        event->AddInteger( meansofdeath );
        event->AddInteger( location );

        ProcessEvent( event );
*/
        return;
    }

    // See if the damage is melee and high enough on actor

    /*
    if( deadflag )
    {
        // Spawn a blood spurt if this model has one
        if( ShouldBleed( meansofdeath, true ) )
        {
            AddBloodSpurt( direction );

            if( ShouldGib( meansofdeath, damage ) )
                ProcessEvent( EV_Sentient_SpawnBloodyGibs );
        }

        means_of_death = meansofdeath;

        if( meansofdeath == MOD_FIRE )
            TryLightOnFire( meansofdeath, attacker );

        // Send pain event
        event = new Event( EV_Pain );
        event->AddEntity( attacker );
        event->AddFloat( damage );
        event->AddVector( position );
        event->AddVector( direction );
        event->AddVector( normal );
        event->AddInteger( knockback );
        event->AddInteger( dflags );
        event->AddInteger( meansofdeath );
        event->AddInteger( location );

        ProcessEvent( event );

        return;
    }
*/

    /*
if( meansofdeath == MOD_SLIME )
{
    damage_green = damage / 50;
    if( damage_green > 1.0f )
        damage_green = 1.0f;
    if( ( damage_green < 0.2 ) && ( damage_green > 0 ) )
        damage_green = 0.2f;
    damage_red = 0;
}
else
{
    damage_red = damage / 50;
    if( damage_red > 1.0f )
        damage_red = 1.0f;
    if( ( damage_red < 0.2 ) && ( damage_red > 0 ) )
        damage_red = 0.2f;
    damage_green = 0;
}

damage_time = damage / 50;

if( damage_time > 2 )
    damage_time = 2;

SetOffsetColor( damage_red, damage_green, 0, damage_time );
*/

    // Do the kick
    if (!(dflags & DAMAGE_NO_KNOCKBACK)) {
        if ((knockback) && (movetype != MOVETYPE_NONE) && (movetype != MOVETYPE_STATIONARY)
            && (movetype != MOVETYPE_BOUNCE) && (movetype != MOVETYPE_PUSH) && (movetype != MOVETYPE_STOP)) {
            float  m;
            Event *immunity_event;

            if (mass < 50) {
                m = 50;
            } else {
                m = mass;
            }

            direction.normalize();
            if (isClient() && (attacker == this) && g_gametype->integer) {
                momentum = direction * (1700.0f * (float)knockback / m); // the rocket jump hack...
            } else {
                momentum = direction * (500.0f * (float)knockback / m);
            }

            if (dflags & DAMAGE_BULLET) {
                // Clip the z velocity for bullet weapons
                if (momentum.z > 75) {
                    momentum.z = 75;
                }
            }
            velocity += momentum;

            // Make this sentient vulnerable to falling damage now

            if (Immune(MOD_FALLING)) {
                immunity_event = new Event(EV_Entity_RemoveImmunity);
                immunity_event->AddString("falling");
                ProcessEvent(immunity_event);
            }
        }
    }

    if (g_debugdamage->integer) {
        G_DebugDamage(damage, this, attacker, inflictor);
    }

    if (!(flags & FL_GODMODE)
        && ((g_gametype->integer) || !(attacker) || (attacker) == this || !(attacker->IsSubclassOfPlayer())
            || !(attacker->IsTeamMate(this)))) {
        health -= damage;
    }

    // Set means of death
    means_of_death = meansofdeath;

    /*
    // Spawn a blood spurt if this model has one
    if( ShouldBleed( meansofdeath, false ) && !blocked )
    {
        AddBloodSpurt( direction );

        if( ( this->isSubclassOf( Actor ) || damage > 10 ) && ShouldGib( meansofdeath, damage ) )
            ProcessEvent( EV_Sentient_SpawnBloodyGibs );
    }
*/

    if (health <= 0) {
        // See if we can kill this actor or not

        if (this->IsSubclassOfActor()) {
            Actor *act = (Actor *)this;

            if (act->IsImmortal()) {
                health = 1;
            }
        }
    }

    if (health > 0) {
        // Send pain event
        event = new Event(EV_Pain, 10);
        event->AddEntity(attacker);
        event->AddFloat(damage);
        event->AddEntity(inflictor);
        event->AddVector(position);
        event->AddVector(direction);
        event->AddVector(normal);
        event->AddInteger(knockback);
        event->AddInteger(dflags);
        event->AddInteger(meansofdeath);
        event->AddInteger(location);

        ProcessEvent(event);
    }

    if (health < 0.1) {
        // Make sure health is now 0

        health = 0;

        event = new Event(EV_Killed, 10);
        event->AddEntity(attacker);
        event->AddFloat(damage);
        event->AddEntity(inflictor);
        event->AddVector(position);
        event->AddVector(direction);
        event->AddVector(normal);
        event->AddInteger(knockback);
        event->AddInteger(dflags);
        event->AddInteger(meansofdeath);
        event->AddInteger(location);

        ProcessEvent(event);
    }

    return;
}

qboolean Sentient::CanBlock(int meansofdeath, qboolean full_block)
{
    // Check to see what a full block can't even block

    switch (meansofdeath) {
    case MOD_TELEFRAG:
    case MOD_SLIME:
    case MOD_LAVA:
    case MOD_FALLING:
    case MOD_IMPALE:
    case MOD_ON_FIRE:
    case MOD_ELECTRICWATER:
        return false;
    }

    // Full blocks block everything else

    if (full_block) {
        return true;
    }

    // Check to see what a small block can't block

    switch (meansofdeath) {
    case MOD_FIRE:
    case MOD_CRUSH_EVERY_FRAME:
        return false;
    }

    // Everything else is blocked

    return true;
}

void Sentient::AddBloodSpurt(Vector direction)
{
    Entity *blood;
    Vector  dir;
    Event  *event;
    str     blood_splat_name;
    float   blood_splat_size;
    float   length;
    trace_t trace;
    float   scale;

    if (!com_blood->integer) {
        return;
    }

    next_bleed_time = level.time + .5;

    // Calculate a good scale for the blood

    if (mass < 50) {
        scale = .5;
    } else if (mass > 300) {
        scale = 1.5;
    } else if (mass >= 200) {
        scale = mass / 200.0;
    } else {
        scale = .5 + (mass - 50) / 300;
    }

    // Add blood spurt

    blood = new Animate;
    blood->setModel(blood_model);

    dir[0]        = -direction[0];
    dir[1]        = -direction[1];
    dir[2]        = -direction[2];
    blood->angles = dir.toAngles();
    blood->setAngles(blood->angles);

    blood->setOrigin(centroid);
    blood->origin.copyTo(blood->edict->s.origin2);
    blood->setSolidType(SOLID_NOT);
    blood->setScale(scale);

    event = new Event(EV_Remove);
    blood->PostEvent(event, 1);

    // Add blood splats near feet

    blood_splat_name = GetBloodSplatName();
    blood_splat_size = GetBloodSplatSize();

    if (blood_splat_name.length() && G_Random() < 0.5) {
        dir = origin - centroid;
        dir.z -= 50;
        dir.x += G_CRandom(20);
        dir.y += G_CRandom(20);

        length = dir.length();

        dir.normalize();

        dir = dir * (length + 10);

        trace = G_Trace(centroid, vec_zero, vec_zero, centroid + dir, NULL, MASK_DEADSOLID, false, "AddBloodSpurt");

        if (trace.fraction < 1) {
            Decal *decal = new Decal;
            decal->setShader(blood_splat_name);
            decal->setOrigin(Vector(trace.endpos) + (Vector(trace.plane.normal) * 0.2f));
            decal->setDirection(trace.plane.normal);
            decal->setOrientation("random");
            decal->setRadius(blood_splat_size + G_Random(blood_splat_size));
        }
    }
}

qboolean Sentient::ShouldBleed(int meansofdeath, qboolean dead)
{
    // Make sure we have a blood model

    if (!blood_model.length()) {
        return false;
    }

    // See if we can bleed now based on means of death

    switch (meansofdeath) {
        // Sometimes bleed (based on time)

    case MOD_BULLET:
    case MOD_CRUSH_EVERY_FRAME:
    case MOD_ELECTRICWATER:

        if (next_bleed_time > level.time) {
            return false;
        }

        break;

        // Sometimes bleed (based on chance)

    case MOD_SHOTGUN:

        if (G_Random() > 0.1) {
            return false;
        }

        break;

        // Never bleed

    case MOD_SLIME:
    case MOD_LAVA:
    case MOD_FIRE:
    case MOD_FLASHBANG:
    case MOD_ON_FIRE:
    case MOD_FALLING:
        return false;
    }

    // Always bleed by default

    return true;
}

// ShouldGib assumes that ShouldBleed has already been called

qboolean Sentient::ShouldGib(int meansofdeath, float damage)

{
    // See if we can gib based on means of death

    switch (meansofdeath) {
        // Always gib

    case MOD_CRUSH_EVERY_FRAME:

        return true;

        break;

        // Sometimes gib

    case MOD_BULLET:

        if (G_Random(100) < damage * 10) {
            return true;
        }

        break;

    case MOD_BEAM:

        if (G_Random(100) < damage * 5) {
            return true;
        }

        break;

        // Never gib

    case MOD_SLIME:
    case MOD_LAVA:
    case MOD_FIRE:
    case MOD_FLASHBANG:
    case MOD_ON_FIRE:
    case MOD_FALLING:
    case MOD_ELECTRICWATER:
        return false;
    }

    // Default is random based on how much damage done

    if (G_Random(100) < damage * 2) {
        return true;
    }

    return false;
}

str Sentient::GetBloodSpurtName(void)
{
    str blood_spurt_name;

    if (blood_model == "fx_bspurt.tik") {
        blood_spurt_name = "fx_bspurt2.tik";
    } else if (blood_model == "fx_gspurt.tik") {
        blood_spurt_name = "fx_gspurt2.tik";
    } else if (blood_model == "fx_bspurt_blue.tik") {
        blood_spurt_name = "fx_bspurt2_blue.tik";
    }

    return blood_spurt_name;
}

str Sentient::GetBloodSplatName(void)
{
    str blood_splat_name;

    if (blood_model == "fx_bspurt.tik") {
        blood_splat_name = "bloodsplat.spr";
    } else if (blood_model == "fx_gspurt.tik") {
        blood_splat_name = "greensplat.spr";
    } else if (blood_model == "fx_bspurt_blue.tik") {
        blood_splat_name = "bluesplat.spr";
    }

    return blood_splat_name;
}

float Sentient::GetBloodSplatSize(void)
{
    float m;

    m = mass;

    if (m < 50) {
        m = 50;
    } else if (m > 250) {
        m = 250;
    }

    return (10 + (m - 50) / 200 * 6);
}

str Sentient::GetGibName(void)
{
    str gib_name;

    if (blood_model == "fx_bspurt.tik") {
        gib_name = "fx_rgib";
    } else if (blood_model == "fx_gspurt.tik") {
        gib_name = "fx_ggib";
    }

    return gib_name;
}

int Sentient::NumInventoryItems(void)

{
    return inventory.NumObjects();
}

Item *Sentient::NextItem(Item *item)

{
    Item    *next_item;
    int      i;
    int      n;
    qboolean item_found = false;

    if (!item) {
        item_found = true;
    } else if (!inventory.ObjectInList(item->entnum)) {
        error("NextItem", "Item not in list");
    }

    n = inventory.NumObjects();

    for (i = 1; i <= n; i++) {
        next_item = (Item *)G_GetEntity(inventory.ObjectAt(i));
        assert(next_item);

        if (next_item->isSubclassOf(InventoryItem) && item_found) {
            return next_item;
        }

        if (next_item == item) {
            item_found = true;
        }
    }

    return NULL;
}

Item *Sentient::PrevItem(Item *item)

{
    Item    *prev_item;
    int      i;
    int      n;
    qboolean item_found = false;

    if (!item) {
        item_found = true;
    } else if (!inventory.ObjectInList(item->entnum)) {
        error("NextItem", "Item not in list");
    }

    n = inventory.NumObjects();

    for (i = n; i >= 1; i--) {
        prev_item = (Item *)G_GetEntity(inventory.ObjectAt(i));
        assert(prev_item);

        if (prev_item->isSubclassOf(InventoryItem) && item_found) {
            return prev_item;
        }

        if (prev_item == item) {
            item_found = true;
        }
    }

    return NULL;
}

void Sentient::DropInventoryItems(void)
{
    int   num;
    int   i;
    Item *item;

    // Drop any inventory items
    num = inventory.NumObjects();
    for (i = num; i >= 1; i--) {
        item = (Item *)G_GetEntity(inventory.ObjectAt(i));
        if (item->isSubclassOf(InventoryItem)) {
            if (m_bDontDropWeapons && item->IsSubclassOfWeapon()) {
                item->Delete();
            } else {
                item->Drop();
            }
        }
    }
}

qboolean Sentient::PowerupActive(void)

{
    if (poweruptype && this->client) {
        gi.SendServerCommand(edict - g_entities, "print \"You are already using a powerup\n\"");
    }

    return poweruptype;
}

void Sentient::setModel(const char *mdl)
{
    // Rebind all active weapons

    DetachAllActiveWeapons();
    Entity::setModel(mdl);
    AttachAllActiveWeapons();
}

void Sentient::TurnOffShadow(Event *ev)

{
    edict->s.renderfx &= ~RF_SHADOW;
}

void Sentient::TurnOnShadow(Event *ev)

{
    edict->s.renderfx |= RF_SHADOW;
}

void Sentient::Archive(Archiver& arc)
{
    int i;
    int num;

    Animate::Archive(arc);

    arc.ArchiveSafePointer(&m_pNextSquadMate);
    arc.ArchiveSafePointer(&m_pPrevSquadMate);

    inventory.Archive(arc);
    if (arc.Saving()) {
        num = ammo_inventory.NumObjects();
    } else {
        ammo_inventory.ClearObjectList();
    }
    arc.ArchiveInteger(&num);
    for (i = 1; i <= num; i++) {
        Ammo *ptr;

        if (arc.Loading()) {
            ptr = new Ammo;
            ammo_inventory.AddObject(ptr);
        } else {
            ptr = ammo_inventory.ObjectAt(i);
        }
        arc.ArchiveObject(ptr);
    }

    arc.ArchiveFloat(&LMRF);

    arc.ArchiveInteger(&poweruptype);
    arc.ArchiveInteger(&poweruptimer);

    arc.ArchiveVector(&offset_color);
    arc.ArchiveVector(&offset_delta);
    arc.ArchiveFloat(&charge_start_time);
    arc.ArchiveString(&blood_model);

    for (i = 0; i < MAX_ACTIVE_WEAPONS; i++) {
        arc.ArchiveSafePointer(&activeWeaponList[i]);
    }

    newActiveWeapon.Archive(arc);
    arc.ArchiveSafePointer(&holsteredWeapon);
    arc.ArchiveBool(&weapons_holstered_by_code);
    lastActiveWeapon.Archive(arc);

    for (int i = 0; i < MAX_DAMAGE_MULTIPLIERS; i++) {
        arc.ArchiveFloat(&m_fDamageMultipliers[i]);
    }

    arc.ArchiveSafePointer(&m_pVehicle);
    arc.ArchiveSafePointer(&m_pTurret);
    arc.ArchiveSafePointer(&m_pLadder);

    arc.ArchiveString(&m_sHelmetSurface1);
    arc.ArchiveString(&m_sHelmetSurface2);
    arc.ArchiveString(&m_sHelmetTiki);

    arc.ArchiveFloat(&m_fHelmetSpeed);

    arc.ArchiveVector(&gunoffset);
    arc.ArchiveVector(&eyeposition);
    arc.ArchiveInteger(&viewheight);
    arc.ArchiveVector(&m_vViewVariation);
    arc.ArchiveInteger(&means_of_death);

    arc.ArchiveBool(&in_melee_attack);
    arc.ArchiveBool(&in_block);
    arc.ArchiveBool(&in_stun);
    arc.ArchiveBool(&on_fire);
    arc.ArchiveFloat(&on_fire_stop_time);
    arc.ArchiveFloat(&next_catch_on_fire_time);
    arc.ArchiveInteger(&on_fire_tagnums[0]);
    arc.ArchiveInteger(&on_fire_tagnums[1]);
    arc.ArchiveInteger(&on_fire_tagnums[2]);
    arc.ArchiveSafePointer(&fire_owner);

    arc.ArchiveBool(&attack_blocked);
    arc.ArchiveFloat(&attack_blocked_time);

    arc.ArchiveFloat(&max_mouth_angle);
    arc.ArchiveInteger(&max_gibs);

    arc.ArchiveFloat(&next_bleed_time);

    arc.ArchiveBool(&m_bFootOnGround_Right);
    arc.ArchiveBool(&m_bFootOnGround_Left);

    arc.ArchiveObjectPointer((Class **)&m_NextSentient);
    arc.ArchiveObjectPointer((Class **)&m_PrevSentient);

    arc.ArchiveVector(&mTargetPos);
    arc.ArchiveFloat(&mAccuracy);

    arc.ArchiveInteger(&m_Team);
    arc.ArchiveInteger(&m_iAttackerCount);

    arc.ArchiveSafePointer(&m_pLastAttacker);
    arc.ArchiveSafePointer(&m_Enemy);

    arc.ArchiveFloat(&m_fPlayerSightLevel);

    arc.ArchiveBool(&m_bIsDisguised);
    arc.ArchiveBool(&m_bHasDisguise);

    arc.ArchiveInteger(&m_ShowPapersTime);
    arc.ArchiveInteger(&m_iLastHitTime);
    arc.ArchiveInteger(&m_iThreatBias);

    arc.ArchiveBool(&m_bDontDropWeapons);
    arc.ArchiveBool(&m_bIsAnimal);
    arc.ArchiveBool(&m_bForceDropHealth);
    arc.ArchiveBool(&m_bForceDropWeapon);

    if (arc.Loading()) {
        if (WeaponsOut()) {
            Holster(true);
        }
    }

    //
    // Openmohaa additions
    //
    arc.ArchiveInteger(&iNextLandTime);
}

static bool IsItemName(const char *name)
{
    if (!str::icmp(name, "models/items/camera.tik")) {
        return true;
    } else if (!str::icmp(name, "models/items/binoculars.tik")) {
        return true;
    } else if (!str::icmp(name, "models/items/papers.tik")) {
        return true;
    } else if (!str::icmp(name, "models/items/papers2.tik")) {
        return true;
    }

    return false;
}

void Sentient::ArchivePersistantData(Archiver& arc)
{
    int i;
    int num;

    // archive the inventory
    if (arc.Saving()) {
        // count up the total number
        num = inventory.NumObjects();
    } else {
        inventory.ClearObjectList();
    }
    // archive the number
    arc.ArchiveInteger(&num);
    // archive each item
    for (i = 1; i <= num; i++) {
        str   name;
        int   amount;
        Item *item;

        if (arc.Saving()) {
            Entity *ent;

            ent = G_GetEntity(inventory.ObjectAt(i));
            if (ent && ent->isSubclassOf(Item)) {
                item   = (Item *)ent;
                name   = item->model;
                amount = item->getAmount();
            } else {
                error("ArchivePersistantData", "Non Item in inventory\n");
            }
        }
        arc.ArchiveString(&name);
        arc.ArchiveInteger(&amount);
        if (arc.Loading()) {
            item = giveItem(name, amount);
            item->CancelEventsOfType(EV_Weapon_GiveStartingAmmo);
        }
    }

    // archive the ammo inventory
    if (arc.Saving()) {
        // count up the total number
        num = ammo_inventory.NumObjects();
    } else {
        ammo_inventory.ClearObjectList();
    }
    // archive the number
    arc.ArchiveInteger(&num);
    // archive each item
    for (i = 1; i <= num; i++) {
        str   name;
        int   amount;
        int   maxamount;
        Ammo *ptr;

        if (arc.Saving()) {
            ptr       = ammo_inventory.ObjectAt(i);
            name      = ptr->getName();
            amount    = ptr->getAmount();
            maxamount = ptr->getMaxAmount();
        }
        arc.ArchiveString(&name);
        arc.ArchiveInteger(&amount);
        arc.ArchiveInteger(&maxamount);
        if (arc.Loading()) {
            GiveAmmo(name, amount, maxamount);
        }
    }

    for (i = 0; i < MAX_ACTIVE_WEAPONS; i++) {
        str name;
        if (arc.Saving()) {
            if (activeWeaponList[i]) {
                name = activeWeaponList[i]->getName();
            } else {
                name = "none";
            }
        }
        arc.ArchiveString(&name);
        if (arc.Loading()) {
            if (name != "none") {
                Weapon *weapon;
                weapon = (Weapon *)FindItem(name);
                if (weapon) {
                    ChangeWeapon(weapon, (weaponhand_t)i);
                }
            }
        }
    }

    arc.ArchiveFloat(&health);
    arc.ArchiveFloat(&max_health);
}

void Sentient::DoubleArmor(void)
{
    int i, n;

    n = inventory.NumObjects();

    for (i = 1; i <= n; i++) {
        Item *item;
        item = (Item *)G_GetEntity(inventory.ObjectAt(i));

        if (item->isSubclassOf(Armor)) {
            item->setAmount(item->getAmount() * 2);
        }
    }
}

void Sentient::JumpXY(Event *ev)

{
    float  forwardmove;
    float  sidemove;
    float  distance;
    float  time;
    float  speed;
    Vector yaw_forward;
    Vector yaw_left;

    forwardmove = ev->GetFloat(1);
    sidemove    = ev->GetFloat(2);
    speed       = ev->GetFloat(3);

    Vector(0, angles.y, 0).AngleVectors(&yaw_forward, &yaw_left);

    velocity = yaw_forward * forwardmove - yaw_left * sidemove;
    distance = velocity.length();
    velocity *= speed / distance;
    time        = distance / speed;
    velocity[2] = sv_gravity->integer * time * 0.5f;
}

void Sentient::BlockStart(Event *ev)

{
    in_block = true;
}

void Sentient::BlockEnd(Event *ev)

{
    in_block = false;
}

void Sentient::StunStart(Event *ev)

{
    in_stun = true;
}

void Sentient::StunEnd(Event *ev)

{
    in_stun = false;
}

void Sentient::ListInventory(void)

{
    int i, count;

    // Display normal inventory
    count = inventory.NumObjects();

    gi.Printf("'Name' : 'Amount'\n");

    for (i = 1; i <= count; i++) {
        int   entnum = inventory.ObjectAt(i);
        Item *item   = (Item *)G_GetEntity(entnum);
        gi.Printf("'%s' : '%d'\n", item->getName().c_str(), item->getAmount());
    }

    // Display ammo inventory
    count = ammo_inventory.NumObjects();

    for (i = 1; i <= count; i++) {
        Ammo *ammo = ammo_inventory.ObjectAt(i);
        gi.Printf("'%s' : '%d'\n", ammo->getName().c_str(), ammo->getAmount());
    }
}

void Sentient::SetAttackBlocked(bool blocked)

{
    attack_blocked      = blocked;
    attack_blocked_time = level.time;
}

void Sentient::SetMaxMouthAngle(Event *ev)
{
    max_mouth_angle = ev->GetFloat(1);
}

void Sentient::TryLightOnFire(int meansofdeath, Entity *attacker)
{
    gi.Printf("Sentient::TryLightOnFire not implemented. Needs fixed");
}

void Sentient::OnFire(Event *ev)
{
    gi.Printf("Sentient::OnFire not implemented. Needs fixed");
}

void Sentient::StopOnFire(Event *ev)
{
    gi.Printf("Sentient::StopOnFire not implemented. Needs fixed");
}

void Sentient::SpawnBloodyGibs(Event *ev)
{
    str      gib_name;
    int      number_of_gibs;
    float    scale;
    Animate *ent;
    str      real_gib_name;

    if (!com_blood->integer) {
        return;
    }

    //if ( GetActorFlag( ACTOR_FLAG_FADING_OUT ) )
    //	return;

    gib_name = GetGibName();

    if (!gib_name.length()) {
        return;
    }

    // Determine the number of gibs to spawn

    if (ev->NumArgs() > 0) {
        number_of_gibs = ev->GetInteger(1);
    } else {
        if (max_gibs == 0) {
            return;
        }

        if (deadflag) {
            number_of_gibs = G_Random(max_gibs / 2) + 1;
        } else {
            number_of_gibs = G_Random(max_gibs) + 1;
        }
    }

    // Make sure we don't have too few or too many gibs

    if (number_of_gibs <= 0 || number_of_gibs > 9) {
        return;
    }

    if (ev->NumArgs() > 1) {
        scale = ev->GetFloat(2);
    } else {
        // Calculate a good scale value

        if (mass <= 50) {
            scale = 1.0f;
        } else if (mass <= 100) {
            scale = 1.1f;
        } else if (mass <= 250) {
            scale = 1.2f;
        } else {
            scale = 1.3f;
        }
    }

    // Spawn the gibs

    real_gib_name = gib_name;
    real_gib_name += number_of_gibs;
    real_gib_name += ".tik";

    ent = new Animate;
    ent->setModel(real_gib_name.c_str());
    ent->setScale(scale);
    ent->setOrigin(centroid);
    ent->NewAnim("idle");
    ent->PostEvent(EV_Remove, 1);

    Sound("snd_decap", CHAN_BODY, 1, 300);
}

void Sentient::SetMaxGibs(Event *ev)
{
    max_gibs = ev->GetInteger(1);
}

void Sentient::GetStateAnims(Container<const char *> *c) {}

void Sentient::CheckAnimations(Event *ev)

{
    int                     i, j;
    Container<const char *> co;
    const char             *cs;

    GetStateAnims(&co);

    gi.DPrintf("Unused Animations in TIKI\n");
    gi.DPrintf("-------------------------\n");
    for (i = 0; i < NumAnims(); i++) {
        const char *c;

        c = gi.Anim_NameForNum(edict->tiki, i);

        for (j = 1; j <= co.NumObjects(); j++) {
            cs = co.ObjectAt(j);

            if (!Q_stricmp(c, cs)) {
                goto out;
            } else if (!Q_stricmpn(c, cs, strlen(cs))) // partial match
            {
                size_t state_len = strlen(cs);

                // Animation in tik file is longer than the state machine's anim
                if (strlen(c) > state_len) {
                    if (c[state_len] != '_') // If next character is an '_' then no match
                    {
                        goto out;
                    }
                } else {
                    goto out;
                }
            }
        }
        // No match made
        gi.DPrintf("%s used in TIK file but not statefile\n", c);
    out:;
    }

    gi.DPrintf("Unknown Animations in Statefile\n");
    gi.DPrintf("-------------------------------\n");
    for (j = 1; j <= co.NumObjects(); j++) {
        if (!HasAnim(co.ObjectAt(j))) {
            gi.DPrintf("%s in statefile is not in TIKI\n", co.ObjectAt(j));
        }
    }
}

void Sentient::EventGerman(Event *ev)
{
    bool bRejoinSquads = false;

    if (ev->IsFromScript()) {
        if (m_Team) {
            bRejoinSquads = true;
        }
    }

    if (bRejoinSquads) {
        ClearEnemies();
        DisbandSquadMate(this);
    }

    Unlink();
    m_Team = TEAM_GERMAN;
    Link();

    if (bRejoinSquads) {
        JoinNearbySquads(1024.0f);
    }
}

void Sentient::EventAmerican(Event *ev)
{
    bool bRejoinSquads = false;

    if (ev->IsFromScript()) {
        if (m_Team != TEAM_AMERICAN) {
            bRejoinSquads = true;
        }
    }

    if (bRejoinSquads) {
        ClearEnemies();
        DisbandSquadMate(this);
    }


    Unlink();
    m_Team = TEAM_AMERICAN;
    Link();

    if (bRejoinSquads) {
        JoinNearbySquads(1024);
    }

    if (IsSubclassOfActor()) {
        Actor *pActor        = static_cast<Actor *>(this);
        pActor->m_csMood     = STRING_NERVOUS;
        pActor->m_csIdleMood = STRING_NERVOUS;
    }
}

void Sentient::EventGetTeam(Event *ev)
{
    if (m_Team == TEAM_AMERICAN) {
        ev->AddConstString(STRING_AMERICAN);
    } else if (m_Team == TEAM_GERMAN) {
        ev->AddConstString(STRING_GERMAN);
    } else {
        ev->AddConstString(STRING_EMPTY);
    }
}

void Sentient::ClearEnemies() {}

void Sentient::EventGetThreatBias(Event *ev)
{
    ev->AddInteger(m_iThreatBias);
}

void Sentient::EventSetThreatBias(Event *ev)
{
    str sBias;

    if (ev->IsStringAt(1)) {
        sBias = ev->GetString(1);

        if (!Q_stricmp(sBias, "ignoreme")) {
            m_iThreatBias = THREATBIAS_IGNOREME;
            return;
        }
    }

    m_iThreatBias = ev->GetInteger(1);
}

void Sentient::SetDamageMult(Event *ev)
{
    int index = ev->GetInteger(1);
    if (index < 0 || index >= MAX_DAMAGE_MULTIPLIERS) {
        ScriptError("Index must be between 0-" STRING(MAX_DAMAGE_MULTIPLIERS - 1) ".");
    }

    m_fDamageMultipliers[index] = ev->GetFloat(2);
}

void Sentient::SetupHelmet(str sHelmetTiki, float fSpeed, float fDamageMult, str sHelmetSurface1, str sHelmetSurface2)
{
    m_sHelmetTiki     = sHelmetTiki;
    m_sHelmetSurface1 = sHelmetSurface1;
    m_sHelmetSurface2 = sHelmetSurface2;

    m_fHelmetSpeed          = fSpeed;
    m_fDamageMultipliers[1] = fDamageMult;
}

void Sentient::EventSetupHelmet(Event *ev)
{
    str sHelmetTiki;
    str sHelmetSurface;

    sHelmetTiki    = ev->GetString(1);
    sHelmetSurface = ev->GetString(4);

    if (ev->NumArgs() == 4) {
        SetupHelmet(sHelmetTiki, ev->GetFloat(2), ev->GetFloat(3), sHelmetSurface, sHelmetSurface);
    } else {
        SetupHelmet(sHelmetTiki, ev->GetFloat(2), ev->GetFloat(3), sHelmetSurface, ev->GetString(5));
    }
}

bool Sentient::WearingHelmet(void)
{
    if (!m_sHelmetSurface1.length()) {
        return false;
    }

    int iSurf = gi.Surface_NameToNum(edict->tiki, m_sHelmetSurface1);
    if (iSurf >= 0) {
        return (~edict->s.surfaces[iSurf] & MDL_SURFACE_NODRAW) != 0;
    } else {
        return false;
    }
}

void Sentient::EventPopHelmet(Event *ev)
{
    int           iSurf;
    vec3_t        vWorldAngles;
    vec3_t        vXAxis, vYAxis, vZAxis;
    orientation_t oLocalTag, oWorldTag;
    int           iHeadTag;
    float         fRandom;
    float         fPitchVelocity, fYawVelocity;
    HelmetObject *obj;

    if (!WearingHelmet()) {
        return;
    }

    iSurf = gi.Surface_NameToNum(edict->tiki, m_sHelmetSurface1.c_str());
    // Hide the helmet
    edict->s.surfaces[iSurf] |= MDL_SURFACE_NODRAW;

    if (m_sHelmetSurface2.length()) {
        iSurf = gi.Surface_NameToNum(edict->tiki, m_sHelmetSurface2.c_str());
        if (iSurf >= 0) {
            // Hide the second helmet
            edict->s.surfaces[iSurf] |= MDL_SURFACE_NODRAW;
        } else {
            Com_Printf(
                "Warning: Surface %s found, but %s not found in setting up helmet for %s.\n",
                m_sHelmetSurface1.c_str(),
                m_sHelmetSurface2.c_str(),
                edict->tiki->name
            );
        }
    }

    if (!m_sHelmetTiki.length()) {
        return;
    }

    iHeadTag  = gi.Tag_NumForName(edict->tiki, "Bip01 Head");
    oLocalTag = G_TIKI_Orientation(edict, iHeadTag);

    for (int i = 0; i < 3; i++) {
        vXAxis[i] = oLocalTag.axis[0][i];
        vYAxis[i] = oLocalTag.axis[1][i];
        vZAxis[i] = oLocalTag.axis[2][i];
    }

    for (int i = 0; i < 3; i++) {
        oLocalTag.axis[0][i] = -vYAxis[i];
        oLocalTag.axis[1][i] = -vZAxis[i];
        oLocalTag.axis[2][i] = vXAxis[i];
    }

    VectorCopy(origin, oWorldTag.origin);

    for (int i = 0; i < 3; i++) {
        VectorMA(oWorldTag.origin, oLocalTag.origin[i], orientation[i], oWorldTag.origin);
    }

    MatrixMultiply(oLocalTag.axis, orientation, oWorldTag.axis);
    MatrixToEulerAngles(oWorldTag.axis, vWorldAngles);

    obj = new HelmetObject();
    obj->setOrigin(oWorldTag.origin);
    obj->setAngles(vWorldAngles);
    obj->setModel(m_sHelmetTiki);

    fRandom = crandom() * 30;
    VectorScale(obj->velocity, fRandom, oWorldTag.axis[0]);

    fRandom = crandom() * 30;
    VectorMA(obj->velocity, fRandom, oWorldTag.axis[1], obj->velocity);

    fRandom = (crandom() * 0.3f + 1.0f) * m_fHelmetSpeed;
    VectorMA(obj->velocity, fRandom, oWorldTag.axis[2], obj->velocity);

    fPitchVelocity = crandom() * 300;
    fYawVelocity   = crandom() * 400;

    obj->avelocity.x = fPitchVelocity;
    obj->avelocity.y = fYawVelocity;
    obj->avelocity.z = crandom() * 300.0;
}

void Sentient::ReceivedItem(Item *item) {}

void Sentient::RemovedItem(Item *item) {}

void Sentient::AssertValidSquad()
{
    for (Sentient *pSquadMate = this; pSquadMate != this; pSquadMate = pSquadMate->m_pNextSquadMate) {}
}

bool Sentient::IsTeamMate(Sentient *pOther)
{
    return (pOther->m_bIsDisguised || pOther->m_Team == m_Team);
}

void Sentient::JoinNearbySquads(float fJoinRadius)
{
    float fJoinRadiusSquared = Square(fJoinRadius);

    for (Sentient *pFriendly = level.m_HeadSentient[m_Team]; pFriendly != NULL; pFriendly = pFriendly->m_NextSentient) {
        if (pFriendly->IsDead() || IsSquadMate(pFriendly) || pFriendly->m_Team != m_Team) {
            continue;
        }

        if (fJoinRadius >= Vector::DistanceSquared(pFriendly->origin, origin)) {
            MergeWithSquad(pFriendly);
        }
    }
}

void Sentient::MergeWithSquad(Sentient *pFriendly)
{
    Sentient *pFriendNext;
    Sentient *pSelfPrev;

    if (!pFriendly || IsDead() || pFriendly->IsDead()) {
        return;
    }

    pFriendNext = pFriendly->m_pNextSquadMate;
    pSelfPrev   = m_pPrevSquadMate;

    pFriendly->m_pNextSquadMate = this;
    m_pPrevSquadMate            = pFriendly;

    pFriendNext->m_pPrevSquadMate = pSelfPrev;
    pSelfPrev->m_pNextSquadMate   = pFriendNext;
}

void Sentient::DisbandSquadMate(Sentient *pExFriendly)
{
    Sentient *pPrev;
    Sentient *pNext;

    AssertValidSquad();

    pPrev = pExFriendly->m_pPrevSquadMate;
    pNext = pExFriendly->m_pNextSquadMate;

    pPrev->m_pNextSquadMate = pNext;
    pNext->m_pPrevSquadMate = pPrev;

    pExFriendly->m_pPrevSquadMate = pExFriendly;
    pExFriendly->m_pNextSquadMate = pExFriendly;

    AssertValidSquad();
    pNext->AssertValidSquad();
}

bool Sentient::IsSquadMate(Sentient *pFriendly)
{
    Sentient *pSquadMate = this;

    while (1) {
        if (pSquadMate == pFriendly) {
            return true;
        }

        pSquadMate = pSquadMate->m_pNextSquadMate;
        if (pSquadMate == this) {
            return false;
        }
    }
}

bool Sentient::IsDisabled() const
{
    return false;
}

VehicleTank *Sentient::GetVehicleTank(void)
{
    if (m_pVehicle && m_pVehicle->IsSubclassOfVehicleTank()) {
        return (VehicleTank *)m_pVehicle.Pointer();
    } else {
        return NULL;
    }
}

void Sentient::UpdateFootsteps(void)
{
    int iAnimNum;
    int iAnimFlags;
    int iTagNum;

    iAnimFlags = 0;

    for (iAnimNum = 0; iAnimNum < MAX_FRAMEINFOS; iAnimNum++) {
        if (edict->s.frameInfo[iAnimNum].weight != 0 && CurrentAnim(iAnimNum) >= 0) {
            iAnimFlags |= gi.Anim_Flags(edict->tiki, CurrentAnim(iAnimNum));
        }
    }

    if (!(iAnimFlags & TAF_AUTOSTEPS_RUNNING) || !(iAnimFlags & TAF_AUTOSTEPS)) {
        // if walking, or if the animation doesn't step
        m_bFootOnGround_Right = true;
        m_bFootOnGround_Left  = true;
        return;
    }

    if (m_bFootOnGround_Right) {
        iTagNum = gi.Tag_NumForName(edict->tiki, "Bip01 R Foot");
        if (iTagNum >= 0) {
            m_bFootOnGround_Right = G_TIKI_IsOnGround(edict, iTagNum, 13.653847f);
        } else {
            m_bFootOnGround_Right = true;
        }
    } else {
        iTagNum = gi.Tag_NumForName(edict->tiki, "Bip01 R Foot");
        if (iTagNum >= 0) {
            if (G_TIKI_IsOnGround(edict, iTagNum, 13.461539f)) {
                BroadcastAIEvent(10, G_AIEventRadius(10));
                // simulate footstep sounds
                Footstep("Bip01 L Foot", (iAnimFlags & TAF_AUTOSTEPS_RUNNING), (iAnimFlags & TAF_AUTOSTEPS_EQUIPMENT));
                m_bFootOnGround_Right = true;
            }
        } else {
            m_bFootOnGround_Right = true;
        }
    }

    if (m_bFootOnGround_Left) {
        iTagNum = gi.Tag_NumForName(edict->tiki, "Bip01 L Foot");
        if (iTagNum >= 0) {
            m_bFootOnGround_Left = G_TIKI_IsOnGround(edict, iTagNum, 13.653847f);
        } else {
            m_bFootOnGround_Left = true;
        }
    } else {
        iTagNum = gi.Tag_NumForName(edict->tiki, "Bip01 L Foot");
        if (iTagNum >= 0) {
            if (G_TIKI_IsOnGround(edict, iTagNum, 13.461539f)) {
                BroadcastAIEvent(10, G_AIEventRadius(10));
                // simulate footstep sounds
                Footstep("Bip01 R Foot", (iAnimFlags & TAF_AUTOSTEPS_RUNNING), (iAnimFlags & TAF_AUTOSTEPS_EQUIPMENT));
                m_bFootOnGround_Left = true;
            }
        } else {
            m_bFootOnGround_Left = true;
        }
    }
}

qboolean Sentient::AIDontFace() const
{
    return qfalse;
}

void Sentient::EventDropItems(Event *ev)
{
    DropInventoryItems();
}

void Sentient::EventDontDropWeapons(Event *ev)
{
    if (ev->NumArgs() > 0) {
        m_bDontDropWeapons = ev->GetBoolean(1);
    } else {
        m_bDontDropWeapons = true;
    }
}

void Sentient::EventForceDropWeapon(Event *ev)
{
    if (ev->NumArgs() > 0) {
        m_bForceDropWeapon = ev->GetBoolean(1);
    } else {
        m_bForceDropWeapon = true;
    }
}

void Sentient::EventForceDropHealth(Event *ev)
{
    if (ev->NumArgs() > 0) {
        m_bForceDropHealth = ev->GetBoolean(1);
    } else {
        m_bForceDropHealth = true;
    }
}

void Sentient::EventGetForceDropWeapon(Event *ev)
{
    ev->AddInteger(m_bForceDropWeapon);
}

void Sentient::EventGetForceDropHealth(Event *ev)
{
    ev->AddInteger(m_bForceDropHealth);
}

void Sentient::SetViewAngles(Vector angles) {}

void Sentient::SetTargetViewAngles(Vector angles) {}

Vector Sentient::GetViewAngles(void)
{
    return angles;
}

void Sentient::AddViewVariation(const Vector& vVariation)
{
    m_vViewVariation += vVariation;
}

void Sentient::SetMinViewVariation(const Vector& vVariation)
{
    m_vViewVariation.x = Q_min(m_vViewVariation.x, vVariation.x);
    m_vViewVariation.y = Q_min(m_vViewVariation.y, vVariation.y);
    m_vViewVariation.z = Q_min(m_vViewVariation.z, vVariation.z);
}

void Sentient::SetHolsteredByCode(bool holstered) {
    weapons_holstered_by_code = holstered;
}

Vehicle* Sentient::GetVehicle() const
{
    return m_pVehicle;
}

void Sentient::SetVehicle(Vehicle* pVehicle)
{
    m_pVehicle = NULL;
}

TurretGun* Sentient::GetTurret() const
{
    return m_pTurret;
}

void Sentient::SetTurret(TurretGun* pTurret)
{
    m_pTurret = pTurret;
}

#define GROUND_DISTANCE        8
#define WATER_NO_SPLASH_HEIGHT 16

void Sentient::EventClientLanding(Event* ev)
{
    float fVolume = ev->NumArgs() >= 1 ? ev->GetFloat(1) : 1;
    int iEquipment = ev->NumArgs() >= 2 ? ev->GetInteger(2) : 1;

    LandingSound(fVolume, iEquipment);
}

void Sentient::FootstepMain(trace_t* trace, int iRunning, int iEquipment)
{
    int    contents;
    int    surftype;
    float  fVolume;
    vec3_t vPos;
    vec3_t midlegs;
    str    sSoundName;

    VectorCopy(trace->endpos, vPos);
    sSoundName = "snd_step_";

    contents = gi.pointcontents(trace->endpos, -1);
    if (contents & MASK_WATER) {
        // take our ground position and trace upwards
        VectorCopy(trace->endpos, midlegs);
        midlegs[2] += WATER_NO_SPLASH_HEIGHT;
        contents = gi.pointcontents(midlegs, -1);
        if (contents & MASK_WATER) {
            sSoundName += "wade";
        }
        else {
            sSoundName += "puddle";
        }
    }
    else {
        surftype = trace->surfaceFlags & MASK_SURF_TYPE;
        switch (surftype) {
        case SURF_FOLIAGE:
            sSoundName += "foliage";
            break;
        case SURF_SNOW:
            sSoundName += "snow";
            break;
        case SURF_CARPET:
            sSoundName += "carpet";
            break;
        case SURF_SAND:
            sSoundName += "sand";
            break;
        case SURF_PUDDLE:
            sSoundName += "puddle";
            break;
        case SURF_GLASS:
            sSoundName += "glass";
            break;
        case SURF_GRAVEL:
            sSoundName += "gravel";
            break;
        case SURF_MUD:
            sSoundName += "mud";
            break;
        case SURF_DIRT:
            sSoundName += "dirt";
            break;
        case SURF_GRILL:
            sSoundName += "grill";
            break;
        case SURF_GRASS:
            sSoundName += "grass";
            break;
        case SURF_ROCK:
            sSoundName += "stone";
            break;
        case SURF_PAPER:
            sSoundName += "paper";
            break;
        case SURF_WOOD:
            sSoundName += "wood";
            break;
        case SURF_METAL:
            sSoundName += "metal";
            break;
        default:
            sSoundName += "stone";
            break;
        }
    }

    if (iRunning) {
        if (iRunning == -1) {
            fVolume = 0.5;
        } else {
            fVolume = 1.0;
        }
    }
    else {
        fVolume = 0.25;
    }

    if (!iRunning && g_gametype->integer == GT_SINGLE_PLAYER) {
        return;
    }

    PlayNonPvsSound(sSoundName, fVolume);

    if (iEquipment && random() < 0.3) {
        // also play equipment sound
        PlayNonPvsSound("snd_step_equipment", fVolume);
    }
}

void Sentient::Footstep(const char* szTagName, int iRunning, int iEquipment)
{
    int           i;
    int           iTagNum;
    vec3_t        vStart, vEnd;
    vec3_t        midlegs;
    vec3_t        vMins, vMaxs;
    str           sSoundName;
    trace_t       trace;
    orientation_t oTag;

    // send a trace down from the player to the ground
    VectorCopy(this->origin, vStart);
    vStart[2] += GROUND_DISTANCE;

    if (szTagName) {
        iTagNum = gi.Tag_NumForName(this->edict->tiki, szTagName);
        if (iTagNum != -1) {
            oTag = G_TIKI_Orientation(this->edict, iTagNum);

            for (i = 0; i < 2; i++) {
                VectorMA(vStart, oTag.origin[i], this->orientation[i], vStart);
            }
        }
    }

    if (iRunning == -1) {
        AngleVectors(this->angles, midlegs, NULL, NULL);
        VectorMA(vStart, -16, midlegs, vStart);
        VectorMA(vStart, 64, midlegs, vEnd);

        VectorSet(vMins, -2, -2, -8);
        VectorSet(vMaxs, 2, 2, 8);
    }
    else {
        VectorSet(vMins, -4, -4, 0);
        VectorSet(vMaxs, 4, 4, 2);

        // add 16 units above feets
        vStart[2] += 16.0;
        VectorCopy(vStart, vEnd);
        vEnd[2] -= 64.0;
    }

    if (IsSubclassOfPlayer()) {
        trace = G_Trace(
            vStart,
            vMins,
            vMaxs,
            vEnd,
            edict,
            MASK_PLAYERSOLID,
            qtrue,
            "Player Footsteps"
        );
    }
    else {
        trace = G_Trace(
            vStart,
            vMins,
            vMaxs,
            vEnd,
            edict,
            MASK_MONSTERSOLID,
            qfalse,
            "Monster Footsteps"
        );
    }

    if (trace.fraction == 1.0f) {
        return;
    }

    FootstepMain(&trace, iRunning, iEquipment);
}

void Sentient::LandingSound(float volume, int iEquipment)
{
    int     contents;
    int     surftype;
    vec3_t  vStart, vEnd;
    vec3_t  midlegs;
    str     sSoundName;
    trace_t trace;
    static vec3_t g_vFootstepMins = { -4, -4, 0 };
    static vec3_t g_vFootstepMaxs = { 4, 4, 2 };

    if (this->iNextLandTime > level.inttime) {
        this->iNextLandTime = level.inttime + 200;
        return;
    }

    this->iNextLandTime = level.time + 200;
    VectorCopy(this->origin, vStart);
    vStart[2] += GROUND_DISTANCE;

    VectorCopy(vStart, vEnd);
    vEnd[2] -= 64.0;

    if (IsSubclassOfPlayer()) {
        trace = G_Trace(
            vStart,
            g_vFootstepMins,
            g_vFootstepMaxs,
            vEnd,
            edict,
            MASK_PLAYERSOLID,
            qtrue,
            "Player Footsteps"
        );
    } else {
        trace = G_Trace(
            vStart,
            g_vFootstepMins,
            g_vFootstepMaxs,
            vEnd,
            edict,
            MASK_MONSTERSOLID,
            qfalse,
            "Monster Footsteps"
        );
    }

    if (trace.fraction == 1.0) {
        return;
    }

    sSoundName += "snd_landing_";

    contents = gi.pointcontents(trace.endpos, -1);
    if (contents & MASK_WATER) {
        // take our ground position and trace upwards
        VectorCopy(trace.endpos, midlegs);
        midlegs[2] += WATER_NO_SPLASH_HEIGHT;
        contents = gi.pointcontents(midlegs, -1);
        if (contents & MASK_WATER) {
            sSoundName += "wade";
        } else {
            sSoundName += "puddle";
        }
    } else {
        surftype = trace.surfaceFlags & MASK_SURF_TYPE;
        switch (surftype) {
        case SURF_FOLIAGE:
            sSoundName += "foliage";
            break;
        case SURF_SNOW:
            sSoundName += "snow";
            break;
        case SURF_CARPET:
            sSoundName += "carpet";
            break;
        case SURF_SAND:
            sSoundName += "sand";
            break;
        case SURF_PUDDLE:
            sSoundName += "puddle";
            break;
        case SURF_GLASS:
            sSoundName += "glass";
            break;
        case SURF_GRAVEL:
            sSoundName += "gravel";
            break;
        case SURF_MUD:
            sSoundName += "mud";
            break;
        case SURF_DIRT:
            sSoundName += "dirt";
            break;
        case SURF_GRILL:
            sSoundName += "grill";
            break;
        case SURF_GRASS:
            sSoundName += "grass";
            break;
        case SURF_ROCK:
            sSoundName += "stone";
            break;
        case SURF_PAPER:
            sSoundName += "paper";
            break;
        case SURF_WOOD:
            sSoundName += "wood";
            break;
        case SURF_METAL:
            sSoundName += "metal";
            break;
        default:
            sSoundName += "stone";
            break;
        }
    }

    PlayNonPvsSound(sSoundName, volume);

    if (iEquipment && random() < 0.5) {
        PlayNonPvsSound("snd_step_equipment", volume);
    }
}
