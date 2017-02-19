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

// soundman.cpp: Sound Manager
//

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "player.h"
#include "soundman.h"

SoundManager SoundMan;

/******************************************************************************

  Sound Manager

******************************************************************************/

Event EV_SoundManager_AddSpeaker
	(
	"addspeaker", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Add a new sound where the player is standing.",
	EV_NORMAL
	);
Event EV_SoundManager_AddRandomSpeaker
	(
	"addrandomspeaker", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Add a new sound where the player is standing.",
	EV_NORMAL
	);
Event EV_SoundManager_AddMusicTrigger
	(
	"addmusictrigger", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Add a new music trigger where the player is standing.",
	EV_NORMAL
	);
Event EV_SoundManager_AddReverbTrigger
	(
	"addreverbtrigger", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Add a new reverb trigger where the player is standing.",
	EV_NORMAL
	);
Event EV_SoundManager_Replace
	( 
	"replace", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Replace the current sound position with the player's.",
	EV_NORMAL
	);
Event EV_SoundManager_Delete
	( 
	"delete", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Delete the current sound.",
	EV_NORMAL
	);
Event EV_SoundManager_MovePlayer
	( 
	"moveplayer", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Move the player to the current sound position.",
	EV_NORMAL
	);
Event EV_SoundManager_Next
	( 
	"next", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Go to the next sound.",
	EV_NORMAL
	);
Event EV_SoundManager_Previous
	( 
	"prev", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Go to the previous sound.",
	EV_NORMAL
	);

Event EV_SoundManager_Show
	( 
	"show", 
	EV_CONSOLE,
   "E",
   "path",
   "Show all the sounds.",
	EV_NORMAL
	);

Event EV_SoundManager_ShowingSounds
	( 
	"_showing_sounds", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Internal event for showing the sounds.",
	EV_NORMAL
	);
Event EV_SoundManager_Hide
	( 
	"hide", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Hides the sounds.",
	EV_NORMAL
	);

Event EV_SoundManager_Save
	( 
	"save", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Saves the sounds.",
	EV_NORMAL
	);
Event EV_SoundManager_UpdateInput
	( 
	"updateinput", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Updates the current sound with user interface values.",
	EV_NORMAL
	);

Event EV_SoundManager_Reset
	( 
	"reset", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Resets the state of all sounds and triggers.",
	EV_NORMAL
	);

Event EV_SoundManager_GlobalTranslate
	( 
	"globaltranslate", 
	EV_CONSOLE,
   "v",
   "translate_amount",
   "Translates all sounds and triggers by specified amount.",
	EV_NORMAL
	);

Event EV_SoundManager_SwitchFacet
	( 
	"switchfacet", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Switch the current facet that we are editing.",
	EV_NORMAL
	);

Event EV_SoundManager_PreviewReverb
	( 
	"previewreverb", 
	EV_CONSOLE,
   NULL,
   NULL,
   "Test out the current reverb settings.",
	EV_NORMAL
	);

Event EV_SoundManager_ResetReverb
	( 
	"resetreverb", 
	EV_CONSOLE,
   NULL,
   NULL,
   "reset the reverb settings to a normal.",
	EV_NORMAL
	);

CLASS_DECLARATION( Listener, SoundManager, NULL )
	{
      { &EV_SoundManager_AddSpeaker,		&SoundManager::AddSpeaker },
      { &EV_SoundManager_AddRandomSpeaker,	&SoundManager::AddRandomSpeaker },
      { &EV_SoundManager_AddMusicTrigger,	&SoundManager::AddMusicTrigger },
      { &EV_SoundManager_AddReverbTrigger,	&SoundManager::AddReverbTrigger },
      { &EV_SoundManager_Replace,			&SoundManager::Replace },
      { &EV_SoundManager_Delete,			&SoundManager::Delete },
      { &EV_SoundManager_MovePlayer,		&SoundManager::MovePlayer },
      { &EV_SoundManager_Next,				&SoundManager::Next },
      { &EV_SoundManager_Previous,			&SoundManager::Previous },
      { &EV_SoundManager_Show,				&SoundManager::Show },
      { &EV_SoundManager_ShowingSounds,		&SoundManager::ShowingSounds },
      { &EV_SoundManager_Hide,				&SoundManager::Hide },
      { &EV_SoundManager_Save,				&SoundManager::Save },
      { &EV_SoundManager_UpdateInput,		&SoundManager::UpdateEvent },
      { &EV_SoundManager_Reset,				&SoundManager::ResetEvent },
      { &EV_SoundManager_GlobalTranslate,	&SoundManager::GlobalTranslateEvent },
      { &EV_SoundManager_SwitchFacet,		&SoundManager::SwitchFacetEvent },
      { &EV_SoundManager_PreviewReverb,		&SoundManager::PreviewReverbEvent },
      { &EV_SoundManager_ResetReverb,		&SoundManager::ResetReverbEvent },
		{ NULL, NULL }
	};

Player *SoundManager_GetPlayer
   (
   void
   )

   {
   assert( g_entities[ 0 ].entity && g_entities[ 0 ].entity->isSubclassOf( Player ) );
   if ( !g_entities[ 0 ].entity || !( g_entities[ 0 ].entity->isSubclassOf( Player ) ) )
      {
      gi.Printf( "No player found.\n" );
      return NULL;
      }

   return ( Player * )g_entities[ 0 ].entity;
   }

SoundManager::SoundManager()
   {
   currentFacet = 0;
   Reset();
   }

void SoundManager::Reset
   (
   void
   )
   {
   current = NULL;
   soundList.ClearObjectList();
   }

