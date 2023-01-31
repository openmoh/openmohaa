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

// doors.h: Doors are environment objects that slide open when activated by triggers
// or when used by the player.
// 

#ifndef __DOORS_H__
#define __DOORS_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "scriptslave.h"

extern Event EV_Door_TryOpen;
extern Event EV_Door_GoDown;
extern Event EV_Door_GoUp;
extern Event EV_Door_HitBottom;
extern Event EV_Door_HitTop;
extern Event EV_Door_Fire;
extern Event EV_Door_Link;
extern Event EV_Door_SetSpeed;
extern Event EV_Door_Lock;
extern Event EV_Door_Unlock;

class Door;

typedef SafePtr<Door> DoorPtr;

class Door : public ScriptSlave
	{
	protected:
		str			sound_open_start;
		str			sound_open_end;
		str			sound_close_start;
		str			sound_close_end;
		str			sound_message;
		str			sound_locked;
      float       lastblocktime;
		float			angle;
		Vector		dir;
      float       diropened;
		int			state;
      int         previous_state;
		int			trigger;
		int			nextdoor;
		DoorPtr		master;
		float			next_locked_time;
		bool			m_bAlwaysAway;

		void			SetDir( Event *ev );
		void			OpenEnd( Event *ev );
		void			CloseEnd( Event *ev );
		void			Close( Event *ev );
		void			Open( Event *ev );
		void			DoorUse( Event *ev );
		void			DoorFire( Event *ev );
		void			DoorBlocked( Event *ev );
		void			FieldTouched( Event *ev );
		void			TryOpen( Event *ev );
		void			SpawnTriggerField( Vector fmins, Vector fmaxs );
		qboolean		DoorTouches( Door *e1 );
		void			LinkDoors( Event *ev );	
		void			SetTime( Event *ev );
		void			LockDoor( Event *ev );
		void			UnlockDoor( Event *ev );
		void			SetOpenStartSound( str sound );
		void			SetOpenStartSound( Event *ev );
		void			SetOpenEndSound( str sound );
		void			SetOpenEndSound( Event *ev );
		void			SetCloseStartSound( Event *ev );
		void			SetCloseStartSound( str sound );
		void			SetCloseEndSound( Event *ev );
		void			SetCloseEndSound( str sound );
		void			SetMessageSound( str sound );
		void			SetMessageSound( Event *ev );
		void			SetLockedSound( str sound );
		void			SetLockedSound( Event *ev );
		void			SetWait( Event *ev );
		void			SetDmg( Event *ev );
		void			EventTrySolid( Event *ev );
		void			EventAlwaysAway( Event *ev );
		void			EventDoorType( Event *ev );

		qboolean		BlocksAIMovement( void ) const;
		qboolean		AIDontFace( void ) const override;

		void			UpdatePathConnection( void );
		void			SetLocked( qboolean newLocked );
		void			SetState( int newState );

	public:
      CLASS_PROTOTYPE( Door );

      qboolean		locked;

						Door();
		qboolean		isOpen( void );
		qboolean		isCompletelyClosed( void );
		qboolean		CanBeOpenedBy( Entity *ent );
	   void Archive( Archiver &arc ) override;
	};

inline void Door::Archive
	(
	Archiver &arc
	)
   {
   ScriptSlave::Archive( arc );

   arc.ArchiveString( &sound_open_start );
   arc.ArchiveString( &sound_open_end );
   arc.ArchiveString( &sound_close_start );
   arc.ArchiveString( &sound_close_end );
   arc.ArchiveString( &sound_message );
   arc.ArchiveString( &sound_locked );
	if ( arc.Loading() )
	{
		SetOpenStartSound( sound_open_start );
		SetOpenEndSound( sound_open_end );
		SetCloseStartSound( sound_close_start );
		SetCloseEndSound( sound_close_end );
		SetMessageSound( sound_message );
		SetLockedSound( sound_locked );
	}
   arc.ArchiveFloat( &lastblocktime );
   arc.ArchiveFloat( &angle );
   arc.ArchiveVector( &dir );
   arc.ArchiveFloat( &diropened );
   arc.ArchiveInteger( &state );
   arc.ArchiveInteger( &previous_state );
   arc.ArchiveInteger( &trigger );
   arc.ArchiveInteger( &nextdoor );
   arc.ArchiveSafePointer( &master );
   arc.ArchiveBoolean( &locked );
	arc.ArchiveFloat( &next_locked_time );
   }

class RotatingDoor : public Door
	{
	protected:
		float		angle;
      Vector   startangle;
      int      init_door_direction;

	public:
      CLASS_PROTOTYPE( RotatingDoor );

      void           DoOpen( Event *ev );
      void           DoClose( Event *ev );
      void           OpenAngle( Event *ev );
	   void Archive( Archiver &arc ) override;

					RotatingDoor();
	};

inline void RotatingDoor::Archive
	(
	Archiver &arc
	)
   {
   Door::Archive( arc );

   arc.ArchiveFloat( &angle );
   arc.ArchiveVector( &startangle );
   arc.ArchiveInteger( &init_door_direction );
   }

class SlidingDoor : public Door
	{
	protected:
      float    totalmove;
      float    lip;
      Vector   pos1;
      Vector   pos2;
      float    basespeed;
      Vector   movedir;

	public:
      CLASS_PROTOTYPE( SlidingDoor );

      void     SetMoveDir( Event *ev );
      void     Setup( Event *ev );
      void     SetLip( Event *ev );
      void     SetSpeed( Event *ev );
      void     DoOpen( Event *ev );
      void     DoClose( Event *ev );
	   void Archive( Archiver &arc ) override;

					SlidingDoor();
	};

inline void SlidingDoor::Archive
	(
	Archiver &arc
	)
   {
   Door::Archive( arc );

   arc.ArchiveFloat( &totalmove );
   arc.ArchiveFloat( &lip );
   arc.ArchiveVector( &pos1 );
   arc.ArchiveVector( &pos2 );
   arc.ArchiveVector( &movedir );
   arc.ArchiveFloat( &basespeed );
   }

class ScriptDoor : public Door
	{
	protected:
      ScriptThreadLabel		initlabel;
      ScriptThreadLabel		openlabel;
      ScriptThreadLabel		closelabel;
      float    doorsize;
		Vector	startangle;
      Vector   startorigin;
      Vector   movedir;

	public:
      CLASS_PROTOTYPE( ScriptDoor );

      void           SetMoveDir( Event *ev );
      void           DoInit( Event *ev );
      void           DoOpen( Event *ev );
      void           DoClose( Event *ev );
      void           SetOpenThread( Event *ev );
      void           SetCloseThread( Event *ev );
      void           SetInitThread( Event *ev );
	   void Archive( Archiver &arc ) override;
					      ScriptDoor();
	};

inline void ScriptDoor::Archive
	(
	Archiver &arc
	)
   {
   Door::Archive( arc );

   arc.ArchiveFloat( &doorsize );
   arc.ArchiveVector( &startangle );
   arc.ArchiveVector( &startorigin );
   arc.ArchiveVector( &movedir );
   }

#endif /* doors.h */
