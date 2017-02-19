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

// sentient.cpp: Base class of entity that can carry other entities, and use weapons.
//

#include "g_local.h"
#include "entity.h"
#include "sentient.h"
#include "weapon.h"
#include "weaputils.h"
#include "scriptmaster.h"
#include "ammo.h"
#include "armor.h"
#include "misc.h"
#include "inventoryitem.h"
#include "player.h"
#include "actor.h"
#include "decals.h"
#include <g_spawn.h>

// FIXME
// remove this when actor gets checked back in
Event EV_Sentient_UselessCheck
	( 
	"getridofthis",
	EV_DEFAULT,
   "",
   "",
   "",
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
Event EV_Sentient_GiveTargetname
	( 
	"give",
	EV_DEFAULT,
   "s",
   "name",
   "Gives the sentient the targeted item.",
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
Event EV_Sentient_UpdateOffsetColor
	( 
	"updateoffsetcolor",
	EV_DEFAULT,
   NULL,
   NULL,
   "Updates the offset color.",
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
Event EV_Sentient_ActivateNewWeapon
	(
	"activatenewweapon",
	EV_DEFAULT,
   NULL,
   NULL,
   "Activate the new weapon specified by useWeapon. handsurf allows specifying which hand to use for the player",
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
Event EV_Sentient_DontDropWeapons
	(
	"dontdropweapons",
	EV_DEFAULT,
	"B",
	"dont_drop",
	"Make the sentient not drop weapons",
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
Event EV_Sentient_UseLastWeapon
	(
	"uselast",
	EV_DEFAULT,
	NULL,
	NULL,
	"Activates the last active weapon",
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
Event EV_Sentient_ToggleItemUse
	(
	"toggleitem",
	EV_CONSOLE,
	NULL,
	NULL,
	"Toggles the use of the player's item (first item if he has multiple)",
	EV_NORMAL
	);
Event EV_Sentient_ReloadWeapon
	(
	"reloadweapon",
	EV_DEFAULT,
	"s",
	"hand",
	"Reloads the weapon in the specified hand",
	EV_NORMAL
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

Event EV_Sentient_GetNewActiveWeap
(
	"getnewactiveweap",
	EV_DEFAULT,
	NULL,
	NULL,
	"gets new active weapon",
	EV_RETURN
);
Event EV_Sentient_German
(
	"german",
	EV_DEFAULT,
	NULL,
	NULL,
	"Makes the sentient a German."
);
Event EV_Sentient_American
(
	"american",
	EV_DEFAULT,
	NULL,
	NULL,
	"Makes the sentient an American."
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
	"Sets the damage multiplier for a particular body location"
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
	"Sets the threat bias for this player / AI"
);
Event EV_Sentient_SetupHelmet
(
	"sethelmet",
	EV_DEFAULT,
	"sffss",
	"tikifile popspeed dmgmult surfacename [optional_additional_surface_name]",
	"Gives the sentient a helmet and sets the needed info for it"
);
Event EV_Sentient_PopHelmet
(
	"pophelmet",
	EV_DEFAULT,
	NULL,
	NULL,
	"Pops a sentient's helmet off if he's got one"
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
Event EV_Sentient_PutawayWeapon
(
	"putawayweapon",
	EV_DEFAULT,
	"s",
	"whichHand",
	"Put away or deactivate the current weapon, whichHand can be left or right.",
	EV_NORMAL
);

CLASS_DECLARATION( Animate, Sentient, NULL )
{
		{ &EV_Sentient_Attack,					&Sentient::FireWeapon },
		{ &EV_Sentient_StopFire,				&Sentient::StopFireWeapon },
		{ &EV_Sentient_Charge,					&Sentient::ChargeWeapon },
		{ &EV_Sentient_ReleaseAttack,			&Sentient::ReleaseFireWeapon },
		{ &EV_Sentient_GiveAmmo,				&Sentient::EventGiveAmmo },
		{ &EV_Sentient_GiveWeapon,				&Sentient::EventGiveItem },
		{ &EV_Sentient_GiveArmor,				&Sentient::EventGiveItem },
		{ &EV_Sentient_GiveItem,				&Sentient::EventGiveItem },
		{ &EV_Sentient_Take,					&Sentient::EventTake },
		{ &EV_Sentient_TakeAll,					&Sentient::EventFreeInventory },
		{ &EV_Sentient_SetBloodModel,			&Sentient::SetBloodModel },
		{ &EV_Sentient_GiveTargetname,			&Sentient::EventGiveTargetname },
		{ &EV_Damage,							&Sentient::ArmorDamage },
  		{ &EV_Sentient_TurnOffShadow,			&Sentient::TurnOffShadow },
		{ &EV_Sentient_TurnOnShadow,			&Sentient::TurnOnShadow },
		{ &EV_Sentient_UpdateOffsetColor,		&Sentient::UpdateOffsetColor },
		{ &EV_Sentient_JumpXY,					&Sentient::JumpXY },
		{ &EV_Sentient_MeleeAttackStart,		&Sentient::MeleeAttackStart },
		{ &EV_Sentient_MeleeAttackEnd,			&Sentient::MeleeAttackEnd },
		{ &EV_Sentient_BlockStart,				&Sentient::BlockStart },
		{ &EV_Sentient_BlockEnd,				&Sentient::BlockEnd },
		{ &EV_Sentient_StunStart,				&Sentient::StunStart },
		{ &EV_Sentient_StunEnd,					&Sentient::StunEnd },
		{ &EV_Sentient_SetMaxMouthAngle,		&Sentient::SetMaxMouthAngle },
		{ &EV_Sentient_OnFire,					&Sentient::OnFire },
		{ &EV_Sentient_StopOnFire,				&Sentient::StopOnFire },
		{ &EV_Sentient_SpawnBloodyGibs,			&Sentient::SpawnBloodyGibs },
		{ &EV_Sentient_SetMaxGibs,				&Sentient::SetMaxGibs },
		{ &EV_Sentient_CheckAnimations,			&Sentient::CheckAnimations },
		{ &EV_Sentient_ActivateNewWeapon,		&Sentient::ActivateNewWeapon },
		{ &EV_Sentient_DeactivateWeapon,		&Sentient::DeactivateWeapon },
		{ &EV_Sentient_Weapon,					&Sentient::WeaponCommand },
		{ &EV_Sentient_PutawayWeapon,			&Sentient::PutawayWeapon },
		{ &EV_Sentient_UseItem,					&Sentient::EventUseItem },
		{ &EV_Sentient_UseLastWeapon,			&Sentient::EventActivateLastActiveWeapon },
		{ &EV_Sentient_DontDropWeapons,			&Sentient::EventDontDropWeapons },
		{ &EV_Sentient_UseWeaponClass,			&Sentient::EventUseWeaponClass },
		{ &EV_Sentient_ToggleItemUse,			&Sentient::EventToggleItem },
		{ &EV_Sentient_ReloadWeapon,			&Sentient::ReloadWeapon },
		{ &EV_Sentient_GetActiveWeap,			&Sentient::GetActiveWeap },
		{ &EV_Sentient_GetNewActiveWeap,		&Sentient::GetNewActiveWeapon },
		{ &EV_Sentient_German,					&Sentient::EventGerman },
		{ &EV_Sentient_American,				&Sentient::EventAmerican },
		{ &EV_Sentient_GetTeam,					&Sentient::EventGetTeam },
		{ &EV_Sentient_SetDamageMult,			&Sentient::SetDamageMult },
		{ &EV_Sentient_GetThreatBias,			&Sentient::EventGetThreatBias },
		{ &EV_Sentient_SetThreatBias,			&Sentient::EventSetThreatBias },
		{ &EV_Sentient_SetThreatBias2,			&Sentient::EventSetThreatBias },
		{ &EV_Sentient_SetupHelmet,				&Sentient::EventSetupHelmet },
		{ &EV_Sentient_PopHelmet,				&Sentient::EventPopHelmet },
		{ NULL, NULL }
};

Container<Sentient *> SentientList;

Sentient::Sentient()
{
	SentientList.AddObject( ( Sentient * )this );
	entflags |= EF_SENTIENT;

	if( LoadingSavegame )
	{
		return;
	}

	setContents( CONTENTS_BODY );
	inventory.ClearObjectList();

	m_pNextSquadMate = this;
	m_pPrevSquadMate = this;

	m_Enemy.Clear();

	m_fPlayerSightLevel = 0;

	eyeposition = "0 0 64";
	charge_start_time = 0;
	poweruptype = 0;
	poweruptimer = 0;
	// do better lighting on all sentients
	edict->s.renderfx |= RF_EXTRALIGHT;
	edict->s.renderfx |= RF_SHADOW;
	// sentients have precise shadows
	edict->s.renderfx |= RF_SHADOW_PRECISE;

	m_vViewVariation = "0 0 0";
	memset( activeWeaponList, 0, sizeof( activeWeaponList ) );
	in_melee_attack = false;
	in_block = false;
	in_stun = false;
	attack_blocked = qfalse;
	max_mouth_angle = 10;
	// touch triggers by default
	flags |= FL_TOUCH_TRIGGERS;

	m_pVehicle.Clear();
	m_pTurret.Clear();
	m_pLadder.Clear();
	m_pLastAttacker.Clear();

	m_bIsDisguised = false;
	m_iLastHitTime = 0;
	m_bHasDisguise = false;
	m_ShowPapersTime = 0;
	m_Team = 1;
	m_iThreatBias = 0;
	m_bFootOnGround_Right = true;
	m_bFootOnGround_Left = true;
	m_bDontDropWeapons = false;

	m_fDamageMultipliers[ 0 ] = 4.0f;
	m_fDamageMultipliers[ 1 ] = 4.0f;
	m_fDamageMultipliers[ 2 ] = 4.0f;
	m_fDamageMultipliers[ 3 ] = 1.0f;
	m_fDamageMultipliers[ 4 ] = 1.0f;
	m_fDamageMultipliers[ 5 ] = 1.0f;
	m_fDamageMultipliers[ 6 ] = 0.9f;
	m_fDamageMultipliers[ 7 ] = 0.8f;
	m_fDamageMultipliers[ 8 ] = 0.8f;
	m_fDamageMultipliers[ 9 ] = 0.8f;
	m_fDamageMultipliers[ 10 ] = 0.8f;
	m_fDamageMultipliers[ 11 ] = 0.6f;
	m_fDamageMultipliers[ 12 ] = 0.6f;
	m_fDamageMultipliers[ 13 ] = 0.6f;
	m_fDamageMultipliers[ 14 ] = 0.6f;
	m_fDamageMultipliers[ 15 ] = 0.5f;
	m_fDamageMultipliers[ 16 ] = 0.5f;
	m_fDamageMultipliers[ 17 ] = 0.5f;
	m_fDamageMultipliers[ 18 ] = 0.5f;

	m_PrevSentient = NULL;
	m_NextSentient = level.m_HeadSentient[ m_Team ];

	if( m_NextSentient )
	{
		m_NextSentient->m_PrevSentient = this;
	}

	level.m_HeadSentient[ m_Team ] = this;

	on_fire = false;
	max_gibs = 0;
	next_bleed_time = 0;
}

Sentient::~Sentient()
{
	if( m_NextSentient )
	{
		m_NextSentient->m_PrevSentient = m_PrevSentient;
	}

	if( m_PrevSentient )
	{
		m_PrevSentient->m_NextSentient = m_NextSentient;
	}
	else
	{
		level.m_HeadSentient[ m_Team ] = m_NextSentient;
	}

	m_PrevSentient = NULL;
	m_NextSentient = NULL;

	DisbandSquadMate( this );

	SentientList.RemoveObject( ( Sentient * )this );
	FreeInventory();

	entflags &= ~EF_SENTIENT;
}


// HACK HACK HACK
void Sentient::UpdateOffsetColor
   (
   Event *ev
   )
   {
   G_SetConstantLight( &edict->s.constantLight, &offset_color[ 0 ], &offset_color[ 1 ], &offset_color[ 2 ], NULL );
   offset_color -= offset_delta;
   offset_time -= FRAMETIME;
   if ( offset_time > 0 )
      {
      PostEvent( EV_Sentient_UpdateOffsetColor, FRAMETIME );
      }
   else
      {
      CancelEventsOfType( EV_Sentient_UpdateOffsetColor );
      edict->s.renderfx &= ~RF_LIGHTOFFSET;
      offset_color[ 0 ] = offset_color[ 1 ] = offset_color[ 2 ] = 0;
      G_SetConstantLight( &edict->s.constantLight, &offset_color[ 0 ], &offset_color[ 1 ], &offset_color[ 2 ], NULL );
      }
   }

void Sentient::SetOffsetColor
   ( 
   float r, 
   float g, 
   float b, 
   float time 
   )

   {
   // kill all pending events
   CancelEventsOfType( EV_Sentient_UpdateOffsetColor );

   offset_color[ 0 ] = r;
   offset_color[ 1 ] = g;
   offset_color[ 2 ] = b;

   G_SetConstantLight( &edict->s.constantLight, &offset_color[ 0 ], &offset_color[ 1 ], &offset_color[ 2 ], NULL );

   // delta is a little less so we don't go below zero
   offset_delta = offset_color * ( FRAMETIME / ( time + ( 0.5f * FRAMETIME ) ) );
   offset_time = time;

   edict->s.renderfx |= RF_LIGHTOFFSET;

   PostEvent( EV_Sentient_UpdateOffsetColor, FRAMETIME );
   }

Vector Sentient::EyePosition
	(
	void
	)

	{
   return origin + eyeposition;
	}

Vector Sentient::GunPosition
	(
	void
	)

{
	Vector vPos;

	if( activeWeaponList[ WEAPON_MAIN ] )
	{
		activeWeaponList[ WEAPON_MAIN ]->GetMuzzlePosition( &vPos );
	}
	else
	{
		vPos = origin;
	}

	return vPos;
}

Vector Sentient::GunTarget
	(
	bool bNoCollision
	)

{
	Vector vPos;

	if( mTargetPos[ 0 ] && mTargetPos[ 1 ] && mTargetPos[ 2 ] )
	{
		AnglesToAxis( angles, orientation );
		vPos = GunPosition() + Vector( orientation[ 0 ] ) * 2048.0f;
	}
	else
	{
		if( G_Random() > mAccuracy )
		{
			float rand = G_Random( 5.0f );

			if( G_Random() <= 0.5f )
			{
				vPos[ 0 ] = rand - 32.0f;
			}
			else
			{
				vPos[ 0 ] = rand + 32.0f;
			}

			rand = G_Random( 5.0f );

			if( G_Random() <= 0.5f )
			{
				vPos[ 1 ] = rand - 32.0f;
			}
			else
			{
				vPos[ 1 ] = rand + 32.0f;
			}

			vPos[ 2 ] = -96.0f - G_Random( 5.0f );
		}

		vPos += mTargetPos;
	}

	return vPos;
}

void Sentient::SetBloodModel
   (
   Event *ev
   )

   {
	str name;
	str cache_name;
	str models_dir = "models/";

	if ( ev->NumArgs() < 1 )
		return;

	blood_model = ev->GetString( 1 );
	cache_name = models_dir + blood_model;
   CacheResource( cache_name.c_str() );

	name = GetBloodSpurtName();
	if ( name.length() )
		{
		cache_name = models_dir + name;
		CacheResource( cache_name.c_str() );
		}

	name = GetBloodSplatName();
	if ( name.length() )
		CacheResource( name.c_str() );

	name = GetGibName();
	if ( name.length() )
		{
		cache_name = models_dir + name;
		CacheResource( cache_name.c_str() );
		}
	}

void Sentient::ChargeWeapon
	(
	Event *ev
	)

{
	firemode_t		mode = FIRE_PRIMARY;
	weaponhand_t	hand = WEAPON_MAIN;

	if( charge_start_time )
	{
		// Charging has already been started, so return
		return;
	}

	if( ev->NumArgs() > 0 )
	{
		hand = WeaponHandNameToNum( ev->GetString( 1 ) );

		if( hand < 0 )
			return;

		if( ev->NumArgs() == 2 )
		{
			mode = WeaponModeNameToNum( ev->GetString( 2 ) );

			if( mode < 0 )
				return;
		}
	}

	ChargeWeapon( hand, mode );
}

void Sentient::ChargeWeapon
	(
	weaponhand_t hand,
	firemode_t mode
	)

{
	Weapon * activeWeapon;

	if( hand > MAX_ACTIVE_WEAPONS )
	{
		warning( "Sentient::ChargeWeapon", "Weapon hand number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n", hand, MAX_ACTIVE_WEAPONS );
		return;
	}

	// start charging the active weapon 
	activeWeapon = activeWeaponList[ ( int )hand ];

	if( ( activeWeapon ) && activeWeapon->ReadyToFire( mode ) && activeWeapon->MuzzleClear() )
	{
		charge_start_time = level.time;

		if( mode == FIRE_PRIMARY )
			activeWeapon->SetWeaponAnim( "charge" );
		else if( mode == FIRE_SECONDARY )
			activeWeapon->SetWeaponAnim( "secondarycharge" );
	}
}

void Sentient::FireWeapon
	(
	Event *ev
	)

{
	firemode_t mode = FIRE_PRIMARY;
	int number = 0;
	str modestring;
	str side;

	if( ev->NumArgs() > 0 )
	{
		side = ev->GetString( 1 );

		if( side.icmp( "mainhand" ) )
		{
			if( !side.icmp( "offhand" ) )
			{
				number = WEAPON_OFFHAND;
			}
			else
			{
				number = atoi( side );
			}
		}
		else
		{
			number = WEAPON_MAIN;

			if( ev->NumArgs() == 2 )
			{
				modestring = ev->GetString( 2 );

				if( !modestring.icmp( "primary" ) )
				{
					mode = FIRE_PRIMARY;
				}
				else if( !modestring.icmp( "secondary" ) )
				{
					mode = FIRE_SECONDARY;
				}
				else
				{
					warning( "Sentient::FireWeapon", "Invalid mode %s\n", modestring.c_str() );
				}
			}
		}
	}

	FireWeapon( number, mode );
}

void Sentient::FireWeapon
	(
	int number,
	firemode_t mode
	)

{
	Weapon *activeWeapon = activeWeaponList[ number ];

	if( activeWeapon )
	{
		if( mode == FIRE_SECONDARY &&
			activeWeapon->GetZoom() &&
			!activeWeapon->GetAutoZoom() &&
			IsSubclassOfPlayer() )
		{
			Player *p = ( Player * )this;
			p->ToggleZoom( activeWeapon->GetZoom() );
		}
		else
		{
			activeWeapon->Fire( mode );
		}
	}
	else
	{
		gi.DPrintf( "No active weapon in slot #: \"%i\"\n", number );
	}
}

void Sentient::StopFireWeapon
	(
	Event *ev
	)

{
	Weapon      *activeWeapon;
	int         number = 0;
	str         side;

	if( ev->NumArgs() > 0 )
	{
		side = ev->GetString( 1 );

		number = WeaponHandNameToNum( side );
	}

	if( ( number > MAX_ACTIVE_WEAPONS ) || ( number < 0 ) )
	{
		warning( "Sentient::StopFireWeapon", "Weapon number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n", number, MAX_ACTIVE_WEAPONS );
		return;
	}

	activeWeapon = activeWeaponList[ number ];

	if( activeWeapon )
	{
		activeWeapon->ForceIdle();
	}
	else
	{
		if( !activeWeapon )
			gi.DPrintf( "No active weapon in slot #: \"%i\"\n", number );
	}
}

void Sentient::ReleaseFireWeapon
	(
	int number,
	firemode_t mode
	)

{
	float charge_time = level.time - charge_start_time;

	charge_start_time = 0;

	if( ( number > MAX_ACTIVE_WEAPONS ) || ( number < 0 ) )
	{
		warning( "Sentient::FireWeapon", "Weapon number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n", number, MAX_ACTIVE_WEAPONS );
		return;
	}
	else
	{
		if( activeWeaponList[ number ] )
		{
			activeWeaponList[ number ]->ReleaseFire( mode, charge_time );
		}
	}
}

void Sentient::ReleaseFireWeapon
	(
	Event *ev
	)

{
	firemode_t mode = FIRE_PRIMARY;
	int number = 0;
	str modestring;
	str side;

	if( ev->NumArgs() > 0 )
	{
		side = ev->GetString( 1 );

		if( side.icmp( "mainhand" ) )
		{
			if( !side.icmp( "offhand" ) )
			{
				number = WEAPON_OFFHAND;
			}
			else
			{
				number = atoi( side );
			}
		}
		else
		{
			number = WEAPON_MAIN;

			if( ev->NumArgs() == 2 )
			{
				modestring = ev->GetString( 2 );

				if( !modestring.icmp( "primary" ) )
				{
					mode = FIRE_PRIMARY;
				}
				else if( !modestring.icmp( "secondary" ) )
				{
					mode = FIRE_SECONDARY;
				}
				else
				{
					warning( "Sentient::ReleaseFireWeapon", "Invalid mode %s\n", modestring.c_str() );
				}
			}
		}
	}

	ReleaseFireWeapon( number, mode );
}

void Sentient::AddItem
	(
	Item *object
	)

{
	inventory.AddObject( object->entnum );
}

void Sentient::RemoveItem
	(
	Item *object
	)

{
	if( !inventory.IndexOfObject( object->entnum ) ) {
		return;
	}

	inventory.RemoveObject( object->entnum );

	if( object->IsSubclassOfWeapon() )
		DeactivateWeapon( ( Weapon * )object );

	// 
	// let the sent know about it
	//
	RemovedItem( object );
}

void Sentient::RemoveWeapons
	(
	void
	)

{
	for( int i = inventory.NumObjects(); i > 0; i-- )
	{
		int entnum = inventory.ObjectAt( i );
		Weapon *item = ( Weapon * )G_GetEntity( entnum );

		if( item->IsSubclassOfWeapon() )
		{
			item->Delete();
		}
	}
}

Weapon *Sentient::GetWeapon
	(
	int index
	)

{
	for( int i = inventory.NumObjects(); i > 0; i-- )
	{
		int entnum = inventory.ObjectAt( i );
		Weapon *item = ( Weapon * )G_GetEntity( entnum );

		if( item->IsSubclassOfWeapon() )
		{
			if( !index )
			{
				return item;
			}

			index--;
		}
	}

	return NULL;
}

Item *Sentient::FindItemByExternalName
	(
	const char *itemname
	)

{
	int	num;
	int	i;
	Item	*item;

	num = inventory.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		assert( item );
		if( !Q_stricmp( item->getName(), itemname ) )
		{
			return item;
		}
	}

	return NULL;
}

Item *Sentient::FindItemByModelname
	(
	const char *mdl
	)

{
	int	num;
	int	i;
	Item	*item;
	str   tmpmdl;

	if( Q_stricmpn( "models/", mdl, 7 ) )
	{
		tmpmdl = "models/";
	}
	tmpmdl += mdl;

	num = inventory.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		assert( item );
		if( !Q_stricmp( item->model, tmpmdl ) )
		{
			return item;
		}
	}

	return NULL;
}

Item *Sentient::FindItemByClassName
	(
	const char *classname
	)

{
	int	num;
	int	i;
	Item	*item;

	num = inventory.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		assert( item );
		if( !Q_stricmp( item->edict->entname, classname ) )
		{
			return item;
		}
	}

	return NULL;
}

Item *Sentient::FindItem
	(
	const char *itemname
	)

{
	Item	*item;

	item = FindItemByExternalName( itemname );
	if( !item )
	{
		item = FindItemByModelname( itemname );
		if( !item )
		{
			item = FindItemByClassName( itemname );
		}
	}
	return item;
}

void Sentient::AttachAllActiveWeapons
	(
	void
	)

{
	int i;

	for( i = 0; i<MAX_ACTIVE_WEAPONS; i++ )
	{
		Weapon *weap = activeWeaponList[ i ];

		if( weap )
			weap->AttachToOwner( ( weaponhand_t )i );
	}

	if( this->isSubclassOf( Player ) )
	{
		Player *player = ( Player * )this;
		player->UpdateWeapons();
	}
}

void Sentient::DetachAllActiveWeapons
	(
	void
	)

{
	int i;

	for( i = 0; i<MAX_ACTIVE_WEAPONS; i++ )
	{
		Weapon *weap = activeWeaponList[ i ];

		if( weap )
			weap->DetachFromOwner();
	}
}

void Sentient::FreeInventory
	(
	void
	)

{
	int	num;
	int	i;
	Item	*item;
	Ammo  *ammo;

	// Detach all Weapons
	DetachAllActiveWeapons();

	// Delete all inventory items ( this includes weapons )
	num = inventory.NumObjects();
	for( i = num; i > 0; i-- )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		delete item;
	}
	inventory.ClearObjectList();

	// Remove all ammo
	num = ammo_inventory.NumObjects();
	for( i = num; i > 0; i-- )
	{
		ammo = ( Ammo * )ammo_inventory.ObjectAt( i );
		delete ammo;
	}
	ammo_inventory.ClearObjectList();

	if( IsSubclassOfPlayer() )
	{
		( ( Player * )this )->InitMaxAmmo();
	}
}

qboolean Sentient::HasItem
	(
	const char *itemname
	)

{
	return ( FindItem( itemname ) != NULL );
}

qboolean Sentient::HasWeaponClass
	(
	int iWeaponClass
	)

{
	int i;
	Weapon *weapon;

	// look up for a weapon class
	for( i = 1; i <= inventory.NumObjects(); i++ )
	{
		weapon = ( Weapon * )G_GetEntity( inventory.ObjectAt( i ) );

		if( weapon->IsSubclassOfWeapon() )
		{
			if( weapon->GetWeaponClass() & iWeaponClass )
			{
				// weapon class found
				return qtrue;
			}
		}
	}

	return qfalse;
}

qboolean Sentient::HasPrimaryWeapon
	(
	void
	)

{
	int i;
	Weapon *weapon;

	// look up for a primary weapon
	for( i = 1; i <= inventory.NumObjects(); i++ )
	{
		weapon = ( Weapon * )G_GetEntity( inventory.ObjectAt( i ) );

		if( weapon->IsSubclassOfWeapon() )
		{
			if( !( weapon->GetWeaponClass() & WEAPON_CLASS_MISC ) && !weapon->IsSecondaryWeapon() )
			{
				// Sentient has a primary weapon
				return qtrue;
			}
		}
	}

	return qfalse;
}

qboolean Sentient::HasSecondaryWeapon
	(
	void
	)

{
	int i;
	Weapon *weapon;

	// look up for a secondary weapon
	for( i = 1; i <= inventory.NumObjects(); i++ )
	{
		weapon = ( Weapon * )G_GetEntity( inventory.ObjectAt( i ) );

		if( weapon->IsSubclassOfWeapon() )
		{
			if( weapon->IsSecondaryWeapon() )
			{
				// Sentient has a secondary weapon
				return qtrue;
			}
		}
	}

	return qfalse;
}

int Sentient::NumWeapons
	(
   void
	)

	{
	int	num;
	int	i;
	Item	*item;
   int   numweaps;

   numweaps = 0;

	num = inventory.NumObjects();
	for( i = 1; i <= num; i++ )
		{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
      if ( checkInheritance( &Weapon::ClassInfo, item->getClassname() ) )
			{
         numweaps++;
			}
		}

   return numweaps;
	}

void Sentient::PutawayWeapon
	(
	Event *ev
	)

{
	Weapon * weapon;
	weaponhand_t hand;
	str   side;

	side = ev->GetString( 1 );

	hand = WeaponHandNameToNum( side );

	if( hand == WEAPON_ERROR )
		return;

	weapon = GetActiveWeapon( hand );

	if( weapon->isSubclassOf( Weapon ) )
	{
		weapon->NewAnim( "putaway" );
	}
}

void Sentient::WeaponCommand
	(
	Event *ev
	)

{
	weaponhand_t   hand;
	Weapon         *weap;
	int            i;

	if( ev->NumArgs() < 2 )
		return;

	hand = WeaponHandNameToNum( ev->GetString( 1 ) );
	weap = GetActiveWeapon( hand );

	if( !weap )
		return;

	Event *e;
	e = new Event( ev->GetToken( 2 ) );

	for( i = 3; i <= ev->NumArgs(); i++ )
		e->AddToken( ev->GetToken( i ) );

	weap->ProcessEvent( e );
}

void Sentient::ChangeWeapon
	(
	Weapon         *weapon,
	weaponhand_t   hand
	)

{
	if( ( hand > MAX_ACTIVE_WEAPONS ) || ( hand < 0 ) )
	{
		warning( "Sentient::ChangeWeapon", "Weapon hand number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n", hand, MAX_ACTIVE_WEAPONS );
		return;
	}

	// Check if weapon is already active in the slot
	if( weapon == activeWeaponList[ hand ] )
		return;

	ActivateWeapon( weapon, hand );
}

void Sentient::DeactivateWeapon
	(
	weaponhand_t hand
	)

{
	int i;

	if( !activeWeaponList[ hand ] )
	{
		warning( "Sentient::DeactivateWeapon", "Tried to deactivate a non-active weapon in hand %d\n", hand );
		return;
	}

	activeWeaponList[ hand ]->AttachToHolster( hand );
	activeWeaponList[ hand ]->SetPutAway( false );
	activeWeaponList[ hand ]->NewAnim( "putaway" );

	// Check the player's inventory and detach any weapons that are already attached to that spot
	for( i = 1; i <= inventory.NumObjects(); i++ )
	{
		Item *item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );

		if( item->IsSubclassOfWeapon() )
		{
			Weapon *weap = ( Weapon * )item;

			if(
				( weap != activeWeaponList[ hand ] ) &&
				( !str::cmp( weap->GetCurrentAttachToTag(), activeWeaponList[ hand ]->GetCurrentAttachToTag() ) )
				)
			{
				weap->DetachFromOwner();
			}
		}
	}

	lastActiveWeapon.weapon = activeWeaponList[ hand ];
	lastActiveWeapon.hand = hand;
	activeWeaponList[ hand ] = NULL;
}

void Sentient::DeactivateWeapon
	(
	Weapon *weapon
	)

{
	int i;

	for( i = 0; i<MAX_ACTIVE_WEAPONS; i++ )
	{
		if( activeWeaponList[ i ] == weapon )
		{
			activeWeaponList[ i ]->DetachFromOwner();
			activeWeaponList[ i ]->SetPutAway( false );
			activeWeaponList[ i ] = NULL;
		}
	}
}

void Sentient::ActivateWeapon
	(
	Weapon         *weapon,
	weaponhand_t   hand
	)

{
	int i;

	if( hand == WEAPON_ERROR )
	{
		gi.DPrintf( "^~^~^ ActivateWeapon: bad weapon hand\n" );
		return;
	}

	activeWeaponList[ hand ] = weapon;
	str holsterTag = weapon->GetHolsterTag();

	// Check the player's inventory and detach any weapons that are currently attached to that tag.
	for( i = 1; i <= inventory.NumObjects(); i++ )
	{
		Item *item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );

		if( item->isSubclassOf( Weapon ) )
		{
			Weapon *weap = ( Weapon * )item;

			if(
				( !str::cmp( holsterTag, weap->GetCurrentAttachToTag() ) )
				)
			{
				weap->DetachFromOwner();
			}
		}
	}
	weapon->AttachToOwner( hand );
	weapon->NewAnim( "raise" );
}

Weapon *Sentient::BestWeapon( Weapon *ignore, qboolean bGetItem, int iIgnoreClass )
{
	Weapon		*next;
	int			n;
	int			j;
	int			bestrank;
	Weapon		*bestweapon;

	n = inventory.NumObjects();

	// Search forewards until we find a weapon
	bestweapon = NULL;
	bestrank = -999999;

	for( j = 1; j <= n; j++ )
	{
		next = ( Weapon * )G_GetEntity( inventory.ObjectAt( j ) );

		assert( next );

		if( ( next != ignore ) && ( ( next->IsSubclassOfWeapon() && !( next->GetWeaponClass() & iIgnoreClass ) ) || ( next->IsSubclassOfItem() && bGetItem ) ) &&
			( next->GetRank() > bestrank ) &&
			( next->HasAmmo( FIRE_PRIMARY ) || next->GetFireType( FIRE_SECONDARY ) == FT_MELEE ) )
		{
			bestweapon = ( Weapon * )next;
			bestrank = bestweapon->GetRank();
		}
	}

	return bestweapon;
}

Weapon *Sentient::WorstWeapon( Weapon *ignore, qboolean bGetItem, int iIgnoreClass )
{
	Weapon		*next;
	int			n;
	int			j;
	int			iWorstRank;
	Weapon		*worstweapon;

	n = inventory.NumObjects();

	worstweapon = NULL;
	iWorstRank = 999999;

	for( j = 1; j <= n; j++ )
	{
		next = ( Weapon * )G_GetEntity( inventory.ObjectAt( j ) );

		assert( next );

		if( ( next != ignore ) && ( ( next->IsSubclassOfWeapon() && !( next->GetWeaponClass() & iIgnoreClass ) ) || ( next->IsSubclassOfWeapon() && bGetItem ) ) &&
			( next->GetRank() < iWorstRank ) &&
			( next->HasAmmo( FIRE_PRIMARY ) || next->GetFireType( FIRE_SECONDARY ) == FT_MELEE ) )
		{
			worstweapon = ( Weapon * )next;
			iWorstRank = worstweapon->GetRank();
		}
	}

	return worstweapon;
}

Weapon *Sentient::NextWeapon
	(
	Weapon *weapon
	)

{
	Item		*item;
	int		i;
	int		n;
	int		weaponorder;
	Weapon	*choice;
	int		choiceorder;
	Weapon	*bestchoice;
	int		bestorder;
	Weapon	*worstchoice;
	int		worstorder;

	if( !inventory.ObjectInList( weapon->entnum ) )
	{
		ScriptError( "NextWeapon", "Weapon not in list" );
	}

	weaponorder = weapon->GetOrder();
	bestchoice = weapon;
	bestorder = 65535;
	worstchoice = weapon;
	worstorder = weaponorder;

	n = inventory.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );

		assert( item );

		if( item->isSubclassOf( Weapon ) )
		{
			choice = ( Weapon * )item;
			if( ( !choice->HasAmmo( FIRE_PRIMARY ) && !choice->GetUseNoAmmo() ) || !choice->AutoChange() )
			{
				continue;
			}

			choiceorder = choice->GetOrder();
			if( ( choiceorder > weaponorder ) && ( choiceorder < bestorder ) )
			{
				bestorder = choiceorder;
				bestchoice = choice;
			}

			if( choiceorder < worstorder )
			{
				worstorder = choiceorder;
				worstchoice = choice;
			}
		}
	}

	if( bestchoice == weapon )
	{
		return worstchoice;
	}

	return bestchoice;
}

