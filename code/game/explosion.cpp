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

// explosion.cpp: Standard explosion object that is spawned by other entites and not map designers.
// Explosion is used by many of the weapons for the blast effect, but is also used
// by the Exploder and MultiExploder triggers.  These triggers create one or more
// explosions each time they are activated.
//

#include "g_local.h"
#include "actor.h"
#include "entity.h"
#include "trigger.h"
#include "explosion.h"
#include "weaputils.h"

#define MULTI_USE       (1<<0)
#define RANDOM_TIME     (1<<1)
#define VISIBLE         (1<<2)
#define RANDOM_SCALE    (1<<3)
#define NO_EXPLOSIONS   (1<<4)

Event EV_Exploder_SetDmg
	( 
	"dmg",
	EV_DEFAULT,
   "i",
   "damage",
   "Sets the damage the explosion does.",
   EV_NORMAL
	);
Event EV_Exploder_SetDuration
	( 
	"duration",
	EV_DEFAULT,
   "f",
   "duration",
   "Sets the duration of the explosion.",
   EV_NORMAL
	);
Event EV_Exploder_SetWait
	( 
	"wait",
	EV_DEFAULT,
   "f",
   "explodewait",
   "Sets the wait time of the explosion.",
   EV_NORMAL
	);
Event EV_Exploder_SetRandom
	( 
	"random",
	EV_DEFAULT,
   "f",
   "randomness",
   "Sets the randomness value of the explosion.",
   EV_NORMAL
	);

void CreateExplosion
	(
	Vector pos,
	float  damage,
	Entity *inflictor,
	Entity *attacker,
	Entity *ignore,
	const char *explosionModel,
	float  scale
	)

{
	Explosion   *explosion;
	Event       *ev;

	assert( inflictor );

	if( !inflictor )
	{
		return;
	}

	if( !attacker )
	{
		attacker = world;
	}

	if( !explosionModel )
	{
		explosionModel = "fx_explosion.tik";
	}

	explosion = new Explosion;

	// Create a new explosion entity and set it off
	explosion->setModel( explosionModel );

	explosion->setSolidType( SOLID_NOT );
	explosion->ProcessInitCommands();

	explosion->owner = attacker->entnum;
	explosion->edict->r.ownerNum = attacker->entnum;
	explosion->setMoveType( MOVETYPE_FLYMISSILE );
	explosion->edict->clipmask = MASK_PROJECTILE;
	explosion->setSize( explosion->mins, explosion->maxs );
	explosion->setOrigin( pos );
	explosion->origin.copyTo( explosion->edict->s.origin2 );

	if( explosion->dlight_radius )
	{
		G_SetConstantLight( &explosion->edict->s.constantLight,
			&explosion->dlight_color[ 0 ],
			&explosion->dlight_color[ 1 ],
			&explosion->dlight_color[ 2 ],
			&explosion->dlight_radius
			);
	}

	explosion->BroadcastAIEvent( AI_EVENT_EXPLOSION );

	explosion->NewAnim( "idle" );
	RadiusDamage( inflictor->origin, inflictor, attacker, damage, ignore, MOD_EXPLOSION );

	if( explosion->life )
	{
		ev = new Event( EV_Remove );
		explosion->PostEvent( ev, explosion->life );
	}
}


/*****************************************************************************/
/*QUAKED func_exploder (0 0.25 0.5) (0 0 0) (8 8 8)

  Spawns an explosion when triggered.  Triggers any targets.

  "dmg" specifies how much damage to cause. Default indicates 120.
  "key" The item needed to activate this. (default nothing)
  "thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.
******************************************************************************/

CLASS_DECLARATION( Trigger, Exploder, "func_exploder" )
	{
		{ &EV_Touch,				         NULL },
		{ &EV_Trigger_Effect,				&Exploder::MakeExplosion },
      { &EV_Exploder_SetDmg,				&Exploder::SetDmg },
		{ NULL, NULL }
	};

void Exploder::MakeExplosion
	(
	Event *ev
	)

	{
   CreateExplosion
      (
      origin,
      damage,
      this,
      ev->GetEntity( 1 ),
      this
      );
	}

Exploder::Exploder()
	{
	damage      = 120;
	respondto   = TRIGGER_PLAYERS | TRIGGER_MONSTERS | TRIGGER_PROJECTILES;
	}

void Exploder::SetDmg
   (
   Event *ev
   )

   {
   damage = ev->GetInteger( 1 );
	if ( damage < 0 )
		{
		damage = 0;
		}
   }

