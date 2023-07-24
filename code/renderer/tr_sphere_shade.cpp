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


static vec3_t offsets[26] =
{
	{ 1.0, 0.0, 0.0 },
	{ -1.0, 0.0, 0.0 },
	{ 0.0, 1.0, 0.0 },
	{ 0.0, -1.0, 0.0 },
	{ 0.0, 0.0, 1.0 },
	{ 0.0, 0.0, -1.0 },
	{ sqrt(0.5), sqrt(0.5), 0.0},
	{ sqrt(0.5), -sqrt(0.5), 0.0 },
	{ -sqrt(0.5), sqrt(0.5), 0.0 },
	{ -sqrt(0.5), -sqrt(0.5), 0.0 },
	{ sqrt(0.5), 0.0, sqrt(0.5) },
	{ sqrt(0.5), 0.0, -sqrt(0.5) },
	{ -sqrt(0.5), 0.0, sqrt(0.5) },
	{ -sqrt(0.5), 0.0, -sqrt(0.5) },
	{ 0.0, sqrt(0.5), sqrt(0.5) },
	{ 0.0, sqrt(0.5), -sqrt(0.5) },
	{ 0.0, -sqrt(0.5), sqrt(0.5) },
	{ 0.0, -sqrt(0.5), -sqrt(0.5) },
	{ sqrt(1.0 / 3.0), sqrt(1.0 / 3.0), sqrt(1.0 / 3.0) },
	{ sqrt(1.0 / 3.0), sqrt(1.0 / 3.0), -sqrt(1.0 / 3.0) },
	{ sqrt(1.0 / 3.0), -sqrt(1.0 / 3.0), sqrt(1.0 / 3.0) },
	{ sqrt(1.0 / 3.0), -sqrt(1.0 / 3.0), -sqrt(1.0 / 3.0) },
	{ -sqrt(1.0 / 3.0), sqrt(1.0 / 3.0), sqrt(1.0 / 3.0) },
	{ -sqrt(1.0 / 3.0), sqrt(1.0 / 3.0), -sqrt(1.0 / 3.0) },
	{ -sqrt(1.0 / 3.0), -sqrt(1.0 / 3.0), sqrt(1.0 / 3.0) },
	{ -sqrt(1.0 / 3.0), -sqrt(1.0 / 3.0), -sqrt(1.0 / 3.0) }
};

int compare_light_intensities(const void* p1, const void* p2)
{
	return ((const reallightinfo_t*)p2)->fIntensity - ((const reallightinfo_t*)p1)->fIntensity;
}

