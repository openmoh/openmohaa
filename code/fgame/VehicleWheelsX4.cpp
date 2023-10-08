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

#include "vehicle.h"
#include "player.h"
#include "debuglines.h"
#include "g_phys.h"

CLASS_DECLARATION(DrivableVehicle, VehicleWheelsX4, "VehicleWheelsX4") {
    {&EV_Damage, &VehicleWheelsX4::EventDamage},
    {&EV_Killed, &VehicleWheelsX4::Killed     },
    {NULL,       NULL                         }
};

VehicleWheelsX4::VehicleWheelsX4()
{
    m_iGear              = 1;
    m_iRPM               = 0;
    gravity              = 1.0;
    m_fDifferentialRatio = 4.8800001f;
    m_fGearEfficiency    = 0.69999999f;
    m_fGearRatio[0]      = -2.9400001f;
    m_fGearRatio[1]      = 2.9400001f;
    m_fGearRatio[2]      = 1.9400001f;
    m_fGearRatio[3]      = 1.0f;
    m_fAccelerator       = 0.0f;
    m_bBackSlipping      = qfalse;
    m_bAutomatic         = qtrue;
    m_bFrontSlipping     = qfalse;
}

void VehicleWheelsX4::UpdateVariables(
    Vector *acceleration, Vector *vpn, Vector *vup, Vector *vright, Vector *t_vpn, Vector *t_vup, Vector *t_vright
)
{
    int    i;
    Vector vNewCG;
    Vector vBoxSize;
    float  longspeed;
    Vector _i, _j, _k;
    Vector r_i, r_j, r_k;
    Vector r_velocity;
    float  dots[4];

    r_velocity = velocity;

    m_bIsSkidding = false;
    vBoxSize      = maxs - mins;

    SetControllerAngles(0, Vector(0, turnangle, 0));
    SetControllerAngles(1, Vector(0, turnangle, 0));

    vNewCG = localorigin;

    if (vNewCG != m_vCG) {
        m_vCG = vNewCG;
        vNewCG -= localorigin;

        m_fWheelBase      = vBoxSize.x;
        m_fWheelFrontDist = vBoxSize.x * 0.5 - vNewCG.x;
        m_fWheelBackDist  = vBoxSize.x * -0.5 + vNewCG.x;
        m_fTrackWidth     = vBoxSize.y;
    }

    Vector vNewVpn = angles;
    vNewVpn.AngleVectorsLeft(&r_i, &r_j, &r_k);
    vNewVpn.y += turnangle;
    vNewVpn.AngleVectorsLeft(&_i, &_j, &_k);

    dots[3] = r_velocity * *vpn * 0.02;
    dots[2] = r_velocity * *vright * 0.02;
    dots[1] = r_velocity * *t_vpn * 0.02;
    dots[0] = r_velocity * *t_vright * 0.02;

    m_sWheels[0].fTraction  = 0.8f;
    m_sWheels[1].fTraction  = 0.9f;
    m_sWheels[2].fTraction  = 0.8f;
    m_sWheels[3].fTraction  = 0.9f;
    m_sWheels[0].fYawOffset = 0;
    m_sWheels[1].fYawOffset = DEG2RAD(turnangle);
    m_sWheels[2].fYawOffset = 0;
    m_sWheels[3].fYawOffset = DEG2RAD(turnangle);
    m_sWheels[0].fLongDist  = m_fWheelBackDist * 0.02;
    m_sWheels[1].fLongDist  = m_fWheelFrontDist * 0.02;
    m_sWheels[2].fLongDist  = m_fWheelBackDist * 0.02;
    m_sWheels[3].fLongDist  = m_fWheelFrontDist * 0.02;
    m_sWheels[0].fLatDist   = m_fTrackWidth * 0.5 * -0.02;
    m_sWheels[1].fLatDist   = m_fTrackWidth * 0.5 * 0.02;
    m_sWheels[2].fLatDist   = m_fTrackWidth * 0.5 * -0.02;
    m_sWheels[3].fLatDist   = m_fTrackWidth * 0.5 * 0.02;

    // initialize the wheels
    for (i = 0; i < 4; i++) {
        m_sWheels[i].fLongForce = 0;
        m_sWheels[i].fForce     = 0;

        m_sWheels[i].vLongForce  = vec_zero;
        m_sWheels[i].vLatForce   = vec_zero;
        m_sWheels[i].vForce      = vec_zero;
        m_sWheels[i].vVelocity   = vec_zero;
        m_sWheels[i].bSkidding   = false;
        m_sWheels[i].bSpinning   = false;
        m_sWheels[i].fTorque     = 0;
        m_sWheels[i].fLatTorque  = 0;
        m_sWheels[i].fLongTorque = 0;
    }

    for (i = 0; i < 4; i++) {
        Vector p1, p3, p4;
        Vector vTp, vTr, vTu;
        vec3_t p2;

        p3 = Corners[i];
        p3.normalize();

        vectoangles(p3, p2);
        p1    = vec_zero;
        p1[1] = angles[1] + p2[1] + 90;
        p1.AngleVectorsLeft(&vTp, &vTr, &vTu);

        m_sWheels[i].vVelocity = vTp * RAD2DEG(m_vAngularVelocity.y) * fabs(m_sWheels[i].fLongDist) * level.frametime;

        G_LineWidth(5.0);

        G_DebugLine(
            origin + Corners[i][0] * r_i + Corners[i][1] * r_j,
            origin + Corners[i][0] * r_i + Corners[i][1] * r_j + m_sWheels[i].vVelocity * 50,
            0.7f,
            0.5f,
            1.0f,
            1.0f
        );

        p1    = vec_zero;
        p1[1] = angles[1] + RAD2DEG(m_sWheels[i].fYawOffset);
        p1.AngleVectorsLeft(&vTp, &vTr, &vTu);

        m_sWheels[i].vOrientation = vTp;

        G_LineWidth(5.0);

        G_DebugLine(
            origin + Corners[i][0] * r_i + Corners[i][1] * r_j,
            origin + Corners[i][0] * r_i + Corners[i][1] * r_j + m_sWheels[i].vOrientation * 50,
            0.7f,
            0.0f,
            1.0f,
            1.0f
        );
    }

    Vector  start, end;
    Vector  boxoffset;
    Vector  t_mins(-8, -8, -8);
    Vector  t_maxs(8, 8, 8);
    trace_t trace;

    start = origin + *vpn * (m_vCG[0] - localorigin[0]) + *vright * (m_vCG[1] - localorigin[1])
          + *vup * (m_vCG[2] - localorigin[2]);

    end = start;
    end[2] -= maxtracedist;

    trace = G_Trace(start, t_mins, t_maxs, end, this, MASK_DAMAGEMODEL, qfalse, "Vehicle::PostThink Corners");

    m_fInertia        = m_fMass * 100;
    m_fWheelFrontLoad = m_fMass * 9.81 * fabs(m_fWheelFrontDist / m_fWheelBase);
    m_fWheelBackLoad  = m_fMass * 9.81 * fabs(m_fWheelBackDist / m_fWheelBase);

    longspeed = m_vAcceleration.length() * 0.02 * m_fMass / m_fWheelBase;
    m_fWheelFrontLoad -= longspeed;
    m_fWheelBackLoad += longspeed;
    m_sWheels[0].fLoad = m_fWheelBackLoad / 2.0;
    m_sWheels[1].fLoad = m_fWheelBackLoad / 2.0;
    m_sWheels[2].fLoad = m_fWheelFrontLoad / 2.0;
    m_sWheels[3].fLoad = m_fWheelFrontLoad / 2.0;

    float fMassShifted     = m_fTireRadius * M_PI;
    m_fTireRotationalSpeed = dots[3] / (fMassShifted + fMassShifted);
    m_iRPM                 = m_fTireRotationalSpeed * m_fGearRatio[m_iGear] * m_fDifferentialRatio * 10;
    m_fMaxTraction =
        TorqueLookup(m_iRPM) * m_fGearRatio[m_iGear] * m_fDifferentialRatio * m_fGearEfficiency / m_fTireRadius;
    m_fTractionForce = m_fAccelerator * m_fMaxTraction;

    if (m_bIsBraking && dots[3] > 0) {
        m_fTractionForce = -fSign(dots[3]) * 1000 * dots[3];
    }

    for (i = 0; i < 4; i++) {
        float maxgrip;

        maxgrip = m_sWheels[i].fTraction * m_sWheels[i].fLoad * 9.81;
        if (m_fTractionForce > maxgrip) {
            m_fTractionForce       = maxgrip * 0.6;
            m_sWheels[i].bSpinning = true;
            m_sWheels[i].bSkidding = true;
            Com_Printf("Skidding!\n");
        }

        if (m_fTractionForce < -maxgrip) {
            m_fTractionForce       = -maxgrip * 0.6;
            m_sWheels[i].bSkidding = true;
            Com_Printf("Skidding!\n");
        }

        m_sWheels[i].fLongForce = this->m_sWheels[i].fLongForce + m_fTractionForce;
        m_sWheels[i].vLongForce += m_sWheels[i].vOrientation * m_sWheels[i].fLongForce;
    }

    m_vResistance[0] =
        -(velocity[0] * 0.02 * m_fRollingResistance + velocity[0] * 0.02 * m_fDrag * fabs(velocity[0] * 0.02));
    m_vResistance[1] =
        -(velocity[1] * 0.02 * m_fRollingResistance + velocity[1] * 0.02 * m_fDrag * fabs(velocity[1] * 0.02));
    m_vResistance[2] =
        -(velocity[2] * 0.02 * m_fRollingResistance + velocity[2] * 0.02 * m_fDrag * fabs(velocity[2] * 0.02));

    m_fBrakingPerformance = -m_fRollingResistance - *vpn * Vector(0, 0, -1) * m_fMass;
    if (dots[3] == 0) {
        m_fBrakingPerformance = 0;
    } else {
        m_fBrakingPerformance = (dots[3] - (m_fTireRotationalSpeed * m_fTireRadius - m_fBrakingPerformance)) / dots[3];
        m_fBrakingPerformance = (1.0 - m_fBrakingPerformance) * -(-m_fFrontBrakingForce - m_fBackBrakingForce);
    }

    if (r_velocity.length() * 0.02 > 0.01) {
        m_fTangForce = 0;
        m_vTangForce = vec_zero;

        for (i = 0; i < 2; i++) {
            Vector va, vr;
            Vector vVel;
            Vector v1, v2;

            va   = angles;
            va.x = 0;
            va.y += RAD2DEG(m_sWheels[i].fYawOffset);
            va.z = 0;
            va.AngleVectorsLeft(NULL, &vr, NULL);

            vVel = m_sWheels[i].vVelocity + velocity;
            // get the direction
            v1 = vVel;
            v1.normalize();
            // get the angles
            vectoangles(v1, v2);

            m_sWheels[i].fSlipAngle = DEG2RAD(angledist(va.y - v2.y));
            m_sWheels[i].fGripCoef  = m_sWheels[i].fSlipAngle * 0.15;
            m_sWheels[i].fGripCoef  = Q_clamp_float(m_sWheels[i].fGripCoef, -1.2f, 1.2f);
            m_sWheels[i].fLatForce += m_sWheels[i].fGripCoef * m_sWheels[i].fLoad * 9.81;

            if (m_sWheels[i].fLoad * 2 * m_sWheels[i].fTraction < fabs(m_sWheels[i].fLatForce)) {
                m_sWheels[i].fLatForce = m_sWheels[i].fLatForce / 9.81;
                m_sWheels[i].bSkidding = true;
                Com_Printf("Skidding %i!\n", i);
            }

            m_sWheels[i].vLatForce += vr * m_sWheels[i].fLatForce;

            G_LineWidth(3.0);

            G_DebugLine(
                origin + Corners[i][0] * r_i + Corners[i][1] * r_j,
                origin + Corners[i][0] * r_i + Corners[i][1] * r_j + m_sWheels[i].vLatForce * 50 * (1.0 / m_fMass),
                0.7f,
                0.5f,
                0.2f,
                1.0f
            );
        }
    } else {
        m_fTangForce = 0;
        m_vTangForce = vec_zero;
    }

    m_vForce = vec_zero;

    for (i = 0; i < 4; i++) {
        m_sWheels[i].vForce = m_sWheels[i].vLatForce + m_sWheels[i].vLongForce;
        m_vForce += m_sWheels[i].vForce;
    }

    m_vForce += m_vResistance;

    G_LineWidth(5.0);

    G_DebugLine(origin, origin + m_vTangForce * (1.0 / m_fMass) * 50, 1.0, 0.0, 1.0, 1.0);

    m_vAcceleration = m_vForce * (1.0 / m_fMass) * 50;
    m_fTorque       = 0;

    for (i = 0; i < 4; i++) {
        m_sWheels[i].fTorque += m_sWheels[i].fLongTorque + m_sWheels[i].fLatTorque;
        m_fTorque += m_sWheels[i].fTorque;
    }

    m_vAngularAcceleration[1] = angledist(RAD2DEG(m_fTorque));
    m_vAngularVelocity += m_vAngularAcceleration * level.frametime;
    m_vAngularVelocity *= 0.825f;
    avelocity = m_vAngularVelocity;

    if (m_bAutomatic) {
        //
        // set the gear
        //
        if (m_iRPM >= 2100 && m_iGear <= 2) {
            m_iGear++;
        }
        if (m_iRPM <= 600 && m_iGear > 1) {
            m_iGear--;
        }
    }

    prev_moveimpulse = moveimpulse;
}

