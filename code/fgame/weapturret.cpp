/*
===========================================================================
Copyright (C) 2025 the OpenMoHAA team

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
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110EV_DEFAULT301  USA
===========================================================================
*/

// weapturret.cpp: MOH Turret. Usable by sentients and monsters.
//

#include "g_phys.h"
#include "weapturret.h"
#include "vehicleturret.h"
#include "player.h"
#include "../script/scriptexception.h"

Event EV_Turret_IdleCheckOffset
(
    "idleCheckOffset",
    EV_DEFAULT,
    "v",
    "offset",
    "Sets the offset to trace to for collision checking when idling",
    EV_NORMAL
);
Event EV_Turret_P_ViewOffset
(
    "viewOffset",
    EV_DEFAULT,
    "v",
    "offset",
    "Sets the view offset to use for the turret",
    EV_NORMAL
);
Event EV_Turret_MaxIdlePitch
(
    "maxIdlePitch",
    EV_DEFAULT,
    "f",
    "offset",
    "Sets the offset to trace to for collision checking when idling",
    EV_NORMAL
);
Event EV_Turret_MaxIdleYaw
(
    "maxIdleYaw",
    EV_DEFAULT,
    "f",
    "offset",
    "Sets the offset to trace to for collision checking when idling",
    EV_NORMAL
);
Event EV_Turret_P_SetPlayerUsable
(
    "setPlayerUsable",
    EV_DEFAULT,
    "i",
    "state",
    "Sets whether the turret can be used by players. 0 means no, 1 means yes.",
    EV_NORMAL
);
Event EV_Turret_AI_SetAimTarget
(
    "setAimTarget",
    EV_DEFAULT,
    "S",
    "target",
    "Makes the turret aim at an entity",
    EV_NORMAL
);
Event EV_Turret_AI_SetAimOffset
(
    "setAimOffset",
    EV_DEFAULT,
    "v",
    "offset",
    "Makes the turret aim with specified offset",
    EV_NORMAL
);
Event EV_Turret_AI_ClearAimTarget
(
    "clearAimTarget",
    EV_DEFAULT,
    "S",
    "target",
    "Makes the turret aim at an entity",
    EV_NORMAL
);
Event EV_Turret_AI_StartFiring
(
    "startFiring",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes the turret start shooting",
    EV_NORMAL
);
Event EV_Turret_AI_StopFiring
(
    "stopFiring",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes the turret stop shooting",
    EV_NORMAL
);
Event EV_Turret_AI_TurnSpeed
(
    "turnSpeed",
    EV_DEFAULT,
    "f",
    "speed",
    "Sets the turret's turn speed",
    EV_NORMAL
);
Event EV_Turret_AI_PitchSpeed
(
    "pitchSpeed",
    EV_DEFAULT,
    "f",
    "speed",
    "Sets the turret's pitch speed",
    EV_NORMAL
);
Event EV_Turret_PitchCaps
(
    "pitchCaps",
    EV_DEFAULT,
    "v",
    "caps",
    "Sets the pitch caps for the turret. First number is upward cap, second is downward cap, and the third just makes "
    "it "
    "nice little vector",
    EV_NORMAL
);
Event EV_Turret_MaxYawOffset
(
    "maxYawOffset",
    EV_DEFAULT,
    "f",
    "maxoffset",
    "Sets the max yaw offset from the turrets central facing direction",
    EV_NORMAL
);
Event EV_Turret_YawCenter
(
    "yawCenter",
    EV_DEFAULT,
    "f",
    "yaw",
    "Sets the yaw for the center of the turret's turning arc",
    EV_NORMAL
);
Event EV_Turret_P_UserDistance
(
    "userdistance",
    EV_DEFAULT,
    "f",
    "dist",
    "Sets the distance the user should be placed at while using this turret",
    EV_NORMAL
);
Event EV_Turret_P_ViewJitter
(
    "viewjitter",
    EV_DEFAULT,
    "f",
    "amount",
    "Sets the amount that the owner's view should be jittered when fired",
    EV_NORMAL
);
Event EV_Turret_EventDoJitter
(
    "dojitter",
    EV_DEFAULT,
    "F",
    "jitterAmount",
    "Apply the jitter without firing",
    EV_NORMAL
);
Event EV_Turret_AI_BurstFireSettings
(
    "burstFireSettings",
    EV_DEFAULT,
    "ffff",
    "mintime maxtime mindelay maxdelay",
    "Sets the settings for burst mode firing",
    EV_NORMAL
);
Event EV_Turret_Enter
(
    "turretenter",
    EV_DEFAULT,
    "eS",
    "turret driver_anim",
    "Called when someone gets into a turret.",
    EV_NORMAL
);
Event EV_Turret_Exit
(
    "turretexit",
    EV_DEFAULT,
    "e",
    "turret",
    "Called when driver gets out of the turret.",
    EV_NORMAL
);
Event EV_Turret_P_SetThread
(
    "setthread",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the name of the thread called when a player uses the turret",
    EV_NORMAL
);
Event EV_Turret_AI_SetTargetType
(
    "targettype",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the target type to be none, any, or player",
    EV_NORMAL
);
Event EV_Turret_AI_SetTargetType2
(
    "targettype",
    EV_DEFAULT,
    "s",
    "value",
    "Sets the target type to be none any, or player",
    EV_SETTER
);
Event EV_Turret_AI_GetTargetType
(
    "targettype",
    EV_DEFAULT,
    NULL,
    NULL,
    "Gets the target type",
    EV_GETTER
);
Event EV_Turret_AI_ConvergeTime
(
    "convergeTime",
    EV_DEFAULT,
    "f",
    "time",
    "Sets the convergence time",
    EV_NORMAL
);
Event EV_Turret_AI_SuppressTime
(
    "suppressTime",
    EV_DEFAULT,
    "f",
    "time",
    "Sets the suppression time",
    EV_NORMAL
);
Event EV_Turret_AI_SuppressWaitTime
(
    "suppressWaitTime",
    EV_DEFAULT,
    "f",
    "time",
    "Sets the suppression wait nonfiring time before turret returns to default position",
    EV_NORMAL
);
Event EV_Turret_AI_SuppressWidth
(
    "suppressWidth",
    EV_DEFAULT,
    "f",
    "radius",
    "Sets the horizontal radius of suppression fire",
    EV_NORMAL
);
Event EV_Turret_AI_SuppressHeight
(
    "suppressHeight",
    EV_DEFAULT,
    "f",
    "radius",
    "Sets the vertical radius of suppression fire",
    EV_NORMAL
);
Event EV_Turret_SetUsable
(
    "setusable",
    EV_DEFAULT,
    "i",
    "state",
    "Sets whether the turret can be used as a weapon. 0 means no, 1 means yes.",
    EV_NORMAL
);
Event EV_Turret_AI_SetBulletSpread
(
    "aibulletspread",
    EV_DEFAULT,
    "ff",
    "bulletSpreadX bulletSpreadY",
    "Set the spread of the bullets in the x and y axis",
    EV_NORMAL
);
Event EV_Turret_SetMaxUseAngle
(
    "maxuseangle",
    EV_DEFAULT,
    "f",
    "maxuseangle",
    "Set max use angle to allow player to mount the turret.",
    EV_NORMAL
);
Event EV_Turret_SetStartYaw
(
    "startyaw",
    EV_DEFAULT,
    "f",
    "startyaw",
    "Sets the yaw to be used as the center of our allowed turn arc.",
    EV_NORMAL
);
Event EV_Turret_P_SetViewAngles
(
    "psetviewangles",
    EV_DEFAULT,
    "v",
    "offset",
    "Sets the view angles.",
    EV_NORMAL
);

static cvar_t *pTurretCameras = NULL;

