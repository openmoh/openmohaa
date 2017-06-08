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

// misc.cpp: Basically the big stew pot of the DLLs, or maybe a garbage bin, whichever
// metaphore you prefer.  This really should be cleaned up.  Anyway, this
// should contain utility functions that could be used by any entity.
// Right now it contains everything from entities that could be in their
// own file to my mother's pot roast recipes.
//

#include "glb_local.h"
#include "entity.h"
#include "trigger.h"
#include "explosion.h"
#include "misc.h"
#include "navigate.h"
#include "specialfx.h"
#include "player.h"
#include "weaputils.h"
#include <g_spawn.h>

/*****************************************************************************/
/*QUAKED detail (0.5 0 1.0) ?

Used to fake detail brushes, convenient for grouping

******************************************************************************/

/*****************************************************************************/
/*QUAKED func_group (0.5 0.5 0.5) ?

Used to group brushes together just for editor convenience.

******************************************************************************/

/*****************************************************************************/
/*QUAKED func_remove (0.75 0.75 0.75) ?

Used for lighting and such

******************************************************************************/

CLASS_DECLARATION( Entity, FuncRemove, "func_remove" )
	{
		{ NULL, NULL }
	};

FuncRemove::FuncRemove()
	{
	PostEvent( EV_Remove, EV_REMOVE );
	}

/*****************************************************************************/
/*QUAKED misc_model (1 0.5 1) (0 0 0) (0 0 0)
"model"     arbitrary .tik file to display
******************************************************************************/

CLASS_DECLARATION( Entity, MiscModel, "misc_model" )
	{
		{ NULL, NULL }
	};

MiscModel::MiscModel()
	{
	PostEvent( EV_Remove, EV_REMOVE );
	}


/*****************************************************************************/
/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)

Used as a positional target for spotlights, etc.

******************************************************************************/

CLASS_DECLARATION( Entity, InfoNull, "info_null" )
	{
		{ NULL, NULL }
	};

InfoNull::InfoNull()
	{
	PostEvent( EV_Remove, EV_REMOVE );
	}

/*****************************************************************************/
/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)

Used as a positional target for lightning.

******************************************************************************/

CLASS_DECLARATION( Entity, InfoNotNull, "info_notnull" )
	{
		{ NULL, NULL }
	};



/*****************************************************************************/
/*QUAKED func_explodingwall (0 0.25 0.5) ? RANDOMANGLES LANDSHATTER NOT_PLAYERS MONSTERS PROJECTILES INVISIBLE ACCUMALATIVE TWOSTAGE

Blows up on activation or when attacked

"explosions"      number of explosions to spawn ( default 1 )
"land_angles"     The angles you want this piece to\
                  orient to when it lands on the ground
"land_radius"     The distance of the ground the piece\
                  should be when on the ground ( default 16 )
"anglespeed"      Speed at which pieces rotate ( default 100 ) \
                  if RANDOMANGLES ( default is 600 )
"key"             The item needed to activate this. (default nothing)
"base_velocity"   The speed that the debris will have when triggered.  (default 0 0 280)
"random_velocity" The variation of the velocity. x & y will be from -n < X,Y < n and z is 0 <= Z < n.  (default 140 140 140)


IF RANDOMANGLES is set, object randomly spins while in the air.
IF LANDSHATTER is set, object shatters when it hits the ground.
IF TWOSTAGE is set, object can be shattered once it lands on the ground.
IF ACCUMALATIVE is set, damage is accumlative not threshold
IF INVISIBLE is set, these are invisible and not solid until triggered
If NOT_PLAYERS is set, the trigger does not respond to players
If MONSTERS is set, the trigger will respond to monsters
If PROJECTILES is set, the trigger will respond to projectiles (rockets, grenades, etc.)

******************************************************************************/
#define RANDOMANGLES ( 1 << 0 )
#define LANDSHATTER  ( 1 << 1 )
#define INVISIBLE    ( 1 << 5 )
#define ACCUMULATIVE ( 1 << 6 )
#define TWOSTAGE     ( 1 << 7 )

Event EV_ExplodingWall_StopRotating
	( 
	"stoprotating",
	EV_DEFAULT,
   NULL,
   NULL,
   "Stop rotating the wall.",
	EV_NORMAL
	);
Event EV_ExplodingWall_OnGround
	( 
	"checkonground",
	EV_DEFAULT,
   NULL,
   NULL,
   "Check if exploding wall is on ground.",
	EV_NORMAL
	);
Event EV_ExplodingWall_AngleSpeed
	( 
	"anglespeed",
	EV_DEFAULT,
   "f",
   "speed",
   "Set the angle speed.",
	EV_NORMAL
	);
Event EV_ExplodingWall_LandRadius
	( 
	"land_radius",
	EV_DEFAULT,
   "f",
   "radius",
   "Set the land radius.",
	EV_NORMAL
	);
Event EV_ExplodingWall_LandAngles
	( 
	"land_angles",
	EV_DEFAULT,
   "v",
   "angles",
   "Set the land angles.",
	EV_NORMAL
	);
Event EV_ExplodingWall_BaseVelocity
	( 
	"base_velocity",
	EV_DEFAULT,
   "v",
   "velocity",
   "Set the base velocity.",
	EV_NORMAL
	);
Event EV_ExplodingWall_RandomVelocity
	( 
	"random_velocity",
	EV_DEFAULT,
   "v",
   "velocity",
   "Set the amount of random variation of the base velocity.",
	EV_NORMAL
	);
Event EV_ExplodingWall_SetDmg
	( 
	"dmg",
	EV_DEFAULT,
   "i",
   "dmg",
   "Set the damage from the exploding wall.",
	EV_NORMAL
	);
Event EV_ExplodingWall_SetExplosions
	( 
	"explosions",
	EV_DEFAULT,
   "i",
   "explosions",
   "Set the number of explosions.",
	EV_NORMAL
	);
Event EV_ExplodingWall_Setup
	( 
	"setup",
   EV_CODEONLY,
   NULL,
   NULL,
   "Initializes the exploding wall.",
	EV_NORMAL
	);

CLASS_DECLARATION( Trigger, ExplodingWall, "func_explodingwall" )
	{
		{ &EV_ExplodingWall_Setup,				&ExplodingWall::Setup },
		{ &EV_Trigger_Effect,					&ExplodingWall::Explode },
		{ &EV_Damage,							&ExplodingWall::DamageEvent },
		{ &EV_Touch,							&ExplodingWall::TouchFunc },
		{ &EV_ExplodingWall_StopRotating,		&ExplodingWall::StopRotating },
		{ &EV_ExplodingWall_OnGround,			&ExplodingWall::CheckOnGround },
      { &EV_ExplodingWall_AngleSpeed,			&ExplodingWall::AngleSpeed },
      { &EV_ExplodingWall_LandRadius,			&ExplodingWall::LandRadius },
      { &EV_ExplodingWall_LandAngles,			&ExplodingWall::LandAngles },
      { &EV_ExplodingWall_BaseVelocity,			&ExplodingWall::BaseVelocity },
      { &EV_ExplodingWall_RandomVelocity,		&ExplodingWall::RandomVelocity },
      { &EV_ExplodingWall_SetDmg,				&ExplodingWall::SetDmg },
      { &EV_ExplodingWall_SetExplosions,		&ExplodingWall::SetExplosions },
		{ NULL, NULL }
	};

void ExplodingWall::AngleSpeed
   (
   Event *ev
   )

   {
   angle_speed = ev->GetFloat( 1 );
   }

void ExplodingWall::LandRadius
   (
   Event *ev
   )

   {
   land_radius = ev->GetFloat( 1 );
   }

void ExplodingWall::LandAngles
   (
   Event *ev
   )

   {
   land_angles = ev->GetVector( 1 );
   }

void ExplodingWall::BaseVelocity
   (
   Event *ev
   )

   {
   base_velocity = ev->GetVector( 1 );
   }

void ExplodingWall::RandomVelocity
   (
   Event *ev
   )

   {
   random_velocity = ev->GetVector( 1 );
   }

void ExplodingWall::SetDmg
   (
   Event *ev
   )

   {
   dmg = ev->GetInteger( 1 );
   }

void ExplodingWall::SetExplosions
   (
   Event *ev
   )

   {
   explosions = ev->GetInteger( 1 );
   }

void ExplodingWall::Explode
	(
	Event *ev
	)

	{
	Entity		*other;
	Vector		pos;
   Vector      mins, maxs;
	int			i;

   if ( spawnflags & INVISIBLE )
      {
	   showModel();
	   setSolidType( SOLID_BSP );
   	takedamage = DAMAGE_YES;
      }

	if ( takedamage == DAMAGE_NO )
		{
		return;
		}

	other = ev->GetEntity( 1 );

	health = 0;
	takedamage = DAMAGE_NO;

	// Create explosions
	for( i = 0; i < explosions; i++ )
		{
		pos[ 0 ] = absmin[ 0 ] + G_Random( size[ 0 ] );
		pos[ 1 ] = absmin[ 1 ] + G_Random( size[ 1 ] );
		pos[ 2 ] = absmin[ 2 ] + G_Random( size[ 2 ] );

		CreateExplosion( pos, dmg, this, other, this );
		}

	// throw itself
   state = 1;
   on_ground = false;
   PostEvent( EV_ExplodingWall_OnGround, FRAMETIME );

   velocity.x = base_velocity.x + G_CRandom( random_velocity.x );
   velocity.y = base_velocity.y + G_CRandom( random_velocity.y );
   velocity.z = base_velocity.z + G_Random( random_velocity.z );

	setMoveType( MOVETYPE_BOUNCE );
	setSolidType( SOLID_BBOX );
   if ( spawnflags & RANDOMANGLES )
      {
	   avelocity[ 0 ] = G_Random( angle_speed );
	   avelocity[ 1 ] = G_Random( angle_speed );
	   avelocity[ 2 ] = G_Random( angle_speed );
      }
   else
      {
      Vector delta;
      float most;
      float time;
      int   t;

      delta = land_angles - angles;
      if ( delta[ 0 ] > 180 )
         delta[ 0 ] -= 360;
      if ( delta[ 0 ] < -180 )
         delta[ 0 ] += 360;
      if ( delta[ 1 ] > 180 )
         delta[ 1 ] -= 360;
      if ( delta[ 1 ] < -180 )
         delta[ 1 ] += 360;
      if ( delta[ 2 ] > 180 )
         delta[ 2 ] -= 360;
      if ( delta[ 2 ] < -180 )
         delta[ 2 ] += 360;
      most = MaxValue( delta );
      if ( !angle_speed )
         angle_speed = 1;
      t = 10 * most / angle_speed;
      time = (float)t / 10;
      delta = delta * (1.0/time);
      avelocity = delta;
      PostEvent( EV_ExplodingWall_StopRotating, time );
      state = 2;
      }

	ActivateTargets( ev );

   if ( land_radius > 0 )
      {
      mins[0] = mins[1] = mins[2] = -land_radius;
      maxs[0] = maxs[1] = maxs[2] = land_radius;
	   setSize( mins, maxs );
      }

	attack_finished = 0;
	}

