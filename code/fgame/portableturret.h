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

#pragma once

#include "inventoryitem.h"
#include "weapturret.h"

class CarryableTurret : public InventoryItem
{
private:
    str      turretModel;
    qboolean isPickable;

public:
    CLASS_PROTOTYPE(CarryableTurret);

    CarryableTurret();

    void EventPickup(Event *ev);

    qboolean Removable(void) override;
    void     DetachFromOwner(void) override;
    void     AttachToHolster(weaponhand_t hand) override;
    void     DropTurret(Event *ev);
    qboolean Drop(void) override;
    void     EventTurretTik(Event *ev);
    void     EventPlaceTurret(Event *ev);
    void     EventEnablePickup(Event *ev);

    bool IsCarryableTurret() const override;

    void Archive(Archiver& arc) override;
};

class PortableTurret : public TurretGun
{
private:
    EntityPtr baseEntity;
    float     groundPitch;
    Vector    ownerPosition;
    float     nextUsableTime;
    qboolean  packingUp;
    qboolean  mustDetach;
    str       currentModel;
    str       setupModel;
    Vector    cameraOrigin;
    float     endReloadTime;

public:
    CLASS_PROTOTYPE(PortableTurret);

public:
    PortableTurret();
    ~PortableTurret();

    void SetGroundPitch(float pitch);
    void SetOwnerPosition(const Vector& pos);
    void PortablePlaceTurret(Event *ev);
    void MakeBaseEntity();
    void EventSetClipSize(Event *ev);
    void AbortTurretSetup();

    void P_UserAim(usercmd_t *cmd) override;
    void P_TurretEndUsed() override;
    void P_TurretBeginUsed(Player *pEnt) override;
    void P_ThinkActive() override;
    void P_TurretUsed(Player *player) override;

    float GetGroundPitch() const;
    float GetStartYaw() const;
    bool  IsReloading() const;
    bool  IsSettingUp() const;
    bool  IsPackingUp() const;

    void Archive(Archiver& arc) override;
};
