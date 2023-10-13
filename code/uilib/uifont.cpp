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

qboolean UI_FontDBCSIsLeadByte(fontheader_t *font, unsigned short uch);
int      UI_FontCodeSearch(const fontheader_t *font, unsigned short uch);
int      UI_FontStringMaxWidth(fontheader_t *pFont, const char *pszString, int iMaxLen);

const unsigned char DBCS_Maekin_TraditionalChinese[] = {
    0x0A1, 0x0E3, 0x0A1, 0x0CA, 0x0A1, 0x5B,  0x0A2, 0x4E,  0x0A2, 0x0B1, 0x0A1, 0x0CC, 0x0A2, 0x0B2, 0x0A2, 0x43,
    0x0A2, 0x4C,  0x0A1, 0x0EC, 0x0A2, 0x4D,  0x0A2, 0x0B4, 0x0A1, 0x73,  0x0A1, 0x6F,  0x0A3, 0x4E,  0x0F9, 0x0DE,
    0x0A2, 0x73,  0x0F9, 0x0F0, 0x0A2, 0x0B5, 0x0A1, 0x0CD, 0x0A2, 0x0B6, 0x0A1, 0x0B0, 0x0A1, 0x0B8, 0x0A2, 0x0B7,
    0x0A1, 0x5D,  0x0A1, 0x5F,  0x0A1, 0x0B3, 0x0A1, 0x0B4, 0x0A2, 0x0B8, 0x0A1, 0x5E,  0x0A1, 0x60,  0x0A2, 0x0AF,
    0x0A1, 0x0C5, 0x0A1, 0x0DE, 0x0A1, 0x0F0, 0x0A1, 0x0F1, 0x0A1, 0x0F2, 0x0A1, 0x0D3, 0x0A2, 0x71,  0x0A2, 0x0A6,
    0x0F9, 0x0E1, 0x0F9, 0x0EA, 0x0F9, 0x0F3, 0x0A1, 0x0A1, 0x57,  0x0A1, 0x59,  0x0A2, 0x78,  0x0F9, 0x0F8, 0x0A2,
    0x79,  0x0A1, 0x49,  0x0A2, 0x49,  0x0A1, 0x0AD, 0x0A2, 0x43,  0x0A2, 0x48,  0x0A1, 0x0F4, 0x0A1, 0x0AE, 0x0A1,
    0x0AF, 0x0A1, 0x7D,  0x0A1, 0x7E,  0x0A1, 0x0C4, 0x0A1, 0x0CF, 0x0A1, 0x0FC, 0x0A1, 0x61,  0x0A1, 0x63,  0x0A1,
    0x0A1, 0x0A1, 0x62,  0x0A1, 0x64,  0x0A1, 0x0A2, 0x0A1, 0x0A7, 0x0A1, 0x0A8, 0x0A1, 0x0A9, 0x0A1, 0x0B2, 0x0A1,
    0x46,  0x0A1, 0x51,  0x0A3, 0x0B5, 0x0A1, 0x41,  0x0A1, 0x4D,  0x0A1, 0x42,  0x0A1, 0x4E,  0x0A3, 0x0AE, 0x0A1,
    0x0D5, 0x0A1, 0x0E0, 0x0A1, 0x71,  0x0A1, 0x0D8, 0x0A1, 0x0F6, 0x0A1, 0x0E7, 0x0A1, 0x6D,  0x0A1, 0x6F,  0x0A1,
    0x73,  0x0A1, 0x0F8, 0x0A2, 0x0A9, 0x0A2, 0x0AB, 0x0A3, 0x47,  0x0A2, 0x7A,  0x0A2, 0x7E,  0x0F9, 0x0DD, 0x0F9,
    0x0E6, 0x0F9, 0x0EF, 0x0F9, 0x0FA, 0x0A1, 0x0E8, 0x0A1, 0x0FA, 0x0A2, 0x7C,  0x0A2, 0x0A2, 0x0F9, 0x0E3, 0x0F9,
    0x0EC, 0x0F9, 0x0F5, 0x0F9, 0x0FC, 0x0A1, 0x44,  0x0A1, 0x43,  0x0A1, 0x4F,  0x0A1, 0x45,  0x0A1, 0x50,  0x0A3,
    0x0B2, 0x0A1, 0x0D2, 0x0A1, 0x0DC, 0x0A1, 0x0EE, 0x0A1, 0x0EF, 0x0A1, 0x0F3, 0x0A1, 0x4B,  0x0A1, 0x4C,  0x0A1,
    0x0D6, 0x0A1, 0x0E1, 0x0A1, 0x72,  0x0A1, 0x0D9, 0x0A1, 0x0F7, 0x0A1, 0x6E,  0x0A1, 0x70,  0x0A2, 0x7D,  0x0A2,
    0x0A3, 0x0F9, 0x0E5, 0x0F9, 0x0EE, 0x0F9, 0x0F7, 0x0A1, 0x0FB, 0x0A1, 0x0F9, 0x0A1, 0x74,  0x0A1, 0x0E9, 0x0A2,
    0x74,  0x0A2, 0x0A7, 0x0F9, 0x0E2, 0x0F9, 0x0EB, 0x0F9, 0x0F4, 0x0A2, 0x0A8, 0x0A2, 0x0AA, 0x0A1, 0x48,  0x0A1,
    0x53,  0x0,   0x0};

