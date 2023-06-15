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

// armor.cpp: Standard armor that prevents a percentage of damage per hit.
// 

#include "g_local.h"
#include "armor.h"

CLASS_DECLARATION( Item, Armor, NULL )
	{
		{ NULL, NULL }
	};

Armor::Armor()
	{
   if ( LoadingSavegame )
      {
      // no need to proceed if loading a game
      return;
      }

	if ( DM_FLAG( DF_NO_ARMOR ) )
		{
		PostEvent( EV_Remove, EV_REMOVE );
		return;
		}

   setAmount( 0 );
	}

void Armor::Setup
	(
	const char *model,
	int amount
	)

	{
	assert( model );
	setModel( model );
   setAmount( amount );
   }

void Armor::Add
	(
	int num
	)

	{
   // Armor never adds, it only replaces
	amount = num;
   if ( amount >= MaxAmount() )
      {
      amount = MaxAmount();
      }
	}

qboolean Armor::Pickupable
   (
   Entity *other
   )

   {
	if ( !other->isSubclassOf( Sentient ) )
		{
		return false;
		}
   else
      {
      Sentient * sent;
      Item * item;

      sent = ( Sentient * )other;
      item = sent->FindItem( getName() );

      // If our armor is > than our current armor or armor has no value, then leave it alone.
      if ( item && ( ( item->getAmount() >= this->getAmount() ) || !this->getAmount() ) )
         {
         return false;
         }
      }
   return true;
   }
