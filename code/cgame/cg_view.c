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
// cg_view.c -- setup all the parameters (position, angle, etc)
// for a 3D rendering
#include "cg_local.h"

/*
=============================================================================

  MODEL TESTING

The viewthing and gun positioning tools from Q2 have been integrated and
enhanced into a single model testing facility.

Model viewing can begin with either "testmodel <modelname>" or "testgun <modelname>".

The names must be the full pathname after the basedir, like 
"models/weapons/v_launch/tris.md3" or "players/male/tris.md3"

Testmodel will create a fake entity 100 units in front of the current view
position, directly facing the viewer.  It will remain immobile, so you can
move around it to view it from different angles.

Testgun will cause the model to follow the player around and supress the real
view weapon model.  The default frame 0 of most guns is completely off screen,
so you will probably have to cycle a couple frames to see it.

"nextframe", "prevframe", "nextskin", and "prevskin" commands will change the
frame or skin of the testmodel.  These are bound to F5, F6, F7, and F8 in
q3default.cfg.

If a gun is being tested, the "gun_x", "gun_y", and "gun_z" variables will let
you adjust the positioning.

Note that none of the model testing features update while the game is paused, so
it may be convenient to test with deathmatch set to 1 so that bringing down the
console doesn't pause the game.

=============================================================================
*/

/*
=================
CG_TestModel_f

Creates an entity in front of the current position, which
can then be moved around
=================
*/
void CG_TestModel_f (void) {
	vec3_t		angles;
	char		tmp[128];

	memset( &cg.testModelEntity, 0, sizeof(cg.testModelEntity) );
	if ( cgi.Argc() < 2 ) {
		return;
	}

	Q_strncpyz (cg.testModelName, CG_Argv( 1 ), MAX_QPATH );
	cg.testModelEntity.model = cgi.R_RegisterModel( cg.testModelName );

	if ( cgi.Argc() == 3 ) {
		cg.testModelEntity.backlerp = atof( CG_Argv( 2 ) );
		cg.testModelEntity.frame = 1;
		cg.testModelEntity.oldframe = 0;
	}
	if( !cg.testModelEntity.model ) {
		strcpy(tmp,"models/");
		strcat(tmp,cg.testModelName);
		strcpy(cg.testModelName,tmp);
		cg.testModelEntity.model = cgi.R_RegisterModel( cg.testModelName );
		if( !cg.testModelEntity.model ) {
			CG_Printf( "Can't register model\n" );
			return;
		}
	}

	VectorMA( cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], cg.testModelEntity.origin );

	angles[PITCH] = 0;
	angles[YAW] = 180 + cg.refdefViewAngles[1];
	angles[ROLL] = 0;

	AnglesToAxis( angles, cg.testModelEntity.axis );
	cg.testGun = qfalse;
}

void CG_TestModelAnim_f (void) {
	Q_strncpyz (cg.testModelAnim, CG_Argv( 1 ), MAX_QPATH );
}

void CG_TestModelAnimSpeedScale_f(void) {
	cg.testModelAnimSpeedScale = atof(CG_Argv(1));
}

/*
=================
CG_TestGun_f

Replaces the current view weapon with the given model
=================
*/
void CG_TestGun_f (void) {
	CG_TestModel_f();
	cg.testGun = qtrue;
	cg.testModelEntity.renderfx = RF_MINLIGHT | RF_DEPTHHACK | RF_FIRST_PERSON;
}


