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
#include "cl_uimaprotationsetup.h"

Event EV_MapListBox_RefreshMapList
(
    "RefreshMapList",
    EV_DEFAULT,
    NULL,
    NULL,
    "Refreshes the map list."
);

Event EV_MapRotationListBox_RefreshRotationList
(
    "RefreshRotationList",
    EV_DEFAULT,
    NULL,
    NULL,
    "Refreshed the map rotation list."
);

CLASS_DECLARATION(UIListBox, UIMapRotationListBox, NULL) {
    {&EV_MapRotationListBox_RefreshRotationList, &UIMapRotationListBox::RefreshRotationList},
    {NULL,                                       NULL                                      }
};

UIMapRotationListBox::UIMapRotationListBox() {}

void UIMapRotationListBox::RefreshRotationList(Event *ev)
{
    DeleteAllItems();
    PopulateRotationList();
}

void UIMapRotationListBox::PopulateRotationList()
{
    cvar_t     *filter;
    cvar_t     *maplistVar;
    const char *token;
    char        mapBuffer[1024];

    filter = Cvar_Get("g_maprotation_filter", "ffa", 0);
    if (!Q_stricmp(filter->string, "obj")) {
        maplistVar = Cvar_Get("ui_maplist_obj", "", 0);
    } else if (!Q_stricmp(filter->string, "tow")) {
        maplistVar = Cvar_Get("ui_maplist_tow", "", 0);
    } else if (!Q_stricmp(filter->string, "lib")) {
        maplistVar = Cvar_Get("ui_maplist_lib", "", 0);
    } else if (!Q_stricmp(filter->string, "team")) {
        maplistVar = Cvar_Get("ui_maplist_team", "", 0);
    } else if (!Q_stricmp(filter->string, "round")) {
        maplistVar = Cvar_Get("ui_maplist_round", "", 0);
    } else {
        maplistVar = Cvar_Get("ui_maplist_ffa", "", 0);
    }

    Q_strncpyz(mapBuffer, maplistVar->string, sizeof(mapBuffer));

    for (token = strtok(mapBuffer, " /"); token; token = strtok(NULL, " /")) {
        if (!Q_stricmp(token, "dm")) {
            continue;
        }
        if (!Q_stricmp(token, "obj")) {
            continue;
        }
        if (!Q_stricmp(token, "lib")) {
            continue;
        }

        AddItem(token, NULL);
    }
}

CLASS_DECLARATION(UIListBox, UIMapListBox, NULL) {
    {&EV_MapListBox_RefreshMapList, &UIMapListBox::RefreshMapList},
    {NULL,                          NULL                         }
};

UIMapListBox::UIMapListBox() {}

void UIMapListBox::RefreshMapList(Event *ev)
{
    DeleteAllItems();
    PopulateMapList();
}

