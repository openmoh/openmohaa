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

#ifndef __UIHORIZSCROLL_H__
#define __UIHORIZSCROLL_H__

typedef enum { VS_NONE, VS_UP_ARROW, VS_DOWN_ARROW, VS_THUMB, VS_PAGE_DOWN, VS_PAGE_UP } whatspressed;

class UIHorizScroll : public UIWidget {
protected:
	int m_numitems;
	int m_pagewidth;
	int m_topitem;
	UIFont m_marlett;
	whatspressed m_pressed;
	UIRect2D thumbRect;

	struct {
		int itemOffset;
		int orgItem;
	} m_dragThumbState;

	bool m_frameinitted;
	UColor m_thumbcolor;
	UColor m_solidbordercolor;
public:
	CLASS_PROTOTYPE( UIHorizScroll );

protected:
	int			getItemFromWidth( float height );
	bool		isEnoughItems( void );

public:
	UIHorizScroll();

	void		Draw( void );
	void		DrawArrow( float top, const char *text, bool pressed );
	void		DrawThumb();
	void		MouseDown( Event *ev );
	void		MouseUp( Event *ev );
	void		MouseDragged( Event *ev );
	void		MouseEnter( Event *ev );
	void		MouseLeave( Event *ev );
	void		Scroll( Event *ev );
	bool		AttemptScrollTo( int to );
	void		setNumItems( int i );
	void		setPageWidth( int i );
	void		setTopItem( int i );
	int			getTopItem( void );
	int			getPageWidth( void );
	int			getNumItems( void );
	void		setThumbColor( const UColor& thumb );
	void		setSolidBorderColor( const UColor& col );
	void		InitFrameAlignRight( UIWidget *parent );
};

#endif

