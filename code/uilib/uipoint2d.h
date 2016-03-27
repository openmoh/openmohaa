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

#ifndef __UIPOINT2D_H__
#define __UIPOINT2D_H__

class UIPoint2D {
public:
	float x;
	float y;

	UIPoint2D();
	UIPoint2D( float x, float y );

	bool operator==( const UIPoint2D& pos ) const;
	bool operator!=( const UIPoint2D& pos ) const;
};

inline
UIPoint2D::UIPoint2D()
{
	x = y = 0;
}

inline
UIPoint2D::UIPoint2D
	(
	float x,
	float y
	)

{
	this->x = x;
	this->y = y;
}

inline
bool UIPoint2D::operator==( const UIPoint2D& pos ) const
{
	return ( x == pos.x && y == pos.y );
}

inline
bool UIPoint2D::operator!=( const UIPoint2D& pos ) const
{
	return ( x != pos.x || y != pos.y );
}

#endif /* __UIPOINT2D_H__ */
