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

#ifndef __UI_CONSOLE_H__
#define __UI_CONSOLE_H__

typedef void ( *consoleHandler_t )( const char *text );

class item {
public:
	str string;
	int lines;
	int begins[ 10 ];
	int breaks[ 10 ];
	UColor *pColor;

	item();
};

inline
item::item()
{
	lines = 0;

	for( int i = 0; i < 10; i++ )
	{
		begins[ i ] = 0;
		breaks[ i ] = 0;
	}

	pColor = NULL;
}

#define MAX_CONSOLE_ITEMS 300

class UIConsole : public UIWidget {
protected:
	UList<str> m_history;
	void *m_historyposition;
	item m_items[MAX_CONSOLE_ITEMS];
	str m_currentline;
	UIVertScroll *m_scroll;
	int m_firstitem;
	int m_numitems;
	int m_caret;
	str m_completionbuffer;
	bool m_refreshcompletionbuffer;
	int m_cntcmdnumber;
	int m_cntcvarnumber;
	consoleHandler_t m_consolehandler;

public:
	CLASS_PROTOTYPE( UIConsole );

protected:
	int					getFirstItem( void );
	int					getNextItem( int prev );
	int					getLastItem( void );
	int					AddLine( void );
	void				DrawBottomLine( void );
	void				AddHistory( void );

	virtual void		Print( Event *ev );
	virtual void		KeyEnter( void );

public:
	UIConsole();

	void		setConsoleHandler( consoleHandler_t handler );
	void		AddText( const char *text, UColor *pColor );
	void		CalcLineBreaks( item& theItem );
	void		Clear( void );
	void		FrameInitialized( void ) override;
	void		Draw( void ) override;
	void		CharEvent( int ch ) override;
	qboolean	KeyEvent( int key, unsigned int time ) override;
	void		OnSizeChanged( Event *ev );
};

class UIFloatingConsole : public UIFloatingWindow {
protected:
	UIStatusBar *m_status;
	SafePtr<UIConsole> m_console;
	consoleHandler_t m_handler;
	UColor m_consoleColor;
	UColor m_consoleBackground;
	float m_consoleAlpha;

public:
	CLASS_PROTOTYPE( UIFloatingConsole );

	UIFloatingConsole();
	~UIFloatingConsole();

	void		FrameInitialized( void ) override;
	void		OnChildSizeChanged( Event *ev );
	void		AddText( const char *text, UColor *pColor );
	void		setConsoleHandler( consoleHandler_t handler );
	void		Clear( void );
	void		OnClosePressed( Event *ev );
	void		setConsoleBackground( const UColor& color, float alpha );
	void		setConsoleColor( const UColor& color );
};

class UIDMConsole : public UIConsole {
	qboolean m_bQuickMessageMode;
	int m_iMessageMode;

public:
	CLASS_PROTOTYPE( UIDMConsole );

private:
	void	KeyEnter( void ) override;

public:
	UIDMConsole();

	void			AddDMMessageText( const char *text, UColor *pColor );
	void			Draw( void ) override;
	qboolean		KeyEvent( int key, unsigned int time ) override;
	qboolean		GetQuickMessageMode( void );
	void			SetQuickMessageMode( qboolean bQuickMessage );
	int				GetMessageMode( void );
	void			SetMessageMode( int iMode );
};

class UIFloatingDMConsole : public UIFloatingWindow {
protected:
	UIStatusBar *m_status;
	SafePtr<UIDMConsole> m_console;
	consoleHandler_t m_handler;
	UColor m_consoleColor;
	UColor m_consoleBackground;
	float m_consoleAlpha;

public:
	CLASS_PROTOTYPE( UIFloatingDMConsole );

	UIFloatingDMConsole();
	~UIFloatingDMConsole();

	void			FrameInitialized( void ) override;
	void			OnChildSizeChanged( Event *ev );
	void			AddText( const char *text, UColor *pColor );
	void			AddDMMessageText( const char *text, UColor *pColor );
	void			setConsoleHandler( consoleHandler_t handler );
	void			Clear( void );
	void			OnClosePressed( Event *ev );
	void			setConsoleBackground( const UColor& color, float alpha );
	void			setConsoleColor( const UColor& color );
	qboolean		GetQuickMessageMode( void );
	void			SetQuickMessageMode( qboolean bQuickMessage );
	int				GetMessageMode( void );
	void			SetMessageMode( int iMode );
};

#endif