const unsigned char DBCS_Maekin_Korean[] = {
    0x0A1, 0x0C6, 0x0A1, 0x0AF, 0x0A1, 0x0B1, 0x0A1, 0x0C7, 0x0A1, 0x0C8, 0x0A1, 0x0C9, 0x0A1,
    0x0B5, 0x0A1, 0x0B7, 0x0A1, 0x0B9, 0x0A1, 0x0BB, 0x0A1, 0x0BD, 0x0A1, 0x0B3, 0x0A3, 0x0A1,
    0x0A3, 0x0A5, 0x0A3, 0x0A9, 0x0A3, 0x0AC, 0x0A3, 0x0AE, 0x0A3, 0x0BA, 0x0A3, 0x0BB, 0x0A3,
    0x0BF, 0x0A3, 0x0DD, 0x0A3, 0x0FD, 0x0A1, 0x0CB, 0x0,   0x0,   0x0,   0x0,   0x0,
};

const unsigned char DBCS_Maekin_SimplifiedChinese[] = {
    0x0A1, 0x0A7, 0x0A1, 0x0A4, 0x0A1, 0x0A6, 0x0A1, 0x0A5, 0x0A8, 0x44,  0x0A1, 0x0AC, 0x0A1, 0x0AF, 0x0A1,
    0x0B1, 0x0A1, 0x0AD, 0x0A1, 0x0C3, 0x0A1, 0x0A2, 0x0A1, 0x0A3, 0x0A1, 0x0A8, 0x0A1, 0x0A9, 0x0A1, 0x0B5,
    0x0A1, 0x0B7, 0x0A1, 0x0B9, 0x0A1, 0x0BB, 0x0A1, 0x0BF, 0x0A1, 0x0B3, 0x0A1, 0x0BD, 0x0A3, 0x0A1, 0x0A3,
    0x0A2, 0x0A3, 0x0A7, 0x0A3, 0x0A9, 0x0A3, 0x0AC, 0x0A3, 0x0AE, 0x0A3, 0x0BA, 0x0A3, 0x0BB, 0x0A3, 0x0BF,
    0x0A3, 0x0DD, 0x0A3, 0x0E0, 0x0A3, 0x0FC, 0x0A3, 0x0A1, 0x0AB, 0x0A1, 0x0,   0x0,
};
const unsigned char DBCS_Maekin_Japanese[] = {
    0x81,  0x8B,  0x81, 0x66,  0x81, 0x68,  0x81, 0x0F1, 0x81, 0x8C,  0x81, 0x8D,  0x81, 0x8E,  0x81, 0x41,  0x81,
    0x42,  0x81,  0x58, 0x81,  0x72, 0x81,  0x74, 0x81,  0x76, 0x81,  0x78, 0x81,  0x7A, 0x81,  0x6C, 0x82,  0x9F,
    0x82,  0x0A1, 0x82, 0x0A3, 0x82, 0x0A5, 0x82, 0x0A7, 0x82, 0x0C1, 0x82, 0x0E1, 0x82, 0x0E3, 0x82, 0x0E5, 0x82,
    0x0EC, 0x81,  0x4A, 0x81,  0x4B, 0x81,  0x54, 0x81,  0x55, 0x83,  0x40, 0x83,  0x42, 0x83,  0x44, 0x83,  0x46,
    0x83,  0x48,  0x83, 0x62,  0x83, 0x83,  0x83, 0x85,  0x83, 0x87,  0x83, 0x8E,  0x83, 0x95,  0x83, 0x96,  0x81,
    0x45,  0x81,  0x5B, 0x81,  0x52, 0x81,  0x53, 0x81,  0x49, 0x81,  0x93, 0x81,  0x6A, 0x81,  0x43, 0x81,  0x44,
    0x81,  0x46,  0x81, 0x47,  0x81, 0x48,  0x81, 0x6E,  0x81, 0x70,  0x0,  0x0,   0x0,  0x0,
};

