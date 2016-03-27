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
#include <scriptmaster.h>

extern Event EV_Weapon_GiveStartingAmmo;
extern Event EV_Weapon_SetAmmoInClip;
extern Event EV_Weapon_Shoot;

#define WEAPON_CLASS_PISTOL			(1<<0)
#define WEAPON_CLASS_RIFLE			(1<<1)
#define WEAPON_CLASS_SMG			(1<<2)
#define WEAPON_CLASS_MG				(1<<3)
#define WEAPON_CLASS_GRENADE		(1<<4)
#define WEAPON_CLASS_HEAVY			(1<<5)
#define WEAPON_CLASS_CANNON			(1<<6)
#define WEAPON_CLASS_ITEM			(1<<7)
#define WEAPON_CLASS_ITEM1			(1<<8)
#define WEAPON_CLASS_ITEM2			(1<<9)
#define WEAPON_CLASS_ITEM3			(1<<10)
#define WEAPON_CLASS_ITEM4			(1<<11)

#define WEAPON_CLASS_PRIMARY		(!(WEAPON_CLASS_PISTOL|WEAPON_CLASS_GRENADE))
#define WEAPON_CLASS_SECONDARY		(WEAPON_CLASS_PISTOL|WEAPON_CLASS_GRENADE)
#define WEAPON_CLASS_THROWABLE		(WEAPON_CLASS_GRENADE|WEAPON_CLASS_ITEM)
#define WEAPON_CLASS_MISC			(WEAPON_CLASS_CANNON|WEAPON_CLASS_ITEM|WEAPON_CLASS_ITEM1|WEAPON_CLASS_ITEM2|WEAPON_CLASS_ITEM3|WEAPON_CLASS_ITEM4)
#define WEAPON_CLASS_ITEMINDEX		(WEAPON_CLASS_ITEM1|WEAPON_CLASS_ITEM2|WEAPON_CLASS_ITEM3|WEAPON_CLASS_ITEM4)

typedef enum
{
	FT_NONE,
	FT_BULLET,
	FT_FAKEBULLET,
	FT_PROJECTILE,
	FT_MELEE,
	FT_SPECIAL_PROJECTILE,
	FT_CLICKITEM,
	FT_HEAVY
} firetype_t;

typedef enum
{
	WEAPON_READY,
	WEAPON_FIRING,
	WEAPON_LOWERING,
	WEAPON_RAISING,
	WEAPON_HOLSTERED,
	WEAPON_RELOADING,
	WEAPON_CHANGING
} weaponstate_t;

typedef enum
{
	RANGE_SHORT,
	RANGE_MEDIUM,
	RANGE_LONG,
	RANGE_SNIPER,
	RANGE_NUM_RANGES
} AIRanges;

#define INITIALIZE_WEAPONMODE_VAR(var,value) \
   { \
   int _ii; \
   for( _ii=0; _ii<MAX_FIREMODES; _ii++ ) \
      { \
      var[_ii] = value; \
      } \
   }

class Player;