CLASS_DECLARATION(Weapon, TurretGun, NULL) {
    {&EV_Trigger_Effect,              NULL                                   },
    {&EV_Item_DropToFloor,            &TurretGun::PlaceTurret                },
    {&EV_Item_Pickup,                 NULL                                   },
    {&EV_Weapon_Shoot,                &TurretGun::Shoot                      },
    {&EV_Use,                         &TurretGun::TurretUsed                 },
    {&EV_Turret_P_SetPlayerUsable,    &TurretGun::P_SetPlayerUsable          },
    {&EV_Turret_SetUsable,            &TurretGun::EventSetUsable             },
    {&EV_Turret_IdleCheckOffset,      &TurretGun::SetIdleCheckOffset         },
    {&EV_Turret_P_ViewOffset,         &TurretGun::P_SetViewOffset            },
    {&EV_Turret_MaxIdlePitch,         &TurretGun::EventMaxIdlePitch          },
    {&EV_Turret_MaxIdleYaw,           &TurretGun::EventMaxIdleYaw            },
    {&EV_Turret_AI_SetAimTarget,      &TurretGun::AI_EventSetAimTarget       },
    {&EV_Turret_AI_SetAimOffset,      &TurretGun::AI_EventSetAimOffset       },
    {&EV_Turret_AI_ClearAimTarget,    &TurretGun::AI_EventClearAimTarget     },
    {&EV_Turret_AI_StartFiring,       &TurretGun::AI_EventStartFiring        },
    {&EV_Turret_AI_StopFiring,        &TurretGun::AI_EventStopFiring         },
    {&EV_Turret_AI_TurnSpeed,         &TurretGun::AI_EventTurnSpeed          },
    {&EV_Turret_AI_PitchSpeed,        &TurretGun::AI_EventPitchSpeed         },
    {&EV_Turret_PitchCaps,            &TurretGun::EventPitchCaps             },
    {&EV_Turret_MaxYawOffset,         &TurretGun::EventMaxYawOffset          },
    {&EV_Turret_YawCenter,            &TurretGun::EventYawCenter             },
    {&EV_Turret_P_UserDistance,       &TurretGun::P_EventUserDistance        },
    {&EV_Turret_P_ViewJitter,         &TurretGun::P_EventViewJitter          },
    {&EV_Turret_EventDoJitter,        &TurretGun::P_EventDoJitter            },
    {&EV_Turret_AI_BurstFireSettings, &TurretGun::AI_EventBurstFireSettings  },
    {&EV_Turret_P_SetThread,          &TurretGun::EventSetThread             },
    {&EV_Turret_SetMaxUseAngle,       &TurretGun::P_SetMaxUseAngle           },
    {&EV_Turret_SetStartYaw,          &TurretGun::P_SetStartYaw              },
    {&EV_Turret_P_SetViewAngles,      &TurretGun::P_SetViewAnglesForTurret   },
    {&EV_SetViewangles,               &TurretGun::P_SetViewangles            },
    {&EV_GetViewangles,               &TurretGun::P_GetViewangles            },
    {&EV_Turret_AI_SetTargetType,     &TurretGun::AI_EventSetTargetType      },
    {&EV_Turret_AI_SetTargetType2,    &TurretGun::AI_EventSetTargetType      },
    {&EV_Turret_AI_GetTargetType,     &TurretGun::AI_EventGetTargetType      },
    {&EV_Turret_AI_ConvergeTime,      &TurretGun::AI_EventSetConvergeTime    },
    {&EV_Turret_AI_SuppressTime,      &TurretGun::AI_EventSetSuppressTime    },
    {&EV_Turret_AI_SuppressWaitTime,  &TurretGun::AI_EventSetSuppressWaitTime},
    {&EV_Turret_AI_SuppressWidth,     &TurretGun::AI_EventSetSuppressWidth   },
    {&EV_Turret_AI_SuppressHeight,    &TurretGun::AI_EventSetSuppressHeight  },
    {&EV_Turret_AI_SetBulletSpread,   &TurretGun::AI_EventSetBulletSpread    },
    {NULL,                            NULL                                   }
};

TurretGun::TurretGun()
{
    entflags |= ECF_TURRET;

    AddWaitTill(STRING_ONTARGET);

    if (LoadingSavegame) {
        return;
    }

    // turrets must not respawn
    // it can't be picked up
    setRespawn(qfalse);

    // allow monsters to use the turret
    respondto = TRIGGER_PLAYERS | TRIGGER_MONSTERS;

    // set the clipmask
    edict->clipmask = MASK_AUTOCALCLIFE;

    // make the turret already usable
    m_bUsable       = true;
    m_bPlayerUsable = true;
    m_bRestable     = true;

    // set the fakebullets
    m_bFakeBullets = (spawnflags & FAKEBULLETS);

    m_fIdlePitchSpeed = 0;
    m_iIdleHitCount   = 0;
    m_fMaxIdlePitch   = -80;
    m_fMaxIdleYaw     = 180;

    // set the size
    setSize(Vector(-16, -16, -8), Vector(16, 16, 32));

    // setup the turret angles cap
    m_fTurnSpeed    = 180;
    m_fAIPitchSpeed = 180;
    m_fPitchUpCap   = -45;
    m_fPitchDownCap = 45;
    m_fMaxYawOffset = 180;
    m_fUserDistance = 64;

    // set the idle angles
    m_vIdleCheckOffset = Vector(-56, 0, 0);

    // set the burst time
    m_fMinBurstTime = 0;
    m_fMaxBurstTime = 0;
    // set the burst delay
    m_fMinBurstDelay = 0;
    m_fMaxBurstDelay = 0;

    m_fFireToggleTime = level.time;
    m_iFiring         = TURRETFIRESTATE_NONE;
    m_iTargetType     = 0;

    // set the camera
    m_pUserCamera = NULL;

    // setup the view jitter
    m_fViewJitter     = 0;
    m_fCurrViewJitter = 0;
    // turret doesn't have an owner when spawning
    m_bHadOwner  = false;
    m_pViewModel = NULL;

    m_iAIState        = TURRETAISTATE_DEFAULT;
    m_fAIConvergeTime = g_turret_convergetime->value;
    if (m_fAIConvergeTime < 0) {
        m_fAIConvergeTime = 0;
    }

    m_iAISuppressTime = g_turret_suppresstime->value * 1000;
    if (m_iAISuppressTime < 0) {
        m_iAISuppressTime = 0;
    }

    m_iAISuppressWaitTime = g_turret_suppresswaittime->value * 1000;
    if (m_iAISuppressWaitTime < 0) {
        m_iAISuppressWaitTime = 0;
    }

    m_iAILastTrackTime     = 0;
    m_iAIStartSuppressTime = 0;
    VectorClear(m_vDesiredTargetAngles);
    m_fAIDesiredTargetSpeed = 0;
    VectorClear(m_vAIDesiredTargetPosition);
    VectorClear(m_vAITargetPosition);
    VectorClear(m_vAICurrentTargetPosition);
    VectorClear(m_vAITargetSpeed);
    m_iAINextSuppressTime = 0;
    m_fAISuppressWidth    = 256;
    m_fAISuppressHeight   = 64;
    VectorClear(m_vMuzzlePosition);
    fire_delay[FIRE_PRIMARY] = 0.05f;
    m_fMaxUseAngle           = 80;
}

TurretGun::~TurretGun()
{
    Unregister(STRING_ONTARGET);

    //
    // Added in OPM:
    //  Remove the user camera if any.
    //
    if (m_pUserCamera) {
        RemoveUserCamera();
    }

    //
    // Added in OPM:
    //  Remove the view model.
    //
    if (m_pViewModel) {
        m_pViewModel->Delete();
        m_pViewModel = NULL;
    }

    //
    // Added in OPM:
    //  Detach the owner from the turret.
    //
    if (owner && owner->IsSubclassOfPlayer()) {
        Player *player = static_cast<Player *>(owner.Pointer());
        player->ExitTurret();
    }

    entflags &= ~ECF_TURRET;
}

void TurretGun::PlaceTurret(Event *ev)
{
    // Don't make the turret solid
    setSolidType(SOLID_NOT);

    // The turret shouldn't move
    setMoveType(MOVETYPE_NONE);

    showModel();

    m_fStartYaw  = angles[1];
    groundentity = NULL;

    if (m_bFakeBullets) {
        firetype[FIRE_PRIMARY] = FT_FAKEBULLET;
    }

    flags |= FL_THINK;
}

