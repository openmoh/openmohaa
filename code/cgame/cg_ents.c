/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
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

Also called by event processing code
======================
*/
void CG_SetEntitySoundPosition( centity_t *cent ) {
	if ( cent->currentState.solid == SOLID_BMODEL ) {
		vec3_t	origin;
		float	*v;

		v = cgs.inlineModelMidpoints[ cent->currentState.modelindex ];
		VectorAdd( cent->lerpOrigin, v, origin );
		cgi.S_UpdateEntityPosition( cent->currentState.number, origin );
	} else {
		cgi.S_UpdateEntityPosition( cent->currentState.number, cent->lerpOrigin );
	}
}

/*
==================
CG_EntityEffects

Add continuous entity effects, like local entity emission and lighting
==================
*/
static void CG_EntityEffects( centity_t *cent ) {

	// update sound origins
	CG_SetEntitySoundPosition( cent );

	// add loop sound
	if ( cent->currentState.loopSound ) {
		if (cent->currentState.eType != ET_SPEAKER) {
			cgi.S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, 
				cgs.gameSounds[ cent->currentState.loopSound ] );
		} else {
			cgi.S_AddRealLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, 
				cgs.gameSounds[ cent->currentState.loopSound ] );
		}
	}


	// constant light glow
	if ( cent->currentState.constantLight != -1 ) {
		int		cl;
		int		i, r, g, b;

		cl = cent->currentState.constantLight;
		r = cl & 255;
		g = ( cl >> 8 ) & 255;
		b = ( cl >> 16 ) & 255;
		i = ( ( cl >> 24 ) & 255 ) * CONSTANTLIGHT_RADIUS_SCALE;
		cgi.R_AddLightToScene( cent->lerpOrigin, i, r, g, b );
	}
}


/*
==================
CG_General
==================
*/
static void CG_General( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// if set to invisible, skip
	if (!s1->modelindex) {
		return;
	}

	memset (&ent, 0, sizeof(ent));

	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	ent.model = cgs.gameModels[ s1->modelindex ];

	// player model
	if (s1->number == cg.snap->ps.clientNum) {
		ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
	}

	// convert angles to axis
	AnglesToAxis( cent->lerpAngles, ent.axis );

	// add to refresh list
	cgi.R_AddRefEntityToScene (&ent);
}

//============================================================================

/*
===============
CG_Mover
===============
*/
static void CG_Mover( centity_t *cent ) {
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
		ent.model = cgs.inlineDrawModel[ s1->modelindex ];
	} else {
		ent.model = cgs.gameModels[ s1->modelindex ];
	}

	// add to refresh list
	cgi.R_AddRefEntityToScene(&ent);
}

