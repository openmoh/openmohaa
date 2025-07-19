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

Event EV_Notepad_SaveAs
(
    "_notepad_saveas",
    EV_DEFAULT,
    NULL,
    NULL,
    "Event sent when user selected Save As..."
);

Event EV_Notepad_Save
(
    "_notepad_save",
    EV_DEFAULT,
    NULL,
    NULL,
    "Event sent when user selected Save"
);

Event EV_Notepad_Open
(
    "_notepad_open",
    EV_DEFAULT,
    NULL,
    NULL,
    "Event sent when user selected Open"
);

Event EV_Notepad_OpenFile
(
    "openfile",
    EV_DEFAULT,
    "s",
    "nameOfFile",
    "called to open a file in the notepad"
);

Event EV_Notepad_Find
(
    "_notepad_find",
    EV_DEFAULT,
    NULL,
    NULL,
    "Event sent when user selected Find"
);

Event EV_Notepad_Goto
(
    "_notepad_goto",
    EV_DEFAULT,
    NULL,
    NULL,
    "Event sent when user selected Go to line"
);

Event EV_Notepad_Copy
(
    "_notepad_copy",
    EV_DEFAULT,
    NULL,
    NULL,
    "Event sent when user selected Copy"
);

Event EV_Notepad_Cut
(
    "_notepad_cut",
    EV_DEFAULT,
    NULL,
    NULL,
    "Event sent when user selected Cut"
);

Event EV_Notepad_Paste
(
    "_notepad_paste",
    EV_DEFAULT,
    NULL,
    NULL,
    "Event sent when user selected Paste"
);

Event W_Notepad_ChildSizeChanged
(
    "_notepad_childsizechanged",
    EV_DEFAULT,
    NULL,
    NULL,
    "Signal that the child area of the floating window has changed"
);

CLASS_DECLARATION(UIMultiLineEdit, UINotepadEdit, NULL) {
    {&W_LeftMouseDown, &UINotepadEdit::MousePressed},
    {NULL,             NULL                        }
};

CLASS_DECLARATION(UIFloatingWindow, UINotepad, NULL) {
    {&W_Notepad_ChildSizeChanged,       &UINotepad::ChildSizeChanged},
    {&EV_Notepad_SaveAs,                &UINotepad::SaveAs          },
    {&EV_Notepad_Save,                  &UINotepad::Save            },
    {&EV_Notepad_Open,                  &UINotepad::Open            },
    {&EV_Notepad_OpenFile,              &UINotepad::OpenFile        },
    {&UIFloatingWindow::W_ClosePressed, &UINotepad::ClosePressed    },
    {&EV_Notepad_Find,                  &UINotepad::OnFind          },
    {&EV_Notepad_Goto,                  &UINotepad::OnGoto          },
    {&EV_Notepad_Copy,                  &UINotepad::OnCopy          },
    {&EV_Notepad_Paste,                 &UINotepad::OnPaste         },
    {&EV_Notepad_Cut,                   &UINotepad::OnCut           },
    {NULL,                              NULL                        }
};

#define CTRL_EVENT_COUNT 10
static ctrlevent_s controlEvents[CTRL_EVENT_COUNT] = {
    {'o', &EV_Notepad_Open                 },
    {'s', &EV_Notepad_Save                 },
    {'a', &EV_Notepad_SaveAs               },
    {'w', &UIFloatingWindow::W_ClosePressed},
    {'f', &EV_Notepad_Find                 },
    {'g', &EV_Notepad_Goto                 },
    {'c', &EV_Notepad_Copy                 },
    {'x', &EV_Notepad_Cut                  },
    {'v', &EV_Notepad_Paste                },
    {0,   NULL                             },
};

bool UI_LoadNotepadFile(const char *filename)
{
    UINotepad *uinp = new UINotepad();
    UIRect2D   rect = UIRect2D(100.0f, 100.0f, 450.0f, 300.0f);
    if (uinp->Create(NULL, rect, filename)) {
        uWinMan.ActivateControl(uinp);
        return true;
    }

    if (uinp) {
        delete uinp;
    }

    return false;
}

