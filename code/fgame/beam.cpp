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

// beam.cpp:
// 

#include "beam.h"
#include "../qcommon/qfiles.h"
#include "game.h"

/*****************************************************************************/
/*QUAKED func_beam (0 0.25 .5) (-8 -8 -8) (8 8 8) START_ON PERSIST WAVE NOISE

This creates a beam effect from the origin to the target's origin. If no
target is specified, uses angles and projects beam out from there.

"model" Specifies the model to use as the beam
"overlap" Specifies the amount of overlap each beam link should have. Use this to fill
in the cracks when using electric on beams. (Default is 0)
"minoffset" Minimum amount of electrical variation (Default is 0)
"maxoffset" Maximum amount of electrical variation (Default is 5)
"color" Vector specifiying the red,green, and blue components. (Default is "1 1 1")
"alpha" Alpha of the beam (Default is 1.0)
"damage" Amount of damage the beam inflicts if beam hits someone (Default is 0)
"angles" Sets the angle of the beam if no target is specified.
"life" Sets the life of the beam for use with the persist spawnflag.  This is how long a beam will
be displayed.
"numsegments" Number of segments in a beam (Default is 4)
"delay" Delay between beam updates.  (i.e. slows the effect of the beam down)
"shader" Set the shader of the beam
"scale" Set the width of the beam

START_ON  - Starts the beam on
PERSIST   - Keeps the last few beams around and fades them out over the life of the beam
WAVE      - Make the beam follow a sin wave pattern
NOISE     - Use a more computationally expensive random effect, but the results are smoother

If the model field is not set, then a renderer generated beam will be created
using the color, minoffset, maxoffset, scale, and subdivisions fields

If the targetname is set, it will use the target specified as the endpoint of the beam

/*****************************************************************************/

Event EV_FuncBeam_Activate
   ( 
   "activate",
   EV_DEFAULT,
   NULL,
   NULL,
   "Activate the beam",
	EV_NORMAL
   );
Event EV_FuncBeam_Deactivate
   (
   "deactivate",
   EV_DEFAULT,
   NULL,
   NULL,
   "Deactivate the beam",
	EV_NORMAL
   );
Event EV_FuncBeam_Diameter
   (
   "diameter",
   EV_DEFAULT,
   "f",
   "diameter",
   "Set the diameter of the beam",
	EV_NORMAL
   );
Event EV_FuncBeam_Maxoffset
   (
   "maxoffset",
   EV_DEFAULT,
   "f",
   "max_offset",
   "Set the maximum offset the beam can travel above, below, forward or back of it's endpoints",
	EV_NORMAL
   );
Event EV_FuncBeam_Minoffset
   (
   "minoffset",
   EV_DEFAULT,
   "f",
   "min_offset",
   "Set the minimun offset the beam can travel above, below, forward or back of it's endpoints",
	EV_NORMAL
   );
Event EV_FuncBeam_Overlap
   (
   "overlap",
   EV_DEFAULT,
   "f",
   "beam_overlap",
   "Set the amount of overlap the beams have when they are being strung together",
	EV_NORMAL
   );
Event EV_FuncBeam_Color
   (
   "color",
   EV_DEFAULT,
   "v[0,1][0,1][0,1]",
   "beam_color",
   "Set the color of the beam",
	EV_NORMAL
   );
Event EV_FuncBeam_SetTarget
   (
   "target",
   EV_DEFAULT,
   "s",
   "beam_target",
   "Set the target of the beam. The beam will be drawn from the origin\n"
	"to the origin of the target entity",
	EV_NORMAL
   );
Event EV_FuncBeam_SetEndPoint
   (
   "endpoint",
   EV_DEFAULT,
   "v",
   "beam_end_point",
   "Set the end point of the beam. The beam will be draw from the origin to\n"
	"the end point.",
	EV_NORMAL
   );
