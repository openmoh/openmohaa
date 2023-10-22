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
#include "bg_local.h"

void Actor::InitMachineGunner(GlobalFuncs_t *func)
{
    func->BeginState        = &Actor::Begin_MachineGunner;
    func->EndState          = &Actor::End_MachineGunner;
    func->ThinkState        = &Actor::Think_MachineGunner;
    func->IsState           = &Actor::IsMachineGunnerState;
    func->FinishedAnimation = &Actor::FinishedAnimation_MachineGunner;
}

bool Actor::IsMachineGunnerState(int state)
{
    return true;
}

void Actor::Begin_MachineGunner(void)
{
    m_csMood     = STRING_ALERT;
    m_csIdleMood = STRING_NERVOUS;

    if (!m_pTurret) {
        Com_Printf(
            "^~^~^ Actor (entnum %d, radnum %d, targetname '%s') has no turret specified\n",
            entnum,
            radnum,
            targetname.c_str()
        );

        return;
    }

    Sentient *pOwner;
    m_pTurret->m_bHadOwner = true;
    pOwner                 = m_pTurret->GetOwner();
    if (m_pTurret->GetOwner()) {
        Com_Printf(
            "^~^~^ Actor (entnum %d, radnum %d, targetname '%s') cannot use turret (entnum %d, radnum %d, "
            "targetname '%s')"
            " since it is already being used by Actor (entnum %d, radnum %d, targetname '%s')\n",
            entnum,
            radnum,
            targetname.c_str(),
            m_pTurret->entnum,
            m_pTurret->radnum,
            m_pTurret->targetname.c_str(),
            m_pTurret->GetOwner()->entnum,
            m_pTurret->GetOwner()->radnum,
            m_pTurret->GetOwner()->targetname.c_str()
        );
    } else {
        Holster();

        m_pTurret->TurretBeginUsed(this);
        TransitionState(ACTOR_STATE_MACHINE_GUNNER_READY);
    }
}

void Actor::End_MachineGunner(void)
{
    if (!GetWeapon(WEAPON_MAIN)) {
        giveItem("models/weapons/mp40.tik");
    }

    Unholster();
    if (m_pTurret && m_pTurret->GetOwner() == this) {
        m_pTurret->TurretEndUsed();
    }
}

void Actor::ThinkHoldGun_TurretGun(void)
{
    Vector  vForward;
    vec3_t  new_angles;
    vec3_t  offset;
    vec3_t  newOrigin;
    float   right;
    float   heightDiff;
    float   machine_gunner_hands_up_stand;
    float   temp;
    trace_t trace;
    Vector  end;

    if (m_State != ACTOR_STATE_MACHINE_GUNNER_READY) {
        DesiredAnimation(ANIM_MODE_FROZEN, STRING_ANIM_MG42_RELOAD_SCR);
        m_bAnimScriptSet = false;
    } else if (m_pTurret->IsFiring()) {
        if (m_pTurret->aim_target == G_GetEntity(0)) {
            m_bNoSurprise = true;
        }

        DesiredAnimation(ANIM_MODE_FROZEN, STRING_ANIM_MG42_SHOOT_SCR);
    } else {
        DesiredAnimation(ANIM_MODE_FROZEN, STRING_ANIM_MG42_IDLE_SCR);
    }

    CheckUnregister();
    StopTurning();

    VectorSet(new_angles, 0, m_pTurret->angles[1], 0);
    setAngles(new_angles);

    VectorScale(m_pTurret->orientation[0], -39, offset);
    VectorAdd(m_pTurret->origin, offset, newOrigin);

    if (m_State == ACTOR_STATE_MACHINE_GUNNER_RELOADING) {
        heightDiff = 71.6f;
    } else {
        heightDiff = 71.8f;
    }

    newOrigin[2] -= heightDiff;
    machine_gunner_hands_up_stand = origin[2] - newOrigin[2];

    if (machine_gunner_hands_up_stand < 0) {
        if (m_State == ACTOR_STATE_MACHINE_GUNNER_RELOADING) {
            m_fCrouchWeight = 0;

            temp = (origin[2] + 71.6f - m_pTurret->origin[2]) / 39.f;
            if (temp >= -1.0 && temp <= 1.0) {
                m_pTurret->angles[0] = RAD2DEG(atan(temp / sqrt(temp * -temp + 1)));
                m_pTurret->setAngles(m_pTurret->angles);
            }
        } else {
            m_fCrouchWeight = machine_gunner_hands_up_stand / 17.1f;
            if (m_fCrouchWeight < -1.0) {
                m_fCrouchWeight = -1.0;
            }
        }

        VectorScale2D(orientation[0], m_fCrouchWeight * -9.3f + 23.4f, offset);
        VectorAdd2D(newOrigin, offset, newOrigin);
        VectorScale2D(orientation[1], m_fCrouchWeight * 2.6f + 10.3f, offset);
        VectorAdd2D(newOrigin, offset, newOrigin);
    } else {
        m_fCrouchWeight = machine_gunner_hands_up_stand / (heightDiff - 38.7f);
        if (m_fCrouchWeight > 1.0) {
            m_fCrouchWeight = 1.0;
        }

        VectorScale2D(orientation[0], m_fCrouchWeight * -3.f + 23.4f, offset);
        VectorAdd2D(newOrigin, offset, newOrigin);
        VectorScale2D(orientation[1], m_fCrouchWeight * -1.6f + 10.3f, offset);
        VectorAdd2D(newOrigin, offset, newOrigin);
    }

    if (m_fCrouchWeight >= 0.5) {
        m_csCurrentPosition = STRING_CROUCH;
    } else {
        m_csCurrentPosition = STRING_STAND;
    }

    UpdateAimMotion();
    UpdateAnim();

    end[0] = newOrigin[0];
    end[1] = newOrigin[1];
    end[2] = newOrigin[2] - 94;
    trace  = G_Trace(newOrigin, MINS, MAXS, end, this, MASK_PATHSOLID, qfalse, "Actor::ThinkHoldGun_TurretGun");

    if (trace.fraction != 1.0 && !trace.startsolid && !trace.allsolid && trace.ent) {
        SafeSetOrigin(trace.endpos);
    }

    VectorClear(velocity);
    UpdateBoneControllers();
    UpdateFootsteps();
}

