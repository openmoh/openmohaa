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

#include "cg_local.h"
#include "cg_parsemsg.h"

//============================================================================

/*
=================
CG_CalcVrect

Sets the coordinates of the rendered window
=================
*/
static void CG_CalcVrect(void)
{
    int size;

    // the intermission should allways be full screen
    if (cg.snap->ps.pm_flags & PMF_INTERMISSION) {
        size = 100;
    } else {
        // bound normal viewsize
        if (cg_viewsize->integer < 30) {
            cgi.Cvar_Set("viewsize", "30");
            size = 30;
        } else if (cg_viewsize->integer > 100) {
            cgi.Cvar_Set("viewsize", "100");
            size = 100;
        } else {
            size = cg_viewsize->integer;
        }
    }
    cg.refdef.width = cgs.glconfig.vidWidth * size / 100;
    cg.refdef.width &= ~1;

    cg.refdef.height = cgs.glconfig.vidHeight * size / 100;
    cg.refdef.height &= ~1;

    cg.refdef.x = (cgs.glconfig.vidWidth - cg.refdef.width) / 2;
    cg.refdef.y = (cgs.glconfig.vidHeight - cg.refdef.height) / 2;
}

//==============================================================================

/*
===============
CG_OffsetThirdPersonView

===============
*/
#define CAMERA_MINIMUM_DISTANCE 40

static void CG_OffsetThirdPersonView(void)
{
    vec3_t        forward;
    vec3_t        original_camera_position;
    vec3_t        new_vieworg;
    trace_t       trace;
    vec3_t        min, max;
    float        *look_offset;
    float        *target_angles;
    float        *target_position;
    vec3_t        delta;
    vec3_t        original_angles;
    qboolean      lookactive, resetview;
    static vec3_t saved_look_offset;
    vec3_t        camera_offset;

    target_angles   = cg.refdefViewAngles;
    target_position = cg.refdef.vieworg;

    // see if angles are absolute
    if (cg.predicted_player_state.camera_flags & CF_CAMERA_ANGLES_ABSOLUTE) {
        VectorClear(target_angles);
    }

    // see if we need to ignore yaw
    if (cg.predicted_player_state.camera_flags & CF_CAMERA_ANGLES_IGNORE_YAW) {
        target_angles[YAW] = 0;
    }

    // see if we need to ignore pitch
    if (cg.predicted_player_state.camera_flags & CF_CAMERA_ANGLES_IGNORE_PITCH) {
        target_angles[PITCH] = 0;
    }

    // offset the current angles by the camera offset
    VectorSubtract(target_angles, cg.predicted_player_state.camera_offset, target_angles);

    // Get the position of the camera after any needed rotation
    look_offset = cgi.get_camera_offset(&lookactive, &resetview);

    if ((!resetview) && ((cg.predicted_player_state.camera_flags & CF_CAMERA_ANGLES_ALLOWOFFSET) || (lookactive))) {
        VectorSubtract(look_offset, saved_look_offset, camera_offset);
        VectorAdd(target_angles, camera_offset, target_angles);
        if (target_angles[PITCH] > 90) {
            target_angles[PITCH] = 90;
        } else if (target_angles[PITCH] < -90) {
            target_angles[PITCH] = -90;
        }
    } else {
        VectorCopy(look_offset, saved_look_offset);
    }

    target_angles[YAW]   = AngleNormalize360(target_angles[YAW]);
    target_angles[PITCH] = AngleNormalize180(target_angles[PITCH]);

    // Move reference point up

    target_position[2] += cg_cameraheight->value;

    VectorCopy(target_position, original_camera_position);

    // Move camera back from reference point

    AngleVectors(target_angles, forward, NULL, NULL);

    VectorMA(target_position, -cg_cameradist->value, forward, new_vieworg);

    new_vieworg[2] += cg_cameraverticaldisplacement->value;

    // Create a bounding box for our camera

    min[0] = -5;
    min[1] = -5;
    min[2] = -5;

    max[0] = 5;
    max[1] = 5;
    max[2] = 5;

    // Make sure camera does not collide with anything
    CG_Trace(&trace, cg.playerHeadPos, min, max, new_vieworg, 0, MASK_CAMERASOLID, qfalse, qtrue, "ThirdPersonTrace 1");

    VectorCopy(trace.endpos, target_position);

    // calculate distance from end position to head position
    VectorSubtract(target_position, cg.playerHeadPos, delta);
    // kill any negative z difference in delta
    if (delta[2] < CAMERA_MINIMUM_DISTANCE) {
        delta[2] = 0;
    }
    if (VectorLength(delta) < CAMERA_MINIMUM_DISTANCE) {
        VectorNormalize(delta);
        /*
      // see if we are going straight up
      if ( ( delta[ 2 ] > 0.75 ) && ( height > 0.85f * cg.predicted_player_state.viewheight ) )
         {
         // we just need to lower our start position slightly, since we are on top of the player
         new_vieworg[ 2 ] -= 16;
	      CG_Trace(&trace, cg.playerHeadPos, min, max, new_vieworg, 0, MASK_CAMERASOLID, qfalse, true, "ThirdPersonTrace 2" );
	      VectorCopy(trace.endpos, target_position);
         }
      else
*/
        {
            // we are probably up against the wall so we want the camera to pitch up on top of the player
            // save off the original angles
            VectorCopy(target_angles, original_angles);
            // start cranking up the target angles, pitch until we are the correct distance away from the player
            while (target_angles[PITCH] < 90) {
                target_angles[PITCH] += 2;

                AngleVectors(target_angles, forward, NULL, NULL);

                VectorMA(original_camera_position, -cg_cameradist->value, forward, new_vieworg);

                new_vieworg[2] += cg_cameraverticaldisplacement->value;

                CG_Trace(
                    &trace,
                    cg.playerHeadPos,
                    min,
                    max,
                    new_vieworg,
                    0,
                    MASK_CAMERASOLID,
                    qfalse,
                    qtrue,
                    "ThirdPersonTrace 3"
                );

                VectorCopy(trace.endpos, target_position);

                // calculate distance from end position to head position
                VectorSubtract(target_position, cg.playerHeadPos, delta);
                // kill any negative z difference in delta
                if (delta[2] < 0) {
                    delta[2] = 0;
                }
                if (VectorLength(delta) >= CAMERA_MINIMUM_DISTANCE) {
                    target_angles[PITCH] = (0.25f * target_angles[PITCH]) + (0.75f * original_angles[PITCH]);
                    // set the pitch to be that of the angle we are currently looking
                    //target_angles[ PITCH ] = original_angles[ PITCH ];
                    break;
                }
            }
            if (target_angles[PITCH] > 90) {
                // if we failed, go with the original angles
                target_angles[PITCH] = original_angles[PITCH];
            }
        }
    }
}