Weapon *Sentient::PreviousWeapon
	(
	Weapon *weapon
	)

{
	Item		*item;
	int		i;
	int		n;
	int		weaponorder;
	Weapon	*choice;
	int		choiceorder;
	Weapon	*bestchoice;
	int		bestorder;
	Weapon	*worstchoice;
	int		worstorder;

	if( !inventory.ObjectInList( weapon->entnum ) )
	{
		ScriptError( "PreviousWeapon", "Weapon not in list" );
	}

	weaponorder = weapon->GetOrder();
	bestchoice = weapon;
	bestorder = -65535;
	worstchoice = weapon;
	worstorder = weaponorder;

	n = inventory.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );

		assert( item );

		if( item->isSubclassOf( Weapon ) )
		{
			choice = ( Weapon * )item;
			if( ( !choice->HasAmmo( FIRE_PRIMARY ) && !choice->GetUseNoAmmo() ) || !choice->AutoChange() )
			{
				continue;
			}

			choiceorder = choice->GetOrder();
			if( ( choiceorder < weaponorder ) && ( choiceorder > bestorder ) )
			{
				bestorder = choiceorder;
				bestchoice = choice;
			}

			if( choiceorder > worstorder )
			{
				worstorder = choiceorder;
				worstchoice = choice;
			}
		}
	}

	if( bestchoice == weapon )
	{
		return worstchoice;
	}

	return bestchoice;
}

