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

extern Event EV_Layout_AddListItem;
extern Event EV_UIListBase_ItemDoubleClicked;
extern Event EV_UIListBase_ItemSelected;

class UIListBase : public UIWidget {
protected:
	int m_currentItem;
	class UIVertScroll *m_vertscroll;
	qboolean m_bUseVertScroll;
public:
	CLASS_PROTOTYPE( UIListBase );

protected:
	virtual void		TrySelectItem( int which );
	qboolean			KeyEvent( int key, unsigned int time ) override;

public:
	UIListBase();

	void			FrameInitialized( void ) override;
	int				getCurrentItem( void );
	virtual int		getNumItems( void );
	virtual void	DeleteAllItems( void );
	virtual void	DeleteItem( int which );
	UIVertScroll	*GetScrollBar( void );
	void			SetUseScrollBar( qboolean bUse );
};
typedef class UIListBase UIListBase;
class ListItem : public Class {
public:
	str string;
	str command;
	int index;

public:
	ListItem();
	ListItem( str string, int index, str command );
};

class UIListBox : public UIListBase {
protected:
	Container<ListItem *> m_itemlist;
	struct {
		int time;
		int selected;
		UIPoint2D point;
	} m_clickState;

public:
	CLASS_PROTOTYPE( UIListBox );

protected:
	void			Draw( void ) override;
	void			MousePressed( Event *ev );
	void			MouseReleased( Event *ev );
	void			DeleteAllItems( Event *ev );
	void			SetListFont( Event *ev );
	void			TrySelectItem( int which ) override;

public:
	UIListBox();

	void			AddItem( const char *item, const char *command );
	void			AddItem( int index, const char *command );
	void			FrameInitialized( void ) override;
	void			LayoutAddListItem( Event *ev );
	void			LayoutAddConfigstringListItem( Event *ev );
	str				getItemText( int which );
	int				getNumItems( void ) override;
	void			DeleteAllItems( void ) override;
	void			DeleteItem( int which ) override;
};