void ExplodingWall::DamageEvent
	(
	Event *ev
	)

	{
	Event			*event;
	Entity		*inflictor;
	Entity		*attacker;
	int			damage;

	if ( takedamage == DAMAGE_NO )
		{
		return;
		}

   if ( on_ground )
      {
      GroundDamage( ev );
      return;
      }

   attacker		= ev->GetEntity( 1 );
	damage		= ev->GetInteger( 2 );
	inflictor	= ev->GetEntity( 3 );

   if ( spawnflags & ACCUMULATIVE )
      {
      health -= damage;
      if ( health > 0 )
         return;
      }
   else
      {
	   if ( damage < health )
		   {
		   return;
		   }
      }

	event = new Event( EV_Activate );
	event->AddEntity( attacker );
	ProcessEvent( event );
	}

void ExplodingWall::GroundDamage
	(
	Event *ev
	)

	{
	Entity		*inflictor;
	Entity		*attacker;
	Vector		pos;
	int			damage;

	if ( takedamage == DAMAGE_NO )
		{
		return;
		}

	damage		= ev->GetInteger( 1 );
	inflictor	= ev->GetEntity( 2 );
	attacker		= ev->GetEntity( 3 );

   if ( spawnflags & ACCUMULATIVE )
      {
      health -= damage;
      if ( health > 0 )
         return;
      }
   else
      {
	   if ( damage < health )
		   {
		   return;
		   }
      }

   if ( explosions )
      {
		pos[ 0 ] = absmin[ 0 ] + G_Random( size[ 0 ] );
		pos[ 1 ] = absmin[ 1 ] + G_Random( size[ 1 ] );
		pos[ 2 ] = absmin[ 2 ] + G_Random( size[ 2 ] );

		CreateExplosion( pos, damage, this, attacker, this );
      }
   takedamage = DAMAGE_NO;
   hideModel();
   BroadcastAIEvent();
   PostEvent( EV_Remove, 0 );
	}

void ExplodingWall::SetupSecondStage
	(
   void
	)

	{
   health = max_health;
	takedamage = DAMAGE_YES;
	}

void ExplodingWall::StopRotating
	(
	Event *ev
	)

	{
   avelocity = vec_zero;
   setAngles( land_angles );
   if ( spawnflags & TWOSTAGE )
      SetupSecondStage();
	}

void ExplodingWall::CheckOnGround
	(
	Event *ev
	)

	{
   if ( ( velocity == vec_zero ) && groundentity )
      {
      Vector delta;
      float most;
      float time;
      int   t;

      delta = land_angles - angles;
      if ( delta.length() > 1 )
         {
         if ( delta[ 0 ] > 180 )
            delta[ 0 ] -= 360;
         if ( delta[ 0 ] < -180 )
            delta[ 0 ] += 360;
         if ( delta[ 1 ] > 180 )
            delta[ 1 ] -= 360;
         if ( delta[ 1 ] < -180 )
            delta[ 1 ] += 360;
         if ( delta[ 2 ] > 180 )
            delta[ 2 ] -= 360;
         if ( delta[ 2 ] < -180 )
            delta[ 2 ] += 360;
         most = MaxValue( delta );
         if ( angle_speed > 3 )
            t = 10.0f * most / ( angle_speed / 3 );
         else
            t = 10.0f * most;
         time = (float)t / 10;
         delta = delta * (1.0/time);
         avelocity = delta;
         PostEvent( EV_ExplodingWall_StopRotating, time );
         }
      state = 2;
	   setSize( orig_mins, orig_maxs );
      on_ground = true;
      }
   else
      PostEvent( ev, FRAMETIME );
	}

void ExplodingWall::TouchFunc
	(
	Event *ev
	)

	{
	Entity *other;

	if ( ( velocity == vec_zero ) || ( level.time < attack_finished ) )
		{
		return;
		}

   other = ev->GetEntity( 1 );

   if ( ( spawnflags & LANDSHATTER ) && ( other == world ) )
      {
      Vector pos;

   	takedamage = DAMAGE_NO;

      if ( explosions )
         {
		   pos[ 0 ] = absmin[ 0 ] + G_Random( size[ 0 ] );
		   pos[ 1 ] = absmin[ 1 ] + G_Random( size[ 1 ] );
		   pos[ 2 ] = absmin[ 2 ] + G_Random( size[ 2 ] );

		   CreateExplosion( pos, dmg, this, other, this );
         }
      hideModel();
	  BroadcastAIEvent();
      PostEvent( EV_Remove, 0 );
      return;
      }

	if ( other->takedamage )
		{
      other->Damage( this, activator, dmg, origin, vec_zero, vec_zero, 20, 0, MOD_EXPLODEWALL );
		Sound( "debris_generic", CHAN_WEAPON );
		attack_finished = level.time + FRAMETIME;
		}
	}

void ExplodingWall::Setup
   (
   Event *ev
   )

	{
   if ( spawnflags & INVISIBLE )
      {
	   if ( Targeted() )
   	  	takedamage = DAMAGE_YES;
      else
         takedamage = DAMAGE_NO;
      hideModel();
	   setSolidType( SOLID_NOT );
      }
   else
      {
	   showModel();
	   setSolidType( SOLID_BSP );
     	takedamage = DAMAGE_YES;
      }

	setMoveType( MOVETYPE_PUSH );
	setOrigin();
	}

ExplodingWall::ExplodingWall()
   {
   if ( LoadingSavegame )
      {
      return;
      }

	health = 60;
	max_health = health;
   on_ground = false;

   state = 0;
   angle_speed = ( spawnflags & RANDOMANGLES ) ? 600 : 100;
	land_radius = 16;
	dmg = 10;
	explosions = 1;

   base_velocity     = Vector( 0, 0, 280 );
   random_velocity   = Vector( 140, 140, 140 );

   orig_mins = mins;
   orig_maxs = maxs;

	respondto = spawnflags ^ TRIGGER_PLAYERS;

   PostEvent( EV_ExplodingWall_Setup, EV_POSTSPAWN );
   }

/*****************************************************************************/
/*QUAKED trigger_teleport (0.5 0.5 0.5) ? VISIBLE x NOT_PLAYERS NOT_MONSTERS NOT_PROJECTILES NO_EFFECTS

Touching this entity will teleport players to the targeted object.

"key"						The item needed to activate this. (default nothing)

"teleportthread"		The thread that is run when the player is teleported

If NOT_PLAYERS is set, the teleporter does not teleport players
If NOT_MONSTERS is set, the teleporter does not teleport monsters
If NOT_PROJECTILES is set, the teleporter does not teleport projectiles (rockets, grenades, etc.)
If NO_EFFECTS is set, the special effect will not happen and the teleport will be instant

******************************************************************************/

#define NO_EFFECTS ( 1 << 5 )

Event EV_Teleporter_Teleport
	( 
	"teleport",
   EV_CODEONLY,
   "e",
   "entity",
   "Teleports the entity to destination.",
	EV_NORMAL
	);

Event EV_Teleporter_StopTeleport
	( 
	"stopteleport",
   EV_CODEONLY,
   "e",
   "entity",
   "Releases the entity at the end of the teleport.",
	EV_NORMAL
	);

Event EV_Teleporter_SetThread
	( 
	"teleportthread",
   EV_CODEONLY,
   "s",
   "thread_name",
   "Sets the thread to run when the player is teleported.",
	EV_NORMAL
	);

CLASS_DECLARATION( Trigger, Teleporter, "trigger_teleport" )
	{
	   { &EV_Trigger_Effect,					&Teleporter::StartTeleport },
		{ &EV_Teleporter_Teleport,				&Teleporter::Teleport },
		{ &EV_Teleporter_StopTeleport,			&Teleporter::StopTeleport },
		{ &EV_Teleporter_SetThread,				&Teleporter::SetThread },
		{ NULL, NULL }
	};

void Teleporter::SetThread
	(
	Event *ev
	)
	{
	teleport_label.SetThread( ev->GetValue( 1 ) );
	}

void Teleporter::StartTeleport
	(
	Event *ev
	)
	{
	Animate	*fx;
	Entity	*other;
	Event		*event;
	qboolean is_sentient;
	Vector	new_position;


	if ( in_use )
		return;

	in_use = true;

	other = ev->GetEntity( 1 );

	if ( !other )
		return;

	if ( spawnflags & NO_EFFECTS )
		{
		event = new Event( EV_Teleporter_Teleport );
		event->AddEntity( other );
		ProcessEvent( event );
		return;
		}

	if ( other->isSubclassOf( Sentient ) )
		is_sentient = true;
	else
		is_sentient = false;

	if ( is_sentient )
		{
		new_position = origin;
		new_position.z += mins.z;
		other->setOrigin( new_position );
		other->NoLerpThisFrame();
		}

	// Create the teleport special effect

	fx = new Animate;
	fx->setOrigin( other->origin );

	fx->NewAnim( "idle", EV_Remove );

	if ( is_sentient )
		{
		fx->setModel( "fx_bigteleport.tik" );
		//fx->Sound( "sound/environment/electric/singles/dimming.wav" );
		}
	else
		{
		fx->setModel( "fx_teleport2.tik" );
		}

	if ( is_sentient )
		{
		// Freeze the entity that went into the teleporter

		other->flags |= FL_IMMOBILE;
		other->takedamage = DAMAGE_NO;
		}

	// Make the entity teleport

	event = new Event( EV_Teleporter_Teleport );
	event->AddEntity( other );

	if ( is_sentient )
		PostEvent( event, 4 );
	else
		PostEvent( event, 0 );

	if ( is_sentient )
		{
		other->PostEvent( EV_CODEONLY, 2 );

		/* if ( !( spawnflags & NO_EFFECTS ) )
			{
			event = new Event( EV_Sound );
			event->AddString( "snd_teleport" );
			other->PostEvent( event, 2 );
			} */
		}
	}