/*****************************************************************************/
/*QUAKED func_multi_exploder (0 0.25 0.5) ? MULTI_USE RANDOM_TIME VISIBLE RANDOM_SCALE

  Spawns an explosion when triggered.  Triggers any targets.
  size of brush determines where explosions will occur.

  "dmg" specifies how much damage to cause from each explosion. (Default 120)
  "delay" delay before exploding (Default 0 seconds)
  "duration" how long to explode for (Default 1 second)
  "wait" time between each explosion (default 0.25 seconds)
  "random" random factor (default 0.25)
  "key" The item needed to activate this. (default nothing)
  "thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.
  "health" makes the object damageable
  "scale" set the maximum size for spawned debris and explosions.

  MULTI_USE allows the func_multi_exploder to be used more than once
  RANDOM_TIME adjusts the wait between each explosion by the random factor
  VISIBLE allows you to make the trigger visible
  RANDOM_SCALE scale explosions randomly. size will be between 0.25 and 1 times scale

******************************************************************************/

CLASS_DECLARATION( Trigger, MultiExploder, "func_multi_exploder" )
	{
		{ &EV_Touch,				         NULL },
		{ &EV_Trigger_Effect,				&MultiExploder::MakeExplosion },
      { &EV_Exploder_SetDmg,				&MultiExploder::SetDmg },
      { &EV_Exploder_SetDuration,			&MultiExploder::SetDuration },
      { &EV_Exploder_SetWait,				&MultiExploder::SetWait },
      { &EV_Exploder_SetRandom,				&MultiExploder::SetRandom },
		{ NULL, NULL }
	};

void MultiExploder::MakeExplosion
	(
	Event *ev
	)

	{
	Vector pos;
	float t, scale;
   Entity *other;
   Event *event;

	other = ev->GetEntity( 1 );

   // make sure other is valid
   if ( !other )
      {
      other = world;
      }

	// prevent the trigger from triggering again
	trigger_time = -1;

	if ( !explode_time )
		{
      hideModel();
		explode_time = level.time + duration;
		}

	if ( spawnflags & RANDOM_TIME )
		{
		t = explodewait * ( 1 + G_CRandom( randomness ) );
		}
	else
		{
		t = explodewait;
		}

	event = new Event( EV_Trigger_Effect );
	event->AddEntity( other );
   PostEvent( event, t );

	if ( level.time > explode_time )
		{
      if ( spawnflags & MULTI_USE )
         {
         //
         // reset the trigger in a half second
         //
         trigger_time = level.time + 0.5f;
         explode_time = 0;
         CancelEventsOfType( EV_Trigger_Effect );
         //
         // reset health if necessary
         //
         health = max_health;
         return;
         }
      else
         {
		   PostEvent( EV_Remove, 0 );
		   return;
         }
		}

	pos[ 0 ] = absmin[ 0 ] + G_Random( absmax[ 0 ] - absmin[ 0 ] );
	pos[ 1 ] = absmin[ 1 ] + G_Random( absmax[ 1 ] - absmin[ 1 ] );
	pos[ 2 ] = absmin[ 2 ] + G_Random( absmax[ 2 ] - absmin[ 2 ] );

   if ( spawnflags & RANDOM_SCALE )
      {
      scale = edict->s.scale * 0.25f;
      scale += G_Random( 3 * scale );
      }
   else
      {
      scale = 1.0f;
      }

   CreateExplosion
      (
      pos,
      damage,
      this,
      other,
      this,
      NULL,
      scale
      );
	}

MultiExploder::MultiExploder()
	{
   if ( LoadingSavegame )
      {
      return;
      }

	damage         = 120;
	duration			= 1.0;
	explodewait		= 0.25;
	randomness		= 0.25;
	explode_time	= 0;

   if ( spawnflags & VISIBLE )
      {
      PostEvent( EV_Show, EV_POSTSPAWN );
      }
   else
      {
      PostEvent( EV_Hide, EV_POSTSPAWN );
      }

	// So that we don't get deleted after we're triggered
	count = -1;

	respondto = TRIGGER_PLAYERS | TRIGGER_MONSTERS | TRIGGER_PROJECTILES;
	}

void MultiExploder::SetDmg
   (
   Event *ev
   )

   {
   damage = ev->GetInteger( 1 );
	if ( damage < 0 )
		{
		damage = 0;
		}
   }

void MultiExploder::SetDuration
   (
   Event *ev
   )

   {
	duration = ev->GetFloat( 1 );
   }

