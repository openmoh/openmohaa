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

class FAKKLoadGameItem : public UIListCtrlItem
{
    str strings[4];

public:
    FAKKLoadGameItem(const str& missionName, const str& elapsedTime, const str& dateTime, const str& fileName);

    int            getListItemValue(int which) const override;
    griditemtype_t getListItemType(int which) const override;
    str            getListItemString(int which) const override;
    void           DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont) override;
    qboolean       IsHeaderEntry() const override;
};

static UIFAKKLoadGameClass *loadgame_ui = NULL;

Event EV_FAKKLoadGame_LoadGame
(
    "loadgame",
    EV_DEFAULT,
    NULL,
    NULL,
    "Load the currently selected game"
);
Event EV_FAKKLoadGame_RemoveGame
(
    "removegame",
    EV_DEFAULT,
    NULL,
    NULL,
    "Delete the currently selected game"
);
Event EV_FAKKLoadGame_DeleteGame
(
    "deletegame",
    EV_DEFAULT,
    NULL,
    NULL,
    "Delete the currently selected game... for real"
);
Event EV_FAKKLoadGame_NoDeleteGame
(
    "nodeletegame",
    EV_DEFAULT,
    NULL,
    NULL,
    "Delete the currently selected game... for real"
);
Event EV_FAKKLoadGame_SaveGame
(
    "savegame",
    EV_DEFAULT,
    NULL,
    NULL,
    "Save the currently selected game"
);

CLASS_DECLARATION(UIListCtrl, UIFAKKLoadGameClass, NULL) {
    {&EV_UIListBase_ItemSelected,      &UIFAKKLoadGameClass::SelectGame  },
    {&EV_UIListBase_ItemDoubleClicked, &UIFAKKLoadGameClass::LoadGame    },
    {&EV_FAKKLoadGame_RemoveGame,      &UIFAKKLoadGameClass::RemoveGame  },
    {&EV_FAKKLoadGame_DeleteGame,      &UIFAKKLoadGameClass::DeleteGame  },
    {&EV_FAKKLoadGame_NoDeleteGame,    &UIFAKKLoadGameClass::NoDeleteGame},
    {&EV_FAKKLoadGame_LoadGame,        &UIFAKKLoadGameClass::LoadGame    },
    {&EV_FAKKLoadGame_SaveGame,        &UIFAKKLoadGameClass::SaveGame    },
    {NULL,                             NULL                              }
};

UIFAKKLoadGameClass::UIFAKKLoadGameClass()
{
    Connect(this, EV_UIListBase_ItemDoubleClicked, EV_UIListBase_ItemDoubleClicked);
    Connect(this, EV_UIListBase_ItemSelected, EV_UIListBase_ItemSelected);
    AllowActivate(true);
    m_bRemovePending = false;

    setHeaderFont("facfont-20");
    loadgame_ui = this;
}

UIFAKKLoadGameClass::~UIFAKKLoadGameClass()
{
    loadgame_ui = NULL;
}

void UIFAKKLoadGameClass::UpdateUIElement(void)
{
    float width;

    RemoveAllColumns();

    width = getClientFrame().size.width;
    AddColumn(Sys_LV_CL_ConvertString("Mission"), 0, width * 0.555, false, false);
    AddColumn(Sys_LV_CL_ConvertString("Elapsed Time"), 1, width * 0.17f, true, true);
    AddColumn(Sys_LV_CL_ConvertString("Date & Time Logged"), 2, width * 0.275f, true, true);

    uWinMan.ActivateControl(this);

    SetupFiles();
}

void UIFAKKLoadGameClass::SetupFiles(void)
{
    char      **filenames;
    int         numfiles;
    int         i;
    const char *searchFolder = Com_GetArchiveFolder();

    // cleanup
    DeleteAllItems();

    filenames = FS_ListFiles(searchFolder, "ssv", qfalse, &numfiles);

    for (i = 0; i < numfiles; i++) {
        const char      *filename;
        str              work;
        str              gametime;
        str              date;
        fileHandle_t     f;
        savegamestruct_t save;

        filename = filenames[i];
        work     = searchFolder;
        work += "/";
        work += filename;

        FS_FOpenFileRead(work, &f, qfalse, qtrue);
        if (!f) {
            continue;
        }

        FS_Read(&save, sizeof(savegamestruct_t), f);
        FS_FCloseFile(f);
        Com_SwapSaveStruct(&save);

        if (save.version != SAVEGAME_STRUCT_VERSION) {
            // wrong save game version
            continue;
        }

        if (save.type != com_target_game->integer) {
            continue;
        }

        gametime = (save.mapTime / 1000);
        date     = save.time;

        AddItem(new FAKKLoadGameItem(save.comment, gametime, date, save.saveName));
    }

    FS_FreeFileList(filenames);

    // sort by date
    SortByColumn(2);

    // select the first item
    TrySelectItem(1);
    SelectGame(NULL);
}

