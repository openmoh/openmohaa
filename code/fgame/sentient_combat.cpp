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

#include "sentient.h"
#include "weapon.h"
#include "game.h"
#include "player.h"
#include "../script/scriptexception.h"

ActiveWeapon::ActiveWeapon()
{
    hand = WEAPON_ERROR;
}

void ActiveWeapon::Archive(Archiver& arc)
{
    arc.ArchiveObjectPointer((Class **)&weapon);
    ArchiveEnum(hand, weaponhand_t);
}

bool Sentient::IsNewActiveWeapon(void)
{
    return (newActiveWeapon.weapon != NULL);
}

weaponhand_t Sentient::GetNewActiveWeaponHand(void)
{
    return newActiveWeapon.hand;
}

Weapon *Sentient::GetNewActiveWeapon(void)
{
    return newActiveWeapon.weapon;
}

void Sentient::ClearNewActiveWeapon(void)
{
    newActiveWeapon.weapon.Clear();
    newActiveWeapon.hand = WEAPON_ERROR;
}

void Sentient::EventGiveAmmo(Event *ev)
{
    int         amount, maxamount = -1;
    const char *type;

    type   = ev->GetString(1);
    amount = ev->GetInteger(2);

    if (ev->NumArgs() == 3) {
        maxamount = ev->GetInteger(3);
    }

    GiveAmmo(type, amount, maxamount);
}

int Sentient::AmmoIndex(str type)

{
    Ammo *ammo;

    ammo = FindAmmoByName(type);

    if (ammo) {
        return ammo->getIndex();
    } else {
        return 0;
    }
}

int Sentient::AmmoCount(str type)
{
    Ammo *ammo;

    if (!type.length()) {
        return 0;
    }

    ammo = FindAmmoByName(type);

    if (ammo) {
        return ammo->getAmount();
    } else {
        return 0;
    }
}

int Sentient::MaxAmmoCount(str type)
{
    Ammo *ammo;

    ammo = FindAmmoByName(type);

    if (ammo) {
        return ammo->getMaxAmount();
    } else {
        return 0;
    }
}

void Sentient::GiveAmmo(str type, int amount, int maxamount)
{
    Ammo *ammo;

    ammo = FindAmmoByName(type);

    if (ammo) {
        if (maxamount >= 0) {
            ammo->setMaxAmount(maxamount);
        }

        // Add amount to current amount
        ammo->setAmount(ammo->getAmount() + amount);
    } else {
        // Create a new inventory entry with this name
        ammo = new Ammo;

        if (maxamount >= 0) {
            ammo->setMaxAmount(maxamount);
        }

        ammo->setAmount(amount);

        ammo->setName(type);
        ammo_inventory.AddObject(ammo);
    }
}

int Sentient::UseAmmo(str type, int amount)

{
    int count, i;

    count = ammo_inventory.NumObjects();

    for (i = 1; i <= count; i++) {
        Ammo *ammo = ammo_inventory.ObjectAt(i);
        if (type == ammo->getName()) {
            int ammo_amount = ammo->getAmount();

            // Less ammo than what we specified to use
            if (ammo_amount < amount) {
                ammo->setAmount(0);
                AmmoAmountChanged(ammo);
                return ammo_amount;
            } else {
                ammo->setAmount(ammo->getAmount() - amount);
                AmmoAmountChanged(ammo);
                return amount;
            }
        }
    }
    return 0;
}

void Sentient::AmmoAmountInClipChanged(str type, int amount_in_clip)
{
    int count, i;

    count = ammo_inventory.NumObjects();

    for (i = 1; i <= count; i++) {
        Ammo *ammo = ammo_inventory.ObjectAt(i);
        if (type == ammo->getName()) {
            AmmoAmountChanged(ammo, amount_in_clip);
        }
    }
}

void Sentient::AmmoAmountChanged(Ammo *ammo, int ammo_in_clip) {}

void Sentient::MeleeAttackStart(Event *ev)
{
    in_melee_attack = true;
}

void Sentient::MeleeAttackEnd(Event *ev)
{
    in_melee_attack = false;
}

void Sentient::WeaponKnockedFromHands(void)
{
    str              realname;
    AliasListNode_t *ret;

    realname = GetRandomAlias("snd_lostweapon", &ret);
    if (realname.length() > 1) {
        Sound(realname.c_str(), CHAN_VOICE);
    } else {
        Sound("snd_pain", CHAN_VOICE);
    }
}

