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

#include "portableturret.h"
#include "g_spawn.h"
#include "g_phys.h"
#include "player.h"

Event EV_CarryableTurret_Drop
(
    "dropturret",
    EV_DEFAULT,
    NULL,
    NULL,
    "Makes the carryable turret have itself dropped",
    EV_NORMAL
);
Event EV_CarryableTurret_TurretTik
(
    "turrettik",
    EV_DEFAULT,
    NULL,
    NULL,
    "Tiki of the turret to place",
    EV_NORMAL
);
Event EV_CarryableTurret_Place
(
    "placeturret",
    EV_DEFAULT,
    NULL,
    NULL,
    "Try to place the carryable turret",
    EV_NORMAL
);
Event EV_CarryableTurret_EnablePickup
(
    "_internalenablepickup",
    EV_DEFAULT,
    NULL,
    NULL,
    NULL,
    EV_NORMAL
);

CLASS_DECLARATION(InventoryItem, CarryableTurret, NULL) {
    {&EV_Touch,                        NULL                               },
    {&EV_Item_Pickup,                  &CarryableTurret::EventPickup      },
    {&EV_CarryableTurret_Drop,         &CarryableTurret::DropTurret       },
    {&EV_CarryableTurret_TurretTik,    &CarryableTurret::EventTurretTik   },
    {&EV_CarryableTurret_Place,        &CarryableTurret::EventPlaceTurret },
    {&EV_CarryableTurret_EnablePickup, &CarryableTurret::EventEnablePickup},
    {NULL,                             NULL                               }
};

CarryableTurret::CarryableTurret()
{
    if (LoadingSavegame) {
        return;
    }

    if (spawnflags & 1) {
        Event *ev = new Event(EV_CarryableTurret_Place);
        ev->AddInteger(1);

        PostEvent(ev, EV_POSTSPAWN);
    }

    isPickable = true;
}

void CarryableTurret::EventPickup(Event *ev)
{
    Entity   *ent;
    Sentient *sent;
    Item     *item;
    Weapon   *activeWeap;

    if (!isPickable) {
        return;
    }

    ent = ev->GetEntity(1);
    if (!Pickupable(ent)) {
        // can't be picked up by the entity
        return;
    }

    sent = static_cast<Sentient *>(ent);
    if (sent->HasItem(item_name)) {
        // already in possession of the carryable turret
        return;
    }

    item = ItemPickup(sent);
    if (!item) {
        // couldn't pick it up
        return;
    }

    if (!item->GetOwner()) {
        // invalid owner?!
        return;
    }

    activeWeap = item->GetOwner()->GetActiveWeapon(WEAPON_MAIN);
    if (activeWeap && activeWeap != item) {
        activeWeap->DetachFromOwner();
    }

    // make the sentient use the carryable turret
    item->GetOwner()->ChangeWeapon(static_cast<Weapon *>(item), WEAPON_MAIN);
}

qboolean CarryableTurret::Removable(void)
{
    return qtrue;
}

void CarryableTurret::DetachFromOwner(void)
{
    Weapon::DetachFromOwner();

    PostEvent(EV_CarryableTurret_Drop, 0);
}

void CarryableTurret::AttachToHolster(weaponhand_t hand)
{
    Weapon::AttachToHolster(hand);

    PostEvent(EV_CarryableTurret_Drop, 0);
}

void CarryableTurret::DropTurret(Event *ev)
{
    Weapon   *activeWeap;
    Sentient *currentOwner;

    if (!owner) {
        return;
    }

    currentOwner = owner;
    activeWeap   = currentOwner->GetActiveWeapon(WEAPON_MAIN);
    Drop();
    isPickable = false;

    PostEvent(EV_CarryableTurret_EnablePickup, 1.5f);

    if (activeWeap == this) {
        currentOwner->edict->s.eFlags |= EF_UNARMED;
    }
}

