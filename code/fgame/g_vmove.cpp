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

// g_vmove.cpp : Vehicle movement code
//

#include "g_local.h"
#include "entity.h"
#include "movegrid.h"

typedef struct {
    qboolean validGroundTrace;
    float    previous_origin[3];
    float    previous_velocity[3];
} vml_t;

vmove_t *vm;
vml_t    vml;

void VM_ClipVelocity(float *in, float *normal, float *out, float overbounce)
{
    float backoff;
    float dir_z;
    float normal2[3];

    if (normal[2] >= 0.70f) {
        if (in[0] == 0.0f && in[1] == 0.0f) {
            VectorClear(out);
            return;
        }

        normal2[0] = in[0] + DotProduct2D(in, normal);
        normal2[1] = in[1] + DotProduct2D(in, normal);
        normal2[2] = normal[2] * DotProduct2D(in, in);

        VectorNormalize(normal2);

        dir_z = -normal2[2];

        out[0] = in[0];
        out[1] = in[1];
        out[2] = DotProduct2D(in, normal2) / dir_z;
    } else {
        backoff = DotProduct(in, normal);

        if (backoff < 0) {
            backoff *= overbounce;
        } else {
            backoff /= overbounce;
        }

        out[0] = in[0] - normal[0] * backoff;
        out[1] = in[1] - normal[1] * backoff;
        out[2] = in[2] - normal[2] * backoff;
    }
}

void VM_AddTouchEnt(int entityNum)
{
    int i;

    if (entityNum == ENTITYNUM_NONE || entityNum == ENTITYNUM_WORLD) {
        return;
    }

    if (vm->numtouch > 32) {
        return;
    }

    // see if it is already added
    for (i = 0; i < vm->numtouch; i++) {
        if (vm->touchents[i] == entityNum) {
            return;
        }
    }

    // add it
    vm->touchents[vm->numtouch] = entityNum;
    vm->numtouch++;
}

qboolean VM_SlideMove(qboolean gravity)
{
    int     bumpcount;
    vec3_t  dir;
    float   d;
    int     numplanes;
    vec3_t  planes[5];
    vec3_t  clipVelocity;
    int     i;
    int     j;
    int     k;
    trace_t trace;
    vec3_t  end;
    float   time_left;

    if (gravity) {
        vm->vs->velocity[2] = vm->vs->velocity[2] - vm->frametime * sv_gravity->integer;
        if (vm->vs->groundPlane) {
            VM_ClipVelocity(vm->vs->velocity, vm->vs->groundTrace.plane.normal, vm->vs->velocity, OVERCLIP);
        }
    }

    time_left = vm->frametime;

    if (vm->vs->groundPlane) {
        numplanes = 1;
        VectorCopy(vm->vs->groundTrace.plane.normal, planes[0]);
    } else {
        numplanes = 0;
    }

    // never turn against original velocity
    VectorNormalize2(vm->vs->velocity, planes[numplanes]);
    numplanes++;

    for (bumpcount = 0; bumpcount < 4; bumpcount++) {
        // calculate position we are trying to move to
        VectorMA(vm->vs->origin, time_left, vm->vs->velocity, end);

        // see if we can make it there
        gi.trace(&trace, vm->vs->origin, vm->mins, vm->maxs, end, vm->vs->entityNum, vm->tracemask, qtrue, qfalse);

        if (trace.allsolid) {
            break;
        }

        if (trace.fraction > 0) {
            // actually covered some distance
            VectorCopy(trace.endpos, vm->vs->origin);
        }

        if (trace.fraction == 1) {
            return bumpcount != 0;
        }

        memcpy(&vm->vs->groundTrace, &trace, sizeof(vm->vs->groundTrace));
        vml.validGroundTrace = true;

        // save entity for contact
        VM_AddTouchEnt(trace.entityNum);

        time_left -= time_left * trace.fraction;

        if (numplanes >= MAX_CLIP_PLANES) {
            VectorClear(vm->vs->velocity);
            return qtrue;
        }

        //
        // if this is the same plane we hit before, nudge velocity
        // out along it, which fixes some epsilon issues with
        // non-axial planes
        //
        for (i = 0; i < numplanes; i++) {
            if (DotProduct(trace.plane.normal, planes[i]) > 0.99) {
                VectorAdd(trace.plane.normal, vm->vs->velocity, vm->vs->velocity);
                break;
            }
        }

        if (i >= numplanes) {
            //
            // modify velocity so it parallels all of the clip planes
            //

            // find a plane that it enters
            for (i = 0; i < numplanes; i++) {
                if (DotProduct(vm->vs->velocity, planes[i]) >= 0.1) {
                    continue; // move doesn't interact with the plane
                }

                // slide along the plane
                VM_ClipVelocity(vm->vs->velocity, planes[i], clipVelocity, OVERCLIP);

                // see if there is a second plane that the new move enters
                for (j = 0; j < numplanes; j++) {
                    if (j == i) {
                        continue;
                    }

                    // slide along the plane
                    VM_ClipVelocity(vm->vs->velocity, planes[j], clipVelocity, OVERCLIP);

                    if (DotProduct(clipVelocity, planes[j]) >= 0.0f) {
                        continue; // move doesn't interact with the plane
                    }

                    // slide the original velocity along the crease
                    CrossProduct(planes[i], planes[j], dir);
                    VectorNormalize(dir);
                    d = DotProduct(dir, vm->vs->velocity);
                    VectorScale(dir, d, clipVelocity);

                    // see if there is a third plane the the new move enters
                    for (k = 0; k < numplanes; k++) {
                        if (k == i || k == j) {
                            continue;
                        }

                        if (DotProduct(clipVelocity, planes[k]) >= 0.1f) {
                            continue; // move doesn't interact with the plane
                        }

                        // stop dead at a tripple plane interaction
                        VectorClear(vm->vs->velocity);
                        return qtrue;
                    }
                }

                // if we have fixed all interactions, try another move
                VectorCopy(clipVelocity, vm->vs->velocity);
                break;
            }
        }
    }

    if (vm->vs->velocity[0] || vm->vs->velocity[1]) {
        if (vm->vs->groundPlane) {
            VectorCopy(vm->vs->velocity, dir);
            VectorNegate(dir, dir);
            VectorNormalize(dir);

            VM_AddTouchEnt(trace.entityNum);
            VectorAdd(vm->vs->obstacle_normal, dir, vm->vs->obstacle_normal);
        }

        VectorClear(vm->vs->velocity);
        return true;
    }

    vm->vs->velocity[2] = 0;
    return false;
}

