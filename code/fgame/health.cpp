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

// health.cpp: Health powerup 
//

#include "g_local.h"
#include "item.h"
#include "inventoryitem.h"
#include "sentient.h"
#include "health.h"
#include "weaputils.h"
#include "player.h"

CLASS_DECLARATION( Item, Health, "health_020" )
{
	{ &EV_Item_Pickup,				&Health::PickupHealth },
	{ NULL, NULL }
};

Health::Health()
{
	if( DM_FLAG( DF_NO_HEALTH ) )
	{
		PostEvent( EV_Remove, EV_REMOVE );
		return;
	}

	setAmount( 20 );
}

void Health::PickupHealth
	(
	Event *ev
	)

{
	Sentient *sen;
	Entity *other;

	other = ev->GetEntity( 1 );
	if( !other || !other->IsSubclassOfSentient() )
	{
		return;
	}

	sen = ( Sentient * )other;

	if( sen->health >= sen->max_health )
		return;

	if( !ItemPickup( other, qfalse ) )
	{
		return;
	}

	sen->health += amount / 100.0 * sen->max_health;

	if( sen->health > sen->max_health )
	{
		sen->health = sen->max_health;
	}

	gi.SendServerCommand( sen->edict - g_entities, "print \"" HUD_MESSAGE_YELLOW "%s \"", gi.LV_ConvertString( va( "Recovered %d", amount ) ) );
}
