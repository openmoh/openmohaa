/*
===========================================================================
Copyright (C) 2024 the OpenMoHAA team

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

// tr_models.cpp -- model loading and caching

#include "tr_local.h"
#include "../corepp/tiki.h"
#include "../corepp/vector.h"

#define LL(x) x = LittleLong(x)

qboolean   g_bInfoworldtris = qfalse;
static int entityNumIndexes[MAX_ENTITIES];
static int staticModelNumIndexes[4095];

static int R_CullSkelModel(dtiki_t *tiki, refEntity_t *e, skelAnimFrame_t *newFrame, float fScale, float *vLocalOrg);

/*
** R_GetModelByHandle
*/
model_t *R_GetModelByHandle(qhandle_t hModel)
{
    model_t *mod;

    // out of range gets the default model
    if (hModel < 1 || hModel >= tr.numModels) {
        return tr.models[0];
    }

    mod = tr.models[hModel];

    return mod;
}

/*
** R_Model_GetHandle
*/
dtiki_t *R_Model_GetHandle(qhandle_t handle)
{
    model_t *model = R_GetModelByHandle(handle);

    if (model->type == MOD_TIKI) {
        return model->d.tiki;
    }

    return NULL;
}

//===============================================================================

/*
** R_FreeModel
*/
void R_FreeModel(model_t *mod)
{
    if (mod->type == MOD_TIKI) {
        ri.CG_EndTiki(mod->d.tiki);
    }

    memset(mod, 0, sizeof(*mod));
}

/*
** R_AllocModel
*/
model_t *R_AllocModel(void)
{
	model_t *mod;
    int i;

    for (i = 0; i < tr.numModels; i++) {
        mod = tr.models[i];
        if (!mod->name[0]) {
            break;
        }
    }

    if (i == tr.numModels) {
        if (i == MAX_MOD_KNOWN) {
            return NULL;
        }

        mod = (model_t*)ri.Hunk_Alloc(sizeof(*tr.models[tr.numModels]), h_low);
        tr.models[tr.numModels] = mod;
        tr.numModels++;
    } else {
        mod = tr.models[i];
    }

    mod->index = i;

    return mod;
}

/*
** RE_FreeModels
*/
void RE_FreeModels(void)
{
    int hModel;

    for (hModel = 0; hModel < tr.numModels; hModel++) {
        if (!tr.models[hModel]->name[0]) {
            continue;
        }

        R_FreeModel(tr.models[hModel]);
    }
}

/*
** R_RegisterShaders
*/
void R_RegisterShaders(model_t *mod)
{
    dtiki_t        *tiki;
    int             i, j;
    dtikisurface_t *psurface;
    shader_t       *sh;

    tiki = mod->d.tiki;

    for (i = 0; i < tiki->num_surfaces; i++) {
        psurface = &tiki->surfaces[i];

        assert(psurface->numskins <= MAX_TIKI_SHADER);
        for (j = 0; j < psurface->numskins; j++) {
            if (psurface->shader[j][0]) {
                sh                   = R_FindShader(psurface->shader[j], LIGHTMAP_NONE, qtrue);
                psurface->hShader[j] = sh->index;
            } else {
                psurface->hShader[j] = 0;
            }
        }
    }
}

/*
** RE_UnregisterServerModel
*/
void RE_UnregisterServerModel(qhandle_t hModel)
{
    if (hModel < 0 || hModel >= MAX_MOD_KNOWN) {
        return;
    }

    if (tr.models[hModel]->serveronly) {
        R_FreeModel(tr.models[hModel]);
    }
}

/*
** R_RegisterModelInternal
*/
static qhandle_t R_RegisterModelInternal(const char *name, qboolean bBeginTiki, qboolean use)
{
    model_t    *mod;
    qhandle_t   hModel;
    const char *ptr;

    if (!name || !*name) {
        ri.Printf(PRINT_ALL, "RE_RegisterModel: NULL name\n");
        return 0;
    }

    if (strlen(name) >= 128) {
        Com_Printf("Model name exceeds MAX_MODEL_NAME\n");
        return 0;
    }

    //
    // search the currently loaded models
    //
    for (hModel = 1; hModel < tr.numModels; hModel++) {
        mod = tr.models[hModel];
        if (!Q_stricmp(mod->name, name)) {
            if (mod->type == MOD_BAD) {
                return 0;
            }
            return hModel;
        }
    }

    // allocate a new model_t

    if ((mod = R_AllocModel()) == NULL) {
        ri.Printf(PRINT_WARNING, "RE_RegisterModel: R_AllocModel() failed for '%s'\n", name);
        return 0;
    }

    // only set the name after the model has been successfully loaded
    Q_strncpyz(mod->name, name, sizeof(mod->name));

    // make sure the render thread is stopped
    R_IssuePendingRenderCommands();

    mod->serveronly = qtrue;

    //
    // load the files
    //
    ptr = strrchr(name, '.');

    if (ptr) {
        ptr++;

        if (!stricmp(ptr, "spr")) {
            mod->d.sprite = SPR_RegisterSprite(name);
            Q_strncpyz(mod->name, name, sizeof(mod->name));

            if (mod->d.sprite) {
                mod->type = MOD_SPRITE;
                return mod->index;
            }
        } else if (!stricmp(ptr, "tik")) {
            mod->d.tiki = ri.TIKI_RegisterTikiFlags(name, use);
            Q_strncpyz(mod->name, name, sizeof(mod->name));

            if (mod->d.tiki) {
                mod->type = MOD_TIKI;
                R_RegisterShaders(mod);

                if (bBeginTiki) {
                    ri.CG_ProcessInitCommands(mod->d.tiki, NULL);
                }

                return mod->index;
            }
        }
    }

    ri.Printf(PRINT_ERROR, "RE_RegisterModel: Registration failed for '%s'\n", name);
    mod->type = MOD_BAD;

    return 0;
}

/*
** RE_RegisterServerModel
*/
qhandle_t RE_RegisterServerModel(const char *name)
{
    return R_RegisterModelInternal(name, qtrue, qfalse);
}

/*
** RE_SpawnEffectModel
*/
qhandle_t RE_SpawnEffectModel(const char *szModel, vec3_t vPos, vec3_t *axis)
{
    refEntity_t new_entity;

    memset(&new_entity, 0, sizeof(refEntity_t));
    memset(&new_entity.shaderRGBA, 255, sizeof(byte) * 4);

    VectorCopy(vPos, new_entity.origin);
    new_entity.scale = 1.0;

    if (axis) {
        AxisCopy(axis, new_entity.axis);
    }

    new_entity.hModel = R_RegisterModelInternal(szModel, qfalse, qtrue);

    if (new_entity.hModel) {
        tr.models[new_entity.hModel]->serveronly = qfalse;
        ri.CG_ProcessInitCommands(tr.models[new_entity.hModel]->d.tiki, &new_entity);
    }

    return new_entity.hModel;
}

/*
** RE_RegisterModel
*/
qhandle_t RE_RegisterModel(const char *name)
{
    qhandle_t handle;

    handle = R_RegisterModelInternal(name, qtrue, qtrue);

    if (handle) {
        tr.models[handle]->serveronly = qfalse;
    }
    return handle;
}

//=============================================================================

