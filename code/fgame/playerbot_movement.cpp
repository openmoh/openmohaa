#include "playerbot.h"
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
// playerbot_movement.cpp: Manages bot movements

#include "playerbot.h"
#include "debuglines.h"

BotMovement::BotMovement()
{
    controlledEntity = NULL;

    m_Path.SetFallHeight(400);
    m_bPathing     = false;
    m_bTempAway    = false;
    m_fAttractTime = 0;

    m_iCheckPathTime = 0;
    m_iTempAwayTime  = 0;
    m_iNumBlocks     = 0;
}

void BotMovement::SetControlledEntity(Player *newEntity)
{
    controlledEntity = newEntity;
}

void BotMovement::MoveThink(usercmd_t& botcmd)
{
    Vector vDir;
    Vector vAngles;
    Vector vWishDir;

    botcmd.forwardmove = 0;
    botcmd.rightmove   = 0;

    CheckAttractiveNodes();

    if (!IsMoving()) {
        return;
    }

    if (m_vCurrentOrigin != controlledEntity->origin) {
        m_vCurrentOrigin = controlledEntity->origin;

        if (m_Path.CurrentNode()) {
            // recalculate paths because of a new origin
            m_Path.ReFindPath(controlledEntity->origin, controlledEntity);
        }
    }

    if (m_bTempAway && level.inttime >= m_iTempAwayTime) {
        m_bTempAway = false;
        m_Path.FindPath(controlledEntity->origin, m_vTargetPos, controlledEntity, 0, NULL, 0);
    }

    if (!m_bTempAway) {
        if (m_Path.CurrentNode()) {
            m_Path.UpdatePos(controlledEntity->origin, 8);

            m_vCurrentGoal = controlledEntity->origin;
            VectorAdd2D(m_vCurrentGoal, m_Path.CurrentDelta(), m_vCurrentGoal);

            if (MoveDone()) {
                // Clear the path
                m_Path.Clear();
            }
        }
    }

    if (ai_debugpath->integer) {
        G_DebugLine(controlledEntity->centroid, m_vCurrentGoal + Vector(0, 0, 36), 1, 1, 0, 1);
    }

    // Check if we're blocked
    if (level.inttime >= m_iCheckPathTime) {
        m_iCheckPathTime = level.inttime + 1000;

        if (m_iNumBlocks >= 5) {
            // Give up
            ClearMove();
        }

        m_bTempAway = false;

        if (controlledEntity->groundentity || controlledEntity->client->ps.walking) {
            if (controlledEntity->GetMoveResult() >= MOVERESULT_BLOCKED
                || controlledEntity->velocity.lengthSquared() <= Square(8)) {
                m_bTempAway = true;
            } else if ((controlledEntity->origin - m_vLastCheckPos[0]).lengthSquared() <= Square(32)
                       && (controlledEntity->origin - m_vLastCheckPos[1]).lengthSquared() <= Square(32)) {
                m_bTempAway = true;
            }
        } else {
            // falling
            if (controlledEntity->GetMoveResult() >= MOVERESULT_BLOCKED) {
                // stuck while falling
                m_bTempAway = true;
            }
        }

        if (m_bTempAway) {
            m_bTempAway     = true;
            m_iTempAwayTime = level.inttime + 750;
            m_iNumBlocks++;

            // Try to backward a little
            m_Path.Clear();
            m_Path.ForceShortLookahead();
            m_vCurrentGoal = controlledEntity->origin + Vector(G_CRandom(512), G_CRandom(512), G_CRandom(512));
        } else {
            m_iNumBlocks = 0;

            if (!m_Path.CurrentNode()) {
                m_vTargetPos   = controlledEntity->origin + Vector(G_CRandom(512), G_CRandom(512), G_CRandom(512));
                m_vCurrentGoal = m_vTargetPos;
            }
        }

        m_vLastCheckPos[1] = m_vLastCheckPos[0];
        m_vLastCheckPos[0] = controlledEntity->origin;
    }

    if (ai_debugpath->integer) {
        PathInfo *pos = m_Path.CurrentNode();

        if (pos != NULL) {
            while (pos != m_Path.LastNode()) {
                Vector vStart = pos->point + Vector(0, 0, 32);

                pos--;

                Vector vEnd = pos->point + Vector(0, 0, 32);

                G_DebugLine(vStart, vEnd, 1, 0, 0, 1);
            }
        }
    }

    if (m_Path.CurrentNode()) {
        if ((m_vTargetPos - controlledEntity->origin).lengthSquared() <= Square(16)) {
            ClearMove();
        }
    } else {
        if ((m_vTargetPos - controlledEntity->origin).lengthXYSquared() <= Square(16)) {
            ClearMove();
        }
    }

    // Rotate the dir
    if (m_Path.CurrentNode()) {
        vDir[0] = m_Path.CurrentDelta()[0];
        vDir[1] = m_Path.CurrentDelta()[1];
    } else {
        vDir = m_vCurrentGoal - controlledEntity->origin;
    }
    vDir[2] = 0;

    VectorNormalize2D(vDir);
    vAngles = vDir.toAngles() - controlledEntity->angles;
    vAngles.AngleVectorsLeft(&vWishDir);

    m_vLastValidDir  = vDir;
    m_vLastValidGoal = m_vCurrentGoal;

    // Forward to the specified direction
    float x = vWishDir.x * 127;
    float y = -vWishDir.y * 127;

    botcmd.forwardmove = (signed char)Q_clamp(x, -127, 127);
    botcmd.rightmove   = (signed char)Q_clamp(y, -127, 127);

    CheckJump(botcmd);
}

