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

// weaputils.cpp: General Weapon Utility Functions

#include "g_phys.h"
#include "g_spawn.h"
#include "weaputils.h"
#include "specialfx.h"
#include "sentient.h"
#include "actor.h"
#include "decals.h"
#include "weapon.h"
#include "player.h"
#include "VehicleCollisionEntity.h"
#include "weapturret.h"
#include "vehicleturret.h"
#include "earthquake.h"
#include "trigger.h"
#include "debuglines.h"

static void FlashPlayers
   (
   Vector   org,
   float    r,
   float    g, 
   float    b,
   float    a,
   float    rad,
   float    time,
   int      type
   );

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
	float knockback,
	qboolean hit_dead,
	Container<Entity *>*victimlist
	)

{
	trace_t trace;
	Entity *victim;
	Vector dir;
	float world_dist;
	Vector new_pos;
	Entity *skip_ent;
	qboolean hit_something = false;
	Vector mins;
	Vector maxs;
	Container<Entity *> potential_victimlist;
	int i;
	int num_traces;
	Vector start;

	// See how far the world is away

	dir = end - pos;
	world_dist = dir.length();

	new_pos = pos;

	skip_ent = attacker;

	num_traces = 0;

	while( new_pos != end )
	{
		trace = G_Trace( pos, vec_zero, vec_zero, end, skip_ent, MASK_SOLID, false, "MeleeAttack - World test" );

		num_traces++;

		if( trace.fraction < 1 )
		{
			if( ( trace.entityNum == ENTITYNUM_WORLD ) || ( trace.ent && trace.ent->entity && !trace.ent->entity->takedamage ) )
			{
				dir = trace.endpos - pos;
				world_dist = dir.length();
				break;
			}
			else
			{
				// Make sure we don't go backwards any in our trace
				if( Vector( new_pos - pos ).length() + 0.001 >= Vector( trace.endpos - pos ).length() )
					break;

				if( num_traces > 10 )
				{
					// We have done too many traces, stop here
					dir = trace.endpos - pos;
					world_dist = dir.length();
					break;
				}

				new_pos = trace.endpos;

				if( trace.ent )
					skip_ent = trace.ent->entity;
			}
		}
		else
		{
			break;
		}
	}

	// Find things hit

	dir = end - pos;
	dir.normalize();
	end = pos + ( dir * world_dist );

	start = pos - dir * ( attack_width * 1.2f );
	start.z = pos.z - dir.z * ( end.z - start.z );

	victim = NULL;

	mins = Vector( -attack_width, -attack_width, attack_min_height );
	maxs = Vector( attack_width, attack_width, attack_max_height );

	G_TraceEntities( pos, mins, maxs, end, &potential_victimlist, MASK_MELEE );

	/*int previous_contents = attacker->edict->r.contents;
	attacker->edict->r.contents = 0;
	trace = G_Trace( pos, mins, maxs, end, ( ( Sentient * )attacker )->GetActiveWeapon( WEAPON_MAIN ), MASK_MELEE, false, "MeleeAttack" );

	if( trace.ent && trace.ent->entity )
	{
		gi.Printf( "HIT\n" );
		potential_victimlist.AddObject( trace.ent->entity );
	}

	attacker->edict->r.contents = previous_contents;*/

	for( i = 1; i <= potential_victimlist.NumObjects(); i++ )
	{
		victim = potential_victimlist.ObjectAt( i );

		if( victim && victim->takedamage && victim != attacker )
		{
			dir = end - pos;
			dir.normalize();

			if( dir == vec_zero )
			{
				dir = victim->centroid - pos;
				dir.normalize();
			}

			if( victim->IsSubclassOfSentient() && !victim->IsDead() )
				hit_something = true;

			if( victim->health > 0 || hit_dead )
			{
				if( victimlist && victim->IsSubclassOfSentient() && !victim->IsDead() )
					victimlist->AddObject( victim );

				victim->Damage( attacker, attacker, damage, pos, dir, vec_zero, knockback, 0, means_of_death );

				if( victim->edict->r.contents & CONTENTS_SOLID )
				{
					victim->Sound( "pistol_hit" );
				}
			}
		}
	}

	return hit_something;
}

#define DEFAULT_SWORD_DAMAGE 10
#define DEFAULT_SWORD_KNOCKBACK 50

Event EV_Projectile_Speed
   (
   "speed",
   EV_DEFAULT,
   "f",
   "projectileSpeed",
   "set the speed of the projectile",
   EV_NORMAL
   );
Event EV_Projectile_MinSpeed
   (
   "minspeed",
   EV_DEFAULT,
   "f",
   "minspeed",
   "set the minimum speed of the projectile (this is for charge up weapons)",
   EV_NORMAL
   );
Event EV_Projectile_ChargeSpeed
   (
   "chargespeed",
   EV_DEFAULT,
   NULL,
   NULL,
   "set the projectile's speed to be determined by the charge time",
   EV_NORMAL
   );
Event EV_Projectile_Damage
   (
   "hitdamage",
   EV_DEFAULT,
   "f",
   "projectileHitDamage",
   "set the damage a projectile does when it hits something",
   EV_NORMAL
   );
Event EV_Projectile_Life
   (
   "life",
   EV_DEFAULT,
   "f",
   "projectileLife",
   "set the life of the projectile",
   EV_NORMAL
   );
Event EV_Projectile_DMLife
   (
   "dmlife",
   EV_DEFAULT,
   "f",
   "projectileLife",
   "set the life of the projectile in DM",
   EV_NORMAL
   );
Event EV_Projectile_MinLife
   (
   "minlife",
   EV_DEFAULT,
   "f",
   "minProjectileLife",
   "set the minimum life of the projectile (this is for charge up weapons)",
   EV_NORMAL
   );
Event EV_Projectile_ChargeLife
   (
   "chargelife",
   EV_DEFAULT,
   NULL,
   NULL,
   "set the projectile's life to be determined by the charge time",
   EV_NORMAL
   );
Event EV_Projectile_Knockback
   (
   "knockback", 
   EV_DEFAULT,
   "f",
   "projectileKnockback",
   "set the knockback of the projectile when it hits something",
   EV_NORMAL
   );
Event EV_Projectile_DLight
   (
   "dlight",
   EV_DEFAULT,
   "ffff",
   "red green blue intensity",
   "set the color and intensity of the dynamic light on the projectile",
   EV_NORMAL
   );
Event EV_Projectile_Avelocity
   (
   "avelocity",
   EV_DEFAULT,
   "SFSFSF",
   "[random|crandom] yaw [random|crandom] pitch [random|crandom] roll",
   "set the angular velocity of the projectile",
   EV_NORMAL
   );
Event EV_Projectile_MeansOfDeath
   (
   "meansofdeath",
   EV_DEFAULT,
   "s",
   "meansOfDeath",
   "set the meansOfDeath of the projectile",
   EV_NORMAL
   );
Event EV_Projectile_BeamCommand
   (
   "beam",
   EV_DEFAULT,
   "sSSSSSS",
   "command arg1 arg2 arg3 arg4 arg5 arg6",
   "send a command to the beam of this projectile",
   EV_NORMAL
   );
Event EV_Projectile_UpdateBeam
   (
   "updatebeam",
   EV_DEFAULT,
   NULL,
   NULL,
   "Update the attached beam",
   EV_NORMAL
   );
Event EV_Projectile_BounceTouch
   (
   "bouncetouch",
   EV_DEFAULT,
   NULL,
   NULL,
   "Make the projectile bounce when it hits a non-damageable solid",
   EV_NORMAL
   );
Event EV_Projectile_BounceSound
   (
   "bouncesound",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set the name of the sound that is played when the projectile bounces",
   EV_NORMAL
   );
Event EV_Projectile_BounceSound_Metal
   (
   "bouncesound_metal",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set the name of the sound that is played when the projectile bounces off metal",
   EV_NORMAL
   );
Event EV_Projectile_BounceSound_Hard
   (
   "bouncesound_hard",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set the name of the sound that is played when the projectile bounces off hard surfaces",
   EV_NORMAL
   );
Event EV_Projectile_BounceSound_Water
   (
   "bouncesound_water",
   EV_DEFAULT,
   NULL,
   NULL,
   "Set the name of the sound that is played when the projectile bounces off water",
   EV_NORMAL
   );
Event EV_Projectile_Explode
   (
   "explode",
   EV_DEFAULT,
   NULL,
   NULL,
   "Make the projectile explode",
   EV_NORMAL
   );
Event EV_Projectile_ImpactMarkShader
   (
   "impactmarkshader",
   EV_DEFAULT,
   "s",
   "shader",
   "Set the impact mark of the shader",
   EV_NORMAL
   );
Event EV_Projectile_ImpactMarkRadius
   (
   "impactmarkradius",
   EV_DEFAULT,
   "f",
   "radius",
   "Set the radius of the impact mark",
   EV_NORMAL
   );
Event EV_Projectile_ImpactMarkOrientation
   (
   "impactmarkorientation",
   EV_DEFAULT,
   "f",
   "degrees",
   "Set the orientation of the impact mark",
   EV_NORMAL
   );
Event EV_Projectile_SetExplosionModel
   (
   "explosionmodel",
   EV_DEFAULT,
   "s",
   "modelname",
   "Set the modelname of the explosion to be spawned",
   EV_NORMAL
   );