/*
===============
R_ModelInit
===============
*/
void R_ModelInit(void)
{
    model_t *mod;
    int      i;

    // leave a space for NULL model
    tr.numModels = 0;

    mod = R_AllocModel();
    Q_strncpyz(mod->name, "** BAD MODEL **", sizeof(mod->name));
    mod->type = MOD_BAD;

    for (i = 0; i < ARRAY_LEN(tr.skel_index); i++) {
        tr.skel_index[i] = -1;
    }
}

/*
================
R_Modellist_f
================
*/
void R_Modellist_f(void)
{
    int i;

    for (i = 1; i < tr.numModels; i++) {
        ri.Printf(PRINT_ALL, "%s\n", tr.models[i]->name);
    }
}

/*
====================
R_ModelRadius
====================
*/
float R_ModelRadius(qhandle_t handle)
{
    int      j;
    vec3_t   bounds[2];
    model_t *model;
    float    radius, maxRadius;
    vec3_t   tmpVec;
    float    w;

    model = R_GetModelByHandle(handle);

    switch (model->type) {
    case MOD_BRUSH:
        maxRadius = 0.0;

        VectorCopy(model->bmodel->bounds[0], bounds[0]);
        VectorCopy(model->bmodel->bounds[1], bounds[1]);

        for (j = 0; j < 8; j++) {
            tmpVec[0] = bounds[j & 1 ? 1 : 0][0];
            tmpVec[1] = bounds[j & 2 ? 1 : 0][1];
            tmpVec[2] = bounds[j & 4 ? 1 : 0][2];

            radius = VectorLength(tmpVec);

            if (maxRadius < radius) {
                maxRadius = radius;
            }
        }
        break;
    case MOD_TIKI:
        return ri.TIKI_GlobalRadius(model->d.tiki);
    case MOD_SPRITE:
        maxRadius = model->d.sprite->width * model->d.sprite->scale * 0.5;
        w         = model->d.sprite->height * model->d.sprite->scale * 0.5;

        if (maxRadius <= w) {
            maxRadius = w;
        }
        break;
    default:
        maxRadius = 0.0;
    }

    return maxRadius;
}

/*
====================
R_ModelBounds
====================
*/
void R_ModelBounds(qhandle_t handle, vec3_t mins, vec3_t maxs)
{
    model_t *model;

    model = R_GetModelByHandle(handle);

    switch (model->type) {
    default:
    case MOD_BAD:
        VectorClear(mins);
        VectorClear(maxs);
        break;
    case MOD_BRUSH:
        VectorCopy(model->bmodel->bounds[0], mins);
        VectorCopy(model->bmodel->bounds[1], maxs);
        break;
    case MOD_TIKI:
        ri.TIKI_CalculateBounds(model->d.tiki, 1.0, mins, maxs);
        break;
    case MOD_SPRITE:
        mins[0] = -model->d.sprite->width * model->d.sprite->scale * 0.5;
        mins[1] = -model->d.sprite->height * model->d.sprite->scale * 0.5;
        mins[2] = -0.0;
        maxs[0] = model->d.sprite->width * model->d.sprite->scale * 0.5;
        maxs[1] = model->d.sprite->height * model->d.sprite->scale * 0.5;
        maxs[2] = 0.0;
        break;
    }
}

#if 0
// Replaced by TIKI_FindSkelByHeader
/*
====================
GetModelPath
====================
*/
const char *GetModelPath( skelHeaderGame_t *skelmodel ) {
	int			i;
	int			num;
	skelcache_t	*cache;

	num = cache_numskel;

	for( i = 0; i < TIKI_MAX_SKELCACHE; i++ )
	{
		cache = &skelcache[ i ];

		if( cache->skel )
		{
			if( cache->skel == skelmodel ) {
				return cache->path;
			}

			num--;
			if( num < 0 ) {
				break;
			}
		}
	}

	return NULL;
}
#endif

/*
====================
GetLodCutoff
====================
*/
int GetLodCutoff(skelHeaderGame_t *skelmodel, float lod_val, int renderfx)
{
    lodControl_t *LOD;
    float         f;
    float         fLODCap;

    LOD = skelmodel->pLOD;

    if (renderfx & RF_DEPTHHACK) {
        fLODCap = LOD->maxMetric + (LOD->minMetric - LOD->maxMetric) * r_lodviewmodelcap->value;
    } else {
        f       = (LOD->minMetric - LOD->maxMetric) * r_lodcap->value + LOD->maxMetric;
        fLODCap = (lod_val - LOD->maxMetric) * r_lodscale->value + LOD->maxMetric;

        if (fLODCap > f) {
            fLODCap = f;
        }
    }

    if (fLODCap >= LOD->minMetric || !r_uselod->integer) {
        return LOD->curve[0].val;
    } else if (fLODCap <= LOD->maxMetric) {
        return LOD->curve[4].val;
    } else if (fLODCap <= LOD->consts[3].cutoff) {
        return fLODCap * LOD->consts[3].scale + LOD->consts[3].base;
    } else if (fLODCap <= LOD->consts[2].cutoff) {
        return fLODCap * LOD->consts[2].scale + LOD->consts[2].base;
    } else if (fLODCap <= LOD->consts[1].cutoff) {
        return fLODCap * LOD->consts[1].scale + LOD->consts[1].base;
    } else {
        return fLODCap * LOD->consts[0].scale + LOD->consts[0].base;
    }
}

/*
===============
R_SaveLODFile
===============
*/
static void R_SaveLODFile(const char *path, lodControl_t *LOD)
{
    fileHandle_t file = ri.FS_OpenFileWrite(path);
    if (!file) {
        ri.Printf(PRINT_WARNING, "SaveLODFile: Failed to open file %s\n", path);
        return;
    }

    ri.FS_Write(LOD, sizeof(lodControl_t), file);
}

/*
====================
GetToolLodCutoff
====================
*/
int GetToolLodCutoff(skelHeaderGame_t *skelmodel, float lod_val)
{
    lodControl_t *LOD;
    float         totalRange;
    int           i;
    char          lodPath[256];
    char         *ext;

    LOD        = skelmodel->pLOD;
    totalRange = 0.0;
    for (i = 0; i < 10; i++) {
        if (skelmodel->lodIndex[i] > 0) {
            totalRange = skelmodel->lodIndex[i];
            break;
        }
    }

    if (lod_save->integer == 1) {
        ri.Cvar_Set("lod_save", "0");
        Q_strncpyz(lodPath, ri.TIKI_FindSkelByHeader(skelmodel)->path, sizeof(lodPath));
        ext = strstr(lodPath, "skd");
        strcpy(ext, "lod");
        R_SaveLODFile(lodPath, LOD);
    }

    if (lod_mesh->modified) {
        lod_mesh->modified = qfalse;
        ri.Cvar_Set("lod_minLOD", va("%f", LOD->minMetric));
        ri.Cvar_Set("lod_maxLOD", va("%f", LOD->maxMetric));
        ri.Cvar_Set("lod_LOD_slider", va("%f", 0.5));
        ri.Cvar_Set("lod_curve_0_slider", va("%f", LOD->curve[0].val / totalRange));
        ri.Cvar_Set("lod_curve_1_slider", va("%f", LOD->curve[1].val / totalRange));
        ri.Cvar_Set("lod_curve_2_slider", va("%f", LOD->curve[2].val / totalRange));
        ri.Cvar_Set("lod_curve_3_slider", va("%f", LOD->curve[3].val / totalRange));
        ri.Cvar_Set("lod_curve_4_slider", va("%f", LOD->curve[4].val / totalRange));
    }

    ri.Cvar_Set("lod_curve_0_val", va("%f", lod_curve_0_slider->value * totalRange));
    ri.Cvar_Set("lod_curve_1_val", va("%f", lod_curve_1_slider->value * totalRange));
    ri.Cvar_Set("lod_curve_2_val", va("%f", lod_curve_2_slider->value * totalRange));
    ri.Cvar_Set("lod_curve_3_val", va("%f", lod_curve_3_slider->value * totalRange));
    ri.Cvar_Set("lod_curve_4_val", va("%f", lod_curve_4_slider->value * totalRange));

    LOD->minMetric    = lod_minLOD->value;
    LOD->maxMetric    = lod_maxLOD->value;
    LOD->curve[0].val = lod_curve_0_val->value;
    LOD->curve[1].val = lod_curve_1_val->value;
    LOD->curve[2].val = lod_curve_2_val->value;
    LOD->curve[3].val = lod_curve_3_val->value;
    LOD->curve[4].val = lod_curve_4_val->value;

    ri.TIKI_CalcLodConsts(LOD);
    return GetLodCutoff(skelmodel, lod_val, 0);
}