Weapon *Sentient::GetActiveWeapon
   (
   weaponhand_t hand
   )

   {
   if ( ( hand > MAX_ACTIVE_WEAPONS ) || ( hand < 0 ) )
      {
      warning( "Sentient::GetActiveWeapon", "Weapon hand number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n", hand, MAX_ACTIVE_WEAPONS );
      return NULL;
      }
   else
      {
      return activeWeaponList[hand];
      }
   }

qboolean Sentient::IsActiveWeapon
   (
   Weapon *weapon
   )

   {
   int i;

   for( i=0; i<MAX_ACTIVE_WEAPONS; i++ )
      {
      Weapon *activeWeap = activeWeaponList[i];

      if ( activeWeap == weapon )
         return true;
      }

   return false;
   }

void Sentient::EventGiveTargetname
   (
   Event *ev
   )

   {
   int         i;
   ConSimple	*tlist;
   str         name;
   const char *ptr;
   qboolean    found;

   name = ev->GetString( 1 );

   ptr = name.c_str();

   // skip over the $
   ptr++;

   found = qfalse;

   str sName = ptr;
   tlist = world->GetTargetList( sName );
   for ( i = 1; i <= tlist->NumObjects(); i++ )
      {
      Entity * ent;

      ent = ( Entity * )tlist->ObjectAt( i ).Pointer();
		assert( ent );

		if ( ent->isSubclassOf( Item ) )
			{
         Item *item;

         item = ( Item * )ent;
         item->SetOwner( this );
         item->ProcessPendingEvents();
         AddItem( item );
         found = qtrue;
         }
      }

   if ( !found )
      {
      ScriptError( "Could not give item with targetname %s to this sentient.\n", name.c_str() );
      }
   }

