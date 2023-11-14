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

// player_combat.cpp: Player combat system and combat utility functions
//

#include "player.h"
#include "weapturret.h"
#include "weaputils.h"

//Forward
//Back
//TurnRight
//TurnLeft
//Moveleft (strafe)
//Moveright (strafe)
//Moveup (Jump)
//Movedown (Duck)
//Action (Use)
//Sneak (Toggle or Momentary)
//Speed/Walk (Toggle or Momentary)
//Fire Left hand
//Fire Right hand

#define SLOPE_45_MIN         0.7071f
#define SLOPE_45_MAX         0.831f
#define SLOPE_22_MIN         SLOPE_45_MAX
#define SLOPE_22_MAX         0.95f

#define MIN_Z                -999999
#define PUSH_OBJECT_DISTANCE 16.0f

#define ARMS_NAME            "Bip01 Spine2"

static Vector min_box_8x8(-4, -4, -4);
static Vector max_box_8x8(4, 4, 4);
static Vector min4x4(-4, -4, 0);
static Vector max4x4x0(4, 4, 0);
static Vector max4x4x8(4, 4, 8);

qboolean Player::CondTrue(Conditional& condition)
{
    return true;
}

qboolean Player::CondChance(Conditional& condition)

{
    float percent_chance;

    percent_chance = atof(condition.getParm(1));

    return (G_Random() < percent_chance);
}

qboolean Player::CondHealth(Conditional& condition)

{
    return health < atoi(condition.getParm(1));
}

qboolean Player::CondBlocked(Conditional& condition)
{
    int test_moveresult;

    test_moveresult = moveresult;

    if (flags & FL_IMMOBILE) {
        test_moveresult = MOVERESULT_BLOCKED;
    }

    if (condition.numParms()) {
        return test_moveresult >= atoi(condition.getParm(1));
    }

    return test_moveresult >= MOVERESULT_BLOCKED;
}

qboolean Player::CondPain(Conditional& condition)

{
    return (pain != 0 || knockdown != 0);
}

qboolean Player::CondOnGround(Conditional& condition)
{
    if (groundentity || client->ps.walking) {
        falling = 0;
        return qtrue;
    } else {
        return qfalse;
    }
}

qboolean Player::CondHasWeapon(Conditional& condition)

{
    return WeaponsOut();
}

qboolean Player::CondNewWeapon(Conditional& condition)

{
    Weapon *weapon;

    weapon = GetNewActiveWeapon();

    if (weapon) {
        return true;
    } else {
        return false;
    }
}

qboolean Player::CondImmediateSwitch(Conditional& condition)
{
    static cvar_t *g_immediateswitch = NULL;

    if (!g_immediateswitch) {
        g_immediateswitch = gi.Cvar_Get("g_immediateswitch", "0", 0);
    }

    return (g_gametype->integer && g_immediateswitch->integer);
}

// Check to see if a weapon has been raised
qboolean Player::CondUseWeapon(Conditional& condition)

{
    const char *weaponName;
    const char *parm;

    weaponhand_t hand;
    Weapon      *weap;

    weap = GetNewActiveWeapon();
    parm = condition.getParm(1);

    if (!str::icmp(parm, "ERROR")) {
        if (weap) {
            warning("Player::CondUseweapon", "%s does not have a valid RAISE_WEAPON state\n", weap->item_name.c_str());
        } else {
            warning("Player::CondUseweapon", "New Active weapon does not exist\n");
        }

        ClearNewActiveWeapon();
        return qtrue;
    }

    hand = WeaponHandNameToNum(parm);

    if (hand == WEAPON_ERROR) {
        return false;
    }

    weaponName = condition.getParm(2);

    if ((weap != NULL) && (GetNewActiveWeaponHand() == hand) && (!Q_stricmp(weap->item_name, weaponName))) {
        return true;
    } else {
        return false;
    }
}

qboolean Player::CondUseWeaponClass(Conditional& condition)
{
    const char *weaponClass;
    const char *parm;

    weaponhand_t hand;
    Weapon      *weap;

    weap = GetNewActiveWeapon();
    parm = condition.getParm(1);

    if (!str::icmp(parm, "ERROR")) {
        if (weap) {
            warning(
                "Player::CondUseweaponclass", "%s does not have a valid RAISE_WEAPON state\n", weap->getName().c_str()
            );
        } else {
            warning("Player::CondUseweaponclass", "New Active weapon does not exist\n");
        }

        ClearNewActiveWeapon();
        return qtrue;
    }

    hand = WeaponHandNameToNum(parm);

    if (hand == WEAPON_ERROR) {
        return false;
    }

    weaponClass = condition.getParm(2);

    if ((weap != NULL) && (weap->isSubclassOf(Weapon)) && (GetNewActiveWeaponHand() == hand)
        && (weap->GetWeaponClass() & G_WeaponClassNameToNum(weaponClass))) {
        return true;
    } else {
        return false;
    }
}

// Checks to see if weapon is active
qboolean Player::CondWeaponActive(Conditional& condition)
{
    const char  *weaponName;
    weaponhand_t hand;

    weaponName = condition.getParm(2);
    hand       = WeaponHandNameToNum(condition.getParm(1));

    if (hand == WEAPON_ERROR) {
        return false;
    }

    Weapon *weapon = GetActiveWeapon(hand);

    return (weapon && !Q_stricmp(weaponName, weapon->item_name));
}

qboolean Player::CondWeaponClassActive(Conditional& condition)
{
    const char  *weaponClass;
    weaponhand_t hand;

    weaponClass = condition.getParm(2);
    hand        = WeaponHandNameToNum(condition.getParm(1));

    if (hand == WEAPON_ERROR) {
        return false;
    }

    Weapon *weapon = GetActiveWeapon(hand);

    return (weapon && G_WeaponClassNameToNum(weaponClass) & weapon->GetWeaponClass());
}

qboolean Player::CondWeaponCurrentFireAnim(Conditional& condition)
{
    weaponhand_t hand      = WeaponHandNameToNum(condition.getParm(1));
    int          iFireAnim = atoi(condition.getParm(2));
    Weapon      *weapon;

    if (hand == WEAPON_ERROR) {
        return false;
    }

    weapon = GetActiveWeapon(hand);

    return weapon && weapon->m_iCurrentFireAnim == iFireAnim;
}

// Checks to see if weapon is active and ready to fire
qboolean Player::CondWeaponReadyToFire(Conditional& condition)
{
    firemode_t   mode       = FIRE_PRIMARY;
    str          weaponName = "None";
    weaponhand_t hand;
    qboolean     ready;

    if (level.playerfrozen || m_bFrozen || (flags & FL_IMMOBILE)) {
        return false;
    }

    hand = WeaponHandNameToNum(condition.getParm(1));

    if (condition.numParms() > 1) {
        weaponName = condition.getParm(2);
    }

    if (hand == WEAPON_ERROR) {
        return false;
    }

    Weapon *weapon = GetActiveWeapon(hand);

    // Weapon there check
    if (!weapon) {
        return false;
    }

    // Name check
    if (condition.numParms() > 1) {
        if (strcmp(weaponName, weapon->item_name)) {
            return false;
        }
    }

    // Ammo check
    ready = weapon->ReadyToFire(mode);
    return (ready);
}