void SoundManager::UpdateUI
   (
   void
   )
   {
   if ( current )
      {
      gi.Cvar_Set( "snd_multifaceted", "" );
      gi.Cvar_Set( "snd_currentfacet", "" );
      gi.Cvar_Set( "snd_onetime", "0" );
      gi.Cvar_Set( "snd_useangles", "0" );
      gi.Cvar_Set( "snd_yaw", "0" );

      gi.Cvar_Set( "snd_origin", va( "%.2f %.2f %.2f", current->origin[ 0 ], current->origin[ 1 ], current->origin[ 2 ] ) );
      gi.Cvar_Set( "snd_targetname", current->targetname.c_str() );
      if ( current->isSubclassOf( TriggerSpeaker ) )
         {
         TriggerSpeaker * speaker;

         speaker = ( TriggerSpeaker * )current;
         if ( speaker->volume != DEFAULT_VOL )
            {
            gi.Cvar_Set( "snd_volume", va( "%.1f", speaker->volume ) );
            }
         else
            {
            gi.Cvar_Set( "snd_volume", "Default" );
            }
         if ( speaker->min_dist != DEFAULT_MIN_DIST )
            {
            gi.Cvar_Set( "snd_mindist", va( "%.1f", speaker->min_dist ) );
            }
         else
            {
            gi.Cvar_Set( "snd_mindist", "Default" );
            }

         // setup the sound
         gi.Cvar_Set( "ui_pickedsound", speaker->Noise().c_str() );

         if ( current->isSubclassOf( RandomSpeaker ) )
            {
            RandomSpeaker * random;

            random = ( RandomSpeaker * )current;
            gi.Cvar_Set( "snd_mindelay", va( "%.1f", random->mindelay ) );
            gi.Cvar_Set( "snd_maxdelay", va( "%.1f", random->maxdelay ) );
            gi.Cvar_Set( "snd_channel", va( "%d", random->channel ) );
            gi.Cvar_Set( "snd_chance", va( "%.1f", random->chance ) );
            gi.Cvar_Set( "snd_type", "RandomSpeaker" );
            }
         else
            {
            gi.Cvar_Set( "snd_type", "Speaker" );
            }
         }
      else if ( current->isSubclassOf( TriggerMusic )  || current->isSubclassOf( TriggerReverb ) )
         {
         int      multiFaceted;
         Trigger  *trigger;

         trigger = ( Trigger * )current;
         gi.Cvar_Set( "snd_width", va( "%.0f", trigger->maxs[ 0 ] ) );
         gi.Cvar_Set( "snd_length", va( "%.0f", trigger->maxs[ 1 ] ) );
         gi.Cvar_Set( "snd_height", va( "%.0f", trigger->maxs[ 2 ] ) );
         if ( trigger->UsingTriggerDir() )
            {
            gi.Cvar_Set( "snd_useangles", "1" );
            }
         else
            {
            gi.Cvar_Set( "snd_useangles", "0" );
            }
         gi.Cvar_Set( "snd_yaw", va( "%.0f", trigger->angles[ 1 ] ) );
         multiFaceted = trigger->GetMultiFaceted();
         if ( multiFaceted )
            {
            if ( multiFaceted == 1 )
               {
               gi.Cvar_Set( "snd_multifaceted", "North/South" );
               if ( currentFacet )
                  {
                  gi.Cvar_Set( "snd_currentfacet", "South" );
                  }
               else
                  {
                  gi.Cvar_Set( "snd_currentfacet", "North" );
                  }
               }
            else
               {
               gi.Cvar_Set( "snd_multifaceted", "East/West" );
               if ( currentFacet )
                  {
                  gi.Cvar_Set( "snd_currentfacet", "West" );
                  }
               else
                  {
                  gi.Cvar_Set( "snd_currentfacet", "East" );
                  }
               }
            }
         else
            {
            gi.Cvar_Set( "snd_multifaceted", "Not" );
            }

         if ( current->isSubclassOf( TriggerMusic ) )
            {
            TriggerMusic * music;

            music = ( TriggerMusic * )trigger;

            gi.Cvar_Set( "snd_type", "MusicTrigger" );
            if ( music->oneshot )
               {
               gi.Cvar_Set( "snd_onetime", "1" );
               }
            else
               {
               gi.Cvar_Set( "snd_onetime", "0" );
               }

            if ( !currentFacet )
               {
               gi.Cvar_Set( "snd_currentmood", music->current.c_str() );
               gi.Cvar_Set( "snd_fallbackmood", music->fallback.c_str() );
               }
            else
               {
               gi.Cvar_Set( "snd_currentmood", music->altcurrent.c_str() );
               gi.Cvar_Set( "snd_fallbackmood", music->altfallback.c_str() );
               }
            }
         else if ( current->isSubclassOf( TriggerReverb ) )
            {
            TriggerReverb * reverb;

            reverb = ( TriggerReverb * )trigger;

            gi.Cvar_Set( "snd_type", "ReverbTrigger" );
            if ( reverb->oneshot )
               {
               gi.Cvar_Set( "snd_onetime", "1" );
               }
            else
               {
               gi.Cvar_Set( "snd_onetime", "0" );
               }

            if ( !currentFacet )
               {
               gi.Cvar_Set( "snd_reverbtypedisplay", EAXMode_NumToName( reverb->reverbtype ) );
               gi.Cvar_Set( "snd_reverbtype", va( "%d", reverb->reverbtype ) );
               gi.Cvar_Set( "snd_reverblevel", va( "%.2f", reverb->reverblevel ) );
               }
            else
               {
               gi.Cvar_Set( "snd_reverbtypedisplay", EAXMode_NumToName( reverb->altreverbtype ) );
               gi.Cvar_Set( "snd_reverbtype", va( "%d", reverb->altreverbtype ) );
               gi.Cvar_Set( "snd_reverblevel", va( "%.2f", reverb->altreverblevel ) );
               }
            }
         }
      if ( EventPending( EV_SoundManager_ShowingSounds ) )
         {
         gi.Cvar_Set( "snd_hiddenstate", "visible" );
         }
      else
         {
         gi.Cvar_Set( "snd_hiddenstate", "hidden" );
         }
      gi.Cvar_Set( "snd_speakernum", va( "%d", soundList.IndexOfObject( current ) - 1 ) );
      }
   }

