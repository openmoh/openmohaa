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
// Temporary models effects

#include "cg_local.h"
#include "cg_commands.h"
#include "tiki.h"

cvar_t *cg_showtempmodels;
cvar_t *cg_max_tempmodels;
cvar_t *cg_reserve_tempmodels;
cvar_t *cg_detail;
cvar_t *cg_effectdetail;
cvar_t *cg_effect_physicsrate;

extern refEntity_t *current_entity;
extern int          current_entity_number;
extern centity_t   *current_centity;
extern float        current_scale;
extern dtiki_t     *current_tiki;

extern Event EV_Client_Swipe;
extern Event EV_Client_SwipeOn;
extern Event EV_Client_SwipeOff;

//=============
// AllocateTempModel
//=============
ctempmodel_t *ClientGameCommandManager::AllocateTempModel(void)
{
    ctempmodel_t *p;

    p = m_free_tempmodels;
    if (!p) {
        // no free entities
        return NULL;
    }

    m_free_tempmodels = m_free_tempmodels->next;

    // link into the active list
    p->next                        = m_active_tempmodels.next;
    p->prev                        = &m_active_tempmodels;
    m_active_tempmodels.next->prev = p;
    m_active_tempmodels.next       = p;

    return p;
}

//===============
// FreeTempModel
//===============
void ClientGameCommandManager::FreeTempModel(ctempmodel_t *p)
{
    if (!p->prev) {
        cgi.Error(ERR_DROP, "CCM::FreeTempModel: not active");
    }

    RemoveClientEntity(p->number, p->cgd.tiki, NULL, p);

    // remove from the doubly linked active list
    p->prev->next = p->next;
    p->next->prev = p->prev;

    // the free list is only singly linked
    p->next           = m_free_tempmodels;
    m_free_tempmodels = p;
}

//===============
// FreeAllTempModels
//===============
void ClientGameCommandManager::FreeAllTempModels(void)
{
    ctempmodel_t *p, *next;

    // Go through all the temp models and run the physics if necessary,
    // then add them to the ref
    p = m_active_tempmodels.prev;
    for (; p != &m_active_tempmodels; p = next) {
        // grab next now, so if the local entity is freed we still have it
        next = p->prev;
        FreeTempModel(p);
    }
}

//===============
// FreeSomeTempModels
//===============
void ClientGameCommandManager::FreeSomeTempModels(void)
{
    ctempmodel_t* model;
    int count = 0;
    unsigned int i;
    unsigned int numToFree;

    if (!m_free_tempmodels) {
        return;
    }

    for (model = m_active_tempmodels.prev; model != &m_active_tempmodels; model = model->prev) {
        count++;
    }

    if (cg_reserve_tempmodels->integer <= (cg_max_tempmodels->integer - count)) {
        // nothing to free
        return;
    }

    numToFree = cg_reserve_tempmodels->integer - (cg_max_tempmodels->integer - count);

    for (i = 0; i < numToFree; i++) {
        FreeTempModel(m_active_tempmodels.prev);
    }
}

//===============
// FreeSpawnthing
//===============
void ClientGameCommandManager::FreeSpawnthing(spawnthing_t* sp)
{
    ctempmodel_t* model;
    ctempmodel_t* prev;

    if (sp->numtempmodels) {
        for (model = m_active_tempmodels.prev; model != &m_active_tempmodels; model = prev) {
            prev = model->prev;

            if (model->m_spawnthing == sp) {
                FreeTempModel(model);
            }
        }
    } else {
        m_emitters.RemoveObject(sp);

        if (sp == m_spawnthing) {
            m_spawnthing = NULL;
        }

        delete sp;
    }
}

//===============
// ResetTempModels
//===============
void ClientGameCommandManager::ResetTempModels(void)
{
    // Go through all the active tempmodels and free them
    ctempmodel_t *p, *next;

    p = m_active_tempmodels.prev;
    for (; p != &m_active_tempmodels; p = next) {
        next = p->prev;
        FreeTempModel(p);
    }
}

static int lastTempModelFrameTime = 0;

void CG_ResetTempModels(void)
{
    commandManager.ResetTempModels();
    lastTempModelFrameTime = cg.time;
}

//=============
// InitializeTempModels
//=============
void ClientGameCommandManager::InitializeTempModels(void)
{
    int i;
    int numtempmodels = MAX_TEMPMODELS;

    m_active_tempmodels.next = &m_active_tempmodels;
    m_active_tempmodels.prev = &m_active_tempmodels;

    m_free_tempmodels = &m_tempmodels[0];

    for (i = 0; i < numtempmodels - 1; i++) {
        m_tempmodels[i].next = &m_tempmodels[i + 1];
    }
    m_tempmodels[numtempmodels - 1].next = NULL;
}

void ClientGameCommandManager::InitializeTempModelCvars(void)
{
    cg_showtempmodels     = cgi.Cvar_Get("cg_showtempmodels", "0", 0);
    cg_detail             = cgi.Cvar_Get("detail", "1", CVAR_ARCHIVE);
    cg_effectdetail       = cgi.Cvar_Get("cg_effectdetail", "0.2", CVAR_ARCHIVE);
    cg_effect_physicsrate = cgi.Cvar_Get("cg_effect_physicsrate", "10", CVAR_ARCHIVE);
    cg_max_tempmodels     = cgi.Cvar_Get("cg_max_tempmodels", "1100", CVAR_ARCHIVE);
    cg_reserve_tempmodels = cgi.Cvar_Get("cg_reserve_tempmodels", "200", CVAR_ARCHIVE);

    if (cg_max_tempmodels->integer > MAX_TEMPMODELS) {
        // 2.40 sets the integer value directly rather than calling Cvar_Set()
        //cg_max_tempmodels->integer = MAX_TEMPMODELS
        cgi.Cvar_Set("cg_max_tempmodels", va("%i", MAX_TEMPMODELS));
    }

    if (cg_reserve_tempmodels->integer * 5 > cg_max_tempmodels->integer) {
        // 2.40 sets the integer value directly rather than calling Cvar_Set()
        //cg_reserve_tempmodels->integer = cg_max_tempmodels->integer / 5;
        cgi.Cvar_Set("cg_reserve_tempmodels", va("%i", cg_max_tempmodels->integer / 5));
    }
}