static void VM_GroundTraceInternal2(void);

void VM_GroundTraceInternal(void)
{
    VM_GroundTraceInternal2();
    VM_AddTouchEnt(vm->vs->groundTrace.entityNum);
}

void VM_GroundTraceInternal2(void)
{
    if (vm->vs->groundTrace.fraction == 1.0f) {
        vm->vs->groundEntityNum = ENTITYNUM_NONE;
        vm->vs->groundPlane     = qfalse;
        vm->vs->walking         = qfalse;
        return;
    }

    if (vm->vs->velocity[2] > 0.0f) {
        if (DotProduct(vm->vs->velocity, vm->vs->groundTrace.plane.normal) > 10.0f) {
            vm->vs->groundEntityNum = ENTITYNUM_NONE;
            vm->vs->groundPlane     = qfalse;
            vm->vs->walking         = qfalse;
            return;
        }
    }

    // slopes that are too steep will not be considered onground
    if (vm->vs->groundTrace.plane.normal[2] < MIN_WALK_NORMAL) {
        vec3_t oldvel;
        float  d;

        VectorCopy(vm->vs->velocity, oldvel);
        VectorSet(vm->vs->velocity, 0, 0, -1.0f / vm->frametime);
        VM_SlideMove(qfalse);

        d = VectorLength(vm->vs->velocity);
        VectorCopy(oldvel, vm->vs->velocity);

        if (d > (0.1f / vm->frametime)) {
            vm->vs->groundEntityNum = ENTITYNUM_NONE;
            vm->vs->groundPlane     = qtrue;
            vm->vs->walking         = qfalse;
            return;
        }
    }

    vm->vs->groundPlane     = qtrue;
    vm->vs->walking         = qtrue;
    vm->vs->groundEntityNum = vm->vs->groundTrace.entityNum;
}

void VM_GroundTrace(void)
{
    float point[3];

    point[0] = vm->vs->origin[0];
    point[1] = vm->vs->origin[1];
    point[2] = vm->vs->origin[2] - 0.25f;

    gi.trace(
        &vm->vs->groundTrace, vm->vs->origin, vm->mins, vm->maxs, point, vm->vs->entityNum, vm->tracemask, qtrue, qfalse
    );
    VM_GroundTraceInternal();
}