class Weapon : public Item
	{
	protected:
		int					m_iAnimSlot;
		qboolean					attached;                  // Is this weapon attached to something?
		float				nextweaponsoundtime;       // The next time this weapon should sound off
		float				m_fLastFireTime;
		str                  current_attachToTag;       // The current name of the tag to attach itself to on the owner      
		str                  attachToTag_main;          // Tag to use when weapon is wielded in the main hand
		str                  attachToTag_offhand;       // ...offhand hand
		str                  holster_attachToTag;		// Tag to use when weapon is put away
		float                lastScale;                 // Used for attaching to holster
		Vector               lastAngles;                // Used for attaching to holster
		qboolean             lastValid;                 // Used for attaching to holster
		qboolean             auto_putaway;              // Weapon will put itself away when out of ammo
		qboolean             use_no_ammo;               // Weapon will be able to be used when it has no ammo
		qboolean             crosshair;                 // Whether or not to display a crosshair with this weapon
		int					m_iZoom;
		float				m_fZoomSpreadMult;
		qboolean			m_bAutoZoom;
		qboolean			m_bSemiAuto;

	protected:
		float						maxrange;				      // maximum effective firing distance (for AI)
		float						minrange;				      // minimum safe firing distance (for AI)
		str						viewmodel;                 // View model of the weapon (not used in FAKK)
		weaponstate_t			weaponstate;               // current state of the weapon
		int						rank;                      // rank of the weapon (relative to other weapons)
		int						order;                     // The order of this weapon in the inventory
		SentientPtr				last_owner;                // The last owner of the weapon
		float						last_owner_trigger_time;   // The time when the last owner may re-pickup this weapon
		qboolean             notdroppable;              // makes the weapon not able to be dropped
		int                  aimanim;                   // The aim animation to use for this weapon (so it shoots straight)
		int                  aimframe;                  // The aim frame to use for this weapon (so it shoots straight)
		Vector               holsterOffset;				// Angles to set the weapon to when it's holstered
		Vector               holsterAngles;				// Angles to set the weapon to when it's holstered
		float                holsterScale;              // Scale the weapon should be set to when it's holstered
		float				next_noise_time;           // next time weapon will alert actors
		float				next_noammo_time;          // next time we can play out of ammo sound

		qboolean			m_bShouldReload;
		// Each of these arrays is used to describe the properties of the weapon 
		// in its primary(index 0) and alternate(index 1) mode

		float					firedelay[ MAX_FIREMODES ];
		str                  ammo_type[ MAX_FIREMODES ];       // The type of ammo used
		int						ammorequired[ MAX_FIREMODES ];    // The amount of ammo required to fire this weapon
		int						startammo[ MAX_FIREMODES ];       // The starting amount of ammo when the weapon is picked up
		str                  projectileModel[ MAX_FIREMODES ]; // The model of the projectile fired
		float                bulletdamage[ MAX_FIREMODES ];    // The amount of damate a single bullet causes
		float                bulletcount[ MAX_FIREMODES ];     // The number of bullets the weapon fires
		float                bulletrange[ MAX_FIREMODES ];     // The range of the bullet
		float                bulletknockback[ MAX_FIREMODES ]; // The amount of knockback a bullet causes
		float						projectilespeed[ MAX_FIREMODES ]; // The speed of the projectile fired
		Vector               bulletspread[ MAX_FIREMODES ];    // The amount of spread bullets can have
		Vector               bulletspreadmax[ MAX_FIREMODES ];    // The max amount of spread bullets can have
		firetype_t           firetype[ MAX_FIREMODES ];        // The type of fire (projectile or bullet)
		int						ammo_clip_size[ MAX_FIREMODES ];  // The amount of rounds the clip can hold
		int						ammo_in_clip[ MAX_FIREMODES ];    // The current amount of ammo in the clip
		float                max_charge_time[ MAX_FIREMODES ]; // The max amount of time the weapon may be charged.
		float                min_charge_time[ MAX_FIREMODES ]; // The min amount of time the weapon may be charged.
		meansOfDeath_t       meansofdeath[ MAX_FIREMODES ];	  // The means of death for this mode
		qboolean             loopfire[ MAX_FIREMODES ];        // The weapon loopfires and will not idle when shooting
		int                  action_level_increment[ MAX_FIREMODES ]; // Increments the action level everytime the weapon is fired
		str                  worldhitspawn[ MAX_FIREMODES ];   // The models to spawn when the weapon strikes the world
		int					tracerfrequency[ 2 ];
		int					tracercount[ 2 ];
		Vector				viewkickmin[ 2 ];
		Vector				viewkickmax[ 2 ];
		qboolean			quiet[ 2 ];
		float				m_fFireSpreadMultAmount[ 2 ];
		float				m_fFireSpreadMultFalloff[ 2 ];
		float				m_fFireSpreadMultCap[ 2 ];
		float				m_fFireSpreadMultTimeCap[ 2 ];
		float				m_fFireSpreadMultTime[ 2 ];
		float				m_fFireSpreadMult[ 2 ];
		qboolean			m_bShareClip;
		qboolean			m_bCanPartialReload;

		qboolean			autoaim;                        // Whether or not the weapon will autoaim
		float				charge_fraction;                // Fraction of a charge up time 
		qboolean			putaway;                        // This is set to true by the state system to signal a weapon to be putaway
		firemode_t			firemodeindex;
		int					weapon_class;
		const_str			m_csWeaponGroup;
		float				m_fMovementSpeed;
		str					m_sAmmoPickupSound;
		str					m_NoAmmoSound;

	public:
		int					m_iNumShotsFired;
		int					m_iNumHits;
		float				m_fAccuracy;
		float				m_fTimeUsed;
		int					m_iNumHeadShots;
		int					m_iNumTorsoShots;
		int					m_iNumLeftLegShots;
		int					m_iNumRightLegShots;
		int					m_iNumGroinShots;
		int					m_iNumLeftArmShots;
		int					m_iNumRightArmShots;
		AIRanges			mAIRange;
		SafePtr< Entity >	aim_target;

      void						SetMaxRangeEvent( Event *ev );  
      void						SetMinRangeEvent( Event *ev );		
      void                 SetSecondaryAmmo( const char *type, int amount, int startamount );
      friend               class Player;
      void			         DetachGun( void );
      void			         AttachGun( weaponhand_t hand, qboolean holstering=qfalse );
      void			         PickupWeapon( Event *ev );
      void			         DoneRaising( Event *ev );
      void			         DoneFiring( Event *ev );
	  void			         Idle( Event *ev );
	  void			         IdleInit( Event *ev );
      qboolean		         CheckReload( firemode_t mode );
      void			         DoneReloading( Event *ev );
      void					SetAimAnim( Event *ev );
      virtual void			Shoot( Event *ev );      
      void					Secondary( Event *ev );
	  void					SetFireType( Event *ev );
	  void					SetAIRange( Event *ev );
	  void					SetProjectile( Event *ev );
	  void					SetDMProjectile( Event *ev );
	  void					SetBulletDamage( Event *ev );
	  void					SetDMBulletDamage( Event *ev );
	  void					SetBulletRange( Event *ev );
	  void					SetDMBulletRange( Event *ev );
	  void					SetBulletKnockback( Event *ev );
	  void					SetDMBulletKnockback( Event *ev );
	  void					SetBulletCount( Event *ev );
	  void					SetDMBulletCount( Event *ev );
	  void					SetBulletSpread( Event *ev );
	  void					SetDMBulletSpread( Event *ev );
	  void					SetZoomSpreadMult( Event *ev );
	  void					SetDMZoomSpreadMult( Event *ev );
	  void					SetFireSpreadMult( Event *ev );
	  void					SetDMFireSpreadMult( Event *ev );
	  void					SetTracerFrequency( Event *ev );
      void					SetAutoPutaway( Event *ev );
      void					SetRange( Event *ev );
      void					SetSpecialMove( Event *ev );
      void					SetUseNoAmmo( Event *ev );
      void					OffHandAttachToTag( Event *ev );
      void					MainAttachToTag( Event *ev );
	  void					HolsterAttachToTag( Event *ev );
	  void					SetHolsterOffset( Event *ev );
      void					SetHolsterAngles( Event *ev );
      void					SetHolsterScale( Event *ev );
	  void					SetWorldHitSpawn( Event *ev );
	  void					SetViewKick( Event *ev );
	  void					SetMovementSpeed( Event *ev );
	  void					SetDMMovementSpeed( Event *ev );
	  void					EventAmmoPickupSound( Event *ev );
	  void					EventNoAmmoSound( Event *ev );
	  void					SetWeaponGroup( Event *ev );
	  void					SetWeaponType( Event *ev );
	  void					EventClipAdd( Event *ev );
	  void					EventClipEmpty( Event *ev );
	  void					EventClipFill( Event *ev );
	  float					GetBulletRange( firemode_t firemode );

   public:
      CLASS_PROTOTYPE( Weapon );

						         Weapon();
                           Weapon( const char *file );
				               ~Weapon();

	virtual void				Delete( void );
	virtual Listener			*GetScriptOwner( void );
      int				      GetRank( void );
      int				      GetOrder( void );
	  int					GetWeaponClass( void ) const { return weapon_class; };
	  const_str				GetWeaponGroup( void ) const { return m_csWeaponGroup; };
      void		   	      SetRank( int order, int rank );
	  void					SetRankEvent( Event *ev );
      float				      GetMaxRange( void );
      float				      GetMinRange( void );
      inline qboolean      GetPutaway( void ){ return putaway; };
      inline void          SetPutAway( qboolean p ){ putaway = p; };
      void				      SetMaxRange( float val );
      void				      SetMinRange( float val );
	  qboolean				IsSecondaryWeapon( void );
      void					ForceIdle( void );
	  virtual qboolean		SetWeaponAnim( const char *anim, Event *ev = NULL );
	  qboolean				SetWeaponAnim( const char *anim, Event& ev );
	  void					SetWeaponAnimEvent( Event *ev );
	  void					SetWeaponIdleAnim( void );
	  void					SetWeaponIdleAnimEvent( Event *ev );
	  virtual void			StopWeaponAnim( void );

	  void                 SetAmmoRequired( Event *ev );
	  void                 SetDMAmmoRequired( Event *ev );
	  void                 SetStartAmmo( Event *ev );
	  void                 SetDMStartAmmo( Event *ev );
      int                  GetStartAmmo( firemode_t mode );
      int                  GetMaxAmmo( firemode_t mode );
      str                  GetAmmoType( firemode_t mode );
      firetype_t           GetFireType( firemode_t mode );
	  firemode_t			GetFireMode( void );
      void                 SetAmmoType( Event *ev );      
      void			         SetAmmoAmount( int amount, firemode_t mode );
      void                 UseAmmo( int amount, firemode_t mode );
      void			         SetAmmoClipSize( Event *ev );
	  void                 SetAmmoInClip( Event *ev );
	  void                 SetShareClip( Event *ev );
      void			         SetModels( const char *world, const char *view );
      void			         SetOwner( Sentient *ent );
	  void					SetMaxChargeTime( Event *ev );
	  void					SetMinChargeTime( Event *ev );
	  float					GetMinChargeTime( firemode_t );
	  float					GetMaxChargeTime( firemode_t );
      int				      AmmoAvailable( firemode_t mode );
      qboolean             UnlimitedAmmo( firemode_t mode );
      qboolean		         HasAmmo( firemode_t mode );
      qboolean             HasAmmoInClip( firemode_t mode );
	  qboolean				IsSemiAuto( void );
      int                  GetClipSize( firemode_t mode );
      qboolean		         ReadyToFire( firemode_t mode, qboolean playsound=qtrue );
      qboolean		         MuzzleClear( void );
      void			         PutAway( void );
      qboolean		         Drop( void );
      void			         Fire( firemode_t mode );
      void                 Charge( firemode_t mode );
      void                 ReleaseFire( firemode_t mode, float chargetime );
      void			         ClientFireDone( void );
      qboolean	            Removable( void );
      qboolean             Pickupable( Entity *other );
      void                 DetachFromOwner( void );
      void                 AttachToOwner( weaponhand_t hand );
      void			         WeaponSound( Event *ev );
      void			         GetMuzzlePosition( Vector *position, Vector *forward = NULL, Vector *right = NULL, Vector *up = NULL, Vector *vBarrelPos = NULL );
      qboolean             AutoChange( void );
      int                  ClipAmmo( firemode_t mode );
      void                 ProcessWeaponCommandsEvent(Event *ev);
	  qboolean             IsDroppable( void );
	  float					FireDelay( firemode_t mode );
	  void					EventSetFireDelay( Event *ev );
	  void					EventSetDMFireDelay( Event *ev );
	  qboolean				HasCrosshair() { return crosshair; }
      int                  ActionLevelIncrement( firemode_t mode );
	  void                 SetActionLevelIncrement( Event *ev );
	  weaponstate_t			GetState( void );
      void                 ForceState( weaponstate_t state );
      void                 NotDroppableEvent( Event *ev );
      void                 GiveStartingAmmoToOwner( Event *ev );
      void                 AutoAim( Event *ev );
	  void                 Crosshair( Event *ev );
	  void                 DMCrosshair( Event *ev );
	  void                 SetZoom( Event *ev );
	  void                 SetSemiAuto( Event *ev );
	  void                 AttachToHand( Event *ev );
	  void                 SetCantPartialReload( Event *ev );
	  void                 SetDMCantPartialReload( Event *ev );
		void                 SetQuiet( Event *ev );
      void                 SetLoopFire( Event *ev );
      inline virtual void  SpecialFireProjectile( Vector pos, Vector forward, Vector right, Vector up, Entity *owner, str projectileModel, float charge_fraction ) {};
      void                 AttachToHolster( weaponhand_t hand );
      inline str           GetCurrentAttachToTag( void ){ return current_attachToTag; };
      inline void          SetCurrentAttachToTag( str s ){ current_attachToTag = s; };
      inline str           GetHolsterTag( void ){ return holster_attachToTag; };
      inline qboolean      GetUseNoAmmo( void ){ return use_no_ammo; };
	  int					GetZoom( void );
	  qboolean				GetAutoZoom( void );
		void						SetMeansOfDeath( Event *ev );
		meansOfDeath_t			GetMeansOfDeath( firemode_t mode );
      void                 SetAimTarget( Entity * );
	  qboolean				ShouldReload( void );
	  void					SetShouldReload( qboolean should_reload );
	  void					StartReloading( void );
      void                 WorldHitSpawn( firemode_t mode, Vector org, Vector angles, float life );
	  void						MakeNoise( Event *ev );
	  void						FallingAngleAdjust( Event *ev );
      void                 Archive(	Archiver &arc );
      
   };
