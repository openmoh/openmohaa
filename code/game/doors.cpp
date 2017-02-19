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

// doors.cpp: Doors are environment objects that rotate open when activated by triggers
// or when used by the player.
//

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "mover.h"
#include "doors.h"
#include "sentient.h"
#include "scriptmaster.h"
#include "item.h"
#include "actor.h"
#include "player.h"

Event EV_Door_OpenStartSound
(
	"sound_open_start",
	EV_DEFAULT,
	"s",
	"sound_open_start",
	"Sets the sound to use when the door starts to opens."
);
Event EV_Door_OpenEndSound
(
	"sound_open_end",
	EV_DEFAULT,
	"s",
	"sound_open_end",
	"Sets the sound to use when the door stops to opens."
);
Event EV_Door_CloseStartSound
(
	"sound_close_start",
	EV_DEFAULT,
	"s",
	"sound_close",
	"Sets the sound to use when the door closes."
);
Event EV_Door_CloseEndSound
(
	"sound_close_end",
	EV_DEFAULT,
	"s",
	"sound_close",
	"Sets the sound to use when the door closes."
);
Event EV_Door_MessageSound
	( 
	"sound_message",
	EV_DEFAULT,
   "s",
   "sound_message",
   "Sets the sound to use when the door displays a message.",
	EV_NORMAL
	);
Event EV_Door_LockedSound
	( 
	"sound_locked",
	EV_DEFAULT,
   "s",
   "sound_locked",
   "Sets the sound to use when the door is locked.",
	EV_NORMAL
	);
Event EV_Door_SetWait
	( 
	"wait",
	EV_DEFAULT,
   "f",
   "wait",
   "Sets the amount of time to wait before automatically shutting.",
	EV_NORMAL
	);
Event EV_Door_SetDmg
	( 
	"dmg",
	EV_DEFAULT,
   "i",
   "damage",
   "Sets the amount of damage the door will do to entities that get stuck in it.",
	EV_NORMAL
	);
Event EV_Door_TriggerFieldTouched
	( 
	"door_triggerfield",
	EV_DEFAULT,
   "e",
   "other",
   "Is called when a doors trigger field is touched.",
	EV_NORMAL
	);
Event EV_Door_TryOpen
	( 
	"tryToOpen",
	EV_DEFAULT,
   "e",
   "other",
   "Tries to open the door.",
	EV_NORMAL
	);
Event EV_Door_Close
	( 
	"close",
	EV_DEFAULT,
   NULL,
   NULL,
   "Closes the door.",
	EV_NORMAL
	);
Event EV_Door_Open
	( 
	"open",
	EV_DEFAULT,
   "e",
   "other",
   "Opens the door.",
	EV_NORMAL
	);
Event EV_Door_DoClose
	( 
	"doclose",
	EV_DEFAULT,
   NULL,
   NULL,
   "Closes the door (special doors).",
	EV_NORMAL
	);
Event EV_Door_DoOpen
	( 
	"doopen",
	EV_DEFAULT,
   "e",
   "other",
   "Opens the door (special doors).",
	EV_NORMAL
	);
Event EV_Door_CloseEnd
	( 
	"doorclosed",
	EV_DEFAULT,
   NULL,
   NULL,
   "Called when the door finishes closing.",
	EV_NORMAL
	);
Event EV_Door_OpenEnd
	( 
	"dooropened",
	EV_DEFAULT,
   NULL,
   NULL,
   "Called when the door finishes opening.",
	EV_NORMAL
	);
Event EV_Door_Fire
	( 
	"toggledoor",
	EV_DEFAULT,
   "e",
   "other",
   "Toggles the state of the door (open/close).",
	EV_NORMAL
	);
Event EV_Door_Link
	( 
	"linkdoor",
	EV_DEFAULT,
   NULL,
   NULL,
   "Link doors together.",
	EV_NORMAL
	);
Event EV_Door_SetTime
	( 
	"time",
	EV_DEFAULT,
   "f",
   "traveltime",
   "Sets the time it takes for the door to open an close.",
	EV_NORMAL
	);
Event EV_Door_Lock
	(
	"lock",
	EV_DEFAULT,
	NULL,
	NULL,
	"Lock the door.",
	EV_NORMAL
	);
Event EV_Door_Unlock
	(
	"unlock",
	EV_DEFAULT,
	NULL,
	NULL,
	"Unlock the door.",
	EV_NORMAL
	);
Event EV_Door_TrySolid
	(
	"trysolid",
	EV_DEFAULT,
	NULL,
	NULL,
	"Trys to make the door solid.",
	EV_NORMAL
	);
Event EV_Door_AlwaysAway
	(
	"alwaysaway",
	EV_DEFAULT,
	"B",
	"bAlwaysAway",
	"Makes the door always open away from the person opening it.",
	EV_NORMAL
	);
Event EV_Door_DoorType
	(
	"doortype",
	EV_DEFAULT,
	"s",
	"door_type",
	"Sets the defaults for this door\n"
	"Door Defaults: wood(default)\n"
	"metal"
	);

#define DOOR_START_OPEN			1
#define DOOR_OPEN_DIRECTION		2
#define DOOR_DONT_LINK			4
#define DOOR_TOGGLE				32
#define DOOR_AUTO_OPEN			64
#define DOOR_TARGETED 			128

#define STATE_OPEN		1
#define STATE_OPENING	2
#define STATE_CLOSING   3
#define STATE_CLOSED	4

