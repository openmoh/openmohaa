/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

#include "../uilib/uilistbox.h"

class UIMapRotationListBox : public UIListBox
{
    CLASS_PROTOTYPE(UIMapRotationListBox);

public:
    UIMapRotationListBox();

    void RefreshRotationList(Event *ev);
    void PopulateRotationList();
};

class UIMapListBox : public UIListBox
{
    CLASS_PROTOTYPE(UIMapListBox);

public:
    UIMapListBox();

    void RefreshMapList(Event *ev);
    void PopulateMapList();
};

class UIAddToRotationButton : public UIButton
{
    CLASS_PROTOTYPE(UIAddToRotationButton);

public:
    void Released(Event *ev);
};

class UIRemoveFromRotationButton : public UIButton
{
    CLASS_PROTOTYPE(UIRemoveFromRotationButton);

public:
    void Released(Event *ev);
};

class UIRotationApplyButton : public UIButton
{
    CLASS_PROTOTYPE(UIRotationApplyButton);

public:
    void Released(Event *ev);
};

class UIRotationMoveItemUpButton : public UIButton
{
    CLASS_PROTOTYPE(UIRotationMoveItemUpButton);

public:
    void Released(Event *ev);
};

class UIRotationMoveItemDownButton : public UIButton
{
    CLASS_PROTOTYPE(UIRotationMoveItemDownButton);

public:
    void Released(Event *ev);
};