static void RB_OptimizeLights()
{
	static int iCubeBuildInfo[24][9];
	int i, j;
	int nCubeMapLights;
	float fMaxIntensity;
	vec3_t cubecolor[256];
	vec3_t colorout;
	float(*cube)[3][4];
	reallightinfo_t* pLight;

	nCubeMapLights = 0;

	if (backEnd.currentSphere->numRealLights > 2)
	{
		qsort(
			backEnd.currentSphere->light,
			backEnd.currentSphere->numRealLights,
			sizeof(reallightinfo_t),
			compare_light_intensities
        );

        for (i = 0; i < backEnd.currentSphere->numRealLights && i < r_entlight_maxcalc->integer; i++) {
            pLight = &backEnd.currentSphere->light[i];

            if (pLight->fIntensity < r_entlight_cubelevel->value
                || pLight->fIntensity * r_entlight_cubefraction->value > pLight->fIntensity)
            {
				break;
            }
        }

        nCubeMapLights = backEnd.currentSphere->numRealLights - i;
		if (nCubeMapLights <= 1) {
			nCubeMapLights = 0;
		}
	}

	backEnd.currentSphere->bUsesCubeMap = nCubeMapLights > 0;
	if (backEnd.currentSphere->bUsesCubeMap)
	{
		for (i = 0; i < 26; i++) {
			float fDot;
			vec3_t v;

			colorout[0] = backEnd.currentSphere->ambient.level[0];
			colorout[1] = backEnd.currentSphere->ambient.level[1];
			colorout[2] = backEnd.currentSphere->ambient.level[2];
		
			for (j = backEnd.currentSphere->numRealLights - nCubeMapLights; j < backEnd.currentSphere->numRealLights; j++) {
				pLight = &backEnd.currentSphere->light[j];

				switch (pLight->eType)
				{
                case LIGHT_SPOT:
					{
						fDot = DotProduct(pLight->vDirection, offsets[i]);
                        if (fDot > 0) {
                            float fProjSquared;
                            float fDistSquared;

							VectorSubtract(pLight->vOrigin, backEnd.currentSphere->origin, v);
							fProjSquared = DotProduct(v, pLight->vDirection) * DotProduct(v, pLight->vDirection);
							fDistSquared = VectorLengthSquared(v);
							fMaxIntensity = (pLight->fSpotConst - fDistSquared / fProjSquared) * pLight->fSpotScale;
							if (fMaxIntensity > 0) {
								fDot /= fDistSquared;
								if (fMaxIntensity < 1) {
									fDot *= fMaxIntensity;
								}

								VectorMA(colorout, fDot, pLight->color, colorout);
							}
						}
					}
					break;
				case LIGHT_SPOT_FAST:
					fDot = DotProduct(pLight->vDirection, offsets[i]);
                    if (fDot > 0) {
                        float fDistSquared;

						VectorSubtract(pLight->vOrigin, backEnd.currentSphere->origin, v);
                        fDistSquared = VectorLengthSquared(v);

                        fDot /= fDistSquared;
						VectorMA(colorout, fDot, pLight->color, colorout);
					}
					break;
				case LIGHT_DIRECTIONAL:
					fDot = DotProduct(pLight->vDirection, offsets[i]);
					if (fDot > 0) {
						VectorMA(colorout, fDot, pLight->color, colorout);
					}
					break;
                case LIGHT_POINT:
					VectorSubtract(pLight->vOrigin, backEnd.currentSphere->origin, v);
					fDot = DotProduct(v, offsets[i]);
					if (fDot > 0) {
						fDot /= VectorLengthSquared(v);
						VectorMA(colorout, fDot, pLight->color, colorout);
					}
					break;
				default:
					assert(!"unhandled light type");
					break;
				}
            }

			cubecolor[i][0] = colorout[0];
			if (cubecolor[i][0] > 255.f) {
				cubecolor[i][0] = 255.f;
            }

            cubecolor[i][1] = colorout[1];
            if (cubecolor[i][1] > 255.f) {
                cubecolor[i][1] = 255.f;
            }

            cubecolor[i][2] = colorout[2];
            if (cubecolor[i][2] > 255.f) {
                cubecolor[i][2] = 255.f;
            }
		}

		cube = backEnd.currentSphere->cubemap;

		for (i = 0; i < 24; i++) {
			for (j = 0; j < 3; j++) {
				vec4_t f;

                f[0] = cubecolor[iCubeBuildInfo[i][5]][j];
                f[1] = cubecolor[iCubeBuildInfo[i][6]][j];
				f[2] = cubecolor[iCubeBuildInfo[i][7]][j];
				f[3] = cubecolor[iCubeBuildInfo[i][8]][j];

                cube[i][j][0] = f[0];
                cube[i][j][1] = (f[1] - f[0]) * iCubeBuildInfo[i][3];
                cube[i][j][2] = (f[2] - f[0]) * iCubeBuildInfo[i][4];
                cube[i][j][3] = (f[0] - f[1] - f[2] + f[3]) * iCubeBuildInfo[i][3] * iCubeBuildInfo[i][4];
			}
		}

		backEnd.currentSphere->numRealLights -= nCubeMapLights;
	}

	for (i = 0; i < backEnd.currentSphere->numRealLights; i++) {
		float fDistParallel;
		float fDistPerp;
		float fDistSquared;
		float fRadiusSquared;
		float fSlope;
		vec3_t v;

		pLight = &backEnd.currentSphere->light[i];

		switch (pLight->eType)
		{
		case LIGHT_SPOT:
			VectorSubtract(pLight->vOrigin, backEnd.currentSphere->origin, v);
			fDistSquared = VectorLengthSquared(v);
			fSlope = DotProduct(v, pLight->vDirection);
			fDistPerp = sqrt(fDistSquared - fSlope * fSlope);
			fRadiusSquared = backEnd.currentSphere->radius * backEnd.currentSphere->radius;
			fDistParallel = fDistPerp + fSlope * (pLight->fSpotSlope * 0.9f);
			if (fDistParallel * fDistParallel >= fRadiusSquared + pLight->fSpotSlope * 0.9f * (pLight->fSpotSlope * 0.9f) * fRadiusSquared) {
				pLight->eType = LIGHT_SPOT_FAST;
			}
			break;
		case LIGHT_POINT:
			if ((pLight->fDist - backEnd.currentSphere->radius) * r_entlight_errbound->value * pLight->fDist > backEnd.currentSphere->radius * pLight->fIntensity) {
				pLight->color[0] *= 1.f / pLight->fDist;
				pLight->color[1] *= 1.f / pLight->fDist;
				pLight->color[2] *= 1.f / pLight->fDist;
				pLight->eType = LIGHT_DIRECTIONAL;
				VectorNormalize2(pLight->vOrigin, pLight->vDirection);
			}
			break;
		}
	}
}

