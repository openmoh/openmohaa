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

// gravpath.h: Gravity path - Used for underwater currents and wells.

#ifndef __GRAVPATH_H__
#define __GRAVPATH_H__

#include "g_local.h"
#include "class.h"
#include "container.h"


class GravPathNode : public Entity
   {
   private:
      float    speed;
      float    radius;
      qboolean headnode;
		float		max_speed;
   
   public:
      qboolean       active; 

      CLASS_PROTOTYPE(GravPathNode);
                     GravPathNode();
      void           SetSpeed( Event *ev );
		void           SetMaxSpeed( Event *ev );
      void           SetRadius( Event *ev );
      void           CreatePath( Event *ev );
      void           Activate( Event *ev );
      void           Deactivate( Event *ev );
      float          Speed( void );
		float          MaxSpeed( void );
      float          Radius( void ) { return radius; };
	   void Archive( Archiver &arc ) override;
   };

inline void GravPathNode::Archive
	(
	Archiver &arc
	)
   {
   Entity::Archive( arc );

   arc.ArchiveFloat( &speed );
   arc.ArchiveFloat( &radius );
   arc.ArchiveBoolean( &headnode );
   arc.ArchiveBoolean( &active );
	arc.ArchiveFloat( &max_speed );
   }

typedef SafePtr<GravPathNode> GravPathNodePtr;

class GravPath : public Listener
	{
	private:
		Container<GravPathNodePtr>	pathlist;
		float							   pathlength;
      
		GravPathNodePtr    	      from;
		GravPathNodePtr   		   to;
		int							   nextnode;
		
	public:
      CLASS_PROTOTYPE( GravPath );

						      GravPath();
                        ~GravPath();
		void			      Clear(void);
		void			      Reset(void);
		void			      AddNode(GravPathNode *node);
		GravPathNode      *GetNode(int num);
		GravPathNode      *NextNode(void);
		Vector		      ClosestPointOnPath(Vector pos, Entity &ent,float *bestdist,float *speed,float *radius);
		float			      DistanceAlongPath(Vector pos, float *speed);
		Vector		      PointAtDistance( Vector pos, float dist, qboolean is_player, float *max_distance );
		void			      DrawPath(float r, float g, float b);
		int			      NumNodes(void);
		float			      Length(void);
		GravPathNode		*Start(void);
		GravPathNode		*End(void);
	   void Archive( Archiver &arc ) override;

      Vector                     mins;
      Vector                     maxs;
      Vector                     origin;
      qboolean                   force;
	};

inline void GravPath::Archive
	(
	Archiver &arc
	)

   {
   GravPathNodePtr *tempPtr;
   int i, num;

   Listener::Archive( arc );

   if ( arc.Loading() )
      {
      Reset();
      }
   else
      {
      num = pathlist.NumObjects();
      }
   arc.ArchiveInteger( &num );
   if ( arc.Loading() )
      {
      pathlist.Resize( num );
      }

   for ( i = 1; i <= num; i++ )
      {
      tempPtr = pathlist.AddressOfObjectAt( i );
      arc.ArchiveSafePointer( tempPtr );
      }

   arc.ArchiveFloat( &pathlength );
   arc.ArchiveSafePointer( &from );
   arc.ArchiveSafePointer( &to );
   arc.ArchiveInteger( &nextnode );
   arc.ArchiveVector( &mins );
   arc.ArchiveVector( &maxs );
   arc.ArchiveVector( &origin );
   arc.ArchiveBoolean( &force );
   }

class GravPathManager : public Class
   {
	private:
	   Container<GravPath *>	pathList;

	public:
      CLASS_PROTOTYPE( GravPathManager );
               ~GravPathManager();
      void     Reset( void );
      void     AddPath(GravPath *p);
      void     RemovePath(GravPath *p);
      Vector   CalculateGravityPull(Entity &ent, Vector position, qboolean *force, float *max_speed);
      void     DrawGravPaths( void );
	   void Archive( Archiver &arc ) override;
   };

inline void GravPathManager::Archive
	(
	Archiver &arc
	)
   {
   GravPath * ptr;
   int i, num;

   Class::Archive( arc );

   if ( arc.Saving() )
      {
      num = pathList.NumObjects();
      }
   else
      {
      Reset();
      }
   arc.ArchiveInteger( &num );
   for ( i = 1; i <= num; i++ )
      {
      if ( arc.Saving() )
         {
         ptr = pathList.ObjectAt( i );
         }
      else
         {
         ptr = new GravPath;
         }
      arc.ArchiveObject( ptr );
      if ( arc.Loading() )
         {
         pathList.AddObject( ptr );
         }
      }
   }

extern GravPathManager gravPathManager;

#endif /* gravpath.h */
