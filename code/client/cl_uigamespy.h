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

// cl_gamespy.h
#ifndef __CL_GAMESPY_H__
#define __CL_GAMESPY_H__

#include "cl_ui.h"
#include "keycodes.h"

class GameSpyDialog : public UIFloatingWindow
{
private:
    UIButton *overlay;
    UILabel  *label;
    UIButton *closeButton;

protected:
    void FrameInitialized(void) override;

public:
    GameSpyDialog();
    ~GameSpyDialog();
    void
    Create(UIWidget *parent, const UIRect2D& rect, const char *title, const UColor& bgColor, const UColor& fgColor);

    CLASS_PROTOTYPE(GameSpyDialog);
};

void UI_LaunchGameSpy_f(void);
#endif
