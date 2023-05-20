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

#include "tr_local.h"

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

static bool RB_Sphere_SetupGlobals()
{
	int i;

	if (backEnd.refdef.rdflags & RDF_NOWORLDMODEL) {
		if (!(backEnd.refdef.rdflags & RDF_FULLBRIGHT)) {
			backEnd.currentSphere->TessFunction = &RB_Light_Fullbright;
			return false;
		}
	}
	else if (!(backEnd.refdef.rdflags & RDF_FULLBRIGHT)) {
		if (!RB_Sphere_CalculateSphereOrigin()) {
			backEnd.currentSphere->TessFunction = &RB_CalcLightGridColor;

			if (!backEnd.currentEntity->bLightGridCalculated) {
				RB_SetupEntityGridLighting();
			}

			return false;
		}
	}
	else
	{
		if (backEnd.refdef.vieworg[0] == 0 && backEnd.refdef.vieworg[1] == 0 && backEnd.refdef.vieworg[2] == 0) {
			backEnd.currentSphere->TessFunction = &RB_Light_Fullbright;
			return false;
		}

		VectorCopy(backEnd.refdef.vieworg, backEnd.currentSphere->worldOrigin);
		VectorClear(backEnd.currentSphere->origin);
		backEnd.currentSphere->radius = 2.0;
	}

	light_reference_count++;
	for (i = 0; i < 8; i++) {
		backEnd.currentSphere->leaves[i] = 0;
	}

	R_SphereInLeafs(backEnd.currentSphere->worldOrigin, backEnd.currentSphere->radius, backEnd.currentSphere->leaves, 8);

	backEnd.currentSphere->TessFunction = &RB_Light_Real;
	return true;
}

static bool RB_Sphere_ResetPointColors()
{
	vec3_t light_offset, amb;

	R_GetLightingGridValue(backEnd.currentSphere->worldOrigin, light_offset);
	light_offset[0] = ambientlight[0] + light_offset[0] * 0.18;
	light_offset[1] = ambientlight[1] + light_offset[1] * 0.18;
	light_offset[2] = ambientlight[2] + light_offset[2] * 0.18;
	if (tr.refdef.rdflags & RDF_FULLBRIGHT)
	{
		float fMin = tr.identityLight * 20.0;

		if (fMin <= light_offset[0] || fMin <= light_offset[1] || fMin <= light_offset[2]) {
			light_offset[0] += fMin;
			light_offset[1] += fMin;
			light_offset[2] += fMin;
		}
	}

	backEnd.currentSphere->ambient.level[0] = r_entlight_scale->value * tr.overbrightMult * light_offset[0];
	if (backEnd.currentSphere->ambient.level[0] >= 255.0) {
		backEnd.currentSphere->ambient.level[0] = -1;
	}

	backEnd.currentSphere->ambient.level[1] = r_entlight_scale->value * tr.overbrightMult * light_offset[0];
	if (backEnd.currentSphere->ambient.level[1] >= 255.0) {
		backEnd.currentSphere->ambient.level[1] = -1;
	}

	backEnd.currentSphere->ambient.level[2] = r_entlight_scale->value * tr.overbrightMult * light_offset[0];
	if (backEnd.currentSphere->ambient.level[2] >= 255.0) {
		backEnd.currentSphere->ambient.level[2] = -1;
	}

	backEnd.currentSphere->ambient.level[3] = -1;

	return true;
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
	if (r_light_nolight->integer)
	{
		backEnd.currentSphere->TessFunction = &RB_Light_Fullbright;
	}
	else if (RB_Sphere_SetupGlobals() && RB_Sphere_ResetPointColors())
	{
		backEnd.currentSphere->numRealLights = 0;
		RB_Sphere_Light_Sun();
		RB_Sphere_BuildStaticLights();
		RB_Sphere_BuildDLights();
		backEnd.pc.c_characterlights += backEnd.currentSphere->numRealLights;
		RB_OptimizeLights();
	}
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
	const char* ents;
	const char* ret;
	qboolean bFlareDirSet;

	ents = ri.CM_EntityString();
	bFlareDirSet = qfalse;
	s_sun.szFlareName[0] = 0;
	s_sun.exists = qfalse;

	for (int i = 0; i < 128; i++) {
		backEnd.spheres[i].TessFunction = &RB_Light_Real;
	}

	VectorClear(ambientlight);
	backEnd.spareSphere.TessFunction = &RB_Light_Real;
	backEnd.currentSphere = &backEnd.spareSphere;
	bEntityOverbright = qfalse;
	iEntityLightingMax = tr.identityLightByte;
	s_sun.color[0] = s_sun.color[1] = s_sun.color[2] = tr.overbrightMult * 70;

	while (ents)
	{
		ret = COM_Parse((char**)&ents);
		if (*ret == '{' || *ret == '}') {
			continue;
		}

		if (!strcmp(ret, "suncolor") || !strcmp(ret, "sunlight"))
		{
			sscanf(COM_Parse((char**)&ents), "%f %f %f", &s_sun.color[0], &s_sun.color[1], &s_sun.color[2]);
			s_sun.color[0] *= tr.overbrightMult;
			s_sun.color[1] *= tr.overbrightMult;
			s_sun.color[2] *= tr.overbrightMult;
			s_sun.exists = qtrue;
		}
		else if (!strcmp(ret, "sundirection"))
		{
			vec3_t dir;

			sscanf(COM_Parse((char**)&ents), "%f %f %f", &dir[0], &dir[1], &dir[2]);
			AngleVectorsLeft(dir, s_sun.direction, 0, 0);

			s_sun.exists = qtrue;
			if (!bFlareDirSet)
			{
				s_sun.flaredirection[0] = s_sun.direction[0];
				s_sun.flaredirection[1] = s_sun.direction[1];
				s_sun.flaredirection[2] = s_sun.direction[2];
			}
		}
		else if (!strcmp(ret, "sunflaredirection"))
		{
			vec3_t dir;

			sscanf(COM_Parse((char**)&ents), "%f %f %f", &dir[0], &dir[1], &dir[2]);
			AngleVectorsLeft(dir, s_sun.flaredirection, 0, 0);
			bFlareDirSet = qtrue;
		}
		else if (!strcmp(ret, "sunflarename"))
		{
			strcpy(s_sun.szFlareName, COM_Parse((char**)&ents));
		}
		else if (!strcmp(ret, "ambientlight"))
		{
			sscanf(COM_Parse((char**)&ents), "%f %f %f", &ambientlight[0], &ambientlight[1], &ambientlight[2]);
		}
		else if (!strcmp(ret, "overbright"))
		{
			ret = COM_Parse((char**)&ents);
			if (strcmp(ret, "world") || strcmp(ret, "none"))
			{
				bEntityOverbright = qtrue;
				iEntityLightingMax = 0xFF;
			}
		}
		else
		{
			COM_Parse((char**)&ents);
		}
	}

	if (s_sun.exists)
	{
		if (!s_sun.szFlareName[0]) {
			strcpy(s_sun.szFlareName, "sun");
		}
	}
}

int R_GatherLightSources(const vec3_t vPos, vec3_t* pvLightPos, vec3_t* pvLightIntensity, int iMaxLights)
{
    // FIXME: unimplemented
    return 0;
}