Event EV_Projectile_SetAddVelocity
   (
   "addvelocity",
   EV_DEFAULT,
   "fff",
   "velocity_x velocity_y velocity_z",
   "Set a velocity to be added to the projectile when it is created",
   EV_NORMAL
   );
Event EV_Projectile_AddOwnerVelocity
   (
   "addownervelocity",
   EV_DEFAULT,
   "b",
   "bool",
   "Set whether or not the owner's velocity is added to the projectile's velocity",
   EV_NORMAL
   );
Event EV_Projectile_HeatSeek
   (
   "heatseek",
   EV_DEFAULT,
   NULL,
   NULL,
   "Make the projectile heat seek",
   EV_NORMAL
   );
Event EV_Projectile_Drunk
   (
   "drunk",
   EV_DEFAULT,
   "ff",
   "amount rate",
   "Make the projectile drunk",
   EV_NORMAL
   );
Event EV_Projectile_Prethink
   (
   "prethink",
   EV_DEFAULT,
   NULL,
   NULL,
   "Make the projectile think to update it's velocity",
   EV_NORMAL
   );
Event EV_Projectile_SetCanHitOwner
   (
   "canhitowner",
   EV_DEFAULT,
   NULL,
   NULL,
   "Make the projectile be able to hit its owner",
   EV_NORMAL
   );
Event EV_Projectile_ClearOwner
   (
   "clearowner",
   EV_DEFAULT,
   NULL,
   NULL,
   "Make the projectile be able to hit its owner now",
   EV_NORMAL
   );
Event EV_Projectile_RemoveWhenStopped
   (
   "removewhenstopped",
   EV_DEFAULT,
   NULL,
   NULL,
   "Make the projectile get removed when it stops",
   EV_NORMAL
   );
Event EV_Projectile_ExplodeOnTouch
	(
	"explodeontouch",
	EV_DEFAULT,
	NULL,
	NULL,
	"Make the projectile explode when it touches something damagable",
	EV_NORMAL
	);
Event EV_Projectile_NoTouchDamage
	(
	"notouchdamage",
	EV_DEFAULT,
	NULL,
	NULL,
	"Makes the projectile not blow up or deal damage when it touches a damagable object",
	EV_NORMAL
	);
Event EV_Projectile_SetSmashThroughGlass
	(
	"smashthroughglass",
	EV_DEFAULT,
	"i",
	"speed",
	"Makes the projectile smash through windows & other damageble glass objects when going above a set speed",
	EV_NORMAL
	);
Event EV_Projectile_SmashThroughGlass
	(
	"_smashthroughglass",
	EV_DEFAULT,
	NULL,
	NULL,
	"Think function for smashing through glass",
	EV_NORMAL
	);


CLASS_DECLARATION( Animate, Projectile, NULL )
{
	{ &EV_Touch,								&Projectile::Touch },
	{ &EV_Projectile_Speed,						&Projectile::SetSpeed },
	{ &EV_Projectile_MinSpeed,					&Projectile::SetMinSpeed },
	{ &EV_Projectile_ChargeSpeed,				&Projectile::SetChargeSpeed },
	{ &EV_Projectile_Damage,					&Projectile::SetDamage },
	{ &EV_Projectile_Life,						&Projectile::SetLife },
	{ &EV_Projectile_DMLife,					&Projectile::SetDMLife },
	{ &EV_Projectile_MinLife,					&Projectile::SetMinLife },
	{ &EV_Projectile_ChargeLife,				&Projectile::SetChargeLife },
	{ &EV_Projectile_Knockback,					&Projectile::SetKnockback },
	{ &EV_Projectile_DLight,					&Projectile::SetDLight },
	{ &EV_Projectile_Avelocity,					&Projectile::SetAvelocity },
	{ &EV_Projectile_MeansOfDeath,				&Projectile::SetMeansOfDeath },
	{ &EV_Projectile_BounceTouch,				&Projectile::SetBounceTouch },
	{ &EV_Projectile_BounceSound,				&Projectile::SetBounceSound },
	{ &EV_Projectile_BounceSound_Metal,			&Projectile::SetBounceSoundMetal },
	{ &EV_Projectile_BounceSound_Hard,			&Projectile::SetBounceSoundHard },
	{ &EV_Projectile_BounceSound_Water,			&Projectile::SetBounceSoundWater },
	{ &EV_Projectile_BeamCommand,				&Projectile::BeamCommand },
	{ &EV_Projectile_UpdateBeam,				&Projectile::UpdateBeam },
	{ &EV_Projectile_Explode,					&Projectile::Explode },
	{ &EV_Projectile_ImpactMarkShader,			&Projectile::SetImpactMarkShader },
	{ &EV_Projectile_ImpactMarkRadius,			&Projectile::SetImpactMarkRadius },
	{ &EV_Projectile_ImpactMarkOrientation,		&Projectile::SetImpactMarkOrientation },
	{ &EV_Projectile_SetExplosionModel,			&Projectile::SetExplosionModel },
	{ &EV_Projectile_SetAddVelocity,			&Projectile::SetAddVelocity },
	{ &EV_Projectile_AddOwnerVelocity,			&Projectile::AddOwnerVelocity },
	{ &EV_Projectile_HeatSeek,					&Projectile::HeatSeek },
	{ &EV_Projectile_Drunk,						&Projectile::Drunk },
	{ &EV_Projectile_Prethink,					&Projectile::Prethink },
	{ &EV_Projectile_SetCanHitOwner,			&Projectile::SetCanHitOwner },
	{ &EV_Projectile_ClearOwner,				&Projectile::ClearOwner },
	{ &EV_Projectile_RemoveWhenStopped,			&Projectile::RemoveWhenStopped },
	{ &EV_Projectile_ExplodeOnTouch,			&Projectile::ExplodeOnTouch },
	{ &EV_Projectile_NoTouchDamage,				&Projectile::SetNoTouchDamage },
	{ &EV_Projectile_SetSmashThroughGlass,		&Projectile::SetSmashThroughGlass },
	{ &EV_Projectile_SmashThroughGlass,			&Projectile::SmashThroughGlassThink },
	{ &EV_Killed,		      					&Projectile::Explode },
	{ &EV_Stop,									&Projectile::Stopped },
	{ NULL, NULL }
};

Projectile::Projectile()
{
	entflags |= EF_PROJECTILE;

	if( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}

	m_beam = NULL;
	speed = 0;
	minspeed = 0;
	damage = 0;
	life = 5;
	dmlife = 0;
	knockback = 0;
	dlight_radius = 0;
	dlight_color = Vector( 1, 1, 1 );
	avelocity = Vector( 0, 0, 0 );
	mins = Vector( -1, -1, -1 );
	maxs = Vector( 1, 1, 1 );
	meansofdeath = ( meansOfDeath_t )0;
	projFlags = 0;
	fLastBounceTime = 0;
	gravity = 0;
	impactmarkradius = 10;
	charge_fraction = 1.0;
	target = NULL;
	fDrunk = 0;
	fDrunkRate = 0;
	m_iSmashThroughGlass = 0;
	addownervelocity = qtrue;
	can_hit_owner = false;
	remove_when_stopped = false;
	m_bExplodeOnTouch = false;
	m_bHurtOwnerOnly = false;
	takedamage = DAMAGE_NO;
	owner = ENTITYNUM_NONE;
	edict->r.ownerNum = ENTITYNUM_NONE;
	m_iOwnerTeam = 0;
	m_bHadPlayerOwner = false;

	// make this shootable but non-solid on the client
	setContents( CONTENTS_SHOOTONLY );

	//
	// touch triggers by default
	//
	flags |= FL_TOUCH_TRIGGERS;
}

float Projectile::ResolveMinimumDistance
	(
	Entity *potential_target,
	float currmin
	)

   {
   float		currdist;
	float		dot;
	Vector	angle;
	Vector	delta;
	Vector	norm;
	float		sine = 0.4f;

	delta = potential_target->centroid - this->origin;

	norm = delta;
	norm.normalize();

	// Test if the target is in front of the missile
	dot = norm * orientation[ 0 ];
	if ( dot < 0 )
		{
		return currmin;
		}

	// Test if we're within the rocket's viewcone (45 degree cone)
	dot = norm * orientation[ 1 ];
	if ( fabs( dot ) > sine )
		{
		return currmin;
		}

	dot = norm * orientation[ 2 ];
	if ( fabs( dot ) > sine )
		{
		return currmin;
		}

	currdist = delta.length();
	if ( currdist < currmin )
		{
		currmin = currdist;
		target = potential_target;
		}

	return currmin;
	}

float Projectile::AdjustAngle
	(
	float maxadjust,
	float currangle,
	float targetangle
	)

   {
   float dangle;
	float magangle;

	dangle = currangle - targetangle;

	if ( dangle )
		{
		magangle = ( float )fabs( dangle );

		while( magangle >= 360.0f )
			{
			magangle -= 360.0f;
			}

		if ( magangle < maxadjust )
			{
			currangle = targetangle;
			}
		else 
			{
			if ( magangle > 180.0f  )
				{
				maxadjust = -maxadjust;
				}
			if ( dangle > 0 )
				{
				maxadjust = -maxadjust;
				}
			currangle += maxadjust;
			}
		}

	while( currangle >= 360.0f )
		{
		currangle -= 360.0f;
		}

	while( currangle < 0.0f )
		{
		currangle += 360.0f;
		}

	return currangle;
	}

