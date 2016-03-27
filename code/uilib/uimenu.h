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

#ifndef __UIMENU_H__
#define __UIMENU_H__

class Menu : public Listener {
	qboolean m_fullscreen;
	int m_vidmode;

public:
	CLASS_PROTOTYPE( Menu );

public:
	Container<UIWidget *> m_itemlist;
	str m_name;

public:
	Menu();
	Menu( str name );

	void			AddMenuItem( UIWidget *item );
	void			DeleteMenuItem( UIWidget *item );
	void			setName( str name );
	void			ShowMenu( Event *ev );
	void			HideMenu( Event *ev );
	void			ForceShow( void );
	void			ForceHide( void );
	UIWidget		*GetContainerWidget( void );
	UIWidget		*GetNamedWidget( const char *pszName );
	void			Update( void );
	void			RealignWidgets( void );
	float			GetMaxMotionTime( void );
	void			ActivateMenu( void );
	qboolean		isFullscreen( void );
	void			setFullscreen( qboolean bFullScreen );
	int				getVidMode( void );
	void			setVidMode( int iMode );
	qboolean		isVisible( void );
	void			SaveCVars( void );
	void			RestoreCVars( void );
	void			ResetCVars( void );
	void			PassEventToWidget( str name, Event *ev );
	void			PassEventToAllWidgets( Event& ev );
	void			CheckRestart( void );
};

class MenuManager : public Listener {
	Container<Menu *> m_menulist;
	Stack<Menu *> m_showmenustack;
	Stack<Menu *> m_menustack;
	bool m_lock;

public:
	CLASS_PROTOTYPE( MenuManager );

public:
	MenuManager();

	void		RealignMenus( void );
	void		AddMenu( Menu *m );
	void		DeleteMenu( Menu *m );
	void		DeleteAllMenus( void );
	Menu		*FindMenu( str name );
	bool		PushMenu( str name );
	bool		ShowMenu( str name );
	void		PushMenu( Event *ev );
	void		PopMenu( qboolean restore_cvars );
	Menu		*CurrentMenu( void );
	bool		ClearMenus( bool force );
	void		ListMenus( void );
	void		UpdateAllMenus( void );
	void		Lock( Event *ev );
	void		Unlock( Event *ev );
	void		PassEventToWidget( str name, Event *ev );
	void		PassEventToAllWidgets( Event& ev );
	void		ResetCVars( void );
	void		CheckRestart( void );
	bool		ForceMenu( str name );
};

extern MenuManager menuManager;

extern Event EV_PushMenu;
extern Event EV_LockMenus;
extern Event EV_UnlockMenus;
extern Event EV_ShowMenu;
extern Event EV_HideMenu;

#endif