void TurretGun::ThinkIdle(void)
{
    Vector  vDir, vNewAngles, vEnd;
    trace_t trace;

    if (!m_bRestable) {
        return;
    }

    if (angles[0] > 180) {
        angles[0] -= 360;
    }

    if (angles[0] <= m_fMaxIdlePitch) {
        m_fIdlePitchSpeed = 0;
        m_iIdleHitCount   = 0;
        angles[0]         = m_fMaxIdlePitch;
        setAngles(angles);
        return;
    }

    if (m_iIdleHitCount >= 2) {
        return;
    }

    m_fIdlePitchSpeed -= level.frametime * 300.0f;

    vNewAngles = Vector(Q_max(angles[0] + level.frametime * m_fIdlePitchSpeed, m_fMaxIdlePitch), angles[1], angles[2]);

    vNewAngles.AngleVectorsLeft(&vDir);
    vEnd = origin + vDir * m_vIdleCheckOffset[0];

    trace = G_Trace(origin, vec_zero, vec_zero, vEnd, this, edict->clipmask, false, "TurretGun::Think");

    if (trace.fraction == 1.0f) {
        setAngles(vNewAngles);
        m_iIdleHitCount = 0;
        return;
    }

    int iTry;
    for (iTry = 3; iTry > 0; iTry--) {
        vNewAngles[0] = angles[0] + level.frametime * m_fIdlePitchSpeed * iTry * 0.25f;
        if (vNewAngles[0] < m_fMaxIdlePitch) {
            continue;
        }

        vNewAngles.AngleVectorsLeft(&vDir);
        vEnd = origin + vDir * m_vIdleCheckOffset[0];

        trace = G_Trace(origin, vec_zero, vec_zero, vEnd, this, edict->clipmask, false, "TurretGun::Think");

        if (trace.fraction == 1) {
            setAngles(vNewAngles);

            m_iIdleHitCount = 0;
            m_fIdlePitchSpeed *= 0.25f * iTry;
            break;
        }
    }

    if (!iTry) {
        m_fIdlePitchSpeed = 0;

        Entity *ent = G_GetEntity(trace.entityNum);

        if (ent && ent == world) {
            m_iIdleHitCount++;
        } else {
            m_iIdleHitCount = 0;
        }
    }
}

void TurretGun::AI_SetTargetAngles(vec3_t vTargAngles, float speed)
{
    float fDiff;
    float fPitchDiff, fYawDiff;
    float fTurnYawSpeed;
    float fTurnPitchSpeed;

    if (vTargAngles[0] > 180.0f) {
        vTargAngles[0] -= 360.0f;
    } else if (vTargAngles[0] < -180.0f) {
        vTargAngles[0] += 360.0f;
    }

    if (vTargAngles[0] < m_fPitchUpCap) {
        vTargAngles[0] = m_fPitchUpCap;
    } else if (vTargAngles[0] > m_fPitchDownCap) {
        vTargAngles[0] = m_fPitchDownCap;
    }

    fDiff = AngleSubtract(vTargAngles[1], m_fStartYaw);
    if (fDiff > m_fMaxYawOffset) {
        vTargAngles[1] = m_fStartYaw + m_fMaxYawOffset;
    } else if (fDiff < -m_fMaxYawOffset) {
        vTargAngles[1] = m_fStartYaw - m_fMaxYawOffset;
    }

    fYawDiff   = AngleSubtract(vTargAngles[1], angles[1]);
    fPitchDiff = AngleSubtract(vTargAngles[0], angles[0]);

    if (speed == 0) {
        fTurnYawSpeed   = m_fTurnSpeed * level.frametime;
        fTurnPitchSpeed = m_fAIPitchSpeed * level.frametime;
    } else {
        float pitchFrameSpeed, yawFrameSpeed;

        // pitch
        pitchFrameSpeed = fabs(fPitchDiff) / speed;
        if (pitchFrameSpeed > 720) {
            pitchFrameSpeed = 720;
        }
        fTurnPitchSpeed = pitchFrameSpeed * level.frametime;

        // yaw
        yawFrameSpeed = fabs(fYawDiff) / speed;
        if (yawFrameSpeed > 720) {
            yawFrameSpeed = 720;
        }
        fTurnYawSpeed = yawFrameSpeed * level.frametime;
    }

    if (fabs(fPitchDiff) < fTurnPitchSpeed) {
        angles[0] = vTargAngles[0];
    } else if (fPitchDiff > 0) {
        angles[0] += fTurnPitchSpeed;
    } else {
        angles[0] -= fTurnPitchSpeed;
    }

    if (fabs(fYawDiff) < fTurnYawSpeed) {
        angles[1] = vTargAngles[1];
    } else if (fYawDiff > 0) {
        angles[1] += fTurnYawSpeed;
    } else {
        angles[1] -= fTurnYawSpeed;
    }

    setAngles(angles);

    if (fabs(fDiff) < 2) {
        Unregister(STRING_ONTARGET);
    }
}

void TurretGun::AI_SetDesiredTargetAngles(const vec3_t angles, float speed)
{
    VectorCopy(angles, m_vDesiredTargetAngles);
    m_fAIDesiredTargetSpeed = speed;
}

void TurretGun::P_SetTargetAngles(Vector& vTargAngles)
{
    float fDiff;

    if (vTargAngles[0] > 180.0f) {
        vTargAngles[0] -= 360.0f;
    } else if (vTargAngles[0] < -180.0f) {
        vTargAngles[0] += 360.0f;
    }

    if (vTargAngles[0] < m_fPitchUpCap) {
        vTargAngles[0] = m_fPitchUpCap;
    } else if (vTargAngles[0] > m_fPitchDownCap) {
        vTargAngles[0] = m_fPitchDownCap;
    }

    fDiff = AngleSubtract(m_fPitchDownCap, angles[0]);
    if (fabs(fDiff) >= 180) {
        if (fDiff > 0.0f) {
            angles[0] += 180;
        } else {
            angles[0] -= 180;
        }
    } else {
        angles[0] = vTargAngles[0];
    }

    fDiff = AngleSubtract(vTargAngles[1], m_fStartYaw);
    if (fDiff > m_fMaxYawOffset) {
        fDiff = m_fMaxYawOffset;
    } else if (fDiff < -m_fMaxYawOffset) {
        fDiff = -m_fMaxYawOffset;
    }
    vTargAngles[1] = m_fStartYaw + fDiff;

    fDiff = AngleSubtract(vTargAngles[1], angles[1]);

    if (fabs(fDiff) >= 180) {
        if (fDiff > 0.0f) {
            angles[1] += 180;
        } else {
            angles[1] -= 180;
        }
    } else {
        angles[1] = vTargAngles[1];
    }

    setAngles(angles);
}

bool TurretGun::AI_CanTarget(const vec3_t pos)
{
    vec3_t delta;
    Vector vAngles;
    float  ang;
    float  yawCap;

    VectorSubtract(pos, origin, delta);
    vectoangles(delta, vAngles);

    ang = AngleSubtract(vAngles[1], m_fStartYaw);

    if (vAngles[0] > 180) {
        vAngles[0] -= 360;
    } else if (vAngles[0] < -180) {
        vAngles[0] += 360;
    }

    if (vAngles[0] < m_fPitchUpCap || vAngles[0] > m_fPitchDownCap) {
        return false;
    }

    yawCap = AngleSubtract(vAngles[1], m_fStartYaw);
    if (yawCap > m_fMaxYawOffset) {
        return false;
    } else if (yawCap < -m_fMaxYawOffset) {
        return false;
    }

    return true;
}