Event EV_FuncBeam_SetLife
   (
   "life",
   EV_DEFAULT,
   "f",
   "beam_life",
   "Set the amount of time the beam stays on when activated",
	EV_NORMAL
   );
Event EV_FuncBeam_Shader
   (
   "shader",
   EV_DEFAULT,
   "s",
   "beam_shader",
   "Set the shader that the beam will use",
	EV_NORMAL
   );
Event EV_FuncBeam_TileShader
   (
   "tileshader",
   EV_DEFAULT,
   "s",
   "beam_shader",
   "Set the shader that the beam will use.  This shader will be tiled.",
	EV_NORMAL
   );
Event EV_FuncBeam_Segments
   (
   "numsegments",
   EV_DEFAULT,
   "i",
   "numsegments",
   "Set the number of segments for the beam",
	EV_NORMAL
   );
Event EV_FuncBeam_Delay
   (
   "delay",
   EV_DEFAULT,
   "f",
   "delay",
   "Set the amount of delay on the beam updater",
	EV_NORMAL
   );
Event EV_FuncBeam_NumSphereBeams
   (
   "numspherebeams",
   EV_DEFAULT,
   "i",
   "num",
   "Set the number of beams that will be shot out in a sphere like formation",
	EV_NORMAL
   );
Event EV_FuncBeam_SphereRadius
   (
   "radius",
   EV_DEFAULT,
   "f",
   "radius",
   "Set the starting radius of the beams if this is a beamsphere",
	EV_NORMAL
   );
Event EV_FuncBeam_ToggleDelay
   (
   "toggledelay",
   EV_DEFAULT,
   "SF",
   "[random] time",
   "Causes a beam toggling effect.  Sets the time between toggling.  If random is specified, The time will be between 0 and time",
	EV_NORMAL
   );
Event EV_FuncBeam_FindEndpoint
   (
   "findendpoint",
   EV_DEFAULT,
   NULL,
   NULL,
   "Find the endpoint of a beam",
	EV_NORMAL
   );
Event EV_FuncBeam_UpdateEndpoint
   (
   "updateendpoint",
   EV_DEFAULT,
   NULL,
   NULL,
   "Update the endpoint of a beam",
	EV_NORMAL
   );
Event EV_FuncBeam_UpdateOrigin
   (
   "updateorigin",
   EV_DEFAULT,
   NULL,
   NULL,
   "Update the origin of a beam",
	EV_NORMAL
   );
Event EV_FuncBeam_EndAlpha
   (
   "endalpha",
   EV_DEFAULT,
   "f",
   "alpha",
   "Set the endpoint alpha value of the beam",
	EV_NORMAL
   );
Event EV_FuncBeam_Shoot
   (
   "shoot",
   EV_DEFAULT,
   NULL,
   NULL,
   "Make the beam cause damage to entities that get in the way",
	EV_NORMAL
   );
Event EV_FuncBeam_ShootRadius
   (
   "shootradius",
   EV_DEFAULT,
   "f",
   "radius",
   "Set the radius of the damage area between beam endpoints",
	EV_NORMAL
   );
Event EV_FuncBeam_Persist
   (
   "persist",
   EV_DEFAULT,
   "b",
   "bool",
   "Set the persist property of the beam",
	EV_NORMAL
   );
Event EV_FuncBeam_IgnoreWalls
	(
	"ignorewalls",
	EV_DEFAULT,
	"B",
	"bool",
	"Set if the beam can traverse walls",
	EV_NORMAL
	);