// Checks to see if weapon is active and ready to fire
qboolean Player::CondWeaponClassReadyToFire(Conditional& condition)
{
    firemode_t   mode        = FIRE_PRIMARY;
    str          weaponClass = "None";
    weaponhand_t hand;
    qboolean     ready;
    Weapon      *weapon;

    if (level.playerfrozen || m_bFrozen || (flags & FL_IMMOBILE)) {
        return false;
    }

    hand = WeaponHandNameToNum(condition.getParm(1));

    if (condition.numParms() > 1) {
        weaponClass = condition.getParm(2);
    }

    if (hand == WEAPON_ERROR) {
        return false;
    }

    weapon = GetActiveWeapon(hand);

    // Weapon there check
    if (!weapon) {
        return qfalse;
    }

    // Name check
    if (condition.numParms() > 1) {
        if (!(G_WeaponClassNameToNum(weaponClass) & weapon->GetWeaponClass())) {
            return qfalse;
        }

        if (condition.numParms() > 2) {
            mode = WeaponModeNameToNum(condition.getParm(3));
        }
    }

    // Ammo check
    ready = weapon->ReadyToFire(mode);
    return (ready);
}

qboolean Player::CondUsingVehicle(Conditional& condition)
{
    return (m_pVehicle != NULL);
}

qboolean Player::CondVehicleType(Conditional& condition)
{
    str sType = condition.getParm(1);
    if (m_pVehicle && m_pVehicle->IsSubclassOfVehicle()) {
        return !str::cmp(sType, "vehicle");
    } else {
        return !str::cmp(sType, "none");
    }
}

qboolean Player::CondIsPassenger(Conditional& condition)
{
    return m_pVehicle && m_pVehicle->IsSubclassOfVehicle() && m_pVehicle->FindPassengerSlotByEntity(this);
}

qboolean Player::CondIsDriver(Conditional& condition)
{
    return m_pVehicle && m_pVehicle->IsSubclassOfVehicle() && m_pVehicle->FindDriverSlotByEntity(this);
}

qboolean Player::CondUsingTurret(Conditional& condition)
{
    return (m_pTurret != NULL);
}

qboolean Player::CondIsEscaping(Conditional& condition)
{
    return m_jailstate == JAILSTATE_ESCAPE;
}

qboolean Player::CondAbleToDefuse(Conditional& condition)
{
    Weapon *weapon = GetActiveWeapon(WEAPON_MAIN);
    float   maxrange;

    if (!weapon) {
        weapon = GetActiveWeapon(WEAPON_OFFHAND);
    }

    Vector vForward, vRight, vUp;
    AngleVectors(m_vViewPos, vForward, vRight, vUp);

    maxrange = weapon->GetMaxRange();

    return FindDefusableObject(vForward, this, maxrange) ? true : false;
}

qboolean Player::CondCanPlaceLandmine(Conditional& condition)
{
    Weapon *weapon = GetActiveWeapon(WEAPON_MAIN);
    if (!weapon) {
        weapon = GetActiveWeapon(WEAPON_OFFHAND);
    }

    Vector vPos, vForward, vRight, vUp, vBarrel;
    weapon->GetMuzzlePosition(vPos, vBarrel, vForward, vRight, vUp);

    return CanPlaceLandmine(vPos, this);
}

qboolean Player::CondOnLandmine(Conditional& condition)
{
    MeasureLandmineDistances();

    return m_fMineDist <= 1.f;
}

qboolean Player::CondNearLandmine(Conditional& condition)
{
    MeasureLandmineDistances();

    return m_fMineDist < 3.f && m_fMineDist > 1.f;
}

void Player::MeasureLandmineDistances()
{
    // FIXME: unimplemented
}

qboolean Player::CondIsAssistingEscape(Conditional& condition)
{
    return m_jailstate == JAILSTATE_ASSIST_ESCAPE;
}

qboolean Player::CondTurretType(Conditional& condition)
{
    str name = condition.getParm(1);

    if (m_pTurret) {
        return m_pTurret->getName() == name;
    } else {
        return name == "none";
    }
}

qboolean Player::CondWeaponReadyToFireNoSound(Conditional& condition)
{
    firemode_t   mode       = FIRE_PRIMARY;
    str          weaponName = "None";
    weaponhand_t hand;
    qboolean     ready;

    if (level.playerfrozen || m_bFrozen || (flags & FL_IMMOBILE)) {
        return false;
    }

    hand = WeaponHandNameToNum(condition.getParm(1));

    if (condition.numParms() > 1) {
        weaponName = condition.getParm(2);
    }

    if (hand == WEAPON_ERROR) {
        return false;
    }

    Weapon *weapon = GetActiveWeapon(hand);

    // Weapon there check
    if (!weapon) {
        return qfalse;
    }

    // Name check
    if (condition.numParms() > 1) {
        if (strcmp(weaponName, weapon->item_name)) {
            return qfalse;
        }
    }

    // Ammo check
    ready = weapon->ReadyToFire(mode, qfalse);
    return (ready);
}

qboolean Player::CondPutAwayMain(Conditional& condition)
{
    Weapon *weapon = GetActiveWeapon(WEAPON_MAIN);

    return weapon && weapon->GetPutaway();
}

// Check to see if any of the active weapons need to be put away
qboolean Player::CondPutAwayOffHand(Conditional& condition)

{
    Weapon *weapon = GetActiveWeapon(WEAPON_OFFHAND);

    return weapon && weapon->GetPutaway();
}

// Checks to see if any weapon is active in the specified hand
qboolean Player::CondAnyWeaponActive(Conditional& condition)

{
    weaponhand_t hand;
    Weapon      *weap;

    hand = WeaponHandNameToNum(condition.getParm(1));

    if (hand == WEAPON_ERROR) {
        return false;
    }

    weap = GetActiveWeapon(hand);
    return (weap != NULL);
}

qboolean Player::CondAttackBlocked(Conditional& condition)

{
    if (attack_blocked) {
        attack_blocked = qfalse;
        return true;
    } else {
        return false;
    }
}

qboolean Player::CondSemiAuto(Conditional& condition)
{
    firemode_t   mode = FIRE_PRIMARY;
    str          handname;
    weaponhand_t hand;

    handname = condition.getParm(1);

    hand = WeaponHandNameToNum(handname);

    if (hand != WEAPON_ERROR) {
        return GetActiveWeapon(hand)->m_bSemiAuto;
    } else {
        return qfalse;
    }
}

qboolean Player::CondMinChargeTime(Conditional& condition)
{
    const char  *handname;
    weaponhand_t hand;
    Weapon      *weap;

    handname = condition.getParm(1);
    hand     = WeaponHandNameToNum(handname);

    if (hand != WEAPON_ERROR) {
        weap = GetActiveWeapon(hand);
        if (weap) {
            float charge_time = weap->GetMinChargeTime(FIRE_PRIMARY);
            if (charge_time) {
                if (charge_start_time) {
                    return level.time - charge_start_time >= charge_time;
                } else {
                    return qfalse;
                }
            } else {
                return qtrue;
            }
        }
    }

    return qfalse;
}

qboolean Player::CondMaxChargeTime(Conditional& condition)
{
    const char  *handname;
    weaponhand_t hand;
    Weapon      *weap;

    handname = condition.getParm(1);
    hand     = WeaponHandNameToNum(handname);

    if (hand != WEAPON_ERROR) {
        weap = GetActiveWeapon(hand);
        if (weap) {
            float charge_time = weap->GetMaxChargeTime(FIRE_PRIMARY);
            if (charge_time) {
                if (charge_start_time) {
                    return level.time - charge_start_time >= charge_time;
                } else {
                    return qfalse;
                }
            } else {
                return qtrue;
            }
        }
    }

    return qfalse;
}

