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
// playerbot.cpp: Multiplayer bot system.

#include "g_local.h"
#include "actor.h"
#include "playerbot.h"
#include "consoleevent.h"
#include "debuglines.h"
#include "scriptexception.h"
#include "vehicleturret.h"
#include "weaputils.h"

// We assume that we have limited access to the server-side
// and that most logic come from the playerstate_s structure

cvar_t *bot_manualmove;

CLASS_DECLARATION(Player, PlayerBot, NULL) {
    {&EV_Killed,           &PlayerBot::Killed        },
    {&EV_GotKill,          &PlayerBot::GotKill       },
    {&EV_Player_StuffText, &PlayerBot::EventStuffText},
    {NULL,                 NULL                      }
};

PlayerBot::PlayerBot()
{
    entflags |= ECF_BOT;
    controller = NULL;
}

void PlayerBot::setController(BotController *controlledBy)
{
    controller = controlledBy;
}

void PlayerBot::Spawned(void)
{
    controller->Spawned();

    Player::Spawned();
}

void PlayerBot::Killed(Event *ev)
{
    Player::Killed(ev);

    controller->Killed(ev);
}

void PlayerBot::GotKill(Event *ev)
{
    Player::GotKill(ev);

    controller->GotKill(ev);
}

CLASS_DECLARATION(Listener, BotController, NULL) {
    {NULL, NULL}
};

BotController::botfunc_t BotController::botfuncs[MAX_BOT_FUNCTIONS];

BotController::BotController()
{
    if (LoadingSavegame) {
        return;
    }

    m_Path.SetFallHeight(400);
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
    m_botEyes.ofs[2]    = DEFAULT_VIEWHEIGHT;

    m_vAngSpeed      = vec_zero;
    m_vTargetAng     = vec_zero;
    m_vCurrentAng    = vec_zero;
    m_iCheckPathTime = 0;
    m_iTempAwayTime  = 0;
    m_iNumBlocks     = 0;
    m_fYawSpeedMult  = 1.0f;

    m_iCuriousTime = 0;
    m_iAttackTime  = 0;

    m_iNextTauntTime = 0;

    m_StateFlags = 0;
    m_RunLabel.TrySetScript("global/bot_run.scr");
}