/*

Doors are similar to buttons, but can spawn a fat trigger field around them
to open without a touch, and they link together to form simultanious
double/quad doors.

Door.master is the master door.  If there is only one door, it points to itself.
If multiple doors, all will point to a single one.

Door.enemy chains from the master door through all doors linked in the chain.

*/

CLASS_DECLARATION( ScriptSlave, Door, "func_door" )
{
	{ &EV_Door_OpenStartSound,				&Door::SetOpenStartSound },
	{ &EV_Door_OpenEndSound,				&Door::SetOpenEndSound },
	{ &EV_Door_CloseStartSound,				&Door::SetCloseStartSound },
	{ &EV_Door_CloseEndSound,				&Door::SetCloseEndSound },
	{ &EV_Door_MessageSound,				&Door::SetMessageSound },
	{ &EV_Door_LockedSound,					&Door::SetLockedSound },
	{ &EV_Door_SetWait,						&Door::SetWait },
	{ &EV_Door_SetDmg,						&Door::SetDmg },
	{ &EV_Door_TriggerFieldTouched,			&Door::FieldTouched },
	{ &EV_Trigger_Effect,					&Door::TryOpen },
	{ &EV_Activate,							&Door::TryOpen },
	{ &EV_Door_TryOpen,						&Door::TryOpen },
	{ &EV_Door_Close,						&Door::Close },
	{ &EV_Door_Open,						&Door::Open },
	{ &EV_Door_CloseEnd,					&Door::CloseEnd },
	{ &EV_Door_OpenEnd,						&Door::OpenEnd },
	{ &EV_Door_Fire,						&Door::DoorFire },
	{ &EV_Door_Link,						&Door::LinkDoors },
	{ &EV_Door_SetTime,						&Door::SetTime },
	{ &EV_Use,								&Door::DoorUse },
	{ &EV_Killed,							&Door::DoorFire },
	{ &EV_Blocked,							&Door::DoorBlocked },
	{ &EV_Door_Lock,						&Door::LockDoor },
	{ &EV_Door_Unlock,						&Door::UnlockDoor },
	{ &EV_SetAngle,							&Door::SetDir },
	{ &EV_Door_TrySolid,					&Door::EventTrySolid },
	{ &EV_Door_AlwaysAway,					&Door::EventAlwaysAway },
	{ &EV_Door_DoorType,					&Door::EventDoorType },
	{ &EV_Touch,							NULL },
	{ NULL, NULL }
};

Door::Door()
	{
	float t;

   entflags |= EF_DOOR;
   if ( LoadingSavegame )
      {
      return;
      }

	nextdoor = 0;
	trigger = 0;
   locked = false;
   master = this;
   lastblocktime = 0;
   diropened = 0;
   m_bAlwaysAway = true;

   dir = G_GetMovedir( 0 );
	t = dir[ 0 ];
	dir[ 0 ] = -dir[ 1 ];
	dir[ 1 ] = t;

	showModel();

	SetOpenStartSound( "door_wood_open_move" );
	SetOpenEndSound( "door_wood_open_stop" );

	SetCloseStartSound( "door_wood_close_move" );
	SetCloseEndSound( "door_wood_close_stop" );

	SetLockedSound( "door_wood_locked" );

	traveltime = 1.0f;
	speed = 1.0f / traveltime;

   wait = ( spawnflags & DOOR_TOGGLE ) ? 0 : 3;
	dmg = 0;

	setSize( mins, maxs );

	setOrigin( localorigin );

	// DOOR_START_OPEN is to allow an entity to be lighted in the closed position
	// but spawn in the open position
	if ( spawnflags & DOOR_START_OPEN )
		{
		state = STATE_OPEN;
   	PostEvent( EV_Door_Open, EV_POSTSPAWN );
		}
	else
		{
		state = STATE_CLOSED;
		}
   previous_state = state;

	if ( health )
		{
		takedamage = DAMAGE_YES;
		}

	// LinkDoors can't be done until all of the doors have been spawned, so
	// the sizes can be detected properly.
	nextdoor = 0;
	PostEvent( EV_Door_Link, EV_LINKDOORS );

	// Default to work with monsters and players
	respondto = TRIGGER_PLAYERS | TRIGGER_MONSTERS;
	if ( spawnflags & 8 )
		{
		respondto &= ~TRIGGER_PLAYERS;
		}
	if ( spawnflags & 16 )
		{
		respondto &= ~TRIGGER_MONSTERS;
		}

	next_locked_time = 0;
	}

void Door::SetDir( Event *ev )
{
	float t;
	float angle;

	angle = ev->GetFloat( 1 );
	dir = G_GetMovedir( angle );
	t = dir[ 0 ];
	dir[ 0 ] = -dir[ 1 ];
	dir[ 1 ] = t;
}

void Door::SetOpenStartSound( str sound )
{
	sound_open_start = sound;
	if( sound_open_start.length() > 1 )
	{
		CacheResource( sound_open_start.c_str() );
	}
}

void Door::SetOpenEndSound( str sound )
{
	sound_open_end = sound;
	if( sound_open_end.length() > 1 )
	{
		CacheResource( sound_open_end.c_str() );
	}
}

void Door::SetCloseStartSound( str sound )
{
	sound_close_start = sound;
	if( sound_close_start.length() > 1 )
	{
		CacheResource( sound_close_start.c_str() );
	}
}

void Door::SetCloseEndSound( str sound )
{
	sound_close_end = sound;
	if( sound_close_end.length() > 1 )
	{
		CacheResource( sound_close_end.c_str() );
	}
}

void Door::SetMessageSound
   (
   str sound
   )

   {
	sound_message = sound;
   if ( sound_message.length() > 1 )
      {
      CacheResource( sound_message.c_str() );
      }
   }

