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

// spawners.cpp: Various spawning entities

/*****************************************************************************/
/*QUAKED func_spawn(0 0.25 0.5) (-8 -8 -8) (8 8 8)
"modelname" The name of the TIKI file you wish to spawn. (Required)
"spawntargetname" This will be the targetname of the spawned model. (default is null)
"spawntarget" This will be the target of the spawned model. (default is null)
"pickup_thread" passed on to the spawned model
"key"       The item needed to activate this. (default nothing)
"attackmode" Attacking mode of the spawned actor (default 0)
******************************************************************************/

#include "spawners.h"
#include "game.h"

Event EV_Spawn_ModelName
	( 
	"modelname",
	EV_DEFAULT,
   "s",
   "model_name",
   "Sets the model name for this spawn entity.",
   EV_NORMAL
	);
Event EV_Spawn_SpawnTargetName
	( 
	"spawntargetname",
	EV_DEFAULT,
   "s",
   "spawntargetname",
   "Sets spawn target name for this spawn entity.",
   EV_NORMAL
	);
Event EV_Spawn_SpawnTarget
	( 
	"spawntarget",
	EV_DEFAULT,
   "s",
   "spawntarget",
   "Sets spawn target for this spawn entity.",
   EV_NORMAL
	);
Event EV_Spawn_AttackMode
	( 
	"attackmode",
	EV_DEFAULT,
   "i",
   "attackmode",
   "Sets the attackmode for this spawn entity.",
   EV_NORMAL
	);

Event EV_Spawn_PickupThread
	( 
	"pickup_thread",
	EV_DEFAULT,
   "s",
   "threadName",
   "Sets the pickup thread for the spawned entity.",
   EV_NORMAL
	);

Event EV_Spawn_AddSpawnItem
	( 
	"spawnitem",
	EV_DEFAULT,
	"s",
	"spawn_item_name",
	"Adds this named item to what will be spawned when this spawned entity is killed, if it is an actor.",
	EV_NORMAL
	);
Event EV_Spawn_SetSpawnChance
	( 
	"spawnchance",
	EV_DEFAULT,
	"f",
	"spawn_chance",
	"Sets the chance that this spawned entity will spawn something when killed, if it is an actor.",
	EV_NORMAL
	);


CLASS_DECLARATION( ScriptSlave, Spawn, "func_spawn" )
	{
      { &EV_Activate,					&Spawn::DoSpawn },
      { &EV_Spawn_ModelName,			&Spawn::ModelName },
      { &EV_Spawn_SpawnTargetName,		&Spawn::SpawnTargetName },
      { &EV_Spawn_AttackMode,			&Spawn::AttackMode },
      { &EV_Spawn_SpawnTarget,			&Spawn::SpawnTarget },
      { &EV_Spawn_PickupThread,			&Spawn::SetPickupThread },
		{ &EV_SetAngle,					&Spawn::SetAngleEvent },
		{ &EV_Spawn_AddSpawnItem,		&Spawn::SetSpawnItem },
		{ &EV_Spawn_SetSpawnChance,		&Spawn::SetSpawnChance },
      { NULL, NULL }
	};

void Spawn::SetAngleEvent
	(
	Event *ev
	)
	{
	Entity::SetAngleEvent( ev );
   }

void Spawn::SetPickupThread
	(
	Event *ev
	)
	{
	pickup_thread = ev->GetString( 1 );
   }

void Spawn::ModelName
   (
   Event *ev
   )

   {
   modelname = ev->GetString( 1 );
   CacheResource( modelname );
   }

void Spawn::SpawnTargetName
   (
   Event *ev
   )

   {
   spawntargetname = ev->GetString( 1 );
   }

void Spawn::SpawnTarget
   (
   Event *ev
   )

   {
   spawntarget = ev->GetString( 1 );
   }

void Spawn::AttackMode
   (
   Event *ev
   )

   {
   attackmode = ev->GetInteger( 1 );
   }

void Spawn::SetSpawnItem
   (
   Event *ev
   )

   {
   spawnitem = ev->GetString( 1 );
   }

void Spawn::SetSpawnChance
   (
   Event *ev
   )

   {
   spawnchance = ev->GetFloat( 1 );
   }

Spawn::Spawn()
	{
	setSolidType( SOLID_NOT );
	setMoveType( MOVETYPE_NONE );
   hideModel();

   spawnchance = 0;
   attackmode = 0;
   }

