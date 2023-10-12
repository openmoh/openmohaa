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

// actor_machinegunner.cpp

#include "actor.h"
#include "weapturret.h"

extern Vector MINS;
extern Vector MAXS;

void Actor::InitMachineGunner(GlobalFuncs_t *func)
{
    func->BeginState        = &Actor::Begin_MachineGunner;
    func->EndState          = &Actor::End_MachineGunner;
    func->ThinkState        = &Actor::Think_MachineGunner;
    func->IsState           = &Actor::IsMachineGunnerState;
    func->FinishedAnimation = &Actor::FinishedAnimation_MachineGunner;
}

void Actor::Begin_MachineGunner(void)
{
    m_csMood     = STRING_ALERT;
    m_csIdleMood = STRING_NERVOUS;

    if (m_pTurret) {
        Sentient *pOwner;
        m_pTurret->m_bHadOwner = true;
        pOwner                 = m_pTurret->GetOwner();
        if (pOwner) {
            Com_Printf(
                "^~^~^ Actor (entnum %d, radnum %d, targetname '%s') cannot use turret (entnum %d, radnum %d, "
                "targetname '%s')"
                " since it is already being used by Actor (entnum %d, radnum %d, targetname '%s')\n",
                entnum,
                radnum,
                targetname.c_str(),
                m_pTurret->entnum,
                m_pTurret->radnum,
                m_pTurret->TargetName().c_str(),
                pOwner->entnum,
                pOwner->radnum,
                pOwner->TargetName().c_str()
            );
        } else {
            Holster();

            m_pTurret->TurretBeginUsed(this);
            TransitionState(1200, 0);
        }
    } else {
        Com_Printf(
            "^~^~^ Actor (entnum %d, radnum %d, targetname '%s') has no turret specified\n",
            entnum,
            radnum,
            targetname.c_str()
        );
    }
}

void Actor::End_MachineGunner(void)
{
    if (!GetWeapon(WEAPON_MAIN)) {
        giveItem("models/weapons/mp40.tik");
    }

    Unholster();
    if (m_pTurret) {
        if (m_pTurret->GetOwner() == this) {
            m_pTurret->TurretEndUsed();
        }
    }
}

void Actor::ThinkHoldGun_TurretGun(void)
{
    // FIXME: unimplemented
}

void Actor::Think_MachineGunner_TurretGun(void)
{
    // FIXME: unimplemented
}

void Actor::Think_MachineGunner(void)
{
    if (RequireThink()) {
        if (m_pTurret && m_pTurret->GetOwner() == this && !m_bNoPlayerCollision) {
            if (!m_bEnableEnemy) {
                ThinkHoldGun();
                return;
            }

            if (level.inttime < m_iEnemyCheckTime + 200) {
                ThinkHoldGun();
                return;
            }

            m_iEnemyCheckTime = level.inttime;

            if (m_pTurret->AI_CanTarget(G_GetEntity(0)->centroid)) {
                ThinkHoldGun();
                return;
            }
            if (!m_pGrenade || m_fGrenadeAwareness < rand() * 0.000000046566129) {
                if (!G_SightTrace(
                        EyePosition(),
                        vec_zero,
                        vec_zero,
                        G_GetEntity(0)->centroid,
                        this,
                        G_GetEntity(0),
                        33819417,
                        qfalse,
                        "Actor::Think_MachineGunner"
                    )) {
                    ThinkHoldGun();
                    return;
                }
                if (m_ThinkStates[THINKLEVEL_NORMAL] != THINKSTATE_IDLE) {
                    BecomeTurretGuy();
                    return;
                }

                if (m_Enemy && !m_Enemy->IsSubclassOfActor() && !EnemyIsDisguised()
                    && m_PotentialEnemies.GetCurrentVisibility() >= 1) {
                    BecomeTurretGuy();
                    return;
                }

                if (!m_Enemy || !m_Enemy->IsSubclassOfActor() || EnemyIsDisguised()
                    || m_PotentialEnemies.GetCurrentVisibility() > 1) {
                    ThinkHoldGun();
                    return;
                }

                SetCuriousAnimHint(6);
            }
        }
        BecomeTurretGuy();
    }
}

