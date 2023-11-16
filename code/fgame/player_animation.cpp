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

// player_animation.cpp: Animation utility functions
//

#include "player.h"
#include "g_phys.h"
#include "portableturret.h"

extern Event EV_Player_AnimLoop_Torso;
extern Event EV_Player_AnimLoop_Legs;

void Player::EndAnim_Legs(Event *ev)
{
    animdone_Legs = true;

    if (IsRepeatType(m_iPartSlot[legs])) {
        SetAnimDoneEvent(EV_Player_AnimLoop_Legs, m_iPartSlot[legs]);
    }

    EvaluateState();
}

void Player::EndAnim_Torso(Event *ev)
{
    animdone_Torso = true;

    if (IsRepeatType(m_iPartSlot[torso])) {
        SetAnimDoneEvent(EV_Player_AnimLoop_Torso, m_iPartSlot[torso]);
    }

    EvaluateState();
}

void Player::EndAnim_Pain(Event *ev)
{
    animdone_Pain = true;
}

void Player::SetPartAnim(const char *anim, bodypart_t slot)
{
    int animnum;

    if (getMoveType() == MOVETYPE_NOCLIP && slot) {
        StopPartAnimating(torso);
        return;
    }

    animnum = gi.Anim_NumForName(edict->tiki, anim);
    if (animnum == CurrentAnim() && partAnim[slot] == anim) {
        return;
    }

    if (animnum == -1) {
        Event *ev;

        if (slot) {
            ev = new Event(EV_Player_AnimLoop_Torso);
        } else {
            ev = new Event(EV_Player_AnimLoop_Legs);
        }

        PostEvent(ev, level.frametime);
        gi.DPrintf("^~^~^ Warning: Can't find player animation '%s'.\n", anim);

        return;
    }

    if (m_fPartBlends[slot] < 0.5f) {
        SetAnimDoneEvent(NULL, m_iPartSlot[slot]);

        float m_fCrossTime = gi.Anim_CrossTime(edict->tiki, animnum);

        partBlendMult[slot] = m_fCrossTime;
        if (m_fCrossTime <= 0.0f) {
            partOldAnim[slot]   = "";
            m_fPartBlends[slot] = 0.0f;
        } else {
            m_iPartSlot[slot] ^= 1;
            partBlendMult[slot] = 1.0f / m_fCrossTime;
            partOldAnim[slot]   = partAnim[slot];
            m_fPartBlends[slot] = 1.0f;
        }
    }

    if (slot) {
        animdone_Torso = false;
    } else {
        animdone_Legs = false;
    }

    edict->s.frameInfo[m_iPartSlot[slot]].index = gi.Anim_NumForName(edict->tiki, "idle");

    partAnim[slot] = anim;

    if (slot) {
        NewAnim(animnum, EV_Player_AnimLoop_Torso, m_iPartSlot[slot]);
    } else {
        NewAnim(animnum, EV_Player_AnimLoop_Legs, m_iPartSlot[legs]);
    }

    RestartAnimSlot(m_iPartSlot[slot]);
}

static float g_fPartBlendTime[2] = {0.2f, 0.2f};

void Player::StopPartAnimating(bodypart_t part)
{
    if (partAnim[part] == "") {
        return;
    }

    if (m_fPartBlends[part] < 0.5f) {
        SetAnimDoneEvent(NULL, m_iPartSlot[part]);

        m_iPartSlot[part] ^= 1;
        partOldAnim[part]   = partAnim[part];
        m_fPartBlends[part] = 1.0f;
    }

    partAnim[part]      = "";
    partBlendMult[part] = 1.0f / g_fPartBlendTime[part];

    StopAnimating(m_iPartSlot[part]);

    if (part) {
        animdone_Torso = false;
    } else {
        animdone_Legs = false;
    }
}

void Player::PausePartAnim(bodypart_t part)
{
    Pause(m_iPartSlot[part], 1);
    Pause(m_iPartSlot[part] ^ 1, 1);
}

int Player::CurrentPartAnim(bodypart_t part) const
{
    if (!*partAnim[part]) {
        return -1;
    }

    return CurrentAnim(m_iPartSlot[part]);
}