Item *Sentient::giveItem
	(
	str itemname,
	int amount
	)

{
	ClassDef	*cls;
	Item     *item;

	item = FindItem( itemname );
	if( item )
	{
		item->Add( amount );
		return item;
	}
	else
	{
		qboolean set_the_model = qfalse;

		// we don't have it, so lets try to resolve the item name
		// first lets see if it is a registered class name
		cls = getClass( itemname );
		if( !cls )
		{
			SpawnArgs args;

			// if that didn't work lets try to resolve it as a model
			args.setArg( "model", itemname );

			cls = args.getClassDef();
			if( !cls )
			{
				gi.DPrintf( "No item called '%s'\n", itemname.c_str() );
				return NULL;
			}
			set_the_model = qtrue;
		}
		assert( cls );
		item = ( Item * )cls->newInstance();

		if( !item )
		{
			gi.DPrintf( "Could not spawn an item called '%s'\n", itemname.c_str() );
			return NULL;
		}

		if( !item->isSubclassOf( Item ) )
		{
			gi.DPrintf( "Could not spawn an item called '%s'\n", itemname.c_str() );
			delete item;
			return NULL;
		}

		if( set_the_model )
		{
			// Set the model
			item->setModel( itemname );
		}

		item->SetOwner( this );
		item->ProcessPendingEvents();
		item->setAmount( amount );

		AddItem( item );

		if( item->isSubclassOf( Weapon ) )
		{
			// Post an event to give the ammo to the sentient
			Event *ev1;

			ev1 = new Event( EV_Weapon_GiveStartingAmmo );
			ev1->AddEntity( this );
			item->PostEvent( ev1, 0 );
		}

		return item;
	}
	return NULL;
}

void Sentient::takeItem
	(
	const char *name
	)

{
	Item * item;

	item = FindItem( name );
	if( item )
	{
		gi.DPrintf( "Taking item %s away from player\n", item->getName().c_str() );

		item->PostEvent( EV_Remove, 0 );
		return;
	}

	Ammo *ammo;
	ammo = FindAmmoByName( name );
	if( ammo )
	{
		gi.DPrintf( "Taking ammo %s away from player\n", name );

		ammo->setAmount( 0 );
	}
}

void Sentient::takeAmmoType
	(
	const char *name
	)

{
	Ammo *ammo;

	ammo = FindAmmoByName( name );
	if( ammo )
	{
		gi.DPrintf( "Taking ammo %s away from player\n", name );

		ammo->setAmount( 0 );
	}
}

void Sentient::useWeapon
(
const char		*weaponname,
weaponhand_t	hand
)

{
	Weapon *weapon;

	assert( weaponname );

	if( !weaponname )
	{
		warning( "Sentient::useWeapon", "weaponname is NULL\n" );
		return;
	}

	// Find the item in the sentient's inventory
	weapon = ( Weapon * )FindItem( weaponname );

	// If it exists, then make the change to the slot number specified
	if( weapon )
	{
		useWeapon( weapon, hand );
	}
}


void Sentient::useWeapon
	(
	Weapon			*weapon,
	weaponhand_t	hand
	)

{
	assert( weapon );

	if( !weapon )
	{
		warning( "Sentient::useWeapon", "Null weapon used.\n" );
		return;
	}

	if( newActiveWeapon.weapon )
	{
		newActiveWeapon.weapon = weapon;
		newActiveWeapon.hand = hand;
		return;
	}

	if( !weapon->HasAmmo( FIRE_PRIMARY ) && !weapon->GetUseNoAmmo() )
	{
		return;
	}

	if( activeWeaponList[ WEAPON_OFFHAND ] )
	{
		activeWeaponList[ WEAPON_OFFHAND ]->PutAway();
	}

	if( activeWeaponList[ WEAPON_MAIN ] && activeWeaponList[ WEAPON_MAIN ] != weapon )
	{
		activeWeaponList[ WEAPON_MAIN ]->PutAway();
	}

	newActiveWeapon.weapon = weapon;
	newActiveWeapon.hand = hand;

	//ChangeWeapon( weapon, hand );
}

void Sentient::EventTake
	(
	Event *ev
	)

{
	takeItem( ev->GetString( 1 ) );
}

void Sentient::EventFreeInventory
	(
	Event *ev
	)

{
	FreeInventory();
}

void Sentient::EventGiveAmmo
	(
	Event *ev
	)

	{
   int			amount,maxamount=-1;
	const char	*type;

	type        = ev->GetString( 1 );
   amount      = ev->GetInteger( 2 );

   if ( ev->NumArgs() == 3 )
      maxamount   = ev->GetInteger( 3 );

   GiveAmmo( type, amount, maxamount );
   }

void Sentient::EventGiveItem
   (
   Event *ev
   )

   {
	const char	*type;
   float       amount;

	type     = ev->GetString( 1 );
   if ( ev->NumArgs() > 1 )
      amount   = ev->GetInteger( 2 );
   else
      amount = 1;

   giveItem( type, amount );
   }

qboolean Sentient::DoGib
   (
   int meansofdeath,
   Entity *inflictor
   )

   {
   if ( !com_blood->integer )
      {
      return false;
      }

   if (
       ( meansofdeath == MOD_TELEFRAG ) ||
       ( meansofdeath == MOD_LAVA )
      )
      {
      return true;
      }

   if ( health > -75 )
      {
      return false;
      }

   // Impact and Crush < -75 health
   if ( ( meansofdeath == MOD_IMPACT ) || ( meansofdeath == MOD_CRUSH ) )
      {
      return true;
      }

   return false;
   }

void Sentient::SpawnEffect
   (
   str modelname,
   Vector pos
   )

   {
   Animate *block;

   block = new Animate;
   block->setModel( modelname );
   block->setOrigin( pos );
   block->setSolidType( SOLID_NOT );
   block->setMoveType( MOVETYPE_NONE );
   block->NewAnim( "idle" );
   block->PostEvent( EV_Remove, 1 );
   }

#define WATER_CONVERSION_FACTOR 1.0f
void Sentient::ArmorDamage 
	(
	Event *ev
	)