void Door::SetLockedSound
   (
   str sound
   )

   {
	sound_locked = sound;
   if ( sound_locked.length() > 1 )
      {
      CacheResource( sound_locked.c_str() );
      }
   }

void Door::SetOpenStartSound( Event *ev )
{
	SetOpenStartSound( ev->GetString( 1 ) );
}

void Door::SetOpenEndSound( Event *ev )
{
	SetOpenEndSound( ev->GetString( 1 ) );
}

void Door::SetCloseStartSound( Event *ev )
{
	SetCloseStartSound( ev->GetString( 1 ) );
}

void Door::SetCloseEndSound( Event *ev )
{
	SetCloseEndSound( ev->GetString( 1 ) );
}

void Door::SetMessageSound
	(
	Event *ev
	)

{
	SetMessageSound( ev->GetString( 1 ) );
}

void Door::SetLockedSound
	(
	Event *ev
	)

{
	SetLockedSound( ev->GetString( 1 ) );
}

void Door::SetWait
	(
	Event *ev
	)

{
	wait = ev->GetFloat( 1 );
}

void Door::SetDmg
	(
	Event *ev
	)

{
	dmg = ev->GetInteger( 1 );
}

void Door::EventAlwaysAway
	(
	Event *ev
	)

{
	qboolean bAlwaysAway = true;

	if( ev->NumArgs() > 0 )
		bAlwaysAway = ev->GetInteger( 1 );

	m_bAlwaysAway = bAlwaysAway != 0;
}

void Door::EventTrySolid
	(
	Event *ev
	)

{
	Entity *check;
	gentity_t *ent;
	int touch[ MAX_GENTITIES ];
	int num;

	if( edict->r.contents == 1 )
		return;

	num = gi.AreaEntities( absmin, absmax, touch, MAX_GENTITIES );
	if( num > 0 )
	{
		for( int i = 0; i < num; i++ )
		{
			ent = &g_entities[ touch[ i ] ];
			check = ent->entity;

			if( check->getMoveType() > MOVETYPE_STOP )
			{
				if( edict->r.contents != CONTENTS_SHOOTONLY &&
					edict->solid &&
					IsTouching( check ) )
				{
					PostEvent( EV_Door_TrySolid, 1.0f );
					return;
				}
			}
		}
	}

	edict->r.contents = CONTENTS_SOLID;
	setSolidType( SOLID_BSP );
}

void Door::EventDoorType
	(
	Event *ev
	)

{
	str s = ev->GetString( 1 );

	if( s != "wood" && s != "metal" )
	{
		ScriptError( "Invalid Door Type." );
	}

	SetOpenStartSound( "door_" + s + "_open_move" );
	SetOpenEndSound( "door_" + s + "_open_stop" );

	SetCloseStartSound( "door_" + s + "_close_move" );
	SetCloseEndSound( "door_" + s + "_close_stop" );

	SetLockedSound( "door_" + s + "_locked" );
}

qboolean Door::isOpen
	(
	void
	)

{
	return ( state == STATE_OPEN );
}

qboolean Door::isCompletelyClosed
	(
	void
	)

{
	return ( state == STATE_CLOSED );
}

void Door::OpenEnd
	(
	Event *ev
	)

{
	if( edict->r.contents != CONTENTS_SOLID )
	{
		PostEvent( EV_Door_TrySolid, 1.0f );
	}

	if( sound_open_end.length() > 1 )
	{
		BroadcastAIEvent();
		Sound( sound_open_end, CHAN_VOICE );
	}
	else
	{
		StopSound( CHAN_VOICE );
	}

	previous_state = state;
	SetState( STATE_OPEN );
	if( spawnflags & DOOR_TOGGLE )
	{
		// don't close automatically
		return;
	}

	if( ( wait > 0 ) && ( master == this ) )
	{
		PostEvent( EV_Door_Close, wait );
	}
}

void Door::CloseEnd
	(
	Event *ev
	)

{
	if( sound_close_end.length() > 1 )
	{
		BroadcastAIEvent();
		Sound( sound_close_end, CHAN_VOICE );
	}
	else
	{
		StopSound( CHAN_VOICE );
	}

	if( master == this )
	{
		gi.AdjustAreaPortalState( this->edict, false );
	}

	edict->r.contents = CONTENTS_SOLID;
	setSolidType( SOLID_BSP );

	previous_state = state;
	SetState( STATE_CLOSED );
}

void Door::Close
	(
	Event *ev
	)

{
	Door *door;

	CancelEventsOfType( EV_Door_Close );

	previous_state = state;
	SetState( STATE_CLOSING );

	ProcessEvent( EV_Door_DoClose );

	if( sound_close_start.length() > 1 )
	{
		BroadcastAIEvent();
		Sound( sound_close_start, CHAN_VOICE );
	}
	if( master == this )
	{
		if( max_health )
		{
			takedamage = DAMAGE_YES;
			health = max_health;
		}

		// trigger all paired doors
		door = ( Door * )G_GetEntity( nextdoor );
		assert( door->isSubclassOf( Door ) );
		while( door && ( door != this ) )
		{
			door->ProcessEvent( EV_Door_Close );
			door = ( Door * )G_GetEntity( door->nextdoor );
			assert( door->isSubclassOf( Door ) );
		}
	}
}

void Door::Open
	(
	Event *ev
	)