//===============
// AnimateTempModel - animate temp models
//===============
void ClientGameCommandManager::AnimateTempModel(ctempmodel_t *p, Vector origin, refEntity_t *newEnt)
{
    int numframes;
    int deltatime;
    int frametime;

    // This code is for animating tempmodels that are spawned from the client
    // side

    if (!p->cgd.tiki) {
        return;
    }

    // Calc frame stuff
    frametime = 1000.0f * cgi.Anim_Frametime(p->cgd.tiki, p->ent.frameInfo[0].index);
    deltatime = cg.time - p->lastAnimTime;
    numframes = cgi.Anim_NumFrames(p->cgd.tiki, p->ent.frameInfo[0].index);

    if (!p->addedOnce) {
        // Process entry commands
        CG_ProcessEntityCommands(TIKI_FRAME_ENTRY, p->ent.frameInfo[0].index, -1, &p->ent, NULL);
    }

    if (numframes < 2) {
        return;
    }

    // Go through all the frames, and process any commands associated with the
    // tempmodel as well
    while (deltatime >= frametime) {
        p->lastAnimTime += frametime;
        p->ent.wasframe = (p->ent.wasframe + 1) % numframes;
        CG_ProcessEntityCommands(p->ent.wasframe, p->ent.frameInfo[0].index, -1, &p->ent, NULL);
    }
}

//===============
// UpdateSwarm
//===============
void ClientGameCommandManager::UpdateSwarm(ctempmodel_t *p)
{
    if (p->cgd.swarmfreq == 0) {
        return;
    }

    // If the frequency is hit, set a new velocity
    if (!(rand() % p->cgd.swarmfreq)) {
        p->cgd.velocity.x = crandom() * p->cgd.swarmmaxspeed;
        p->cgd.velocity.y = crandom() * p->cgd.swarmmaxspeed;
        p->cgd.velocity.z = crandom() * p->cgd.swarmmaxspeed;
    }

    // Try to move toward the origin by the specified delta
    if (p->cgd.origin.x < p->cgd.parentOrigin.x) {
        p->cgd.velocity.x += p->cgd.swarmdelta;
    } else {
        p->cgd.velocity.x -= p->cgd.swarmdelta;
    }

    if (p->cgd.origin.y < p->cgd.parentOrigin.y) {
        p->cgd.velocity.y += p->cgd.swarmdelta;
    } else {
        p->cgd.velocity.y -= p->cgd.swarmdelta;
    }

    if (p->cgd.origin.z < p->cgd.parentOrigin.z) {
        p->cgd.velocity.z += p->cgd.swarmdelta;
    } else {
        p->cgd.velocity.z -= p->cgd.swarmdelta;
    }
}

qboolean ClientGameCommandManager::TempModelRealtimeEffects(ctempmodel_t *p, float ftime, float dtime, float scale)
{
    float fade, fadein;
    byte  tempColor[4];

    if (p->cgd.flags & (T_FADE | T_SCALEUPDOWN)) {
        fade = 1.0f - (float)(p->aliveTime - p->cgd.fadedelay) / (float)(p->cgd.life - p->cgd.fadedelay);

        // Clamp the fade
        if (fade > 1) {
            fade = 1;
        }
        if (fade < 0) {
            fade = 0;
        }
    } else {
        fade = 1.0f;
    }

    dtime = (cg.time - p->cgd.createTime);

    // Calculate fade in value
    if (p->cgd.flags & T_FADEIN) {
        fadein = dtime / (float)p->cgd.fadeintime;
    } else {
        fadein = 0;
    }

    // Convert dtime to seconds
    dtime *= 0.001f;

    // Do the scale animation
    if (ftime && p->cgd.scaleRate) {
        p->ent.scale += p->cgd.scale * (p->cgd.scaleRate * ftime);
    } else if (p->cgd.flags & T_DLIGHT) {
        p->cgd.lightIntensity += p->cgd.scaleRate * ftime * p->cgd.lightIntensity;
        if (p->cgd.lightIntensity < 0.0f) {
            return qfalse;
        }
    }

    if (p->cgd.flags & T_SCALEUPDOWN) {
        p->ent.scale = p->cgd.scale * sin((fade)*M_PI);

        if (p->ent.scale < p->cgd.scalemin) {
            p->ent.scale = p->cgd.scalemin;
        }
        if (p->ent.scale > p->cgd.scalemax) {
            p->ent.scale = p->cgd.scalemax;
        }
    }

    if (p->cgd.lightstyle >= 0) {
        int   i;
        float color[4];
        CG_LightStyleColor(p->cgd.lightstyle, dtime * 1000, color);
        for (i = 0; i < 4; i++) {
            tempColor[i] = (byte)(color[i] * 255.0f);
        }
    } else {
        if (p->cgd.flags2 & T2_COLOR_AVEL) {
            p->cgd.color[0] += p->cgd.avelocity.x * ftime;
            p->cgd.color[1] += p->cgd.avelocity.y * ftime;
            p->cgd.color[2] += p->cgd.avelocity.z * ftime;

            if (p->cgd.color[0] < 0.0f) {
                p->cgd.color[0] = 0.0f;
            }
            if (p->cgd.color[1] < 0.0f) {
                p->cgd.color[1] = 0.0f;
            }
            if (p->cgd.color[2] < 0.0f) {
                p->cgd.color[2] = 0.0f;
            }
        }

        tempColor[0] = (int)(p->cgd.color[0] * 255.0f);
        tempColor[1] = (int)(p->cgd.color[1] * 255.0f);
        tempColor[2] = (int)(p->cgd.color[2] * 255.0f);
        tempColor[3] = (int)(p->cgd.color[3] * 255.0f);
    }

    if (p->cgd.flags & T_TWINKLE) {
        // See if we should toggle the twinkle
        if (cg.time > p->twinkleTime) {
            // If off, turn it on
            if (p->cgd.flags & T_TWINKLE_OFF) {
                p->cgd.flags &= ~T_TWINKLE_OFF;
                p->twinkleTime = cg.time + p->cgd.min_twinkletimeon + random() * p->cgd.max_twinkletimeon;
            } else {
                p->cgd.flags |= T_TWINKLE_OFF;
                p->twinkleTime = cg.time + p->cgd.min_twinkletimeoff + random() * p->cgd.max_twinkletimeoff;
            }
        }

        if (p->cgd.flags & T_TWINKLE_OFF) {
            memset(tempColor, 0, sizeof(tempColor));
        }
    }

    if (p->cgd.flags & T_COLLISION) {
        vec3_t vLighting;
        cgi.R_GetLightingForSmoke(vLighting, p->ent.origin);

        p->ent.shaderRGBA[0] = (int)((float)tempColor[0] * vLighting[0]);
        p->ent.shaderRGBA[1] = (int)((float)tempColor[1] * vLighting[1]);
    } else {
        p->ent.shaderRGBA[0] = tempColor[0];
        p->ent.shaderRGBA[1] = tempColor[1];
    }

    if (p->cgd.flags & T_FADEIN && (fadein < 1)) // Do the fadein effect
    {
        p->ent.shaderRGBA[3] = (int)((float)tempColor[3] * (fadein * p->cgd.alpha));
    } else if (p->cgd.flags & T_FADE) // Do a fadeout effect
    {
        p->ent.shaderRGBA[3] = (int)((float)tempColor[3] * (fade * p->cgd.alpha));
    } else {
        p->ent.shaderRGBA[3] = (int)((float)tempColor[3] * p->cgd.alpha);
    }

    if (p->cgd.flags & T_FLICKERALPHA) {
        float random = random();

        if (p->cgd.flags & (T_FADE | T_FADEIN)) {
            p->ent.shaderRGBA[3] *= random;
        } else {
            p->ent.shaderRGBA[3] = p->cgd.color[3] * random;
        }
    }

    // Check for completely faded out model
    if (fade <= 0 && p->addedOnce) {
        return false;
    }

    // Check for completely scaled out model
    if ((p->ent.scale <= 0 && p->addedOnce) && !(p->cgd.flags & T_SCALEUPDOWN)) {
        return false;
    }

    // Do swarming flies effects
    if (p->cgd.flags & T_SWARM) {
        UpdateSwarm(p);
    }

    return true;
}