void Player::AdjustAnimBlends(void)
{
    int      iPartSlot;
    int      iOldPartSlot;
    float    fWeightTotal;
    qboolean playTurretAnim = false;

    if (movetype == MOVETYPE_PORTABLE_TURRET) {
        PortableTurret *turret = static_cast<PortableTurret *>(m_pTurret.Pointer());

        if (turret->IsReloading()) {
            if (partAnim[legs] != "mg42tripod_reload") {
                SetPartAnim("mg42tripod_reload", legs);
                StopAnimating(m_iPartSlot[torso]);
                StopAnimating(m_iPartSlot[torso] ^ 1);
                partAnim[torso] = "";
            }
        } else if (turret->IsSettingUp()) {
            if (partAnim[legs] != "mg42tripod_setup") {
                SetPartAnim("mg42tripod_setup", legs);
                StopAnimating(m_iPartSlot[torso]);
                StopAnimating(m_iPartSlot[torso] ^ 1);
                partAnim[torso] = "";
            }
        } else if (turret->IsPackingUp()) {
            if (partAnim[legs] != "mg42tripod_packup") {
                SetPartAnim("mg42tripod_packup", legs);
                StopAnimating(m_iPartSlot[torso]);
                StopAnimating(m_iPartSlot[torso] ^ 1);
                partAnim[torso] = "";
            }
        } else {
            playTurretAnim = true;
        }
    }

    if (playTurretAnim) {
        PortableTurret *turret = static_cast<PortableTurret *>(m_pTurret.Pointer());

        if (partAnim[legs] != "mg42tripod_aim") {
            StopPartAnimating(legs);
        }

        if (partAnim[torso] != "mg42tripod_aim") {
            StopPartAnimating(torso);
        }

        partAnim[legs]  = "mg42tripod_aim";
        partAnim[torso] = "mg42tripod_aim";

        float pitch, yaw;
        float legsPitchAngle, torsoPitchAngle;
        float pitchAlpha, yawAlpha;
        float legsYawAngle, torsoYawAngle;
        float weight;
        int   animNum;
        str   vertLegsStr, vertTorsoStr;
        str   horzLegsStr, horzTorsoStr;
        str   anim;

        pitch = AngleSubtract(turret->GetGroundPitch(), turret->angles[0]);
        yaw   = AngleSubtract(turret->GetStartYaw(), turret->angles[1]);

        if (pitch >= 0) {
            legsPitchAngle  = 0.0;
            torsoPitchAngle = 15.0;
            vertLegsStr     = "_straight";
            vertTorsoStr    = "_up15";
        } else {
            legsPitchAngle  = -15.0;
            torsoPitchAngle = 0.0;
            vertLegsStr     = "_down15";
            vertTorsoStr    = "_straight";
        }

        pitchAlpha = (pitch - legsPitchAngle) / (torsoPitchAngle - legsPitchAngle);
        pitchAlpha = Q_clamp_float(pitchAlpha, 0, 1);

        if (yaw < -20) {
            legsYawAngle  = -40.0;
            torsoYawAngle = -20.0;
            horzLegsStr   = "_left40";
            horzTorsoStr  = "_left20";
        } else if (yaw < 0) {
            legsYawAngle  = -20.0;
            torsoYawAngle = 0.0;
            horzLegsStr   = "_left20";
            horzTorsoStr  = "_straight";
        } else if (yaw <= 20) {
            legsYawAngle  = 0.0;
            torsoYawAngle = 20.0;
            horzLegsStr   = "_straight";
            horzTorsoStr  = "_right20";
        } else {
            legsYawAngle  = 20.0;
            torsoYawAngle = 40.0;
            horzLegsStr   = "_right20";
            horzTorsoStr  = "_right40";
        }

        yawAlpha = (yaw - legsYawAngle) / (torsoYawAngle - legsYawAngle);
        yawAlpha = Q_clamp_float(yawAlpha, 0, 1);

        anim    = "mg42tripod_aim" + horzLegsStr + vertLegsStr;
        weight  = (1 - pitchAlpha) * (1 - yawAlpha);
        animNum = gi.Anim_NumForName(edict->tiki, anim.c_str());
        if (animNum == -1) {
            gi.DPrintf("^~^~^ Warning: Can't find player animation '%s'.\n", anim.c_str());
            anim    = "mg42tripod_aim_straight_straight";
            animNum = gi.Anim_NumForName(edict->tiki, anim.c_str());
        }

        if (animNum == CurrentAnim(0)) {
            SetWeight(0, weight);
        } else {
            NewAnim(animNum, 0, weight);
        }

        anim    = "mg42tripod_aim" + horzLegsStr + vertTorsoStr;
        weight  = (1.0 - yawAlpha) * pitchAlpha;
        animNum = gi.Anim_NumForName(edict->tiki, anim.c_str());
        if (animNum == -1) {
            gi.DPrintf("^~^~^ Warning: Can't find player animation '%s'.\n", anim.c_str());
            anim    = "mg42tripod_aim_straight_straight";
            animNum = gi.Anim_NumForName(edict->tiki, anim.c_str());
        }

        if (animNum == CurrentAnim(1)) {
            SetWeight(1, weight);
        } else {
            NewAnim(animNum, 1, weight);
        }

        anim    = "mg42tripod_aim" + horzTorsoStr + vertTorsoStr;
        weight  = pitchAlpha * yawAlpha;
        animNum = gi.Anim_NumForName(edict->tiki, anim.c_str());
        if (animNum == -1) {
            gi.DPrintf("^~^~^ Warning: Can't find player animation '%s'.\n", anim.c_str());
            anim    = "mg42tripod_aim_straight_straight";
            animNum = gi.Anim_NumForName(edict->tiki, anim.c_str());
        }

        if (animNum == CurrentAnim(2)) {
            SetWeight(2, weight);
        } else {
            NewAnim(animNum, 2, weight);
        }

        anim    = "mg42tripod_aim" + horzTorsoStr + vertTorsoStr;
        weight  = (1.0 - pitchAlpha) * yawAlpha;
        animNum = gi.Anim_NumForName(edict->tiki, anim.c_str());
        if (animNum == -1) {
            gi.DPrintf("^~^~^ Warning: Can't find player animation '%s'.\n", anim.c_str());
            anim    = "mg42tripod_aim_straight_straight";
            animNum = gi.Anim_NumForName(edict->tiki, anim.c_str());
        }

        if (animNum == CurrentAnim(3)) {
            SetWeight(3, weight);
        } else {
            NewAnim(animNum, 3, weight);
        }
    }

    if (deadflag == DEAD_DEAD) {
        if (m_fPainBlend) {
            StopAnimating(ANIMSLOT_PAIN);
            SetWeight(ANIMSLOT_PAIN, 0);
            m_fPainBlend  = 0;
            animdone_Pain = false;
        }
        return;
    }

    iPartSlot    = m_iPartSlot[legs];
    iOldPartSlot = m_iPartSlot[legs] ^ 1;

    if (m_fPartBlends[legs] <= 0.0f) {
        if (partOldAnim[legs] != "") {
            StopAnimating(iOldPartSlot);

            if (partAnim[legs] == "") {
                SetWeight(iPartSlot, 0.0);
            } else {
                SetWeight(iPartSlot, 1.0);
            }
        } else {
            if (partAnim[legs] == "") {
                SetWeight(iPartSlot, 0.0);
            } else {
                SetWeight(iPartSlot, 1.0);
            }
        }
    } else {
        m_fPartBlends[legs] -= level.frametime * partBlendMult[legs];
        if (m_fPartBlends[legs] >= 0.01f) {
            if (partOldAnim[legs] != "") {
                SetWeight(iOldPartSlot, m_fPartBlends[legs]);
            }
            if (partAnim[legs] != "") {
                SetWeight(iPartSlot, 1.0f - m_fPartBlends[legs]);
            }
        } else {
            m_fPartBlends[legs] = 0.0f;
            StopAnimating(iOldPartSlot);
            partOldAnim[legs] = "";

            if (partAnim[legs] == "") {
                SetWeight(iPartSlot, 0.0);
            } else {
                SetWeight(iPartSlot, 1.0);
            }
        }
    }

    iPartSlot    = m_iPartSlot[torso];
    iOldPartSlot = m_iPartSlot[torso] ^ 1;

    if (m_fPartBlends[torso] <= 0.0f) {
        if (partOldAnim[torso] != "") {
            StopAnimating(iOldPartSlot);
            partOldAnim[torso] = "";

            if (partAnim[torso] == "") {
                SetWeight(iPartSlot, 0.0);
            } else {
                SetWeight(iPartSlot, 1.0);
            }
        }
    } else {
        m_fPartBlends[torso] = m_fPartBlends[torso] - level.frametime * partBlendMult[torso];
        if (m_fPartBlends[torso] >= 0.01f) {
            fWeightTotal = 0.0f;

            if (partOldAnim[torso] != "") {
                SetWeight(iOldPartSlot, m_fPartBlends[torso]);
                fWeightTotal += m_fPartBlends[torso];
            }
            if (partAnim[torso] != "") {
                SetWeight(iPartSlot, m_fPartBlends[torso]);
                fWeightTotal += 1.0f - m_fPartBlends[torso];
            }

            edict->s.actionWeight = fWeightTotal;
        } else {
            m_fPartBlends[torso] = 0.0f;
            StopAnimating(iOldPartSlot);
            partOldAnim[torso] = "";

            if (partAnim[torso] == "") {
                SetWeight(iPartSlot, 0.0);
                edict->s.actionWeight = 0.0;
            } else {
                SetWeight(iPartSlot, 1.0);
                edict->s.actionWeight = 1.0;
            }
        }
    }

    if (m_fPainBlend) {
        if (m_sPainAnim == "") {
            StopAnimating(ANIMSLOT_PAIN);
            edict->s.frameInfo[ANIMSLOT_PAIN].weight = 0;
            m_fPainBlend                             = 0;
            animdone_Pain                            = false;
        } else if (animdone_Pain) {
            m_fPainBlend -= level.frametime * (10.0 / 3.0);
            if (m_fPainBlend < 0.01f) {
                StopAnimating(ANIMSLOT_PAIN);
                edict->s.frameInfo[ANIMSLOT_PAIN].weight = 0;
                m_fPainBlend                             = 0;
                animdone_Pain                            = false;
            }
        }

        if (m_fPainBlend) {
            int   i;
            float w;

            edict->s.frameInfo[ANIMSLOT_PAIN].weight = m_fPainBlend;
            w                                        = 1.0 - m_fPainBlend * 0.5;

            for (i = 0; i < ANIMSLOT_PAIN; i++) {
                if (edict->s.frameInfo[i].weight) {
                    edict->s.frameInfo[i].weight *= w;
                }
            }

            edict->s.actionWeight *= w;
        }
    }
}

