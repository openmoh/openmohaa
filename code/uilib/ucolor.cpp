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

#include "ucolor.h"

UColor UClear( 0, 0, 0, 0 );
UColor UWhite( 1, 1, 1, 1 );
UColor UBlack( 0, 0, 0, 1 );
UColor ULightGrey( 0.875, 0.875, 0.875, 1.0 );
UColor UGrey( 0.75, 0.75, 0.75, 1.0 );
UColor UDarkGrey( 0.5, 0.5, 0.5, 1.0 );
UColor ULightRed( 1.0, 0.5, 0.5, 1.0 );
UColor URed( 1.0, 0.0, 0.0, 1.0 );
UColor UGreen( 0.0, 1.0, 0.0, 1.0 );
UColor ULightGreen( 0.5, 1.0, 0.5, 1.0 );
UColor UBlue( 0.0, 0.0, 1.0, 1.0 );
UColor UYellow( 1.0, 1.0, 0.0, 1.0 );
UColor UHudColor( 0.7f, 0.6f, 0.05f, 1.0f );

UColor::UColor
	(
	void
	)

{
	r = g = b = 0;
	a = 1.0f;
}

UColor::UColor
	(
	float r,
	float g,
	float b,
	float a
	)

{
	set( r, g, b, a );
}

UColor::UColor
	(
	UColorHSV hsv
	)

{
	float	hh, p, q, t, ff;
	int		i;

	if( hsv.s <= 0.0 ) {
		r = hsv.v;
		g = hsv.v;
		b = hsv.v;
		return;
	}

	hh = hsv.h;
	if( hh >= 360.0 ) hh = 0.0;
	hh /= 60.0;
	i = ( long )hh;
	ff = hh - i;
	p = hsv.v * ( 1.0f - hsv.s );
	q = hsv.v * ( 1.0f - ( hsv.s * ff ) );
	t = hsv.v * ( 1.0f - ( hsv.s * ( 1.0f - ff ) ) );

	switch( i ) {
	case 0:
		r = hsv.v;
		g = t;
		b = p;
		break;
	case 1:
		r = q;
		g = hsv.v;
		b = p;
		break;
	case 2:
		r = p;
		g = hsv.v;
		b = t;
		break;
	case 3:
		r = p;
		g = q;
		b = hsv.v;
		break;
	case 4:
		r = t;
		g = p;
		b = hsv.v;
		break;
	case 5:
	default:
		r = hsv.v;
		g = p;
		b = q;
		break;
	}

	a = hsv.a;
}

UColor::operator float *( )
{
	return ( float * )this;
}

UColor::operator float *( ) const
{
	return ( float * )this;
}

void UColor::ScaleColor
	(
	float scale
	)

{
	r *= scale;
	g *= scale;
	b *= scale;
}

void UColor::ScaleAlpha
	(
	float scale
	)

{
	a *= scale;
}

void UColor::set
	(
	float r,
	float g,
	float b,
	float a
	)

{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

UColorHSV::UColorHSV()
{
	h = s = v = a = 0;
}

UColorHSV::UColorHSV
	(
	UColor rgb
	)

{
	float min;
	float max;
	float delta;

	min = rgb.r < rgb.g ? rgb.r : rgb.g;
	min = min  < rgb.b ? min : rgb.b;

	max = rgb.r > rgb.g ? rgb.r : rgb.g;
	max = max  > rgb.b ? max : rgb.b;

	v = max;
	delta = max - min;
	if( delta < 0.00001 )
	{
		s = 0;
		h = 0; // undefined, maybe NaN?
		return;
	}

	// NOTE: if Max is == 0, this divide would cause a crash
	if( max > 0.0 ) {
		s = ( delta / max );
	} else {
		// if max is 0, then r = g = b = 0              
		// s = 0, v is undefined
		s = 0.0;
		h = 0/s; // (NAN), its now undefined
		return;
	}

	if( rgb.r >= max ) {
		// between yellow & magenta
		h = ( rgb.g - rgb.b ) / delta;
	} else {
		if( rgb.g >= max ) {
			// between cyan & yellow
			h = 2.0f + ( rgb.b - rgb.r ) / delta;
		} else {
			// between magenta & cyan
			h = 4.0f + ( rgb.r - rgb.g ) / delta;
		}

		// degrees
		h *= 60.0;
	}

	if( h < 0.0 )
		h += 360.0;

	a = rgb.a;
}

UColorHSV::UColorHSV
	(
	float h,
	float s,
	float v,
	float a
	)

{
	set( h, s, v, a );
}

void UColorHSV::set
	(
	float h,
	float s,
	float v,
	float a
	)

{
	this->h = h;
	this->s = s;
	this->v = v;
	this->a = a;
}

UBorderColor::UBorderColor()
{

}

UBorderColor::UBorderColor
	(
	const UColor& dark,
	const UColor& reallydark,
	const UColor& light
	)

{
	this->dark = dark;
	this->reallydark = reallydark;
	this->light = light;
}

UBorderColor::UBorderColor
	(
	const UColor& color
	)

{
	UColorHSV lighttemp;

	dark.r = color.r * 0.6f;
	dark.g = color.g * 0.6f;
	dark.b = color.b * 0.6f;
	dark.a = color.a;
	reallydark.r = color.r * 0.3f;
	reallydark.g = color.g * 0.3f;
	reallydark.b = color.b * 0.3f;
	reallydark.b = color.a;
	lighttemp = color;
	lighttemp.s *= 0.75f;
	lighttemp.v *= 1.3f;
	light = lighttemp;
	original = color;
}

void UBorderColor::CreateSolidBorder
	(
	const UColor& color,
	colorType_t type
	)

{
	// FIXME: stub
}
