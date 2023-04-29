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

// cg_ents.c -- present snapshot entities, happens every single frame

#include "cg_local.h"

/*
==========================================================================

FUNCTIONS CALLED EACH FRAME

==========================================================================
*/

/*
======================
CG_SetEntitySoundPosition
======================
*/
void CG_SetEntitySoundPosition( centity_t *cent )
   {
	vec3_t	origin;

	if ( cent->currentState.solid == SOLID_BMODEL )
      {
		float  *v;
		vec3_t vel;

		v = cgs.inlineModelMidpoints[ cent->currentState.modelindex ];
		VectorAdd( cent->lerpOrigin, v, origin );

		vel[0] = 0.0;
		vel[1] = 0.0;
		vel[2] = 0.0;

   	cgi.S_UpdateEntity( cent->currentState.number, origin, vel, qfalse );
	   } 
   else
      {
		if ( cent && cg.snap && cent->currentState.parent == cg.snap->ps.clientNum )
			{
			vec3_t origin;
			vec3_t velocity;

			origin[0] = 0;
			origin[1] = 0;
			origin[2] = 0;

			velocity[0] = 0;
			velocity[1] = 0;
			velocity[2] = 0;

			cgi.S_UpdateEntity( cent->currentState.number, origin, velocity, qtrue );
			}
		else
			{
			CG_GetOrigin( cent, origin );
			cgi.S_UpdateEntity( cent->currentState.number, origin, cent->currentState.pos.trDelta, qfalse );
			}
	   }
   }

/*
==================
CG_EntityEffects

Add continuous entity effects, like local entity emission and lighting
==================
*/
void CG_EntityEffects( centity_t *cent )
   {
   // initialize with the client colors
   cent->color[ 0 ] = cent->client_color[ 0 ];
   cent->color[ 1 ] = cent->client_color[ 1 ];
   cent->color[ 2 ] = cent->client_color[ 2 ];
   cent->color[ 3 ] = cent->client_color[ 3 ];

	if ( cent->currentState.constantLight != 0xffffff )
      {
		int		style;
      unsigned cl;
		float		i, r, g, b;

		cl = cent->currentState.constantLight;
      style = ( cl & 255 );
		r = ( float )style / 255.0f;
		g = ( float )( ( cl >> 8 ) & 255 ) / 255.0f;
		b = ( float )( ( cl >> 16 ) & 255 ) / 255.0f;
		i = ( ( cl >> 24 ) & 255 ) * CONSTANTLIGHT_RADIUS_SCALE;
      if ( cent->currentState.renderfx & RF_LIGHTSTYLE_DLIGHT )
         {
         float color[ 4 ];

         CG_LightStyleColor( style, cg.time, color, qfalse );
         r = color[ 0 ];
         g = color[ 1 ];
         b = color[ 2 ];
         i *= color[ 3 ];
         }
      if ( i )
         {
         int flags;

         flags = 0;
         if ( cent->currentState.renderfx & RF_LENSFLARE )
            {
            flags |= lensflare;
            }
         else if ( cent->currentState.renderfx & RF_VIEWLENSFLARE )
            {
            flags |= viewlensflare;
            }
         if ( cent->currentState.renderfx & RF_ADDITIVE_DLIGHT )
            {
            flags |= additive;
            }
         cgi.R_AddLightToScene( cent->lerpOrigin, i, r, g, b, flags );
         }
      if ( r < cent->color[ 0 ] )
         cent->color[ 0 ] = r;
      if ( g < cent->color[ 1 ] )
         cent->color[ 1 ] = g;
      if ( b < cent->color[ 2 ] )
         cent->color[ 2 ] = b;
	   }
   }