void Player::PlayerAnimDelta(float *vDelta)
{
    float fTimeDelta;
    float fBackTime;
    float vNewDelta[3];
    int   animnum;

    VectorClear(vDelta);

    if (m_fLastDeltaTime >= level.time) {
        return;
    }

    fTimeDelta = level.time - m_fLastDeltaTime;

    animnum = -1;

    if (partAnim[legs] != "") {
        animnum = CurrentAnim(m_iPartSlot[legs]);
    }

    if (animnum != -1) {
        fBackTime = GetTime(m_iPartSlot[legs]) - fTimeDelta;
        if (fBackTime < 0.0f) {
            fBackTime = 0.0f;
        }

        float fTime = GetTime(m_iPartSlot[legs]);

        // get the anim delta
        gi.Anim_DeltaOverTime(edict->tiki, animnum, fBackTime, fTime, vNewDelta);

        VectorMA(vDelta, edict->s.frameInfo[m_iPartSlot[legs]].weight, vNewDelta, vDelta);
    }

    animnum = -1;

    if (partAnim[torso] != "") {
        animnum = CurrentAnim(m_iPartSlot[torso]);
    }

    if (animnum != -1) {
        fBackTime = GetTime(m_iPartSlot[torso]) - fTimeDelta;
        if (fBackTime < 0.0f) {
            fBackTime = 0.0f;
        }

        float fTime = GetTime(m_iPartSlot[torso]);

        gi.Anim_DeltaOverTime(edict->tiki, animnum, fBackTime, fTime, vNewDelta);

        VectorMA(vDelta, edict->s.frameInfo[m_iPartSlot[torso]].weight, vNewDelta, vDelta);
    }
}

void Player::EventTestAnim(Event *ev)
{
    // FIXME: unimplemented
}