void Actor::ThinkHoldGun(void)
{
    Vector  end;
    trace_t trace;
    float   temp;
    float   temp2;
    float   machine_gunner_hands_up_stand;
    float   heightDiff;
    float   right;
    //vec3_t newOrigin;
    Vector offset;
    Vector start;
    //vec3_t new_angles;
    Vector vForward;

    UpdateEyeOrigin();
    m_pszDebugState = "";
    if (m_State == 1200) {
        if (m_pTurret->IsFiring()) {
            if (m_pTurret->aim_target == G_GetEntity(0)) {
                m_bNoSurprise = true;
            }
            m_csNextAnimString = STRING_ANIM_MG42_SHOOT_SCR;
        } else {
            m_csNextAnimString = STRING_ANIM_MG42_IDLE_SCR;
        }
    } else {
        m_csNextAnimString = STRING_ANIM_MG42_RELOAD_SCR;
        m_bAnimScriptSet   = false;
    }
    m_eNextAnimMode   = 8;
    m_bNextForceStart = false;

    CheckUnregister();

    StopTurning();

    setAngles(Vector(0, m_pTurret->angles[1], 0));

    vForward = Vector(m_pTurret->orientation[0]) * -39 + m_pTurret->origin;

    if (m_State == 1201) {
        heightDiff = 71.6f;
    } else {
        heightDiff = 71.8f;
    }

    machine_gunner_hands_up_stand = origin[2] - (m_pTurret->origin[2] - heightDiff);

    if (machine_gunner_hands_up_stand < 0) {
        if (m_State == 1201) {
            temp = (machine_gunner_hands_up_stand - m_pTurret->origin[2]) / 39;
            if (temp >= -1.0 && temp <= 1.0) {
                right = atan2(temp / sqrt(temp * -temp + 1), 1) * 180 / M_PI;
                m_pTurret->setAngles(Vector(right, m_pTurret->angles[1], m_pTurret->angles[2]));
            }
        } else {
            m_fCrouchWeight = machine_gunner_hands_up_stand / 17.1;
            if (m_fCrouchWeight < -1.0) {
                m_fCrouchWeight = -1.0;
            }
        }
        temp2    = m_fCrouchWeight * 2.6;
        offset.x = (-(-9.3 * m_fCrouchWeight + 23.4)) * orientation[0][0] + vForward.x;
        offset.y = (-(-9.3 * m_fCrouchWeight + 23.4)) * orientation[0][0] + vForward.y;
    } else {
        m_fCrouchWeight = machine_gunner_hands_up_stand / (heightDiff - 38.7);
        if (m_fCrouchWeight > 1.0) {
            m_fCrouchWeight = 1.0;
        }

        temp2    = m_fCrouchWeight * -1.6;
        offset.x = (-(-3 * m_fCrouchWeight + 23.4)) * orientation[0][0] + vForward.x;
        offset.y = (-(-3 * m_fCrouchWeight + 23.4)) * orientation[0][0] + vForward.y;
    }

    start.x = (temp2 + 10.3) * orientation[1][0] + offset.x;
    start.y = (temp2 + 10.3) * orientation[1][1] + offset.y;

    if (m_fCrouchWeight >= 0.5) {
        m_csCurrentPosition = STRING_CROUCH;
    } else {
        m_csCurrentPosition = STRING_STAND;
    }

    UpdateAimMotion();
    UpdateAnim();
    start.z = origin.z + 18.0;

    end   = start;
    end.z = origin.z - 94.0;

    trace = G_Trace(start, MINS, MAXS, end, this, 1107437825, qfalse, "Actor::ThinkHoldGun");

    if (trace.fraction != 1.0 && !trace.startsolid && !trace.allsolid && trace.ent) {
        SafeSetOrigin(trace.endpos);
    }

    velocity = vec_zero;

    UpdateBoneControllers();
    UpdateFootsteps();
}

void Actor::FinishedAnimation_MachineGunner(void)
{
    if (!m_bAnimScriptSet && m_State == 1201) {
        TransitionState(1200, 0);
        Unregister(STRING_ANIMDONE);
    }
}

bool Actor::MachineGunner_CanSee(Entity *ent, float fov, float vision_distance)
{
    float delta[2];

    delta[0] = ent->centroid[0] - centroid[0];
    delta[1] = ent->centroid[1] - centroid[1];
    if ((vision_distance <= 0.0f)
        || (delta[0] * delta[0] + delta[1] * delta[1]) <= (vision_distance * vision_distance)) {
        if (gi.AreasConnected(edict->r.areanum, ent->edict->r.areanum)
            && ((fov <= 0.0f || fov >= 360.0f) || (FovCheck(delta, cos(fov * (0.5 * M_PI / 180.0)))))) {
            return G_SightTrace(
                centroid,
                vec_zero,
                vec_zero,
                ent->centroid,
                m_pTurret,
                ent,
                MASK_CANSEE,
                0,
                "Actor::MachineGunner_CanSee"
            );
        }
    }

    return false;
}