qboolean Player::CondBlockDelay(Conditional& condition)

{
    float t = atof(condition.getParm(1));
    return (level.time > (attack_blocked_time + t));
}

qboolean Player::CondMuzzleClear(Conditional& condition)

{
    weaponhand_t hand;

    hand = WeaponHandNameToNum(condition.getParm(1));

    if (hand == WEAPON_ERROR) {
        return false;
    }

    Weapon *weapon = GetActiveWeapon(hand);
    return (weapon && weapon->MuzzleClear());
}

// Checks to see if any weapon is active in the specified hand
qboolean Player::CondWeaponHasAmmo(Conditional& condition)

{
    weaponhand_t hand;
    Weapon      *weap;
    firemode_t   mode = FIRE_PRIMARY;

    hand = WeaponHandNameToNum(condition.getParm(1));

    if (condition.numParms() > 1) {
        mode = WeaponModeNameToNum(condition.getParm(2));
    }

    if (hand == WEAPON_ERROR) {
        return false;
    }

    weap = GetActiveWeapon(hand);

    if (!weap) {
        return false;
    } else {
        return (weap->HasAmmo(mode));
    }
}

qboolean Player::CondWeaponHasAmmoInClip(Conditional& condition)
{
    weaponhand_t hand;
    Weapon      *weap;
    firemode_t   mode = FIRE_PRIMARY;

    hand = WeaponHandNameToNum(condition.getParm(1));

    if (condition.numParms() > 1) {
        mode = WeaponModeNameToNum(condition.getParm(2));
    }

    if (hand == WEAPON_ERROR) {
        return false;
    }

    weap = GetActiveWeapon(hand);

    if (!weap) {
        return false;
    } else {
        return (weap->HasAmmoInClip(mode));
    }
}

qboolean Player::CondReload(Conditional& condition)
{
    Weapon      *weapon;
    weaponhand_t hand = WEAPON_MAIN;

    if (condition.numParms() > 0) {
        hand = WeaponHandNameToNum(condition.getParm(1));
        if (hand == WEAPON_ERROR) {
            return qfalse;
        }
    }

    weapon = GetActiveWeapon(WEAPON_MAIN);

    if (!weapon) {
        return qfalse;
    }

    if (weapon->ShouldReload() && weapon->HasAmmo(FIRE_PRIMARY)) {
        return qtrue;
    }

    return qfalse;
}

qboolean Player::CondWeaponsHolstered(Conditional& condition)
{
    if (holsteredWeapon) {
        return qtrue;
    } else {
        return qfalse;
    }
}

qboolean Player::CondWeaponIsItem(Conditional& condition)
{
    weaponhand_t hand = WeaponHandNameToNum(condition.getParm(1));
    Weapon      *weapon;

    if (hand == WEAPON_ERROR) {
        return false;
    }

    weapon = GetActiveWeapon(hand);

    return weapon && weapon->IsSubclassOfInventoryItem();
}

qboolean Player::CondNewWeaponIsItem(Conditional& condition)
{
    Weapon *weapon = GetNewActiveWeapon();
    return weapon && weapon->IsSubclassOfInventoryItem();
}

qboolean Player::CondTurnLeft(Conditional& condition)
{
    float yaw;

    yaw = SHORT2ANGLE(last_ucmd.angles[YAW] + client->ps.delta_angles[YAW]);

    return (angledist(old_v_angle[YAW] - yaw) < -8.0f);
}

qboolean Player::CondTurnRight(Conditional& condition)
{
    float yaw;

    yaw = SHORT2ANGLE(last_ucmd.angles[YAW] + client->ps.delta_angles[YAW]);

    return (angledist(old_v_angle[YAW] - yaw) > 8.0f);
}

qboolean Player::CondForward(Conditional& condition)
{
    return last_ucmd.forwardmove > 0;
}

qboolean Player::CondBackward(Conditional& condition)
{
    return last_ucmd.forwardmove < 0;
}

qboolean Player::CondStrafeLeft(Conditional& condition)
{
    return last_ucmd.rightmove < 0;
}

qboolean Player::CondStrafeRight(Conditional& condition)

{
    return last_ucmd.rightmove > 0;
}

qboolean Player::CondJump(Conditional& condition)

{
    return last_ucmd.upmove > 0;
}

qboolean Player::CondCrouch(Conditional& condition)
{
    return (last_ucmd.upmove) < 0;
}

qboolean Player::CondJumpFlip(Conditional& condition)

{
    return velocity.z < (sv_gravity->value * 0.5f);
}

qboolean Player::CondAnimDoneLegs(Conditional& condition)
{
    return animdone_Legs;
}

qboolean Player::CondAnimDoneTorso(Conditional& condition)
{
    return animdone_Torso;
}

qboolean Player::CondAttackPrimary(Conditional& condition)
{
    if (level.playerfrozen || m_bFrozen || (flags & FL_IMMOBILE)) {
        return false;
    }

    if (g_gametype->integer != GT_SINGLE_PLAYER && !m_bAllowFighting) {
        return false;
    }

    if (last_ucmd.buttons & BUTTON_ATTACKLEFT) {
        Weapon *weapon;

        last_attack_button = BUTTON_ATTACKLEFT;

        weapon = GetActiveWeapon(WEAPON_MAIN);
        if (weapon) {
            return true;
        }

        // No ammo
        return false;
    } else {
        return false;
    }
}

qboolean Player::CondAttackButtonPrimary(Conditional& condition)
{
    if (level.playerfrozen || m_bFrozen || (flags & FL_IMMOBILE)) {
        return false;
    }

    if (g_gametype->integer != GT_SINGLE_PLAYER && !m_bAllowFighting) {
        return false;
    }

    return (last_ucmd.buttons & BUTTON_ATTACKLEFT);
}

qboolean Player::CondAttackSecondary(Conditional& condition)
{
    if (level.playerfrozen || m_bFrozen || (flags & FL_IMMOBILE)) {
        return false;
    }

    if (g_gametype->integer != GT_SINGLE_PLAYER && !m_bAllowFighting) {
        return false;
    }

    if (last_ucmd.buttons & BUTTON_ATTACKRIGHT) {
        Weapon *weapon;

        last_attack_button = BUTTON_ATTACKRIGHT;

        weapon = GetActiveWeapon(WEAPON_MAIN);
        if (weapon) {
            return true;
        }

        // No ammo
        return false;
    } else {
        return false;
    }
}

qboolean Player::CondAttackButtonSecondary(Conditional& condition)
{
    if (level.playerfrozen || m_bFrozen || (flags & FL_IMMOBILE)) {
        return false;
    }

    if (g_gametype->integer != GT_SINGLE_PLAYER && !m_bAllowFighting) {
        return false;
    }

    return (last_ucmd.buttons & BUTTON_ATTACKRIGHT);
}

