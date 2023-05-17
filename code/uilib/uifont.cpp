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

#include "ui_local.h"

UIFont::UIFont()
{
	m_font = uii.Rend_LoadFont( "verdana-14" );
	if( !m_font ) {
		uii.Sys_Error( ERR_DROP, "Couldn't load font Verdana\n" );
	}

	color = UBlack;
	setColor( color );
}

UIFont::UIFont
	(
	const char *fn
	)

{
	setFont( fn );
}

void UIFont::Print
	(
	float x,
	float y,
	const char *text,
	size_t maxlen,
	qboolean bVirtualScreen
	)

{
	uii.Rend_SetColor( color );
	uii.Rend_DrawString( m_font, text, x, y, maxlen, bVirtualScreen );
}

void UIFont::PrintJustified
	(
	const UIRect2D& rect,
	fonthorzjustify_t horz,
	fontvertjustify_t vert,
	const char *text,
	float *vVirtualScale
	)

{
	float newx, newy;
	int textwidth, textheight;
	UIRect2D sizedRect;
	const char* source;
	char* dest;
	char string[2048];

	if (vVirtualScale)
	{
		sizedRect.pos.x = rect.pos.x / vVirtualScale[0];
		sizedRect.pos.y = rect.pos.y / vVirtualScale[1];
		sizedRect.size.width = rect.size.width / vVirtualScale[0];
		sizedRect.size.height = rect.size.height / vVirtualScale[1];
	}
	else
	{
		sizedRect = rect;
	}

	if (horz == FONT_JUSTHORZ_LEFT && vert == FONT_JUSTVERT_TOP)
	{
		// no need to justify
		Print(sizedRect.pos.x, sizedRect.pos.y, text, -1, vVirtualScale != NULL);
		return;
	}

	textheight = getHeight(text, -1, qfalse);

	switch (vert)
	{
	case FONT_JUSTVERT_TOP:
		newy = sizedRect.pos.y;
		break;
	case FONT_JUSTVERT_CENTER:
		newy = sizedRect.pos.y + (sizedRect.size.height - textheight) * 0.5;
		break;
	case FONT_JUSTVERT_BOTTOM:
		newy = sizedRect.pos.y + sizedRect.size.height - textheight;
		break;
	default:
		newy = 0;
		break;
	}

	source = text;
	while (*source)
	{
		// skip new lines
		while (*source == '\n') {
			source++;
		}

		if (!*source) {
			// don't print an empty string
			return;
		}

		dest = string;

		do
			*dest++ = *source++;
		while (*source && *source != '\n');

		*dest = 0;

		textwidth = getWidth(string, -1);

		switch (horz)
		{
		case FONT_JUSTHORZ_CENTER:
			newx = sizedRect.pos.x + (sizedRect.size.width - textwidth) * 0.5;
			break;
		case FONT_JUSTHORZ_LEFT:
			newx = sizedRect.pos.x;
			break;
		case FONT_JUSTHORZ_RIGHT:
			newx = sizedRect.pos.x + sizedRect.size.width - textwidth;
			break;
		default:
			newx = 0.0;
			break;
		}

		Print(newx, newy, string, -1, vVirtualScale != NULL);

		// expand for newline
		newy += getHeight(" ", -1, qfalse);
	}
}

void UIFont::setColor
	(
	UColor col
	)

{
	color = col;
}

void UIFont::setAlpha
	(
	float alpha
	)

{
	color.a = alpha;
}

void UIFont::setFont
	(
	const char *fontname
	)

{
	m_font = uii.Rend_LoadFont( fontname );
	if( !m_font ) {
		uii.Sys_Error( ERR_DROP, "Couldn't load font %s\n", fontname );
	}
}

int UIFont::getWidth
	(
	const char *text,
	int maxlen
	)

{
	return UI_FontStringWidth( m_font, text, maxlen );
}

int UIFont::getCharWidth
	(
	char ch
	)

{
	int indirected = m_font->indirection[ 32 ];

	if( ch == '\t' )
	{
		indirected = m_font->indirection[ 32 ];

		if( indirected > 255 ) {
			Com_Printf( "getCharWidth: no space-character in font!\n" );
			return 0;
		}

		return m_font->locations[ indirected ].size[ 0 ] * 256.0 * 3.0;
	}
	else
	{
		indirected = m_font->indirection[ ch ];

		if( indirected > 255 ) {
			Com_Printf( "getCharWidth: no 0x%02x-character in font!\n", ch );
			return 0;
		}

		return m_font->locations[ indirected ].size[ 0 ] * 256.0;
	}
}

int UIFont::getHeight
	(
	const char *text,
	int maxlen,
	qboolean bVirtual
	)

{
	float height;
	int i;

	if( !m_font ) {
		return 0;
	}

	height = getHeight( bVirtual );

	for(i = 0; text[i]; i++)
	{
		if( maxlen != -1 && i > maxlen ) {
			break;
		}

		if( text[ i ] == '\n' )
		{
			height += getHeight( bVirtual );
		}
	}

	return height;
}

int UIFont::getHeight
	(
	qboolean bVirtual
	)

{
	if( bVirtual )
	{
		if( m_font )
		{
			return ( m_font->height * uid.vidHeight / 480.0 );
		}
		else
		{
			return ( 16.0 * uid.vidHeight / 480.0 );
		}
	}
	else
	{
		if( m_font )
		{
			return m_font->height;
		}
		else
		{
			return 16;
		}
	}
}

int UI_FontStringWidth
	(
	fontheader_t *pFont,
	const char *pszString,
	int iMaxLen
	)

{
	float widths = 0.0;
	float maxwidths = 0.0;
	int indirected;
	int i;

	if( !pFont ) {
		return 0;
	}

	if( iMaxLen == -1 ) {
		iMaxLen = strlen( pszString ) + 1;
	}

	for( i = 0; i < iMaxLen; i++ )
	{
		unsigned char c = pszString[ i ];

		if( c == 0 )
		{
			break;
		}
		else if( c == '\t' )
		{
			indirected = pFont->indirection[ 32 ];

			if( indirected > 255 ) {
				Com_Printf( "UIFont::getWidth: no space-character in font!\n" );
				continue;
			} else {
				widths += pFont->locations[ indirected ].size[ 0 ] * 3.0;
			}
		}
		else if( c == '\n' )
		{
			widths = 0.0;
		}
		else
		{
			indirected = pFont->indirection[ c ];

			if( indirected > 255 ) {
				Com_Printf( "UIFont::getWidth: no 0x%02x-character in font!\n", c );
				continue;
			} else {
				widths += pFont->locations[ indirected ].size[ 0 ];
			}
		}

		if( maxwidths < widths ) {
			maxwidths = widths;
		}
	}

	return maxwidths * 256.0;
}