void CG_TestModelNextFrame_f (void) {
	cg.testModelEntity.frame++;
	CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelPrevFrame_f (void) {
	cg.testModelEntity.frame--;
	if ( cg.testModelEntity.frame < 0 ) {
		cg.testModelEntity.frame = 0;
	}
	CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelNextSkin_f (void) {
	cg.testModelEntity.skinNum++;
	CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

void CG_TestModelPrevSkin_f (void) {
	cg.testModelEntity.skinNum--;
	if ( cg.testModelEntity.skinNum < 0 ) {
		cg.testModelEntity.skinNum = 0;
	}
	CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

static void CG_AddTestModel (void) {
	int		i;
	tiki_t	*tiki;
	// re-register the model, because the level may have changed
	cg.testModelEntity.model = cgi.R_RegisterModel( cg.testModelName );
	tiki = cgi.TIKI_RegisterModel( cg.testModelName );
	if( !cg.testModelEntity.model ) {
		CG_Printf ("Can't register model\n");
		return;
	}

	// if testing a gun, set the origin reletive to the view origin
	if ( cg.testGun ) {
		VectorCopy( cg.refdef.vieworg, cg.testModelEntity.origin );
		VectorCopy( cg.refdef.viewaxis[0], cg.testModelEntity.axis[0] );
		VectorCopy( cg.refdef.viewaxis[1], cg.testModelEntity.axis[1] );
		VectorCopy( cg.refdef.viewaxis[2], cg.testModelEntity.axis[2] );

		// allow the position to be adjusted
		for (i=0 ; i<3 ; i++) {
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[0][i] * cg_gun_x->value;
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[1][i] * cg_gun_y->value;
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[2][i] * cg_gun_z->value;
		}
	}
	if( tiki ) {
		int animIndex;

		cg.testModelEntity.bones = cgi.TIKI_GetBones(tiki->numBones);
		ClearBounds(cg.testModelEntity.bounds[0],cg.testModelEntity.bounds[1]);
		cg.testModelEntity.radius = 0;

		animIndex = cgi.TIKI_GetAnimIndex(tiki,cg.testModelAnim);
		if(cg.testModelAnim[0]) {
			float t = cg.time / 1000.f;
			if(cg.testModelAnimSpeedScale != 0.f) {
				t *= cg.testModelAnimSpeedScale;
			}
			cgi.TIKI_AppendFrameBoundsAndRadius(tiki,animIndex,t,&cg.testModelEntity.radius,cg.testModelEntity.bounds);
			cgi.TIKI_SetChannels(tiki,animIndex,t,1,cg.testModelEntity.bones);
		} else {
			cgi.TIKI_AppendFrameBoundsAndRadius(tiki,0,0,&cg.testModelEntity.radius,cg.testModelEntity.bounds);
			cgi.TIKI_SetChannels(tiki,0,0,1,cg.testModelEntity.bones);
		}
		cgi.TIKI_Animate(tiki,cg.testModelEntity.bones);
	}
	cgi.R_AddRefEntityToScene( &cg.testModelEntity );
}



//============================================================================


/*
=================
CG_CalcVrect

Sets the coordinates of the rendered window
=================
*/
static void CG_CalcVrect (void) {
	int		size;

	// bound normal viewsize
	if (cg_viewsize->integer < 30) {
		cgi.Cvar_Set ("cg_viewsize","30");
		size = 30;
	} else if (cg_viewsize->integer > 100) {
		cgi.Cvar_Set ("cg_viewsize","100");
		size = 100;
	} else {
		size = cg_viewsize->integer;
	}

	cg.refdef.width = cgs.glconfig.vidWidth*size/100;
	cg.refdef.width &= ~1;

	cg.refdef.height = cgs.glconfig.vidHeight*size/100;
	cg.refdef.height &= ~1;

	cg.refdef.x = (cgs.glconfig.vidWidth - cg.refdef.width)/2;
	cg.refdef.y = (cgs.glconfig.vidHeight - cg.refdef.height)/2;
}

//==============================================================================


/*
===============
CG_OffsetThirdPersonView

===============
*/
#define	FOCUS_DISTANCE	512
static void CG_OffsetThirdPersonView( void ) {
	vec3_t		forward, right, up;
	vec3_t		view;
	vec3_t		focusAngles;
	trace_t		trace;
	static vec3_t	mins = { -4, -4, -4 };
	static vec3_t	maxs = { 4, 4, 4 };
	vec3_t		focusPoint;
	float		focusDist;
	float		forwardScale, sideScale;

	cg.refdef.vieworg[2] += cg.predictedPlayerState.viewheight;

	VectorCopy( cg.refdefViewAngles, focusAngles );

	// if dead, look at killer
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		focusAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
		cg.refdefViewAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
	}

	if ( focusAngles[PITCH] > 45 ) {
		focusAngles[PITCH] = 45;		// don't go too far overhead
	}
	AngleVectors( focusAngles, forward, NULL, NULL );

	VectorMA( cg.refdef.vieworg, FOCUS_DISTANCE, forward, focusPoint );

	VectorCopy( cg.refdef.vieworg, view );

	view[2] += 8;

	cg.refdefViewAngles[PITCH] *= 0.5;

	AngleVectors( cg.refdefViewAngles, forward, right, up );

	forwardScale = cos( cg_thirdPersonAngle->value / 180 * M_PI );
	sideScale = sin( cg_thirdPersonAngle->value / 180 * M_PI );
	VectorMA( view, -cg_thirdPersonRange->value * forwardScale, forward, view );
	VectorMA( view, -cg_thirdPersonRange->value * sideScale, right, view );

	// trace a ray from the origin to the viewpoint to make sure the view isn't
	// in a solid block.  Use an 8 by 8 block to prevent the view from near clipping anything

	if (!cg_cameraMode->integer) {
		CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );

		if ( trace.fraction != 1.0 ) {
			VectorCopy( trace.endpos, view );
			view[2] += (1.0 - trace.fraction) * 32;
			// try another trace to this position, because a tunnel may have the ceiling
			// close enogh that this is poking out

			CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );
			VectorCopy( trace.endpos, view );
		}
	}


	VectorCopy( view, cg.refdef.vieworg );

	// select pitch to look at focus point from vieword
	VectorSubtract( focusPoint, cg.refdef.vieworg, focusPoint );
	focusDist = sqrt( focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1] );
	if ( focusDist < 1 ) {
		focusDist = 1;	// should never happen
	}
	cg.refdefViewAngles[PITCH] = -180 / M_PI * atan2( focusPoint[2], focusDist );
	cg.refdefViewAngles[YAW] -= cg_thirdPersonAngle->value;
}