/*
==============
R_GetTagPositionAndOrientation
==============
*/
orientation_t R_GetTagPositionAndOrientation(refEntity_t *ent, int tagnum)
{
    int           i;
    orientation_t tag_or, new_or;

    tag_or = RE_TIKI_Orientation(ent, tagnum);

    VectorCopy(ent->origin, new_or.origin);

    for (i = 0; i < 3; i++) {
        VectorMA(new_or.origin, tag_or.origin[i], ent->axis[i], new_or.origin);
    }

    MatrixMultiply(tag_or.axis, ent->axis, new_or.axis);
    return new_or;
}

/*
==============
RB_DrawSkeletor
==============
*/
void RB_DrawSkeletor(trRefEntity_t *ent)
{
    // FIXME: Unimplemented (GL2)
}

surfaceType_t skelSurface = SF_TIKI_SKEL;

/*
==============
R_AddSkelSurfaces
==============
*/
void R_AddSkelSurfaces(trRefEntity_t *ent)
{
    dtiki_t         *tiki;
    qboolean         personalModel;
    float            tiki_scale;
    vec3_t           tiki_localorigin;
    vec3_t           tiki_worldorigin;
    skelBoneCache_t *outbones;
    //int tikiSurfNumOffset;
    static cvar_t   *vmEntity = NULL;
    skeletor_c      *skeletor;
    float            radius;
    SkelVec3         centroid;
    skelAnimFrame_t *newFrame;
    int              added;
    shader_t        *shader;
    dtikisurface_t  *dsurf;
    byte            *bsurf;
    //float range;
    //int render_count, total_tris;
    skelSurfaceGame_t *surface;
    //int skinnum;
    //float target;
    Vector newDistance;
    //vec3_t org;
    int i;
    int mesh;
    int iRadiusCull = 0;
    int num_tags;

    tiki = ent->e.tiki;

    if (!vmEntity) {
        vmEntity = ri.Cvar_Get("viewmodelentity", "", 0);
    }

    R_UpdatePoseInternal(&ent->e);

    // don't add third_person objects if in a portal
    personalModel = (ent->e.renderfx & RF_THIRD_PERSON) && !tr.viewParms.isPortal;

    outbones = &TIKI_Skel_Bones[TIKI_Skel_Bones_Index];

    num_tags = ri.TIKI_GetNumChannels(tiki);

    if (num_tags + TIKI_Skel_Bones_Index > MAX_SKELBONES) {
        ri.Printf(PRINT_DEVELOPER, "R_AddSkelSurfaces: too many skeleton models visible on '%s'\n", tiki->a->name);
        return;
    }

    tiki_scale = tiki->load_scale * ent->e.scale;
    VectorScale(tiki->load_origin, tiki_scale, tiki_localorigin);
    R_LocalPointToWorld(tiki_localorigin, tiki_worldorigin);

    radius = R_GetRadius(&ent->e);

    if (!lod_tool->integer) {
        iRadiusCull = R_CullPointAndRadius(tiki_worldorigin, radius);
        if (r_showcull->integer & 2) {
            switch (iRadiusCull) {
            case CULL_IN:
                R_DebugCircle(tiki_worldorigin, radius * 1.2f, 0, 1, 0, 0.5f, qfalse);
                break;
            case CULL_CLIP:
                R_DebugCircle(tiki_worldorigin, radius * 1.2f, 0, 1, 0, 0.5f, qfalse);
                break;
            case CULL_OUT:
                R_DebugCircle(tiki_worldorigin, radius * 1.2f + 32.f, 1, 0.2f, 0.2f, 0.5f, qfalse);
                break;
            }
        }

        switch (iRadiusCull) {
        case CULL_IN:
            tr.pc.c_sphere_cull_md3_in++;
            break;
        case CULL_CLIP:
            tr.pc.c_sphere_cull_md3_clip++;
            break;
        case CULL_OUT:
            tr.pc.c_sphere_cull_md3_out++;
            break;
        }
    }

    if (tiki->a->bIsCharacter) {
        if (tr.viewParms.isPortal) {
            ent->lodpercentage[1] = R_CalcLod(tiki_worldorigin, 92.f / ent->e.scale);
        } else {
            ent->lodpercentage[0] = R_CalcLod(tiki_worldorigin, 92.f / ent->e.scale);
        }
    } else {
        if (tr.viewParms.isPortal) {
            ent->lodpercentage[1] = R_CalcLod(tiki_worldorigin, radius / ent->e.scale);
        } else {
            ent->lodpercentage[0] = R_CalcLod(tiki_worldorigin, radius / ent->e.scale);
        }
    }

    newFrame = (skelAnimFrame_t *)ri.Hunk_AllocateTempMemory(
        sizeof(skelAnimFrame_t) + ri.TIKI_GetNumChannels(tiki) * sizeof(SkelMat4)
    );
    R_GetFrame(&ent->e, newFrame);

    if (lod_tool->integer || iRadiusCull != CULL_CLIP
        || R_CullSkelModel(tiki, &ent->e, newFrame, tiki_scale, tiki_localorigin) != CULL_OUT) {
        //
        // copy bones position and axis
        //
        for (i = 0; i < num_tags; i++) {
            VectorCopy(newFrame->bones[i][3], outbones->offset);
            outbones->matrix[0][0] = newFrame->bones[i][0][0];
            outbones->matrix[0][1] = newFrame->bones[i][0][1];
            outbones->matrix[0][2] = newFrame->bones[i][0][2];
            outbones->matrix[0][3] = 0;
            outbones->matrix[1][0] = newFrame->bones[i][1][0];
            outbones->matrix[1][1] = newFrame->bones[i][1][1];
            outbones->matrix[1][2] = newFrame->bones[i][1][2];
            outbones->matrix[1][3] = 0;
            outbones->matrix[2][0] = newFrame->bones[i][2][0];
            outbones->matrix[2][1] = newFrame->bones[i][2][1];
            outbones->matrix[2][2] = newFrame->bones[i][2][2];
            outbones->matrix[2][3] = 0;
            outbones++;
        }
    }

    ri.Hunk_FreeTempMemory(newFrame);

    ent->e.bonestart = TIKI_Skel_Bones_Index;
    TIKI_Skel_Bones_Index += num_tags;

    ent->e.hasMorph = qfalse;

    //
    // get the skeletor
    //
    skeletor = (skeletor_c *)ri.TIKI_GetSkeletor(tiki, ent->e.entityNumber);

    //
    // add morphs
    //
    added = ri.SKEL_GetMorphWeightFrame(
        skeletor, ent->e.frameInfo[0].index, ent->e.frameInfo[0].time, &skeletorMorphCache[skeletorMorphCacheIndex]
    );
    ent->e.morphstart = skeletorMorphCacheIndex;

    if (added) {
        // found morphs
        skeletorMorphCacheIndex += added;
        ent->e.hasMorph = qtrue;
    }

    //
    // draw all meshes
    //
    dsurf = tiki->surfaces;
    bsurf = &ent->e.surfaces[0];
    for (mesh = 0; mesh < tiki->numMeshes; mesh++) {
        skelHeaderGame_t *skelmodel = ri.TIKI_GetSkel(tiki->mesh[mesh]);

        if (!skelmodel) {
            ri.Printf(PRINT_DEVELOPER, "R_AddSkelSurfaces: couldn't get skel model in '%s'\n", tiki->a->name);
            return;
        }

        if (lod_tool->integer && !stricmp(ent->e.tiki->a->name, lod_tikiname->string)) {
            if (lod_mesh->integer > tiki->numMeshes - 1) {
                ri.Cvar_Set("lod_mesh", va("%d", tiki->numMeshes - 1));
            }

            if (mesh == lod_mesh->integer) {
                if (skelmodel->pLOD) {
                    break;
                }
            }
        }

        //
        // draw all surfaces
        //
        surface = skelmodel->pSurfaces;
        for (i = 0; i < skelmodel->numSurfaces; i++, dsurf++, bsurf++, surface = surface->pNext) {
            if (*bsurf & 4) {
                continue;
            }

            shader         = NULL;
            surface->ident = SF_TIKI_SKEL;

            // use a custom shader if specified
            if (!(ent->e.customShader) || (ent->e.renderfx & RF_CUSTOMSHADERPASS)) {
                int iShaderNum = ent->e.skinNum + (*bsurf & 3);

                if (iShaderNum >= dsurf->numskins) {
                    iShaderNum = 0;
                }
                shader = tr.shaders[dsurf->hShader[iShaderNum]];
            } else {
                shader = R_GetShaderByHandle(ent->e.customShader);
            }

            if (!personalModel) {
                if ((*bsurf & 0x40) && (dsurf->numskins > 1)) {
                    int iShaderNum = ent->e.skinNum + (*bsurf & 2);

                    R_AddDrawSurf((surfaceType_t *)surface, tr.shaders[dsurf->hShader[iShaderNum]], 0, 0, 0, 0);
                    R_AddDrawSurf((surfaceType_t *)surface, tr.shaders[dsurf->hShader[iShaderNum + 1]], 0, 0, 0, 0);
                } else {
                    R_AddDrawSurf((surfaceType_t *)surface, shader, 0, 0, 0, 0);
                }
            }

            if ((ent->e.customShader) && (ent->e.renderfx & RF_CUSTOMSHADERPASS)) {
                shader = R_GetShaderByHandle(ent->e.customShader);
                R_AddDrawSurf((surfaceType_t *)surface, shader, 0, 0, 0, 0);
            }
        }
    }

    // FIXME: setup LOD
}

