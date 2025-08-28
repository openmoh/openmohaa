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
    m_vAngDelta   = vec_zero;
    m_vAngSpeed   = vec_zero;
    m_vTargetAng  = vec_zero;
    m_vCurrentAng = vec_zero;
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
    float diff;
    float deltaDiff;
    float factor;
    float maxChange;
    float maxChangeDelta;
    float minChange;
    float changeSpeed;
    float speed;
    int   i;

    factor      = 1.0;
    maxChange   = Q_max(360, g_bot_turn_speed->integer);
    minChange   = 20;
    changeSpeed = g_bot_turn_speed->integer;

    if (m_vTargetAng[PITCH] > 180) {
        m_vTargetAng[PITCH] -= 360;
    }

    for (i = 0; i < 2; i++) {
        m_vCurrentAng[i] = AngleMod(m_vCurrentAng[i]);
        m_vTargetAng[i]  = AngleMod(m_vTargetAng[i]);

        diff      = AngleDifference(m_vCurrentAng[i], m_vTargetAng[i]);
        deltaDiff = fabs(diff);

        maxChangeDelta = maxChange * level.frametime;
        if (maxChangeDelta > deltaDiff) {
            maxChangeDelta = deltaDiff;
        }

        if (deltaDiff >= minChange) {
            m_vAngSpeed[i] = Q_min(1.0, m_vAngSpeed[i] + changeSpeed * level.frametime);
            maxChangeDelta *= m_vAngSpeed[i];
        } else {
            m_vAngSpeed[i] = Q_max(0.0, m_vAngSpeed[i] - changeSpeed * level.frametime);
        }

        speed = diff * level.frametime * 10 * factor;

        m_vAngDelta[i]   = Q_clamp_float(speed, -maxChangeDelta, maxChangeDelta);
        m_vCurrentAng[i] = AngleMod(m_vCurrentAng[i] - m_vAngDelta[i]);
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
    Vector vDelta = vPos - controlledEntity->EyePosition();
    Vector vTarget;

    VectorNormalize(vDelta);
    vectoangles(vDelta, vTarget);

    SetTargetAngles(vTarget);
}