void Teleporter::Teleport
	(
	Event *ev
	)

	{
	Entity		*dest;
	int			i;
	Entity		*other;
	Vector		mid;
	Animate		*fx;
	Event			*event;

	other = ev->GetEntity( 1 );

   if ( !other || ( other == world ) )
      return;

	dest = ( Entity * )G_FindTarget( NULL, Target() );
	if ( !dest )
		{
		warning( "Teleport", "Couldn't find destination\n" );
		return;
		}

	assert( dest );

	// unlink to make sure it can't possibly interfere with KillBox
	other->unlink();

	if ( other->isSubclassOf( Sentient ) )
		{
      other->origin = dest->origin + Vector( 0, 0, 1 );
		other->velocity = vec_zero;
		}
	else
		{
		mid = ( absmax - absmin ) * 0.5;
      other->origin = dest->origin + Vector( 0, 0, 1 );
      other->origin += mid;
		}

   // set angles
	other->setAngles( dest->angles );

   if ( other->client )
		{
		client = other->client;

		// clear the velocity and hold them in place briefly
		client->ps.pm_time = 100;
      client->ps.pm_flags |= PMF_RESPAWNED;

      // cut the camera on the client
      ( ( Player * )other )->CameraCut();

      for( i = 0; i < 3; i++ )
			{
			client->ps.delta_angles[ i ] = ANGLE2SHORT( dest->angles[ i ] - client->cmd_angles[ i ] );
			}

		VectorCopy( angles, client->ps.viewangles );
		}

	if ( dest->isSubclassOf( TeleporterDestination ) )
		{
		float len;

		len = other->velocity.length();
      //
      // give them a bit of a push
      //
      if ( len < 400 )
         len = 400;
		other->velocity = ( ( TeleporterDestination * )dest )->movedir * len;
		}

	// kill anything at the destination
	KillBox( other );

   other->setOrigin( other->origin );
   other->origin.copyTo( other->edict->s.origin2 );

	// Run the teleport thread if set

	if ( teleport_label.IsSet() && other->isSubclassOf( Player ) )
		{
		teleport_label.Execute( this );
		}

	// Skip effects if no_effects set

	if ( spawnflags & NO_EFFECTS )
		{
		Event *event = new Event( EV_Teleporter_StopTeleport );
		event->AddEntity( other );
		ProcessEvent( event );
		return;
		}

	// Spawn in effect

	fx = new Animate;

	fx->setOrigin( other->origin );

	fx->NewAnim( "idle", EV_Remove );

	if ( other->isSubclassOf( Sentient ) )
		{
		fx->setModel( "fx_bigteleport.tik" );
		}
	else
		{
		fx->setModel( "fx_teleport2.tik" );
		}

	event = new Event( EV_Teleporter_StopTeleport );
	event->AddEntity( other );

	if ( other->isSubclassOf( Sentient ) )
		PostEvent( event, 1.75 );
	else
		PostEvent( event, FRAMETIME );
	}

void Teleporter::StopTeleport
	(
	Event *ev
	)

	{
	Entity *other;

	other = ev->GetEntity( 1 );

	if ( other->isSubclassOf( Sentient ) )
		{
		other->flags &= ~FL_IMMOBILE;
		other->takedamage = DAMAGE_AIM;
		other->showModel();
		}

	//if ( !( spawnflags & NO_EFFECTS ) )
	//	other->Sound( "snd_teleport" );

	in_use = false;
	}

Teleporter::Teleporter()
	{
   if ( LoadingSavegame )
      {
      return;
      }

	if ( spawnflags & 1 )
		{
      PostEvent( EV_Show, EV_POSTSPAWN );
		}

	//respondto = spawnflags ^ ( TRIGGER_PLAYERS | TRIGGER_MONSTERS | TRIGGER_PROJECTILES );
	respondto = spawnflags ^ ( TRIGGER_PLAYERS );

	in_use = false;

	// Cache all needed stuff

	if ( !( spawnflags & NO_EFFECTS ) )
		{
		CacheResource( "models/fx_bigteleport.tik" );
		//CacheResource( "sound/environment/electric/singles/dimming.wav", this );
		CacheResource( "fx_teleport2.tik" );
		//CacheResource( "snd_teleport", this );
		}
	}

/*****************************************************************************/
/*QUAKED func_teleportdest (0 0.25 0.5) (-32 -32 0) (32 32 8)

Point trigger_teleport at these.

******************************************************************************/

CLASS_DECLARATION( Entity, TeleporterDestination, "func_teleportdest" )
{
	{ &EV_SetAngle,		&TeleporterDestination::SetMoveDir },
	{ NULL, NULL }
};

TeleporterDestination::TeleporterDestination()
	{
	movedir = G_GetMovedir( 0 );
	}

void TeleporterDestination::SetMoveDir
	(
	Event *ev
	)
	{
   float angle;

   angle = ev->GetFloat( 1 );
   movedir = G_GetMovedir( angle );
	setAngles( movedir.toAngles() );
   }

/*****************************************************************************/
/*QUAKED func_useanim (0 0.5 0) ? VISIBLE TOUCHABLE CONTINUOUS

This object allows you to place the player into a specific animation for the
purposes of using an object within the world->

This object should point at a func_useanimdest which contains specific
information about how the player is supposed to be posed.

"count" - how many times this should trigger (default -1, infinite)
"thread" - thread to fire when used
"triggertarget" - what to trigger when used.
"delay" - how long it takes to be re-triggered ( default 3 seconds )
"key" - item needed to activate this

VISIBLE - if this is checked the trigger itself will be visible
TOUCHABLE - if this is set we can activate the trigger by standing in it.
CONTINUOUS - if this is checked the thing will re-trigger continously, otherwise
it waits until the player has left the trigger field.

******************************************************************************/

Event EV_UseAnim_Reset
	( 
	"_reset",
   EV_CODEONLY,
   NULL,
   NULL,
   "Reset's the Use Anim after it has no longer been touched.",
	EV_NORMAL
	);

Event EV_UseAnim_Thread
	( 
	"setthread",
	EV_DEFAULT,
	"s",
	"label",
	"Sets which thread to use when this UseAnim is triggered.",
	EV_NORMAL
	);

Event EV_UseAnim_Count
	( 
	"count",
	EV_DEFAULT,
	"i",
	"newCount",
	"Sets how many times the UseAnim can be triggered.",
	EV_NORMAL
	);

Event EV_UseAnim_TriggerTarget
	( 
	"triggertarget",
	EV_DEFAULT,
	"s",
	"targetname",
	"Sets what should be triggered, when this UseAnim is triggered.",
	EV_NORMAL
	);

Event EV_UseAnim_SetAnim
   ( 
   "anim",
   EV_DEFAULT,
   "s",
   "animName",
   "set the animation to use for player.",
	EV_NORMAL
   );

Event EV_UseAnim_SetKey
   ( 
   "key",
   EV_DEFAULT,
   "s",
   "keyName",
   "set the key needed to make this UseAnim function.",
	EV_NORMAL
   );


Event EV_UseAnim_SetState
   ( 
   "state",
   EV_CHEAT,
   "s",
   "stateName",
   "set the state to use for the player.",
	EV_NORMAL
   );

Event EV_UseAnim_SetCamera
   ( 
   "camera",
   EV_DEFAULT,
   "s",
   "cameraPosition",
   "set the camera to use when in this animation.\n"
   "topdown, behind, front, side, behind_fixed, side_left, side_right",
	EV_NORMAL
   );

Event EV_UseAnim_SetNumLoops
   ( 
   "num_loops",
   EV_DEFAULT,
   "i",
   "loopCount",
   "set the number of times to loop an animation per use.",
	EV_NORMAL
   );

Event EV_UseAnim_SetDelay
   ( 
   "delay",
   EV_DEFAULT,
   "f",
   "delayTime",
   "how long it takes for the UseAnim to be retriggered once you leave it.",
	EV_NORMAL
   );


CLASS_DECLARATION( Entity, UseAnim, "func_useanim" )
	{
      { &EV_Use,					NULL },
	   { &EV_Touch,					&UseAnim::Touched },
	   { &EV_UseAnim_Reset,			&UseAnim::Reset },
	   { &EV_UseAnim_Thread,		&UseAnim::SetThread },
	   { &EV_UseAnim_TriggerTarget,	&UseAnim::SetTriggerTarget },
	   { &EV_UseAnim_Count,			&UseAnim::SetCount },
      { &EV_UseAnim_SetAnim,		&UseAnim::SetAnim },
      { &EV_UseAnim_SetState,		&UseAnim::SetState },
      { &EV_UseAnim_SetKey,			&UseAnim::SetKey },
      { &EV_UseAnim_SetNumLoops,	&UseAnim::SetNumLoops },
      { &EV_UseAnim_SetCamera,		&UseAnim::SetCamera },
		{ NULL, NULL }
	};

UseAnim::UseAnim()
	{
   if ( LoadingSavegame )
      {
      return;
      }

	setMoveType( MOVETYPE_NONE );

   anim = "stand_use";
   num_loops = 1;
   hideModel();

   //
   // make it not solid unless we want it touchable
   //
   if ( !( spawnflags & 2 ) )
      {
	   setSolidType( SOLID_BBOX );
      setContents( CONTENTS_BODY );
      }
   else
      {
   	setSolidType( SOLID_TRIGGER );
      edict->r.svFlags |= SVF_NOCLIENT;
      }

	// by default this can activated infinitely
   count = -1;
   // clear out the triggertarget
   triggertarget = "";
   // clear out the thread
   thread.Set( "" );
   // set the default delay
   delay = 3;
   // initialize the last time the door was triggered
   last_active_time = -delay;
   // initially its not active
   active = 0;
   // set the default camera to be side view
   camera = "behind";
   //
   // only make it visible if so desired
   //
	if ( spawnflags & 1 )
		{
      PostEvent( EV_Show, EV_POSTSPAWN );
		}
	}

void UseAnim::Touched
   (
   Event *ev
   )

	{
   Entity               *other;
   
   if ( active && ( !( spawnflags & 4 ) ) )
      {
   	CancelEventsOfType( EV_UseAnim_Reset );
      PostEvent( EV_UseAnim_Reset, 0.25f );
      return;
      }

   // don't retrigger to soon
   if ( level.time < last_active_time )
      return;

   other = ev->GetEntity( 1 );
   if ( other->isSubclassOf( Player ) )
      {
      ( ( Player * ) other )->TouchedUseAnim( this );
      }
   }

bool UseAnim::canBeUsed
   (
   Entity * activator
   )
   
	{
	Entity *dest;

   // if this is no longer usable, return false
   if ( !count )
      {
      return false;
      }

   // don't retrigger to soon
   if ( level.time < last_active_time )
      {
      return false;
      }

   if ( key.length() )
      {
      if ( !activator->isSubclassOf( Sentient ) )
         {
         return false;
         }
      if ( !( ( (Sentient *)activator )->HasItem( key.c_str() ) ) )
         {
         qboolean    setModel;
         Item        *item;
         ClassDef		*cls;
         str         dialog;

         cls = FindClass( key.c_str(), &setModel );
		   if ( !cls || !checkInheritance( "Item", cls->classname ) )
			   {
            gi.DPrintf( "No item named '%s'\n", key.c_str() );
			   return true;
			   }
		   item = ( Item * )cls->newInstance();
         if ( setModel )
            {
            item->setModel( key.c_str() );
            }
         item->CancelEventsOfType( EV_Item_DropToFloor );
	      item->CancelEventsOfType( EV_Remove );
         item->ProcessPendingEvents();
         dialog = item->GetDialogNeeded();
         if ( dialog.length() > 0 )
            {
            activator->Sound( dialog );
            }
         else
            {
            gi.centerprintf ( activator->edict, "You need the %s", item->getName().c_str() );
            }
         delete item;

         // don't retrigger for 5 seconds
         last_active_time = level.time + 5;
         return false;
         }
      else
         {
         return qtrue;
         }
      }

   if ( isSubclassOf( TouchAnim ) )
      {
      return true;
      }

	dest = ( Entity * )G_FindTarget( NULL, Target() );
	if ( !dest || !dest->isSubclassOf( UseAnimDestination ) )
		{
		warning( "UseAnim", "Couldn't find destination\n" );
		return false;
		}

   return true;
   }