/*
===============
CG_OffsetFirstPersonView

===============
*/
void CG_OffsetFirstPersonView(refEntity_t *pREnt, qboolean bUseWorldPosition)
{
    float     *origin;
    centity_t *pCent;
    dtiki_t   *tiki;
    int        iTag;
    int        i;
    int        iMask;
    vec3_t     vDelta;
    float      mat[3][3];
    vec3_t     vOldOrigin;
    vec3_t     vStart, vEnd, vMins, vMaxs;
    trace_t    trace;

    VectorSet(vMins, -6, -6, -6);
    VectorSet(vMaxs, 6, 6, 6);

    //
    //
    //
    //
    origin = cg.refdef.vieworg;

    pCent = &cg_entities[cg.predicted_player_state.clientNum];

    tiki = cgi.R_Model_GetHandle(cgs.model_draw[pCent->currentState.modelindex]);
    iTag = cgi.Tag_NumForName(tiki, "eyes bone");
    if (iTag != -1) {
        if (bUseWorldPosition) {
            orientation_t oHead;
            float         mat3[3][3];
            vec3_t        vHeadAng, vDelta;

            VectorCopy(pCent->lerpOrigin, origin);
            AxisCopy(pREnt->axis, mat);
            oHead = cgi.TIKI_Orientation(pREnt, iTag);

            for (i = 0; i < 3; i++) {
                VectorMA(origin, oHead.origin[i], mat[i], origin);
            }

            R_ConcatRotations(oHead.axis, mat, mat3);
            MatrixToEulerAngles(mat3, vHeadAng);
            AnglesSubtract(vHeadAng, cg.refdefViewAngles, vDelta);
            VectorMA(cg.refdefViewAngles, cg.fEyeOffsetFrac, vDelta, cg.refdefViewAngles);
            VectorCopy(vHeadAng, cg.refdefViewAngles);
        } else {
            orientation_t oHead;
            vec3_t        vHeadAng;

            VectorCopy(pCent->lerpOrigin, origin);
            AxisCopy(pREnt->axis, mat);
            MatrixToEulerAngles(mat, vHeadAng);
            oHead = cgi.TIKI_Orientation(pREnt, iTag);

            for (i = 0; i < 3; i++) {
                VectorMA(origin, oHead.origin[i], mat[i], origin);
            }

            cg.refdefViewAngles[2] += cg.predicted_player_state.fLeanAngle * 0.3;
        }
    } else {
        cgi.DPrintf("CG_OffsetFirstPersonView warning: Couldn't find 'eyes bone' for player\n");
    }

    VectorCopy(origin, vOldOrigin);

    if (bUseWorldPosition) {
        iMask = MASK_VIEWSOLID;
    } else {
        float  fTargHeight;
        float  fHeightDelta, fHeightChange;
        float  fPhase, fVel;
        vec3_t vDelta;
        vec3_t vPivotPoint;
        vec3_t vForward, vLeft;

        origin[0]    = cg.predicted_player_state.origin[0];
        origin[1]    = cg.predicted_player_state.origin[1];
        fTargHeight  = cg.predicted_player_state.origin[2] + cg.predicted_player_state.viewheight;
        fHeightDelta = fTargHeight - cg.fCurrentViewHeight;

        if (fabs(fHeightDelta) < 0.1 || !cg.fCurrentViewHeight) {
            cg.fCurrentViewHeight = fTargHeight;
        } else {
            if (fHeightDelta > 32.f) {
                fHeightDelta          = 32.f;
                cg.fCurrentViewHeight = fTargHeight - 32.0;
            } else if (fHeightDelta < -32.f) {
                fHeightDelta          = -32.f;
                cg.fCurrentViewHeight = fTargHeight + 32.0;
            }

            fHeightChange = cg.frametime / 1000.0 * fHeightDelta * 12.5;
            if (!cg.predicted_player_state.walking) {
                fHeightChange += fHeightChange;
            }

            if (fabs(fHeightDelta) < fabs(fHeightChange)) {
                fHeightChange = fHeightDelta;
            }

            cg.fCurrentViewHeight += fHeightChange;
        }

        origin[2]      = cg.fCurrentViewHeight;
        vPivotPoint[0] = cg.refdefViewAngles[0];
        vPivotPoint[1] = cg.refdefViewAngles[1];
        vPivotPoint[2] = 0.0;
        AngleVectorsLeft(vPivotPoint, vForward, vLeft, NULL);

        VectorCopy(origin, vStart);

        if (cg.predicted_player_state.pm_type != PM_CLIMBWALL) {
            if (cg.refdefViewAngles[0] > 0.0) {
                vStart[2] -= (cg.fCurrentViewHeight - cg.predicted_player_state.origin[2]) * 0.4;
            } else {
                vStart[2] -= (cg.fCurrentViewHeight - cg.predicted_player_state.origin[2]) * 0.2;
            }
        } else {
            vStart[2] -= (cg.fCurrentViewHeight - cg.predicted_player_state.origin[2]) * 0.15;
        }

        VectorSubtract(origin, vStart, vDelta);
        RotatePointAroundVector(vEnd, vLeft, vDelta, cg.refdefViewAngles[0] * 0.4);
        VectorAdd(vStart, vEnd, origin);

        if (cg.predicted_player_state.fLeanAngle) {
            VectorCopy(origin, vStart);
            vStart[2] -= 28.7f;

            VectorSubtract(origin, vStart, vDelta);
            RotatePointAroundVector(vEnd, vForward, vDelta, cg.predicted_player_state.fLeanAngle);
            VectorAdd(vStart, vEnd, origin);
        }

        if (cg.predicted_player_state.walking) {
            fVel   = VectorLength(cg.predicted_player_state.velocity);
            fPhase = fVel * 0.0015 + 0.9;
            cg.fCurrentViewBobPhase += (cg.frametime / 1000.0 + cg.frametime / 1000.0) * M_PI * fPhase;

            if (cg.fCurrentViewBobAmp) {
                cg.fCurrentViewBobAmp = fVel;
            } else {
                cg.fCurrentViewBobAmp = fVel * 0.5;
            }

            if (cg.predicted_player_state.fLeanAngle != 0.0) {
                cg.fCurrentViewBobAmp *= 0.75;
            }

            cg.fCurrentViewBobAmp *= (1.0 - fabs(cg.refdefViewAngles[0]) * (1.0 / 90.0) * 0.5) * 0.5;
        } else if (cg.fCurrentViewBobAmp > 0.0) {
            cg.fCurrentViewBobAmp -=
                (cg.frametime / 1000.0 * cg.fCurrentViewBobAmp) + (cg.frametime / 1000.0 * cg.fCurrentViewBobAmp);

            if (cg.fCurrentViewBobAmp < 0.1) {
                cg.fCurrentViewBobAmp = 0.0;
            }
        }

        if (cg.fCurrentViewBobAmp > 0.0) {
            fPhase = sin(cg.fCurrentViewBobPhase) * cg.fCurrentViewBobAmp * 0.03;

            if (fPhase > 16.0) {
                fPhase = 16.0;
            } else if (fPhase < -16.0) {
                fPhase = -16.0;
            }

            VectorMA(origin, fPhase, vLeft, origin);

            fPhase = sin(cg.fCurrentViewBobPhase - 0.94);
            fPhase = (fabs(fPhase) - 0.5) * cg.fCurrentViewBobAmp * 0.06;

            if (fPhase > 16.0) {
                fPhase = 16.0;
            } else if (fPhase < -16.0) {
                fPhase = -16.0;
            }

            origin[2] += fPhase;
        }

        iMask = MASK_PLAYERSOLID;
    }

    vStart[0] = cg.predicted_player_state.origin[0];
    vStart[1] = cg.predicted_player_state.origin[1];
    vStart[2] = cg.predicted_player_state.origin[2] + cg.predicted_player_state.viewheight;
    vEnd[0]   = cg.predicted_player_state.origin[0];
    vEnd[1]   = cg.predicted_player_state.origin[1];
    vEnd[2]   = origin[2];

    CG_Trace(
        &trace, vStart, vMins, vMaxs, vEnd, cg.snap->ps.clientNum, iMask, qfalse, qtrue, "FirstPerson Height Check"
    );

    VectorCopy(trace.endpos, vStart);
    vEnd[0] = origin[0];
    vEnd[1] = origin[1];
    vEnd[2] = trace.endpos[2];
    CG_Trace(&trace, vStart, vMins, vMaxs, vEnd, cg.snap->ps.clientNum, iMask, 0, 1, "FirstPerson Lateral Check");

    VectorCopy(trace.endpos, origin);
    VectorSubtract(origin, vOldOrigin, vDelta);
    VectorAdd(pREnt->origin, vDelta, pREnt->origin);

    if (!bUseWorldPosition) {
        VectorCopy(cg.refdefViewAngles, vDelta);
        vDelta[0] *= 0.5;
        vDelta[2] *= 0.75;

        AngleVectorsLeft(vDelta, mat[0], mat[1], mat[2]);

        CG_CalcViewModelMovement(
            cg.fCurrentViewBobPhase, cg.fCurrentViewBobAmp, cg.predicted_player_state.velocity, vDelta
        );

        VectorMA(pREnt->origin, vDelta[0], mat[0], pREnt->origin);
        VectorMA(pREnt->origin, vDelta[1], mat[1], pREnt->origin);
        VectorMA(pREnt->origin, vDelta[2], mat[2], pREnt->origin);
    }

    VectorCopy(origin, cg.playerHeadPos);
}

