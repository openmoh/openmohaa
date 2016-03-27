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

#include "cl_ui.h"

CLASS_DECLARATION( UIWidget, FakkItemList, NULL )
{
	{ NULL, NULL }
};

FakkItemList::FakkItemList()
{
	// FIXME: stub
}

FakkItemList::~FakkItemList()
{
	// FIXME: stub
}

void FakkItemList::VerifyItemUp( inventory_item_t *item, qboolean warpmouse )
{
	// FIXME: stub
}

void FakkItemList::setType( inventory_type_t *t )
{
	// FIXME: stub
}

void FakkItemList::Create( float x, float y, FakkInventory *parent )
{
	// FIXME: stub
}

bool FakkItemList::HasAnyItems( void )
{
	// FIXME: stub
	return false;
}

void FakkItemList::Draw( void )
{
	// FIXME: stub
}

void FakkItemList::OnLeftMouseDown( Event *ev )
{
	// FIXME: stub
}

void FakkItemList::OnRightMouseDown( Event *ev )
{
	// FIXME: stub
}

void FakkItemList::OnLeftMouseUp( Event *ev )
{
	// FIXME: stub
}

void FakkItemList::OnRightMouseUp( Event *ev )
{
	// FIXME: stub
}

void FakkItemList::OnMouseMove( Event *ev )
{
	// FIXME: stub
}

void FakkItemList::OnMouseEnter( Event *ev )
{
	// FIXME: stub
}

void FakkItemList::OnMouseLeave( Event *ev )
{
	// FIXME: stub
}

void FakkItemList::OnMenuKilled( Event *ev )
{
	// FIXME: stub
}

void FakkItemList::EquipItem( Event *ev )
{
	// FIXME: stub
}

CLASS_DECLARATION( UIWidget, FakkInventory, NULL )
{
	{ NULL, NULL }
};


FakkInventory::FakkInventory()
{
	// FIXME: stub
}

FakkInventory::~FakkInventory()
{
	// FIXME: stub
}

void FakkInventory::VerifyItemUp( inventory_item_t *item, qboolean warpmouse )
{
	// FIXME: stub
}

void FakkInventory::setInventory( inventory_t *i )
{
	// FIXME: stub
}

void FakkInventory::Draw( void )
{
	// FIXME: stub
}

void FakkInventory::OnMouseMove( Event *ev )
{
	// FIXME: stub
}

void FakkInventory::OnMouseLeave( Event *ev )
{
	// FIXME: stub
}

void FakkInventory::OnMouseEnter( Event *ev )
{
	// FIXME: stub
}

void FakkInventory::OnMouseDown( Event *ev )
{
	// FIXME: stub
}

void FakkInventory::WarpTo( const char *name )
{
	// FIXME: stub
}

void FakkInventory::WarpTo( int slotnum )
{
	// FIXME: stub
}

void FakkInventory::NextItem( void )
{
	// FIXME: stub
}

void FakkInventory::PrevItem( void )
{
	// FIXME: stub
}

qboolean FakkInventory::isDying( void )
{
	// FIXME: stub
	return qfalse;
}

qboolean FakkInventory::KeyEvent( int key, unsigned int time )
{
	// FIXME: stub
	return qfalse;
}

void FakkInventory::PlaySound( invsound_t type )
{
	// FIXME: stub
}

void FakkInventory::Timeout( Event *ev )
{
	// FIXME: stub
}

void FakkInventory::ChangeItem( int sign )
{
	// FIXME: stub
}

int FakkInventory::FindFirstItem( int itemindex, int sign )
{
	// FIXME: stub
	return 0;
}

void CL_Draw3DModel( float x, float y, float w, float h, qhandle_t model, vec3_t origin, vec3_t rotateoffset, vec3_t offset, vec3_t angle, vec3_t color, str anim )
{
	// FIXME: stub
}

qboolean UI_CloseInventory( void )
{
	// FIXME/ stub
	return qfalse;
}