qboolean CarryableTurret::Drop(void)
{
    qboolean result = Weapon::Drop();
    if (result) {
        spawnflags &= ~SPAWNFLAG_NOPC;
        CancelEventsOfType(EV_Remove);
    }

    return result;
}

void CarryableTurret::EventTurretTik(Event *ev)
{
    turretModel = ev->GetString(1);
}

void CarryableTurret::EventPlaceTurret(Event *ev)
{
    Event  *newev;
    Player *ownerP;
    Entity *placer;
    Vector  turretAngles, targetPos;
    Vector  forward, right, up;
    Vector  endpos, endpos2;
    Vector  start, end;
    Vector  mins, maxs;
    trace_t trace;
    float   groundAngle;
    bool    autoPlace = false;

    if (ev->NumArgs() && ev->GetInteger(1)) {
        autoPlace = true;

        AngleVectors(angles, forward, NULL, NULL);
        mins  = Vector(-32, -32, 0);
        maxs  = Vector(32, 32, 32);
        start = origin;
        end   = origin - forward * 55;

        trace =
            G_Trace(start, mins, maxs, end, this, MASK_CARRYABLETURRET, qtrue, "CarryableTurret::EventPlaceTurret1");

        if (trace.startsolid || trace.fraction < 1 || trace.ent) {
            gi.DPrintf(
                "~*~*~ Not enough room to auto place protable turret at (%i %i %i)\n",
                (int)origin[0],
                (int)origin[1],
                (int)origin[2]
            );
            angles = Vector(-90, 0, 0);
            setAngles(angles);
            return;
        }

        mins  = Vector(-16, -16, 0);
        maxs  = Vector(16, 16, 8);
        start = origin - forward * 55;
        end   = start;
        end[2] -= 128;

        trace =
            G_Trace(start, mins, maxs, end, this, MASK_CARRYABLETURRET, qtrue, "CarryableTurret::EventPlaceTurret1");

        mins  = Vector(-16, -16, 0);
        maxs  = Vector(16, 16, 95);
        start = trace.endpos;

        trace =
            G_Trace(start, mins, maxs, end, this, MASK_CARRYABLETURRET, qtrue, "CarryableTurret::EventPlaceTurret1");

        if (trace.startsolid || trace.fraction < 1) {
            gi.DPrintf(
                "~*~*~ Not enough room to auto place protable turret at (%i %i %i)\n",
                (int)origin[0],
                (int)origin[1],
                (int)origin[2]
            );
            angles = Vector(-90, 0, 0);
            setAngles(angles);
            return;
        }

        setOrigin(start);
        angles[0] = 0;
        angles[2] = 0;
        setAngles(angles);
        placer = this;
    } else {
        if (!owner) {
            return;
        }

        placer = owner;

        if (!placer->groundentity) {
            return;
        }

        if (placer->IsSubclassOfPlayer()) {
            ownerP = static_cast<Player *>(placer);
            if (ownerP->client->ps.pm_flags & (PMF_NO_GRAVITY | PMF_VIEW_JUMP_START | PMF_VIEW_DUCK_RUN)) {
                if (level.time > next_noammo_time) {
                    Sound(m_NoAmmoSound);
                }

                next_noammo_time = FireDelay(FIRE_PRIMARY) + level.time + level.frametime;
                ownerP->HUDPrint(va("%s\n", gi.LV_ConvertString("You cannot place a turret right now.")));
                return;
            } else if (ownerP->client->ps.pm_flags & PMF_DUCKED) {
                mins  = Vector(-16, -16, 0);
                maxs  = Vector(16, 16, 95);
                start = placer->origin;

                trace = G_Trace(
                    start, mins, maxs, end, owner, MASK_PLAYERSOLID, qtrue, "CarryableTurret::EventPlaceTurret1"
                );

                if (trace.startsolid || trace.fraction < 1) {
                    if (level.time > next_noammo_time) {
                        Sound(m_NoAmmoSound);
                    }

                    next_noammo_time = FireDelay(FIRE_PRIMARY) + level.time + level.frametime;
                    ownerP->HUDPrint(va("%s\n", gi.LV_ConvertString("Not enough room to place the turret.")));
                    return;
                }
            }
        }
    }

    turretAngles[0] = 0;
    turretAngles[1] = placer->angles[1];
    turretAngles[2] = 0;
    AngleVectors(turretAngles, forward, right, NULL);

    targetPos = placer->origin + forward * 55 + Vector(0, 0, 65.43f);
    mins      = Vector(-32, -32, -16);
    maxs      = Vector(32, 32, 8);

    trace = G_Trace(
        targetPos, mins, maxs, targetPos, placer, MASK_CARRYABLETURRET, qtrue, "CarryableTurret::EventPlaceTurret1"
    );
    if (trace.startsolid || trace.fraction < 1 || trace.ent) {
        if (level.time > next_noammo_time) {
            Sound(m_NoAmmoSound);
        }

        next_noammo_time = FireDelay(FIRE_PRIMARY) + level.time + level.frametime;
        if (autoPlace) {
            gi.DPrintf("Not enough room to place the turret.\n");
        } else if (placer->IsSubclassOfPlayer()) {
            ownerP->HUDPrint(va("%s\n", gi.LV_ConvertString("Not enough room to place the turret.")));
        }
        return;
    }

    mins  = Vector(-4, -4, 0);
    maxs  = Vector(4, 4, 4);
    start = placer->origin + Vector(0, 0, 16) + right * 12;
    end   = start + Vector(0, 0, -128);

    trace = G_Trace(
        start, mins, maxs, end, placer, CONTENTS_FENCE | CONTENTS_SOLID, qtrue, "CarryableTurret::EventPlaceTurret2"
    );
    endpos = trace.endpos;

    start = placer->origin + Vector(0, 0, 16) - right * 12;
    end   = start + Vector(0, 0, -128);

    trace = G_Trace(
        start, mins, maxs, end, placer, CONTENTS_FENCE | CONTENTS_SOLID, qtrue, "CarryableTurret::EventPlaceTurret2"
    );
    endpos2 = trace.endpos;

    groundAngle = RAD2DEG(atan2(endpos[2] - endpos2[2], 12));
    if (fabs(groundAngle) > 40) {
        if (level.time > next_noammo_time) {
            Sound(m_NoAmmoSound);
        }

        next_noammo_time = FireDelay(FIRE_PRIMARY) + level.time + level.frametime;
        if (autoPlace) {
            gi.DPrintf(
                "Ground is too uneven to place the turret. (Too much ground roll to the side. Ground is at %g, limit "
                "is %g)\n",
                fabs(groundAngle),
                40.f
            );
        } else if (placer->IsSubclassOfPlayer()) {
            ownerP = static_cast<Player *>(placer);
            ownerP->HUDPrint(va("%s", gi.LV_ConvertString("Ground is too uneven to place the turret.")));
        }
        return;
    }
    mins  = Vector(-2, -2, 0);
    maxs  = Vector(2, 2, 4);
    start = targetPos + forward * 13;
    end   = start + Vector(0, 0, -129.43f);

    trace = G_Trace(
        start, mins, maxs, end, placer, CONTENTS_FENCE | CONTENTS_SOLID, qtrue, "CarryableTurret::EventPlaceTurret2"
    );

    groundAngle = RAD2DEG(atan2(placer->origin[2] - trace.endpos[2], 55));
    if (fabs(groundAngle) > 30) {
        if (level.time > next_noammo_time) {
            Sound(m_NoAmmoSound);
        }

        next_noammo_time = FireDelay(FIRE_PRIMARY) + level.time + level.frametime;
        if (autoPlace) {
            gi.DPrintf(
                "Ground is too uneven to place the turret. (Too much ground pitch. Ground is at %g, limit is %g)\n",
                fabs(groundAngle),
                30.f
            );
        } else if (placer->IsSubclassOfPlayer()) {
            Player *ownerP = static_cast<Player *>(placer);

            ownerP->HUDPrint(va("%s", gi.LV_ConvertString("Ground is too uneven to place the turret.")));
        }
        return;
    }

    turretAngles[0] = groundAngle;
    AngleVectors(turretAngles, forward, NULL, up);

    targetPos = placer->origin + forward * 55 + up * 65.43f;
    if (!turretModel.length()) {
        gi.DPrintf("WARNING: Carryable turret item '%s' doesn't have a turrettik specified\n", model.c_str());
        return;
    }

    ClassDef       *c;
    PortableTurret *ent;
    SpawnArgs       spawnArgs;

    spawnArgs.setArg("model", turretModel);
    c = spawnArgs.getClassDef();
    if (!c || !checkInheritance(Entity::classinfostatic(), c)) {
        gi.DPrintf(
            "WARNING: Carryable turret item '%s' can't find turrettik '%s'\n", model.c_str(), turretModel.c_str()
        );
        return;
    }

    if (c != PortableTurret::classinfostatic()) {
        gi.DPrintf(
            "WARNING: Carryable turret item '%s' set to invalid turrettik '%s'\n", model.c_str(), turretModel.c_str()
        );
        return;
    }

    ent = static_cast<PortableTurret *>(c->newInstance());
    ent->setModel(turretModel);
    ent->setOrigin(placer->origin + forward * 32 + up * 65.43f);
    ent->setAngles(turretAngles);
    ent->ProcessInitCommands();
    ent->SetGroundPitch(turretAngles[0]);
    ent->SetOwnerPosition(placer->origin);
    ent->CancelEventsOfType(EV_Item_DropToFloor);

    if (autoPlace) {
        newev = new Event(EV_Item_DropToFloor, 1);
        newev->AddInteger(1);
        ent->ProcessEvent(newev);

        ent->setOrigin(vec_zero);
        PostEvent(EV_Remove, level.frametime);
    } else {
        ent->ProcessEvent(EV_Item_DropToFloor);

        DetachGun();
        RemoveFromOwner();
        setOrigin(vec_zero);
        PostEvent(EV_Remove, level.frametime);
        placer->velocity = vec_zero;

        newev = new Event(EV_Use, 1);
        newev->AddEntity(placer);
        ent->ProcessEvent(newev);
    }
}