qboolean Player::CondPositionType(Conditional& condition)
{
    int flags = 0;
    str s;

    s = condition.getParm(1);

    if (!s.icmp("crouching")) {
        flags = MPF_POSITION_CROUCHING;
    } else if (!s.icmp("prone")) {
        flags = MPF_POSITION_PRONE;
    } else if (!s.icmp("offground")) {
        flags = MPF_POSITION_OFFGROUND;
    } else {
        flags = MPF_POSITION_STANDING;
    }

    return (m_iMovePosFlags & flags);
}

qboolean Player::CondMovementType(Conditional& condition)
{
    int flags = 0;
    str s;

    s = condition.getParm(1);

    if (!s.icmp("walking")) {
        flags = MPF_MOVEMENT_WALKING;
    } else if (!s.icmp("running")) {
        flags = MPF_MOVEMENT_RUNNING;
    } else if (!s.icmp("falling")) {
        flags = MPF_MOVEMENT_FALLING;
    }

    return (m_iMovePosFlags & flags);
}

qboolean Player::CondRun(Conditional& condition)
{
    return (last_ucmd.buttons & BUTTON_RUN) != 0;
}

qboolean Player::CondUse(Conditional& condition)

{
    return (last_ucmd.buttons & BUTTON_USE) != 0;
}

qboolean Player::CondCanTurn(Conditional& condition)

{
    float    yaw;
    Vector   oldang(v_angle);
    qboolean result;

    yaw = atof(condition.getParm(1));

    v_angle[YAW] = (int)(anglemod(v_angle[YAW] + yaw) / 22.5f) * 22.5f;
    SetViewAngles(v_angle);

    result = CheckMove(vec_zero);

    SetViewAngles(oldang);

    return result;
}

qboolean Player::CondLeftVelocity(Conditional& condition)
{
    if (condition.numParms()) {
        return move_left_vel >= atof(condition.getParm(1));
    } else {
        return move_left_vel > 4.0f;
    }

    return qfalse;
}

qboolean Player::CondRightVelocity(Conditional& condition)
{
    if (condition.numParms()) {
        return move_right_vel >= atof(condition.getParm(1));
    } else {
        return move_right_vel > 4.0f;
    }

    return qfalse;
}

qboolean Player::CondBackwardVelocity(Conditional& condition)
{
    if (condition.numParms()) {
        return move_backward_vel >= atof(condition.getParm(1));
    } else {
        return move_backward_vel > 4.0f;
    }

    return qfalse;
}

qboolean Player::CondForwardVelocity(Conditional& condition)
{
    if (condition.numParms()) {
        return move_forward_vel >= atof(condition.getParm(1));
    } else {
        return move_forward_vel > 4.0f;
    }

    return qfalse;
}

qboolean Player::CondUpVelocity(Conditional& condition)
{
    if (condition.numParms()) {
        return move_up_vel >= atof(condition.getParm(1));
    } else {
        return move_up_vel > 4.0f;
    }

    return qfalse;
}

qboolean Player::CondDownVelocity(Conditional& condition)
{
    if (condition.numParms()) {
        return move_down_vel >= atof(condition.getParm(1));
    } else {
        return move_down_vel > 4.0f;
    }

    return qfalse;
}

qboolean Player::CondHasVelocity(Conditional& condition)
{
    float fSpeed;

    if (condition.numParms()) {
        fSpeed = atof(condition.getParm(1));
    } else {
        fSpeed = 4.0f;
    }

    return (
        (move_forward_vel > fSpeed) || (move_backward_vel > fSpeed) || (move_right_vel > fSpeed)
        || (move_left_vel > fSpeed)
    );
}

qboolean Player::Cond22DegreeSlope(Conditional& condition)

{
    if (client->ps.walking && client->ps.groundPlane && (client->ps.groundTrace.plane.normal[2] < SLOPE_22_MAX)
        && (client->ps.groundTrace.plane.normal[2] >= SLOPE_22_MIN)) {
        return qtrue;
    }

    return qfalse;
}

qboolean Player::Cond45DegreeSlope(Conditional& condition)

{
    if (client->ps.walking && client->ps.groundPlane && (client->ps.groundTrace.plane.normal[2] < SLOPE_45_MAX)
        && (client->ps.groundTrace.plane.normal[2] >= SLOPE_45_MIN)) {
        return qtrue;
    }

    return qfalse;
}

qboolean Player::CondRightLegHigh(Conditional& condition)

{
    float groundyaw;
    float yawdelta;
    int   which;

    groundyaw = (int)vectoyaw(client->ps.groundTrace.plane.normal);
    yawdelta  = anglemod(v_angle.y - groundyaw);
    which     = ((int)yawdelta + 45) / 90;

    return (which == 3);
}

qboolean Player::CondLeftLegHigh(Conditional& condition)

{
    float groundyaw;
    float yawdelta;
    int   which;

    groundyaw = (int)vectoyaw(client->ps.groundTrace.plane.normal);
    yawdelta  = anglemod(v_angle.y - groundyaw);
    which     = ((int)yawdelta + 45) / 90;

    return (which == 1);
}

qboolean Player::CondFacingUpSlope(Conditional& condition)

{
    float groundyaw;
    float yawdelta;
    int   which;

    groundyaw = (int)vectoyaw(client->ps.groundTrace.plane.normal);
    yawdelta  = anglemod(v_angle.y - groundyaw);
    which     = ((int)yawdelta + 45) / 90;

    return (which == 2);
}

qboolean Player::CondFacingDownSlope(Conditional& condition)

{
    float groundyaw;
    float yawdelta;
    int   which;

    groundyaw = (int)vectoyaw(client->ps.groundTrace.plane.normal);
    yawdelta  = anglemod(v_angle.y - groundyaw);
    which     = ((int)yawdelta + 45) / 90;

    return ((which == 0) || (which == 4));
}

qboolean Player::CondFalling(Conditional& condition)

{
    return falling;
}

qboolean Player::CondGroundEntity(Conditional& condition)

{
    return (groundentity != NULL);
}

qboolean Player::CondMediumImpact(Conditional& condition)
{
    return mediumimpact;
}

qboolean Player::CondHardImpact(Conditional& condition)
{
    return hardimpact;
}

qboolean Player::CondCanFall(Conditional& condition)
{
    return canfall;
}

qboolean Player::CondAtDoor(Conditional& condition)

{
    // Check if the player is at a door
    return (atobject && atobject->isSubclassOf(Door));
}

qboolean Player::CondAtUseAnim(Conditional& condition)

{
    // Check if the player is at a useanim
    if (atobject && atobject->isSubclassOf(UseAnim)) {
        return ((UseAnim *)(Entity *)atobject)->canBeUsed(this);
    }

    return false;
}

qboolean Player::CondTouchUseAnim(Conditional& condition)

{
    if (toucheduseanim) {
        return ((UseAnim *)(Entity *)toucheduseanim)->canBeUsed(this);
    }

    return qfalse;
}

qboolean Player::CondUseAnimFinished(Conditional& condition)

{
    return (useanim_numloops <= 0);
}

qboolean Player::CondAtUseObject(Conditional& condition)

{
    // Check if the player is at a useanim
    if (atobject && atobject->isSubclassOf(UseObject)) {
        return ((UseObject *)(Entity *)atobject)->canBeUsed(origin, yaw_forward);
    }

    return false;
}

qboolean Player::CondLoopUseObject(Conditional& condition)

{
    // Check if the player is at a useanim
    if (useitem_in_use && useitem_in_use->isSubclassOf(UseObject)) {
        return ((UseObject *)(Entity *)useitem_in_use)->Loop();
    }

    return false;
}