void VM_StepSlideMove(void)
{
    vec3_t   start_o;
    vec3_t   start_v;
    vec3_t   nostep_o;
    vec3_t   nostep_v;
    trace_t  trace;
    qboolean bWasOnGoodGround;
    vec3_t   up;
    vec3_t   down;
    qboolean start_hit_wall;
    vec3_t   start_wall_normal;
    qboolean first_hit_wall;
    vec3_t   first_wall_normal;
    vec3_t   start_hit_origin;
    vec3_t   first_hit_origin;
    trace_t  nostep_groundTrace;

    VectorCopy(vm->vs->origin, start_o);
    VectorCopy(vm->vs->velocity, start_v);
    start_hit_wall = vm->vs->hit_obstacle;
    VectorCopy(vm->vs->hit_origin, start_hit_origin);
    VectorCopy(vm->vs->obstacle_normal, start_wall_normal);

    if (VM_SlideMove(qtrue) == 0) {
        if (!vml.validGroundTrace) {
            VM_GroundTrace();
        }

        return;
    }

    VectorCopy(start_o, down);
    down[2] -= STEPSIZE;
    gi.trace(&trace, start_o, vm->mins, vm->maxs, down, vm->vs->entityNum, vm->tracemask, qtrue, qfalse);
    VectorSet(up, 0, 0, 1);

    // never step up when you still have up velocity
    if (vm->vs->velocity[2] > 0 && (trace.fraction == 1.0f || DotProduct(trace.plane.normal, up) < MIN_WALK_NORMAL)) {
        if (!vml.validGroundTrace) {
            VM_GroundTrace();
        } else {
            VM_GroundTraceInternal();
        }

        return;
    }

    if (vm->vs->groundPlane && vm->vs->groundTrace.plane.normal[2] >= MIN_WALK_NORMAL) {
        bWasOnGoodGround = qtrue;
    } else {
        bWasOnGoodGround = qfalse;
    }

    VectorCopy(vm->vs->origin, nostep_o);
    VectorCopy(vm->vs->velocity, nostep_v);
    memcpy(&nostep_groundTrace, &vm->vs->groundTrace, sizeof(trace_t));

    VectorCopy(start_o, vm->vs->origin);
    VectorCopy(start_v, vm->vs->velocity);

    first_hit_wall = vm->vs->hit_obstacle;
    VectorCopy(vm->vs->hit_origin, first_hit_origin);
    VectorCopy(vm->vs->obstacle_normal, first_wall_normal);

    vm->vs->hit_obstacle = start_hit_wall;
    VectorCopy(start_hit_origin, vm->vs->hit_origin);
    VectorCopy(start_wall_normal, vm->vs->obstacle_normal);
    VM_SlideMove(qtrue);

    VectorCopy(vm->vs->origin, down);
    down[2] -= STEPSIZE * 2;

    // test the player position if they were a stepheight higher
    gi.trace(&trace, vm->vs->origin, vm->mins, vm->maxs, down, vm->vs->entityNum, vm->tracemask, qtrue, qfalse);
    if (trace.entityNum > ENTITYNUM_NONE) {
        VectorCopy(nostep_o, vm->vs->origin);
        VectorCopy(nostep_v, vm->vs->velocity);
        memcpy(&vm->vs->groundTrace, &nostep_groundTrace, sizeof(vm->vs->groundTrace));
        vm->vs->hit_obstacle = first_hit_wall;
        VectorCopy(first_hit_origin, vm->vs->hit_origin);
        VectorCopy(first_wall_normal, vm->vs->obstacle_normal);

        if (!vml.validGroundTrace) {
            VM_GroundTrace();
        } else {
            VM_GroundTraceInternal();
        }

        return;
    }

    if (!trace.allsolid) {
        memcpy(&vm->vs->groundTrace, &trace, sizeof(vm->vs->groundTrace));
        vml.validGroundTrace = qtrue;

        if (bWasOnGoodGround && trace.fraction && trace.plane.normal[2] < MIN_WALK_NORMAL) {
            VectorCopy(nostep_o, vm->vs->origin);
            VectorCopy(nostep_v, vm->vs->velocity);

            if (first_hit_wall) {
                vm->vs->hit_obstacle = first_hit_wall;
                VectorCopy(first_hit_origin, vm->vs->hit_origin);
                VectorCopy(first_wall_normal, vm->vs->obstacle_normal);
            }

            VM_GroundTraceInternal();
            return;
        }

        VectorCopy(trace.endpos, vm->vs->origin);
    }

    if (trace.fraction < 1.0f) {
        VM_ClipVelocity(vm->vs->velocity, trace.plane.normal, vm->vs->velocity, OVERCLIP);
    }

    if (!vml.validGroundTrace) {
        VM_GroundTrace();
    } else {
        VM_GroundTraceInternal();
    }
}