static void RB_Light_CubeMap(unsigned char* colors)
{
	int i, j;
    float* normal;
    float* xyz;
	unsigned char* color;
	vec3_t colorout;
	reallightinfo_t* pLight;

	color = colors;
    normal = (float*)tess.normal;
    xyz = (float*)tess.xyz;

    for (i = 0; i < tess.numVertexes; i++, xyz += 4, normal += 4, color += 4) {
		unsigned int components[3];

		if (normal[1] == -1) {
			components[0] = 1;
			components[1] = 0;
			components[2] = 2;
        } else if (normal[1] == 1 && !normal[0] && !normal[2]) {
			components[0] = 1;
			components[1] = 0;
			components[2] = 2;
		} else if (Q_fabs(normal[0]) >= Q_fabs(normal[2])) {
			components[0] = 0;
			components[1] = 1;
			components[2] = 2;
		} else {
			components[0] = 2;
			components[1] = 1;
			components[2] = 0;
		}

		unsigned int cubeIndex = (normal[2] < 0 ? 1 : 0) | (normal[1] < 0 ? 2 : 0) | (normal[0] < 0 ? 4 : 0) | (components[0] << 3);
		float newnorms[3];
		float mapping[3];
		float (*cube)[3][4];

		newnorms[0] = normal[components[0]];
		newnorms[1] = normal[components[1]];
		newnorms[2] = normal[components[2]];

		mapping[0] = 1.f / newnorms[0] * newnorms[1];
		mapping[1] = 1.f / newnorms[0] * newnorms[2];
        mapping[2] = mapping[0] * mapping[1];

		cube = &backEnd.currentSphere->cubemap[cubeIndex];
        colorout[0] = (*cube)[0][0] + (*cube)[0][1] * mapping[1] + (*cube)[0][2] * mapping[1] + (*cube)[0][3] * mapping[2];
		colorout[1] = (*cube)[1][0] + (*cube)[1][1] * mapping[1] + (*cube)[1][2] * mapping[1] + (*cube)[1][3] * mapping[2];
		colorout[2] = (*cube)[2][0] + (*cube)[2][1] * mapping[1] + (*cube)[2][2] * mapping[1] + (*cube)[2][3] * mapping[2];

		for (j = 0; j < backEnd.currentSphere->numRealLights; j++) {
            float fDistSquared;
            float fProjSquared;
            float fMaxIntensity;
			float fDot;
			vec3_t v;

            pLight = &backEnd.currentSphere->light[j];

            switch (pLight->eType)
            {
            case LIGHT_DIRECTIONAL:
                fDot = DotProduct(pLight->vDirection, normal);
                if (fDot > 0) {
                    VectorMA(colorout, fDot, pLight->color, colorout);
                }
                break;
            case LIGHT_SPOT:
                fDot = DotProduct(pLight->vDirection, normal);
                if (fDot > 0) {
                    VectorSubtract(pLight->vOrigin, xyz, v);

                    fProjSquared = DotProduct(v, pLight->vDirection) * DotProduct(v, pLight->vDirection);
                    fDistSquared = VectorLengthSquared(v);
                    fMaxIntensity = (pLight->fSpotConst - fDistSquared / fProjSquared) * pLight->fSpotScale;
                    if (fMaxIntensity > 0) {
                        fDot /= fDistSquared;
                        if (fMaxIntensity < 1) {
                            fDot *= fMaxIntensity;
                        }

                        VectorMA(colorout, fDot, pLight->color, colorout);
                    }
                }
                break;
            case LIGHT_SPOT_FAST:
                fDot = DotProduct(pLight->vDirection, normal);
                if (fDot > 0) {
                    VectorSubtract(pLight->vOrigin, xyz, v);
                    fDistSquared = VectorLengthSquared(v);
                    VectorMA(colorout, fDot / fDistSquared, pLight->color, colorout);
                }
                break;
            case LIGHT_POINT:
                VectorSubtract(pLight->vOrigin, xyz, v);
                fDot = DotProduct(v, normal);
                if (fDot > 0) {
                    fDistSquared = VectorLengthSquared(v);
                    VectorMA(colorout, fDot / fDistSquared, pLight->color, colorout);
                }
                break;
            }
		}

		color[0] = Q_clamp_int(colorout[0], 0, 255);
		color[1] = Q_clamp_int(colorout[1], 0, 255);
		color[2] = Q_clamp_int(colorout[2], 0, 255);
	}
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

	color = colors;
    if (backEnd.currentSphere->bUsesCubeMap) {
        RB_Light_CubeMap(colors);
        return;
    }

	if (!backEnd.currentSphere->numRealLights) {
		for (i = 0; i < tess.numVertexes; i++, color += 4) {
            color[0] = backEnd.currentSphere->ambient.level[0];
            color[1] = backEnd.currentSphere->ambient.level[1];
            color[2] = backEnd.currentSphere->ambient.level[2];
            color[3] = backEnd.currentSphere->ambient.level[3];
		}
		return;
	}

	if (backEnd.currentSphere->numRealLights != 1) {
		normal = (float*)tess.normal;
		xyz = (float*)tess.xyz;

		for (i = 0; i < tess.numVertexes; i++, xyz += 4, normal += 4, color += 4) {
			colorout[0] = colorout[1] = colorout[2] = 0;
			
			for (j = 0; j < backEnd.currentSphere->numRealLights; j++) {
				float fDistSquared;
				float fProjSquared;
				float fMaxIntensity;

				pLight = &backEnd.currentSphere->light[j];
				
				switch (pLight->eType)
				{
				case LIGHT_DIRECTIONAL:
					fDot = DotProduct(pLight->vDirection, normal);
					if (fDot > 0) {
						VectorMA(colorout, fDot, pLight->color, colorout);
					}
                    break;
                case LIGHT_SPOT:
                    fDot = DotProduct(pLight->vDirection, normal);
                    if (fDot > 0) {
                        VectorSubtract(pLight->vOrigin, xyz, v);

                        fProjSquared = DotProduct(v, pLight->vDirection) * DotProduct(v, pLight->vDirection);
                        fDistSquared = VectorLengthSquared(v);
                        fMaxIntensity = (pLight->fSpotConst - fDistSquared / fProjSquared) * pLight->fSpotScale;
                        if (fMaxIntensity > 0) {
                            fDot /= fDistSquared;
                            if (fMaxIntensity < 1) {
                                fDot *= fMaxIntensity;
                            }

                            VectorMA(colorout, fDot, pLight->color, colorout);
                        }
                    }
                    break;
                case LIGHT_SPOT_FAST:
					fDot = DotProduct(pLight->vDirection, normal);
                    if (fDot > 0) {
                        VectorSubtract(pLight->vOrigin, xyz, v);
                        fDistSquared = VectorLengthSquared(v);
						VectorMA(colorout, fDot / fDistSquared, pLight->color, colorout);
					}
                    break;
                case LIGHT_POINT:
					VectorSubtract(pLight->vOrigin, xyz, v);
					fDot = DotProduct(v, normal);
					if (fDot > 0) {
						fDistSquared = VectorLengthSquared(v);
						VectorMA(colorout, fDot / fDistSquared, pLight->color, colorout);
					}
                    break;
				}
			}

			color[0] = Q_clamp_int((int)colorout[0] + backEnd.currentSphere->ambient.level[0], 0, 255);
			color[1] = Q_clamp_int((int)colorout[1] + backEnd.currentSphere->ambient.level[1], 0, 255);
			color[2] = Q_clamp_int((int)colorout[2] + backEnd.currentSphere->ambient.level[2], 0, 255);
			color[3] = 0xff;
		}
	} else {
		normal = (float*)tess.normal;
		xyz = (float*)tess.xyz;

        pLight = &backEnd.currentSphere->light[0];

		switch (backEnd.currentSphere->light[0].eType)
		{
		case LIGHT_DIRECTIONAL:
			for (i = 0; i < tess.numVertexes; i++, xyz += 4, normal += 4, color += 4) {
                fDot = DotProduct(pLight->vDirection, normal);
                if (fDot > 0) {
					VectorScale(pLight->color, fDot, colorout);

                    color[0] = Q_clamp_int((int)colorout[0] + backEnd.currentSphere->ambient.level[0], 0, 255);
                    color[1] = Q_clamp_int((int)colorout[1] + backEnd.currentSphere->ambient.level[1], 0, 255);
                    color[2] = Q_clamp_int((int)colorout[2] + backEnd.currentSphere->ambient.level[2], 0, 255);
					color[3] = 0xff;
                } else {
                    color[0] = backEnd.currentSphere->ambient.level[0];
                    color[1] = backEnd.currentSphere->ambient.level[1];
                    color[2] = backEnd.currentSphere->ambient.level[2];
                    color[3] = backEnd.currentSphere->ambient.level[3];
				}
			}
			break;
        case LIGHT_SPOT:
			for (i = 0; i < tess.numVertexes; i++, xyz += 4, normal += 4, color += 4) {
                float fDistSquared;
                float fProjSquared;
                float fMaxIntensity;

				color[0] = backEnd.currentSphere->ambient.level[0];
				color[1] = backEnd.currentSphere->ambient.level[1];
				color[2] = backEnd.currentSphere->ambient.level[2];
				color[3] = backEnd.currentSphere->ambient.level[3];
				
				fDot = DotProduct(pLight->vDirection, normal);
                if (fDot > 0) {
                    VectorSubtract(pLight->vOrigin, xyz, v);

                    fProjSquared = DotProduct(v, pLight->vDirection) * DotProduct(v, pLight->vDirection);
                    fDistSquared = VectorLengthSquared(v);
                    fMaxIntensity = (pLight->fSpotConst - fDistSquared / fProjSquared) * pLight->fSpotScale;
                    if (fMaxIntensity > 0) {
                        fDot /= fDistSquared;
                        if (fMaxIntensity < 1) {
                            fDot *= fMaxIntensity;
                        }

                        VectorScale(pLight->color, fDot, colorout);
                        
						color[0] = Q_clamp_int((int)colorout[0] + backEnd.currentSphere->ambient.level[0], 0, 255);
                        color[1] = Q_clamp_int((int)colorout[1] + backEnd.currentSphere->ambient.level[1], 0, 255);
                        color[2] = Q_clamp_int((int)colorout[2] + backEnd.currentSphere->ambient.level[2], 0, 255);
                        color[3] = 0xff;
					}
                }
			}
			break;
        case LIGHT_SPOT_FAST:
            for (i = 0; i < tess.numVertexes; i++, xyz += 4, normal += 4, color += 4) {
                color[0] = backEnd.currentSphere->ambient.level[0];
                color[1] = backEnd.currentSphere->ambient.level[1];
                color[2] = backEnd.currentSphere->ambient.level[2];
                color[3] = backEnd.currentSphere->ambient.level[3];
				
				fDot = DotProduct(pLight->vDirection, normal);
                if (fDot > 0) {
					float fDistSquared;

                    VectorSubtract(pLight->vOrigin, xyz, v);
                    fDistSquared = VectorLengthSquared(v);
                    VectorScale(pLight->color, fDot / fDistSquared, colorout);

                    color[0] = Q_clamp_int((int)colorout[0] + backEnd.currentSphere->ambient.level[0], 0, 255);
                    color[1] = Q_clamp_int((int)colorout[1] + backEnd.currentSphere->ambient.level[1], 0, 255);
                    color[2] = Q_clamp_int((int)colorout[2] + backEnd.currentSphere->ambient.level[2], 0, 255);
                    color[3] = 0xff;
                }
            }
			break;
        case LIGHT_POINT:
			for (i = 0; i < tess.numVertexes; i++, xyz += 4, normal += 4, color += 4) {
				VectorSubtract(pLight->vOrigin, xyz, v);
				fDot = DotProduct(v, normal);
				if (fDot > 0) {
					float fDistSquared;

                    fDistSquared = VectorLengthSquared(v);
                    VectorScale(pLight->color, fDot / fDistSquared, colorout);

                    color[0] = Q_clamp_int((int)colorout[0] + backEnd.currentSphere->ambient.level[0], 0, 255);
                    color[1] = Q_clamp_int((int)colorout[1] + backEnd.currentSphere->ambient.level[1], 0, 255);
                    color[2] = Q_clamp_int((int)colorout[2] + backEnd.currentSphere->ambient.level[2], 0, 255);
                    color[3] = 0xff;
				} else {
					color[0] = backEnd.currentSphere->ambient.level[0];
					color[1] = backEnd.currentSphere->ambient.level[1];
					color[2] = backEnd.currentSphere->ambient.level[2];
					color[3] = backEnd.currentSphere->ambient.level[3];
				}
			}
			break;
		}
	}
}

