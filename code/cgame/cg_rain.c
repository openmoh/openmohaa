/*
===========================================================================
Copyright (C) 2011 su44

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
// cg_rain.c - MoHAA rain/snow effects

#include "cg_local.h"

void CG_InitRainEffect() {
	cg.rain.density = 0;
	cg.rain.speed = 2048.0;
	cg.rain.length = 90.0;
	cg.rain.min_dist = 512.0;
	cg.rain.width = 1.f;
	cg.rain.speed_vary = 512;
	cg.rain.slant = 50;
	memset(cg.rain.shader,0,sizeof(cg.rain.shader));
	strcpy(cg.rain.shader[0],"textures/hud/allies"); // only for debuging
	cg.rain.numshaders = 0;
}

void CG_RainCSUpdated(int num, const char *str) {
	if( num == CS_RAIN_DENSITY ) {
		cg.rain.density = atof(str);
	} else if( num == CS_RAIN_SPEED ) {
		cg.rain.speed = atof(str);
	} else if( num == CS_RAIN_SPEEDVARY ) {
		cg.rain.speed_vary = atof(str);
	} else if( num == CS_RAIN_SLANT ) {
		cg.rain.slant = atof(str);
	} else if( num == CS_RAIN_LENGTH ) {
		cg.rain.length = atof(str);
	} else if( num == CS_RAIN_MINDIST ) {
		cg.rain.min_dist = atof(str);
	} else if( num == CS_RAIN_WIDTH ) {
		cg.rain.width = atof(str);
	} else if( num == CS_RAIN_SHADER ) {
		if(cg.rain.numshaders) {
			int i;
			for(i = 0; i < cg.rain.numshaders; i++) {
				sprintf(cg.rain.shader[i],"%s%i",str,i);
			}
		}
	} else if( num == CS_RAIN_NUMSHADERS ) {
		cg.rain.numshaders = atoi(str);
	} else {
		CG_Error("CG_RainCSUpdated: unknown cs index\n");
	}
}

void CG_Rain(centity_t *cent) {
	char *shadername;
	vec3_t vEnd;
	vec3_t vStart;
	//vec4_t fcolor;
	byte fcolor[4]; // su44: in MoHAA that's a float[4]
	vec3_t maxs;
	vec3_t mins;
	float f;
	vec3_t tMins,tMaxs;
	vec3_t size;
	int i;
	int density;
	int r;

	fcolor[0] = 1.f;
	fcolor[1] = 1.f;
	fcolor[2] = 1.f;
	fcolor[3] = 1.f;
	//if ( !cg_rain.integer  )
	//		return;
	cgi.R_ModelBounds(cgs.gameModels[cent->currentState.modelindex],mins,maxs);

	tMins[0] = mins[0] + cent->lerpOrigin[0];
	f = cg.snap->ps.origin[0] - cg.rain.min_dist;
	if ( f > tMins[0] )
		tMins[0] = f;

	tMins[1] = mins[1] + cent->lerpOrigin[1];
	f = cg.snap->ps.origin[1] - cg.rain.min_dist;
	if ( f > tMins[1] )
		tMins[1] = f;
	tMins[2] = mins[2] + cent->lerpOrigin[2];

	tMaxs[0] = maxs[0] + cent->lerpOrigin[0];
	f = cg.snap->ps.origin[0] + cg.rain.min_dist;
	if ( f > tMaxs[0] )
		tMaxs[0] = f;

	tMaxs[1] = maxs[1] + cent->lerpOrigin[1];
	f = cg.snap->ps.origin[1] + cg.rain.min_dist;
	if( f > tMaxs[1] )
		tMaxs[1] = f;

	tMaxs[2] = maxs[2] + cent->lerpOrigin[2];
	if ( tMins[0] >= tMaxs[0] )
		return;

	if (tMins[1] >= tMaxs[1] )
		return;
	

	size[0] = tMaxs[0] - tMins[0];
	size[1] = tMaxs[1] - tMins[1];
	size[2] = tMaxs[2] - tMins[2];
	density = (signed int)(cg.rain.density / 200.0 * sqrt(size[1] * size[0]));
	if ( density > 4096 )
		density = 4096;
	r = rand();
	if ( cg.rain.numshaders ) {
		shadername = &cg.rain.shader[r % cg.rain.numshaders][0];
	} else {
		shadername = &cg.rain.shader[0][0];
	}
	for(i = 0; i < density; i++) {
		int iLife;

		iLife = (signed int)(size[2] / ((long double)(r % cg.rain.speed_vary) + cg.rain.speed) * 1000.0);
		f = (long double)(r % (signed int)(size[0] + 1.0));
		r = ((214013 * r + 2531011) >> 16) & 0x7FFF;
		vStart[2] = tMaxs[2];
		vStart[0] = f + tMins[0];
		vStart[1] = (long double)(r % (signed int)(size[1] + 1.0)) + tMins[1];
		//if( )
		{
			f = r % cg.rain.slant;
			r = ((214013 * r + 2531011) >> 16) & 0x7FFF;
			vEnd[0] = f + vStart[0] + /*vss_wind_x->value*/8;
			vEnd[1] = (r % cg.rain.slant) + vStart[1] + /*vss_wind_y->value*/4;
			vEnd[2] = tMins[2];
			CG_CreateBeam(
			   vStart, // start 
			   vec3_origin,// dir
			   0, // owner
			   1, // model
			   1.0, // alpha
			   cg.rain.width, // scale
			   16384, // flags
			   0, // length <- isnt smth wrong there?
			   iLife, // life
			   1, // create
			   vEnd, // endpointvec
			   0, // min_offset
			   0, // max_offset
			   0, // overlap
			   1, // numSubdivisions
			   0, // delay
			   shadername,  // beamshadername
			   fcolor, // modulate (byte[4] here, float[4] in MoHAA)
			   0, // numspherebeams
			   0.0, //  sphereradius
			   cg.rain.length, // toggledelay
			   1.0, // endalpha
			   0, // renderfx
			   "raineffect"); // name
		}
	}
}