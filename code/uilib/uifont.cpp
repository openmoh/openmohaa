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

qboolean UI_FontDBCSIsLeadByte(fontheader_t* font, unsigned short uch);
int UI_FontCodeSearch(const fontheader_t* font, unsigned short uch);

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
	unsigned short ch
	)

{
	int index;
	int indirected;

	if (!m_font) {
		return 0;
	}

	if (ch == '\t') {
		ch = ' ';
	}

	if (m_font->numPages)
	{
		int code = CodeSearch(ch);
		if (code < 0) {
			return 0;
		}

		index = m_font->charTable[code].index;
		indirected = m_font->charTable[code].loc;
	}
	else
	{
		indirected = m_font->sgl[0]->indirection[ch];
		if (indirected < 0 || indirected > 255) {
			return 0;
		}

		index = 0;
	}

    if (ch == '\t') {
        return m_font->sgl[index]->locations[indirected].size[0] * 256.0 * 3.0;
	} else {
		return m_font->sgl[index]->locations[indirected].size[0] * 256.0;
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
			return ( m_font->sgl[0]->height * uid.vidHeight / 480.0 );
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
			return m_font->sgl[0]->height;
		}
		else
		{
			return 16;
		}
	}
}

int UIFont::CodeSearch(unsigned short uch)
{
	return UI_FontCodeSearch(m_font, uch);
}

qboolean UI_FontDBCSIsLeadByte(fontheader_t* font, unsigned short uch)
{
    // Byte ranges found in Wikipedia articles with relevant search strings in each case
    switch (font->codePage) {
    case 932:
        // Shift_jis
        return ((uch >= 0x81) && (uch <= 0x9F)) ||
            ((uch >= 0xE0) && (uch <= 0xFC));
        // Lead bytes F0 to FC may be a Microsoft addition.
    case 936:
        // GBK
        return (uch >= 0x81) && (uch <= 0xFE);
    case 949:
        // Korean Wansung KS C-5601-1987
        return (uch >= 0x81) && (uch <= 0xFE);
    case 950:
        // Big5
        return (uch >= 0x81) && (uch <= 0xFE);
    case 1361:
        // Korean Johab KS C-5601-1992
        return
            ((uch >= 0x84) && (uch <= 0xD3)) ||
            ((uch >= 0xD8) && (uch <= 0xDE)) ||
            ((uch >= 0xE0) && (uch <= 0xF9));
    }
    return false;
}

int UI_FontCodeSearch(const fontheader_t* font, unsigned short uch) {
    int mid;
    int l, r;

    r = font->charTableLength;
    l = 0;
    while (l < r) {
        mid = (l + r) / 2;

        if (font->charTable[mid].cp > uch) {
            r = (l + r) / 2;
            continue;
        }

        if (uch == font->charTable[mid].cp) {
            return (l + r) / 2;
        }

        l = mid + 1;
    }

    if (uch != font->charTable[l].cp) {
        return -1;
    }

    return l;
}

float UI_FontgetCharWidthf(fontheader_t* font, unsigned short uch)
{
	int index;
	int indirected;

	if (!font) {
		return 0.f;
	}

	if (uch == '\t') {
		uch = ' ';
	}

	if (font->numPages)
	{
		int code = UI_FontCodeSearch(font, uch);
		if (code < 0) {
			return 0.f;
		}

		index = font->charTable[code].index;
		indirected = font->charTable[code].loc;
	}
	else
	{
		indirected = font->sgl[0]->indirection[uch];
		if (indirected < 0 || indirected > 255) {
			return 0.f;
		}

		index = 0;
	}

	if (uch == '\t') {
		return font->sgl[index]->locations[indirected].size[0] * 3.0;
	} else {
		return font->sgl[index]->locations[indirected].size[0];
	}
}

int UI_FontStringMaxWidth
	(
	fontheader_t *pFont,
	const char *pszString,
	int iMaxLen
	)

{
    float widths = 0.0;
    float maxwidths = iMaxLen / 256.0f;
    int i;

    if (!pFont) {
        return 0;
    }

	i = 0;
    for(;;)
    {
        unsigned short uch = pszString[i++];

        if (uch == 0 || (iMaxLen != -1 && i > iMaxLen))
        {
            break;
        }

		if (UI_FontDBCSIsLeadByte(pFont, uch))
		{
			uch = (uch << 8) | pszString[i];
			if (!pszString[i]) {
				break;
			}

			i++;
		}

        if (uch == '\n')
        {
            widths = 0.0;
        }
        else
        {
            widths += UI_FontgetCharWidthf(pFont, uch);

            if (maxwidths < widths) {
                return i - 1;
            }
        }
    }

    return -1;
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
    int i;

    if (!pFont) {
        return 0;
    }

	i = 0;
    for(;;)
    {
        unsigned short uch = pszString[i++];

        if (uch == 0 || (iMaxLen != -1 && i > iMaxLen))
        {
            break;
        }

		if (UI_FontDBCSIsLeadByte(pFont, uch))
		{
			uch = (uch << 8) | pszString[i];
			if (!pszString[i]) {
				break;
			}

			i++;
		}

        if (uch == '\n')
        {
            widths = 0.0;
        }
        else
        {
            widths += UI_FontgetCharWidthf(pFont, uch);

            if (maxwidths < widths) {
                maxwidths = widths;
            }
        }
    }

    return maxwidths * 256.0;
}
