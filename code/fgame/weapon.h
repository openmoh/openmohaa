/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

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

// weapon.h: Header file for Weapon class.  The weapon class is the base class for
// all weapons in Sin.  Any entity created from a class derived from the weapon
// class will be usable by any Sentient (players and monsters) as a weapon.
//

#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "g_local.h"
#include "item.h"
#include "ammo.h"
#include "queue.h"
#include "sentient.h"
#include "../fgame/scriptmaster.h"

extern Event EV_Weapon_Shoot;
extern Event EV_Weapon_SetAmmoClipSize;
extern Event EV_Weapon_SetAmmoInClip;
extern Event EV_Weapon_FireDelay;
extern Event EV_Weapon_SetProjectile;
extern Event EV_Weapon_SetBulletDamage;
extern Event EV_Weapon_SetBulletLarge;
extern Event EV_Weapon_SetTracerSpeed;
extern Event EV_Weapon_SetBulletKnockback;
extern Event EV_Weapon_SetBulletThroughWood;
extern Event EV_Weapon_SetBulletThroughMetal;
extern Event EV_Weapon_SetBulletCount;
extern Event EV_Weapon_SetBulletRange;
extern Event EV_Weapon_SetBulletSpread;
extern Event EV_Weapon_SetTracerFrequency;
extern Event EV_Weapon_GiveStartingAmmo;
extern Event EV_Weapon_SetMeansOfDeath;

typedef enum {
    FT_NONE,
    FT_BULLET,
    FT_FAKEBULLET,
    FT_PROJECTILE,
    FT_MELEE,
    FT_SPECIAL_PROJECTILE,
    FT_CLICKITEM,
    FT_HEAVY,
    FT_LANDMINE,
    FT_DEFUSE,
} firetype_t;

typedef enum {
    WEAPON_READY,
    WEAPON_FIRING,
    WEAPON_LOWERING,
    WEAPON_RAISING,
    WEAPON_HOLSTERED,
    WEAPON_RELOADING,
    WEAPON_CHANGING
} weaponstate_t;

typedef enum {
    RANGE_SHORT,
    RANGE_MEDIUM,
    RANGE_LONG,
    RANGE_SNIPER,
    RANGE_NUM_RANGES
} AIRanges;

#define INITIALIZE_WEAPONMODE_VAR(var, value)       \
    {                                               \
        int _ii;                                    \
        for (_ii = 0; _ii < MAX_FIREMODES; _ii++) { \
            var[_ii] = value;                       \
        }                                           \
    }

class Player;

class Weapon : public Item
{
protected:
    int        m_iAnimSlot;
    qboolean   attached;            // Is this weapon attached to something?
    float      nextweaponsoundtime; // The next time this weapon should sound off
    float      m_fLastFireTime;
    firemode_t m_eLastFireMode;
    str        current_attachToTag; // The current name of the tag to attach itself to on the owner
    str        attachToTag_main;    // Tag to use when weapon is wielded in the main hand
    str        attachToTag_offhand; // ...offhand hand
    str        holster_attachToTag; // Tag to use when weapon is put away
    float      lastScale;           // Used for attaching to holster
    Vector     lastAngles;          // Used for attaching to holster
    qboolean   lastValid;           // Used for attaching to holster
    qboolean   auto_putaway;        // Weapon will put itself away when out of ammo
    qboolean   use_no_ammo;         // Weapon will be able to be used when it has no ammo
    qboolean   crosshair;           // Whether or not to display a crosshair with this weapon
    int        m_iZoom;
    float      m_fZoomSpreadMult;
    qboolean   m_bAutoZoom;
    qboolean   m_bSemiAuto;

protected:
    float         maxrange;                // maximum effective firing distance (for AI)
    float         minrange;                // minimum safe firing distance (for AI)
    str           viewmodel;               // View model of the weapon (not used in FAKK)
    weaponstate_t weaponstate;             // current state of the weapon
    int           rank;                    // rank of the weapon (relative to other weapons)
    int           order;                   // The order of this weapon in the inventory
    SentientPtr   last_owner;              // The last owner of the weapon
    float         last_owner_trigger_time; // The time when the last owner may re-pickup this weapon
    qboolean      notdroppable;            // makes the weapon not able to be dropped
    int           aimanim;                 // The aim animation to use for this weapon (so it shoots straight)
    int           aimframe;                // The aim frame to use for this weapon (so it shoots straight)
    Vector        holsterOffset;           // Angles to set the weapon to when it's holstered
    Vector        holsterAngles;           // Angles to set the weapon to when it's holstered
    float         holsterScale;            // Scale the weapon should be set to when it's holstered
    float         next_noise_time;         // next time weapon will alert actors
    float         next_noammo_time;        // next time we can play out of ammo sound
    float         next_maxmovement_time;