void BotMovement::CheckAttractiveNodes()
{
    for (int i = m_attractList.NumObjects(); i > 0; i--) {
        nodeAttract_t *a = m_attractList.ObjectAt(i);

        if (a->m_pNode == NULL || !a->m_pNode->CheckTeam(controlledEntity) || level.time > a->m_fRespawnTime) {
            delete a;
            m_attractList.RemoveObjectAt(i);
        }
    }
}

void BotMovement::CheckEndPos(Entity *entity)
{
    Vector  start;
    Vector  end;
    trace_t trace;

    if (!m_Path.LastNode()) {
        return;
    }

    start = m_Path.LastNode()->point;
    end   = m_vTargetPos;

    trace =
        G_Trace(start, entity->mins, entity->maxs, end, entity, MASK_TARGETPATH, true, "BotController::CheckEndPos");

    if (trace.fraction < 0.95f) {
        m_vTargetPos = trace.endpos;
    }
}

void BotMovement::CheckJump(usercmd_t& botcmd)
{
    Vector  start;
    Vector  end;
    Vector  dir;
    trace_t trace;

    if (controlledEntity->GetLadder()) {
        if (!botcmd.upmove) {
            botcmd.upmove = 127;
        } else {
            botcmd.upmove = 0;
        }
        return;
    }

    dir = m_vLastValidDir;

    start = controlledEntity->origin + Vector(0, 0, STEPSIZE);
    end =
        controlledEntity->origin + Vector(0, 0, STEPSIZE) + dir * (controlledEntity->maxs.y - controlledEntity->mins.y);

    if (ai_debugpath->integer) {
        G_DebugLine(start, end, 1, 0, 1, 1);
    }

    // Check if the bot needs to jump
    trace = G_Trace(
        start,
        controlledEntity->mins,
        controlledEntity->maxs,
        end,
        controlledEntity,
        MASK_PLAYERSOLID,
        false,
        "BotController::CheckJump"
    );

    // No need to jump
    if (trace.fraction > 0.5f) {
        botcmd.upmove = 0;
        return;
    }

    start = controlledEntity->origin;
    end   = controlledEntity->origin + Vector(0, 0, STEPSIZE * 3);

    if (ai_debugpath->integer) {
        G_DebugLine(start, end, 1, 0, 1, 1);
    }

    // Check if the bot can jump up
    trace = G_Trace(
        start,
        controlledEntity->mins,
        controlledEntity->maxs,
        end,
        controlledEntity,
        MASK_PLAYERSOLID,
        true,
        "BotController::CheckJump"
    );

    start = trace.endpos;
    end   = trace.endpos + dir * (controlledEntity->maxs.y - controlledEntity->mins.y);

    if (ai_debugpath->integer) {
        G_DebugLine(start, end, 1, 0, 1, 1);
    }

    Vector bounds[2];
    bounds[0] = Vector(controlledEntity->mins[0], controlledEntity->mins[1], 0);
    bounds[1] = Vector(
        controlledEntity->maxs[0],
        controlledEntity->maxs[1],
        (controlledEntity->maxs[0] + controlledEntity->maxs[1]) * 0.5
    );

    // Check if the bot can jump at the location
    trace = G_Trace(
        start, bounds[0], bounds[1], end, controlledEntity, MASK_PLAYERSOLID, false, "BotController::CheckJump"
    );

    if (trace.fraction < 1) {
        botcmd.upmove = 0;
        return;
    }

    // Make the bot climb walls
    if (!botcmd.upmove) {
        botcmd.upmove = 127;
    } else {
        botcmd.upmove = 0;
    }
}

