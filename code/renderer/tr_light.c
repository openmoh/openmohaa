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
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_light.c

#include "tr_local.h"

#define	DLIGHT_AT_RADIUS		16
// at the edge of a dlight's influence, this amount of light will be added

#define	DLIGHT_MINIMUM_RADIUS	16		
// never calculate a range less than this to prevent huge light numbers

typedef struct {
  dlight_t *dl;
  float power;
  vec3_t origin;
} incidentLight_t;

typedef struct {
  int dlightMap;
  int allocated[LIGHTMAP_SIZE];
  byte lightmap_buffer[LIGHTMAP_SIZE * LIGHTMAP_SIZE * 4];
  byte* srcBase;
  byte* dstBase;
  incidentLight_t lights[32];
  int numLights;
} dlightInfo_t;

typedef struct {
  vec3_t point;
  int s;
  int t;
} patchLightBlock_t;

dlightInfo_t dli;

void R_SetupEntityLightingGrid(trRefEntity_t* ent);
qboolean R_DlightSample(byte* src, const vec3_t vec, byte* dst);
qboolean R_AllocLMBlock(int w, int h, int* x, int* y);

static int R_RecursiveDlightPatch(patchLightBlock_t* plb) {
    patchLightBlock_t cut[4];
    qboolean added;
    int index;

    if (plb[0].s < plb[1].s - 1)
	{
        cut[0] = plb[0];
        cut[2] = plb[2];

        VectorAdd(plb[0].point, plb[1].point, cut[1].point);
        VectorScale(cut[1].point, 0.5f, cut[1].point);
        VectorAdd(plb[2].point, plb[3].point, cut[3].point);
        VectorScale(cut[3].point, 0.5f, cut[3].point);

        cut[3].s = (plb[0].s + plb[1].s) >> 1;
        cut[1].s = cut[3].s;
        cut[1].t = plb[0].t;
        cut[3].t = plb[2].t;

        added = R_RecursiveDlightPatch(cut);

        cut[0] = cut[1];
        cut[2] = cut[3];

        index = 1;
	}
	else if (plb[0].t < plb[2].t - 1)
    {
        cut[0] = plb[0];
        cut[1] = plb[1];

        VectorAdd(plb[0].point, plb[2].point, cut[2].point);
        VectorScale(cut[2].point, 0.5f, cut[2].point);
        VectorAdd(plb[1].point, plb[3].point, cut[3].point);
        VectorScale(cut[3].point, 0.5f, cut[3].point);

        cut[3].t = (plb[0].t + plb[2].t) >> 1;
        cut[2].t = cut[3].t;
        cut[2].s = plb[0].s;
        cut[3].s = plb[1].s;

        added = R_RecursiveDlightPatch(cut);
        cut[0] = cut[2];
        cut[1] = cut[3];

        index = 2;
	}
	else
	{
		return R_DlightSample(
			&dli.srcBase[(LIGHTMAP_SIZE * 3) + plb[0].s * 3],
			plb[0].point,
            &dli.dstBase[(LIGHTMAP_SIZE * 4) + plb[0].s * 4]
        );
	}

	cut[index] = plb[index];
	cut[3] = plb[3];

	return R_RecursiveDlightPatch(cut) + added;
}