bool Sentient::CanSee(Entity *ent, float fov, float vision_distance)
{
    float delta[2];

    delta[0] = ent->centroid[0] - centroid[0];
    delta[1] = ent->centroid[1] - centroid[1];

    if ((vision_distance <= 0.0f) || VectorLength2DSquared(delta) <= (vision_distance * vision_distance)) {
        if (gi.AreasConnected(edict->r.areanum, ent->edict->r.areanum)
            && ((fov <= 0.0f || fov >= 360.0f) || (FovCheck(delta, cos(fov * (0.5 * M_PI / 180.0)))))) {
            //
            // Parent checking for subclass is absolute non-sense...
            //
            if (ent->IsSubclassOfPlayer()) {
                Player *p = (Player *)ent;

                Vector vStart = EyePosition();
                Vector vEnd   = p->EyePosition();

                return G_SightTrace(vStart, vec_zero, vec_zero, vEnd, this, ent, MASK_CANSEE, 0, "Sentient::CanSee 1");
            } else {
                Vector vStart = EyePosition();
                Vector vEnd   = ent->centroid;

                return G_SightTrace(vStart, vec_zero, vec_zero, vEnd, this, ent, MASK_CANSEE, 0, "Sentient::CanSee 2");
            }
        }
    }

    return false;
}

Vector Sentient::GunPosition(void)
{
    Vector vPos;

    if (activeWeaponList[WEAPON_MAIN]) {
        activeWeaponList[WEAPON_MAIN]->GetMuzzlePosition(&vPos);
    } else {
        vPos = origin;
    }

    return vPos;
}

Vector Sentient::GunTarget(bool bNoCollision)
{
    Vector vPos;

    if (mTargetPos[0] && mTargetPos[1] && mTargetPos[2]) {
        AnglesToAxis(angles, orientation);
        vPos = GunPosition() + Vector(orientation[0]) * 2048.0f;
    } else {
        if (G_Random() > mAccuracy) {
            float rand = G_Random(5.0f);

            if (G_Random() <= 0.5f) {
                vPos[0] = rand - 32.0f;
            } else {
                vPos[0] = rand + 32.0f;
            }

            rand = G_Random(5.0f);

            if (G_Random() <= 0.5f) {
                vPos[1] = rand - 32.0f;
            } else {
                vPos[1] = rand + 32.0f;
            }

            vPos[2] = -96.0f - G_Random(5.0f);
        }

        vPos += mTargetPos;
    }

    return vPos;
}

void Sentient::ChargeWeapon(weaponhand_t hand, firemode_t mode)
{
    Weapon *activeWeapon;

    if (hand > MAX_ACTIVE_WEAPONS) {
        warning(
            "Sentient::ChargeWeapon",
            "Weapon hand number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n",
            hand,
            MAX_ACTIVE_WEAPONS
        );
        return;
    }

    // start charging the active weapon
    activeWeapon = activeWeaponList[(int)hand];

    if ((activeWeapon) && activeWeapon->ReadyToFire(mode) && activeWeapon->MuzzleClear()) {
        charge_start_time = level.time;

        if (mode == FIRE_PRIMARY) {
            activeWeapon->SetWeaponAnim("charge");
        } else if (mode == FIRE_SECONDARY) {
            activeWeapon->SetWeaponAnim("secondarycharge");
        }
    }
}

void Sentient::ChargeWeapon(Event *ev)
{
    firemode_t   mode = FIRE_PRIMARY;
    weaponhand_t hand = WEAPON_MAIN;

    if (charge_start_time) {
        // Charging has already been started, so return
        return;
    }

    if (ev->NumArgs() > 0) {
        hand = WeaponHandNameToNum(ev->GetString(1));

        if (hand < 0) {
            return;
        }

        if (ev->NumArgs() == 2) {
            mode = WeaponModeNameToNum(ev->GetString(2));

            if (mode < 0) {
                return;
            }
        }
    }

    ChargeWeapon(hand, mode);
}

void Sentient::ReloadWeapon(Event *ev)
{
    Weapon      *weapon;
    weaponhand_t hand = WEAPON_MAIN;

    if (ev->NumArgs() > 0) {
        hand = WeaponHandNameToNum(ev->GetString(1));
    }

    if (hand == WEAPON_ERROR) {
        return;
    }

    weapon = GetActiveWeapon(hand);

    if (weapon) {
        weapon->StartReloading();
    }
}

