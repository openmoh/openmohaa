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

#ifndef __UILISTCTRL_H__
#define __UILISTCTRL_H__

typedef enum { TYPE_STRING, TYPE_OWNERDRAW } griditemtype_t;

class UIListCtrlItem {
public:
	virtual griditemtype_t		getListItemType( int which ) const;
	virtual str					getListItemString( int which ) const;
	virtual int					getListItemValue( int which ) const;
	virtual void				DrawListItem( int iColumn, const UIRect2D &drawRect, bool bSelected, UIFont *pFont );
	virtual qboolean			IsHeaderEntry( void ) const;
};

typedef struct m_clickState_s {
	int time;
	int selected;
	UIPoint2D point;
} m_clickState_t;

class UIListCtrl : public UIListBase {
public:
	struct columndef_t {
		str title;
		int name;
		int width;
		bool numeric;
		bool reverse_sort;
	};

protected:
	static bool s_qsortreverse;
	static int s_qsortcolumn;
	static class UIListCtrl *s_qsortobject;
	int m_iLastSortColumn;
	class UIFont *m_headerfont;
	Container<UIListCtrlItem *> m_itemlist;
	Container<UIListCtrl::columndef_t> m_columnlist;

	qboolean m_bDrawHeader;
	struct {
		int column;
		int min;
	} m_sizestate;
	m_clickState_s m_clickState;

	int( *m_comparefunction ) (/* unknown */ );

public:
	CLASS_PROTOTYPE( UIListCtrl );

protected:
	static int				StringCompareFunction( const UIListCtrlItem *i1, const UIListCtrlItem *i2, int columnname );
	static int				StringNumberCompareFunction( const UIListCtrlItem *i1, const UIListCtrlItem *i2, int columnname );
	static int				QsortCompare( const void *e1, const void *e2 );
	void 					Draw( void );
	int						getHeaderHeight( void );
	void					MousePressed( Event *ev );
	void					MouseDragged( Event *ev );
	void					MouseReleased( Event *ev );
	void					MouseEntered( Event *ev );
	void					OnSizeChanged( Event *ev );
	void					DrawColumns( void );
	void					DrawContent( void );
public:
	UIListCtrl();

	void				FrameInitialized( void );
	void				SetDrawHeader( qboolean bDrawHeader );
	void				AddItem( UIListCtrlItem *item );
	void				InsertItem( UIListCtrlItem *item, int where );
	int					FindItem( UIListCtrlItem *item );
	UIListCtrlItem		*GetItem( int item );
	void				AddColumn( str title, int name, int width, bool numeric, bool reverse_sort );
	void				RemoveAllColumns( void );
	int					getNumItems( void );
	void				DeleteAllItems( void );
	void				DeleteItem( int which );
	virtual void		SortByColumn( int column );
	void				SortByLastSortColumn( void );
	void				setCompareFunction( int( *func ) ( const UIListCtrlItem *i1, const UIListCtrlItem *i2, int columnname ) );
	void				setHeaderFont( const char *name );
};

#endif