bool UseAnim::GetInformation
   (
   Entity *activator,
   Vector *org, 
   Vector *angles, 
   str *animation, 
   int *loopcount,
   str *state,
   str *camera
   )

	{
	Entity		         *dest;
   UseAnimDestination   *uadest;

   // if this is no longer usable, return false
   if ( !count )
      {
      return false;
      }

   dest = ( Entity * )G_FindTarget( NULL, Target() );

	if ( !dest || !dest->isSubclassOf( UseAnimDestination ) )
		{
      // grab the information from this entity instead
      // set the destination origin
      *org = origin;
      // set the destination angles
      *angles = this->angles;
      // set the desination animation
      *animation = anim;
      // set the number of loops
      *loopcount = num_loops;
      // get the state if necessary
      *state = this->state;
      // set the camera
      *camera = this->camera;
		}
   else
      {
      uadest = ( UseAnimDestination * )dest;

      // set the destination origin
      *org = uadest->origin;
      // set the destination angles
      *angles = uadest->angles;
      // set the desination animation
      *animation = uadest->GetAnim();
      // set the number of loops
      *loopcount = uadest->GetNumLoops();
      // get the state if necessary
      *state = uadest->GetState();
      // set the camera
      *camera = this->camera;
      }


   // make this guy active
   active = qtrue;

   // if this is a TouchAnim see if it is linked to another TouchAnim
   if ( isSubclassOf( TouchAnim ) )
      {
      dest = NULL;
      do {
		  dest = ( Entity * )G_FindTarget( dest, Target() );
	      if ( dest )
            {
            if ( dest->isSubclassOf( UseAnim ) )
               {
               // make our linked UseAnim's active as well
               ( ( UseAnim * )dest )->active = qtrue;
               }
            }
         else
            {
            break;
            }
         } 
      while( 1 );
      }
   // 
   // decrement the use
   //
   if ( count > 0 )
      {
      count--;
      }

   return true;
   }

void UseAnim::TriggerTargets
   (
   Entity *activator
   )

	{
   //
   // fire off our trigger target if appropriate
   //
	if ( triggertarget.length() )
		{
      Event  *event;
      Entity *ent;
    
      ent = NULL;
		do
			{
			ent = ( Entity * )G_FindTarget( ent, triggertarget.c_str() );
			if ( !ent )
				{
				break;
				}
		   event = new Event( EV_Activate );
         event->AddEntity( activator );
			ent->PostEvent( event, 0 );
         }
      while ( 1 );
      }

   //
   // fire off a thread if necessary
   //
   if ( thread.IsSet() )
      {
	   thread.Execute( this );
      }
   }

void UseAnim::Reset
   (
   Event *ev
   )

	{
   //
   // find out if our triggertarget is of type door and only reset if the door is closed
   //
	if ( triggertarget.length() )
		{
      Entity *ent;
    
      ent = NULL;
		do
			{
			ent = ( Entity * )G_FindTarget( ent, triggertarget.c_str() );
			if ( !ent )
				{
				break;
				}
         if ( ent->isSubclassOf( Door ) )
            {
            if ( !( ( Door * )ent )->isCompletelyClosed() )
               {
   	         CancelEventsOfType( EV_UseAnim_Reset );
               PostEvent( EV_UseAnim_Reset, 0.25f );
               //
               // wait for a little bit
               //
               return;
               }
            }
         }
      while ( 1 );
      }

   active = qfalse;
   last_active_time = level.time + delay;

   // if this is a TouchAnim see if it is linked to another TouchAnim
   if ( isSubclassOf( TouchAnim ) )
      {
      Entity *dest;

      dest = NULL;
      do {
		  dest = ( Entity * )G_FindTarget( dest, Target() );
	      if ( dest )
            {
            if ( dest->isSubclassOf( UseAnim ) )
               {
               // make our linked UseAnim's reset as well
               ( ( UseAnim * )dest )->active = qfalse;
               ( ( UseAnim * )dest )->last_active_time = level.time + delay;
               }
            }
         else
            {
            break;
            }
         } 
      while( 1 );
      }
   }

void UseAnim::SetThread
   (
   Event *ev
   )

	{
	thread.SetThread( ev->GetValue( 1 ) );
   }

void UseAnim::SetDelay
   (
   Event *ev
   )

	{
   delay = ev->GetFloat( 1 );
   }

void UseAnim::SetTriggerTarget
   (
   Event *ev
   )

	{
   triggertarget = ev->GetString( 1 );
   }

void UseAnim::SetCount
   (
   Event *ev
   )

	{
   count = ev->GetInteger( 1 );
   }

void UseAnim::SetAnim
	(
	Event *ev
	)
	{
   anim = ev->GetString( 1 );
   }

void UseAnim::SetState
	(
	Event *ev
	)
	{
   state = ev->GetString( 1 );
   }

void UseAnim::SetKey
	(
	Event *ev
	)
	{
   key = ev->GetString( 1 );
   }

void UseAnim::SetCamera
	(
	Event *ev
	)
	{
   camera = ev->GetString( 1 );
   }

void UseAnim::SetNumLoops
	(
	Event *ev
	)
	{
   num_loops = ev->GetInteger( 1 );
   }


CLASS_DECLARATION( UseAnim, TouchAnim, "func_touchanim" )
	{
		{ NULL, NULL }
	};

TouchAnim::TouchAnim()
	{
   if ( LoadingSavegame )
      {
      return;
      }

   spawnflags |= 2;

   if ( spawnflags & 8 )
      {
      setSize( Vector( "-32 -32 0" ), Vector( "32 32 96" ) );
      }
   else
      {
      setSize( Vector( "-16 -16 0" ), Vector( "16 16 96" ) );
      }

  	setSolidType( SOLID_TRIGGER );
   edict->r.svFlags |= SVF_NOCLIENT;
	}

/*****************************************************************************/
/*       func_useanimdest (0 0.25 0.5) (0 0 0) (0 0 0)

Point func_useanim's at these.

The player will be lerped to this position and this orientation
and placed into the specified animation

"anim" specifies the animation that the player should be in.
"state" instead of an animation, sets a state the player should go into
"camera" camera position to use when player is in animation
"num_loops" number of animation loops to play.


******************************************************************************/

CLASS_DECLARATION( Entity, UseAnimDestination, "func_useanimdest" )
	{
      { &EV_UseAnim_SetAnim,		&UseAnimDestination::SetAnim },
      { &EV_UseAnim_SetState,		&UseAnimDestination::SetState },
      { &EV_UseAnim_SetNumLoops,	&UseAnimDestination::SetNumLoops },
		{ NULL, NULL }
	};

UseAnimDestination::UseAnimDestination()
	{
   if ( LoadingSavegame )
      {
      // Archive function will setup all necessary data
      return;
      }
   //
   // default animation to use
   //
   anim = "stand_use";
   num_loops = 1;
   setSolidType( SOLID_NOT );
   hideModel();
	}

void UseAnimDestination::SetAnim
	(
	Event *ev
	)
	{
   anim = ev->GetString( 1 );
   }

void UseAnimDestination::SetState
	(
	Event *ev
	)
	{
   state = ev->GetString( 1 );
   }

str UseAnimDestination::GetAnim
	(
   void
	)
	{
   return anim;
   }

str UseAnimDestination::GetState
	(
   void
	)
	{
   return state;
   }

void UseAnimDestination::SetNumLoops
	(
	Event *ev
	)
	{
   num_loops = ev->GetInteger( 1 );
   }

int UseAnimDestination::GetNumLoops
	(
   void
	)
	{
   return num_loops;
   }

Event EV_UseObject_MoveThread
	( 
	"move_thread",
	EV_DEFAULT,
	"s",
	"label",
	"Sets which move thread to use when this UseObject has finshed looping.",
	EV_NORMAL
	);

Event EV_UseObject_StopThread
	( 
	"stop_thread",
	EV_DEFAULT,
	"s",
	"label",
	"Sets which stop thread to use when this UseObject is finished.",
	EV_NORMAL
	);

Event EV_UseObject_ResetThread
	( 
	"reset_thread",
	EV_DEFAULT,
	"s",
	"label",
	"Sets which thread to call when resetting.",
	EV_NORMAL
	);

Event EV_UseObject_Count
	( 
	"count",
	EV_DEFAULT,
	"i",
	"newCount",
	"Sets how many times the UseObject can be triggered.",
	EV_NORMAL
	);

Event EV_UseObject_Cone
	( 
	"cone",
	EV_DEFAULT,
	"f",
	"newCone",
	"Sets the cone in angles of where the Useobject can be used.",
	EV_NORMAL
	);

Event EV_UseObject_Offset
	( 
	"offset",
	EV_DEFAULT,
	"v",
	"newOffset",
	"Sets the offset to use for this UseObject.",
	EV_NORMAL
	);

Event EV_UseObject_YawOffset
	( 
	"yaw_offset",
	EV_DEFAULT,
	"f",
	"newYawOffset",
	"Sets the yaw offset to use for this UseObject.",
	EV_NORMAL
	);

Event EV_UseObject_State
	( 
	"state",
	EV_DEFAULT,
	"s",
	"newState",
	"Sets the state to use for this UseObject.",
	EV_NORMAL
	);

Event EV_UseObject_StateBackwards
	( 
	"state_backwards",
	EV_DEFAULT,
	"s",
	"newState",
	"Sets the backward state to use for this UseObject.",
	EV_NORMAL
	);

Event EV_UseObject_TriggerTarget
	( 
	"triggertarget",
	EV_DEFAULT,
	"s",
	"targetname",
	"Sets what should be triggered, when this UseObject is triggered.",
	EV_NORMAL
	);

Event EV_UseObject_ResetTime
	( 
	"reset_time",
	EV_DEFAULT,
	"f",
	"newResetTime",
	"Sets the time it takes for the UseObject to reset itself.",
	EV_NORMAL
	);

Event EV_UseObject_DamageType
	( 
	"damage_type",
	EV_DEFAULT,
	"s",
	"newDamageType",
	"Sets what kind of damage is needed to activate the trigger.",
	EV_NORMAL
	);

Event EV_UseObject_Reset
	( 
	"_useobject_reset",
	EV_DEFAULT,
	NULL,
	NULL,
	"Resets the useobject to the start state after a certain amount of time.",
	EV_NORMAL
	);