void TurretGun::P_ThinkActive(void)
{
    Player *player;
    float   fDiff;
    Vector  vTarg;
    Vector  vTargAngles;
    Vector  vAngles;
    Vector  vPos;
    Vector  vEnd;
    Vector  vForward;
    Vector  vMins;
    Vector  vMaxs;
    trace_t trace;

    // Limit the pitch
    if (m_vUserViewAng[0] < m_fPitchUpCap) {
        m_vUserViewAng[0] = m_fPitchUpCap;
    } else if (m_vUserViewAng[0] > m_fPitchDownCap) {
        m_vUserViewAng[0] = m_fPitchDownCap;
    }

    //
    // Limit the yaw
    //

    fDiff = AngleSubtract(m_vUserViewAng[1], m_fStartYaw);
    fDiff = Q_clamp_float(fDiff, -m_fMaxYawOffset, m_fMaxYawOffset);

    m_vUserViewAng[1] = m_fStartYaw + fDiff;

    // Set the user's view angles
    owner->SetViewAngles(m_vUserViewAng);

    //
    // Set the weapon angles to make it point straight
    // to the user's target
    //

    vTarg       = owner->GunTarget(false) - origin;
    vTargAngles = vTarg.toAngles();
    P_SetTargetAngles(vTargAngles);

    // Handle firing
    if (m_iFiring != TURRETFIRESTATE_NONE) {
        m_iFiring = TURRETFIRESTATE_FIRING;

        if (ReadyToFire(FIRE_PRIMARY)) {
            Fire(FIRE_PRIMARY);
            m_fCurrViewJitter = m_fViewJitter;
        }
    }

    //
    // Setup the camera
    //

    player  = static_cast<Player *>(owner.Pointer());
    vAngles = m_vUserViewAng;

    if (!m_pUserCamera) {
        m_pUserCamera = new Camera;
    }

    P_ApplyFiringViewJitter(vAngles);

    m_pUserCamera->setOrigin(origin);
    m_pUserCamera->setAngles(vAngles);
    m_pUserCamera->SetPositionOffset(m_vViewOffset);

    owner->client->ps.camera_flags |= CF_CAMERA_ANGLES_TURRETMODE;

    if (!player->IsZoomed()) {
        player->ToggleZoom(80);
    }

    //
    // Position the player appropriately
    // so it looks like they're holding the turret
    //

    Vector(0, angles[1], 0).AngleVectorsLeft(&vForward);

    vPos = origin - vForward * m_fUserDistance;
    vPos[2] -= 16.0f;

    vEnd = vPos;
    vEnd[2] -= 64.0f;

    vMins    = owner->mins;
    vMaxs    = owner->maxs;
    vMaxs[2] = owner->mins[2] + 4.0f;

    trace = G_Trace(vPos, vMins, vMaxs, vEnd, owner, MASK_PLAYERSOLID, qtrue, "TurretGun::P_ThinkActive 1");

    vPos = trace.endpos;

    //
    // Test position, to avoid positioning the player inside a solid object
    //
    trace = G_Trace(vPos, owner->mins, owner->maxs, vPos, owner, MASK_PLAYERSOLID, qtrue, "TurretGun::P_ThinkActive 2");

    if (!trace.allsolid && !trace.startsolid) {
        owner->setOrigin(vPos);
    }
}

void TurretGun::AI_DoTargetNone()
{
    vec3_t desiredAngles;
    vec3_t delta;

    if (!aim_target) {
        return;
    }

    VectorSubtract(aim_target->centroid, origin, delta);
    VectorAdd(delta, m_Aim_offset, delta);
    vectoangles(delta, desiredAngles);
    AI_SetTargetAngles(desiredAngles, 0);
}

void TurretGun::AI_MoveToDefaultPosition()
{
    vec3_t desiredAngles;
    // get the first client number
    Entity *pEnt = G_GetEntity(0);
    if (pEnt) {
        vec3_t delta;

        VectorSubtract(pEnt->centroid, origin, delta);
        vectoangles(delta, desiredAngles);
    } else {
        desiredAngles[0] = 0;
    }

    desiredAngles[1] = m_fStartYaw;
    desiredAngles[2] = 0;
    AI_SetDesiredTargetAngles(desiredAngles, 0);
}

void TurretGun::AI_DoTargetAutoDefault()
{
    m_iFiring = TURRETFIRESTATE_NONE;
    if (owner->m_Enemy) {
        Actor *actor = static_cast<Actor *>(owner.Pointer());
        if (actor->CanSeeEnemy(200)) {
            AI_StartTrack();
        } else {
            AI_MoveToDefaultPosition();
        }
    }
}

void TurretGun::AI_StartDefault()
{
    m_iFiring  = TURRETFIRESTATE_NONE;
    m_iAIState = TURRETAISTATE_DEFAULT;
}

void TurretGun::AI_StartSuppress()
{
    m_iAIStartSuppressTime = level.inttime;
    m_iAIState             = TURRETAISTATE_SUPPRESS;
    VectorCopy(m_vAIDesiredTargetPosition, m_vAITargetPosition);
    VectorClear(m_vAICurrentTargetPosition);
    VectorClear(m_vAITargetSpeed);
    m_iAINextSuppressTime = 0;
}

void TurretGun::AI_StartSuppressWait()
{
    m_iAIStartSuppressTime = level.inttime;
    m_iAIState             = TURRETAISTATE_SUPPRESS_WAIT;
}

void TurretGun::AI_StartTrack()
{
    if (m_iAIState == TURRETAISTATE_DEFAULT) {
        m_iAILastTrackTime = level.inttime;
    }
    m_iAIState = TURRETAISTATE_TRACK;
}

void TurretGun::AI_DoTargetAutoTrack()
{
    Actor *actor;
    Vector end;
    Vector delta;
    vec3_t desiredAngles;
    float  turnSpeed;

    if (!owner->m_Enemy) {
        AI_StartDefault();
        return;
    }

    if (owner->m_Enemy != m_pAIEnemy) {
        if (m_pAIEnemy) {
            m_iAILastTrackTime = level.inttime;
        }
        m_pAIEnemy = owner->m_Enemy;
    }

    actor = static_cast<Actor *>(owner.Pointer());
    if (!actor->CanSeeEnemy(0)) {
        AI_StartSuppress();
        return;
    }

    if (m_iFiring == TURRETFIRESTATE_NONE) {
        m_iFiring = TURRETFIRESTATE_BEGIN_FIRE;
    }

    end = owner->m_Enemy->centroid;
    if (!G_SightTrace(
            m_vMuzzlePosition,
            vec_zero,
            vec_zero,
            end,
            this,
            owner->m_Enemy,
            MASK_AITURRET,
            qfalse,
            "TurretGun::AI_DoTargetAutoTrack"
        )) {
        end = owner->m_Enemy->EyePosition();
    }

    VectorCopy(end, m_vAIDesiredTargetPosition);
    delta = m_vAIDesiredTargetPosition - origin;
    vectoangles(delta, desiredAngles);

    turnSpeed = m_fAIConvergeTime - (level.inttime - m_iAILastTrackTime) / 1000;
    if (turnSpeed < 0) {
        turnSpeed = 0;
    }

    AI_SetDesiredTargetAngles(desiredAngles, turnSpeed);
}

void TurretGun::AI_DoSuppressionAiming()
{
    float  suppressRatio;
    float  suppressTimeRatio;
    float  height;
    float  length;
    vec2_t dir;
    vec3_t deltaPos;
    vec2_t speedxy;
    vec3_t desiredAngles;

    suppressRatio = m_fAISuppressHeight / m_fAISuppressWidth;

    if (level.inttime >= m_iAINextSuppressTime) {
        suppressTimeRatio = this->m_fAISuppressWidth / 250.0;
        dir[0]            = m_vAIDesiredTargetPosition[1] - origin[1];
        dir[1]            = m_vAIDesiredTargetPosition[0] - origin[0];
        VectorNormalize2D(dir);

        m_iAINextSuppressTime = level.inttime + (((random() + 1) / 2 * suppressTimeRatio) * 1000);

        VectorSubtract(m_vAITargetPosition, m_vAIDesiredTargetPosition, deltaPos);

        //
        // calculate the speed at which the AI should turn
        //
        speedxy[0] = DotProduct2D(deltaPos, dir);
        speedxy[1] = deltaPos[2];
        VectorNormalize2D(speedxy);

        speedxy[0] += crandom() / 2.0;
        speedxy[1] += crandom() / 2.0;
        VectorNormalize2D(speedxy);

        speedxy[0] = 250 / (suppressTimeRatio * 0.5) * speedxy[0];
        speedxy[1] = 250 / (suppressTimeRatio * 0.5) * speedxy[1];
        speedxy[1] *= suppressRatio;
        m_vAITargetSpeed[0] = speedxy[0] * dir[0];
        m_vAITargetSpeed[1] = speedxy[0] * dir[1];
        m_vAITargetSpeed[1] = speedxy[1];
    }

    VectorMA(m_vAICurrentTargetPosition, level.frametime, m_vAITargetSpeed, m_vAICurrentTargetPosition);

    height = suppressRatio * 250;
    if (m_vAICurrentTargetPosition[2] > height) {
        m_vAICurrentTargetPosition[2] = height;
    } else if (m_vAICurrentTargetPosition[2] < -height) {
        m_vAICurrentTargetPosition[2] = -height;
    }

    length = VectorLength2D(m_vAICurrentTargetPosition);
    if (length > 250) {
        VectorScale2D(m_vAICurrentTargetPosition, 250 / length, m_vAICurrentTargetPosition);
    }

    VectorSubtract(m_vAIDesiredTargetPosition, m_vAITargetPosition, deltaPos);
    VectorMA(deltaPos, level.frametime, m_vAICurrentTargetPosition, deltaPos);

    if (deltaPos[2] > m_fAISuppressHeight) {
        deltaPos[2]           = m_fAISuppressHeight;
        m_iAINextSuppressTime = 0;
    } else if (deltaPos[2] < -m_fAISuppressHeight) {
        deltaPos[2]           = -m_fAISuppressHeight;
        m_iAINextSuppressTime = 0;
    }

    length = VectorLength2D(deltaPos);
    if (length > m_fAISuppressWidth) {
        VectorScale(deltaPos, m_fAISuppressWidth / length, deltaPos);
        m_iAINextSuppressTime = 0;
    }

    // calculate the target angles
    VectorAdd(m_vAITargetPosition, deltaPos, m_vAIDesiredTargetPosition);
    VectorSubtract(m_vAIDesiredTargetPosition, origin, deltaPos);

    // set the desired angles
    vectoangles(deltaPos, desiredAngles);
    AI_SetDesiredTargetAngles(desiredAngles, 0);
}