int R_RealDlightPatch(srfGridMesh_t* srf, int dlightBits) {
    int x, y;
    int i, j;
    int i2, j2;
    int steps[2][2];
    dlight_t* dl;
    qboolean added;
    float* origin;
    drawVert_t* dv;

    if (!srf->lmHeight || !srf->lmWidth)
    {
        srf->dlightMap[tr.smpFrame] = 0;
        return 0;
    }

	dli.numLights = 0;

	for (i = 0; i < tr.refdef.num_dlights; i++) {
		if (!(dlightBits & (1 << i))) {
			continue;
		}

		dl = &tr.refdef.dlights[i];
		if (dl->origin[0] + dl->radius < srf->meshBounds[0][0]
			|| dl->origin[0] - dl->radius > srf->meshBounds[1][0]
			|| dl->origin[1] + dl->radius < srf->meshBounds[0][1]
			|| dl->origin[1] - dl->radius > srf->meshBounds[1][1]
			|| dl->origin[2] + dl->radius < srf->meshBounds[0][2]
			|| dl->origin[2] - dl->radius > srf->meshBounds[1][2])
		{
			dlightBits &= ~(1 << i);
			continue;
		}

		VectorCopy(dl->transformed, dli.lights[dli.numLights].origin);
		dli.lights[dli.numLights].dl = dl;
		dli.lights[dli.numLights].power = 1.f / dl->radius;
		dli.numLights++;
	}

	if (!dli.numLights) {
		srf->dlightMap[tr.smpFrame] = 0;
		return 0;
	}

	if (!R_AllocLMBlock(srf->lmWidth, srf->lmHeight, &x, &y)) {
        srf->dlightMap[tr.smpFrame] = 0;
        return 0;
	}

    dli.srcBase = srf->lmData;
    dli.dstBase = &dli.lightmap_buffer[y * 4 * LIGHTMAP_SIZE + x * 4];

    srf->lightmapOffset[0] = (float)x / LIGHTMAP_SIZE - (float)srf->lmX / LIGHTMAP_SIZE;
    srf->lightmapOffset[1] = (float)y / LIGHTMAP_SIZE - (float)srf->lmY / LIGHTMAP_SIZE;

    tr.pc.c_dlightSurfaces++;
    tr.pc.c_dlightTexels += srf->lmWidth * srf->lmHeight;

	added = 0;
	
	if (srf->verts[srf->width].lightmap[0] != srf->verts[0].lightmap[0]) {
		if (srf->verts[srf->width].lightmap[0] < srf->verts[0].lightmap[0]) {
			steps[0][0] = 0;
			steps[0][1] = -1;
		} else {
			steps[0][0] = 0;
			steps[0][1] = 1;
		}

		if (srf->verts[srf->width * (srf->height - 1)].lightmap[1] < srf->verts[0].lightmap[1]) {
            steps[1][0] = -1;
            steps[1][1] = 0;
		} else {
            steps[1][0] = 1;
            steps[1][1] = 0;
		}
	} else {
		if (srf->verts[srf->width * (srf->height - 1)].lightmap[0] < srf->verts[0].lightmap[0]) {
            steps[0][0] = -1;
            steps[0][1] = 0;
		} else {
            steps[0][0] = 1;
            steps[0][1] = 0;
		}

        if (srf->verts[srf->width].lightmap[1] < srf->verts[0].lightmap[1]) {
            steps[1][0] = 0;
            steps[1][1] = -1;
        } else {
            steps[1][0] = 0;
            steps[1][1] = -1;
        }
	}

	for (i = 0; i < srf->height; i++) {
		for (j = 0; j < srf->width; j++) {
			patchLightBlock_t plb[4];

			//
			// Vert 1
			//
			dv = &srf->verts[i * srf->width + j];

			VectorCopy(dv->xyz, plb[0].point);
			plb[0].s = dv->lightmap[0] * 128.0 - srf->lmX;
			plb[0].t = dv->lightmap[1] * 128.0 - srf->lmY;

			i2 = steps[0][0] + i;
			if (i2 < 0) { i2 = 0; }
			else if (i2 >= srf->height) { i2 = srf->height - 1; }

            j2 = steps[0][1] + j;
			if (j2 < 0) { j2 = 0; }
            else if (j2 >= srf->width) { j2 = srf->width - 1; }

            //
            // Vert 2
            //
            dv = &srf->verts[srf->width * i2 + j2];

            VectorCopy(dv->xyz, plb[1].point);
            plb[1].s = dv->lightmap[0] * 128.0 - srf->lmX;
            plb[1].t = dv->lightmap[1] * 128.0 - srf->lmY;

            i2 = steps[1][0] + i;
			if (i2 < 0) { i2 = 0; }
			else if (i2 >= srf->height) { i2 = srf->height - 1; }

            j2 = steps[1][1] + j;
            if (j2 < 0) { j2 = 0; }
            else if (j2 >= srf->width) { j2 = srf->width - 1; }

            //
            // Vert 3
            //
            dv = &srf->verts[srf->width * i2 + j2];

            VectorCopy(dv->xyz, plb[2].point);
            plb[2].s = dv->lightmap[0] * 128.0 - srf->lmX;
            plb[2].t = dv->lightmap[1] * 128.0 - srf->lmY;

            i2 = steps[1][0] + steps[0][0] + i;
			if (i2 < 0) { i2 = 0; }
			else if (i2 >= srf->height) { i2 = srf->height - 1; }

            j2 = steps[1][1] + steps[0][1] + j;
            if (j2 < 0) { j2 = 0; }
            else if (j2 >= srf->width) { j2 = srf->width - 1; }

            //
            // Vert 4
            //
            dv = &srf->verts[srf->width * i2 + j2];

            VectorCopy(dv->xyz, plb[3].point);
            plb[3].s = dv->lightmap[0] * 128.0 - srf->lmX;
            plb[3].t = dv->lightmap[1] * 128.0 - srf->lmY;

			added |= R_RecursiveDlightPatch(plb);
		}
	}

    if (!added) {
        srf->dlightMap[tr.smpFrame] = 0;
        return 0;
	}

	for (i = 0; i < srf->width; i++) {
		dli.allocated[x + i] = srf->lmHeight + y;
	}

    srf->dlightMap[tr.smpFrame] = dli.dlightMap + 1;
    return srf->dlightMap[tr.smpFrame];
}