qboolean Player::CondDead(Conditional& condition)

{
    return (deadflag);
}

qboolean Player::CondKnockDown(Conditional& condition)

{
    if (knockdown) {
        knockdown = false;
        return true;
    } else {
        return false;
    }
}

qboolean Player::CondPainType(Conditional& condition)
{
    if (pain_type == MOD_string_to_int(condition.getParm(1))) {
        return qtrue;
    } else {
        return qfalse;
    }
}

qboolean Player::CondPainDirection(Conditional& condition)
{
    if (pain_dir == Pain_string_to_int(condition.getParm(1))) {
        return qtrue;
    } else {
        return qfalse;
    }
}

qboolean Player::CondPainLocation(Conditional& condition)
{
    str sLocationName;
    int iLocationNum;

    sLocationName = condition.getParm(1);

    if (!sLocationName.icmp("miss")) {
        iLocationNum = HITLOC_MISS;
    } else if (!sLocationName.icmp("general")) {
        iLocationNum = HITLOC_GENERAL;
    } else if (!sLocationName.icmp("head")) {
        iLocationNum = HITLOC_HEAD;
    } else if (!sLocationName.icmp("helmet")) {
        iLocationNum = HITLOC_HELMET;
    } else if (!sLocationName.icmp("neck")) {
        iLocationNum = HITLOC_NECK;
    } else if (!sLocationName.icmp("torso_upper")) {
        iLocationNum = HITLOC_TORSO_UPPER;
    } else if (!sLocationName.icmp("torso_mid")) {
        iLocationNum = HITLOC_TORSO_MID;
    } else if (!sLocationName.icmp("torso_lower")) {
        iLocationNum = HITLOC_TORSO_LOWER;
    } else if (!sLocationName.icmp("pelvis")) {
        iLocationNum = HITLOC_PELVIS;
    } else if (!sLocationName.icmp("r_arm_upper")) {
        iLocationNum = HITLOC_R_ARM_UPPER;
    } else if (!sLocationName.icmp("l_arm_upper")) {
        iLocationNum = HITLOC_L_ARM_UPPER;
    } else if (!sLocationName.icmp("r_leg_upper")) {
        iLocationNum = HITLOC_R_LEG_UPPER;
    } else if (!sLocationName.icmp("l_leg_upper")) {
        iLocationNum = HITLOC_L_LEG_UPPER;
    } else if (!sLocationName.icmp("r_arm_lower")) {
        iLocationNum = HITLOC_R_ARM_LOWER;
    } else if (!sLocationName.icmp("l_arm_lower")) {
        iLocationNum = HITLOC_L_ARM_LOWER;
    } else if (!sLocationName.icmp("r_leg_lower")) {
        iLocationNum = HITLOC_R_LEG_LOWER;
    } else if (!sLocationName.icmp("l_leg_lower")) {
        iLocationNum = HITLOC_L_LEG_LOWER;
    } else if (!sLocationName.icmp("r_hand")) {
        iLocationNum = HITLOC_R_HAND;
    } else if (!sLocationName.icmp("l_hand")) {
        iLocationNum = HITLOC_L_HAND;
    } else if (!sLocationName.icmp("r_foot")) {
        iLocationNum = HITLOC_R_FOOT;
    } else if (!sLocationName.icmp("l_foot")) {
        iLocationNum = HITLOC_L_FOOT;
    } else {
        Com_Printf("CondPainLocation: Unknown player hit location %s\n", sLocationName.c_str());
    }

    return (pain_location == iLocationNum);
}

qboolean Player::CondPainThreshold(Conditional& condition)

{
    float threshold = atof(condition.getParm(1));

    if ((pain >= threshold) && (level.time > nextpaintime)) {
        pain = 0; // zero out accumulation since we are going into a pain anim right now
        return true;
    } else {
        return false;
    }
}

qboolean Player::CondLegsState(Conditional& condition)

{
    if (currentState_Legs) {
        str current = currentState_Legs->getName();
        str compare = condition.getParm(1);

        if (current == compare) {
            return true;
        }
    }

    return false;
}

qboolean Player::CondTorsoState(Conditional& condition)

{
    if (currentState_Torso) {
        str current = currentState_Torso->getName();
        str compare = condition.getParm(1);

        if (current == compare) {
            return true;
        }
    }

    return false;
}

qboolean Player::CondStateName(Conditional& condition)

{
    str part      = condition.getParm(1);
    str statename = condition.getParm(2);

    if (currentState_Legs && !part.icmp("legs")) {
        return (!statename.icmpn(currentState_Legs->getName(), statename.length()));
    } else if (!part.icmp("torso")) {
        return (!statename.icmpn(currentState_Torso->getName(), statename.length()));
    }

    return false;
}

qboolean Player::CondPush(Conditional& condition)

{
    // Check if the player is at a pushobject
    if (atobject && atobject->isSubclassOf(PushObject) && (atobject_dist < (PUSH_OBJECT_DISTANCE + 15.0f))) {
        Vector dir;

        dir = atobject_dir * 8.0f;
        return ((PushObject *)(Entity *)atobject)->canPush(dir);
    }

    return qfalse;
}

qboolean Player::CondPull(Conditional& condition)

{
    // Check if the player is at a pushobject
    if (atobject && atobject->isSubclassOf(PushObject) && (atobject_dist < (PUSH_OBJECT_DISTANCE + 15.0f))) {
        Vector dir;

        dir = atobject_dir * -64.0f;
        return ((PushObject *)(Entity *)atobject)->canPush(dir);
    }

    return qfalse;
}

#define LADDER_HAND_HEIGHT (MAXS_X - MINS_X)

qboolean Player::CondLadder(Conditional& condition)
{
    trace_t trace;
    Vector  forward;
    Vector  start, end;

    AngleVectors(m_vViewAng, forward, NULL, NULL);

    start = (m_vViewPos - forward * 12.0f);
    end   = (m_vViewPos + forward * 128.0f);

    trace = G_Trace(start, vec_zero, vec_zero, end, this, MASK_LADDER, qfalse, "checkladder");
    if (trace.fraction == 1.0f || !trace.ent || !trace.ent->entity || !trace.ent->entity->isSubclassOf(FuncLadder)) {
        return qfalse;
    }

    return ((FuncLadder *)trace.ent->entity)->CanUseLadder(this);
}

qboolean Player::CondTopOfLadder(Conditional& condition)
{
    if (!m_pLadder) {
        return false;
    }

    if (maxs[2] + origin[2] > m_pLadder->absmax[2]) {
        return true;
    }

    return false;
}

qboolean Player::CondOnLadder(Conditional& condition)
{
    return m_pLadder != NULL;
}

qboolean Player::CondCanClimbUpLadder(Conditional& condition)
{
    trace_t trace;
    Vector  fwd;
    Vector  vec;
    Vector  start, end;

    AngleVectorsLeft(angles, fwd, NULL, NULL);

    start = origin - fwd * 12.0f;
    start[2] += maxs[2] - 8.0f;

    end = start + fwd * 40.0f;

    // check the normal bounding box first and trace to that position
    trace = G_Trace(start, vec_zero, vec_zero, end, this, MASK_LADDER, qtrue, "Player::CondCanClimbUpLadder");
    if ((trace.fraction == 1.0f) || (!trace.ent) || (!trace.ent->entity)
        || (!trace.ent->entity->isSubclassOf(FuncLadder))) {
        return qfalse;
    }

    Vector vEnd = (origin + Vector(0, 0, 16));

    return G_SightTrace(origin, mins, maxs, vEnd, this, NULL, MASK_BEAM, qtrue, "Player::CondCanClimbUpLadder");
}