{
	Door *door;
	Event *e;
	Entity *other;

	if( ev->NumArgs() < 1 )
	{
		ScriptError( "No entity specified to open door.  Door may open the wrong way." );
		other = world;
	}
	else
	{
		other = ev->GetEntity( 1 );
	}

	if( state == STATE_OPENING )
	{
		// already going up
		return;
	}

	if( state == STATE_OPEN )
	{
		// reset top wait time
		if( wait > 0 )
		{
			CancelEventsOfType( EV_Door_Close );
			PostEvent( EV_Door_Close, wait );
		}
		return;
	}

	previous_state = state;
	SetState( STATE_OPENING );

	e = new Event( EV_Door_DoOpen );
	e->AddEntity( other );
	ProcessEvent( e );

	if( sound_open_start.length() > 1 )
	{
		BroadcastAIEvent();
		Sound( sound_open_start, CHAN_VOICE );
	}
	if( master == this )
	{
		// trigger all paired doors
		door = ( Door * )G_GetEntity( nextdoor );
		assert( door->isSubclassOf( Door ) );
		while( door && ( door != this ) )
		{
			e = new Event( EV_Door_Open );
			e->AddEntity( other );
			door->ProcessEvent( e );
			door = ( Door * )G_GetEntity( door->nextdoor );
			assert( door->isSubclassOf( Door ) );
		}

		if( previous_state == STATE_CLOSED )
		{
			gi.AdjustAreaPortalState( this->edict, true );
		}
	}
}

void Door::DoorUse
	(
	Event *ev
	)

	{
	Entity *other;
   qboolean respond;
	Event *e;

	other = ev->GetEntity( 1 );

	respond = ( ( ( respondto & TRIGGER_PLAYERS ) && other->isClient() ) ||
		( ( respondto & TRIGGER_MONSTERS ) && other->isSubclassOf( Actor ) ) );

   if ( !respond )
      {
      return;
      }

	// only allow use when not triggerd by other events
	if ( health || ( spawnflags & ( DOOR_AUTO_OPEN | DOOR_TARGETED ) ) )
		{
      if ( other->isSubclassOf( Sentient ) && ( state == STATE_CLOSED ) )
         {
         if ( health )
            {
            gi.SendServerCommand( NULL, "print \"This door is jammed.\"" );
            }
         else if ( spawnflags & DOOR_TARGETED )
            {
				Sound( "door_triggered", CHAN_VOICE );
            }
         }

		if ( spawnflags & DOOR_AUTO_OPEN && locked && other->isSubclassOf( Player ) && sound_locked.length() )
			{
			other->Sound( sound_locked, CHAN_VOICE );
			}

		return;
		}

   assert( master );
   if ( !master )
      {
      // bulletproofing
      master = this;
      }

	if ( master->state == STATE_CLOSED )
		{
		e = new Event( EV_Door_TryOpen );
		e->AddEntity( other );
		master->ProcessEvent( e );
		}
	else if ( master->state == STATE_OPEN )
		{
		e = new Event( EV_Door_Close );
		e->AddEntity( other );
		master->ProcessEvent( e );
		}
	}

void Door::DoorFire
	(
	Event *ev
	)

	{
	Event *e;
	Entity *other;

	other = ev->GetEntity( 1 );

	assert( master == this );
	if ( master != this )
		{
		gi.Error( ERR_DROP, "DoorFire: master != self" );
		}

	// no more messages
	SetMessage( NULL );

	// reset health in case we were damage triggered
	health = max_health;

	// will be reset upon return
	takedamage = DAMAGE_NO;

	if ( ( spawnflags & ( DOOR_TOGGLE | DOOR_START_OPEN ) ) && ( state == STATE_OPENING || state == STATE_OPEN ) )
		{
		spawnflags &= ~DOOR_START_OPEN;
		ProcessEvent( EV_Door_Close );
		}
	else
		{
		e = new Event( EV_Door_Open );
		e->AddEntity( other );
		ProcessEvent( e );
		}
	}

void Door::DoorBlocked
	(
	Event *ev
	)

{
	Event *e;
	Entity *other;

	assert( master );
	if( ( master ) && ( master != this ) )
	{
		master->ProcessEvent( new Event( ev ) );
		return;
	}

	if( lastblocktime > level.time )
	{
		return;
	}

	lastblocktime = level.time + 0.3;

	other = ev->GetEntity( 1 );

	if( other->IsSubclassOfActor() )
	{
		edict->r.contents = CONTENTS_WEAPONCLIP;
		setSolidType( SOLID_BSP );
	}
	else
	{
		if( dmg )
		{
			other->Damage( this, this, ( int )dmg, origin, vec_zero, vec_zero, 0, 0, MOD_CRUSH );
		}

		//
		// if we killed him, lets keep on going
		//
		if( other->deadflag )
		{
			return;
		}

		if( state == STATE_OPENING || state == STATE_OPEN )
		{
			spawnflags &= ~DOOR_START_OPEN;
			ProcessEvent( EV_Door_Close );
		}
		else
		{
			e = new Event( EV_Door_Open );
			e->AddEntity( other );
			ProcessEvent( e );
		}
	}
}

void Door::FieldTouched
	(
	Event *ev
	)

	{
	Entity *other;

	other = ev->GetEntity( 1 );

	if ( !other )
		return;

	if ( ( state != STATE_OPEN ) && !( spawnflags & DOOR_AUTO_OPEN ) && !other->isSubclassOf( Actor ) )
		return;

	TryOpen( ev );
	}