static void RB_Sphere_Light_Sun()
{
	int curleaf;
	qboolean hitSun;
	vec3_t end;
	mnode_t* leaf;
	trace_t trace;

	if (!s_sun.exists) {
		return;
	}

	if (tr.world->vis && backEnd.currentSphere->leaves[0] && tr.sSunLight.leaf == (struct mnode_s*)-1) {
		vec3_t transpos;
		vec3_t temppos;

		for (curleaf = 0, leaf = backEnd.currentSphere->leaves[0]; curleaf < 8; curleaf++) {
			if (leaf->numlights && leaf->lights[0] == &tr.sSunLight) {
				break;
			}

			leaf = backEnd.currentSphere->leaves[curleaf];
			if (!leaf) {
				break;
			}
		}

		if (!leaf || (leaf->lights && leaf->lights[0] != &tr.sSunLight)) {
			if (r_light_sun_line->integer) {
				VectorMA(backEnd.currentSphere->worldOrigin, 16384.0, s_sun.direction, end);

				VectorCopy(backEnd.currentEntity->e.origin, temppos);
				MatrixTransformVectorRight(backEnd.currentEntity->e.axis, temppos, transpos);

				GL_State(GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE);

				qglPushMatrix();

				qglLoadMatrixf(backEnd.ori.modelMatrix);
				// Clear the depth range
				qglDepthRange(0.0, 0.0);

				qglDisable(GL_TEXTURE_2D);

				qglBegin(GL_LINES);
				qglColor3f(0.0, 0.0, 0.0);
				qglVertex3fv(backEnd.currentSphere->origin);
				qglColor3f(0.5, 0.5, 0.5);
				qglVertex3fv(transpos);
				qglEnd();

				qglEnable(GL_TEXTURE_2D);

				// Bring the depth range back
				qglDepthRange(0.0, 1.0);

				qglPopMatrix();
			}

			return;
		}
	}

	VectorMA(backEnd.currentSphere->traceOrigin, 16384.0, s_sun.direction, end);
	ri.CM_BoxTrace(
		&trace,
		backEnd.currentSphere->traceOrigin,
		end,
		vec3_origin,
		vec3_origin,
		0,
		CONTENTS_SOLID,
		0
	);

	hitSun = (trace.surfaceFlags >> 2) & 1;
    if (r_light_sun_line->integer) {
        vec3_t transpos;
        vec3_t temppos;

		VectorSubtract(trace.endpos, backEnd.currentEntity->e.origin, temppos);
        MatrixTransformVectorRight(backEnd.currentEntity->e.axis, temppos, transpos);

        GL_State(GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE);

        qglPushMatrix();

        qglLoadMatrixf(backEnd.ori.modelMatrix);
        // Clear the depth range
        qglDepthRange(0.0, 0.0);

        qglDisable(GL_TEXTURE_2D);

        qglBegin(GL_LINES);

		if (hitSun) {
			qglColor3f(1.0, 1.0, 0.0);
		} else {
			qglColor3f(0.0, 0.0, 0.0);
		}

        qglVertex3fv(backEnd.currentSphere->origin);
        qglColor3f(0.5, 0.5, 0.5);
        qglVertex3fv(transpos);
        qglEnd();

        qglEnable(GL_TEXTURE_2D);

        // Bring the depth range back
        qglDepthRange(0.0, 1.0);

        qglPopMatrix();
	}

	if (hitSun) {
		vec3_t sunnormal;
		sphereor_t* pSphere;
		reallightinfo_t* pLight;

		pSphere = backEnd.currentSphere;

        MatrixTransformVectorRight(backEnd.currentEntity->e.axis, s_sun.direction, sunnormal);
		
		pLight = &pSphere->light[pSphere->numRealLights];
		pLight->color[0] = s_sun.color[0] * tr.overbrightMult * r_entlight_scale->value;
		pLight->color[1] = s_sun.color[1] * tr.overbrightMult * r_entlight_scale->value;
		pLight->color[2] = s_sun.color[2] * tr.overbrightMult * r_entlight_scale->value;
	
		VectorCopy(sunnormal, pLight->vDirection);
		pLight->eType = LIGHT_DIRECTIONAL;
		pLight->fIntensity = pLight->color[0] * 0.299f + pLight->color[1] * 0.587f + pLight->color[2] * 0.114f;
		
		pSphere->numRealLights++;
	}
}