/*
====================
AvoidPath

Avoid the specified position within the radius and start from a direction
====================
*/
void BotMovement::AvoidPath(
    Vector vAvoid, float fAvoidRadius, Vector vPreferredDir, float *vLeashHome, float fLeashRadius
)
{
    Vector vDir;

    if (vPreferredDir == vec_zero) {
        vDir = controlledEntity->origin - vAvoid;
        VectorNormalizeFast(vDir);
    } else {
        vDir = vPreferredDir;
    }

    m_Path.FindPathAway(
        controlledEntity->origin, vAvoid, vDir, controlledEntity, fAvoidRadius, vLeashHome, fLeashRadius * fLeashRadius
    );
    NewMove();

    if (!m_Path.CurrentNode()) {
        // Random movements
        m_vTargetPos = controlledEntity->origin + Vector(G_Random(256) - 128, G_Random(256) - 128, G_Random(256) - 128);
        m_vCurrentGoal = m_vTargetPos;
        return;
    }

    m_vTargetPos = m_Path.LastNode()->point;
}

/*
====================
MoveNear

Move near the specified position within the radius
====================
*/
void BotMovement::MoveNear(Vector vNear, float fRadius, float *vLeashHome, float fLeashRadius)
{
    m_Path.FindPathNear(
        controlledEntity->origin, vNear, controlledEntity, 0, fRadius * fRadius, vLeashHome, fLeashRadius * fLeashRadius
    );
    NewMove();

    if (!m_Path.CurrentNode()) {
        m_bPathing = false;
        return;
    }

    m_vTargetPos = m_Path.LastNode()->point;
}

/*
====================
MoveTo

Move to the specified position
====================
*/
void BotMovement::MoveTo(Vector vPos, float *vLeashHome, float fLeashRadius)
{
    m_vTargetPos = vPos;
    m_Path.FindPath(
        controlledEntity->origin, m_vTargetPos, controlledEntity, 0, vLeashHome, fLeashRadius * fLeashRadius
    );

    NewMove();

    if (!m_Path.CurrentNode()) {
        m_bPathing = false;
        return;
    }

    CheckEndPos(controlledEntity);
}