/*
=============
SkelVertGetNormal
=============
*/
inline static void SkelVertGetNormal(skeletorVertex_t *vert, skelBoneCache_t *bone, vec3_t out)
{
    out[0] = vert->normal[0] * bone->matrix[0][0] + vert->normal[1] * bone->matrix[1][0]
           + vert->normal[2] * bone->matrix[2][0];

    out[1] = vert->normal[0] * bone->matrix[0][1] + vert->normal[1] * bone->matrix[1][1]
           + vert->normal[2] * bone->matrix[2][1];

    out[2] = vert->normal[0] * bone->matrix[0][2] + vert->normal[1] * bone->matrix[1][2]
           + vert->normal[2] * bone->matrix[2][2];
}

/*
=============
SkelMorphGetXyz
=============
*/
inline static void SkelMorphGetXyz(skeletorMorph_t *morph, int *morphcache, vec3_t out)
{
    VectorMA(out, *morphcache, morph->offset, out);
}

/*
=============
SkelWeightGetXyz
=============
*/
inline static void SkelWeightGetXyz(skelWeight_t *weight, skelBoneCache_t *bone, vec3_t out)
{
    out[0] += ((weight->offset[0] * bone->matrix[0][0] + weight->offset[1] * bone->matrix[1][0]
                + weight->offset[2] * bone->matrix[2][0])
               + bone->offset[0])
            * weight->boneWeight;

    out[1] += ((weight->offset[0] * bone->matrix[0][1] + weight->offset[1] * bone->matrix[1][1]
                + weight->offset[2] * bone->matrix[2][1])
               + bone->offset[1])
            * weight->boneWeight;

    out[2] += ((weight->offset[0] * bone->matrix[0][2] + weight->offset[1] * bone->matrix[1][2]
                + weight->offset[2] * bone->matrix[2][2])
               + bone->offset[2])
            * weight->boneWeight;
}

/*
=============
SkelWeightMorphGetXyz
=============
*/
inline static void SkelWeightMorphGetXyz(skelWeight_t *weight, skelBoneCache_t *bone, vec3_t totalmorph, vec3_t out)
{
    vec3_t point;

    VectorAdd(totalmorph, weight->offset, point);

    out[0] += ((point[0] * bone->matrix[0][0] + point[1] * bone->matrix[1][0] + point[2] * bone->matrix[2][0])
               + bone->offset[0])
            * weight->boneWeight;

    out[1] += ((point[0] * bone->matrix[0][1] + point[1] * bone->matrix[1][1] + point[2] * bone->matrix[2][1])
               + bone->offset[1])
            * weight->boneWeight;

    out[2] += ((point[0] * bone->matrix[0][2] + point[1] * bone->matrix[1][2] + point[2] * bone->matrix[2][2])
               + bone->offset[2])
            * weight->boneWeight;
}