/*
===============
CG_Beam
===============
*/
#define DECODE_BEAM_PARM( x ) ( ((double)x)*0.0625 )
void CG_Beam( centity_t *cent ) {
#if 0
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( s1->origin, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	AxisClear( ent.axis );
	ent.reType = RT_BEAM;

	ent.renderfx = RF_NOSHADOW;

	// add to refresh list
	cgi.R_AddRefEntityToScene(&ent);
#else
	entityState_t  *s1;
	vec3_t         vz={0,0,0},origin={0,0,0};
	byte           modulate[4];
	int            i;

	s1 = &cent->currentState;

	for ( i=0;i<4;i++ )
		modulate[i] = cent->color[i] * 255;

#if 0
	memset(&modulate,0xff,4);
#endif

	if ( s1->beam_entnum != ENTITYNUM_NONE ) {
		centity_t *parent;
		parent = &cg_entities[s1->beam_entnum];
		VectorAdd( s1->origin, parent->lerpOrigin, origin );
	} else {
		VectorCopy( s1->origin, origin );
	}

	CG_CreateBeam( origin,           // start
                  vz,               // dir ( auto calculated by using origin2-origin )
                  s1->number,       // owner number
                  cgs.gameModels[s1->modelindex], //hModel
                  s1->alpha,        // alpha
                  s1->scale,        // scale
                  s1->skinNum,      // flags
                  0,                // length ( auto calculated )
                  DECODE_BEAM_PARM( s1->surfaces[0] ) * 1000, // life
                  qfalse,           // don't always create the beam, just update it
                  s1->origin2,      // endpoint
                  s1->bone_angles[0][0], // min offset
                  s1->bone_angles[0][1], // max offset
                  DECODE_BEAM_PARM( s1->surfaces[3] ), // overlap
                  s1->surfaces[4],   // subdivisions
                  DECODE_BEAM_PARM( s1->surfaces[5] ) * 1000, // delay
                  CG_ConfigString( CS_IMAGES + s1->tag_num ), // index for shader configstring
                  modulate,          // modulate color
                  s1->surfaces[6],   // num sphere beams
                  DECODE_BEAM_PARM( s1->surfaces[7] ),   // sphere radius
                  DECODE_BEAM_PARM( s1->surfaces[8] ),   // toggle delay
                  DECODE_BEAM_PARM( s1->surfaces[9] ),   // end alpha
                  s1->renderfx,
                  ""
                  ); 
#endif
}


/*
===============
CG_Portal
===============
*/
static void CG_Portal( centity_t *cent ) {
	//refEntity_t			ent;
	//entityState_t		*s1;

	//s1 = &cent->currentState;

	//// create the render entity
	//memset (&ent, 0, sizeof(ent));
	//VectorCopy( cent->lerpOrigin, ent.origin );
	//VectorCopy( s1->origin2, ent.oldorigin );
	//ByteToDir( s1->eventParm, ent.axis[0] );
	//PerpendicularVector( ent.axis[1], ent.axis[0] );

	//// negating this tends to get the directions like they want
	//// we really should have a camera roll value
	//VectorSubtract( vec3_origin, ent.axis[1], ent.axis[1] );

	//CrossProduct( ent.axis[0], ent.axis[1], ent.axis[2] );
	//ent.reType = RT_PORTALSURFACE;
	//ent.oldframe = s1->powerups;
	//ent.frame = s1->frame;		// rotation speed
	//ent.skinNum = s1->clientNum/256.0 * 360;	// roll offset

	//// add to refresh list
	//cgi.R_AddRefEntityToScene(&ent);
}


/*
=========================
CG_AdjustPositionForMover

Also called by client movement prediction code
=========================
*/
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out ) {
	//centity_t	*cent;
	//vec3_t	oldOrigin, origin, deltaOrigin;
	//vec3_t	oldAngles, angles, deltaAngles;

	//if ( moverNum <= 0 || moverNum >= ENTITYNUM_MAX_NORMAL ) {
	//	VectorCopy( in, out );
	//	return;
	//}

	//cent = &cg_entities[ moverNum ];
	//if ( cent->currentState.eType != ET_MOVER ) {
	//	VectorCopy( in, out );
	//	return;
	//}

	//BG_EvaluateTrajectory( &cent->currentState.pos, fromTime, oldOrigin );
	//BG_EvaluateTrajectory( &cent->currentState.apos, fromTime, oldAngles );

	//BG_EvaluateTrajectory( &cent->currentState.pos, toTime, origin );
	//BG_EvaluateTrajectory( &cent->currentState.apos, toTime, angles );

	//VectorSubtract( origin, oldOrigin, deltaOrigin );
	//VectorSubtract( angles, oldAngles, deltaAngles );

	//VectorAdd( in, deltaOrigin, out );

	//// FIXME: origin change when on a rotating object
}


/*
=============================
CG_InterpolateEntityPosition
=============================
*/
static void CG_InterpolateEntityPosition( centity_t *cent ) {
	float		*current, *next;
	float		f;

	// it would be an internal error to find an entity that interpolates without
	// a snapshot ahead of the current one
	if ( cg.nextSnap == NULL ) {
		CG_Error( "CG_InterpoateEntityPosition: cg.nextSnap == NULL" );
	}

	f = cg.frameInterpolation;

	// this will linearize a sine or parabolic curve, but it is important
	// to not extrapolate player positions if more recent data is available
	// su44: adjusted for MOH
	current = &cent->currentState.origin[0];
	next = &cent->nextState.origin[0];

	cent->lerpOrigin[0] = current[0] + f * ( next[0] - current[0] );
	cent->lerpOrigin[1] = current[1] + f * ( next[1] - current[1] );
	cent->lerpOrigin[2] = current[2] + f * ( next[2] - current[2] );

	current = &cent->currentState.angles[0];
	next = &cent->nextState.angles[0];

	cent->lerpAngles[0] = LerpAngle( current[0], next[0], f );
	cent->lerpAngles[1] = LerpAngle( current[1], next[1], f );
	cent->lerpAngles[2] = LerpAngle( current[2], next[2], f );

}