/*
====================
CG_CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
#define WAVE_AMPLITUDE 1
#define WAVE_FREQUENCY 0.4

static int CG_CalcFov(void)
{
    float x;
    float phase;
    float v;
    int   contents;
    float fov_x, fov_y;
    int   inwater;

    fov_x = cg.camera_fov;
    x     = cg.refdef.width / tan(fov_x / 360 * M_PI);
    fov_y = atan2(cg.refdef.height, x);
    fov_y = fov_y * 360 / M_PI;

    // warp if underwater
    contents = CG_PointContents(cg.refdef.vieworg, -1);
    if (contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA)) {
        phase = cg.time / 1000.0 * WAVE_FREQUENCY * M_PI * 2;
        v     = WAVE_AMPLITUDE * sin(phase);
        fov_x += v;
        fov_y -= v;
        inwater = qtrue;
    } else {
        inwater = qfalse;
    }

    // set it
    cg.refdef.fov_x    = fov_x;
    cg.refdef.fov_y    = fov_y;
    cg.fRefFovXCos     = cos(fov_x / 114.0f);
    cg.fRefFovXSin     = sin(fov_x / 114.0f);
    cg.fRefFovYCos     = cos(fov_y / 114.0f);
    cg.fRefFovYSin     = sin(fov_y / 114.0f);
    cg.zoomSensitivity = cg.refdef.fov_y / 75.0;
    return inwater;
}

/*
===============
CG_SetupFog

Prepares fog values for rendering
===============
*/
void CG_SetupFog() {
    cg.refdef.farplane_distance = cg.farplane_distance;
    cg.refdef.farplane_bias = cg.farplane_bias;
    cg.refdef.farplane_color[0] = cg.farplane_color[0];
    cg.refdef.farplane_color[1] = cg.farplane_color[1];
    cg.refdef.farplane_color[2] = cg.farplane_color[2];
    cg.refdef.farplane_cull = cg.farplane_cull;
    cg.refdef.skybox_farplane = cg.skyboxFarplane;
    cg.refdef.renderTerrain = cg.renderTerrain;
    cg.refdef.farclipOverride = cg.farclipOverride;
    cg.refdef.farplaneColorOverride[0] = cg.farplaneColorOverride[0];
    cg.refdef.farplaneColorOverride[1] = cg.farplaneColorOverride[1];
    cg.refdef.farplaneColorOverride[2] = cg.farplaneColorOverride[2];
}