void Sentient::FireWeapon(int number, firemode_t mode)
{
    Weapon *activeWeapon = activeWeaponList[number];

    if (activeWeapon) {
        if (mode == FIRE_SECONDARY && activeWeapon->GetZoom() && !activeWeapon->GetAutoZoom() && IsSubclassOfPlayer()) {
            Player *p = (Player *)this;
            p->ToggleZoom(activeWeapon->GetZoom());
        } else {
            activeWeapon->Fire(mode);
        }
    } else {
        gi.DPrintf("No active weapon in slot #: \"%i\"\n", number);
    }
}

void Sentient::FireWeapon(Event *ev)
{
    firemode_t mode   = FIRE_PRIMARY;
    int        number = 0;
    str        modestring;
    str        side;

    if (ev->NumArgs() > 0) {
        side = ev->GetString(1);

        if (side.icmp("mainhand")) {
            if (!side.icmp("offhand")) {
                number = WEAPON_OFFHAND;
            } else {
                number = atoi(side);
            }
        } else {
            number = WEAPON_MAIN;

            if (ev->NumArgs() == 2) {
                modestring = ev->GetString(2);

                if (!modestring.icmp("primary")) {
                    mode = FIRE_PRIMARY;
                } else if (!modestring.icmp("secondary")) {
                    mode = FIRE_SECONDARY;
                } else {
                    warning("Sentient::FireWeapon", "Invalid mode %s\n", modestring.c_str());
                }
            }
        }
    }

    FireWeapon(number, mode);
}

void Sentient::StopFireWeapon(Event *ev)
{
    Weapon *activeWeapon;
    int     number = 0;
    str     side;

    if (ev->NumArgs() > 0) {
        side = ev->GetString(1);

        number = WeaponHandNameToNum(side);
    }

    if ((number > MAX_ACTIVE_WEAPONS) || (number < 0)) {
        warning(
            "Sentient::StopFireWeapon",
            "Weapon number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n",
            number,
            MAX_ACTIVE_WEAPONS
        );
        return;
    }

    activeWeapon = activeWeaponList[number];

    if (activeWeapon) {
        activeWeapon->ForceIdle();
    } else {
        if (!activeWeapon) {
            gi.DPrintf("No active weapon in slot #: \"%i\"\n", number);
        }
    }
}

void Sentient::ReleaseFireWeapon(int number, firemode_t mode)
{
    float charge_time = level.time - charge_start_time;

    charge_start_time = 0;

    if ((number > MAX_ACTIVE_WEAPONS) || (number < 0)) {
        warning(
            "Sentient::FireWeapon",
            "Weapon number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n",
            number,
            MAX_ACTIVE_WEAPONS
        );
        return;
    } else {
        if (activeWeaponList[number]) {
            activeWeaponList[number]->ReleaseFire(mode, charge_time);
        }
    }
}

void Sentient::ReleaseFireWeapon(Event *ev)
{
    firemode_t mode   = FIRE_PRIMARY;
    int        number = 0;
    str        modestring;
    str        side;

    if (ev->NumArgs() > 0) {
        side = ev->GetString(1);

        if (side.icmp("mainhand")) {
            if (!side.icmp("offhand")) {
                number = WEAPON_OFFHAND;
            } else {
                number = atoi(side);
            }
        } else {
            number = WEAPON_MAIN;

            if (ev->NumArgs() == 2) {
                modestring = ev->GetString(2);

                if (!modestring.icmp("primary")) {
                    mode = FIRE_PRIMARY;
                } else if (!modestring.icmp("secondary")) {
                    mode = FIRE_SECONDARY;
                } else {
                    warning("Sentient::ReleaseFireWeapon", "Invalid mode %s\n", modestring.c_str());
                }
            }
        }
    }

    ReleaseFireWeapon(number, mode);
}

void Sentient::Holster(qboolean putaway)
{
    Weapon *rightWeap;

    rightWeap = GetActiveWeapon(WEAPON_MAIN);

    // Holster
    if (putaway) {
        if (rightWeap) {
            rightWeap->SetPutAway(true);
            holsteredWeapon = rightWeap;
        }
    } else {
        if (!putaway) {
            // Unholster
            if (holsteredWeapon) {
                useWeapon(holsteredWeapon, WEAPON_MAIN);
            }

            holsteredWeapon = NULL;
        }
    }
}

