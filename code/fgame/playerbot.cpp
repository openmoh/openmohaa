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
// playerbot.cpp: Multiplayer bot system.

#include "g_local.h"
#include "actor.h"
#include "playerbot.h"
#include "consoleevent.h"
#include "debuglines.h"
#include "scriptexception.h"

// We assume that we have limited access to the server-side
// and that most logic come from the playerstate_s structure

cvar_t *bot_manualmove;

CLASS_DECLARATION(Player, PlayerBot, NULL) {
    {&EV_Killed,           &PlayerBot::Killed        },
    {&EV_GotKill,          &PlayerBot::GotKill       },
    {&EV_Player_StuffText, &PlayerBot::EventStuffText},
    {NULL,                 NULL                      }
};

PlayerBot::botfunc_t PlayerBot::botfuncs[MAX_BOT_FUNCTIONS];

PlayerBot::PlayerBot()
{
    entflags |= EF_BOT;

    if (LoadingSavegame) {
        return;
    }

    m_Path.SetFallHeight(96);
    m_bPathing   = false;
    m_bTempAway  = false;
    m_bDeltaMove = true;

    m_botCmd.serverTime = 0;
    m_botCmd.msec       = 0;
    m_botCmd.buttons    = 0;
    m_botCmd.angles[0]  = ANGLE2SHORT(0);
    m_botCmd.angles[1]  = ANGLE2SHORT(0);
    m_botCmd.angles[2]  = ANGLE2SHORT(0);

    m_botCmd.forwardmove = 0;
    m_botCmd.rightmove   = 0;
    m_botCmd.upmove      = 0;

    m_botEyes.angles[0] = 0;
    m_botEyes.angles[1] = 0;
    m_botEyes.ofs[0]    = 0;
    m_botEyes.ofs[1]    = 0;
    m_botEyes.ofs[2]    = viewheight;

    m_vAngSpeed      = vec_zero;
    m_vTargetAng     = vec_zero;
    m_vCurrentAng    = vec_zero;
    m_iCheckPathTime = 0;
    m_iTempAwayTime  = 0;
    m_fYawSpeedMult  = 1.0f;

    m_iCuriousTime = 0;
    m_iAttackTime  = 0;

    m_StateFlags = 0;
    m_RunLabel.TrySetScript("global/bot_run.scr");
}

void PlayerBot::Init(void)
{
    bot_manualmove = gi.Cvar_Get("bot_manualmove", "0", 0);

    for (int i = 0; i < MAX_BOT_FUNCTIONS; i++) {
        botfuncs[i].BeginState = &PlayerBot::State_DefaultBegin;
        botfuncs[i].EndState   = &PlayerBot::State_DefaultEnd;
    }

    InitState_Attack(&botfuncs[0]);
    InitState_Curious(&botfuncs[1]);
    InitState_Grenade(&botfuncs[2]);
    InitState_Idle(&botfuncs[3]);
    InitState_Weapon(&botfuncs[4]);
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

void PlayerBot::TurnThink(void)
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
        m_vAngSpeed[i] += (m_vAngSpeed[i] - desired_speed);

        if (m_vAngSpeed[i] > 180) {
            m_vAngSpeed[i] = maxchange;
        }

        if (m_vAngSpeed[i] < -180) {
            m_vAngSpeed[i] = -maxchange;
        }

        anglespeed = m_vAngSpeed[i];

        if (anglespeed > maxchange) {
            anglespeed = maxchange;
        }

        if (anglespeed < -maxchange) {
            anglespeed = -maxchange;
        }

        m_vCurrentAng[i] += anglespeed;
        m_vCurrentAng[i] = AngleMod(m_vCurrentAng[i]);

        //demping
        m_vAngSpeed[i] *= 0.45 * (1 - factor);
    }

    if (m_vCurrentAng[PITCH] > 180) {
        m_vCurrentAng[PITCH] -= 360;
    }

    m_botEyes.angles[0] = m_vCurrentAng[0];
    m_botEyes.angles[1] = m_vCurrentAng[1];
    m_botCmd.angles[0]  = ANGLE2SHORT(m_vCurrentAng[0]) - client->ps.delta_angles[0];
    m_botCmd.angles[1]  = ANGLE2SHORT(m_vCurrentAng[1]) - client->ps.delta_angles[1];
    m_botCmd.angles[2]  = ANGLE2SHORT(m_vCurrentAng[2]) - client->ps.delta_angles[2];
}