void SoundManager::UpdateRandomSpeaker
   (
   RandomSpeaker * speaker
   )
   {
   cvar_t * cvar;
   Vector tempvec;

   if ( !speaker )
      {
      return;
      }

   // get origin
   cvar = gi.Cvar_Get( "snd_origin", "", 0 );
   sscanf( cvar->string, "%f %f %f", &tempvec[ 0 ], &tempvec[ 1 ], &tempvec[ 2 ] );
   speaker->setOrigin( tempvec );

   // get targetname
   cvar = gi.Cvar_Get( "snd_targetname", "", 0 );
   speaker->SetTargetName( cvar->string );

   // get volume
   cvar = gi.Cvar_Get( "snd_volume", "", 0 );
   if ( str( cvar->string ) == "Default" )
      {
      speaker->SetVolume( DEFAULT_VOL );
      }
   else
      {
      speaker->SetVolume( cvar->value );
      }

   // get min_dist
   cvar = gi.Cvar_Get( "snd_mindist", "", 0 );
   if ( str( cvar->string ) == "Default" )
      {
      speaker->SetMinDist( DEFAULT_MIN_DIST );
      }
   else
      {
      speaker->SetMinDist( cvar->value );
      }

   // get mindelay
   cvar = gi.Cvar_Get( "snd_mindelay", "", 0 );
   speaker->SetMinDelay( cvar->value );

   // get maxdelay
   cvar = gi.Cvar_Get( "snd_maxdelay", "", 0 );
   speaker->SetMaxDelay( cvar->value );

   // get chance
   cvar = gi.Cvar_Get( "snd_chance", "", 0 );
   speaker->SetChance( cvar->value );

   // get sound
   cvar = gi.Cvar_Get( "ui_pickedsound", "", 0 );
   if ( cvar->string[ 0 ] )
      {
      speaker->SetNoise( cvar->string );
      speaker->ScheduleSound();
      }
   }


void SoundManager::UpdateSpeaker
   (
   TriggerSpeaker * speaker
   )
   {
   cvar_t * cvar;
   Vector tempvec;

   if ( !speaker )
      {
      return;
      }

   //
   // make sure the speaker is an ambient speaker
   //
   speaker->ambient = true;

   // get origin
   cvar = gi.Cvar_Get( "snd_origin", "", 0 );
   sscanf( cvar->string, "%f %f %f", &tempvec[ 0 ], &tempvec[ 1 ], &tempvec[ 2 ] );
   speaker->setOrigin( tempvec );

   // get targetname
   cvar = gi.Cvar_Get( "snd_targetname", "", 0 );
   speaker->SetTargetName( cvar->string );

   // get volume
   cvar = gi.Cvar_Get( "snd_volume", "", 0 );
   if ( str( cvar->string ) == "Default" )
      {
      speaker->SetVolume( DEFAULT_VOL );
      }
   else
      {
      speaker->SetVolume( cvar->value );
      }

   // get min_dist
   cvar = gi.Cvar_Get( "snd_mindist", "", 0 );
   if ( str( cvar->string ) == "Default" )
      {
      speaker->SetMinDist( DEFAULT_MIN_DIST );
      }
   else
      {
      speaker->SetMinDist( cvar->value );
      }

   // get sound
   cvar = gi.Cvar_Get( "ui_pickedsound", "", 0 );
   if ( cvar->string[ 0 ] )
      {
      speaker->SetNoise( cvar->string );
      speaker->StartSound();
      }
   }

void SoundManager::UpdateTriggerMusic
   (
   TriggerMusic * music
   )
   {
   cvar_t   *cvar;
   str      current;
   str      fallback;
   str      faceted;
   Vector   tempvec;

   if ( !music )
      {
      return;
      }

   //
   // go to a known state
   //

   music->SetOneShot( false );
   // no angle use
   music->useTriggerDir = false;
   // make sure it doesn't trigger
   music->triggerable = false;

   // get origin
   cvar = gi.Cvar_Get( "snd_origin", "", 0 );
   sscanf( cvar->string, "%f %f %f", &tempvec[ 0 ], &tempvec[ 1 ], &tempvec[ 2 ] );
   music->setOrigin( tempvec );

   // get targetname
   cvar = gi.Cvar_Get( "snd_targetname", "", 0 );
   music->SetTargetName( cvar->string );

   // get width
   cvar = gi.Cvar_Get( "snd_width", "", 0 );
   music->mins[ 0 ] = -cvar->value;
   music->maxs[ 0 ] = cvar->value;

   // get length
   cvar = gi.Cvar_Get( "snd_length", "", 0 );
   music->mins[ 1 ] = -cvar->value;
   music->maxs[ 1 ] = cvar->value;

   // get height
   cvar = gi.Cvar_Get( "snd_height", "", 0 );
   music->mins[ 2 ] = 0;
   music->maxs[ 2 ] = cvar->value;

   music->setSize( music->mins, music->maxs );

   // get current multi faceted ness
   cvar = gi.Cvar_Get( "snd_multifaceted", "", 0 );
   faceted = cvar->string;

   if ( faceted == "North/South" )
      {
      music->SetMultiFaceted( 1 );
      }
   else if ( faceted == "East/West" )
      {
      music->SetMultiFaceted( 2 );
      }
   else
      {
      music->SetMultiFaceted( 0 );
      }

   // get current mood
   cvar = gi.Cvar_Get( "snd_currentmood", "", 0 );
   current = cvar->string;

   // get fallback mood
   cvar = gi.Cvar_Get( "snd_fallbackmood", "", 0 );
   fallback = cvar->string;

   if ( music->multiFaceted && currentFacet )
      {
      music->SetAltMood( current, fallback );
      }
   else
      {
      music->SetMood( current, fallback );
      }

   // get onetime
   cvar = gi.Cvar_Get( "snd_onetime", "", 0 );
   if ( cvar->integer )
      {
      music->SetOneShot( true );
      }

   // get yaw
   cvar = gi.Cvar_Get( "snd_yaw", "", 0 );
   music->angles[ YAW ] = cvar->value;
   music->setAngles();

   // get useangles
   cvar = gi.Cvar_Get( "snd_useangles", "", 0 );
   if ( cvar->integer )
      {
      music->SetTriggerDir( music->angles[ YAW ] );
      }
   UpdateUI();
   }

