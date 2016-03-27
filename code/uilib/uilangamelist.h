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

#ifndef __UILANGAMELIST_H__
#define __UILANGAMELIST_H__

class UILanGameList : public UIWidget {
	bool m_created;
	UIVertScroll *m_Vscroll;
	class UIHorizScroll *m_Hscroll;
	UIReggedMaterial *m_noservers_mat;
	UILabel *m_noservers_wid;
	UIReggedMaterial *m_fill_mat;
	Container<UIWidget *> m_widgetlist;
	Container<UIWidget *> m_titlewidgets;
	Container<UIWidget *> m_miscwidgets;
	int m_activerow;
	int m_activeitem;
	Container<serverInfo_t *> m_servers;
	int m_iNumColumns;
	int m_iPrevNumServers;
	int m_iCurrNumServers;
	float m_fCurColumnWidth;

public:
	CLASS_PROTOTYPE( UILanGameList );

private:
	void				CreateServerWidgets( void );
	void				DestroyServerWidgets( void );
	void				RepositionServerWidgets( void );
	void				DrawNoServers( UIRect2D frame );
	void				AddColumn( str sName, UIReggedMaterial *pMaterial, int iWidth, Container<str> *csEntries );
	void				AddNoServer( void );
	virtual void		UpdateServers( void );

protected:
	void	FrameInitialized( void );
	void	EventScanNetwork( Event *ev );
	void	EventScaningNetwork( Event *ev );

public:
	UILanGameList();

	bool		isDying( void );
	void		Draw();
	qboolean	KeyEvent( int key, unsigned int time );
	void		Highlight( UIWidget *wid );
	void		Connect( void );
	void		EventConnect( Event *ev );
	void		PlayEnterSound( void );
	qboolean	SetActiveRow( UIWidget *w );
	void		Realign( void );
};

class UILanGameListLabel : public UILabel {
	int m_iLastPressedTime;
	UILanGameList *m_list;

public:
	CLASS_PROTOTYPE( UILanGameListLabel );

	UILanGameListLabel();
	UILanGameListLabel( UILanGameList *list );

	void	Pressed( Event *ev );
	void	Unpressed( Event *ev );
};

#endif