void BotController::Init(void)
{
    bot_manualmove = gi.Cvar_Get("bot_manualmove", "0", 0);

    for (int i = 0; i < MAX_BOT_FUNCTIONS; i++) {
        botfuncs[i].BeginState = &BotController::State_DefaultBegin;
        botfuncs[i].EndState   = &BotController::State_DefaultEnd;
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

void BotController::TurnThink(void)
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

    m_botEyes.angles[0] = m_vCurrentAng[0];
    m_botEyes.angles[1] = m_vCurrentAng[1];
    m_botCmd.angles[0]  = ANGLE2SHORT(m_vCurrentAng[0]) - controlledEnt->client->ps.delta_angles[0];
    m_botCmd.angles[1]  = ANGLE2SHORT(m_vCurrentAng[1]) - controlledEnt->client->ps.delta_angles[1];
    m_botCmd.angles[2]  = ANGLE2SHORT(m_vCurrentAng[2]) - controlledEnt->client->ps.delta_angles[2];
}

void BotController::CheckAttractiveNodes(void)
{
    for (int i = m_attractList.NumObjects(); i > 0; i--) {
        nodeAttract_t *a = m_attractList.ObjectAt(i);

        if (a->m_pNode == NULL || !a->m_pNode->CheckTeam(controlledEnt) || level.time > a->m_fRespawnTime) {
            delete a;
            m_attractList.RemoveObjectAt(i);
        }
    }
}

void BotController::MoveThink(void)
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
        m_Path.FindPath(controlledEnt->origin, m_vTargetPos, controlledEnt, 0, NULL, 0);
    }

    if (!m_bTempAway) {
        if (m_Path.CurrentNode()) {
            m_Path.UpdatePos(controlledEnt->origin, 8);

            m_vCurrentGoal = controlledEnt->origin;
            VectorAdd2D(m_vCurrentGoal, m_Path.CurrentDelta(), m_vCurrentGoal);

            if (MoveDone()) {
                // Clear the path
                m_Path.Clear();
            }
        }
    }

    if (ai_debugpath->integer) {
        G_DebugLine(controlledEnt->centroid, m_vCurrentGoal + Vector(0, 0, 36), 1, 1, 0, 1);
    }

    // Check if we're blocked
    if (level.inttime >= m_iCheckPathTime) {
        m_bDeltaMove = false;

        m_iCheckPathTime = level.inttime + 1000;

        if (m_iNumBlocks >= 5) {
            // Give up
            ClearMove();
        }

        m_bTempAway = false;

        if (controlledEnt->groundentity || controlledEnt->client->ps.walking) {
            if (controlledEnt->GetMoveResult() >= MOVERESULT_BLOCKED
                || controlledEnt->velocity.lengthSquared() <= Square(8)) {
                m_bTempAway = true;
            } else if ((controlledEnt->origin - m_vLastCheckPos[0]).lengthSquared() <= Square(32)
                       && (controlledEnt->origin - m_vLastCheckPos[1]).lengthSquared() <= Square(32)) {
                m_bTempAway = true;
            }
        } else {
            // falling
            if (controlledEnt->GetMoveResult() >= MOVERESULT_BLOCKED) {
                // stuck while falling
                m_bTempAway = true;
            }
        }

        if (m_bTempAway) {
            m_bTempAway     = true;
            m_bDeltaMove    = false;
            m_iTempAwayTime = level.inttime + 750;
            m_iNumBlocks++;

            // Try to backward a little
            m_Path.Clear();
            m_Path.ForceShortLookahead();
            m_vCurrentGoal = controlledEnt->origin + Vector(G_CRandom(512), G_CRandom(512), G_CRandom(512));
        } else {
            m_iNumBlocks = 0;

            if (!m_Path.CurrentNode()) {
                m_vTargetPos   = controlledEnt->origin + Vector(G_CRandom(512), G_CRandom(512), G_CRandom(512));
                m_vCurrentGoal = m_vTargetPos;
            }
        }

        m_vLastCheckPos[1] = m_vLastCheckPos[0];
        m_vLastCheckPos[0] = controlledEnt->origin;
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
        if ((m_vTargetPos - controlledEnt->origin).lengthSquared() <= Square(16)) {
            ClearMove();
        }
    } else {
        if ((m_vTargetPos - controlledEnt->origin).lengthXYSquared() <= Square(16)) {
            ClearMove();
        }
    }

    // Rotate the dir
    if (m_Path.CurrentNode()) {
        vDir[0] = m_Path.CurrentDelta()[0];
        vDir[1] = m_Path.CurrentDelta()[1];
    } else {
        vDir = m_vCurrentGoal - controlledEnt->origin;
    }
    vDir[2] = 0;

    VectorNormalize2D(vDir);
    vAngles = vDir.toAngles() - controlledEnt->angles;
    vAngles.AngleVectorsLeft(&vWishDir);

    m_vLastValidDir  = vDir;
    m_vLastValidGoal = m_vCurrentGoal;

    // Forward to the specified direction
    float x = vWishDir.x * 127;
    float y = -vWishDir.y * 127;

    m_botCmd.forwardmove = (signed char)Q_clamp(x, -127, 127);
    m_botCmd.rightmove   = (signed char)Q_clamp(y, -127, 127);
    CheckJump();

    Weapon *pWeap = controlledEnt->GetActiveWeapon(WEAPON_MAIN);

    if (pWeap && !pWeap->ShouldReload()) {
        m_RunLabel.Execute(this);
    }
}