void CarryableTurret::EventEnablePickup(Event *ev)
{
    isPickable = true;
}

bool CarryableTurret::IsCarryableTurret() const
{
    return true;
}

void CarryableTurret::Archive(Archiver& arc)
{
    Weapon::Archive(arc);

    arc.ArchiveString(&turretModel);
    arc.ArchiveBoolean(&isPickable);
}

CLASS_DECLARATION(TurretGun, PortableTurret, NULL) {
    {&EV_Item_DropToFloor,       &PortableTurret::PortablePlaceTurret},
    {&EV_Weapon_SetAmmoClipSize, &PortableTurret::EventSetClipSize   },
    {NULL,                       NULL                                }
};

PortableTurret::PortableTurret()
{
    groundPitch       = 0.0;
    nextUsableTime    = 0.0;
    packingUp         = 0;
    mustDetach        = 0;
    ammo_in_clip[0]   = 0;
    ammo_clip_size[0] = 0;
    endReloadTime     = 0.0;
}

PortableTurret::~PortableTurret()
{
    if (baseEntity) {
        baseEntity->ProcessEvent(EV_Remove);
        baseEntity = NULL;
    }

    TurretGun::RemoveUserCamera();
}

void PortableTurret::SetGroundPitch(float pitch)
{
    groundPitch = pitch;
}

