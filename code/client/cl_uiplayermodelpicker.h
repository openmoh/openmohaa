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

class UIFloatingWindow;

class PlayerModelPickerClass : public USignal
{
    UIButton         *overlay;
    UIFloatingWindow *window;
    UIListCtrl       *listbox;
    str               currentDirectory;
    str               rootDirectory;
    qboolean          m_bGermanModels;

public:
    CLASS_PROTOTYPE(PlayerModelPickerClass);

private:
    void SetupFiles(void);
    void Initialize(const char *root_directory, const char *current_directory, qboolean bGermanModels);

protected:
    void         CloseWindow(void);
    void         FileSelected(Event *ev);
    // The name parameter was added in 2.0.
    virtual void FileSelected(const str& name, const str& currentDirectory, const str& partialName, const str& fullname);
    void         FileChosen(Event *ev);
    // The name parameter was added in 2.0.
    virtual void FileChosen(const str& name, const str& currentDirectory, const str& partialName, const str& fullname);
    void         OnDeactivated(Event *ev);

public:
    PlayerModelPickerClass();
    ~PlayerModelPickerClass();

    void Setup(const char *root_directory, const char *current_directory, qboolean bGermanModels);
};

const char* PM_FilenameToDisplayname(const char* fileName);
const char* PM_DisplaynameToFilename(const char* displayName);
