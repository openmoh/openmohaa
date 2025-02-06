/*
===========================================================================
Copyright (C) 2025 the OpenMoHAA team

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

// cl_uigamespy.cpp
#include "cl_uigamespy.h"

CLASS_DECLARATION(UIFloatingWindow, GameSpyDialog, NULL) {
    {&W_Deactivated, &UIFloatingWindow::ClosePressed},
    {NULL,           NULL                           }
};

GameSpyDialog::GameSpyDialog()
    : overlay(NULL)
    , label(NULL)
    , closeButton(NULL)
{
    AddFlag(WF_ALWAYS_TOP);
}

GameSpyDialog::~GameSpyDialog()
{
    if (overlay) {
        delete overlay;
        overlay = NULL;
    }

    if (label) {
        delete label;
        label = NULL;
    }

    if (closeButton) {
        delete closeButton;
        closeButton = NULL;
    }
}

void GameSpyDialog::FrameInitialized(void)
{
    UIFloatingWindow::FrameInitialized();

    label = new UILabel();

    label->InitFrame(getChildSpace(), getChildSpace()->getClientFrame(), 0);
    label->setTitle(
        "GameSpy's multiplayer matchmaking\n"
        "and server browsing services, which were\n"
        "essential for online gaming in many classic\n"
        "titles including Medal of Honor: Allied Assault,\n"
        "were permanently shut down in 2014."
    );
    label->setForegroundColor(UHudColor);

    closeButton = new UIButton();
    closeButton->InitFrame(getChildSpace(), UIRect2D(100, 150, 100, 30), 0);
    closeButton->setTitle("Close");
    closeButton->AllowActivate(true);
    closeButton->Connect(this, W_Button_Pressed, W_Deactivated);

    overlay = new UIButton();
    overlay->InitFrame(NULL, UIRect2D(0, 0, uid.vidWidth, uid.vidHeight), 0);
    overlay->setBackgroundColor(UColor(0, 0, 0, 0.5f), true);
    overlay->AllowActivate(true);

    overlay->Connect(this, W_Button_Pressed, W_Deactivated);
}

void GameSpyDialog::Create(
    UIWidget *parent, const UIRect2D& rect, const char *title, const UColor& bgColor, const UColor& fgColor
)
{
    // First call parent's Create
    UIFloatingWindow::Create(parent, rect, title, bgColor, fgColor);

    // After creation, find minimize button by name and hide it
    for (UIWidget *child = getFirstChild(); child; child = getNextChild(child)) {
        if (strcmp(child->getName(), "minimizebutton") == 0) {
            child->setShow(false);
            break;
        }
    }
}

void UI_LaunchGameSpy_f(void)
{
    GameSpyDialog *dialog = new GameSpyDialog();

    dialog->Create(
        NULL,
        UIRect2D((uid.vidWidth - 300) / 2, (uid.vidHeight - 200) / 2, 300, 200),
        "GameSpy",
        UColor(0.15f, 0.195f, 0.278f),
        UHudColor
    );

    uWinMan.ActivateControl(dialog);

    dialog->Connect(dialog, W_Deactivated, W_Deactivated);
}