void PortableTurret::SetOwnerPosition(const Vector& pos)
{
    ownerPosition = pos + Vector(0, 0, 16);
}

void PortableTurret::PortablePlaceTurret(Event *ev)
{
    Vector turretAngles;
    Vector forward, up;
    bool   attachPlayer = false;
    char   baseModel[1024];

    PlaceTurret(ev);
    if (ev->NumArgs() && ev->GetInteger(1)) {
        attachPlayer = true;
    }

    m_fPitchUpCap += groundPitch;
    m_fPitchDownCap += groundPitch;
    m_fMaxIdlePitch += groundPitch;

    m_pUserCamera = new Camera();
    currentModel  = model;

    COM_StripExtension(model.c_str(), baseModel, sizeof(baseModel));
    strcat(baseModel, "_setup.tik");
    setupModel = CanonicalTikiName(baseModel);

    turretAngles[0] = groundPitch;
    turretAngles[1] = m_fStartYaw;
    turretAngles[2] = 0;
    AngleVectors(turretAngles, forward, NULL, up);

    cameraOrigin = origin + (forward * m_vViewOffset[0]) + (up * m_vViewOffset[2]);

    if (attachPlayer) {
        nextUsableTime = 0;
        packingUp      = false;
        mustDetach     = false;

        m_pUserCamera->setOrigin(origin);
        m_pUserCamera->setAngles(angles);

        m_pUserCamera->SetPositionOffset(m_vViewOffset);
        MakeBaseEntity();

        endReloadTime     = 0;
        owner             = NULL;
        edict->r.ownerNum = ENTITYNUM_NONE;

        m_fIdlePitchSpeed = 0;
        m_iIdleHitCount   = 0;
        m_iFiring         = 0;

        StopWeaponAnim();
        StopWeaponAnim();
        StopWeaponAnim();
        StopWeaponAnim();

        ForceIdle();
    } else {
        nextUsableTime = level.time + 2;
        packingUp      = false;

        StopWeaponAnim();
        StopWeaponAnim();
        StopWeaponAnim();
        StopWeaponAnim();

        model = setupModel;
        if (setModel()) {
            ForceIdle();
            P_DeleteViewModel();
            mustDetach = true;

            m_pUserCamera->setOrigin(cameraOrigin);
            m_pUserCamera->setAngles(turretAngles);
            m_pUserCamera->SetPositionOffset(vec_zero);
        } else {
            Com_Printf("^~^~^PortableTurret::PortablePlaceTurret: Bad model name '%s'\n", setupModel.c_str());
        }
    }
}