/*
===============
CG_SetupPortalSky

Sets portalsky values for rendering
===============
*/
void CG_SetupPortalSky() {
    cg.refdef.sky_alpha = cg.sky_alpha;
    cg.refdef.sky_portal = cg.sky_portal;
    VectorCopy(cg.sky_axis[0], cg.refdef.sky_axis[0]);
    VectorCopy(cg.sky_axis[1], cg.refdef.sky_axis[1]);
    VectorCopy(cg.sky_axis[2], cg.refdef.sky_axis[2]);
    VectorMA(cg.sky_origin, cg.skyboxSpeed, cg.refdef.vieworg, cg.refdef.sky_origin);
}

/*
===============
CG_CalcViewValues

Sets cg.refdef view values
===============
*/
static int CG_CalcViewValues(void)
{
    playerState_t *ps;
    float          SoundAngles[3];

    memset(&cg.refdef, 0, sizeof(cg.refdef));

    // calculate size of 3D view
    CG_CalcVrect();
    CG_SetupFog();

    // setup fog and far clipping plane
    cg.refdef.farplane_distance = cg.farplane_distance;
    VectorCopy(cg.farplane_color, cg.refdef.farplane_color);
    cg.refdef.farplane_cull = cg.farplane_cull;

    // setup portal sky
    cg.refdef.sky_alpha  = cg.sky_alpha;
    cg.refdef.sky_portal = cg.sky_portal;
    memcpy(cg.refdef.sky_axis, cg.sky_axis, sizeof(cg.sky_axis));
    VectorCopy(cg.sky_origin, cg.refdef.sky_origin);

    ps = &cg.predicted_player_state;

    VectorCopy(ps->origin, cg.refdef.vieworg);
    VectorCopy(ps->viewangles, cg.refdefViewAngles);

    if (cg.snap->ps.stats[STAT_HEALTH] > 0) {
        VectorSubtract(cg.refdefViewAngles, cg.predicted_player_state.damage_angles, cg.refdefViewAngles);
        cg.refdefViewAngles[0] += cg.viewkick[0];
        cg.refdefViewAngles[1] += cg.viewkick[1];

        if (cg.viewkick[0] || cg.viewkick[1]) {
            int   i;
            float fDecay;

            for (i = 0; i < 2; i++) {
                fDecay = cg.viewkick[i] * cg.viewkickRecenter;
                if (fDecay > cg.viewkickMaxDecay) {
                    fDecay = cg.viewkickMaxDecay;
                } else if (fDecay < -cg.viewkickMaxDecay) {
                    fDecay = -cg.viewkickMaxDecay;
                }

                if (fabs(fDecay) < cg.viewkickMinDecay) {
                    if (fDecay > 0.0) {
                        fDecay = cg.viewkickMinDecay;
                    } else {
                        fDecay = -cg.viewkickMinDecay;
                    }
                }

                if (cg.viewkick[i] > 0.0) {
                    cg.viewkick[i] -= fDecay * (float)cg.frametime / 1000.0;
                    if (cg.viewkick[i] < 0.0) {
                        cg.viewkick[i] = 0.0;
                    }
                } else {
                    cg.viewkick[i] -= fDecay * (float)cg.frametime / 1000.0;
                    if (cg.viewkick[i] > 0.0) {
                        cg.viewkick[i] = 0.0;
                    }
                }
            }
        }
    }

    // FIXME: fffx screen shake on win32 builds?

    // add error decay
    if (cg_errorDecay->value > 0) {
        int   t;
        float f;

        t = cg.time - cg.predictedErrorTime;
        f = (cg_errorDecay->value - t) / cg_errorDecay->value;
        if (f > 0 && f < 1) {
            VectorMA(cg.refdef.vieworg, f, cg.predictedError, cg.refdef.vieworg);
        } else {
            cg.predictedErrorTime = 0;
        }
    }

    // calculate position of player's head
    cg.refdef.vieworg[2] += cg.predicted_player_state.viewheight;
    // save off the position of the player's head
    VectorCopy(cg.refdef.vieworg, cg.playerHeadPos);

    // Set the aural position of the player
    VectorCopy(cg.playerHeadPos, cg.SoundOrg);

    // Set the aural axis of the player
    VectorCopy(cg.refdefViewAngles, SoundAngles);
    // yaw is purposely inverted because of the miles sound system
    // Commented out in OPM
    //  Useless as SDL audio/AL is used
    //SoundAngles[YAW] = -SoundAngles[YAW];
    AnglesToAxis(SoundAngles, cg.SoundAxis);

    // decide on third person view
    cg.renderingThirdPerson = cg_3rd_person->integer;

    if (cg.renderingThirdPerson) {
        // back away from character
        CG_OffsetThirdPersonView();
    }

    // if we are in a camera view, we take our audio cues directly from the camera
    if (ps->pm_flags & PMF_CAMERA_VIEW) {
        // Set the aural position to that of the camera
        VectorCopy(cg.camera_origin, cg.refdef.vieworg);

        // Set the aural axis to the camera's angles
        VectorCopy(cg.camera_angles, cg.refdefViewAngles);

        if (cg_protocol >= PROTOCOL_MOHTA_MIN && (ps->pm_flags & PMF_DAMAGE_ANGLES)) {
            // Handle camera shake
            VectorSubtract(cg.refdefViewAngles, cg.predicted_player_state.damage_angles, cg.refdefViewAngles);
        }

        if (ps->camera_posofs[0] || ps->camera_posofs[1] || ps->camera_posofs[2]) {
            vec3_t vAxis[3], vOrg;
            AnglesToAxis(cg.refdefViewAngles, vAxis);
            MatrixTransformVector(ps->camera_posofs, vAxis, vOrg);
            VectorAdd(cg.refdef.vieworg, vOrg, cg.refdef.vieworg);
        }

        // copy view values
        VectorCopy(cg.refdef.vieworg, cg.currentViewPos);
        VectorCopy(cg.refdefViewAngles, cg.currentViewAngles);
        // since 2.0: also copy location data for sound
        VectorCopy(cg.refdef.vieworg, cg.SoundOrg);
        AnglesToAxis(cg.refdefViewAngles, cg.SoundAxis);
    }

    // position eye reletive to origin
    AnglesToAxis(cg.refdefViewAngles, cg.refdef.viewaxis);

    if (cg.hyperspace) {
        cg.refdef.rdflags |= RDF_NOWORLDMODEL | RDF_HYPERSPACE;
    }

    // field of view
    return CG_CalcFov();
}

