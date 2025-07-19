/*
===========================================================================
Copyright (C) 2015-2024 the OpenMoHAA team

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
#include "../qcommon/localization.h"

Event EV_Time_Dragged
(
    "_umledit_drag_timer",
    EV_DEFAULT,
    NULL,
    NULL,
    "Used to time mouse drag events"
);
Event EV_UIMultiLineEdit_UpdateCvarEvent
(
    "updatecvar",
    EV_DEFAULT,
    NULL,
    NULL,
    "Causes text box to update itself using the assigned cvar"
);
Event EV_UIMultiLineEdit_SetEdit
(
    "edit",
    EV_DEFAULT,
    "b",
    "bool",
    "Set whether or not the text is editable"
);

CLASS_DECLARATION(UIWidget, UIMultiLineEdit, NULL) {
    {&W_LeftMouseDown,                    &UIMultiLineEdit::MouseDown      },
    {&W_LeftMouseUp,                      &UIMultiLineEdit::MouseUp        },
    {&W_LeftMouseDragged,                 &UIMultiLineEdit::MouseDragged   },
    {&EV_Time_Dragged,                    &UIMultiLineEdit::DragTimer      },
    {&W_SizeChanged,                      &UIMultiLineEdit::SizeChanged    },
    {&EV_UIMultiLineEdit_UpdateCvarEvent, &UIMultiLineEdit::UpdateCvarEvent},
    {&EV_UIMultiLineEdit_SetEdit,         &UIMultiLineEdit::SetEdit        },
    {NULL,                                NULL                             }
};

UIMultiLineEdit::UIMultiLineEdit()
{
    m_vertscroll      = NULL;
    m_mouseState      = M_NONE;
    m_shiftForcedDown = 0;
    m_edit            = true;
    Empty();
}

void UIMultiLineEdit::Empty(void)
{
    m_lines.RemoveAllItems();
    m_lines.AddTail(str());

    m_selection.begin = {};
    m_selection.end   = {};

    if (m_vertscroll) {
        m_vertscroll->setNumItems(1);
        m_vertscroll->setTopItem(0);
    }

    m_changed = false;
}

void UIMultiLineEdit::SetEdit(Event *ev)
{
    m_edit = ev->GetBoolean(1);
}

void UIMultiLineEdit::setData(const char *data)
{
    str         toadd;
    char        s[2] {};
    const char *p;

    m_lines.RemoveAllItems();

    for (p = data; *p; p++) {
        if (*p == '\n') {
            m_lines.AddTail(toadd);
            toadd = "";
        } else if (*p == '\r') {
            continue;
        } else {
            s[0] = *p;
            toadd += s;
        }
    }

    m_lines.AddTail(toadd);
    m_selection.begin = {};
    m_selection.end   = {};

    if (m_vertscroll) {
        m_vertscroll->setNumItems(m_lines.getCount());
        m_vertscroll->setTopItem(0);
    }

    m_changed = false;
}

void UIMultiLineEdit::getData(str& data)
{
    int i;

    data = "";

    for (i = 0, m_lines.IterateFromHead(); m_lines.IsCurrentValid(); i++, m_lines.IterateNext()) {
        str text = m_lines.getCurrent();
        if (i == 0 && !text.length() && m_lines.getCount() <= 1) {
            // file is empty
            return;
        }

        data += text;

        if (i != m_lines.getCount() - 1) {
            data += "\n";
        }
    }
}

void UIMultiLineEdit::FrameInitialized(void)
{
    delete m_vertscroll;
    m_vertscroll = new UIVertScroll();
    m_vertscroll->setPageHeight(m_frame.size.height / m_font->getHeight(getHighResScale()));
    m_vertscroll->setTopItem(0);
    m_vertscroll->setNumItems(m_lines.getCount());
    m_vertscroll->InitFrameAlignRight(this, 0, 0);

    Connect(this, W_SizeChanged, W_SizeChanged);
    AllowActivate(true);
}

void UIMultiLineEdit::SizeChanged(Event *ev)
{
    m_vertscroll->setPageHeight(m_frame.size.height / m_font->getHeight(getVirtualScale()));
    m_vertscroll->InitFrameAlignRight(this, 0, 0);
}

void UIMultiLineEdit::SortSelection(selectionpoint_t **topsel, selectionpoint_t **botsel)
{
    if (m_selection.begin.line < m_selection.end.line) {
        *topsel = &m_selection.begin;
        *botsel = &m_selection.end;
    } else if (m_selection.begin.line <= m_selection.end.line && m_selection.begin.column <= m_selection.end.column) {
        *topsel = &m_selection.begin;
        *botsel = &m_selection.end;
    } else {
        *topsel = &m_selection.end;
        *botsel = &m_selection.begin;
    }
}

void UIMultiLineEdit::UpdateCvarEvent(Event *ev)
{
    if (m_cvarname.length()) {
        const char *s = uii.Cvar_GetString(m_cvarname, "");
        if (s) {
            setData(s);
        }
    }
}

void UIMultiLineEdit::Draw(void)
{
    float             aty;
    int               i;
    UColor            selectionColor(UWhite);
    UColor            selectionBG(0, 0, 0.5f, 1);
    selectionpoint_t *topsel = NULL;
    selectionpoint_t *botsel = NULL;

    SortSelection(&topsel, &botsel);

    m_lines.IterateFromHead();
    for (i = 0; i < m_vertscroll->getTopItem(); i++) {
        m_lines.IterateNext();
    }

    for (i = m_vertscroll->getTopItem(), aty = 0; m_lines.IsCurrentValid() && aty < m_frame.size.height;
         m_lines.IterateNext(), i++) {
        str& cur   = m_lines.getCurrent();
        int  caret = 0;

        if (i < topsel->line || i > botsel->line) {
            // Print regular line without any selection or cursor present
            m_font->setColor(m_foreground_color);
            m_font->Print(0, aty / getVirtualScale()[1], cur, -1, getVirtualScale());
        } else {
            // Current line contains cursor and/or selected text
            float linewidth = m_font->getWidth(cur, -1);

            if (i > topsel->line && i < botsel->line) {
                // all text in current line is selected, it's part of a larger selection,
                // print entire line with the selection highlight box around it
                DrawBox(
                    0.0f, aty, linewidth * getVirtualScale()[0], m_font->getHeight(getVirtualScale()), selectionBG, 1.f
                );
                m_font->setColor(selectionColor);
                // Fixed in OPM:
                // don't spam LOCALIZATION ERROR messages to console
                // for clicking lines in the opened text document
                //m_font->Print(0, aty, Sys_LV_CL_ConvertString(cur), -1, m_bVirtual);
                m_font->Print(0, aty / getVirtualScale()[1], cur, -1, getVirtualScale());
            } else if (i != topsel->line) {
                // part of this line is selected, and selection continues/began above
                if (i == botsel->line) {                                // sanity check, should always be true
                    int toplen = m_font->getWidth(cur, botsel->column); // X coord of highlighting end
                    if (toplen) {
                        // selection contains text from the beginning of the line,
                        // print it with the selection highlight box around it
                        m_font->setColor(selectionColor);
                        DrawBox(
                            0,
                            aty,
                            toplen * getVirtualScale()[0],
                            m_font->getHeight(getVirtualScale()),
                            selectionBG,
                            1.f
                        );
                        m_font->Print(0, aty / getVirtualScale()[1], cur, botsel->column, getVirtualScale());
                    }

                    if (toplen < linewidth) { // is there still text on this line after the selection?
                        m_font->setColor(m_foreground_color);
                        m_font->Print(toplen, aty / getVirtualScale()[1], &cur[botsel->column], -1, getVirtualScale());
                    }

                    if (botsel == &m_selection.end) {
                        // Place cursor at the end of the selection if it was started from above
                        caret = toplen;
                    }
                }
            } else if (i != botsel->line) {
                // part of this line is selected, and selection continues/began below
                int toplen = m_font->getWidth(cur, topsel->column); // X coord of highlighting start
                if (topsel->column) { // is there any text on this line before the selection?
                    m_font->setColor(m_foreground_color);
                    m_font->Print(0, aty / getVirtualScale()[1], cur, topsel->column, getVirtualScale());
                }

                if (toplen < linewidth) { // is there any selected text before the end of the line?
                    // print the selected text with the selection highlight box around it
                    m_font->setColor(selectionColor);
                    DrawBox(
                        toplen * getVirtualScale()[0],
                        aty,
                        (linewidth - toplen) * getVirtualScale()[0],
                        m_font->getHeight(getVirtualScale()),
                        selectionBG,
                        1.f
                    );
                    m_font->Print(toplen, aty / getVirtualScale()[1], &cur[topsel->column], -1, getVirtualScale());
                }

                if (topsel == &m_selection.end) {
                    // Place cursor before the highlight box if selection was made from the bottom up
                    caret = toplen;
                }
            } else {
                // current line contains the cursor
                if (topsel->column == botsel->column) {
                    // no selection or highlighted text
                    caret = m_font->getWidth(cur, topsel->column);
                    m_font->setColor(m_foreground_color);
                    // Fixed in OPM:
                    // don't spam LOCALIZATION ERROR messages to console
                    // for clicking lines in the opened text document
                    //m_font->Print(0, aty, Sys_LV_CL_ConvertString(cur), -1, m_bVirtual);
                    m_font->Print(0, aty / getVirtualScale()[1], cur, -1, getVirtualScale());
                } else {
                    // selection starts and ends on this line
                    int toplen = m_font->getWidth(cur, topsel->column); // X coord of highlighting start
                    int botlen = toplen
                               + m_font->getWidth(
                                   &cur[topsel->column], botsel->column - topsel->column
                               ); // X coord of selection end

                    if (toplen) { // is there any text on this line before the selection?
                        m_font->setColor(m_foreground_color);
                        m_font->Print(0, aty / getVirtualScale()[1], cur, topsel->column, getVirtualScale());
                    }

                    if (botlen != toplen) { // is the selection wider than 0 pixels? (sanity check, always true)
                        // print the selected text with the selection highlight box around it
                        DrawBox(
                            toplen * getVirtualScale()[0],
                            aty,
                            (botlen - toplen) * getVirtualScale()[0],
                            m_font->getHeight(getVirtualScale()),
                            selectionBG,
                            1.f
                        );
                        m_font->setColor(selectionColor);
                        m_font->Print(
                            toplen,
                            aty / getVirtualScale()[1],
                            &cur[topsel->column],
                            botsel->column - topsel->column,
                            getVirtualScale()
                        );
                    }

                    if (cur.length() != botsel->column) { // is there still text on this line after the selection?
                        // print the leftover text
                        m_font->setColor(m_foreground_color);

                        // Fixed in OPM:
                        // highlighting text made the rest of the text after the selection on the line disappear.
                        // Cause: the last two arguments were incorrectly passed in originally,
                        // always specifying maxlen as m_bVirtual (which is usually zero).
                        // m_font->Print(botlen, aty, &cur[botsel->column], m_bVirtual, false);
                        m_font->Print(botlen, aty / getVirtualScale()[1], &cur[botsel->column], -1, getVirtualScale());
                    }

                    // Place the cursor at the end of the selection...
                    caret = botlen;
                    if (topsel == &m_selection.end) {
                        // ... except if selection was made from the bottom up
                        caret = toplen;
                    }
                }

                m_font->getWidth(cur, botsel->column);
                m_font->getWidth(cur, topsel->column);
            }
        }

        if (m_selection.end.line == i && (uid.time % 750) >= 375 && IsActive()) {
            // draw cursor caret
            DrawBox(caret * getVirtualScale()[0], aty, 2.f, m_font->getHeight(getVirtualScale()), UBlack, 1.f);
        }

        aty += m_font->getHeight(getVirtualScale());
    }
}

// num is the ZERO-BASED index of the sought line!
str& UIMultiLineEdit::LineFromLineNumber(int num, bool resetpos)
{
    static str emptyLine;
    void      *pos;
    int        i;

    pos = m_lines.getPosition();

    for (i = 0, m_lines.IterateFromHead(); i < num && m_lines.IsCurrentValid(); i++) {
        m_lines.IterateNext();
    }

    if (m_lines.IsCurrentValid()) {
        str& ret = m_lines.getCurrent();
        if (resetpos) {
            m_lines.setPosition(pos);
        }

        return ret;
    }

    if (resetpos) {
        m_lines.setPosition(pos);
    }

    return emptyLine;
}

void UIMultiLineEdit::PointToSelectionPoint(const UIPoint2D& p, selectionpoint_t& sel)
{
    int   clickedLine;
    int   i;
    float totalWidth = 0;
    float lastWidth  = 0;

    clickedLine = m_vertscroll->getTopItem() + p.y / m_font->getHeight(getVirtualScale());
    clickedLine = Q_min(clickedLine, m_lines.getCount() - 1);

    if (clickedLine < 0) {
        sel.line   = 0;
        sel.column = 0;
        return;
    }

    const char *line = LineFromLineNumber(clickedLine, true).c_str();
    for (i = 0; line[i] && totalWidth < p.x; i++) {
        lastWidth = m_font->getCharWidth(line[i]) * getVirtualScale()[0];
        totalWidth += lastWidth;
    }

    if (line[i] && i) {
        lastWidth *= 0.5f;
        if (totalWidth - lastWidth >= p.x) {
            i--;
        }
    }

    sel.line   = clickedLine;
    sel.column = i;
}

void UIMultiLineEdit::MouseDown(Event *ev)
{
    PointToSelectionPoint(MouseEventToClientPoint(ev), m_selection.begin);
    m_selection.end = m_selection.begin;
    EnsureSelectionPointVisible(m_selection.end);
    m_mouseState = M_DRAGGING;
    uWinMan.setFirstResponder(this);
}

void UIMultiLineEdit::MouseUp(Event *ev)
{
    CancelEventsOfType(EV_Time_Dragged);

    if (uWinMan.getFirstResponder() == this) {
        uWinMan.setFirstResponder(NULL);
    }

    if (m_mouseState == M_DRAGGING) {
        EnsureSelectionPointVisible(m_selection.end);
    }

    m_mouseState = M_NONE;
}

void UIMultiLineEdit::MouseDragged(Event *ev)
{
    m_dragState.lastPos = MouseEventToClientPoint(ev);
    PointToSelectionPoint(m_dragState.lastPos, m_selection.end);

    if (!EventPending(EV_Time_Dragged)) {
        PostEvent(new Event(EV_Time_Dragged), 0);
    }
}

void UIMultiLineEdit::DragTimer(Event *ev)
{
    int oldtop;

    PointToSelectionPoint(m_dragState.lastPos, m_selection.end);

    oldtop = m_vertscroll->getTopItem();
    EnsureSelectionPointVisible(m_selection.end);

    if (m_vertscroll->getTopItem() != oldtop) {
        PostEvent(new Event(EV_Time_Dragged), 0.25f);
    }
}

void UIMultiLineEdit::EnsureSelectionPointVisible(selectionpoint_t& point)
{
    int newtop;

    if (point.line > m_vertscroll->getTopItem()
        && point.line < m_vertscroll->getPageHeight() + m_vertscroll->getTopItem()) {
        return;
    }

    if (m_vertscroll->getTopItem() > point.line) {
        newtop = point.line;
    } else {
        newtop = point.line - m_vertscroll->getPageHeight() + 1;
    }

    if (newtop < 0) {
        newtop = 0;
    }

    m_vertscroll->setTopItem(newtop);
}

void UIMultiLineEdit::BoundSelectionPoint(selectionpoint_t& point)
{
    // since LineFromLineNumber expects a zero-based line index,
    // clamp it to one less than the number of lines if the selection point
    // is right at the end of the text document
    point.line = Q_clamp_int(point.line, 0, m_lines.getCount() - 1);

    str& line    = LineFromLineNumber(point.line, true);
    point.column = Q_clamp_int(point.column, 0, line.length());
}

qboolean UIMultiLineEdit::KeyEvent(int key, unsigned int time)
{
    bool     caret_moved = false;
    qboolean key_rec     = false;

    switch (key) {
    case K_UPARROW:
        m_selection.end.line--;
        caret_moved = true;
        key_rec     = true;
        break;
    case K_DOWNARROW:
        m_selection.end.line++;
        caret_moved = true;
        key_rec     = true;
        break;
    case K_LEFTARROW:
        m_selection.end.column--;
        if (m_selection.end.column < 0) {
            m_selection.end.column = 999999;
            m_selection.end.line--;
        }
        if (m_selection.end.line < 0) {
            m_selection.end.column = 0;
        }

        caret_moved = true;
        key_rec     = true;
        break;
    case K_RIGHTARROW:
        {
            int oldcol = m_selection.end.column;

            m_selection.end.column++;
            BoundSelectionPoint(m_selection.end);
            if (m_selection.end.column == oldcol) {
                m_selection.end.column = 0;
                m_selection.end.line++;
                if (m_selection.end.line >= m_lines.getCount()) {
                    m_selection.end.line--;
                    m_selection.end.column = 999999;
                }
            }

            caret_moved = true;
            key_rec     = true;
            break;
        }
    case K_INS:
        if (!IsSelectionEmpty()) {
            if (uii.Sys_IsKeyDown(K_CTRL)) {
                CopySelection();
            } else if (uii.Sys_IsKeyDown(K_SHIFT)) {
                PasteSelection();
            }
        }
        break;
    case K_DEL:
        // Fixed in OPM:
        // DEL key needed to be pressed twice to delete a single character,
        // as the first hit would only create an "internal" selection,
        // but wouldn't actually delete the character after the cursor.
        // The DeleteSelection() method should ALWAYS be called,
        // not only when there's an active selection.
        if (IsSelectionEmpty()) {
            m_shiftForcedDown = true;
            KeyEvent(K_RIGHTARROW, 0);
            m_shiftForcedDown = false;
        } else if (uii.Sys_IsKeyDown(K_SHIFT)) {
            CopySelection();
        }

        DeleteSelection();
        break;
    case K_PGDN:
        if (m_selection.end.line == (m_vertscroll->getTopItem() + m_vertscroll->getPageHeight() - 1)) {
            m_selection.end.line = m_vertscroll->getPageHeight() + m_selection.end.line;
        } else {
            m_selection.end.line = m_vertscroll->getTopItem() + m_vertscroll->getPageHeight() - 1;
        }

        caret_moved = true;
        key_rec     = true;
        break;
    case K_PGUP:
        if (m_selection.end.line == m_vertscroll->getTopItem()) {
            m_selection.end.line -= m_vertscroll->getPageHeight();
        } else {
            m_selection.end.line = m_vertscroll->getTopItem();
        }

        caret_moved = true;
        key_rec     = true;
        break;
    case K_HOME:
        if (uii.Sys_IsKeyDown(K_CTRL)) {
            m_selection.end.column = 0;
            m_selection.end.line   = 0;
        } else {
            m_selection.end.column = 0;
        }

        caret_moved = true;
        key_rec     = true;
        break;
    case K_END:
        if (uii.Sys_IsKeyDown(K_CTRL)) {
            m_selection.end.column = 999999;
            m_selection.end.line   = m_lines.getCount();
        } else {
            m_selection.end.column = 999999;
        }

        caret_moved = true;
        key_rec     = true;
        break;
    case K_MWHEELDOWN:
        m_vertscroll->AttemptScrollTo(m_vertscroll->getTopItem() + 2);
        key_rec = true;
        break;
    case K_MWHEELUP:
        m_vertscroll->AttemptScrollTo(m_vertscroll->getTopItem() - 2);
        key_rec = true;
        break;
    }

    if (caret_moved) {
        BoundSelectionPoint(m_selection.end);
        if (!uii.Sys_IsKeyDown(K_SHIFT) && !m_shiftForcedDown) {
            m_selection.begin.line   = m_selection.end.line;
            m_selection.begin.column = m_selection.end.column;
        }
        EnsureSelectionPointVisible(m_selection.end);
    }

    return key_rec;
}

void UIMultiLineEdit::DeleteSelection(void)
{
    selectionpoint_t *topsel;
    selectionpoint_t *botsel;
    int               i;

    if (IsSelectionEmpty()) {
        return;
    }

    m_changed = true;
    SortSelection(&topsel, &botsel);

    if (topsel->line == botsel->line) {
        str&     line   = LineFromLineNumber(topsel->line, true);
        intptr_t newlen = line.length() - (botsel->column - topsel->column);

        for (i = topsel->column; i < newlen; i++) {
            line[i] = line[i + botsel->column - topsel->column];
        }

        line.CapLength(newlen);
        *botsel = *topsel;
        EnsureSelectionPointVisible(*topsel);
        return;
    } else if (botsel->line - topsel->line > 1) {
        for (i = 0, m_lines.IterateFromHead(); m_lines.IsCurrentValid() && i < botsel->line;
             i++, m_lines.IterateNext()) {
            if (i > topsel->line) {
                m_lines.RemoveCurrentSetPrev();
                i--;
                botsel->line--;
            }
        }
    }

    for (i = 0, m_lines.IterateFromHead(); m_lines.IsCurrentValid() && i < topsel->line; i++, m_lines.IterateNext()) {}

    // delete topmost line of the selection, but only up to topsel->column
    str& topline = m_lines.getCurrent();
    if (topline.length() > topsel->column) {
        topline.CapLength(topsel->column);
    }
    m_lines.IterateNext();
    str line = m_lines.getCurrent();

    // merge remainder of topmost line with the remainder after the end of selection
    if (line.length() > botsel->column) {
        topline += &line[botsel->column];
    }

    m_lines.RemoveCurrentSetPrev();
    *botsel = *topsel;
    m_vertscroll->setNumItems(m_lines.getCount());
    EnsureSelectionPointVisible(*topsel);
}

void UIMultiLineEdit::CharEvent(int ch)
{
    intptr_t i;

    if (!m_edit) {
        return;
    }

    if (ch >= ' ' || ch == '\t') {
        DeleteSelection();
        m_changed = true;

        str& line = LineFromLineNumber(m_selection.begin.line, true);
        line += " ";

        for (i = line.length() - 1; i > m_selection.begin.column; i--) {
            line[i] = line[i - 1];
        }
        line[m_selection.begin.column] = ch;
        m_selection.begin.column++;
        m_selection.end.column++;
    } else if (ch == '\b') {
        if (IsSelectionEmpty()) {
            m_shiftForcedDown = true;
            KeyEvent(K_LEFTARROW, 0);
            m_shiftForcedDown = false;
        }
        DeleteSelection();
    } else if (ch == '\r') {
        DeleteSelection();
        m_changed = true;

        str& line      = LineFromLineNumber(m_selection.begin.line, false);
        str  otherline = "";
        if (line.length() > m_selection.begin.column) {
            otherline = &line[m_selection.begin.column];
            line.CapLength(m_selection.begin.column);
        }

        if (m_lines.IsCurrentValid()) {
            m_lines.InsertAfterCurrent(otherline);
        } else {
            m_lines.AddTail(otherline);
        }

        m_selection.begin.column = 0;
        m_selection.begin.line++;
        m_selection.end.column = m_selection.begin.column;
        m_selection.end.line   = m_selection.begin.line;
        m_vertscroll->setNumItems(m_lines.getCount());

        EnsureSelectionPointVisible(m_selection.end);
    }
}

bool UIMultiLineEdit::IsSelectionEmpty(void)
{
    return m_selection.begin.column == m_selection.end.column && m_selection.begin.line == m_selection.end.line;
}

void UIMultiLineEdit::CopySelection(void)
{
    selectionpoint_t *topsel;
    selectionpoint_t *botsel;
    str               line;
    str               clipText;

    if (IsSelectionEmpty()) {
        return;
    }

    SortSelection(&topsel, &botsel);

    line = LineFromLineNumber(topsel->line, true);

    if (line.length() > topsel->column) {
        clipText += &line[topsel->column];
    }

    if (topsel->line == botsel->line) {
        clipText.CapLength(botsel->column - topsel->column);
    } else {
        for (int i = topsel->line + 1; i < botsel->line; ++i) {
            clipText += "\n" + LineFromLineNumber(i, 1);
        }

        line = LineFromLineNumber(botsel->line, true);
        if (line.length() > botsel->column) {
            line.CapLength(botsel->column);
        }
        clipText += "\n" + line;
    }

    uii.Sys_SetClipboard(clipText);
}

void UIMultiLineEdit::PasteSelection(void)
{
    str sel;
    int i;

    // variable added in OPM as str cannot handle NULL assignment
    // we can get NULL here if clipboard is empty/couldn't be retrieved
    const char *clipboardData = uii.Sys_GetClipboard();
    if (clipboardData == NULL) {
        return;
    }

    sel = clipboardData;

    DeleteSelection();

    for (i = 0; i < sel.length(); i++) {
        if (sel[i] == '\n') {
            CharEvent('\r');
        } else if (sel[i] == '\r') {
            // Changed in OPM:
            // NOP, drop CR characters.
            // On Linux/Mac they aren't present anyway,
            // on Windows we already have LF chars next to them.
            // The filtering for CR on the Windows side was originally done
            // in Sys_GetWholeClipboard with a "manual" selective strcpy,
            // but here we iterate over all characters of the clipboard anyways,
            // so this feels like a better place to do the filtering.
        } else {
            CharEvent(sel[i]); // FIXME: this is VERY slow and jams up the EventQueue!
        }
    }
}

UIPoint2D UIMultiLineEdit::getEndSelPoint(void)
{
    return UIPoint2D(m_selection.end.column, m_selection.end.line);
}

void UIMultiLineEdit::setChanged(bool b)
{
    m_changed = b;
}

bool UIMultiLineEdit::IsChanged(void)
{
    return m_changed;
}

void UIMultiLineEdit::Scroll(Event *ev) {}