/*
===============
CG_CalcEntityLerpPositions

===============
*/
static void CG_CalcEntityLerpPositions( centity_t *cent ) {
#if 0
	// if this player does not want to see extrapolated players
	if ( !cg_smoothClients.integer ) {
		// make sure the clients use TR_INTERPOLATE
		if ( cent->currentState.number < MAX_CLIENTS ) {
			cent->currentState.pos.trType = TR_INTERPOLATE;
			cent->nextState.pos.trType = TR_INTERPOLATE;
		}
	}

	if ( cent->interpolate && cent->currentState.pos.trType == TR_INTERPOLATE ) {
		CG_InterpolateEntityPosition( cent );
		return;
	}

	// first see if we can interpolate between two snaps for
	// linear extrapolated clients
	if ( cent->interpolate && cent->currentState.pos.trType == TR_LINEAR_STOP &&
											cent->currentState.number < MAX_CLIENTS) {
		CG_InterpolateEntityPosition( cent );
		return;
	}

	// just use the current frame and evaluate as best we can
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );
#else
	if(cg.nextSnap) {
		CG_InterpolateEntityPosition( cent );
	} else {
		VectorCopy(cent->currentState.origin,cent->lerpOrigin );
		VectorCopy(cent->currentState.angles,cent->lerpAngles );
	}
 
#endif
	// adjust for riding a mover if it wasn't rolled into the predicted
	// player state
	//if ( cent != &cg.predictedPlayerEntity ) {
	//	CG_AdjustPositionForMover( cent->lerpOrigin, cent->currentState.groundEntityNum, 
	//	cg.snap->serverTime, cg.time, cent->lerpOrigin );
	//}
}

// su44: prints all unknown eFlags
// NOTE: it seems that flag 4 has something to do with player weapons,
// try holstering/unholstering player weapon and watch changes
// NOTE2: flags 64 is present on vehicles/opeltruck.tik,
// vehicles/tigertank.tik (both body model and its cannon),
// vehicles/panzer_tank.tik, vehicles/tigertank.tik
// All of those modes have animated tank-wheels shader...
static void CG_ShowEntUnknownEFlags(centity_t *cent) {
	int f = cent->currentState.eFlags;
	f &= ~EF_ALLIES;
	f &= ~EF_AXIS;
	f &= ~EF_DEAD;
	f &= ~EF_UNARMED;

	if(f) {
		CG_Printf("Timenow %i, ent %i ef %i\n",cg.time, cent->currentState.number,f);
	}
}
// renderFX 16 - thompsonsmg.tik, bar.tik, etc, tigertank.tik
// su44: From gamex86.dll rendereffects event
// invisible - 33554432 - 
// preciseshadow - 16777216
// shadow - 2048 - 2^11 - matches FAKK
// lightstyledynamiclight - 8388608
// additivedynamiclight -  ??????????????
// fullbright - no longer supported
// skyorigin - 8192
// lightoffset - no longer supported
// viewlensflare - 8
// lensflare - 256
// dontdraw - 128

