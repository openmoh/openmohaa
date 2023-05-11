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

#ifndef __UIVERTSCROLL_H__
#define __UIVERTSCROLL_H__

class unnamed_struct {
public:
	int itemOffset;
	int orgItem;
};

class UIVertScroll : public UIWidget {
protected:
	int m_numitems;
	int m_pageheight;
	int m_topitem;
	UIFont m_marlett;
	whatspressed m_pressed;
	UIRect2D thumbRect;
	unnamed_struct m_dragThumbState;
	bool m_frameinitted;
	UColor m_thumbcolor;
	UColor m_solidbordercolor;

public:
	CLASS_PROTOTYPE( UIVertScroll );

protected:
	int		getItemFromHeight( float height );
	bool	isEnoughItems( void );

public:
	UIVertScroll();

	void			Draw( void ) override;
	void			DrawArrow( float top, const char *text, bool pressed );
	void			DrawThumb( void );
	void			MouseDown( Event *ev );
	void			MouseUp( Event *ev );
	void			MouseDragged( Event *ev );
	void			MouseEnter( Event *ev );
	void			MouseLeave( Event *ev );
	void			Scroll( Event *ev );
	bool			AttemptScrollTo( int to );
	void			setNumItems( int i );
	void			setPageHeight( int i );
	void			setTopItem( int i );
	int				getTopItem( void );
	int				getPageHeight( void );
	int				getNumItems( void );
	void			setThumbColor( const UColor& thumb );
	void			setSolidBorderColor( const UColor& col );
	void			InitFrameAlignRight( UIWidget *parent, float fWidthPadding, float fHeightPadding );
};

#endif