void MultiExploder::SetWait
   (
   Event *ev
   )

   {
	explodewait = ev->GetFloat( 1 );
   }

void MultiExploder::SetRandom
   (
   Event *ev
   )

   {
	randomness = ev->GetFloat( 1 );
   }


#define METAL_DEBRIS (1<<5)
#define ROCK_DEBRIS  (1<<6)
#define NOTSOLID     (1<<7)

/*****************************************************************************/
/*QUAKED func_explodeobject (0 0.25 0.5) ? MULTI_USE RANDOM_TIME VISIBLE RANDOM_SCALE NO_EXPLOSIONS METAL_DEBRIS ROCK_DEBRIS NOTSOLID

  Spawns different kinds of debris when triggered.  Triggers any targets.
  size of brush determines where explosions and debris will be spawned.

  "dmg" specifies how much damage to cause from each explosion. (Default 120)
  "delay" delay before exploding (Default 0 seconds)
  "duration" how long to explode for (Default 1 second)
  "wait" time between each explosion (default 0.25 seconds)
  "random" random factor (default 0.25)
  "health" if specified, object must be damaged to trigger
  "key" The item needed to activate this. (default nothing)
  "severity" how violent the debris should be ejected from the object( default 1.0 )
  "debrismodel" What kind of debris to spawn (default nothing)
  "amount" how much debris to spawn for each explosion (default 4)
  "thread" name of thread to trigger.  This can be in a different script file as well\
by using the '::' notation.
  "health" makes the object damageable
  "scale" set the maximum size for spawned debris and explosions

  MULTI_USE allows the func_explodeobject to be used more than once
  RANDOM_TIME adjusts the wait between each explosion by the random factor
  VISIBLE allows you to make the trigger visible
  RANDOM_SCALE scale explosions and debris randomly. size will be between 0.25 and 1 times scale
  NO_EXPLOSIONS, if checked no explosions will be created
  METAL_DEBRIS automatically spawn metal debris, no need for debrismodel to be set
  ROCK_DEBRIS automatically spawn rock debris, no need for debrismodel to be set
  NOTSOLID debris is not solid

  other valid tiki files include:

  obj_debris_glass1-4.tik
  obj_debris_wood1-4.tik

******************************************************************************/

Event EV_ExplodeObject_SetSeverity
	( 
	"severity",
	EV_DEFAULT,
   "f",
   "newSeverity",
   "How violently the debris should be ejected.",
   EV_NORMAL
	);

Event EV_ExplodeObject_SetDebrisModel
	( 
	"debrismodel",
	EV_DEFAULT,
   "s",
   "debrisModel",
   "What kind of debris to spawn when triggered.",
   EV_NORMAL
	);

Event EV_ExplodeObject_SetDebrisAmount
	( 
	"amount",
	EV_DEFAULT,
   "i",
   "amountOfDebris",
   "How much debris to spawn each time.",
   EV_NORMAL
	);


CLASS_DECLARATION( MultiExploder, ExplodeObject, "func_explodeobject" )
	{
		{ &EV_Touch,								NULL },
		{ &EV_Trigger_Effect,						&ExplodeObject::MakeExplosion },
		{ &EV_ExplodeObject_SetSeverity,			&ExplodeObject::SetSeverity },
		{ &EV_ExplodeObject_SetDebrisModel,			&ExplodeObject::SetDebrisModel },
		{ &EV_ExplodeObject_SetDebrisAmount,		&ExplodeObject::SetDebrisAmount },
		{ NULL, NULL }
	};

void ExplodeObject::SetDebrisModel
   (
   Event *ev
   )

   {
   char   string[ 1024 ];
   const char *ptr;

   // there could be multiple space delimited models, so we need to search for the spaces.
   strcpy( string,  ev->GetString( 1 ) );
   ptr = strtok( string, " " );
   while ( ptr )
      {
      debrismodels.AddUniqueObject( str( ptr ) );
		CacheResource( ptr );
      ptr = strtok( NULL, " " );
      }
   }

void ExplodeObject::SetSeverity
   (
   Event *ev
   )

   {
	severity = ev->GetFloat( 1 );
   }

void ExplodeObject::SetDebrisAmount
   (
   Event *ev
   )

   {
	debrisamount = ev->GetInteger( 1 );
   }