    qboolean m_bShouldReload;

    // Starting loadout
    Container<str> m_additionalStartAmmoTypes;
    Container<int> m_additionalStartAmmoAmounts;
    Container<str> m_startItems;

    // Each of these arrays is used to describe the properties of the weapon
    // in its primary(index 0) and alternate(index 1) mode

    float          fire_delay[MAX_FIREMODES];
    str            ammo_type[MAX_FIREMODES];          // The type of ammo used
    int            ammorequired[MAX_FIREMODES];       // The amount of ammo required to fire this weapon
    int            startammo[MAX_FIREMODES];          // The starting amount of ammo when the weapon is picked up
    str            projectileModel[MAX_FIREMODES];    // The model of the projectile fired
    float          bulletdamage[MAX_FIREMODES];       // The amount of damate a single bullet causes
    float          bulletcount[MAX_FIREMODES];        // The number of bullets the weapon fires
    int            bulletlarge[MAX_FIREMODES];        // The number of bullets the weapon fires
    float          bulletrange[MAX_FIREMODES];        // The range of the bullet
    float          bulletknockback[MAX_FIREMODES];    // The amount of knockback a bullet causes
    float          bulletthroughwood[MAX_FIREMODES];  // The amount of knockback a bullet causes
    float          bulletthroughmetal[MAX_FIREMODES]; // The amount of knockback a bullet causes
    float          projectilespeed[MAX_FIREMODES];    // The speed of the projectile fired
    Vector         bulletspread[MAX_FIREMODES];       // The amount of spread bullets can have
    Vector         bulletspreadmax[MAX_FIREMODES];    // The max amount of spread bullets can have
    firetype_t     firetype[MAX_FIREMODES];           // The type of fire (projectile or bullet)
    int            ammo_clip_size[MAX_FIREMODES];     // The amount of rounds the clip can hold
    int            ammo_in_clip[MAX_FIREMODES];       // The current amount of ammo in the clip
    float          max_charge_time[MAX_FIREMODES];    // The max amount of time the weapon may be charged.
    float          min_charge_time[MAX_FIREMODES];    // The min amount of time the weapon may be charged.
    meansOfDeath_t meansofdeath[MAX_FIREMODES];       // The means of death for this mode
    qboolean       loopfire[MAX_FIREMODES];           // The weapon loopfires and will not idle when shooting
    str            worldhitspawn[MAX_FIREMODES];      // The models to spawn when the weapon strikes the world
    int            tracerfrequency[MAX_FIREMODES];
    int            tracercount[MAX_FIREMODES];
    float          tracerspeed[MAX_FIREMODES];
    Vector         viewkickmin[MAX_FIREMODES];
    Vector         viewkickmax[MAX_FIREMODES];
    qboolean       quiet[MAX_FIREMODES];
    float          m_fFireSpreadMultAmount[MAX_FIREMODES];
    float          m_fFireSpreadMultFalloff[MAX_FIREMODES];
    float          m_fFireSpreadMultCap[MAX_FIREMODES];
    float          m_fFireSpreadMultTimeCap[MAX_FIREMODES];
    float          m_fFireSpreadMultTime[MAX_FIREMODES];
    float          m_fFireSpreadMult[MAX_FIREMODES];
    qboolean       m_bShareClip;
    qboolean       m_bCanPartialReload;