static void CG_ShowEntUnknownRenderFX(centity_t *cent) {
	int f = cent->currentState.renderfx;
	if( f & RF_DEPTHHACK ) {
		f &= ~RF_DEPTHHACK;	
	}
	if( f & RF_SHADOW ) {
		f &= ~RF_SHADOW;	
	}
	if( f & RF_FULLBRIGHT ) {
		f &= ~RF_FULLBRIGHT;	
	}
	if( f & RF_VIEWLENSFLARE ) {
		f &= ~RF_VIEWLENSFLARE;	
	}
	if( f & RF_PRECISESHADOW ) {
		f &= ~RF_PRECISESHADOW;	
	}
	if( f & RF_LIGHTSTYLE_DLIGHT ) {
		f &= ~RF_LIGHTSTYLE_DLIGHT;
	}
	if(f) {
		CG_Printf("Timenow %i, ent %i renderFX %i\n",cg.time, cent->currentState.number,f);
	}
}
/*
===============
CG_AddCEntity

===============
*/
static void CG_AddCEntity( centity_t *cent ) {
	// event-only entities will have been dealt with already
	if ( cent->currentState.eType >= ET_EVENTS ) {
		return;
	}

	// calculate the current origin
	CG_CalcEntityLerpPositions( cent );

	switch ( cent->currentState.eType ) {
	case 0: break; // 0 spams console with bad entity type
	default:
		//su44: changed to printf, so I can debug our gamex code without getting Com_Error
		//CG_Error( "Bad entity type: %i\n", cent->currentState.eType );
		CG_Printf( "Bad entity type: %i\n", cent->currentState.eType ); 
		break;
case ET_MOVER: // su44: ET_MOVER is used only if running a local openmohaa server.
	// this needs to be fixed soon in our game code
	case ET_GENERAL:
		CG_ModelAnim( cent );
		//CG_General( cent );
		break;
	case ET_PLAYER:
	case ET_VEHICLE:
	case ET_ITEM:
	case ET_MODELANIM:
		CG_ModelAnim( cent );
		//CG_ShowEntUnknownEFlags(cent);
		//CG_ShowEntUnknownRenderFX(cent);

		if( cent->currentState.number < cgs.maxclients ) {

			// su44: save player team to clientInfo_t
			CG_ExtractPlayerTeam( cent );
			// su44: draw axis/allies icon over player head.
			// Function below must be called after CG_ModelAnim,
			// because it might need to use centity_t::bones.
			CG_PlayerSprites( cent ); 
		}
		break;
	case ET_RAIN: // su44: ET_RAIN is used on dm/mohdm5 for snow effect
		CG_Rain( cent );
		break;
	// su44: beams are used on some servers with custom scripts
	case ET_BEAM:
		CG_Beam( cent );
		break;
	case ET_ROPE:
		CG_Rope( cent );
		break;
	case ET_MULTIBEAM:
		CG_MultiBeam( cent );
		break;
	}

	// add automatic effects
	CG_EntityEffects( cent );
}

/*
===============
CG_AddPacketEntities

===============
*/
void CG_AddPacketEntities( void ) {
	int					num;
	centity_t			*cent;
	playerState_t		*ps;

	// set cg.frameInterpolation
	if ( cg.nextSnap ) {
		int		delta;

		delta = (cg.nextSnap->serverTime - cg.snap->serverTime);
		if ( delta == 0 ) {
			cg.frameInterpolation = 0;
		} else {
			cg.frameInterpolation = (float)( cg.time - cg.snap->serverTime ) / delta;
		}
	} else {
		cg.frameInterpolation = 0;	// actually, it should never be used, because 
									// no entities should be marked as interpolating
	}

	// the auto-rotating items will all have the same axis
	cg.autoAngles[0] = 0;
	cg.autoAngles[1] = ( cg.time & 2047 ) * 360 / 2048.0;
	cg.autoAngles[2] = 0;

	cg.autoAnglesFast[0] = 0;
	cg.autoAnglesFast[1] = ( cg.time & 1023 ) * 360 / 1024.0f;
	cg.autoAnglesFast[2] = 0;

	AnglesToAxis( cg.autoAngles, cg.autoAxis );
	AnglesToAxis( cg.autoAnglesFast, cg.autoAxisFast );

#if 1 // su44: that's not done here in MoHAA (?)
	// generate and add the entity from the playerstate
	ps = &cg.predictedPlayerState;
	BG_PlayerStateToEntityState( ps, &cg.predictedPlayerEntity.currentState, qfalse );
	CG_AddCEntity( &cg.predictedPlayerEntity );
#else
	ps = &cg.predictedPlayerState;
	BG_PlayerStateToEntityState( ps, &cg.predictedPlayerEntity.currentState, qfalse );
	if(cg_entities[ ps->clientNum ].currentValid==qtrue) {
		cg_entities[ ps->clientNum ].currentState.groundEntityNum = ps->groundEntityNum;
	} else {
		//CG_AddCEntity( &cg.predictedPlayerEntity );
	}
#endif
	// lerp the non-predicted value for lightning gun origins
	CG_CalcEntityLerpPositions( &cg_entities[ cg.snap->ps.clientNum ] );

	// add each entity sent over by the server
	for ( num = 0 ; num < cg.snap->numEntities ; num++ )
	{
		cent = &cg_entities[ cg.snap->entities[ num ].number ];
		CG_AddCEntity( cent );
	}
}