int R_RealDlightFace(srfSurfaceFace_t* srf, int dlightBits) {
    int x, y;
    byte* src, * dst;
    int i, j;
    vec3_t vec;
    vec3_t vecStepS, vecStepT;
    dlight_t* dl;
    float d;
    qboolean added;
    float* origin;

	if (!srf->lmHeight || !srf->lmWidth) {
		srf->dlightMap[tr.smpFrame] = 0;
		return 0;
	}

	dli.numLights = 0;

	for (i = 0; i < tr.refdef.num_dlights; i++) {
        if (!(dlightBits & (1 << i))) {
            continue;
        }

        dl = &tr.refdef.dlights[i];

        d = DotProduct(dl->transformed, srf->plane.normal) - srf->plane.dist;

        if ((d < 0.f && !r_dlightBacks->integer)
            || (d > dl->radius || d < -dl->radius)) {
            // dlight doesn't reach the plane
            dlightBits &= ~(1 << i);
        }
        else {
            VectorCopy(dl->transformed, dli.lights[dli.numLights].origin);
            dli.lights[dli.numLights].dl = dl;
            dli.lights[dli.numLights].power = 1.0 / dl->radius;
			dli.numLights++;
        }
    }

    if (!dli.numLights) {
        srf->dlightMap[tr.smpFrame] = 0;
        return 0;
    }

	if (!R_AllocLMBlock(srf->lmWidth, srf->lmHeight, &x, &y)) {
		srf->dlightMap[tr.smpFrame] = 0;
        return 0;
	}

    src = srf->lmData;
    dst = &dli.lightmap_buffer[y * 4 * LIGHTMAP_SIZE + x * 4];

	srf->lightmapOffset[0] = (float)x / LIGHTMAP_SIZE - (float)srf->lmX / LIGHTMAP_SIZE;
	srf->lightmapOffset[1] = (float)y / LIGHTMAP_SIZE - (float)srf->lmY / LIGHTMAP_SIZE;

	tr.pc.c_dlightSurfaces++;
	tr.pc.c_dlightTexels += srf->lmWidth * srf->lmHeight;

	VectorCopy(srf->lmVecs[0], vecStepS);
	VectorMA(srf->lmVecs[1], -srf->lmWidth, srf->lmVecs[0], vecStepT);

	added = qfalse;
	VectorCopy(srf->lmOrigin, vec);

	for (i = 0; i < srf->lmHeight; i++) {
		for (j = 0; j < srf->lmWidth; j++) {
			added |= R_DlightSample(src, vec, dst);
			VectorAdd(vec, vecStepS, vec);
			src += 3;
			dst += 4;
		}

		VectorAdd(vec, vecStepT, vec);
		src += 3 * (LIGHTMAP_SIZE - j);
		dst += 4 * (LIGHTMAP_SIZE - j);
	}

	if (!added) {
        srf->dlightMap[tr.smpFrame] = 0;
        return 0;
	}

    for (i = 0; i < srf->lmWidth; i++) {
        dli.allocated[x + i] = srf->lmHeight + y;
    }

    srf->dlightMap[tr.smpFrame] = dli.dlightMap + 1;
    return srf->dlightMap[tr.smpFrame];
}