    qboolean   autoaim;         // Whether or not the weapon will autoaim
    float      charge_fraction; // Fraction of a charge up time
    qboolean   putaway;         // This is set to true by the state system to signal a weapon to be putaway
    firemode_t firemodeindex;
    int        weapon_class;
    const_str  m_csWeaponGroup;
    //
    // Movement speed
    //
    float m_fMovementSpeed;
    float m_fMaxFireMovement;
    float m_fZoomMovement;
    //
    // Special sounds
    //
    str m_sAmmoPickupSound;
    str m_NoAmmoSound;
    str m_sMaxMovementSound;
    // Weapon subtype
    int m_iWeaponSubtype;
    //
    // Weapon animations
    //
    int m_iNumFireAnims;
    int m_iCurrentFireAnim;
    str m_sTagBarrel;
    //
    // Cook variables
    //
    float      m_fCookTime;
    firemode_t m_eCookModeIndex;
    qboolean   m_bSecondaryAmmoInHud;

public:
    //
    // Stats variables
    //
    int             m_iNumShotsFired;
    int             m_iNumHits;
    int             m_iNumHeadShots;
    int             m_iNumTorsoShots;
    int             m_iNumLeftLegShots;
    int             m_iNumRightLegShots;
    int             m_iNumGroinShots;
    int             m_iNumLeftArmShots;
    int             m_iNumRightArmShots;
    AIRanges        mAIRange;
    SafePtr<Entity> aim_target;

protected:
    friend class Player;
    friend class PlayerBot;
    friend class Sentient;

    void SetMaxRangeEvent(Event *ev);
    void SetMinRangeEvent(Event *ev);
    void SetSecondaryAmmo(const char *type, int amount, int startamount);

    void         DetachGun(void);
    void         AttachGun(weaponhand_t hand, qboolean holstering = qfalse);
    qboolean     IsSecondaryWeapon(void);
    void         PickupWeapon(Event *ev);
    void         DoneRaising(Event *ev);
    void         DoneFiring(Event *ev);
    void         Idle(Event *ev);
    void         IdleInit(Event *ev);
    void         FillAmmoClip(Event *ev);
    void         EmptyAmmoClip(Event *ev);
    void         AddToAmmoClip(Event *ev);
    void         DoneReloading(Event *ev);
    virtual void ApplyFireKickback(const Vector& org, float kickback);
    void         SetAimAnim(Event *ev);
    virtual void Shoot(Event *ev);
    void         Secondary(Event *ev);
    void         SetFireType(Event *ev);
    void         SetAIRange(Event *ev);
    void         SetProjectile(Event *ev);
    void         SetDMProjectile(Event *ev);
    void         SetBulletDamage(Event *ev);
    void         SetBulletLarge(Event *ev); // Added in 2.0
    void         SetTracerSpeed(Event *ev); // Added in 2.0
    void         SetDMBulletDamage(Event *ev);
    void         SetBulletRange(Event *ev);
    void         SetDMBulletRange(Event *ev);
    void         SetBulletKnockback(Event *ev);
    void         SetDMBulletKnockback(Event *ev);
    void         SetBulletThroughWood(Event *ev);  // Added in 2.0
    void         SetBulletThroughMetal(Event *ev); // Added in 2.0
    void         SetBulletCount(Event *ev);
    void         SetDMBulletCount(Event *ev);
    void         SetBulletSpread(Event *ev);
    void         SetDMBulletSpread(Event *ev);
    void         SetZoomSpreadMult(Event *ev);
    void         SetDMZoomSpreadMult(Event *ev);
    void         SetFireSpreadMult(Event *ev);
    void         SetDMFireSpreadMult(Event *ev);
    void         SetTracerFrequency(Event *ev);
    void         SetAutoPutaway(Event *ev);
    void         SetRange(Event *ev);
    void         SetWeaponType(Event *ev);
    void         SetWeaponGroup(Event *ev);
    void         SetUseNoAmmo(Event *ev);
    void         MainAttachToTag(Event *ev);
    void         OffHandAttachToTag(Event *ev);
    void         HolsterAttachToTag(Event *ev);
    void         SetHolsterOffset(Event *ev);
    void         SetHolsterAngles(Event *ev);
    void         SetHolsterScale(Event *ev);
    void         SetWorldHitSpawn(Event *ev);
    void         SetViewKick(Event *ev);
    void         SetMovementSpeed(Event *ev);
    void         SetDMMovementSpeed(Event *ev);
    void         SetMaxFireMovement(Event *ev); // Added in 2.0
    void         SetZoomMovement(Event *ev);    // Added in 2.0
    void         EventAmmoPickupSound(Event *ev);
    void         EventNoAmmoSound(Event *ev);
    void         EventMaxMovementSound(Event *ev); // Added in 2.0
    //====
    // Added in 2.0
    void SetNumFireAnims(Event *ev);
    void SetWeaponSubtype(Event *ev);
    void SetCookTime(Event *ev);
    void SetCurrentFireAnim(Event *ev);
    void SetSecondaryAmmoInHud(Event *ev);
    //====
    void ShowInfo(float fDot, float fDist) override;

public:
    CLASS_PROTOTYPE(Weapon);