void VM_Friction(void)
{
    vec3_t vec;
    float *vel;
    float  speed, newspeed, control;
    float  drop;

    vel = vm->vs->velocity;

    VectorCopy(vel, vec);
    if (vm->vs->walking) {
        // ignore slope movement
        vec[2] = 0;
    }

    speed = VectorLength(vec);
    if (speed < 1) {
        // allow sinking underwater
        vel[0] = 0;
        vel[1] = 0;

        return;
    }

    drop = 0;

    if (vm->vs->walking) {
        control = (speed < 50.0f) ? 50.0f : speed;

        // if getting knocked back, no friction
        drop += control * 6.0f * vm->frametime;
    }

    // scale the velocity
    newspeed = speed - drop;
    if (newspeed < 0) {
        newspeed = 0;
    }

    newspeed /= speed;

    vel[0] = vel[0] * newspeed;
    vel[1] = vel[1] * newspeed;
    vel[2] = vel[2] * newspeed;
}

void VM_ClipVelocity2D(float *in, float *normal, float *out, float overbounce)
{
    float backoff;
    float dir_z;
    float normal2[3];

    if (normal[2] >= 0.70f) {
        if (in[0] == 0.0f && in[1] == 0.0f) {
            VectorClear(out);
            return;
        }

        normal2[0] = in[0] + DotProduct2D(in, normal);
        normal2[1] = in[1] + DotProduct2D(in, normal);
        normal2[2] = normal[2] * DotProduct2D(in, in);

        VectorNormalize(normal2);

        dir_z = -normal2[2];

        out[0] = in[0];
        out[1] = in[1];
        out[2] = DotProduct2D(in, normal2) / -normal2[2];
    } else {
        backoff = DotProduct2D(in, normal);

        if (backoff < 0) {
            backoff *= overbounce;
        } else {
            backoff /= overbounce;
        }

        out[0] = in[0] - normal[0] * backoff;
        out[1] = in[1] - normal[1] * backoff;
        out[2] = -(backoff * normal[2]);
    }
}

void VmoveSingle(vmove_t *vmove)
{
    float   point[3];
    trace_t trace;

    vm = vmove;

    vmove->numtouch         = 0;
    vmove->vs->hit_obstacle = false;
    VectorCopy(vec_origin, vmove->vs->obstacle_normal);

    memset(&vml, 0, sizeof(vml_t));

    VectorCopy(vmove->vs->origin, vml.previous_origin);
    VectorCopy(vmove->vs->velocity, vml.previous_velocity);

    VM_GroundTraceInternal2();

    if (vmove->vs->walking) {
        float wishdir[3];

        VM_Friction();
        VM_ClipVelocity2D(vm->vs->desired_dir, vm->vs->groundTrace.plane.normal, wishdir, OVERCLIP);
        VectorNormalize(wishdir);

        vm->vs->velocity[0] = vm->desired_speed * wishdir[0];
        vm->vs->velocity[1] = vm->desired_speed * wishdir[1];
    } else if (vmove->vs->groundPlane) {
        VM_ClipVelocity(vm->vs->velocity, vm->vs->groundTrace.plane.normal, vm->vs->velocity, OVERCLIP);
    }

    VM_StepSlideMove();

    if (!vm->vs->walking && vml.previous_velocity[2] >= 0.0f && vm->vs->velocity[2] <= 0.0f) {
        point[0] = vm->vs->origin[0];
        point[1] = vm->vs->origin[1];
        point[2] = vm->vs->origin[2] - 18.0f;

        gi.trace(&trace, vm->vs->origin, vm->mins, vm->maxs, point, vm->vs->entityNum, vm->tracemask, qtrue, qfalse);

        if (trace.fraction < 1.0f && !trace.allsolid) {
            VM_GroundTrace();
            return;
        }
    }
}
