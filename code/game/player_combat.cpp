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

// player_combat.cpp: Player combat system and combat utility functions
// 

#include "player.h"
#include "weaputils.h"

static Entity *FindClosestEntityInRadius
   (
   Vector origin,
   Vector forward,
   float  fov,
   float  maxdist
   )

   {
   float    dist,dot;
   float    fovdot = cos( fov * 0.5 * M_PI / 180.0 );
   Entity   *ent;
   Entity   *bestent=NULL;
   int      bestdist = 999999;
	qboolean valid_entity;

   // Find closest enemy in radius
   ent = findradius( NULL, origin, maxdist ); 

   while( ent )
      {      
		valid_entity = false;

		if( ent->flags & FL_AUTOAIM )
         {
         valid_entity = true;
         }

		if ( valid_entity )
         {
         // Check to see if the enemy is closest to us
      	Vector delta = ( ent->centroid ) - origin;

      	dist = delta.length();

         if ( dist < bestdist )
            {
            delta.normalize();

            // It's close, now check to see if it's in our FOV.
	         dot = DotProduct( forward, delta );
      
            if ( dot > fovdot )
               {
               trace_t trace;
               // Do a trace to see if we can get to it
               trace = G_Trace( origin,
                                 vec_zero,
                                 vec_zero,
                                 ent->centroid,
                                 NULL,
                                 MASK_OPAQUE,
                                 false, 
                                 "FindClosestEntityInRadius" );
               
               if ( ( trace.ent && trace.entityNum == ent->entnum ) || ( trace.fraction == 1 ) )
                  {
                  // dir = delta;
                  bestent  = ent;
                  bestdist = dist;
                  }
               }
            }
         }
      ent = findradius( ent, origin, maxdist );
      }
   return bestent;
   }

Entity *Player::FindEnemyInFOVFromTagWithOffset
   (
   float fov,
   float maxdist,
   str   tagname,
   Vector offset
   )

   {
   vec3_t mat[3];
   orientation_t tag_or;

   GetTagPositionAndOrientation( gi.Tag_NumForName( edict->tiki, tagname ), &tag_or );
   
   AnglesToAxis( torsoAngles, mat );

   //G_DebugLine( tag_or.origin + offset, tag_or.origin + offset + Vector( mat[0] ) * 100, 1,1,1,1 );

   return FindClosestEntityInRadius( tag_or.origin + offset, mat[0], fov, maxdist );
   }

//====================
//FindEnemyInFOV
//Returns entity if an enemy is in the player's FOV
//====================
Entity *Player::FindEnemyInFOV
   (
   float fov,
   float maxdist
   )

   {
   vec3_t mat[3];

   AnglesToAxis( headAngles, mat );
   return FindClosestEntityInRadius( this->centroid, mat[0], fov, maxdist );
   }

//====================
//AdjustAnglesForAttack 
//Adjust the player angles toward an enemy if they are attacking it
//====================
void Player::AdjustAnglesForAttack
   (
   void
   )

   {
   /*
   Vector dir;
   Vector newAngles;
   Vector moveToAngles;

   if ( FindEnemyInFOV( 180, 1000 ) ) 
      {
      G_DrawCoordSystem( origin, dir,dir,dir,100 );

      if ( buttons & ( BUTTON_ATTACKRIGHT|BUTTON_ATTACKLEFT ) )
         {
         // Adjust for a percentage of the total
         float deltayaw = AngleSubtract( dir.toYaw(), v_angle[YAW] );

         if ( fabs( deltayaw ) > 5 )
            {
            v_angle[YAW] += deltayaw * 0.1f;
     		   client->ps.delta_angles[YAW] += ANGLE2SHORT( deltayaw * 0.1f );
            }
         }
      }
      */
   }


//====================
//useWeapon
//====================
void Player::useWeapon
	(
	const char     *weaponname,
   weaponhand_t   hand
	)

	{
   Weapon *weapon;

   weapon = ( Weapon * )FindItem( weaponname );

   // Check to see if player has the weapon
   if ( !weapon )
      {
      warning( "Player::useWeapon", "Player does not have weapon %s", weaponname );
      return;
      }

   useWeapon( weapon, hand );
   }

void Player::useWeapon
   (
   Weapon         *weapon,
   weaponhand_t   hand
   )

{
	Weapon * activeWeapon;

	if( !weapon )
	{
		warning( "Player::useWeapon", "Null weapon used.\n" );
		return;
	}

	// Check to see if we are already in the process of using a new weapon.
	if( newActiveWeapon.weapon )
	{
		return;
	}

	if( weapon->isSubclassOf( Weapon ) )
	{

		// Check to see if weapon has ammo and if useNoAmmo is allowed
		if( !weapon->HasAmmo( FIRE_PRIMARY ) && !weapon->HasAmmo( FIRE_SECONDARY ) && !weapon->GetUseNoAmmo() )
		{
			Sound( "snd_noammo" );
			return;
		}
	}

	// Now get the active weapon in the specified hand
	activeWeapon = GetActiveWeapon( hand );

	// Check to see if this weapon is already being used in this hand and just put it away and return
	if( activeWeapon == weapon )
	{
		// Set the putaway flag to true.  The state machine will then play the correct animation to put away the active weapon
		activeWeapon->PutAway();
		return;
	}

	// If activeWeapon is set, and it's not == weapon then put away this weapon
	if( activeWeapon )
	{
		// Set the putaway flag to true.  The state machine will then play the correct animation to put away the active weapon
		activeWeapon->PutAway();
	}

	// Check to see if this weapon is being used in a different hand and put it away as well (if it's in a different hand)
	if( IsActiveWeapon( weapon ) )
	{
		weapon->PutAway();
	}

	// Set the newActiveWeapon as the weapon specified, the state machine will play the appropriate animation and
	// trigger when to attach it to the player model.
	newActiveWeapon.weapon = weapon;
	newActiveWeapon.hand = hand;
}

void Player::AcquireHeadTarget
   (
   void
   )

{

}