// this causes a compiler bug on mac MrC compiler
static void CG_StepOffset( void ) {
	int		timeDelta;
	
	// smooth out stair climbing
	timeDelta = cg.time - cg.stepTime;
	if ( timeDelta < STEP_TIME ) {
		cg.refdef.vieworg[2] -= cg.stepChange 
			* (STEP_TIME - timeDelta) / STEP_TIME;
	}
}

/*
===============
CG_OffsetFirstPersonView

===============
*/
static void CG_OffsetFirstPersonView( void ) {
	float			*origin;
	float			*angles;
	float			bob;
	float			ratio;
	float			delta;
	float			f;
	int				timeDelta;
	vec3_t tmp;
	vec3_t right;
	trace_t trace;
	vec3_t mins = { -8, -8, -8 };
	vec3_t maxs = { 8, 8, 8 };
	matrix_t m;
	vec3_t pos, v;

	origin = cg.refdef.vieworg;
	angles = cg.refdefViewAngles;

	// if dead, fix the angle and don't add any kick
	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 ) {
		angles[ROLL] = 40;
		angles[PITCH] = -15;
		angles[YAW] = cg.snap->ps.stats[STAT_DEAD_YAW];
		origin[2] += cg.predictedPlayerState.viewheight;
		return;
	}

	// add angles based on weapon kick
	VectorAdd (angles, cg.kick_angles, angles);

	// add angles based on damage kick
	if ( cg.damageTime ) {
		ratio = cg.time - cg.damageTime;
		if ( ratio < DAMAGE_DEFLECT_TIME ) {
			ratio /= DAMAGE_DEFLECT_TIME;
			angles[PITCH] += ratio * cg.v_dmg_pitch;
			angles[ROLL] += ratio * cg.v_dmg_roll;
		} else {
			ratio = 1.0 - ( ratio - DAMAGE_DEFLECT_TIME ) / DAMAGE_RETURN_TIME;
			if ( ratio > 0 ) {
				angles[PITCH] += ratio * cg.v_dmg_pitch;
				angles[ROLL] += ratio * cg.v_dmg_roll;
			}
		}
	}

	// add pitch based on fall kick
#if 0
	ratio = ( cg.time - cg.landTime) / FALL_TIME;
	if (ratio < 0)
		ratio = 0;
	angles[PITCH] += ratio * cg.fall_value;
#endif

