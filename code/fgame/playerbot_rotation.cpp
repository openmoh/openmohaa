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
// playerbot_rotation.cpp: Manages bot rotation

#include "playerbot.h"

BotRotation::BotRotation()
{
    m_vAngSpeed     = vec_zero;
    m_vTargetAng    = vec_zero;
    m_vCurrentAng   = vec_zero;
    m_fYawSpeedMult = 1.0f;
}

void BotRotation::SetControlledEntity(Player *newEntity)
{
    controlledEntity = newEntity;
}

float AngleDifference(float ang1, float ang2)
{
    float diff;

    diff = ang1 - ang2;
    if (ang1 > ang2) {
        if (diff > 180.0) {
            diff -= 360.0;
        }
    } else {
        if (diff < -180.0) {
            diff += 360.0;
        }
    }
    return diff;
}

void BotRotation::TurnThink(usercmd_t& botcmd, usereyes_t& eyeinfo)
{
    float diff, factor, maxchange, anglespeed, desired_speed;
    int   i;

    if (m_vTargetAng[PITCH] > 180) {
        m_vTargetAng[PITCH] -= 360;
    }

    factor    = 0.25f;
    maxchange = 360;

    if (maxchange < 240) {
        maxchange = 240;
    }

    maxchange *= level.frametime;

    for (i = 0; i < 2; i++) {
        //over reaction view model
        m_vCurrentAng[i] = AngleMod(m_vCurrentAng[i]);
        m_vTargetAng[i]  = AngleMod(m_vTargetAng[i]);
        diff             = AngleDifference(m_vCurrentAng[i], m_vTargetAng[i]);
        desired_speed    = diff * factor;

        m_vAngSpeed[i] = Q_clamp_float(m_vAngSpeed[i] + (m_vAngSpeed[i] - desired_speed), -180, 180);
        anglespeed     = Q_clamp_float(m_vAngSpeed[i], -maxchange, maxchange);

        m_vCurrentAng[i] += anglespeed;
        m_vCurrentAng[i] = AngleMod(m_vCurrentAng[i]);

        //demping
        m_vAngSpeed[i] *= 0.2 * (1 - factor);
    }

    if (m_vCurrentAng[PITCH] > 180) {
        m_vCurrentAng[PITCH] -= 360;
    }

    eyeinfo.angles[0] = m_vCurrentAng[0];
    eyeinfo.angles[1] = m_vCurrentAng[1];
    botcmd.angles[0]  = ANGLE2SHORT(m_vCurrentAng[0]) - controlledEntity->client->ps.delta_angles[0];
    botcmd.angles[1]  = ANGLE2SHORT(m_vCurrentAng[1]) - controlledEntity->client->ps.delta_angles[1];
    botcmd.angles[2]  = ANGLE2SHORT(m_vCurrentAng[2]) - controlledEntity->client->ps.delta_angles[2];
}

/*
====================
GetTargetAngles

Return the target angle
====================
*/
const Vector& BotRotation::GetTargetAngles() const
{
    return m_vTargetAng;
}

/*
====================
SetTargetAngles

Set the bot's angle
====================
*/
void BotRotation::SetTargetAngles(Vector vAngles)
{
    m_vTargetAng = vAngles;
}

/*
====================
AimAt

Make the bot face to the specified direction
====================
*/
void BotRotation::AimAt(Vector vPos)
{
    Vector vDelta = vPos - controlledEntity->centroid;

    VectorNormalize(vDelta);
    vectoangles(vDelta, m_vTargetAng);
}