void Actor::Think_MachineGunner_TurretGun(void)
{
    if (!m_bEnableEnemy) {
        ThinkHoldGun_TurretGun();
        return;
    }

    if (level.inttime < m_iEnemyCheckTime + 200) {
        ThinkHoldGun_TurretGun();
        return;
    }

    UpdateEnemyInternal();

    if (m_pTurret->AI_CanTarget(G_GetEntity(0)->centroid)) {
        ThinkHoldGun_TurretGun();
        return;
    }

    if (m_pGrenade && rand() / 21474836.f <= m_fGrenadeAwareness) {
        BecomeTurretGuy();
        return;
    }

    if (!G_SightTrace(
            EyePosition(),
            vec_zero,
            vec_zero,
            static_cast<Sentient *>(G_GetEntity(0))->EyePosition(),
            this,
            G_GetEntity(0),
            MASK_CANSEE,
            qfalse,
            "Actor::Think_MachineGunner"
        )) {
        ThinkHoldGun_TurretGun();
        return;
    }

    if (m_ThinkStates[THINKLEVEL_IDLE] != THINKSTATE_IDLE) {
        BecomeTurretGuy();
        return;
    }

    if (m_Enemy && !(m_Enemy->flags & FL_NOTARGET) && !EnemyIsDisguised() && m_PotentialEnemies.IsEnemyConfirmed()) {
        BecomeTurretGuy();
        return;
    }

    if (m_Enemy && m_iCuriousTime || (m_Enemy && !EnemyIsDisguised() && !m_PotentialEnemies.IsEnemyConfirmed())) {
        m_iCuriousAnimHint = 6;
        BecomeTurretGuy();
        return;
    }

    ThinkHoldGun_TurretGun();
}

void Actor::Think_MachineGunner(void)
{
    if (!RequireThink()) {
        return;
    }

    if (m_pTurret && m_pTurret->GetOwner() == this && !m_bNoPlayerCollision) {
        UpdateEyeOrigin();
        Think_MachineGunner_TurretGun();
    } else {
        BecomeTurretGuy();
    }
}

void Actor::FinishedAnimation_MachineGunner(void)
{
    if (!m_bAnimScriptSet && m_State == ACTOR_STATE_MACHINE_GUNNER_RELOADING) {
        TransitionState(ACTOR_STATE_MACHINE_GUNNER_READY, 0);
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