#if 0

	// add angles based on velocity
	VectorCopy( cg.predictedPlayerState.velocity, predictedVelocity );

	delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[0]);
	angles[PITCH] += delta * cg_runpitch->value;
	
	delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[1]);
	angles[ROLL] -= delta * cg_runroll->value;

	// add angles based on bob

	// make sure the bob is visible even at low speeds
	speed = cg.xyspeed > 200 ? cg.xyspeed : 200;

	delta = cg.bobfracsin * cg_bobpitch->value * speed;
	if (cg.predictedPlayerState.pm_flags & PMF_DUCKED)
		delta *= 3;		// crouching
	angles[PITCH] += delta;
	delta = cg.bobfracsin * cg_bobroll->value * speed;
	if (cg.predictedPlayerState.pm_flags & PMF_DUCKED)
		delta *= 3;		// crouching accentuates roll
	if (cg.bobcycle & 1)
		delta = -delta;
	angles[ROLL] += delta;
#endif

//===================================

	// add view height
	origin[ 2 ] += cg.predictedPlayerState.viewheight;

	// smooth out duck height changes
	timeDelta = cg.time - cg.duckTime;
	if ( timeDelta < DUCK_TIME) {
		cg.refdef.vieworg[ 2 ] -= cg.duckChange
			* ( DUCK_TIME - timeDelta ) / DUCK_TIME;
	}

	if( cg.predictedPlayerState.groundEntityNum != ENTITYNUM_NONE )
	{
		float vel = VectorLength( cg.predictedPlayerState.velocity );

		cg.fCurrentViewBobPhase = ( ( ( float )cg.frametime ) / 1000 ) * M_PI
			* ( vel * 0.001500000013038516 + 0.8999999761581421 ) * 2 + cg.fCurrentViewBobPhase;

		if( cg.fCurrentViewBobAmp != 0.0 )
			vel = vel * 0.5;

		cg.fCurrentViewBobAmp = vel;

		if( cg.predictedPlayerState.fLeanAngle != 0.f ) {
			cg.fCurrentViewBobAmp = cg.fCurrentViewBobAmp * 0.75;
		}

		cg.fCurrentViewBobAmp = ( 1.0 - fabs( cg.refdefViewAngles[ 0 ] ) * 0.01111111138015985 * 0.5 )
			* 0.5
			* cg.fCurrentViewBobAmp;
	}
	else if( cg.fCurrentViewBobAmp > 0.0 )
	{
		float f;

		f = ( ( float )cg.frametime ) * 0.001 * cg.fCurrentViewBobAmp;
		cg.fCurrentViewBobAmp -= ( f + f );
	}

	if( cg.fCurrentViewBobAmp > 0.0 ) {

	}

	// add bob values
	VectorClear( pos );

	bob = cg.fCurrentViewBobAmp * ( sin( cg.fCurrentViewBobPhase - 0.9424778335276408 + cg.fCurrentViewBobPhase - 0.9424778335276408 + 3.141592653589793 )
		+ sin( ( cg.fCurrentViewBobPhase - 0.9424778335276408 ) * 4.0 + 1.570796326794897 ) * 0.125 ) * 0.05f;

	pos[ 1 ] = -( sin( cg.fCurrentViewBobPhase + 0.3141592700403172 ) * cg.fCurrentViewBobAmp * 0.05f ) * 1.5f;

	MatrixFromAngles( m, cg.refdefViewAngles[ 0 ], cg.refdefViewAngles[ 1 ], cg.refdefViewAngles[ 2 ] );
	MatrixTransformPoint( m, pos, v );

	VectorSubtract( origin, v, origin );

	origin[ 2 ] += bob;

	// add fall height
	delta = cg.time - cg.landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		f = delta / LAND_DEFLECT_TIME;
		cg.refdef.vieworg[2] += cg.landChange * f;
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		delta -= LAND_DEFLECT_TIME;
		f = 1.0 - ( delta / LAND_RETURN_TIME );
		cg.refdef.vieworg[2] += cg.landChange * f;
	}

	// add step offset
	CG_StepOffset();

	// add kick offset

	VectorAdd (origin, cg.kick_origin, origin);

	// pivot the eye based on a neck length
#if 0
	{
#define	NECK_LENGTH		8
	vec3_t			forward, up;
 
	cg.refdef.vieworg[2] -= NECK_LENGTH;
	AngleVectors( cg.refdefViewAngles, forward, NULL, up );
	VectorMA( cg.refdef.vieworg, 3, forward, cg.refdef.vieworg );
	VectorMA( cg.refdef.vieworg, NECK_LENGTH, up, cg.refdef.vieworg );
	}
