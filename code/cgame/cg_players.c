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
// cg_players.c -- handle the media and animation for player entities
#include "cg_local.h"

/*
=============================================================================

CLIENT INFO

=============================================================================
*/

/*
======================
CG_NewClientInfo
======================
*/
// su44: it seems MoHAA's CS_PLAYERS
// contains only player names.
void CG_NewClientInfo( int clientNum ) {
	clientInfo_t *ci;
	clientInfo_t newInfo;
	const char	*configstring;
	const char	*v;

	ci = &cgs.clientinfo[clientNum];

	configstring = CG_ConfigString( clientNum + CS_PLAYERS );
	if ( !configstring[0] ) {
		memset( ci, 0, sizeof( *ci ) );
		return;		// player just left
	}

	// build into a temp buffer so the defer checks can use
	// the old value
	memset( &newInfo, 0, sizeof( newInfo ) );

	// isolate the player's name
	v = Info_ValueForKey(configstring, "name");
	Q_strncpyz( newInfo.name, v, sizeof( newInfo.name ) );

	// replace whatever was there with the new one
	*ci = newInfo;
}

//==========================================================================

/*
===============
CG_PlayerFloatSprite

Float a sprite over the player's head
===============
*/
static void CG_PlayerFloatSprite( centity_t *cent, qhandle_t shader ) {
	int				rf;
	refEntity_t		ent;
	int				b;

	if ( cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson ) {
		rf = RF_THIRD_PERSON;		// only show in mirrors
	} else {
		rf = 0;
	}

	memset( &ent, 0, sizeof( ent ) );
	// su44: try to get "eyes bone" bone position and use it to position the sprite
	if(cent->bones && cgs.gameTIKIs[cent->currentState.modelindex] &&
		(b = cgi.TIKI_GetBoneIndex(cgs.gameTIKIs[cent->currentState.modelindex],"eyes bone")) != -1) {
		vec3_t dummy;
		CG_BoneLocal2World(&cent->bones[b],cent->lerpOrigin,cent->lerpAngles,ent.origin,dummy);
		ent.origin[2] += 24;
	} else {
		VectorCopy( cent->lerpOrigin, ent.origin );
		ent.origin[2] += 98;
	}
	ent.reType = RT_SPRITE;
	ent.customShader = shader;
	ent.radius = 8;
	ent.renderfx = rf;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 255;
	cgi.R_AddRefEntityToScene( &ent );
}

/*
===============
CG_ExtractPlayerTeam

su44: extract player team from entityState_t::eFlags
and save it to cg.clientinfo
===============
*/
void CG_ExtractPlayerTeam( centity_t *cent ) {
	clientInfo_t *ci;
	if( cent->currentState.number >= cgs.maxclients ) {
		CG_Error("CG_ExtractPlayerTeam on non-client entity (%i)", cent->currentState.number );
	}

	ci = &cgs.clientinfo[cent->currentState.number];

	// MoHAA sends ET_PLAYER team in entityFlags
	if(cent->currentState.eFlags & EF_AXIS) {
		ci->team = TEAM_AXIS;
	} else if(cent->currentState.eFlags & EF_ALLIES) {
		ci->team = TEAM_ALLIES;
	//} else if(cent->currentState.eFlags & EF_SPECTATOR) {
	//	ci->team = TEAM_SPECTATOR;
	} else {
		ci->team = TEAM_FREEFORALL;
	}
}

