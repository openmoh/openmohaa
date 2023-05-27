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

// tr_sphere_shade.cpp -- sphere shade

#include "../tr_local.h"

vec3_t spheredef[6];
suninfo_t s_sun;
static vec3_t ambientlight;
static qboolean bEntityOverbright;
static int iEntityLightingMax;
static int light_reference_count = 0;

int compare_light_intensities(const void* p1, const void* p2)
{
	return ((const reallightinfo_t*)p2)->fIntensity - ((const reallightinfo_t*)p1)->fIntensity;
}

static void RB_OptimizeLights()
{
	// FIXME: unimplemented
}

static void RB_Light_CubeMap(const color4ub_t colors)
{
	// FIXME: unimplemented
}

void RB_Light_Real(unsigned char* colors)
{
	int i, j;
	float* normal;
	float* xyz;
	unsigned char* color;
	vec3_t v;
	vec3_t colorout;
	float fDot;
	reallightinfo_t* pLight;


	// FIXME: unimplemented
}

static void RB_Sphere_Light_Sun()
{
	// FIXME: unimplemented
}

static qboolean RB_Sphere_CalculateSphereOrigin()
{
	// FIXME: unimplemented
	return qfalse;
}

static void RB_Sphere_DrawDebugLine(const spherel_t* thislight, float falloff, const vec3_t origin)
{
	// FIXME: unimplemented
}

static void RB_Sphere_AddSpotLight(const spherel_t* thislight)
{
	// FIXME: unimplemented
}

static void RB_Sphere_AddLight(const spherel_t* thislight)
{
	// FIXME: unimplemented
}

static void RB_Sphere_BuildStaticLights()
{
	// FIXME: unimplemented
}

void RB_Sphere_BuildDLights()
{
	// FIXME: unimplemented
}

void RB_Sphere_SetupEntity()
{
    // FIXME: unimplemented
}

void RB_Grid_SetupEntity()
{
	// FIXME: unimplemented
}

void RB_Grid_SetupStaticModel()
{
    // FIXME: unimplemented
}

void RB_Light_Fullbright(unsigned char* colors)
{
	memset((void*)colors, 0xFF, tess.numVertexes);
}

void R_Sphere_InitLights()
{
    // FIXME: unimplemented
}

int R_GatherLightSources(const vec3_t vPos, vec3_t* pvLightPos, vec3_t* pvLightIntensity, int iMaxLights)
{
    // FIXME: unimplemented
    return 0;
}