UINotepadEdit::UINotepadEdit()
{
    m_notepad = NULL;
}

void UINotepadEdit::CharEvent(int ch)
{
    if (!m_notepad->ProcessCharEvent(ch)) {
        UIMultiLineEdit::CharEvent(ch);
    }
}

void UINotepadEdit::setNotepad(UINotepad *notepad)
{
    m_notepad = notepad;
}

bool UINotepadEdit::GotoLine(int line)
{
    if (line < 1 || line - 1 >= m_lines.getCount()) {
        return false;
    }

    m_selection.begin.line = m_selection.end.line = line - 1;
    m_selection.begin.column                      = 0;
    m_selection.end.column                        = 999999;
    BoundSelectionPoint(m_selection.end);
    EnsureSelectionPointVisible(m_selection.end);
    return true;
}

bool UINotepadEdit::FindText(const char *text, int offsetFromSel)
{
    if (!text) {
        return false;
    }

    str lowersearch = text;
    lowersearch.tolower();

    selectionpoint_s *topsel, *botsel;
    SortSelection(&topsel, &botsel);

    int  foundat       = -1;
    bool searchfromtop = false;

    // This loop runs at most twice:
    //  - once for searchfromtop == false,
    //  - optionally for searchfromtop == true,
    // but only if text couldn't be found between the cursor and the end of the file
    while (true) {
        int i = 0;
        m_lines.IterateFromHead();
        for (; m_lines.IsCurrentValid(); i++, m_lines.IterateNext()) {
            if (i < topsel->line && !searchfromtop) {
                // skip lines before the cursor if not searching from the top
                continue;
            }
            if (i > topsel->line && searchfromtop) {
                // no need to continue if we've looped back to the cursor
                return false;
            }

            str findin = m_lines.getCurrent();
            findin.tolower();
            const char *where = findin.c_str();

            // Added in OPM to avoid code repetition
            bool searchLineFromCursor = i == topsel->line && !searchfromtop;
            if (searchLineFromCursor) {
                // Start searching on the current line, BUT ONLY from the cursor position.
                // Offset it with the selection offset:
                // the purpose of this is to prevent matching
                // on a string that was just found and is still selected,
                // by offsetting it
                if (findin.length() <= topsel->column + offsetFromSel) {
                    // cursor is at the end of the line, nothing left to search in this one
                    continue;
                }

                where = &where[topsel->column + offsetFromSel];
            }

            const char *found = strstr(where, lowersearch);

            if (found) {
                // use the beginning of the line to calculate where the matched text is
                foundat = found - where;
                if (searchLineFromCursor) {
                    // take cursor pos. into account if a match is found on the same line as the cursor
                    foundat += topsel->column + offsetFromSel;
                }
                break;
            }
        }

        if (foundat == -1) {
            // couldn't find text in current search iteration
            if (!searchfromtop) {
                // try searching from the beginning of the document
                searchfromtop = true;
                continue;
            }

            // text not found
            return false;
        } else {
            // jump to found text and select it
            m_selection.begin.column = foundat;
            m_selection.end.column   = foundat + strlen(text);
            m_selection.begin.line = m_selection.end.line = i;
            EnsureSelectionPointVisible(m_selection.end);
            return true;
        }
    }
}

void UINotepadEdit::MousePressed(Event *ev)
{
    if (m_notepad->m_state != STATE_TIMED_MESSAGE) {
        m_notepad->m_state = STATE_NONE;
    }

    UIMultiLineEdit::MouseDown(ev);
}

UINotepad::UINotepad()
{
    m_state  = STATE_NONE;
    m_edit   = NULL;
    m_menu   = NULL;
    m_status = NULL;
}

UINotepad::~UINotepad()
{
    for (int i = m_popups.NumObjects(); i > 0; i--) {
        Container<uipopup_describe *> *inner = m_popups.ObjectAt(i);
        for (int j = inner->NumObjects(); j > 0; j--) {
            uipopup_describe *uipd = inner->ObjectAt(j);
            inner->RemoveObjectAt(j);
            delete uipd;
        }
    }
}

