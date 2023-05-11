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

#ifndef __UIBUTTON_H__
#define __UIBUTTON_H__

class UIButtonBase : public UIWidget {
protected:
	mouseState_t m_mouseState;
	str m_hoverSound;
	str m_hoverCommand;

public:
	CLASS_PROTOTYPE( UIButtonBase );

protected:
	void		Pressed( Event *ev );
	void		Released( Event *ev );
	void		MouseEntered( Event *ev );
	void		MouseExited( Event *ev );
	void		Dragged( Event *ev );
	void		SetHoverSound( Event *ev );
	void		SetHoverCommand( Event *ev );

public:
	UIButtonBase();

	void		Action( void );
};

class UIButton : public UIButtonBase {
public:
	CLASS_PROTOTYPE( UIButton );

private:
	void				Draw( void ) override;
	virtual void		DrawPressed( void );
	virtual void		DrawUnpressed( void );

public:
	UIButton();

	qboolean KeyEvent( int key, unsigned int time ) override;
};

class ToggleCVar : public USignal {
protected:
	str m_cvarname;
	UIButton *m_button;

protected:
	CLASS_PROTOTYPE( ToggleCVar );

	void		Press( Event *ev );

public:
	ToggleCVar();
	ToggleCVar( UIButton *button, const char *cvar );

	void		setCVar( const char *cvar );
	void		setButton( UIButton *button );
};

class ExecCmd : public USignal {
protected:
	UIButton *m_button;
	str m_cmd;

public:
	CLASS_PROTOTYPE( ExecCmd );

protected:
	void	Press( Event *ev );

public:
	ExecCmd();
	ExecCmd( UIButton *button, const char *cmd );

	void	setCommand( const char *cmd );
	void	setButton( UIButton *button );
};

#endif