void VehicleWheelsX4::Killed(Event *ev)
{
    deadflag = DEAD_DEAD;
}

float VehicleWheelsX4::TorqueLookup(int rpm)
{
    if (rpm <= 2099) {
        return 20.0;
    }
    if (rpm > 3599) {
        return 0.0;
    }
    return (20 * (3600 - rpm)) * 0.001;
}

void VehicleWheelsX4::Think()
{
    vmove_t vm;
    flags |= FL_POSTTHINK;

    prev_velocity = velocity;

    angles -= m_vAnglesOffset;
    origin -= m_vOriginOffset;

    mins = m_vOldMins;
    maxs = m_vOldMaxs;

    SetMoveInfo(&vm);
    VmoveSingle(&vm);
    GetMoveInfo(&vm);
}

void VehicleWheelsX4::Postthink()
{
    float        turn;
    Vector       i, j, k;
    int          index;
    trace_t      trace;
    Vector       normalsum;
    Vector       normalsum_Front;
    int          numnormals_Front;
    Vector       normalsum_Back;
    int          numnormals_Back;
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
        } else {
            moveimpulse *= 0.825f;
            turnimpulse *= 0.825f;
        }

        currentspeed = moveimpulse;
        turnangle    = turnangle * 0.25f + turnimpulse;
        turnangle    = Q_clamp_float(turnangle, -maxturnrate, maxturnrate);

        if (turnangle > maxturnrate) {
            turnangle = maxturnrate;
        } else if (turnangle < -maxturnrate) {
            turnangle = -maxturnrate;
        } else if (fabs(turnangle) < 2) {
            turnangle = 0;
        }

        real_velocity     = origin - prev_origin;
        prev_origin       = origin;
        prev_acceleration = real_acceleration;
        real_acceleration = real_velocity - prev_velocity;
        prev_velocity     = real_velocity;
        acceleration      = real_acceleration - prev_acceleration;

        angles[0] = 0;
        angles[2] = 0;

        temp[0] = angles[0];
        temp[1] = angles[1];
        temp[2] = angles[2];
        temp.AngleVectorsLeft(&i, &j, &k);

        j = vec_zero - j;

        index            = 0;
        numnormals_Front = 0;
        numnormals_Back  = 0;

        for (index = 0; index < 4; index++) {
            Vector start, end;
            Vector boxoffset;
            Vector t_mins(-8, -8, -8);
            Vector t_maxs(8, 8, 8);

            boxoffset = Corners[index];

            start = origin + i * boxoffset[0] + j * boxoffset[1] + k * boxoffset[2];
            end   = start;
            end[2] -= maxtracedist;

            trace = G_Trace(start, mins, maxs, end, this, MASK_DAMAGEMODEL, qfalse, "Vehicle::PostThink Corners");

            if (trace.fraction != 1 && !trace.startsolid) {
                if (index > 2) {
                    normalsum_Front += trace.plane.normal;
                    numnormals_Front++;
                } else {
                    normalsum_Back += trace.plane.normal;
                    numnormals_Back++;
                }

                normalsum += trace.plane.normal;
            }
        }

        if (angles[2] < 60 || angles[2] > 300) {
            if (numnormals_Front > 1) {
                m_vFrontNormal = normalsum_Front * (1.0 / numnormals_Front);

                temp              = m_vFrontNormal;
                m_vFrontAngles[0] = -temp.CrossProduct(temp, j).toPitch();
                // no yaw
                m_vFrontAngles[1] = 0;

                temp              = m_vFrontNormal;
                m_vFrontAngles[2] = temp.CrossProduct(temp, i).toPitch();
            }

            if (numnormals_Back > 1) {
                m_vBackNormal = normalsum_Back * (1.0 / numnormals_Back);

                temp             = m_vBackNormal;
                m_vBackAngles[0] = -temp.CrossProduct(temp, j).toPitch();
                // no yaw
                m_vBackAngles[1] = 0;

                temp             = m_vBackNormal;
                m_vBackAngles[2] = temp.CrossProduct(temp, i).toPitch();
            }

            if (numnormals_Front + numnormals_Back > 1) {
                m_vBaseNormal = (normalsum_Front + normalsum_Back) * (1.0 / numnormals_Front + numnormals_Back);

                temp             = m_vBaseNormal;
                m_vBaseAngles[0] = -temp.CrossProduct(temp, j).toPitch();
                // no yaw
                m_vBaseAngles[1] = 0;

                temp             = m_vBaseNormal;
                m_vBaseAngles[2] = temp.CrossProduct(temp, i).toPitch();
            }
        }

        n_angles    = m_vBaseAngles - angles;
        n_angles[0] = angledist(n_angles[0]);
        n_angles[1] = angledist(n_angles[1]);
        n_angles[2] = angledist(n_angles[2]);

        angles[0] += n_angles[0] * 4.5 * level.frametime;
        angles[2] += n_angles[2] * 4.5 * level.frametime;

        m_vPrevNormal = m_vBaseNormal;

        if (currentspeed < 0 && velocity * i > 0) {
            m_fFrontBrakingForce = 0.2f;
            m_fBackBrakingForce  = 0.1f;
        } else {
            m_fFrontBrakingForce = 0;
            m_fBackBrakingForce  = 0;
        }

        turn = turnangle * 0.005;

        Vector _i, _j, _k;
        Vector vNewVpn = angles;
        vNewVpn.y += turnangle;

        vNewVpn.AngleVectorsLeft(&_i, &_j, &_k);
        UpdateVariables(&acceleration, &i, &j, &k, &_i, &_j, &_k);

        m_vAngles = angles;

        if (currentspeed < 0 && velocity * i > 0) {
            currentspeed = 0;
        }

        normalsum = vec_zero;

        for (index = 0; index < 4; index++) {
            Vector start, end;
            Vector boxoffset;
            Vector t_mins(-8, -8, -8);
            Vector t_maxs(8, 8, 8);

            boxoffset = Corners[index];

            start = origin + i * boxoffset[0] + j * boxoffset[1] + k * boxoffset[2] + Vector(0, 0, 10);
            end   = start;
            end[2] -= maxtracedist;

            trace = G_Trace(start, mins, maxs, end, this, MASK_DAMAGEMODEL, qfalse, "Vehicle::PostThink Corners");

            if (trace.fraction != 1.0) {
                float fCoef;

                if (index > 2) {
                    fCoef = m_fWheelFrontLoad / (m_fWheelFrontDist / m_fWheelBase * m_fMass);
                    fCoef = Q_clamp_float(fCoef, 0, 1);

                    normalsum += _i * currentspeed * fCoef;
                    normalsum += i * currentspeed * (1.0 - fCoef);
                } else {
                    fCoef = m_fWheelBackLoad / (m_fWheelBackDist / m_fWheelBase * m_fMass);
                    fCoef = Q_clamp_float(fCoef, 0, 1);

                    normalsum += i * currentspeed * fCoef;
                    normalsum += _i * currentspeed * (1.0 - fCoef);
                }
            }
        }

        AnglesToAxis(angles, orientation);

        vmove_t vm;
        SetMoveInfo(&vm);
        m_sMoveGrid->SetMoveInfo(&vm);
        m_sMoveGrid->SetOrientation(orientation);

        if (m_sMoveGrid->CheckStuck()) {
            angles = primal_angles;
            AnglesToAxis(angles, orientation);
        }

        m_fForwardForce = acceleration * i * 0.1;
        m_fBackForce += -m_vAnglesOffset[0] * m_fBouncyCoef;
        m_fBackForce *= m_fSpringyCoef;

        m_vAnglesOffset[0] += m_fForwardForce + m_fBackForce * 12.0 * level.frametime;
        m_vAnglesOffset[0] = Q_clamp_float(m_vAnglesOffset[0], m_fYawMin, m_fYawMax);

        m_fLeftForce += acceleration * j * 0.1;
        m_fRightForce += -m_vAnglesOffset[2] * m_fBouncyCoef;
        m_fRightForce *= m_fSpringyCoef;

        m_vAnglesOffset[2] += (m_fLeftForce + m_fRightForce) * 12.0 * level.frametime;
        m_vAnglesOffset[2] = Q_clamp_float(m_vAnglesOffset[2], m_fRollMin, m_fRollMax);

        m_fDownForce = acceleration[2];
        m_fUpForce += -m_vOriginOffset[2] * m_fBouncyCoef;
        m_fUpForce *= m_fSpringyCoef;

        m_vAnglesOffset[2] += (m_fDownForce + m_fUpForce) * 12.0 * level.frametime;
        m_vAnglesOffset[2] = Q_clamp_float(m_vAnglesOffset[2], m_fZMin, m_fZMax);

        if (groundentity && (angles[2] < 60 || angles[2] > 300)) {
            velocity += m_vAcceleration * level.frametime;
        } else {
            if (angles[2] > 60) {
                angles[2]; // FIXME: ??
            }

            velocity += m_vAcceleration * level.frametime;
        }

        angles += avelocity * level.frametime;
        setAngles(angles);

        prev_origin = origin;
    }

    drivespeed = acceleration * orientation[0];

    if (drivable && driver.ent) {
        str sound_name;

        if (currentspeed > 0) {
            sound_name = "snd_forward";
        } else if (currentspeed == 0) {
            sound_name = "snd_idle";
        } else {
            sound_name = "snd_backward";
        }

        LoopSound(sound_name);
    }

    i = orientation[0];
    j = orientation[1];
    k = orientation[2];

    last_origin = origin;

    vTmp = (angles - primal_angles) * level.frametime;
    if (vTmp.x > 180 || vTmp.x < -180) {
        vTmp.x = 0;
    }
    if (vTmp.y > 180 || vTmp.y < -180) {
        vTmp.y = 0;
    }
    if (vTmp.z > 180 || vTmp.z < -180) {
        vTmp.z = 0;
    }

    if (vTmp.x > -1 || vTmp.x < 1) {
        vTmp.x = 0;
    }
    if (vTmp.y > -1 || vTmp.y < 1) {
        vTmp.y = 0;
    }
    if (vTmp.z > -1 || vTmp.z < 1) {
        vTmp.z = 0;
    }

    avelocity = vTmp;
    origin += m_vOriginOffset;
    setOrigin(origin);

    angles += m_vAnglesOffset;
    setAngles(angles);

    if (driver.ent) {
        driver.ent->setOrigin(origin + i * driveroffset[0] + j * driveroffset[1] + k * driveroffset[2]);

        if (drivable) {
            // clear the driver velocity
            driver.ent->velocity = vec_zero;
            // set the driver angles to the vehicle's angle
            driver.ent->setAngles(angles);
        }
    }

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

    if (!driver.ent && !velocity.length() && groundentity && !(watertype & CONTENTS_LAVA)) {
        // make the vehicle stationary if it's completely immobile
        flags &= ~FL_POSTTHINK;
        if (drivable) {
            setMoveType(MOVETYPE_STATIONARY);
        }
    }
}