/*
===============
CG_PlayerSprites

Float sprites over the player's head
===============
*/
void CG_PlayerSprites( centity_t *cent ) {
	//int		team;

	//if ( cent->currentState.eFlags & EF_CONNECTION ) {
	//	CG_PlayerFloatSprite( cent, cgs.media.connectionShader );
	//	return;
	//}

	//if ( cent->currentState.eFlags & EF_TALK ) {
	//	CG_PlayerFloatSprite( cent, cgs.media.balloonShader );
	//	return;
	//}

	//team = cgs.clientinfo[ cent->currentState.clientNum ].team;
	//if ( !(cent->currentState.eFlags & EF_DEAD) &&
	//	cg.snap->ps.stats[STAT_TEAM] == team &&
	//	cgs.gametype >= GT_TEAM) {
	//	if (cg_drawFriend->integer) {
	//		//CG_PlayerFloatSprite( cent, cgs.media.friendShader );
	//	}
	//	return;
	//}

	// show team icons over friend heads
	if(cgs.gametype >= GT_TEAM && !(cent->currentState.eFlags & EF_DEAD)) {
		if(cent->currentState.eFlags & EF_AXIS && cg.snap->ps.stats[STAT_TEAM] == TEAM_AXIS) {
			CG_PlayerFloatSprite(cent,cgi.R_RegisterShader("textures/hud/axis.tga"));
		} else if(cent->currentState.eFlags & EF_ALLIES && cg.snap->ps.stats[STAT_TEAM] == TEAM_ALLIES) {
			CG_PlayerFloatSprite(cent,cgi.R_RegisterShader("textures/hud/allies.tga"));
		}
	}
}

/*
===============
CG_PlayerShadow

Returns the Z component of the surface being shadowed

  should it return a full plane instead of a Z?
===============
*/
#define	SHADOW_DISTANCE		128
static qboolean CG_PlayerShadow( centity_t *cent, float *shadowPlane ) {
	vec3_t		end, mins = {-15, -15, 0}, maxs = {15, 15, 2};
	trace_t		trace;
	float		alpha;

	*shadowPlane = 0;

	if ( cg_shadows->integer == 0 ) {
		return qfalse;
	}

	// send a trace down from the player to the ground
	VectorCopy( cent->lerpOrigin, end );
	end[2] -= SHADOW_DISTANCE;

	cgi.CM_BoxTrace( &trace, cent->lerpOrigin, end, mins, maxs, 0, MASK_PLAYERSOLID, qfalse );

	// no shadow if too high
	if ( trace.fraction == 1.0 || trace.startsolid || trace.allsolid ) {
		return qfalse;
	}

	*shadowPlane = trace.endpos[2] + 1;

	if ( cg_shadows->integer != 1 ) {	// no mark for stencil or projection shadows
		return qtrue;
	}

	// fade the shadow out with height
	alpha = 1.0 - trace.fraction;

	// hack / FPE - bogus planes?
	//assert( DotProduct( trace.plane.normal, trace.plane.normal ) != 0.0f )

	// add the mark as a temporary, so it goes directly to the renderer
	// without taking a spot in the cg_marks array
	CG_ImpactMark( cgs.media.shadowMarkShader, trace.endpos, trace.plane.normal,
		/*cent->pe.legs.yawAngle*/ 0 , alpha,alpha,alpha,1, qfalse, 24, qtrue );

	return qtrue;
}