int R_RealDlightTerrain(cTerraPatchUnpacked_t* srf, int dlightBits) {
    dlight_t* dl;
    int x, y;
    byte* dst, * src;
    vec3_t vec;
    float delta, dist;
    float* origin;
    int i, j, k;
    int di, dj;
    qboolean added;
    float lmScale;
    int lumelsPerHeight;
    float heightPerLumelSquared;
    float z00, z01;
    float z10, z11;

	for (i = 0; i < tr.refdef.num_dlights; i++) {
		dl = &tr.refdef.dlights[i];

		if (dl->radius < srf->x0 - dl->transformed[0]
			|| -512.f - dl->radius > srf->x0 - dl->transformed[0]
			|| -512.f - dl->radius > srf->y0 - dl->transformed[1]
			|| dl->radius < srf->y0 - dl->transformed[1])
		{
			continue;
		}

		if (dl->radius > 128.f) {
			dist = 0.f;

			delta = dl->transformed[0] - srf->x0;
			if (delta <= 0.f) {
				dist += delta * delta;
			} else {
				delta -= 512.f;
				if (delta > 0.f) dist += delta * delta;
            }

            delta = dl->transformed[1] - srf->y0;
            if (delta <= 0.f) {
                dist += delta * delta;
            } else {
                delta -= 512.f;
                if (delta > 0.f) dist += delta * delta;
            }

			delta = dl->transformed[0] - srf->z0;
			if (delta <= 0.f) {
				dist += delta * delta;
			} else {
				delta -= srf->zmax;
				if (delta > 0.f) dist += delta * delta;
			}

			if (dl->radius * dl->radius < dist) {
				continue;
			}
		}

		VectorCopy(dl->origin, dli.lights[dli.numLights].origin);
		dli.lights[dli.numLights].dl = dl;
		dli.lights[dli.numLights].power = 1.f / dl->radius;
		dli.numLights++;
	}

    if (!dli.numLights)
    {
        srf->drawinfo.dlightMap[tr.smpFrame] = 0;
        return 0;
    }

    if (!R_AllocLMBlock(srf->drawinfo.lmapSize, srf->drawinfo.lmapSize, &x, &y))
    {
        srf->drawinfo.dlightMap[tr.smpFrame] = 0;
        return 0;
    }

	src = srf->drawinfo.lmData;
	dst = &dli.lightmap_buffer[y * 4 * LIGHTMAP_SIZE + x * 4];

	srf->drawinfo.lmapX = x / 128.f;
	srf->drawinfo.lmapY = y / 128.f;

    tr.pc.c_dlightSurfaces++;
    tr.pc.c_dlightTexels += srf->drawinfo.lmapSize * srf->drawinfo.lmapSize;

	added = qfalse;

	lumelsPerHeight = 64.f / srf->drawinfo.lmapStep;
	if (lumelsPerHeight == 1) {
		vec[1] = srf->y0;

		for (i = 0; i < 9; i++) {
			vec[0] = srf->x0;

			for (j = 0; j < 9; j++) {
				vec[2] = srf->z0 + (srf->heightmap[0] << 1);
				added |= R_DlightSample(src, vec, dst);

				src += 3;
				dst += 4;
				vec[0] += srf->drawinfo.lmapStep;
			}

			src += (LIGHTMAP_SIZE - 9) * 3;
			dst += (LIGHTMAP_SIZE - 9) * 4;
			vec[1] += srf->drawinfo.lmapStep;
		}
	} else if (lumelsPerHeight == 2) {
		vec[1] = srf->y0;

		k = 0;

		for (j = 0; j < 8; j++) {
			vec[0] = srf->x0;

			for (i = 0; i < 8; i++) {
				z00 = srf->heightmap[k];
				vec[2] = srf->z0 + z00 + z00;
				added |= R_DlightSample(src, vec, dst);
				// increase lightmap step
				vec[0] += srf->drawinfo.lmapStep;

				z01 = srf->heightmap[k];
                vec[2] = srf->z0 + z00 + z01;
                added |= R_DlightSample(src, vec, dst);

                vec[0] += srf->drawinfo.lmapStep;
			}

            vec[2] = srf->z0 + z01 + z01;
            added |= R_DlightSample(src, vec, dst);
			src += (LIGHTMAP_SIZE - 16) * 3;
			dst += (LIGHTMAP_SIZE - 16) * 4;
			k -= 8;

			vec[0] = srf->x0;
			vec[1] += srf->drawinfo.lmapStep;

			for (i = 0; i < 8; i++) {
				z00 = srf->heightmap[k] + srf->heightmap[k + 9];
				vec[2] = srf->z0 + z00;

				added |= R_DlightSample(src, vec, dst);
				k++;

				vec[0] += srf->drawinfo.lmapStep;

				z01 = srf->heightmap[k] + srf->heightmap[k + 9];
                vec[2] = srf->z0 + (z00 + z01) * 0.5f;

                added |= R_DlightSample(src, vec, dst);

				src += 3;
				dst += 4;
				vec[0] += srf->drawinfo.lmapStep;
			}
		}

		vec[0] = srf->x0;

		for (i = 0; i < 7; i++) {
			z00 = srf->heightmap[k];
			vec[2] = srf->z0 + z00 + z00;

			k++;
			added |= R_DlightSample(src, vec, dst);
			vec[0] += srf->drawinfo.lmapStep;

			z01 = srf->heightmap[k];
            vec[2] = srf->z0; z00 + z01;
            added |= R_DlightSample(src, vec, dst);

			src += 3;
            dst += 4;
            vec[0] += srf->drawinfo.lmapStep;
		}

        vec[2] = srf->z0 + z01 + z01;
        added |= R_DlightSample(src, vec, dst);
        vec[0] += srf->drawinfo.lmapStep;
		vec[1] += srf->drawinfo.lmapStep;
	}
	else
	{
		vec[1] = srf->y0;

		for (j = 0; j < 8; j++) {
			for (;;) {
				if (j == 7) {
					if (dj >= lumelsPerHeight + 1) {
						break;
					}
				} else if (dj >= lumelsPerHeight) {
					break;
				}

				vec[0] = srf->x0;

				for (i = 0; i < 8; i++) {
					for (;;) {
                        if (i == 7) {
                            if (di >= lumelsPerHeight + 1) {
                                break;
                            }
                        } else if (dj >= lumelsPerHeight) {
                            break;
                        }

						z00 = srf->heightmap[9 * j + i];
                        z01 = srf->heightmap[9 * (j + 1) + i];
                        z10 = srf->heightmap[9 * j + (i + 1)];
                        z01 = srf->heightmap[9 * (j + 1) + (i + 1)];

						heightPerLumelSquared = 2.f / (lumelsPerHeight * lumelsPerHeight);
					
						vec[2] = srf->z0 + (dj * (di * z11)
							+ (lumelsPerHeight - dj) * (z10 * di)
							+ z01 * (lumelsPerHeight - di) * dj
							+ z00 * (lumelsPerHeight - di) * (lumelsPerHeight - dj))
							* heightPerLumelSquared;

                        added |= R_DlightSample(src, vec, dst);
                        src += 3;
                        dst += 4;

						vec[0] += srf->drawinfo.lmapStep;
					}
				}

                src += 3 * (LIGHTMAP_SIZE - 1 - (8 * lumelsPerHeight));
				dst += 4 * (LIGHTMAP_SIZE - 1 - (8 * lumelsPerHeight));
				vec[1] += srf->drawinfo.lmapStep;
			}
		}
	}

    if (!added) {
        srf->drawinfo.dlightMap[tr.smpFrame] = 0;
        return 0;
	}

	for (i = 0; i < srf->drawinfo.lmapSize; i++) {
		dli.allocated[x + i] = srf->drawinfo.lmapSize + y;
	}

	lmScale = (1.f / LIGHTMAP_SIZE) / srf->drawinfo.lmapStep;
	srf->drawinfo.lmapX -= (srf->x0 * lmScale - (LIGHTMAP_SIZE * 2.f));
	srf->drawinfo.lmapY -= (srf->y0 * lmScale - (LIGHTMAP_SIZE * 2.f));

	srf->drawinfo.dlightMap[tr.smpFrame] = dli.dlightMap + 1;
    return srf->drawinfo.dlightMap[tr.smpFrame];
}