void PortableTurret::MakeBaseEntity()
{
    char   baseModel[1024];
    Vector turretAngles;

    if (!baseEntity) {
        COM_StripExtension(model, baseModel, sizeof(baseModel));
        strcat(baseModel, "_base.tik");
        baseEntity = new Entity();
    }

    baseEntity->takedamage = DAMAGE_NO;
    baseEntity->setModel(baseModel);
    baseEntity->setMoveType(MOVETYPE_NONE);
    baseEntity->setSolidType(SOLID_NOT);

    baseEntity->setSize(Vector(4, 4, 4), Vector(-4, -4, -50));
    baseEntity->setOrigin(origin);

    turretAngles[0] = groundPitch;
    turretAngles[1] = angles[1];
    turretAngles[2] = 0;
    baseEntity->setAngles(turretAngles);
}

void PortableTurret::EventSetClipSize(Event *ev)
{
    ammo_clip_size[FIRE_PRIMARY] = ev->GetFloat(1);
    ammo_in_clip[FIRE_PRIMARY]   = ammo_clip_size[FIRE_PRIMARY];
}

void PortableTurret::AbortTurretSetup()
{
    Sentient *currentOwner;
    bool      isPackingUp = packingUp;

    currentOwner   = owner;
    nextUsableTime = 0;
    packingUp      = false;
    P_TurretEndUsed();

    m_iFiring = 0;

    if (isPackingUp) {
        StopWeaponAnim();
        StopWeaponAnim();
        StopWeaponAnim();
        StopWeaponAnim();

        model = currentModel;
        if (setModel()) {
            ForceIdle();
            MakeBaseEntity();
        } else {
            Com_Printf("^~^~^PortableTurret::P_ThinkActive: Bad model name '%s'\n", currentModel.c_str());
        }
    } else {
        Item *item;

        item = currentOwner->giveItem("weapons/mg42carryable.tik");
        if (item) {
            item->Drop();
        } else {
            gi.DPrintf("couldn't spawn carryable turret for player\n");
        }

        RemoveUserCamera();
        hideModel();
        setOrigin(vec_zero);

        PostEvent(EV_Remove, level.frametime);

        if (baseEntity) {
            baseEntity->hideModel();
            baseEntity->setOrigin(vec_zero);
            baseEntity->PostEvent(EV_Remove, level.frametime);
            baseEntity = NULL;
        }
    }
}