void ClientGameCommandManager::OtherTempModelEffects(ctempmodel_t *p, Vector origin, refEntity_t *newEnt)
{
    vec3_t axis[3];

    if (p->number != -1) {
        // Set the axis
        AnglesToAxis(p->cgd.angles, axis);

        current_scale         = newEnt->scale;
        current_entity        = newEnt;
        current_tiki          = p->cgd.tiki;
        current_entity_number = p->number;

        // Update any emitters that are active on this tempmodel
        UpdateEmitter(p->cgd.tiki, axis, p->number, p->cgd.parent, origin);

        // Add in trails for this tempmodel
        if (p->cgd.flags2 & T2_TRAIL) {
            Event *ev = new Event(EV_Client_Swipe);
            ev->AddVector(origin);
            commandManager.ProcessEvent(ev);
        }

        current_entity_number = -1;
        current_tiki          = NULL;
        current_entity        = NULL;
        current_scale         = -1;
    }
}

qboolean ClientGameCommandManager::TempModelPhysics(ctempmodel_t *p, float ftime, float time2, float scale)
{
    int     dtime;
    Vector  parentOrigin(0, 0, 0);
    Vector  parentAngles(0, 0, 0);
    Vector  tempangles;
    trace_t trace;
    float   dot;

    VectorCopy(p->ent.origin, p->lastEnt.origin);
    AxisCopy(p->ent.axis, p->lastEnt.axis);

    dtime = (cg.time - p->cgd.createTime);

    // Save oldorigin
    p->cgd.oldorigin = p->cgd.origin;

    // Update based on swarm
    if (p->cgd.flags & T_SWARM) {
        p->cgd.origin += ftime * p->cgd.velocity * scale;
    }

    // Update the orign and the angles based on velocities first
    if (p->cgd.flags2 & (T2_MOVE | T2_ACCEL)) {
        p->cgd.origin = p->cgd.origin + (p->cgd.velocity * ftime * scale) + (time2 * p->cgd.accel);
    }

    // If linked to the parent or hardlinked, get the parent's origin
    if ((p->cgd.flags & (T_PARENTLINK | T_HARDLINK)) && (p->cgd.parent != ENTITYNUM_NONE)) {
        centity_t *pc;
        pc = &cg_entities[p->cgd.parent];

        if (pc->currentValid) {
            refEntity_t *e;

            e = cgi.R_GetRenderEntity(p->cgd.parent);

            if (!e) {
                return false;
            }

            parentOrigin = e->origin;
            vectoangles(e->axis[0], parentAngles);
        } else {
            return false;
        }
    } else if (p->cgd.flags & T_SWARM) {
        p->cgd.parentOrigin = p->cgd.velocity + p->cgd.accel * ftime * scale;
    }

    // Align the object along it's traveling vector
    if (p->cgd.flags & T_ALIGN) {
        p->cgd.angles = p->cgd.velocity.toAngles();
        parentAngles  = vec_zero;
    }

    if (p->cgd.flags & T_RANDOMROLL) {
        p->cgd.angles[ROLL] = random() * 360;
    }

    // Update the angles based on angular velocity
    if (p->cgd.flags2 & T2_AMOVE) {
        p->cgd.angles = p->cgd.angles + (ftime * p->cgd.avelocity);
    }

    // Mod the angles if needed
    p->cgd.angles[0] = AngleMod(p->cgd.angles[0]);
    p->cgd.angles[1] = AngleMod(p->cgd.angles[1]);
    p->cgd.angles[2] = AngleMod(p->cgd.angles[2]);

    // Add in parent angles
    tempangles = p->cgd.angles + parentAngles;

    // Convert to axis
    if ((p->cgd.flags & (T_ALIGN | T_RANDOMROLL | T_PARENTLINK | T_HARDLINK | T_ANGLES))
        || (p->cgd.flags2 & T2_AMOVE)) {
        AnglesToAxis(tempangles, p->ent.axis);
    }

    // Only do real collision if necessary
    if (p->cgd.flags & T_COLLISION) {
        // trace a line from previous position to new position
        CG_Trace(
            &trace,
            p->cgd.oldorigin,
            vec3_origin,
            vec3_origin,
            p->cgd.origin,
            -1,
            p->cgd.collisionmask,
            qfalse,
            qfalse,
            "Collision"
        );
    } else {
        // Fake it out so it never collides
        trace.fraction = 1.0;
    }

    // Check for collision
    if (trace.fraction == 1.0) {
        // Acceleration of velocity
        if (p->cgd.flags2 & T2_ACCEL) {
            p->cgd.velocity = p->cgd.velocity + ftime * p->cgd.accel;
        }

        if (p->cgd.flags2 & T2_FRICTION) {
            float fFriction = 1.0f - ftime * p->cgd.friction;
            if (fFriction > 0.0f) {
                p->cgd.velocity *= fFriction;
            } else {
                p->cgd.velocity = vec_zero;
            }
        }

        if (p->cgd.flags2 & T2_CLAMP_VEL) {
            if (p->cgd.velocity.x < p->cgd.minVel.x) {
                p->cgd.velocity.x = p->cgd.minVel.x;
            }
            if (p->cgd.velocity.y < p->cgd.minVel.y) {
                p->cgd.velocity.y = p->cgd.minVel.y;
            }
            if (p->cgd.velocity.z < p->cgd.minVel.z) {
                p->cgd.velocity.z = p->cgd.minVel.z;
            }

            if (p->cgd.velocity.x > p->cgd.maxVel.x) {
                p->cgd.velocity.x = p->cgd.maxVel.x;
            }
            if (p->cgd.velocity.y > p->cgd.maxVel.y) {
                p->cgd.velocity.y = p->cgd.maxVel.y;
            }
            if (p->cgd.velocity.z > p->cgd.maxVel.z) {
                p->cgd.velocity.z = p->cgd.maxVel.z;
            }
        }

        if (p->cgd.flags2 & T2_CLAMP_VEL_AXIS) {
            Vector localVelocity;
            localVelocity.x = DotProduct(p->cgd.velocity, p->ent.axis[0]);
            localVelocity.y = DotProduct(p->cgd.velocity, p->ent.axis[1]);
            localVelocity.z = DotProduct(p->cgd.velocity, p->ent.axis[2]);

            if (localVelocity.x < p->cgd.minVel.x) {
                localVelocity.x = p->cgd.minVel.x;
            }
            if (localVelocity.y < p->cgd.minVel.y) {
                localVelocity.y = p->cgd.minVel.y;
            }
            if (localVelocity.z < p->cgd.minVel.z) {
                localVelocity.z = p->cgd.minVel.z;
            }

            if (localVelocity.x > p->cgd.maxVel.x) {
                localVelocity.x = p->cgd.maxVel.x;
            }
            if (localVelocity.y > p->cgd.maxVel.y) {
                localVelocity.y = p->cgd.maxVel.y;
            }
            if (localVelocity.z > p->cgd.maxVel.z) {
                localVelocity.z = p->cgd.maxVel.z;
            }

            p->cgd.velocity.x = DotProduct(localVelocity, p->ent.axis[0]);
            p->cgd.velocity.y = DotProduct(localVelocity, p->ent.axis[1]);
            p->cgd.velocity.z = DotProduct(localVelocity, p->ent.axis[2]);
        }

        if (p->cgd.flags2 & T_ALIGN) {
            // FIXME: vss wind
        }
    } else {
        if (p->touchfcn) {
            p->touchfcn(p, &trace);
        }

        if (p->cgd.flags & T_DIETOUCH) {
            return false;
        }

        Vector normal;

        // Set the origin
        p->cgd.origin = trace.endpos;

        if ((p->cgd.flags2 & T2_BOUNCE_DECAL) && (p->cgd.bouncecount < p->cgd.maxbouncecount)) {
            // Put down a bounce decal
            qhandle_t shader = cgi.R_RegisterShader(p->cgd.shadername);

            CG_ImpactMarkSimple(
                shader,
                trace.endpos,
                trace.plane.normal,
                p->cgd.decal_orientation,
                p->cgd.color[0],
                p->cgd.color[1],
                p->cgd.color[2],
                p->cgd.alpha,
                p->cgd.flags & T_FADE,
                p->cgd.decal_radius,
                p->cgd.flags2 & T2_TEMPORARY_DECAL,
                p->cgd.lightstyle,
                p->cgd.flags & T_FADEIN
            );

            p->cgd.bouncecount++;
        }

        if (p->cgd.flags & T_DIETOUCH) {
            return false;
        }

        // calculate the bounce
        normal = trace.plane.normal;

        // reflect the velocity on the trace plane
        if (p->cgd.flags2 & T2_ACCEL) {
            p->cgd.velocity = p->cgd.velocity + ftime * trace.fraction * p->cgd.accel;
        }

        dot             = p->cgd.velocity * normal;
        p->cgd.velocity = p->cgd.velocity + ((-2 * dot) * normal);
        p->cgd.velocity *= p->cgd.bouncefactor;

        // check for stop
        if (trace.plane.normal[2] > 0 && p->cgd.velocity[2] < 40) {
            p->cgd.velocity  = Vector(0, 0, 0);
            p->cgd.avelocity = Vector(0, 0, 0);
            p->cgd.flags &= ~T_WAVE;
        } else {
            if (p->cgd.flags & T_BOUNCESOUNDONCE) {
                vec3_t org;
                VectorCopy(p->cgd.origin, org);
                PlaySound(p->cgd.bouncesound, org);
                p->cgd.flags &= ~(T_BOUNCESOUNDONCE | T_BOUNCESOUND);
            } else if ((p->cgd.flags & T_BOUNCESOUND) && (p->next_bouncesound_time < cg.time)) {
                vec3_t org;
                VectorCopy(p->cgd.origin, org);
                PlaySound(p->cgd.bouncesound, org);
                p->next_bouncesound_time = cg.time + p->cgd.bouncesound_delay;
            }
        }
    }

    // copy over origin
    VectorCopy(p->cgd.origin, p->ent.origin);

    // Add in parent's origin if linked
    if (p->cgd.flags & (T_PARENTLINK | T_HARDLINK)) {
        VectorAdd(p->ent.origin, parentOrigin, p->ent.origin);
    }

    if (!p->lastEntValid) {
        // Make the lastEnt valid, by setting it to p->ent and setting the
        // origin to the tempmodel's oldorigin
        p->lastEnt = p->ent;
        VectorCopy(p->cgd.oldorigin, p->lastEnt.origin);
        p->lastEntValid = true;
    }

    return true;
}

