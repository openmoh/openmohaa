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

// gibs.cpp: Gibs - nuff said

#include "gibs.h"
#include "decals.h"
#include "level.h"
#include "g_phys.h"

Event EV_ThrowGib
	(
	"throwgib",
	EV_DEFAULT,
   "eif",
   "ent velocity scale",
   "Throw a gib.",
   EV_NORMAL
	);

CLASS_DECLARATION( Mover, Gib, "gib" )
	{
      { &EV_ThrowGib,			&Gib::Throw },
      { &EV_Touch,				&Gib::Splat },
      { &EV_Stop,				&Gib::Stop },
		{ &EV_Damage,			&Gib::Damage },
      { NULL, NULL }
	};

Gib::Gib
	(
	str name,
	qboolean blood_trail,
	str bloodtrailname,
	str bloodspurtname,
	str bloodsplatname,
	float bloodsplatsize,
	float pitch
	)

   {
   setSize( Vector( "0 0 0" ), Vector( "0 0 0" ) );

	if ( name.length() )
		setModel( name.c_str() );

	setMoveType( MOVETYPE_GIB );
	setSolidType( SOLID_BBOX );
	takedamage = DAMAGE_YES;
   sprayed     = false;
   fadesplat   = true;
   scale       = 2.0f;

	next_bleed_time = 0;

	final_pitch	= pitch;

	if ( blood_trail )
		{
		// Make a blood emitter and bind it to the head
		blood = new Mover;
   
		if ( bloodtrailname.length() )
			blood->setModel( bloodtrailname.c_str() );

		blood->setMoveType( MOVETYPE_BOUNCE );
		blood->setSolidType( SOLID_NOT );
		blood->bind( this );

		// Save the blood spurt name

		if ( bloodspurtname.length() )
			blood_spurt_name = bloodspurtname;

		// Save the blood splat name

		if ( bloodsplatname.length() )
			blood_splat_name = bloodsplatname;

		blood_splat_size = bloodsplatsize;
		}
	else
		{
		blood = NULL;
		}

   Sound( "snd_decap", CHAN_BODY, 1, 300 );
   }

Gib::~Gib()
	{
	if ( blood )
      blood->PostEvent( EV_Remove, 0 );
   blood = NULL;
	}

Gib::Gib()
   {
   if ( LoadingSavegame )
      {
      return;
      }

   setSize( Vector( "0 0 0" ), Vector( "0 0 0" ) );
   setModel("gib1.def");
	setMoveType( MOVETYPE_GIB );
	setSolidType( SOLID_BBOX );
   sprayed           = 0;
   fadesplat         = true;
   scale             = 2.0f;
   }

void Gib::Stop 
   (
   Event *ev 
   )

   {
   //setSolidType( SOLID_NOT );
   if ( blood )
      blood->PostEvent( EV_Remove, 0 );
   blood = NULL;
   }

void Gib::Splat
	(
	Event *ev
	)

   {
   if ( g_gametype->integer )
      return;

   if ( sprayed > 3 )
      {
      //setSolidType(SOLID_NOT);
      return;
      }

   sprayed++;
   scale -= 0.2f;

	// Stop spinning / force to final pitch

	avelocity = vec_zero;

	if ( final_pitch != NO_FINAL_PITCH )
		{
		angles[PITCH] = final_pitch;
		setAngles( angles );
		}

	SprayBlood( origin );
   Sound( "snd_gibhit" );
   }

void Gib::Damage
	(
	Event *ev
	)

   {
   Vector direction;
	Entity *blood;
	Vector dir;

	if ( next_bleed_time > level.time )
		return;
	
   direction = ev->GetVector ( 5 );

	// Spawn a blood spurt

	if ( blood_spurt_name.length() > 0 )
		{
		blood = new Animate;
		blood->setModel( blood_spurt_name.c_str() );

		dir[0] = -direction[0];
		dir[1] = -direction[1];
		dir[2] = -direction[2];

		blood->angles = dir.toAngles();
		blood->setAngles( blood->angles );

		blood->setOrigin( centroid );
		blood->origin.copyTo( blood->edict->s.origin2 );
		blood->setSolidType( SOLID_NOT );

		blood->PostEvent( EV_Remove, 1 );

		next_bleed_time = level.time + 0.5;
		}
   }

void Gib::SprayBlood
	(
	Vector start
	)

   {
   Vector      norm;
	trace_t		trace;
	Vector		trace_end;

	trace_end = velocity;
	trace_end.normalize();
	trace_end *= 1000;
	trace_end += start;

   trace = G_Trace( start, vec_zero, vec_zero, trace_end, this, MASK_SOLID, false, "Gib::SprayBlood" );

	if ( HitSky( &level.impact_trace ) || ( !level.impact_trace.ent ) || ( level.impact_trace.ent->solid != SOLID_BSP ) )
		{
		return;
		}

	// Do a bloodsplat
   if ( blood_splat_name.length() )
		{
		Decal *decal = new Decal;
		decal->setShader( blood_splat_name );
		decal->setOrigin( Vector( trace.endpos ) + ( Vector( level.impact_trace.plane.normal ) * 0.2f ) );
		decal->setDirection( level.impact_trace.plane.normal );
		decal->setOrientation( "random" );
		decal->setRadius( blood_splat_size + G_Random( blood_splat_size ) );
		}
   }

void Gib::ClipGibVelocity
	(
	void
	)

	{
	if (velocity[0] < -400)
		velocity[0] = -400;
	else if (velocity[0] > 400)
		velocity[0] = 400;
	if (velocity[1] < -400)
		velocity[1] = -400;
	else if (velocity[1] > 400)
		velocity[1] = 400;
	if (velocity[2] < 200)
		velocity[2] = 200;	// always some upwards
	else if (velocity[2] > 600)
		velocity[2] = 600;
}

void Gib::SetVelocity
	(
	float damage
	)

   {
   velocity[0] = 100.0 * crandom();
	velocity[1] = 100.0 * crandom();
	velocity[2] = 200.0 + 100.0 * random();

   avelocity = Vector( G_Random( 600 ), G_Random( 600 ), G_Random( 600 ) );

   if ( ( damage < -150 )  && ( G_Random() > 0.95f ) )
      velocity *= 2.0f;
   else if ( damage < -100 )
      velocity *= 1.5f;

   ClipGibVelocity();
   }

void Gib::Throw
	(
	Event *ev
	)

   {
   Entity *ent;

   ent = ev->GetEntity(1);
   setOrigin(ent->centroid);
   origin.copyTo(edict->s.origin2);
   SetVelocity(ev->GetInteger(2));
   edict->s.scale = ev->GetFloat(3);
   PostEvent(EV_FadeOut, 10 + G_Random(5));
   }


void CreateGibs
   (
   Entity * ent,
   float damage,
   float scale,
   int num,
   const char * modelname
   )
   
   {

   }