/*
==================
CG_General
==================
*/
void CG_General( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;
   int               i;
   int               tikihandle;

	s1 = &cent->currentState;

   // add loop sound
   if ( s1->loopSound )
      {
      cgi.S_AddLoopingSound( cent->lerpOrigin, vec3_origin, cgs.sound_precache[s1->loopSound], s1->loopSoundVolume, s1->loopSoundMinDist );
      }
   if ( cent->tikiLoopSound )
      cgi.S_AddLoopingSound( cent->lerpOrigin, vec3_origin, cent->tikiLoopSound, cent->tikiLoopSoundVolume, cent->tikiLoopSoundMinDist );

	// if set to invisible, skip
	if (!s1->modelindex) {
		return;
	}

	memset (&ent, 0, sizeof(ent));

	// set frame

	ent.frame = s1->frame;
	ent.oldframe = ent.frame;
	ent.backlerp = 0;
   ent.uselegs = qtrue;

	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

   // set skin
   ent.skinNum = s1->skinNum;
   ent.customSkin = 0;

	ent.hModel = cgs.model_draw[s1->modelindex];

   // set surfaces
   memcpy( ent.surfaces, s1->surfaces, MAX_MODEL_SURFACES );

   // Modulation based off the color
   for( i=0; i<3; i++ )
      ent.shaderRGBA[ i ] = cent->color[ i ] * 255;

   // take the alpha from the entity if less than 1, else grab it from the client commands version
   if ( s1->alpha < 1 )
      {
      ent.shaderRGBA[ 3 ] = s1->alpha * 255;
      }
   else
      {
      ent.shaderRGBA[ 3 ] = cent->color[ 3 ] * 255;
      }

   // convert angles to axis
	AnglesToAxis( cent->lerpAngles, ent.axis );

   // Interpolated state variables
   if ( cent->interpolate )
      {
      ent.scale = s1->scale + cg.frameInterpolation * ( cent->nextState.scale - cent->currentState.scale );
      }
   else
      {
      ent.scale = s1->scale;
      }
   // set the entity number
   ent.entityNumber = s1->number;

   // copy shader specific data
   ent.shader_data[ 0 ] = s1->tag_num;
   ent.shader_data[ 1 ] = s1->skinNum;
   ent.renderfx |= s1->renderfx;

   if ( ent.renderfx & RF_SKYORIGIN )
      {
      memcpy( cg.sky_axis, ent.axis, sizeof( cg.sky_axis ) );
      VectorCopy( ent.origin, cg.sky_origin );
      }

   tikihandle = cgs.model_tiki[ s1->modelindex ]; 
   if ( tikihandle >= 0 )
      {
      // update any emitter's...
      CG_UpdateEntity( tikihandle, &ent, cent );
      }

   if ( s1->eFlags & ( EF_LEFT_TARGETED | EF_RIGHT_TARGETED ) )
      {
      CG_EntityTargeted( tikihandle, cent, &ent );
      }

	// add to refresh list
	if ( !( ent.renderfx & RF_DONTDRAW ) )
   	cgi.R_AddRefEntityToScene (&ent);
}

/*
==================
CG_Speaker

Speaker entities can automatically play sounds
==================
*/
void CG_Speaker( centity_t *cent )
   {
	if ( ! cent->currentState.clientNum ) // FIXME: use something other than clientNum...
      {	
		return;		// not auto triggering
	   }

	if ( cg.time < cent->miscTime )
      {
		return;
	   }
	
	// FIXME
   //cgi.S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.sound_precache[cent->currentState.eventParm] );

	//	ent->s.frame = ent->wait * 10;
	//	ent->s.clientNum = ent->random * 10;
	cent->miscTime = cg.time + cent->currentState.frame * 100 + cent->currentState.clientNum * 100 * crandom();
   }


/*
===============
CG_Mover
===============
*/
void CG_Mover( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);
	AnglesToAxis( cent->lerpAngles, ent.axis );

	ent.renderfx &= ~RF_SHADOW;

	// flicker between two skins (FIXME?)
	ent.skinNum = ( cg.time >> 6 ) & 1;

	// get the model, either as a bmodel or a modelindex
	if ( s1->solid == SOLID_BMODEL ) {
		ent.hModel = cgs.inlineDrawModel[s1->modelindex];
	} else {
		ent.hModel = cgs.model_draw[s1->modelindex];
	}

	// add to refresh list
	cgi.R_AddRefEntityToScene(&ent);
}