/*
===============
R_TransformDlights

Transforms the origins of an array of dlights.
Used by both the front end (for DlightBmodel) and
the back end (before doing the lighting calculation)
===============
*/
void R_TransformDlights( int count, dlight_t *dl, orientationr_t *ori) {
	int		i;
	vec3_t	temp;

	for ( i = 0 ; i < count ; i++, dl++ ) {
		VectorSubtract( dl->origin, ori->origin, temp );
		dl->transformed[0] = DotProduct( temp, ori->axis[0] );
		dl->transformed[1] = DotProduct( temp, ori->axis[1] );
		dl->transformed[2] = DotProduct( temp, ori->axis[2] );
	}
}

/*
=============
R_DlightBmodel

Determine which dynamic lights may effect this bmodel
=============
*/
void R_DlightBmodel( bmodel_t *bmodel ) {
	int			i, j;
	dlight_t	*dl;
	int			mask;
	msurface_t	*surf;

	// transform all the lights
	R_TransformDlights( tr.refdef.num_dlights, tr.refdef.dlights, &tr.ori );

	mask = 0;
	for ( i=0 ; i<tr.refdef.num_dlights ; i++ ) {
		dl = &tr.refdef.dlights[i];

		// see if the point is close enough to the bounds to matter
		for ( j = 0 ; j < 3 ; j++ ) {
			if ( dl->transformed[j] - bmodel->bounds[1][j] > dl->radius ) {
				break;
			}
			if ( bmodel->bounds[0][j] - dl->transformed[j] > dl->radius ) {
				break;
			}
		}
		if ( j < 3 ) {
			continue;
		}

		// we need to check this light
		mask |= 1 << i;
	}

	tr.currentEntity->needDlights = (mask != 0);

	// set the dlight bits in all the surfaces
	for ( i = 0 ; i < bmodel->numSurfaces ; i++ ) {
		surf = bmodel->firstSurface + i;

		if ( *surf->data == SF_FACE ) {
			((srfSurfaceFace_t *)surf->data)->dlightBits[ tr.smpFrame ] = mask;
		} else if ( *surf->data == SF_GRID ) {
			((srfGridMesh_t *)surf->data)->dlightBits[ tr.smpFrame ] = mask;
		} else if ( *surf->data == SF_TRIANGLES ) {
			((srfTriangles_t *)surf->data)->dlightBits[ tr.smpFrame ] = mask;
		}
	}
}

static byte* R_GetLightGridPalettedColor(int iColor)
{
	// FIXME: unimplemented
}