void BotController::CheckJump(void)
{
    Vector  start;
    Vector  end;
    Vector  dir;
    trace_t trace;

    if (controlledEnt->GetLadder()) {
        if (!m_botCmd.upmove) {
            m_botCmd.upmove = 127;
        } else {
            m_botCmd.upmove = 0;
        }
        return;
    }

    dir = m_vLastValidDir;

    start = controlledEnt->origin + Vector(0, 0, STEPSIZE);
    end   = controlledEnt->origin + Vector(0, 0, STEPSIZE) + dir * (controlledEnt->maxs.y - controlledEnt->mins.y);

    if (ai_debugpath->integer) {
        G_DebugLine(start, end, 1, 0, 1, 1);
    }

    // Check if the bot needs to jump
    trace = G_Trace(
        start,
        controlledEnt->mins,
        controlledEnt->maxs,
        end,
        controlledEnt,
        MASK_PLAYERSOLID,
        false,
        "BotController::CheckJump"
    );

    // No need to jump
    if (trace.fraction > 0.5f) {
        m_botCmd.upmove = 0;
        return;
    }

    start = controlledEnt->origin;
    end   = controlledEnt->origin + Vector(0, 0, STEPSIZE * 3);

    if (ai_debugpath->integer) {
        G_DebugLine(start, end, 1, 0, 1, 1);
    }

    // Check if the bot can jump up
    trace = G_Trace(
        start,
        controlledEnt->mins,
        controlledEnt->maxs,
        end,
        controlledEnt,
        MASK_PLAYERSOLID,
        true,
        "BotController::CheckJump"
    );

    start = trace.endpos;
    end   = trace.endpos + dir * (controlledEnt->maxs.y - controlledEnt->mins.y);

    if (ai_debugpath->integer) {
        G_DebugLine(start, end, 1, 0, 1, 1);
    }

    Vector bounds[2];
    bounds[0] = Vector(controlledEnt->mins[0], controlledEnt->mins[1], 0);
    bounds[1] =
        Vector(controlledEnt->maxs[0], controlledEnt->maxs[1], (controlledEnt->maxs[0] + controlledEnt->maxs[1]) * 0.5);

    // Check if the bot can jump at the location
    trace =
        G_Trace(start, bounds[0], bounds[1], end, controlledEnt, MASK_PLAYERSOLID, false, "BotController::CheckJump");

    if (trace.fraction < 1) {
        m_botCmd.upmove = 0;
        return;
    }

    // Make the bot climb walls
    if (!m_botCmd.upmove) {
        m_botCmd.upmove = 127;
    } else {
        m_botCmd.upmove = 0;
    }
}

void BotController::CheckEndPos(void)
{
    Vector  start;
    Vector  end;
    trace_t trace;

    if (!m_Path.LastNode()) {
        return;
    }

    start = m_Path.LastNode()->point;
    end   = m_vTargetPos;

    trace = G_Trace(
        start,
        controlledEnt->mins,
        controlledEnt->maxs,
        end,
        controlledEnt,
        MASK_TARGETPATH,
        true,
        "BotController::CheckEndPos"
    );

    if (trace.fraction < 0.95f) {
        m_vTargetPos = trace.endpos;
    }
}

void BotController::CheckUse(void)
{
    Vector  dir;
    Vector  start;
    Vector  end;
    trace_t trace;

    controlledEnt->angles.AngleVectorsLeft(&dir);

    start = controlledEnt->origin + Vector(0, 0, controlledEnt->viewheight);
    end   = controlledEnt->origin + Vector(0, 0, controlledEnt->viewheight) + dir * 32;

    trace = G_Trace(start, vec_zero, vec_zero, end, controlledEnt, MASK_USABLE, false, "BotController::CheckUse");

    // It may be a door
    if ((trace.allsolid || trace.startsolid || trace.fraction != 1.0f) && trace.entityNum) {
        if (trace.ent->entity->IsSubclassOfDoor()) {
            Door *door = static_cast<Door *>(trace.ent->entity);
            if (door->isOpen()) {
                m_botCmd.buttons &= ~BUTTON_USE;
                return;
            }
        }

        //
        // Toggle the use button
        //
        m_botCmd.buttons ^= BUTTON_USE;
        m_Path.ForceShortLookahead();
    } else {
        m_botCmd.buttons &= ~BUTTON_USE;
    }
}

void BotController::GetUsercmd(usercmd_t *ucmd)
{
    *ucmd = m_botCmd;
}

void BotController::GetEyeInfo(usereyes_t *eyeinfo)
{
    *eyeinfo = m_botEyes;
}