CLASS_DECLARATION( ScriptSlave, FuncBeam, "func_beam" )
{
	{ &EV_Activate,						&FuncBeam::Activate },
	{ &EV_FuncBeam_Activate,			&FuncBeam::Activate },
	{ &EV_FuncBeam_Deactivate,			&FuncBeam::Deactivate },
	{ &EV_FuncBeam_Maxoffset,			&FuncBeam::SetMaxoffset },
	{ &EV_FuncBeam_Minoffset,			&FuncBeam::SetMinoffset },
	{ &EV_FuncBeam_Overlap,				&FuncBeam::SetOverlap },
	{ &EV_FuncBeam_Color,				&FuncBeam::SetColor },
	{ &EV_SetAngle,						&FuncBeam::SetAngle },
	{ &EV_FuncBeam_Segments,			&FuncBeam::SetSegments },
	{ &EV_SetAngles,					&FuncBeam::SetAngles },
	{ &EV_FuncBeam_SetEndPoint,			&FuncBeam::SetEndPoint },
	{ &EV_Model,						&FuncBeam::SetModel },
	{ &EV_Damage,						&FuncBeam::SetDamage },
	{ &EV_FuncBeam_SetLife,				&FuncBeam::SetLife },
	{ &EV_FuncBeam_Shader,				&FuncBeam::SetBeamShader },
	{ &EV_FuncBeam_TileShader,			&FuncBeam::SetBeamTileShader },
	{ &EV_FuncBeam_Delay,				&FuncBeam::SetDelay },
	{ &EV_FuncBeam_NumSphereBeams,		&FuncBeam::SetNumSphereBeams },
	{ &EV_FuncBeam_SphereRadius,		&FuncBeam::SetSphereRadius },
	{ &EV_FuncBeam_ToggleDelay,			&FuncBeam::SetToggleDelay },
	{ &EV_FuncBeam_ShootRadius,			&FuncBeam::SetShootRadius },
	{ &EV_FuncBeam_EndAlpha,			&FuncBeam::SetEndAlpha },
	{ &EV_FuncBeam_Persist,				&FuncBeam::SetPersist },
	{ &EV_FuncBeam_FindEndpoint,		&FuncBeam::FindEndpoint },
	{ &EV_FuncBeam_UpdateEndpoint,		&FuncBeam::UpdateEndpoint },
	{ &EV_FuncBeam_UpdateOrigin,		&FuncBeam::UpdateOrigin },
	{ &EV_FuncBeam_Shoot,				&FuncBeam::Shoot },
	{ &EV_FuncBeam_IgnoreWalls,			&FuncBeam::SetIgnoreWalls },
	{ NULL, NULL }
};

FuncBeam::FuncBeam()
{
	setSolidType( SOLID_NOT );
 	setOrigin();

	damage			= 0;
	life			= 0;
	shootradius		= 0;
	use_angles		= false;
	m_bIgnoreWalls	= false;
   
	edict->s.renderfx    |= RF_BEAM;
	edict->s.eType       = ET_BEAM;		// Entity type beam
	edict->s.modelindex  = 1;			// must be non-zero
	SetBeamShader( "beamshader" );

	if ( !LoadingSavegame )
	{
		edict->s.alpha				= 1;				// alpha  
		edict->s.surfaces[4]		= 4;				// num segments
		edict->s.surfaces[0]		= 16;				// life
		edict->s.surfaces[9]		= 16;				// endalpha
		edict->s.beam_entnum		= ENTITYNUM_NONE;
		edict->s.bone_angles[0][1]	= 5;				// Max offset
		
		if ( spawnflags & 0x0001 ) // Start On
			PostEvent( EV_Activate, EV_POSTSPAWN );
		else
			hideModel();
		edict->s.skinNum = 0;
		if ( spawnflags & 0x0002 ) 
			edict->s.skinNum |= BEAM_PERSIST_EFFECT;
		if ( spawnflags & 0x0004 ) 
			edict->s.skinNum |= BEAM_WAVE_EFFECT;
		if ( spawnflags & 0x0008 ) 
			edict->s.skinNum |= BEAM_USE_NOISE;
		
		// Try to find the endpoint of this beam after everything has been spawned
		PostEvent( EV_FuncBeam_FindEndpoint, EV_LINKBEAMS );
	}
}

