/*
===========================================================================
Copyright (C) 2015-2023 the OpenMoHAA team

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

CLASS_DECLARATION(UIWidget, UIField, NULL) {
    {&W_LeftMouseDown, &UIField::Pressed},
    {NULL,             NULL             }
};

UIField::UIField()
{
    AllowActivate(qtrue);
    m_iPreStep = 0;
}

void UIField::Draw(void)
{
    UpdateData();

    // the width of the widget that can be filled with valid text
    float fVirtualWidth = m_frame.size.width / getVirtualScale()[0] - m_font->getWidth(" ..", -1);
    float indentLength  = m_indent;
    float cursorSize    = m_font->getCharWidth('_');

    // The total pixel length of the entire string in the editfield buffer
    float totalDrawLength = m_font->getWidth(m_edit.m_buffer, -1) + indentLength;

    // The index of the character that has to be shown first
    // when viewing the end of the string to properly fill out the field
    int iMaxPreStep;

    float fEndLen = totalDrawLength + cursorSize;
    int   buflen  = strlen(m_edit.m_buffer);

    // Calculate the max prestep
    for (iMaxPreStep = 0; fEndLen > fVirtualWidth && iMaxPreStep < buflen; iMaxPreStep++) {
        float fCharWidth = m_font->getCharWidth(m_edit.m_buffer[iMaxPreStep]);
        fEndLen -= fCharWidth;
    } // fEndLen is discarded

    // Index of the last currently visible character in the textfield
    int iLastChar;

    fEndLen = indentLength + cursorSize;
    for (iLastChar = m_iPreStep; fEndLen < fVirtualWidth; iLastChar++) {
        if (char c = m_edit.m_buffer[iLastChar]) {
            float fCharWidth = m_font->getCharWidth(c);
            fEndLen += fCharWidth;
        } else {
            // important to quit early!
            // otherwise iLastChar would get incremented one extra time
            break;
        }
    }

    if (IsActive()) {
        if (m_edit.m_cursor >= m_iPreStep) {
            // Fixed in OPM
            // m_iPreStep < iLastChar check added to avoid passing negative size to Q_strncpyz and crashing the game
            while (iLastChar <= m_edit.m_cursor && m_iPreStep < iMaxPreStep
                   && m_iPreStep < iLastChar) { // is text too long to fit?
                fEndLen -= m_font->getCharWidth(m_edit.m_buffer[m_iPreStep]);
                m_iPreStep++;

                // FIXME: repetitive logic
                if (char c = m_edit.m_buffer[iLastChar]) {
                    float fCharWidth = m_font->getCharWidth(c);
                    fEndLen += fCharWidth;
                } else {
                    break;
                }

                while (fEndLen < fVirtualWidth) {
                    iLastChar++;

                    if (char c = m_edit.m_buffer[iLastChar]) {
                        float fCharWidth = m_font->getCharWidth(c);
                        fEndLen += fCharWidth;
                    } else {
                        break;
                    }
                }
            }
        } else {
            m_iPreStep = m_edit.m_cursor;
        }

        m_iPreStep = Q_min(m_iPreStep, iMaxPreStep);
    }

    // Put the portion of the text intended for display into the temptext buffer
    char temptext[EDITFIELD_BUFSIZE];
    Q_strncpyz(temptext, &m_edit.m_buffer[m_iPreStep], iLastChar - m_iPreStep + 1);

    // Calculate cursor position
    float cursorPos = indentLength - 1.0f;

    // Added in OPM
    // new i < EDITFIELD_BUFSIZE check for extra safety
    for (int i = m_iPreStep; i < m_edit.m_cursor && i < EDITFIELD_BUFSIZE; i++) {
        float fCharWidth = m_font->getCharWidth(m_edit.m_buffer[i]);
        cursorPos += fCharWidth;
    }

    cursorPos = Q_min(cursorPos, fVirtualWidth);

    // Calculate text height
    float y = m_frame.size.height / getVirtualScale()[1] - m_font->getHeight();
    if (m_bottomindent * 2 <= y) // upper and lower margin
    {
        y -= m_bottomindent;
    } else {
        y /= 2;
    }

    // Show text
    m_font->setColor(m_foreground_color);
    m_font->Print(m_indent, y, temptext, -1, getVirtualScale());

    // Display blinking cursor if field is in focus
    if (IsActive()) {
        const char *cursorChar = (uid.time / 250) & 1 ? "|" : "_";
        m_font->Print(cursorPos, y, cursorChar, -1, getVirtualScale());
    }

    Q_strncpyz(temptext, "..", EDITFIELD_BUFSIZE);

    if (m_iPreStep) {
        // Display leading dots if not starting from the beginning
        m_font->Print(2.0f, y, temptext, -1, getVirtualScale());
    }

    // Added in OPM
    // new iLastChar < EDITFIELD_BUFSIZE check for extra safety
    if (iLastChar < EDITFIELD_BUFSIZE && m_edit.m_buffer[iLastChar]) {
        // Display trailing dots if not ending at the end
        fVirtualWidth = m_frame.size.width / getVirtualScale()[0] - m_font->getWidth(temptext, -1) - 2.0f;
        m_font->Print(fVirtualWidth, y, temptext, -1, getVirtualScale());
    }
}

qboolean UIField::KeyEvent(int key, unsigned int time)
{
    if (key == 'l' && uii.Sys_IsKeyDown(K_CTRL)) {
        return qtrue;
    }

    if (key == K_ENTER || key == K_KP_ENTER) {
        if (m_cvarname.length()) {
            uii.Cvar_Set(m_cvarname, m_edit.m_buffer);
        }

        if (m_commandhandler) {
            m_commandhandler(str(m_edit.m_buffer) + '\n', NULL);
        }

        if (m_command.length()) {
            str result = m_command + ' ' + m_edit.m_buffer + '\n';
            Cbuf_AddText(result);
        }
        return qtrue;
    }

    if ((key == K_INS || key == K_KP_INS) && uii.Sys_IsKeyDown(K_SHIFT)) {
        return qtrue;
    }

    int len = strlen(m_edit.m_buffer);
    switch (key) {
    case K_DEL: // DELETE key, backspace is handled in CharEvent
        if (m_edit.m_cursor < len) {
            memmove(&m_edit.m_buffer[m_edit.m_cursor], &m_edit.m_buffer[m_edit.m_cursor + 1], len - m_edit.m_cursor);

            if (m_cvarname.length()) {
                uii.Cvar_Set(m_cvarname, m_edit.m_buffer);
            }
        }
        return qtrue;
    case K_RIGHTARROW:
        if (m_edit.m_cursor < len) {
            m_edit.m_cursor++;
        }
        return qtrue;
    case K_LEFTARROW:
        if (m_edit.m_cursor > 0) {
            m_edit.m_cursor--;
        }
        return qtrue;
    }

    // HOME or Ctrl + A
    if (key == K_HOME || (tolower(key) == 'a' && uii.Sys_IsKeyDown(K_CTRL))) {
        m_edit.m_cursor = 0;
        return qtrue;
    }

    // END or Ctrl + E
    if (key == K_END || (tolower(key) == 'e' && uii.Sys_IsKeyDown(K_CTRL))) {
        m_edit.m_cursor = len;
        return qtrue;
    }

    return key == K_INS;
}

void UIField::CharEvent(int ch)
{
    uii.Snd_PlaySound("sound/menu/typekey.wav");

    if (ch == 3 || ch == 22) // Ctrl + C (^C) or Ctrl + V (^V)
    {
        return;
    }

    int len = strlen(m_edit.m_buffer);
    switch (ch) {
    case '\b': // backspace or Ctrl + H (^H) - DEL is handled in KeyEvent
        if (m_edit.m_cursor <= 0) {
            // already at beginning of field
            return;
        }

        // Move all text after the cursor one position back
        memmove(&m_edit.m_buffer[m_edit.m_cursor - 1], &m_edit.m_buffer[m_edit.m_cursor], len + 1 - m_edit.m_cursor);

        // move back cursor by one
        m_edit.m_cursor--;

        if (m_cvarname.length()) {
            uii.Cvar_Set(m_cvarname, m_edit.m_buffer);
        }

        return;
    case 1: // Home (^A)
        m_edit.m_cursor = 0;
        return;
    case 5: // End (^E)
        m_edit.m_cursor = len;
        return;
    }

    // original check only does ch < 32,
    // this one excludes DEL too but it shouldn't be a problem,
    // DEL is handled in KeyEvent
    if (iscntrl(ch)) {
        return;
    }

    len++; // new printable character was typed

    // Fixed in OPM
    // originally this was len == EDITFIELD_BUFSIZE, just being extra safe
    if (len >= EDITFIELD_BUFSIZE) // is it out of bounds?
    {
        return;
    }

    // make room for the character in the buffer
    memmove(&m_edit.m_buffer[m_edit.m_cursor + 1], &m_edit.m_buffer[m_edit.m_cursor], len - m_edit.m_cursor);

    // insert the new character at the cursor's location
    m_edit.m_buffer[m_edit.m_cursor] = ch;

    // advance the cursor
    m_edit.m_cursor++;

    // if the character was inserted at the end of the string,
    // make sure it remains NULL-terminated
    if (m_edit.m_cursor == len) {
        m_edit.m_buffer[m_edit.m_cursor] = 0;
    }

    if (m_cvarname.length()) {
        uii.Cvar_Set(m_cvarname, m_edit.m_buffer);
    }
}

void UIField::UpdateData(void)
{
    if (m_cvarname.length()) {
        str::snprintf(m_edit.m_buffer, EDITFIELD_BUFSIZE, "%s", CvarGetForUI(m_cvarname, ""));
        m_edit.m_cursor = Q_min(m_edit.m_cursor, strlen(m_edit.m_buffer));
    }
}

void UIField::Pressed(Event *ev)
{
    float xpos  = ev->GetFloat(1);
    float point = (xpos - m_frame.pos.x) / getVirtualScale()[0];
    point       = Q_max(point, 0.0f); // added in 2.15

    int   iStep        = m_iPreStep;
    float fStringWidth = m_indent;
    while (char c = m_edit.m_buffer[iStep]) {
        iStep++;
        fStringWidth += m_font->getCharWidth(c);
        if (point <= fStringWidth) {
            if (iStep > 0) {
                iStep--; // put the cursor BEFORE the clicked character
            }
            break;
        }
    }

    m_edit.m_cursor = iStep;
}