void Spawn::SetArgs
   ( 
   SpawnArgs &args 
   )
   {
   args.setArg( "origin",     va( "%f %f %f", origin[ 0 ], origin[ 1 ], origin[ 2 ] ) );
   args.setArg( "angle",      va( "%f", angles[ 1 ] ) );
   args.setArg( "angles",     va( "%f %f %f", angles[ 0 ], angles[ 1 ], angles[ 2 ] ) );
   args.setArg( "model",      modelname.c_str() );
   args.setArg( "attackmode", va( "%i",attackmode ) );
   args.setArg( "scale",      va( "%f",edict->s.scale ) );
   if ( spawntargetname.length() )
      {
      args.setArg( "targetname", spawntargetname.c_str() );
      }
   if ( spawntarget.length() )
      {
      args.setArg( "target",     spawntarget.c_str() );
      }
   if ( pickup_thread.length() )
      {
      args.setArg( "pickup_thread", pickup_thread.c_str() );
      }
   if ( spawnitem.length() )
      {
      args.setArg( "spawnitem", spawnitem.c_str() );
      args.setArg( "spawnchance", va( "%f", spawnchance ) );
      }
   }

void Spawn::DoSpawn
   ( 
   Event *ev 
   )
   {
   Entity *spawn;
   SpawnArgs args;

   if ( !modelname.length() )
      {
      warning("Spawn", "modelname not set" );
      }

   SetArgs( args );

   spawn = ( Entity * )args.Spawn();

   if ( spawn )
      {
      // make sure spawned entity starts falling if necessary
	   spawn->velocity = "0 0 -1";

		Event *e = new Event( EV_SetAnim );
		e->AddString( "idle" );
		spawn->PostEvent( e, EV_SPAWNARG );
      }
   }

/*****************************************************************************/
/*QUAKED func_randomspawn(0 0.25 0.5) (-8 -8 -8) (8 8 8) START_OFF
Randomly spawns an entity.  The time between spawns is determined by min_time and max_time
The entity can be turned off and on by triggering it
"modelname"   The name of the TIKI file you wish to spawn. (Required)
"key"         The item needed to activate this. (default nothing)
"min_time" The minimum time between spawns (default 0.2 seconds)
"max_time" The maximum time between spawns (default 1 seconds)
START_OFF - spawn is off by default
******************************************************************************/

Event EV_RandomSpawn_MinTime
	( 
	"min_time",
	EV_DEFAULT,
   "f",
   "minTime",
   "Minimum time between random spawns.",
   EV_NORMAL
	);

Event EV_RandomSpawn_MaxTime
	( 
	"max_time",
	EV_DEFAULT,
   "f",
   "maxTime",
   "Maximum time between random spawns.",
   EV_NORMAL
	);

Event EV_RandomSpawn_Think
	( 
	"_randomspawn_think",
	EV_DEFAULT,
   NULL,
   NULL,
   "The function that actually spawns things in.",
   EV_NORMAL
	);

CLASS_DECLARATION( Spawn, RandomSpawn, "func_randomspawn" )
	{
      { &EV_Activate,						&RandomSpawn::ToggleSpawn },
      { &EV_RandomSpawn_MinTime,			&RandomSpawn::MinTime },
      { &EV_RandomSpawn_MaxTime,			&RandomSpawn::MaxTime },
      { &EV_RandomSpawn_Think,				&RandomSpawn::Think },
      { NULL, NULL }
	};

RandomSpawn::RandomSpawn()
   {
   min_time = 0.2f;
   max_time = 1.0f;
   if ( !LoadingSavegame && !( spawnflags & 1 ) )
      {
      PostEvent( EV_RandomSpawn_Think, min_time + ( G_Random( max_time - min_time ) ) );
      }
   }

void RandomSpawn::MinTime
   (
   Event *ev
   )

   {
   min_time = ev->GetFloat( 1 );
   }

void RandomSpawn::MaxTime
   (
   Event *ev
   )

   {
   max_time = ev->GetFloat( 1 );
   }

void RandomSpawn::ToggleSpawn
   (
   Event *ev
   )

   {
   if ( EventPending( EV_RandomSpawn_Think ) )
      {
      // if currently on, turn it off
      CancelEventsOfType( EV_RandomSpawn_Think );
      }
   else
      {
      Think( NULL );
      }
   }

void RandomSpawn::Think( Event *ev )
   {
   CancelEventsOfType( EV_RandomSpawn_Think );

   //
   // spawn our entity
   //
   DoSpawn( NULL );

   // 
   // post the next time
   //
   PostEvent( EV_RandomSpawn_Think, min_time + ( G_Random( max_time - min_time ) ) );
   }

