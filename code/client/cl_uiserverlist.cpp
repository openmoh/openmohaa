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

CLASS_DECLARATION( UIListCtrl, UIFAKKServerList, NULL )
{
	{ NULL, NULL }
};

UIFAKKServerList::UIFAKKServerList()
{
	setBackgroundColor(UWhite, true);
    Connect(this, EV_UIListBase_ItemDoubleClicked, EV_UIListBase_ItemDoubleClicked);
    Connect(this, EV_UIListBase_ItemSelected, EV_UIListBase_ItemSelected);

	AllowActivate(true);
	setHeaderFont("facfont-20");
	m_serverList = NULL;
	m_bHasList = false;
	m_bGettingList = false;
	m_bUpdatingList = false;
	m_bLANListing = false;
	m_iLastSortColumn = 2;
}

void UIFAKKServerList::SelectServer( Event *ev )
{
	// FIXME: stub
}

void UIFAKKServerList::ConnectServer( Event *ev )
{
	// FIXME: stub
}

qboolean UIFAKKServerList::KeyEvent( int key, unsigned int time )
{
	// FIXME: stub
	return qfalse;
}

void UIFAKKServerList::UpdateUIElement( void )
{
	// FIXME: stub
}

void UIFAKKServerList::RefreshServerList( Event *ev )
{
	// FIXME: stub
}

void UIFAKKServerList::RefreshLANServerList( Event *ev )
{
	// FIXME: stub
}

void UIFAKKServerList::CancelRefresh( Event *ev )
{
	// FIXME: stub
}

void UIFAKKServerList::NewServerList( void )
{
	// FIXME: stub
}

void UIFAKKServerList::MakeLANListing( Event *ev )
{
	// FIXME: stub
}

void UIFAKKServerList::UpdateServer( Event *ev )
{
	// FIXME: stub
}

int UIFAKKServerList::ServerCompareFunction( const UIListCtrlItem *i1, const UIListCtrlItem *i2, int columnname )
{
	// FIXME: stub
	return 0;
}

void UIFAKKServerList::Draw( void )
{
	// FIXME: stub
}

void UIFAKKServerList::SortByColumn( int column )
{
	// FIXME: stub
}