qboolean	Door::CanBeOpenedBy
	(
	Entity *ent
	)

	{
   assert( master );
	if ( ( master ) && ( master != this ) )
		{
		return master->CanBeOpenedBy( ent );
		}

	if ( !locked && !key.length() )
		{
		return true;
		}

	if ( ent && ent->isSubclassOf( Sentient ) && ( ( Sentient * )ent )->HasItem( key.c_str() ) )
		{
		return true;
		}

	return false;
	}

void Door::TryOpen
	(
	Event *ev
	)

	{
	Entity *other;
	Event *event;

	//FIXME
	// hack so that doors aren't triggered by guys when game starts.
	// have to fix delay that guys go through before setting up their threads
	if ( level.time < 0.4 )
		{
		return;
		}

	other = ev->GetEntity( 1 );

   assert( master );
	if ( master && ( this != master ) )
		{
		event = new Event( EV_Door_TryOpen );
		event->AddEntity( other );
		master->ProcessEvent( event );
		return;
		}

	if ( !other || other->deadflag )
		{
		return;
		}

   if ( locked )
      {
		if ( next_locked_time <= level.time )
			{
			if ( sound_locked.length() > 1 && !other->isSubclassOf( Actor ) )
				{
				other->Sound( sound_locked, CHAN_VOICE );
				}
			else if ( other->isSubclassOf( Player ) )
				{
				other->Sound( "snd_locked", CHAN_VOICE );
	//         gi.centerprintf ( other->edict, "This door is locked." );
				}
			}

		// Always increment next locked time
		
		next_locked_time = level.time + 0.5;

		// locked doors don't open for anyone
      return;
      }

   if ( !CanBeOpenedBy( other ) )
		{
      Item        *item;
      ClassDef		*cls;

		if ( other->isClient() )
			{
			cls = getClass( key.c_str() );
			if ( !cls )
				{
            gi.DPrintf( "No item named '%s'\n", key.c_str() );
				return;
				}
			item = ( Item * )cls->newInstance();
			item->CancelEventsOfType( EV_Item_DropToFloor );
			item->CancelEventsOfType( EV_Remove );
			item->ProcessPendingEvents();
         gi.centerprintf ( other->edict, "You need the %s", item->getName().c_str() );
			delete item;
			}
      return;
      }

	// once we're opened by an item, we no longer need that item to open the door
	key = "";

	if ( Message().length() )
		{
      gi.centerprintf( other->edict, Message().c_str() );
		Sound( sound_message, CHAN_VOICE );
		}

	event = new Event( EV_Door_Fire );
	event->AddEntity( other );
	ProcessEvent( event );
	}

void Door::SpawnTriggerField
	(
	Vector fmins,
	Vector fmaxs
	)

	{
	TouchField *trig;
	Vector min;
	Vector max;

	min = fmins - Vector( "60 60 8" );
	max = fmaxs + Vector( "60 60 8" );

	trig = new TouchField;
	trig->Setup( this, EV_Door_TriggerFieldTouched, min, max, respondto );

	trigger = trig->entnum;
	}


void Door::LinkDoors
	(
	Event *ev
	)

{
	Entity *entptr;
	Door	*ent;
	Door	*next;
	Vector			cmins;
	Vector			cmaxs;
	int				i;

	setSolidType( SOLID_BSP );
	setMoveType( MOVETYPE_PUSH );

	if( nextdoor )
	{
		// already linked by another door
		return;
	}

	// master doors own themselves
	master = this;

	if( spawnflags & DOOR_DONT_LINK )
	{
		// don't want to link this door
		nextdoor = entnum;
		return;
	}

	cmins = absmin;
	cmaxs = absmax;

	ent = this;
	for( entptr = this; entptr; entptr = ( Entity * )G_FindClass( entptr, getClassID() ) )
	{
		next = ( Door * )entptr;
		if( !ent->DoorTouches( next ) )
		{
			continue;
		}

		if( next->nextdoor )
		{
			error( "Door::LinkDoors", "cross connected doors.  Targetname = %s entity %d\n", TargetName().c_str(), entnum );
		}

		ent->nextdoor = next->entnum;
		ent = next;

		for( i = 0; i < 3; i++ )
		{
			if( ent->absmin[ i ] < cmins[ i ] )
			{
				cmins[ i ] = ent->absmin[ i ];
			}
			if( ent->absmax[ i ] > cmaxs[ i ] )
			{
				cmaxs[ i ] = ent->absmax[ i ];
			}
		}

		// set master door
		ent->master = this;

		if( ent->health )
		{
			health = ent->health;
		}

		if( ent->Targeted() )
		{
			if( !Targeted() )
			{
				SetTargetName( ent->TargetName() );
			}
			else if( strcmp( TargetName(), ent->TargetName() ) )
			{
				// not a critical error, but let them know about it.
				gi.DPrintf( "cross connected doors\n" );

				ent->SetTargetName( TargetName() );
			}
		}

		if( ent->Message().length() )
		{
			if( Message().length() && !strcmp( Message().c_str(), ent->Message().c_str() ) )
			{
				// not a critical error, but let them know about it.
				gi.DPrintf( "Different messages on linked doors.  Targetname = %s", TargetName().c_str() );
			}

			// only master should have a message
			SetMessage( ent->Message().c_str() );
			ent->SetMessage( NULL );
		}
	}

	// make the chain a loop
	ent->nextdoor = entnum;

	// open up any portals we control
	if( spawnflags & DOOR_START_OPEN )
	{
		gi.AdjustAreaPortalState( this->edict, true );
	}

	// shootable or targeted doors don't need a trigger
	if( health || ( spawnflags & DOOR_TARGETED ) )
	{
		// Don't let the player trigger the door
		return;
	}

	// Don't spawn trigger field when set to toggle
	if( !( spawnflags & DOOR_TOGGLE ) )
	{
		SpawnTriggerField( cmins, cmaxs );
	}
}