qboolean ClientGameCommandManager::LerpTempModel(refEntity_t *newEnt, ctempmodel_t *p, float frac)
{
    int i, j;

    // If the tempmodel is parentlinked, then we need to get the origin of the
    // parent and add it to the tempmodel's origin
    if (p->cgd.flags & (T_PARENTLINK | T_HARDLINK)) {
        centity_t *pc;
        Vector     parentOrigin;

        // Lerp the tempmodel's local origin
        for (i = 0; i < 3; i++) {
            newEnt->origin[i] = p->cgd.oldorigin[i] + frac * (p->cgd.origin[i] - p->cgd.oldorigin[i]);
        }

        // Find the parent entity
        pc = &cg_entities[p->cgd.parent];

        if (pc->currentValid) {
            refEntity_t *e;

            e = cgi.R_GetRenderEntity(p->cgd.parent);

            if (!e) {
                return false;
            }

            parentOrigin = e->origin;
        } else {
            return false;
        }

        // Add the parent ent's origin to the local origin
        VectorAdd(newEnt->origin, parentOrigin, newEnt->origin);
    } else {
        if (p->cgd.flags2 & (T2_MOVE | T2_ACCEL)) {
            // Lerp the ent's origin
            for (i = 0; i < 3; i++) {
                newEnt->origin[i] = p->lastEnt.origin[i] + frac * (p->ent.origin[i] - p->lastEnt.origin[i]);
            }
        }
    }

    if (p->cgd.flags2 & T2_PARALLEL) {
        Vector v1 = p->cgd.origin - cg.refdef.vieworg;
        vectoangles(v1, p->cgd.angles);
        AnglesToAxis(p->cgd.angles, newEnt->axis);
    } else if ((p->cgd.flags & (T_ALIGN | T_RANDOMROLL | T_PARENTLINK | T_HARDLINK)) || (p->cgd.flags2 & T2_AMOVE)) {
        // Lerp axis
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 3; j++) {
                newEnt->axis[i][j] = p->lastEnt.axis[i][j] + frac * (p->ent.axis[i][j] - p->lastEnt.axis[i][j]);
            }
        }
    }

    return true;
}

