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

typedef struct UINotepad_s UINotepad;

typedef struct UINotepadEdit_s {
	UIMultiLineEdit baseClass;
	UINotepad *m_notepad;
} UINotepadEdit;

typedef enum { STATE_NONE, STATE_GOTO_LINE, STATE_FIND_TEXT, STATE_SAVE_AS, STATE_TIMED_MESSAGE, STATE_CONFIRMCLOSE } state_t;

typedef struct textinput_s {
	str text;
} textinput_t;

typedef struct timedmessage_s {
	int die;
	str text;
} timedmessage_t;

typedef struct ctrlevent_s {
	char ch;
	Event *ev;
} ctrlevent_t;

typedef struct UINotepad_s {
	UIFloatingWindow baseClass;
	Container_t m_popups;

	state_t m_state;
	textinput_s m_textinput;
	timedmessage_s m_timedmessage;
	str m_lastfind;
	str m_filename;
	UINotepadEdit *m_edit;
	UIStatusBar *m_status;
	UIPulldownMenu *m_menu;
} UINotepad;