qboolean Player::CondCanClimbDownLadder(Conditional& condition)
{
    Vector vEnd = origin - Vector(0, 0, 16);

    return G_SightTrace(origin, mins, maxs, vEnd, this, NULL, MASK_BEAM, qtrue, "Player::CondCanClimbDownLadder");
}

qboolean Player::CondCanGetOffLadderTop(Conditional& condition)
{
    Vector  vForward, vStart, vEnd;
    trace_t trace;

    angles.AngleVectorsLeft(&vForward);

    vStart = origin - vForward * 12.0f;
    vStart[2] += maxs[2] - 8.0f;

    vEnd = vStart + vForward * 40.0f;

    trace = G_Trace(vStart, vec_zero, vec_zero, vEnd, this, MASK_LADDER, qtrue, "Player::CondCanGetOffLadderTop 1");

    if (trace.fraction >= 1.0f) {
        vStart = origin;

        vEnd = origin;
        vEnd[2] += 98.0f;

        if (G_SightTrace(vStart, mins, maxs, vEnd, this, NULL, MASK_BEAM, true, "Player::CondCanGetOffLadderTop 2")) {
            vStart = vEnd;
            vEnd   = vStart + yaw_forward * 16.0f;

            return G_SightTrace(
                vStart, mins, maxs, vEnd, this, NULL, MASK_BEAM, true, "Player::CondCanGetOffLadderTop 3"
            );
        }
    }

    return false;
}

qboolean Player::CondCanGetOffLadderBottom(Conditional& condition)
{
    Vector  vStart, vEnd;
    trace_t trace;

    vStart = origin;

    vEnd = origin;
    vEnd[2] -= 40.0f;

    trace = G_Trace(vStart, mins, maxs, vEnd, edict, MASK_BEAM, true, "Player::CondCangetoffladerbottom");

    if (trace.fraction != 1.0f) {
        return (trace.entityNum == ENTITYNUM_WORLD);
    }

    return false;
}

qboolean Player::CondLookingUp(Conditional& condition)
{
    float angle = 0 - atof(condition.getParm(1));

    return angle > m_vViewAng[0];
}

qboolean Player::CondCanStand(Conditional& condition)

{
    Vector  newmins(mins);
    Vector  newmaxs(maxs);
    trace_t trace;

    newmins[2] = MINS_Z;
    newmaxs[2] = MAXS_Z;

    trace = G_Trace(origin, newmins, newmaxs, origin, this, MASK_PLAYERSOLID, true, "checkcanstand");
    if (trace.startsolid) {
        return qfalse;
    }

    return qtrue;
}

qboolean Player::CondSolidForward(Conditional& condition)
{
    // Trace out forward to see if there is a solid ahead
    float  dist = atof(condition.getParm(1));
    Vector end(centroid + yaw_forward * dist);
    Vector vMins(mins.x, mins.y, -8);
    Vector vMaxs(maxs.x, maxs.y, 8);

    trace_t trace = G_Trace(centroid, vMins, vMaxs, end, this, MASK_SOLID, true, "Player::CondSolidforward");

    return (trace.fraction < 0.7f);
}

qboolean Player::CondCheckHeight(Conditional& condition)
{
    str     sHeight = condition.getParm(1);
    float   fHeight;
    Vector  newmaxs;
    trace_t trace;

    if (!sHeight.icmp("stand")) {
        fHeight = 94.0f;
    } else if (!sHeight.icmp("duckrun")) {
        fHeight = 60.0f;
    } else if (!sHeight.icmp("duck")) {
        fHeight = 54.0f;
    } else if (!sHeight.icmp("prone")) {
        fHeight = 20.0f;
    } else {
        fHeight = atoi(sHeight.c_str());
    }

    if (fHeight < 16.0f) {
        fHeight = 16.0f;
    }

    if (maxs[2] >= fHeight) {
        return true;
    } else {
        newmaxs    = maxs;
        newmaxs[2] = fHeight;

        trace = G_Trace(origin, mins, newmaxs, origin, edict, MASK_PLAYERSOLID, true, "Player::CondCheckHeight");

        if (trace.startsolid) {
            return false;
        } else {
            return true;
        }
    }
}

qboolean Player::CondViewInWater(Conditional& condition)
{
    return (gi.pointcontents(m_vViewPos, 0) & MASK_WATER) != 0;
}

qboolean Player::CondDuckedViewInWater(Conditional& condition)
{
    Vector vPos = origin;
    vPos[2] += 48.0f;

    return (gi.pointcontents(vPos, 0) & MASK_WATER) != 0;
}

qboolean Player::CondCheckMovementSpeed(Conditional& condition)
{
    weaponhand_t hand;
    Weapon      *weapon;

    hand = WeaponHandNameToNum(condition.getParm(1));
    if (hand == WEAPON_ERROR) {
        return false;
    }

    weapon = GetActiveWeapon(hand);
    if (!weapon) {
        return false;
    }

    if (weapon->m_fMaxFireMovement == 1.f) {
        return true;
    }

    return (velocity.length() / sv_runspeed->value) <= (weapon->m_fMaxFireMovement * weapon->m_fMovementSpeed + 0.1f);
}

qboolean Player::CondActionAnimDone(Conditional& condition)
{
    // was removed in mohaas 2.0
    return false;
}

qboolean Player::CondAnimDoneVM(Conditional& condition)
{
    return animDoneVM;
}

qboolean Player::CondClientCommand(Conditional& condition)
{
    str command = condition.getParm(1);

    if (!command.icmp(m_lastcommand)) {
        return qtrue;
    } else {
        return qfalse;
    }
}

qboolean Player::CondVMAnim(Conditional& condition)
{
    return condition.getParm(1) == m_sVMcurrent;
}