void UINotepad::TimeMessage(const char *message, int howlong)
{
    m_state             = STATE_TIMED_MESSAGE;
    m_timedmessage.die  = uid.time + howlong;
    m_timedmessage.text = message;
}

bool UINotepad::OpenFile(const char *filename)
{
    char *data = NULL;
    if (!filename || FS_ReadFile(filename, (void **)&data) == -1) {
        return false;
    }

    setFileName(filename);
    if (m_edit) {
        m_edit->Empty();
        m_edit->setData(data);
    }

    FS_FreeFile(data);
    return true;
}

bool UINotepad::Create(UIWidget *parent, UIRect2D& rect, const char *filename)
{
    UIFloatingWindow::Create(parent, rect, "", UColor(0.15f, 0.196f, 0.278f), UHudColor);

    m_edit = new UINotepadEdit();
    m_edit->setNotepad(this);

    m_edit->InitFrame(getChildSpace(), 0.0f, 0.0f, 20.0f, 20.0f, 0, "verdana-12");
    m_edit->setBackgroundColor(UWhite, true);
    m_edit->setForegroundColor(UBlack);
    m_edit->setFont("verdana-14");

    m_status = new UIStatusBar(WND_ALIGN_BOTTOM, 20.0f);
    m_status->InitFrame(getChildSpace(), 0.0f, 0.0f, 20.0f, 20.0f, 0, "verdana-12");
    m_status->EnableSizeBox(this);

    m_menu = new UIPulldownMenu();
    m_menu->CreateAligned(getChildSpace(), this);
    // Fixed in OPM: menu text was hard to read when notepad is active
    //m_menu->setForegroundColor(UBlack);
    m_menu->setForegroundColor(UHudColor);

    Container<uipopup_describe *> *pops = new Container<uipopup_describe *>();

    uipopup_describe *uipd = new uipopup_describe("Open (Ctrl-O)", UIP_EVENT, &EV_Notepad_Open, NULL);
    pops->AddObject(uipd);
    uipd = new uipopup_describe("Save (Ctrl-S)", UIP_EVENT, &EV_Notepad_Save, NULL);
    pops->AddObject(uipd);
    uipd = new uipopup_describe("Save As...(Ctrl-A)", UIP_EVENT, &EV_Notepad_SaveAs, NULL);
    pops->AddObject(uipd);

    uipd = new uipopup_describe(str(), UIP_SEPARATOR, NULL, NULL);
    pops->AddObject(uipd);

    uipd = new uipopup_describe("Close (Ctrl-W)", UIP_EVENT, &UIFloatingWindow::W_ClosePressed, NULL);
    pops->AddObject(uipd);

    m_popups.AddObject(pops);
    for (int i = 1; i <= pops->NumObjects(); i++) {
        m_menu->AddUIPopupDescribe("File", pops->ObjectAt(i));
    }

    pops = new Container<uipopup_describe *>();
    uipd = new uipopup_describe("Copy (Ctrl-C)", UIP_EVENT, &EV_Notepad_Copy, NULL);
    pops->AddObject(uipd);
    uipd = new uipopup_describe("Cut (Ctrl-X)", UIP_EVENT, &EV_Notepad_Cut, NULL);
    pops->AddObject(uipd);
    uipd = new uipopup_describe("Paste (Ctrl-V)", UIP_EVENT, &EV_Notepad_Paste, NULL);
    pops->AddObject(uipd);

    uipd = new uipopup_describe(str(), UIP_SEPARATOR, NULL, NULL);
    pops->AddObject(uipd);

    uipd = new uipopup_describe("Find (Ctrl-F)", UIP_EVENT, &EV_Notepad_Find, NULL);
    pops->AddObject(uipd);
    uipd = new uipopup_describe("Go to line... (Ctrl-G)", UIP_EVENT, &EV_Notepad_Goto, NULL);
    pops->AddObject(uipd);

    m_popups.AddObject(pops);
    for (int i = 1; i <= pops->NumObjects(); i++) {
        m_menu->AddUIPopupDescribe("Edit", pops->ObjectAt(i));
    }

    setFileName("");
    OpenFile(filename);
    getChildSpace()->Connect(this, W_SizeChanged, W_Notepad_ChildSizeChanged);
    getChildSpace()->SendSignal(W_SizeChanged);
    getChildSpace()->AllowActivate(false);
    return true;
}

