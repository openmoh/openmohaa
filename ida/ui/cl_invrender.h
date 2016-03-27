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

typedef enum { selectsound, rejectsound, changesound } invsound_t;

typedef struct FakkInventory_s FakkInventory;

typedef struct FakkItemList_s {
	UIWidget baseClass;
	inventory_type_t *type;
	uipopup_describe m_describe[ 10 ];
	inventory_item_t *m_hoveritem;
	inventory_item_t *m_lastmenuitem;
	FakkInventory *m_parent;
	float m_hovertop;
} FakkItemList;

typedef struct FakkInventory_s {
	UIWidget baseClass;
	inventory_t *m_inv;
	SafePtr2_t m_currentlist;
	inventory_type_t *m_currenttype;
	int m_currentitemnum;
} FakkInventory;
