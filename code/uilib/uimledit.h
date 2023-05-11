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

#ifndef __UIMLEDIT_H__
#define __UIMLEDIT_H__

typedef struct selectionpoint_s {
	int line;
	int column;
} selectionpoint_t;

typedef struct selection_s {
	selectionpoint_s begin;
	selectionpoint_s end;
} selection_t;

typedef struct dragState_s {
	UIPoint2D lastPos;
} dragState_t;


class UIMultiLineEdit : public UIWidget {
protected:
	selection_s m_selection;
	dragState_s m_dragState;
	UList<str> m_lines;
	UIVertScroll *m_vertscroll;
	mouseState_t m_mouseState;
	bool m_shiftForcedDown;
	bool m_edit;
	bool m_changed;

public:
	CLASS_PROTOTYPE( UIMultiLineEdit );

protected:
	void	FrameInitialized( void );
	void	PointToSelectionPoint( UIPoint2D& p, selectionpoint_t& sel );
	str&	LineFromLineNumber( int num, bool resetpos );
	void	EnsureSelectionPointVisible( selectionpoint_t& point );
	void	BoundSelectionPoint( selectionpoint_t& point );
	void	SortSelection( selectionpoint_t **topsel, selectionpoint_t **botsel );
	void	UpdateCvarEvent( Event *ev );
	void	SetEdit( Event *ev );
	bool	IsSelectionEmpty( void );

public:
	UIMultiLineEdit();

	void		Draw( void ) override;
	qboolean	KeyEvent( int key, unsigned int time );
	void		CharEvent( int ch );
	UIPoint2D	getEndSelPoint( void );
	void		MouseDown( Event *ev );
	void		MouseUp( Event *ev );
	void		MouseDragged( Event *ev );
	void		Scroll( Event *ev );
	void		DragTimer( Event *ev );
	void		SizeChanged( Event *ev );
	void		setData( const char *data );
	void		getData( str& data );
	void		Empty( void );
	void		CopySelection( void );
	void		PasteSelection( void );
	void		DeleteSelection( void );
	void		setChanged( bool b );
	bool		IsChanged( void );
};

#endif