void SoundManager::UpdateTriggerReverb
   (
   TriggerReverb * reverb
   )
   {
   cvar_t   *cvar;
   int      reverbtype;
   float    reverblevel;
   str      faceted;
   Vector   tempvec;

   if ( !reverb )
      {
      return;
      }

   //
   // go to a known state
   //

   reverb->SetOneShot( false );
   // no angle use
   reverb->useTriggerDir = false;
   // make sure it doesn't trigger
   reverb->triggerable = false;

   // get origin
   cvar = gi.Cvar_Get( "snd_origin", "", 0 );
   sscanf( cvar->string, "%f %f %f", &tempvec[ 0 ], &tempvec[ 1 ], &tempvec[ 2 ] );
   reverb->setOrigin( tempvec );

   // get targetname
   cvar = gi.Cvar_Get( "snd_targetname", "", 0 );
   reverb->SetTargetName( cvar->string );

   // get width
   cvar = gi.Cvar_Get( "snd_width", "", 0 );
   reverb->mins[ 0 ] = -cvar->value;
   reverb->maxs[ 0 ] = cvar->value;

   // get length
   cvar = gi.Cvar_Get( "snd_length", "", 0 );
   reverb->mins[ 1 ] = -cvar->value;
   reverb->maxs[ 1 ] = cvar->value;

   // get height
   cvar = gi.Cvar_Get( "snd_height", "", 0 );
   reverb->mins[ 2 ] = 0;
   reverb->maxs[ 2 ] = cvar->value;

   reverb->setSize( reverb->mins, reverb->maxs );

   // get current multi faceted ness
   cvar = gi.Cvar_Get( "snd_multifaceted", "", 0 );
   faceted = cvar->string;

   if ( faceted == "North/South" )
      {
      reverb->SetMultiFaceted( 1 );
      }
   else if ( faceted == "East/West" )
      {
      reverb->SetMultiFaceted( 2 );
      }
   else
      {
      reverb->SetMultiFaceted( 0 );
      }

   // get reverb type
   cvar = gi.Cvar_Get( "snd_reverbtype", "", 0 );
   reverbtype = cvar->integer;

   // get reverb level
   cvar = gi.Cvar_Get( "snd_reverblevel", "", 0 );
   reverblevel = cvar->value;

   if ( reverb->multiFaceted && currentFacet )
      {
      reverb->SetAltReverb( reverbtype, reverblevel );
      }
   else
      {
      reverb->SetReverb( reverbtype, reverblevel );
      }

   // get onetime
   cvar = gi.Cvar_Get( "snd_onetime", "", 0 );
   if ( cvar->integer )
      {
      reverb->SetOneShot( true );
      }

   // get yaw
   cvar = gi.Cvar_Get( "snd_yaw", "", 0 );
   reverb->angles[ YAW ] = cvar->value;
   reverb->setAngles();

   // get useangles
   cvar = gi.Cvar_Get( "snd_useangles", "", 0 );
   if ( cvar->integer )
      {
      reverb->SetTriggerDir( reverb->angles[ YAW ] );
      }
   UpdateUI();
   }

void SoundManager::UpdateEvent
   (
   Event *ev
   )
   {
   if ( !current )
      {
      return;
      }
   if ( current->isSubclassOf( RandomSpeaker ) )
      {
      UpdateRandomSpeaker( ( RandomSpeaker * )current );
      }
   else if ( current->isSubclassOf( TriggerSpeaker ) )
      {
      UpdateSpeaker( ( TriggerSpeaker * )current );
      }
   else if ( current->isSubclassOf( TriggerMusic ) )
      {
      UpdateTriggerMusic( ( TriggerMusic * )current );
      }
   else if ( current->isSubclassOf( TriggerReverb ) )
      {
      UpdateTriggerReverb( ( TriggerReverb * )current );
      }
   }

void SoundManager::AddSpeaker
   (
   Event *ev
   )

   {
   Player *player;
   Vector ang;
   Vector pos;

   player = SoundManager_GetPlayer();
   if ( player )
      {
      player->GetPlayerView( &pos, &ang );

      current = new TriggerSpeaker;
      current->setOrigin( pos );
      current->setAngles( ang );

      soundList.AddUniqueObject( current );

      Show();
      }
   UpdateUI();
   }

void SoundManager::AddRandomSpeaker
   (
   Event *ev
   )

   {
   Player *player;
   Vector ang;
   Vector pos;

   player = SoundManager_GetPlayer();
   if ( player )
      {
      player->GetPlayerView( &pos, &ang );

      current = new RandomSpeaker;
      current->setOrigin( pos );
      current->setAngles( ang );

      soundList.AddUniqueObject( current );

      Show();
      }
   UpdateUI();
   }

