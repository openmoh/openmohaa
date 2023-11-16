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

// inventoryitem.cpp: Inventory items

#include "inventoryitem.h"
#include "weaputils.h"
#include "level.h"

Event EV_InventoryItem_Shoot
(
	"activateitem",
	EV_DEFAULT,
	"S",
	"mode",
	"Activates the item"
);

Event EV_InventoryItem_Activate_Papers
(
	"activatepapers",
	EV_DEFAULT,
	NULL,
	NULL,
	"The activation of the papers item"
);

CLASS_DECLARATION( Weapon, InventoryItem, NULL )
{
	{ &EV_InventoryItem_Shoot,					&InventoryItem::ActivateItem },
	{ &EV_InventoryItem_Activate_Papers,		&InventoryItem::ActivatePapers },
	{ NULL, NULL }
};

InventoryItem::InventoryItem()
{
	entflags |= ECF_INVENTORYITEM;

	if ( LoadingSavegame )
	{
		return;
	}

	weapon_class = WEAPON_CLASS_ITEM;
}

InventoryItem::~InventoryItem()
{
	entflags &= ~ECF_INVENTORYITEM;
}

void InventoryItem::ActivateItem
	(
	Event *ev
	)
{
	if( firetype[ FIRE_PRIMARY ] == FT_CLICKITEM )
	{
		Vector pos, forward, right, up, vBarrel;

		GetMuzzlePosition( pos, vBarrel, forward, right, up );
		ClickItemAttack(
			pos,
			forward,
			bulletrange[ FIRE_PRIMARY ],
			owner );
	}

	m_fLastFireTime = level.time;
}

void InventoryItem::ActivatePapers
	(
	Event *ev
	)
{
	GetOwner()->m_ShowPapersTime = level.inttime;
}
