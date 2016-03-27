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

// soundman.h: Sound Manager
// 

#ifndef __SOUND_MANAGER_H__
#define __SOUND_MANAGER_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"

class SoundManager : public Listener
   {
   protected:
      int               currentFacet;
      Entity            *current;
      Container<Entity *> soundList;

      void              AddSpeaker( Event *ev );
      void              AddRandomSpeaker( Event *ev );
      void              AddMusicTrigger( Event *ev );
      void              AddReverbTrigger( Event *ev );
      void              Replace( Event *ev );
      void              Delete( Event *ev );
      void              MovePlayer( Event *ev );
      void              Next( Event *ev );
      void              Previous( Event *ev );
      void              ShowingSounds( Event *ev );
      void              Show( Event *ev );
      void              Hide( Event *ev );
      void              Save( Event *ev );
      void              UpdateEvent( Event *ev );
      void              ResetEvent( Event *ev );
      void              GlobalTranslateEvent( Event *ev );
      void              SwitchFacetEvent( Event *ev );
      void              PreviewReverbEvent( Event *ev );
      void              ResetReverbEvent( Event *ev );

      void              Show( void );
      void              UpdateUI( void );
      void              Save( void );
      void              CurrentLostFocus( void );
      void              CurrentGainsFocus( void );
      void              UpdateSpeaker( TriggerSpeaker * speaker );
      void              UpdateRandomSpeaker( RandomSpeaker * speaker );
      void              UpdateTriggerMusic( TriggerMusic * music );
      void              UpdateTriggerReverb( TriggerReverb * reverb );

   public:
      CLASS_PROTOTYPE( SoundManager );

                        SoundManager();
      void              Reset( void );
      void              Load( void );
      void              AddEntity( Entity * ent );
	   virtual void      Archive( Archiver &arc );
   };

inline void SoundManager::Archive
	(
	Archiver &arc
	)

   {
   int               i;
   int               num;
   int               currentFacet;

   Listener::Archive( arc );

   arc.ArchiveInteger( &currentFacet );
   arc.ArchiveObjectPointer( ( Class ** )&current );

   if ( arc.Saving() )
      {
      num = soundList.NumObjects();
      arc.ArchiveInteger( &num );
      }
   else
      {
      soundList.ClearObjectList();
      arc.ArchiveInteger( &num );
      soundList.Resize( num );
      }
   for( i = 1; i <= num; i++ )
      {
      arc.ArchiveObjectPointer( ( Class ** )soundList.AddressOfObjectAt( i ) );
      }
   }

extern SoundManager SoundMan;

#endif /* camera.h */
