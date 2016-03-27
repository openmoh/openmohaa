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

CLASS_DECLARATION( UIWidget, UIDMBox, NULL )
{
	{ NULL, NULL }
};

void UIDMBox::VerifyBoxOut( void )
{
	// FIXME: stub
}

void UIDMBox::ChangeBoxState( boxstate_t state )
{
	// FIXME: stub
}

void UIDMBox::HandleBoxMoving( void )
{
	// FIXME: stub
}

void UIDMBox::PostMoveinEvent( void )
{
	// FIXME: stub
}

void UIDMBox::PostDecayEvent( void )
{
	// FIXME: stub
}

void UIDMBox::setShowState( void )
{
	// FIXME: stub
}

void UIDMBox::RemoveTopItem( void )
{
	// FIXME: stub
}

str UIDMBox::CalculateBreaks( UIFont *font, str text, float max_width )
{
	// FIXME: stub
	return "";
}

float UIDMBox::PrintWrap( UIFont *font, float x, float y, str text )
{
	// FIXME: stub
	return 0.0f;
}

float UIDMBox::DrawItem( item_t *in, float x, float y, float alpha )
{
	// FIXME: stub
	return 0.0f;
}

void UIDMBox::Print( const char *text )
{
	// FIXME: stub
}

void UIDMBox::OnSizeChanged( Event *ev )
{
	// FIXME: stub
}

void UIDMBox::Create( const UIRect2D& rect, const UColor& fore, const UColor& back, float alpha )
{
	// FIXME: stub
}

void UIDMBox::MoveInEvent( Event *ev )
{
	// FIXME: stub
}

void UIDMBox::DecayEvent( Event *ev )
{
	// FIXME: stub
}

void UIDMBox::Draw( void )
{
	// FIXME: stub
}

void UIDMBox::setRealShow( bool b )
{
	// FIXME: stub
}

void UIDMBox::Clear( void )
{
	// FIXME: stub
}