void FuncBeam::SetEndAlpha
   (
   Event *ev
   )

   {
   BEAM_PARM_TO_PKT( ev->GetFloat( 1 ), edict->s.surfaces[9] );
   }

void FuncBeam::SetToggleDelay
   (
   Event *ev
   )

   {
   edict->s.skinNum |= BEAM_TOGGLE;
   
   if ( ev->NumArgs() > 2 )
      {
      str arg = ev->GetString( 1 );
      if ( !arg.icmp( "random" ) )
         {
         edict->s.skinNum |= BEAM_RANDOM_TOGGLEDELAY;
         }
      BEAM_PARM_TO_PKT( ev->GetFloat( 2 ), edict->s.surfaces[8] );
      }
   else
      {
      BEAM_PARM_TO_PKT( ev->GetFloat( 1 ), edict->s.surfaces[8] );
      }   
   }

void FuncBeam::SetSphereRadius
   (
   Event *ev
   )

   {
   edict->s.skinNum |= BEAM_SPHERE_EFFECT;
   BEAM_PARM_TO_PKT( ev->GetFloat( 1 ), edict->s.surfaces[7] );
   }

void FuncBeam::SetNumSphereBeams
   (
   Event *ev
   )

   {
   edict->s.skinNum |= BEAM_SPHERE_EFFECT;
   edict->s.surfaces[6] = ev->GetInteger( 1 );
   }

void FuncBeam::SetAngle
   (
   Event *ev
   )

   {
   Vector   movedir;

   movedir = G_GetMovedir( ev->GetFloat( 1 ) );
	setAngles( movedir.toAngles() );
   }

void FuncBeam::SetAngles
   (
   Event *ev
   )

   {
   setAngles( ev->GetVector( 1 ) );
   }

// Override setAngles to update the endpoint of the beam if it's rotated
void FuncBeam::setAngles
	(
	Vector angles
	)
{
	trace_t  trace;
	Vector   endpoint;

	ScriptSlave::setAngles( angles );

	// If there is no target, then use the angles to determine where to put the 
	// endpoint
	if( !end )
	{
		endpoint = origin + Vector( orientation[ 0 ] ) * MAP_SIZE;

		trace = G_Trace( origin, vec_zero, vec_zero, endpoint, this, MASK_LINE, false, "FuncBeam" );

		VectorCopy( trace.endpos, edict->s.origin2 );

		use_angles = true;
	}
}

void FuncBeam::SetEndPoint
	(
	Event *ev
	)
{
	trace_t  trace;

	end_point = ev->GetVector( 1 );

	trace = G_Trace( origin, vec_zero, vec_zero, end_point, this, MASK_LINE, false, "FuncBeam" );
	VectorCopy( trace.endpos, edict->s.origin2 );

	use_angles = false;
}

void FuncBeam::SetModel
	(
	Event *ev
	)
{
	setModel( ev->GetString( 1 ) );
	edict->s.renderfx &= ~RF_BEAM;
	edict->s.eType = ET_BEAM;
	edict->s.skinNum |= BEAM_USEMODEL;
}

void FuncBeam::SetDamage
   (
   Event *ev
   )

   {
   damage = ev->GetFloat( 2 );
   }

void FuncBeam::SetLife
   (
   Event *ev
   )

   {
   BEAM_PARM_TO_PKT( ev->GetFloat( 1 ), edict->s.surfaces[0] );
   }

void FuncBeam::SetColor
   (
   Event *ev
   )

   {
   Vector color = ev->GetVector( 1 );
   G_SetConstantLight( &edict->s.constantLight, &color[ 0 ], &color[ 1 ], &color[ 2 ], NULL );
   }

void FuncBeam::SetSegments
   (
   Event *ev
   )

   {
   edict->s.surfaces[4] = ev->GetInteger( 1 );
   }

void FuncBeam::SetBeamShader
   (
   str beam_shader
   )

   {
   str temp_shader;

   shader = beam_shader;
   edict->s.tag_num     = gi.imageindex( shader );

	temp_shader = shader + ".spr";
	CacheResource( temp_shader );
   }