void CG_EyePosition(vec3_t *o_vPos)
{
    (*o_vPos)[0] = cg.playerHeadPos[0];
    (*o_vPos)[1] = cg.playerHeadPos[1];
    (*o_vPos)[2] = cg.playerHeadPos[2];
}

void CG_EyeOffset(vec3_t *o_vOfs)
{
    (*o_vOfs)[0] = cg.playerHeadPos[0] - cg.predicted_player_state.origin[0];
    (*o_vOfs)[1] = cg.playerHeadPos[1] - cg.predicted_player_state.origin[1];
    (*o_vOfs)[2] = cg.playerHeadPos[2] - cg.predicted_player_state.origin[2];
}

void CG_EyeAngles(vec3_t *o_vAngles)
{
    (*o_vAngles)[0] = cg.refdefViewAngles[0];
    (*o_vAngles)[1] = cg.refdefViewAngles[1];
    (*o_vAngles)[2] = cg.refdefViewAngles[2];
}

float CG_SensitivityScale()
{
    return cg.zoomSensitivity;
}

void CG_AddLightShow()
{
    // FIXME: unimplemented
}

qboolean CG_FrustumCullSphere(const vec3_t vPos, float fRadius) {
    vec3_t delta;
    float fDotFwd, fDotSide, fDotUp;

    VectorSubtract(vPos, cg.refdef.vieworg, delta);

    fDotFwd = DotProduct(delta, cg.refdef.viewaxis[0]);
    if (-fRadius >= fDotFwd) {
        return qtrue;
    }

    if (cg.refdef.farplane_distance && cg.refdef.farplane_distance + fRadius <= fDotFwd) {
        return qtrue;
    }

    fDotSide = DotProduct(delta, cg.refdef.viewaxis[1]);
    if (fDotSide < 1.f) {
        fDotSide = -fDotSide;
    }

    if (fDotSide * cg.fRefFovXCos - fDotFwd * cg.fRefFovXSin >= fRadius) {
        return qtrue;
    }

    fDotUp = DotProduct(delta, cg.refdef.viewaxis[2]);
    if (fDotUp < 0.f) {
        fDotUp = -fDotUp;
    }

    if (fDotUp * cg.fRefFovYCos - fDotFwd * cg.fRefFovYSin >= fRadius) {
        return qtrue;
    }

    return qfalse;
}

