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
// cg_weapons.c -- events and effects dealing with weapons
#include "cg_local.h"

/*
==========================
CG_MachineGunEjectBrass
==========================
*/
static void CG_MachineGunEjectBrass( centity_t *cent ) {
	/*localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			velocity, xvelocity;
	vec3_t			offset, xoffset;
	float			waterScale = 1.0f;
	vec3_t			v[3];

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 0;
	velocity[1] = -50 + 40 * crandom();
	velocity[2] = 100 + 50 * crandom();

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + cg_brassTime.integer + ( cg_brassTime.integer / 4 ) * random();

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time - (rand()&15);

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 8;
	offset[1] = -4;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd( cent->lerpOrigin, xoffset, re->origin );

	VectorCopy( re->origin, le->pos.trBase );

	if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale( xvelocity, waterScale, le->pos.trDelta );

	AxisCopy( axisDefault, re->axis );
	re->hModel = cgs.media.machinegunBrassModel;

	le->bounceFactor = 0.4 * waterScale;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()&31;
	le->angles.trBase[1] = rand()&31;
	le->angles.trBase[2] = rand()&31;
	le->angles.trDelta[0] = 2;
	le->angles.trDelta[1] = 1;
	le->angles.trDelta[2] = 0;

	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;*/
}

/*
==========================
CG_ShotgunEjectBrass
==========================
*/
static void CG_ShotgunEjectBrass( centity_t *cent ) {
	//localEntity_t	*le;
	//refEntity_t		*re;
	//vec3_t			velocity, xvelocity;
	//vec3_t			offset, xoffset;
	//vec3_t			v[3];
	//int				i;

	//if ( cg_brassTime.integer <= 0 ) {
	//	return;
	//}

	//for ( i = 0; i < 2; i++ ) {
	//	float	waterScale = 1.0f;

	//	le = CG_AllocLocalEntity();
	//	re = &le->refEntity;

	//	velocity[0] = 60 + 60 * crandom();
	//	if ( i == 0 ) {
	//		velocity[1] = 40 + 10 * crandom();
	//	} else {
	//		velocity[1] = -40 + 10 * crandom();
	//	}
	//	velocity[2] = 100 + 50 * crandom();

	//	le->leType = LE_FRAGMENT;
	//	le->startTime = cg.time;
	//	le->endTime = le->startTime + cg_brassTime.integer*3 + cg_brassTime.integer * random();

	//	le->pos.trType = TR_GRAVITY;
	//	le->pos.trTime = cg.time;

	//	AnglesToAxis( cent->lerpAngles, v );

	//	offset[0] = 8;
	//	offset[1] = 0;
	//	offset[2] = 24;

	//	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	//	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	//	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	//	VectorAdd( cent->lerpOrigin, xoffset, re->origin );
	//	VectorCopy( re->origin, le->pos.trBase );
	//	if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
	//		waterScale = 0.10f;
	//	}

	//	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	//	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	//	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	//	VectorScale( xvelocity, waterScale, le->pos.trDelta );

	//	AxisCopy( axisDefault, re->axis );
	//	re->hModel = cgs.media.shotgunBrassModel;
	//	le->bounceFactor = 0.3f;

	//	le->angles.trType = TR_LINEAR;
	//	le->angles.trTime = cg.time;
	//	le->angles.trBase[0] = rand()&31;
	//	le->angles.trBase[1] = rand()&31;
	//	le->angles.trBase[2] = rand()&31;
	//	le->angles.trDelta[0] = 1;
	//	le->angles.trDelta[1] = 0.5;
	//	le->angles.trDelta[2] = 0;

	//	le->leFlags = LEF_TUMBLE;
	//	le->leBounceSoundType = LEBS_BRASS;
	//	le->leMarkType = LEMT_NONE;
	//}
}


#ifdef MISSIONPACK
/*
==========================
CG_NailgunEjectBrass
==========================
*/
static void CG_NailgunEjectBrass( centity_t *cent ) {
	localEntity_t	*smoke;
	vec3_t			origin;
	vec3_t			v[3];
	vec3_t			offset;
	vec3_t			xoffset;
	vec3_t			up;

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 0;
	offset[1] = -12;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd( cent->lerpOrigin, xoffset, origin );

	VectorSet( up, 0, 0, 64 );

	smoke = CG_SmokePuff( origin, up, 32, 1, 1, 1, 0.33f, 700, cg.time, 0, 0, cgs.media.smokePuffShader );
	// use the optimized local entity add
	smoke->leType = LE_SCALE_FADE;
}
#endif


/*
========================================================================================

VIEW WEAPON

========================================================================================
*/