void R_GetLightingGridValue(const vec3_t vPos, vec3_t vLight)
{
	if (!tr.world || !tr.world->lightGridData || !tr.world->lightGridOffsets) {
		vLight[0] = vLight[1] = vLight[2] = tr.identityLightByte;
	}

	vLight[0] = vLight[1] = vLight[2] = tr.identityLightByte;
	// FIXME: unimplemented
}

void R_GetLightingGridValueFast(const vec3_t vPos, vec3_t vLight)
{
    // FIXME: unimplemented
    VectorSet(vLight, 1.f, 1.f, 1.f);
}

void R_GetLightingForDecal(vec3_t vLight, const vec3_t vFacing, const vec3_t vOrigin)
{
    // FIXME: unimplemented
    VectorSet(vLight, 1.f, 1.f, 1.f);
}

void R_GetLightingForSmoke(vec3_t vLight, const vec3_t vOrigin)
{
    // FIXME: unimplemented
	VectorSet(vLight, 1.f, 1.f, 1.f);
}

static int RB_GetEntityGridLighting()
{
	int iColor;
	int i;
	dlight_t* dl;
	float power;
	vec3_t vLight;
	vec3_t dir;
	float d;
	float* lightOrigin;

	lightOrigin = backEnd.currentSphere->traceOrigin;
	if (!(backEnd.refdef.rdflags & RDF_NOWORLDMODEL) && tr.world->lightGridData) {
		R_GetLightingGridValue(backEnd.currentSphere->traceOrigin, vLight);
	} else {
		vLight[0] = vLight[1] = vLight[2] = tr.identityLight * 150.0;
	}

	for (i = 0; i < backEnd.refdef.num_dlights; i++) {
		dl = &backEnd.refdef.dlights[i];
		VectorSubtract(dl->origin, lightOrigin, dir);
		d = VectorLengthSquared(dir);

		power = dl->radius * dl->radius;
		if (power >= d) {
			d = dl->radius * 7500.0 / d;
			VectorMA(vLight, d, dl->color, vLight);
		}
	}

	if (tr.overbrightShift)
	{
		vLight[0] = tr.overbrightMult * vLight[0];
		vLight[1] = tr.overbrightMult * vLight[1];
		vLight[2] = tr.overbrightMult * vLight[2];
	}

	// normalize
	if (vLight[0] < 255.0 || vLight[1] < 255.0 || vLight[2] < 255.0) {
		float scale = 255.0 / fmin(vLight[0], fmin(vLight[1], vLight[2]));
		VectorScale(vLight, scale, vLight);
	}

	// clamp ambient
	for (i = 0; i < 3; i++) {
		if (vLight[i] > tr.identityLightByte) {
			vLight[i] = tr.identityLightByte;
		}
	}

	// save out the byte packet version
	((byte*)&iColor)[0] = myftol(vLight[0]);
	((byte*)&iColor)[1] = myftol(vLight[1]);
	((byte*)&iColor)[2] = myftol(vLight[2]);
	((byte*)&iColor)[3] = 0xff;

	return iColor;

#if 0
	int				i;
	dlight_t* dl;
	float			power;
	vec3_t			dir;
	float			d;
	vec3_t			lightDir;
	vec3_t			lightOrigin;
	int				ambientlightInt = 0;
	trRefEntity_t	*ent = backEnd.currentEntity;
	trRefdef_t		*refdef = &backEnd.refdef;

	//
	// trace a sample point down to find ambient light
	//
	if (ent->e.renderfx & RF_LIGHTING_ORIGIN) {
		// seperate lightOrigins are needed so an object that is
		// sinking into the ground can still be lit, and so
		// multi-part models can be lit identically
		VectorCopy(ent->e.lightingOrigin, lightOrigin);
	}
	else {
		VectorCopy(ent->e.origin, lightOrigin);
	}

	// if NOWORLDMODEL, only use dynamic lights (menu system, etc)
	if (!(refdef->rdflags & RDF_NOWORLDMODEL)
		&& tr.world->lightGridData) {
		R_SetupEntityLightingGrid(ent);
	}
	else {
		ent->ambientLight[0] = ent->ambientLight[1] =
			ent->ambientLight[2] = tr.identityLight * 150;
		ent->directedLight[0] = ent->directedLight[1] =
			ent->directedLight[2] = tr.identityLight * 150;
		VectorCopy(tr.sunDirection, ent->lightDir);
	}

	// bonus items and view weapons have a fixed minimum add
	if (1 /* ent->e.renderfx & RF_MINLIGHT */) {
		// give everything a minimum light add
		ent->ambientLight[0] += tr.identityLight * 32;
		ent->ambientLight[1] += tr.identityLight * 32;
		ent->ambientLight[2] += tr.identityLight * 32;
	}

	//
	// modify the light by dynamic lights
	//
	d = VectorLength(ent->directedLight);
	VectorScale(ent->lightDir, d, lightDir);

	for (i = 0; i < refdef->num_dlights; i++) {
		dl = &refdef->dlights[i];
		VectorSubtract(dl->origin, lightOrigin, dir);
		d = VectorNormalize(dir);

		power = DLIGHT_AT_RADIUS * (dl->radius * dl->radius);
		if (d < DLIGHT_MINIMUM_RADIUS) {
			d = DLIGHT_MINIMUM_RADIUS;
		}
		d = power / (d * d);

		VectorMA(ent->directedLight, d, dl->color, ent->directedLight);
		VectorMA(lightDir, d, dir, lightDir);
	}

	// clamp ambient
	for (i = 0; i < 3; i++) {
		if (ent->ambientLight[i] > tr.identityLightByte) {
			ent->ambientLight[i] = tr.identityLightByte;
		}
	}

	// save out the byte packet version
	((byte*)&ambientlightInt)[0] = myftol(ent->ambientLight[0]);
	((byte*)&ambientlightInt)[1] = myftol(ent->ambientLight[1]);
	((byte*)&ambientlightInt)[2] = myftol(ent->ambientLight[2]);
	((byte*)&ambientlightInt)[3] = 0xff;

	// transform the direction to local space
	VectorNormalize(lightDir);
	ent->lightDir[0] = DotProduct(lightDir, ent->e.axis[0]);
	ent->lightDir[1] = DotProduct(lightDir, ent->e.axis[1]);
	ent->lightDir[2] = DotProduct(lightDir, ent->e.axis[2]);

	return ambientlightInt;
#endif
	// FIXME: unimplemented
}