#endif

	// add leaning rotation
	cg.refdefViewAngles[ROLL] = cg.predictedPlayerState.fLeanAngle * 0.300000011920929;

	// add leaning offset
#if 0
	a[0] = cg.refdefViewAngles[0];
	a[1] = cg.refdefViewAngles[1];
	a[2] = 0;
	AngleVectors(a,forward,0,0);

	//VectorCopy(cg.refdef.vieworg,tmp);
	RotatePointAroundVector(tmp, forward, vec3_origin, cg.predictedPlayerState.fLeanAngle);
	VectorAdd(tmp,cg.refdef.vieworg,cg.refdef.vieworg);
#else
	//add leaning offset
	AngleVectors( cg.refdefViewAngles, 0, right, 0 );
	VectorMA( cg.refdef.vieworg, cg.predictedPlayerState.fLeanAngle*0.5f, right, tmp );
	CG_Trace(&trace,cg.refdef.vieworg,mins,maxs,tmp,-1,CONTENTS_SOLID);
	VectorCopy(trace.endpos,cg.refdef.vieworg);
	// fLeanAngle range: <-40,40>
	//CG_Printf("Leanangle: %f\n",cg.predictedPlayerState.fLeanAngle);
#endif
}

//======================================================================

void CG_ZoomDown_f( void ) { 
	if ( cg.zoomed ) {
		return;
	}
	cg.zoomed = qtrue;
	cg.zoomTime = cg.time;
}

void CG_ZoomUp_f( void ) { 
	if ( !cg.zoomed ) {
		return;
	}
	cg.zoomed = qfalse;
	cg.zoomTime = cg.time;
}


/*
====================
CG_CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	0.4

static int CG_CalcFov( void ) {
	float	x;
	float	phase;
	float	v;
	int		contents;
	float	fov_x, fov_y;
	int		inwater;

#if 0
	if ( cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		// if in intermission, use a fixed value
		fov_x = 90;
	} else {
		// user selectable
		if ( cgs.dmflags & DF_FIXED_FOV ) {
			// dmflag to prevent wide fov for all clients
			fov_x = 90;
		} else {
			fov_x = cg_fov->value;
			if ( fov_x < 1 ) {
				fov_x = 1;
			} else if ( fov_x > 160 ) {
				fov_x = 160;
			}
		}

		// account for zooms
		zoomFov = cg_zoomFov->value;
		if ( zoomFov < 1 ) {
			zoomFov = 1;
		} else if ( zoomFov > 160 ) {
			zoomFov = 160;
		}

		if ( cg.zoomed ) {
			f = ( cg.time - cg.zoomTime ) / (float)ZOOM_TIME;
			if ( f > 1.0 ) {
				fov_x = zoomFov;
			} else {
				fov_x = fov_x + f * ( zoomFov - fov_x );
			}
		} else {
			f = ( cg.time - cg.zoomTime ) / (float)ZOOM_TIME;
			if ( f > 1.0 ) {
				fov_x = fov_x;
			} else {
				fov_x = zoomFov + f * ( fov_x - zoomFov );
			}
		}
	}
#else
	// su44: use FOV value send by MoHAA server
	if(cg.snap)
		fov_x = cg.snap->ps.fov;
	else 
		fov_x = 90;
#endif

	x = cg.refdef.width / tan( fov_x / 360 * M_PI );
	fov_y = atan2( cg.refdef.height, x );
	fov_y = fov_y * 360 / M_PI;

	// warp if underwater
	contents = CG_PointContents( cg.refdef.vieworg, -1 );
	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ){
		phase = cg.time / 1000.0 * WAVE_FREQUENCY * M_PI * 2;
		v = WAVE_AMPLITUDE * sin( phase );
		fov_x += v;
		fov_y -= v;
		inwater = qtrue;
	}
	else {
		inwater = qfalse;
	}


	// set it
	cg.refdef.fov_x = fov_x;
	cg.refdef.fov_y = fov_y;

	if ( !cg.zoomed ) {
		cg.zoomSensitivity = 1;
	} else {
		cg.zoomSensitivity = cg.refdef.fov_y / 75.0;
	}

	return inwater;
}



/*
===============
CG_DamageBlendBlob

===============
*/
static void CG_DamageBlendBlob( void ) {
	//int			t;
	//int			maxTime;
	//refEntity_t		ent;

	//if ( !cg.damageValue ) {
	//	return;
	//}

	////if (cg.cameraMode) {
	////	return;
	////}

	//// ragePro systems can't fade blends, so don't obscure the screen
	//if ( cgs.glconfig.hardwareType == GLHW_RAGEPRO ) {
	//	return;
	//}

	//maxTime = DAMAGE_TIME;
	//t = cg.time - cg.damageTime;
	//if ( t <= 0 || t >= maxTime ) {
	//	return;
	//}


	//memset( &ent, 0, sizeof( ent ) );
	//ent.reType = RT_SPRITE;
	//ent.renderfx = RF_FIRST_PERSON;

	//VectorMA( cg.refdef.vieworg, 8, cg.refdef.viewaxis[0], ent.origin );
	//VectorMA( ent.origin, cg.damageX * -8, cg.refdef.viewaxis[1], ent.origin );
	//VectorMA( ent.origin, cg.damageY * 8, cg.refdef.viewaxis[2], ent.origin );

	//ent.radius = cg.damageValue * 3;
	//ent.customShader = cgs.media.viewBloodShader;
	//ent.shaderRGBA[0] = 255;
	//ent.shaderRGBA[1] = 255;
	//ent.shaderRGBA[2] = 255;
	//ent.shaderRGBA[3] = 200 * ( 1.0 - ((float)t / maxTime) );
	//cgi.R_AddRefEntityToScene( &ent );
}