void TurretGun::AI_DoTargetAutoSuppress()
{
    Actor *actor;

    if (!owner->m_Enemy) {
        AI_StartDefault();
        return;
    }

    actor = static_cast<Actor *>(owner.Pointer());
    if (actor->CanSeeEnemy(200)) {
        AI_StartTrack();
        return;
    }

    if (level.inttime >= m_iAISuppressTime + m_iAIStartSuppressTime) {
        AI_StartSuppressWait();
        return;
    }

    if (m_iFiring == TURRETFIRESTATE_NONE) {
        m_iFiring = TURRETFIRESTATE_BEGIN_FIRE;
    }
    AI_DoSuppressionAiming();
}

void TurretGun::AI_DoTargetAutoSuppressWait()
{
    m_iFiring = TURRETFIRESTATE_NONE;
    Actor *actor;

    if (!owner->m_Enemy) {
        AI_StartDefault();
        return;
    }

    actor = static_cast<Actor *>(owner.Pointer());
    if (actor->CanSeeEnemy(200)) {
        AI_StartTrack();
        return;
    }

    if (level.inttime >= m_iAISuppressWaitTime + m_iAIStartSuppressTime) {
        AI_StartDefault();
        return;
    }

    AI_DoSuppressionAiming();
}

void TurretGun::AI_DoTargetAuto()
{
    if (!owner) {
        return;
    }

    GetMuzzlePosition(m_vMuzzlePosition, NULL, NULL, NULL, NULL);

    switch (m_iAIState) {
    case TURRETAISTATE_DEFAULT:
        AI_DoTargetAutoDefault();
        break;
    case TURRETAISTATE_TRACK:
        AI_DoTargetAutoTrack();
        break;
    case TURRETAISTATE_SUPPRESS:
        AI_DoTargetAutoSuppress();
        break;
    case TURRETAISTATE_SUPPRESS_WAIT:
        AI_DoTargetAutoSuppressWait();
        break;
    default:
        break;
    }

    AI_SetTargetAngles(m_vDesiredTargetAngles, m_fAIDesiredTargetSpeed);
}

void TurretGun::AI_DoAiming()
{
    switch (m_iTargetType) {
    case 0:
        AI_DoTargetNone();
        break;
    case 1:
        AI_DoTargetAuto();
        break;
    default:
        break;
    }
}

void TurretGun::AI_DoFiring()
{
    float minBurstTime, maxBurstTime;
    float minBurstDelay, maxBurstDelay;

    if (g_target_game == target_game_e::TG_MOH) {
        //
        // Removed in 2.0
        //  The fire delay is always constant on 1.11 and below
        //
        if (m_iFiring == TURRETFIRESTATE_BEGIN_FIRE) {
            m_iFiring = TURRETFIRESTATE_FIRING;
        }

        if (IsFiring() && ReadyToFire(FIRE_PRIMARY)) {
            Fire(FIRE_PRIMARY);
        }

        return;
    }

    minBurstTime  = m_fMinBurstTime;
    maxBurstTime  = m_fMaxBurstTime;
    minBurstDelay = m_fMinBurstDelay;
    maxBurstDelay = m_fMaxBurstDelay;

    if (!maxBurstTime) {
        // Added in OPM
        //  default values if not set
        maxBurstTime = minBurstTime = 0.001f;
        maxBurstDelay = minBurstDelay = fire_delay[FIRE_PRIMARY] / 10.f;
    }

    switch (m_iFiring) {
    case TURRETFIRESTATE_BEGIN_FIRE:
        if (m_fMaxBurstTime > 0) {
            if (m_fFireToggleTime < level.time) {
                m_iFiring         = TURRETFIRESTATE_FIRING;
                m_fFireToggleTime = level.time + m_fMinBurstTime + (m_fMaxBurstTime - m_fMinBurstTime) * random();
            }
        } else {
            m_iFiring = TURRETFIRESTATE_FIRING;
        }
        break;
    case TURRETFIRESTATE_FIRING:
        Fire(FIRE_PRIMARY);

        if (m_fMaxBurstTime > 0) {
            if (m_fFireToggleTime < level.time) {
                m_iFiring         = TURRETFIRESTATE_BEGIN_FIRE;
                m_fFireToggleTime = level.time + m_fMinBurstDelay + (m_fMaxBurstDelay - m_fMinBurstDelay) * random();
            }
        }
        break;
    default:
        break;
    }
}

void TurretGun::AI_ThinkActive()
{
    if (!g_ai->integer) {
        return;
    }

    AI_DoAiming();
    AI_DoFiring();
}

void TurretGun::Think(void)
{
    if (!owner && (m_bHadOwner || !aim_target)) {
        ThinkIdle();
    } else if (owner && owner->IsSubclassOfPlayer()) {
        P_ThinkActive();
    } else {
        AI_ThinkActive();
    }
}

void TurretGun::P_UserAim(usercmd_t *ucmd)
{
    Vector vNewCmdAng;

    vNewCmdAng = Vector(SHORT2ANGLE(ucmd->angles[0]), SHORT2ANGLE(ucmd->angles[1]), SHORT2ANGLE(ucmd->angles[2]));

    if (m_vUserLastCmdAng[0] || m_vUserLastCmdAng[1] || m_vUserLastCmdAng[2]) {
        m_vUserViewAng[0] += AngleSubtract(vNewCmdAng[0], m_vUserLastCmdAng[0]);
        m_vUserViewAng[1] += AngleSubtract(vNewCmdAng[1], m_vUserLastCmdAng[1]);
        m_vUserViewAng[2] += AngleSubtract(vNewCmdAng[2], m_vUserLastCmdAng[2]);
    }

    m_vUserLastCmdAng = vNewCmdAng;

    if (ucmd->buttons & BUTTON_ATTACKLEFT) {
        if (m_iFiring == TURRETFIRESTATE_NONE) {
            m_iFiring = TURRETFIRESTATE_BEGIN_FIRE;
        }
    } else {
        m_iFiring = TURRETFIRESTATE_NONE;
    }

    flags |= FL_THINK;
}

qboolean TurretGun::UserAim(usercmd_t *ucmd)
{
    P_UserAim(ucmd);
    return qtrue;
}

void TurretGun::P_SetViewangles(Event *ev)
{
    m_vUserViewAng = ev->GetVector(1);
}

void TurretGun::P_SetViewAnglesForTurret(Event *ev)
{
    m_vUserViewAng = ev->GetVector(1);
}

void TurretGun::P_GetViewangles(Event *ev)
{
    ev->AddVector(m_vUserViewAng);
}

void TurretGun::AI_TurretBeginUsed(Sentient *pEnt)
{
    owner = pEnt;

    edict->r.ownerNum = pEnt->entnum;
    m_bHadOwner       = true;

    Sound(sPickupSound);

    current_attachToTag = "";
    ForceIdle();
}

void TurretGun::P_TurretBeginUsed(Player *pEnt)
{
    Player *player;

    if (!pTurretCameras) {
        pTurretCameras = gi.Cvar_Get("g_turretcameras", "1", 0);
    }

    owner = pEnt;

    edict->r.ownerNum = pEnt->entnum;
    m_bHadOwner       = true;

    Sound(sPickupSound);

    if (m_vUserViewAng[0] > 180.0f) {
        m_vUserViewAng[0] -= 360.0f;
    }

    m_vUserLastCmdAng = vec_zero;

    player = static_cast<Player *>(owner.Pointer());
    player->EnterTurret(this);

    if (!m_pUserCamera) {
        m_pUserCamera = new Camera;
    }

    m_pUserCamera->setOrigin(origin);
    m_pUserCamera->setAngles(angles);

    if (pTurretCameras->integer) {
        player->SetCamera(m_pUserCamera, 0.5f);
    }

    current_attachToTag = "";
    ForceIdle();
    P_CreateViewModel();
}