    Weapon();
    Weapon(const char *file);
    ~Weapon();

    void Delete(void) override;
    int  GetRank(void);
    int  GetOrder(void);

    int GetWeaponClass(void) const;

    const_str GetWeaponGroup(void) const;

    void  SetItemSlot(int slot);
    void  SetRank(int order, int rank);
    void  SetRankEvent(Event *ev);
    float GetMaxRange(void);
    float GetMinRange(void);

    qboolean GetPutaway(void);

    void SetPutAway(qboolean p);

    void             SetMaxRange(float val);
    void             SetMinRange(float val);
    void             ForceIdle(void);
    virtual qboolean SetWeaponAnim(const char *anim, Event *ev = NULL);
    qboolean         SetWeaponAnim(const char *anim, Event        &ev);
    void             SetWeaponAnimEvent(Event *ev);
    void             SetWeaponIdleAnim(void);
    void             SetWeaponIdleAnimEvent(Event *ev);
    virtual void     StopWeaponAnim(void);

    void       SetAmmoRequired(Event *ev);
    void       SetDMAmmoRequired(Event *ev);
    void       SetStartAmmo(Event *ev);
    void       SetDMStartAmmo(Event *ev);
    int        GetStartAmmo(firemode_t mode);
    int        GetMaxAmmo(firemode_t mode);
    str        GetAmmoType(firemode_t mode);
    firetype_t GetFireType(firemode_t mode);
    void       SetAmmoType(Event *ev);
    void       SetAmmoAmount(int amount, firemode_t mode);
    void       UseAmmo(int amount, firemode_t mode);
    void       SetAmmoClipSize(Event *ev);
    void       SetAmmoInClip(Event *ev);
    void       SetShareClip(Event *ev);
    void       SetTagBarrel(const char *tagBarrel); // Added in 2.0
    str        GetTagBarrel() const;                // Added in 2.0
    void       SetModels(const char *world, const char *view);
    void       SetOwner(Sentient *ent) override;
    void       SetMaxChargeTime(Event *ev);
    void       SetMinChargeTime(Event *ev);
    float      GetMinChargeTime(firemode_t);
    float      GetMaxChargeTime(firemode_t);
    int        AmmoAvailable(firemode_t mode);
    qboolean   UnlimitedAmmo(firemode_t mode);
    qboolean   HasAmmo(firemode_t mode);
    qboolean   HasAmmoInClip(firemode_t mode);
    int        GetClipSize(firemode_t mode);

    qboolean CheckReload(firemode_t mode);
    qboolean ShouldReload(void);
    void     SetShouldReload(qboolean should_reload);
    void     StartReloading(void);

    virtual qboolean ReadyToFire(firemode_t mode, qboolean playsound = qtrue);
    qboolean         MuzzleClear(void);