void PortableTurret::P_UserAim(usercmd_t *cmd)
{
    bool doDetach = false;

    if (cmd->buttons & BUTTON_ATTACKRIGHT) {
        if (!mustDetach) {
            doDetach = true;
        }

        mustDetach = true;
    } else {
        mustDetach = false;
    }

    if (nextUsableTime > level.time) {
        angles[0] = groundPitch;
        angles[1] = m_fStartYaw;

        setAngles(angles);

        TurretGun::P_UserAim(cmd);
        m_iFiring = 0;
        flags |= FL_THINK;
    } else if (doDetach) {
        nextUsableTime = level.time + 2;
        packingUp      = true;

        StopWeaponAnim();
        StopWeaponAnim();
        StopWeaponAnim();
        StopWeaponAnim();

        model = setupModel;

        if (setModel()) {
            SetWeaponAnim("packup");

            P_DeleteViewModel();

            if (baseEntity) {
                baseEntity->ProcessEvent(EV_Remove);
                baseEntity = NULL;
            }
        } else {
            Com_Printf("^~^~^PortableTurret::PortablePlaceTurret: Bad model name '%s'\n", model.c_str());
        }
    } else {
        TurretGun::P_UserAim(cmd);
    }
}

void PortableTurret::P_TurretEndUsed()
{
    if (nextUsableTime > level.time) {
        AbortTurretSetup();
        return;
    }

    endReloadTime = 0;
    TurretGun::P_TurretEndUsed();

    RemoveUserCamera();
    ForceIdle();
}

void PortableTurret::P_TurretBeginUsed(Player *pEnt)
{
    Event  *ev;
    Weapon *activeWeap;

    TurretGun::P_TurretBeginUsed(pEnt);

    activeWeap = pEnt->GetActiveWeapon(WEAPON_MAIN);
    if (activeWeap && activeWeap->GetPutaway()) {
        ev = new Event("deactivateweapon", 1);
        ev->AddString("righthand");
        pEnt->PostEvent(ev, 0);
    }

    ev = new Event("modheight", 1);
    ev->AddString("stand");
    pEnt->ProcessEvent(ev);

    ev = new Event("moveposflags", 1);
    ev->AddString("standing");
    pEnt->ProcessEvent(ev);
}