/*
===============
CG_Sprite
===============
*/
void CG_Sprite( centity_t *cent )
   {
   refEntity_t    ent;
	entityState_t	*s1;
   int            i;   

   s1 = &cent->currentState;

   memset( &ent, 0, sizeof( refEntity_t ) );

   if ( s1->parent != ENTITYNUM_NONE )
      {
      refEntity_t *parent;
      int         tikihandle;

      parent = cgi.R_GetRenderEntity( s1->parent );

      if ( !parent )
         {
         cgi.DPrintf( "CG_Sprite: Could not find parent entity\n" );
         return;
         }

      tikihandle = cgi.TIKI_GetHandle( parent->hModel );
      CG_AttachEntity( &ent, parent, tikihandle, s1->tag_num & TAG_MASK, s1->attach_use_angles, s1->attach_offset );
      }
   else
      {
   	VectorCopy( cent->lerpOrigin, ent.origin);
	   VectorCopy( cent->lerpOrigin, ent.oldorigin);
      }

	ent.hModel = cgs.model_draw[s1->modelindex];

   // Modulation based off the color
   for( i=0; i<3; i++ )
      ent.shaderRGBA[ i ] = cent->color[ i ] * 255;

   // take the alpha from the entity if less than 1, else grab it from the client commands version
   if ( s1->alpha < 1 )
      ent.shaderRGBA[ 3 ] = s1->alpha * 255;
   else
      ent.shaderRGBA[ 3 ] = cent->color[ 3 ] * 255;

   ent.renderfx |= s1->renderfx;

   // convert angles to axis
	AnglesToAxis( cent->lerpAngles, ent.axis );

   // Interpolated state variables
   if ( cent->interpolate )
      {
      ent.scale = s1->scale + cg.frameInterpolation * ( cent->nextState.scale - cent->currentState.scale );
      }
   else
      {
      ent.scale = s1->scale;
      }

	// add to refresh list
	if ( !( cent->currentState.renderfx & RF_DONTDRAW ) )
   	cgi.R_AddRefSpriteToScene( &ent );
   }

