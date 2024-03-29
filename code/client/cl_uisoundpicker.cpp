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

CLASS_DECLARATION(FilePickerClass, SoundPickerClass, NULL) {
    {NULL, NULL}
};

SoundPickerClass::SoundPickerClass()
{
    str         soundpath;
    str         currentpath;
    const char *varValue;
    int         i;

    soundpath = "sound";

    varValue = UI_GetCvarString("ui_pickedsound", NULL);
    if (varValue && *varValue) {
        currentpath = varValue;

        for (i = currentpath.length(); i > 0; i--) {
            if (currentpath[i] == '/') {
                break;
            }
        }

        currentpath = str(currentpath, 0, i + 1);
    } else {
        currentpath = soundpath;
    }
}

void SoundPickerClass::FileSelected(const str& currentDirectory, const str& partialName, const str& fullname)
{
    uii.Snd_PlaySound(fullname);
}

void SoundPickerClass::FileChosen(const str& currentDirectory, const str& partialName, const str& fullname)
{
    uii.Cvar_Set("ui_pickedsound", fullname);
    CloseWindow();
}