void Sentient::SafeHolster(qboolean putaway)
{
    if (WeaponsOut()) {
        if (putaway) {
            weapons_holstered_by_code = qtrue;
            Holster(qtrue);
        }
    } else {
        if (weapons_holstered_by_code) {
            weapons_holstered_by_code = qfalse;
            Holster(qfalse);
        }
    }
}

void Sentient::AttachAllActiveWeapons(void)
{
    int i;

    for (i = 0; i < MAX_ACTIVE_WEAPONS; i++) {
        Weapon *weap = activeWeaponList[i];

        if (weap) {
            weap->AttachToOwner((weaponhand_t)i);
        }
    }

    if (this->isSubclassOf(Player)) {
        Player *player = (Player *)this;
        player->UpdateWeapons();
    }
}

void Sentient::DetachAllActiveWeapons(void)
{
    int i;

    for (i = 0; i < MAX_ACTIVE_WEAPONS; i++) {
        Weapon *weap = activeWeaponList[i];

        if (weap) {
            weap->DetachFromOwner();
        }
    }
}

int Sentient::NumWeapons(void)
{
    int   num;
    int   i;
    Item *item;
    int   numweaps;

    numweaps = 0;

    num = inventory.NumObjects();
    for (i = 1; i <= num; i++) {
        item = (Item *)G_GetEntity(inventory.ObjectAt(i));
        if (checkInheritance(&Weapon::ClassInfo, item->getClassname())) {
            numweaps++;
        }
    }

    return numweaps;
}

void Sentient::ChangeWeapon(Weapon *weapon, weaponhand_t hand)
{
    if ((hand > MAX_ACTIVE_WEAPONS) || (hand < 0)) {
        warning(
            "Sentient::ChangeWeapon",
            "Weapon hand number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n",
            hand,
            MAX_ACTIVE_WEAPONS
        );
        return;
    }

    // Check if weapon is already active in the slot
    if (weapon == activeWeaponList[hand]) {
        return;
    }

    ActivateWeapon(weapon, hand);
}

void Sentient::DeactivateWeapon(weaponhand_t hand)
{
    int i;

    if (!activeWeaponList[hand]) {
        warning("Sentient::DeactivateWeapon", "Tried to deactivate a non-active weapon in hand %d\n", hand);
        return;
    }

    activeWeaponList[hand]->AttachToHolster(hand);
    activeWeaponList[hand]->SetPutAway(false);
    activeWeaponList[hand]->NewAnim("putaway");

    // Check the player's inventory and detach any weapons that are already attached to that spot
    for (i = 1; i <= inventory.NumObjects(); i++) {
        Item *item = (Item *)G_GetEntity(inventory.ObjectAt(i));

        if (item->IsSubclassOfWeapon()) {
            Weapon *weap = (Weapon *)item;

            if ((weap != activeWeaponList[hand])
                && (!str::cmp(weap->GetCurrentAttachToTag(), activeWeaponList[hand]->GetCurrentAttachToTag()))) {
                weap->DetachFromOwner();
            }
        }
    }

    lastActiveWeapon.weapon = activeWeaponList[hand];
    lastActiveWeapon.hand   = hand;
    activeWeaponList[hand]  = NULL;
}

void Sentient::DeactivateWeapon(Weapon *weapon)
{
    int i;

    for (i = 0; i < MAX_ACTIVE_WEAPONS; i++) {
        if (activeWeaponList[i] == weapon) {
            activeWeaponList[i]->DetachFromOwner();
            activeWeaponList[i]->SetPutAway(false);
            activeWeaponList[i] = NULL;
        }
    }
}

void Sentient::EventDeactivateWeapon(Event *ev)
{
    // Deactivate the weapon
    weaponhand_t hand;
    str          side;

    side = ev->GetString(1);

    hand = WeaponHandNameToNum(side);

    if (hand == WEAPON_ERROR) {
        return;
    }

    DeactivateWeapon(hand);
    edict->s.eFlags |= EF_UNARMED;
}