/*
==============
CG_CalculateWeaponPosition
==============
*/
static void CG_CalculateWeaponPosition( vec3_t origin, vec3_t angles ) {
	float	scale;
	int		delta;
	float	fracsin;

	VectorCopy( cg.refdef.vieworg, origin );
	VectorCopy( cg.refdefViewAngles, angles );

	// on odd legs, invert some angles
	if ( cg.bobcycle & 1 ) {
		scale = -cg.xyspeed;
	} else {
		scale = cg.xyspeed;
	}

	// gun angles from bobbing
	angles[ROLL] += scale * cg.bobfracsin * 0.005;
	angles[YAW] += scale * cg.bobfracsin * 0.01;
	angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.005;

	// drop the weapon when landing
	delta = cg.time - cg.landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		origin[2] += cg.landChange*0.25 * delta / LAND_DEFLECT_TIME;
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		origin[2] += cg.landChange*0.25 *
			(LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
	}

#if 0
	// drop the weapon when stair climbing
	delta = cg.time - cg.stepTime;
	if ( delta < STEP_TIME/2 ) {
		origin[2] -= cg.stepChange*0.25 * delta / (STEP_TIME/2);
	} else if ( delta < STEP_TIME ) {
		origin[2] -= cg.stepChange*0.25 * (STEP_TIME - delta) / (STEP_TIME/2);
	}
#endif

	// idle drift
	scale = cg.xyspeed + 40;
	fracsin = sin( cg.time * 0.001 );
	angles[ROLL] += scale * fracsin * 0.01;
	angles[YAW] += scale * fracsin * 0.01;
	angles[PITCH] += scale * fracsin * 0.01;
}


/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon( playerState_t *ps ) {
	//refEntity_t	hand;
	//centity_t	*cent;
	//clientInfo_t	*ci;
	//float		fovOffset;
	//vec3_t		angles;

	////if ( ps->stats[STAT_TEAM] == TEAM_SPECTATOR ) {
	////	return;
	////}

	//if ( ps->pm_type == PM_INTERMISSION ) {
	//	return;
	//}

	//// no gun if in third person view or a camera is active
	////if ( cg.renderingThirdPerson || cg.cameraMode) {
	//if ( cg.renderingThirdPerson ) {
	//	return;
	//}


	////// allow the gun to be completely removed
	////if ( !cg_drawGun.integer ) {
	////	vec3_t		origin;

	////	if ( cg.predictedPlayerState.eFlags & EF_FIRING ) {
	////		// special hack for lightning gun...
	////		VectorCopy( cg.refdef.vieworg, origin );
	////		VectorMA( origin, -8, cg.refdef.viewaxis[2], origin );
	////		CG_LightningBolt( &cg_entities[ps->clientNum], origin );
	////	}
	////	return;
	////}

	//// don't draw if testing a gun model
	//if ( cg.testGun ) {
	//	return;
	//}

	//// drop gun lower at higher fov
	//if ( cg_fov.integer > 90 ) {
	//	fovOffset = -0.2 * ( cg_fov.integer - 90 );
	//} else {
	//	fovOffset = 0;
	//}

	//cent = &cg.predictedPlayerEntity;	// &cg_entities[cg.snap->ps.clientNum];


	//memset (&hand, 0, sizeof(hand));

	//// set up gun position
	//CG_CalculateWeaponPosition( hand.origin, angles );

	//VectorMA( hand.origin, cg_gun_x.value, cg.refdef.viewaxis[0], hand.origin );
	//VectorMA( hand.origin, cg_gun_y.value, cg.refdef.viewaxis[1], hand.origin );
	//VectorMA( hand.origin, (cg_gun_z.value+fovOffset), cg.refdef.viewaxis[2], hand.origin );

	//AnglesToAxis( angles, hand.axis );

	//// map torso animations to weapon animations
	//if ( cg_gun_frame.integer ) {
	//	// development tool
	//	hand.frame = hand.oldframe = cg_gun_frame.integer;
	//	hand.backlerp = 0;
	//} else {
	//	// get clientinfo for animation map
	//	ci = &cgs.clientinfo[ cent->currentState.clientNum ];
	//	hand.frame = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.frame );
	//	hand.oldframe = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.oldFrame );
	//	hand.backlerp = cent->pe.torso.backlerp;
	//}

	//hand.hModel = weapon->handsModel;
	//hand.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON | RF_MINLIGHT;

	//// add everything onto the hand
	//CG_AddPlayerWeapon( &hand, ps, &cg.predictedPlayerEntity, ps->stats[STAT_TEAM] );
}

/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/

/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f( void ) {
	cg.iWeaponCommand = 12;
	cg.iWeaponCommandSend = 0;
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f( void ) {
	cg.iWeaponCommand = 11;
	cg.iWeaponCommandSend = 0;
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_UseLastWeapon_f( void )
{
	cg.iWeaponCommand = 13;
	cg.iWeaponCommandSend = 0;
}

/*
===============
CG_HolsterWeapon_f
===============
*/
void CG_HolsterWeapon_f( void )
{
	cg.iWeaponCommand = 14;
	cg.iWeaponCommandSend = 0;
}

/*
===============
CG_DropWeapon_f
===============
*/
void CG_DropWeapon_f( void )
{
	cg.iWeaponCommand = 15;
	cg.iWeaponCommandSend = 0;
}

/*
===============
CG_ToggleItem_f
===============
*/
void CG_ToggleItem_f()
{
	cg.iWeaponCommand = 7;
	cg.iWeaponCommandSend = 0;
}

// su44: encode current weapon command for MoHAA
int CG_WeaponCommandButtonBits() {
	int ret;
	if( !cg.iWeaponCommand )
		return 0;

	ret = (cg.iWeaponCommand << 7) & 0x780;

	cg.iWeaponCommandSend++;

	if( cg.iWeaponCommandSend > 2 )
		cg.iWeaponCommand = 0;

	return ret;
}