/*
===============
CG_CalcViewValues

Sets cg.refdef view values
===============
*/
static int CG_CalcViewValues( void ) {
	playerState_t	*ps;

	memset( &cg.refdef, 0, sizeof( cg.refdef ) );

	// su44: copy MoHAA fog data
	VectorCopy(cg.farplane_color,cg.refdef.farplane_color);
	cg.refdef.farplane_cull = cg.farplane_cull;
	cg.refdef.farplane_distance = cg.farplane_distance;

	// strings for in game rendering
	// Q_strncpyz( cg.refdef.text[0], "Park Ranger", sizeof(cg.refdef.text[0]) );
	// Q_strncpyz( cg.refdef.text[1], "19", sizeof(cg.refdef.text[1]) );

	// calculate size of 3D view
	CG_CalcVrect();

	ps = &cg.predictedPlayerState;
/*
	if (cg.cameraMode) {
		vec3_t origin, angles;
		if (cgi.getCameraInfo(cg.time, &origin, &angles)) {
			VectorCopy(origin, cg.refdef.vieworg);
			angles[ROLL] = 0;
			VectorCopy(angles, cg.refdefViewAngles);
			AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
			return CG_CalcFov();
		} else {
			cg.cameraMode = qfalse;
		}
	}
*/
	cg.bobcycle = ( ps->bobCycle & 128 ) >> 7;
	cg.bobfracsin = fabs( sin( ( ps->bobCycle & 127 ) / 127.0 * M_PI ) );
	cg.xyspeed = sqrt( ps->velocity[0] * ps->velocity[0] +
		ps->velocity[1] * ps->velocity[1] );


	VectorCopy( ps->origin, cg.refdef.vieworg );
	VectorCopy( ps->viewangles, cg.refdefViewAngles );

	if (cg_cameraOrbit->integer) {
		if (cg.time > cg.nextOrbitTime) {
			cg.nextOrbitTime = cg.time + cg_cameraOrbitDelay->integer;
			cg_thirdPersonAngle->value += cg_cameraOrbit->value;
		}
	}
	// add error decay
	if ( cg_errorDecay->value > 0 ) {
		int		t;
		float	f;

		t = cg.time - cg.predictedErrorTime;
		f = ( cg_errorDecay->value - t ) / cg_errorDecay->value;
		if ( f > 0 && f < 1 ) {
			VectorMA( cg.refdef.vieworg, f, cg.predictedError, cg.refdef.vieworg );
		} else {
			cg.predictedErrorTime = 0;
		}
	}

	if ( cg.renderingThirdPerson ) {
		// back away from character
		CG_OffsetThirdPersonView();
	} else {
		// offset for local bobbing and kicks
		CG_OffsetFirstPersonView();
	}

	// position eye reletive to origin
	AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );

	if ( cg.hyperspace ) {
		cg.refdef.rdflags |= RDF_NOWORLDMODEL | RDF_HYPERSPACE;
	}

	// field of view
	return CG_CalcFov();
}