void ExplodeObject::MakeExplosion
	(
	Event *ev
	)

	{
	Vector pos;
	float t, scale;
   Entity *other;
   Event *event;

	other = ev->GetEntity( 1 );

   // make sure other is valid
   if ( !other )
      {
      other = world;
      }

	// prevent the trigger from triggering again
	trigger_time = -1;

	if ( !explode_time )
		{
      setSolidType( SOLID_NOT );
      hideModel();
		explode_time = level.time + duration;
		}

	if ( spawnflags & RANDOM_TIME )
		{
		t = explodewait * ( 1 + G_CRandom( randomness ) );
		}
	else
		{
		t = explodewait;
		}

	event = new Event( EV_Trigger_Effect );
	event->AddEntity( other );
   PostEvent( event, t );

	if ( level.time > explode_time )
		{
      if ( spawnflags & MULTI_USE )
         {
         //
         // reset the trigger in a half second
         //
         trigger_time = level.time + 0.5f;
         explode_time = 0;
         CancelEventsOfType( EV_Trigger_Effect );
         //
         // reset health if necessary
         //
         health = max_health;
         if ( health )
            {
   		   setSolidType( SOLID_BBOX );
            }
         if ( spawnflags & VISIBLE )
            {
            PostEvent( EV_Show, 0.5f );
            }
         return;
         }
      else
         {
		   PostEvent( EV_Remove, 0 );
		   return;
         }
		}

	pos[ 0 ] = absmin[ 0 ] + G_Random( absmax[ 0 ] - absmin[ 0 ] );
	pos[ 1 ] = absmin[ 1 ] + G_Random( absmax[ 1 ] - absmin[ 1 ] );
	pos[ 2 ] = absmin[ 2 ] + G_Random( absmax[ 2 ] - absmin[ 2 ] );

   if ( spawnflags & RANDOM_SCALE )
      {
      scale = edict->s.scale * 0.25f;
      scale += G_Random( 3 * scale );
      }
   else
      {
      scale = 1.0f;
      }

   if ( !( spawnflags & NO_EXPLOSIONS ) )
      {
      CreateExplosion
         (
         pos,
         damage,
         this,
         other,
         this,
         NULL,
         scale
         );
      }
   if ( debrismodels.NumObjects() )
      {
      TossObject *to;
      int i;
      for ( i = 0; i < debrisamount; i++ )
         {
         int num;

         if ( spawnflags & RANDOM_SCALE )
            {
            scale = edict->s.scale * 0.25f;
            scale += G_Random( 3 * scale );
            }
         else
            {
            scale = 1.0f;
            }

         num = G_Random( debrismodels.NumObjects() ) + 1;
         to = new TossObject( debrismodels.ObjectAt( num ) );
         to->setScale( scale );
         to->setOrigin( pos );
         to->SetVelocity( severity );
         if ( spawnflags & NOTSOLID )
            {
            to->setSolidType( SOLID_NOT );
            }
	      pos[ 0 ] = absmin[ 0 ] + G_Random( absmax[ 0 ] - absmin[ 0 ] );
	      pos[ 1 ] = absmin[ 1 ] + G_Random( absmax[ 1 ] - absmin[ 1 ] );
	      pos[ 2 ] = absmin[ 2 ] + G_Random( absmax[ 2 ] - absmin[ 2 ] );
         }
      }

	}

ExplodeObject::ExplodeObject()
	{
   if ( !LoadingSavegame )
      {
	   duration			= 1;
	   explodewait		= 0.25f;
      severity       = 1.0f;
      debrismodels.ClearObjectList();
      debrisamount   = 2;
      if ( spawnflags & METAL_DEBRIS )
         {
         debrismodels.AddUniqueObject( str( "obj_debris_metal1.tik" ) );
         debrismodels.AddUniqueObject( str( "obj_debris_metal2.tik" ) );
         debrismodels.AddUniqueObject( str( "obj_debris_metal3.tik" ) );
			CacheResource( "obj_debris_metal1.tik" );
			CacheResource( "obj_debris_metal2.tik" );
			CacheResource( "obj_debris_metal3.tik" );
         }
      else if ( spawnflags & ROCK_DEBRIS )
         {
         debrismodels.AddUniqueObject( str( "obj_debris_rock1.tik" ) );
         debrismodels.AddUniqueObject( str( "obj_debris_rock2.tik" ) );
         debrismodels.AddUniqueObject( str( "obj_debris_rock3.tik" ) );
         debrismodels.AddUniqueObject( str( "obj_debris_rock4.tik" ) );
			CacheResource( "obj_debris_rock1.tik" );
			CacheResource( "obj_debris_rock2.tik" );
			CacheResource( "obj_debris_rock3.tik" );
			CacheResource( "obj_debris_rock4.tik" );
         }
      }
	}

