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

#ifndef __UIWINMAN_H__
#define __UIWINMAN_H__

class Binding {
public:
	str binding;
	int key1;
	int key2;
};

class UIWindowManager : public UIWidget {
	unsigned int m_lastbuttons;
	UIPoint2D m_oldpos;
	SafePtr<UIWidget> m_oldview;
	SafePtr<UIWidget> m_firstResponder;
	class UIWidget *m_activeControl;
	class UIWidget *m_backgroundwidget;
	class UIReggedMaterial *m_cursor;
	str m_cursorname;
	bool m_showcursor;
	Container<UIReggedMaterial *> m_materiallist;
	bool m_amidead;
	class UIWidget *m_bindactive;
	Container<Binding *> bindings;

public:
	CLASS_PROTOTYPE( UIWindowManager );

private:
	void			ViewEvent( UIWidget *view, Event& event, UIPoint2D& pos, int buttons );
	UIWidget		*getResponder( UIPoint2D& pos );

public:
	UIWindowManager();
	~UIWindowManager();

	void				Init( const UIRect2D& frame, const char *fontname );
	void				setBackgroundWidget( UIWidget *widget );
	void				UpdateViews( void );
	void				ServiceEvents( void );
	void				setFirstResponder( UIWidget *responder );
	UIWidget			*getFirstResponder( void );
	UIWidget			*ActiveControl( void );
	void				ActivateControl( UIWidget *control );
	void				DeactivateCurrentControl( void );
	UIWidget			*FindNextControl( UIWidget *control );
	UIWidget			*FindPrevControl( UIWidget *control );
	void				setCursor( const char *string );
	void				showCursor( bool show );
	virtual void		CharEvent( int ch );
	virtual qboolean	KeyEvent( int key, unsigned int time );
	void				CreateMenus( void );
	UIReggedMaterial	*RegisterShader( const str& name );
	UIReggedMaterial	*RefreshShader( const str& name );
	void				CleanupShadersFromList( void );
	void				RefreshShadersFromList( void );
	void				DeactivateCurrentSmart( void );
	bool				IsDead( void );
	int					AddBinding( str binding );
	str					GetKeyStringForCommand( str command, int index, qboolean alternate, int *key1, int *key2 );
	void				BindKeyToCommand( str command, int key, int index, qboolean alternate );
	UIWidget			*BindActive( void );
	void				SetBindActive( UIWidget *w );
	virtual void		Shutdown( void );
	void				DeactiveFloatingWindows( void );
	bool				DialogExists( void );
	void				RemoveAllDialogBoxes( void );
};

extern Event W_MouseExited;
extern Event W_MouseEntered;
extern Event W_LeftMouseDragged;
extern Event W_RightMouseDragged;
extern Event W_CenterMouseDragged;
extern Event W_MouseMoved;
extern Event W_LeftMouseDown;
extern Event W_LeftMouseUp;
extern Event W_RightMouseDown;
extern Event W_RightMouseUp;
extern Event W_CenterMouseDown;
extern Event W_CenterMouseUp;
extern Event W_AnyMouseDown;
extern Event W_AnyMouseUp;

extern UIWindowManager uWinMan;

#endif