/*
==================
CG_Item
==================
*/
void CG_Item( centity_t *cent ) 
   {
	entityState_t	*es;
   int            thandle;
   float          radius, scale;
   float          frac;
   vec3_t         mins, maxs, mid, start, save;
	polyVert_t	   verts[4];

	es = &cent->currentState;

	// if set to invisible, skip
	if ( !es->modelindex || ( es->parent != ENTITYNUM_NONE ) || ( es->renderfx & RF_DONTDRAW ) ) 
      {
		return;
	   }

	// autorotate
   VectorCopy( cg.autoAngles, cent->lerpAngles );

   thandle = cgs.model_tiki[ es->modelindex ]; 
   if ( thandle == -1 )
      return;

   radius = cgi.R_ModelRadius( cgs.model_draw[ es->modelindex ] ) * 0.5f * es->scale;
   cgi.R_ModelBounds( cgs.model_draw[ es->modelindex ], mins, maxs );
   VectorAdd( mins, maxs, mid );
   VectorScale( mid, 0.5f * es->scale, mid );

   // offset the model from the ground
   cent->lerpOrigin[ 2 ] += radius;

   // save off the center of the model for the rings
   VectorCopy( cent->lerpOrigin, save );

	// items bob up and down continuously
	scale = 0.005 + es->number * 0.00001;
	cent->lerpOrigin[2] += ( radius * 0.2f ) + cos( ( cg.time + 1000 ) * scale ) * ( radius * 0.2f );

   // offset the origin to the centroid 
	cent->lerpOrigin[0] -= 
		mid[0] * cg.autoAxis[0][0] +
		mid[1] * cg.autoAxis[1][0] +
		mid[2] * cg.autoAxis[2][0];
	cent->lerpOrigin[1] -= 
		mid[0] * cg.autoAxis[0][1] +
		mid[1] * cg.autoAxis[1][1] +
		mid[2] * cg.autoAxis[2][1];

	cent->lerpOrigin[2] -= 
		mid[0] * cg.autoAxis[0][2] +
		mid[1] * cg.autoAxis[1][2] +
		mid[2] * cg.autoAxis[2][2];

   scale *= 0.5f;

	frac = cos( ( cg.time + 1000 ) * scale ) * radius;
   VectorCopy( save, start );
   start[ 2 ] += frac;
   frac = 1.5f * ( radius - fabs( frac * 0.5f ) );

   // add the item rings
   VectorCopy( start, verts[ 0 ].xyz );
   verts[ 0 ].xyz[ 0 ] -= frac;
   verts[ 0 ].xyz[ 1 ] -= frac;
	verts[ 0 ].st[0] = 0;
	verts[ 0 ].st[1] = 0;
	verts[ 0 ].modulate[0] = 255;
	verts[ 0 ].modulate[1] = 255;
	verts[ 0 ].modulate[2] = 255;
	verts[ 0 ].modulate[3] = 255;

   VectorCopy( start, verts[ 1 ].xyz );
   verts[ 1 ].xyz[ 0 ] -= frac;
   verts[ 1 ].xyz[ 1 ] += frac;
	verts[ 1 ].st[0] = 0;
	verts[ 1 ].st[1] = 1;
	verts[ 1 ].modulate[0] = 255;
	verts[ 1 ].modulate[1] = 255;
	verts[ 1 ].modulate[2] = 255;
	verts[ 1 ].modulate[3] = 255;

   VectorCopy( start, verts[ 2 ].xyz );
   verts[ 2 ].xyz[ 0 ] += frac;
   verts[ 2 ].xyz[ 1 ] += frac;
	verts[ 2 ].st[0] = 1;
	verts[ 2 ].st[1] = 1;
	verts[ 2 ].modulate[0] = 255;
	verts[ 2 ].modulate[1] = 255;
	verts[ 2 ].modulate[2] = 255;
	verts[ 2 ].modulate[3] = 255;

   VectorCopy( start, verts[ 3 ].xyz );
   verts[ 3 ].xyz[ 0 ] += frac;
   verts[ 3 ].xyz[ 1 ] -= frac;
	verts[ 3 ].st[0] = 1;
	verts[ 3 ].st[1] = 0;
	verts[ 3 ].modulate[0] = 255;
	verts[ 3 ].modulate[1] = 255;
	verts[ 3 ].modulate[2] = 255;
	verts[ 3 ].modulate[3] = 255;

   cgi.R_AddPolyToScene( cgs.media.itemRingShader, 4, verts, 0 );

	frac = cos( ( cg.time + 1000 ) * scale + DEG2RAD( 180 ) ) * radius;
   VectorCopy( save, start );
   start[ 2 ] += frac;
   frac = 1.5f * ( radius - fabs( frac * 0.5f ) );

   VectorCopy( start, verts[ 0 ].xyz );
   verts[ 0 ].xyz[ 0 ] -= frac;
   verts[ 0 ].xyz[ 1 ] -= frac;

   VectorCopy( start, verts[ 1 ].xyz );
   verts[ 1 ].xyz[ 0 ] -= frac;
   verts[ 1 ].xyz[ 1 ] += frac;

   VectorCopy( start, verts[ 2 ].xyz );
   verts[ 2 ].xyz[ 0 ] += frac;
   verts[ 2 ].xyz[ 1 ] += frac;

   VectorCopy( start, verts[ 3 ].xyz );
   verts[ 3 ].xyz[ 0 ] += frac;
   verts[ 3 ].xyz[ 1 ] -= frac;

   cgi.R_AddPolyToScene( cgs.media.itemRingShader, 4, verts, 0 );

   }