void Sentient::ActivateWeapon(Weapon *weapon, weaponhand_t hand)
{
    int i;

    if (hand == WEAPON_ERROR) {
        gi.DPrintf("^~^~^ ActivateWeapon: bad weapon hand\n");
        return;
    }

    activeWeaponList[hand] = weapon;
    str holsterTag         = weapon->GetHolsterTag();

    // Check the player's inventory and detach any weapons that are currently attached to that tag.
    for (i = 1; i <= inventory.NumObjects(); i++) {
        Item *item = (Item *)G_GetEntity(inventory.ObjectAt(i));

        if (item->isSubclassOf(Weapon)) {
            Weapon *weap = (Weapon *)item;

            if ((!str::cmp(holsterTag, weap->GetCurrentAttachToTag()))) {
                weap->DetachFromOwner();
            }
        }
    }
    weapon->AttachToOwner(hand);
    weapon->NewAnim("raise");
}

void Sentient::ActivateLastActiveWeapon(void)
{
    if (lastActiveWeapon.weapon && lastActiveWeapon.weapon != activeWeaponList[WEAPON_MAIN]) {
        useWeapon(lastActiveWeapon.weapon, lastActiveWeapon.hand);
    }
}

void Sentient::EventActivateLastActiveWeapon(Event *ev)
{
    if (!deadflag) {
        ActivateLastActiveWeapon();
    }
}

void Sentient::EventToggleItemUse(Event *ev)
{
    Weapon *item;

    if (deadflag) {
        return;
    }

    item = GetActiveWeapon(WEAPON_MAIN);

    if (item && item->IsSubclassOfInventoryItem()) {
        if (lastActiveWeapon.weapon) {
            ActivateLastActiveWeapon();
        }

        Holster(qtrue);
    } else {
        Event *ev = new Event(EV_Sentient_UseWeaponClass);
        ev->AddString("item1");

        ProcessEvent(ev);
    }
}

Weapon *Sentient::BestWeapon(Weapon *ignore, qboolean bGetItem, int iIgnoreClass)
{
    Weapon *next;
    int     n;
    int     j;
    int     bestrank;
    Weapon *bestweapon;

    n = inventory.NumObjects();

    // Search forewards until we find a weapon
    bestweapon = NULL;
    bestrank   = -999999;

    for (j = 1; j <= n; j++) {
        next = (Weapon *)G_GetEntity(inventory.ObjectAt(j));

        assert(next);

        if ((next != ignore)
            && ((next->IsSubclassOfWeapon() && !(next->GetWeaponClass() & iIgnoreClass))
                || (next->IsSubclassOfItem() && bGetItem))
            && (next->GetRank() > bestrank)
            && (next->HasAmmo(FIRE_PRIMARY) || next->GetFireType(FIRE_SECONDARY) == FT_MELEE)) {
            bestweapon = (Weapon *)next;
            bestrank   = bestweapon->GetRank();
        }
    }

    return bestweapon;
}

Weapon *Sentient::WorstWeapon(Weapon *ignore, qboolean bGetItem, int iIgnoreClass)
{
    Weapon *next;
    int     n;
    int     j;
    int     iWorstRank;
    Weapon *worstweapon;

    n = inventory.NumObjects();

    worstweapon = NULL;
    iWorstRank  = 999999;

    for (j = 1; j <= n; j++) {
        next = (Weapon *)G_GetEntity(inventory.ObjectAt(j));

        assert(next);

        if ((next != ignore)
            && ((next->IsSubclassOfWeapon() && !(next->GetWeaponClass() & iIgnoreClass))
                || (next->IsSubclassOfWeapon() && bGetItem))
            && (next->GetRank() < iWorstRank)
            && (next->HasAmmo(FIRE_PRIMARY) || next->GetFireType(FIRE_SECONDARY) == FT_MELEE)) {
            worstweapon = (Weapon *)next;
            iWorstRank  = worstweapon->GetRank();
        }
    }

    return worstweapon;
}

