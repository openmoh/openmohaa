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

#pragma once

class UIFAKKServerList : public UIListCtrl {
protected:
	// need a new struct instead of gamespy
	void *m_serverList;
	bool m_bHasList;
	bool m_bLANListing;

public:
	CLASS_PROTOTYPE( UIFAKKServerList );

	bool m_bGettingList;
	bool m_bUpdatingList;

protected:
	void			SelectServer( Event *ev );
	void			ConnectServer( Event *ev );
	qboolean		KeyEvent( int key, unsigned int time ) override;
	void			UpdateUIElement( void ) override;
	void			RefreshServerList( Event *ev );
	void			RefreshLANServerList( Event *ev );
	void			CancelRefresh( Event *ev );
	void			NewServerList( void );
	void			MakeLANListing( Event *ev );
	void			UpdateServer( Event *ev );
	static int		ServerCompareFunction( const UIListCtrlItem *i1, const UIListCtrlItem *i2, int columnname );
public:
	UIFAKKServerList();

	void		Draw( void ) override;
	void		SortByColumn( int column ) override;
};
