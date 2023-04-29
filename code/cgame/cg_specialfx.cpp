/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

// DESCRIPTION:
// Special Effects code

#include "cg_local.h"
#include "cg_commands.h"

extern refEntity_t *current_entity;
extern int current_tiki;

/*
==============================================================

FOOTSTEP CODE

==============================================================
*/

#define GROUND_DISTANCE 32
#define WATER_NO_SPLASH_HEIGHT 16

void CG_Footstep
	(
	centity_t * ent,
   float volume
	)

	{
   vec3_t   end, midlegs;
	trace_t	trace;
   int      contents, surftype;
	spawnthing_t   effect;
	refEntity_t    *old_entity;
	int            old_tiki;
	refEntity_t    new_entity;
	int            new_tiki;
	qhandle_t      hModel;

   // send a trace down from the player to the ground
   VectorCopy( ent->lerpOrigin, end );
   end[2] -= GROUND_DISTANCE;

	if ( ent->currentState.eType == ET_PLAYER )
      {
      CG_Trace( &trace, ent->lerpOrigin, NULL, NULL, end, ent->currentState.number, MASK_PLAYERSOLID, qtrue, qtrue, "Player Footsteps" );
      }
   else
      {
      CG_Trace( &trace, ent->lerpOrigin, NULL, NULL, end, ent->currentState.number, MASK_MONSTERSOLID, qfalse, qfalse, "Monster Footsteps" );
      }

   if ( trace.fraction == 1.0f )
      {
      return;
      }

	contents = CG_PointContents( trace.endpos, -1 );
   if ( contents & MASK_WATER )
      {
      // take our ground position and trace upwards 
      VectorCopy( trace.endpos, midlegs );
      midlegs[ 2 ] += WATER_NO_SPLASH_HEIGHT;
	   contents = CG_PointContents( midlegs, -1 );
      if ( contents & MASK_WATER )
         {
         commandManager.PlaySound( "footstep_wade", NULL, CHAN_AUTO, volume );
         if ( cg_debugFootsteps->integer )
            {
            cgi.DPrintf( "Footstep: wade    volume: %.2f\n", volume ); 
            }
         }
      else
         {
         commandManager.PlaySound( "footstep_splash", NULL, CHAN_AUTO, volume );
         if ( cg_debugFootsteps->integer )
            {
            cgi.DPrintf( "Footstep: splash  volume: %.2f\n", volume ); 
            }
         }
      }
   else
      {
      surftype = trace.surfaceFlags & MASK_SURF_TYPE;
		switch ( surftype )
			{
         case SURF_TYPE_WOOD:
            commandManager.PlaySound( "footstep_wood", NULL, CHAN_AUTO, volume );
            if ( cg_debugFootsteps->integer )
               {
               cgi.DPrintf( "Footstep: wood    volume: %.2f\n", volume ); 
               }
            break;
         case SURF_TYPE_METAL:
            commandManager.PlaySound( "footstep_metal", NULL, CHAN_AUTO, volume );
            if ( cg_debugFootsteps->integer )
               {
               cgi.DPrintf( "Footstep: metal   volume: %.2f\n", volume ); 
               }
            break;
         case SURF_TYPE_ROCK:
            commandManager.PlaySound( "footstep_rock", NULL, CHAN_AUTO, volume );
            if ( cg_debugFootsteps->integer )
               {
               cgi.DPrintf( "Footstep: rock    volume: %.2f\n", volume ); 
               }
            break;
         case SURF_TYPE_DIRT:
            memset( &new_entity, 0, sizeof( refEntity_t ) );
            
            commandManager.PlaySound( "footstep_dirt", NULL, CHAN_AUTO, volume );

            if ( cg_debugFootsteps->integer )
               {
               cgi.DPrintf( "Footstep: dirt    volume: %.2f\n", volume ); 
               }

				// Save the old stuff

				old_entity = current_entity;
				old_tiki   = current_tiki;

				// Setup the new entity
            memset( &new_entity.shaderRGBA, 0xff, sizeof( new_entity.shaderRGBA ) );
				new_entity.origin[0] = trace.endpos[0];
				new_entity.origin[1] = trace.endpos[1];
				new_entity.origin[2] = trace.endpos[2] + 5;
            new_entity.scale     = 1;

				current_entity = &new_entity;

				// Setup the new tiki

				hModel = cgi.R_RegisterModel( "models/fx_dirtstep.tik" );
				new_tiki = cgi.TIKI_GetHandle( hModel );
				current_tiki = new_tiki;

				// Process new entity

				CG_ProcessInitCommands( current_tiki );

				// Put the old stuff back

				current_entity = old_entity;
				current_tiki   = old_tiki;

				/* commandManager.InitializeSpawnthing( &effect );

				effect.SetModel( "models/fx_dirtstep.tik" );

				effect.cgd.origin[0]	= trace.endpos[0];
				effect.cgd.origin[1]	= trace.endpos[1];
				effect.cgd.origin[2]	= trace.endpos[2] + 5;   
   
				effect.cgd.scale = 1;

				//effect.cgd.scaleRate = 1;

				effect.cgd.life = 2000;

				effect.cgd.flags |= T_FADE;

				effect.cgd.flags |= T_ANGLES;

				effect.randangles[ 0 ] = NOT_RANDOM;
				effect.cgd.angles[ 0 ] = 0;
				effect.randangles[ 1 ] = NOT_RANDOM;
				effect.cgd.angles[ 1 ] = 0;
				effect.randangles[ 2 ] = NOT_RANDOM;
				effect.cgd.angles[ 2 ] = 0;

				commandManager.SpawnTempModel( 1, &effect ); */

            break;
         case SURF_TYPE_GRILL:
            commandManager.PlaySound( "footstep_grill", NULL, CHAN_AUTO, volume );
            if ( cg_debugFootsteps->integer )
               {
               cgi.DPrintf( "Footstep: grill   volume: %.2f\n", volume ); 
               }
            break;
         case SURF_TYPE_ORGANIC:
            commandManager.PlaySound( "footstep_organic", NULL, CHAN_AUTO, volume );
            if ( cg_debugFootsteps->integer )
               {
               cgi.DPrintf( "Footstep: organic volume: %.2f\n", volume ); 
               }
            break;
         case SURF_TYPE_SQUISHY:
            commandManager.PlaySound( "footstep_squishy", NULL, CHAN_AUTO, volume );
            if ( cg_debugFootsteps->integer )
               {
               cgi.DPrintf( "Footstep: squishy volume: %.2f\n", volume ); 
               }
            break;
         }
      }
	}

