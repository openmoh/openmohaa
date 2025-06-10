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

class FilePickerItem : public UIListCtrlItem
{
    str strings[3];

public:
    FilePickerItem(const str& fileName, const str& date, const str& size);
    int            getListItemValue(int which) const override;
    griditemtype_t getListItemType(int which) const override;
    str            getListItemString(int which) const override;
    void           DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont) override;
    qboolean       IsHeaderEntry() const override;
};

CLASS_DECLARATION(USignal, FilePickerClass, NULL) {
    {&EV_UIListBase_ItemSelected,      &FilePickerClass::FileSelected },
    {&EV_UIListBase_ItemDoubleClicked, &FilePickerClass::FileChosen   },
    {&W_Deactivated,                   &FilePickerClass::OnDeactivated},
    {NULL,                             NULL                           }
};

FilePickerClass::FilePickerClass()
{
    window = new UIFloatingWindow();
    window->Create(
        NULL,
        UIRect2D((uid.vidWidth - 400) / 2, (uid.vidHeight - 300) / 2, 400, 300),
        "File Picker",
        UColor(0.15f, 0.195f, 0.278f),
        UHudColor
    );
    window->Connect(this, W_Deactivated, W_Deactivated);
    listbox = new UIListCtrl();
    listbox->InitFrame(window->getChildSpace(), window->getChildSpace()->getClientFrame(), 0);
    listbox->setBackgroundColor(UColor(0.02f, 0.07f, 0.005f), true);
    listbox->setForegroundColor(UHudColor);
    listbox->AddColumn("File Name", 0, 175, false, false);
    listbox->AddColumn("Date", 1, 125, false, false);
    listbox->AddColumn("Size", 2, 100, true, false);

    listbox->Connect(this, EV_UIListBase_ItemDoubleClicked, EV_UIListBase_ItemDoubleClicked);
    listbox->Connect(this, EV_UIListBase_ItemSelected, EV_UIListBase_ItemSelected);
    listbox->AllowActivate(true);

    // Added in 2.0
    //  Don't localize elements
    listbox->SetDontLocalize();
}

FilePickerClass::~FilePickerClass()
{
    if (listbox) {
        delete listbox;
        listbox = NULL;
    }

    if (window) {
        delete window;
        window = NULL;
    }
}

void FilePickerClass::Setup(
    const char *root_directory, const char *current_directory, const char *ext, const char *ignore_files
)
{
    Initialize(root_directory, current_directory, ext, ignore_files);
}

//
//  Added in OPM
//
void FilePickerClass::SetWindowName(str name)
{
    if (window) {
        window->setName(name);
    }
}

void FilePickerClass::Initialize(
    const char *root_directory, const char *current_directory, const char *ext, const char *ignore_files
)
{
    rootDirectory = root_directory;
    if (rootDirectory.length() > 1 && rootDirectory[rootDirectory.length() - 1] != '/') {
        rootDirectory += "/";
    }

    currentDirectory = current_directory;
    if (currentDirectory.length() > 1 && currentDirectory[currentDirectory.length() - 1] != '/') {
        currentDirectory += "/";
    }

    extension = ext;
    if (ignore_files) {
        ignoredFiles = ignore_files;
    }

    SetupFiles();
}

void FilePickerClass::GotoParentDirectory(void)
{
    int i;

    if (currentDirectory == rootDirectory) {
        return;
    }

    for (i = currentDirectory.length() - 2; i > 0; i--) {
        if (currentDirectory[i] == '/') {
            break;
        }
    }

    if (currentDirectory[i] == '/') {
        i++;
    }

    currentDirectory = str(currentDirectory, 0, i);

    // refresh files
    SetupFiles();
}

void FilePickerClass::GotoSubDirectory(str subdir)
{
    currentDirectory += subdir + "/";

    // refresh files
    SetupFiles();
}

void FilePickerClass::SetupFiles(void)
{
    str    mappath;
    str    work;
    char **filenames;
    int    numfiles;
    int    i;
    char   date[128];
    char   size[128];

    listbox->DeleteAllItems();

    if (currentDirectory != rootDirectory) {
        // create the parent directory item
        listbox->AddItem(new FilePickerItem("..", "", ""));
    }

    // retrieve directories
    filenames = FS_ListFiles(currentDirectory, "/", qfalse, &numfiles);

    for (i = 0; i < numfiles; i++) {
        const char *filename = filenames[i];

        if (filename[0] == '.' || !strlen(filename)) {
            continue;
        }

        work = "[";
        work += filename;
        work += "]";

        FS_FileTime(currentDirectory + filename, date, size);

        listbox->AddItem(new FilePickerItem(work, date, size));
    }

    FS_FreeFileList(filenames);

    filenames = FS_ListFiles(currentDirectory, extension, qfalse, &numfiles);

    for (i = 0; i < numfiles; i++) {
        const char *filename = filenames[i];

        if (ignoredFiles.length() && strstr(filename, ignoredFiles)) {
            // Added in 2.0
            //  Check for ignored files
            continue;
        }

        work = filename;

        FS_FileTime(currentDirectory + work, date, size);

        listbox->AddItem(new FilePickerItem(work, date, size));
    }

    FS_FreeFileList(filenames);

    window->setTitle(currentDirectory);
}

void FilePickerClass::FileSelected(const str& currentDirectory, const str& partialName, const str& fullname) {}

void FilePickerClass::FileSelected(Event *ev)
{
    str name = listbox->GetItem(listbox->getCurrentItem())->getListItemString(0);

    if (*name != '[' && *name != '.') {
        FileSelected(currentDirectory, name, currentDirectory + name);
    }

    uii.Snd_PlaySound("sound/menu/scroll.wav");
}

void FilePickerClass::FileChosen(const str& currentDirectory, const str& partialName, const str& fullname) {}

void FilePickerClass::FileChosen(Event *ev)
{
    str filename = listbox->GetItem(listbox->getCurrentItem())->getListItemString(0);

    if (*filename == '[') {
        uii.Snd_PlaySound("sound/menu/scroll.wav");

        GotoSubDirectory(str(filename, 1, filename.length() - 1));
    } else if (*filename == '.') {
        GotoParentDirectory();
    } else {
        uii.Snd_PlaySound("sound/menu/apply.wav");

        FileChosen(currentDirectory, filename, currentDirectory + filename);
    }
}

void FilePickerClass::CloseWindow(void)
{
    PostEvent(EV_Remove, 0);
}

void FilePickerClass::OnDeactivated(Event *ev)
{
    CloseWindow();
}

FilePickerItem::FilePickerItem(const str& fileName, const str& date, const str& size)
{
    strings[0] = fileName;
    strings[1] = date;
    strings[2] = size;
}

int FilePickerItem::getListItemValue(int which) const
{
    return atoi(strings[which]);
}

griditemtype_t FilePickerItem::getListItemType(int which) const
{
    return griditemtype_t::TYPE_STRING;
}

str FilePickerItem::getListItemString(int which) const
{
    return strings[which];
}

void FilePickerItem::DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont) {}

qboolean FilePickerItem::IsHeaderEntry() const
{
    return qfalse;
}