void Projectile::Drunk
	(
	Event *ev
	)

{
	if( fDrunk ) {
		return;
	}

	fDrunk = ev->GetFloat( 1 );
	fDrunkRate = ev->GetFloat( 2 );

	PostEvent( EV_Projectile_Prethink, 0 );
}

void Projectile::HeatSeek
	(
	Event *ev
	)

	{
	float		mindist;
	Entity	*ent;
	trace_t  trace;
	Vector	delta;
	Vector	v;
   int      n;
   int      i;

   if ( ( !target ) || ( target == world ) )
		{
		mindist = 8192.0f;

	   n = SentientList.NumObjects();
	   for( i = 1; i <= n; i++ )
		   {
		   ent = SentientList.ObjectAt( i );
			if ( ent->entnum == owner )
				{
				continue;
				}

			if ( ( ( ent->takedamage != DAMAGE_AIM ) || ( ent->health <= 0 ) ) )
				{
				continue;
				}

			trace = G_Trace( this->origin, vec_zero, vec_zero, ent->centroid, this, MASK_SHOT, qfalse, "DrunkMissile::HeatSeek" );
			if ( ( trace.fraction != 1.0 ) && ( trace.ent != ent->edict ) )
				{
				continue;
				}
	
			mindist = ResolveMinimumDistance( ent, mindist );
			}
		}
	else 
		{
      float angspeed;

      delta = target->centroid - this->origin;
		v = delta.toAngles();

      angspeed = 5.0f;
		angles.x = AdjustAngle( angspeed, angles.x, v.x );
		angles.y = AdjustAngle( angspeed, angles.y, v.y );
		angles.z = AdjustAngle( angspeed, angles.z, v.z );
		}
   PostEvent( EV_Projectile_HeatSeek, 0.1f );
   PostEvent( EV_Projectile_Prethink, 0 );
	}

void Projectile::Prethink
	(
	Event *ev
	)

{
	Vector end;

	if( fDrunk >= 0.0f )
	{
		if( fDrunk )
		{
			float rnd1 = G_Random();
			float rnd2 = G_Random();

			angles += Vector( ( rnd1 + rnd1 - 1.0f ) * fDrunk, ( rnd2 + rnd2 - 1.0f ) * fDrunk, 0 );

			speed *= 0.98f;
			fDrunk += fDrunkRate;
			if( speed < 500.0f )
			{
				fDrunk = -fDrunk;
			}
		}

		setAngles( angles );
		velocity = Vector( orientation[ 0 ] ) * speed;
	}
	else
	{
		fDrunk *= 0.80f;

		if( fDrunk > -1.0f ) {
			fDrunk = -1.0f;
		}

		//float rnd1 = rand() & 0x7FFF;
		//float rnd2 = rand() & 0x7FFF;

		//angles += Vector( ( rnd1 * 0.00003f + rnd1 * 0.00003f - 1.0f ) * fDrunk, ( rnd2 * 0.00003f + rnd2 * 0.00003f - 1.0f ) * fDrunk, 0 );
		velocity[ 2 ] -= 0.05f * sv_gravity->integer * 0.15f;

		angles = velocity.toAngles();
		setAngles( angles );
	}

	PostEvent( EV_Projectile_Prethink, 0.05f );
}

void Projectile::AddOwnerVelocity
   (
   Event *ev
   )

   {
   addownervelocity = ev->GetBoolean( 1 );
   }

void Projectile::SetAddVelocity
   (
   Event *ev
   )

   {
   addvelocity.x = ev->GetFloat( 1 );
   addvelocity.y = ev->GetFloat( 2 );
   addvelocity.z = ev->GetFloat( 3 );
   }

void Projectile::SetExplosionModel
   (
   Event *ev
   )

   {
   explosionmodel = ev->GetString( 1 );
   }

void Projectile::SetImpactMarkShader
   (
   Event *ev
   )

   {
   impactmarkshader = ev->GetString( 1 );
   }

void Projectile::SetImpactMarkRadius
   (
   Event *ev
   )

   {
   impactmarkradius = ev->GetFloat( 1 );
   }

void Projectile::SetImpactMarkOrientation
   (
   Event *ev
   )

   {
   impactmarkorientation = ev->GetString( 1 );
   }

void Projectile::Explode
	(
	Event *ev
	)

{
	Entity *owner;
	Entity *ignoreEnt = NULL;

	if( !CheckTeams() )
	{
		PostEvent( EV_Remove, EV_REMOVE );
		return;
	}

	if( ev->NumArgs() == 1 )
		ignoreEnt = ev->GetEntity( 1 );

	// Get the owner of this projectile
	owner = G_GetEntity( this->owner );

	// If the owner's not here, make the world the owner
	if( !owner )
		owner = world;

	if( owner->IsDead() || owner == world )
		weap = NULL;

	health = 0;
	deadflag = DEAD_DEAD;
	takedamage = DAMAGE_NO;

	// Spawn an explosion model
	if( explosionmodel.length() )
	{
		// Move the projectile back off the surface a bit so we can see
		// explosion effects.
		Vector dir, v;
		v = velocity;
		v.normalize();
		dir = v;
		v = origin - v * 36;
		setOrigin( v );

		ExplosionAttack( v, owner, explosionmodel, dir, ignoreEnt, 1.0f, weap, m_bHurtOwnerOnly );
	}

	CancelEventsOfType( EV_Projectile_UpdateBeam );

	// Kill the beam
	if( m_beam )
	{
		m_beam->ProcessEvent( EV_Remove );
		m_beam = NULL;
	}

	// Remove the projectile
	PostEvent( EV_Remove, level.frametime );
}

void Projectile::SetBounceTouch
	(
	Event *ev
	)

{
	projFlags |= P_BOUNCE_TOUCH;
	setMoveType( MOVETYPE_BOUNCE );
}

void Projectile::SetNoTouchDamage
	(
	Event *ev
	)

{
	projFlags |= P_NO_TOUCH_DAMAGE;
}

void Projectile::SetSmashThroughGlass
	(
	Event *ev
	)

{
	m_iSmashThroughGlass = ev->GetInteger( 1 );

	PostEvent( EV_Projectile_SmashThroughGlass, level.frametime );
}

void Projectile::SmashThroughGlassThink
	(
	Event *ev
	)

{
	if( velocity.length() > m_iSmashThroughGlass )
	{
		Vector vEnd;
		trace_t trace;

		Entity *ent = G_GetEntity( owner );

		vEnd = velocity * level.frametime + velocity * level.frametime + origin;

		trace = G_Trace( origin,
			vec_zero,
			vec_zero,
			vEnd,
			ent,
			MASK_BEAM,
			false,
			"SmashThroughGlassThink" );

		if( ( trace.ent ) && ( trace.ent->entity != world ) )
		{
			Entity *obj = trace.ent->entity;

			if( ( trace.surfaceFlags & SURF_GLASS ) && ( obj->takedamage ) )
			{
				obj->Damage( this,
					ent,
					obj->health + 1,
					origin,
					velocity,
					trace.plane.normal,
					knockback,
					0,
					meansofdeath
					);
			}
		}
	}

	PostEvent( EV_Projectile_SmashThroughGlass, level.frametime );
}

void Projectile::BeamCommand
   (
   Event *ev
   )

   {
   int i;

   if ( !m_beam )
      {
      m_beam = new FuncBeam;

      m_beam->setOrigin( this->origin );
      m_beam->Ghost( NULL );
      }
   
   Event *beamev = new Event( ev->GetToken( 1 ) );

   for( i=2; i<=ev->NumArgs(); i++ )
      {
      beamev->AddToken( ev->GetToken( i ) );
      }

   m_beam->ProcessEvent( beamev );
   PostEvent( EV_Projectile_UpdateBeam, level.frametime );
   }

void Projectile::UpdateBeam
   (
   Event *ev
   )

   {
   if ( m_beam )
      {
      m_beam->setOrigin( this->origin );
      PostEvent( EV_Projectile_UpdateBeam, level.frametime );
      }
   }

void Projectile::SetBounceSound
	(
	Event *ev
	)

{
	bouncesound = ev->GetString( 1 );
}

void Projectile::SetBounceSoundMetal
	(
	Event *ev
	)

{
	bouncesound_metal = ev->GetString( 1 );
}

void Projectile::SetBounceSoundHard
	(
	Event *ev
	)

{
	bouncesound_hard = ev->GetString( 1 );
}

void Projectile::SetBounceSoundWater
	(
	Event *ev
	)

{
	bouncesound_water = ev->GetString( 1 );
}

void Projectile::SetChargeLife
   (
   Event *ev
   )

   {
   projFlags   |= P_CHARGE_LIFE;
   }

void Projectile::SetMinLife
   (
   Event *ev
   )

   {
   minlife  = ev->GetFloat( 1 );
   projFlags   |= P_CHARGE_LIFE;
   }

void Projectile::SetLife
	(
	Event *ev
	)

{
	life = ev->GetFloat( 1 );
}

