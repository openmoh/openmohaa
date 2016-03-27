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

#ifndef __UIBINDLIST_H__
#define __UIBINDLIST_H__

class UIFakkBindList : public UIWidget {
	bool m_created;
	UIVertScroll *m_scroll;
	UIReggedMaterial *m_presskey_mat;
	UILabel *m_presskey_wid;
	Container<UIWidget *> m_widgetlist;
	Container<UIWidget *> m_miscwidgets;
	int m_activerow;
	int m_activeitem;
	bind_t *m_bind;

public:
	CLASS_PROTOTYPE( UIFakkBindList );

private:
	void		CreateBindWidgets( void );
	void		DestroyBindWidgets( void );
	void		RepositionBindWidgets( void );
	void		DrawPressKey( UIRect2D frame );

protected:
	void		FrameInitialized( void );
	void		Filename( Event *ev );
	void		StopBind( Event *ev );

public:
	UIFakkBindList();

	void		setBind( bind_t *b );
	void		Draw( void );
	bool		isDying( void );
	qboolean	KeyEvent( int key, unsigned int time );
	void		Highlight( UIWidget *wid );
	void		PlayEnterSound( void );
	qboolean	SetActiveRow( UIWidget *w );
	void		Realign( void );
};

class UIFakkBindListLabel : public UILabel {
	UIFakkBindList *m_list;

public:
	CLASS_PROTOTYPE( UIFakkBindListLabel );

public:
	UIFakkBindListLabel();
	UIFakkBindListLabel( UIFakkBindList *list );

	void	Pressed( Event *ev );
	void	Draw( void );
};

extern Event EV_UIFakkBindList_Filename;
extern Event EV_UIFakkBindList_StopBind;

#endif