//===============
// CG_AddTempModels
//===============
void CG_AddTempModels(void)
{
    commandManager.AddTempModels();
}

//===============
// AddTempModels - Update and add tempmodels to the ref
//===============
#define TOO_MUCH_TIME_PASSED 500

void ClientGameCommandManager::AddTempModels(void)
{
    ctempmodel_t *p, *next;
    int           count = 0; // Tempmodel count
    int           frameTime;
    float         effectTime, effectTime2;
    int           mstime       = 0;
    float         ftime        = 0;
    float         time2        = 0;
    float         scale        = 1.0f;
    float         lerpfrac     = 0;
    int           physics_rate = 0;
    qboolean      ret;
    refEntity_t   newEnt;
    dtiki_t      *old_tiki;
    int           old_num;
    refEntity_t  *old_ent;

    // To counteract cg.time going backwards
    if (lastTempModelFrameTime) {
        if ((cg.time < lastTempModelFrameTime) || (cg.time - lastTempModelFrameTime > TOO_MUCH_TIME_PASSED)) {
            p = m_active_tempmodels.prev;
            for (; p != &m_active_tempmodels; p = next) {
                next               = p->prev;
                p->lastPhysicsTime = cg.time;
            }
            lastTempModelFrameTime = cg.time;
            return;
        }
        frameTime = cg.time - lastTempModelFrameTime;
    } else {
        frameTime = 0;
    }

    if (paused->integer) {
        lastTempModelFrameTime = 0;
    } else {
        lastTempModelFrameTime = cg.time;
    }

    memset(&newEnt, 0, sizeof(newEnt));
    // Set this frame time for the next one
    effectTime  = (float)frameTime / 1000.0f;
    effectTime2 = effectTime * effectTime;

    // If there is a current entity, it's scale is used as a factor
    if (current_entity) {
        scale = current_entity->scale;
    }

    // Go through all the temp models and run the physics if necessary,
    // then add them to the ref
    old_ent  = current_entity;
    old_tiki = current_tiki;
    old_num  = current_entity_number;

    p = m_active_tempmodels.prev;
    for (; p != &m_active_tempmodels; p = next) {
        // grab next now, so if the local entity is freed we still have it
        next = p->prev;

        if ((p->cgd.flags & T_DETAIL) && !cg_detail->integer) {
            FreeTempModel(p);
            continue;
        }

        p->ent.tiki           = p->cgd.tiki;
        current_entity        = &p->ent;
        current_tiki          = p->cgd.tiki;
        current_entity_number = p->number;

        TempModelRealtimeEffects(p, effectTime, effectTime2, scale);

        if (p->lastPhysicsTime) {
            mstime = cg.time - p->lastPhysicsTime;

            // Check for physics
            physics_rate = 1000 / p->cgd.physicsRate; // Physics rate in milliseconds

            // Avoid large jumps in time
            if (mstime > physics_rate * 2) {
                mstime = physics_rate;
            }

            if ((mstime >= physics_rate) || (p->cgd.flags2 & T2_PHYSICS_EVERYFRAME)) {
                ftime = mstime / 1000.0f;
                time2 = ftime * ftime;
                ret   = TempModelPhysics(p, ftime, time2, scale);

                if (!ret) {
                    FreeTempModel(p);
                    continue;
                }

                p->lastPhysicsTime = cg.time;
            }
        }

        // Calculate the lerp value based on the time passed since last physics
        // time of this tempmodel
        lerpfrac = (float)(cg.time - p->lastPhysicsTime) / (float)physics_rate;

        // Clamp
        if (lerpfrac > 1) {
            lerpfrac = 1;
        }
        if (lerpfrac < 0) {
            lerpfrac = 0;
        }

        // Increment the time this tempmodel has been alive
        p->aliveTime += frameTime;

        // Dead, and free up the tempmodel
        if (p->aliveTime >= p->cgd.life && p->addedOnce) {
            FreeTempModel(p);
            continue;
        }

        // Run physics if the lastEnt is not valid to get a valid lerp
        if (!p->lastEntValid) {
            float t, t2;
            t  = physics_rate / 1000.0f;
            t2 = t * t;

            ret = TempModelPhysics(p, t, t2, scale);
            if (!ret) {
                FreeTempModel(p);
                continue;
            }

            lerpfrac           = 0;
            p->lastPhysicsTime = cg.time;
        }

        // clear out the new entity and initialize it
        // this will become the current_entity if anything is spawned off it

        newEnt.scale = p->ent.scale;
        memcpy(newEnt.shaderRGBA, p->ent.shaderRGBA, 4);
        AxisCopy(p->ent.axis, newEnt.axis);
        VectorCopy(p->ent.origin, newEnt.origin);

        // Lerp the tempmodel
        if (!LerpTempModel(&newEnt, p, lerpfrac)) {
            FreeTempModel(p);
            continue;
        }

        if (p->cgd.flags & T_WAVE) {
            Vector origin;
            float  axis[3][3];

            origin = Vector(p->m_spawnthing->linked_origin) + Vector(p->m_spawnthing->linked_axis[0]) * newEnt.origin[0]
                   + Vector(p->m_spawnthing->linked_axis[1]) * newEnt.origin[1]
                   + Vector(p->m_spawnthing->linked_axis[2]) * newEnt.origin[2];

            VectorCopy(origin, newEnt.origin);

            MatrixMultiply(newEnt.axis, p->m_spawnthing->linked_axis, axis);
            AxisCopy(axis, newEnt.axis);
        }

        // Animate and do trails (swipes)
        newEnt.renderfx   = p->ent.renderfx;
        newEnt.hModel     = p->ent.hModel;
        newEnt.reType     = p->ent.reType;
        newEnt.shaderTime = p->ent.shaderTime;

        newEnt.frameInfo[0] = p->ent.frameInfo[0];
        newEnt.wasframe     = p->ent.wasframe;
        newEnt.actionWeight = 1.0;
        newEnt.entityNumber = ENTITYNUM_NONE;
        newEnt.tiki         = p->ent.tiki;

        AnimateTempModel(p, newEnt.origin, &newEnt);

        OtherTempModelEffects(p, newEnt.origin, &newEnt);

        // Add to the ref
        if (p->cgd.flags & T_DLIGHT) {
            // Tempmodel is a Dynamic Light
            cgi.R_AddLightToScene(
                p->cgd.origin,
                p->cgd.lightIntensity * scale,
                p->cgd.color[0],
                p->cgd.color[1],
                p->cgd.color[2],
                p->cgd.lightType
            );
        } else if (p->ent.reType == RT_SPRITE) {
            vec3_t vTestAngles;
            cgi.R_AddRefSpriteToScene(&newEnt); // Sprite
            MatrixToEulerAngles(newEnt.axis, vTestAngles);
        } else {
            cgi.R_AddRefEntityToScene(&newEnt, ENTITYNUM_NONE); // Model
        }

        // Set the added once flag so we can delete it later
        p->addedOnce = qtrue;

        // Local tempmodel count stat
        count++;
    }

    current_entity        = old_ent;
    current_tiki          = old_tiki;
    current_entity_number = old_num;

    // stats
    if (cg_showtempmodels->integer) {
        cgi.DPrintf("TC:%i\n", count);
    }
}

