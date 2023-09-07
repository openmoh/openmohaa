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

// viewthing.cpp: Actor code for the Viewthing. 
//

#include "animate.h"
#include "viewthing.h"
#include "game.h"
#include "level.h"
#include "scriptexception.h"

Event EV_ViewThing_Think
   (
   "viewthing_think",
	EV_DEFAULT,
   NULL,
   NULL,
   "Called every frame to process the view thing.",
	EV_NORMAL
   );
Event EV_ViewThing_ToggleAnimate
   (
   "viewanimate",
   EV_CHEAT,
   NULL,
   NULL,
   "Cycle through the animations modes of the current viewthing\n"
   "No Animation\n"
   "Animation with no motion\n"
   "Animation with looping motion\n"
   "Animation with motion\n",
	EV_NORMAL
   );
Event EV_ViewThing_SetModel
   (
   "viewmodel",
   EV_CHEAT,
   "s",
   "viewthingModel",
   "Set the model of the current viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_NextFrame
   (
   "viewnext",
   EV_CHEAT,
   NULL,
   NULL,
   "Advance to the next frame of animation of the current viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_PrevFrame
   (
   "viewprev",
   EV_CHEAT,
   NULL,
   NULL,
   "Advance to the previous frame of animation of the current viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_NextAnim
   (
   "viewnextanim", 
   EV_CHEAT,
   NULL,
   NULL,
   "Advance to the next animation of the current viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_PrevAnim
   (
   "viewprevanim",
   EV_CHEAT,
   NULL,
   NULL,
   "Advance to the previous animation of the current viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_ScaleUp
   (
   "viewscaleup",
   EV_CHEAT,
   NULL,
   NULL,
   "Increase the scale of the current viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_ScaleDown
   (
   "viewscaledown",
   EV_CHEAT,
   NULL,
   NULL,
   "Decrease the scale of the current viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_SetScale
   (
   "viewscale",
   EV_CHEAT,
   "f",
   "scale",
   "Set the scale of the current viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_SetYaw
   (
   "viewyaw",
   EV_CHEAT,
   "f",
   "yaw",
   "Set the yaw of the current viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_SetPitch
   (
   "viewpitch",
   EV_CHEAT,
   "f",
   "pitch",
   "Set the pitch of the current viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_SetRoll
   (
   "viewroll",
   EV_CHEAT,
   "f",
   "roll",
   "Set the roll of the current viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_SetAngles
   (
   "viewangles",
   EV_CHEAT,
   "f[0,360]f[0,360]f[0,360]",
   "pitch yaw roll",
   "Set the angles of the current viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_Spawn
   (
   "viewspawn",
   EV_CHEAT,
   "s",
   "model",
   "Create a viewthing with the specified model",
	EV_NORMAL
   );
Event EV_ViewThing_Next
   (
   "viewthingnext",
   EV_CHEAT,
   NULL,
   NULL,
   "Change the active viewthing to the next viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_Prev
   (
   "viewthingprev",
   EV_CHEAT,
   NULL,
   NULL,
   "Change the active viewthing to the previous viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_Attach
   (
   "viewattach",
   EV_CHEAT,
   "ss",
   "tagname model",
   "Attach a model the the specified tagname",
	EV_NORMAL
   );
Event EV_ViewThing_Detach
   (
   "viewdetach",
   EV_CHEAT,
   NULL,
   NULL,
   "Detach the current viewthing from its parent",
	EV_NORMAL
   );
Event EV_ViewThing_DetachAll
   (
   "viewdetachall",
   EV_CHEAT,
   NULL,
   NULL,
   "Detach all the models attached to the current viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_Delete
   (
   "viewdelete",
   EV_CHEAT,
   NULL,
   NULL,
   "Delete the current viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_SetOrigin
   (
   "vieworigin",
   EV_CHEAT,
   "fff",
   "x y z",
   "Set the origin of the current viewthing",
	EV_NORMAL
   );
Event EV_ViewThing_DeleteAll
   (
   "viewdeleteall",
   EV_CHEAT,
   NULL,
   NULL,
   "Delete all viewthings",
	EV_NORMAL
   );
Event EV_ViewThing_LastFrame
   (
   "viewlastframe",
	EV_DEFAULT,
   NULL,
   NULL,
   "Called when the view things last animation frame is displayed.",
	EV_NORMAL
   );
Event EV_ViewThing_SaveOffSurfaces
   (
   "viewsavesurfaces",
	EV_DEFAULT,
   NULL,
   NULL,
   "Called after the model is spawned to save off the models original surfaces.",
	EV_NORMAL
   );
Event EV_ViewThing_PrintTime
   (
   "_viewthing_printtime",
	EV_DEFAULT,
   NULL,
   NULL,
   "Prints out the current level.time.",
	EV_NORMAL
   );
Event EV_ViewThing_SetAnim
   (
   "viewsetanim",
   EV_CHEAT,
   "f",
   "animNum",
   "Set the animation absolutely based off a floating point value",
	EV_NORMAL
   );

CLASS_DECLARATION( Animate, ViewMaster, NULL )
	{
	   { &EV_ViewThing_Spawn,				&ViewMaster::Spawn },
	   { &EV_ViewThing_Next,				&ViewMaster::Next },
	   { &EV_ViewThing_Prev,				&ViewMaster::Prev },
	   { &EV_ViewThing_SetModel,			&ViewMaster::SetModelEvent },
		{ &EV_ViewThing_DeleteAll,			&ViewMaster::DeleteAll },
	   { &EV_ViewThing_ToggleAnimate,		&ViewMaster::PassEvent },
	   { &EV_ViewThing_NextFrame,			&ViewMaster::PassEvent },
	   { &EV_ViewThing_PrevFrame,			&ViewMaster::PassEvent },
	   { &EV_ViewThing_NextAnim,			&ViewMaster::PassEvent },
	   { &EV_ViewThing_PrevAnim,			&ViewMaster::PassEvent },
	   { &EV_ViewThing_ScaleUp,				&ViewMaster::PassEvent },
	   { &EV_ViewThing_ScaleDown,			&ViewMaster::PassEvent },
	   { &EV_ViewThing_SetScale,			&ViewMaster::PassEvent },
	   { &EV_ViewThing_SetYaw,				&ViewMaster::PassEvent },
	   { &EV_ViewThing_SetPitch,			&ViewMaster::PassEvent },
	   { &EV_ViewThing_SetRoll,				&ViewMaster::PassEvent },
		{ &EV_ViewThing_SetAngles,			&ViewMaster::PassEvent },
	   { &EV_ViewThing_Attach,				&ViewMaster::PassEvent },
	   { &EV_ViewThing_Detach,				&ViewMaster::PassEvent },
	   { &EV_ViewThing_DetachAll,			&ViewMaster::PassEvent },
	   { &EV_ViewThing_Delete,				&ViewMaster::PassEvent },
	   { &EV_ViewThing_SetOrigin,			&ViewMaster::PassEvent },
	   { &EV_ViewThing_SetAnim,				&ViewMaster::PassEvent },
		{ NULL, NULL }
	};

ViewMaster Viewmodel;

ViewMaster::ViewMaster()
   {
   current_viewthing = NULL;
   }

void ViewMaster::Init( void )
{
	gi.AddCommand( "viewanimate", NULL );
	gi.AddCommand( "viewmodel", NULL );
	gi.AddCommand( "viewnext", NULL );
	gi.AddCommand( "viewprev", NULL );
	gi.AddCommand( "viewnextanim", NULL );
	gi.AddCommand( "viewprevanim", NULL );
	gi.AddCommand( "viewscaleup", NULL );
	gi.AddCommand( "viewscaledown", NULL );
	gi.AddCommand( "viewscale", NULL );
	gi.AddCommand( "viewyaw", NULL );
	gi.AddCommand( "viewpitch", NULL );
	gi.AddCommand( "viewroll", NULL );
	gi.AddCommand( "viewangles", NULL );
	gi.AddCommand( "viewspawn", NULL );
	gi.AddCommand( "viewthingnext", NULL );
	gi.AddCommand( "viewthingprev", NULL );
	gi.AddCommand( "viewattach", NULL );
	gi.AddCommand( "viewdetach", NULL );
	gi.AddCommand( "viewdetachall", NULL );
	gi.AddCommand( "viewdelete", NULL );
	gi.AddCommand( "vieworigin", NULL );
	gi.AddCommand( "viewdeleteall", NULL );
	gi.AddCommand( "viewsetanim", NULL );
}

void ViewMaster::Next
	(
	Event *ev
	)

	{
	Viewthing *viewthing;
   Entity * ent;

   ent = ( Entity * )G_FindClass( current_viewthing, "viewthing" );
	if ( ent )
		{
      current_viewthing = ent;

      viewthing = ( Viewthing * )( ( Entity * )current_viewthing );
      gi.Printf( "current viewthing model %s.\n", viewthing->model.c_str() );
      viewthing->UpdateCvars();
      }
   else
		{
      gi.Printf( "no more viewthings on map.\n" );
		}
	}

void ViewMaster::Prev
	(
	Event *ev
	)

	{
	Viewthing   *viewthing;
	Entity      *prev;
	Entity      *next;

	next = NULL;
	do
		{
		prev = next;
		next = ( Entity * )G_FindClass( prev, "viewthing" );
		}
	while( next != current_viewthing );

	if ( prev )
		{
      current_viewthing = prev;

      viewthing = ( Viewthing * )( ( Entity * )current_viewthing );
      gi.Printf( "current viewthing model %s.\n", viewthing->model.c_str() );
      viewthing->UpdateCvars();
      }
   else
		{
      gi.Printf( "no more viewthings on map.\n" );
		}
	}

void ViewMaster::DeleteAll
	(
	Event *ev
	)

	{
	Entity *next;

	for( next = ( Entity * )G_FindClass( NULL, "viewthing" ); next != NULL; next = ( Entity * )G_FindClass( next, "viewthing" ) )
		{
	   next->PostEvent( EV_Remove, 0 );
      }

   current_viewthing = NULL;
	}

void ViewMaster::Spawn
	(
	Event *ev
	)

	{
	Viewthing	*viewthing;
   const char	*mdl;
   Vector		forward;
	Vector		up;
   Vector		delta;
	Event			*event;
	Entity		*ent;

   mdl = ev->GetString( 1 );
   if ( !mdl || !mdl[ 0 ] )
		{
		ScriptError( "Must specify a model name" );
		return;
		}

	// Check if we have a client
	ent = g_entities[ 0 ].entity;
	assert( ent );
	if ( !ent )
		{
		return;
		}

   // create a new viewthing
   viewthing = new Viewthing;

   // set the current_viewthing
   current_viewthing = viewthing;

   //FIXME FIXME
	ent->angles.AngleVectors( &forward, NULL, &up );

   viewthing->baseorigin = ent->origin;
   viewthing->baseorigin += forward * 48;
   viewthing->baseorigin += up * 48;

   viewthing->setOrigin( viewthing->baseorigin );
   viewthing->droptofloor( 256 );

   viewthing->baseorigin = viewthing->origin;

   delta = ent->origin - viewthing->origin;
	viewthing->setAngles( delta.toAngles() );

	event = new Event( EV_ViewThing_SetModel );
	event->AddString( mdl );
	viewthing->ProcessEvent( event );

	if( !gi.modeltiki( viewthing->model ) )
      {
		ScriptError( "model %s not found, viewmodel not spawned.", mdl );
      delete viewthing;
      current_viewthing = NULL;
      return;
      }
   }

void ViewMaster::SetModelEvent
	(
	Event *ev
	)

	{
   const char	*mdl;
   char			str[ 128 ];
	Event			*event;
	Viewthing	*viewthing;

   mdl = ev->GetString( 1 );
   if ( !mdl || !mdl[ 0 ] )
		{
		ScriptError( "Must specify a model name" );
      return;
		}

	if ( !current_viewthing )
		{
      // try to find one on the map
		current_viewthing = ( Entity * )G_FindClass( NULL, "viewthing" );
	   if ( !current_viewthing )
         {
		   ScriptError( "No viewmodel" );
         return;
         }
		}

   viewthing = ( Viewthing * )( ( Entity * )current_viewthing );
	
	// Prepend 'models/' to make things easier
	str[ 0 ] = 0;
   if ( ( mdl[ 1 ] != ':' ) && Q_stricmpn( mdl, "models", 6 ) )
      {
      strcpy( str, "models/" );
      }
	strcat( str, mdl );

	event = new Event( EV_ViewThing_SetModel );
	event->AddString( str );
	viewthing->ProcessEvent( event );
   viewthing->UpdateCvars();
	}

void ViewMaster::PassEvent
	(
	Event *ev
	)

	{
	Viewthing *viewthing;
	Event *event;

	if ( !current_viewthing )
		{
		ScriptError( "No viewmodel" );
      return;
		}

   viewthing = ( Viewthing * )( ( Entity * )current_viewthing );
	if ( viewthing )
		{
		viewthing->ProcessEvent( *ev );
		}
	}

CLASS_DECLARATION( Animate, Viewthing, "viewthing" )
	{
	   { &EV_ViewThing_Think,				&Viewthing::ThinkEvent },
	   { &EV_ViewThing_LastFrame,     		&Viewthing::LastFrameEvent },
	   { &EV_ViewThing_ToggleAnimate,		&Viewthing::ToggleAnimateEvent },
	   { &EV_ViewThing_SetModel,			&Viewthing::SetModelEvent },
	   { &EV_ViewThing_NextFrame,			&Viewthing::NextFrameEvent },
	   { &EV_ViewThing_PrevFrame,			&Viewthing::PrevFrameEvent },
	   { &EV_ViewThing_NextAnim,			&Viewthing::NextAnimEvent },
	   { &EV_ViewThing_PrevAnim,			&Viewthing::PrevAnimEvent },
	   { &EV_ViewThing_ScaleUp,				&Viewthing::ScaleUpEvent },
	   { &EV_ViewThing_ScaleDown,			&Viewthing::ScaleDownEvent },
	   { &EV_ViewThing_SetScale,			&Viewthing::SetScaleEvent },
	   { &EV_ViewThing_SetYaw,				&Viewthing::SetYawEvent },
	   { &EV_ViewThing_SetPitch,			&Viewthing::SetPitchEvent },
	   { &EV_ViewThing_SetRoll,				&Viewthing::SetRollEvent },
	   { &EV_ViewThing_SetAngles,			&Viewthing::SetAnglesEvent },
	   { &EV_ViewThing_Attach,				&Viewthing::AttachModel },
	   { &EV_ViewThing_Detach,				&Viewthing::Delete },
	   { &EV_ViewThing_DetachAll, 			&Viewthing::DetachAll },
	   { &EV_ViewThing_Delete, 				&Viewthing::Delete },
	   { &EV_ViewThing_SetOrigin,			&Viewthing::ChangeOrigin },
      { &EV_ViewThing_SaveOffSurfaces,		&Viewthing::SaveSurfaces },
	   { &EV_ViewThing_PrintTime,			&Viewthing::PrintTime },
	   { &EV_ViewThing_SetAnim,				&Viewthing::SetAnim },
		{ NULL, NULL }
	};

Viewthing::Viewthing
	(
	void
	)

	{
   frame = 0;
   animstate = 0;
	setSolidType( SOLID_NOT );
   baseorigin = origin;
   Viewmodel.current_viewthing = this;
   edict->s.eType = ET_MODELANIM;
   edict->s.renderfx |= RF_SHADOW;  
   edict->s.renderfx |= RF_SHADOW_PRECISE;  
   edict->s.renderfx |= RF_EXTRALIGHT;  

   // save off surfaces once the model is spawned
   PostEvent( EV_ViewThing_SaveOffSurfaces, FRAMETIME );

	PostEvent( EV_ViewThing_Think, FRAMETIME );
   }

void Viewthing::PrintTime
	(
	Event *ev
	)
	
	{
   gi.Printf( "ev current frame %d leveltime %.2f\n", ev->GetInteger( 1 ), level.time );
   }

void Viewthing::UpdateCvars
   (
   qboolean quiet
   )

{
	gi.cvar_set("viewmodelentity", va("%d", edict->s.number));
}


void Viewthing::ThinkEvent
	(
	Event *ev
	)
	
{
    int f;
    if (animstate >= 2)
    {
        Vector   forward;
        Vector	left;
        Vector	up;
        Vector   realmove;

        angles.AngleVectors(&forward, &left, &up);
        realmove = left * accel[1] + up * accel[2] + forward * accel[0];
        setOrigin(baseorigin + realmove);
        gi.cvar_set("viewthingorigin", va("%0.2f,%0.2f,%0.2f", edict->s.origin[0], edict->s.origin[1], edict->s.origin[2]));
    }
    PostEvent(EV_ViewThing_Think, FRAMETIME);
    if ((animstate > 0) && (Viewmodel.current_viewthing == this))
    {
        f = GetTime() / gi.Anim_Frametime(edict->tiki, CurrentAnim());
        if (f != lastframe)
        {
            float time;
            lastframe = f;
            time = f * AnimTime() / NumFrames();
            gi.Printf("current frame %d time %.2f\n", f, time);
            gi.cvar_set("viewmodeltime", va("%.2f", time));
            gi.cvar_set("viewmodelframe", va("%d", f));
            gi.cvar_set("viewmodelanim", AnimName());
        }
    }
}

void Viewthing::LastFrameEvent
	(
	Event *ev
	)

	{
	}

void Viewthing::ToggleAnimateEvent
	(
	Event *ev
	)

	{
   animstate = ( animstate + 1 ) % 4;
   setOrigin( baseorigin );
   // reset to a known state
   switch( animstate )
      {
      case 0:
         SetFrame();         
         gi.Printf( "Animation stopped.\n" );
         gi.cvar_set( "viewmodelanimmode", "Stopped" );
         break;
      case 1:
         NewAnim( CurrentAnim() );
         gi.Printf( "Animation no motion.\n" );
         gi.cvar_set( "viewmodelanimmode", "No Motion" );
         break;
      case 2:
         NewAnim( CurrentAnim(), EV_ViewThing_LastFrame );
         gi.Printf( "Animation with motion and looping.\n" );
         gi.cvar_set( "viewmodelanimmode", "Motion and Looping" );
         break;
      case 3:
         NewAnim( CurrentAnim(), EV_ViewThing_LastFrame );
         gi.Printf( "Animation with motion no looping.\n" );
         gi.cvar_set( "viewmodelanimmode", "Motion and No Looping" );
         break;
      }
   UpdateCvars( qtrue );
	}

void Viewthing::SetModelEvent
	(
	Event *ev
	)

	{
	str modelname;

   modelname = ev->GetString( 1 );

   setModel( modelname );

   if( gi.modeltiki( model ) )
      {
	   NewAnim( 0 );
      SetFrame( );
      }
   UpdateCvars();
	}

void Viewthing::NextFrameEvent
	(
	Event *ev
	)

	{
   int numframes;

   numframes = NumFrames();
   if ( numframes )
      {
      frame = (frame+1)%numframes;
      SetFrame();
      animstate = 0;
      UpdateCvars();
      }
	}

void Viewthing::PrevFrameEvent
	(
	Event *ev
	)

	{
   int numframes;

   numframes = NumFrames();
   if ( numframes )
      {
      frame = (frame-1)%numframes;
      SetFrame();
      animstate = 0;
      UpdateCvars();
      }
	}

void Viewthing::SetAnim
	(
	Event *ev
	)

	{
   int numanims, anim;

   numanims = NumAnims();
   if ( numanims )
      {
      // restore original surfaces
      memcpy( edict->s.surfaces, origSurfaces, sizeof( origSurfaces ) );

      anim = ev->GetFloat( 1 ) * numanims;
      if ( anim >= numanims )
         anim = numanims - 1;
	   NewAnim( anim % numanims );
      frame = 0;
      SetFrame();
      animstate = 0;
      UpdateCvars();
      }
	}


void Viewthing::NextAnimEvent
	(
	Event *ev
	)

	{
   int numanims;

   numanims = NumAnims();
   if ( numanims )
      {
      // restore original surfaces
      memcpy( edict->s.surfaces, origSurfaces, sizeof( origSurfaces ) );

	   NewAnim( ( CurrentAnim() + 1 ) % numanims );
      frame = 0;
      SetFrame();
      animstate = 0;
      UpdateCvars();
      }
	}

void Viewthing::PrevAnimEvent
	(
	Event *ev
	)

	{
   int anim;
   int numanims;

   numanims = NumAnims();
   if ( numanims )
      {
      // restore original surfaces
      memcpy( edict->s.surfaces, origSurfaces, sizeof( origSurfaces ) );

	   anim = CurrentAnim() - 1;
      while( anim < 0 )
         {
         anim += numanims;
         }
	   NewAnim( anim );
      frame = 0;
      SetFrame();
      animstate = 0;
      UpdateCvars();
      }
	}

void Viewthing::ScaleUpEvent
	(
	Event *ev
	)

	{
   edict->s.scale += 0.01f;
   UpdateCvars();
	}

void Viewthing::ScaleDownEvent
	(
	Event *ev
	)

	{
   edict->s.scale -= 0.01f;
   UpdateCvars();
	}

void Viewthing::SetScaleEvent
	(
	Event *ev
	)

	{
   float s;

   if ( ev->NumArgs() )
      {
      s = ev->GetFloat( 1 );
      edict->s.scale = s;
      UpdateCvars();
      }
   else
      {
      gi.Printf( "viewscale = %f\n", edict->s.scale );
      }
   }

void Viewthing::SetYawEvent
	(
	Event *ev
	)

	{
	if ( ev->NumArgs() > 0 )
		{
		angles.setYaw( ev->GetFloat( 1 ) );
		setAngles( angles );
		}
   gi.Printf( "yaw = %f\n", angles.yaw() );
	}

void Viewthing::SetPitchEvent
	(
	Event *ev
	)

	{
	if ( ev->NumArgs() > 0 )
		{
		angles.setPitch( ev->GetFloat( 1 ) );
		setAngles( angles );
		}
   gi.Printf( "pitch = %f\n", angles.pitch() );
	}

void Viewthing::SetRollEvent
	(
	Event *ev
	)

	{
	if ( ev->NumArgs() > 0 )
		{
		angles.setRoll( ev->GetFloat( 1 ) );
		setAngles( angles );
		}
   gi.Printf( "roll = %f\n", angles.roll() );
	}

void Viewthing::SetAnglesEvent
	(
	Event *ev
	)

	{
	if ( ev->NumArgs() > 2 )
		{
		angles.x = ev->GetFloat( 1 );
		angles.y = ev->GetFloat( 2 );
		angles.z = ev->GetFloat( 3 );
		setAngles( angles );
		}

   gi.Printf( "angles = %f, %f, %f\n", angles.x, angles.y, angles.z );
	}

void Viewthing::AttachModel
	(
	Event *ev
	)

	{
   Event * event;
   Viewthing * child;


   child = new Viewthing;
   child->setModel( ev->GetString( 2 ) );

   // 
   // attach the child
   //
   event = new Event( EV_Attach );
   event->AddEntity( this );
   event->AddString( ev->GetString( 1 ) );
   child->ProcessEvent( event );
	}

void Viewthing::Delete
	(
	Event *ev
	)

	{
   Viewmodel.current_viewthing = NULL;
   PostEvent( EV_Remove, 0 );
   Viewmodel.PostEvent( EV_ViewThing_Next, 0 );
	}

void Viewthing::DetachAll
	(
	Event *ev
	)

	{
   int i;
   int num;

   num = numchildren;
   for (i=0;i<MAX_MODEL_CHILDREN;i++)
      {
      Entity * ent;
      if (!children[i])
         continue;
      ent = ( Entity * )G_GetEntity( children[i] );
      ent->PostEvent( EV_Remove, 0 );
      num--;
      if (!num)
         break;
      }
	}

void Viewthing::ChangeOrigin
	(
	Event *ev
	)

	{
   if ( ev->NumArgs() )
      {
      origin.x = ev->GetFloat( 1 );
      origin.y = ev->GetFloat( 2 );
      origin.z = ev->GetFloat( 3 );
      setOrigin( origin );
      baseorigin = origin;
      UpdateCvars();
      }
   gi.Printf( "vieworigin = x%f y%f z%f\n", origin.x, origin.y, origin.z );
   }

void Viewthing::SaveSurfaces
	(
	Event *ev
	)
	
	{
   memcpy( origSurfaces, edict->s.surfaces, sizeof( origSurfaces ) );
   }

