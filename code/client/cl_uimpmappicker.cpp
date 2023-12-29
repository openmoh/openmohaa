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

#include "cl_ui.h"

class MpMapPickerItem : public UIListCtrlItem
{
    str m_string;

public:
    int            getListItemValue(int which) const override;
    griditemtype_t getListItemType(int which) const override;
    str            getListItemString(int which) const override;
    void           DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont) override;
    qboolean       IsHeaderEntry() const override;
};

CLASS_DECLARATION(USignal, MpMapPickerClass, NULL) {
    {&EV_UIListBase_ItemSelected,      &MpMapPickerClass::FileSelected },
    {&EV_UIListBase_ItemDoubleClicked, &MpMapPickerClass::FileChosen   },
    {&W_Deactivated,                   &MpMapPickerClass::OnDeactivated},
    {NULL,                             NULL                            }
};

MpMapPickerClass::MpMapPickerClass()
{
    // FIXME: stub
}

MpMapPickerClass::~MpMapPickerClass()
{
    // FIXME: stub
}

void MpMapPickerClass::Setup(const char *root_directory, const char *current_directory)
{
    // FIXME: stub
}

void MpMapPickerClass::Initialize(const char *root_directory, const char *current_directory)
{
    // FIXME: stub
}

void MpMapPickerClass::GotoParentDirectory(void)
{
    // FIXME: stub
}

void MpMapPickerClass::GotoSubDirectory(str subdir)
{
    // FIXME: stub
}

void MpMapPickerClass::SetupFiles(void)
{
    // FIXME: stub
}

void MpMapPickerClass::FileSelected(str& currentDirectory, str& partialName, str& fullname)
{
    // FIXME: stub
}

void MpMapPickerClass::FileSelected(Event *ev)
{
    // FIXME: stub
}

void MpMapPickerClass::FileChosen(str& currentDirectory, str& partialName, str& fullname)
{
    // FIXME: stub
}

void MpMapPickerClass::FileChosen(Event *ev)
{
    // FIXME: stub
}

void MpMapPickerClass::CloseWindow(void)
{
    // FIXME: stub
}

void MpMapPickerClass::OnDeactivated(Event *ev)
{
    // FIXME: stub
}

int MpMapPickerItem::getListItemValue(int which) const
{
    // FIXME: stub
    return 0;
}

griditemtype_t MpMapPickerItem::getListItemType(int which) const
{
    // FIXME: stub
    return griditemtype_t::TYPE_STRING;
}

str MpMapPickerItem::getListItemString(int which) const
{
    // FIXME: stub
    return str();
}

void MpMapPickerItem::DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont)
{
    // FIXME: stub
}

qboolean MpMapPickerItem::IsHeaderEntry() const
{
    // FIXME: stub
    return qfalse;
}