void UINotepad::ChildSizeChanged(Event *ev)
{
    m_menu->Realign();
    UISize2D menuSize       = m_menu->getSize();
    UISize2D childSpaceSize = getChildSpace()->getSize();

    UIRect2D frame(0.0f, menuSize.height, childSpaceSize.width, childSpaceSize.height - 20.0f - menuSize.height);

    m_edit->setFrame(frame);
}

void UINotepad::SaveAs(Event *ev)
{
    m_state          = STATE_SAVE_AS;
    m_textinput.text = "";
}

void UINotepad::Save(Event *ev)
{
    if (!m_filename.length()) {
        SaveAs(NULL);
        // Fixed in OPM:
        // devs probably forgot to return here - saving was attempted,
        // even though there was no filename to save the file with
        return;
    }

    str filecontents;
    m_edit->getData(filecontents);
    m_edit->setChanged(false);

    FS_WriteTextFile(m_filename, filecontents, filecontents.length());

    str result = "Saved " + m_filename;
    TimeMessage(result, 3000);
}

void UINotepad::Open(Event *ev)
{
    PickFile(m_filename, this, EV_Notepad_OpenFile);
}

void UINotepad::OpenFile(Event *ev)
{
    OpenFile(ev->GetString(1));
}

void UINotepad::ClosePressed(Event *ev)
{
    uWinMan.ActivateControl(this);
    if (m_edit->IsChanged()) {
        m_state          = STATE_CONFIRMCLOSE;
        m_textinput.text = "y";
    } else {
        PostEvent(EV_Remove, 0.0f);

        // Fixed in OPM:
        // in the original game, the control is not deactivated after removal,
        // so the already heap-freed UINotepad instance still "reacts" to
        // KeyEvents and CharEvents, creating a use-after-free scenario
        // that causes crashes when debug heap is enabled
        uWinMan.DeactivateCurrentControl();
    }
}

void UINotepad::OnFind(Event *ev)
{
    m_state          = STATE_FIND_TEXT;
    m_textinput.text = m_lastfind;
}

void UINotepad::OnGoto(Event *ev)
{
    m_state          = STATE_GOTO_LINE;
    m_textinput.text = "";
}

void UINotepad::OnCopy(Event *ev)
{
    m_edit->CopySelection();
}

void UINotepad::OnPaste(Event *ev)
{
    m_edit->PasteSelection();
}

void UINotepad::OnCut(Event *ev)
{
    m_edit->CopySelection();
    m_edit->DeleteSelection();
}

bool UINotepad::ProcessControlEvents(int ch)
{
    if (ch < CTRL('a') || ch > CTRL('z')) {
        // not a control character
        return false;
    }

    // Fixed in OPM:
    // Original code didn't bounds-check controlEvents
    for (int i = 0; i < CTRL_EVENT_COUNT; i++) {
        ctrlevent_s *event = &controlEvents[i];
        if (!event->ev) {
            // no event assigned
            return false;
        }

        if (ch == CTRL(event->ch)) {
            // found it
            PostEvent(
                new Event(*event->ev), 0.0f
            ); // create a new Event as it will be deleted by ProcessEvent in L_ProcessPendingEvents
            return true;
        }
    }

    return false;
}

