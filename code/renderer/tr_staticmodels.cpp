/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

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

// tr_staticmodels.cpp -- static model rendering

#include "tr_local.h"
#include "tiki.h"

#define MAX_STATIC_MODELS_SURFS		8192

int g_nStaticSurfaces;
staticSurface_t g_staticSurfaces[MAX_STATIC_MODELS_SURFS];
qboolean g_bInfostaticmodels = qfalse;

/*
==============
R_InitStaticModels
==============
*/
void R_InitStaticModels(void) {
    cStaticModelUnpacked_t* pSM;
    char	szTemp[1024];
    bool	exists;
    skelBoneCache_t		bones[128];
    float	radius;
    int		i, j, k, l;

    g_bInfostaticmodels = qfalse;

    if (tr.overbrightShift)
    {
        for (i = 0; i < tr.world->numStaticModelData; i++)
        {
            int		r, g, b;

            r = (int)((float)tr.world->staticModelData[i * 4] * tr.overbrightMult);
            g = (int)((float)tr.world->staticModelData[i * 4 + 1] * tr.overbrightMult);
            b = (int)((float)tr.world->staticModelData[i * 4 + 2] * tr.overbrightMult);

            if (r > 0xFF || g > 0xFF || b > 0xFF)
            {
                float t;

                t = 255.0 / (float)Q_max(r, Q_max(g, b));

                r = (int)((float)r * t);
                g = (int)((float)g * t);
                b = (int)((float)b * t);
            }

            tr.world->staticModelData[i * 4] = r;
            tr.world->staticModelData[i * 4 + 1] = g;
            tr.world->staticModelData[i * 4 + 2] = b;
        }
    }

    for (i = 0; i < tr.world->numStaticModels; i++)
    {
        vec3_t mins, maxs;

        pSM = &tr.world->staticModels[i];

        pSM->bRendered = qfalse;
        AngleVectorsLeft(pSM->angles, pSM->axis[0], pSM->axis[1], pSM->axis[2]);

        if (!strnicmp(pSM->model, "models", 6)) {
            strcpy(szTemp, pSM->model);
        }
        else {
            sprintf(szTemp, "models/%s", pSM->model);
        }

        FS_CanonicalFilename(szTemp);
        exists = TIKI_FindTiki(szTemp) != NULL;
        pSM->tiki = TIKI_RegisterTiki(szTemp);

        if (!pSM->tiki) {
            ri.Printf(PRINT_WARNING, "^~^~^: Warning: Cannot Load Static Model %s\n", szTemp);
            continue;
        }

        pSM->radius = TIKI_GlobalRadius(pSM->tiki);

        //
        // register shaders
        //
        for (j = 0; j < pSM->tiki->num_surfaces; j++)
        {
            dtikisurface_t* surf = &pSM->tiki->surfaces[j];

            for (k = 0; k < surf->numskins; k++)
            {
                if (surf->shader[k][0]) {
                    shader_t* sh = R_FindShader(surf->shader[k], -1, !(surf->flags & TIKI_SURF_NOMIPMAPS), !(surf->flags & TIKI_SURF_NOPICMIP), qtrue, qtrue);
                    surf->hShader[k] = sh->index;
                }
                else {
                    surf->hShader[k] = 0;
                }
            }
        }

        // prepare the skeleton frame for the static model
        TIKI_GetSkelAnimFrame(pSM->tiki, bones, &radius, &mins, &maxs);
        pSM->cull_radius = radius * pSM->tiki->load_scale * pSM->scale;

        // Suggestion:
        // It would be cool to have animated static model in the future

        if (!exists)
        {
            for (j = 0; j < pSM->tiki->numMeshes; j++)
            {
                skelHeaderGame_t* skelmodel = TIKI_GetSkel(pSM->tiki->mesh[j]);
                skelSurfaceGame_t* surf;

                if (!skelmodel) {
                    ri.Printf(PRINT_WARNING, "^~^~^: Warning: Missing mesh in Static Model %s\n", skelmodel->name);
                    continue;
                }

                surf = skelmodel->pSurfaces;

                for (k = 0; k < skelmodel->numSurfaces; k++, surf = surf->pNext)
                {
                    byte* buf;
                    byte* p;
                    skelWeight_t* weight;
                    skeletorVertex_t* vert;

                    if (surf->pStaticXyz) {
                        continue;
                    }

                    // allocate static vectors
                    p = buf = (byte*)ri.TIKI_Alloc((sizeof(vec4_t) + sizeof(vec4_t) + sizeof(vec2_t) * 2) * surf->numVerts);
                    surf->pStaticXyz = (vec4_t*)p;
                    p += sizeof(vec4_t) * surf->numVerts;
                    surf->pStaticNormal = (vec4_t*)p;
                    p += sizeof(vec4_t) * surf->numVerts;
                    surf->pStaticTexCoords = (vec2_t(*)[2])p;

                    vert = surf->pVerts;

                    for (l = 0; l < surf->numVerts; l++)
                    {
                        int channel;
                        skelBoneCache_t* bone;

                        weight = (skelWeight_t*)((byte*)vert + sizeof(skeletorVertex_t) + vert->numMorphs * sizeof(skeletorMorph_t));

                        if (j > 0) {
                            channel = pSM->tiki->m_boneList.GetLocalFromGlobal(skelmodel->pBones[weight->boneIndex].channel);
                        }
                        else {
                            channel = weight->boneIndex;
                        }

                        bone = &bones[channel];

                        surf->pStaticXyz[l][0] = ((weight->offset[0] * bone->matrix[0][0]
                            + weight->offset[1] * bone->matrix[1][0]
                            + weight->offset[2] * bone->matrix[2][0])
                            + bone->offset[0]) * weight->boneWeight;
                        surf->pStaticXyz[l][1] = ((weight->offset[0] * bone->matrix[0][1]
                            + weight->offset[1] * bone->matrix[1][1]
                            + weight->offset[2] * bone->matrix[2][1])
                            + bone->offset[1]) * weight->boneWeight;
                        surf->pStaticXyz[l][2] = ((weight->offset[0] * bone->matrix[0][2]
                            + weight->offset[1] * bone->matrix[1][2]
                            + weight->offset[2] * bone->matrix[2][2])
                            + bone->offset[2]) * weight->boneWeight;

                        surf->pStaticNormal[l][0] = vert->normal[0] * bone->matrix[0][0]
                            + vert->normal[1] * bone->matrix[1][0]
                            + vert->normal[2] * bone->matrix[2][0];
                        surf->pStaticNormal[l][1] = vert->normal[0] * bone->matrix[0][1]
                            + vert->normal[1] * bone->matrix[1][1]
                            + vert->normal[2] * bone->matrix[2][1];
                        surf->pStaticNormal[l][2] = vert->normal[0] * bone->matrix[0][2]
                            + vert->normal[1] * bone->matrix[1][2]
                            + vert->normal[2] * bone->matrix[2][2];

                        surf->pStaticTexCoords[l][0][0] = vert->texCoords[0];
                        surf->pStaticTexCoords[l][0][1] = vert->texCoords[1];
                        surf->pStaticTexCoords[l][1][0] = vert->texCoords[0];
                        surf->pStaticTexCoords[l][1][1] = vert->texCoords[1];

                        vert = (skeletorVertex_t*)((byte*)vert + sizeof(skeletorVertex_t) + sizeof(skeletorMorph_t) * vert->numMorphs + sizeof(skelWeight_t) * vert->numWeights);
                    }
                }
            }
        }
    }

    tr.refdef.numStaticModels = tr.world->numStaticModels;
    tr.refdef.staticModels = tr.world->staticModels;
    tr.refdef.numStaticModelData = tr.world->numStaticModelData;
    tr.refdef.staticModelData = tr.world->staticModelData;
}