void UIFAKKLoadGameClass::SelectGame(Event *ev)
{
    UIWidget   *wid;
    const char *shotName;

    if (getCurrentItem() > 0) {
        shotName = Com_GetArchiveFileName(GetItem(getCurrentItem())->getListItemString(3), "tga");
    } else {
        shotName = "textures/menu/no_saved_games.tga";
    }

    wid = findSibling("LoadSaveShot");
    if (!wid) {
        return;
    }

    wid->setMaterial(uWinMan.RefreshShader(shotName));
}

void UIFAKKLoadGameClass::RemoveGame(Event *ev)
{
    if (m_bRemovePending || getCurrentItem() <= 0) {
        return;
    }

    Cbuf_ExecuteText(
        EXEC_NOW,
        "dialog \"\" \"\" \"widgetcommand LoadSaveList deletegame\" \"widgetcommand LoadSaveList nodeletegame\" 256 64 "
        "confirm_delete menu_button_trans menu_button_trans\n"
    );
    m_bRemovePending = true;
}

void UIFAKKLoadGameClass::NoDeleteGame(Event *ev)
{
    m_bRemovePending = false;
}

void UIFAKKLoadGameClass::DeleteGame(Event *ev)
{
    str     name;
    cvar_t *var;
    m_bRemovePending = false;

    if (getCurrentItem() <= 0) {
        return;
    }

    name = GetItem(getCurrentItem())->getListItemString(3);
    var  = Cvar_Get("g_lastsave", "", 0);

    if (!strcmp(name, var->string)) {
        // Make sure the last save is not the save being deleted
        Cvar_Set("g_lastsave", "");
    }

    Com_WipeSavegame(name);
    SetupFiles();
}

void UIFAKKLoadGameClass::LoadGame(Event *ev)
{
    char cmdString[266];
    str  name;

    if (getCurrentItem() <= 0) {
        return;
    }

    name = GetItem(getCurrentItem())->getListItemString(3);
    // Execute the command
    Com_sprintf(cmdString, sizeof(cmdString), "loadgame %s\n", name.c_str());
    Cbuf_AddText(cmdString);
}

void UIFAKKLoadGameClass::SaveGame(Event *ev)
{
    Cbuf_ExecuteText(EXEC_NOW, "savegame");
}

qboolean UIFAKKLoadGameClass::KeyEvent(int key, unsigned int time)
{
    switch (key) {
    case K_DEL:
        RemoveGame(NULL);
        return qtrue;
    case K_ENTER:
    case K_KP_ENTER:
        LoadGame(NULL);
        return qtrue;
    case K_UPARROW:
        if (getCurrentItem() > 1) {
            TrySelectItem(getCurrentItem() - 1);
            SelectGame(NULL);
            return qtrue;
        }
        break;
    case K_DOWNARROW:
        if (getCurrentItem() < getNumItems()) {
            TrySelectItem(getCurrentItem() + 1);
            SelectGame(NULL);
            return qtrue;
        }
        break;
    default:
        return UIListCtrl::KeyEvent(key, time);
    }

    return qfalse;
}

void UI_SetupFiles(void)
{
    if (loadgame_ui && loadgame_ui->getShow()) {
        loadgame_ui->SetupFiles();
    }
}

FAKKLoadGameItem::FAKKLoadGameItem(
    const str& missionName, const str& elapsedTime, const str& dateTime, const str& fileName
)
{
    strings[0] = missionName;
    strings[1] = elapsedTime;
    strings[2] = dateTime;
    strings[3] = fileName;
}

int FAKKLoadGameItem::getListItemValue(int which) const
{
    return atoi(strings[which]);
}

griditemtype_t FAKKLoadGameItem::getListItemType(int which) const
{
    return griditemtype_t::TYPE_STRING;
}

str FAKKLoadGameItem::getListItemString(int which) const
{
    str itemstring;

    switch (which) {
    case 0:
    case 3:
        itemstring = strings[which];
        break;
    case 1:
        {
            int numseconds;
            int numseconds_hours;
            int seconds;

            // hours
            numseconds = atol(strings[1]);
            itemstring += (numseconds / 3600);
            itemstring += ":";

            // minutes
            numseconds_hours = numseconds % 3600;
            if (numseconds_hours / 60 < 10) {
                itemstring += "0";
            }
            itemstring += (numseconds_hours / 60);
            itemstring += ":";

            // seconds
            seconds = numseconds_hours % 60;
            if (seconds < 10) {
                itemstring += "0";
            }
            itemstring += seconds;
        }
        break;
    case 2:
        {
            time_t time;
            char   buffer[2048];

            time = atol(strings[2]);
            strftime(buffer, sizeof(buffer), "%a %b %d %Y %H:%M:%S", localtime(&time));
            itemstring = buffer;
        }
        break;
    }

    return itemstring;
}

void FAKKLoadGameItem::DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont) {}

qboolean FAKKLoadGameItem::IsHeaderEntry() const
{
    return qfalse;
}
