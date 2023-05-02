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

// DESCRIPTION:
// Functions for doing model animation and attachments

#include "cg_local.h"
#include "tiki.h"

void CG_PlayerTeamUpdate(refEntity_t* pModel, entityState_t* pPlayerState)
{
    // FIXME: unimplemented
}

/*
===============
CG_EntityShadow

Returns the Z component of the surface being shadowed

  should it return a full plane instead of a Z?
===============
*/
#define  SHADOW_DISTANCE      96
qboolean CG_EntityShadow( centity_t *cent, refEntity_t *model ) {
   vec3_t      end;
   trace_t     trace;
   float       alpha, radius;

   if ( cg_shadows->integer == 0 ) {
      return qfalse;
   }

   if ( model->renderfx & RF_SKYENTITY )
      {
      // no shadows on sky entities
      return qfalse;
      }

   // no shadows when invisible
//   if ( cent->currentState.powerups & ( 1 << PW_INVIS ) ) {
//      return qfalse;
//   }

   // send a trace down from the player to the ground
   VectorCopy( model->origin, end );
   end[2] -= SHADOW_DISTANCE;

   cgi.CM_BoxTrace( &trace, model->origin, end, vec3_origin, vec3_origin, 0, MASK_PLAYERSOLID, qfalse );

   // no shadow if too high
   if ( trace.fraction == 1.0 ) {
      return qfalse;
   }

   if ( ( cg_shadows->integer == 2 ) && ( model->renderfx & RF_SHADOW_PRECISE ) )
      {
      return qtrue;
      }

   //
   // get the bounds of the current frame
   //
   radius = model->scale * cgi.R_ModelRadius( model->hModel );

   if ( radius < 1 )
      {
      return qfalse;
      }

   // fade the shadow out with height
   alpha = ( 1.0 - trace.fraction ) * 0.8f;

   if ( ( cg_shadows->integer == 3 ) && ( model->renderfx & RF_SHADOW_PRECISE ) )
      {
      if ( model->shaderRGBA[ 3 ] == 255 )
         {
         model->shaderRGBA[ 3 ] = alpha * 255;
         }
      return qtrue;
      }

   // add the mark as a temporary, so it goes directly to the renderer
   // without taking a spot in the cg_marks array
   CG_ImpactMark(cgs.media.shadowMarkShader, trace.endpos,
                 trace.plane.normal, cent->lerpAngles[YAW], alpha, alpha,
                 alpha, 1, qfalse, radius, qtrue, -1, qfalse, 0.f, 0.f);

   return qtrue;
}

//
//
// NEW ANIMATION AND THREE PART MODEL SYSTEM
//
//

//=================
//CG_AnimationDebugMessage
//=================
void CG_AnimationDebugMessage
   (
   int            number,
   const char     *fmt, 
   ...
   )
   {
#ifndef NDEBUG
   if ( cg_debugAnim->integer ) 
      {
	   va_list		argptr;
	   char		msg[1024];

	   va_start (argptr,fmt);
	   vsprintf (msg,fmt,argptr);
	   va_end (argptr);

      if ( ( !cg_debugAnimWatch->integer ) || ( ( cg_debugAnimWatch->integer - 1 ) == number ) )
         {
         if ( cg_debugAnim->integer == 2 ) 
            {
 		      cgi.DebugPrintf( msg );
            }
         else
            {
 		      cgi.Printf( msg );
            }
         }
	   }
#endif
   }