/*
=============
RB_SkelMesh
=============
*/
void RB_SkelMesh(skelSurfaceGame_t *sf)
{
    unsigned int       baseIndex, baseVertex;
    unsigned int       render_count;
    unsigned int       indexes;
    float             *outXyz;
    int16_t           *outNormal;
    skelIndex_t       *triangles;
    skelIndex_t       *collapse_map;
    skeletorVertex_t  *newVerts;
    skeletorMorph_t   *morph;
    skelWeight_t      *weight;
    int                vertNum;
    int                morphNum;
    int                weightNum;
    skelBoneCache_t   *bones;
    skelBoneCache_t   *bone;
    int               *morphs;
    int               *morphcache;
    float              scale;
    dtiki_t           *tiki;
    int                mesh;
    int                surf;
    int                i;
    skelHeaderGame_t  *skelmodel;
    skelSurfaceGame_t *psurface;
    qboolean           bFound;
    short              collapse[TIKI_MAX_VERTEXES];

    if (!r_drawentitypoly->integer) {
        return;
    }

    tiki = backEnd.currentEntity->e.tiki;

    scale = tiki->load_scale * backEnd.currentEntity->e.scale;

    //
    // get the mesh associated with the surface
    //
    bFound = qfalse;
    for (mesh = 0; mesh < tiki->numMeshes; mesh++) {
        skelmodel = ri.TIKI_GetSkel(tiki->mesh[mesh]);
        psurface  = skelmodel->pSurfaces;

        // find the surface
        for (surf = 0; surf < skelmodel->numSurfaces; surf++) {
            if (psurface == sf) {
                bFound = qtrue;
                break;
            }
            psurface = psurface->pNext;
        }

        if (bFound) {
            break;
        }
    }

    assert(bFound);

    //
    // Process LOD
    //
    if (skelmodel->pLOD) {
        float lod_val;
        int   renderfx;

        lod_val  = backEnd.currentEntity->lodpercentage[0];
        renderfx = backEnd.currentEntity->e.renderfx;

        if (sf->numVerts > 3) {
            skelIndex_t *collapseIndex;
            int          mid, low, high;
            int          lod_cutoff;

            if (lod_tool->integer && !strcmp(backEnd.currentEntity->e.tiki->a->name, lod_tikiname->string)
                && mesh == lod_mesh->integer) {
                lod_cutoff = GetToolLodCutoff(skelmodel, backEnd.currentEntity->lodpercentage[0]);
            } else {
                lod_cutoff = GetLodCutoff(skelmodel, backEnd.currentEntity->lodpercentage[0], renderfx);
            }

            collapseIndex = sf->pCollapseIndex;
            if (collapseIndex[2] < lod_cutoff) {
                return;
            }

            low = mid = 3;
            high      = sf->numVerts;
            while (high >= low) {
                mid = (low + high) >> 1;
                if (collapseIndex[mid] < lod_cutoff) {
                    high = mid - 1;
                    if (collapseIndex[mid - 1] >= lod_cutoff) {
                        break;
                    }
                } else {
                    mid++;
                    low = mid;
                    if (high == mid || collapseIndex[mid] < lod_cutoff) {
                        break;
                    }
                }
            }

            render_count = mid;
        } else {
            render_count = sf->numVerts;
        }

        if (!render_count) {
            return;
        }
    } else {
        render_count = sf->numVerts;
    }

    indexes = sf->numTriangles * 3;
    RB_CHECKOVERFLOW(render_count, indexes);

    collapse_map = sf->pCollapse;
    triangles    = sf->pTriangles;
    baseIndex    = tess.numIndexes;
    baseVertex   = tess.numVertexes;
    tess.numVertexes += render_count;

    outXyz    = tess.xyz[baseVertex];
    outNormal = tess.normal[baseVertex];
    newVerts  = sf->pVerts;

    if (render_count == sf->numVerts) {
        for (i = 0; i < indexes; i++) {
            tess.indexes[baseIndex + i] = baseVertex + triangles[i];
        }

        entityNumIndexes[backEnd.currentEntity - backEnd.refdef.entities] += indexes;
        tess.numIndexes += indexes;
    } else {
        assert(sf->numVerts < TIKI_MAX_VERTEXES);

        for (i = 0; i < render_count; i++) {
            collapse[i] = i;
        }

        for (i = render_count; i < sf->numVerts; i++) {
            collapse[i] = collapse[collapse_map[i]];
        }

        for (i = 0; i < indexes; i += 3) {
            assert(collapse[triangles[i]] < sf->numVerts);
            assert(collapse[triangles[i + 1]] < sf->numVerts);
            assert(collapse[triangles[i + 2]] < sf->numVerts);

            if (collapse[triangles[i]] == collapse[triangles[i + 1]]
                || collapse[triangles[i + 1]] == collapse[triangles[i + 2]]
                || collapse[triangles[i + 2]] == collapse[triangles[i]]) {
                break;
            }

            tess.indexes[baseIndex + i]     = baseVertex + collapse[triangles[i]];
            tess.indexes[baseIndex + i + 1] = baseVertex + collapse[triangles[i + 1]];
            tess.indexes[baseIndex + i + 2] = baseVertex + collapse[triangles[i + 2]];
        }

        entityNumIndexes[backEnd.currentEntity - backEnd.refdef.entities] += indexes;
        tess.numIndexes += i;
    }

    //
    // just copy the vertexes
    //
    bones  = &TIKI_Skel_Bones[backEnd.currentEntity->e.bonestart];
    morphs = &skeletorMorphCache[backEnd.currentEntity->e.morphstart];

    if (backEnd.currentEntity->e.hasMorph) {
        if (mesh > 0) {
            for (vertNum = 0; vertNum < render_count; vertNum++) {
                vec3_t normal;
                vec3_t out;
                vec3_t totalmorph;
                int    channelNum;
                int    boneNum;

                VectorClear(out);
                VectorClear(outXyz);
                VectorClear(totalmorph);

                weight = (skelWeight_t *)((byte *)newVerts + sizeof(skeletorVertex_t)
                                          + sizeof(skeletorMorph_t) * newVerts->numMorphs);
                morph  = (skeletorMorph_t *)((byte *)newVerts + sizeof(skeletorVertex_t));

                for (morphNum = 0; morphNum < newVerts->numMorphs; morphNum++) {
                    morphcache = &morphs[morph->morphIndex];

                    if (*morphcache) {
                        SkelMorphGetXyz(morph, morphcache, totalmorph);
                    }

                    morph++;
                }

                if (newVerts->numMorphs) {
                    channelNum = skelmodel->pBones[morph->morphIndex].channel;
                } else {
                    channelNum = skelmodel->pBones[weight->boneIndex].channel;
                }

                boneNum = ri.TIKI_GetLocalChannel(tiki, channelNum);
                bone    = &bones[boneNum];

                SkelVertGetNormal(newVerts, bone, normal);

                for (weightNum = 0; weightNum < newVerts->numWeights; weightNum++) {
                    channelNum = skelmodel->pBones[weight->boneIndex].channel;
                    boneNum    = ri.TIKI_GetLocalChannel(tiki, channelNum);
                    bone       = &bones[boneNum];

                    if (!weightNum) {
                        SkelWeightMorphGetXyz(weight, bone, totalmorph, out);
                    } else {
                        SkelWeightGetXyz(weight, bone, out);
                    }

                    weight++;
                }

                R_VaoPackNormal(outNormal, normal);
                VectorScale(out, scale, outXyz);

                tess.texCoords[baseVertex + vertNum][0] = newVerts->texCoords[0];
                tess.texCoords[baseVertex + vertNum][1] = newVerts->texCoords[1];
                // FIXME: fill in lightmapST for completeness?

                newVerts = (skeletorVertex_t *)((byte *)newVerts + sizeof(skeletorVertex_t)
                                                + sizeof(skeletorMorph_t) * newVerts->numMorphs
                                                + sizeof(skelWeight_t) * newVerts->numWeights);
                outXyz += 4;
                outNormal += 4;
            }
        } else {
            for (vertNum = 0; vertNum < render_count; vertNum++) {
                vec3_t normal;
                vec3_t out;
                vec3_t totalmorph;

                VectorClear(out);
                VectorClear(outXyz);
                VectorClear(totalmorph);

                weight = (skelWeight_t *)((byte *)newVerts + sizeof(skeletorVertex_t)
                                          + sizeof(skeletorMorph_t) * newVerts->numMorphs);
                morph  = (skeletorMorph_t *)((byte *)newVerts + sizeof(skeletorVertex_t));

                for (morphNum = 0; morphNum < newVerts->numMorphs; morphNum++) {
                    morphcache = &morphs[morph->morphIndex];

                    if (*morphcache) {
                        SkelMorphGetXyz(morph, morphcache, totalmorph);
                    }

                    morph++;
                }

                if (newVerts->numMorphs) {
                    bone = &bones[morph->morphIndex];
                } else {
                    bone = &bones[weight->boneIndex];
                }

                SkelVertGetNormal(newVerts, bone, normal);

                for (weightNum = 0; weightNum < newVerts->numWeights; weightNum++) {
                    bone = &bones[weight->boneIndex];

                    if (!weightNum) {
                        SkelWeightMorphGetXyz(weight, bone, totalmorph, out);
                    } else {
                        SkelWeightGetXyz(weight, bone, out);
                    }

                    weight++;
                }

                R_VaoPackNormal(outNormal, normal);
                VectorScale(out, scale, outXyz);

                tess.texCoords[baseVertex + vertNum][0] = newVerts->texCoords[0];
                tess.texCoords[baseVertex + vertNum][1] = newVerts->texCoords[1];
                // FIXME: fill in lightmapST for completeness?

                newVerts = (skeletorVertex_t *)((byte *)newVerts + sizeof(skeletorVertex_t)
                                                + sizeof(skeletorMorph_t) * newVerts->numMorphs
                                                + sizeof(skelWeight_t) * newVerts->numWeights);
                outXyz += 4;
                outNormal += 4;
            }
        }
    } else {
        if (mesh > 0) {
            for (vertNum = 0; vertNum < render_count; vertNum++) {
                vec3_t normal;
                vec3_t out;
                int    channelNum;
                int    boneNum;

                VectorClear(out);
                VectorClear(outXyz);

                weight = (skelWeight_t *)((byte *)newVerts + sizeof(skeletorVertex_t)
                                          + sizeof(skeletorMorph_t) * newVerts->numMorphs);

                channelNum = skelmodel->pBones[weight->boneIndex].channel;
                boneNum    = ri.TIKI_GetLocalChannel(tiki, channelNum);
                bone       = &bones[boneNum];

                SkelVertGetNormal(newVerts, bone, normal);

                for (weightNum = 0; weightNum < newVerts->numWeights; weightNum++) {
                    channelNum = skelmodel->pBones[weight->boneIndex].channel;
                    boneNum    = ri.TIKI_GetLocalChannel(tiki, channelNum);
                    bone       = &bones[boneNum];

                    SkelWeightGetXyz(weight, bone, out);

                    weight++;
                }

                R_VaoPackNormal(outNormal, normal);
                VectorScale(out, scale, outXyz);

                tess.texCoords[baseVertex + vertNum][0] = newVerts->texCoords[0];
                tess.texCoords[baseVertex + vertNum][1] = newVerts->texCoords[1];
                // FIXME: fill in lightmapST for completeness?

                newVerts = (skeletorVertex_t *)((byte *)newVerts + sizeof(skeletorVertex_t)
                                                + sizeof(skeletorMorph_t) * newVerts->numMorphs
                                                + sizeof(skelWeight_t) * newVerts->numWeights);
                outXyz += 4;
                outNormal += 4;
            }
        } else {
            for (vertNum = 0; vertNum < render_count; vertNum++) {
                vec3_t normal;
                vec3_t out;

                VectorClear(out);
                VectorClear(outXyz);

                weight = (skelWeight_t *)((byte *)newVerts + sizeof(skeletorVertex_t)
                                          + sizeof(skeletorMorph_t) * newVerts->numMorphs);

                bone = &bones[weight->boneIndex];
                SkelVertGetNormal(newVerts, bone, normal);

                for (weightNum = 0; weightNum < newVerts->numWeights; weightNum++) {
                    bone = &bones[weight->boneIndex];

                    SkelWeightGetXyz(weight, bone, out);

                    weight++;
                }

                R_VaoPackNormal(outNormal, normal);
                VectorScale(out, scale, outXyz);

                tess.texCoords[baseVertex + vertNum][0] = newVerts->texCoords[0];
                tess.texCoords[baseVertex + vertNum][1] = newVerts->texCoords[1];
                // FIXME: fill in lightmapST for completeness?

                newVerts = (skeletorVertex_t *)((byte *)newVerts + sizeof(skeletorVertex_t)
                                                + sizeof(skeletorMorph_t) * newVerts->numMorphs
                                                + sizeof(skelWeight_t) * newVerts->numWeights);
                outXyz += 4;
                outNormal += 4;
            }
        }
    }

#if 0
	if( backEnd.currentEntity->e.staticModelIndex ) {
		mstaticModel_t *sm;
		color4ub_t *out;
		color3ub_t *in;
		int cdofs;
		skdSurface_t *sf2;

		sm = &tr.world->staticModels[ backEnd.currentEntity->e.staticModelIndex - 1 ];

		tess.useStaticModelVertexColors = qtrue;

		cdofs = 0;
		sf2 = tiki->surfs;
		while( sf2 != sf ) {
			cdofs += sf2->numVerts;
			sf2 = ( skdSurface_t* )( ( ( byte* )sf2 ) + sf2->ofsEnd );
		}

		in = &tr.world->smColors[ sm->firstVert + cdofs ];
		out = tess.vertexColors + baseVertex;
		for( i = 0; i < sf->numVerts; i++, in++, out++ ) {
#    if 1
			( *out )[ 0 ] = ( *in )[ 0 ];
			( *out )[ 1 ] = ( *in )[ 1 ];
			( *out )[ 2 ] = ( *in )[ 2 ];
			( *out )[ 3 ] = 255;
#    elif 0
			( ( int* )out ) = tr.identityLightByte;
#    else		
			// su44: set it to something special so
			// I can debug vertex colors rendering
			( *out )[ 0 ] = 255;
			( *out )[ 1 ] = 0;
			( *out )[ 2 ] = 0;
			( *out )[ 3 ] = 255;
#    endif
		}
	} //else
#endif
    //{
    //	// an attemp to fix bizarre vertex colors bug
    //	color4ub_t *col;
    //
    //	col = &tess.vertexColors[baseVertex];
    //	for(i = 0; i < sf->numVerts; i++,col++) {
    //		(*col)[0] = 255;
    //		(*col)[1] = 255;
    //		(*col)[2] = 255;
    //		(*col)[3] = 255;
    //	}
    //}
    //tess.numVertexes += sf->numVerts;
}