void Door::SetTime
	(
	Event *ev
	)

	{
	traveltime = ev->GetFloat( 1 );
	if ( traveltime < FRAMETIME )
		{
		traveltime = FRAMETIME;
		}

	speed = 1.0f / traveltime;
	}

void Door::LockDoor
	(
	Event *ev
	)

{
	if( !level.PreSpawned() )
		ScriptError( "Cannot lock door before level prespawn" );

	SetLocked( true );
}

void Door::UnlockDoor
	(
	Event *ev
	)

{
	SetLocked( false );
}

qboolean Door::DoorTouches
	(
	Door *e1
	)

{
	if( e1->absmin.x > absmax.x )
	{
		return false;
	}
	if( e1->absmin.y > absmax.y )
	{
		return false;
	}
	if( e1->absmin.z > absmax.z )
	{
		return false;
	}
	if( e1->absmax.x < absmin.x )
	{
		return false;
	}
	if( e1->absmax.y < absmin.y )
	{
		return false;
	}
	if( e1->absmax.z < absmin.z )
	{
		return false;
	}

	return true;
}

void Door::UpdatePathConnection
	(
	void
	)

{
	if( BlocksAIMovement() )
	{
		DisconnectPaths();
	}
	else
	{
		ConnectPaths();
	}
}

void Door::SetLocked
	(
	qboolean newLocked
	)

{
	if( locked == newLocked )
		return;

	locked = newLocked;

	UpdatePathConnection();
}

void Door::SetState
	(
	int newState
	)

{
	if( state == newState )
		return;

	state = newState;

	UpdatePathConnection();
}

qboolean Door::BlocksAIMovement
	(
	void
	) const

{
	return ( locked || state == STATE_OPEN );
}

qboolean Door::AIDontFace
	(
	void
	) const

{
	return ( locked || state == STATE_OPEN );
}

/*****************************************************************************/
/*QUAKED func_rotatingdoor (0 0.25 0.5) ? START_OPEN OPEN_DIRECTION DOOR_DONT_LINK NOT_PLAYERS NOT_MONSTERS TOGGLE AUTO_OPEN TARGETED
if two doors touch, they are assumed to be connected and operate as a unit.

TOGGLE causes the door to wait in both the start and end states for a trigger event.
DOOR_DONT_LINK is for when you have two doors that are touching but you want to operate independently.

START_OPEN causes the door to move to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not usefull for touch or takedamage doors).
OPEN_DIRECTION indicates which direction to open when START_OPEN is set.
AUTO_OPEN causes the door to open when a player is near instead of waiting for the player to use the door.
TARGETED door is only operational from triggers or script

"message"		is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"openangle"    how wide to open the door
"angle"			determines the opening direction.  point toward the middle of the door (away from the hinge)
"targetname"	if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"			if set, door must be shot open
"time"			move time (0.3 default)
"wait"			wait before returning (3 default, -1 = never return)
"dmg"				damage to inflict when blocked (0 default)
"key"          The item needed to open this door (default nothing)

"sound_stop"		Specify the sound that plays when the door stops moving (default global door_stop)
"sound_move"		Specify the sound that plays when the door opens or closes (default global door_moving)
"sound_message"	Specify the sound that plays when the door displays a message
"sound_locked"	   Specify the sound that plays when the door is locked

******************************************************************************/

Event EV_RotatingDoor_OpenAngle
	( 
	"openangle",
	EV_DEFAULT,
   "f",
   "open_angle",
   "Sets the open angle of the door.",
	EV_NORMAL
	);

CLASS_DECLARATION( Door, RotatingDoor, "func_rotatingdoor" )
{
	{ &EV_Door_DoClose,					&RotatingDoor::DoClose },
	{ &EV_Door_DoOpen,					&RotatingDoor::DoOpen },
	{ &EV_RotatingDoor_OpenAngle,		&RotatingDoor::OpenAngle },
	{ NULL, NULL }
};

void RotatingDoor::DoOpen( Event *ev )
{
	Vector ang;

	if( previous_state == STATE_CLOSED )
	{
		if( ev->NumArgs() > 0 )
		{
			Entity *other;
			Vector p;

			other = ev->GetEntity( 1 );

			if( other )
			{
				Vector vMMOrigin ( mins + maxs );
				Vector vMMCenter = vMMOrigin * 0.5f;
				Vector vDoorDir;
				Vector vPerpDir;

				vMMOrigin = vMMOrigin - vMMCenter;
				vMMOrigin.normalize();

				vDoorDir[ 0 ] = dir[ 1 ];
				vDoorDir[ 1 ] = -dir[ 0 ];
				vDoorDir[ 2 ] = dir[ 2 ];

				p = other->origin - origin;

				vPerpDir = p;
				vPerpDir[ 2 ] = 0.f;
				vPerpDir.normalize();

				if( m_bAlwaysAway )
				{
					diropened = DotProduct( vPerpDir, vDoorDir );

					if( DotProduct( dir, vMMOrigin ) > 0.0f ) {
						diropened = -diropened;
					}
				}
				else
				{
					diropened = DotProduct( dir, vMMOrigin );
				}
			}
			else
			{
				ScriptError( "Door trying to be opened with a NULL entity." );
			}
		}
		else
		{
			diropened = 0 - init_door_direction;
		}
	}
	else
	{
		diropened = -init_door_direction;
	}

	if( diropened < 0.0f )
	{
		ang = startangle + Vector( 0.0f, angle, 0.0f );
	}
	else
	{
		ang = startangle - Vector( 0.0f, angle, 0.0f );
	}

   MoveTo( origin, ang, fabs( speed*angle ), EV_Door_OpenEnd );
}