void FuncBeam::SetBeamShader
   (
   Event *ev
   )

   {
   SetBeamShader( ev->GetString( 1 ) );
   }

void FuncBeam::SetBeamTileShader
   (
   Event *ev
   )

   {
   SetBeamShader( ev->GetString( 1 ) );
   edict->s.skinNum |= BEAM_TILESHADER;
   }

void FuncBeam::SetMaxoffset
   (
   Event *ev
   )

   {
   edict->s.bone_angles[0][1] = ev->GetFloat( 1 );
   }

void FuncBeam::SetMinoffset
   (
   Event *ev
   )

   {
   edict->s.bone_angles[0][0] = ev->GetFloat( 1 );
   }

void FuncBeam::SetOverlap
   (
   Event *ev
   )

   {
   BEAM_PARM_TO_PKT( ev->GetFloat( 1 ), edict->s.surfaces[3] );
   }

void FuncBeam::SetDelay
   (
   Event *ev
   )

   { 
   if ( ev->NumArgs() > 2 )
      {
      str arg = ev->GetString( 1 );
      if ( !arg.icmp( "random" ) )
         {
         edict->s.skinNum |= BEAM_RANDOM_DELAY;
         }
      BEAM_PARM_TO_PKT( ev->GetFloat( 2 ), edict->s.surfaces[5] );
      }
   else
      {
      BEAM_PARM_TO_PKT( ev->GetFloat( 1 ), edict->s.surfaces[5] );
      }   
   }

void FuncBeam::Deactivate
   (
   Event *ev
   )

   {
   hideModel();

   // Cancel all of the events to activate
   CancelEventsOfType( EV_FuncBeam_Activate );
   CancelEventsOfType( EV_Activate );
   CancelEventsOfType( EV_FuncBeam_UpdateEndpoint );
   CancelEventsOfType( EV_FuncBeam_UpdateOrigin );
   }

void FuncBeam::SetShootRadius
   (
   Event *ev
   )

   {
   shootradius = ev->GetFloat( 1 );
   }

void FuncBeam::SetPersist
   (
   Event *ev
   )

   {
   qboolean persist = ev->GetBoolean( 1 );

   if ( persist )
      edict->s.skinNum |= BEAM_PERSIST_EFFECT;
   else 
      edict->s.skinNum &= ~BEAM_PERSIST_EFFECT;
   }

void FuncBeam::Shoot
   (
   Event *ev
   )

   {
   trace_t  trace;
   Vector   start, end;

   start = edict->s.origin;
   end   = edict->s.origin2;

   Vector   dir( end - start );
   Vector   b1( -shootradius, -shootradius, -shootradius );
   Vector   b2( shootradius, shootradius, shootradius );

   trace = G_Trace( start, b1, b2, end, this, MASK_SHOT, false, "FuncBeam::Activate" );

   if ( trace.ent && trace.ent->entity && trace.ent->entity->takedamage )
      {
      // damage the ent
   	trace.ent->entity->Damage( this,
                                 this,
                                 damage,
                                 trace.endpos,
                                 dir,
                                 trace.plane.normal,
                                 0,
                                 0,
                                 MOD_BEAM
                                 );
      }

   PostEvent( EV_FuncBeam_Shoot, 0.1f );
   }

void FuncBeam::SetIgnoreWalls
	(
	Event *ev
	)
{
	if( ev->NumArgs() > 0 )
	{
		m_bIgnoreWalls = ev->GetBoolean( 1 );
	}
	else
	{
		m_bIgnoreWalls = true;
	}
}