/*
===============
CG_Beam
===============
*/
void CG_Beam( centity_t *cent ) {
	entityState_t  *s1;
   vec3_t         vz={0,0,0},origin={0,0,0};
   byte           modulate[4];
   int            i;

	s1 = &cent->currentState;

   for ( i=0;i<4;i++ )
      modulate[i] = cent->color[i] * 255;

   if ( s1->torso_anim != ENTITYNUM_NONE )
      {
      refEntity_t *parent;
      parent = cgi.R_GetRenderEntity( s1->torso_anim );

      if ( !parent )
         {
         cgi.DPrintf( "CG_Beam: Could not find parent entity\n" );
         return;
         }

      VectorAdd( s1->origin, parent->origin, origin );
      }
   else
      {
      VectorCopy( s1->origin, origin );
      }

   CG_CreateBeam( origin,           // start
                  vz,               // dir ( auto calculated by using origin2-origin )
                  s1->number,       // owner number
                  cgs.model_draw[s1->modelindex], //hModel
                  s1->alpha,        // alpha
                  s1->scale,        // scale
                  s1->skinNum,      // flags
                  0,                // length ( auto calculated )
                  PKT_TO_BEAM_PARM( s1->surfaces[0] ) * 1000, // life
                  qfalse,           // don't always create the beam, just update it
                  s1->origin2,      // endpoint
                  s1->bone_angles[0][0], // min offset
                  s1->bone_angles[0][1], // max offset
                  PKT_TO_BEAM_PARM( s1->surfaces[3] ), // overlap
                  s1->surfaces[4],   // subdivisions
                  PKT_TO_BEAM_PARM( s1->surfaces[5] ) * 1000, // delay
                  CG_ConfigString( CS_IMAGES + s1->tag_num ), // index for shader configstring
                  modulate,          // modulate color
                  s1->surfaces[6],   // num sphere beams
                  PKT_TO_BEAM_PARM( s1->surfaces[7] ),   // sphere radius
                  PKT_TO_BEAM_PARM( s1->surfaces[8] ),   // toggle delay
                  PKT_TO_BEAM_PARM( s1->surfaces[9] ),   // end alpha
                  s1->renderfx,
                  ""
                  );   
}


void CG_Decal
   (
   centity_t *cent
   )

   {
   qhandle_t      shader;
   vec3_t         dir;
   entityState_t	*s1;

	s1 = &cent->currentState;

   shader = cgi.R_RegisterShader( CG_ConfigString( CS_IMAGES + s1->tag_num ) );
   ByteToDir( s1->surfaces[0], dir );
   CG_ImpactMark( shader,
                  s1->origin,
                  dir,
                  s1->angles[2],
                  cent->color[0],
                  cent->color[1],
                  cent->color[2],
                  cent->color[3],
                  qtrue,
                  s1->scale,
                  qfalse,
                  -1,
                  qfalse);
   }

/*
===============
CG_Portal
===============
*/
void CG_Portal( centity_t *cent )
   {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	AnglesToAxis( cent->currentState.angles, ent.axis );

	// negating this tends to get the directions like they want
	// we really should have a camera roll value
	VectorSubtract( vec3_origin, ent.axis[1], ent.axis[1] );
   VectorSubtract( vec3_origin, ent.axis[2], ent.axis[2] );
	
	ent.reType  = RT_PORTALSURFACE;
	ent.frame   = s1->frame;		// rotation speed
	ent.skinNum = s1->clientNum/256.0 * 360;	// roll offset

	// add to refresh list
	cgi.R_AddRefEntityToScene(&ent);
   }


/*
=========================
CG_AdjustPositionForMover
=========================
*/
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out ) {
	centity_t	*cent;
	vec3_t	oldOrigin, origin, deltaOrigin;
	vec3_t	oldAngles, angles;

	if ( moverNum == ENTITYNUM_NONE ) {
		VectorCopy( in, out );
		return;
	}

	cent = &cg_entities[ moverNum ];

   if ( cent->currentState.pos.trType == TR_LERP ) {
      int i;

      if ( cent->interpolate )
         {
         // we calculate lerpOrigin so we have always have the latest info.  It is possible to call this before lerpOrigin
         // is updated by AddEntities
         for ( i=0; i<3; i++ ) 
            {
            cent->lerpOrigin[i] = cent->currentState.origin[i] + 
               cg.frameInterpolation * ( cent->nextState.origin[i] - cent->currentState.origin[i] );
            }
         }
      for ( i=0; i<3; i++ ) 
         {
         out[ i ] = in[ i ] + ( cent->lerpOrigin[i] - cent->currentState.origin[i] );
         }
      return;

   } else if ( cent->currentState.eType != ET_MOVER ) {
		VectorCopy( in, out );
		return;
	}

	EvaluateTrajectory( &cent->currentState.pos, fromTime, oldOrigin );
	EvaluateTrajectory( &cent->currentState.apos, fromTime, oldAngles );

	EvaluateTrajectory( &cent->currentState.pos, toTime, origin );
	EvaluateTrajectory( &cent->currentState.apos, toTime, angles );

	VectorSubtract( origin, oldOrigin, deltaOrigin );

	VectorAdd( in, deltaOrigin, out );

	// FIXME: origin change when on a rotating object
}