/*
======================
CG_AttachEntity

Modifies the entities position and axis by the given
tag location
======================
*/
void CG_AttachEntity( refEntity_t *entity, refEntity_t *parent, dtiki_t* tiki, int tagnum, qboolean use_angles, vec3_t attach_offset )
{
    int				i;
    orientation_t   or;
    vec3_t			tempAxis[3];
    vec3_t          vOrigin;

    or = cgi.TIKI_Orientation(parent, tagnum);
    //cgi.Printf( "th = %d %.2f %.2f %.2f\n", tikihandle, or.origin[ 0 ], or.origin[ 1 ], or.origin[ 2 ] );

    VectorCopy(parent->origin, entity->origin);

    for (i = 0; i < 3; i++)
    {
        VectorMA(entity->origin, or.origin[i], parent->axis[i], entity->origin);
    }

    if (attach_offset[0] || attach_offset[1] || attach_offset[2])
    {
        MatrixMultiply(or.axis, parent->axis, tempAxis);

        for (i = 0; i < 3; i++) {
            VectorMA(entity->origin, attach_offset[i], tempAxis[i], entity->origin);
        }
    }

    VectorCopy(entity->origin, entity->oldorigin);

    if (use_angles)
    {
        MatrixMultiply(entity->axis, parent->axis, tempAxis);
        MatrixMultiply(or.axis, tempAxis, entity->axis);
    }

    entity->scale *= parent->scale;
    entity->renderfx |= (parent->renderfx & ~(RF_FLAGS_NOT_INHERITED | RF_LIGHTING_ORIGIN));

    MatrixTransformVectorRight(entity->axis, entity->lightingOrigin, vOrigin);
    VectorAdd(entity->lightingOrigin, vOrigin, entity->lightingOrigin);
}

void CG_AttachEyeEntity(refEntity_t* entity, refEntity_t* parent, dtiki_t* tiki, int tagnum, qboolean use_angles, vec_t* attach_offset)
{
    // FIXME: unimplemented
}

void CG_UpdateForceModels()
{
    // FIXME: unimplemented
}

