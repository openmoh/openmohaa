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

#ifndef __UISIZE2D_H__
#define __UISIZE2D_H__

class UISize2D {
public:
	float width;
	float height;

public:
	UISize2D();
	UISize2D( float width, float height );

	bool operator==( const UISize2D& pos ) const;
	bool operator!=( const UISize2D& pos ) const;
};

inline
UISize2D::UISize2D()
{
	width = height = 0.0f;
}

inline
UISize2D::UISize2D
	(
	float width,
	float height
	)

{
	this->width = width;
	this->height = height;
}

inline
bool UISize2D::operator==( const UISize2D& pos ) const
{
	return ( width == pos.width && height == pos.height );
}

inline
bool UISize2D::operator!=( const UISize2D& pos ) const
{
	return ( width != pos.width || height != pos.height );
}

#endif /* __UISIZE2D_H__ */