void RB_SetupEntityGridLighting()
{
	trRefEntity_t* ent;
	int iColor;

	if (backEnd.currentEntity->bLightGridCalculated) {
		return;
	}

	for (ent = backEnd.currentEntity; ent->e.parentEntity != ENTITYNUM_NONE; ent = &backEnd.refdef.entities[ent->e.parentEntity])
	{
		trRefEntity_t* newref = &backEnd.refdef.entities[ent->e.parentEntity];
		if (newref == ent) {
			assert(!"backEnd.refdef.entities[ent->e.parentEntity] refers to itself\n");
			iColor = newref->iGridLighting;
			break;
		}

		if (newref->bLightGridCalculated) {
			iColor = newref->iGridLighting;
			break;
		}
	}

	if (ent->e.parentEntity == ENTITYNUM_NONE) {
		iColor = RB_GetEntityGridLighting();
	}

	ent = backEnd.currentEntity;
	for (;;)
	{
		ent->bLightGridCalculated = 1;
		ent->iGridLighting = iColor;
		if (ent->e.parentEntity == ENTITYNUM_NONE) {
			break;
		}

		if (ent == &backEnd.refdef.entities[ent->e.parentEntity]) {
			assert(!"backEnd.refdef.entities[ent->e.parentEntity] refers to itself\n");
			break;
		}

		ent = &backEnd.refdef.entities[ent->e.parentEntity];
	}
}

void RB_SetupStaticModelGridLighting(trRefdef_t* refdef, cStaticModelUnpacked_t* ent, const vec3_t lightOrigin)
{
	int iColor;
	int i;
	dlight_t* dl;
	float power;
	vec3_t vLight;
	vec3_t dir;
	float d;

	if (ent->bLightGridCalculated) {
		return;
	}
	ent->bLightGridCalculated = qtrue;

	if (!(refdef->rdflags & RDF_NOWORLDMODEL) && tr.world->lightGridData) {
		R_GetLightingGridValue(backEnd.currentSphere->traceOrigin, vLight);
	}
	else {
		vLight[0] = vLight[1] = vLight[2] = tr.identityLight * 150.0;
	}

	for (i = 0; i < refdef->num_dlights; i++) {
		dl = &refdef->dlights[i];
		VectorSubtract(dl->origin, lightOrigin, dir);
		d = VectorLengthSquared(dir);

		power = dl->radius * dl->radius;
		if (power >= d) {
			d = dl->radius * 7500.0 / d;
			VectorMA(vLight, d, dl->color, vLight);
		}
	}

	if (tr.overbrightShift)
	{
		vLight[0] = tr.overbrightMult * vLight[0];
		vLight[1] = tr.overbrightMult * vLight[1];
		vLight[2] = tr.overbrightMult * vLight[2];
	}

	// normalize
	if (vLight[0] < 255.0 || vLight[1] < 255.0 || vLight[2] < 255.0) {
		float scale = 255.0 / fmin(vLight[0], fmin(vLight[1], vLight[2]));
		VectorScale(vLight, scale, vLight);
	}

	// clamp ambient
	for (i = 0; i < 3; i++) {
		if (vLight[i] > tr.identityLightByte) {
			vLight[i] = tr.identityLightByte;
		}
	}

	// save out the byte packet version
	((byte*)&ent->iGridLighting)[0] = myftol(vLight[0]);
	((byte*)&ent->iGridLighting)[1] = myftol(vLight[1]);
	((byte*)&ent->iGridLighting)[2] = myftol(vLight[2]);
	((byte*)&ent->iGridLighting)[3] = 0xff;
}

