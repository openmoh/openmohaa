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

// weaputils.h:
// 

#ifndef __WEAPUTILS_H__
#define __WEAPUTILS_H__

#include "g_local.h"
#include "animate.h"
#include "beam.h"

#define P_BOUNCE_TOUCH		0x00000001
#define P_CHARGE_LIFE		0x00000002
#define P_CHARGE_SPEED		0x00000004
#define P_NO_TOUCH_DAMAGE	0x00000008
#define P_FUSE				0x00000010

extern Event EV_Projectile_Explode;
extern Event EV_Projectile_UpdateBeam;

class Weapon;
class Sentient;

class Projectile : public Animate
{
public:
	CLASS_PROTOTYPE( Projectile );

	int owner;
	float speed;
	float minspeed;
	float damage;
	float knockback;
	float life;
	float dmlife;
	float minlife;
	float dlight_radius;
	float charge_fraction;

	SafePtr<Weapon> weap;
	Vector dlight_color;
	Vector addvelocity;
	meansOfDeath_t meansofdeath;
	class FuncBeam *m_beam;
	int projFlags;
	str bouncesound;
	str bouncesound_metal;
	str bouncesound_hard;
	str bouncesound_water;
	float fLastBounceTime;
	str impactmarkshader;
	str impactmarkorientation;
	float impactmarkradius;
	str explosionmodel;
	SafePtr<Entity> target;
	float fDrunk;
	float fDrunkRate;
	bool addownervelocity;
	bool can_hit_owner;
	bool remove_when_stopped;
	bool m_bExplodeOnTouch;
	bool m_bHurtOwnerOnly;
	int m_iSmashThroughGlass;
	bool m_bArcToTarget;
	bool m_bDieInWater;
	int m_iTeam;
	bool m_bHadPlayerOwner;
	SafePtr<Entity> m_pOwnerPtr;

	Projectile();
	void Archive( Archiver &arc ) override;
	virtual void Touch( Event *ev );
	virtual void Explode( Event *ev );
	virtual void DoDecal( void );

	void			SetAvelocity( Event *ev );
	void			SetAddVelocity( Event *ev );
	void			SetDLight( Event *ev );
	void			SetLife( Event *ev );
	void			SetDMLife( Event *ev );
	void			SetMinLife( Event *ev );
	void			SetChargeLife( Event *ev );
	void			SetFuse( Event *ev ); // Added in 2.0
	void			SetSpeed( Event *ev );
	void			SetMinSpeed( Event *ev );
	void			SetChargeSpeed( Event *ev );
	void			SetDamage( Event *ev );
	void			SetKnockback( Event *ev );
	void			SetProjectileDLight( Event *ev );
	void			SetMeansOfDeath( Event *ev );
	void			SetBounceTouch( Event *ev );
	void			SetBounceSound( Event *ev );
	void			SetBounceSoundMetal( Event *ev );
	void			SetBounceSoundHard( Event *ev );
	void			SetBounceSoundWater( Event *ev );
	void			SetNoTouchDamage( Event *ev );
	void			SetImpactMarkShader( Event *ev );
	void			SetImpactMarkRadius( Event *ev );
	void			SetImpactMarkOrientation( Event *ev );
	void			SetExplosionModel( Event *ev );
	void			SetSmashThroughGlass( Event *ev );
	void			UpdateBeam( Event *ev );
	void			BeamCommand( Event *ev );
	void			HeatSeek( Event *ev );
	void			Drunk( Event *ev );
	void			SmashThroughGlassThink( Event *ev );
	void			AddOwnerVelocity( Event *ev );
	void			Prethink( Event *ev );
	float			ResolveMinimumDistance( Entity *potential_target, float currmin );
	float			AdjustAngle( float maxadjust, float currangle, float targetangle );
	void			SetCanHitOwner( Event *ev );
	void			ClearOwner( Event *ev );
	void			RemoveWhenStopped( Event *ev );
	void			ExplodeOnTouch( Event *ev );
	void			Stopped( Event *ev );
	void			Think() override;
	Sentient		*GetOwner( void );
	void			SetOwner(Entity* owner); // Added in 2.0
	void			SetMartyr( int entnum );
	void			ArcToTarget(Event* ev);
	void			BecomeBomb(Event* ev);
	void			DieInWater(Event* ev);
	//
	// Openmohaa additions
	//
	bool			CheckTeams( void );
};

