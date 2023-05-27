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

#include "../tr_local.h"

cvar_t* r_reset_tc_array;
cvar_t* r_numdebuglines;


cvar_t* r_staticlod;
cvar_t* r_lodscale;
cvar_t* r_lodcap;
cvar_t* r_lodviewmodelcap;

cvar_t* r_uselod;
cvar_t* lod_LOD;
cvar_t* lod_minLOD;
cvar_t* lod_maxLOD;
cvar_t* lod_LOD_slider;
cvar_t* lod_curve_0_val;
cvar_t* lod_curve_1_val;
cvar_t* lod_curve_2_val;
cvar_t* lod_curve_3_val;
cvar_t* lod_curve_4_val;
cvar_t* lod_edit_0;
cvar_t* lod_edit_1;
cvar_t* lod_edit_2;
cvar_t* lod_edit_3;
cvar_t* lod_edit_4;
cvar_t* lod_curve_0_slider;
cvar_t* lod_curve_1_slider;
cvar_t* lod_curve_2_slider;
cvar_t* lod_curve_3_slider;
cvar_t* lod_curve_4_slider;
cvar_t* lod_pitch_val;
cvar_t* lod_zee_val;
cvar_t* lod_mesh;
cvar_t* lod_meshname;
cvar_t* lod_tikiname;
cvar_t* lod_metric;
cvar_t* lod_tris;
cvar_t* lod_position;
cvar_t* lod_save;
cvar_t* lod_tool;

cvar_t* r_showSkeleton;

int r_sequencenumber;

//=============================================================================

/*
** RE_BeginRegistration
*/
void RE_BeginRegistration(glconfig_t* glconfigOut) {
    int	i;

    R_Init();

    *glconfigOut = glConfig;

    R_IssuePendingRenderCommands();

    tr.visIndex = 0;
    // force markleafs to regenerate
    for (i = 0; i < MAX_VISCOUNTS; i++) {
        tr.visClusters[i] = -2;
    }

    R_ClearFlares();
    RE_ClearScene();

    tr.registered = qtrue;
}

const char* RE_GetGraphicsInfo() {
    // FIXME: unimplemented
    return NULL;
}

qboolean R_SetMode(int mode, const glconfig_t* glConfig) {
    // FIXME: unimplemented
    return qfalse;
}

void R_SetFullscreen(qboolean fullscreen) {
    // FIXME: unimplemented
}

void RE_SetRenderTime(int t) {
    // FIXME: unimplemented
}

void GetRefAPI_new(refimport_t* ri, refexport_t* re)
{
    re->FreeModels = RE_FreeModels;
    re->SpawnEffectModel = RE_SpawnEffectModel;
    re->RegisterServerModel = RE_RegisterServerModel;
    re->UnregisterServerModel = RE_UnregisterServerModel;
    re->RefreshShaderNoMip = RE_RefreshShaderNoMip;
    re->PrintBSPFileSizes = RE_PrintBSPFileSizes;
    re->MapVersion = RE_MapVersion;
    re->LoadFont = R_LoadFont;

    re->BeginFrame = RE_BeginFrame;
    
    re->MarkFragments = R_MarkFragments_New;
    re->MarkFragmentsForInlineModel = R_MarkFragmentsForInlineModel;
    re->GetInlineModelBounds = R_GetInlineModelBounds;
    re->GetLightingForDecal = R_GetLightingForDecal;
    re->GetLightingForSmoke = R_GetLightingForSmoke;
    re->R_GatherLightSources = R_GatherLightSources;
    re->ModelBounds = R_ModelBounds;
    re->ModelRadius = R_ModelRadius;

    re->AddRefEntityToScene = RE_AddRefEntityToScene;
    re->AddRefSpriteToScene = RE_AddRefSpriteToScene;
    re->AddPolyToScene = RE_AddPolyToScene;
    re->AddTerrainMarkToScene = RE_AddTerrainMarkToScene;
    re->AddLightToScene = RE_AddLightToScene;
    re->RenderScene = RE_RenderScene;
    re->GetRenderEntity = RE_GetRenderEntity;

    re->SavePerformanceCounters = R_SavePerformanceCounters;

    re->R_Model_GetHandle = R_Model_GetHandle;
    re->DrawStretchRaw = RE_StretchRaw;
    re->DebugLine = R_DebugLine;
    re->DrawStretchPic = Draw_StretchPic;
    re->DrawTilePic = Draw_TilePic;
    re->DrawTilePicOffset = Draw_TilePicOffset;
    re->DrawTrianglePic = Draw_TrianglePic;
    re->DrawBox = DrawBox;
    re->AddBox = AddBox;
    re->Set2DWindow = Set2DWindow;
    re->Scissor = RE_Scissor;
    re->DrawLineLoop = DrawLineLoop;
    re->DrawString = R_DrawString;
    re->GetFontHeight = R_GetFontHeight;
    re->GetFontStringWidth = R_GetFontStringWidth;
    re->SwipeBegin = RE_SwipeBegin;
    re->SwipeEnd = RE_SwipeEnd;
    re->SetRenderTime = RE_SetRenderTime;
    re->Noise = R_NoiseGet4f;

    re->SetMode = R_SetMode;
    re->SetFullscreen = R_SetFullscreen;

    re->GetShaderHeight = RE_GetShaderHeight;
    re->GetShaderWidth = RE_GetShaderWidth;
    re->GetGraphicsInfo = RE_GetGraphicsInfo;
    re->ForceUpdatePose = RE_ForceUpdatePose;
    re->TIKI_Orientation = RE_TIKI_Orientation;
    re->TIKI_IsOnGround = RE_TIKI_IsOnGround;
    re->SetFrameNumber = RE_SetFrameNumber;
}