static qboolean RB_Sphere_CalculateSphereOrigin()
{
	vec3_t *axis;
	vec3_t sphereOrigin;
    trRefEntity_t* refent;
    trRefEntity_t* newref;

    axis = backEnd.currentEntity->e.axis;
	if (!backEnd.currentEntity->e.tiki) {
		return qfalse;
	}

	backEnd.currentSphere->radius = backEnd.currentEntity->e.radius;
	VectorCopy(backEnd.currentEntity->e.lightingOrigin, backEnd.currentSphere->worldOrigin);

	VectorSubtract(backEnd.currentSphere->worldOrigin, backEnd.currentEntity->e.origin, sphereOrigin);
	MatrixTransformVectorRight(axis, sphereOrigin, backEnd.currentSphere->origin);

    for (refent = backEnd.currentEntity; ; refent = newref) {
        if (refent->e.parentEntity == ENTITYNUM_NONE) {
            break;
        }

        newref = &backEnd.refdef.entities[refent->e.parentEntity];
        if (refent == newref) {
            break;
        }
    }

    VectorCopy(refent->e.lightingOrigin, backEnd.currentSphere->traceOrigin);

	return qtrue;
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

	amb[0] = light_offset[0] * tr.overbrightMult * r_entlight_scale->value;
	if (amb[0] > 255.0) {
		backEnd.currentSphere->ambient.level[0] = 0xff;
    } else {
        backEnd.currentSphere->ambient.level[0] = amb[0];
    }

    amb[1] = light_offset[1] * tr.overbrightMult * r_entlight_scale->value;
    if (amb[1] > 255.0) {
        backEnd.currentSphere->ambient.level[1] = 0xff;
    } else {
        backEnd.currentSphere->ambient.level[1] = amb[1];
    }

    amb[2] = light_offset[2] * tr.overbrightMult * r_entlight_scale->value;
    if (amb[2] > 255.0) {
        backEnd.currentSphere->ambient.level[2] = 0xff;
    } else {
        backEnd.currentSphere->ambient.level[2] = amb[2];
    }

	backEnd.currentSphere->ambient.level[3] = 0xff;

	return true;
}