void PortableTurret::P_ThinkActive()
{
    Event  *ev;
    Weapon *activeWeap;
    Entity *ent;
    Player *ownerP = static_cast<Player *>(owner.Pointer());

    if (!ownerP->IsZoomed()) {
        ownerP->ToggleZoom(80);
    }

    owner->setOrigin(ownerPosition);

    if (nextUsableTime != 0) {
        if (nextUsableTime > level.time) {
            float yawCap;

            // cap the pitch
            if (m_vUserViewAng[0] < m_fPitchUpCap) {
                m_vUserViewAng[0] = m_fPitchUpCap;
            } else if (m_vUserViewAng[0] > m_fPitchDownCap) {
                m_vUserViewAng[0] = m_fPitchDownCap;
            }

            yawCap = AngleSubtract(m_vUserViewAng[1], m_fStartYaw);
            if (yawCap > m_fMaxYawOffset) {
                yawCap = m_fMaxYawOffset;
            } else if (yawCap < -m_fMaxYawOffset) {
                yawCap = -m_fMaxYawOffset;
            }

            m_vUserViewAng[1] = m_fStartYaw + yawCap;
            // set the view angles
            owner->SetViewAngles(m_vUserViewAng);
            m_pUserCamera->setAngles(m_vUserViewAng);

            return;
        }

        nextUsableTime = 0;

        if (packingUp) {
            SpawnArgs spawnArgs;
            ClassDef *c;

            ownerP->SetHolsteredByCode(false);
            P_TurretEndUsed();

            m_iFiring = 0;

            spawnArgs.setArg("model", "weapons/mg42carryable.tik");
            c = spawnArgs.getClassDef();
            if (c && checkInheritance(Entity::classinfostatic(), c)) {
                ent = static_cast<Entity *>(c->newInstance());
                if (ent) {
                    activeWeap = ownerP->GetActiveWeapon(WEAPON_MAIN);
                    if (activeWeap) {
                        activeWeap->DetachFromOwner();
                    }

                    ent->setModel("weapons/mg42carryable.tik");
                    ent->setOrigin(vec_zero);
                    ent->ProcessInitCommands();

                    ev = new Event(EV_Item_Pickup, 1);
                    ev->AddEntity(ownerP);
                    ent->ProcessEvent(ev);
                    ent->PostEvent(EV_Remove, 0);
                } else {
                    gi.DPrintf("couldn't spawn carryable turret for player\n");
                }

                // remove the carryable turret entity and the base
                hideModel();
                setOrigin(vec_zero);
                PostEvent(EV_Remove, level.frametime);

                if (baseEntity) {
                    baseEntity->hideModel();
                    baseEntity->setOrigin(vec_zero);
                    baseEntity->PostEvent(EV_Remove, level.frametime);
                    baseEntity = NULL;
                }
            } else {
                gi.DPrintf("WARNING: couldn't spawn tiki '%s'\n", "weapons/mg42carryable.tik");
            }

            return;
        }
        StopWeaponAnim();
        StopWeaponAnim();
        StopWeaponAnim();
        StopWeaponAnim();

        model = currentModel;
        if (setModel()) {
            ForceIdle();
            MakeBaseEntity();
            P_CreateViewModel();
            ForceIdle();
        } else {
            Com_Printf("^~^~^PortableTurret::P_ThinkActive: Bad model name '%s'\n", currentModel.c_str());
            return;
        }
    }

    if (endReloadTime != 0) {
        float  yawCap;
        Vector jittering;

        m_iFiring = 0;
        if (endReloadTime < level.time) {
            ammo_in_clip[FIRE_PRIMARY] = ammo_clip_size[FIRE_PRIMARY];
            endReloadTime              = 0;
        }

        // cap the pitch
        if (m_vUserViewAng[0] < m_fPitchUpCap) {
            m_vUserViewAng[0] = m_fPitchUpCap;
        } else if (m_vUserViewAng[0] > m_fPitchDownCap) {
            m_vUserViewAng[0] = m_fPitchDownCap;
        }

        yawCap = AngleSubtract(m_vUserViewAng[1], m_fStartYaw);
        if (yawCap > m_fMaxYawOffset) {
            yawCap = m_fMaxYawOffset;
        } else if (yawCap < -m_fMaxYawOffset) {
            yawCap = -m_fMaxYawOffset;
        }

        m_vUserViewAng[1] = m_fStartYaw + yawCap;
        // set the view angles
        owner->SetViewAngles(m_vUserViewAng);
        m_pUserCamera->setAngles(m_vUserViewAng);
    } else {
        if (ammo_clip_size[FIRE_PRIMARY] && ammo_in_clip[FIRE_PRIMARY] > 0) {
            float  yawCap;
            Vector delta;
            Vector targetAngles;
            Vector jittering;

            // cap the pitch
            if (m_vUserViewAng[0] < m_fPitchUpCap) {
                m_vUserViewAng[0] = m_fPitchUpCap;
            } else if (m_vUserViewAng[0] > m_fPitchDownCap) {
                m_vUserViewAng[0] = m_fPitchDownCap;
            }

            yawCap = AngleSubtract(m_vUserViewAng[1], m_fStartYaw);
            if (yawCap > m_fMaxYawOffset) {
                yawCap = m_fMaxYawOffset;
            } else if (yawCap < -m_fMaxYawOffset) {
                yawCap = -m_fMaxYawOffset;
            }

            m_vUserViewAng[1] = m_fStartYaw + yawCap;
            // set the view angles
            owner->SetViewAngles(m_vUserViewAng);

            delta        = owner->GunTarget() - origin;
            targetAngles = delta.toAngles();
            P_SetTargetAngles(targetAngles);
            if (m_iFiring) {
                m_iFiring = 4;

                if (ReadyToFire(FIRE_PRIMARY)) {
                    Fire(FIRE_PRIMARY);
                    m_fCurrViewJitter = m_fViewJitter;
                    if (ammo_clip_size[FIRE_PRIMARY]) {
                        // decrease the ammo count
                        ammo_in_clip[FIRE_PRIMARY]--;
                    }
                }
            }

            // apply some jitter
            jittering = m_vUserViewAng;
            P_ApplyFiringViewJitter(jittering);
            m_pUserCamera->setOrigin(origin);
            m_pUserCamera->setAngles(jittering);
            m_pUserCamera->SetPositionOffset(m_vViewOffset);
            ownerP->client->ps.camera_flags |= CF_CAMERA_ANGLES_TURRETMODE;
        } else {
            m_iFiring     = 0;
            endReloadTime = level.time + 1.5;

            angles[0] = GetGroundPitch();
            angles[1] = GetStartYaw();
            setAngles(angles);

            SetWeaponAnim("reload");
        }
    }
}