const unsigned char DBCS_Tokin_Japanese[] = {
    0x81, 0x92, 0x81, 0x65, 0x81, 0x67, 0x81, 0x71, 0x81, 0x73, 0x81, 0x75, 0x81, 0x77, 0x81, 0x79,
    0x81, 0x6B, 0x81, 0x90, 0x81, 0x69, 0x81, 0x6D, 0x81, 0x6F, 0x81, 0x92, 0x81, 0x8F, 0x0,
};

const unsigned char DBCS_Tokin_Korean[] = {
    0x0A1, 0x0AE, 0x0A1, 0x0B0, 0x0A1, 0x0B4, 0x0A1, 0x0B6, 0x0A1, 0x0B8, 0x0A1, 0x0BA, 0x0A1, 0x0BC,
    0x0A1, 0x0B2, 0x0A3, 0x0A4, 0x0A3, 0x0A8, 0x0A3, 0x0DB, 0x0A3, 0x0FB, 0x0A3, 0x0DC, 0x0,   0x0,
};

int UIFont::CodeSearch(unsigned short uch)
{
    return UI_FontCodeSearch(m_font, uch);
}

bool UIFont::DBCSIsLeadByte(unsigned short uch)
{
    return UI_FontDBCSIsLeadByte(m_font, uch);
}

bool UIFont::DBCSIsMaekin(unsigned short uch)
{
    const unsigned char   *tokinList;
    const unsigned char   *p;
    unsigned char ch;

    if (uch < 0x100) {
        return false;
    }

    ch = (uch >> 8) & 0xFF;

    switch (m_font->codePage) {
    case 932:
        tokinList = DBCS_Maekin_Japanese;
        break;
    case 936:
        tokinList = DBCS_Maekin_SimplifiedChinese;
        break;
    case 949:
        tokinList = DBCS_Maekin_Korean;
        break;
    case 950:
        tokinList = DBCS_Maekin_TraditionalChinese;
        break;
    default:
        return false;
    }

    for (p = tokinList; *p; p += 2) {
        if (p[0] == ch && p[1] == uch) {
            return true;
        }
    }

    return false;
}

bool UIFont::DBCSIsAtokin(unsigned short uch)
{
    const unsigned char   *tokinList;
    const unsigned char   *p;
    unsigned char ch;

    if (uch < 0x100) {
        return false;
    }

    ch = (uch >> 8) & 0xFF;

    switch (m_font->codePage) {
    case 932:
        tokinList = DBCS_Tokin_Japanese;
        break;
    case 936:
    case 950:
        tokinList = (const unsigned char*)"";
        break;
    case 949:
        tokinList = DBCS_Tokin_Korean;
        break;
    default:
        return false;
    }

    for (p = tokinList; *p; p += 2) {
        if (p[0] == ch && p[1] == uch) {
            return true;
        }
    }

    return false;
}