//=================
// SpawnTempModel
//=================
void ClientGameCommandManager::SpawnTempModel(int count, spawnthing_t *sp)
{
    m_spawnthing = sp;
    SpawnTempModel(count);
}

//=================
// SpawnTempModel
//=================
void ClientGameCommandManager::SpawnTempModel(int mcount)
{
    int           i;
    ctempmodel_t *p;
    refEntity_t   ent;
    int           count;
    float         current_entity_scale = 1.0f;
    Vector        newForward;
    Vector        delta;
    Vector        start;
    Vector        vForward, vLeft, vUp;

    if (current_entity) {
        current_entity_scale = current_entity->scale;
    } else {
        current_entity_scale = 1.0f;
    }

    if (current_scale > 0) {
        current_entity_scale *= current_scale;
    }

    if (mcount > 1) {
        mcount *= cg_effectdetail->value;
    }

    if (mcount < 1) {
        mcount = 1;
    }

    if (m_spawnthing->cgd.flags2 & T2_SPIN) {
        float cosa, sina;
        float fAngle;

        fAngle = (cg.time - cgs.levelStartTime) * m_spawnthing->cgd.spin_rotation / 160.0f;
        cosa   = cos(fAngle);
        sina   = sin(fAngle);

        vForward = cosa * Vector(m_spawnthing->axis[0]);
        VectorMA(vForward, -sina, m_spawnthing->axis[1], vForward);

        vLeft = sina * Vector(m_spawnthing->axis[0]);
        VectorMA(vLeft, cosa, m_spawnthing->axis[1], vLeft);
    } else {
        vForward = m_spawnthing->axis[0];
        vLeft    = m_spawnthing->axis[1];
    }

    vUp = m_spawnthing->axis[2];

    for (count = 0; count < mcount; count++) {
        p = AllocateTempModel();

        if (!p) {
            cgi.DPrintf("Out of tempmodels\n");
            return;
        }

        memset(&ent, 0, sizeof(refEntity_t));
        memset(&p->lastEnt, 0, sizeof(refEntity_t));

        if (m_spawnthing->cgd.flags & T_WAVE) {
            p->m_spawnthing = m_spawnthing;
            m_spawnthing->numtempmodels++;
            start           = Vector(0, 0, 0);
        } else {
            p->m_spawnthing = NULL;
            start           = m_spawnthing->cgd.origin;
        }

        // Copy over the common data block
        p->cgd = m_spawnthing->cgd;

        // newForward may be changed by spehere or circle
        newForward = vForward;

        // Set up the origin of the tempmodel
        if (m_spawnthing->cgd.flags & T_SPHERE) {
            // Create a random forward vector so the particles burst out in a
            // sphere
            newForward = Vector(crandom(), crandom(), crandom());
        } else if (m_spawnthing->cgd.flags & T_CIRCLE) {
            if (m_spawnthing->sphereRadius != 0) // Offset by the radius
            {
                Vector dst;
                // Create a circular shaped burst around the up vector
                float angle = ((float)count / (float)m_spawnthing->count) * 360; // * M_PI * 2;

                Vector end = Vector(vLeft) * m_spawnthing->sphereRadius * current_entity_scale;
                RotatePointAroundVector(dst, Vector(vForward), end, angle);

                VectorAdd(dst, m_spawnthing->cgd.origin, p->cgd.origin);
                newForward = p->cgd.origin - m_spawnthing->cgd.origin;
                newForward.normalize();
            }
        } else if (m_spawnthing->cgd.flags & T_SPHERE) {
            // Project the origin along a random ray, and set the forward
            // vector pointing back to the origin
            Vector dir, end;

            dir = Vector(crandom(), crandom(), crandom());

            end = m_spawnthing->cgd.origin + dir * m_spawnthing->sphereRadius * current_entity_scale;
            VectorCopy(end, p->cgd.origin);
            newForward = dir * -1;
        } else if (m_spawnthing->cgd.flags2 & T2_CONE) {
            float fHeight;
            float fRadius;
            float fAngle;
            float sina;
            float cosa;

            fHeight = random();
            fRadius = random();

            if (fHeight < fRadius) {
                float fTemp = fHeight;
                fHeight     = fRadius;
                fRadius     = fTemp;
            }

            fHeight *= m_spawnthing->coneHeight;
            fRadius = m_spawnthing->sphereRadius;

            fAngle = random() * M_PI * 2;
            sina   = sin(fAngle);
            cosa   = cos(fAngle);

            p->cgd.origin = start + vForward * fHeight;
            p->cgd.origin += start + vLeft * (cosa * fRadius);
            p->cgd.origin += start + vUp * (sina * fRadius);
        } else if (m_spawnthing->sphereRadius != 0) // Offset in a spherical pattern
        {
            Vector dir, end;

            dir = Vector(crandom(), crandom(), crandom());

            dir.normalize();

            end = m_spawnthing->cgd.origin + dir * m_spawnthing->sphereRadius * current_entity_scale;
            VectorCopy(end, p->cgd.origin);
            newForward = dir;
        } else {
            VectorCopy(m_spawnthing->cgd.origin, p->cgd.origin);
        }

        if (m_spawnthing->cgd.flags & T_SWARM && !(m_spawnthing->cgd.flags & (T_HARDLINK | T_PARENTLINK))) {
            p->cgd.parentOrigin = p->cgd.origin;
        }

        // Randomize the origin based on offsets
        for (i = 0; i < 3; i++) {
            p->cgd.origin[i] +=
                (random() * m_spawnthing->origin_offset_amplitude[i] + m_spawnthing->origin_offset_base[i])
                * current_entity_scale;
        }

        p->cgd.oldorigin = p->cgd.origin;
        p->modelname     = m_spawnthing->GetModel();

        if (!(p->cgd.flags & T_DLIGHT) && !p->modelname.length()) {
            FreeTempModel(p);
            continue;
        }

        p->addedOnce    = qfalse;
        p->lastEntValid = qfalse;

        if (p->modelname.length()) {
            ent.hModel = cgi.R_RegisterModel(p->modelname.c_str());
        }

        // Initialize the refEntity
        ent.shaderTime = cg.time / 1000.0f;

        // Get the tikihandle
        p->cgd.tiki = cgi.R_Model_GetHandle(ent.hModel);

        // Set the reftype based on the modelname
        if (p->modelname == "*beam") {
            ent.reType       = RT_BEAM;
            ent.customShader = cgi.R_RegisterShader("beamshader");
        } else if (strstr(p->modelname, ".spr")) {
            ent.reType = RT_SPRITE;
        } else {
            ent.reType = RT_MODEL;
        }

        // Set the animation
        if (m_spawnthing->animName.length() && p->cgd.tiki) {
            ent.frameInfo[0].index = cgi.Anim_NumForName(p->cgd.tiki, m_spawnthing->animName);

            if (ent.frameInfo[0].index < 0) {
                ent.frameInfo[0].index = 0;
            }

            ent.frameInfo[0].weight = 1.0f;
            ent.frameInfo[0].time   = 0.0f;
            ent.actionWeight        = 1.0f;
        } else if (ent.reType == RT_MODEL && p->cgd.tiki) {
            ent.frameInfo[0].index = cgi.Anim_NumForName(p->cgd.tiki, "idle");

            if (ent.frameInfo[0].index < 0) {
                ent.frameInfo[0].index = 0;
            }

            ent.frameInfo[0].weight = 1.0f;
            ent.frameInfo[0].time   = 0.0f;
            ent.actionWeight        = 1.0f;
        }

        // Randomize the scale
        if (m_spawnthing->cgd.flags & T_RANDSCALE) // Check for random scale
        {
            ent.scale    = RandomizeRange(m_spawnthing->cgd.scalemin, m_spawnthing->cgd.scalemax);
            p->cgd.scale = ent.scale;
        } else {
            ent.scale = m_spawnthing->cgd.scale;
        }

        ent.scale *= current_entity_scale;

        // CURRENT ENTITY INFLUENCES ON THE TEMPMODELS HAPPEN HERE
        // copy over the renderfx from the current_entity, but only the flags we
        // want
        if (current_entity) {
            // explicitly add RF_LIGHTING ORIGIN and RF_SHADOWPLANE because we
            // don't want those on dynamic objects
            ent.renderfx |= (current_entity->renderfx & ~(RF_FLAGS_NOT_INHERITED | RF_LIGHTING_ORIGIN));
        }

        // Set up modulation for constant color
        for (i = 0; i < 4; i++) {
            p->cgd.color[i]   = m_spawnthing->cgd.color[i];
            ent.shaderRGBA[i] = (byte)(p->cgd.color[i] * 255.0);
        }

        if (p->cgd.flags2 & T2_VARYCOLOR) {
            for (i = 0; i < 3; i++) {
                p->cgd.color[i] *= 0.8f + random() * 0.2f;
            }
        }

        // Apply the alpha from the current_entity to the tempmodel
        if (current_entity) {
            if (current_entity->shaderRGBA[3] != 255) {
                // pre-multiply the alpha from the entity
                for (i = 0; i < 4; i++) {
                    p->cgd.color[3] *= current_entity->shaderRGBA[3] / 255.0;
                    ent.shaderRGBA[3] = current_entity->shaderRGBA[3];
                }
            }

            if (m_spawnthing->cgd.color[3] < 1.0f) {
                p->cgd.color[3] *= m_spawnthing->cgd.color[3];
                ent.shaderRGBA[3] = (int)(p->cgd.color[3] * 255.0f);
            }
        }

        p->ent     = ent;
        p->lastEnt = ent;
        p->number  = -1;

        // If createTime is specified, the use it.  Otherwise use the createTime
        // from the spawnthing.
        p->aliveTime = 0;

        // If animateonce is set, set the life = to the length of the anim
        if ((m_spawnthing->cgd.flags & T_ANIMATEONCE) && (p->ent.frameInfo[0].index > 0)) {
            p->cgd.life = cgi.Anim_Time(p->cgd.tiki, p->ent.frameInfo[0].index) * 1000.0f;
        } else {
            p->cgd.life = m_spawnthing->cgd.life;

            if (m_spawnthing->life_random) {
                p->cgd.life += m_spawnthing->life_random * random();
            }
        }

        p->lastAnimTime          = p->cgd.createTime;
        p->lastPhysicsTime       = p->cgd.createTime;
        p->killTime              = cg.time + p->cgd.life; // The time the tempmodel will die
        p->seed                  = m_seed++;              // For use with randomizations
        p->cgd.velocity          = Vector(0, 0, 0);       // Zero out the velocity
        p->next_bouncesound_time = 0;                     // Init the next bouncesound time

        if (p->cgd.flags & T_TWINKLE) {
            if (random() > 0.5f) {
                p->cgd.flags |= T_TWINKLE_OFF;
            }
        }

        if (p->cgd.flags2 & T2_TRAIL) {
            // Assign a global number to this entity from the
            // command_time_manager Tempmodels with trails need their own unique
            // number.
            p->number = m_command_time_manager.AssignNumber();
            p->cgd.flags |= T_ASSIGNED_NUMBER;
            int        oldnum     = current_entity_number;
            centity_t *oldcent    = current_centity;
            current_centity       = NULL;
            current_entity_number = p->number;

            Event *swipeEv = new Event(EV_Client_SwipeOn);
            swipeEv->AddString(p->cgd.swipe_shader);
            swipeEv->AddString(p->cgd.swipe_tag_start);
            swipeEv->AddString(p->cgd.swipe_tag_end);
            swipeEv->AddFloat(p->cgd.swipe_life);

            commandManager.ProcessEvent(swipeEv);

            current_centity       = oldcent;
            current_entity_number = oldnum;
        }

        // Check to see if this tiki has any emitters bound to it and update
        // it's number.  This is used for updating emitters that are attached to
        // tempmodels.
        if (p->cgd.tiki) {
            for (i = 1; i <= m_emitters.NumObjects(); i++) {
                spawnthing_t *st = m_emitters.ObjectAt(i);

                if (st->cgd.tiki == p->cgd.tiki) {
                    // Assign this tempmodel a number if he doesn't already have
                    // one
                    if (p->number == -1) {
                        p->number = st->AssignNumber();
                    }

                    st->GetEmitTime(p->number);
                }
            }
        }

        for (i = 0; i < 3; i++) {
            // Randomize avelocity or set absolute
            p->cgd.avelocity[i] = m_spawnthing->avelocity_amplitude[i] * random() + m_spawnthing->avelocity_base[i];

            // Randomize angles or set absolute
            p->cgd.angles[i] += random() * m_spawnthing->angles_amplitude[i];
        }

        // If forward velocity is set, just use that otherwise use random
        // variation of the velocity
        if (m_spawnthing->forwardVelocity != 0) {
            for (i = 0; i < 3; i++) {
                p->cgd.velocity[i] = newForward[i] * m_spawnthing->forwardVelocity * current_entity_scale;
            }
        }

        if (p->cgd.flags2 & T2_PARALLEL) {
            Vector v1 = p->cgd.origin - cg.refdef.vieworg;
            vectoangles(v1, p->cgd.angles);
        }

        AnglesToAxis(p->cgd.angles, m_spawnthing->axis);
        AxisCopy(m_spawnthing->axis, p->ent.axis);

        // Random offsets along axis
        for (i = 0; i < 3; i++) {
            if (p->cgd.flags2 & T2_PARALLEL) {
                p->cgd.origin +=
                    Vector(m_spawnthing->axis[i])
                    * ((m_spawnthing->axis_offset_amplitude[i] * random() + m_spawnthing->axis_offset_base[i])
                       * current_entity_scale);
            } else {
                p->cgd.origin +=
                    Vector(m_spawnthing->tag_axis[i])
                    * ((m_spawnthing->axis_offset_amplitude[i] * random() + m_spawnthing->axis_offset_base[i])
                       * current_entity_scale);
            }
        }

        // Calculate one tick of velocity based on time alive ( passed in )
        p->cgd.origin = p->cgd.origin + (p->cgd.velocity * ((float)p->aliveTime / 1000.0f) * current_entity_scale);

        if (p->cgd.flags2 & T2_ACCEL) {
            float fLength;

            p->cgd.velocity = p->cgd.origin - start;

            fLength = p->cgd.velocity.length();
            if (fLength) {
                float fVel = m_spawnthing->cgd.velocity.x
                           + (m_spawnthing->cgd.velocity.y + m_spawnthing->cgd.velocity.z * crandom()) / fLength;

                p->cgd.velocity *= fVel;
            }
        }

        for (i = 0; i < 3; i++) {
            float fVel =
                (m_spawnthing->randvel_base[i] + m_spawnthing->randvel_amplitude[i] * random()) * current_entity_scale;

            if (m_spawnthing->cgd.flags & T_RANDVELAXIS) {
                p->cgd.velocity += fVel * Vector(m_spawnthing->tag_axis[i]);
            } else {
                p->cgd.velocity[i] += fVel;
            }
        }

        if (p->cgd.flags & (T_ALIGN | T_DETAIL)) {
            if (p->cgd.velocity.x && p->cgd.velocity.y) {
                p->cgd.angles = p->cgd.velocity.toAngles();
            }

            p->cgd.origin += p->cgd.velocity * (p->aliveTime / 1000.0) * current_entity_scale;
        }

        if (p->cgd.flags & T_AUTOCALCLIFE) {
            Vector  end, delta;
            float   length, speed;
            vec3_t  vForward;
            trace_t trace;

            AngleVectorsLeft(p->cgd.angles, vForward, NULL, NULL);

            end = p->cgd.origin + Vector(vForward) * MAP_SIZE;
            CG_Trace(
                &trace,
                p->cgd.origin,
                vec_zero,
                vec_zero,
                end,
                ENTITYNUM_NONE,
                CONTENTS_SOLID | CONTENTS_WATER,
                qfalse,
                qfalse,
                "AutoCalcLife"
            );

            delta  = trace.endpos - p->cgd.origin;
            length = delta.length();
            speed  = p->cgd.velocity.length();

            p->cgd.life = (length / speed) * 1000.0f;
        }

        // global fading is based on the number of animations in the
        // current_entity's animation
        if (current_entity) {
            if (m_spawnthing->cgd.flags & (T_GLOBALFADEIN | T_GLOBALFADEOUT)) {
                int numframes = cgi.Anim_NumFrames(current_tiki, current_entity->frameInfo[0].index);

                p->cgd.alpha = (float)current_entity->wasframe / (float)numframes;

                if (m_spawnthing->cgd.flags & T_GLOBALFADEOUT) {
                    p->cgd.alpha = 1.0f - p->cgd.alpha;
                }
            }
        }

        // Make sure to not spawn the world model as a tempmodel
        // Of course it would become extremely confusing...
        if (!(p->cgd.flags & T_DLIGHT) && p->ent.reType == RT_MODEL && !p->ent.hModel) {
            Com_Printf("^~^~^ not spawning tempmodel because it is using the world as a brush model\n");
            FreeTempModel(p);
            break;
        }

        if (m_spawnthing->cgd.flags2 & T2_RELATIVEANGLES) {
            float mat[3][3];

            MatrixMultiply(m_spawnthing->axis, p->ent.axis, mat);
            VectorCopy(mat[0], p->ent.axis[0]);
            VectorCopy(mat[1], p->ent.axis[1]);
            VectorCopy(mat[2], p->ent.axis[2]);
            MatrixToEulerAngles(p->ent.axis, p->cgd.angles);
        }
    }
}