void PortableTurret::P_TurretUsed(Player *player)
{
    if (player != owner) {
        for (Player *p = static_cast<Player *>(findradius(NULL, centroid, 50)); p;
             p         = static_cast<Player *>(findradius(p, centroid, 50))) {
            if (!p->IsSubclassOfPlayer()) {
                continue;
            }

            if (p != player && !p->IsSpectator() && !p->IsDead()) {
                return;
            }
        }
    }

    TurretGun::P_TurretUsed(player);
}

float PortableTurret::GetGroundPitch() const
{
    return groundPitch;
}

float PortableTurret::GetStartYaw() const
{
    return m_fStartYaw;
}

bool PortableTurret::IsReloading() const
{
    return endReloadTime > level.time;
}

bool PortableTurret::IsSettingUp() const
{
    return nextUsableTime > level.time && !packingUp;
}

bool PortableTurret::IsPackingUp() const
{
    return nextUsableTime > level.time && packingUp;
}

void PortableTurret::Archive(Archiver& arc)
{
    TurretGun::Archive(arc);

    arc.ArchiveSafePointer(&baseEntity);
    arc.ArchiveFloat(&groundPitch);
    arc.ArchiveVector(&ownerPosition);
    arc.ArchiveFloat(&nextUsableTime);
    arc.ArchiveBoolean(&packingUp);
    arc.ArchiveBoolean(&mustDetach);
    arc.ArchiveString(&currentModel);
    arc.ArchiveString(&setupModel);
    arc.ArchiveVector(&cameraOrigin);
    arc.ArchiveFloat(&endReloadTime);
}
