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

Vector Player::GunTarget(bool bNoCollision)
{
    Vector  vForward;
    Vector  vOut;
    Vector  vDest;
    trace_t trace;
    solid_t prev_solid = SOLID_BBOX;

    if (bNoCollision) {
        AngleVectors(m_vViewAng, vForward, NULL, NULL);
        vOut = m_vViewPos + vForward * 1024.0f;

        return vOut;
    } else if (m_pVehicle) {
        AngleVectors(m_vViewAng, vForward, NULL, NULL);
        vDest = m_vViewPos + vForward * 4096.0f;

        prev_solid = m_pVehicle->edict->solid;

        m_pVehicle->setSolidType(SOLID_NOT);

        if (m_pVehicle->IsSubclassOfVehicle()) {
            m_pVehicle->SetSlotsNonSolid();
        }

        trace = G_Trace(m_vViewPos, vec_zero, vec_zero, vDest, this, MASK_OPAQUE, qfalse, "Player::GunTarget");

        vOut = trace.endpos;
    } else {
        AngleVectors(m_vViewAng, vForward, NULL, NULL);
        vDest = m_vViewPos + vForward * 1024.0f;

        trace = G_Trace(m_vViewPos, vec_zero, vec_zero, vDest, this, MASK_GUNTARGET, qfalse, "Player::GunTarget");

        vOut = trace.endpos;
        if (m_pTurret) {
            if ((Vector(trace.endpos) - m_vViewPos).lengthSquared() < 16384) {
                vOut = vDest;
            }
        }
    }

    if (m_pVehicle) {
        m_pVehicle->setSolidType(prev_solid);

        if (m_pVehicle->IsSubclassOfVehicle()) {
            m_pVehicle->SetSlotsSolid();
        }
    }

    return vOut;
}

void Player::PlayerReload(Event *ev)
{
    Weapon *weapon;

    if (deadflag) {
        return;
    }

    weapon = GetActiveWeapon(WEAPON_MAIN);

    if (!weapon) {
        return;
    }

    if (weapon->CheckReload(FIRE_PRIMARY)) {
        weapon->SetShouldReload(true);
    }
}

void Player::EventCorrectWeaponAttachments(Event *ev)
{
    int      iChild;
    int      iNumChildren;
    int      iTagRight;
    int      iTagLeft;
    qboolean iUseAngles;
    Vector   vOffset;
    Entity  *pChild;

    iTagRight    = gi.Tag_NumForName(edict->tiki, "tag_weapon_right");
    iTagLeft     = gi.Tag_NumForName(edict->tiki, "tag_weapon_left");
    iNumChildren = numchildren;

    for (int i = 0; i < iNumChildren; i++) {
        iChild = children[i];

        if (iChild == ENTITYNUM_NONE) {
            continue;
        }

        pChild = G_GetEntity(iChild);
        if (!pChild) {
            continue;
        }

        if (pChild->edict->s.tag_num == iTagLeft || pChild->edict->s.tag_num == iTagRight) {
            if (pChild->IsSubclassOfWeapon()) {
                if (pChild->edict->s.tag_num == iTagLeft) {
                    iUseAngles = edict->s.attach_use_angles;
                    vOffset    = edict->s.attach_offset;

                    // reattach to the right tag
                    detach();
                    attach(entnum, iTagRight, iUseAngles, vOffset);
                }
            } else {
                // Remove entities like ammoclip
                pChild->PostEvent(EV_Remove, 0);
            }
        }
    }
}