Weapon *Sentient::NextWeapon(Weapon *weapon)
{
    Item   *item;
    int     i;
    int     n;
    int     weaponorder;
    Weapon *choice;
    int     choiceorder;
    Weapon *bestchoice;
    int     bestorder;
    Weapon *worstchoice;
    int     worstorder;

    if (!inventory.ObjectInList(weapon->entnum)) {
        ScriptError("NextWeapon", "Weapon not in list");
    }

    weaponorder = weapon->GetOrder();
    bestchoice  = weapon;
    bestorder   = 65535;
    worstchoice = weapon;
    worstorder  = weaponorder;

    n = inventory.NumObjects();
    for (i = 1; i <= n; i++) {
        item = (Item *)G_GetEntity(inventory.ObjectAt(i));

        assert(item);

        if (item->isSubclassOf(Weapon)) {
            choice = (Weapon *)item;
            if ((!choice->HasAmmo(FIRE_PRIMARY) && !choice->GetUseNoAmmo()) || !choice->AutoChange()) {
                continue;
            }

            choiceorder = choice->GetOrder();
            if ((choiceorder > weaponorder) && (choiceorder < bestorder)) {
                bestorder  = choiceorder;
                bestchoice = choice;
            }

            if (choiceorder < worstorder) {
                worstorder  = choiceorder;
                worstchoice = choice;
            }
        }
    }

    if (bestchoice == weapon) {
        return worstchoice;
    }

    return bestchoice;
}

Weapon *Sentient::PreviousWeapon(Weapon *weapon)
{
    Item   *item;
    int     i;
    int     n;
    int     weaponorder;
    Weapon *choice;
    int     choiceorder;
    Weapon *bestchoice;
    int     bestorder;
    Weapon *worstchoice;
    int     worstorder;

    if (!inventory.ObjectInList(weapon->entnum)) {
        ScriptError("PreviousWeapon", "Weapon not in list");
    }

    weaponorder = weapon->GetOrder();
    bestchoice  = weapon;
    bestorder   = -65535;
    worstchoice = weapon;
    worstorder  = weaponorder;

    n = inventory.NumObjects();
    for (i = 1; i <= n; i++) {
        item = (Item *)G_GetEntity(inventory.ObjectAt(i));

        assert(item);

        if (item->isSubclassOf(Weapon)) {
            choice = (Weapon *)item;
            if ((!choice->HasAmmo(FIRE_PRIMARY) && !choice->GetUseNoAmmo()) || !choice->AutoChange()) {
                continue;
            }

            choiceorder = choice->GetOrder();
            if ((choiceorder < weaponorder) && (choiceorder > bestorder)) {
                bestorder  = choiceorder;
                bestchoice = choice;
            }

            if (choiceorder > worstorder) {
                worstorder  = choiceorder;
                worstchoice = choice;
            }
        }
    }

    if (bestchoice == weapon) {
        return worstchoice;
    }

    return bestchoice;
}

Weapon *Sentient::GetActiveWeapon(weaponhand_t hand) const

{
    if ((hand > MAX_ACTIVE_WEAPONS) || (hand < 0)) {
        warning(
            "Sentient::GetActiveWeapon",
            "Weapon hand number \"%d\" is out of bounds of 0 to MAX_ACTIVE_WEAPONS:%d\n",
            hand,
            MAX_ACTIVE_WEAPONS
        );
        return NULL;
    } else {
        return activeWeaponList[hand];
    }
}

qboolean Sentient::IsActiveWeapon(Weapon *weapon)

{
    int i;

    for (i = 0; i < MAX_ACTIVE_WEAPONS; i++) {
        Weapon *activeWeap = activeWeaponList[i];

        if (activeWeap == weapon) {
            return true;
        }
    }

    return false;
}

void Sentient::useWeapon(const char *weaponname, weaponhand_t hand)
{
    Weapon *weapon;

    assert(weaponname);

    if (!weaponname) {
        warning("Sentient::useWeapon", "weaponname is NULL\n");
        return;
    }

    // Find the item in the sentient's inventory
    weapon = (Weapon *)FindItem(weaponname);

    // If it exists, then make the change to the slot number specified
    if (weapon) {
        useWeapon(weapon, hand);
    }
}

void Sentient::useWeapon(Weapon *weapon, weaponhand_t hand)
{
    assert(weapon);

    if (!weapon) {
        warning("Sentient::useWeapon", "Null weapon used.\n");
        return;
    }

    if (newActiveWeapon.weapon) {
        newActiveWeapon.weapon = weapon;
        newActiveWeapon.hand   = hand;
        return;
    }

    if (!weapon->HasAmmo(FIRE_PRIMARY) && !weapon->GetUseNoAmmo()) {
        return;
    }

    if (activeWeaponList[WEAPON_OFFHAND]) {
        activeWeaponList[WEAPON_OFFHAND]->PutAway();
    }

    if (activeWeaponList[WEAPON_MAIN] && activeWeaponList[WEAPON_MAIN] != weapon) {
        activeWeaponList[WEAPON_MAIN]->PutAway();
    }

    newActiveWeapon.weapon = weapon;
    newActiveWeapon.hand   = hand;

    //ChangeWeapon( weapon, hand );
}