/*
=============
RB_StaticMesh
=============
*/
void RB_StaticMesh(staticSurface_t *staticSurf)
{
    int                i, j;
    dtiki_t           *tiki;
    skelSurfaceGame_t *surf;
    int                meshNum;
    skelHeaderGame_t  *skelmodel;
    int                render_count;
    skelIndex_t       *collapse_map;
    skelIndex_t       *triangles;
    int                indexes;
    int                baseIndex, baseVertex;
    short              collapse[1000];

    if (!r_drawstaticmodelpoly->integer) {
        return;
    }

    assert(backEnd.currentStaticModel);
    tiki = backEnd.currentStaticModel->tiki;
    surf = staticSurf->surface;

    assert(surf->pStaticXyz);
    if (!surf->pStaticXyz) {
        return;
    }

    meshNum   = staticSurf->meshNum;
    skelmodel = ri.TIKI_GetSkel(tiki->mesh[meshNum]);

    //
    // Process LOD
    //
    if (skelmodel->pLOD && r_staticlod->integer) {
        float lod_val;

        lod_val = backEnd.currentStaticModel->lodpercentage[0];

        if (surf->numVerts > 3) {
            skelIndex_t *collapseIndex;
            int          mid, low, high;
            int          lod_cutoff;

            if (lod_tool->integer && !strcmp(backEnd.currentStaticModel->tiki->a->name, lod_tikiname->string)
                && meshNum == lod_mesh->integer) {
                lod_cutoff = GetToolLodCutoff(skelmodel, backEnd.currentStaticModel->lodpercentage[0]);
            } else {
                lod_cutoff = GetLodCutoff(skelmodel, backEnd.currentStaticModel->lodpercentage[0], 0);
            }

            collapseIndex = surf->pCollapseIndex;
            if (collapseIndex[2] < lod_cutoff) {
                return;
            }

            low = mid = 3;
            high      = surf->numVerts;
            while (high >= low) {
                mid = (low + high) >> 1;
                if (collapseIndex[mid] < lod_cutoff) {
                    high = mid - 1;
                    if (collapseIndex[mid - 1] >= lod_cutoff) {
                        break;
                    }
                } else {
                    mid++;
                    low = mid;
                    if (high == mid || collapseIndex[mid] < lod_cutoff) {
                        break;
                    }
                }
            }

            render_count = mid;
        } else {
            render_count = surf->numVerts;
        }

        if (!render_count) {
            return;
        }
    } else {
        render_count = surf->numVerts;
    }

    indexes = surf->numTriangles * 3;
    RB_CHECKOVERFLOW(render_count, surf->numTriangles);

    collapse_map = surf->pCollapse;
    triangles    = surf->pTriangles;
    baseIndex    = tess.numIndexes;
    baseVertex   = tess.numVertexes;
    tess.numVertexes += render_count;

    if (render_count == surf->numVerts) {
        for (j = 0; j < indexes; j++) {
            tess.indexes[baseIndex + j] = baseVertex + triangles[j];
        }

        staticModelNumIndexes[backEnd.currentStaticModel - backEnd.refdef.staticModels] += indexes;
        tess.numIndexes += indexes;
    } else {
        for (i = 0; i < render_count; i++) {
            collapse[i] = i;
        }
        for (i = render_count; i < surf->numVerts; i++) {
            collapse[i] = collapse[collapse_map[i]];
        }

        for (j = 0; j < indexes; j += 3) {
            if (collapse[triangles[j]] == collapse[triangles[j + 1]]
                || collapse[triangles[j + 1]] == collapse[triangles[j + 2]]
                || collapse[triangles[j + 2]] == collapse[triangles[j]]) {
                break;
            }

            tess.indexes[baseIndex + j]     = baseVertex + collapse[triangles[j]];
            tess.indexes[baseIndex + j + 1] = baseVertex + collapse[triangles[j + 1]];
            tess.indexes[baseIndex + j + 2] = baseVertex + collapse[triangles[j + 2]];
        }

        staticModelNumIndexes[backEnd.currentStaticModel - backEnd.refdef.staticModels] += j;
        tess.numIndexes += j;
    }

    for (j = 0; j < render_count; j++) {
        Vector4Copy(surf->pStaticXyz[j], tess.xyz[baseVertex + j]);
        Vector4Copy(surf->pStaticNormal[j], tess.normal[baseVertex + j]);
        tess.texCoords[baseVertex + j][0]   = surf->pStaticTexCoords[j][0][0];
        tess.texCoords[baseVertex + j][1]   = surf->pStaticTexCoords[j][0][1];
        tess.lightCoords[baseVertex + j][0] = surf->pStaticTexCoords[j][1][0];
        tess.lightCoords[baseVertex + j][1] = surf->pStaticTexCoords[j][1][1];
    }

    if (backEndData->staticModelData) {
        const size_t offset =
            backEnd.currentStaticModel->firstVertexData + staticSurf->ofsStaticData * sizeof(color4ub_t);
        assert(offset < tr.world->numStaticModelData * sizeof(color4ub_t));
        assert(offset + render_count * sizeof(color4ub_t) <= tr.world->numStaticModelData * sizeof(color4ub_t));

        const color4ub_t *in = (const color4ub_t *)&backEndData->staticModelData[offset];

        for (i = 0; i < render_count; i++) {
            tess.color[baseVertex + i][0] = in[i][0] * 0xffff / 0xff;
            tess.color[baseVertex + i][1] = in[i][1] * 0xffff / 0xff;
            tess.color[baseVertex + i][2] = in[i][2] * 0xffff / 0xff;
            tess.color[baseVertex + i][3] = in[i][3] * 0xffff / 0xff;
        }
    } else {
        for (i = 0; i < render_count; i++) {
            tess.color[baseVertex + i][0] = 0xffff;
            tess.color[baseVertex + i][1] = 0xffff;
            tess.color[baseVertex + i][2] = 0xffff;
            tess.color[baseVertex + i][3] = 0xffff;
        }
    }
}

