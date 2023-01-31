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

// explosion.h: Standard explosion object that is spawned by other entites and not map designers.
// Explosion is used by many of the weapons for the blast effect, but is also used
// by the Exploder and MultiExploder triggers.  These triggers create one or more
// explosions each time they are activated.
// 

#ifndef __EXPLOSION_H__
#define __EXPLOSION_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"

class Exploder : public Trigger
	{
	private:
		int	            damage;

		void              MakeExplosion( Event *ev );
      void              SetDmg( Event *ev );

	public:
      CLASS_PROTOTYPE( Exploder );
		
		                  Exploder();
	   void Archive( Archiver &arc ) override;
	};

inline void Exploder::Archive
	(
	Archiver &arc
	)

   {
   Trigger::Archive( arc );

   arc.ArchiveInteger( &damage );
   }

class MultiExploder : public Trigger
	{
	protected:
		float	         explodewait;
		float	         explode_time;
		float          duration;
		int	         damage;
		float          randomness;

		void           MakeExplosion( Event *ev );
      void           SetDmg( Event *ev );
      void           SetDuration( Event *ev );
      void           SetWait( Event *ev );
      void           SetRandom( Event *ev );

	public:
      CLASS_PROTOTYPE( MultiExploder );

		               MultiExploder();
	   void Archive( Archiver &arc ) override;
	};

inline void MultiExploder::Archive
	(
	Archiver &arc
	)
   {
   Trigger::Archive( arc );

   arc.ArchiveFloat( &explodewait );
   arc.ArchiveFloat( &explode_time );
   arc.ArchiveFloat( &duration );
   arc.ArchiveInteger( &damage );
   arc.ArchiveFloat( &randomness );
   }

void CreateExplosion 
   (
   Vector pos, 
   float  damage = 120,
   Entity *inflictor = NULL,
   Entity *attacker = NULL,
   Entity *ignore = NULL,
   const char *explosionModel = NULL,
   float  scale = 1.0f
   );

class ExplodeObject : public MultiExploder
	{
	private:
      Container<str> debrismodels;
      int            debrisamount;
      float          severity;

      void           SetDebrisModel( Event *ev );
      void           SetSeverity( Event *ev );
      void           SetDebrisAmount( Event *ev );
      void           MakeExplosion( Event *ev );

	public:
      CLASS_PROTOTYPE( ExplodeObject );

		               ExplodeObject();
	   void Archive( Archiver &arc ) override;
	};

inline void ExplodeObject::Archive
	(
	Archiver &arc
	)
   {
   MultiExploder::Archive( arc );

   arc.ArchiveFloat( &severity );
   arc.ArchiveInteger( &debrisamount );
   debrismodels.Archive( arc );
   }

#endif /* explosion.h */