void Sentient::EventUseWeaponClass(Event *ev)
{
    const char *name;
    int         weapon_class;

    if (deadflag) {
        return;
    }

    name         = ev->GetString(1);
    weapon_class = G_WeaponClassNameToNum(name);

    int     num;
    Weapon *pActive = GetActiveWeapon(WEAPON_MAIN);
    Weapon *pMatch  = NULL;
    Weapon *pWeap   = NULL;

    num = inventory.NumObjects();

    for (int i = 1; i <= num; i++) {
        pWeap = (Weapon *)G_GetEntity(inventory.ObjectAt(i));

        if (pWeap->IsSubclassOfWeapon() && (pWeap->GetWeaponClass() & weapon_class)
            && (pWeap->HasAmmo(FIRE_PRIMARY) || pWeap->GetUseNoAmmo())) {
            if (!pMatch && (!pActive || pActive != pWeap)) {
                pMatch = pWeap;
            }

            if (!pActive) {
                useWeapon(pWeap, WEAPON_MAIN);
                return;
            }

            if (pActive == pWeap) {
                pActive = NULL;
            }
        }

        pWeap = NULL;
    }

    if (pMatch) {
        useWeapon(pMatch, WEAPON_MAIN);
    }
}

//====================
//ActivateNewWeapon
//====================
void Sentient::ActivateNewWeapon(Event *ev)
{
    if (deadflag) {
        return;
    }

    ActivateNewWeapon();

    if (GetActiveWeapon(WEAPON_MAIN)) {
        edict->s.eFlags &= ~EF_UNARMED;
    }
}

//====================
//ActivateNewWeapon
//====================
void Sentient::ActivateNewWeapon(void)
{
    // Change the weapon to the currently active weapon as specified by useWeapon
    ChangeWeapon(newActiveWeapon.weapon, newActiveWeapon.hand);

    // Update weapons
    UpdateWeapons();

    // Clear out the newActiveWeapon
    ClearNewActiveWeapon();
}

void Sentient::PutawayWeapon(Event *ev)
{
    Weapon      *weapon;
    weaponhand_t hand;
    str          side;

    side = ev->GetString(1);

    hand = WeaponHandNameToNum(side);

    if (hand == WEAPON_ERROR) {
        return;
    }

    weapon = GetActiveWeapon(hand);

    if (weapon->isSubclassOf(Weapon)) {
        weapon->NewAnim("putaway");
    }
}

void Sentient::WeaponCommand(Event *ev)
{
    weaponhand_t hand;
    Weapon      *weap;
    int          i;

    if (ev->NumArgs() < 2) {
        return;
    }

    hand = WeaponHandNameToNum(ev->GetString(1));
    weap = GetActiveWeapon(hand);

    if (!weap) {
        return;
    }

    Event *e;
    e = new Event(ev->GetToken(2));

    for (i = 3; i <= ev->NumArgs(); i++) {
        e->AddToken(ev->GetToken(i));
    }

    weap->ProcessEvent(e);
}

qboolean Sentient::WeaponsOut(void)
{
    return (GetActiveWeapon(WEAPON_OFFHAND) || GetActiveWeapon(WEAPON_MAIN));
}

void Sentient::UpdateWeapons(void)
{
    GetActiveWeapon(WEAPON_MAIN);
}

Ammo *Sentient::FindAmmoByName(str name)
{
    int count, i;

    count = ammo_inventory.NumObjects();

    for (i = 1; i <= count; i++) {
        Ammo *ammo = ammo_inventory.ObjectAt(i);
        if (name == ammo->getName()) {
            return ammo;
        }
    }
    return NULL;
}

void Sentient::GetNewActiveWeapon(Event *ev)
{
    ev->AddEntity(GetNewActiveWeapon());
}

void Sentient::GetActiveWeap(Event *ev)
{
    weaponhand_t weaponhand;
    Item        *weapon;

    weaponhand = (weaponhand_t)ev->GetInteger(1);

    if (weaponhand < 0 && weaponhand > 2) {
        ScriptError("Weaponhand number is out of allowed range 0 - 2 for getactiveweap!\n");
        return;
    }

    weapon = GetActiveWeapon(weaponhand);

    ev->AddEntity(weapon);
}