void UIMapListBox::PopulateMapList()
{
    cvar_t *filter;
    char  **filenames;
    int     numfiles;
    str     path;
    int     i;
    bool    bTugOfWar   = false;
    bool    bObjective  = false;
    bool    bLiberation = false;

    filter = Cvar_Get("g_maprotation_filter", "ffa", 0);
    if (!Q_stricmp(filter->string, "lib")) {
        path        = "maps/lib/";
        bLiberation = true;
    } else if (!Q_stricmp(filter->string, "obj") || !Q_stricmp(filter->string, "tow")) {
        path = "maps/obj/";
        if (!Q_stricmp(filter->string, "tow")) {
            bTugOfWar = true;
        } else {
            bObjective = true;
        }
    } else {
        path = "maps/dm/";
    }

    filenames = FS_ListFiles(path, ".bsp", qfalse, &numfiles);

    for (i = 0; i < numfiles; i++) {
        const char *filename = filenames[i];
        const char *token;
        char        mapName[256];
        char        tokenized[256];
        bool        bHasMP  = false;
        bool        bHasTOW = false;
        bool        bHasLib = false;

        Q_strncpyz(mapName, filename, sizeof(mapName));
        mapName[strlen(mapName) - 4] = 0;

        if (!COM_IsMapValid(mapName)) {
            continue;
        }

        if (bTugOfWar || bLiberation) {
            Q_strncpyz(tokenized, mapName, sizeof(tokenized));

            for (token = strtok(tokenized, "_"); token; token = strtok(NULL, "_")) {
                if (bLiberation) {
                    if (!Q_stricmp(token, "MP")) {
                        bHasMP = true;
                    }
                    if (!Q_stricmp(token, "LIB")) {
                        bHasLib = true;
                    }
                    if (bHasMP && bHasLib) {
                        AddItem(mapName, NULL);
                        break;
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
                        AddItem(mapName, NULL);
                        break;
                    }
                }
            }
        } else if (bObjective) {
            Q_strncpyz(tokenized, mapName, sizeof(tokenized));

            for (token = strtok(tokenized, "_"); token; token = strtok(NULL, "_")) {
                if (!Q_stricmp(token, "obj")) {
                    AddItem(mapName, NULL);
                    break;
                }
            }

            if (!Q_stricmp(mapName, "mp_ship_lib")) {
                AddItem(mapName, NULL);
            }
        } else {
            AddItem(mapName, NULL);
        }
    }

    if (bObjective) {
        path = "maps/lib/";

        filenames = FS_ListFiles(path, ".bsp", qfalse, &numfiles);

        for (i = 0; i < numfiles; i++) {
            const char *filename = filenames[i];
            const char *token;
            char        mapName[256];
            char        tokenized[256];

            Q_strncpyz(mapName, filename, sizeof(mapName));
            mapName[strlen(mapName) - 4] = 0;
            Q_strncpyz(tokenized, mapName, sizeof(tokenized));

            token = strtok(tokenized, "_");
            if (!Q_stricmp(token, "obj") || !Q_stricmp(mapName, "mp_ship_lib")) {
                AddItem(mapName, NULL);
                break;
            }
        }
    }

    if (!bTugOfWar && !bObjective && !bLiberation) {
        int type;

        for (type = 0; type < 2; type++) {
            if (type == 0) {
                path = "maps/obj/";
            } else {
                path = "maps/lib/";
            }

            filenames = FS_ListFiles(path, ".bsp", qfalse, &numfiles);

            for (i = 0; i < numfiles; i++) {
                const char *filename = filenames[i];
                char        mapName[256];

                Q_strncpyz(mapName, filename, sizeof(mapName));
                mapName[strlen(mapName) - 4] = 0;

                if (!COM_IsMapValid(mapName)) {
                    continue;
                }

                if (!Q_stricmp(mapName, "obj_team2") || !Q_stricmp(mapName, "obj_team4")) {
                    continue;
                }

                AddItem(mapName, NULL);
            }
        }
    }
}

CLASS_DECLARATION(UIButton, UIAddToRotationButton, NULL) {
    {&W_LeftMouseUp, &UIAddToRotationButton::Released},
    {NULL,           NULL                            }
};

void UIAddToRotationButton::Released(Event *ev)
{
    UIListBox *mapList;
    UIListBox *rotationList;
    size_t     prefixLength;
    int        i;

    mapList      = static_cast<UIListBox *>(uWinMan.FindWidget("Map List"));
    rotationList = static_cast<UIListBox *>(uWinMan.FindWidget("Rotation List"));

    if (Cvar_Get("g_maprotation_filter", "ffa", 0)) {
        prefixLength = strlen("obj/") + 1;
    } else {
        prefixLength = 1;
    }

    if (mapList && rotationList) {
        size_t mapLength = 0;
        int    item      = mapList->getCurrentItem();
        str    currentItemText;

        if (item) {
            currentItemText = mapList->getItemText(item);
        }

        for (i = 1; i <= rotationList->getNumItems(); i++) {
            str text = rotationList->getItemText(i);
            if (text == currentItemText) {
                // Added in OPM
                //  Prevent adding duplicates
                item = 0;
                break;
            }

            mapLength += text.length();
        }

        if (item && prefixLength + mapLength + currentItemText.length() < 256) {
            rotationList->AddItem(currentItemText, NULL);
        }
    }

    UIButtonBase::Released(ev);
}

CLASS_DECLARATION(UIButton, UIRemoveFromRotationButton, NULL) {
    {&W_LeftMouseUp, &UIRemoveFromRotationButton::Released},
    {NULL,           NULL                                 }
};

void UIRemoveFromRotationButton::Released(Event *ev)
{
    UIListBox *list;

    list = static_cast<UIListBox *>(uWinMan.FindWidget("Rotation List"));
    if (list) {
        list->DeleteItem(list->getCurrentItem());
    }

    UIButtonBase::Released(ev);
}