void Projectile::SetDMLife
	(
	Event *ev
	)

{
	dmlife = ev->GetFloat( 1 );
}

void Projectile::SetSpeed
   (
   Event *ev
   )

   {
   speed = ev->GetFloat( 1 );
   }

void Projectile::SetMinSpeed
   (
   Event *ev
   )

   {
   minspeed = ev->GetFloat( 1 );
   projFlags   |= P_CHARGE_SPEED;
   }

void Projectile::SetChargeSpeed
   (
   Event *ev
   )

   {
   projFlags   |= P_CHARGE_SPEED;
   }

void Projectile::SetAvelocity
   (
   Event *ev
   )

   {
   int i=1;
   int j=0;
   str vel;

   if ( ev->NumArgs() < 3 )
      {
      warning( "ClientGameCommandManager::SetAngularVelocity", "Expecting at least 3 args for command randvel" );
      }

   while ( j < 3 )
      {      
      vel = ev->GetString( i++ );
      if ( vel == "crandom" )
         {
         avelocity[j++] = ev->GetFloat( i++ ) * crandom();
         }
      else if ( vel == "random" )
         {
         avelocity[j++] = ev->GetFloat( i++ ) * random();
         }
      else
         {
         avelocity[j++] = atof( vel.c_str() );
         }
      }
   }

void Projectile::SetDamage
   (
   Event *ev
   )

   {
   damage = ev->GetFloat( 1 );
   }

void Projectile::SetKnockback
   (
   Event *ev
   )

   {
   knockback = ev->GetFloat( 1 );
   }

void Projectile::SetDLight
   (
   Event *ev
   )

   {
   dlight_color[0] = ev->GetFloat( 1 );
   dlight_color[1] = ev->GetFloat( 2 );
   dlight_color[2] = ev->GetFloat( 3 );
   dlight_radius   = ev->GetFloat( 4 );
   }

void Projectile::SetMeansOfDeath
   (
   Event *ev
   )

   {
	meansofdeath = (meansOfDeath_t )MOD_NameToNum( ev->GetString( 1 ) );
   }

void Projectile::DoDecal
	(
	void
	)

{
	if( impactmarkshader.length() )
	{
		Decal *decal = new Decal;
		decal->setShader( impactmarkshader );
		decal->setOrigin( level.impact_trace.endpos );
		decal->setDirection( level.impact_trace.plane.normal );
		decal->setOrientation( impactmarkorientation );
		decal->setRadius( impactmarkradius );
	}
}

void Projectile::Touch
	(
	Event *ev
	)

{
	Entity   *other;
	Entity   *owner;
	str      realname;

	// Other is what got hit
	other = ev->GetEntity( 1 );
	assert( other );

	// Don't touch teleporters
	if( other->isSubclassOf( Teleporter ) )
	{
		return;
	}

	// Can't hit yourself with a projectile
	if( other->entnum == this->owner )
	{
		return;
	}

	// Remove it if we hit the sky
	if( HitSky() )
	{
		PostEvent( EV_Remove, 0 );
		return;
	}

	if( !CheckTeams() )
	{
		PostEvent( EV_Remove, EV_REMOVE );
		return;
	}

	// Bouncy Projectile
	if( ( projFlags & P_BOUNCE_TOUCH ) )
	{
		str snd;

		if( level.time - fLastBounceTime < 0.1f )
		{
			fLastBounceTime = level.time;
			return;
		}

		if( level.impact_trace.surfaceFlags & CONTENTS_WATER || ( gi.PointContents( level.impact_trace.endpos, 0 ) & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) )
		{
			if( bouncesound_water.length() )
				this->Sound( bouncesound_water, CHAN_BODY );
		}
		else
		{
			if( bouncesound_metal.length() )
			{
				snd = bouncesound_metal;
			}
			else if( bouncesound_hard.length() )
			{
				snd = bouncesound_hard;
			}
			else
			{
				snd = bouncesound;
			}

			int flags = level.impact_trace.surfaceFlags;

			if( flags & SURF_MUD )
			{
				if( bouncesound.length() )
					Sound( bouncesound, CHAN_BODY );
			}
			else if( flags & SURF_ROCK )
			{
				if( bouncesound_hard.length() )
					Sound( bouncesound_hard, CHAN_BODY );
			}
			else if( flags & SURF_GRILL )
			{
				if( bouncesound_metal.length() )
					Sound( bouncesound_metal, CHAN_BODY );
			}
			else if( flags & SURF_WOOD )
			{
				if( bouncesound_hard.length() )
					Sound( bouncesound_hard, CHAN_BODY );
			}
			else if( flags & SURF_METAL )
			{
				if( bouncesound_metal.length() )
					Sound( bouncesound_metal, CHAN_BODY );
			}
			else if( flags & SURF_GLASS )
			{
				if( bouncesound_hard.length() )
					Sound( bouncesound_hard, CHAN_BODY );
			}
			else
			{
				if( bouncesound.length() )
					Sound( bouncesound, CHAN_BODY );
			}
		}

		BroadcastAIEvent( AI_EVENT_WEAPON_IMPACT );
		return;
	}

	if( !m_bExplodeOnTouch && damage == 0.0f )
		return;

	// Get the owner of this projectile
	owner = G_GetEntity( this->owner );

	// If the owner's not here, make the world the owner
	if( !owner )
		owner = world;

	if( owner->IsDead() || owner == world )
		weap = NULL;

	// Damage the thing that got hit
	if( other->takedamage )
	{
		other->Damage( this,
			owner,
			damage,
			origin,
			velocity,
			level.impact_trace.plane.normal,
			knockback,
			0,
			meansofdeath
			);
	}

	if( !g_gametype->integer && weap )
	{
		if( other->IsSubclassOfPlayer() ||
			other->IsSubclassOfVehicle() ||
			other->IsSubclassOfVehicleTank() ||
			other->isSubclassOf( VehicleCollisionEntity ) )
		{
			weap->m_iNumHits++;
			weap->m_iNumTorsoShots++;

			if( weap->IsSubclassOfVehicleTurretGun() )
			{
				VehicleTurretGun *t = ( VehicleTurretGun * )weap.Pointer();
				Player *p = ( Player * )t->GetRemoteOwner();

				if( p && p->IsSubclassOfPlayer() )
				{
					p->m_iNumHits++;
					p->m_iNumTorsoShots++;
				}
			}
		}
	}

	if( !m_bExplodeOnTouch )
		return;

	// Make the projectile not solid
	setSolidType( SOLID_NOT );
	setMoveType( MOVETYPE_NONE );
	hideModel();

	// Do a decal
	DoDecal();

	BroadcastAIEvent( AI_EVENT_WEAPON_FIRE );

	// Remove the projectile
	PostEvent( EV_Remove, 0 );

	// Call the explosion event
	Event *explEv;
	explEv = new Event( EV_Projectile_Explode );
	explEv->AddEntity( other );
	ProcessEvent( explEv );
}

void Projectile::SetCanHitOwner
   (
   Event *ev
   )

   {
	can_hit_owner = true;
	}

void Projectile::ClearOwner
   (
   Event *ev
   )

   {
	this->owner     = ENTITYNUM_NONE;
	edict->r.ownerNum = ENTITYNUM_NONE;
	}

void Projectile::RemoveWhenStopped
	(
	Event *ev
	)

{
	remove_when_stopped = true;
}

void Projectile::ExplodeOnTouch
	(
	Event *ev
	)

{
	m_bExplodeOnTouch = true;
}

Sentient *Projectile::GetOwner
	(
	void
	)
{
	Sentient *pOwner = (Sentient *)G_GetEntity(owner);
	if (!pOwner || !pOwner->IsSubclassOfSentient())
		return NULL;
	else
		return pOwner;
}

void Projectile::Stopped
   (
   Event *ev
   )

   {
	if ( remove_when_stopped )
		PostEvent( EV_Remove, 0 );
	}

bool Projectile::CheckTeams
	(
	void
	)