bool UINotepad::ProcessCharEvent(int ch)
{
    if (ProcessControlEvents(ch)) {
        return true;
    }

    if (m_state == STATE_NONE || m_state == STATE_TIMED_MESSAGE) {
        return false;
    }

    str text;
    switch (ch) {
    case '\b': // Backspace
        m_textinput.text.CapLength(m_textinput.text.length() - 1);
        break;
    case K_ENTER:
        switch (m_state) {
        case STATE_GOTO_LINE:
            {
                if (!m_textinput.text.length()) {
                    m_state = STATE_NONE;
                    return true;
                }

                int line = atoi(m_textinput.text);
                if (m_edit->GotoLine(line)) {
                    m_state = STATE_NONE;
                    return true;
                }

                text = "Line '" + m_textinput.text + "' doesn't exist.";
                TimeMessage(text, 3000);
                break;
            }
        case STATE_FIND_TEXT:
            if (!m_textinput.text.length()) {
                m_state = STATE_NONE;
                return true;
            }

            if (!m_edit->FindText(m_textinput.text, 1)) {
                text = "Text: '" + m_textinput.text + "' could not be found.";
                TimeMessage(text, 3000);
            }

            m_lastfind = m_textinput.text;
            return true;
        case STATE_SAVE_AS:
            m_state = STATE_NONE;

            if (!m_textinput.text.length()) {
                return true;
            }

            setFileName(m_textinput.text);
            Save(NULL);
            return true;
        case STATE_CONFIRMCLOSE:
            {
                m_state = STATE_NONE;

                if (!m_textinput.text.length()) {
                    return true;
                }

                char choice = tolower(m_textinput.text[0]);
                if (choice != 'n' && choice != 'y') {
                    return true;
                }

                if (choice == 'y') {
                    Save(NULL);
                }

                // close window
                PostEvent(new Event(EV_Remove), 0.0f);

                // Fixed in OPM:
                // in the original game, the control is not deactivated after removal,
                // so the already heap-freed UINotepad instance still "reacts" to
                // KeyEvents and CharEvents, creating a use-after-free scenario,
                // causing crashes when debug heap is enabled
                uWinMan.DeactivateCurrentControl();

                return true;
            }
        default:
            m_state = STATE_NONE;
            return true;
        }
        return true;
    case K_ESCAPE:
    case K_TAB:
        m_state = STATE_NONE;
        return true;
    }

    if (!isprint(ch)) {
        return true;
    }

    if (m_state == STATE_CONFIRMCLOSE) {
        m_textinput.text = "";
    }

    m_textinput.text += (char)ch;
    return true;
}

void UINotepad::Draw(void)
{
    UColor out = URed;
    out.ScaleColor(0.5f);
    out.ScaleAlpha(0.5f);

    m_status->setBackgroundColor(out, true);
    m_status->setForegroundColor(UHudColor);

    str text;
    switch (m_state) {
    case STATE_GOTO_LINE:
        text = "Goto line (tab cancels): " + m_textinput.text;
        break;
    case STATE_FIND_TEXT:
        text = "Find text (tab cancels): " + m_textinput.text;
        break;
    case STATE_SAVE_AS:
        text = "Save As (tab cancels): " + m_textinput.text;
        break;
    case STATE_CONFIRMCLOSE:
        text = "Save " + m_filename + "? (yes/no/cancel) " + m_textinput.text;
        break;
    case STATE_TIMED_MESSAGE:
        if (uid.time > m_timedmessage.die) {
            m_state = STATE_NONE;
        default:
            m_status->setBackgroundColor(getBackgroundColor(), true);
            m_status->setForegroundColor(getForegroundColor()); // Fixed in OPM
            m_menu->setForegroundColor(getForegroundColor());   // Fixed in OPM
            UIPoint2D selpoint = m_edit->getEndSelPoint();
            text               = va("Line %d, column %d", (int)selpoint.y + 1, (int)selpoint.x + 1);
        } else {
            m_status->setBackgroundColor(getBackgroundColor(), true);
            m_status->setForegroundColor(URed);
            text = m_timedmessage.text;
        }
        break;
    }

    m_status->setTitle(text);
    UIFloatingWindow::Draw();
}

void UINotepad::setFileName(const char *filename)
{
    str text = str(filename) + " - Notepad";
    setTitle(text);
    m_filename = filename;
}