/*
===============
CG_PlayerSplash

Draw a mark at the water surface
===============
*/
static void CG_PlayerSplash( centity_t *cent ) {
	//vec3_t		start, end;
	//trace_t		trace;
	//int			contents;
	//polyVert_t	verts[4];

	//if ( !cg_shadows->integer ) {
	//	return;
	//}

	//VectorCopy( cent->lerpOrigin, end );
	//end[2] -= 24;

	//// if the feet aren't in liquid, don't make a mark
	//// this won't handle moving water brushes, but they wouldn't draw right anyway...
	//contents = cgi.CM_PointContents( end, 0 );
	//if ( !( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) ) {
	//	return;
	//}

	//VectorCopy( cent->lerpOrigin, start );
	//start[2] += 32;

	//// if the head isn't out of liquid, don't make a mark
	//contents = cgi.CM_PointContents( start, 0 );
	//if ( contents & ( CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
	//	return;
	//}

	//// trace down to find the surface
	//cgi.CM_BoxTrace( &trace, start, end, NULL, NULL, 0, ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) );

	//if ( trace.fraction == 1.0 ) {
	//	return;
	//}

	//// create a mark polygon
	//VectorCopy( trace.endpos, verts[0].xyz );
	//verts[0].xyz[0] -= 32;
	//verts[0].xyz[1] -= 32;
	//verts[0].st[0] = 0;
	//verts[0].st[1] = 0;
	//verts[0].modulate[0] = 255;
	//verts[0].modulate[1] = 255;
	//verts[0].modulate[2] = 255;
	//verts[0].modulate[3] = 255;

	//VectorCopy( trace.endpos, verts[1].xyz );
	//verts[1].xyz[0] -= 32;
	//verts[1].xyz[1] += 32;
	//verts[1].st[0] = 0;
	//verts[1].st[1] = 1;
	//verts[1].modulate[0] = 255;
	//verts[1].modulate[1] = 255;
	//verts[1].modulate[2] = 255;
	//verts[1].modulate[3] = 255;

	//VectorCopy( trace.endpos, verts[2].xyz );
	//verts[2].xyz[0] += 32;
	//verts[2].xyz[1] += 32;
	//verts[2].st[0] = 1;
	//verts[2].st[1] = 1;
	//verts[2].modulate[0] = 255;
	//verts[2].modulate[1] = 255;
	//verts[2].modulate[2] = 255;
	//verts[2].modulate[3] = 255;

	//VectorCopy( trace.endpos, verts[3].xyz );
	//verts[3].xyz[0] += 32;
	//verts[3].xyz[1] -= 32;
	//verts[3].st[0] = 1;
	//verts[3].st[1] = 0;
	//verts[3].modulate[0] = 255;
	//verts[3].modulate[1] = 255;
	//verts[3].modulate[2] = 255;
	//verts[3].modulate[3] = 255;

	//cgi.R_AddPolyToScene( cgs.media.wakeMarkShader, 4, verts );
}

/*
=================
CG_LightVerts
=================
*/
int CG_LightVerts( vec3_t normal, int numVerts, polyVert_t *verts )
{
#if 0
	int				i, j;
	float			incoming;
	vec3_t			ambientLight;
	vec3_t			lightDir;
	vec3_t			directedLight;

	for (i = 0; i < numVerts; i++) {
		incoming = DotProduct (normal, lightDir);
		if ( incoming <= 0 ) {
			verts[i].modulate[0] = ambientLight[0];
			verts[i].modulate[1] = ambientLight[1];
			verts[i].modulate[2] = ambientLight[2];
			verts[i].modulate[3] = 255;
			continue;
		}
		j = ( ambientLight[0] + incoming * directedLight[0] );
		if ( j > 255 ) {
			j = 255;
		}
		verts[i].modulate[0] = j;

		j = ( ambientLight[1] + incoming * directedLight[1] );
		if ( j > 255 ) {
			j = 255;
		}
		verts[i].modulate[1] = j;

		j = ( ambientLight[2] + incoming * directedLight[2] );
		if ( j > 255 ) {
			j = 255;
		}
		verts[i].modulate[2] = j;

		verts[i].modulate[3] = 255;
	}
#endif
	return qtrue;
}

/*
===============
CG_Player
===============
*/
void CG_Player( centity_t *cent ) {

}


//=====================================================================

/*
===============
CG_ResetPlayerEntity

A player just came into view or teleported, so reset all animation info
===============
*/
void CG_ResetPlayerEntity( centity_t *cent ) {
	cent->errorTime = -99999;		// guarantee no error decay added
	cent->extrapolated = qfalse;

	//BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
	//BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

	VectorCopy( cent->currentState.origin, cent->lerpOrigin );
	VectorCopy( cent->currentState.angles, cent->lerpAngles );

	VectorCopy( cent->lerpOrigin, cent->rawOrigin );
	VectorCopy( cent->lerpAngles, cent->rawAngles );
}