    void         PutAway(void);
    qboolean     Drop(void) override;
    void         Fire(firemode_t mode);
    void         Charge(firemode_t mode);
    void         OnOverCookedWarning(Event *ev); // Added in 2.0
    void         OnOverCooked(Event *ev);        // Added in 2.0
    void         ReleaseFire(firemode_t mode, float chargetime);
    const char  *GetFireAnim() const; // Added in 2.0
    void         ClientFireDone(void);
    qboolean     Removable(void) override;
    qboolean     Pickupable(Entity *other) override;
    virtual void DetachFromOwner(void);
    void         AttachToOwner(weaponhand_t hand);
    void         WeaponSound(Event *ev);
    virtual void GetMuzzlePosition(
        vec3_t position, vec3_t vBarrelPos = NULL, vec3_t forward = NULL, vec3_t right = NULL, vec3_t up = NULL
    );
    qboolean AutoChange(void);
    int      ClipAmmo(firemode_t mode);
    qboolean IsDroppable(void);
    float    FireDelay(firemode_t mode);
    void     SetFireDelay(Event *ev);
    void     SetDMFireDelay(Event *ev);

    weaponstate_t GetState(void);
    void          ForceState(weaponstate_t state);

    void         NotDroppableEvent(Event *ev);
    void         GiveStartingAmmoToOwner(Event *ev);
    void         AutoAim(Event *ev);
    void         Crosshair(Event *ev);
    void         DMCrosshair(Event *ev);
    void         SetZoom(Event *ev);
    void         SetSemiAuto(Event *ev);
    void         AttachToHand(Event *ev);
    void         SetCantPartialReload(Event *ev);
    void         SetDMCantPartialReload(Event *ev);
    void         AddAdditionalStartAmmo(Event *ev); // Added in 2.0
    void         AddStartItem(Event *ev);           // Added in 2.0
    void         SetQuiet(Event *ev);
    void         SetLoopFire(Event *ev);
    virtual void SpecialFireProjectile(
        Vector pos, Vector forward, Vector right, Vector up, Entity *owner, str projectileModel, float charge_fraction
    );

    virtual void AttachToHolster(weaponhand_t hand);
    str          GetCurrentAttachToTag(void);
    void         SetCurrentAttachToTag(str s);
    str          GetHolsterTag(void);
    qboolean     GetUseNoAmmo(void);

    int            GetZoom(void);
    qboolean       GetAutoZoom(void);
    void           SetMeansOfDeath(Event *ev);
    meansOfDeath_t GetMeansOfDeath(firemode_t mode);
    void           SetAimTarget(Entity *);
    void           SetIdleState(int state); // Added in 2.0
    void           WorldHitSpawn(firemode_t mode, Vector org, Vector angles, float life);
    void           MakeNoise(Event *ev);
    void           FallingAngleAdjust(Event *ev);
    float          GetMovementSpeed() const;
    qboolean       GetUseCrosshair() const;

    virtual bool IsCarryableTurret() const; // Added in 2.0

    void Archive(Archiver& arc) override;

    //
    // Added in openmohaa
    //
    Listener  *GetScriptOwner(void) override;
    float      GetBulletRange(firemode_t firemode);
    firemode_t GetFireMode(void);
    qboolean   IsSemiAuto(void);
};

