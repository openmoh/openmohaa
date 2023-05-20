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

#ifndef __UISTATUS_H__
#define __UISTATUS_H__

typedef enum { M_NONE, M_DRAGGING } mouseState_t;

class UIWindowSizer : public UIWidget {
protected:
	class UIWidget *m_draggingwidget;
	mouseState_t m_mouseState;
	UIPoint2D m_screenDragPoint;

public:
	CLASS_PROTOTYPE( UIWindowSizer );

	UIWindowSizer();
	UIWindowSizer(UIWidget* w);

	void			Draw( void ) override;
	void			FrameInitialized( void ) override;
	void			MouseDown( Event *ev );
	void			MouseUp( Event *ev );
	void			MouseDragged( Event *ev );
	void			setDraggingWidget( UIWidget *w );
	UIWidget		*getDraggingWidget( void );
};

typedef enum { WND_ALIGN_NONE, WND_ALIGN_BOTTOM } alignment_t;

typedef struct align_s {
public:
	float dist;
	alignment_t alignment;
} align_t;

class UIStatusBar : public UIWidget {
protected:
	align_t m_align;
	class UIWidget *m_sizeenabled;
	bool m_created;

	UIWindowSizer *m_sizer;

public:
	CLASS_PROTOTYPE( UIStatusBar );

protected:
	void		FrameInitialized( void ) override;

public:
	UIStatusBar();
	UIStatusBar( alignment_t align, float height );

	void				Draw( void ) override;
	void				AlignBar( alignment_t align, float height );
	void				DontAlignBar( void );
	void				EnableSizeBox( UIWidget *which );
	void				ParentSized( Event *ev );
	void				SelfSized( Event *ev );
};

#endif