/*
====================
MoveToBestAttractivePoint

Move to the nearest attractive point with a minimum priority
Returns true if no attractive point was found
====================
*/
bool BotMovement::MoveToBestAttractivePoint(int iMinPriority)
{
    Container<AttractiveNode *> list;
    AttractiveNode             *bestNode;
    float                       bestDistanceSquared;
    int                         bestPriority;

    if (m_pPrimaryAttract) {
        MoveTo(m_pPrimaryAttract->origin);

        if (!IsMoving()) {
            m_pPrimaryAttract = NULL;
        } else {
            if (MoveDone()) {
                if (!m_fAttractTime) {
                    m_fAttractTime = level.time + m_pPrimaryAttract->m_fMaxStayTime;
                }
                if (level.time > m_fAttractTime) {
                    nodeAttract_t *a  = new nodeAttract_t;
                    a->m_fRespawnTime = level.time + m_pPrimaryAttract->m_fRespawnTime;
                    a->m_pNode        = m_pPrimaryAttract;

                    m_pPrimaryAttract = NULL;
                }
            }

            return true;
        }
    }

    if (!attractiveNodes.NumObjects()) {
        return false;
    }

    bestNode            = NULL;
    bestDistanceSquared = 99999999.0f;
    bestPriority        = iMinPriority;

    for (int i = attractiveNodes.NumObjects(); i > 0; i--) {
        AttractiveNode *node = attractiveNodes.ObjectAt(i);
        float           distSquared;
        bool            m_bRespawning = false;

        for (int j = m_attractList.NumObjects(); j > 0; j--) {
            AttractiveNode *node2 = m_attractList.ObjectAt(j)->m_pNode;

            if (node2 == node) {
                m_bRespawning = true;
                break;
            }
        }

        if (m_bRespawning) {
            continue;
        }

        if (node->m_iPriority < bestPriority) {
            continue;
        }

        if (!node->CheckTeam(controlledEntity)) {
            continue;
        }

        distSquared = VectorLengthSquared(controlledEntity->origin - node->origin);

        if (node->m_fMaxDistanceSquared >= 0 && distSquared > node->m_fMaxDistanceSquared) {
            continue;
        }

        if (!CanMoveTo(node->origin)) {
            continue;
        }

        if (distSquared < bestDistanceSquared) {
            bestDistanceSquared = distSquared;
            bestNode            = node;
            bestPriority        = node->m_iPriority;
        }
    }

    if (bestNode) {
        m_pPrimaryAttract = bestNode;
        m_fAttractTime    = 0;
        MoveTo(bestNode->origin);
        return true;
    } else {
        // No attractive point found
        return false;
    }
}

/*
====================
NewMove

Called when there is a new move
====================
*/
void BotMovement::NewMove()
{
    m_bPathing         = true;
    m_vLastCheckPos[0] = controlledEntity->origin;
    m_vLastCheckPos[1] = controlledEntity->origin;
}

/*
====================
CanMoveTo

Returns true if the bot has done moving
====================
*/
bool BotMovement::CanMoveTo(Vector vPos)
{
    return m_Path.DoesTheoreticPathExist(controlledEntity->origin, vPos, NULL, 0, NULL, 0);
}

/*
====================
MoveDone

Returns true if the bot has done moving
====================
*/
bool BotMovement::MoveDone()
{
    PathInfo *next;

    if (!m_bPathing) {
        return true;
    }

    if (m_bTempAway) {
        return false;
    }

    if (!m_Path.CurrentNode()) {
        return true;
    }

    Vector delta = Vector(m_Path.CurrentPathGoal()) - controlledEntity->origin;
    if (delta.lengthXYSquared() < Square(16) && delta.z < controlledEntity->maxs.z) {
        return true;
    }

    return false;
}

/*
====================
IsMoving

Returns true if the bot has a current path
====================
*/
bool BotMovement::IsMoving(void)
{
    return m_bPathing;
}

/*
====================
ClearMove

Stop the bot from moving
====================
*/
void BotMovement::ClearMove(void)
{
    m_Path.Clear();
    m_bPathing   = false;
    m_iNumBlocks = 0;
}

/*
====================
GetCurrentGoal

Return the current goal, usually the nearest node the player should look at
====================
*/
Vector BotMovement::GetCurrentGoal() const
{
    if (!m_Path.CurrentNode()) {
        return m_vCurrentGoal;
    }

    if (!m_Path.Complete(controlledEntity->origin)) {
        return controlledEntity->origin + Vector(m_Path.CurrentDelta()[0], m_Path.CurrentDelta()[1], 0);
    }

    return controlledEntity->origin;
}