void SoundManager::AddMusicTrigger
   (
   Event *ev
   )

   {
   Player *player;
   Vector ang;

   player = SoundManager_GetPlayer();
   if ( player )
      {
      player->GetPlayerView( NULL, &ang );

      current = new TriggerMusic;
      // we grab the origin from the feet of the player
      current->setOrigin( player->origin );
      current->setAngles( ang );
      current->setSize( Vector( "-16 -16 0" ), Vector( "16 16 64" ) );
      // make sure it doesn't trigger
      ( ( TriggerMusic * )current )->triggerable = false;

      soundList.AddUniqueObject( current );

      Show();
      }
   UpdateUI();
   }

void SoundManager::AddReverbTrigger
   (
   Event *ev
   )

   {
   Player *player;
   Vector ang;

   player = SoundManager_GetPlayer();
   if ( player )
      {
      player->GetPlayerView( NULL, &ang );

      current = new TriggerReverb;
      // we grab the origin from the feet of the player
      current->setOrigin( player->origin );
      current->setAngles( ang );
      current->setSize( Vector( "-16 -16 0" ), Vector( "16 16 64" ) );
      // make sure it doesn't trigger
      ( ( TriggerReverb * )current )->triggerable = false;

      soundList.AddUniqueObject( current );

      Show();
      }
   UpdateUI();
   }

void SoundManager::Replace
   (
   Event *ev
   )

   {
   Player *player;
   Vector ang;
   Vector pos;

   player = SoundManager_GetPlayer();
   if ( current && player )
      {
      player->GetPlayerView( &pos, &ang );

      if ( current->isSubclassOf( TriggerMusic ) || current->isSubclassOf( TriggerReverb )  )
         {
         current->setOrigin( player->origin );
         }
      else
         {
         current->setOrigin( pos );
         }
      current->setAngles( ang );
      }
   UpdateUI();
   }

void SoundManager::Delete
   (
   Event *ev
   )

   {
   int index = 0;

   if ( !current )
      return;

   if ( soundList.ObjectInList( current ) )
      {
      index = soundList.IndexOfObject( current );
      // remove the speaker
      soundList.RemoveObject( current );
      }
   current->PostEvent( EV_Remove, 0 );

   if ( ( index > 0 ) && ( index < soundList.NumObjects() ) )
      {
      current = soundList.ObjectAt( index );
      CurrentGainsFocus();
      }
   else
      {
      current = NULL;
      }
   
   UpdateUI();
   }

void SoundManager::MovePlayer
   (
   Event *ev
   )

   {
   Player *player;
   Vector pos;

   player = SoundManager_GetPlayer();
   if ( current && player )
      {
      player->GetPlayerView( &pos, NULL );

      if ( current->isSubclassOf( TriggerMusic ) || current->isSubclassOf( TriggerReverb ) )
         {
         player->setOrigin( current->origin );
         }
      else
         {
         player->setOrigin( current->origin - pos + player->origin );
         }
      player->SetViewAngles( current->angles );
      }
   }

void SoundManager::CurrentLostFocus
   (
   void
   )

   {
   if ( current )
      {
      if ( current->isSubclassOf( TriggerMusic ) || current->isSubclassOf( TriggerReverb ) )
         {
         current->PostEvent( EV_Trigger_SetTriggerable, 0.1f );
         }
      }
   }

void SoundManager::CurrentGainsFocus
   (
   void
   )

   {
   if ( current )
      {
      if ( current->isSubclassOf( TriggerMusic ) || current->isSubclassOf( TriggerReverb ) )
         {
         current->ProcessEvent( EV_Trigger_SetNotTriggerable );
         }
      }
   }

void SoundManager::Next
   (
   Event *ev
   )

   {
   int index;

   currentFacet = 0;

   if ( current )
      {
      //
      // find current sound in container of sounds
      //
      index = soundList.IndexOfObject( current );
      if ( index < soundList.NumObjects() )
         {
         index++;
         }
      else
         {
         index = 1;
         }
      CurrentLostFocus();
      }
   else
      {
      index = 1;
      }

   if ( index <= soundList.NumObjects() )
      {
      current = soundList.ObjectAt( index );
      CurrentGainsFocus();
      UpdateUI();
      }
   }

void SoundManager::Previous
   (
   Event *ev
   )

   {
   int index;

   currentFacet = 0;

   if ( current )
      {
      //
      // find current sound in container of sounds
      //
      index = soundList.IndexOfObject( current );
      if ( index > 1 )
         {
         index--;
         }
      else
         {
         index = soundList.NumObjects();
         }
      CurrentLostFocus();
      }
   else
      {
      index = 1;
      }

   if ( index <= soundList.NumObjects() )
      {
      current = soundList.ObjectAt( index );
      CurrentGainsFocus();
      UpdateUI();
      }
   }

void SoundManager::ResetEvent
   (
   Event *ev
   )
   {
   int i;
   Entity * ent;

   for( i = 1; i <= soundList.NumObjects(); i++ )
      {
      ent = soundList.ObjectAt( i );
      if ( ent->isSubclassOf( TriggerSpeaker ) )
         {
         // nothing to reset
         }
      else if ( ent->isSubclassOf( TriggerMusic ) )
         {
         TriggerMusic * music;

         music = ( TriggerMusic * )ent;
         music->SetOneShot( music->oneshot );
         }
      else if ( ent->isSubclassOf( TriggerReverb ) )
         {
         TriggerReverb * reverb;

         reverb = ( TriggerReverb * )ent;
         reverb->SetOneShot( reverb->oneshot );
         }
      }
   }

void SoundManager::GlobalTranslateEvent
   (
   Event *ev
   )
   {
   int i;
   Entity * ent;
   Vector amount;

   amount = ev->GetVector( 1 );

   for( i = 1; i <= soundList.NumObjects(); i++ )
      {
      ent = soundList.ObjectAt( i );
      ent->addOrigin( amount );
      }
   }