inline void Weapon::Archive
	(
	Archiver &arc
	)

{
	Item::Archive( arc );
	
	arc.ArchiveInteger( &m_iAnimSlot );
	arc.ArchiveBoolean( &attached );
	arc.ArchiveFloat( &nextweaponsoundtime );
	arc.ArchiveFloat( &m_fLastFireTime );
	arc.ArchiveString( &current_attachToTag );
	arc.ArchiveString( &attachToTag_main );
	arc.ArchiveString( &attachToTag_offhand );
	arc.ArchiveString( &holster_attachToTag );
	arc.ArchiveFloat( &lastScale );
	arc.ArchiveVector( &lastAngles );
	arc.ArchiveBoolean( &lastValid );
	arc.ArchiveBoolean( &auto_putaway );
	arc.ArchiveBoolean( &use_no_ammo );
	arc.ArchiveBoolean( &crosshair );
	arc.ArchiveInteger( &m_iZoom );
	arc.ArchiveFloat( &m_fZoomSpreadMult );
	arc.ArchiveBoolean( &m_bAutoZoom );
	arc.ArchiveBoolean( &m_bSemiAuto );
	arc.ArchiveSafePointer( &aim_target );
	arc.ArchiveFloat( &maxrange );
	arc.ArchiveFloat( &minrange );
	arc.ArchiveString( &viewmodel );

	ArchiveEnum( weaponstate, weaponstate_t );

	arc.ArchiveInteger( &rank );
	arc.ArchiveInteger( &order );
	arc.ArchiveSafePointer( &last_owner );
	arc.ArchiveFloat( &last_owner_trigger_time );
	arc.ArchiveBoolean( &notdroppable );
	arc.ArchiveInteger( &aimanim );
	arc.ArchiveInteger( &aimframe );
	arc.ArchiveVector( &holsterOffset );
	arc.ArchiveVector( &holsterAngles );
	arc.ArchiveFloat( &holsterScale );
	arc.ArchiveFloat( &next_noise_time );
	arc.ArchiveFloat( &next_noammo_time );
	arc.ArchiveBoolean( &m_bShouldReload );
	arc.ArchiveFloat( &firedelay[ 0 ] );
	arc.ArchiveFloat( &firedelay[ 1 ] );
	arc.ArchiveString( &ammo_type[ 0 ] );
	arc.ArchiveString( &ammo_type[ 1 ] );
	arc.ArchiveInteger( ammorequired );
	arc.ArchiveInteger( &ammorequired[ 1 ] );
	arc.ArchiveInteger( startammo );
	arc.ArchiveInteger( &startammo[ 1 ] );
	arc.ArchiveString( &projectileModel[ 0 ] );
	arc.ArchiveString( &projectileModel[ 1 ] );
	arc.ArchiveFloat( &bulletdamage[ 0 ] );
	arc.ArchiveFloat( &bulletdamage[ 1 ] );
	arc.ArchiveFloat( &bulletcount[ 0 ] );
	arc.ArchiveFloat( &bulletcount[ 1 ] );
	arc.ArchiveFloat( &bulletrange[ 0 ] );
	arc.ArchiveFloat( &bulletrange[ 1 ] );
	arc.ArchiveFloat( &bulletknockback[ 0 ] );
	arc.ArchiveFloat( &bulletknockback[ 1 ] );
	arc.ArchiveFloat( &projectilespeed[ 0 ] );
	arc.ArchiveFloat( &projectilespeed[ 1 ] );
	arc.ArchiveVector( &bulletspread[ 0 ] );
	arc.ArchiveVector( &bulletspread[ 1 ] );
	arc.ArchiveVector( &bulletspreadmax[ 0 ] );
	arc.ArchiveVector( &bulletspreadmax[ 1 ] );
	arc.ArchiveString( &worldhitspawn[ 0 ] );
	arc.ArchiveString( &worldhitspawn[ 1 ] );
	arc.ArchiveInteger( &tracerfrequency[ 0 ] );
	arc.ArchiveInteger( &tracerfrequency[ 1 ] );
	arc.ArchiveInteger( &tracercount[ 0 ] );
	arc.ArchiveInteger( &tracercount[ 1 ] );
	arc.ArchiveVector( &viewkickmin[ 0 ] );
	arc.ArchiveVector( &viewkickmin[ 1 ] );
	arc.ArchiveVector( &viewkickmax[ 0 ] );
	arc.ArchiveVector( &viewkickmax[ 1 ] );
	arc.ArchiveBoolean( &quiet[ 0 ] );
	arc.ArchiveBoolean( &quiet[ 1 ] );
	arc.ArchiveFloat( &m_fFireSpreadMultAmount[ 0 ] );
	arc.ArchiveFloat( &m_fFireSpreadMultAmount[ 1 ] );
	arc.ArchiveFloat( &m_fFireSpreadMultFalloff[ 0 ] );
	arc.ArchiveFloat( &m_fFireSpreadMultFalloff[ 1 ] );
	arc.ArchiveFloat( &m_fFireSpreadMultCap[ 0 ] );
	arc.ArchiveFloat( &m_fFireSpreadMultCap[ 1 ] );
	arc.ArchiveFloat( &m_fFireSpreadMultTimeCap[ 0 ] );
	arc.ArchiveFloat( &m_fFireSpreadMultTimeCap[ 1 ] );
	arc.ArchiveFloat( &m_fFireSpreadMultTime[ 0 ] );
	arc.ArchiveFloat( &m_fFireSpreadMultTime[ 1 ] );
	arc.ArchiveFloat( &m_fFireSpreadMult[ 0 ] );
	arc.ArchiveFloat( &m_fFireSpreadMult[ 1 ] );

	ArchiveEnum( firetype[ 0 ], firetype_t );
	ArchiveEnum( firetype[ 1 ], firetype_t );

	arc.ArchiveInteger( &ammo_clip_size[ 0 ] );
	arc.ArchiveInteger( &ammo_clip_size[ 1 ] );
	arc.ArchiveInteger( &ammo_in_clip[ 0 ] );
	arc.ArchiveInteger( &ammo_in_clip[ 1 ] );
	arc.ArchiveFloat( &max_charge_time[ 0 ] );
	arc.ArchiveFloat( &max_charge_time[ 1 ] );
	arc.ArchiveFloat( &min_charge_time[ 0 ] );
	arc.ArchiveFloat( &min_charge_time[ 1 ] );

	ArchiveEnum( meansofdeath[ 0 ], meansOfDeath_t );
	ArchiveEnum( meansofdeath[ 1 ], meansOfDeath_t );

	arc.ArchiveBoolean( &loopfire[ 0 ] );
	arc.ArchiveBoolean( &loopfire[ 1 ] );
	arc.ArchiveBoolean( &m_bShareClip );
	arc.ArchiveBoolean( &m_bCanPartialReload );
	arc.ArchiveBoolean( &autoaim );
	arc.ArchiveFloat( &charge_fraction );
	arc.ArchiveBoolean( &putaway );

	ArchiveEnum( firemodeindex, firemode_t );

	Director.ArchiveString( arc, m_csWeaponGroup );
	arc.ArchiveFloat( &m_fMovementSpeed );
	arc.ArchiveString( &m_sAmmoPickupSound );
	arc.ArchiveString( &m_NoAmmoSound );
	arc.ArchiveInteger( &m_iNumShotsFired );
	arc.ArchiveInteger( &m_iNumHits );
	arc.ArchiveFloat( &m_fAccuracy );
	arc.ArchiveFloat( &m_fTimeUsed );
	arc.ArchiveInteger( &m_iNumHeadShots );
	arc.ArchiveInteger( &m_iNumTorsoShots );
	arc.ArchiveInteger( &m_iNumLeftLegShots );
	arc.ArchiveInteger( &m_iNumRightLegShots );
	arc.ArchiveInteger( &m_iNumGroinShots );
	arc.ArchiveInteger( &m_iNumLeftArmShots );
	arc.ArchiveInteger( &m_iNumRightArmShots );

	ArchiveEnum( mAIRange, AIRanges );
}

typedef SafePtr<Weapon> WeaponPtr;

#endif /* weapon.h */