/*
===============
CG_CalcEntityLerpPositions

===============
*/
void CG_CalcEntityLerpPositions( centity_t *cent ) {
   int   i;
   float	f;

   f = cg.frameInterpolation;

   if ( cg.snap->ps.pm_type < PM_DEAD )
      {
	   if ( cent->currentState.number == cg.snap->ps.clientNum ) 
         {
		   // if the player, take position from prediction
		   VectorCopy( cg.predicted_player_state.origin, cent->lerpOrigin );
         for ( i=0; i<3; i++ ) {
            cent->lerpAngles[i] = LerpAngle( cent->currentState.angles[i], cent->nextState.angles[i], f );
         }

         return;
	      }
      }

   if ( cent->currentState.pos.trType == TR_LERP ) {
      float quat[ 4 ];
      float mat[ 3 ][ 3 ];

      if ( !cent->interpolate ) {
         VectorCopy( cent->currentState.angles, cent->lerpAngles );
         VectorCopy( cent->currentState.origin, cent->lerpOrigin );
         return;
      }

		// it would be an internal error to find an entity that interpolates without
		// a snapshot ahead of the current one
		if ( cg.nextSnap == NULL ) {
			cgi.Error( ERR_DROP, "CG_AddCEntity: cg.nextSnap == NULL" );
		}

      for ( i=0; i<3; i++ ) {
         cent->lerpOrigin[i] = cent->currentState.origin[i] + 
            f * ( cent->nextState.origin[i] - cent->currentState.origin[i] );
      }

      if ( !memcmp( cent->currentState.angles, cent->nextState.angles, sizeof( vec3_t ) ) ) {
         VectorCopy( cent->currentState.angles, cent->lerpAngles );
      } else {
         // use spherical interpolation using quaternions so that bound objects
         // rotate properly without gimble lock.
	      SlerpQuaternion( cent->currentState.quat, cent->nextState.quat, f, quat );
	      QuatToMat( quat, mat );
	      MatrixToEulerAngles( mat, cent->lerpAngles );
      }
   } else if ( cent->interpolate && cent->currentState.pos.trType == TR_INTERPOLATE ) {
		// if the entity has a valid next state, interpolate a value between the frames
		// unless it is a mover with a known start and stop
		vec3_t	current, next;

		// it would be an internal error to find an entity that interpolates without
		// a snapshot ahead of the current one
		if ( cg.nextSnap == NULL ) {
			cgi.Error( ERR_DROP, "CG_AddCEntity: cg.nextSnap == NULL" );
		}

		// this will linearize a sine or parabolic curve, but it is important
		// to not extrapolate player positions if more recent data is available
		EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, current );
		EvaluateTrajectory( &cent->nextState.pos, cg.nextSnap->serverTime, next );

		cent->lerpOrigin[0] = current[0] + f * ( next[0] - current[0] );
		cent->lerpOrigin[1] = current[1] + f * ( next[1] - current[1] );
		cent->lerpOrigin[2] = current[2] + f * ( next[2] - current[2] );

		EvaluateTrajectory( &cent->currentState.apos, cg.snap->serverTime, current );
		EvaluateTrajectory( &cent->nextState.apos, cg.nextSnap->serverTime, next );

      // Lerp legs, torso, and head angles
      for ( i=0; i<3; i++ ) {
         cent->lerpAngles[i]       = LerpAngle( current[i], next[i], f );
      }
   } else {
	   // just use the current frame and evaluate as best we can
	   EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
	   EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

	   // adjust for riding a mover
	   CG_AdjustPositionForMover( cent->lerpOrigin, cent->currentState.groundEntityNum, 
		   cg.snap->serverTime, cg.time, cent->lerpOrigin );
   } 
}

