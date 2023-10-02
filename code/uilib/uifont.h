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

#pragma once

class UIRect2D;

typedef enum {
    FONT_JUSTHORZ_CENTER,
    FONT_JUSTHORZ_LEFT,
    FONT_JUSTHORZ_RIGHT
} fonthorzjustify_t;

typedef enum {
    FONT_JUSTVERT_TOP,
    FONT_JUSTVERT_CENTER,
    FONT_JUSTVERT_BOTTOM
} fontvertjustify_t;

class UIFont
{
protected:
    unsigned int  m_listbase;
    UColor        color;
    fontheader_t *m_font;

public:
    UIFont();
    UIFont(const char *fn);

    void Print(float x, float y, const char *text, size_t maxlen, qboolean bVirtualScreen);
    void PrintJustified(
        const UIRect2D& rect, fonthorzjustify_t horz, fontvertjustify_t vert, const char *text, float *vVirtualScale
    );
    void PrintOutlinedJustified(
        const UIRect2D& rect, fonthorzjustify_t horz, fontvertjustify_t vert, const char* text, const UColor& outlineColor, float* vVirtualScale
    );
    void setColor(UColor col);
    void setAlpha(float alpha);
    void setFont(const char *fontname);
    int  getMaxWidthIndex(const char* text, int maxlen);
    int  getWidth(const char *text, int maxlen);
    int  getCharWidth(unsigned short ch);
    int  getHeight(const char *text, int maxlen, qboolean bVirtual);
    int  getHeight(qboolean bVirtual);
    int  CodeSearch(unsigned short uch);
};

int UI_FontStringWidth(fontheader_t *pFont, const char *pszString, int iMaxLen);