int UIFont::DBCSGetWordBlockCount(const char *text, int maxlen)
{
    unsigned char *p;
    unsigned short uch, uch2;
    unsigned int   DBCSLen;
    bool           isATokin;
    bool           isControl;
    bool           hasDBCS;

    if (maxlen < 0) {
        maxlen = strlen(text);
    }

    if (maxlen == 0) {
        return 0;
    }

    uch = text[0];
    p   = (unsigned char *)(text + 1);

    if (!text[0]) {
        return uch;
    }

    if (DBCSIsLeadByte(uch)) {
        DBCSLen = 2;
        uch     = *p | (uch << 8);
        p++;
    } else {
        DBCSLen = 1;
    }

    DBCSIsMaekin(uch);
    isATokin  = DBCSIsAtokin(uch);
    hasDBCS   = DBCSLen > 1;
    isControl = uch < ' ';

    for (;;) {
        unsigned int subDBCSLen;
        bool         subIsATokin;

        uch = *p++;

        if (!uch || DBCSLen >= maxlen) {
            break;
        }

        if (DBCSIsLeadByte(uch)) {
            subDBCSLen = 2;
            uch2       = *p++ | (uch2 << 8);
        }

        subIsATokin = DBCSIsAtokin(uch);

        if (isControl || uch2 < ' ' || !DBCSIsMaekin(uch)) {
            break;
        }

        if (!isATokin && (hasDBCS || subDBCSLen > 1 || (uch == ' ' || uch == '-') && uch2 != ' ' && uch2 != '-')) {
            break;
        }

        isATokin  = subIsATokin;
        isControl = false;
        DBCSLen += subDBCSLen;
    }

    if (DBCSLen > maxlen) {
        DBCSLen = maxlen;
    }

    return DBCSLen;
}

UIFont::UIFont()
{
    m_font = uii.Rend_LoadFont("verdana-14");
    if (!m_font) {
        uii.Sys_Error(ERR_DROP, "Couldn't load font Verdana\n");
    }

    color = UBlack;
    setColor(color);
}

UIFont::UIFont(const char *fn)
{
    setFont(fn);
}

void UIFont::setColor(UColor col)
{
    color = col;
}

void UIFont::setAlpha(float alpha)
{
    color.a = alpha;
}

void UIFont::setFont(const char *fontname)
{
    m_font = uii.Rend_LoadFont(fontname);
    if (!m_font) {
        uii.Sys_Error(ERR_DROP, "Couldn't load font %s\n", fontname);
    }
}

void UIFont::Print(float x, float y, const char *text, size_t maxlen, qboolean bVirtualScreen)
{
    uii.Rend_SetColor(color);
    uii.Rend_DrawString(m_font, text, x, y, maxlen, bVirtualScreen);
}