/*
==============
R_CullStaticModel
==============
*/
static int R_CullStaticModel(dtiki_t* tiki, float fScale, const vec3_t vLocalOrg) {
    vec3_t	bounds[2];
    int		i;
    int		cull;

    for (i = 0; i < 3; i++)
    {
        bounds[0][i] = vLocalOrg[i] + tiki->a->mins[i] * fScale;
        bounds[1][i] = vLocalOrg[i] + tiki->a->maxs[i] * fScale;
    }

    cull = R_CullLocalBox(bounds);

    // FIXME: r_showcull

    switch (cull)
    {
    case CULL_CLIP:
        tr.pc.c_box_cull_md3_clip++;
        break;
    case CULL_IN:
        tr.pc.c_box_cull_md3_in++;
        break;
    case CULL_OUT:
        tr.pc.c_box_cull_md3_out++;
        break;
    }

    return cull;
}

/*
==============
R_AddStaticModelSurfaces
==============
*/
void R_AddStaticModelSurfaces(void) {
    cStaticModelUnpacked_t* SM;
    int i, j;
    int ofsStaticData;
    int iRadiusCull;
    dtiki_t* tiki;
    float tiki_scale;
    vec3_t tiki_localorigin;
    vec3_t tiki_worldorigin;

    if (!tr.world->numStaticModels) {
        return;
    }

    tr.shiftedIsStatic = (1 << QSORT_STATICMODEL_SHIFT);

    for (i = 0; i < tr.world->numStaticModels; i++)
    {
        SM = &tr.world->staticModels[i];

        //if( SM->visCount != tr.visCounts[ tr.visIndex ] ) {
        //	continue;
        //}

        tiki = SM->tiki;

        if (!tiki) {
            continue;
        }

        tr.currentEntityNum = i;
        tr.shiftedEntityNum = i << QSORT_ENTITYNUM_SHIFT;

        R_RotateForStaticModel(SM, &tr.viewParms, &tr.ori );

        ofsStaticData = 0;

        // get the world position
        tiki_scale = tiki->load_scale * SM->scale;
        VectorScale(tiki->load_origin, tiki_scale, tiki_localorigin);
        R_LocalPointToWorld(tiki_localorigin, tiki_worldorigin);

        iRadiusCull = R_CullPointAndRadius(tiki_worldorigin, SM->cull_radius);

        // FIXME: r_showcull

        if (iRadiusCull != CULL_OUT && (iRadiusCull != CULL_IN || R_CullStaticModel(SM->tiki, tiki_scale, tiki_localorigin) != CULL_OUT))
        {
            dtikisurface_t* dsurf;

			if (tr.viewParms.isPortal) {
				SM->lodpercentage[1] = R_CalcLod(tiki_worldorigin, SM->cull_radius / SM->scale);
			} else {
				SM->lodpercentage[0] = R_CalcLod(tiki_worldorigin, SM->cull_radius / SM->scale);
			}

            //
            // draw all meshes
            //
            dsurf = tiki->surfaces;
            for (int mesh = 0; mesh < tiki->numMeshes; mesh++)
            {
                skelHeaderGame_t* skelmodel = TIKI_GetSkel(tiki->mesh[mesh]);
                skelSurfaceGame_t* surface;
                staticSurface_t* s_surface;
                shader_t* shader;

                if (!skelmodel) {
                    continue;
                }

                //
                // draw all surfaces
                //
                surface = skelmodel->pSurfaces;
                for (j = 0; j < skelmodel->numSurfaces; j++, ofsStaticData += surface->numVerts, surface = surface->pNext, dsurf++)
                {
                    if (g_nStaticSurfaces >= MAX_STATIC_MODELS_SURFS)
                    {
                        ri.Printf(PRINT_DEVELOPER, "^~^~^ ERROR: MAX_STATIC_MODELS_SURFS exceeded - surface of '%s' skipped\n", tiki->a->name);
                        continue;
                    }

                    s_surface = &g_staticSurfaces[g_nStaticSurfaces++];
					s_surface->ident = SF_TIKI_STATIC;
					s_surface->ofsStaticData = ofsStaticData;
					s_surface->surface = surface;
					s_surface->meshNum = mesh;

                    shader = tr.shaders[dsurf->hShader[0]];

                    if (shader->numUnfoggedPasses == 1)
                    {
                        if (shader->unfoggedStages[0]->alphaGen == AGEN_DIST_FADE)
                        {
                            if (R_DistanceCullPointAndRadius(shader->fDistNear + shader->fDistRange, tiki_worldorigin, SM->cull_radius) == CULL_OUT) {
                                continue;
                            }
						}
						else if (shader->unfoggedStages[0]->alphaGen == AGEN_ONE_MINUS_DIST_FADE)
						{
							if (R_DistanceCullPointAndRadius(shader->fDistNear, tiki_worldorigin, SM->cull_radius) == CULL_IN) {
								continue;
							}
						}
                    }

                    SM->bRendered = qtrue;
                    R_AddDrawSurf((surfaceType_t*)s_surface, shader, 0);

                    if (r_showstaticlod->integer) {
                        //
                        // FIXME: draw debug lods
                        //
                    }

                    if (r_showstaticbboxes->integer) {
                        //
                        // FIXME: draw debug bbox
                        //
                    }
                }
            }
        }
    }

    tr.shiftedIsStatic = 0;
}

