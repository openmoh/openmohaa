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
#include "../qcommon/localization.h"

class MpMapPickerItem : public UIListCtrlItem
{
    str m_string;
    // Added in 2.0
    str m_directory;

public:
    MpMapPickerItem(const str& string, const str& directory);

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
    window = new UIFloatingWindow();
    window->Create(
        NULL,
        UIRect2D((cls.glconfig.vidWidth - 300) / 2, (cls.glconfig.vidHeight - 200) / 2, 300, 200),
        "Select a Map",
        UColor(0.15f, 0.195f, 0.278f),
        UHudColor
    );
    window->setFont("facfont-20");
    window->PassEventToWidget("closebutton", new Event(EV_Widget_Disable));
    window->PassEventToWidget("minimizebutton", new Event(EV_Widget_Disable));
    window->Connect(this, W_Deactivated, W_Deactivated);

    listbox = new UIListCtrl();
    listbox->InitFrame(window->getChildSpace(), window->getChildSpace()->getClientFrame(), 0);
    listbox->SetDrawHeader(false);
    listbox->setFont("facfont-20");
    listbox->FrameInitialized();
    listbox->AddColumn(Sys_LV_CL_ConvertString("Select a Map"), 0, 400, false, false);

    listbox->Connect(this, EV_UIListBase_ItemDoubleClicked, EV_UIListBase_ItemDoubleClicked);
    listbox->Connect(this, EV_UIListBase_ItemSelected, EV_UIListBase_ItemSelected);
    listbox->AllowActivate(true);

    // Added in 2.0
    //  Don't localize elements
    listbox->SetDontLocalize();
}

MpMapPickerClass::~MpMapPickerClass()
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

void MpMapPickerClass::Setup(const char *root_directory, const char *current_directory, const char *game_type)
{
    Initialize(root_directory, current_directory, game_type);
}

void MpMapPickerClass::Initialize(const char *root_directory, const char *current_directory, const char *game_type)
{
    rootDirectory = root_directory;
    if (rootDirectory.length() > 1 && rootDirectory[rootDirectory.length() - 1] != '/') {
        rootDirectory += "/";
    }

    currentDirectory = current_directory;
    if (currentDirectory.length() > 1 && currentDirectory[currentDirectory.length() - 1] != '/') {
        currentDirectory += "/";
    }

    if (game_type) {
        gameType = game_type;
    }

    SetupFiles();
}