void FuncBeam::Activate
	(
	Event *ev
	)
{
	Vector      forward;
	trace_t     trace;

	showModel();

	// An entity is targeted
	if( end )
	{
		VectorCopy( end->origin, edict->s.origin2 );
		// Post an event so that the beam will update itself every frame
		PostEvent( EV_FuncBeam_UpdateEndpoint, level.frametime );
	}
	else if( use_angles )
	{
		angles.AngleVectorsLeft( &forward, NULL, NULL );

		Vector endpoint( orientation[ 0 ] );
		endpoint *= MAP_SIZE;

		if( m_bIgnoreWalls )
		{
			VectorCopy( endpoint, trace.endpos );
		}
		else
		{
			trace = G_Trace( origin, vec_zero, vec_zero, endpoint, this, MASK_LINE, false, "FuncBeam::Activate" );
		}

		VectorCopy( trace.endpos, edict->s.origin2 );
	}
	else
	{
		if( m_bIgnoreWalls )
		{
			VectorCopy( end_point, trace.endpos );
		}
		else
		{
			trace = G_Trace( origin, vec_zero, vec_zero, end_point, this, MASK_LINE, false, "FuncBeam::Activate" );
		}

		VectorCopy( trace.endpos, edict->s.origin2 );
	}

	if( origin_target )
	{
		PostEvent( EV_FuncBeam_UpdateOrigin, level.frametime );
	}

	if( damage )
	{
		// Shoot beam and cause damage every frame
		ProcessEvent( EV_FuncBeam_Shoot );
	}

	// If life is set, then post a deactivate message
	if( life > 0 && !EventPending( EV_FuncBeam_Deactivate ) )
	{
		PostEvent( EV_FuncBeam_Deactivate, life );
		return;
	}
}

void FuncBeam::UpdateEndpoint
   (
   Event *ev
   )

   {
   if ( end )
      {
      Event *ev1 = new Event( ev );

      VectorCopy( end->origin, edict->s.origin2 );
	  PostEvent( ev1, level.frametime );
      }
   }

void FuncBeam::UpdateOrigin
   (
   Event *ev
   )

   {
   if ( origin_target )
      {
      Event *ev1 = new Event( ev );

      setOrigin(  origin_target->centroid );
	  PostEvent( ev1, level.frametime );
      }
   }

void FuncBeam::FindEndpoint
   (
   Event *ev
   )

   {
   if ( target && strlen( target ) )
      {
      end = ( Entity * )G_FindTarget( NULL, target );
      if ( end )
         {
         VectorCopy( end->origin, edict->s.origin2 );
         }
      }
   }

FuncBeam *CreateBeam
   (
   const char *model,
   const char *shader,
   Vector start,
   Vector end,
   int numsegments,
   float scale,
   float life,
   float damage,
   Entity *origin_target
   )

   {
   FuncBeam *beam;
   trace_t  trace;

   // set start point
   beam = new FuncBeam;
   beam->setOrigin( start );

   if ( origin_target )
      beam->origin_target = origin_target;

   // set endpoint
	beam->end_point = end;
	trace = G_Trace( start, vec_zero, vec_zero, end, beam, MASK_LINE, false, "CreateBeam" );
   VectorCopy( trace.endpos, beam->edict->s.origin2 );
	beam->use_angles = false;

   if ( model )
      {
      // set the model if we have one
      beam->setModel( model );
      beam->edict->s.renderfx &= ~RF_BEAM;
      beam->edict->s.eType    = ET_BEAM;
      beam->edict->s.skinNum |= BEAM_USEMODEL;
      }

   if ( shader )
      {
      // Set the shader as an image configstring   
      beam->SetBeamShader( shader );
      }

   // set num segments
   beam->edict->s.surfaces[4] = numsegments;

   // set scale
   beam->setScale( scale );

   // set life
   BEAM_PARM_TO_PKT( life, beam->edict->s.surfaces[0] );
	beam->life = life;

   // set damage
   beam->damage = damage;

   // activate it
   beam->ProcessEvent( EV_Activate );
   beam->PostEvent( EV_Remove, life );
   return beam;
   }