void TurretGun::TurretBeginUsed(Sentient *pEnt)
{
    AI_TurretBeginUsed(pEnt);
}

void TurretGun::AI_TurretEndUsed()
{
    owner             = NULL;
    edict->r.ownerNum = ENTITYNUM_NONE;

    m_fIdlePitchSpeed = 0;
    m_iIdleHitCount   = 0;
    m_iFiring         = TURRETFIRESTATE_NONE;
}

void TurretGun::RemoveUserCamera()
{
    if (!m_pUserCamera) {
        return;
    }

    if (owner && owner->IsSubclassOfPlayer()) {
        Player *player = static_cast<Player *>(owner.Pointer());

        player->SetCamera(NULL, 1.0f);
        player->ZoomOff();
        player->client->ps.camera_flags &= ~CF_CAMERA_ANGLES_TURRETMODE;
    }

    m_pUserCamera->PostEvent(EV_Remove, 0);
    m_pUserCamera = NULL;
}

void TurretGun::P_TurretEndUsed()
{
    Player *player = (Player *)owner.Pointer();
    float   yawCap;

    if (m_pUserCamera) {
        player->SetCamera(NULL, 1.0f);
        player->ZoomOff();
        player->client->ps.camera_flags &= ~CF_CAMERA_ANGLES_TURRETMODE;
    }

    player->ExitTurret();
    P_DeleteViewModel();

    owner             = NULL;
    edict->r.ownerNum = ENTITYNUM_NONE;

    m_fIdlePitchSpeed = 0;
    m_iIdleHitCount   = 0;
    m_iFiring         = TURRETFIRESTATE_NONE;

    yawCap = AngleSubtract(angles[1], m_fStartYaw);
    if (yawCap > m_fMaxIdleYaw) {
        yawCap = m_fMaxIdleYaw;
    } else if (yawCap < -m_fMaxIdleYaw) {
        yawCap = -m_fMaxIdleYaw;
    }

    angles[1] = m_fStartYaw + yawCap;
    setAngles(angles);
}

void TurretGun::TurretEndUsed(void)
{
    AI_TurretEndUsed();
}

void TurretGun::P_TurretUsed(Player *player)
{
    if (player == owner) {
        if ((!m_bPlayerUsable || !m_bUsable) && owner->health > 0.0f) {
            return;
        }
    }

    if (owner) {
        if (owner == player) {
            P_TurretEndUsed();
            m_iFiring = TURRETFIRESTATE_NONE;
        }
    } else {
        m_vUserViewAng = player->GetViewAngles();

        if (fabs(AngleSubtract(m_vUserViewAng[1], angles[1])) <= m_fMaxUseAngle) {
            if (player->charge_start_time) {
                return;
            }

            P_TurretBeginUsed(player);

            flags &= ~FL_THINK;
            m_iFiring = TURRETFIRESTATE_NONE;
            m_UseThread.Execute(this);
        }
    }
}

void TurretGun::TurretUsed(Sentient *pEnt)
{
    P_TurretUsed(static_cast<Player *>(pEnt));
}

void TurretGun::TurretUsed(Event *ev)
{
    Entity *pEnt = ev->GetEntity(1);

    if (!pEnt || !pEnt->IsSubclassOfPlayer()) {
        ScriptError("Bad entity trying to use turret");
        return;
    }

    if (m_bUsable && m_bPlayerUsable) {
        // Make the sentient use the turret
        P_TurretUsed(static_cast<Player *>(pEnt));
    }
}

void TurretGun::P_SetPlayerUsable(Event *ev)
{
    if (ev->GetInteger(1)) {
        m_bPlayerUsable = true;
    } else {
        m_bPlayerUsable = false;
    }
}

void TurretGun::EventSetUsable(Event *ev)
{
    if (ev->GetInteger(1)) {
        m_bUsable   = true;
        m_bRestable = true;
    } else {
        m_bUsable   = false;
        m_bRestable = false;
    }
}

void TurretGun::P_SetViewOffset(Event *ev)
{
    m_vViewOffset = ev->GetVector(1);
}

void TurretGun::EventMaxIdlePitch(Event *ev)
{
    m_fMaxIdlePitch = ev->GetFloat(1);
}

void TurretGun::EventMaxIdleYaw(Event *ev)
{
    m_fMaxIdleYaw = ev->GetFloat(1);
}

void TurretGun::SetIdleCheckOffset(Event *ev)
{
    m_vIdleCheckOffset = ev->GetVector(1);
}

void TurretGun::AI_EventSetAimTarget(Event *ev)
{
    aim_target = ev->GetEntity(1);
}

void TurretGun::AI_EventSetAimOffset(Event *ev)
{
    m_Aim_offset = ev->GetVector(1);
}

void TurretGun::AI_EventClearAimTarget(Event *ev)
{
    aim_target = NULL;

    // Clear idle values
    m_fIdlePitchSpeed = 0;
    m_iIdleHitCount   = 0;
}

void TurretGun::AI_EventStartFiring(Event *ev)
{
    if (m_iFiring == TURRETFIRESTATE_NONE) {
        m_iFiring = TURRETFIRESTATE_BEGIN_FIRE;
    }
}

void TurretGun::AI_EventStopFiring(Event *ev)
{
    m_iFiring = TURRETFIRESTATE_NONE;
}

void TurretGun::AI_EventTurnSpeed(Event *ev)
{
    AI_TurnSpeed(ev->GetFloat(1));
}

void TurretGun::AI_EventPitchSpeed(Event *ev)
{
    AI_PitchSpeed(ev->GetFloat(1));
}

void TurretGun::EventPitchCaps(Event *ev)
{
    Vector caps = ev->GetVector(1);
    PitchCaps(caps[0], caps[1]);
}

void TurretGun::EventMaxYawOffset(Event *ev)
{
    MaxYawOffset(ev->GetFloat(1));
}

void TurretGun::EventYawCenter(Event *ev)
{
    YawCenter(ev->GetFloat(1));
}

void TurretGun::P_EventUserDistance(Event *ev)
{
    P_UserDistance(ev->GetFloat(1));
}

void TurretGun::P_EventViewJitter(Event *ev)
{
    m_fViewJitter = ev->GetFloat(1);
}

void TurretGun::P_EventDoJitter(Event *ev)
{
    if (ev->NumArgs() > 0) {
        m_fCurrViewJitter = ev->GetFloat(1);
    } else {
        m_fCurrViewJitter = m_fViewJitter;
    }
}

void TurretGun::AI_EventBurstFireSettings(Event *ev)
{
    if (ev->NumArgs() < 4) {
        return;
    }

    AI_BurstFireSettings(ev->GetFloat(1), ev->GetFloat(2), ev->GetFloat(3), ev->GetFloat(4));
}

bool TurretGun::IsFiring(void)
{
    return m_iFiring == TURRETFIRESTATE_FIRING;
}

void TurretGun::P_ApplyFiringViewJitter(Vector& vAng)
{
    if (m_fCurrViewJitter > 0) {
        vAng[0] += G_CRandom() * m_fCurrViewJitter;
        vAng[1] += G_CRandom() * m_fCurrViewJitter;
        vAng[2] += G_CRandom() * m_fCurrViewJitter * 0.8;

        // decrease the jittering over time
        m_fCurrViewJitter -= level.frametime * 6;
        if (m_fCurrViewJitter < 0) {
            m_fCurrViewJitter = 0;
        }
    }
}

void TurretGun::AI_TurnSpeed(float speed)
{
    m_fTurnSpeed = speed;
}

void TurretGun::AI_PitchSpeed(float speed)
{
    m_fAIPitchSpeed = speed;
}

void TurretGun::PitchCaps(float upcap, float downcap)
{
    m_fPitchUpCap   = upcap;
    m_fPitchDownCap = downcap;

    if (upcap > downcap) {
        m_fPitchUpCap   = -30.0f;
        m_fPitchDownCap = 10.0f;
    }
}

void TurretGun::MaxYawOffset(float max)
{
    m_fMaxYawOffset = max;
    if (max < 0.0f) {
        m_fMaxYawOffset = 0.0f;
    } else if (max > 180.0f) {
        m_fMaxYawOffset = 180.0f;
    }
}

