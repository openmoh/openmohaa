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

typedef void ( *consoleHandler_t )( const char *text );

typedef struct {
	str string;
	int lines;
	int begins[ 10 ];
	int breaks[ 10 ];
	UColor *pColor;
} item;

typedef struct UIConsole_s {
	UIWidget baseClass;
	UList_str_ m_history;
	void *m_historyposition;
	item m_items[ 300 ];
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
} UIConsole;

typedef struct UIFloatingConsole_s {
	UIFloatingWindow baseClass;
	UIStatusBar *m_status;
	SafePtr2_t m_console;
	consoleHandler_t m_handler;
	UColor m_consoleColor;
	UColor m_consoleBackground;
	float m_consoleAlpha;
} UIFloatingConsole;

typedef struct UIDMConsole_s {
	UIConsole baseClass;
	qboolean m_bQuickMessageMode;
	int m_iMessageMode;
} UIDMConsole;

typedef struct UIFloatingDMConsole_s {
	UIFloatingWindow baseClass;
	UIStatusBar *m_status;
	SafePtr2_t m_console;
	consoleHandler_t m_handler;
	UColor m_consoleColor;
	UColor m_consoleBackground;
	float m_consoleAlpha;
} UIFloatingDMConsole;