static void RB_Sphere_DrawDebugLine(const spherel_t* thislight, float falloff, const vec3_t origin)
{
	int i;
	vec3_t newColor;
	vec3_t fakeLine, fakeLine2;

	if (!r_light_lines->integer) {
		return;
	}

    qglPushMatrix();

    qglLoadMatrixf(backEnd.ori.modelMatrix);
    // Clear the depth range
    qglDepthRange(0.0, 0.0);

    qglDisable(GL_TEXTURE_2D);

	if (r_light_lines->integer == 2)
	{
		newColor[0] = thislight->color[0] * (falloff * r_entlight_scale->value / 255.0);
		newColor[1] = thislight->color[1] * (falloff * r_entlight_scale->value / 255.0);
		newColor[2] = thislight->color[2] * (falloff * r_entlight_scale->value / 255.0);
	
		if (newColor[0] > 1 || newColor[1] > 1 || newColor[2] > 1) {
			NormalizeColor(newColor, newColor);
		}
    }
    else if (thislight->spot_light)
    {
        newColor[0] = 1.0;
        newColor[1] = 0.7;
        newColor[2] = 0.7;
    }
    else
    {
        newColor[0] = 1.0;
        newColor[1] = 1.0;
        newColor[2] = 1.0;
    }

    qglColor3f(newColor[0], newColor[1], newColor[2]);

    qglBegin(GL_LINES);
    qglVertex3fv(origin);
    qglVertex3fv(backEnd.currentSphere->origin);

	for (i = 0; i < 5; i += 2) {
		VectorMA(origin, 16.0, spheredef[i], fakeLine);
		VectorMA(origin, -16.0, spheredef[i], fakeLine2);

        qglVertex3fv(fakeLine);
        qglVertex3fv(fakeLine2);
	}

	if (thislight->spot_light) {
		qglColor3f(0.7, 1.0, 0.7);

		MatrixTransformVectorRight(backEnd.currentEntity->e.axis, thislight->spot_dir, fakeLine);
		VectorMA(origin, 32.0, fakeLine, fakeLine);

        qglVertex3fv(origin);
        qglVertex3fv(fakeLine);
	}

    qglEnd();
    qglEnable(GL_TEXTURE_2D);
	// Restore depth
    qglDepthRange(0.0, 1.0);
    qglPopMatrix();
}