void SoundManager::SwitchFacetEvent
   (
   Event *ev
   )
   {
   if ( current && current->isSubclassOf( Trigger ) )
      {
      Trigger * trigger;

      trigger = ( Trigger * )current;
      if ( trigger->GetMultiFaceted() && !currentFacet )
         {
         currentFacet = 1;
         }
      else
         {
         currentFacet = 0;
         }
      }
   UpdateUI();
   }

void SoundManager::PreviewReverbEvent
   (
   Event *ev
   )
   {
   cvar_t   *cvar;
   int      reverbtype;
   float    reverblevel;

   // get reverb type
   cvar = gi.Cvar_Get( "snd_reverbtype", "", 0 );
   reverbtype = cvar->integer;

   // get reverb level
   cvar = gi.Cvar_Get( "snd_reverblevel", "", 0 );
   reverblevel = cvar->value;

   if ( g_entities[ 0 ].inuse && g_entities[ 0 ].client )
      {
      Player *client;

      client = ( Player * )g_entities[ 0 ].entity;
      client->SetReverb( reverbtype, reverblevel );
      }
   }

void SoundManager::ResetReverbEvent
   (
   Event *ev
   )
   {
   if ( g_entities[ 0 ].inuse && g_entities[ 0 ].client )
      {
      Player *client;

      client = ( Player * )g_entities[ 0 ].entity;
      client->SetReverb( "Generic", 0 );
      }
   }


void SoundManager::ShowingSounds
   (
   Event *ev
   )

   {
   int i;
   Entity * ent;

   for( i = 1; i <= soundList.NumObjects(); i++ )
      {
      ent = soundList.ObjectAt( i );
      if ( ent->isSubclassOf( TriggerSpeaker ) )
         {
         TriggerSpeaker * speaker;

         speaker = ( TriggerSpeaker * )ent;
         if ( current == ent )
            {
            if ( speaker->volume != DEFAULT_VOL )
               G_DrawDebugNumber( speaker->origin + Vector( 0, 0, 10 ), speaker->volume, 0.5, 0, 1, 0, 2 );
            if ( speaker->min_dist != DEFAULT_MIN_DIST )
               G_DrawDebugNumber( speaker->origin + Vector( 0, 0, 20 ), speaker->min_dist, 0.5, 0, 0, 1, 0 );

            // falloff circles
            if ( speaker->min_dist != DEFAULT_VOL )
               {
               G_DebugCircle( speaker->origin, speaker->min_dist, 0, 1, 1, 1, true );
               G_DebugCircle( speaker->origin, 2*speaker->min_dist, 0, 0.5, 0.5, 0.5, true );
               G_DebugCircle( speaker->origin, 4*speaker->min_dist, 0, 0.25, 0.25, 0.25, true );
               }

            if ( speaker->isSubclassOf( RandomSpeaker ) )
               {
               RandomSpeaker * random;

               G_DebugPyramid( speaker->origin, 24, 1, 1, 0.5, 1 );
               random = ( RandomSpeaker * )ent;

               G_DrawDebugNumber( random->origin + Vector( 0, 0, 30 ), random->mindelay, 0.5, 0.25, 0, 1, 1 );
               G_DrawDebugNumber( random->origin + Vector( 0, 0, 38 ), random->maxdelay, 0.5, 0.25, 0, 1, 1 );
               G_DrawDebugNumber( random->origin + Vector( 0, 0, 46 ), random->chance, 0.5, 0.25, 0, 1, 2 );
               }
            else
               {
               G_DebugPyramid( speaker->origin, 24, 1, 1, 0, 1 );
               }
            }
         else
            {
            if ( speaker->isSubclassOf( RandomSpeaker ) )
               {
               G_DebugPyramid( speaker->origin, 24, 1, 0, 1, 1 );
               }
            else
               {
               G_DebugPyramid( speaker->origin, 24, 1, 0, 0, 1 );
               }
            }
         }
      else if ( ent->isSubclassOf( TriggerMusic ) )
         {
         int            facet;
         TriggerMusic * music;

         music = ( TriggerMusic * )ent;

         if ( current == ent )
            {
            if ( music->oneshot )
               G_DebugBBox( ent->origin, ent->mins, ent->maxs, 0, 1, 1, 1 );
            else
               G_DebugBBox( ent->origin, ent->mins, ent->maxs, 1, 1, 0, 1 );
            }
         else
            {
            if ( music->oneshot )
               G_DebugBBox( ent->origin, ent->mins, ent->maxs, 0, 0.5, 0.5, 1 );
            else
               G_DebugBBox( ent->origin, ent->mins, ent->maxs, 1, 0, 0, 1 );
            }

         if ( music->useTriggerDir )
            {
            Vector org;

            org = ent->origin;
            org[ 2 ] += 0.5f * ent->maxs[ 2 ];

            G_DebugArrow( org, music->GetTriggerDir(), 48, 0.5, 1, 1, 1 );
            }
         facet = music->GetMultiFaceted();
         if ( facet )
            {
            if ( current == ent )
               {
               G_DebugHighlightFacet( ent->origin, ent->mins, ent->maxs, ( facet_t )( ( ( facet - 1 ) << 1 ) + currentFacet ), 1, 1, 1, 1 );
               }
            else
               {
               G_DebugHighlightFacet( ent->origin, ent->mins, ent->maxs, ( facet_t )( ( ( facet - 1 ) << 1 ) + 0 ), 0.6f, 0.3f, 0.2f, 0.1f );
               G_DebugHighlightFacet( ent->origin, ent->mins, ent->maxs, ( facet_t )( ( ( facet - 1 ) << 1 ) + 1 ), 0.6f, 0.3f, 0.2f, 0.1f );
               }
            }
         }
      else if ( ent->isSubclassOf( TriggerReverb ) )
         {
         int            facet;
         TriggerReverb  *reverb;

         reverb = ( TriggerReverb * )ent;

         if ( current == ent )
            {
            if ( reverb->oneshot )
               G_DebugBBox( ent->origin, ent->mins, ent->maxs, 0.5, 0.5, 1, 1 );
            else
               G_DebugBBox( ent->origin, ent->mins, ent->maxs, 0, 0.5, 1, 1 );
            }
         else
            {
            if ( reverb->oneshot )
               G_DebugBBox( ent->origin, ent->mins, ent->maxs, 0.5, 0, 1, 1 );
            else
               G_DebugBBox( ent->origin, ent->mins, ent->maxs, 0, 0, 1, 1 );
            }

         if ( reverb->useTriggerDir )
            {
            Vector org;

            org = ent->origin;
            org[ 2 ] += 0.5f * ent->maxs[ 2 ];

            G_DebugArrow( org, reverb->GetTriggerDir(), 48, 0.5, 1, 1, 1 );
            }
         facet = reverb->GetMultiFaceted();
         if ( facet )
            {
            if ( current == ent )
               {
               G_DebugHighlightFacet( ent->origin, ent->mins, ent->maxs, ( facet_t )( ( ( facet - 1 ) << 1 ) + currentFacet ), 1, 1, 1, 1 );
               }
            else
               {
               G_DebugHighlightFacet( ent->origin, ent->mins, ent->maxs, ( facet_t )( ( ( facet - 1 ) << 1 ) + 0 ), 0.3f, 0.3f, 0.6f, 0.1f );
               G_DebugHighlightFacet( ent->origin, ent->mins, ent->maxs, ( facet_t )( ( ( facet - 1 ) << 1 ) + 1 ), 0.3f, 0.3f, 0.6f, 0.1f );
               }
            }
         }
      }
   PostEvent( EV_SoundManager_ShowingSounds, FRAMETIME );
   }