void TurretGun::YawCenter(float center)
{
    m_fStartYaw = center;
}

void TurretGun::P_UserDistance(float dist)
{
    m_fUserDistance = dist;
}

void TurretGun::AI_BurstFireSettings(
    float min_bursttime, float max_bursttime, float min_burstdelay, float max_burstdelay
)
{
    m_fMinBurstTime  = min_bursttime;
    m_fMaxBurstTime  = max_bursttime;
    m_fMinBurstDelay = min_burstdelay;
    m_fMaxBurstDelay = max_burstdelay;
}

void TurretGun::P_EventSetThread(Event *ev)
{
    if (ev->IsFromScript()) {
        m_UseThread.SetThread(ev->GetValue(1));
    } else {
        m_UseThread.Set(ev->GetString(1));
    }
}

void TurretGun::P_SetMaxUseAngle(Event *ev)
{
    m_fMaxUseAngle = ev->GetFloat(1);
}

void TurretGun::P_SetStartYaw(Event *ev)
{
    m_fStartYaw = ev->GetFloat(1);
}

void TurretGun::P_CreateViewModel(void)
{
    char newmodel[MAX_STRING_TOKENS];
    int  tagnum;

    if (!pTurretCameras) {
        pTurretCameras = gi.Cvar_Get("g_turretcameras", "1", 0);
    }

    if (!pTurretCameras->integer) {
        // no camera
        return;
    }

    m_pViewModel = new Animate;

    COM_StripExtension(model.c_str(), newmodel, sizeof(newmodel));
    strcat(newmodel, "_viewmodel.tik");

    m_pViewModel->setScale(edict->s.scale);
    m_pViewModel->detach_at_death = qtrue;
    m_pViewModel->setModel(newmodel);
    m_pViewModel->edict->s.renderfx |= RF_ALWAYSDRAW;

    if (!m_pViewModel->edict->tiki) {
        m_pViewModel->Delete();
        m_pViewModel = NULL;

        warning("CreateViewModel", "Couldn't find turret view model tiki %s", newmodel);
        return;
    }

    // Get the eyes bone
    tagnum = gi.Tag_NumForName(owner->edict->tiki, "eyes bone");
    if (tagnum < 0) {
        warning("CreateViewModel", "Tag eyes bone not found");
    } else if (!m_pViewModel->attach(owner->entnum, tagnum)) {
        warning("CreateViewModel", "Could not attach model %s", newmodel);
        P_DeleteViewModel();
        return;
    }

    m_pViewModel->NewAnim("idle");

    // Make the world model invisible to the owner
    edict->r.svFlags |= SVF_NOTSINGLECLIENT;
    edict->r.singleClient |= owner->edict->s.number;

    // Make the viewmodel visible only to the owner
    m_pViewModel->edict->r.svFlags |= SVF_SINGLECLIENT;
    m_pViewModel->edict->r.singleClient |= owner->edict->s.number;
}

void TurretGun::P_DeleteViewModel(void)
{
    if (!m_pViewModel) {
        return;
    }

    m_pViewModel->Delete();
    m_pViewModel = NULL;

    edict->r.svFlags &= ~SVF_NOTSINGLECLIENT;
}

void TurretGun::Archive(Archiver& arc)
{
    Weapon::Archive(arc);

    arc.ArchiveBool(&m_bFakeBullets);
    arc.ArchiveBool(&m_bPlayerUsable);

    arc.ArchiveFloat(&m_fIdlePitchSpeed);
    arc.ArchiveInteger(&m_iIdleHitCount);
    arc.ArchiveVector(&m_vIdleCheckOffset);
    arc.ArchiveFloat(&m_fMaxIdlePitch);
    arc.ArchiveFloat(&m_fMaxIdleYaw);
    arc.ArchiveVector(&m_vViewOffset);

    arc.ArchiveFloat(&m_fTurnSpeed);
    arc.ArchiveFloat(&m_fAIPitchSpeed);
    arc.ArchiveFloat(&m_fPitchUpCap);
    arc.ArchiveFloat(&m_fPitchDownCap);
    arc.ArchiveFloat(&m_fStartYaw);
    arc.ArchiveFloat(&m_fMaxYawOffset);
    arc.ArchiveFloat(&m_fUserDistance);

    arc.ArchiveFloat(&m_fMinBurstTime);
    arc.ArchiveFloat(&m_fMaxBurstTime);
    arc.ArchiveFloat(&m_fMinBurstDelay);
    arc.ArchiveFloat(&m_fMaxBurstDelay);

    arc.ArchiveFloat(&m_fFireToggleTime);
    arc.ArchiveInteger(&m_iFiring);
    arc.ArchiveVector(&m_vUserViewAng);
    arc.ArchiveSafePointer(&m_pUserCamera);

    arc.ArchiveFloat(&m_fViewJitter);
    arc.ArchiveFloat(&m_fCurrViewJitter);

    arc.ArchiveVector(&m_Aim_offset);
    arc.ArchiveVector(&m_vAIBulletSpread[FIRE_PRIMARY]);
    arc.ArchiveVector(&m_vAIBulletSpread[FIRE_SECONDARY]);
    arc.ArchiveSafePointer(&m_pViewModel);
    arc.ArchiveBool(&m_bUsable);

    m_UseThread.Archive(arc);

    arc.ArchiveBool(&m_bHadOwner);
    arc.ArchiveBool(&m_bRestable);

    arc.ArchiveVector(&m_vUserLastCmdAng);
    arc.ArchiveInteger(&m_iTargetType);
    arc.ArchiveInteger(&m_iAIState);
    arc.ArchiveFloat(&m_fAIConvergeTime);
    arc.ArchiveInteger(&m_iAISuppressTime);
    arc.ArchiveInteger(&m_iAISuppressWaitTime);
    arc.ArchiveInteger(&m_iAILastTrackTime);
    arc.ArchiveInteger(&m_iAIStartSuppressTime);
    arc.ArchiveVec3(m_vDesiredTargetAngles);
    arc.ArchiveFloat(&m_fAIDesiredTargetSpeed);
    arc.ArchiveSafePointer(&m_pAIEnemy);
    arc.ArchiveVec3(m_vAIDesiredTargetPosition);
    arc.ArchiveVec3(m_vAITargetPosition);
    arc.ArchiveVec3(m_vAICurrentTargetPosition);
    arc.ArchiveVec3(m_vAITargetSpeed);
    arc.ArchiveInteger(&m_iAINextSuppressTime);
    arc.ArchiveFloat(&m_fAISuppressWidth);
    arc.ArchiveFloat(&m_fAISuppressHeight);
    arc.ArchiveVec3(m_vMuzzlePosition);
    arc.ArchiveFloat(&m_fMaxUseAngle);

    //
    // Added in 2.30
    //  Clear the last command angles when loading
    //  otherwise it would cause the player to face (0,0,0) when loading
    if (arc.Loading()) {
        m_vUserLastCmdAng = vec_zero;
    }
}

qboolean TurretGun::AI_SetWeaponAnim(const char *anim, Event *ev)
{
    return Weapon::SetWeaponAnim(anim, ev);
}

qboolean TurretGun::P_SetWeaponAnim(const char *anim, Event *ev)
{
    int slot;
    int animnum;

    if (!Weapon::SetWeaponAnim(anim, ev)) {
        return qfalse;
    }

    if (!m_pViewModel) {
        return qtrue;
    }

    slot = (m_iAnimSlot + 3) & 3;

    animnum = gi.Anim_NumForName(m_pViewModel->edict->tiki, anim);
    if (animnum < 0) {
        return qtrue;
    }

    m_pViewModel->StopAnimating(slot);
    m_pViewModel->RestartAnimSlot(slot);

    m_pViewModel->edict->s.frameInfo[slot].index = gi.Anim_NumForName(m_pViewModel->edict->tiki, "idle");

    m_pViewModel->NewAnim(animnum, m_iAnimSlot);
    m_pViewModel->SetOnceType(m_iAnimSlot);
    m_pViewModel->RestartAnimSlot(m_iAnimSlot);

    return qtrue;
}

qboolean TurretGun::SetWeaponAnim(const char *anim, Event *ev)
{
    if (owner && owner->IsSubclassOfPlayer()) {
        return P_SetWeaponAnim(anim, ev);
    } else {
        return AI_SetWeaponAnim(anim, ev);
    }
}

void TurretGun::AI_StopWeaponAnim()
{
    Weapon::StopWeaponAnim();
}