void CG_ModelAnim(centity_t* cent, qboolean bDoShaderTime)
{
    entityState_t* s1;
    entityState_t* sNext = NULL;
    refEntity_t model;
    int i;
    vec3_t vMins, vMaxs, vTmp;
    const char* szTagName;
    static cvar_t* vmEntity = NULL;
    int iAnimFlags;

    s1 = &cent->currentState;

    if ((cg.snap->ps.pm_flags & PMF_INTERMISSION)
        && s1->number == cg.snap->ps.clientNum
        && !cg_3rd_person->integer)
    {
        // don't render if in intermission and the client is self without 3rd person
        return;
    }

    memset(&model, 0, sizeof(model));

    if (!vmEntity) {
        vmEntity = cgi.Cvar_Get("viewmodelanim", "1", 0);
    }

    if (cent->interpolate) {
        sNext = &cent->nextState;
    }

    // add loop sound only if it is not attached
    if (s1->loopSound && (s1->parent == ENTITYNUM_NONE))
    {
        cgi.S_AddLoopingSound(
            cent->lerpOrigin,
            vec3_origin,
            cgs.sound_precache[s1->loopSound],
            s1->loopSoundVolume,
            s1->loopSoundMinDist,
            s1->loopSoundMaxDist,
            s1->loopSoundPitch,
            s1->loopSoundFlags
        );
    }

    if (cent->tikiLoopSound && (s1->parent == ENTITYNUM_NONE))
        cgi.S_AddLoopingSound(
            cent->lerpOrigin,
            vec3_origin,
            cent->tikiLoopSound,
            cent->tikiLoopSoundVolume,
            cent->tikiLoopSoundMinDist,
            cent->tikiLoopSoundMaxDist,
            cent->tikiLoopSoundPitch,
            cent->tikiLoopSoundFlags
        );

    // if set to invisible, skip
    if (!s1->modelindex)
    {
        return;
    }

    // set the entity number
    model.entityNumber = s1->number;

    // take the results of CL_InterpolateEntities
    VectorCopy(cent->lerpOrigin, model.origin);
    VectorCopy(cent->lerpOrigin, model.oldorigin);

    IntegerToBoundingBox(s1->solid, vMins, vMaxs);
    // calculate the light origin
    VectorAdd(vMins, vMaxs, vTmp);
    VectorMA(model.origin, 0.5, vTmp, model.lightingOrigin);
    // calculate the radius
    VectorSubtract(vMins, vMaxs, vTmp);
    model.radius = VectorLength(vTmp) * 0.5;

    if (s1->number == cg.snap->ps.clientNum)
    {
        if (!cg_3rd_person->integer) {
            PmoveAdjustAngleSettings_Client(
                cg.refdefViewAngles,
                cent->lerpAngles,
                &cg.predicted_player_state,
                &cent->currentState
            );
        }

        model.bone_quat = s1->bone_quat;
        model.bone_tag = s1->bone_tag;
    }
    else
    {
        for (i = 0; i < NUM_BONE_CONTROLLERS; i++)
        {
            if (s1->bone_tag[i] >= 0)
            {
                if ((cent->interpolate) && (cent->nextState.bone_tag[i] == s1->bone_tag[i]))
                {
                    SlerpQuaternion(
                        s1->bone_quat[i],
                        cent->nextState.bone_quat[i],
                        cg.frameInterpolation,
                        cent->bone_quat[i]
                    );
                }
                else
                {
                    cent->bone_quat[i][0] = s1->bone_quat[i][0];
                    cent->bone_quat[i][1] = s1->bone_quat[i][1];
                    cent->bone_quat[i][2] = s1->bone_quat[i][2];
                    cent->bone_quat[i][3] = s1->bone_quat[i][3];
                }
            }
        }

        model.bone_quat = cent->bone_quat;
        model.bone_tag = s1->bone_tag;
    }

    // convert angles to axis
    AnglesToAxis(cent->lerpAngles, model.axis);

    // copy shader specific data
    if (s1->shader_data[0]) {
        model.shader_data[0] = s1->shader_data[0];
    }
    else {
        model.shader_data[0] = s1->tag_num;
    }

    if (s1->shader_data[1]) {
        model.shader_data[1] = s1->shader_data[1];
    }
    else {
        model.shader_data[1] = s1->skinNum;
    }

    if (bDoShaderTime)
    {
        if (cent->interpolate) {
            model.shaderTime = s1->shader_time
                     + (sNext->shader_time - s1->shader_time) * cg.frameInterpolation
                     + cg.time / 1000.0;
        }
        else {
            model.shaderTime = cg.time / 1000.0 + s1->shader_time;
        }
    }

    // Interpolated state variables
    if (cent->interpolate)
    {
        model.scale = s1->scale + cg.frameInterpolation * (cent->nextState.scale - s1->scale);
    }
    else
    {
        model.scale = s1->scale;
    }

    model.hOldModel = 0;
    model.tiki = cgi.R_Model_GetHandle(cgs.model_draw[s1->modelindex]);

    if (s1->number != cg.snap->ps.clientNum
        && (s1->eType == ET_PLAYER || (s1->eFlags & EF_DEAD)))
    {
        if (cg_forceModel->integer)
        {
            CG_UpdateForceModels();

            if (s1->eFlags & EF_AXIS)
            {
                model.hModel = cg.hAxisPlayerModelHandle;
                model.tiki = cg.pAxisPlayerModel;
            }
            else
            {
                model.hModel = cg.hAlliedPlayerModelHandle;
                model.tiki = cg.pAlliedPlayerModel;
            }

            if (model.hModel && model.tiki)
            {
                model.hOldModel = cgs.model_draw[s1->modelindex];
            }
            else
            {
                // fallback to non-forced model
                model.tiki = cgi.R_Model_GetHandle(cgs.model_draw[s1->modelindex]);
                model.hModel = cgs.model_draw[s1->modelindex];
            }
        }
        else
        {
            model.hModel = cgs.model_draw[s1->modelindex];
        }

        if (!model.hModel || !model.tiki)
        {
            // Use a model in case it still doesn't exist
            if (s1->eFlags & EF_AXIS) {
                model.hModel = cgi.R_RegisterModel("models/player/german_wehrmacht_soldier.tik");
            }
            else {
                model.hModel = cgi.R_RegisterModel("models/player/american_army.tik");
            }
            model.tiki = cgi.R_Model_GetHandle(model.hModel);
            model.hOldModel = cgs.model_draw[s1->modelindex];
        }
    }
    else
    {
        model.hModel = cgs.model_draw[s1->modelindex];
    }

    if (!model.tiki) {
        // still no model
        return;
    }

    // set skin
    model.skinNum = s1->skinNum;
    model.renderfx |= s1->renderfx;
    cgi.TIKI_SetEyeTargetPos(model.tiki, model.entityNumber, s1->eyeVector);

    if (sNext && sNext->usageIndex == s1->usageIndex) {
        float t;
        float animLength;
        float t1, t2;

        t = (cg.time - cg.snap->serverTime) / (cg.nextSnap->serverTime - cg.snap->serverTime);
        model.actionWeight = (sNext->actionWeight - s1->actionWeight) * t + s1->actionWeight;

        for (i = 0; i < MAX_FRAMEINFOS; i++)
        {
            if (sNext->frameInfo[i].weight)
            {
                model.frameInfo[i].index = sNext->frameInfo[i].index;
                if (sNext->frameInfo[i].index == s1->frameInfo[i].index && s1->frameInfo[i].weight != 0.0)
                {
                    model.frameInfo[i].weight = (sNext->frameInfo[i].weight - s1->frameInfo[i].weight) * t
                        + s1->frameInfo[i].weight;
                    if (sNext->frameInfo[i].time >= s1->frameInfo[i].time)
                    {
                        model.frameInfo[i].time = (sNext->frameInfo[i].time - s1->frameInfo[i].time) * t
                            + s1->frameInfo[i].time;
                    }
                    else
                    {
                        animLength = cgi.Anim_Time(model.tiki, sNext->frameInfo[i].index);
                        if (!animLength) {
                            t1 = 0.0;
                        }
                        else {
                            t1 = (animLength + sNext->frameInfo[i].time - s1->frameInfo[i].time) * t + cg.time;
                        }

                        t2 = t1;
                        while (t2 > animLength) {
                            t2 -= animLength;
                        }

                        model.frameInfo[i].time = t2;
                    }
                }
                else
                {
                    animLength = cgi.Anim_Time(model.tiki, sNext->frameInfo[i].index);
                    if (!animLength) {
                        t1 = 0.0;
                    }
                    else {
                        t1 = sNext->frameInfo[i].time - (cg.nextSnap->serverTime - cg.time) / 1000.0;
                    }

                    t2 = t1;
                    if (t2 < 0.0) {
                        t2 = 0.0;
                    }

                    model.frameInfo[i].time = t2;
                    model.frameInfo[i].weight = sNext->frameInfo[i].weight;
                }
            }
            else if (sNext->frameInfo[i].index == s1->frameInfo[i].index) {
                animLength = cgi.Anim_Time(model.tiki, sNext->frameInfo[i].index);
                if (!animLength) {
                    t1 = 0.0;
                }
                else {
                    t1 = (cg.time - cg.snap->serverTime) / 1000.0 + s1->frameInfo[i].time;
                }

                t2 = t1;
                if (t2 > animLength) {
                    t2 = animLength;
                }

                model.frameInfo[i].index = s1->frameInfo[i].index;
                model.frameInfo[i].time = t2;
                model.frameInfo[i].weight = (1.0 - t) * s1->frameInfo[i].weight;
            }
            else {
                model.frameInfo[i].index = -1;
                model.frameInfo[i].weight = 0.0;
            }
        }
    }
    else
    {
        // no next state, don't blend anims

        model.actionWeight = s1->actionWeight;
        for (i = 0; i < MAX_FRAMEINFOS; i++)
        {
            if (s1->frameInfo[i].weight)
            {
                model.frameInfo[i].index = s1->frameInfo[i].index;
                model.frameInfo[i].time = s1->frameInfo[i].time;
                model.frameInfo[i].weight = s1->frameInfo[i].weight;
            }
            else
            {
                model.frameInfo[i].index = -1;
                model.frameInfo[i].weight = 0.0;
            }
        }
    }

    if (vmEntity->integer == s1->number)
    {
        static cvar_t* curanim;
        if (!curanim) {
            curanim = cgi.Cvar_Get("viewmodelanimslot", "1", 0);
        }

        cgi.Cvar_Set("viewmodelanimclienttime", va("%0.2f", model.frameInfo[curanim->integer].time));
    }

    if (cent->currentState.parent != ENTITYNUM_NONE)
    {
        int iTagNum;
        refEntity_t* parent;
        dtiki_t* tiki;

        parent = cgi.R_GetRenderEntity(cent->currentState.parent);
        if (!parent)
        {
            if (developer->integer > 1) {
                cgi.DPrintf("CG_ModelAnim: Could not find parent entity\n");
            }

            return;
        }

        if (s1->parent != cg.snap->ps.clientNum || cg_3rd_person->integer)
        {
            // attach the model to the world model
            if (parent->hOldModel)
            {
                tiki = cgi.R_Model_GetHandle(parent->hOldModel);
                szTagName = cgi.Tag_NameForNum(tiki, s1->tag_num & TAG_MASK);
                tiki = cgi.R_Model_GetHandle(parent->hModel);
                iTagNum = cgi.Tag_NumForName(tiki, szTagName);
            }
            else
            {
                tiki = cgi.R_Model_GetHandle(parent->hModel);
                iTagNum = s1->tag_num;
            }

            CG_AttachEntity(
                &model,
                parent,
                tiki,
                iTagNum & TAG_MASK,
                s1->attach_use_angles,
                s1->attach_offset
            );
        }
        else
        {
            tiki = cg.pPlayerFPSModel;

            // attach to the first person model
            if (cg.pLastPlayerWorldModel) {
                szTagName = cgi.Tag_NameForNum(cg.pLastPlayerWorldModel, s1->tag_num & TAG_MASK);
            }
            else {
                szTagName = cgi.Tag_NameForNum(tiki, s1->tag_num & TAG_MASK);
            }

            if (!Q_stricmp(szTagName, "eyes bone"))
            {
                iTagNum = cgi.Tag_NumForName(tiki, szTagName);
                CG_AttachEyeEntity(
                    &model,
                    parent,
                    tiki,
                    iTagNum & TAG_MASK,
                    s1->attach_use_angles,
                    s1->attach_offset
                );
            }
            else if (!Q_stricmp(szTagName, "tag_weapon_right") || !Q_stricmp(szTagName, "tag_weapon_left")) {
                iTagNum = cgi.Tag_NumForName(tiki, szTagName);
                CG_AttachEntity(
                    &model,
                    parent,
                    tiki,
                    iTagNum & TAG_MASK,
                    s1->attach_use_angles,
                    s1->attach_offset
                );
            }
        }

        if (s1->loopSound)
        {
            cgi.S_AddLoopingSound(
                model.origin,
                vec3_origin,
                cgs.sound_precache[s1->loopSound],
                s1->loopSoundVolume,
                s1->loopSoundMinDist,
                s1->loopSoundMaxDist,
                s1->loopSoundPitch,
                s1->loopSoundFlags
            );
        }

        if (cent->tikiLoopSound) {
            cgi.S_AddLoopingSound(
                cent->lerpOrigin,
                vec3_origin,
                cent->tikiLoopSound,
                cent->tikiLoopSoundVolume,
                cent->tikiLoopSoundMinDist,
                cent->tikiLoopSoundMaxDist,
                cent->tikiLoopSoundPitch,
                cent->tikiLoopSoundFlags
            );
        }

        // set the attached model to have the same render FX
        model.renderfx &= ~(RF_THIRD_PERSON | RF_THIRD_PERSON | RF_DEPTHHACK);
        model.renderfx |= parent->renderfx & (RF_THIRD_PERSON | RF_THIRD_PERSON | RF_DEPTHHACK);
    }

    for (i = 0; i < 3; i++)
    {
        model.shaderRGBA[i] = cent->color[i] * 255;
    }
    model.shaderRGBA[3] = s1->alpha * 255;

    // set surfaces
    memcpy(model.surfaces, s1->surfaces, MAX_MODEL_SURFACES);

    if (!(s1->renderfx & RF_VIEWMODEL)
        && s1->parent != ENTITYNUM_NONE && s1->parent == cg.snap->ps.clientNum
        && ((!cg_drawviewmodel->integer && !cg_3rd_person->integer) || cg.snap->ps.stats[STAT_INZOOM]))
    {
        // hide all surfaces while zooming or if the viewmodel shouldn't be shown
        for (i = 0; i < MAX_MODEL_SURFACES; i++)
        {
            model.surfaces[i] |= MDL_SURFACE_NODRAW;
        }
    }

    if (!(s1->renderfx & RF_DONTDRAW) && (model.renderfx & RF_SHADOW)) {
        // add the shadow
        CG_EntityShadow(cent, &model);
    }

    iAnimFlags = 0;

    // combine anim flags from all frame infos
    for (i = 0; i < MAX_FRAMEINFOS; i++)
    {
        if (model.frameInfo[i].weight && model.frameInfo[i].index >= 0)
        {
            iAnimFlags |= cgi.Anim_Flags(model.tiki, model.frameInfo[i].index);
        }
    }

    if (iAnimFlags & TAF_AUTOFOOTSTEPS)
    {
        int iTagNum;
        // Automatically calculate the footsteps sounds

        if (cent->bFootOnGround_Right)
        {
            iTagNum = cgi.Tag_NumForName(model.tiki, "Bip01 R Foot");
            if (iTagNum >= 0)
            {
                cent->bFootOnGround_Right = cgi.TIKI_IsOnGround(&model, iTagNum, 13.653847f);
            }
            else {
                cent->bFootOnGround_Right = qtrue;
            }
        }
        else
        {
            iTagNum = cgi.Tag_NumForName(model.tiki, "Bip01 R Foot");
            if (iTagNum >= 0)
            {
                if (cgi.TIKI_IsOnGround(&model, iTagNum, 13.461539f))
                {
                    CG_Footstep("Bip01 R Foot", cent, &model, (iAnimFlags >> 11) & 1, (iAnimFlags >> 12) & 1);
                    cent->bFootOnGround_Right = qtrue;
                }
            }
            else {
                cent->bFootOnGround_Right = qtrue;
            }
        }

        if (cent->bFootOnGround_Left)
        {
            iTagNum = cgi.Tag_NumForName(model.tiki, "Bip01 R Foot");
            if (iTagNum >= 0)
            {
                cent->bFootOnGround_Left = cgi.TIKI_IsOnGround(&model, iTagNum, 13.653847f);
            }
            else {
                cent->bFootOnGround_Left = qtrue;
            }
        }
        else
        {
            iTagNum = cgi.Tag_NumForName(model.tiki, "Bip01 R Foot");
            if (iTagNum >= 0)
            {
                if (cgi.TIKI_IsOnGround(&model, iTagNum, 13.461539f))
                {
                    CG_Footstep("Bip01 L Foot", cent, &model, (iAnimFlags >> 11) & 1, (iAnimFlags >> 12) & 1);
                    cent->bFootOnGround_Left = qtrue;
                }
            }
            else {
                cent->bFootOnGround_Left = qtrue;
            }
        }
    }
    else {
        cent->bFootOnGround_Left = qtrue;
        cent->bFootOnGround_Right = qtrue;
    }

    if (cent->currentState.eType == ET_PLAYER && !(cent->currentState.eFlags & EF_DEAD)) {
        CG_PlayerTeamUpdate(&model, &cent->currentState);
    }

    if (s1->number == cg.snap->ps.clientNum)
    {
        if ((!cg.bFPSModelLastFrame && !cg_3rd_person->integer)
            || (cg.bFPSModelLastFrame && cg_3rd_person->integer))
        {
            // reset the animations when toggling 3rd person
            for (i = 0; i < MAX_FRAMEINFOS; i++)
            {
                cent->animLast[i] = -1;
            }

            cent->animLastWeight = 0;
            cent->usageIndexLast = 0;

            cg.bFPSModelLastFrame = !cg_3rd_person->integer;
        }

        // player footsteps, walking/falling
        if (cg.bFPSOnGround != cg.predicted_player_state.walking)
        {
            cg.bFPSOnGround = cg.predicted_player_state.walking;
            if (cg.predicted_player_state.walking) {
                CG_LandingSound(cent, &model, 1.0, 1);
            }
            else
            {
                if (cent->iNextLandTime < cg.time) {
                    CG_Footstep(0, cent, &model, 1, 1);
                }

                cent->iNextLandTime = cg.time + 200;
            }
        }

        if (!cg_3rd_person->integer)
        {
            // first person view

            if (!(cg.predicted_player_state.pm_flags & PMF_CAMERA_VIEW)
                && (cg.snap->ps.stats[STAT_HEALTH] <= 0 || cg_animationviewmodel->integer))
            {
                // use world position for this case
                CG_OffsetFirstPersonView(&model, qtrue);
            }

            if (!cg.pLastPlayerWorldModel || cg.pLastPlayerWorldModel != model.tiki)
            {
                qhandle_t hModel;
                char fpsname[128];
                COM_StripExtension(model.tiki->a->name, fpsname, sizeof(fpsname));

                hModel = cgi.R_RegisterModel(fpsname);
                if (hModel)
                {
                    cg.hPlayerFPSModelHandle = hModel;
                    cg.pPlayerFPSModel = cgi.R_Model_GetHandle(hModel);
                    if (!cg.pPlayerFPSModel) {
                        cg.pPlayerFPSModel = model.tiki;
                    }
                }
                else
                {
                    if (cg.snap->ps.stats[STAT_TEAM] == TEAM_AXIS) {
                        hModel = cgi.R_RegisterModel("models/player/german_wehrmacht_soldier_fps.tik");
                    }
                    else {
                        hModel = cgi.R_RegisterModel("models/player/american_army_fps.tik");
                    }

                    if (hModel)
                    {
                        cg.hPlayerFPSModelHandle = hModel;
                        cg.pPlayerFPSModel = cgi.R_Model_GetHandle(hModel);

                        if (!cg.pPlayerFPSModel) {
                            cg.pPlayerFPSModel = model.tiki;
                        }
                    }
                    else {
                        cg.hPlayerFPSModelHandle = cgs.model_draw[s1->modelindex];
                        cg.pPlayerFPSModel = model.tiki;
                    }
                }

                cg.pLastPlayerWorldModel = model.tiki;
            }

            model.tiki = cg.pPlayerFPSModel;
            model.hModel = cg.hPlayerFPSModelHandle;
            memset(model.surfaces, 0, sizeof(model.surfaces));

            CG_ViewModelAnimation(&model);
            cgi.ForceUpdatePose(&model);

            if ((cent->currentState.eFlags & EF_UNARMED)
                || cg_drawviewmodel->integer <= 1
                || cg.snap->ps.stats[STAT_INZOOM]
                || cg.snap->ps.stats[STAT_HEALTH])
            {
                // unarmed or zooming, hide the arms
                for (i = 0; i < MAX_MODEL_SURFACES; i++)
                {
                    model.surfaces[i] |= MDL_SURFACE_NODRAW;
                }
            }
            else
            {
                // show/hide the garand hand depending if it's a rifle or not
                // so the hand can hold the rifle correctly

                const char* weaponstring = "";
                int iSurfaceNum;

                if (cg.snap->ps.activeItems[1] >= 0) {
                    weaponstring = CG_ConfigString(CS_WEAPONS + cg.snap->ps.activeItems[1]);
                }

                if (!Q_stricmp(weaponstring, "M1 Garand")
                    || !Q_stricmp(weaponstring, "Springfield '03 Sniper")
                    || !Q_stricmp(weaponstring, "Mauser KAR 98K")
                    || !Q_stricmp(weaponstring, "KAR98 - Sniper"))
                {
                    // show the garand hands

                    iSurfaceNum = cgi.Surface_NameToNum(model.tiki, "lefthand");
                    if (iSurfaceNum >= 0) {
                        model.surfaces[iSurfaceNum] |= MDL_SURFACE_NODRAW;
                    }

                    iSurfaceNum = cgi.Surface_NameToNum(model.tiki, "garandhand");
                    if (iSurfaceNum >= 0) {
                        model.surfaces[iSurfaceNum] &= ~MDL_SURFACE_NODRAW;
                    }
                }
                else
                {

                    // hide the garand hands

                    iSurfaceNum = cgi.Surface_NameToNum(model.tiki, "garandhand");
                    if (iSurfaceNum >= 0) {
                        model.surfaces[iSurfaceNum] |= MDL_SURFACE_NODRAW;
                    }

                    iSurfaceNum = cgi.Surface_NameToNum(model.tiki, "lefthand");
                    if (iSurfaceNum >= 0) {
                        model.surfaces[iSurfaceNum] &= ~MDL_SURFACE_NODRAW;
                    }
                }
            }

            if (!(s1->eFlags & EF_CLIMBWALL)) {
                // when the player is not climbing ladders show the entity
                model.renderfx |= RF_DEPTHHACK;
            }

            if (!(cg.predicted_player_state.pm_flags & PMF_CAMERA_VIEW))
            {
                if (cg.snap->ps.stats[STAT_HEALTH] > 0 && !cg_animationviewmodel->integer) {
                    CG_OffsetFirstPersonView(&model, qfalse);
                }

                AnglesToAxis(cg.refdefViewAngles, cg.refdef.viewaxis);
            }

            model.renderfx &= ~(RF_FIRST_PERSON | RF_THIRD_PERSON);
            // set the first person render flag
            model.renderfx |= RF_FIRST_PERSON;
        }
    }

    if (model.renderfx & RF_SKYORIGIN)
    {
        memcpy(cg.sky_axis, model.axis, sizeof(cg.sky_axis));
        VectorCopy(model.origin, cg.sky_origin);
    }

    model.reType = ET_MODELANIM_SKEL;
    if (!(s1->renderfx & RF_DONTDRAW))
    {
        cgi.R_Model_GetHandle(model.hModel);
        if (VectorCompare(model.origin, vec3_origin))
        {
            model.origin[0] = s1->origin[0];
            model.origin[1] = s1->origin[1];
            model.origin[2] = s1->origin[2];
            AngleVectors(s1->angles, model.axis[0], model.axis[1], model.axis[2]);
        }

        // add to refresh list
        cgi.R_AddRefEntityToScene(&model, s1->parent);
    }

    CG_UpdateEntityEmitters(s1->number, &model, cent);

    if (s1->usageIndex == cent->usageIndexLast)
    {
        // process the exit commands of the last animations
        for (i = 0; i < MAX_FRAMEINFOS; i++)
        {
            if ((cent->animLastWeight >> i) & 1)
            {
                if (!model.frameInfo[i].weight || model.frameInfo[i].index != cent->animLast[i])
                {
                    CG_ProcessEntityCommands(TIKI_FRAME_EXIT, cent->animLast[i], s1->number, &model, cent);
                }
            }
        }
    }

    for (i = 0; i < MAX_FRAMEINFOS; i++)
    {
        // process the entry commands of the current anim
        if (model.frameInfo[i].weight)
        {
            if (!((cent->animLastWeight >> i) & 1) || model.frameInfo[i].index != cent->animLast[i])
            {
                CG_ProcessEntityCommands(TIKI_FRAME_ENTRY, model.frameInfo[i].index, s1->number, &model, cent);
                cent->animLastTimes[i] = 0.0;
            }

            CG_ClientCommands(&model, cent, i);
        }

        cent->animLastTimes[i] = model.frameInfo[i].time;
        cent->animLast[i] = model.frameInfo[i].index;

        if (model.frameInfo[i].weight) {
            cent->animLastWeight |= 1 << i;
        }
        else {
            cent->animLastWeight &= ~(1 << i);
        }
    }

    cent->usageIndexLast = cent->currentState.usageIndex;
}
