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

#pragma once

typedef struct suninfo_s {
    vec3_t color;
    vec3_t direction;
    vec3_t flaredirection;
    char szFlareName[64];
    qboolean exists;
} suninfo_t;

typedef struct {
    vec3_t transformed;
    int index;
} sphere_dlight_t;

typedef enum {
    LIGHT_POINT,
    LIGHT_DIRECTIONAL,
    LIGHT_SPOT,
    LIGHT_SPOT_FAST
} lighttype_t;

typedef struct reallightinfo_s {
    vec3_t color;
    lighttype_t eType;
    float fIntensity;
    float fDist;
    float fSpotSlope;
    float fSpotConst;
    float fSpotScale;
    vec3_t vOrigin;
    vec3_t vDirection;
} reallightinfo_t;

typedef float cube_entry_t[3][4];

typedef struct {
    vec3_t origin;
    vec3_t worldOrigin;
    vec3_t traceOrigin;
    float radius;
    struct mnode_s* leaves[8];
    void(*TessFunction) (unsigned char* dstColors);
    union {
        unsigned char level[4];
        int value;
    } ambient;
    int numRealLights;
    reallightinfo_t light[32];
    int bUsesCubeMap;
    float cubemap[24][3][4];
} sphereor_t;

typedef struct spherel_s {
    vec3_t origin;
    vec3_t color;
    float intensity;
    struct mnode_s* leaf;
    int needs_trace;
    int spot_light;
    float spot_radiusbydistance;
    vec3_t spot_dir;
    int reference_count;
} spherel_t;

typedef enum {
    SPRITE_PARALLEL,
    SPRITE_PARALLEL_ORIENTED,
    SPRITE_ORIENTED,
    SPRITE_PARALLEL_UPRIGHT
} spriteType_t;

typedef struct {
    spriteType_t type;
    float scale;
} spriteParms_t;

typedef struct {
    float width;
    float height;
    float origin_x;
    float origin_y;
    float scale;
    struct shader_s* shader;
} sprite_t;
