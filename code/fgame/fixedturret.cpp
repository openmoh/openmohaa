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

#include "fixedturret.h"
#include "g_phys.h"
#include "weapturret.h"
#include "dm_manager.h"
#include "player.h"

CLASS_DECLARATION(VehicleTank, FixedTurret, NULL) {
    {NULL, NULL}
};

FixedTurret::FixedTurret()
{
    locked = false;
    driver.flags |= SLOT_UNUSED;
}

void FixedTurret::Archive(Archiver& arc)
{
    VehicleTank::Archive(arc);

    arc.ArchiveVector(&lastEntOrigin);
    arc.ArchiveVector(&lastEntAngles);
}

qboolean FixedTurret::Drive(usercmd_t *ucmd)
{
    qboolean result;
    setMoveType(MOVETYPE_NONE);

    result = VehicleTank::Drive(ucmd);

    moveimpulse = 0;
    turnimpulse = 0;
    jumpimpulse = 0;

    return result;
}

void FixedTurret::DetachRemoteOwner()
{
    TurretGun* turret;

    Vehicle::DetachRemoteOwner();

    turret = static_cast<TurretGun*>(Turrets[0].ent.Pointer());
    turret->RemoveUserCamera();
}

void FixedTurret::EventDamage(Event *ev)
{
    meansOfDeath_t mod = static_cast<meansOfDeath_t>(ev->GetInteger(9));

    // ignore damage when the round isn't active in TOW
    if (g_gametype->integer == GT_TOW && !dmManager.RoundActive()) {
        return;
    }

    // ignore bullets
    switch (mod) {
    case MOD_BULLET:
    case MOD_BASH:
    case MOD_FAST_BULLET:
    case MOD_VEHICLE:
    case MOD_SHOTGUN:
        return;
    }

    // ignore grenades in single player mode
    if (g_gametype->integer == GT_SINGLE_PLAYER && mod == MOD_GRENADE) {
        return;
    }

    Vehicle::EventDamage(ev);

    if (g_gametype->integer == GT_SINGLE_PLAYER) {
        if (driver.ent && driver.ent->IsSubclassOfPlayer()) {
            Player* player = static_cast<Player*>(driver.ent.Pointer());
            player->m_iNumHitsTaken;
        }
    }

    if (health <= 0 && driver.ent && driver.ent->IsSubclassOfPlayer()) {
        Player* player = static_cast<Player*>(driver.ent.Pointer());

        DetachDriverSlot(0, vec_zero, NULL);
        DetachRemoteOwner();

        player->takedamage = DAMAGE_YES;
        player->PostEvent(EV_Kill, 0.1f);
    }
}

void FixedTurret::AttachDriverSlot(int slot, Entity *ent, Vector vExitPosition)
{
    lastEntOrigin = ent->origin;
    lastEntAngles = ent->angles;
    Vehicle::AttachDriverSlot(slot, ent, vExitPosition);
}

void FixedTurret::DetachDriverSlot(int slot, Vector vExitPosition, Vector *vExitAngles)
{
    Vehicle::DetachDriverSlot(slot, lastEntOrigin, &lastEntAngles);
}

void FixedTurret::EventSetCollisionModel(Event *ev)
{
    Vehicle::EventSetCollisionModel(ev);

    if (m_pCollisionEntity) {
        m_pCollisionEntity->DisconnectPaths();
    }
}
