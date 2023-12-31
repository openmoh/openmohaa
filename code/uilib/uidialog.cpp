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

CLASS_DECLARATION(UIFloatingWindow, UIDialog, NULL) {
    {NULL, NULL}
};

UIDialog::UIDialog()
{
    m_label  = NULL;
    m_ok     = NULL;
    m_cancel = NULL;
}

UIDialog::~UIDialog()
{
    if (m_label) {
        delete m_label;
    }
    if (m_ok) {
        delete m_ok;
    }
    if (m_cancel) {
        delete m_cancel;
    }
}

void UIDialog::FrameInitialized(void)
{
    UIRect2D rect;

    UIFloatingWindow::FrameInitialized();

    rect = getChildSpace()->getClientFrame();

    m_label = new UILabel();
    m_label->InitFrame(getChildSpace(), rect, 0);

    m_cancel = new UIButton();
    m_cancel->setTitle("Cancel");
    m_cancel->setBackgroundAlpha(0);
    m_cancel->InitFrame(getChildSpace(), rect.size.width - 60 - 48, rect.size.height - 24 - 6, 60, 24, 0);
    m_cancel->Connect(this, W_Button_Pressed, UIFloatingWindow::W_ClosePressed);

    m_ok = new UIButton();
    m_ok->setTitle("Ok");
    m_ok->setBackgroundAlpha(0);
    m_ok->InitFrame(getChildSpace(), 48, rect.size.height - 24 - 6, 60, 24, 0);
    m_ok->Connect(this, W_Button_Pressed, UIFloatingWindow::W_ClosePressed);

    m_minimizeButton->ProcessEvent(new Event("hide"));
    m_closeButton->ProcessEvent(new Event("hide"));
}

void UIDialog::LinkCvar(str cvarname)
{
    cvar_t *cvar;
    str     formatted;
    char   *ptr;
    char   *nextptr;
    char    dialog[256];

    if (!m_label) {
        return;
    }

    cvar = UI_FindCvar(cvarname);
    if (!cvar) {
        return;
    }

    Q_strncpyz(dialog, cvar->string, sizeof(dialog));
    ptr = dialog;

    for (ptr = dialog; ptr; ptr = nextptr + 1) {
        nextptr = strchr(ptr, '$');
        if (nextptr) {
            *nextptr = 0;
        }

        if (strlen(ptr)) {
            formatted += ptr;
        }

        formatted += '\n';

        if (!nextptr) {
            break;
        }

        *nextptr = '$';
    }

    m_label->SetLabel(formatted);
}

void UIDialog::SetOKCommand(str cvarname)
{
    if (m_ok) {
        m_ok->LinkCommand(cvarname);
    }
}

void UIDialog::SetCancelCommand(str cvarname)
{
    if (m_cancel) {
        m_cancel->LinkCommand(cvarname);
    }
}

void UIDialog::SetLabelMaterial(UIReggedMaterial *mat)
{
    m_label->setMaterial(mat);
    m_label->setTitle(str());
}

void UIDialog::SetOkMaterial(UIReggedMaterial *mat)
{
    m_ok->setMaterial(mat);
    m_ok->setTitle(str());
}

void UIDialog::SetCancelMaterial(UIReggedMaterial *mat)
{
    m_cancel->setMaterial(mat);
    m_cancel->setTitle(str());
}
