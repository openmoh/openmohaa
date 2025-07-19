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

#include "uimessage.h"

CLASS_DECLARATION(UIFloatingWindow, UIMessageDialog, NULL) {
    {&W_Deactivated, &UIFloatingWindow::ClosePressed},
    {NULL,           NULL                           }
};

UIMessageDialog::UIMessageDialog()
    : m_label(NULL)
    , m_ok(NULL)
{}

UIMessageDialog::~UIMessageDialog()
{
    if (m_label) {
        delete m_label;
    }

    if (m_ok) {
        delete m_ok;
    }
}

void UIMessageDialog::FrameInitialized(void)
{
    UIFloatingWindow::FrameInitialized();

    m_label = new UILabel();

    m_label->InitFrame(getChildSpace(), getChildSpace()->getClientFrame(), 0);
    m_label->setForegroundColor(UHudColor);

    m_ok = new UIButton();
    m_ok->InitFrame(getChildSpace(), UIRect2D(100, 150, 100, 30), 0);
    m_ok->setTitle("OK");
    m_ok->setBackgroundColor(UColor(0.25f, 0.295f, 0.378f), false);
    m_ok->AllowActivate(true);
    m_ok->Connect(this, W_Button_Pressed, W_Deactivated);

    m_minimizeButton->ProcessEvent(EV_Widget_Hide);
}

void UIMessageDialog::setText(const char *text)
{
    int maxWidth  = m_label->getFont()->getWidth(text, -1);
    int maxHeight = m_label->getFont()->getHeight(text, -1);

    m_label->setTitle(text);

    UIRect2D rect;
    rect.pos.x       = (uid.vidWidth - Q_max(maxWidth, 100)) / 2.0;
    rect.pos.y       = (uid.vidHeight - Q_max(maxHeight, 32)) / 2.0;
    rect.size.width  = maxWidth + 32;
    rect.size.height = 8 + maxHeight + 16 + 24 + 32;

    setFrame(rect);

    const UIRect2D clientFrame = getChildSpace()->getClientFrame();

    UIRect2D labelRect;
    labelRect.pos.x       = 8;
    labelRect.pos.y       = 8;
    labelRect.size.width  = maxWidth;
    labelRect.size.height = maxHeight;
    m_label->setFrame(labelRect);

    UIRect2D buttonRect;
    buttonRect.pos.x       = (clientFrame.size.width - 64) / 2;
    buttonRect.pos.y       = labelRect.pos.y + labelRect.size.height + 16;
    buttonRect.size.width  = 64;
    buttonRect.size.height = 24;
    m_ok->setFrame(buttonRect);
}

void UIMessageDialog::ShowMessageBox(const char *title, const char *text)
{
    UIMessageDialog *dialog;

    dialog = new UIMessageDialog();

    dialog->Create(
        NULL, UIRect2D(uid.vidWidth / 2, uid.vidHeight / 2, 100, 200), title, UColor(0.15f, 0.195f, 0.278f), UHudColor
    );

    dialog->setText(text);

    uWinMan.ActivateControl(dialog);

    dialog->Connect(dialog, W_Deactivated, W_Deactivated);
}