void TurretGun::P_StopWeaponAnim()
{
    if (m_pViewModel) {
        m_pViewModel->RestartAnimSlot(m_iAnimSlot);
        m_pViewModel->StopAnimating(m_iAnimSlot);
        m_pViewModel->StartAnimSlot(m_iAnimSlot, gi.Anim_NumForName(m_pViewModel->edict->tiki, "idle"), 1);
    }

    Weapon::StopWeaponAnim();
}

void TurretGun::StopWeaponAnim(void)
{
    if (owner && owner->IsSubclassOfPlayer()) {
        P_StopWeaponAnim();
    } else {
        AI_StopWeaponAnim();
    }
}

float TurretGun::FireDelay(firemode_t mode)
{
    if (g_target_game == target_game_e::TG_MOH) {
        //
        // Removed in 2.0
        //
        //  On 1.11 and below, the firedelay for players is always 0.06.
        //  Some maps like m1l1 sets the turret firedelay higher for AI
        if (owner && owner->IsSubclassOfPlayer()) {
            return 0.06f;
        }
    }

    return fire_delay[mode];
}

void TurretGun::SetFireDelay(Event *ev)
{
    fire_delay[FIRE_PRIMARY] = ev->GetFloat(1);
}

void TurretGun::ShowInfo(float fDot, float fDist)
{
    if (fDot <= 0.90f && (fDot <= 0.0f || fDist >= 256.0f)) {
        return;
    }

    if (fDist >= 2048.0f || fDist <= 64.0f) {
        return;
    }

    G_DebugString(
        origin + Vector(0, 0, maxs[2] + 56.0f), 1.0f, 1.0f, 1.0f, 1.0f, "%d:%d:%s", entnum, radnum, targetname.c_str()
    );
    G_DebugString(
        origin + Vector(0, 0, maxs[2] + 38.0f),
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        "aim_target: %d",
        aim_target ? aim_target->entnum : -1
    );
    G_DebugString(
        origin + Vector(0, 0, maxs[2] + 20.0f), 1.0f, 1.0f, 1.0f, 1.0f, "owner: %d", owner ? owner->entnum : -1
    );
}

void TurretGun::AI_EventSetTargetType(Event *ev)
{
    int targettype;

    switch (ev->GetConstString(1)) {
    case STRING_DISGUISE_NONE:
        targettype = 0;
        break;
    case STRING_AUTO:
        targettype = 1;
        break;
    default:
        ScriptError("Unknown targettype '%s'", ev->GetString(1).c_str());
        break;
    }

    if (targettype != m_iTargetType) {
        m_iFiring     = TURRETFIRESTATE_NONE;
        m_iTargetType = targettype;
    }
}

void TurretGun::AI_EventGetTargetType(Event *ev)
{
    switch (m_iTargetType) {
    case 0:
        ev->AddConstString(STRING_DISGUISE_NONE);
    case 1:
        ev->AddConstString(STRING_AUTO);
        break;
    default:
        break;
    }
}

void TurretGun::AI_EventSetConvergeTime(Event *ev)
{
    float value = ev->GetFloat(1);
    if (value <= 0) {
        ScriptError("negative value %f not allowed", value);
    }

    m_fAIConvergeTime = value;
}

void TurretGun::AI_EventSetSuppressTime(Event *ev)
{
    float value = ev->GetFloat(1);
    if (value <= 0) {
        ScriptError("negative value %f not allowed", value);
    }

    m_iAISuppressTime = value * 1000;
}

void TurretGun::AI_EventSetSuppressWaitTime(Event *ev)
{
    float value = ev->GetFloat(1);
    if (value <= 0) {
        ScriptError("negative value %f not allowed", value);
    }

    m_iAISuppressWaitTime = value * 1000;
}

void TurretGun::AI_EventSetSuppressWidth(Event *ev)
{
    float value = ev->GetFloat(1);
    if (value <= 0) {
        ScriptError("nonpositive value %f not allowed", value);
    }

    m_fAISuppressWidth = value;
}

void TurretGun::AI_EventSetSuppressHeight(Event *ev)
{
    float value = ev->GetFloat(1);
    if (value <= 0) {
        ScriptError("nonpositive value %f not allowed", value);
    }

    m_fAISuppressHeight = value;
}

void TurretGun::AI_EventSetBulletSpread(Event *ev)
{
    m_vAIBulletSpread[firemodeindex].x = ev->GetFloat(1);
    m_vAIBulletSpread[firemodeindex].y = ev->GetFloat(1);
}

void TurretGun::GetMuzzlePosition(vec3_t position, vec3_t vBarrelPos, vec3_t forward, vec3_t right, vec3_t up)
{
    Vector        delta;
    Vector        aim_angles;
    Sentient     *viewer;
    orientation_t barrel_or;
    vec3_t        weap_axis[3];
    float         mat[3][3];
    int           i;

    viewer = owner;
    if (!viewer && IsSubclassOfVehicleTurretGun()) {
        VehicleTurretGun *vehTurret = static_cast<VehicleTurretGun *>(this);
        viewer                      = vehTurret->GetRemoteOwner();
    }

    if (!viewer) {
        if (forward || right || up) {
            AngleVectors(angles, forward, right, up);
        }

        VectorCopy(origin, position);

        if (GetRawTag(GetTagBarrel(), &barrel_or)) {
            AnglesToAxis(angles, weap_axis);

            for (i = 0; i < 3; i++) {
                VectorMA(position, barrel_or.origin[i], weap_axis[i], position);
            }
        }

        if (vBarrelPos) {
            VectorCopy(position, vBarrelPos);
        }
    } else if (viewer->IsSubclassOfPlayer()) {
        VectorCopy(origin, position);

        if (GetRawTag(GetTagBarrel(), &barrel_or)) {
            AnglesToAxis(angles, weap_axis);

            for (i = 0; i < 3; i++) {
                VectorMA(position, barrel_or.origin[i], weap_axis[i], position);
            }
        }

        if (vBarrelPos) {
            VectorCopy(position, vBarrelPos);
        }

        delta      = viewer->GunTarget(false, position) - position;
        aim_angles = delta.toAngles();

        if (IsSubclassOfVehicleTurretGun()) {
            vec3_t ang;

            MatrixMultiply(barrel_or.axis, weap_axis, mat);
            vectoangles(mat[0], ang);

            for (i = 0; i < 2; i++) {
                float diff;

                diff = AngleSubtract(aim_angles[i], ang[i]);
                if (diff > 90) {
                    aim_angles[i] = ang[i];
                } else if (diff > 20) {
                    aim_angles[i] = ang[i] + 20;
                } else if (diff < -20) {
                    if (diff >= -90) {
                        aim_angles[i] = ang[i] - 20;
                    } else {
                        aim_angles[i] = ang[i];
                    }
                }
            }
        }

        if (forward || right || up) {
            AngleVectors(aim_angles, forward, right, up);
        }
    } else {
        VectorCopy(origin, position);

        if (GetRawTag(GetTagBarrel(), &barrel_or)) {
            AnglesToAxis(angles, weap_axis);

            for (i = 0; i < 3; i++) {
                VectorMA(position, barrel_or.origin[i], weap_axis[i], position);
            }
        }

        if (vBarrelPos) {
            VectorCopy(position, vBarrelPos);
        }

        if (forward || right || up) {
            float spread_x, spread_y;

            AngleVectors(angles, forward, right, up);

            spread_x = crandom() * m_vAIBulletSpread[FIRE_PRIMARY].x / bulletrange[FIRE_PRIMARY];
            spread_y = crandom() * m_vAIBulletSpread[FIRE_PRIMARY].y / bulletrange[FIRE_PRIMARY];

            VectorMA(forward, spread_x, right, forward);
            VectorMA(forward, spread_y, up, forward);
        }
    }
}

Vector TurretGun::EyePosition()
{
    return m_vMuzzlePosition;
}

void TurretGun::setAngles(Vector ang)
{
    Entity::setAngles(ang);

    VectorCopy(ang, m_vDesiredTargetAngles);
}

void TurretGun::CalcFiringViewJitter(void)
{
    if (owner && owner->IsSubclassOfPlayer()) {
        m_fCurrViewJitter = m_fViewJitter;
    }
}

const Vector& TurretGun::GetUserViewAngles() const
{
    return m_vUserViewAng;
}

void TurretGun::SetUserViewAngles(const Vector& vAngles)
{
    m_vUserViewAng = vAngles;
}