inline void Projectile::Archive
	(
	Archiver &arc
	)
{
	Animate::Archive( arc );

	arc.ArchiveInteger( &owner );
	arc.ArchiveFloat( &speed );
	arc.ArchiveFloat( &minspeed );
	arc.ArchiveFloat( &damage );
	arc.ArchiveFloat( &knockback );
	arc.ArchiveFloat( &life );
	arc.ArchiveFloat( &dmlife );
	arc.ArchiveFloat( &minlife );
	arc.ArchiveFloat( &dlight_radius );
	arc.ArchiveFloat( &charge_fraction );
	arc.ArchiveVector( &dlight_color );
	arc.ArchiveVector( &addvelocity );
	ArchiveEnum( meansofdeath, meansOfDeath_t );
	arc.ArchiveObjectPointer( ( Class ** )&m_beam );
	arc.ArchiveInteger( &projFlags );
	arc.ArchiveString( &bouncesound );
	arc.ArchiveString( &bouncesound_metal );
	arc.ArchiveString( &bouncesound_hard );
	arc.ArchiveString( &bouncesound_water );
	arc.ArchiveFloat( &fLastBounceTime );
	arc.ArchiveString( &impactmarkshader );
	arc.ArchiveString( &impactmarkorientation );
	arc.ArchiveFloat( &impactmarkradius );
	arc.ArchiveString( &explosionmodel );
	arc.ArchiveFloat( &fDrunk );
	arc.ArchiveFloat( &fDrunkRate );
	arc.ArchiveBool( &addownervelocity );
	arc.ArchiveBool( &can_hit_owner );
	arc.ArchiveBool( &remove_when_stopped );
	arc.ArchiveBool( &m_bExplodeOnTouch );
	arc.ArchiveBool( &m_bHurtOwnerOnly );
}

class Explosion : public Projectile
{
	public:
	float		flash_r,
				flash_g,
				flash_b,
				flash_a,
				flash_radius,
				flash_time;

	int			flash_type;
	float		radius_damage;
	bool		hurtOwnerOnly;

	   CLASS_PROTOTYPE( Explosion );

	   float			radius;
	   qboolean			constant_damage;
	   qboolean			damage_every_frame;

	   Explosion();
	   void				SetRadius( Event *ev );
	   void				SetRadiusDamage( Event *ev );
	   void				SetConstantDamage( Event *ev );
	   void				SetDamageEveryFrame( Event *ev );
	   void				SetFlash( Event *ev );
	   void				MakeExplosionEffect( Event *ev );

	   void				DamageAgain( Event *ev );

	   void Archive( Archiver &arc ) override;
};

inline void Explosion::Archive
	(
	Archiver &arc
	)
{
	Projectile::Archive( arc );

	arc.ArchiveFloat( &flash_r );
	arc.ArchiveFloat( &flash_g );
	arc.ArchiveFloat( &flash_b );
	arc.ArchiveFloat( &flash_a );
	arc.ArchiveFloat( &flash_radius );
	arc.ArchiveFloat( &flash_time );
	arc.ArchiveInteger( &flash_type );
	arc.ArchiveFloat( &radius_damage );
}

qboolean MeleeAttack
   (
   Vector pos,
   Vector end,
   float damage,
   Entity *attacker,
	meansOfDeath_t means_of_death,
	float attack_width,
	float attack_min_height,
	float attack_max_height,
	float knockback = 0,
	qboolean hit_dead = true,
   Container<Entity *>*victimlist=NULL
   );

Projectile *ProjectileAttack
   (
   Vector         start,
   Vector         dir,
   Entity         *owner,
   str            projectileModel,
   float          speedfraction,
	float				real_speed = 0,
	Weapon			*weap = NULL
   );

void ExplosionAttack
(
	Vector			pos,
	Entity			*owner,
	str				projectileModel,
	Vector			dir = Vector( 0, 0, 0 ),
	Entity			*ignore = NULL,
	float			scale = 1.0f,
	Weapon			*weap = NULL,
	bool			hurtOwnerOnly = false
);

float BulletAttack
	(
	Vector   start,
	Vector   vBarrel,
	Vector   dir,
	Vector   right,
	Vector   up,
	float    range,
	float    damage,
	int      bulletlarge,
	float    knockback,
	int      dflags,
	int      meansofdeath,
	Vector   spread,
	int      count,
	Entity*  owner,
	int      iTracerFrequency,
	int*     piTracerCount,
	float    bulletthroughwood,
	float    bulletthroughmetal,
	Weapon*  weap,
	float    tracerspeed
	);

void FakeBulletAttack
	(
	Vector    start,
	Vector    vBarrel,
	Vector    dir,
	Vector    right,
	Vector    up,
	float     range,
	float     damage,
	int       large,
	Vector    spread,
	int       count,
	Entity*   owner,
	int       iTracerFrequency,
	int*      piTracerCount,
	float     tracerspeed
	);

void ClickItemAttack
	(
	Vector			vStart,
	Vector			vForward,
	float			fRange,
	Entity			*pOwner
	);

Projectile *HeavyAttack
	(
	Vector			start,
	Vector			dir,
	str				projectileModel,
	float			real_speed,
	Entity			*owner,
	Weapon			*weap
	);

void RadiusDamage
	(
	Vector		origin,
	Entity		*inflictor,
	Entity		*attacker,
	float		damage,
	Entity		*ignore,
	int			mod,
	float		radius = 0,
	float		knockback = 0,
	qboolean	constant_damage = false,
	Weapon		*weap = NULL,
	bool		hurtOwnerOnly = false
	);

const char* G_LocationNumToDispString(int iLocation);
Entity* FindDefusableObject(const Vector& dir, Entity* owner, float maxdist);
void DefuseObject(const Vector& dir, Entity* owner, float maxdist);
qboolean CanPlaceLandmine(const Vector& origin, Entity* owner);
void PlaceLandmine(const Vector& origin, Entity* owner, const str& model, Weapon* weap);

#endif // __WEAPUTILS_H__