/*
=============
R_InfoWorldTris_f
=============
*/
void R_InfoWorldTris_f(void)
{
    int i;

    g_bInfoworldtris = qtrue;

    for (i = 0; i < ARRAY_LEN(entityNumIndexes); i++) {
        entityNumIndexes[i] = 0;
    }
    for (i = 0; i < ARRAY_LEN(staticModelNumIndexes); i++) {
        staticModelNumIndexes[i] = 0;
    }
}

/*
=============
R_PrintInfoWorldtris
=============
*/
void R_PrintInfoWorldtris(void)
{
    int               i;
    int               numTris;
    int               totalNumTris;
    dtiki_t          *tiki;
    skelHeaderGame_t *skelmodel;

    totalNumTris = 0;

    for (i = 0; i < ARRAY_LEN(entityNumIndexes); i++) {
        numTris = entityNumIndexes[i] / 3;
        if (!numTris) {
            continue;
        }

        totalNumTris += numTris;
        tiki      = backEnd.refdef.entities[i].e.tiki;
        skelmodel = ri.TIKI_GetSkel(tiki->mesh[0]);
        Com_Printf("ent: %i, tris: %i, %s, version: %i\n", i, numTris, tiki->a->name, skelmodel->version);
    }

    Com_Printf("total entity tris: %i\n\n", totalNumTris);

    totalNumTris = 0;

    for (i = 0; i < ARRAY_LEN(entityNumIndexes); i++) {
        numTris = staticModelNumIndexes[i] / 3;
        if (!numTris) {
            continue;
        }

        totalNumTris += numTris;
        tiki      = backEnd.refdef.staticModels[i].tiki;
        skelmodel = ri.TIKI_GetSkel(tiki->mesh[0]);
        Com_Printf("sm: %i, tris: %i, %s, version: %i\n", i, numTris, tiki->a->name, skelmodel->version);
    }

    Com_Printf("total static model tris: %i\n\n", totalNumTris);
}

/*
=============
RE_SetFrameNumber
=============
*/
void RE_SetFrameNumber(int frameNumber)
{
    tr.frame_skel_index = frameNumber;
}

/*
=============
R_UpdatePoseInternal
=============
*/
void R_UpdatePoseInternal(refEntity_t *model)
{
    if (model->entityNumber != ENTITYNUM_NONE) {
        if (tr.skel_index[model->entityNumber] == tr.frame_skel_index) {
            return;
        }
        tr.skel_index[model->entityNumber] = tr.frame_skel_index;
    }

    ri.TIKI_SetPoseInternal(
        ri.TIKI_GetSkeletor(model->tiki, model->entityNumber),
        model->frameInfo,
        model->bone_tag,
        model->bone_quat,
        model->actionWeight
    );
}

