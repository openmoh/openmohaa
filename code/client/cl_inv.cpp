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

CLASS_DECLARATION( Listener, invlistener, NULL )
{
	{ NULL, NULL }
};

invlistener::invlistener( inventory_t *i )
{
	// FIXME: stub
}

invlistener::invlistener()
{
	// FIXME: stub
}

void invlistener::verify_curitem( void )
{
	// FIXME: stub
}

void invlistener::verify_curtype( void )
{
	// FIXME: stub
}

void invlistener::verify_one_arg( Event *ev )
{
	// FIXME: stub
}

bool invlistener::Load( Script& script )
{
	// FIXME: stub
	return false;
}

void invlistener::InvSelectSound( Event *ev )
{
	// FIXME: stub
}

void invlistener::InvRejectSound( Event *ev )
{
	// FIXME: stub
}

void invlistener::InvChangeSound( Event *ev )
{
	// FIXME: stub
}

void invlistener::InvWidth( Event *ev )
{
	// FIXME: stub
}

void invlistener::InvHeight( Event *ev )
{
	// FIXME: stub
}

void invlistener::InvHorizOffset( Event *ev )
{
	// FIXME: stub
}

void invlistener::InvVertOffset( Event *ev )
{
	// FIXME: stub
}

void invlistener::InvAlign( Event *ev )
{
	// FIXME: stub
}

void invlistener::InvCascade( Event *ev )
{
	// FIXME: stub
}

void invlistener::Typedef( Event *ev )
{
	// FIXME: stub
}

void invlistener::OpenBrace( Event *ev )
{
	// FIXME: stub
}

void invlistener::CloseBrace( Event *ev )
{
	// FIXME: stub
}

void invlistener::ButtonShader( Event *ev )
{
	// FIXME: stub
}

void invlistener::HoverShader( Event *ev )
{
	// FIXME: stub
}

void invlistener::SelShader( Event *ev )
{
	// FIXME: stub
}

void invlistener::Background( Event *ev )
{
	// FIXME: stub
}

void invlistener::BackgroundTile( Event *ev )
{
	// FIXME: stub
}

void invlistener::Width( Event *ev )
{
	// FIXME: stub
}

void invlistener::Height( Event *ev )
{
	// FIXME: stub
}

void invlistener::BarWidth( Event *ev )
{
	// FIXME: stub
}

void invlistener::BarHeight( Event *ev )
{
	// FIXME: stub
}

void invlistener::BarOffsetX( Event *ev )
{
	// FIXME: stub
}

void invlistener::BarOffsetY( Event *ev )
{
	// FIXME: stub
}

void invlistener::Item( Event *ev )
{
	// FIXME: stub
}

void invlistener::Ammo( Event *ev )
{
	// FIXME: stub
}

void invlistener::Equip( Event *ev )
{
	// FIXME: stub
}

void invlistener::CheckAmmo( Event *ev )
{
	// FIXME: stub
}

void invlistener::Command( Event *ev )
{
	// FIXME: stub
}

void invlistener::BGShader( Event *ev )
{
	// FIXME: stub
}

void invlistener::BarShader( Event *ev )
{
	// FIXME: stub
}

void invlistener::SelItemShader( Event *ev )
{
	// FIXME: stub
}

void invlistener::SelItemShaderOnTop( Event *ev )
{
	// FIXME: stub
}

void invlistener::RotateOffset( Event *ev )
{
	// FIXME: stub
}

void invlistener::Offset( Event *ev )
{
	// FIXME: stub
}

void invlistener::Model( Event *ev )
{
	// FIXME: stub
}

void invlistener::Anim( Event *ev )
{
	// FIXME: stub
}

void invlistener::Scale( Event *ev )
{
	// FIXME: stub
}

void invlistener::Angles( Event *ev )
{
	// FIXME: stub
}

void invlistener::AngleDeltas( Event *ev )
{
	// FIXME: stub
}

void invlistener::Move( Event *ev )
{
	// FIXME: stub
}

void invlistener::ModelWindow( Event *ev )
{
	// FIXME: stub
}

void invlistener::HudRotateOffset( Event *ev )
{
	// FIXME: stub
}

void invlistener::HudOffset( Event *ev )
{
	// FIXME: stub
}

void invlistener::HudModel( Event *ev )
{
	// FIXME: stub
}

void invlistener::HudAnim( Event *ev )
{
	// FIXME: stub
}

void invlistener::HudScale( Event *ev )
{
	// FIXME: stub
}

void invlistener::HudAngles( Event *ev )
{
	// FIXME: stub
}

void invlistener::HudAngleDeltas( Event *ev )
{
	// FIXME: stub
}

void invlistener::HudMove( Event *ev )
{
	// FIXME: stub
}

void invlistener::HudCompassAngles( Event *ev )
{
	// FIXME: stub
}

void invlistener::HudCompassNeedleAngles( Event *ev )
{
	// FIXME: stub
}

bool CL_LoadInventory( const char *filename, inventory_t *inv )
{
	// FIXME: stub
	return false;
}

inventory_item_t *CL_GetInvItemByName( inventory_t *inv, const char *name )
{
	// FIXME: stub
	return NULL;
}

qboolean CL_HasInventoryItem( const char *name )
{
	// FIXME: stub
	return qfalse;
}

void CL_AmmoCount( const char *name, int *ammo_count, int *max_ammo_count )
{
	// FIXME: stub
}