void SoundManager::Show
   (
   void
   )

   {
   CurrentGainsFocus();
   CancelEventsOfType( EV_SoundManager_ShowingSounds );
   PostEvent( EV_SoundManager_ShowingSounds, FRAMETIME );
   UpdateUI();
   }

void SoundManager::Show
   (
   Event *ev
   )

   {
   Show();
   }

void SoundManager::Hide
   (
   Event *ev
   )

   {
   CurrentLostFocus();
   CancelEventsOfType( EV_SoundManager_ShowingSounds );
   UpdateUI();
   }

void SoundManager::Save
   (
   void
   )
   {
   Entity      *ent;
   str         buf;
	str         filename;
   size_t         i;

   // get the name of the sound file from the world
   filename = "maps/";
	filename += level.mapname;
	for( i = filename.length() - 1; i >= 0; i-- )
		{
		if ( filename[ i ] == '.' )
			{
			filename[ i ] = 0;
			break;
			}
		}

	filename += ".snd";

   gi.Printf( "Saving soundmanager file to '%s'...\n", filename.c_str() );

   buf = "";
   buf += va( "//\n" );
   buf += va( "// Sound Manager File \"%s\", %d Sound Entities.\n", filename.c_str(), soundList.NumObjects() );
   buf += va( "//\n" );

   //
   // save out normal TriggerSpeakers
   //
   buf += va( "// TriggerSpeakers\n" );
   for( i = 1; i <= soundList.NumObjects(); i++ )
      {
      TriggerSpeaker * speaker;

      ent = soundList.ObjectAt( i );
      if ( !ent->isSubclassOf( TriggerSpeaker ) || ent->isSubclassOf( RandomSpeaker ) )
         continue;

      speaker = ( TriggerSpeaker * )ent;
      //
      // start off the spawn command
      //
      buf += "spawn TriggerSpeaker";
      //
      // set the targetname
      //
      if ( ent->targetname != "" )
         {
         buf += va( " targetname %s", ent->targetname.c_str() );
         }
      //
      // set the origin
      //
      buf += va( " origin \"%.2f %.2f %.2f\"", ent->origin.x, ent->origin.y, ent->origin.z );
      //
      // make the speaker ambient and on
      //
      buf += " spawnflags 1";
      //
      // set the volume
      //
      if ( speaker->volume != DEFAULT_VOL )
         buf += va( " volume %.2f", speaker->volume );
      //
      // set the mindist
      //
      if ( speaker->min_dist != DEFAULT_MIN_DIST )
         buf += va( " min_dist %.1f", speaker->min_dist );
      //
      // save the sound
      //
      buf += va( " sound \"%s\"", speaker->Noise().c_str() );
      //
      // make sure it gets re-added to the sound manager
      //
      buf += " _addtosoundmanager 0";
      buf += va( "\n" );
      }
   //
   // save out RandomSpeakers
   //
   buf += va( "// RandomSpeakers\n" );
   for( i = 1; i <= soundList.NumObjects(); i++ )
      {
      RandomSpeaker * speaker;

      ent = soundList.ObjectAt( i );
      if ( !ent->isSubclassOf( TriggerSpeaker ) || !ent->isSubclassOf( RandomSpeaker ) )
         continue;

      speaker = ( RandomSpeaker * )ent;
      //
      // start off the spawn command
      //
      buf += "spawn RandomSpeaker";
      //
      // set the targetname
      //
      if ( ent->targetname != "" )
         {
         buf += va( " targetname %s", ent->targetname.c_str() );
         }
      //
      // set the origin
      //
      buf += va( " origin \"%.2f %.2f %.2f\"", ent->origin.x, ent->origin.y, ent->origin.z );
      //
      // set the volume
      //
      if ( speaker->volume != DEFAULT_VOL )
         buf += va( " volume %.2f", speaker->volume );
      //
      // set the mindist
      //
      if ( speaker->min_dist != DEFAULT_MIN_DIST )
         buf += va( " min_dist %.1f", speaker->min_dist );
      //
      // set the channel
      //
      buf += va( " channel %d", speaker->channel );
      //
      // set the mindelay
      //
      buf += va( " mindelay %.2f", speaker->mindelay );
      //
      // set the maxdelay
      //
      buf += va( " maxdelay %.2f", speaker->maxdelay );
      //
      // set the chance
      //
      buf += va( " chance %.2f", speaker->chance );
      //
      // save the sound
      //
      buf += va( " sound \"%s\"", speaker->Noise().c_str() );
      //
      // make sure it gets re-added to the sound manager
      //
      buf += " _addtosoundmanager 0";
      buf += va( "\n" );
      }
   //
   // save out TriggerMusic
   //
   buf += va( "// TriggerMusics\n" );
   for( i = 1; i <= soundList.NumObjects(); i++ )
      {
      TriggerMusic * music;

      ent = soundList.ObjectAt( i );
      if ( !ent->isSubclassOf( TriggerMusic ) )
         continue;

      music = ( TriggerMusic * )ent;
      //
      // start off the spawn command
      //
      buf += "spawn TriggerMusic";
      //
      // set the targetname
      //
      if ( ent->targetname != "" )
         {
         buf += va( " targetname %s", ent->targetname.c_str() );
         }
      //
      // set the origin
      //
      buf += va( " origin \"%.2f %.2f %.2f\"", ent->origin.x, ent->origin.y, ent->origin.z );
      //
      // set the angle
      //
      if ( music->useTriggerDir )
         {
         buf += va( " angle %.1f", AngleMod( ent->angles.y ) );
         }
      //
      // set the oneshot
      //
      if ( music->oneshot )
         {
         buf += " oneshot 0";
         }
      //
      // set current
      //
      buf += va( " current %s", music->current.c_str() );

      //
      // set fallback
      //
      buf += va( " fallback %s", music->fallback.c_str() );

      if ( music->GetMultiFaceted() )
         {
         //
         // save out multi faceted
         //
         buf += va( " multifaceted %d", music->GetMultiFaceted() );

         //
         // set alt current
         //
         buf += va( " altcurrent %s", music->altcurrent.c_str() );

         //
         // set alt fallback
         //
         buf += va( " altfallback %s", music->altfallback.c_str() );
         }

      //
      // set mins
      //
      buf += va( " _setmins \"%.2f %.2f %.2f\"", ent->mins.x, ent->mins.y, ent->mins.z );

      //
      // set maxs
      //
      buf += va( " _setmaxs \"%.2f %.2f %.2f\"", ent->maxs.x, ent->maxs.y, ent->maxs.z );
      //
      // make sure it gets re-added to the sound manager
      //
      buf += " _addtosoundmanager 0";
      buf += va( "\n" );
      }

   //
   // save out TriggerReverb
   //
   buf += va( "// TriggerReverbs\n" );
   for( i = 1; i <= soundList.NumObjects(); i++ )
      {
      TriggerReverb * reverb;

      ent = soundList.ObjectAt( i );
      if ( !ent->isSubclassOf( TriggerReverb ) )
         continue;

      reverb = ( TriggerReverb * )ent;
      //
      // start off the spawn command
      //
      buf += "spawn TriggerReverb";
      //
      // set the targetname
      //
      if ( ent->targetname != "" )
         {
         buf += va( " targetname %s", ent->targetname.c_str() );
         }
      //
      // set the origin
      //
      buf += va( " origin \"%.2f %.2f %.2f\"", ent->origin.x, ent->origin.y, ent->origin.z );
      //
      // set the angle
      //
      if ( reverb->useTriggerDir )
         {
         buf += va( " angle %.1f", AngleMod( ent->angles.y ) );
         }
      //
      // set the oneshot
      //
      if ( reverb->oneshot )
         {
         buf += " oneshot 0";
         }
      //
      // set reverb type
      //
      buf += va( " reverbtype %d", reverb->reverbtype );

      //
      // set reverb level
      //
      buf += va( " reverblevel %.2f", reverb->reverblevel );

      if ( reverb->GetMultiFaceted() )
         {
         //
         // save out multi faceted
         //
         buf += va( " multifaceted %d", reverb->GetMultiFaceted() );

         //
         // set alt reverb type
         //
         buf += va( " altreverbtype %d", reverb->altreverbtype );

         //
         // set alt reverb level
         //
         buf += va( " altreverblevel %.2f", reverb->altreverblevel );
         }

      //
      // set mins
      //
      buf += va( " _setmins \"%.2f %.2f %.2f\"", ent->mins.x, ent->mins.y, ent->mins.z );

      //
      // set maxs
      //
      buf += va( " _setmaxs \"%.2f %.2f %.2f\"", ent->maxs.x, ent->maxs.y, ent->maxs.z );
      //
      // make sure it gets re-added to the sound manager
      //
      buf += " _addtosoundmanager 0";
      buf += va( "\n" );
      }

   buf += "end\n";

   gi.FS_WriteFile( filename.c_str(), ( void * )buf.c_str(), buf.length() + 1 );
   gi.Printf( "done.\n" );
   }

void SoundManager::Save
   (
   Event *ev
   )

   {
   Save();
   }

void SoundManager::Load
   (
   void
   )
{
	str filename;
	intptr_t i;

	// get the name of the sound file from the world
	filename = "maps/";
	filename += level.mapname;
	for( i = filename.length() - 1; i >= 0; i-- )
	{
		if( filename[ i ] == '.' )
		{
			filename[ i ] = 0;
			break;
		}
	}

	filename += ".snd";

	// If there isn't a script with the same name as the map, then don't try to load script
	if( gi.FS_ReadFile( filename.c_str(), NULL, true ) != -1 )
	{
		Reset();

		Director.ExecuteThread( filename, "" );
	}
}

void SoundManager::AddEntity
   (
   Entity * ent
   )
   {
   if ( ent )
      {
      soundList.AddUniqueObject( ent );
      }
   }