/*
=============
RE_ForceUpdatePose
=============
*/
void RE_ForceUpdatePose(refEntity_t *model)
{
    if (model->entityNumber != ENTITYNUM_NONE) {
        tr.skel_index[model->entityNumber] = tr.frame_skel_index;
    }

    ri.TIKI_SetPoseInternal(
        ri.TIKI_GetSkeletor(model->tiki, model->entityNumber),
        model->frameInfo,
        model->bone_tag,
        model->bone_quat,
        model->actionWeight
    );
}

/*
=============
RE_TIKI_Orientation
=============
*/
orientation_t RE_TIKI_Orientation(refEntity_t *model, int tagnum)
{
    R_UpdatePoseInternal(model);
    return ri.TIKI_OrientationInternal(model->tiki, model->entityNumber, tagnum, model->scale);
}

/*
=============
RE_TIKI_IsOnGround
=============
*/
qboolean RE_TIKI_IsOnGround(refEntity_t *model, int tagnum, float threshold)
{
    R_UpdatePoseInternal(model);
    return ri.TIKI_IsOnGroundInternal(model->tiki, model->entityNumber, tagnum, threshold);
}

/*
=============
R_GetRadius
=============
*/
float R_GetRadius(refEntity_t *model)
{
    R_UpdatePoseInternal(model);
    return ri.GetRadiusInternal(model->tiki, model->entityNumber, model->scale);
}

/*
=============
R_GetFrame
=============
*/
void R_GetFrame(refEntity_t *model, struct skelAnimFrame_s *newFrame)
{
    R_UpdatePoseInternal(model);
    ri.GetFrameInternal(model->tiki, model->entityNumber, newFrame);
}

/*
=============
R_DebugSkeleton
=============
*/
void R_DebugSkeleton(void)
{
    // FIXME: unimplemented (GL2)
}

/*
=============
ProjectRadius
=============
*/
static float ProjectRadius(float r, const vec3_t location)
{
    Vector separation;
    float  projectedRadius;

    separation      = Vector(tr.viewParms.ori.origin) - Vector(location);
    projectedRadius = separation.length();

    return fabs(r) * (100.0 / tr.viewParms.fovX) / projectedRadius;
}

/*
=============
R_CalcLod
=============
*/
float R_CalcLod(const vec3_t origin, float radius)
{
    return ProjectRadius(radius, origin);
}

/*
=============
R_CullTIKI
=============
*/
static int R_CullSkelModel(dtiki_t *tiki, refEntity_t *e, skelAnimFrame_t *newFrame, float fScale, float *vLocalOrg)
{
    vec3_t bounds[2];
    vec3_t delta;
    int    i;
    int    cull;

    // FIXME: not working properly
    return CULL_IN;

    if (tr.currentEntity->e.renderfx & RF_FRAMELERP) {
        VectorSubtract(e->origin, e->oldorigin, delta);
    } else {
        VectorClear(delta);
    }

    for (i = 0; i < 3; i++) {
        bounds[0][i] = newFrame->bounds[0][i] * fScale + vLocalOrg[i];
        bounds[1][i] = newFrame->bounds[1][i] * fScale + vLocalOrg[i];

        if (delta[i] > 0) {
            bounds[1][i] += delta[i];
        } else {
            bounds[0][i] += delta[i];
        }
    }

    cull = R_CullLocalBox(bounds);

    if (r_showcull->integer & 1) {
        float  fR, fG, fB;
        vec3_t vAngles;

        switch (cull) {
        case CULL_IN:
            fR = 0;
            fG = 1;
            fB = 0;
            break;
        case CULL_CLIP:
            fR = 1;
            fG = 1;
            fB = 0;
            break;
        case CULL_OUT:
            fR = 1;
            fG = 0.2f;
            fB = 0.2f;

            for (i = 0; i < 3; i++) {
                bounds[0][i] -= 16;
                bounds[1][i] += 16;
            }
            break;
        }

        MatrixToEulerAngles(tr.ori.axis, vAngles);
        R_DebugRotatedBBox(tr.ori.origin, vAngles, bounds[0], bounds[1], fR, fG, fB, 0.5);
    }

    switch (cull) {
    case CULL_IN:
        tr.pc.c_box_cull_md3_in++;
        return CULL_IN;
    case CULL_CLIP:
        tr.pc.c_box_cull_md3_clip++;
        return CULL_CLIP;
    case CULL_OUT:
    default:
        tr.pc.c_box_cull_md3_out++;
        return CULL_OUT;
    }
}

/*
==================
R_CountTikiLodTris

Computes the number of triangles to be rendered for the given TIKI model
based on the given LoD percentage, and passes it back with render_tris.
The total number of triangles in the TIKI model are passed back with total_tris.
Currently only used for debugging purposes.

FIXME: Shares some code with RB_StaticMesh, common parts could be extracted.
According to debug symbols, this was originally in tiki_mesh.cpp
==================
*/
void R_CountTikiLodTris(dtiki_t *tiki, float lodpercentage, int *render_tris, int *total_tris)
{
    *render_tris = 0;
    *total_tris  = 0;
    int numtris = 0, totaltris = 0;

    for (int i = 0; i < tiki->numMeshes; i++) {
        skelHeaderGame_t  *skelmodel = ri.TIKI_GetSkel(tiki->mesh[i]);
        skelSurfaceGame_t *surface   = skelmodel->pSurfaces;

        for (int j = 0; j < skelmodel->numSurfaces; j++) {
            int render_count = 0;
            if (skelmodel->pLOD) {
                int          lod_cutoff    = GetLodCutoff(skelmodel, lodpercentage, 0);
                skelIndex_t *collapseIndex = surface->pCollapseIndex;
                render_count               = surface->numVerts;

                // Determine the number of vertices to be rendered based on the LOD cutoff
                while (render_count > 0 && collapseIndex[render_count - 1] < lod_cutoff) {
                    render_count--;
                }
            } else {
                // The surf mesh doesn't have a LOD model, so all vertices will be rendered
                render_count = surface->numVerts;
            }

            skelIndex_t *triangles    = surface->pTriangles;
            int          indexes      = surface->numTriangles * 3; // 3 vertex/index for each tri
            skelIndex_t *collapse_map = surface->pCollapse;
            totaltris += surface->numTriangles;
            skelIndex_t collapse[4096] {};

            // Initialize array for collapsed indices
            int k;
            for (k = 0; k < render_count; ++k) {
                collapse[k] = k;
            }

            // Map remaining vertices to their collapsed indices using the collapse map
            for (k = render_count; k < surface->numVerts; ++k) {
                collapse[k] = collapse[collapse_map[k]];
            }

            // Check the first two collapsed indices of each triangle
            for (k = 0; k < indexes; k += 3) {
                if (collapse[triangles[k]] == collapse[triangles[k + 1]]) {
                    // The collapsed indices of the two vertices are the same:
                    // this, and any subsequent tris do not need to be rendered
                    // because the collapsed vertices (for this surface) coincide from here.
                    break;
                }
            }

            surface = surface->pNext;
            numtris += k / 3;
        }
    }

    *render_tris = numtris;
    *total_tris  = totaltris;
}

/*
==================
R_LerpTag
==================
*/
int R_LerpTag(orientation_t *tag, qhandle_t handle, int startFrame, int endFrame, float frac, const char *tagName)
{
    // stub
    return 0;
}