void RB_Static_BuildDLights()
{
    // FIXME: unimplemented
}

void R_InfoStaticModels_f()
{
    // FIXME: unimplemented
}

/*
=============
RB_StaticMesh
=============
*/
void RB_StaticMesh(staticSurface_t* staticSurf) {
    int					i, j;
    dtiki_t* tiki;
    skelSurfaceGame_t* surf;
    int					meshNum;
    skelHeaderGame_t* skelmodel;
    int					render_count;
    skelIndex_t* collapse_map;
    skelIndex_t* triangles;
    int					indexes;
    int					baseIndex, baseVertex;
    short				collapse[1000];

    assert(backEnd.currentStaticModel);
    tiki = backEnd.currentStaticModel->tiki;
    surf = staticSurf->surface;

    assert(surf->pStaticXyz);

    meshNum = staticSurf->meshNum;
    skelmodel = TIKI_GetSkel(tiki->mesh[meshNum]);

    // FIXME: LOD
    render_count = surf->numVerts;

    if (tess.numVertexes + render_count >= TIKI_MAX_VERTEXES ||
        tess.numIndexes + surf->numTriangles >= TIKI_MAX_TRIANGLES * 3) {
        RB_CHECKOVERFLOW(render_count, surf->numTriangles);
    }

    collapse_map = surf->pCollapse;
    triangles = surf->pTriangles;
    indexes = surf->numTriangles * 3;
    baseIndex = tess.numIndexes;
    baseVertex = tess.numVertexes;
    tess.numVertexes += render_count;

    if (render_count == surf->numVerts)
    {
        for (j = 0; j < indexes; j++) {
            tess.indexes[baseIndex + j] = baseVertex + triangles[j];
        }
        tess.numIndexes += indexes;
    }
    else
    {
        for (i = 0; i < render_count; i++) {
            collapse[i] = i;
        }
        for (i = 0; i < surf->numVerts; i++) {
            collapse[i] = collapse[collapse_map[i]];
        }

        for (j = 0; j < indexes; j += 3)
        {
            if (collapse[triangles[j]] == collapse[triangles[j + 1]] ||
                collapse[triangles[j + 1]] == collapse[triangles[j + 2]] ||
                collapse[triangles[j + 2]] == collapse[triangles[j]])
            {
                break;
            }

            tess.indexes[baseIndex + j] = baseVertex + collapse[triangles[j]];
            tess.indexes[baseIndex + j + 1] = baseVertex + collapse[triangles[j + 1]];
            tess.indexes[baseIndex + j + 2] = baseVertex + collapse[triangles[j + 2]];
        }
        tess.numIndexes += j;
    }

    for (j = 0; j < render_count; j++) {
        Vector4Copy(surf->pStaticXyz[j], tess.xyz[baseVertex + j]);
        Vector4Copy(surf->pStaticNormal[j], tess.normal[baseVertex + j]);
        tess.texCoords[baseVertex + j][0][0] = surf->pStaticTexCoords[j][0][0];
        tess.texCoords[baseVertex + j][0][1] = surf->pStaticTexCoords[j][0][1];
    }

    if (backEndData[backEnd.smpFrame]->staticModelData) {
        const size_t offset = backEnd.currentStaticModel->firstVertexData + staticSurf->ofsStaticData * sizeof(color4ub_t);
		assert(offset < tr.world->numStaticModelData * sizeof(color4ub_t));
		assert(offset + render_count * sizeof(color4ub_t) <= tr.world->numStaticModelData * sizeof(color4ub_t));

        const color4ub_t* in = (const color4ub_t*)&backEndData[backEnd.smpFrame]->staticModelData[offset];

        for (i = 0; i < render_count; i++)
        {
			tess.vertexColors[baseVertex + i][0] = in[i][0];
			tess.vertexColors[baseVertex + i][1] = in[i][1];
			tess.vertexColors[baseVertex + i][2] = in[i][2];
			tess.vertexColors[baseVertex + i][3] = in[i][3];
        }
    }
    else {
        for (i = 0; i < render_count; i++) {
            tess.vertexColors[baseVertex + i][0] = 0xFF;
            tess.vertexColors[baseVertex + i][1] = 0xFF;
            tess.vertexColors[baseVertex + i][2] = 0xFF;
            tess.vertexColors[baseVertex + i][3] = 0xFF;
        }
    }

    tess.vertexColorValid = qtrue;
}

void R_PrintInfoStaticModels()
{
    // FIXME: unimplemented
}