//=========================================================================

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.
=================
*/
void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback ) {
	int		inwater;
	int		i;

	cg.time = serverTime;
	cg.demoPlayback = demoPlayback;

	// update cvars
	CG_UpdateCvars();

	// if we are only updating the screen as a loading
	// pacifier, don't even try to read snapshots
	if ( cg.infoScreenText[0] != 0 ) {
		CG_DrawInformation();
		return;
	}

	// any looped sounds will be respecified as entities
	// are added to the render list
	cgi.S_ClearLoopingSounds(qfalse);

	// clear all the render lists
	cgi.R_ClearScene();

	// set up cg.snap and possibly cg.nextSnap
	CG_ProcessSnapshots();

	// if we haven't received any snapshots yet, all
	// we can draw is the information screen
	if ( !cg.snap || ( cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) ) {
		CG_DrawInformation();
		return;
	}

	// let the client system know what our weapon and zoom settings are
	cgi.SetUserCmdValue( CG_WeaponCommandButtonBits(), cg.zoomSensitivity );

	// this counter will be bumped for every valid scene we generate
	cg.clientFrame++;

	// update cg.predictedPlayerState
	CG_PredictPlayerState();

	// decide on third person view
	cg.renderingThirdPerson = cg_thirdPerson->integer || (cg.snap->ps.stats[STAT_HEALTH] <= 0);

	// build cg.refdef
	inwater = CG_CalcViewValues();

	// first person blend blobs, done after AnglesToAxis
	if ( !cg.renderingThirdPerson ) {
		CG_DamageBlendBlob();
	}

	// build the render lists
	if ( !cg.hyperspace ) {
		CG_AddPacketEntities();			// adter calcViewValues, so predicted player state is correct
		CG_AddBulletTracers();
		CG_AddBulletImpacts();
		CG_AddBeams();
		CG_AddMarks();
		CG_AddParticles ();
		CG_AddLocalEntities();
	}
	// process MoHAA clientside events
	CG_ProcessPendingEvents();

	CG_AddViewWeapon( &cg.predictedPlayerState );

	// finish up the rest of the refdef
	if( cg.testModelEntity.model ) {
		CG_AddTestModel();
	}
	cg.refdef.time = cg.time;
	memcpy( cg.refdef.areamask, cg.snap->areamask, sizeof( cg.refdef.areamask ) );

	// update audio positions
	cgi.S_Respatialize( cg.snap->ps.clientNum, cg.refdef.vieworg, cg.refdef.viewaxis, inwater );

	// make sure the lagometerSample and frame timing isn't done twice when in stereo
	if ( stereoView != STEREO_RIGHT ) {
		cg.frametime = cg.time - cg.oldTime;
		if ( cg.frametime < 0 ) {
			cg.frametime = 0;
		}
		cg.oldTime = cg.time;
		CG_AddLagometerFrameInfo();
	}
	if (cg_timescale->value != cg_timescaleFadeEnd->value) {
		if (cg_timescale->value < cg_timescaleFadeEnd->value) {
			cg_timescale->value += cg_timescaleFadeSpeed->value * ((float)cg.frametime) / 1000;
			if (cg_timescale->value > cg_timescaleFadeEnd->value)
				cg_timescale->value = cg_timescaleFadeEnd->value;
		}
		else {
			cg_timescale->value -= cg_timescaleFadeSpeed->value * ((float)cg.frametime) / 1000;
			if (cg_timescale->value < cg_timescaleFadeEnd->value)
				cg_timescale->value = cg_timescaleFadeEnd->value;
		}
		if (cg_timescaleFadeSpeed->value) {
			cgi.Cvar_Set("timescale", va("%f", cg_timescale->value));
		}
	}

	// actually issue the rendering calls
	CG_DrawActive( stereoView );

	if ( cg_stats->integer ) {
		CG_Printf( "cg.clientFrame:%i\n", cg.clientFrame );
	}

	// bones pointers are no longer valid
	for( i = 0; i < MAX_ENTITIES; i++) {
		cg_entities[i].bones = 0;
	}
}