qboolean Player::CondVariable(Conditional& condition)
{
    // parameters
    str     var_name;
    str     value_operator;
    Player *player = (Player *)this;

    // variables
    int                 cmp_int = 0, var_int = 0;
    float               cmp_float = 0.0f, var_float = 0.0f;
    char               *cmp_str      = NULL;
    char               *var_str      = NULL;
    ScriptVariableList *variableList = NULL;
    ScriptVariable     *variable     = NULL;
    char                _operator[2];
    size_t              i, nLength;
    size_t              indexval = -1;
    int                 founds   = 0;
    qboolean            isString = qfalse, isFloat = qfalse, isInteger = qfalse;

    var_name       = condition.getParm(1);
    value_operator = condition.getParm(2);

    if (!var_name) {
        gi.Printf("Var_CompareValue : the variable was not specified !\n", condition.getName());
        return qfalse;
    } else if (!value_operator) {
        gi.Printf("Var_CompareValue : the value was not specified !\n", condition.getName());
        return qfalse;
    }

    nLength = value_operator.length();

    // Lookup for the operator, until we found one
    for (i = 0; i < nLength; i++) {
        if ((value_operator[i] == '<' && value_operator[i + 1] == '=')
            || (value_operator[i] == '>' && value_operator[i + 1] == '=')
            || (value_operator[i] == '=' && value_operator[i + 1] == '=')
            || (value_operator[i] == '!' && value_operator[i + 1] == '=') || value_operator[i] == '<'
            || value_operator[i] == '>' || value_operator[i] == '&') {
            if (indexval == -1) {
                indexval = i;
            }

            founds++;
        }
    }

    // Fail if we didn't found/found multiples operators
    if (!founds) {
        gi.Printf(
            "Var_CompareValue : unknown/no comparison/relational operator was specified (var_name=\"%s\"|value=\"%s\") "
            "!\n",
            var_name.c_str(),
            value_operator.c_str()
        );
        return qfalse;
    } else if (founds > 1) {
        gi.Printf(
            "Var_CompareValue : more than one operator was specified (var_name='%s'|value='%s') !\n",
            var_name.c_str(),
            value_operator.c_str()
        );
        return qfalse;
    }

    _operator[0] = value_operator[indexval];
    _operator[1] = value_operator[indexval + 1];

    // If this is not a greater/less than operator, then the loop
    // shouldn't encounter a part of the operator
    if ((_operator[0] == '<' && _operator[1] != '=') || (_operator[0] == '>' && _operator[1] != '=')) {
        i = indexval;
    } else {
        i = indexval + 2;
    }

    while ((value_operator[i] == ' ' || value_operator[i] == '\0') && i < nLength) {
        i++;
    }

    indexval = -1;
    founds   = 0;

    // Loop until we find a character after the operator
    for (; i < nLength; i++) {
        if (value_operator[i] != '\0' && value_operator[i] != ' ' && value_operator[i] != _operator[0]
            && value_operator[i] != _operator[1]) {
            if (indexval == -1) {
                indexval = i;
            }

            founds++;
        }
    }

    if (!founds) {
        gi.Printf(
            "Var_CompareValue : no value was specified after the operator ! (var_name=\"%s\") !\n", var_name.c_str()
        );
        return qfalse;
    }

    // Get the variable list from the player

    variableList = this->Vars();

    // Get the variable from the variable list
    variable = variableList->GetVariable(var_name);

    if (variable != NULL) {
        isFloat   = variable->GetType() == VARIABLE_FLOAT;
        isInteger = variable->GetType() == VARIABLE_INTEGER;
        isString  = variable->GetType() == VARIABLE_STRING || variable->GetType() == VARIABLE_CONSTSTRING;

        if (!isFloat && !isString && !isInteger) {
            gi.Printf(
                "Var_CompareValue : invalid type \"%s\" (%d) for variable \"%s\"\n",
                typenames[variable->GetType()],
                variable->GetType(),
                var_name.c_str()
            );
            return qfalse;
        }

        // Retrieve the values from the variable
        if (isFloat) {
            var_float = variable->floatValue();
        } else {
            var_int = variable->intValue();
        }
    }

    cmp_str = (char *)value_operator.c_str() + indexval;

    if (!isString) {
        cmp_int   = atoi(cmp_str);
        cmp_float = (float)atof(cmp_str);
    }

    // If this is a string, compare between the two strings
    if (isString) {
        if (_operator[0] == '=' && _operator[1] == '=') {
            // == (EQUAL TO) operator

            return strcmp(cmp_str, var_str) == 0;
        } else if (_operator[0] == '!' && _operator[1] == '=') {
            // != (NOT EQUAL TO) operator

            return strcmp(cmp_str, var_str) != 0;
        }
    }

    // Now compare between the two values with the right operator and return
    if (_operator[0] == '<') {
        // < (LESS THAN) operator

        if (isFloat) {
            return var_float < cmp_float;
        }

        return var_int < cmp_int;
    } else if (_operator[0] == '>') {
        // > (GREATER THAN) operator

        if (isFloat) {
            return var_float > cmp_float;
        }

        return var_int > cmp_int;
    } else if (_operator[0] == '<' && _operator[1] == '=') {
        // <= (LESS THAN OR EQUAL TO) operator

        if (isFloat) {
            return var_float <= cmp_float;
        }

        return var_int <= cmp_int;
    } else if (_operator[0] == '>' && _operator[1] == '=') {
        // >= (GREATER THAN OR EQUAL TO) operator

        if (isFloat) {
            return var_float >= cmp_float;
        }

        return var_int >= cmp_int;
    } else if (_operator[0] == '!' && _operator[1] == '=') {
        // != (NOT EQUAL TO) operator

        if (isFloat) {
            return var_float != cmp_float;
        }

        return var_int != cmp_int;
    } else if (_operator[0] == '=' && _operator[1] == '=') {
        // == (EQUAL TO) operator

        if (isFloat) {
            return var_float == cmp_float;
        }

        return var_int == cmp_int;
    } else if (_operator[0] == '&') {
        // & (BITWISE AND) operator

        return var_int & cmp_int;
    }

    return qtrue;
}

CLASS_DECLARATION(Class, Conditional, NULL) {
    {NULL, NULL}
};