/*
===============
CG_AddCEntity

===============
*/
void CG_AddCEntity( centity_t *cent ) 
{
	// event-only entities will have been dealt with already
	if ( cent->currentState.eType >= ET_EVENTS ) {
		return;
	}

	// calculate the current origin
	CG_CalcEntityLerpPositions( cent );

	// add automatic effects
	CG_EntityEffects( cent );

	CG_SetEntitySoundPosition( cent );

	switch ( cent->currentState.eType ) {
	default:
		cgi.Error( ERR_DROP, "Bad entity type: %i\n", cent->currentState.eType );
		break;
	case ET_PLAYER:
		CG_Player( cent );
      // intentional fallthrough
   case ET_MODELANIM:
		CG_Splash( cent );
      CG_ModelAnim( cent );
      break;
   case ET_ITEM:
      CG_Item( cent );
      CG_ModelAnim( cent );
      break;
	case ET_GENERAL:
		CG_General( cent );
		break;
	case ET_MOVER:
		CG_Mover( cent );
		break;
	case ET_BEAM:
		CG_Beam( cent );
		break;
	case ET_ROPE: // skip
      CG_Rope( cent );
		break;
	case ET_MULTIBEAM: // skip
		break;
   case ET_SPRITE:
      CG_Sprite( cent );
      break;
	case ET_PORTAL:
		CG_Portal( cent );
		break;
   case ET_RAIN:
   case ET_EMITTER:
      CG_Emitter( cent );
      break;
   case ET_DECAL:
      CG_Decal( cent );
      break;
	}
   }

/*
===============
CG_AddPacketEntities

===============
*/
void CG_AddPacketEntities( void ) {
	int					num;
	centity_t			*cent;
//	playerState_t		*ps;

	// the auto-rotating items will all have the same axis
	cg.autoAngles[0] = 0;
	cg.autoAngles[1] = ( cg.time & 2047 ) * 360 / 2048.0;
	cg.autoAngles[2] = 0;

	cg.autoAnglesSlow[0] = 0;
	cg.autoAnglesSlow[1] = ( cg.time & 4095 ) * 360 / 4096.0f;
	cg.autoAnglesSlow[2] = 0;

	cg.autoAnglesFast[0] = 0;
	cg.autoAnglesFast[1] = ( cg.time & 1023 ) * 360 / 1024.0f;
	cg.autoAnglesFast[2] = 0;

	AnglesToAxis( cg.autoAngles, cg.autoAxis );
	AnglesToAxis( cg.autoAnglesSlow, cg.autoAxisSlow );
	AnglesToAxis( cg.autoAnglesFast, cg.autoAxisFast );

	// generate and add the entity from the playerstate
	//ps = &cg.predicted_player_state;
	//PlayerStateToEntityState( ps, &cg_entities[ ps->clientNum ].currentState );
	//CG_AddCEntity( &cg_entities[ ps->clientNum ] );

	// add each entity sent over by the server
	for ( num = 0 ; num < cg.snap->numEntities ; num++ ) {
		cent = &cg_entities[ cg.snap->entities[ num ].number ];
		CG_AddCEntity( cent );
	}

   // Add in the multibeams at the end
   for ( num = 0 ; num < cg.snap->numEntities ; num++ ) {
		cent = &cg_entities[ cg.snap->entities[ num ].number ];
      if ( cent->currentState.eType == ET_MULTIBEAM )
         CG_MultiBeam( cent );	
	}

}

void CG_GetOrigin( centity_t *cent, vec3_t origin )
	{
	if ( cent->currentState.parent == ENTITYNUM_NONE )
		{
		VectorCopy( cent->lerpOrigin, origin );
		}
	else
		{
		int i;
		orientation_t or;
      refEntity_t *parent;
      int tikihandle;

      parent = cgi.R_GetRenderEntity( cent->currentState.parent );

      if ( !parent )
         {
         cgi.DPrintf( "CG_GetOrigin: Could not find parent entity\n" );
         return;
         }

      tikihandle = cgi.TIKI_GetHandle( parent->hModel );

		// lerp the tag
		if ( r_lerpmodels->integer )
			{
			or = cgi.Tag_LerpedOrientation( tikihandle, parent, cent->currentState.tag_num );
			}
		else
			{
         //FIXME
         // doesn't handle torso animations
			or = cgi.Tag_Orientation( tikihandle, 	parent->anim, parent->frame, cent->currentState.tag_num, parent->scale, 
					parent->bone_tag, parent->bone_quat );
			}

		VectorCopy( parent->origin, origin );

		for ( i = 0 ; i < 3 ; i++ ) 
			{
			VectorMA( origin, or.origin[i], parent->axis[i], origin );
			}
		}
	}