void RotatingDoor::DoClose
	(
	Event *ev
	)

{
	MoveTo( origin, startangle, fabs( speed*angle ), EV_Door_CloseEnd );
}

void RotatingDoor::OpenAngle
   (
   Event *ev
   )

   {
   angle = ev->GetFloat( 1 );
   }

RotatingDoor::RotatingDoor()
	{
   if ( LoadingSavegame )
      {
      return;
      }
	startangle = angles;

	angle = 90;

   init_door_direction = (spawnflags & DOOR_OPEN_DIRECTION);
	}

/*****************************************************************************/
/*QUAKED func_door (0 0.25 0.5) ? START_OPEN x DOOR_DONT_LINK NOT_PLAYERS NOT_MONSTERS TOGGLE AUTO_OPEN TARGETED
if two doors touch, they are assumed to be connected and operate as a unit.

TOGGLE causes the door to wait in both the start and end states for a trigger event.
DOOR_DONT_LINK is for when you have two doors that are touching but you want to operate independently.

START_OPEN causes the door to move to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not usefull for touch or takedamage doors).
OPEN_DIRECTION indicates which direction to open when START_OPEN is set.
AUTO_OPEN causes the door to open when a player is near instead of waiting for the player to use the door.
TARGETED door is only operational from triggers or script

"message"		is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"			determines the opening direction.  point toward the middle of the door (away from the hinge)
"targetname"	if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"			if set, door must be shot open
"speed"			move speed (100 default)
"time"			move time (1/speed default, overides speed)
"wait"			wait before returning (3 default, -1 = never return)
"lip"				lip remaining at end of move (8 default)
"dmg"				damage to inflict when blocked (0 default)
"key"          The item needed to open this door (default nothing)

"sound_stop"		Specify the sound that plays when the door stops moving (default global door_stop)
"sound_move"		Specify the sound that plays when the door opens or closes (default global door_moving)
"sound_message"	Specify the sound that plays when the door displays a message
"sound_locked"	   Specify the sound that plays when the door is locked

******************************************************************************/

Event EV_SlidingDoor_Setup
	( 
	"setup",
	EV_CODEONLY,
   NULL,
   NULL,
   "Sets up the sliding door.",
	EV_NORMAL
	);
Event EV_SlidingDoor_SetLip
	( 
	"lip",
	EV_DEFAULT,
   "f",
   "lip",
   "Sets the lip of the sliding door.",
	EV_NORMAL
	);
Event EV_SlidingDoor_SetSpeed
	( 
	"speed",
	EV_DEFAULT,
   "f",
   "speed",
   "Sets the speed of the sliding door.",
	EV_NORMAL
	);

CLASS_DECLARATION( Door, SlidingDoor, "func_door" )
	{
	   { &EV_Door_DoClose,					&SlidingDoor::DoClose },
	   { &EV_Door_DoOpen,					&SlidingDoor::DoOpen },
      { &EV_SlidingDoor_Setup,				&SlidingDoor::Setup },
      { &EV_SlidingDoor_SetLip,				&SlidingDoor::SetLip },
      { &EV_SlidingDoor_SetSpeed,			&SlidingDoor::SetSpeed },
      { &EV_SetAngle,						&SlidingDoor::SetMoveDir },
		{ NULL, NULL }
	};

void SlidingDoor::SetMoveDir
   (
   Event *ev
   )

   {
   float t;
   float angle;

   angle = ev->GetFloat( 1 );
	movedir = G_GetMovedir( angle );
   dir = movedir;
	t = dir[ 0 ];
	dir[ 0 ] = -dir[ 1 ];
	dir[ 1 ] = t;
   }

void SlidingDoor::DoOpen
   (
 	Event *ev
   )

	{
	MoveTo( pos2, angles, speed*totalmove, EV_Door_OpenEnd );
   }

void SlidingDoor::DoClose
   (
 	Event *ev
   )

	{
	MoveTo( pos1, angles, speed*totalmove, EV_Door_CloseEnd );
   }

void SlidingDoor::SetLip
   (
   Event *ev
   )

   {
   lip = ev->GetFloat( 1 );
   CancelEventsOfType( EV_SlidingDoor_Setup );
   PostEvent( EV_SlidingDoor_Setup, EV_POSTSPAWN );
   }

void SlidingDoor::SetSpeed
   (
   Event *ev
   )

   {
   basespeed = ev->GetFloat( 1 );
   CancelEventsOfType( EV_SlidingDoor_Setup );
   PostEvent( EV_SlidingDoor_Setup, EV_POSTSPAWN );
   }

void SlidingDoor::Setup
   (
   Event *ev
   )

   {
	totalmove = fabs( movedir * size ) - lip;
   pos1 = origin;
	pos2 = pos1 + movedir * totalmove;

	if ( basespeed )
      {
      speed = basespeed / totalmove;
      }
   }

SlidingDoor::SlidingDoor()
	{
   if ( LoadingSavegame )
      {
      return;
      }
	lip = 8;
	basespeed = 0;
	movedir = G_GetMovedir( 0 );

   PostEvent( EV_SlidingDoor_Setup, EV_POSTSPAWN );
	}

