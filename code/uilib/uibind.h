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

#ifndef __UIBIND_H__
#define __UIBIND_H__

class UIBindButton : public UIButton {
	str m_bindcommand;
	int m_bindindex;
	qboolean m_getkey;
	qboolean m_alternate;
	str m_entersound;
	str m_activesound;
	str m_last_keyname;
	UIReggedMaterial *m_mat;

public:
	CLASS_PROTOTYPE( UIBindButton );

public:
	UIBindButton();
	UIBindButton( str entersound, str activesound );

	void		Pressed( Event *ev );
	void		Pressed( void );
	void		SetCommand( Event *ev );
	void		SetCommand( str s );
	void		DrawUnpressed( void );
	void		DrawPressed( void );
	void		Clear( void );
	qboolean	KeyEvent( int key, unsigned int time );
	void		SetAlternate( qboolean a );
};

#endif

