/*
===========================================================================
Copyright (C) 2023-2024 the OpenMoHAA team

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

CLASS_DECLARATION(FilePickerClass, MapRunnerClass, NULL) {
    {NULL, NULL}
};

void MapRunnerClass::FileChosen(const str& currentDirectory, const str& partialName, const str& fullname)
{
    str newName;

    newName = "spmap " + str(fullname, 5, fullname.length() - 4) + "\n";
    // insert the command
    Cbuf_AddText(newName);

    CloseWindow();
}

CLASS_DECLARATION(FilePickerClass, UIPickFileClass, NULL) {
    {NULL, NULL}
};

UIPickFileClass::UIPickFileClass()
{
    retobj   = NULL;
    retevent = NULL;
}

UIPickFileClass::~UIPickFileClass()
{
    if (retevent) {
        delete retevent;
    }
}

void UIPickFileClass::FileChosen(const str& currentDirectory, const str& partialName, const str& fullname)
{
    if (!retobj || !retevent) {
        CloseWindow();
        return;
    }

    retevent->AddString(fullname);
    retobj->ProcessEvent(retevent);

    retevent = NULL;
    retobj   = NULL;

    CloseWindow();
}

void PickFile(const char *name, Listener *obj, Event& event)
{
    UIPickFileClass *picker;
    str              currentpath;
    int              i;

    picker           = new UIPickFileClass();
    picker->retevent = new Event(event);
    picker->retobj   = obj;

    if (name && *name && strchr(name, '/')) {
        currentpath = name;

        for (i = currentpath.length() - 1; i > 0; i--) {
            if (currentpath[i] == '/') {
                break;
            }
        }

        currentpath = str(currentpath, 0, i + 1);
    } else {
        currentpath = "";
    }

    picker->Setup("", currentpath, ".*", "");
}

CLASS_DECLARATION(FilePickerClass, ViewSpawnerClass, NULL) {
    {NULL, NULL}
};

void ViewSpawnerClass::FileChosen(const str& currentDirectory, const str& partialName, const str& fullname)
{
    str newName;

    newName = "viewspawn " + fullname + "\n";
    // insert the command
    Cbuf_AddText(newName);

    CloseWindow();
}

CLASS_DECLARATION(FilePickerClass, LODSpawnerClass, NULL) {
    {NULL, NULL}
};

void LODSpawnerClass::FileChosen(const str& currentDirectory, const str& partialName, const str& fullname)
{
    str newName;

    newName = "lod_spawn " + fullname + "\n";
    // insert the command
    Cbuf_AddText(newName);

    CloseWindow();
}
