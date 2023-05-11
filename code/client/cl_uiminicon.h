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

#ifndef __CL_UIMINICON_H__
#define __CL_UIMINICON_H__

class FakkMiniconsole : public UIWidget {
protected:
	Container<str> m_lines;
	int m_maxlines;
	bool m_reallyshown;
	bool m_boxmovingout;
	boxstate_t m_boxstate;
	int m_boxtime;
	int m_movespeed;

public:
	CLASS_PROTOTYPE( FakkMiniconsole );

protected:
	void		VerifyBoxOut( void );
	void		ChangeBoxState( boxstate_t state );
	void		HandleBoxMoving( void );
	void		PostMoveinEvent( void );
	void		setShowState( void );

public:
	FakkMiniconsole();

	void			OnSizeChanged( Event *ev );
	void			Print( const char *text );
	void			Create( const UISize2D& size, const UColor& fore, const UColor& back, float alpha );
	void			MoveInEvent( Event *ev );
	void			Draw( void ) override;
	void			setRealShow( bool b );
};

#endif