/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/

extern	cvar_t	*r_ambientScale;
extern	cvar_t	*r_directedScale;

/*
===============
LogLight
===============
*/
static void LogLight( trRefEntity_t *ent ) {
	int	max1, max2;

	if ( !(ent->e.renderfx & RF_FIRST_PERSON ) ) {
		return;
	}

	max1 = ent->ambientLight[0];
	if ( ent->ambientLight[1] > max1 ) {
		max1 = ent->ambientLight[1];
	} else if ( ent->ambientLight[2] > max1 ) {
		max1 = ent->ambientLight[2];
	}

	max2 = ent->directedLight[0];
	if ( ent->directedLight[1] > max2 ) {
		max2 = ent->directedLight[1];
	} else if ( ent->directedLight[2] > max2 ) {
		max2 = ent->directedLight[2];
	}

	ri.Printf( PRINT_ALL, "amb:%i  dir:%i\n", max1, max2 );
}

void R_ClearRealDlights() {
	memset(dli.allocated, 0, sizeof(dli.allocated));
	dli.dlightMap = 0;
}

void R_UploadDlights() {
	int i, h;

	if (!tr.pc.c_dlightSurfaces) {
		return;
	}

	h = 0;
	for (i = 0; i < LIGHTMAP_SIZE; i++)
	{
		if (h < dli.allocated[i]) {
			h = dli.allocated[i];
		}
	}

	if (h)
	{
		if (h > LIGHTMAP_SIZE) {
			ri.Error(ERR_DROP, "R_UploadDlights: bad allocated height");
		}

		GL_Bind(tr.dlightImages[dli.dlightMap]);
		qglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, LIGHTMAP_SIZE, h, GL_RGBA, GL_UNSIGNED_BYTE, dli.lightmap_buffer);

		tr.pc.c_dlightMaps++;
		memset(dli.allocated, 0, sizeof(dli.allocated));
	}
}

qboolean R_AllocLMBlock(int w, int h, int* x, int* y) {
	int i, j;
	int best, best2;

	for (;;)
	{
		best = LIGHTMAP_SIZE;
		for (i = 0; i < LIGHTMAP_SIZE - w; i++)
		{
			best2 = 0;

			for (j = 0; j < w && dli.allocated[i] < best; j++)
			{
				if (best2 < dli.allocated[i]) {
					best2 = dli.allocated[i];
				}
			}

			if (j == w)
			{
				*x = i;
				*y = best2;
				best = best2;
			}
		}

		if (h + best <= LIGHTMAP_SIZE) {
			break;
		}

		if (dli.dlightMap == 14) {
			return qfalse;
		}

		R_UploadDlights();
		dli.dlightMap++;
	}

	return qtrue;
}

qboolean R_DlightSample(byte* src, const vec3_t vec, byte* dst) {
	int r, g, b;
	int k;
	qboolean added;
	vec3_t dir;
	float add;

	added = qfalse;
	r = src[0];
	g = src[1];
	b = src[2];

	for (k = 0; k < dli.numLights; k++) {
		incidentLight_t* light = &dli.lights[k];

		VectorSubtract(vec, light->origin, dir);
		add = VectorLength(dir) * light->power;
		if (add <= 1.f) {
			float t = (1.0 - add) * (1.0 - add) * 375.0;

			r = (int)(t * light->dl->color[0] + (float)r);
			g = (int)(t * light->dl->color[1] + (float)g);
			b = (int)(t * light->dl->color[2] + (float)b);
			// light was added
			added = qtrue;
		}
    }

	if (tr.overbrightShift) {
		r <<= tr.overbrightShift & 0xFF;
		g <<= tr.overbrightShift & 0xFF;
		b <<= tr.overbrightShift & 0xFF;
	}

	if (r > 0xFF || g > 0xFF || b > 0xFF) {
        float t;
		// normalize color values
        t = 255.0 / (float)Q_max(r, Q_max(g, b));

        r = (int)((float)r * t);
        g = (int)((float)g * t);
        b = (int)((float)b * t);
    }

	dst[0] = (byte)r;
	dst[1] = (byte)g;
	dst[2] = (byte)b;
	dst[3] = -1;

	return added;
}