static void RB_Sphere_AddSpotLight(const spherel_t* thislight)
{
	vec3_t lightorigin;
	vec3_t newlight;
	vec3_t lightline;
	vec3_t newdir;
	float falloff;
	float dot;
	float radiusByDistance;
	float radiusAtDist;
	float sampleRadius;
	vec3_t pointAtDist;
	
	radiusByDistance = thislight->spot_radiusbydistance;
	if (radiusByDistance < 0) {
		vec3_t proj;
		vec3_t delta;
		float distfromcentroid;

		dot = -(DotProduct(backEnd.currentSphere->worldOrigin, thislight->spot_dir) - DotProduct(thislight->origin, thislight->spot_dir));
		VectorMA(thislight->origin, dot, thislight->spot_dir, proj);
		VectorSubtract(proj, thislight->origin, delta);

		distfromcentroid = VectorLengthSquared(delta);
		if (distfromcentroid > radiusByDistance * radiusByDistance) {
			VectorScale(delta, -radiusByDistance / sqrt(distfromcentroid), delta);
		}

		VectorSubtract(thislight->origin, backEnd.currentEntity->e.origin, lightorigin);
		VectorAdd(lightorigin, delta, lightorigin);
    } else {
        VectorSubtract(thislight->origin, backEnd.currentEntity->e.origin, lightorigin);
	}

	MatrixTransformVectorRight(backEnd.currentEntity->e.axis, lightorigin, lightline);
	VectorSubtract(lightline, backEnd.currentSphere->origin, newlight);

	falloff = thislight->intensity * 7500.0 / VectorLengthSquared(newlight);
	if (falloff >= 5.0) {
		reallightinfo_t* pLight;
		float fMinDist;
		float fRadByDistSquared;

		MatrixTransformVectorRight(backEnd.currentEntity->e.axis, thislight->spot_dir, newdir);
		dot = DotProduct(newlight, newdir);
		if (dot > 0) {
			fMinDist = sqrt((newlight[0] + newdir[0] * dot) * (newlight[0] + newdir[0] * dot)
							+ (newlight[1] + newdir[1] * dot) * (newlight[1] + newdir[1] * dot)
							+ (newlight[2] + newdir[2] * dot) * (newlight[2] + newdir[2] * dot));

			if (fMinDist < radiusByDistance * dot) {
				if (backEnd.currentSphere->numRealLights < MAX_REAL_LIGHTS) {
                    pLight = &backEnd.currentSphere->light[backEnd.currentSphere->numRealLights];
                    pLight->eType = LIGHT_SPOT;
					pLight->color[0] *= thislight->intensity * 7500.0 * tr.overbrightMult;
					pLight->color[1] *= thislight->intensity * 7500.0 * tr.overbrightMult;
					pLight->color[2] *= thislight->intensity * 7500.0 * tr.overbrightMult;
					pLight->fDist = VectorLength(newlight);
					pLight->fIntensity = (pLight->color[0] * 0.299f + pLight->color[1] * 0.587f + pLight->color[2] * 0.114f) * (thislight->intensity * 7500.0 * tr.overbrightMult);

					sampleRadius = pLight->fDist - backEnd.currentSphere->radius;
					if (sampleRadius > 0) {
						pLight->fIntensity /= (sampleRadius * sampleRadius);
					}

					VectorCopy(lightline, pLight->vOrigin);
					VectorNegate(newdir, pLight->vDirection);
					fRadByDistSquared = radiusByDistance * radiusByDistance;
					pLight->fSpotConst = fRadByDistSquared + 1.0;
					pLight->fSpotScale = 1.0 / (fRadByDistSquared * 0.19);
					pLight->fSpotSlope = radiusByDistance;

					backEnd.currentSphere->numRealLights++;
				}

				RB_Sphere_DrawDebugLine(thislight, falloff, lightline);
			}
		}
	}
}