void PlayerBot::CheckAttractiveNodes(void)
{
    for (int i = m_attractList.NumObjects(); i > 0; i--) {
        nodeAttract_t *a = m_attractList.ObjectAt(i);

        if (a->m_pNode == NULL || !a->m_pNode->CheckTeam(this) || level.time > a->m_fRespawnTime) {
            delete a;
            m_attractList.RemoveObjectAt(i);
        }
    }
}

void PlayerBot::MoveThink(void)
{
    Vector vDir;
    Vector vAngles;
    Vector vWishDir;

    m_botCmd.forwardmove = 0;
    m_botCmd.rightmove   = 0;

    CheckAttractiveNodes();

    if (!IsMoving()) {
        return;
    }

    if (m_bTempAway && level.inttime >= m_iTempAwayTime) {
        m_bTempAway = false;
        m_Path.FindPath(origin, m_vTargetPos, this, 0, NULL, 0);
    }

    if (m_Path.CurrentNode()) {
        m_Path.UpdatePos(origin, 8);

        m_vCurrentGoal = origin;
        VectorAdd2D(m_vCurrentGoal, m_Path.CurrentDelta(), m_vCurrentGoal);

        if (m_Path.Complete(origin)) {
            // Clear the path
            m_Path.Clear();
        }
    } else if (!m_bTempAway) {
        m_vCurrentGoal = origin;
    }

    if (ai_debugpath->integer) {
        G_DebugLine(centroid, m_vCurrentGoal + Vector(0, 0, 36), 1, 1, 0, 1);
    }

    // Check if we're blocked
    if (level.inttime >= m_iCheckPathTime) {
        Vector end;

        m_bDeltaMove = false;

        m_iCheckPathTime = level.inttime + 2000;

        if (m_Path.CurrentNode() && m_Path.CurrentNode() != m_Path.LastNode()) {
            end = m_Path.NextNode()->point;
        } else {
            end = m_vCurrentGoal;
        }

        if (GetMoveResult() >= MOVERESULT_BLOCKED || velocity.lengthSquared() <= 8 * 8) {
            gi.DPrintf2("Updating path for bot client %i\n", edict - g_entities);

            m_bTempAway     = true;
            m_bDeltaMove    = false;
            m_iTempAwayTime = level.inttime + 750;

            // Try to backward a little
            //vDir = end - origin;
            //VectorNormalizeFast( vDir );
            //m_Path.FindPathAway( origin, origin + vDir * 16, vDir, this, 256, NULL, 0 );
            //m_Path.FindPathAway( origin, origin + m_vLastValidDir * 16, -m_vLastValidDir, this, 256, NULL, 0 );
            m_Path.Clear();
            m_vCurrentGoal = m_vLastValidGoal - m_vLastValidDir * 256;
            return;
        }
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

    if ((m_vTargetPos - origin).lengthSquared() <= 16 * 16) {
        ClearMove();
    }

    //vDir = m_vCurrentGoal - centroid;

    // Rotate the dir
    vDir[0] = m_Path.CurrentDelta()[0];
    vDir[1] = m_Path.CurrentDelta()[1];
    vDir[2] = 0;
    VectorNormalize(vDir);
    vAngles = vDir.toAngles() - angles;
    vAngles.AngleVectorsLeft(&vWishDir);

    m_vLastValidDir  = vWishDir;
    m_vLastValidGoal = m_vCurrentGoal;

    // Forward to the specified direction
    float x = vWishDir.x * 255.0f;
    float y = -vWishDir.y * 255.0f;

    m_botCmd.forwardmove = (signed char)(x > 127 ? 127 : x < -127 ? -127 : x);
    m_botCmd.rightmove   = (signed char)(y > 127 ? 127 : y < -127 ? -127 : y);
    CheckJump();

    Weapon *pWeap = GetActiveWeapon(WEAPON_MAIN);

    if (pWeap && !pWeap->ShouldReload()) {
        m_RunLabel.Execute(this);
    }
}

void PlayerBot::CheckJump(void)
{
    Vector  start;
    Vector  end;
    Vector  dir;
    trace_t trace;

    if (!m_Path.CurrentNode()) {
        return;
    }

    dir = m_Path.CurrentDelta();
    VectorNormalizeFast(dir);

    if (ai_debugpath->integer) {
        G_DebugLine(origin + Vector(0, 0, STEPSIZE), origin + Vector(0, 0, STEPSIZE) + dir * 32, 1, 0, 1, 1);
        G_DebugLine(origin + Vector(0, 0, 56), origin + Vector(0, 0, 56) + dir * 32, 1, 0, 1, 1);
    }

    Vector vStart = origin + Vector(0, 0, STEPSIZE);
    Vector vEnd   = origin + Vector(0, 0, STEPSIZE) + dir * 4;

    // Check if the bot needs to jump
    trace = G_Trace(vStart, mins, maxs, vEnd, this, MASK_PLAYERSOLID, false, "PlayerBot::CheckJump");

    // No need to jump
    if (trace.fraction >= 0.95f) {
        m_botCmd.upmove = 0;
        return;
    }

    start = origin + Vector(0, 0, 56);
    end   = origin + Vector(0, 0, 56) + dir * 4;

    // Check if the bot can jump
    trace = G_Trace(start, mins, maxs, end, this, MASK_PLAYERSOLID, true, "PlayerBot::CheckJump");

    // Make the bot climb walls
    if (trace.fraction >= 0.95f) {
        if (!m_botCmd.upmove) {
            m_botCmd.upmove = 127;
        } else {
            m_botCmd.upmove = 0;
        }
    } else {
        m_botCmd.upmove = 0;
    }
}

void PlayerBot::CheckEndPos(void)
{
    Vector  start;
    Vector  end;
    trace_t trace;

    if (!m_Path.LastNode()) {
        return;
    }

    start = m_Path.LastNode()->point;
    end   = m_vTargetPos;

    trace = G_Trace(start, mins, maxs, end, this, MASK_TARGETPATH, true, "PlayerBot::CheckEndPos");

    if (trace.fraction < 0.95f) {
        m_vTargetPos = trace.endpos;
    }
}

void PlayerBot::CheckUse(void)
{
    Vector  dir;
    Vector  start;
    Vector  end;
    trace_t trace;

    m_botCmd.buttons &= ~BUTTON_USE;

    angles.AngleVectorsLeft(&dir);

    start = origin + Vector(0, 0, viewheight);
    end   = origin + Vector(0, 0, viewheight) + dir * 32;

    trace = G_Trace(start, vec_zero, vec_zero, end, this, MASK_USABLE, false, "PlayerBot::CheckUse");

    // It may be a door
    if ((trace.allsolid || trace.startsolid || trace.fraction != 1.0f) && trace.entityNum) {
        m_botCmd.buttons |= BUTTON_USE;
    }
}

void PlayerBot::GetUsercmd(usercmd_t *ucmd)
{
    *ucmd = m_botCmd;
}

void PlayerBot::GetEyeInfo(usereyes_t *eyeinfo)
{
    *eyeinfo = m_botEyes;
}

void PlayerBot::UpdateBotStates(void)
{
    if (bot_manualmove->integer) {
        memset(&m_botCmd, 0, sizeof(usercmd_t));
        return;
    }

    if (IsDead()) {
        // The bot should respawn
        m_botCmd.buttons ^= BUTTON_ATTACKLEFT;
        return;
    }

    m_botCmd.buttons |= BUTTON_RUN;

    m_botEyes.ofs[0]    = 0;
    m_botEyes.ofs[1]    = 0;
    m_botEyes.ofs[2]    = viewheight;
    m_botEyes.angles[0] = 0;
    m_botEyes.angles[1] = 0;

    SetTargetAngles(Vector(0, 90, 0));

    CheckStates();

    MoveThink();
    TurnThink();
    CheckUse();
}

void PlayerBot::SendCommand(const char *text)
{
    char        *buffer;
    char        *data;
    size_t       len;
    ConsoleEvent ev;

    len = strlen(text) + 1;

    buffer = (char *)gi.Malloc(len);
    data   = buffer;
    Q_strncpyz(data, text, len);

    const char *com_token = COM_Parse(&data);

    if (!com_token) {
        return;
    }

    m_lastcommand = com_token;

    if (!Event::GetEvent(com_token)) {
        return;
    }

    ev = ConsoleEvent(com_token);

    if (!(ev.GetEventFlags(ev.eventnum) & EV_CONSOLE)) {
        gi.Free(buffer);
        return;
    }

    ev.SetConsoleEdict(edict);

    while (1) {
        com_token = COM_Parse(&data);

        if (!com_token || !*com_token) {
            break;
        }

        ev.AddString(com_token);
    }

    gi.Free(buffer);

    try {
        ProcessEvent(ev);
    } catch (ScriptException& exc) {
        gi.DPrintf("*** Bot Command Exception *** %s\n", exc.string.c_str());
    }
}

void PlayerBot::setAngles(Vector ang)
{
    Entity::setAngles(ang);
    SetTargetAngles(angles);
}

void PlayerBot::updateOrigin(void)
{
    Entity::updateOrigin();

    if (origin == client->ps.origin) {
        // no change because of Pmove
        return;
    }

    m_pPrimaryAttract = NULL;

    if (m_Path.CurrentNode()) {
        // recalculate paths because of a new origin
        m_Path.ReFindPath(origin, this);
    }
}

/*
====================
SetTargetAngles

Set the bot's angle
====================
*/
void PlayerBot::SetTargetAngles(Vector vAngles)
{
    m_vTargetAng = vAngles;
}

/*
====================
AimAt

Make the bot face to the specified direction
====================
*/
void PlayerBot::AimAt(Vector vPos)
{
    Vector vDelta = vPos - centroid;

    VectorNormalize(vDelta);
    vectoangles(vDelta, m_vTargetAng);
}

/*
====================
AimAtAimNode

Make the bot face toward the current path
====================
*/
void PlayerBot::AimAtAimNode(void)
{
    if (m_Path.CurrentDelta()) {
        AimAt(origin + m_Path.CurrentDelta());
    } else {
        AimAt(m_vCurrentGoal);
    }

    m_vTargetAng[PITCH] = 0;
}

/*
====================
CheckReload

Make the bot reload if necessary
====================
*/
void PlayerBot::CheckReload(void)
{
    Weapon *weap = GetActiveWeapon(WEAPON_MAIN);

    if (weap && weap->CheckReload(FIRE_PRIMARY)) {
        SendCommand("reload");
    }
}

/*
====================
MoveTo

Move to the specified position
====================
*/
void PlayerBot::MoveTo(Vector vPos, float *vLeashHome, float fLeashRadius)
{
    m_bPathing   = true;
    m_vTargetPos = vPos;
    m_Path.FindPath(origin, vPos, this, 0, vLeashHome, fLeashRadius * fLeashRadius);

    if (!m_Path.CurrentNode()) {
        m_bPathing = false;
        return;
    }

    CheckEndPos();
}

/*
====================
MoveTo

Move to the nearest attractive point with a minimum priority
Returns true if no attractive point was found
====================
*/
bool PlayerBot::MoveToBestAttractivePoint(int iMinPriority)
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

        if (!node->CheckTeam(this)) {
            continue;
        }

        distSquared = VectorLengthSquared(origin - node->origin);

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
CanMoveTo

Returns true if the bot has done moving
====================
*/
bool PlayerBot::CanMoveTo(Vector vPos)
{
    return m_Path.DoesTheoreticPathExist(origin, vPos, NULL, 0, NULL, 0);
}

/*
====================
MoveDone

Returns true if the bot has done moving
====================
*/
bool PlayerBot::MoveDone(void)
{
    return !m_Path.LastNode() || m_Path.Complete(origin);
}

/*
====================
IsMoving

Returns true if the bot has a current path
====================
*/
bool PlayerBot::IsMoving(void)
{
    return m_bPathing;
}

/*
====================
ClearMove

Stop the bot from moving
====================
*/
void PlayerBot::ClearMove(void)
{
    m_Path.Clear();
    m_bPathing = false;
}

/*
====================
MoveNear

Move near the specified position within the radius
====================
*/
void PlayerBot::MoveNear(Vector vNear, float fRadius, float *vLeashHome, float fLeashRadius)
{
    m_bPathing = true;
    m_Path.FindPathNear(origin, vNear, this, 0, fRadius * fRadius, vLeashHome, fLeashRadius * fLeashRadius);

    if (!m_Path.CurrentNode()) {
        m_bPathing = false;
        return;
    }

    m_vTargetPos = m_Path.LastNode()->point;
}

/*
====================
AvoidPath

Avoid the specified position within the radius and start from a direction
====================
*/
void PlayerBot::AvoidPath(
    Vector vAvoid, float fAvoidRadius, Vector vPreferredDir, float *vLeashHome, float fLeashRadius
)
{
    Vector vDir;

    if (vPreferredDir == vec_zero) {
        vDir = origin - vAvoid;
        VectorNormalizeFast(vDir);
    } else {
        vDir = vPreferredDir;
    }

    m_bPathing = true;
    m_Path.FindPathAway(origin, vAvoid, vDir, this, fAvoidRadius, vLeashHome, fLeashRadius * fLeashRadius);

    if (!m_Path.CurrentNode()) {
        m_bPathing = false;
        return;
    }

    m_vTargetPos = m_Path.LastNode()->point;
}

/*
====================
NoticeEvent

Warn the bot of an event
====================
*/
void PlayerBot::NoticeEvent(Vector vPos, int iType, Entity *pEnt, float fDistanceSquared, float fRadiusSquared)
{
    // Ignore teammates
    if (pEnt->IsSubclassOfPlayer()) {
        Player *p = (Player *)pEnt;

        if (p->GetTeam() == GetTeam()) {
            return;
        }
    } else if (pEnt->IsSubclassOfWeapon()) {
        Weapon *pWeap = (Weapon *)pEnt;

        if (pWeap->GetOwner()) {
            Player *p = (Player *)pWeap->GetOwner();

            if (p->IsSubclassOfPlayer() && p->GetTeam() == GetTeam()) {
                return;
            }
        }
    }

    switch (iType) {
    case AI_EVENT_MISC:
    case AI_EVENT_MISC_LOUD:
        break;
    case AI_EVENT_WEAPON_FIRE:
    case AI_EVENT_WEAPON_IMPACT:
    case AI_EVENT_EXPLOSION:
    case AI_EVENT_AMERICAN_VOICE:
    case AI_EVENT_GERMAN_VOICE:
    case AI_EVENT_AMERICAN_URGENT:
    case AI_EVENT_GERMAN_URGENT:
    case AI_EVENT_FOOTSTEP:
    case AI_EVENT_GRENADE:
    default:
        m_iCuriousTime    = level.inttime + 20000;
        m_vLastCuriousPos = vPos;
        break;
    }
}

/*
====================
ClearEnemy

Clear the bot's enemy
====================
*/
void PlayerBot::ClearEnemy(void)
{
    m_iAttackTime   = 0;
    m_pEnemy        = NULL;
    m_vOldEnemyPos  = vec_zero;
    m_vLastEnemyPos = vec_zero;
}

/*
====================
Bot states
--------------------
____________________
--------------------
____________________
--------------------
____________________
--------------------
____________________
====================
*/

void PlayerBot::CheckStates(void)
{
    m_StateCount = 0;

    for (int i = 0; i < MAX_BOT_FUNCTIONS; i++) {
        botfunc_t *func = &botfuncs[i];

        if (func->CheckCondition) {
            if ((this->*func->CheckCondition)()) {
                if (!(m_StateFlags & (1 << i))) {
                    m_StateFlags |= 1 << i;

                    if (func->BeginState) {
                        (this->*func->BeginState)();
                    }
                }

                if (func->ThinkState) {
                    m_StateCount++;
                    (this->*func->ThinkState)();
                }
            } else {
                if ((m_StateFlags & (1 << i))) {
                    m_StateFlags &= ~(1 << i);

                    if (func->EndState) {
                        (this->*func->EndState)();
                    }
                }
            }
        } else {
            if (func->ThinkState) {
                m_StateCount++;
                (this->*func->ThinkState)();
            }
        }
    }

    assert(m_StateCount);
    if (!m_StateCount) {
        gi.DPrintf("*** WARNING *** %s was stuck with no states !!!", client->pers.netname);
        State_Reset();
    }
}

/*
====================
Default state


====================
*/
void PlayerBot::State_DefaultBegin(void)
{
    ClearMove();
}

void PlayerBot::State_DefaultEnd(void) {}

void PlayerBot::State_Reset(void)
{
    m_iCuriousTime    = 0;
    m_iAttackTime     = 0;
    m_vLastCuriousPos = vec_zero;
    m_vOldEnemyPos    = vec_zero;
    m_vLastEnemyPos   = vec_zero;
    m_vLastDeathPos   = vec_zero;
    m_pEnemy          = NULL;
}

/*
====================
Idle state

Make the bot move to random directions
====================
*/
void PlayerBot::InitState_Idle(botfunc_t *func)
{
    func->CheckCondition = &PlayerBot::CheckCondition_Idle;
    func->ThinkState     = &PlayerBot::State_Idle;
}

bool PlayerBot::CheckCondition_Idle(void)
{
    if (m_iCuriousTime) {
        return false;
    }

    if (m_iAttackTime) {
        return false;
    }

    return true;
}

void PlayerBot::State_Idle(void)
{
    AimAtAimNode();
    CheckReload();

    if (!MoveToBestAttractivePoint() && !IsMoving()) {
        if (m_vLastDeathPos != vec_zero) {
            MoveTo(m_vLastDeathPos);

            if (MoveDone()) {
                m_vLastDeathPos = vec_zero;
            }
        } else {
            AvoidPath(origin - Vector(orientation[0]) * 128.0f, 1024);
        }
    }
}

/*
====================
Curious state

Forward to the last event position
====================
*/
void PlayerBot::InitState_Curious(botfunc_t *func)
{
    func->CheckCondition = &PlayerBot::CheckCondition_Curious;
    func->ThinkState     = &PlayerBot::State_Curious;
}

bool PlayerBot::CheckCondition_Curious(void)
{
    if (m_iAttackTime) {
        m_iCuriousTime = 0;
        return false;
    }

    if (level.inttime > m_iCuriousTime) {
        if (m_iCuriousTime) {
            ClearMove();
            m_iCuriousTime = 0;
        }

        return false;
    }

    return true;
}

void PlayerBot::State_Curious(void)
{
    //AimAt( m_vLastCuriousPos );
    AimAtAimNode();

    if (!MoveToBestAttractivePoint(3) && !IsMoving()) {
        MoveTo(m_vLastCuriousPos);
    }

    if (MoveDone()) {
        m_iCuriousTime = 0;
    }
}

/*
====================
Attack state

Attack the enemy
====================
*/
void PlayerBot::InitState_Attack(botfunc_t *func)
{
    func->CheckCondition = &PlayerBot::CheckCondition_Attack;
    func->EndState       = &PlayerBot::State_EndAttack;
    func->ThinkState     = &PlayerBot::State_Attack;
}

static Vector bot_origin;

static int sentients_compare(const void *elem1, const void *elem2)
{
    Entity *e1, *e2;
    float   delta[3];
    float   d1, d2;

    e1 = *(Entity **)elem1;
    e2 = *(Entity **)elem2;

    VectorSubtract(bot_origin, e1->origin, delta);
    d1 = VectorLengthSquared(delta);

    VectorSubtract(bot_origin, e2->origin, delta);
    d2 = VectorLengthSquared(delta);

    if (d2 <= d1) {
        return d1 > d2;
    } else {
        return -1;
    }
}

bool PlayerBot::CheckCondition_Attack(void)
{
    Container<Sentient *> sents = SentientList;

    bot_origin = origin;
    sents.Sort(sentients_compare);

    for (int i = 1; i <= sents.NumObjects(); i++) {
        Sentient *sent   = sents.ObjectAt(i);
        Player   *player = (Player *)sent;

        if (sent == this) {
            continue;
        }

        if (sent->hidden()) {
            continue;
        }

        if (sent->IsDead()) {
            continue;
        }

        if (sent->IsSubclassOfPlayer()) {
            if (g_gametype->integer >= GT_TEAM && player->GetTeam() == GetTeam()) {
                continue;
            }
        } else {
            if (sent->m_Team == m_Team) {
                continue;
            }
        }

        if (CanSee(sent, 80, world->m_fAIVisionDistance, false)) {
            m_pEnemy      = sent;
            m_iAttackTime = level.inttime + 10000;
            return true;
        }
    }

    if (level.inttime > m_iAttackTime) {
        if (m_iAttackTime) {
            ClearMove();
            m_iAttackTime = 0;
        }

        return false;
    }

    return true;
}

void PlayerBot::State_EndAttack(void)
{
    m_botCmd.buttons &= ~(BUTTON_ATTACKLEFT | BUTTON_ATTACKRIGHT);
}

void PlayerBot::State_Attack(void)
{
    Player *p                   = (Player *)m_pEnemy.Pointer();
    bool    bMelee              = false;
    float   fMinDistance        = 128;
    float   fMinDistanceSquared = fMinDistance * fMinDistance;

    if (!m_pEnemy || m_pEnemy->IsDead() || (!p->IsSubclassOfPlayer() && m_pEnemy->m_Team == m_Team)
        || (g_gametype->integer >= GT_TEAM && p->GetTeam() == GetTeam())) {
        m_iAttackTime = 0;
        return;
    }

    float fDistanceSquared = (m_pEnemy->origin - origin).lengthSquared();

    if (CanSee(m_pEnemy, 20, world->m_fAIVisionDistance, false)) {
        Weapon *pWeap = GetActiveWeapon(WEAPON_MAIN);

        if (!pWeap) {
            return;
        }

        float fPrimaryBulletRange          = pWeap->GetBulletRange(FIRE_PRIMARY) / 1.25f;
        float fPrimaryBulletRangeSquared   = fPrimaryBulletRange * fPrimaryBulletRange;
        float fSecondaryBulletRange        = pWeap->GetBulletRange(FIRE_SECONDARY);
        float fSecondaryBulletRangeSquared = fSecondaryBulletRange * fSecondaryBulletRange;

        fMinDistance = fPrimaryBulletRange;

        if (fMinDistance > 256) {
            fMinDistance = 256;
        }

        fMinDistanceSquared = fMinDistance * fMinDistance;

        if (client->ps.stats[STAT_AMMO] > 0 || client->ps.stats[STAT_CLIPAMMO] > 0) {
            if (fDistanceSquared <= fPrimaryBulletRangeSquared) {
                if (pWeap->IsSemiAuto()) {
                    m_botCmd.buttons ^= BUTTON_ATTACKLEFT;
                } else {
                    m_botCmd.buttons |= BUTTON_ATTACKLEFT;
                }
            } else {
                m_botCmd.buttons &= ~BUTTON_ATTACKLEFT;
            }
        } else if (pWeap->GetFireType(FIRE_SECONDARY) == FT_MELEE) {
            bMelee = true;

            if (fDistanceSquared <= fSecondaryBulletRangeSquared) {
                m_botCmd.buttons ^= BUTTON_ATTACKRIGHT;
            } else {
                m_botCmd.buttons &= ~BUTTON_ATTACKRIGHT;
            }
        } else {
            m_botCmd.buttons &= ~(BUTTON_ATTACKLEFT | BUTTON_ATTACKRIGHT);
        }
    } else {
        m_botCmd.buttons &= ~(BUTTON_ATTACKLEFT | BUTTON_ATTACKRIGHT);
        fMinDistanceSquared = 0;
    }

    AimAt(m_vLastEnemyPos);

    m_vOldEnemyPos  = m_vLastEnemyPos;
    m_vLastEnemyPos = m_pEnemy->centroid;

    if ((!MoveToBestAttractivePoint(5) && !IsMoving()) || (m_vOldEnemyPos != m_vLastEnemyPos && !MoveDone())) {
        if (!bMelee) {
            if ((origin - m_vLastEnemyPos).lengthSquared() < fMinDistanceSquared) {
                Vector vDir = origin - m_vLastEnemyPos;
                VectorNormalizeFast(vDir);

                AvoidPath(m_vLastEnemyPos, fMinDistance, vDir);
            } else {
                MoveNear(m_vLastEnemyPos, fMinDistance);
            }
        } else {
            MoveTo(m_vLastEnemyPos);
        }
    }
}

/*
====================
Grenade state

Avoid any grenades
====================
*/
void PlayerBot::InitState_Grenade(botfunc_t *func)
{
    func->CheckCondition = &PlayerBot::CheckCondition_Grenade;
    func->ThinkState     = &PlayerBot::State_Grenade;
}

bool PlayerBot::CheckCondition_Grenade(void)
{
    // FIXME: TODO
    return false;
}

void PlayerBot::State_Grenade(void)
{
    // FIXME: TODO
}

/*
====================
Weapon state

Change weapon when necessary
====================
*/
void PlayerBot::InitState_Weapon(botfunc_t *func)
{
    func->CheckCondition = &PlayerBot::CheckCondition_Weapon;
    func->BeginState     = &PlayerBot::State_BeginWeapon;
}

bool PlayerBot::CheckCondition_Weapon(void)
{
    return GetActiveWeapon(WEAPON_MAIN) != BestWeapon(NULL, false, WEAPON_CLASS_THROWABLE);
}

void PlayerBot::State_BeginWeapon(void)
{
    Weapon *weap = BestWeapon(NULL, false, WEAPON_CLASS_THROWABLE);

    if (weap == NULL) {
        SendCommand("safeholster 1");
        return;
    }

    SendCommand(va("use \"%s\"", weap->model.c_str()));
}

void PlayerBot::Spawned(void)
{
    ClearEnemy();
    m_iCuriousTime = 0;

    Player::Spawned();
}

void PlayerBot::Killed(Event *ev)
{
    Player::Killed(ev);

    // send the respawn buttons
    if (!(m_botCmd.buttons & BUTTON_ATTACKLEFT)) {
        m_botCmd.buttons |= BUTTON_ATTACKLEFT;
    } else {
        m_botCmd.buttons &= ~BUTTON_ATTACKLEFT;
    }

    m_botEyes.ofs[0]    = 0;
    m_botEyes.ofs[1]    = 0;
    m_botEyes.ofs[2]    = 0;
    m_botEyes.angles[0] = 0;
    m_botEyes.angles[1] = 0;

    m_vLastDeathPos = origin;
}

void PlayerBot::GotKill(Event *ev)
{
    Player::GotKill(ev);

    ClearEnemy();
    m_iCuriousTime = 0;
}

void PlayerBot::EventStuffText(Event *ev)
{
    SendCommand(ev->GetString(1));
}