void BotController::UpdateBotStates(void)
{
    if (bot_manualmove->integer) {
        memset(&m_botCmd, 0, sizeof(usercmd_t));
        return;
    }

    if (!controlledEnt->client->pers.dm_primary[0]) {
        Event *event;

        //
        // Primary weapon
        //
        event = new Event(EV_Player_PrimaryDMWeapon);
        event->AddString("auto");

        controlledEnt->ProcessEvent(event);
    }

    if (controlledEnt->GetTeam() == TEAM_NONE || controlledEnt->GetTeam() == TEAM_SPECTATOR) {
        Event *event;
        float  time;

        // Add some delay to avoid telefragging
        time = controlledEnt->entnum / 20.0;

        if (controlledEnt->EventPending(EV_Player_AutoJoinDMTeam)) {
            return;
        }

        //
        // Team
        //
        controlledEnt->PostEvent(EV_Player_AutoJoinDMTeam, time);
        return;
    }

    if (controlledEnt->IsDead() || controlledEnt->IsSpectator()) {
        // The bot should respawn
        m_botCmd.buttons ^= BUTTON_ATTACKLEFT;
        return;
    }

    m_botCmd.buttons |= BUTTON_RUN;
    m_botCmd.serverTime = level.svsTime;

    m_botEyes.ofs[0]    = 0;
    m_botEyes.ofs[1]    = 0;
    m_botEyes.ofs[2]    = controlledEnt->viewheight;
    m_botEyes.angles[0] = 0;
    m_botEyes.angles[1] = 0;

    CheckStates();

    MoveThink();
    TurnThink();
    CheckUse();
}

void BotController::SendCommand(const char *text)
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

    controlledEnt->m_lastcommand = com_token;

    if (!Event::GetEvent(com_token)) {
        return;
    }

    ev = ConsoleEvent(com_token);

    if (!(ev.GetEventFlags(ev.eventnum) & EV_CONSOLE)) {
        gi.Free(buffer);
        return;
    }

    ev.SetConsoleEdict(controlledEnt->edict);

    while (1) {
        com_token = COM_Parse(&data);

        if (!com_token || !*com_token) {
            break;
        }

        ev.AddString(com_token);
    }

    gi.Free(buffer);

    try {
        controlledEnt->ProcessEvent(ev);
    } catch (ScriptException& exc) {
        gi.DPrintf("*** Bot Command Exception *** %s\n", exc.string.c_str());
    }
}

/*
====================
SetTargetAngles

Set the bot's angle
====================
*/
void BotController::SetTargetAngles(Vector vAngles)
{
    m_vTargetAng = vAngles;
}

/*
====================
AimAt

Make the bot face to the specified direction
====================
*/
void BotController::AimAt(Vector vPos)
{
    Vector vDelta = vPos - controlledEnt->centroid;

    VectorNormalize(vDelta);
    vectoangles(vDelta, m_vTargetAng);
}

/*
====================
AimAtAimNode

Make the bot face toward the current path
====================
*/
void BotController::AimAtAimNode(void)
{
    if (!m_bPathing) {
        return;
    }

    if (!m_Path.CurrentNode()) {
        AimAt(m_vCurrentGoal);
    } else if (!m_Path.Complete(controlledEnt->origin)) {
        AimAt(controlledEnt->origin + Vector(m_Path.CurrentDelta()[0], m_Path.CurrentDelta()[1], 0));
        //int maxIndex = Q_min(3, m_Path.CurrentNode() - m_Path.LastNode());
        //AimAt((m_Path.CurrentNode() - maxIndex)->point);
    }

    m_vTargetAng[PITCH] = 0;
}

/*
====================
CheckReload

Make the bot reload if necessary
====================
*/
void BotController::CheckReload(void)
{
    Weapon *weap = controlledEnt->GetActiveWeapon(WEAPON_MAIN);

    if (weap && weap->CheckReload(FIRE_PRIMARY)) {
        SendCommand("reload");
    }
}

/*
====================
NewMove

Called when there is a new move
====================
*/
void BotController::NewMove()
{
    m_bPathing         = true;
    m_vLastCheckPos[0] = controlledEnt->origin;
    m_vLastCheckPos[1] = controlledEnt->origin;
}