/*
===============
CG_Splash

Draw a mark at the water surface
===============
*/
void CG_Splash( centity_t *cent ) {
	vec3_t		start, end, diff;
	trace_t		trace;
	int			contents;
	float			dist;
	float			time_required;

	spawnthing_t   m_ripple;

	if ( !cg_shadows->integer ) {
		return;
	}

   VectorSubtract( cent->currentState.origin, cent->nextState.origin, diff );
   diff[ 2 ] = 0;
	dist = VectorLength( diff );

	// See if enough time has passed to add another ripple

	if ( dist >= 1 )
		time_required = 100 - dist;
	else
		time_required = 200;

	if ( time_required < 10 )
		time_required = 10;

	if ( cent->splash_last_spawn_time + time_required > cg.time )
		return;

	// Save the current time

	cent->splash_last_spawn_time = cg.time;

	// Make sure the entity is moving
	if ( dist < 1 )
		{
		if ( cent->splash_still_count >= 0 )
			{
			cent->splash_still_count++;

			if ( cent->splash_still_count > 2 )
				cent->splash_still_count = 0;
			else
				return;
			}
		else
			return;
		}


	VectorCopy( cent->lerpOrigin, end );

	// if the feet aren't in liquid, don't make a mark
	// this won't handle moving water brushes, but they wouldn't draw right anyway...
	contents = cgi.CM_PointContents( end, 0 );
	if ( !( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, start );
	start[2] += 88;

	// if the head isn't out of liquid, don't make a mark
	contents = cgi.CM_PointContents( start, 0 );
	if ( contents & ( CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		return;
	}

	// trace down to find the surface
	cgi.CM_BoxTrace( &trace, start, end, NULL, NULL, 0, ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ), qfalse );

	if ( trace.fraction == 1.0 ) {
		return;
	}

	// Add a ripple to the scene

	commandManager.InitializeSpawnthing( &m_ripple );

   m_ripple.SetModel( "ripple.spr" );

   m_ripple.cgd.origin[0]					= trace.endpos[0];
   m_ripple.cgd.origin[1]					= trace.endpos[1];
   m_ripple.cgd.origin[2]					= trace.endpos[2];   
   
   m_ripple.cgd.scale = 0.33 + dist / 100;

	if ( m_ripple.cgd.scale > 1 )
		m_ripple.cgd.scale = 1;

	m_ripple.cgd.scaleRate = 1 + dist / 250;

	if ( m_ripple.cgd.scaleRate > 1.4 )
		m_ripple.cgd.scaleRate = 1.4;

	m_ripple.cgd.life = 2000 - dist * 10;

	if ( m_ripple.cgd.life < 1000 )
		m_ripple.cgd.life = 1000;

	m_ripple.cgd.flags |= T_FADE;

	m_ripple.cgd.flags |= T_ANGLES;

	m_ripple.randangles[ 0 ] = NOT_RANDOM;
   m_ripple.cgd.angles[ 0 ] = 90;
	m_ripple.randangles[ 1 ] = NOT_RANDOM;
   m_ripple.cgd.angles[ 1 ] = 0;
	m_ripple.randangles[ 2 ] = NOT_RANDOM;
   m_ripple.cgd.angles[ 2 ] = 0;

   commandManager.SpawnTempModel( 1, &m_ripple );
}
