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

// spawners.h: Various spawning entities

#ifndef __SPAWNWERS_H__
#define __SPAWNWERS_H__

#include "g_local.h"
#include "scriptslave.h"

class Spawn : public ScriptSlave
	{
   private:
  		str      modelname;
      str      spawntargetname;
      str      spawntarget;
      str      pickup_thread;
      str      spawnitem;
      float    spawnchance;
      int      attackmode;
   
		void           SetAngleEvent( Event *ev );
      void           SetPickupThread( Event *ev );
      void           ModelName( Event *ev );
      void           SpawnTargetName( Event *ev );
      void           SpawnTarget( Event *ev );
      void           AttackMode( Event *ev );
      void           SetSpawnItem( Event *ev );
      void           SetSpawnChance( Event *ev );

   protected:
      void           SetArgs( SpawnArgs &args );
		virtual void	DoSpawn( Event *ev );

	public:
      CLASS_PROTOTYPE( Spawn );


							Spawn();
	   virtual void   Archive( Archiver &arc );
	};

inline void Spawn::Archive
	(
	Archiver &arc
	)
   {
   ScriptSlave::Archive( arc );

   arc.ArchiveString( &modelname );
   arc.ArchiveString( &spawntargetname );
   arc.ArchiveString( &spawntarget );
   arc.ArchiveString( &pickup_thread );
   arc.ArchiveString( &spawnitem );
   arc.ArchiveFloat( &spawnchance );
   arc.ArchiveInteger( &attackmode );
   }

class RandomSpawn : public Spawn
	{
   private:
      float    min_time;
      float    max_time;

      void           MinTime( Event *ev );
      void           MaxTime( Event *ev );
      void           ToggleSpawn( Event *ev );
      void           Think( Event *ev );

	public:
      CLASS_PROTOTYPE( RandomSpawn );


							RandomSpawn();
	   virtual void   Archive( Archiver &arc );
	};

inline void RandomSpawn::Archive
	(
	Archiver &arc
	)
   {
   Spawn::Archive( arc );

   arc.ArchiveFloat( &min_time );
   arc.ArchiveFloat( &max_time );
   }

class ReSpawn : public Spawn
	{
   protected:
		virtual void	DoSpawn( Event *ev );
 	public:
      CLASS_PROTOTYPE( ReSpawn );
	};

class SpawnOutOfSight : public Spawn
	{
   protected:
		virtual void	DoSpawn( Event *ev );
 	public:
      CLASS_PROTOTYPE( SpawnOutOfSight );
	};

class SpawnChain : public Spawn
	{
   protected:
		virtual void	DoSpawn( Event *ev );
 	public:
      CLASS_PROTOTYPE( SpawnChain );
	};

#endif //__SPAWNWERS_H__