/*
====================
MoveTo

Move to the specified position
====================
*/
void BotController::MoveTo(Vector vPos, float *vLeashHome, float fLeashRadius)
{
    m_vTargetPos = vPos;
    m_Path.FindPath(controlledEnt->origin, m_vTargetPos, controlledEnt, 0, vLeashHome, fLeashRadius * fLeashRadius);

    NewMove();

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
bool BotController::MoveToBestAttractivePoint(int iMinPriority)
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

        if (!node->CheckTeam(controlledEnt)) {
            continue;
        }

        distSquared = VectorLengthSquared(controlledEnt->origin - node->origin);

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
bool BotController::CanMoveTo(Vector vPos)
{
    return m_Path.DoesTheoreticPathExist(controlledEnt->origin, vPos, NULL, 0, NULL, 0);
}

/*
====================
MoveDone

Returns true if the bot has done moving
====================
*/
bool BotController::MoveDone(void)
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

    Vector delta = Vector(m_Path.CurrentPathGoal()) - controlledEnt->origin;
    if (delta.lengthXYSquared() < Square(16) && delta.z < controlledEnt->maxs.z) {
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
bool BotController::IsMoving(void)
{
    return m_bPathing;
}

/*
====================
ClearMove

Stop the bot from moving
====================
*/
void BotController::ClearMove(void)
{
    m_Path.Clear();
    m_bPathing   = false;
    m_iNumBlocks = 0;
}

/*
====================
MoveNear

Move near the specified position within the radius
====================
*/
void BotController::MoveNear(Vector vNear, float fRadius, float *vLeashHome, float fLeashRadius)
{
    m_Path.FindPathNear(
        controlledEnt->origin, vNear, controlledEnt, 0, fRadius * fRadius, vLeashHome, fLeashRadius * fLeashRadius
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
AvoidPath

Avoid the specified position within the radius and start from a direction
====================
*/
void BotController::AvoidPath(
    Vector vAvoid, float fAvoidRadius, Vector vPreferredDir, float *vLeashHome, float fLeashRadius
)
{
    Vector vDir;

    if (vPreferredDir == vec_zero) {
        vDir = controlledEnt->origin - vAvoid;
        VectorNormalizeFast(vDir);
    } else {
        vDir = vPreferredDir;
    }

    m_Path.FindPathAway(
        controlledEnt->origin, vAvoid, vDir, controlledEnt, fAvoidRadius, vLeashHome, fLeashRadius * fLeashRadius
    );
    NewMove();

    if (!m_Path.CurrentNode()) {
        // Random movements
        m_vTargetPos   = controlledEnt->origin + Vector(G_Random(256) - 128, G_Random(256) - 128, G_Random(256) - 128);
        m_vCurrentGoal = m_vTargetPos;
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
void BotController::NoticeEvent(Vector vPos, int iType, Entity *pEnt, float fDistanceSquared, float fRadiusSquared)
{
    Sentient *pSentOwner;
    float     fRangeFactor;

    fRangeFactor = 1.0 - (fDistanceSquared / fRadiusSquared);

    if (fRangeFactor < random()) {
        return;
    }

    if (pEnt->IsSubclassOfSentient()) {
        pSentOwner = static_cast<Sentient *>(pEnt);
    } else if (pEnt->IsSubclassOfVehicleTurretGun()) {
        VehicleTurretGun *pVTG = static_cast<VehicleTurretGun *>(pEnt);
        pSentOwner             = pVTG->GetSentientOwner();
    } else if (pEnt->IsSubclassOfItem()) {
        Item *pItem = static_cast<Item *>(pEnt);
        pSentOwner  = pItem->GetOwner();
    } else if (pEnt->IsSubclassOfProjectile()) {
        Projectile *pProj = static_cast<Projectile *>(pEnt);
        pSentOwner        = pProj->GetOwner();
    } else {
        pSentOwner = NULL;
    }

    if (pSentOwner) {
        if (pSentOwner == controlledEnt) {
            // Ignore self
            return;
        }

        if ((pSentOwner->flags & FL_NOTARGET) || pSentOwner->getSolidType() == SOLID_NOT) {
            return;
        }

        // Ignore teammates
        if (pSentOwner->IsSubclassOfPlayer()) {
            Player *p = static_cast<Player *>(pSentOwner);

            if (g_gametype->integer >= GT_TEAM && p->GetTeam() == controlledEnt->GetTeam()) {
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
        m_iCuriousTime   = level.inttime + 20000;
        m_vNewCuriousPos = vPos;
        break;
    }
}

/*
====================
ClearEnemy

Clear the bot's enemy
====================
*/
void BotController::ClearEnemy(void)
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

void BotController::CheckStates(void)
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
        gi.DPrintf("*** WARNING *** %s was stuck with no states !!!", controlledEnt->client->pers.netname);
        State_Reset();
    }
}

/*
====================
Default state


====================
*/
void BotController::State_DefaultBegin(void)
{
    ClearMove();
}

void BotController::State_DefaultEnd(void) {}

void BotController::State_Reset(void)
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
void BotController::InitState_Idle(botfunc_t *func)
{
    func->CheckCondition = &BotController::CheckCondition_Idle;
    func->ThinkState     = &BotController::State_Idle;
}

bool BotController::CheckCondition_Idle(void)
{
    if (m_iCuriousTime) {
        return false;
    }

    if (m_iAttackTime) {
        return false;
    }

    return true;
}

void BotController::State_Idle(void)
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
            Vector randomDir(G_CRandom(16), G_CRandom(16), G_CRandom(16));
            Vector preferredDir = Vector(controlledEnt->orientation[0]) * (rand() % 5 ? 1024 : -1024);
            float  radius       = 512 + G_Random(2048);

            AvoidPath(controlledEnt->origin + randomDir, radius, preferredDir);
        }
    }
}

/*
====================
Curious state

Forward to the last event position
====================
*/
void BotController::InitState_Curious(botfunc_t *func)
{
    func->CheckCondition = &BotController::CheckCondition_Curious;
    func->ThinkState     = &BotController::State_Curious;
}

bool BotController::CheckCondition_Curious(void)
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

void BotController::State_Curious(void)
{
    AimAtAimNode();

    if (!MoveToBestAttractivePoint(3) && (!IsMoving() || m_vLastCuriousPos != m_vNewCuriousPos)) {
        MoveTo(m_vNewCuriousPos);
        m_vLastCuriousPos = m_vNewCuriousPos;
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
void BotController::InitState_Attack(botfunc_t *func)
{
    func->CheckCondition = &BotController::CheckCondition_Attack;
    func->EndState       = &BotController::State_EndAttack;
    func->ThinkState     = &BotController::State_Attack;
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

bool BotController::IsValidEnemy(Sentient *sent) const
{
    if (sent == controlledEnt) {
        return false;
    }

    if (sent->hidden() || (sent->flags & FL_NOTARGET)) {
        // Ignore hidden / non-target enemies
        return false;
    }

    if (sent->IsDead()) {
        // Ignore dead enemies
        return false;
    }

    if (sent->getSolidType() == SOLID_NOT) {
        // Ignore non-solid, like spectators
        return false;
    }

    if (sent->IsSubclassOfPlayer()) {
        Player *player = static_cast<Player *>(sent);

        if (g_gametype->integer >= GT_TEAM && player->GetTeam() == controlledEnt->GetTeam()) {
            return false;
        }
    } else {
        if (sent->m_Team == controlledEnt->m_Team) {
            return false;
        }
    }

    return true;
}

bool BotController::CheckCondition_Attack(void)
{
    Container<Sentient *> sents       = SentientList;
    float                 maxDistance = 0;

    bot_origin = controlledEnt->origin;
    sents.Sort(sentients_compare);

    for (int i = 1; i <= sents.NumObjects(); i++) {
        Sentient *sent = sents.ObjectAt(i);

        if (!IsValidEnemy(sent)) {
            continue;
        }

        maxDistance = Q_min(world->m_fAIVisionDistance, world->farplane_distance * 0.828);

        if (controlledEnt->CanSee(sent, 80, maxDistance, false)) {
            m_pEnemy      = sent;
            m_iAttackTime = level.inttime + 1000;
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

void BotController::State_EndAttack(void)
{
    m_botCmd.buttons &= ~(BUTTON_ATTACKLEFT | BUTTON_ATTACKRIGHT);
}

void BotController::State_Attack(void)
{
    bool    bMelee              = false;
    float   fMinDistance        = 128;
    float   fMinDistanceSquared = fMinDistance * fMinDistance;
    Weapon *pWeap               = controlledEnt->GetActiveWeapon(WEAPON_MAIN);
    bool    bNoMove             = false;

    if (!m_pEnemy || !IsValidEnemy(m_pEnemy)) {
        // Ignore dead enemies
        m_iAttackTime = 0;
        return;
    }
    float fDistanceSquared = (m_pEnemy->origin - controlledEnt->origin).lengthSquared();

    if (controlledEnt->CanSee(
            m_pEnemy, 20, Q_min(world->m_fAIVisionDistance, world->farplane_distance * 0.828), false
        )) {
        if (!pWeap) {
            return;
        }

        float fPrimaryBulletRange          = pWeap->GetBulletRange(FIRE_PRIMARY) / 1.25f;
        float fPrimaryBulletRangeSquared   = fPrimaryBulletRange * fPrimaryBulletRange;
        float fSecondaryBulletRange        = pWeap->GetBulletRange(FIRE_SECONDARY);
        float fSecondaryBulletRangeSquared = fSecondaryBulletRange * fSecondaryBulletRange;

        //
        // check the fire movement speed if the weapon has a max fire movement
        //
        if (pWeap->GetMaxFireMovement() < 1 && pWeap->HasAmmoInClip(FIRE_PRIMARY)) {
            float length;

            length = controlledEnt->velocity.length();
            if ((length / sv_runspeed->value) > (pWeap->GetMaxFireMovement() * pWeap->GetMovementSpeed())) {
                bNoMove = true;
                ClearMove();
            }
        }

        fMinDistance = fPrimaryBulletRange;

        if (fMinDistance > 256) {
            fMinDistance = 256;
        }

        fMinDistanceSquared = fMinDistance * fMinDistance;

        if (controlledEnt->client->ps.stats[STAT_AMMO] > 0 || controlledEnt->client->ps.stats[STAT_CLIPAMMO] > 0) {
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

        m_iAttackTime   = level.inttime + 1000;
        m_vOldEnemyPos  = m_vLastEnemyPos;
        m_vLastEnemyPos = m_pEnemy->centroid;
    } else {
        m_botCmd.buttons &= ~(BUTTON_ATTACKLEFT | BUTTON_ATTACKRIGHT);
        fMinDistanceSquared = 0;
    }

    AimAt(m_pEnemy->centroid + Vector(G_CRandom(8), G_CRandom(8), G_CRandom(8)));

    if (bNoMove) {
        return;
    }

    if ((!MoveToBestAttractivePoint(5) && !IsMoving()) || (m_vOldEnemyPos != m_vLastEnemyPos && !MoveDone())) {
        if (!bMelee) {
            if ((controlledEnt->origin - m_vLastEnemyPos).lengthSquared() < fMinDistanceSquared) {
                Vector vDir = controlledEnt->origin - m_vLastEnemyPos;
                VectorNormalizeFast(vDir);

                AvoidPath(m_vLastEnemyPos, fMinDistance, Vector(controlledEnt->orientation[1]) * 512);
            } else {
                MoveNear(m_vLastEnemyPos, fMinDistance);
            }
        } else {
            MoveTo(m_vLastEnemyPos);
        }
    }

    if (IsMoving()) {
        m_iAttackTime = level.inttime + 1000;
    }
}

/*
====================
Grenade state

Avoid any grenades
====================
*/
void BotController::InitState_Grenade(botfunc_t *func)
{
    func->CheckCondition = &BotController::CheckCondition_Grenade;
    func->ThinkState     = &BotController::State_Grenade;
}

bool BotController::CheckCondition_Grenade(void)
{
    // FIXME: TODO
    return false;
}

void BotController::State_Grenade(void)
{
    // FIXME: TODO
}

/*
====================
Weapon state

Change weapon when necessary
====================
*/
void BotController::InitState_Weapon(botfunc_t *func)
{
    func->CheckCondition = &BotController::CheckCondition_Weapon;
    func->BeginState     = &BotController::State_BeginWeapon;
}

bool BotController::CheckCondition_Weapon(void)
{
    return controlledEnt->GetActiveWeapon(WEAPON_MAIN)
        != controlledEnt->BestWeapon(NULL, false, WEAPON_CLASS_THROWABLE);
}

void BotController::State_BeginWeapon(void)
{
    Weapon *weap = controlledEnt->BestWeapon(NULL, false, WEAPON_CLASS_THROWABLE);

    if (weap == NULL) {
        SendCommand("safeholster 1");
        return;
    }

    SendCommand(va("use \"%s\"", weap->model.c_str()));
}

void BotController::Spawned(void)
{
    ClearEnemy();
    m_iCuriousTime   = 0;
    m_botCmd.buttons = 0;
}

void BotController::Think()
{
    usercmd_t  ucmd;
    usereyes_t eyeinfo;

    UpdateBotStates();
    GetUsercmd(&ucmd);
    GetEyeInfo(&eyeinfo);

    G_ClientThink(controlledEnt->edict, &ucmd, &eyeinfo);

    if (m_vCurrentOrigin != controlledEnt->origin) {
        m_pPrimaryAttract = NULL;
        m_vCurrentOrigin  = controlledEnt->origin;

        if (m_Path.CurrentNode()) {
            // recalculate paths because of a new origin
            m_Path.ReFindPath(controlledEnt->origin, controlledEnt);
        }
    }
}

void BotController::Killed(Event *ev)
{
    Entity *attacker;

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

    attacker = ev->GetEntity(1);

    if (attacker && rand() % 5 == 0) {
        // 1/5 chance to go back to the attacker position
        m_vLastDeathPos = attacker->origin;
    } else {
        m_vLastDeathPos = vec_zero;
    }

    // Choose a new random primary weapon
    Event event(EV_Player_PrimaryDMWeapon);
    event.AddString("auto");

    controlledEnt->ProcessEvent(event);
}

void BotController::GotKill(Event *ev)
{
    ClearEnemy();
    m_iCuriousTime = 0;

    if (level.inttime >= m_iNextTauntTime && (rand() % 5) == 0) {
        //
        // Randomly play a taunt
        //
        Event event("dmmessage");

        event.AddInteger(0);

        if (g_protocol >= protocol_e::PROTOCOL_MOHTA_MIN) {
            event.AddString("*5" + str(1 + (rand() % 8)));
        } else {
            event.AddString("*4" + str(1 + (rand() % 9)));
        }

        controlledEnt->ProcessEvent(event);

        m_iNextTauntTime = level.inttime + 5000;
    }
}

void BotController::EventStuffText(Event *ev)
{
    SendCommand(ev->GetString(1));
}

void BotController::setControlledEntity(Player *player)
{
    controlledEnt = player;
}

Player *BotController::getControlledEntity() const
{
    return controlledEnt;
}

BotController *BotControllerManager::createController(Player *player)
{
    BotController *controller = new BotController();
    controller->setControlledEntity(player);

    controllers.AddObject(controller);

    return controller;
}

void BotControllerManager::removeController(BotController *controller)
{
    controllers.RemoveObject(controller);
    delete controller;
}

BotController *BotControllerManager::findController(Entity *ent)
{
    int i;

    for (i = 1; i <= controllers.NumObjects(); i++) {
        BotController *controller = controllers.ObjectAt(i);
        if (controller->getControlledEntity() == ent) {
            return controller;
        }
    }

    return nullptr;
}

const Container<BotController *>& BotControllerManager::getControllers() const
{
    return controllers;
}

BotControllerManager::~BotControllerManager()
{
    Cleanup();
}

void BotControllerManager::Init()
{
    BotController::Init();
}

void BotControllerManager::Cleanup()
{
    int i;

    BotController::Init();

    for (i = 1; i <= controllers.NumObjects(); i++) {
        BotController *controller = controllers.ObjectAt(i);
        delete controller;
    }

    controllers.FreeObjectList();
}

void BotControllerManager::ThinkControllers()
{
    int i;

    for (i = 1; i <= controllers.NumObjects(); i++) {
        BotController *controller = controllers.ObjectAt(i);
        controller->Think();
    }
}