Condition<Player> Player::Conditions[] = {
    {"default",                         &Player::CondTrue                    },
    {"CHANCE",                          &Player::CondChance                  },
    {"HEALTH",                          &Player::CondHealth                  },
    {"BLOCKED",                         &Player::CondBlocked                 },
    {"PAIN",                            &Player::CondPain                    },
    {"ONGROUND",                        &Player::CondOnGround                }, // Checks to see if the right attack button is pressed
    {"HAS_WEAPON",                      &Player::CondHasWeapon               },
    {"NEW_WEAPON",                      &Player::CondNewWeapon               },
    {"IMMEDIATE_SWITCH",                &Player::CondImmediateSwitch         },
    {"IS_NEW_WEAPON",                   &Player::CondUseWeapon               },
    {"IS_WEAPON_ACTIVE",                &Player::CondWeaponActive            },
    {"WEAPON_CURRENT_FIRE_ANIM",        &Player::CondWeaponCurrentFireAnim   },
    {"IS_WEAPON_READY_TO_FIRE",         &Player::CondWeaponReadyToFire       },
    {"IS_WEAPON_READY_TO_FIRE_NOSOUND", &Player::CondWeaponReadyToFireNoSound},
    {"PUTAWAYMAIN",                     &Player::CondPutAwayMain             },
    {"PUTAWAYLEFT",                     &Player::CondPutAwayOffHand          },
    {"ANY_WEAPON_ACTIVE",               &Player::CondAnyWeaponActive         },
    {"ATTACK_BLOCKED",                  &Player::CondAttackBlocked           },
    {"IS_WEAPON_SEMIAUTO",              &Player::CondSemiAuto                },
    {"MIN_CHARGE_TIME_MET",             &Player::CondMinChargeTime           },
    {"MAX_CHARGE_TIME_MET",             &Player::CondMaxChargeTime           },
    {"IS_NEW_WEAPONCLASS",              &Player::CondUseWeaponClass          },
    {"IS_WEAPONCLASS_ACTIVE",           &Player::CondWeaponClassActive       },
    {"IS_WEAPONCLASS_READY_TO_FIRE",    &Player::CondWeaponClassReadyToFire  },
    {"IS_USING_VEHICLE",                &Player::CondUsingVehicle            },
    {"VEHICLE_TYPE",                    &Player::CondVehicleType             },
    {"IS_PASSENGER",                    &Player::CondIsPassenger             },
    {"IS_DRIVER",                       &Player::CondIsDriver                },
    {"IS_USING_TURRET",                 &Player::CondUsingTurret             },
    {"TURRET_TYPE",                     &Player::CondTurretType              },
    {"BLOCK_DELAY",                     &Player::CondBlockDelay              },
    {"MUZZLE_CLEAR",                    &Player::CondMuzzleClear             },
    {"HAS_AMMO",                        &Player::CondWeaponHasAmmo           },
    {"HAS_AMMO_IN_CLIP",                &Player::CondWeaponHasAmmoInClip     },
    {"RELOAD",                          &Player::CondReload                  },
    {"WEAPONS_HOLSTERED",               &Player::CondWeaponsHolstered        },
    {"IS_WEAPON_AN_ITEM",               &Player::CondWeaponIsItem            },
    {"NEW_WEAPON_AN_ITEM",              &Player::CondNewWeaponIsItem         },
    {"POSITION_TYPE",                   &Player::CondPositionType            },
    {"MOVEMENT_TYPE",                   &Player::CondMovementType            },
    {"RUN",                             &Player::CondRun                     },
    {"USE",                             &Player::CondUse                     },
    {"LEFT",                            &Player::CondTurnLeft                },
    {"RIGHT",                           &Player::CondTurnRight               },
    {"FORWARD",                         &Player::CondForward                 },
    {"BACKWARD",                        &Player::CondBackward                },
    {"STRAFE_LEFT",                     &Player::CondStrafeLeft              },
    {"STRAFE_RIGHT",                    &Player::CondStrafeRight             },
    {"JUMP",                            &Player::CondJump                    },
    {"CROUCH",                          &Player::CondCrouch                  },
    {"DO_JUMP_FLIP",                    &Player::CondJumpFlip                },
    {"ANIMDONE_LEGS",                   &Player::CondAnimDoneLegs            },
    {"ANIMDONE_TORSO",                  &Player::CondAnimDoneTorso           },
    {"CAN_TURN",                        &Player::CondCanTurn                 },
    {"LEFT_VELOCITY",                   &Player::CondLeftVelocity            },
    {"RIGHT_VELOCITY",                  &Player::CondRightVelocity           },
    {"BACKWARD_VELOCITY",               &Player::CondBackwardVelocity        },
    {"FORWARD_VELOCITY",                &Player::CondForwardVelocity         },
    {"UP_VELOCITY",                     &Player::CondUpVelocity              },
    {"DOWN_VELOCITY",                   &Player::CondDownVelocity            },
    {"HAS_VELOCITY",                    &Player::CondHasVelocity             },
    {"SLOPE_22",                        &Player::Cond22DegreeSlope           },
    {"SLOPE_45",                        &Player::Cond45DegreeSlope           },
    {"LOOKING_UP",                      &Player::CondLookingUp               },
    {"RIGHT_LEG_HIGH",                  &Player::CondRightLegHigh            },
    {"LEFT_LEG_HIGH",                   &Player::CondLeftLegHigh             },
    {"CAN_FALL",                        &Player::CondCanFall                 },
    {"AT_DOOR",                         &Player::CondAtDoor                  },
    {"FALLING",                         &Player::CondFalling                 },
    {"MEDIUM_IMPACT",                   &Player::CondMediumImpact            },
    {"HARD_IMPACT",                     &Player::CondHardImpact              },
    {"KILLED",                          &Player::CondDead                    },
    {"PAIN_TYPE",                       &Player::CondPainType                },
    {"PAIN_DIRECTION",                  &Player::CondPainDirection           },
    {"PAIN_LOCATION",                   &Player::CondPainLocation            },
    {"PAIN_THRESHOLD",                  &Player::CondPainThreshold           },
    {"KNOCKDOWN",                       &Player::CondKnockDown               },
    {"LEGS",                            &Player::CondLegsState               },
    {"TORSO",                           &Player::CondTorsoState              },
    {"AT_USEANIM",                      &Player::CondAtUseAnim               },
    {"TOUCHEDUSEANIM",                  &Player::CondTouchUseAnim            },
    {"FINISHEDUSEANIM",                 &Player::CondUseAnimFinished         },
    {"AT_USEOBJECT",                    &Player::CondAtUseObject             },
    {"LOOP_USEOBJECT",                  &Player::CondLoopUseObject           },
    {"CAN_PUSH",                        &Player::CondPush                    },
    {"CAN_PULL",                        &Player::CondPull                    },
    {"AT_LADDER",                       &Player::CondLadder                  },
    {"AT_TOP_OF_LADDER",                &Player::CondTopOfLadder             },
    {"ON_LADDER",                       &Player::CondOnLadder                },
    {"CAN_CLIMB_UP_LADDER",             &Player::CondCanClimbUpLadder        },
    {"CAN_CLIMB_DOWN_LADDER",           &Player::CondCanClimbDownLadder      },
    {"CAN_GET_OFF_LADDER_TOP",          &Player::CondCanGetOffLadderTop      },
    {"CAN_GET_OFF_LADDER_BOTTOM",       &Player::CondCanGetOffLadderBottom   },
    {"CAN_STAND",                       &Player::CondCanStand                },
    {"FACING_UP_SLOPE",                 &Player::CondFacingUpSlope           },
    {"FACING_DOWN_SLOPE",               &Player::CondFacingDownSlope         },
    {"SOLID_FORWARD",                   &Player::CondSolidForward            },
    {"STATE_ACTIVE",                    &Player::CondStateName               },
    {"GROUNDENTITY",                    &Player::CondGroundEntity            },
    {"CHECK_HEIGHT",                    &Player::CondCheckHeight             },
    {"VIEW_IN_WATER",                   &Player::CondViewInWater             },
    {"DUCKED_VIEW_IN_WATER",            &Player::CondDuckedViewInWater       },
    {"IS_ESCAPING",                     &Player::CondIsEscaping              },
    {"IS_ASSISTING_ESCAPE",             &Player::CondIsAssistingEscape       },
    {"NEAR_LANDMINE",                   &Player::CondNearLandmine            },
    {"ON_LANDMINE",                     &Player::CondOnLandmine              },
    {"ABLE_TO_DEFUSE",                  &Player::CondAbleToDefuse            },
    {"CAN_PLACE_LANDMINE",              &Player::CondCanPlaceLandmine        },
    {"IS_USING_TURRET",                 &Player::CondUsingTurret             },
    {"ATTACK_PRIMARY",                  &Player::CondAttackPrimary
    }, // Checks to see if there is an active weapon as well as the button being pressed
    {"ATTACK_SECONDARY",                &Player::CondAttackSecondary
    }, // Checks to see if there is an active weapon as well as the button being pressed
    {"ATTACK_PRIMARY_BUTTON",           &Player::CondAttackButtonPrimary     }, // Checks to see if the left attack button is pressed
    {"ATTACK_SECONDARY_BUTTON",         &Player::CondAttackButtonSecondary   },
    {"CHECK_MOVEMENT_SPEED",            &Player::CondCheckMovementSpeed      },

 // Weapon conditions
    {"ANIMDONE_VM",                     &Player::CondAnimDoneVM              },
    {"CLIENT_COMMAND",                  &Player::CondClientCommand           },
    {"IS_VM_ANIM",                      &Player::CondVMAnim                  },
    {"VAR_OPERATOR",                    &Player::CondVariable                },
    {NULL,                              NULL                                 },
};