Event EV_UseObject_Resetting
	( 
	"_useobject_resetting",
	EV_DEFAULT,
	NULL,
	NULL,
	"Intermediate function for useobject reset.",
	EV_NORMAL
	);

Event EV_UseObject_DamageTriggered
	( 
	"_useobject_damagetriggered",
	EV_DEFAULT,
   "e",
   "activator",
	"Intermediate function for when the useobject was triggered by damage.",
	EV_NORMAL
	);

Event EV_UseObject_Activate
	( 
	"activate",
	EV_DEFAULT,
   NULL,
   NULL,
   "Allow the useobject to be used.",
	EV_NORMAL
	);

Event EV_UseObject_Deactivate
	( 
	"deactivate",
	EV_DEFAULT,
   NULL,
   NULL,
   "Do not allow the useobject to be used.",
	EV_NORMAL
	);

Event EV_UseObject_UseMaterial
	( 
	"usematerial",
	EV_DEFAULT,
   "s",
   "nameOfUseMaterial",
   "the name of the material that glows when active.",
	EV_NORMAL
	);

Event EV_UseObject_SetActiveState
	( 
	"_setactivestate",
	EV_DEFAULT,
   NULL,
   NULL,
   "event that sets up the proper skin for the useobject.",
	EV_NORMAL
	);



#define MULTI_STATE     ( 1 << 0 )

/*****************************************************************************/
/*QUAKED func_useobject (0 0.5 0) ? MULTI-STATE

Allows you to setup a special object that places the player into a specific state
sequence.  Primarily used for levers and cranks.  

Object starts out in the "start" animation, when used the following occurs:

It is determined whether or not the player is in the right position to activate
the object, if it is, the player is moved to the exact offset and angle specified
by "offset" and "yaw_offset".  The right position is determined by a dot product
with "offset" and "yaw_offset".  The "cone" parameter controls the cone in which the
object can be triggered. Once the player is in the right position, the player is placed
into "state" and the "move" animation is played. Once the player animation ends, the
"move_thread" will be called.  If the use button is continued to be held down and count
is not finite, the animation will be continued to be played until the use key is held
down.  Once the use key is let go, the "stop" animation will be played on the lever and
the "stop_thread" will be called.

"activate" - turns the useobject on
"deactivate" - turns the useobject off
"offset" - vector offset of where the player should stand
"state" - state to go into when used
"state_backwards" - what state to use when reversing the object
"yaw_offset" - what direction the player should be facing when using the object
"cone" - the cone in which the object can be used
"count" - how many times this should trigger (default -1, infinite)
"move_thread" - thread that is fired when the object has cycled one animation
"stop_thread" - thread that is fired when the object has finished animating
"reset_thread" - thread that is fired when the object is resetting itself
"reset_time" - the time it takes for the object to reset, (default 0, it doesn't)
"triggertarget" - target to trigger when finished animating, if reset_time is set, target
will be fired again when resetting
"damage_type" - if set, can be triggered by using a weapon to activate it.  If set to "all",
any damage will activate it.

MULTI-STATE - the object has two different states and must be used each time to set the state
when multi state is active, the reset_thread is called instead of stop_thread.  All UseObjects
have two states on and off.  When reset_time is set, the object will automatically return to the 
off state after a preset amount of time.  When multi-state is set this must be done manually.

******************************************************************************/

CLASS_DECLARATION( Animate, UseObject, "func_useobject" )
	{
      { &EV_Use,						NULL },
	   { &EV_UseObject_MoveThread,		&UseObject::SetMoveThread },
	   { &EV_UseObject_StopThread,		&UseObject::SetStopThread },
	   { &EV_UseObject_ResetThread,		&UseObject::SetResetThread },
	   { &EV_UseObject_TriggerTarget,	&UseObject::SetTriggerTarget },
	   { &EV_UseObject_Offset,			&UseObject::SetOffset },
	   { &EV_UseObject_YawOffset,		&UseObject::SetYawOffset },
	   { &EV_UseObject_Count,			&UseObject::SetCount },
	   { &EV_UseObject_Cone,			&UseObject::SetCone },
	   { &EV_UseObject_State,			&UseObject::SetState },
	   { &EV_UseObject_StateBackwards,	&UseObject::SetBackwardsState },
	   { &EV_UseObject_ResetTime,		&UseObject::SetResetTime },
	   { &EV_UseObject_Reset,			&UseObject::Reset },
	   { &EV_UseObject_DamageType,		&UseObject::DamageType },
	   { &EV_UseObject_Resetting,		&UseObject::Resetting },
      { &EV_UseObject_DamageTriggered,	&UseObject::DamageTriggered },
		{ &EV_Damage,					&UseObject::DamageFunc },
      { &EV_UseObject_Activate,			&UseObject::ActivateEvent },
      { &EV_UseObject_Deactivate,		&UseObject::DeactivateEvent },
      { &EV_UseObject_UseMaterial,		&UseObject::UseMaterialEvent },
      { &EV_UseObject_SetActiveState,	&UseObject::SetActiveState },
		{ NULL, NULL }
	};

UseObject::UseObject()
	{
   Event * e;

   if ( LoadingSavegame )
      {
      // Archive function will setup all necessary data
      return;
      }

	setMoveType( MOVETYPE_NONE );
   setSolidType( SOLID_BBOX );
   setContents( CONTENTS_BODY );

   // make sure the bounds get rotated with the object
   flags |= FL_ROTATEDBOUNDS;
	// by default this can activated infinitely
   count = -1;
   // clear out the triggertarget
   triggertarget = "";
   // clear out the move thread
   move_thread.Set( "" );
   // clear out the stop thread
   stop_thread.Set( "" );
   // clear out the reset thread
   reset_thread.Set( "" );
   // clear out the damage type, by default useobjects do not respond to damage
   damage_type = MOD_NONE;
   // turn on damage taking ability
   takedamage = DAMAGE_YES;
   // give it some health
   health = 100;
   // set the default yaw offset
   yaw_offset = 0;
   // set the cone
   cone = cos( DEG2RAD( 90 ) );
   // set the state
   state = "";
   // set the backwards state
   state_backwards = "";
   // clear out the reset_time
   reset_time = 0;
   // clear out the object state
   objectState = 0;
   // the useobject is active by default
   active = qtrue;

   // start off in the start animation
   e = new Event( EV_SetAnim );
   e->AddString( "start" );
   PostEvent( e, 0 );
   PostEvent( EV_Show, 0 );
   // setup our skins once we are spawned
   PostEvent( EV_UseObject_SetActiveState, 0 );

	}

void UseObject::SetActiveState
   (
   Event *ev
   )
   {
   if ( !useMaterial.length() )
      {
      return;
      }

   if ( active && count )
      {
      SurfaceCommand( useMaterial.c_str(), "+skin1" );
      }
   else
      {
      SurfaceCommand( useMaterial.c_str(), "-skin1" );
      }
   if ( objectState )
      {
      SurfaceCommand( useMaterial.c_str(), "+skin2" );
      }
   else
      {
      SurfaceCommand( useMaterial.c_str(), "-skin2" );
      }
   }

void UseObject::SetMoveThread
   (
   Event *ev
   )

	{
	move_thread.SetThread( ev->GetValue( 1 ) );
   }

void UseObject::SetStopThread
   (
   Event *ev
   )

	{
	stop_thread.SetThread( ev->GetValue( 1 ) );
   }

void UseObject::SetResetThread
   (
   Event *ev
   )

	{
	reset_thread.SetThread( ev->GetValue( 1 ) );
   }

void UseObject::ActivateEvent
   (
   Event *ev
   )

	{
   active = qtrue;
   PostEvent( EV_UseObject_SetActiveState, 0 );
   }

void UseObject::DeactivateEvent
   (
   Event *ev
   )

	{
   active = qfalse;
   PostEvent( EV_UseObject_SetActiveState, 0 );
   }

void UseObject::SetTriggerTarget
   (
   Event *ev
   )

	{
   triggertarget = ev->GetString( 1 );
   }

void UseObject::SetOffset
   (
   Event *ev
   )

	{
   offset = ev->GetVector( 1 );
   }

void UseObject::SetYawOffset
   (
   Event *ev
   )

	{
   yaw_offset = ev->GetFloat( 1 );
   }

void UseObject::SetCount
   (
   Event *ev
   )

	{
   count = ev->GetInteger( 1 );
   }

void UseObject::SetCone
   (
   Event *ev
   )

	{
   cone = cos( DEG2RAD( ev->GetFloat( 1 ) ) );
   }

void UseObject::SetState
   (
   Event *ev
   )

	{
   state = ev->GetString( 1 );
   }

void UseObject::SetBackwardsState
   (
   Event *ev
   )

	{
   state_backwards = ev->GetString( 1 );
   }

void UseObject::UseMaterialEvent
   (
   Event *ev
   )

	{
   useMaterial = ev->GetString( 1 );
   }

void UseObject::SetResetTime
   (
   Event *ev
   )

	{
   reset_time = ev->GetFloat( 1 );
   }

void UseObject::Reset( Event *ev )
{
	NewAnim( "move_backward", EV_UseObject_Resetting );
}

void UseObject::Resetting
   (
   Event *ev
   )

	{
   SetActiveState( NULL );
   NewAnim( "start" );

   // reset the count
   count = 1;

   //
   // fire off our trigger target if appropriate
   //
	if ( triggertarget.length() )
		{
      Event  *event;
      Entity *ent;
    
      ent = NULL;
		do
			{
			ent = ( Entity * )G_FindTarget( ent, triggertarget.c_str() );
			if ( !ent )
				{
				break;
				}
		   event = new Event( EV_Activate );
         event->AddEntity( this );
			ent->PostEvent( event, 0 );
         }
      while ( 1 );
      }

   //
   // fire off a thread if necessary
   //
	if( reset_thread.IsSet() )
	{
		reset_thread.Execute( this );
	}
   }

bool UseObject::canBeUsed
   (
   Vector org,
   Vector dir
   )

	{
   float    dot;
   Vector   forward;
   Vector   diff;
   Vector   ang;

   // see if it is active
   if ( !active )
      {
      return false;
      }

   // if this is no longer usable, return false
   if ( !count )
      {
      return false;
      }

   // convert our yawoffset to a vector
   ang = vec_zero;
   ang[ YAW ] = angles[ YAW ] + yaw_offset;
   ang.AngleVectors( &forward );
   dot = forward * dir;
   if ( dot < cone )
      {
      return false;
      }

/*
   // convert our offset to a vector in worldspace
   forward = getLocalVector( offset );
   forward.normalize();
   diff = org - origin;
//   diff = origin - org;
   diff.normalize();
   dot = forward * dir;
   if ( dot < cone )
      {
      return false;
      }
*/

   return true;
   }