/*****************************************************************************/
/*QUAKED script_door (0 0.5 1) ? START_OPEN x DOOR_DONT_LINK NOT_PLAYERS NOT_MONSTERS TOGGLE AUTO_OPEN TARGETED
if two doors touch, they are assumed to be connected and operate as a unit.

TOGGLE causes the door to wait in both the start and end states for a trigger event.
DOOR_DONT_LINK is for when you have two doors that are touching but you want to operate independently.

START_OPEN causes the door to move to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not usefull for touch or takedamage doors).
OPEN_DIRECTION indicates which direction to open when START_OPEN is set.
AUTO_OPEN causes the door to open when a player is near instead of waiting for the player to use the door.
TARGETED door is only operational from triggers or script

"message"		is printed when the door is touched if it is a trigger door and it hasn't been fired yet
"angle"			determines the opening direction.  point toward the middle of the door (away from the hinge)
"targetname"	if set, no touch field will be spawned and a remote button or trigger field activates the door.
"health"			if set, door must be shot open
"speed"			move speed (100 default)
"time"			move time (1/speed default, overides speed)
"wait"			wait before returning (3 default, -1 = never return)
"dmg"				damage to inflict when blocked (0 default)
"key"          The item needed to open this door (default nothing)
"initthread"   code to execute to setup the door (optional)
"openthread"   code to execute when opening the door (required)
               The openthread should send the "dooropened" event to the door, when it is completely open
"closethread"  code to execute when closing the door (required)
               The closethread should send the "doorclosed" event to the door, when it is completely closed

"sound_stop"		Specify the sound that plays when the door stops moving (default global door_stop)
"sound_move"		Specify the sound that plays when the door opens or closes (default global door_moving)
"sound_message"	Specify the sound that plays when the door displays a message
"sound_locked"	   Specify the sound that plays when the door is locked

******************************************************************************/

Event EV_ScriptDoor_DoInit
	( 
	"doinit",
	EV_DEFAULT,
   NULL,
   NULL,
   "Sets up the script door.",
	EV_NORMAL
	);
Event EV_ScriptDoor_SetOpenThread
	( 
	"openthread",
	EV_DEFAULT,
   "s",
   "openthread",
   "Set the thread to run when the door is opened (required).",
	EV_NORMAL
	);
Event EV_ScriptDoor_SetCloseThread
	( 
	"closethread",
	EV_DEFAULT,
   "s",
   "closethread",
   "Set the thread to run when the door is closed (required).",
	EV_NORMAL
	);
Event EV_ScriptDoor_SetInitThread
	( 
	"initthread",
	EV_DEFAULT,
   "s",
   "initthread",
   "Set the thread to run when the door is initialized (optional).",
	EV_NORMAL
	);

CLASS_DECLARATION( Door, ScriptDoor, "script_door" )
	{
	   { &EV_ScriptDoor_DoInit,					&ScriptDoor::DoInit },
	   { &EV_Door_DoClose,						&ScriptDoor::DoClose },
	   { &EV_Door_DoOpen,						&ScriptDoor::DoOpen },
	   { &EV_ScriptDoor_SetInitThread,			&ScriptDoor::SetInitThread },
	   { &EV_ScriptDoor_SetOpenThread,			&ScriptDoor::SetOpenThread },
	   { &EV_ScriptDoor_SetCloseThread,			&ScriptDoor::SetCloseThread },
      { &EV_SetAngle,							&ScriptDoor::SetMoveDir },
		{ NULL, NULL }
	};

void ScriptDoor::SetMoveDir
   (
   Event *ev
   )

   {
   float t;
   float angle;

   angle = ev->GetFloat( 1 );
	movedir = G_GetMovedir( angle );
   dir = movedir;
	t = dir[ 0 ];
	dir[ 0 ] = -dir[ 1 ];
	dir[ 1 ] = t;
   }

void ScriptDoor::SetOpenThread
   (
 	Event *ev
   )
	{
	openlabel.SetThread( ev->GetValue( 1 ) );
   }

void ScriptDoor::SetCloseThread
   (
 	Event *ev
   )
	{
	closelabel.SetThread( ev->GetValue( 1 ) );
   }

void ScriptDoor::DoInit
   (
 	Event *ev
   )
	{
   startorigin = origin;
	doorsize = fabs( movedir * size );

	if( initlabel.IsSet() )
	{
		initlabel.Execute( this );
	}
   }

void ScriptDoor::DoOpen
   (
 	Event *ev
   )

	{
	if( openlabel.IsSet() )
	{
		openlabel.Execute( this );
	}

   if ( previous_state == STATE_CLOSED )
      {
      diropened = 0;
      if ( ev->NumArgs() > 0 )
         {
   	   Entity *other;
	      Vector p;

	      other = ev->GetEntity( 1 );
         p = other->origin - origin;
         p.z = 0;
         diropened = dir * p;
         }
      }
   }

void ScriptDoor::DoClose
   (
 	Event *ev
   )
	{
	if( closelabel.IsSet() )
	{
		closelabel.Execute( this );
	}
   }

void ScriptDoor::SetInitThread
   (
   Event *ev
   )

   {
	initlabel.SetThread( ev->GetValue( 1 ) );
   }

ScriptDoor::ScriptDoor()
	{
   if ( LoadingSavegame )
      {
      return;
      }
	startangle = angles;

   //
   // clear out the sounds if necessary
   // scripted doors typically have their own sounds
   //
   sound_open_start = "";
   sound_open_end = "";
   sound_close_start = "";
   sound_close_end = "";

	movedir = G_GetMovedir( 0 );

   PostEvent( EV_ScriptDoor_DoInit, EV_POSTSPAWN );
	}