{
	Entity	   *inflictor;
	Sentient   *attacker;
	float		damage;
	Vector		momentum;
	Vector      position;
	Vector      normal;
	Vector      direction;
	Event		*event;
	int			dflags;
	int			meansofdeath;
	int			knockback;
	int			location;

/*
	qboolean	blocked;
	float		damage_red;
	float		damage_green;
	float		damage_time;
	qboolean	set_means_of_death;
*/

	static bool tmp = false;
	static cvar_t *AIDamageMult = NULL;

	if( !tmp )
	{
		tmp = true;
		AIDamageMult = gi.Cvar_Get( "g_aiDamageMult", "1.0", 0 );
	}

	if( IsDead() ) {
		return;
	}

	attacker = ( Sentient * )ev->GetEntity( 1 );
	damage = ev->GetFloat( 2 );
	inflictor = ev->GetEntity( 3 );
	position = ev->GetVector( 4 );
	direction = ev->GetVector( 5 );
	normal = ev->GetVector( 6 );
	knockback = ev->GetInteger( 7 );
	dflags = ev->GetInteger( 8 );
	meansofdeath = ev->GetInteger( 9 );
	location = CheckHitLocation( ev->GetInteger( 10 ) );

	if( location == -2 ) {
		return;
	}

	if( ( takedamage == DAMAGE_NO ) || ( movetype == MOVETYPE_NOCLIP ) )
	{
		return;
	}

	if( client && !g_gametype->integer || location < 0 || location > 18 )
	{
		if( attacker && attacker->IsSubclassOfActor() )
		{
			damage *= AIDamageMult->value;
		}
	}
	else
	{
		damage *= m_fDamageMultipliers[ location ];
	}

	// See if sentient is immune to this type of damage
	if( Immune( meansofdeath ) )
	{
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
	if( !( dflags & DAMAGE_NO_KNOCKBACK ) )
	{
		if( ( knockback ) &&
			( movetype != MOVETYPE_NONE ) &&
			( movetype != MOVETYPE_STATIONARY ) &&
			( movetype != MOVETYPE_BOUNCE ) &&
			( movetype != MOVETYPE_PUSH ) &&
			( movetype != MOVETYPE_STOP ) )
		{
			float	m;
			Event *immunity_event;

			if( mass < 50 )
				m = 50;
			else
				m = mass;

			direction.normalize();
			if( isClient() && ( attacker == this ) && g_gametype->integer )
				momentum = direction * ( 1700.0f * ( float )knockback / m ); // the rocket jump hack...
			else
				momentum = direction * ( 500.0f * ( float )knockback / m );

			if( dflags & DAMAGE_BULLET )
			{
				// Clip the z velocity for bullet weapons
				if( momentum.z > 75 )
					momentum.z = 75;
			}
			velocity += momentum;

			// Make this sentient vulnerable to falling damage now

			if( Immune( MOD_FALLING ) )
			{
				immunity_event = new Event( EV_Entity_RemoveImmunity );
				immunity_event->AddString( "falling" );
				ProcessEvent( immunity_event );
			}
		}
	}

	if( g_debugdamage->integer )
		G_DebugDamage( damage, this, attacker, inflictor );

	if( !( flags & FL_GODMODE ) &&
		(
		( g_gametype->integer )
		|| !( attacker )
		|| ( attacker ) == this
		|| !( attacker->IsSubclassOfPlayer() )
		|| !( attacker->IsTeamMate( this ) ) )
		)
	{
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

	if( health <= 0 )
	{
		// See if we can kill this actor or not

		if( this->IsSubclassOfActor() )
		{
			Actor *act = ( Actor * )this;

			if( act->IsImmortal() )
				health = 1;
		}
	}

	if( health > 0 )
	{
		// Send pain event
		event = new Event( EV_Pain );
		event->AddEntity( attacker );
		event->AddFloat( damage );
		event->AddEntity( inflictor );
		event->AddVector( position );
		event->AddVector( direction );
		event->AddVector( normal );
		event->AddInteger( knockback );
		event->AddInteger( dflags );
		event->AddInteger( meansofdeath );
		event->AddInteger( location );

		ProcessEvent( event );
	}

	if( health < 0.1 )
	{
		// Make sure health is now 0

		health = 0;

		event = new Event( EV_Killed );
		event->AddEntity( attacker );
		event->AddFloat( damage );
		event->AddEntity( inflictor );
		event->AddVector( position );
		event->AddVector( direction );
		event->AddVector( normal );
		event->AddInteger( knockback );
		event->AddInteger( dflags );
		event->AddInteger( meansofdeath );
		event->AddInteger( location );

		ProcessEvent( event );
	}

	return;
}

qboolean Sentient::CanBlock
	( 
	int meansofdeath, 
	qboolean full_block
	)
	{

	// Check to see what a full block can't even block

	switch ( meansofdeath )
		{
		case MOD_TELEFRAG :
		case MOD_SLIME :
		case MOD_LAVA :
		case MOD_FALLING :
		case MOD_IMPALE :
		case MOD_ON_FIRE :
		case MOD_ELECTRICWATER :
			return false;
		}

	// Full blocks block everything else

	if ( full_block )
		return true;

	// Check to see what a small block can't block

	switch ( meansofdeath )
		{
		case MOD_FIRE :
		case MOD_CRUSH_EVERY_FRAME :
			return false;
		}

	// Everything else is blocked

	return true;
	}


void Sentient::AddBloodSpurt
	(
	Vector direction
	)
	{
	Entity *blood;
	Vector dir;
	Event *event;
	str blood_splat_name;
	float blood_splat_size;
	float length;
	trace_t trace;
	float scale;


	if ( !com_blood->integer )
		return;

	next_bleed_time = level.time + .5;

	// Calculate a good scale for the blood

	if ( mass < 50 )
		scale = .5;
	else if ( mass > 300 )
		scale = 1.5;
	else if ( mass >= 200 )
		scale = mass / 200.0;
	else
		scale = .5 + (mass - 50 ) / 300;

	// Add blood spurt

	blood = new Animate;
	blood->setModel( blood_model );

	dir[0] = -direction[0];
	dir[1] = -direction[1];
	dir[2] = -direction[2];
	blood->angles = dir.toAngles();
	blood->setAngles( blood->angles );

	blood->setOrigin( centroid );
	blood->origin.copyTo( blood->edict->s.origin2 );
	blood->setSolidType( SOLID_NOT );
	blood->setScale( scale );

	event = new Event( EV_Remove );
	blood->PostEvent( event, 1 );

	// Add blood splats near feet

	blood_splat_name = GetBloodSplatName();
	blood_splat_size = GetBloodSplatSize();

	if ( blood_splat_name.length() && G_Random() < 0.5 )
		{
		dir = origin - centroid;
		dir.z -= 50;
		dir.x += G_CRandom( 20 );
		dir.y += G_CRandom( 20 );

		length = dir.length();

		dir.normalize();

		dir = dir * ( length + 10 );

		trace = G_Trace( centroid, vec_zero, vec_zero, centroid + dir, NULL, MASK_DEADSOLID, false, "AddBloodSpurt" );

		if ( trace.fraction < 1 )
			{
			Decal *decal = new Decal;
			decal->setShader( blood_splat_name );
			decal->setOrigin( Vector( trace.endpos ) + ( Vector( trace.plane.normal ) * 0.2f ) );
			decal->setDirection( trace.plane.normal );
			decal->setOrientation( "random" );
			decal->setRadius( blood_splat_size + G_Random( blood_splat_size ) );
			}
		}
	}

qboolean Sentient::ShouldBleed
	(
	int meansofdeath,
	qboolean dead
	)
	{
	// Make sure we have a blood model

	if ( !blood_model.length() )
		return false;

	// See if we can bleed now based on means of death

	switch ( meansofdeath )
		{
		// Sometimes bleed (based on time)

		case MOD_BULLET :
		case MOD_CRUSH_EVERY_FRAME :
		case MOD_ELECTRICWATER :

			if ( next_bleed_time > level.time )
				return false;

			break;

		// Sometimes bleed (based on chance)

		case MOD_SHOTGUN :

			if ( G_Random() > 0.1 )
				return false;

			break;

		// Never bleed

		case MOD_SLIME :
		case MOD_LAVA :
		case MOD_FIRE :
		case MOD_FLASHBANG :
		case MOD_ON_FIRE :
		case MOD_FALLING :
			return false;
		}

	// Always bleed by default

	return true;
	}

// ShouldGib assumes that ShouldBleed has already been called

qboolean Sentient::ShouldGib
	(
	int meansofdeath,
	float damage
	)

	{
	// See if we can gib based on means of death

	switch ( meansofdeath )
		{
		// Always gib

		case MOD_CRUSH_EVERY_FRAME :

			return true;

			break;

		// Sometimes gib

		case MOD_BULLET :

			if ( G_Random( 100 ) < damage * 10 )
				return true;

			break;

		case MOD_BEAM :

			if ( G_Random( 100 ) < damage * 5 )
				return true;

			break;

		// Never gib

		case MOD_SLIME :
		case MOD_LAVA :
		case MOD_FIRE :
		case MOD_FLASHBANG :
		case MOD_ON_FIRE :
		case MOD_FALLING :
		case MOD_ELECTRICWATER :
			return false;
		}

	// Default is random based on how much damage done

	if ( G_Random( 100 ) < damage * 2 )
		return true;

	return false;
	}

str Sentient::GetBloodSpurtName
	(
	void
	)
	{
	str blood_spurt_name;

	if ( blood_model == "fx_bspurt.tik" )
		blood_spurt_name = "fx_bspurt2.tik";
	else if ( blood_model == "fx_gspurt.tik" )
		blood_spurt_name = "fx_gspurt2.tik";
	else if ( blood_model == "fx_bspurt_blue.tik" )
		blood_spurt_name = "fx_bspurt2_blue.tik";

	return blood_spurt_name;
	}

str Sentient::GetBloodSplatName
	(
	void
	)
	{
	str blood_splat_name;

	if ( blood_model == "fx_bspurt.tik" )
		blood_splat_name = "bloodsplat.spr";
	else if ( blood_model == "fx_gspurt.tik" )
		blood_splat_name = "greensplat.spr";
	else if ( blood_model == "fx_bspurt_blue.tik" )
		blood_splat_name = "bluesplat.spr";

	return blood_splat_name;
	}

float Sentient::GetBloodSplatSize
	(
	void
	)
	{
	float m;

	m = mass;

	if ( m < 50 )
		m = 50;
	else if ( m > 250 )
		m = 250;

	return( 10 + (m - 50) / 200 * 6 );
	}

str Sentient::GetGibName
	(
	void
	)
	{
	str gib_name;

	if ( blood_model == "fx_bspurt.tik" )
		gib_name = "fx_rgib";
	else if ( blood_model == "fx_gspurt.tik" )
		gib_name = "fx_ggib";

	return gib_name;
	}

int Sentient::NumInventoryItems
   (
   void
   )

   {

   return inventory.NumObjects();

   }

Item *Sentient::NextItem
	(
	Item *item
	)

	{
	Item		*next_item;
	int		i;
	int		n;
   qboolean item_found = false;

   if ( !item )
      {
      item_found = true;
      }
   else if ( !inventory.ObjectInList( item->entnum ) )
		{
		error( "NextItem", "Item not in list" );
		}

	n = inventory.NumObjects();

	for( i = 1; i <= n; i++ )
		{
		next_item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
      assert( next_item );

 		if ( next_item->isSubclassOf( InventoryItem ) && item_found )
         return next_item;

      if ( next_item == item )
         item_found = true;
      }

   return NULL;
   }

Item *Sentient::PrevItem
	(
	Item *item
	)

	{
	Item		*prev_item;
	int		i;
	int		n;
   qboolean item_found = false;

   if ( !item )
      {
      item_found = true;
      }
   else if ( !inventory.ObjectInList( item->entnum ) )
		{
		error( "NextItem", "Item not in list" );
		}

	n = inventory.NumObjects();

	for( i = n; i >= 1; i-- )
		{
		prev_item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
      assert( prev_item );

 		if ( prev_item->isSubclassOf( InventoryItem ) && item_found)
         return prev_item;

      if ( prev_item == item )
         item_found = true;
      }

   return NULL;
   }

void Sentient::DropInventoryItems
	(
	void
	)

{
	int   num;
	int   i;
	Item  *item;

	// Drop any inventory items
	num = inventory.NumObjects();
	for( i = num; i >= 1; i-- )
	{
		item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );
		if( item->isSubclassOf( InventoryItem ) )
		{
			if( m_bDontDropWeapons && item->IsSubclassOfWeapon() )
			{
				item->Delete();
			}
			else
			{
				item->Drop();
			}
		}
	}
}

qboolean Sentient::PowerupActive
   (
   void
   )

   {
   if ( poweruptype && this->client )
      {
      gi.SendServerCommand( edict-g_entities, "print \"You are already using a powerup\n\"" );
      }

   return poweruptype;
   }

void Sentient::setModel
	(
	const char *mdl
	)

	{
   // Rebind all active weapons

   DetachAllActiveWeapons();
   Entity::setModel( mdl );
   AttachAllActiveWeapons();
	}

void Sentient::TurnOffShadow
   (
   Event *ev
   )

   {
   edict->s.renderfx &= ~RF_SHADOW;
   }

void Sentient::TurnOnShadow
   (
   Event *ev
   )

   {
   edict->s.renderfx |= RF_SHADOW;
   }

void Sentient::Archive
	(
	Archiver &arc
	)

{
	int i;
	int num;


	Animate::Archive( arc );

	arc.ArchiveSafePointer( &m_pNextSquadMate );
	arc.ArchiveSafePointer( &m_pPrevSquadMate );

	inventory.Archive( arc );
	if( arc.Saving() )
	{
		num = ammo_inventory.NumObjects();
	}
	else
	{
		ammo_inventory.ClearObjectList();
	}
	arc.ArchiveInteger( &num );
	for( i = 1; i <= num; i++ )
	{
		Ammo * ptr;

		if( arc.Loading() )
		{
			ptr = new Ammo;
			ammo_inventory.AddObject( ptr );
		}
		else
		{
			ptr = ammo_inventory.ObjectAt( i );
		}
		arc.ArchiveObject( ptr );
	}

	arc.ArchiveFloat( &LMRF );

	arc.ArchiveInteger( &poweruptype );
	arc.ArchiveInteger( &poweruptimer );

	arc.ArchiveVector( &offset_color );
	arc.ArchiveVector( &offset_delta );
	arc.ArchiveFloat( &offset_time );
	arc.ArchiveFloat( &charge_start_time );
	arc.ArchiveString( &blood_model );

	for( i = 0; i < MAX_ACTIVE_WEAPONS; i++ )
	{
		arc.ArchiveSafePointer( &activeWeaponList[ i ] );
	}

	newActiveWeapon.Archive( arc );
	arc.ArchiveSafePointer( &holsteredWeapon );
	arc.ArchiveBool( &weapons_holstered_by_code );
	lastActiveWeapon.Archive( arc );

	for( int i = 0; i < MAX_DAMAGE_MULTIPLIERS; i++ )
	{
		arc.ArchiveFloat( &m_fDamageMultipliers[ i ] );
	}


	arc.ArchiveSafePointer( &m_pVehicle );
	arc.ArchiveSafePointer( &m_pTurret );
	arc.ArchiveSafePointer( &m_pLadder );

	arc.ArchiveString( &m_sHelmetSurface1 );
	arc.ArchiveString( &m_sHelmetSurface2 );
	arc.ArchiveString( &m_sHelmetTiki );

	arc.ArchiveFloat( &m_fHelmetSpeed );

	arc.ArchiveVector( &gunoffset );
	arc.ArchiveVector( &eyeposition );
	arc.ArchiveInteger( &viewheight );
	arc.ArchiveVector( &m_vViewVariation );
	arc.ArchiveInteger( &means_of_death );

	arc.ArchiveBool( &on_fire );
	arc.ArchiveFloat( &on_fire_stop_time );
	arc.ArchiveFloat( &next_catch_on_fire_time );
	arc.ArchiveInteger( &on_fire_tagnums[ 0 ] );
	arc.ArchiveInteger( &on_fire_tagnums[ 1 ] );
	arc.ArchiveInteger( &on_fire_tagnums[ 2 ] );
	arc.ArchiveSafePointer( &fire_owner );

	arc.ArchiveBool( &attack_blocked );
	arc.ArchiveFloat( &attack_blocked_time );

	arc.ArchiveFloat( &max_mouth_angle );
	arc.ArchiveInteger( &max_gibs );

	arc.ArchiveFloat( &next_bleed_time );

	arc.ArchiveBool( &m_bFootOnGround_Right );
	arc.ArchiveBool( &m_bFootOnGround_Left );

	arc.ArchiveObjectPointer( ( Class ** )&m_NextSentient );
	arc.ArchiveObjectPointer( ( Class ** )&m_PrevSentient );

	arc.ArchiveVector( &mTargetPos );
	arc.ArchiveFloat( &mAccuracy );

	arc.ArchiveInteger( &m_Team );
	arc.ArchiveInteger( &m_iAttackerCount );

	arc.ArchiveSafePointer( &m_pLastAttacker );
	arc.ArchiveSafePointer( &m_Enemy );

	arc.ArchiveFloat( &m_fPlayerSightLevel );

	arc.ArchiveBool( &m_bIsDisguised );
	arc.ArchiveBool( &m_bHasDisguise );

	arc.ArchiveInteger( &m_ShowPapersTime );
	arc.ArchiveInteger( &m_iLastHitTime );
	arc.ArchiveInteger( &m_iThreatBias );

	arc.ArchiveBool( &m_bDontDropWeapons );

	if( arc.Loading() )
	{
		if( WeaponsOut() )
		{
			Holster( true );
		}
	}
}

void Sentient::ArchivePersistantData
   (
	Archiver &arc
   )

   {
   int i;
   int num;

   // archive the inventory
   if ( arc.Saving() )
      {
      // count up the total number
      num = inventory.NumObjects();
      }
   else
      {
      inventory.ClearObjectList();
      }
   // archive the number
   arc.ArchiveInteger( &num );
   // archive each item
	for( i = 1; i <= num; i++ )
		{
      str name;
      int amount;
      Item * item;

      if ( arc.Saving() )
         {
         Entity * ent;

         ent = G_GetEntity( inventory.ObjectAt( i ) );
         if ( ent && ent->isSubclassOf( Item ) )
            {
            item = ( Item * )ent;
            name = item->model;
            amount = item->getAmount();
            }
         else
            {
            error( "ArchivePersistantData", "Non Item in inventory\n" );
            }
         }
      arc.ArchiveString( &name );
      arc.ArchiveInteger( &amount );
      if ( arc.Loading() )
         {
         item = giveItem( name, amount );
			item->CancelEventsOfType( EV_Weapon_GiveStartingAmmo );
         }
      }

   // archive the ammo inventory
   if ( arc.Saving() )
      {
      // count up the total number
      num = ammo_inventory.NumObjects();
      }
   else
      {
      ammo_inventory.ClearObjectList();
      }
   // archive the number
   arc.ArchiveInteger( &num );
   // archive each item
	for( i = 1; i <= num; i++ )
		{
      str name;
      int amount;
		int maxamount;
      Ammo * ptr;

      if ( arc.Saving() )
         {
         ptr = ammo_inventory.ObjectAt( i );
         name = ptr->getName();
         amount = ptr->getAmount();
			maxamount = ptr->getMaxAmount();
         }
      arc.ArchiveString( &name );
      arc.ArchiveInteger( &amount );
		arc.ArchiveInteger( &maxamount );
      if ( arc.Loading() )
         {
         GiveAmmo( name, amount, maxamount );
         }
      }

   for( i = 0; i < MAX_ACTIVE_WEAPONS; i++ )
      {
      str name;
      if ( arc.Saving() )
         {
         if ( activeWeaponList[ i ] )
            {
            name = activeWeaponList[ i ]->getName();
            }
         else
            {
            name = "none";
            }
         }
      arc.ArchiveString( &name );
      if ( arc.Loading() )
         {
         if ( name != "none" )
            {
            Weapon * weapon;
	         weapon = ( Weapon * )FindItem( name );
            if ( weapon )
               {
               ChangeWeapon( weapon, ( weaponhand_t )i );
               }
            }
         }
      }

   arc.ArchiveFloat( &health );
   arc.ArchiveFloat( &max_health );
   }


void Sentient::DoubleArmor
   (
   void
   )

   {
   int i,n;

   n = inventory.NumObjects();

	for( i = 1; i <= n; i++ )
		{
		Item  *item;
      item = ( Item * )G_GetEntity( inventory.ObjectAt( i ) );

 		if ( item->isSubclassOf( Armor ) )
         item->setAmount( item->getAmount() * 2 );
      }
   }

void Sentient::WeaponKnockedFromHands
   (
   void
   )

   {
	str realname;
	AliasListNode_t *ret;

   realname = GetRandomAlias( "snd_lostweapon", &ret );
	if ( realname.length() > 1 )
      {
		Sound( realname.c_str() , CHAN_VOICE );
      }
   else
      {
		Sound( "snd_pain", CHAN_VOICE );
      }
   }

Ammo *Sentient::FindAmmoByName
   (
   str name
   )

   {
   int count, i;

   count = ammo_inventory.NumObjects();

   for ( i=1; i<=count; i++ )
      {
      Ammo *ammo = ammo_inventory.ObjectAt( i );
      if ( name == ammo->getName() )
         {
         return ammo;
         }     
      }
   return NULL;
   }

int Sentient::AmmoIndex
   (
   str type
   )

   {
   Ammo *ammo;

   ammo = FindAmmoByName( type );

   if ( ammo )
      return ammo->getIndex();
   else
      return 0;
   }

int Sentient::AmmoCount
	(
	str type
	)

{
	Ammo *ammo;

	if( !type.length() )
		return 0;

	ammo = FindAmmoByName( type );

	if( ammo )
		return ammo->getAmount();
	else
		return 0;
}

int Sentient::MaxAmmoCount
	(
	str type
	)

{
	Ammo *ammo;

	ammo = FindAmmoByName( type );

	if( ammo )
		return ammo->getMaxAmount();
	else
		return 0;
}

void Sentient::GiveAmmo
	(
	str type,
	int amount,
	int maxamount
	)

{
	Ammo *ammo;

	ammo = FindAmmoByName( type );

	if( ammo )
	{
		if( maxamount >= 0 )
			ammo->setMaxAmount( maxamount );

		// Add amount to current amount
		ammo->setAmount( ammo->getAmount() + amount );
	}
	else
	{
		// Create a new inventory entry with this name
		ammo = new Ammo;

		if( maxamount >= 0 )
			ammo->setMaxAmount( maxamount );

		ammo->setAmount( amount );

		ammo->setName( type );
		ammo_inventory.AddObject( ammo );
	}
}

int Sentient::UseAmmo
   (
   str type,
   int amount
   )

   {
   int count, i;
   
   count = ammo_inventory.NumObjects();

   for ( i=1; i<=count; i++ )
      {
      Ammo *ammo = ammo_inventory.ObjectAt( i );
      if ( type == ammo->getName() )
         {
         int ammo_amount = ammo->getAmount();

         // Less ammo than what we specified to use
         if ( ammo_amount < amount )
            {
            ammo->setAmount( 0 );
            AmmoAmountChanged( ammo );
            return ammo_amount;
            }
         else
            {
            ammo->setAmount( ammo->getAmount() - amount );
            AmmoAmountChanged( ammo );
            return amount;
            }
         }     
      }
   return 0;
   }

void Sentient::AmmoAmountInClipChanged
	(
	str type,
	int amount_in_clip
	)

{
	int count, i;

	count = ammo_inventory.NumObjects();

	for( i = 1; i <= count; i++ )
	{
		Ammo *ammo = ammo_inventory.ObjectAt( i );
		if( type == ammo->getName() )
		{
			AmmoAmountChanged( ammo, amount_in_clip );
		}
	}
}


void Sentient::JumpXY
   (
   Event *ev
   )

   {
   float forwardmove;
   float sidemove;
   float distance;
   float time;
   float speed;
	Vector yaw_forward;
	Vector yaw_left;

   forwardmove = ev->GetFloat( 1 );
   sidemove    = ev->GetFloat( 2 );
   speed       = ev->GetFloat( 3 );

	Vector( 0, angles.y, 0 ).AngleVectors( &yaw_forward, &yaw_left );

   velocity = yaw_forward * forwardmove - yaw_left * sidemove;
   distance = velocity.length();
   velocity *= speed / distance;
   time = distance / speed;
   velocity[ 2 ] = sv_gravity->integer * time * 0.5f;
   }

void Sentient::MeleeAttackStart
   (
   Event *ev
   )

	{
	in_melee_attack = true;
	}

void Sentient::MeleeAttackEnd
   (
   Event *ev
   )

	{
	in_melee_attack = false;
	}

void Sentient::BlockStart
   (
   Event *ev
   )

	{
	in_block = true;
	}

void Sentient::BlockEnd
   (
   Event *ev
   )

	{
	in_block = false;
	}

void Sentient::StunStart
   (
   Event *ev
   )

	{
	in_stun = true;
	}

void Sentient::StunEnd
   (
   Event *ev
   )

	{
	in_stun = false;
	}

void Sentient::ListInventory
   (
   void
   )

   {
   int i,count;

   // Display normal inventory
   count = inventory.NumObjects();

   gi.Printf( "'Name' : 'Amount'\n" );

   for ( i=1; i<=count; i++ )
      {
      int entnum = inventory.ObjectAt( i );
      Item *item = ( Item * )G_GetEntity( entnum );
      gi.Printf( "'%s' : '%d'\n", item->getName().c_str(), item->getAmount() );
      }

   // Display ammo inventory
   count = ammo_inventory.NumObjects();

   for ( i=1; i<=count; i++ )
      {
      Ammo *ammo = ammo_inventory.ObjectAt( i );
      gi.Printf( "'%s' : '%d'\n", ammo->getName().c_str(), ammo->getAmount() );
      }
   }

void Sentient::SetAttackBlocked
   (
   bool blocked
   )

   {
   attack_blocked      = blocked;
   attack_blocked_time = level.time;
   }

void Sentient::SetMaxMouthAngle
	(
	Event *ev
	)
	{
   max_mouth_angle = ev->GetFloat( 1 );
   }

void Sentient::TryLightOnFire
	(
	int meansofdeath,
	Entity *attacker
	)

{
	gi.Printf( "Sentient::TryLightOnFire not implemented. Needs fixed" );
}

void Sentient::OnFire
	(
	Event *ev
	)

{
	gi.Printf( "Sentient::OnFire not implemented. Needs fixed" );
}

void Sentient::StopOnFire
	(
	Event *ev
	)

{
	gi.Printf( "Sentient::StopOnFire not implemented. Needs fixed" );
}

void Sentient::SpawnBloodyGibs
	(
	Event *ev
	)
	{
	str gib_name;
	int number_of_gibs;
	float scale;
	Animate *ent;
	str real_gib_name;

	if ( !com_blood->integer )
		return;

	//if ( GetActorFlag( ACTOR_FLAG_FADING_OUT ) )
	//	return;

	gib_name = GetGibName();

	if ( !gib_name.length() )
		return;

	// Determine the number of gibs to spawn

	if ( ev->NumArgs() > 0 )
		{
		number_of_gibs = ev->GetInteger( 1 );
		}
	else
		{
		if ( max_gibs == 0 )
			return;

		if ( deadflag )
			number_of_gibs = G_Random( max_gibs / 2 ) + 1;
		else
			number_of_gibs = G_Random( max_gibs ) + 1;
		}

	// Make sure we don't have too few or too many gibs

	if ( number_of_gibs <= 0 || number_of_gibs > 9 )
		return;

	if ( ev->NumArgs() > 1 )
		{
		scale = ev->GetFloat( 2 );
		}
	else
		{
		// Calculate a good scale value

		if ( mass <= 50 )
			scale = 1.0f;
		else if ( mass <= 100 )
			scale = 1.1f;
		else if ( mass <= 250 )
			scale = 1.2f;
		else
			scale = 1.3f;
		}

	// Spawn the gibs

	real_gib_name = gib_name;
	real_gib_name += number_of_gibs;
	real_gib_name += ".tik";

	ent = new Animate;
	ent->setModel( real_gib_name.c_str() );
	ent->setScale( scale );
	ent->setOrigin( centroid );
	ent->NewAnim( "idle" );
	ent->PostEvent( EV_Remove, 1 );

	Sound( "snd_decap", CHAN_BODY, 1, 300 );
	}

void Sentient::SetMaxGibs
	(
	Event *ev
	)
	{
	max_gibs = ev->GetInteger( 1 );
	}

void Sentient::GetStateAnims
   (
   Container<const char *> *c
   )

   {
   }

void Sentient::CheckAnimations
   (
   Event *ev 
   )

   {
   int i,j;
   Container<const char *>co;
   const char *cs;

   GetStateAnims( &co );
   
   gi.DPrintf( "Unused Animations in TIKI\n" );
   gi.DPrintf( "-------------------------\n" );
   for( i=0; i<NumAnims(); i++ )
      {
      const char *c;

	  c = gi.Anim_NameForNum( edict->tiki, i );

      for ( j=1; j<=co.NumObjects(); j++ )
         {
         cs = co.ObjectAt( j );

         if ( !Q_stricmp( c, cs ) )
            {
            goto out;
            }
         else if ( !Q_stricmpn( c, cs, strlen( cs ) ) ) // partial match
            {
            size_t state_len = strlen( cs );

            // Animation in tik file is longer than the state machine's anim
            if ( strlen( c ) > state_len )
               {
               if ( c[state_len] != '_' ) // If next character is an '_' then no match
                  {
                  goto out;
                  }
               }
            else
               {
               goto out;
               }
            }
         }
      // No match made
      gi.DPrintf( "%s used in TIK file but not statefile\n", c );
out:
      ;
      }

   gi.DPrintf( "Unknown Animations in Statefile\n" );
   gi.DPrintf( "-------------------------------\n" );
   for ( j=1; j<=co.NumObjects(); j++ )
      {
      if ( !HasAnim( co.ObjectAt( j ) ) )
         {
         gi.DPrintf( "%s in statefile is not in TIKI\n", co.ObjectAt( j ) );
         }
      }
   }

qboolean Sentient::WeaponsOut
   (
   void
   )

   {
   return ( GetActiveWeapon( WEAPON_OFFHAND ) || GetActiveWeapon( WEAPON_MAIN ) );
   }

void Sentient::Holster
   (
   qboolean putaway
   )

{
	Weapon   *rightWeap;

	rightWeap = GetActiveWeapon( WEAPON_MAIN );

	// Holster
	if( putaway )
	{
		if( rightWeap )
		{
			rightWeap->SetPutAway( true );
			holsteredWeapon = rightWeap;
		}
	}
	else
	{
		if( !putaway )
		{
			// Unholster
			if( holsteredWeapon )
			{
				useWeapon( holsteredWeapon, WEAPON_MAIN );
			}

			holsteredWeapon = NULL;
		}
	}
}

void Sentient::SafeHolster
   (
   qboolean putaway
   )
{
	if( WeaponsOut() )
	{
		if( putaway )
		{
			weapons_holstered_by_code = qtrue;
			Holster( qtrue );
		}
	}
	else
	{
		if( weapons_holstered_by_code )
		{
			weapons_holstered_by_code = qfalse;
			Holster( qfalse );
		}
	}
}

void Sentient::ActivateLastActiveWeapon
	(
	void
	)
{
	if( lastActiveWeapon.weapon && lastActiveWeapon.weapon != activeWeaponList[ WEAPON_MAIN ] )
	{
		useWeapon( lastActiveWeapon.weapon, lastActiveWeapon.hand );
	}
}

//====================
//ActivateNewWeapon
//====================
void Sentient::ActivateNewWeapon
	(
	Event *ev
	)

{
	if( deadflag ) {
		return;
	}

	ActivateNewWeapon();

	if( GetActiveWeapon( WEAPON_MAIN ) )
	{
		edict->s.eFlags &= ~EF_UNARMED;
	}
}

//====================
//ActivateNewWeapon
//====================
void Sentient::ActivateNewWeapon
	(
	void
	)

{
	// Change the weapon to the currently active weapon as specified by useWeapon
	ChangeWeapon( newActiveWeapon.weapon, newActiveWeapon.hand );

	// Update weapons
	UpdateWeapons();

	// Clear out the newActiveWeapon
	ClearNewActiveWeapon();
}

void Sentient::UpdateWeapons
	(
	void
	)

{
	GetActiveWeapon( WEAPON_MAIN );
}

//====================
//DeactivateWeapon
//====================
void Sentient::DeactivateWeapon
(
Event *ev
)

{
	// Deactivate the weapon
	weaponhand_t hand;
	str   side;

	side = ev->GetString( 1 );

	hand = WeaponHandNameToNum( side );

	if( hand == WEAPON_ERROR )
		return;

	DeactivateWeapon( hand );
	edict->s.eFlags |= EF_UNARMED;
}

void Sentient::EventUseItem
	(
	Event *ev
	)

{
	str				name;
	weaponhand_t	hand = WEAPON_MAIN;

	if( deadflag ) {
		return;
	}

	name = ev->GetString( 1 );

	if( ev->NumArgs() > 1 )
	{
		hand = WeaponHandNameToNum( ev->GetString( 2 ) );
	}

	useWeapon( name, hand );
}

void Sentient::EventActivateLastActiveWeapon
	(
	Event *ev
	)
{
	if( !deadflag )
	{
		ActivateLastActiveWeapon();
	}
}

void Sentient::EventDontDropWeapons
	(
	Event *ev
	)

{
	if( ev->NumArgs() > 0 )
	{
		m_bDontDropWeapons = ev->GetBoolean( 1 );
	}
	else
	{
		m_bDontDropWeapons = true;
	}
}

void Sentient::EventUseWeaponClass
	(
	Event *ev
	)
{
	const char *name;
	int weapon_class;
	
	if ( deadflag ) {
		return;
	}
	
	name = ev->GetString( 1 );
	weapon_class = G_WeaponClassNameToNum( name );
	
	int	num;
	Weapon *pActive = GetActiveWeapon( WEAPON_MAIN );
	Weapon *pMatch = NULL;
	Weapon *pWeap = NULL;

	num = inventory.NumObjects();

	for( int i = 1; i <= num; i++ )
	{
		pWeap = ( Weapon * )G_GetEntity( inventory.ObjectAt( i ) );

		if( pWeap->IsSubclassOfWeapon() && 
			( pWeap->GetWeaponClass() & weapon_class ) &&
			( pWeap->HasAmmo( FIRE_PRIMARY ) || pWeap->GetUseNoAmmo() ) )
		{
			if( !pMatch && ( !pActive || pActive != pWeap ) )
			{
				pMatch = pWeap;
			}

			if( !pActive )
			{
				useWeapon( pWeap, WEAPON_MAIN );
				return;
			}

			if( pActive == pWeap ) {
				pActive = NULL;
			}
		}

		pWeap = NULL;
	}

	if( pMatch )
	{
		useWeapon( pMatch, WEAPON_MAIN );
	}
}

void Sentient::EventToggleItem
(
Event *ev
)
{
	Weapon *item;

	if( deadflag )
	{
		return;
	}

	item = GetActiveWeapon( WEAPON_MAIN );

	if( item && item->IsSubclassOfInventoryItem() )
	{
		if( lastActiveWeapon.weapon )
		{
			ActivateLastActiveWeapon();
		}

		Holster( qtrue );
	}
	else
	{
		Event *ev = new Event( EV_Sentient_UseWeaponClass );
		ev->AddString( "item1" );

		ProcessEvent( ev );
	}
}

void Sentient::ReloadWeapon
	(
	Event *ev
	)
{
	Weapon *weapon;
	weaponhand_t hand = WEAPON_MAIN;

	if( ev->NumArgs() > 0 )
	{
		hand = WeaponHandNameToNum( ev->GetString( 1 ) );
	}

	if( hand == WEAPON_ERROR ) {
		return;
	}

	weapon = GetActiveWeapon( hand );

	if( weapon )
	{
		weapon->StartReloading();
	}
}

void Sentient::GetNewActiveWeapon( Event * ev )
{
	ev->AddEntity( GetNewActiveWeapon() );
}

void Sentient::GetActiveWeap( Event * ev )
{
	weaponhand_t weaponhand;
	Item *weapon;

	weaponhand = ( weaponhand_t )ev->GetInteger( 1 );

	if( weaponhand < 0 && weaponhand > 2 )
	{
		ScriptError( "Weaponhand number is out of allowed range 0 - 2 for getactiveweap!\n" );
		return;
	}

	weapon = GetActiveWeapon( weaponhand );

	ev->AddEntity( weapon );
}

void Sentient::ReceivedItem
	(
	Item * item
	)

{
}

void Sentient::RemovedItem
	(
	Item *item
	)

{
}

void Sentient::AmmoAmountChanged
	(
	Ammo *ammo,
	int ammo_in_clip
	)

{
}

void Sentient::EventGerman
	(
	Event *ev
	)

{
	bool bRejoinSquads = false;

	if( ev->IsFromScript() )
	{
		if( m_Team ) {
			bRejoinSquads = true;
		}
	}

	if( bRejoinSquads )
	{
		ClearEnemies();
		DisbandSquadMate( this );
	}

	if( m_NextSentient )
	{
		m_NextSentient->m_PrevSentient = m_PrevSentient;
	}

	if( m_PrevSentient )
	{
		m_PrevSentient->m_NextSentient = m_NextSentient;
	}
	else
	{
		level.m_HeadSentient[ m_Team ] = m_NextSentient;
	}

	m_NextSentient = NULL;
	m_Team = TEAM_GERMAN;
	m_PrevSentient = NULL;

	m_NextSentient = level.m_HeadSentient[ TEAM_GERMAN ];
	if( m_NextSentient )
	{
		m_NextSentient->m_PrevSentient = this;
	}

	level.m_HeadSentient[ m_Team ] = this;

	if( bRejoinSquads )
	{
		JoinNearbySquads( 1024.0f );
	}
}

void Sentient::EventAmerican
	(
	Event *ev
	)

{
	bool bRejoinSquads = false;

	if( ev->IsFromScript() )
	{
		if( m_Team != TEAM_AMERICAN ) {
			bRejoinSquads = true;
		}
	}

	if( bRejoinSquads )
	{
		ClearEnemies();
		DisbandSquadMate( this );
	}

	if( m_NextSentient )
	{
		m_NextSentient->m_PrevSentient = m_PrevSentient;
	}

	if( m_PrevSentient )
	{
		m_PrevSentient->m_NextSentient = m_NextSentient;
	}
	else
	{
		level.m_HeadSentient[ m_Team ] = m_NextSentient;
	}

	m_NextSentient = NULL;
	m_PrevSentient = NULL;
	m_Team = TEAM_AMERICAN;

	m_NextSentient = level.m_HeadSentient[ TEAM_AMERICAN ];
	if( m_NextSentient )
	{
		m_NextSentient->m_PrevSentient = this;
	}

	level.m_HeadSentient[ m_Team ] = this;

	if( bRejoinSquads )
	{
		JoinNearbySquads( 1024.0f );
	}

	if( IsSubclassOfActor() )
	{
		// FIXME

		//Actor->m_csMood = STRING_NERVOUS;
		//Actor->m_csIdleMood = STRING_NERVOUS;
	}
}

void Sentient::EventGetTeam
	(
	Event *ev
	)

{
	if( m_Team == TEAM_AMERICAN )
	{
		ev->AddConstString( STRING_AMERICAN );
	}
	else if( m_Team == TEAM_GERMAN )
	{
		ev->AddConstString( STRING_GERMAN );
	}
	else
	{
		ev->AddConstString( STRING_EMPTY );
	}
}

void Sentient::ClearEnemies()
{

}

void Sentient::EventGetThreatBias
	(
	Event *ev
	)

{
	ev->AddInteger( m_iThreatBias );
}

void Sentient::EventSetThreatBias
	(
	Event *ev
	)

{
	str sBias;

	if( ev->IsStringAt( 1 ) )
	{
		sBias = ev->GetString( 1 );

		if( !Q_stricmp( sBias, "ignoreme" ) )
		{
			m_iThreatBias = 0xFFFFE4C7;
			return;
		}
	}

	m_iThreatBias = ev->GetInteger( 1 );
}

void Sentient::SetDamageMult
	(
	Event *ev
	)

{
	int index = ev->GetInteger( 1 );
	if( index < 0 || index >= MAX_DAMAGE_MULTIPLIERS )
	{
		ScriptError( "Index must be between 0-" STRING( MAX_DAMAGE_MULTIPLIERS - 1 ) "." );
	}

	m_fDamageMultipliers[ index ] = ev->GetFloat( 2 );
}

void Sentient::SetupHelmet
	(
	str sHelmetTiki,
	float fSpeed,
	float fDamageMult,
	str sHelmetSurface1,
	str sHelmetSurface2
	)

{
	m_sHelmetTiki = sHelmetTiki;
	m_sHelmetSurface1 = sHelmetSurface1;
	m_sHelmetSurface2 = sHelmetSurface2;

	m_fHelmetSpeed = fSpeed;
	m_fDamageMultipliers[ 1 ] = fDamageMult;
}

void Sentient::EventSetupHelmet
	(
	Event *ev
	)

{
	// FIXME
}

void Sentient::EventPopHelmet
	(
	Event *ev
	)

{
	// FIXME
}

bool Sentient::WearingHelmet
	(
	void
	)

{
	if( !m_sHelmetSurface1.length() ) {
		return false;
	}

	int iSurf = gi.Surface_NameToNum( edict->tiki, m_sHelmetSurface1 );
	if( iSurf >= 0 )
	{
		return ( ~edict->s.surfaces[ iSurf ] & MDL_SURFACE_NODRAW ) != 0;
	}
	else
	{
		return false;
	}
}

int Sentient::CheckHitLocation
	(
	int iLocation
	)

{
	if( iLocation == 1 )
	{
		if( WearingHelmet() )
		{
			return iLocation;
		}
		else
		{
			return LOCATION_HEAD;
		}
	}

	return iLocation;
}

bool Sentient::IsTeamMate
	(
	Sentient *pOther
	)

{
	return ( pOther->m_bIsDisguised || pOther->m_Team == m_Team );
}

void Sentient::JoinNearbySquads
	(
	float fJoinRadius
	)

{
	float fJoinRadiusSquared = fJoinRadius * fJoinRadius;

	for( Sentient *pFriendly = level.m_HeadSentient[ m_Team ]; pFriendly != NULL; pFriendly = pFriendly->m_NextSentient )
	{
		if( pFriendly->IsDead() ||
			IsSquadMate( pFriendly ) ||
			pFriendly->m_Team != m_Team ) {
			continue;
		}

		if( fJoinRadius >= Vector::DistanceSquared( pFriendly->origin, origin ) )
		{
			MergeWithSquad( pFriendly );
		}
	}
}

void Sentient::MergeWithSquad
	(
	Sentient *pFriendly
	)

{
	Sentient *pFriendNext;
	Sentient *pSelfPrev;

	if( !pFriendly || IsDead() || pFriendly->IsDead() ) {
		return;
	}

	pFriendNext = pFriendly->m_pNextSquadMate;
	pSelfPrev = m_pPrevSquadMate;

	pFriendly->m_pNextSquadMate = this;
	m_pPrevSquadMate = pFriendly;

	pFriendNext->m_pPrevSquadMate = pSelfPrev;
	pSelfPrev->m_pNextSquadMate = pFriendNext;
}

void Sentient::DisbandSquadMate
	(
	Sentient *pExFriendly
	)

{
	Sentient *pPrev;
	Sentient *pNext;

	pPrev = pExFriendly->m_pPrevSquadMate;
	pNext = pExFriendly->m_pNextSquadMate;

	pPrev->m_pNextSquadMate = pNext;
	pNext->m_pPrevSquadMate = pPrev;

	pExFriendly->m_pPrevSquadMate = pExFriendly;
	pExFriendly->m_pNextSquadMate = pExFriendly;
}

bool Sentient::IsSquadMate
	(
	Sentient *pFriendly
	)

{
	Sentient *pSquadMate = this;

	while( 1 )
	{
		if( pSquadMate == pFriendly )
		{
			return true;
		}

		pSquadMate = pSquadMate->m_pNextSquadMate;
		if( pSquadMate == this )
		{
			return false;
		}
	}
}

bool Sentient::CanSee
	(
	Entity *ent,
	float fov,
	float vision_distance
	)

{
	float delta[ 2 ];

	delta[ 0 ] = ent->centroid[ 0 ] - centroid[ 0 ];
	delta[ 1 ] = ent->centroid[ 1 ] - centroid[ 1 ];

	if( ( vision_distance <= 0.0f ) || VectorLength2DSquared( delta ) <= ( vision_distance * vision_distance ) )
	{
		if( gi.AreasConnected( edict->r.areanum, ent->edict->r.areanum ) &&
			( ( fov <= 0.0f || fov >= 360.0f ) ||
			( FovCheck( delta, cos( fov * ( 0.5 * M_PI / 180.0 ) ) ) ) ) )
		{
			if( ent->IsSubclassOfPlayer() )
			{
				Player *p = ( Player * )ent;

				Vector vStart = EyePosition();
				Vector vEnd = p->EyePosition();

				return G_SightTrace( vStart,
					vec_zero,
					vec_zero,
					vEnd,
					this,
					ent,
					MASK_CANSEE,
					0,
					"Sentient::CanSee 1" );
			}
			else
			{
				Vector vStart = EyePosition();
				Vector vEnd = ent->centroid;

				return G_SightTrace( vStart,
					vec_zero,
					vec_zero,
					vEnd,
					this,
					ent,
					MASK_CANSEE,
					0,
					"Sentient::CanSee 2" );
			}
		}
	}

	return false;
}

void Sentient::SetViewAngles
	(
	Vector angles
	)

{

}

Vector Sentient::GetViewAngles
	(
	void
	)

{
	return angles;
}

void Sentient::UpdateFootsteps
	(
	void
	)

{
	// FIXME
}
