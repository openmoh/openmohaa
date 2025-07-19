/*
===========================================================================
Copyright (C) 2018-2024 the OpenMoHAA team

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

#include "vehicle.h"
#include "player.h"
#include "g_phys.h"

CLASS_DECLARATION(DrivableVehicle, VehicleHalfTrack, "VehicleHalfTrack") {
    {&EV_Damage, &VehicleHalfTrack::EventDamage},
    {&EV_Killed, &VehicleHalfTrack::Killed     },
    {NULL,       NULL                          }
};

VehicleHalfTrack::VehicleHalfTrack() {}

void VehicleHalfTrack::Think()
{
    vmove_t vm;
    flags |= FL_POSTTHINK;

    prev_velocity = velocity;

    SetSlotsNonSolid();

    SetMoveInfo(&vm);
    VmoveSingle(&vm);
    GetMoveInfo(&vm);

    SetSlotsSolid();
}

void VehicleHalfTrack::Postthink()
{
    float        turn;
    Vector       i, j, k;
    int          index;
    trace_t      trace;
    Vector       normalsum;
    int          numnormals;
    Vector       temp;
    Vector       pitch, roll;
    Vector       acceleration;
    Vector       atmp, atmp2, aup;
    VehicleBase *v;
    VehicleBase *last;
    float        drivespeed;
    Vector       primal_angles;
    Vector       vTmp;
    Vector       n_angles;

    primal_angles = angles;

    if (drivable) {
        if (m_bAutoPilot) {
            AutoPilot();
        }

        currentspeed = moveimpulse;
        turnangle    = turnangle * 0.25 + turnimpulse;
        turnangle    = Q_clamp_float(turnangle, -maxturnrate, maxturnrate);

        real_velocity     = origin - prev_origin;
        prev_origin       = origin;
        prev_acceleration = real_acceleration;
        real_acceleration = real_velocity - prev_velocity;
        prev_velocity     = real_velocity;
        acceleration      = real_acceleration - prev_acceleration;

        temp = angles;
        temp.AngleVectorsLeft(&i, &j, &k);

        j = vec_zero - j;

        numnormals = 0;
        for (index = 0; index < MAX_CORNERS; index++) {
            Vector start, end;
            Vector boxoffset;
            Vector t_mins(-8, -8, -8), t_maxs(8, 8, 8);

            boxoffset = Corners[index];
            start     = origin + i * boxoffset[0] + j * boxoffset[1] + k * boxoffset[2];
            end       = start + Vector(0, 0, -400);

            trace = G_Trace(start, t_mins, t_maxs, end, this, MASK_VEHICLE_TIRES, false, "Vehicle::PostThink Corners");

            if (trace.fraction != 1.0 || trace.startsolid) {
                normalsum += trace.plane.normal;
            }
        }

        angles[2] = 0;

        if (m_iNumNormals > 1) {
            temp = normalsum / numnormals;
            temp.normalize();
            i = temp.CrossProduct(temp, j);

            pitch     = i;
            angles[0] = -pitch.toPitch();
        }

        turn = turnangle / 200.0;
        if (groundentity) {
            Vector newvel;
            Vector flatvel;

            velocity[0] *= 0.925f;
            velocity[1] *= 0.925f;

            flatvel = orientation[0];
            velocity += flatvel * currentspeed;
            flatvel[2] = 0;

            drivespeed = Q_clamp_float(velocity * flatvel, -speed, speed);

            newvel      = flatvel * drivespeed;
            velocity[0] = newvel[0];
            velocity[1] = newvel[1];
            velocity[2] += drivespeed * jumpimpulse;

            avelocity *= 0.05f;

            if (steerinplace) {
                if (drivespeed < 350) {
                    drivespeed = 350;
                }
            }

            avelocity.y += turn * drivespeed;
        } else {
            avelocity *= 0.1f;
        }

        angles += avelocity * level.frametime;
        setAngles(angles);
    }

    drivespeed = acceleration * orientation[0];

    if (drivable && driver.ent) {
        str sound_name;

        if (currentspeed > 0) {
            sound_name = "snd_forward";
        } else if (currentspeed < 0) {
            sound_name = "snd_backward";
        } else {
            sound_name = "snd_idle";
        }

        LoopSound(sound_name);
    }

    i = orientation[0];
    j = orientation[1];
    k = orientation[2];

    if (driver.ent) {
        driver.ent->setOrigin(origin + i * driveroffset[0] + j * driveroffset[1] + k * driveroffset[2]);

        if (drivable) {
            // clear the driver velocity
            driver.ent->velocity = vec_zero;
            // set the driver angles to the vehicle's angle
            driver.ent->setAngles(angles);
        }
    }

    n_angles = (angles - primal_angles) * level.frametime;
    if (n_angles.x > 180 || n_angles.x < -180) {
        n_angles.x = 0;
    }
    if (n_angles.y > 180 || n_angles.y < -180) {
        n_angles.y = 0;
    }
    if (n_angles.z > 180 || n_angles.z < -180) {
        n_angles.z = 0;
    }

    if (n_angles.x > -1 || n_angles.x < 1) {
        n_angles.x = 0;
    }
    if (n_angles.y > -1 || n_angles.y < 1) {
        n_angles.y = 0;
    }
    if (n_angles.z > -1 || n_angles.z < 1) {
        n_angles.z = 0;
    }

    avelocity = n_angles;

    for (last = this; last->vlink; last = v) {
        v = last->vlink;

        v->setOrigin(origin + i * v->offset.x + j * v->offset.y + k * v->offset.z);
        v->avelocity = avelocity;
        v->velocity  = velocity;

        v->angles[ROLL]  = angles[ROLL];
        v->angles[YAW]   = angles[YAW];
        v->angles[PITCH] = (int)(drivespeed / 4.f + v->angles[PITCH]) % 360;
        v->setAngles(v->angles);
    }

    CheckWater();
    WorldEffects();

    last_origin = origin;

    if (!driver.ent && !velocity.length() && groundentity && !(watertype & CONTENTS_LAVA)) {
        // make the vehicle stationary if it's completely immobile
        flags &= ~FL_POSTTHINK;
        if (drivable) {
            setMoveType(MOVETYPE_STATIONARY);
        }
    }
}

void VehicleHalfTrack::Killed(Event *ev)
{
    deadflag = DEAD_DEAD;
}