void MpMapPickerClass::GotoParentDirectory(void)
{
    uintptr_t i;

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

void MpMapPickerClass::GotoSubDirectory(str subdir)
{
    currentDirectory += subdir + "/";

    // refresh files
    SetupFiles();
}

void MpMapPickerClass::SetupFiles(void)
{
    char **filenames;
    int    numfiles;
    int    i;
    char   mapname[128];
    bool   bTugOfWar = false, bObjective = false, bLiberation = false;

    listbox->DeleteAllItems();

    if (gameType == "tow") {
        bTugOfWar = true;
    } else if (gameType == "obj") {
        bObjective = true;
    } else if (gameType == "lib") {
        bLiberation = true;
    } else {
        // retrieve directories
        filenames = FS_ListFiles(rootDirectory, ".bsp", qfalse, &numfiles);

        for (i = 0; i < numfiles; i++) {
            const char *filename = filenames[i];

            strcpy(mapname, filename);
            mapname[strlen(mapname) - 4] = 0;

            if (COM_IsMapValid(mapname)) {
                listbox->AddItem(new MpMapPickerItem(mapname, rootDirectory));
            }
        }

        FS_FreeFileList(filenames);
    }

    if (com_target_game->integer > target_game_e::TG_MOH) {
        if (currentDirectory.length()) {
            if (currentDirectory == "maps/-/") {
                SetupSecondaryFiles(currentDirectory, bTugOfWar, bObjective, bLiberation);
            } else {
                SetupSecondaryFiles("maps/obj/", bTugOfWar, bObjective, bLiberation);
                SetupSecondaryFiles("maps/lib/", bTugOfWar, bObjective, bLiberation);
            }
        }
    }

    listbox->SortByColumn(0);
}

void MpMapPickerClass::SetupSecondaryFiles(const char *path, bool bTugOfWar, bool bObjective, bool bLiberation)
{
    char **filenames;
    int    numfiles;
    int    i;
    char   mapname[128];
    char   string[1024];

    filenames = FS_ListFiles(path, ".bsp", qfalse, &numfiles);

    for (i = 0; i < numfiles; i++) {
        const char *filename = filenames[i];
        const char *token;

        strcpy(mapname, filename);
        mapname[strlen(mapname) - 4] = 0;

        if (!COM_IsMapValid(mapname)) {
            continue;
        }

        if (bTugOfWar || bObjective || bLiberation) {
            bool bHasMP = false, bHasTOW = false, bHasLib = false;

            strcpy(string, mapname);

            for (token = strtok(string, "_"); token; token = strtok(NULL, "_")) {
                if (bObjective) {
                    if (!Q_stricmp(token, "obj")) {
                        listbox->AddItem(new MpMapPickerItem(mapname, path));
                    } else if (!Q_stricmp(token, "ship")) {
                        listbox->AddItem(new MpMapPickerItem(mapname, path));
                    }
                }

                if (bTugOfWar) {
                    if (!Q_stricmp(token, "MP")) {
                        bHasMP = true;
                    }
                    if (!Q_stricmp(token, "TOW")) {
                        bHasTOW = true;
                    }

                    if (bHasMP && bHasTOW) {
                        listbox->AddItem(new MpMapPickerItem(mapname, path));
                    }
                }

                if (bLiberation) {
                    if (!Q_stricmp(token, "MP")) {
                        bHasMP = true;
                    }
                    if (!Q_stricmp(token, "LIB")) {
                        bHasLib = true;
                    }

                    if (bHasMP && bHasLib) {
                        listbox->AddItem(new MpMapPickerItem(mapname, path));
                    }
                }
            }
        } else {
            if (!Q_stricmp(mapname, "obj_team2") || !Q_stricmp(mapname, "obj_team4")) {
                continue;
            }

            listbox->AddItem(new MpMapPickerItem(mapname, path));
        }
    }

    FS_FreeFileList(filenames);
}

void MpMapPickerClass::FileSelected(const str& currentDirectory, const str& partialName, const str& fullname)
{
    FileChosen(currentDirectory, partialName, fullname);
}

void MpMapPickerClass::FileSelected(Event *ev)
{
    if (!listbox->getCurrentItem()) {
        return;
    }

    uii.Snd_PlaySound("sound/menu/apply.wav");

    UIListCtrlItem *item      = listbox->GetItem(listbox->getCurrentItem());
    str             name      = item->getListItemString(0);
    str             directory = item->getListItemString(1);

    FileSelected(directory, name, directory + name);
}

void MpMapPickerClass::FileChosen(const str& currentDirectory, const str& partialName, const str& fullname)
{
    const char *pszFilename;
    str         sCommand;

    pszFilename = fullname.c_str();
    sCommand    = va("ui_dmmap %s\n", pszFilename + 5);

    Cbuf_AddText(sCommand);
    CloseWindow();
}

void MpMapPickerClass::FileChosen(Event *ev)
{
    if (!listbox->getCurrentItem()) {
        return;
    }

    uii.Snd_PlaySound("sound/menu/apply.wav");

    UIListCtrlItem *item      = listbox->GetItem(listbox->getCurrentItem());
    str             name      = item->getListItemString(0);
    str             directory = item->getListItemString(1);

    FileSelected(directory, name, directory + name);
}

void MpMapPickerClass::CloseWindow(void)
{
    PostEvent(EV_Remove, 0);
}

void MpMapPickerClass::OnDeactivated(Event *ev)
{
    CloseWindow();
}

MpMapPickerItem::MpMapPickerItem(const str& string, const str& directory)
{
    m_string    = string;
    m_directory = directory;
}

int MpMapPickerItem::getListItemValue(int which) const
{
    return atoi(m_string);
}

griditemtype_t MpMapPickerItem::getListItemType(int which) const
{
    return griditemtype_t::TYPE_STRING;
}

str MpMapPickerItem::getListItemString(int which) const
{
    switch (which) {
    default:
    case 0:
        return m_string;
    case 1:
        return m_directory;
    }
}

void MpMapPickerItem::DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont) {}

qboolean MpMapPickerItem::IsHeaderEntry() const
{
    return qfalse;
}