void UseObject::DamageFunc
	(
	Event *ev
	)

	{
	Event			*e;
   Entity		*attacker;
   int         mod;

   // if this is no longer usable, return false
   if ( !count )
      {
      return;
      }

   // what kind of damage hit us
   mod = ev->GetInteger( 9 );
   
   // if we don't respond to any kind of damage, and our damage types do not match, return
   if ( !MOD_matches( mod, damage_type ) )
      {
      return;
      }

   // get the attacker
   attacker = ev->GetEntity( 1 );

   // 
   // decrement the use
   //
   if ( count > 0 )
      {
      count--;
      }
   // setup our damage triggered event
   e = new Event( EV_UseObject_DamageTriggered );
   // add our attacker
   e->AddEntity( attacker );
   // start up the object with our special event
   Start( e );
   }

void UseObject::DamageTriggered
   (
   Event * ev
   )

	{
   // grab the attacker from our event
   Stop( ev->GetEntity( 1 ) );
   }

void UseObject::Setup
   (
   Entity *activator,
   Vector *org,
   Vector *ang,
   str    *newstate
   )

	{
   if ( ( spawnflags & MULTI_STATE ) && objectState )
      {
      *newstate = state_backwards;
      }
   else
      {
      *newstate = state;
      }

   // convert our offset to a vector in worldspace
   MatrixTransformVector( offset, orientation, *org );
   *org += origin;

   *ang = angles;
   ang->y += yaw_offset;

   // 
   // decrement the use
   //
   if ( count > 0 )
      {
      count--;
      }
   }

void UseObject::Start
   (
   Event * ev
   )

	{
   //
   // fire off the move_thread
   //
   if ( move_thread.IsSet() )
      {
	   move_thread.Execute( this );
      }

   if ( ( spawnflags & MULTI_STATE ) && objectState )
	{
		NewAnim( "move_backward", ev );
	}
	else
	{
		NewAnim( "move", ev );
	}

   SetActiveState( NULL );
   }

bool UseObject::Loop
   (
   void
   )

	{
   if ( !count )
      return qfalse;

   return qtrue;
   }

void UseObject::Stop
   (
   Entity *activator
   )

	{
	if( ( spawnflags & MULTI_STATE ) && objectState )
	{
		NewAnim( "start" );
	}
	else
	{
		NewAnim( "stop" );
	}

   //
   // fire off our trigger target if appropriate
   //
	if ( triggertarget.length() )
		{
      Event  *event;
      Entity *ent;
    
      ent = NULL;
		do
			{
			ent = ( Entity * )G_FindTarget( ent, triggertarget.c_str() );
			if ( !ent )
				{
				break;
				}
		   event = new Event( EV_Activate );
         event->AddEntity( activator );
			ent->PostEvent( event, 0 );
         }
      while ( 1 );
      }

   //
   // fire off a thread if necessary
   //
   if ( ( spawnflags & MULTI_STATE ) && objectState )
      {
      if ( reset_thread.IsSet() )
         {
		  reset_thread.Execute( this );
         }
      }
   else
      {
      if ( stop_thread.IsSet() )
         {
		  stop_thread.Execute( this );
         }
      }

   // toggle the state
   objectState ^= 1;

   if ( reset_time )
      {
      count = 0;
      PostEvent( EV_UseObject_Reset, reset_time );
      }

   SetActiveState( NULL );
   }

/*****************************************************************************/
/*QUAKED info_waypoint (0 0.5 0) (-8 -8 -8) (8 8 8)

Used as a positioning device for objects

******************************************************************************/

CLASS_DECLARATION( SimpleArchivedEntity, Waypoint, "info_waypoint" )
{
	{ NULL, NULL }
};

CLASS_DECLARATION( SimpleArchivedEntity, TempWaypoint, NULL )
{
	{ NULL, NULL }
};

/*****************************************************************************/
/*QUAKED info_vehiclepoint (0.15 0.5 0.25) (-8 -8 -8) (8 8 8) START_STOPPING START_SKIDDING STOP_SKIDDING

Like info_waypoints, but with spawnflags for vehicles.

******************************************************************************/

Event EV_VehiclePoint_SetSpawnFlags
	(
	"spawnflags",
	EV_DEFAULT,
	"i",
	"spawn_flags",
	"Sets the spawn flags."
	);

CLASS_DECLARATION( Waypoint, VehiclePoint, NULL )
{
	{ &EV_VehiclePoint_SetSpawnFlags,				&VehiclePoint::SetSpawnFlags },
	{ NULL, NULL }
};

void VehiclePoint::SetSpawnFlags
	(
	Event *ev
	)

{
	spawnflags = ev->GetInteger( 1 );
}

/*****************************************************************************/
/*QUAKED func_monkeybars (0.75 0.75 0.75) ?

Monkey bars

******************************************************************************/

CLASS_DECLARATION( Entity, MonkeyBars, "func_monkeybars" )
{
	{ &EV_SetAngle,			 &MonkeyBars::SetAngleEvent },
	{ NULL, NULL }
};

MonkeyBars::MonkeyBars()
	{
   if ( LoadingSavegame )
      {
      // Archive function will setup all necessary data
      return;
      }
	setMoveType( MOVETYPE_NONE );
   setContents( CONTENTS_SHOOTONLY | MASK_SOLID );
   PostEvent( EV_BecomeSolid, 0 );
   dir = 0;
	}

void MonkeyBars::SetAngleEvent
	(
	Event *ev
	)

	{
   dir = ev->GetFloat( 1 );
   }

/*****************************************************************************/
/*QUAKED func_horizontalpipe (0.75 0.75 0.75) ?

Horizontal pipe that play can crawl upside down on.

******************************************************************************/

CLASS_DECLARATION( Entity, HorizontalPipe, "func_horizontalpipe" )
	{
      { &EV_SetAngle,			         &HorizontalPipe::SetAngleEvent },
		{ NULL, NULL }
	};

HorizontalPipe::HorizontalPipe()
	{
   if ( LoadingSavegame )
      {
      // Archive function will setup all necessary data
      return;
      }
	setMoveType( MOVETYPE_NONE );
   setContents( CONTENTS_SHOOTONLY | MASK_SOLID );
   PostEvent( EV_BecomeSolid, 0 );
   dir = 0;
	}

void HorizontalPipe::SetAngleEvent
	(
	Event *ev
	)

	{
   dir = ev->GetFloat( 1 );
   }

/*****************************************************************************/
// TossObject
/*****************************************************************************/

Event EV_TossObject_SetBounceSound
   (
   "bouncesound",
   EV_DEFAULT,
   "s",
   "sound",
   "When bouncing, what sound to play on impact",
	EV_NORMAL
   );

Event EV_TossObject_SetBounceSoundChance
   (
   "bouncesoundchance",
   EV_DEFAULT,
   "f[0,1]",
   "chance",
   "When bouncing, the chance that the bounce sound will be played",
	EV_NORMAL
   );

CLASS_DECLARATION( Animate, TossObject, "TossObject" )
	{
      { &EV_Touch,								&TossObject::Touch },
      { &EV_Stop,								&TossObject::Stop },
      { &EV_TossObject_SetBounceSound,			&TossObject::SetBounceSound },
      { &EV_TossObject_SetBounceSoundChance,	&TossObject::SetBounceSoundChance },
      { NULL, NULL }
	};

TossObject::TossObject()
   {
   if ( LoadingSavegame )
      {
      // Archive function will setup all necessary data
      return;
      }
	setMoveType( MOVETYPE_GIB );
	setSolidType( SOLID_NOT );
   bouncesound        = "";
   bouncesoundchance  = 1.0f; 
   }

TossObject::TossObject( str model )
   {
	setMoveType( MOVETYPE_GIB );
	setSolidType( SOLID_NOT );
   bouncesound        = "";
   bouncesoundchance  = 1.0f; 
   setModel( model );
   }

void TossObject::SetBounceSound
   (
   str bounce
   )

   {
   bouncesound = bounce;
   }

void TossObject::SetBounceSound
   (
   Event *ev
   )

   {
   bouncesound = ev->GetString( 1 );
   }

void TossObject::SetBounceSoundChance
   (
   float chance
   )

   {
   bouncesoundchance = chance;
   }

void TossObject::SetBounceSoundChance
   (
   Event *ev
   )

   {
   bouncesoundchance = ev->GetFloat( 1 );
   }

void TossObject::Stop 
   (
   Event *ev 
   )

   {
   setMoveType( MOVETYPE_NONE );
   setSolidType( SOLID_NOT );
   // cancel the previous fade out command
   CancelEventsOfType( EV_FadeOut );
   PostEvent( EV_FadeOut, 7 + G_Random( 5 ) );
   setAngles( Vector( "0 0 0" ) );
   NewAnim( "landed" );
   }

void TossObject::Touch
	(
	Event *ev
	)

   {
   Entity * ent;

   ent = ev->GetEntity( 1 );

   // only touch the world
   if ( !ent || ( ent != world ) )
      {
      return;
      }
   //
   // every time we bounce try to go back to our nominal angles
   //
   setAngles( angles * 0.5f ); 

   if ( bouncesound.length() )
      {
      if ( G_Random( 1 ) < bouncesoundchance )
         {
         Sound( bouncesound );
         }
      }
   }

void TossObject::SetVelocity
	(
   float severity
	)

   {
   setSolidType( SOLID_BBOX );
   velocity[0] = 100.0 * crandom();
	velocity[1] = 100.0 * crandom();
	velocity[2] = 200.0 + 100.0 * random();

   avelocity = Vector( G_Random( 600 ), G_Random( 600 ), G_Random( 600 ) );

   velocity *= severity;

	if (velocity[0] < -400)
		velocity[0] = -400;
	else if (velocity[0] > 400)
		velocity[0] = 400;
	if (velocity[1] < -400)
		velocity[1] = -400;
	else if (velocity[1] > 400)
		velocity[1] = 400;
	if (velocity[2] < 200)
		velocity[2] = 200;	// always some upwards
	else if (velocity[2] > 600)
		velocity[ 2 ] = 600;

	NewAnim( "idle" );

   // we give it 8 seconds to fall, if not it will get faded out
   PostEvent( EV_FadeOut, 8 );
   }

/*****************************************************************************/
/*QUAKED func_pushobject (0.75 0.75 0.75) ?

Pushable object

"dmg" how much damage to cause when blocked.  (default 2)
"pushsound" Sound to play when object is pushed (default is none)

******************************************************************************/

Event EV_PushObject_Start
	( 
	"start",
	EV_DEFAULT,
	NULL,
	NULL,
	"Sets up the pushobject.",
	EV_NORMAL
	);

Event EV_PushObject_SetDamage
	( 
	"dmg",
	EV_DEFAULT,
	"i",
	"damage",
	"Set the damage.",
	EV_NORMAL
	);

Event EV_PushObject_SetPushSound
	( 
	"pushsound",
	EV_DEFAULT,
	"s",
	"sound",
	"Set the pushing sound",
	EV_NORMAL
	);