inline void Weapon::Archive(Archiver& arc)
{
    Item::Archive(arc);

    arc.ArchiveInteger(&m_iAnimSlot);
    arc.ArchiveBoolean(&attached);
    arc.ArchiveFloat(&nextweaponsoundtime);
    arc.ArchiveFloat(&m_fLastFireTime);
    ArchiveEnum(m_eLastFireMode, firemode_t);
    arc.ArchiveString(&current_attachToTag);
    arc.ArchiveString(&attachToTag_main);
    arc.ArchiveString(&attachToTag_offhand);
    arc.ArchiveString(&holster_attachToTag);
    arc.ArchiveFloat(&lastScale);
    arc.ArchiveVector(&lastAngles);
    arc.ArchiveBoolean(&lastValid);
    arc.ArchiveBoolean(&auto_putaway);
    arc.ArchiveBoolean(&use_no_ammo);
    arc.ArchiveBoolean(&crosshair);
    arc.ArchiveInteger(&m_iZoom);
    arc.ArchiveFloat(&m_fZoomSpreadMult);
    arc.ArchiveBoolean(&m_bAutoZoom);
    arc.ArchiveBoolean(&m_bSemiAuto);
    arc.ArchiveSafePointer(&aim_target);
    arc.ArchiveFloat(&maxrange);
    arc.ArchiveFloat(&minrange);
    arc.ArchiveString(&viewmodel);

    ArchiveEnum(weaponstate, weaponstate_t);

    arc.ArchiveInteger(&rank);
    arc.ArchiveInteger(&order);
    arc.ArchiveSafePointer(&last_owner);
    arc.ArchiveFloat(&last_owner_trigger_time);
    arc.ArchiveBoolean(&notdroppable);
    arc.ArchiveInteger(&aimanim);
    arc.ArchiveInteger(&aimframe);
    arc.ArchiveVector(&holsterOffset);
    arc.ArchiveVector(&holsterAngles);
    arc.ArchiveFloat(&holsterScale);
    arc.ArchiveFloat(&next_noise_time);
    arc.ArchiveFloat(&next_noammo_time);
    arc.ArchiveFloat(&next_maxmovement_time);

    arc.ArchiveBoolean(&m_bShouldReload);

    m_additionalStartAmmoTypes.Archive(arc);
    m_additionalStartAmmoTypes.Archive(arc);
    m_additionalStartAmmoAmounts.Archive(arc);

    arc.ArchiveFloat(&fire_delay[0]);
    arc.ArchiveFloat(&fire_delay[1]);
    arc.ArchiveString(&ammo_type[0]);
    arc.ArchiveString(&ammo_type[1]);
    arc.ArchiveInteger(&ammorequired[0]);
    arc.ArchiveInteger(&ammorequired[1]);
    arc.ArchiveInteger(&startammo[0]);
    arc.ArchiveInteger(&startammo[1]);
    arc.ArchiveString(&projectileModel[0]);
    arc.ArchiveString(&projectileModel[1]);
    arc.ArchiveFloat(&bulletdamage[0]);
    arc.ArchiveFloat(&bulletdamage[1]);
    arc.ArchiveInteger(&bulletlarge[0]);
    arc.ArchiveInteger(&bulletlarge[1]);
    arc.ArchiveFloat(&bulletcount[0]);
    arc.ArchiveFloat(&bulletcount[1]);
    arc.ArchiveFloat(&bulletrange[0]);
    arc.ArchiveFloat(&bulletrange[1]);
    arc.ArchiveFloat(&bulletknockback[0]);
    arc.ArchiveFloat(&bulletknockback[1]);
    arc.ArchiveFloat(&bulletthroughwood[0]);
    arc.ArchiveFloat(&bulletthroughwood[0]);
    arc.ArchiveFloat(&bulletthroughmetal[1]);
    arc.ArchiveFloat(&bulletthroughmetal[1]);
    arc.ArchiveFloat(&projectilespeed[0]);
    arc.ArchiveFloat(&projectilespeed[1]);
    arc.ArchiveVector(&bulletspread[0]);
    arc.ArchiveVector(&bulletspread[1]);
    arc.ArchiveVector(&bulletspreadmax[0]);
    arc.ArchiveVector(&bulletspreadmax[1]);
    arc.ArchiveString(&worldhitspawn[0]);
    arc.ArchiveString(&worldhitspawn[1]);
    arc.ArchiveInteger(&tracerfrequency[0]);
    arc.ArchiveInteger(&tracerfrequency[1]);
    arc.ArchiveInteger(&tracercount[0]);
    arc.ArchiveInteger(&tracercount[1]);
    arc.ArchiveFloat(&tracerspeed[0]);
    arc.ArchiveFloat(&tracerspeed[1]);
    arc.ArchiveVector(&viewkickmin[0]);
    arc.ArchiveVector(&viewkickmin[1]);
    arc.ArchiveVector(&viewkickmax[0]);
    arc.ArchiveVector(&viewkickmax[1]);
    arc.ArchiveBoolean(&quiet[0]);
    arc.ArchiveBoolean(&quiet[1]);
    arc.ArchiveFloat(&m_fFireSpreadMultAmount[0]);
    arc.ArchiveFloat(&m_fFireSpreadMultAmount[1]);
    arc.ArchiveFloat(&m_fFireSpreadMultFalloff[0]);
    arc.ArchiveFloat(&m_fFireSpreadMultFalloff[1]);
    arc.ArchiveFloat(&m_fFireSpreadMultCap[0]);
    arc.ArchiveFloat(&m_fFireSpreadMultCap[1]);
    arc.ArchiveFloat(&m_fFireSpreadMultTimeCap[0]);
    arc.ArchiveFloat(&m_fFireSpreadMultTimeCap[1]);
    arc.ArchiveFloat(&m_fFireSpreadMultTime[0]);
    arc.ArchiveFloat(&m_fFireSpreadMultTime[1]);
    arc.ArchiveFloat(&m_fFireSpreadMult[0]);
    arc.ArchiveFloat(&m_fFireSpreadMult[1]);

    ArchiveEnum(firetype[0], firetype_t);
    ArchiveEnum(firetype[1], firetype_t);

    arc.ArchiveInteger(&ammo_clip_size[0]);
    arc.ArchiveInteger(&ammo_clip_size[1]);
    arc.ArchiveInteger(&ammo_in_clip[0]);
    arc.ArchiveInteger(&ammo_in_clip[1]);
    arc.ArchiveFloat(&max_charge_time[0]);
    arc.ArchiveFloat(&max_charge_time[1]);
    arc.ArchiveFloat(&min_charge_time[0]);
    arc.ArchiveFloat(&min_charge_time[1]);

    ArchiveEnum(meansofdeath[0], meansOfDeath_t);
    ArchiveEnum(meansofdeath[1], meansOfDeath_t);

    arc.ArchiveBoolean(&loopfire[0]);
    arc.ArchiveBoolean(&loopfire[1]);
    arc.ArchiveBoolean(&m_bShareClip);
    arc.ArchiveBoolean(&m_bCanPartialReload);
    arc.ArchiveBoolean(&autoaim);
    arc.ArchiveFloat(&charge_fraction);
    arc.ArchiveBoolean(&putaway);

    ArchiveEnum(firemodeindex, firemode_t);

    arc.ArchiveInteger(&weapon_class);
    Director.ArchiveString(arc, m_csWeaponGroup);
    arc.ArchiveFloat(&m_fMovementSpeed);
    arc.ArchiveFloat(&m_fMaxFireMovement);
    arc.ArchiveFloat(&m_fZoomMovement);
    arc.ArchiveString(&m_sAmmoPickupSound);
    arc.ArchiveString(&m_NoAmmoSound);
    arc.ArchiveString(&m_sMaxMovementSound);
    arc.ArchiveInteger(&m_iNumShotsFired);
    arc.ArchiveInteger(&m_iNumHits);
    arc.ArchiveInteger(&m_iNumHeadShots);
    arc.ArchiveInteger(&m_iNumTorsoShots);
    arc.ArchiveInteger(&m_iNumLeftLegShots);
    arc.ArchiveInteger(&m_iNumRightLegShots);
    arc.ArchiveInteger(&m_iNumGroinShots);
    arc.ArchiveInteger(&m_iNumLeftArmShots);
    arc.ArchiveInteger(&m_iNumRightArmShots);
    arc.ArchiveInteger(&m_iNumFireAnims);
    arc.ArchiveInteger(&m_iCurrentFireAnim);
    arc.ArchiveString(&m_sTagBarrel);

    ArchiveEnum(mAIRange, AIRanges);

    arc.ArchiveInteger(&m_iWeaponSubtype);
    arc.ArchiveFloat(&m_fCookTime);
    ArchiveEnum(m_eCookModeIndex, firemode_t);
    arc.ArchiveBoolean(&m_bSecondaryAmmoInHud);
}

typedef SafePtr<Weapon> WeaponPtr;

#endif /* weapon.h */