{
	Player *pOwner = ( Player * )m_pOwnerPtr.Pointer();

	if( !pOwner )
	{
		// Owner disconnected
		if( m_bHadPlayerOwner )
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	if( pOwner->IsSubclassOfPlayer() )
	{
		if( ( ( m_iOwnerTeam != pOwner->GetTeam() && g_gametype->integer >= GT_TEAM ) || pOwner->GetTeam() <= TEAM_SPECTATOR ) )
		{
			return false;
		}
	}
	else if( pOwner->IsSubclassOfSentient() )
	{
		if( m_iOwnerTeam != pOwner->m_Team )
		{
			return false;
		}
	}

	return true;
}

void Projectile::SetOwner
	(
	Entity *owner
	)

{
	this->owner = owner->entnum;
	m_pOwnerPtr = owner;

	if( owner->IsSubclassOfPlayer() )
	{
		Player *p = ( Player * )owner;

		m_iOwnerTeam = p->GetTeam();
		m_bHadPlayerOwner = true;
	}
	else if( owner->IsSubclassOfSentient() )
	{
		Sentient *s = ( Sentient * )owner;
		m_iOwnerTeam = s->m_Team;
	}
}

Event EV_Explosion_Radius
   (
   "radius",
   EV_DEFAULT,
   "f",
   "projectileRadius",
   "set the radius for the explosion",
   EV_NORMAL
   );
Event EV_Explosion_ConstantDamage
   (
   "constantdamage",
   EV_DEFAULT,
   NULL,
   NULL,
   "Makes the explosion do constant damage over the radius",
   EV_NORMAL
   );
Event EV_Explosion_DamageEveryFrame
   (
   "damageeveryframe",
   EV_DEFAULT,
   NULL,
   NULL,
   "Makes the explosion damage every frame",
   EV_NORMAL
   );
Event EV_Explosion_DamageAgain
   (
   "damageagain",
   EV_DEFAULT,
   NULL,
   NULL,
   "This event is generated each frame if explosion is set to damage each frame",
   EV_NORMAL
   );
Event EV_Explosion_Flash
   (
   "flash",
   EV_DEFAULT,
   "fffff",
   "time r g b radius",
   "Flash player screens",
   EV_NORMAL
   );
Event EV_Explosion_RadiusDamage
   (
   "radiusdamage",
   EV_DEFAULT,
   "f",
   "radiusDamage",
   "set the radius damage an explosion does",
   EV_NORMAL
   );
Event EV_Explosion_Effect
	(
	"explosioneffect",
	EV_DEFAULT,
	"s",
	"explosiontype",
	"Make an explosionType explosion effect",
	EV_NORMAL
	);


CLASS_DECLARATION( Projectile, Explosion, NULL )
{
	{ &EV_Explosion_Radius,						&Explosion::SetRadius },
	{ &EV_Explosion_ConstantDamage,				&Explosion::SetConstantDamage },
	{ &EV_Explosion_DamageEveryFrame,			&Explosion::SetDamageEveryFrame },
	{ &EV_Explosion_DamageAgain,				&Explosion::DamageAgain },
	{ &EV_Explosion_Flash,			 	      	&Explosion::SetFlash },
	{ &EV_Explosion_RadiusDamage,		      	&Explosion::SetRadiusDamage },
	{ &EV_Explosion_Effect,	   					&Explosion::MakeExplosionEffect },
	{ NULL, NULL }
};

Explosion::Explosion()
{
	if( LoadingSavegame )
	{
		// Archive function will setup all necessary data
		return;
	}

	radius = 0;
	constant_damage = false;
	damage_every_frame = false;
	flash_time = 0;
	flash_type = 0;
	owner = ENTITYNUM_NONE;
	hurtOwnerOnly = false;
}

void Explosion::SetFlash
   (
   Event *ev
   )

   {
   flash_time     = ev->GetFloat( 1 );
   flash_r        = ev->GetFloat( 2 );
   flash_g        = ev->GetFloat( 3 );
   flash_b        = ev->GetFloat( 4 );
   flash_a        = ev->GetFloat( 5 );
   flash_radius   = ev->GetFloat( 6 );
   flash_type     = 0;

   if ( ev->NumArgs() > 6 ) 
      {
      str t = ev->GetString( 7 );
      
      if ( !t.icmp( "addblend" ) )
         flash_type = 1;
      else if ( !t.icmp( "alphablend" ) )
         flash_type = 0;
      }
   }

void Explosion::SetRadius
   (
   Event *ev
   )

   {
   radius = ev->GetFloat( 1 );
   }

void Explosion::SetRadiusDamage
   (
   Event *ev
   )

   {
   radius_damage = ev->GetFloat( 1 );
   }

void Explosion::SetConstantDamage
   (
   Event *ev
   )

   {
   constant_damage = true;
   }

void Explosion::SetDamageEveryFrame
   (
   Event *ev
   )

   {
   damage_every_frame = true;
   }

void Explosion::DamageAgain
	(
	Event *ev
	)

{
	Entity *owner_ent;
	Weapon *weapon;
	float dmg;
	float rad;

	if( !CheckTeams() )
	{
		PostEvent( EV_Remove, EV_REMOVE );
		return;
	}

	owner_ent = G_GetEntity( owner );

	if( !owner_ent )
		owner_ent = world;

	weapon = weap;

	if( owner_ent == world || owner_ent->IsDead() )
	{
		weap = NULL;
	}

	dmg = radius_damage;
	rad = radius;

	if( rad == 0.0f )
	{
		rad = radius_damage + 60.0f;
	}

	RadiusDamage( origin, this, owner_ent, dmg, NULL, meansofdeath, rad, knockback, constant_damage, weapon, false );

	PostEvent( EV_Explosion_DamageAgain, level.frametime );
}

void Explosion::MakeExplosionEffect
	(
	Event *ev
	)

{
	str sEffect = ev->GetString( 1 );

	gi.SetBroadcastAll();

	if( !sEffect.icmp( "grenade" ) )
	{
		gi.MSG_StartCGM( 12 );
	}
	else
	{
		gi.MSG_StartCGM( 13 );
	}

	gi.MSG_WriteCoord( origin[ 0 ] );
	gi.MSG_WriteCoord( origin[ 1 ] );
	gi.MSG_WriteCoord( origin[ 2 ] );
	gi.MSG_EndCGM();
}

Projectile *ProjectileAttack
	(
	Vector			start,
	Vector			dir,
	Entity			*owner,
	str				projectileModel,
	float			fraction,
	float			real_speed,
	Weapon			*weap
	)

{
	Event          *ev;
	Projectile     *proj = NULL;
	float          newspeed, newlife;
	SpawnArgs      args;
	Entity         *obj;
	float          dot = 0;

	if( !projectileModel.length() )
	{
		gi.DPrintf( "ProjectileAttack : No model specified for ProjectileAttack" );
		return NULL;
	}

	args.setArg( "model", projectileModel );
	obj = ( Entity * )args.Spawn();

	if( !obj )
	{
		gi.DPrintf( "projectile model '%s' not found\n", projectileModel.c_str() );
		return NULL;
	}

	if( obj->IsSubclassOfProjectile() )
		proj = ( Projectile * )obj;
	else
		gi.DPrintf( "%s is not of class projectile\n", projectileModel.c_str() );

	if( !proj )
		return NULL;

	// Create a new projectile entity and set it off
	proj->setModel( projectileModel );
	proj->setMoveType( MOVETYPE_BOUNCE );
	proj->ProcessInitCommands();
	proj->SetOwner( owner );
	proj->edict->r.ownerNum = owner->entnum;
	proj->angles = dir.toAngles();
	proj->charge_fraction = fraction;

	if( proj->projFlags & P_CHARGE_SPEED )
	{
		newspeed = proj->speed * fraction;

		if( newspeed < proj->minspeed )
			newspeed = proj->minspeed;
	}
	else
	{
		newspeed = proj->speed;
	}

	if( real_speed )
		newspeed = real_speed;

	if( proj->addownervelocity )
	{
		dot = DotProduct( owner->velocity, dir );
		if( dot < 0 )
			dot = 0;
	}

	proj->velocity = dir * ( newspeed + dot );
	proj->velocity += proj->addvelocity;
	proj->setAngles( proj->angles );
	proj->setSolidType( SOLID_BBOX );

	proj->edict->clipmask = MASK_PROJECTILE;

	proj->setSize( proj->mins, proj->maxs );
	proj->setOrigin( start );
	proj->origin.copyTo( proj->edict->s.origin2 );

	if( proj->m_beam )
	{
		proj->m_beam->setOrigin( start );
		proj->m_beam->origin.copyTo( proj->m_beam->edict->s.origin2 );
	}

	if( proj->dlight_radius )
	{
		G_SetConstantLight( &proj->edict->s.constantLight,
			&proj->dlight_color[ 0 ],
			&proj->dlight_color[ 1 ],
			&proj->dlight_color[ 2 ],
			&proj->dlight_radius
			);
	}

	// Calc the life of the projectile
	if( proj->projFlags & P_CHARGE_LIFE )
	{
		if( g_gametype->integer && proj->dmlife )
			newlife = proj->dmlife * fraction;
		else
			newlife = proj->life * fraction;

		if( newlife < proj->minlife )
			newlife = proj->minlife;
	}
	else
	{
		if( g_gametype->integer && proj->dmlife )
			newlife = proj->dmlife;
		else
			newlife = proj->life;
	}

	// Remove the projectile after it's life expires
	ev = new Event( EV_Projectile_Explode );
	proj->PostEvent( ev, newlife );

	proj->NewAnim( "idle" );

	// If can hit owner clear the owner of this projectile in a second

	if( proj->can_hit_owner )
		proj->PostEvent( EV_Projectile_ClearOwner, 1 );

	if( !g_gametype->integer )
	{
		if( weap )
		{
			weap->m_iNumShotsFired++;
			if( owner->IsSubclassOfPlayer() &&
				weap->IsSubclassOfTurretGun() )
			{
				Player *p = ( Player * )owner;
				p->m_iNumShotsFired++;
			}
		}
	}

	return proj;
}

float BulletAttack
	(
	Vector		start,
	Vector		vBarrel,
	Vector		dir,
	Vector		right,
	Vector		up,
	float		range,
	float		damage,
	float		knockback,
	int			dflags,
	int			meansofdeath,
	Vector		spread,
	int			count,
	Entity		*owner,
	int			iTracerFrequency,
	int			*piTracerCount,
	Weapon		*weap
	)

{
	Vector		vDir;
	Vector		vTmpEnd;
	Vector		vTraceStart;
	Vector		vTraceEnd;
	int			i;
	int			iTravelDist;
	trace_t		trace;
	Entity		*ent;
	//Entity		*tmpSkipEnt;
	float		damage_total = 0;
	float		original_value;
	qboolean	bLargeBullet;
	qboolean	bBulletDone;
	int			iContinueCount;
	float		vEndArray[ 64 ][ 3 ];
	int			iTracerCount = 0;
	static int	iNumHit;

	iNumHit = 0;
	bLargeBullet = damage >= 41.0f;

	if( count > 63 ) {
		count = 63;
	}

	if( !owner || owner->IsDead() || owner == world ) {
		weap = NULL;
	}

	for( i = 0; i < count; i++ )
	{
		vTraceEnd = start + ( dir * range ) +
			( right	* grandom() * spread.x ) +
			( up	* grandom() * spread.y );

		vTmpEnd = vTraceEnd - start;

		VectorNormalizeFast( vTmpEnd );

		iContinueCount = 0;
		iTravelDist = 0;
		bBulletDone = qfalse;

		while( !bBulletDone && iTravelDist < 9216 )
		{
			iTravelDist += 9216;
			vTraceEnd = start + vTmpEnd * iTravelDist;

			memset( &trace, 0, sizeof( trace_t ) );

			while( trace.fraction < 1.0f )
			{
				trace = G_Trace( start, vec_zero, vec_zero, vTraceEnd, owner, MASK_SHOT, false, "BulletAttack", true );

				if( trace.ent )
				{
					ent = trace.ent->entity;

					if( ent != world )
					{

						if( trace.ent->entity->takedamage ) {
							break;
						}

						if( ent->edict->solid == 3 && trace.shaderNum >= 0 )
						{
							gi.SetBroadcastVisible( trace.endpos, 0 );
							gi.MSG_StartCGM( 6 );
							gi.MSG_WriteCoord( trace.endpos[ 0 ] );
							gi.MSG_WriteCoord( trace.endpos[ 1 ] );
							gi.MSG_WriteCoord( trace.endpos[ 2 ] );
							gi.MSG_WriteDir( trace.plane.normal );
							gi.MSG_WriteBits( bLargeBullet, 1 );
							gi.MSG_EndCGM();
						}
					}
				}
				else
				{
					ent = NULL;
				}

				if( trace.fraction < 1.0f )
				{
					if( trace.surfaceFlags & ( SURF_SNOW | SURF_FOLIAGE | SURF_GLASS | SURF_PUDDLE | SURF_PAPER ) ||
						trace.contents & ( CONTENTS_NOTTEAM1 | CONTENTS_WATER ) ||
						trace.startsolid ||
						( bLargeBullet && trace.ent && !trace.ent->r.bmodel && trace.ent->entity->takedamage ))
					{
						if( iContinueCount <= 4 )
						{
							vTraceEnd = vTmpEnd + trace.endpos;

							if( g_showbullettrace->integer )
							{
								G_DebugLine( start, vTraceEnd, 1, 1, 1, 1 );
							}

							iContinueCount++;
							continue;
						}
					}
				}

				trace.fraction = 1.0f;
				bBulletDone = qtrue;
			}

			if( !g_gametype->integer && !iNumHit && ent->IsSubclassOfPlayer() && weap )
			{
				weap->m_iNumHits++;
				switch( trace.location )
				{
				case LOCATION_HEAD:
				case LOCATION_HELMET:
				case LOCATION_NECK:
					weap->m_iNumHeadShots++;
					break;
				case LOCATION_PELVIS:
					weap->m_iNumGroinShots++;
					break;
				case LOCATION_R_ARM_UPPER:
				case LOCATION_R_ARM_LOWER:
				case LOCATION_R_HAND:
					weap->m_iNumRightArmShots++;
					break;
				case LOCATION_L_ARM_UPPER:
				case LOCATION_L_ARM_LOWER:
				case LOCATION_L_HAND:
					weap->m_iNumLeftArmShots++;
					break;
				case LOCATION_R_LEG_UPPER:
				case LOCATION_R_LEG_LOWER:
				case LOCATION_R_FOOT:
					weap->m_iNumRightLegShots++;
					break;
				case LOCATION_L_LEG_UPPER:
				case LOCATION_L_LEG_LOWER:
				case LOCATION_L_FOOT:
					weap->m_iNumLeftLegShots++;
					break;
				default:
					weap->m_iNumTorsoShots++;
					break;
				}

				if( owner && owner->IsSubclassOfPlayer() && weap && weap->IsSubclassOfTurretGun() )
				{
					Player *p = ( Player * )owner;

					p->m_iNumHits++;

					switch( trace.location )
					{
					case LOCATION_HEAD:
					case LOCATION_HELMET:
					case LOCATION_NECK:
						p->m_iNumHeadShots++;
						break;
					case LOCATION_PELVIS:
						p->m_iNumGroinShots++;
						break;
					case LOCATION_R_ARM_UPPER:
					case LOCATION_R_ARM_LOWER:
					case LOCATION_R_HAND:
						p->m_iNumRightArmShots++;
						break;
					case LOCATION_L_ARM_UPPER:
					case LOCATION_L_ARM_LOWER:
					case LOCATION_L_HAND:
						p->m_iNumLeftArmShots++;
						break;
					case LOCATION_R_LEG_UPPER:
					case LOCATION_R_LEG_LOWER:
					case LOCATION_R_FOOT:
						p->m_iNumRightLegShots++;
						break;
					case LOCATION_L_LEG_UPPER:
					case LOCATION_L_LEG_LOWER:
					case LOCATION_L_FOOT:
						p->m_iNumLeftLegShots++;
						break;
					default:
						p->m_iNumTorsoShots++;
						break;
					}
				}
			}

			if( ent && ent->takedamage )
			{
				/* if ( !ent->deadflag )
				damage_total += damage;

				if ( ent->IsSubclassOfSentient() )
				{
				Sentient *sent = (Sentient *)ent;

				if ( sent->Immune( meansofdeath ) )
				damage_total = 0;
				} */

				// Get the original value of the victims health or water

				original_value = ent->health;

				ent->Damage( NULL,
					owner,
					damage,
					trace.endpos,
					dir,
					trace.plane.normal,
					knockback,
					dflags,
					meansofdeath,
					trace.location );

				// Get the new value of the victims health or water

				damage_total += original_value - ent->health;
			}

			if( trace.contents > 0 && trace.surfaceFlags & 0xFFFE000 )
			{
				gi.SetBroadcastVisible( trace.endpos, NULL );
				gi.MSG_StartCGM( 6 );
				gi.MSG_WriteCoord( trace.endpos[ 0 ] );
				gi.MSG_WriteCoord( trace.endpos[ 1 ] );
				gi.MSG_WriteCoord( trace.endpos[ 2 ] );
				gi.MSG_WriteDir( trace.plane.normal );
				gi.MSG_WriteBits( bLargeBullet, 1 );
				gi.MSG_EndCGM();
			}
			else if( trace.location >= 0 && ent->IsSubclassOfPlayer() )
			{
				gi.SetBroadcastVisible( trace.endpos, NULL );
				gi.MSG_StartCGM( 7 );
				gi.MSG_WriteCoord( trace.endpos[ 0 ] );
				gi.MSG_WriteCoord( trace.endpos[ 1 ] );
				gi.MSG_WriteCoord( trace.endpos[ 2 ] );
				gi.MSG_WriteDir( trace.plane.normal );
				gi.MSG_WriteBits( bLargeBullet, 1 );
				gi.MSG_EndCGM();
			}
		}

		VectorCopy( vTraceEnd, vEndArray[ i ] );

		if( iTracerFrequency && piTracerCount )
		{
			( *piTracerCount )++;

			if( *piTracerCount == iTracerFrequency )
			{
				iTracerCount++;
				*piTracerCount = 0;
			}
		}

		// Draw a debug trace line to show bullet fire  
		if ( g_showbullettrace->integer )
			G_DebugLine( start, vTraceEnd, 1, 1, 1, 1 );
	}

	if( !g_gametype->integer )
	{
		if( weap )
		{
			weap->m_iNumShotsFired++;
			if( owner && owner->IsSubclassOfPlayer() && weap->IsSubclassOfTurretGun() )
			{
				Player *p = ( Player * )owner;
				p->m_iNumShotsFired++;
			}
		}
	}

	gi.SetBroadcastVisible( start, trace.endpos );

	if( count == 1 )
	{
		if( iTracerCount )
		{
			gi.MSG_StartCGM( 1 );

			gi.MSG_WriteCoord( vBarrel[ 0 ] );
			gi.MSG_WriteCoord( vBarrel[ 1 ] );
			gi.MSG_WriteCoord( vBarrel[ 2 ] );
		}
		else
		{
			gi.MSG_StartCGM( 2 );
		}

		gi.MSG_WriteCoord( start[ 0 ] );
		gi.MSG_WriteCoord( start[ 1 ] );
		gi.MSG_WriteCoord( start[ 2 ] );
		gi.MSG_WriteCoord( trace.endpos[ 0 ] );
		gi.MSG_WriteCoord( trace.endpos[ 1 ] );
		gi.MSG_WriteCoord( trace.endpos[ 2 ] );
		gi.MSG_WriteBits( bLargeBullet, 1 );
	}
	else
	{
		if( iTracerCount )
		{
			gi.MSG_StartCGM( 3 );

			gi.MSG_WriteCoord( vBarrel[ 0 ] );
			gi.MSG_WriteCoord( vBarrel[ 1 ] );
			gi.MSG_WriteCoord( vBarrel[ 2 ] );

			if( iTracerCount > 63 )
				iTracerCount = 63;

			gi.MSG_WriteBits( iTracerCount, 6 );
		}
		else
		{
			gi.MSG_StartCGM( 4 );
		}

		gi.MSG_WriteCoord( start[ 0 ] );
		gi.MSG_WriteCoord( start[ 1 ] );
		gi.MSG_WriteCoord( start[ 2 ] );
		gi.MSG_WriteBits( bLargeBullet, 1 );
		gi.MSG_WriteBits( count, 6 );

		for( int i = count; i > 0; i-- )
		{
			gi.MSG_WriteCoord( vEndArray[ i ][ 0 ] );
			gi.MSG_WriteCoord( vEndArray[ i ][ 1 ] );
			gi.MSG_WriteCoord( vEndArray[ i ][ 2 ] );
		}
	}

	gi.MSG_EndCGM();

	if( damage_total > 0 )
		return damage_total;
	else
		return 0;
}

void FakeBulletAttack
	(
	Vector			start,
	Vector			vBarrel,
	Vector			dir,
	Vector			right,
	Vector			up,
	float			range,
	float			damage,
	Vector			spread,
	int				count,
	Entity			*owner,
	int				iTracerFrequency,
	int				*piTracerCount
	)

{
	Vector vDir;
	Vector vTraceEnd;
	int i;
	qboolean bLargeBullet = damage >= 41.0f;

	for( i = 0; i < count; i++ )
	{
		vTraceEnd = start + ( dir * range ) +
			( right	* grandom() * spread.x ) +
			( up	* grandom() * spread.y );

		vDir = vTraceEnd - start;
		VectorNormalize( vDir );
		vTraceEnd = start + vDir * 9216.0f;

		gi.SetBroadcastVisible( start, vTraceEnd );
		if( iTracerFrequency && piTracerCount )
		{
			( *piTracerCount )++;
			if( *piTracerCount == iTracerFrequency )
			{
				gi.MSG_StartCGM( 25 );
				gi.MSG_WriteCoord( vBarrel[ 0 ] );
				gi.MSG_WriteCoord( vBarrel[ 1 ] );
				gi.MSG_WriteCoord( vBarrel[ 2 ] );
				*piTracerCount = 0;
			}
			else
			{
				gi.MSG_StartCGM( 26 );
			}
		}
		else
		{
			gi.MSG_StartCGM( 26 );
		}

		gi.MSG_WriteCoord( start[ 0 ] );
		gi.MSG_WriteCoord( start[ 1 ] );
		gi.MSG_WriteCoord( start[ 2 ] );
		gi.MSG_WriteCoord( vTraceEnd[ 0 ] );
		gi.MSG_WriteCoord( vTraceEnd[ 1 ] );
		gi.MSG_WriteCoord( vTraceEnd[ 2 ] );
		gi.MSG_WriteBits( bLargeBullet, 1 );
		gi.MSG_EndCGM();
	}
}

void ClickItemAttack
	(
	Vector			vStart,
	Vector			vForward,
	float			fRange,
	Entity			*pOwner
	)

{
	Vector vEnd;
	trace_t trace;

	vEnd = vStart + vForward * fRange;

	trace = G_Trace(
		vStart,
		vec_zero,
		vec_zero,
		vEnd,
		pOwner,
		MASK_ALL,
		qfalse,
		"ClickItemAttack" );

	if( g_showbullettrace->integer )
		G_DebugLine( vStart, vEnd, 1, 1, 1, 1 );

	if( trace.entityNum != ENTITYNUM_NONE && trace.ent &&
		trace.ent->entity && trace.ent->entity->isSubclassOf( TriggerClickItem ) )
	{
		Event *ev = new Event( EV_Activate );
		ev->AddEntity( pOwner );
		trace.ent->entity->PostEvent( ev, 0 );
	}
	else
	{
		ScriptThreadLabel failThread;

		// Try to execute a fail thread
		if( failThread.TrySet( "clickitem_fail" ) )
			failThread.Execute();
	}
}

Projectile *HeavyAttack
	(
	Vector			start,
	Vector			dir,
	str				projectileModel,
	float			real_speed,
	Entity			*owner,
	Weapon			*weap
	)

{
	Event          *ev;
	Projectile     *proj = NULL;
	float          newspeed, newlife;
	SpawnArgs      args;
	Entity         *obj;
	float          dot = 0;

	if( !projectileModel.length() )
	{
		gi.DPrintf( "ProjectileAttack : No model specified for ProjectileAttack" );
		return NULL;
	}

	args.setArg( "model", projectileModel );
	obj = ( Entity * )args.Spawn();

	if( !obj )
	{
		gi.DPrintf( "projectile model '%s' not found\n", projectileModel.c_str() );
		return NULL;
	}

	if( obj->IsSubclassOfProjectile() )
		proj = ( Projectile * )obj;
	else
		gi.DPrintf( "%s is not of class projectile\n", projectileModel.c_str() );

	if( !proj )
		return NULL;

	// Create a new projectile entity and set it off
	proj->setModel( projectileModel );
	proj->setMoveType( MOVETYPE_BOUNCE );
	proj->ProcessInitCommands();
	proj->SetOwner( owner );
	proj->edict->r.ownerNum = owner->entnum;
	proj->angles = dir.toAngles();
	proj->charge_fraction = 1.0f;

	newspeed = real_speed;
	if( real_speed == 0.0f )
		newspeed = proj->speed;

	if( proj->addownervelocity )
	{
		dot = DotProduct( owner->velocity, dir );
		if( dot < 0 )
			dot = 0;
	}

	proj->velocity = dir * ( newspeed + dot );
	proj->velocity += proj->addvelocity;
	proj->setAngles( proj->angles );
	proj->setSolidType( SOLID_BBOX );

	proj->edict->clipmask = MASK_PROJECTILE;

	proj->setSize( proj->mins, proj->maxs );
	proj->setOrigin( start );
	proj->origin.copyTo( proj->edict->s.origin2 );

	if( proj->m_beam )
	{
		proj->m_beam->setOrigin( start );
		proj->m_beam->origin.copyTo( proj->m_beam->edict->s.origin2 );
	}

	if( proj->dlight_radius )
	{
		G_SetConstantLight( &proj->edict->s.constantLight,
			&proj->dlight_color[ 0 ],
			&proj->dlight_color[ 1 ],
			&proj->dlight_color[ 2 ],
			&proj->dlight_radius
			);
	}

	// Calc the life of the projectile
	if( proj->projFlags & P_CHARGE_LIFE )
	{
		if( g_gametype->integer && proj->dmlife )
			newlife = proj->dmlife;
		else
			newlife = proj->life;

		if( newlife < proj->minlife )
			newlife = proj->minlife;
	}
	else
	{
		if( g_gametype->integer && proj->dmlife )
			newlife = proj->dmlife;
		else
			newlife = proj->life;
	}

	// Remove the projectile after it's life expires
	ev = new Event( EV_Projectile_Explode );
	proj->PostEvent( ev, newlife );

	proj->NewAnim( "idle" );

	if( !g_gametype->integer )
	{
		if( weap )
		{
			weap->m_iNumShotsFired++;
			if( owner->IsSubclassOfPlayer() &&
				weap->IsSubclassOfTurretGun() )
			{
				Player *p = ( Player * )owner;
				p->m_iNumShotsFired++;
			}
		}
	}

	return proj;
}

void ExplosionAttack
	(
	Vector		pos,
	Entity		*owner,
	str			explosionModel,
	Vector		dir,
	Entity		*ignore,
	float		scale,
	Weapon		*weap,
	bool		hurtOwnerOnly
	)

{
	Explosion   *explosion;
	Event			*ev;


	if( !owner )
		owner = world;

	if( owner->IsDead() || owner == world )
		weap = NULL;

	if( explosionModel.length() )
	{
		explosion = new Explosion;

		// Create a new explosion entity and set it off
		explosion->setModel( explosionModel );

		explosion->setSolidType( SOLID_NOT );

		// Process the INIT commands right away
		explosion->ProcessInitCommands();

		explosion->SetOwner( owner );

		explosion->edict->r.ownerNum = owner->entnum;
		explosion->angles = dir.toAngles();
		explosion->velocity = dir * explosion->speed;
		explosion->edict->s.scale = scale;
		explosion->setAngles( explosion->angles );
		explosion->setMoveType( MOVETYPE_FLYMISSILE );
		explosion->edict->clipmask = MASK_PROJECTILE;
		explosion->setSize( explosion->mins, explosion->maxs );
		explosion->setOrigin( pos );
		explosion->origin.copyTo( explosion->edict->s.origin2 );
		explosion->hurtOwnerOnly = hurtOwnerOnly;

		if( explosion->dlight_radius )
		{
			G_SetConstantLight( &explosion->edict->s.constantLight,
				&explosion->dlight_color[ 0 ],
				&explosion->dlight_color[ 1 ],
				&explosion->dlight_color[ 2 ],
				&explosion->dlight_radius
				);
		}

		explosion->BroadcastAIEvent( AI_EVENT_WEAPON_FIRE );
		explosion->NewAnim( "idle" );

		RadiusDamage( explosion->origin,
			explosion,
			owner,
			explosion->radius_damage * scale,
			ignore,
			explosion->meansofdeath,
			explosion->radius * scale,
			explosion->knockback,
			explosion->constant_damage,
			weap,
			explosion->hurtOwnerOnly
			);

		if( explosion->flash_radius )
		{
			FlashPlayers( explosion->origin,
				explosion->flash_r,
				explosion->flash_g,
				explosion->flash_b,
				explosion->flash_a,
				explosion->flash_radius * scale,
				explosion->flash_time,
				explosion->flash_type
				);
		}

		if( explosion->damage_every_frame )
		{
			explosion->PostEvent( EV_Explosion_DamageAgain, FRAMETIME );
		}

		// Remove explosion after the life has expired
		if( explosion->life || ( g_gametype->integer && explosion->dmlife ) )
		{
			ev = new Event( EV_Remove );

			if( g_gametype->integer && explosion->dmlife )
				explosion->PostEvent( ev, explosion->dmlife );
			else
				explosion->PostEvent( ev, explosion->life );
		}
	}
}

void StunAttack
	(
   Vector   pos,
   Entity   *attacker,
   Entity   *inflictor,
   float    radius,
   float    time,
   Entity   *ignore
   )

	{
   Entity *ent;

   ent = findradius( NULL, inflictor->origin, radius );

	while( ent )
		{
		if ( ( ent != ignore ) && ( ( ent->takedamage ) || ent->IsSubclassOfActor() ) )
			{ 
         if ( inflictor->CanDamage( ent, attacker ) )
				{
            // Fixme : Add in knockback
            ent->Stun( time );
				}
			}
      ent = findradius( ent, inflictor->origin, radius );
		}
	}

static float radiusdamage_origin[ 3 ];

static int radiusdamage_compare( const void *elem1, const void *elem2 )
{
	Entity *e1, *e2;
	float delta[ 3 ];
	float d1, d2;

	e1 = *( Entity ** )elem1;
	e2 = *( Entity ** )elem2;

	VectorSubtract( radiusdamage_origin, e1->origin, delta );
	d1 = VectorLengthSquared( delta );

	VectorSubtract( radiusdamage_origin, e2->origin, delta );
	d2 = VectorLengthSquared( delta );

	if( d2 <= d1 )
	{
		return d1 > d2;
	}
	else
	{
		return -1;
	}
}

void RadiusDamage
	(
	Vector		origin,
	Entity		*inflictor,
	Entity		*attacker,
	float		damage,
	Entity		*ignore,
	int			mod,
	float		radius,
	float		knockback,
	qboolean	constant_damage,
	Weapon		*weap,
	bool		hurtOwnerOnly
	)

{
	float		points;
	Entity		*ent;
	Vector		org;
	Vector		dir;
	float		dist;
	int			i;
	Container< Entity * > ents;

	if( g_showdamage->integer )
	{
		Com_Printf( "radiusdamage" );
		Com_Printf( "{\n" );
		Com_Printf( "origin: %f %f %f\n", origin[ 0 ], origin[ 1 ], origin[ 2 ] );

		if( inflictor ) {
			Com_Printf( "inflictor: (entnum %d, radnum %d)\n", inflictor->entnum, inflictor->radnum );
		}

		if( attacker ) {
			Com_Printf( "attacker: (entnum %d, radnum %d)\n", attacker->entnum, attacker->radnum );
		}

		Com_Printf( "damage: %f\n", damage );

		if( ignore ) {
			Com_Printf( "ignore: (entnum %d, radnum %d)\n", ignore->entnum, ignore->radnum );
		}

		Com_Printf( "mod: %d\n", mod );
		Com_Printf( "radius: %f\n", radius );
		Com_Printf( "knockback: %f\n", knockback );
		Com_Printf( "constant_damage: %d\n", constant_damage );

		if( weap )
		{
			Com_Printf( "weapon %s\n", weap->getName().c_str() );
		}

		Com_Printf( "hurtOwnerOnly: %d\n", hurtOwnerOnly );
		Com_Printf( "}\n" );
	}

	ent = findradius( NULL, origin, radius );

	while( ent )
	{
		// Add ents that has contents
		if( ent->getContents() )
		{
			ents.AddObject( ent );
		}

		ent = findradius( ent, origin, radius );
	}

	// Sort by the nearest to the fartest entity
	if( ents.NumObjects() )
	{
		VectorCopy( origin, radiusdamage_origin );
		ents.Sort( radiusdamage_compare );
	}

	for( i = 1; i <= ents.NumObjects(); i++ )
	{
		ent = ents.ObjectAt( i );

		if( ent == ignore || !( ent->takedamage ) || ( hurtOwnerOnly && ent != attacker ) ) {
			continue;
		}

		// Add this in for deathmatch maybe

		if( ent->getContents() == CONTENTS_NOTTEAM1 ||
			G_SightTrace( origin, vec_zero, vec_zero, ent->centroid, inflictor, ent, MASK_EXPLOSION, false, "RadiusDamage" ) )
		{
			if( constant_damage )
			{
				points = damage;
			}
			else
			{
				float ent_rad;

				ent_rad = fabs( ent->maxs[ 0 ] - ent->mins[ 0 ] );

				if( ent_rad < fabs( ent->maxs[ 1 ] - ent->mins[ 1 ] ) )
				{
					ent_rad = fabs( ent->maxs[ 1 ] - ent->mins[ 1 ] );
				}

				if( ent_rad < fabs( ent->maxs[ 2 ] - ent->mins[ 2 ] ) )
				{
					ent_rad = fabs( ent->maxs[ 2 ] - ent->mins[ 2 ] );
				}

				org = ent->centroid;
				dir = org - origin;

				dist = dir.length() - ent_rad;
				if( dist < 0.0f ) {
					dist = 0.0f;
				}

				points = damage - damage * ( dist / radius );

				knockback -= knockback * ( dist / radius );

				if( points < 0 )
					points = 0;

				if( knockback < 0 )
					knockback = 0;
			}

			// reduce the damage a little for self-damage
			if( ent == attacker )
			{
				points *= 0.9f;
			}

			if( points > 0 )
			{
				ent->Damage( inflictor,
					attacker,
					points,
					org,
					dir,
					vec_zero,
					knockback,
					DAMAGE_RADIUS,
					mod
					);

				if( !g_gametype->integer && weap )
				{
					if( ent->IsSubclassOfPlayer() ||
						ent->IsSubclassOfVehicle() ||
						ent->IsSubclassOfVehicleTank() ||
						ent->isSubclassOf( VehicleCollisionEntity ) )
					{
						weap->m_iNumHits++;
						weap->m_iNumTorsoShots++;

						if( attacker && attacker->IsSubclassOfPlayer() )
						{
							Player *player = ( Player * )attacker;

							if( weap->IsSubclassOfTurretGun() )
							{
								player->m_iNumHits++;
								player->m_iNumTorsoShots++;
							}
						}
					}
				}
			}
		}
	}

	if( mod == MOD_EXPLOSION )
	{
		// Create an earthquake
		new ViewJitter(
			origin,
			radius + 128.0f,
			0.05f,
			Vector( damage * 0.05f, damage * 0.05f, damage * 0.06f ),
			0,
			vec_zero,
			0
			);
	}
}


void FlashPlayers
   (
   Vector   org,
   float    r,
   float    g, 
   float    b,
   float    a,
   float    radius,
   float    time,
   int      type
   )

   {
   trace_t     trace;
   Vector      delta;
   float       length;
   Player      *player;
   gentity_t   *ed;
   int         i;
   Entity      *ent;
   float       newa = 1;

 	for( i = 0; i < game.maxclients; i++ )
      {
      ed = &g_entities[ i ];
		if ( !ed->inuse || !ed->entity )
			continue;

  		ent = ed->entity;
      if ( !ent->IsSubclassOfPlayer() )
         continue;

      player = ( Player * )ent;

      if ( !player->WithinDistance( org, radius ) )
         continue;

      trace = G_Trace( org, vec_zero, vec_zero, player->origin, player, MASK_OPAQUE, qfalse, "FlashPlayers" );      
      if ( trace.fraction != 1.0 )
         continue;

      delta = org - trace.endpos;
      length = delta.length();

      // If alpha is specified, then modify it by the amount of distance away from the flash the player is
      if ( a != -1 )
         newa = a * ( 1 - length / radius );

      level.m_fade_alpha      = newa;
      level.m_fade_color[0]   = r;
      level.m_fade_color[1]   = g;
      level.m_fade_color[2]   = b;
      level.m_fade_time       = time;
      level.m_fade_time_start = time;

      if ( type == 1 )
         level.m_fade_style = additive;
      else
         level.m_fade_style = alphablend;

//      gi.SendServerCommand( NULL, va( "fadein %0.2f %0.2f %0.2f %0.2f %i",time*1000,r*newa,g*newa,b*newa,type ) );
      }
   }