CLASS_DECLARATION( Entity, PushObject, "func_pushobject" )
	{
      { &EV_PushObject_Start,			&PushObject::Start },
	   { &EV_Blocked,					&PushObject::BlockFunc },
      { &EV_PushObject_SetDamage,		&PushObject::SetDamage },
      { &EV_PushObject_SetPushSound,	&PushObject::SetPushSound },
		{ NULL, NULL }
	};

PushObject::PushObject()
	{
   if ( LoadingSavegame )
      {
      // Archive function will setup all necessary data
      return;
      }
	dmg					= 2;
	attack_finished   = 0;

	pushsound = "object_slide";

   PostEvent( EV_PushObject_Start, EV_POSTSPAWN );
	}

void PushObject::SetPushSound
   (
   Event *ev
   )

   {
   pushsound = ev->GetString( 1 );
   }

void PushObject::Start
   (
   Event *ev
   )

   {
   // make sure that this touches triggers
	flags |= FL_TOUCH_TRIGGERS;
   edict->clipmask = MASK_SOLID;
   setSolidType( SOLID_BSP );
   setMoveType( MOVETYPE_PUSH );

   // fix the bounding box so that the object isn't stuck in the ground
   setSize( mins + Vector( 1, 1, 2 ), maxs - Vector( 1, 1, 1 ) );
   }

qboolean PushObject::canPush
   (
   Vector dir
   )

   {
   trace_t trace;

   Vector end( origin.x + dir.x, origin.y + dir.y, origin.z );

   trace = G_Trace( origin, mins, maxs, end, this, MASK_DEADSOLID, false, "PushObject::Push" );
   return ( !trace.startsolid && ( trace.fraction == 1.0f ) );
   }

qboolean PushObject::Push
   (
   Entity *pusher,
   Vector move
   )

   {
   trace_t trace;

   if ( pushsound.length() )
      {
      if ( !edict->s.loopSound )
         {
         LoopSound( pushsound );
         PostEvent( EV_StopLoopSound, level.frametime * 5 );
         }
      }

   Vector end( origin.x + move.x, origin.y + move.y, origin.z );

   trace = G_Trace( origin, mins, maxs, end, this, MASK_DEADSOLID, false, "PushObject::Push" );
   if ( !trace.startsolid && ( trace.fraction > 0 ) )
      {
      owner = pusher;

      G_PushMove( this, trace.endpos - origin, vec_zero );

      if ( edict->s.loopSound )
         PostEvent( EV_StopLoopSound, 0.f );

      return qtrue;
      }

   return qfalse;
   }

Entity *PushObject::getOwner
   (
   void
   )

   {
   return ( Entity * )owner;
   }

void PushObject::BlockFunc
	(
	Event *ev
	)

	{
	Entity *other;

	if ( ( dmg != 0 ) && ( level.time >= attack_finished ) )
		{
		attack_finished = level.time + 0.5f;
	   other = ev->GetEntity( 1 );
      if ( other != owner )
         {
         other->Damage( this, this, dmg, origin, vec_zero, vec_zero, 0, 0, MOD_CRUSH );
         }
		}
	}

void PushObject::SetDamage
	(
	Event *ev
	)

	{
	dmg = ev->GetInteger( 1 );
	}

#define SPAWN_AUTO_RESET   ( 1 << 0 )
#define NO_RANDOMNESS      ( 1 << 1 )
#define REMOVE_ON_GROUND   ( 1 << 2 )
/*****************************************************************************/
/*QUAKED func_fallingrock (0.75 0.75 0.75) ? AUTO_RESET NO_RANDOMNESS REMOVE_ON_GROUND

Creates a rock that, when triggered, begins falling and bounces along a path
specified by targetname.  Use info_waypoint for the path.

"targetname" the path to follow.
"dmg" how much damage to cause creatures it hits (default 20).
"speed" how fast to move (default 200).
"wait" how long to wait before falling when triggered (default 0).
"noise" sound to play when rock touches the world

AUTO_RESET - when done falling, automatically return to the start
NO_RANDOMNESS - don't use any randomness when making the rocks fall
REMOVE_ON_GROUND - remove the rocks when done

******************************************************************************/

Event EV_FallingRock_Bounce
	( 
	"bounce",
	EV_DEFAULT,
   NULL,
	NULL,
	"sent to entity when touched.",
	EV_NORMAL
	);

Event EV_FallingRock_Rotate
	( 
	"rotate",
	EV_DEFAULT,
   NULL,
	NULL,
	"rotates the falling rock.",
	EV_NORMAL
	);

Event EV_FallingRock_SetWait
	( 
	"wait",
	EV_DEFAULT,
   "f",
	"wait",
	"How long to wait before rock starts falling.",
	EV_NORMAL
	);

Event EV_FallingRock_Start
	( 
	"start",
	EV_DEFAULT,
   NULL,
	NULL,
	"Starts rock falling.",
	EV_NORMAL
	);

Event EV_FallingRock_SetDmg
	( 
	"dmg",
	EV_DEFAULT,
   "i",
   "dmg",
   "Set the damage from the rock.",
	EV_NORMAL
	);

Event EV_FallingRock_SetSpeed
	( 
	"speed",
	EV_DEFAULT,
   "f",
   "speed",
   "Set the speed that the rock moves at.",
	EV_NORMAL
	);

Event EV_FallingRock_SetBounceSound
   (
   "noise",
   EV_DEFAULT,
   "s",
   "sound",
   "Set the sound to play when the rock bounces",
	EV_NORMAL
   );

CLASS_DECLARATION( Entity, FallingRock, "func_fallingrock" )
	{
      { &EV_Activate,							&FallingRock::Activate },
      { &EV_Touch,								&FallingRock::Touch },
      { &EV_FallingRock_Bounce,					&FallingRock::Bounce },
      { &EV_FallingRock_Rotate,					&FallingRock::Rotate },
      { &EV_FallingRock_Start,					&FallingRock::StartFalling },
      { &EV_FallingRock_SetWait,				&FallingRock::SetWait },
      { &EV_FallingRock_SetSpeed,				&FallingRock::SetSpeed },
      { &EV_FallingRock_SetDmg,					&FallingRock::SetDmg },
      { &EV_FallingRock_SetBounceSound,			&FallingRock::SetBounceSound },
      { NULL, NULL }
	};

FallingRock::FallingRock()
   {
   if ( LoadingSavegame )
      {
      // Archive function will setup all necessary data
      return;
      }
   active = 0;
   current = NULL;
	setMoveType( MOVETYPE_NONE );
   wait = 0;
   dmg = 20;
   speed = 200;
   activator = NULL;
   attack_finished = 0;

   SetBounceSound( "impact_rock" );
   }

Entity *FallingRock::SetNextBounceDir
   (
   void
   )

   {
   Entity *ent;

   if ( !current->target.length() )
      {
      return NULL;
      }

   ent = ( Entity * )G_FindTarget( NULL, current->target.c_str() );
	if ( !ent )
		{
		gi.Error( ERR_DROP, "FallingRock :: Entity with targetname of '%s' not found", current->target.c_str() );
		}

   bounce_dir = ent->origin - current->origin;
   bounce_dir.normalize();

   return ent;
   }


void FallingRock::NextBounce
   (
   void
   )

   {
   float time;
   float distance;
   Vector delta, xydelta;
   float xy_speed;
   float vertical_speed;

   delta = current->origin - origin;
   xydelta = delta;
   xydelta.z = 0;
   xy_speed = speed;

   distance = xydelta.normalize();

   time = distance / xy_speed;

   if ( !( spawnflags & NO_RANDOMNESS ) )
      {
      if ( time > 1.0f )
         {
         time = 0.75f + G_Random( 1 );
         }

      if ( time < 0.4f )
         {
         time = 0.4f;
         }
      }

   vertical_speed = ( delta.z / time ) + ( 0.5f * gravity * sv_gravity->value * time );
   if ( vertical_speed < 0 )
      {
      vertical_speed = 0;
      }

   velocity = xydelta * xy_speed;
   velocity.z = vertical_speed;

   Vector ang( 0, vectoyaw( delta ), 0 );
   ang.AngleVectors( NULL, &rotateaxis );

   // make sure it leaves the ground
	groundentity = NULL;
   }

void FallingRock::Rotate
   (
   Event *ev
   )

   {
   float mat[ 3 ][ 3 ];
   float ang;

   ang = 360.0f * FRAMETIME;
   RotatePointAroundVector( mat[ 0 ], rotateaxis, orientation[ 0 ], ang );
   RotatePointAroundVector( mat[ 1 ], rotateaxis, orientation[ 1 ], ang );
   RotatePointAroundVector( mat[ 2 ], rotateaxis, orientation[ 2 ], ang );
   MatrixToEulerAngles( mat, angles );
   setAngles( angles );

   if ( velocity != vec_zero )
      {
      PostEvent( EV_FallingRock_Rotate, FRAMETIME );
      }
   }

void FallingRock::SetWait
   (
   Event *ev
   )

   {
   wait = ev->GetFloat( 1 );
   }

void FallingRock::SetSpeed
   (
   Event *ev
   )

   {
   speed = ev->GetFloat( 1 );
   }

void FallingRock::SetDmg
   (
   Event *ev
   )

   {
   dmg = ev->GetInteger( 1 );
   }

void FallingRock::SetBounceSound
	(
	str sound
	)

{
	bouncesound = sound;
	// cache the sound in
	CacheResource( bouncesound.c_str() );
}

void FallingRock::SetBounceSound
   (
   Event *ev
   )

   {
   SetBounceSound( ev->GetString( 1 ) );
   }

void FallingRock::Activate
   (
   Event *ev
   )

   {
   if ( active == 1 )
      return;

   if ( ( active == 2 ) && ( spawnflags & SPAWN_AUTO_RESET ) )
      {
      current = NULL;
      activator = NULL;
	   setMoveType( MOVETYPE_NONE );
      NoLerpThisFrame();
      setOrigin( start_origin );
      }

   activator = ev->GetEntity( 1 );

   if ( wait )
      {
      PostEvent( EV_FallingRock_Start, wait );
      }
   else
      {
      ProcessEvent( EV_FallingRock_Start );
      }
   }

void FallingRock::StartFalling
   (
   Event *ev
   )
   
   {
   if ( current )
      {
      return;
      }

   if ( !active )
      {
      start_origin = origin;
      }

   active = 1;
   setMoveType( MOVETYPE_BOUNCE );
   setSolidType( SOLID_BBOX );
   PostEvent( EV_FallingRock_Rotate, FRAMETIME );
   edict->clipmask = MASK_SOLID|CONTENTS_BODY;

   last_bounce_origin = origin;
   current = this;
   current = SetNextBounceDir();
   if ( current )
      {
      NextBounce();
      }
   }