CLASS_DECLARATION(UIButton, UIRotationApplyButton, NULL) {
    {&W_LeftMouseUp, &UIRotationApplyButton::Released},
    {NULL,           NULL                            }
};

void UIRotationApplyButton::Released(Event *ev)
{
    UIListBox *list;
    cvar_t    *filter;
    str        cvarName;
    str        maplistStr;
    int        i;
    bool       bObjective  = false;
    bool       bLiberation = false;

    filter = Cvar_Get("g_maprotation_filter", "ffa", 0);
    if (!Q_stricmp(filter->string, "obj")) {
        cvarName   = "ui_maplist_obj";
        bObjective = true;
    } else if (!Q_stricmp(filter->string, "tow")) {
        cvarName   = "ui_maplist_obj";
        bObjective = true;
    } else if (!Q_stricmp(filter->string, "lib")) {
        cvarName    = "ui_maplist_lib";
        bLiberation = true;
    } else if (!Q_stricmp(filter->string, "team")) {
        cvarName = "ui_maplist_team";
    } else if (!Q_stricmp(filter->string, "round")) {
        cvarName = "ui_maplist_round";
    } else {
        cvarName = "ui_maplist_ffa";
    }
    Cvar_Set(cvarName, "");

    list = static_cast<UIListBox *>(uWinMan.FindWidget("Rotation List"));
    if (list) {
        for (i = 1; i <= list->getNumItems(); i++) {
            const char *token;
            str         text = list->getItemText(i);
            char        map[256];
            bool        bHasTOW = false, bHasObj = false, bHasLib = false, bHasShip = false;

            Q_strncpyz(map, text, sizeof(map));
            for (token = strtok(map, "_"); token; token = strtok(NULL, "_")) {
                if (!Q_stricmp(token, "TOW")) {
                    bHasTOW = true;
                    break;
                }
                if (!Q_stricmp(token, "obj")) {
                    bHasObj = true;
                    break;
                }
                if (!Q_stricmp(token, "lib")) {
                    bHasLib = true;
                    break;
                }
                if (!Q_stricmp(token, "ship")) {
                    bHasShip = true;
                    break;
                }
            }

            if (bHasShip) {
                maplistStr += "lib/";
            } else if (bObjective || bHasTOW || bHasObj) {
                maplistStr += "obj/";
            } else if (bHasLib || bLiberation) {
                maplistStr += "lib/";
            } else {
                maplistStr += "dm/";
            }

            maplistStr += text;

            if (i == 1) {
                Cvar_Set("ui_dmmap", maplistStr);
            }

            maplistStr += " ";
        }
    }

    Cvar_Set(cvarName, maplistStr);

    UIButtonBase::Released(ev);
}

CLASS_DECLARATION(UIButton, UIRotationMoveItemUpButton, NULL) {
    {&W_LeftMouseUp, &UIRotationMoveItemUpButton::Released},
    {NULL,           NULL                                 }
};

void UIRotationMoveItemUpButton::Released(Event *ev)
{
    UIListBox *list;
    int        item;

    list = static_cast<UIListBox *>(uWinMan.FindWidget("Rotation List"));
    item = list->getCurrentItem();

    if (item > 1 && list->getNumItems() > 1) {
        str text = list->getItemText(item - 1);

        list->DeleteItem(item - 1);
        list->InsertItem(text, item);
        list->setCurrentItem(item - 1);
    }

    UIButtonBase::Released(ev);
}

CLASS_DECLARATION(UIButton, UIRotationMoveItemDownButton, NULL) {
    {&W_LeftMouseUp, &UIRotationMoveItemDownButton::Released},
    {NULL,           NULL                                   }
};

void UIRotationMoveItemDownButton::Released(Event *ev)
{
    UIListBox *list;
    int        item;

    list = static_cast<UIListBox *>(uWinMan.FindWidget("Rotation List"));
    item = list->getCurrentItem();

    if (item < list->getNumItems() && list->getNumItems() > 1) {
        str text = list->getItemText(item + 1);

        list->DeleteItem(item + 1);
        list->InsertItem(text, item);
        list->setCurrentItem(item + 1);
    }

    UIButtonBase::Released(ev);
}