void UIFont::PrintJustified(
    const UIRect2D& rect, fonthorzjustify_t horz, fontvertjustify_t vert, const char *text, float *vVirtualScale
)
{
    float       newx, newy;
    int         textwidth, textheight;
    UIRect2D    sizedRect;
    const char *source;
    char       *dest;
    char        string[2048];

    if (vVirtualScale) {
        sizedRect.pos.x       = rect.pos.x / vVirtualScale[0];
        sizedRect.pos.y       = rect.pos.y / vVirtualScale[1];
        sizedRect.size.width  = rect.size.width / vVirtualScale[0];
        sizedRect.size.height = rect.size.height / vVirtualScale[1];
    } else {
        sizedRect = rect;
    }

    if (horz == FONT_JUSTHORZ_LEFT && vert == FONT_JUSTVERT_TOP) {
        // no need to justify
        Print(sizedRect.pos.x, sizedRect.pos.y, text, -1, vVirtualScale != NULL);
        return;
    }

    textheight = getHeight(text, -1, qfalse);

    switch (vert) {
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
    while (*source) {
        // skip new lines
        while (*source == '\n') {
            source++;
        }

        if (!*source) {
            // don't print an empty string
            return;
        }

        dest = string;

        do {
            *dest++ = *source++;
        } while (*source && *source != '\n');

        *dest = 0;

        switch (horz) {
        case FONT_JUSTHORZ_CENTER:
            textwidth = getWidth(string, -1);
            newx      = sizedRect.pos.x + (sizedRect.size.width - textwidth) * 0.5;
            break;
        case FONT_JUSTHORZ_LEFT:
            newx = sizedRect.pos.x;
            break;
        case FONT_JUSTHORZ_RIGHT:
            textwidth = getWidth(string, -1);
            newx      = sizedRect.pos.x + sizedRect.size.width - textwidth;
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

void UIFont::PrintOutlinedJustified(
    const UIRect2D  & rect,
    fonthorzjustify_t horz,
    fontvertjustify_t vert,
    const char       *text,
    const UColor    & outlineColor,
    float            *vVirtualScale
)
{
    float       newx, newy;
    int         textwidth, textheight;
    UIRect2D    sizedRect;
    const char *source;
    char       *dest;
    char        string[2048];
    UColor      originalColor;
    bool        bVirtual;

    textwidth = 0;

    if (vVirtualScale) {
        sizedRect.pos.x       = rect.pos.x / vVirtualScale[0];
        sizedRect.pos.y       = rect.pos.y / vVirtualScale[1];
        sizedRect.size.width  = rect.size.width / vVirtualScale[0];
        sizedRect.size.height = rect.size.height / vVirtualScale[1];
    } else {
        sizedRect = rect;
    }

    if (horz == FONT_JUSTHORZ_LEFT && vert == FONT_JUSTVERT_TOP) {
        // no need to justify
        Print(sizedRect.pos.x, sizedRect.pos.y, text, -1, vVirtualScale != NULL);
        return;
    }

    textheight = getHeight(text, -1, qfalse);

    switch (vert) {
    case FONT_JUSTVERT_TOP:
        newy = sizedRect.pos.y + 2.0;
        break;
    case FONT_JUSTVERT_CENTER:
        newy = sizedRect.pos.y + (sizedRect.size.height - textheight) * 0.5;
        break;
    case FONT_JUSTVERT_BOTTOM:
        newy = sizedRect.pos.y + sizedRect.size.height - textheight - 2.0;
        break;
    default:
        newy = 0;
        break;
    }

    originalColor = color;
    bVirtual      = vVirtualScale != NULL;
    source        = text;
    while (*source) {
        // skip new lines
        while (*source == '\n') {
            source++;
        }

        if (!*source) {
            // don't print an empty string
            return;
        }

        dest = string;

        do {
            *dest++ = *source++;
        } while (*source && *source != '\n');

        *dest = 0;

        if (textwidth != FONT_JUSTHORZ_LEFT) {
            textwidth = getWidth(string, -1);
        }

        switch (horz) {
        case FONT_JUSTHORZ_CENTER:
            newx = sizedRect.pos.x + (sizedRect.size.width - textwidth) * 0.5;
            break;
        case FONT_JUSTHORZ_LEFT:
            newx = sizedRect.pos.x + 2.0;
            break;
        case FONT_JUSTHORZ_RIGHT:
            newx = sizedRect.pos.x + sizedRect.size.width - textwidth - 2.0;
            break;
        default:
            newx = 0;
            break;
        }

        //
        // draw the outline
        //
        setColor(outlineColor);
        Print(newx + 1, newy + 2, string, -1, bVirtual);
        Print(newx + 2, newy + 1, string, -1, bVirtual);
        Print(newx - 1, newy + 2, string, -1, bVirtual);
        Print(newx - 2, newy + 1, string, -1, bVirtual);
        Print(newx - 1, newy - 2, string, -1, bVirtual);
        Print(newx - 2, newy - 1, string, -1, bVirtual);
        Print(newx + 1, newy - 2, string, -1, bVirtual);
        Print(newx + 2, newy - 1, string, -1, bVirtual);
        Print(newx + 2, newy, string, -1, bVirtual);
        Print(newx - 2, newy, string, -1, bVirtual);
        Print(newx, newy + 2, string, -1, bVirtual);
        Print(newx, newy - 2, string, -1, bVirtual);
        //
        // draw the text
        //
        setColor(originalColor);
        Print(newx, newy, string, -1, bVirtual);

        // expand for newline
        newy += getHeight(" ", -1, qfalse);
    }
}

int UIFont::getMaxWidthIndex(const char *text, int maxlen)
{
    return UI_FontStringMaxWidth(m_font, text, maxlen);
}

int UIFont::getWidth(const char *text, int maxlen)
{
    return UI_FontStringWidth(m_font, text, maxlen);
}

int UIFont::getCharWidth(unsigned short ch)
{
    int index;
    int indirected;

    if (!m_font) {
        return 0;
    }

    if (ch == '\t') {
        ch = ' ';
    }

    if (m_font->numPages) {
        int code = CodeSearch(ch);
        if (code < 0) {
            return 0;
        }

        index      = m_font->charTable[code].index;
        indirected = m_font->charTable[code].loc;
    } else {
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

int UIFont::getHeight(const char *text, int maxlen, qboolean bVirtual)
{
    float height;
    int   i;

    if (!m_font) {
        return 0;
    }

    height = getHeight(bVirtual);

    for (i = 0; text[i]; i++) {
        if (maxlen != -1 && i > maxlen) {
            break;
        }

        if (text[i] == '\n') {
            height += getHeight(bVirtual);
        }
    }

    return height;
}

int UIFont::getHeight(qboolean bVirtual)
{
    if (bVirtual) {
        if (m_font) {
            return (m_font->sgl[0]->height * uid.vidHeight / 480.0);
        } else {
            return (16.0 * uid.vidHeight / 480.0);
        }
    } else {
        if (m_font) {
            return m_font->sgl[0]->height;
        } else {
            return 16;
        }
    }
}

qboolean UI_FontDBCSIsLeadByte(fontheader_t *font, unsigned short uch)
{
    // Byte ranges found in Wikipedia articles with relevant search strings in each case
    switch (font->codePage) {
    case 932:
        // Shift_jis
        return ((uch >= 0x81) && (uch <= 0x9F)) || ((uch >= 0xE0) && (uch <= 0xFC));
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
        return ((uch >= 0x84) && (uch <= 0xD3)) || ((uch >= 0xD8) && (uch <= 0xDE)) || ((uch >= 0xE0) && (uch <= 0xF9));
    }
    return false;
}

int UI_FontCodeSearch(const fontheader_t *font, unsigned short uch)
{
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

float UI_FontgetCharWidthf(fontheader_t *font, unsigned short uch)
{
    int index;
    int indirected;

    if (!font) {
        return 0.f;
    }

    if (uch == '\t') {
        uch = ' ';
    }

    if (font->numPages) {
        int code = UI_FontCodeSearch(font, uch);
        if (code < 0) {
            return 0.f;
        }

        index      = font->charTable[code].index;
        indirected = font->charTable[code].loc;
    } else {
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

int UI_FontStringMaxWidth(fontheader_t *pFont, const char *pszString, int iMaxLen)
{
    float widths    = 0.0;
    float maxwidths = iMaxLen / 256.0f;
    int   i;

    if (!pFont) {
        return 0;
    }

    i = 0;
    for (;;) {
        unsigned short uch = pszString[i++];

        if (uch == 0 || (iMaxLen != -1 && i > iMaxLen)) {
            break;
        }

        if (UI_FontDBCSIsLeadByte(pFont, uch)) {
            uch = (uch << 8) | pszString[i];
            if (!pszString[i]) {
                break;
            }

            i++;
        }

        if (uch == '\n') {
            widths = 0.0;
        } else {
            widths += UI_FontgetCharWidthf(pFont, uch);

            if (maxwidths < widths) {
                return i - 1;
            }
        }
    }

    return -1;
}

int UI_FontStringWidth(fontheader_t *pFont, const char *pszString, int iMaxLen)
{
    float widths    = 0.0;
    float maxwidths = 0.0;
    int   i;

    if (!pFont) {
        return 0;
    }

    i = 0;
    for (;;) {
        unsigned short uch = pszString[i++];

        if (uch == 0 || (iMaxLen != -1 && i > iMaxLen)) {
            break;
        }

        if (UI_FontDBCSIsLeadByte(pFont, uch)) {
            uch = (uch << 8) | pszString[i];
            if (!pszString[i]) {
                break;
            }

            i++;
        }

        if (uch == '\n') {
            widths = 0.0;
        } else {
            widths += UI_FontgetCharWidthf(pFont, uch);

            if (maxwidths < widths) {
                maxwidths = widths;
            }
        }
    }

    return maxwidths * 256.0;
}