void FallingRock::Touch
	(
	Event *ev
	)

   {
   Entity *other;

   other = ev->GetEntity( 1 );

   if ( other != world )
      {
   	if ( ( velocity != vec_zero ) && ( other->takedamage ) && ( level.time >= attack_finished ) )
	   	{
         other->Damage( this, activator, dmg, origin, vec_zero, vec_zero, 20, 0, MOD_THROWNOBJECT );
	   	attack_finished = level.time + FRAMETIME;
		   }
      }

   if ( !current || ( other != world ) )
      {
      return;
      }
   if ( bouncesound.length() )
      {
      Vector delta( origin - last_bounce_origin );

      if ( delta.length() > 8 )
         {
         last_bounce_origin = origin;
		   Sound( bouncesound.c_str(), CHAN_VOICE );
         }
      }

   // we have to wait to set the velocity since the physics code
   // will modify it when we return.
   PostEvent( EV_FallingRock_Bounce, 0 );
   }

void FallingRock::Bounce
	(
	Event *ev
	)

   {
   Vector delta;

   if ( !current )
      {
      return;
      }

   do
      {
      // check if we've passed the waypoint
      delta = origin - current->origin;
      if ( ( delta * bounce_dir ) >= ( -2 * edict->r.radius ) )       
         {
         // call any threads on the current waypoint
         if ( current->isSubclassOf( Trigger ) )
            {
            current->ProcessEvent( EV_Trigger_StartThread );
            }
         current = SetNextBounceDir();
         if ( !current )
            {
            velocity = vec_zero;
            if ( spawnflags & SPAWN_AUTO_RESET )
               {
               active = 2;
               }
            else if ( spawnflags & REMOVE_ON_GROUND )
               {
               PostEvent( EV_Remove, 0 );
               }
            break;
            }
         }
      else
         {
         NextBounce();
         break;
         }
      }
      while( 1 );
   }

/*****************************************************************************/
/*QUAKED func_ladder (0.75 0.75 0.75) ?

Ladder trigger volume

******************************************************************************/

Event EV_LadderSetup
(
	"_ladder_setup",
	EV_CODEONLY,
	NULL,
	NULL,
	"Does the post spawn setup of the ladder"
);

CLASS_DECLARATION( Entity, FuncLadder, "func_ladder" )
{
	{ &EV_Use,					NULL },
	{ &EV_Touch,				NULL },
	{ &EV_SetAngle,				&FuncLadder::SetLadderFacing },
	{ &EV_LadderSetup,			&FuncLadder::LadderSetup },
	{ NULL, NULL }
};

FuncLadder::FuncLadder()
{
	if( LoadingSavegame )
	{
		return;
	}

	PostEvent( EV_LadderSetup, EV_POSTSPAWN );
}

void FuncLadder::LadderSetup( Event *ev )
{
	setMoveType( MOVETYPE_PUSH );

	setContents( CONTENTS_LADDER );
	setSolidType( SOLID_BSP );

	// never send to clients
	edict->r.svFlags |= SVF_NOCLIENT;
}

qboolean FuncLadder::CanUseLadder( Entity *pUser )
{
	int iMask;
	Vector vDelta;
	Vector vUserFacing;
	Vector start, end;
	trace_t trace;

	vDelta = origin - pUser->origin;
	vDelta[ 2 ] = 0;

	if( vDelta.length() > 52.0f ) {
		return qfalse;
	}

	AngleVectorsLeft( pUser->angles, vUserFacing, NULL, NULL );

	iMask = pUser->client ? MASK_PLAYERSOLID : MASK_MONSTERSOLID;

	if( pUser->origin[ 2 ] + pUser->maxs[ 2 ] > absmax[ 2 ] )
	{
		if( DotProduct( vUserFacing, m_vFacingDir ) <= 0.15f )
		{
			vDelta = pUser->origin - origin;
			vDelta[ 2 ] = 0.0f;

			if( DotProduct( m_vFacingDir, vDelta ) >= 0.0f )
			{
				start = origin + m_vFacingDir * 26.0f;
				start[ 2 ] = absmax[ 2 ] + 16.0f;

				end = start - Vector( 0, 0, 16 );

				trace = G_Trace(
					start,
					pUser->mins,
					pUser->maxs,
					end,
					pUser,
					MASK_PLAYERSOLID,
					qfalse,
					"FuncLadder::CanUseLadder"
					);

				start = trace.endpos;
				trace = G_Trace(
					start,
					pUser->mins,
					pUser->maxs,
					start,
					pUser,
					MASK_PLAYERSOLID,
					qfalse,
					"FuncLadder::CanUseLadder"
					);

				if( !trace.startsolid )
				{
					// client can use ladder
					return qtrue;
				}
			}
		}

		return qfalse;
	}

	vDelta = ( pUser->origin - origin );
	vDelta[ 2 ] = 0.0f;

	if( DotProduct( vUserFacing, m_vFacingDir ) < -0.15f ||
		DotProduct( vDelta, m_vFacingDir ) > 0.0f )
	{
		return qfalse;
	}

	start = origin - m_vFacingDir * 29.0f;
	start[ 2 ] = absmin[ 2 ] + 16.0f;

	end = start;
	end[ 2 ] = start[ 2 ] - 16.0f;

	trace = G_Trace(
		start,
		pUser->mins,
		pUser->maxs,
		end,
		pUser,
		MASK_PLAYERSOLID,
		qfalse,
		"FuncLadder::CanUseLadder"
		);

	if( !trace.allsolid )
	{
		// client can use ladder
		return qtrue;
	}

	gi.DPrintf( "ladder start position is blocked by a solid object\n" );
	return qfalse;
}

void FuncLadder::PositionOnLadder( Entity *pUser )
{
	Vector vPos;
	Vector vStart;
	Vector vEnd;
	trace_t trace;

	if( pUser->origin[ 2 ] + pUser->maxs[ 2 ] > absmax[ 2 ] )
	{
		vEnd = origin + m_vFacingDir * 26.0f;
		vEnd[ 2 ] = absmax[ 2 ] + 2.0f;
	}
	else if( absmin[ 2 ] + 32.0f > pUser->origin[ 2 ] )
	{
		vEnd = origin - m_vFacingDir * 29.0f;
		vEnd[ 2 ] = absmin[ 2 ] + 2.0f;
	}
	else
	{
		vEnd = origin - m_vFacingDir * 16.0f;
		vEnd[ 2 ] = pUser->origin[ 2 ] + 8.0f;
	}

	vStart = vEnd;

	trace = G_Trace(
		vStart,
		pUser->mins,
		pUser->maxs,
		vEnd,
		pUser,
		MASK_PLAYERSOLID,
		qtrue,
		"FuncLadder::PositionOnLadder" );

	if( trace.allsolid )
	{
		vStart = pUser->origin;
		vStart[ 2 ] = pUser->origin[ 2 ] + 4.0f;

		vEnd = origin - m_vFacingDir * 16.0f;
		vEnd[ 2 ] = pUser->origin[ 2 ] + 4.0f;

		trace = G_Trace(
			vStart,
			pUser->mins,
			pUser->maxs,
			vEnd,
			pUser,
			MASK_PLAYERSOLID,
			qtrue,
			"FuncLadder::PositionOnLadder2" );
	}

	pUser->setOrigin( trace.endpos );
	pUser->setAngles( m_vFacingAngles );
}

void FuncLadder::SetLadderFacing( Event *ev )
{
	m_vFacingAngles = vec_zero;
	m_vFacingAngles[ 1 ] = AngleMod( ev->GetFloat( 1 ) );

	AngleVectorsLeft( m_vFacingAngles, m_vFacingDir, NULL, NULL );
}

void FuncLadder::AdjustPositionOnLadder( Entity *pUser )
{
	float fHeight;
	Vector vDelta;
	Vector vPos;
	Vector vStart;
	trace_t trace;

	vDelta = pUser->origin - origin;
	vDelta[ 2 ] = 0;

	fHeight = ( ( int )( pUser->origin[ 2 ] + 8.0f ) & ~15 );

	if( vDelta.length() > 16.0f && pUser->origin[ 2 ] == fHeight )
	{
		return;
	}

	vPos = origin - m_vFacingDir * 16.0f;
	vPos[ 2 ] = fHeight;

	vStart = vPos + m_vFacingDir * -16.0f;

	trace = G_Trace( vStart,
		pUser->mins,
		pUser->maxs,
		vPos,
		pUser,
		MASK_PLAYERSOLID,
		qtrue,
		"FuncLadder::AdjustPositionOnLadder" );

	if( trace.allsolid || trace.startsolid )
	{
		trace = G_Trace( pUser->origin,
			pUser->mins,
			pUser->maxs,
			vPos,
			pUser,
			MASK_PLAYERSOLID,
			qtrue,
			"FuncLadder::AdjustPositionOnLadder" );
	}

	pUser->setOrigin( trace.endpos );
}

void FuncLadder::EnsureOverLadder( Entity *pUser )
{
	float fHeight = absmax[ 2 ];
	Vector vStart;
	Vector vPos;
	trace_t trace;

	if( pUser->origin[ 2 ] - fHeight >= 8.0f ) {
		return;
	}

	vPos = pUser->origin;
	vPos[ 2 ] = fHeight + 16.0f;

	vStart = pUser->origin + m_vFacingDir * -16.0f;

	trace = G_Trace( vStart,
		pUser->mins,
		pUser->maxs,
		vPos,
		pUser->edict,
		MASK_PLAYERSOLID,
		true,
		"FuncLadder::EnsureOverLadder" );

	if( trace.startsolid || trace.allsolid )
	{
		trace = G_Trace( origin,
			pUser->mins,
			pUser->maxs,
			vPos,
			pUser->edict,
			MASK_PLAYERSOLID,
			true,
			"FuncLadder::EnsureOverLadder" );
	}

	pUser->setOrigin( trace.endpos );
}

void FuncLadder::EnsureForwardOffLadder( Entity *pUser )
{
	int iMask;
	Vector vStart;
	Vector vPos;
	Vector vDelta;
	trace_t trace;

	if( pUser->origin[ 2 ] - absmax[ 2 ] >= 8.0f ) {
		return;
	}

	iMask = pUser->client ? MASK_PLAYERSOLID : MASK_MONSTERSOLID;

	vPos = pUser->origin;
	vPos[ 2 ] = absmax[ 2 ] + 16.0f;

	vStart = vPos + m_vFacingDir * -16.0f;

	trace = G_Trace(
		vStart,
		pUser->mins,
		pUser->maxs,
		vPos,
		pUser,
		iMask,
		qtrue,
		"FuncLadder::EnsureOverLadder" );

	if( trace.allsolid || trace.startsolid )
	{
		trace = G_Trace(
			pUser->origin,
			pUser->mins,
			pUser->maxs,
			vPos,
			pUser,
			iMask,
			qtrue,
			"FuncLadder::EnsureOverLadder" );
	}

	pUser->setOrigin( trace.endpos );
}