//=========================================================================

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.
=================
*/
void CG_DrawActiveFrame(int serverTime, int frameTime, stereoFrame_t stereoView, qboolean demoPlayback)
{
    cg.time         = serverTime;
    cg.frametime    = frameTime;
    cg.demoPlayback = demoPlayback;

    // any looped sounds will be respecified as entities
    // are added to the render list
    cgi.S_ClearLoopingSounds();

    // clear all the render lists
    cgi.R_ClearScene();

    // set up cg.snap and possibly cg.nextSnap
    CG_ProcessSnapshots();

    // if we haven't received any snapshots yet, all
    // we can draw is the information screen
    if (!cg.snap || (cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE)) {
        return;
    }

    // this counter will be bumped for every valid scene we generate
    cg.clientFrame++;

    // set cg.frameInterpolation
    if (cg.nextSnap && r_lerpmodels->integer) {
        int delta;

        delta = (cg.nextSnap->serverTime - cg.snap->serverTime);
        if (delta == 0) {
            cg.frameInterpolation = 0;
        } else {
            cg.frameInterpolation = (float)(cg.time - cg.snap->serverTime) / delta;
        }
    } else {
        cg.frameInterpolation = 0; // actually, it should never be used, because
        // no entities should be marked as interpolating
    }

    // update cg.predicted_player_state
    CG_PredictPlayerState();

    // build cg.refdef
    CG_CalcViewValues();

    // display the intermission
    if (cg.snap->ps.pm_flags & PMF_INTERMISSION) {
        if (cgs.gametype != GT_SINGLE_PLAYER) {
            CG_ScoresDown_f();
        } else if (cg.bIntermissionDisplay) {
            if (cg.nextSnap) {
                if (cg_protocol >= PROTOCOL_MOHTA_MIN) {
                    cvar_t* pMission = cgi.Cvar_Get("g_mission", "", CVAR_ARCHIVE);

                    if (cgi.Cvar_Get("g_success", "", 0)->integer) {
                        switch (pMission->integer)
                        {
                        default:
                        case 0:
                            cgi.UI_ShowMenu("mission_success_1", 0);
                            cgi.Cvar_Set("g_t2l1", "1");
                            break;
                        case 2:
                            cgi.UI_ShowMenu("mission_success_2", 0);
                            cgi.Cvar_Set("g_t3l1", "1");
                            break;
                        case 3:
                            cgi.UI_ShowMenu("mission_success_3", 0);
                            break;
                        }
                    } else {
                        switch (pMission->integer)
                        {
                        default:
                        case 0:
                            cgi.UI_ShowMenu("mission_failed_1", 0);
                            break;
                        case 2:
                            cgi.UI_ShowMenu("mission_failed_2", 0);
                            break;
                        case 3:
                            cgi.UI_ShowMenu("mission_failed_3", 0);
                            break;
                        }
                    }
                } else {
                    if (cgi.Cvar_Get("g_success", "", 0)->integer) {
                        cgi.UI_ShowMenu("StatsScreen_Success", qfalse);
                    } else {
                        cgi.UI_ShowMenu("StatsScreen_Failed", qfalse);
                    }
                }
            }
        } else {
            cgi.SendClientCommand("stats");
        }

        cg.bIntermissionDisplay = qtrue;
    } else if (cg.bIntermissionDisplay) {
        if (cgs.gametype != GT_SINGLE_PLAYER) {
            CG_ScoresUp_f();
        } else {
            if (cg_protocol >= PROTOCOL_MOHTA_MIN) {
                cvar_t* pMission = cgi.Cvar_Get("g_mission", "", CVAR_ARCHIVE);

                if (cgi.Cvar_Get("g_success", "", 0)->integer) {
                    switch (pMission->integer)
                    {
                    default:
                    case 0:
                        cgi.UI_HideMenu("mission_success_1", CVAR_ARCHIVE);
                        break;
                    case 2:
                        cgi.UI_HideMenu("mission_success_2", CVAR_ARCHIVE);
                        break;
                    case 3:
                        cgi.UI_HideMenu("mission_success_3", CVAR_ARCHIVE);
                        break;
                    }
                } else {
                    switch (pMission->integer)
                    {
                    default:
                    case 0:
                        cgi.UI_HideMenu("mission_failed_1", CVAR_ARCHIVE);
                        break;
                    case 2:
                        cgi.UI_HideMenu("mission_failed_2", CVAR_ARCHIVE);
                        break;
                    case 3:
                        cgi.UI_HideMenu("mission_failed_3", CVAR_ARCHIVE);
                        break;
                    }
                }
            } else {
                if (cgi.Cvar_Get("g_success", "", 0)->integer) {
                    cgi.UI_HideMenu("StatsScreen_Success", qfalse);
                } else {
                    cgi.UI_HideMenu("StatsScreen_Failed", qfalse);
                }
            }
        }

        cg.bIntermissionDisplay = qfalse;
    }

    // build the render lists
    if (!cg.hyperspace) {
        CG_AddPacketEntities(); // after calcViewValues, so predicted player state is correct
        CG_AddMarks();
    }

    // finish up the rest of the refdef
    CG_SetupPortalSky();

    cg.refdef.time = cg.time;
    memcpy(cg.refdef.areamask, cg.snap->areamask, sizeof(cg.refdef.areamask));

    // update audio positions
    cgi.S_Respatialize(cg.snap->ps.clientNum, cg.SoundOrg, cg.SoundAxis);

    // make sure the lagometerSample and frame timing isn't done twice when in stereo
    if (stereoView != STEREO_RIGHT) {
        CG_AddLagometerFrameInfo();
    }

    CG_UpdateTestEmitter();
    CG_AddPendingEffects();

    if (!cg_hidetempmodels->integer) {
        CG_AddTempModels();
    }

    if (vss_draw->integer) {
        CG_AddVSSSources();
    }

    CG_AddBulletTracers();
    CG_AddBulletImpacts();
    CG_AddBeams();

    if (cg_acidtrip->integer) {
        // lol disco
        CG_AddLightShow();
    }

    // actually issue the rendering calls
    CG_DrawActive(stereoView);

    if (cg_stats->integer) {
        cgi.Printf("cg.clientFrame:%i\n", cg.clientFrame);
    }
}
