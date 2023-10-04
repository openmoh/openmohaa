/*
===========================================================================
Copyright (C) 2018 the OpenMoHAA team

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
#include "vehicleturret.h"
#include "player.h"
#include "g_phys.h"

CLASS_DECLARATION(DrivableVehicle, VehicleTank, "VehicleTank") {
    {&EV_Damage, &VehicleTank::EventDamage},
    {NULL,       NULL                     }
};

VehicleTank::VehicleTank()
{
    entflags |= FL_ROTATEDBOUNDS;
}

VehicleTank::~VehicleTank()
{
    entflags &= ~FL_ROTATEDBOUNDS;
}

qboolean VehicleTank::Drive(usercmd_t *ucmd)
{
    Vector i;
    Vector j;
    Vector k;
    Vector vTmp;

    vTmp = velocity;
    vTmp.normalize();

    if (!driver.ent || !driver.ent->isClient()) {
        return qfalse;
    }

    if (!drivable) {
        driver.ent->client->ps.pm_flags |= PMF_FROZEN;
        ucmd->forwardmove = 0;
        ucmd->rightmove   = 0;
        ucmd->upmove      = 0;
        return qfalse;
    }

    if (!m_bAutoPilot) {
        //
        // ignore client's control if the vehicle is in auto-pilot mode
        //
        moveimpulse = ucmd->forwardmove * (vTmp.length() + 1.0);
        m_bIsBraking = ucmd->forwardmove < 0;
        m_fAccelerator += ucmd->forwardmove * 0.005;
        // acceleration must be an alpha value between [0, 1]
        m_fAccelerator = Q_clamp_float(m_fAccelerator, 0, 1);

        turnimpulse = -ucmd->rightmove;
        jumpimpulse = ucmd->upmove;
    }

    VehicleTurretGun *vtg = (VehicleTurretGun *)Turrets[0].ent.Pointer();

    if (vtg && vtg->IsSubclassOfVehicleTurretGun() && driver.ent->IsSubclassOfSentient()) {
        vtg->RemoteControl(ucmd, (Sentient *)driver.ent.Pointer());

        if (ucmd->upmove) {
            turnimpulse -= AngleSubtract(angles[1], vtg->angles[1]);
        }
    }
    return qtrue;
}

void VehicleTank::EventDamage(Event *ev)
{
    int mod;
    mod = ev->GetInteger(9);

    if (g_gametype->integer == GT_TOW && !dmManager.RoundActive()) {
        return;
    }

    switch (mod) {
    case MOD_BULLET:
    case MOD_BASH:
    case MOD_FAST_BULLET:
    case MOD_VEHICLE:
    case MOD_SHOTGUN:
    case MOD_GRENADE:
        return;
    }

    Vehicle::EventDamage(ev);

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        Player *p = (Player *)driver.ent.Pointer();
        if (p) {
            if (p->IsSubclassOfPlayer()) {
                p->m_iNumHitsTaken++;
            }
        }
    }
}

void VehicleTank::Think()
{
    flags |= FL_POSTTHINK;
}

void VehicleTank::Postthink()
{
    float        turn;
    Vector       i, j, k;
    Vector       normalsum;
    Vector       temp;
    Vector       pitch, roll;
    Vector       acceleration;
    Vector       atmp, atmp2, aup;
    VehicleBase *v, *last;
    float        drivespeed;
    Vector       primal_angles, primal_origin;
    Vector       vTmp;
    Vector       vAddedAngles;
    Vector       n_angles;
    float        dot;
    Vector       newvel, flatvel;
    Vector       anglediff;

    primal_angles = angles;
    primal_origin = origin;

    if (!g_vehicle->integer || edict->s.renderfx & RF_DONTDRAW) {
        // don't process physics
        return;
    }

    if (m_pCollisionEntity) {
        // use the collision entity for collision
        setSolidType(SOLID_NOT);
        m_pCollisionEntity->Solid();
    }

    prev_velocity = velocity;
    SetSlotsNonSolid();

    if (m_bAnimMove) {
        AnimMoveVehicle();
    } else {
        if (!m_bMovementLocked) {
            FactorOutAnglesOffset();
            FactorOutOriginOffset();
        }

        MoveVehicle();
    }

    SetSlotsSolid();

    if (m_vOldMins != vec_zero && m_vOldMaxs != vec_zero) {
        mins = m_vOldMins;
        maxs = m_vOldMaxs;
    } else if (mins != vec_zero && maxs != vec_zero) {
        m_vOldMins = mins;
        m_vOldMaxs = maxs;
    }

    if (m_bAnimMove) {
        moveimpulse = velocity.length() * level.frametime;
        turnimpulse = avelocity[1] * level.frametime;
    } else if (m_bAutoPilot) {
        AutoPilot();
    }

    currentspeed = moveimpulse;
    turnangle    = turnangle * 0.25 + turnimpulse;
    turnangle    = Q_clamp_float(turnangle, -maxturnrate, maxturnrate);

    real_velocity     = origin - prev_origin;
    prev_origin       = origin;
    prev_acceleration = real_acceleration;
    real_acceleration = real_velocity - prev_velocity;
    prev_velocity     = real_velocity;
    acceleration      = real_acceleration - prev_acceleration;

    UpdateBones();
    UpdateShaderOffset();
    UpdateTires();
    UpdateNormals();

    temp = angles;
    temp.AngleVectorsLeft(&i, &j, &k);

    velocity[0] *= 0.925f;
    velocity[1] *= 0.925f;
    turn = turnangle / 200.0;

    flatvel = orientation[0];
    velocity += flatvel * currentspeed;
    flatvel[2] = 0;

    dot         = velocity * flatvel;
    dot         = Q_clamp_float(dot, -speed, speed);
    newvel      = flatvel * dot;
    velocity[0] = newvel[0];
    velocity[1] = newvel[1];
    avelocity *= 0.05f;
    if (dot < 350) {
        dot = 350;
    }

    avelocity.y += turn * dot;
    angles += avelocity * level.frametime;
    setAngles(angles);

    if (Turrets[0].ent && Turrets[0].ent->IsSubclassOfVehicleTurretGun() && driver.ent
        && driver.ent->IsSubclassOfSentient()) {
        VehicleTurretGun *pTurret = static_cast<VehicleTurretGun *>(Turrets[0].ent.Pointer());
        if (jumpimpulse != 0) {
            pTurret->SetUserViewAngles(pTurret->GetUserViewAngles() - Vector(0, avelocity[1] * level.frametime, 0));
        }
    }

    drivespeed = acceleration * orientation[0];
    UpdateSound();

    if (AssertRotation(angles, primal_angles) != true) {
        angles = primal_angles;
    }

    CalculateAnglesOffset(acceleration);
    CalculateOriginOffset();

    last_origin = origin;
    anglediff   = (angles - primal_angles) * level.frametime;

    if (anglediff.x > 180 || anglediff.x < -180) anglediff.x = 0;
    if (anglediff.y > 180 || anglediff.y < -180) anglediff.y = 0;
    if (anglediff.z > 180 || anglediff.z < -180) anglediff.z = 0;

    if (anglediff.x > -1 || anglediff.x < 1) anglediff.x = 0;
    if (anglediff.y > -1 || anglediff.y < 1) anglediff.y = 0;
    if (anglediff.z > -1 || anglediff.z < 1) anglediff.z = 0;

    avelocity = anglediff;

    if (!m_bAnimMove && !m_bMovementLocked) {
        FactorInOriginOffset();
        FactorInAnglesOffset(&vAddedAngles);
    }

    CalculateAnimationData(vAddedAngles, m_vOriginOffset + m_vOriginOffset2);
    if (m_pCollisionEntity) {
        Vector vaDelta;

        SetSlotsNonSolid();

        vaDelta[0] = angledist(angles[0] - m_pCollisionEntity->angles[0]);
        vaDelta[1] = angledist(angles[1] - m_pCollisionEntity->angles[1]);
        vaDelta[2] = angledist(angles[2] - m_pCollisionEntity->angles[2]);

        m_pCollisionEntity->Solid();

        G_PushMove(m_pCollisionEntity, origin - primal_origin, vaDelta);
        G_TouchTriggers(m_pCollisionEntity);

        SetSlotsSolid();

        m_pCollisionEntity->setOrigin(origin);
        m_pCollisionEntity->setAngles(angles);
    }

    SetupVehicleSoundEntities();
    i = orientation[0];
    j = orientation[1];
    k = orientation[2];

    //
    // update slots
    //
    for (int ii = 0; ii < MAX_PASSENGERS; ii++) {
        UpdatePassengerSlot(ii);
    }

    for (int ii = 0; ii < MAX_TURRETS; ii++) {
        UpdateTurretSlot(ii);
    }

    for (last = this; last->vlink; last = v) {
        v = last->vlink;

        setOrigin(origin + i * v->offset.x + j * v->offset.y + k * v->offset.z);
        v->avelocity = avelocity;
        v->velocity  = velocity;

        v->angles[ROLL]  = angles[ROLL];
        v->angles[YAW]   = angles[YAW];
        v->angles[PITCH] = (int)(drivespeed / 4.f + v->angles[PITCH]) % 360;
        v->setAngles(v->angles);
    }

    CheckWater();
    WorldEffects();

    if (m_pCollisionEntity) {
        setSolidType(SOLID_NOT);
        m_pCollisionEntity->Solid();
    } else {
        setSolidType(SOLID_BBOX);
        setContents(CONTENTS_UNKNOWN2);
    }

    if (driver.flags & SLOT_BUSY && driver.ent) {
        if (driver.ent->IsSubclassOfPlayer()) {
            driver.ent->takedamage = DAMAGE_YES;
            setSolidType(SOLID_NOT);
        } else {
            driver.ent->takedamage = DAMAGE_NO;
            driver.ent->setSolidType(SOLID_NOT);
        }
    }
}

void VehicleTank::CalculateOriginOffset()
{
    int    index;
    Vector vTireAvg;
    Vector vMissHit;
    Vector temp;
    int    iNum = 0;
    Vector acceleration;
    Vector oldoffset;

    oldoffset = m_vOriginOffset;
    m_vOriginOffset += m_vOriginOffset2;
    m_vOriginOffset2 = vec_zero;

    for (index = 0; index < NUM_VEHICLE_TIRES; index++) {
        if (m_bTireHit[index]) {
            vTireAvg += origin - m_vTireEnd[index];
            iNum++;
        } else {
            vMissHit = Corners[index];
        }
    }

    if (m_iNumNormals > 0 && iNum == NUM_VEHICLE_TIRES - 1) {
        Vector point;

        temp = m_vNormalSum * (1.f / m_iNumNormals);
        temp.normalize();
        ProjectPointOnPlane(point, vMissHit, temp);
        vTireAvg += point;

        iNum++;
    }

    if (iNum == NUM_VEHICLE_TIRES) {
        vec3_t vOriginCornerOffset;

        vTireAvg *= 1.f / NUM_VEHICLE_TIRES;
        MatrixTransformVector(m_vOriginCornerOffset, orientation, vOriginCornerOffset);
        vTireAvg -= vOriginCornerOffset;
        m_vOriginOffset2 += vTireAvg;
    }

    //m_vOriginOffset2 += vec_zero;
    m_vOriginOffset -= m_vOriginOffset2;

    if (!isfinite(m_vOriginOffset[0]) || !isfinite(m_vOriginOffset[1]) || !isfinite(m_vOriginOffset[2])) {
        m_vOriginOffset = oldoffset;
    }
}

void VehicleTank::UpdateSound()
{
    float pitch  = 1.f;
    float volume = 1.f;

    if (level.time < m_fNextSoundState) {
        return;
    }

    pitch = (velocity.length() - m_fSoundMinSpeed) / (m_fSoundMaxSpeed - m_fSoundMinSpeed);
    pitch = Q_clamp_float(pitch, 0, 1);
    pitch = (m_fSoundMaxPitch - m_fSoundMinPitch) * pitch + m_fSoundMinPitch;

    volume = (velocity.length() - m_fVolumeMinSpeed) / (m_fVolumeMaxSpeed - m_fVolumeMinSpeed);
    volume = Q_clamp_float(volume, 0, 1);
    volume = (m_fVolumeMaxPitch - m_fVolumeMinPitch) * volume + m_fVolumeMinPitch;

    if (level.intermissiontime) {
        m_eSoundState = ST_OFF;
    }

    switch (m_eSoundState) {
    case ST_OFF:
        StopLoopSound();
        TurnOffVehicleSoundEntities();
        m_fNextSoundState = level.time;
        if (driver.ent || m_bAutoPilot) {
            m_eSoundState = ST_OFF_TRANS_IDLE;
        }
        break;
    case ST_OFF_TRANS_IDLE:
        m_fNextSoundState = level.time;
        m_eSoundState     = ST_IDLE;
        Sound(m_sSoundSet + "snd_on", CHAN_AUTO);
        LoopSound(m_sSoundSet + "snd_idle");
        break;
    case ST_IDLE_TRANS_OFF:
        m_fNextSoundState = level.time;
        m_eSoundState     = ST_OFF;
        Sound(m_sSoundSet + "snd_off", CHAN_AUTO);
        StopLoopSound();
        break;
    case ST_IDLE:
        m_fNextSoundState = level.time;
        if (driver.ent || m_bAutoPilot) {
            if (fabs(velocity * orientation[0]) > 150) {
                m_eSoundState = ST_IDLE_TRANS_RUN;
            } else if (fabs(turnimpulse) > 1) {
                m_eSoundState = ST_IDLE_TRANS_TURN;
            }
        } else {
            m_eSoundState = ST_IDLE_TRANS_OFF;
        }

        LoopSound(m_sSoundSet + "snd_idle");
        break;
    case ST_IDLE_TRANS_RUN:
        m_fNextSoundState = level.time;
        m_eSoundState     = ST_RUN;
        Sound(m_sSoundSet + "snd_revup", CHAN_AUTO);
        Sound(m_sSoundSet + "snd_shift", CHAN_AUTO);
        LoopSound(m_sSoundSet + "snd_run", volume, -1, -1, pitch);
        break;
    case ST_RUN:
        m_fNextSoundState = level.time;
        if (fabs(velocity * orientation[0]) < 150) {
            m_eSoundState = ST_RUN_TRANS_IDLE;
        }
        TurnOnVehicleSoundEntities();
        LoopSound(m_sSoundSet + "snd_run", volume, -1, -1, pitch);
        break;
    case ST_RUN_TRANS_IDLE:
    case ST_TURN_TRANS_IDLE:
        m_fNextSoundState = level.time;
        m_eSoundState     = ST_IDLE;
        Sound(m_sSoundSet + "snd_revdown", CHAN_AUTO);
        LoopSound(m_sSoundSet + "snd_idle");
        break;
    case ST_IDLE_TRANS_TURN:
        m_fNextSoundState = level.time;
        m_eSoundState     = ST_TURN;
        Sound(m_sSoundSet + "snd_revup", CHAN_AUTO);
        Sound(m_sSoundSet + "snd_shift", CHAN_AUTO);
        LoopSound(m_sSoundSet + "snd_run", volume, -1, -1, pitch);
        break;
    case ST_TURN:
        m_fNextSoundState = level.time;
        if (fabs(turnimpulse) < 1) {
            m_eSoundState = ST_TURN_TRANS_IDLE;
        }
        LoopSound(m_sSoundSet + "snd_run", volume, -1, -1, pitch);
        break;
    default:
        m_fNextSoundState = level.time;
        m_eSoundState     = ST_OFF;
        break;
    }
}

void VehicleTank::AttachDriverSlot(Event *ev)
{
    Vehicle::AttachDriverSlot(ev);

    if (!driver.ent) {
        //
        // Added in OPM.
        // original mohaa doesn't check if driver.ent is valid, which can cause crash if the attached entity is null.
        // on single-player, it would crash while loading m5l2a when sv_maxclients is above 1.
        // The reason of the crash is that attachdriverslot is called with $player which can be NULL on prespawn.
        // $player is not created on prespawn when sv_maxclients is above 1
        //
        return;
    }

    VehicleTurretGun *vtg = (VehicleTurretGun *)Turrets[0].ent.Pointer();

    if (vtg && vtg->IsSubclassOfVehicleTurretGun() && driver.ent->IsSubclassOfSentient()) {
        vtg->SetRemoteOwner(static_cast<Sentient *>(driver.ent.Pointer()));
        Think();
        Postthink();
        vtg->Think();
    }
}

void VehicleTank::DriverAdded()
{
    int i;

    for (i = 0; i < MAX_TURRETS; i++) {
        if (Turrets[i].flags & SLOT_BUSY && Turrets[i].ent && Turrets[i].ent->IsSubclassOfVehicleTurretGun()) {
            VehicleTurretGun *pTurret = static_cast<VehicleTurretGun *>(Turrets[i].ent.Pointer());
            pTurret->TurretHasBeenMounted();
        }
    }
}