/*****************************************************************************/
/*QUAKED func_respawn(0 0.25 0.5) (-8 -8 -8) (8 8 8)
When the thing that is spawned is killed, this func_respawn will get
triggered.
"modelname"   The name of the TIKI file you wish to spawn. (Required)
"key"         The item needed to activate this. (default nothing)
******************************************************************************/

CLASS_DECLARATION( Spawn, ReSpawn, "func_respawn" )
	{
      { NULL, NULL }
	};

void ReSpawn::DoSpawn( Event *ev )
   {
   Entity      *spawn;
   SpawnArgs   args;

   SetArgs( args );

   // This will trigger the func_respawn when the thing dies
   args.setArg( "targetname", TargetName() );
   args.setArg( "target", TargetName() );

   spawn = ( Entity * )args.Spawn();
   if ( spawn )
      {
      // make sure spawned entity starts falling if necessary
	   spawn->velocity = "0 0 -1";
      }
   }

/*****************************************************************************/
/*QUAKED func_spawnoutofsight(0 0.25 0.5) (-8 -8 -8) (8 8 8)
Will only spawn something out of sight of its targets.
"modelname"   The name of the TIKI file you wish to spawn. (Required)
"spawntargetname" This will be the targetname of the spawned model. (default is null)
"spawntarget" This will be the target of the spawned model. (default is null)
"key"         The item needed to activate this. (default nothing)
******************************************************************************/

CLASS_DECLARATION( Spawn, SpawnOutOfSight, "func_spawnoutofsight" )
	{
      { NULL, NULL }
	};

void SpawnOutOfSight::DoSpawn
	(
	Event *ev
	)

   {
   int         i;
   Entity	   *ent;
   gentity_t   *ed;
   trace_t     trace;
   qboolean    seen = false;

   // Check to see if I can see any players before spawning
 	for( i = 0; i < game.maxclients; i++ )
      {
      ed = &g_entities[ i ];
		if ( !ed->inuse || !ed->entity )
			{
			continue;
			}

		ent = ed->entity;
		if( ( ent->health < 0 ) || ( ent->flags & FL_NOTARGET ) )
			{
			continue;
			}

      trace = G_Trace( origin, vec_zero, vec_zero, ent->centroid, this, MASK_OPAQUE, false, "SpawnOutOfSight::DoSpawn" );
      if ( trace.fraction == 1.0 )
         {
         seen = true;
         break;
         }
      }

   if ( seen )
      {
      return;
      }

   Spawn::DoSpawn( ev );
   }


/*****************************************************************************/
/*QUAKED func_spawnchain(0 0.25 0.5) (-8 -8 -8) (8 8 8)
Tries to spawn something out of the sight of players.  If it fails, it will
trigger its targets.
"modelname"   The name of the TIKI file you wish to spawn. (Required)
"spawntargetname" This will be the targetname of the spawned model. (default is null)
"spawntarget" This will be the target of the spawned model. (default is null)
"key"         The item needed to activate this. (default nothing)
******************************************************************************/

CLASS_DECLARATION( Spawn, SpawnChain, "func_spawnchain" )
	{
      { NULL, NULL }
	};

void SpawnChain::DoSpawn
	(
	Event *ev
	)

   {
   int         i;
   Entity	   *ent;
   gentity_t   *ed;
   trace_t     trace;
   qboolean    seen = false;
   const char  *name;
   Event       *event;

   // Check to see if this can see any players before spawning
 	for( i = 0; i < game.maxclients; i++ )
      {
      ed = &g_entities[ i ];
		if ( !ed->inuse || !ed->entity )
			{
			continue;
			}

		ent = ed->entity;
		if( ( ent->health < 0 ) || ( ent->flags & FL_NOTARGET ) )
			{
			continue;
			}

      trace = G_Trace( origin, vec_zero, vec_zero, ent->centroid, this, MASK_OPAQUE, false, "SpawnChain::DoSpawn" );
      if ( trace.fraction == 1.0 )
         {
         seen = true;
         break;
         }
      }

   // Couldn't spawn anything, so activate targets
   if ( seen )
      {
   	name = Target();
	   if ( name && strcmp( name, "" ) )
		   {
         ent = NULL;
		   do
			   {
			   ent = ( Entity * )G_FindTarget( ent, name );
			   if ( !ent )
				   {
				   break;
				   }
		   	event = new Event( EV_Activate );
            event->AddEntity( world );
			   ent->PostEvent( event, 0 );
            } while ( 1 );
         }
      return;
      }

   Spawn::DoSpawn( ev );
   }