static void RB_Sphere_AddLight(const spherel_t* thislight)
{
	vec3_t lightorigin;
	vec3_t newlight;
	vec3_t lightline;
	float intensity;
	float falloff;

	if (thislight->intensity > 0 && (backEnd.currentEntity->e.renderfx & RF_INVISIBLE)) {
		return;
	}

	if (thislight->intensity < 0 && !(backEnd.currentEntity->e.renderfx & RF_INVISIBLE)) {
		return;
	}

	if (thislight->needs_trace)
	{
        trace_t trace;
		ri.CM_BoxTrace(&trace,
			backEnd.currentSphere->traceOrigin,
			thislight->origin,
			vec3_origin,
			vec3_origin,
			0,
			CONTENTS_SOLID,
			0
		);

		if (trace.fraction < 1) {
			return;
		}
	}

    if (!thislight->spot_light)
    {
		reallightinfo_t* pLight;
		float fDist;
		float fRadius;

        VectorSubtract(thislight->origin, backEnd.currentEntity->e.origin, lightorigin);
        MatrixTransformVectorRight(backEnd.currentEntity->e.axis, lightorigin, lightline);
        VectorSubtract(lightline, backEnd.currentSphere->origin, newlight);
  
		intensity = thislight->intensity * 7500.0;
		falloff = 1.f / VectorLengthSquared(newlight) * intensity;
		if (falloff >= 5.0) {
			RB_Sphere_DrawDebugLine(thislight, falloff, lightline);
			if (backEnd.currentSphere->numRealLights < MAX_REAL_LIGHTS) {
				pLight = &backEnd.currentSphere->light[backEnd.currentSphere->numRealLights];

				fDist = VectorLength(newlight);
				pLight->color[0] = thislight->color[0] * falloff * tr.overbrightMult * r_entlight_scale->value * fDist;
				pLight->color[1] = thislight->color[1] * falloff * tr.overbrightMult * r_entlight_scale->value * fDist;
                pLight->color[2] = thislight->color[2] * falloff * tr.overbrightMult * r_entlight_scale->value * fDist;
			
				VectorCopy(lightline, pLight->vOrigin);
				pLight->fDist = fDist;
				pLight->eType = LIGHT_POINT;
				pLight->fIntensity = pLight->color[0] * 0.299f + pLight->color[1] * 0.587f + pLight->color[2] * 0.114f;
				
				fRadius = fDist - backEnd.currentSphere->radius;
				if (fRadius > 0) {
					pLight->fIntensity /= fRadius;
				}

				backEnd.currentSphere->numRealLights++;
			}
		}
	} else {
        RB_Sphere_AddSpotLight(thislight);
	}
}

static void RB_Sphere_BuildStaticLights()
{
	int i;
	int curleaf;
	mnode_t* leaf;

	if (tr.world->vis)
    {
		int cntarea;

		for (curleaf = 0, leaf = backEnd.currentSphere->leaves[curleaf]; curleaf < 8 && leaf; curleaf++, leaf++) {
			cntarea = leaf->numlights;

			if (cntarea) {
				for (i = (*leaf->lights == &tr.sSunLight ? 1 : 0); i < cntarea; i++) {
					spherel_t* pLight = leaf->lights[i];
					byte mask = backEnd.refdef.areamask[pLight->leaf->area >> 3];


					if (!(mask & (1 << (pLight->leaf->area & 7))) && pLight->reference_count != light_reference_count) {
                        RB_Sphere_AddLight(pLight);
						pLight->reference_count = light_reference_count;
					}
				}
			}
		}
	}
	else
	{
		for (i = 0; i < tr.numSLights; i++) {
			RB_Sphere_AddLight(&tr.sLights[i]);
		}
	}
}

void RB_Sphere_BuildDLights()
{
	int i;
	float length;
	vec3_t lightorigin;
	vec3_t delta;
	float scale;
	float fRadius;
	sphereor_t* pSphere;
	reallightinfo_t* pLight;

    pSphere = backEnd.currentSphere;
    backEnd.currentSphere->TessFunction = RB_Light_Real;

	for (i = 0; i < backEnd.refdef.num_dlights && pSphere->numRealLights < MAX_REAL_LIGHTS; i++) {
		VectorSubtract(backEnd.refdef.dlights[i].origin, pSphere->worldOrigin, delta);
		length = VectorLength(delta);

		fRadius = backEnd.refdef.dlights[i].radius + pSphere->radius;
		if (fRadius < length) {
			continue;
		}

		VectorSubtract(backEnd.refdef.dlights[i].origin, backEnd.currentEntity->e.origin, lightorigin);
		scale = r_entlight_scale->value * tr.overbrightMult * 7500.0 * backEnd.refdef.dlights[i].radius / length;
        pLight = &pSphere->light[pSphere->numRealLights];
        pLight->color[0] = scale * backEnd.refdef.dlights[i].color[0];
        pLight->color[1] = scale * backEnd.refdef.dlights[i].color[1];
        pLight->color[2] = scale * backEnd.refdef.dlights[i].color[2];
		MatrixTransformVectorRight(backEnd.currentEntity->e.axis, lightorigin, pLight->vOrigin);
		
		pLight->eType = LIGHT_POINT;
		pLight->fDist = length;
		pLight->fIntensity = pLight->color[0] * 0.299f + pLight->color[1] * 0.587f + pLight->color[2] * 0.114f;
		
		if (pLight->fDist - backEnd.currentSphere->radius >= 0.f) {
			pLight->fIntensity /= pLight->fDist - backEnd.currentSphere->radius;
		}

		backEnd.currentSphere->numRealLights++;
	}
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
	trRefEntity_t* refent;
	trRefEntity_t* newref;

	for (refent = backEnd.currentEntity; ; refent = newref) {
		if (refent->e.parentEntity == ENTITYNUM_NONE) {
			break;
		}

		newref = &backEnd.refdef.entities[refent->e.parentEntity];
		if (refent == newref) {
			break;
		}
	}

	VectorCopy(refent->e.lightingOrigin, backEnd.currentSphere->traceOrigin);
	RB_SetupEntityGridLighting();
}

void RB_Grid_SetupStaticModel()
{
	RB_SetupStaticModelGridLighting(&tr.refdef, backEnd.currentStaticModel, backEnd.currentStaticModel->origin);
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
