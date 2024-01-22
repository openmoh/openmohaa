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

class PMPickerItem : public UIListCtrlItem
{
    str m_string;

public:
    PMPickerItem(const str& string);

    int            getListItemValue(int which) const override;
    griditemtype_t getListItemType(int which) const override;
    str            getListItemString(int which) const override;
    void           DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont) override;
    qboolean       IsHeaderEntry() const override;
};

typedef struct {
    const char *fileName;
    const char *displayName;
} PMPickerModel_t;

static const PMPickerModel_t pickerModels[] = {
  // 2.0 skins
    {"allied_101st_captain",                    "US - 101st Airborne Captain"    },
    {"allied_101st_infantry",                   "US - 101st Airborne Private 1"  },
    {"allied_101st_scout",                      "US - 101st Airborne Private 2"  },
    {"allied_501st_pir_scout",                  "US - 501st Paratrooper 1"       },
    {"allied_501st_pir_soldier",                "US - 501st Paratrooper 2"       },
 // 1.0 skins
    {"allied_airborne",                         "US - 501st Paratrooper 3"       },
 // Readded in OPM (was removed in 2.0)
    {"allied_manon",                            "US - Manon"                     },
 // 2.0 skins
    {"allied_british_6th_airborne_captain",     "UK - 6th Airborne Captain"      },
    {"allied_british_6th_airborne_paratrooper", "UK - 6th Airborne Paratrooper"  },
 // 1.0 skins
    {"allied_pilot",                            "US - Army Pilot"                },
 // 2.0 skins
    {"allied_russian_corporal",                 "RA - Corporal"                  },
    {"allied_russian_crazy_boris",              "RA - Crazy Boris"               },
    {"allied_russian_recon_scout",              "RA - Junior Lieutenant"         },
    {"allied_russian_recon_soldier",            "RA - Black Sea Petty Officer"   },
 // 1.0 skins
    {"allied_sas",                              "UK - SAS Officer"               },
    {"american_army",                           "US - Soldier"                   },
    {"american_ranger",                         "US - Ranger"                    },
 // 1.0 skins
    {"german_afrika_officer",                   "DE - Afrika Officer"            },
    {"german_afrika_private",                   "DE - Afrika Private"            },
 // 2.0 skins
    {"german_ardennes_artillery_commander",     "DE - Artillery Commander"       },
    {"german_dday_colonel",                     "DE - Normandy Colonel"          },
 // 1.0 skins
    {"german_elite_officer",                    "DE - Elite Officer"             },
    {"german_elite_sentry",                     "DE - Elite Sentry"              },
    {"german_kradshutzen",                      "DE - Kradshutzen"               },
    {"german_panzer_grenadier",                 "DE - Panzer Grenadier"          },
    {"german_panzer_obershutze",                "DE - Panzer Obershutze"         },
    {"german_panzer_shutze",                    "DE - Panzer Shutze"             },
    {"german_panzer_tankcommander",             "DE - Panzer Tank Commander"     },
    {"german_scientist",                        "DE - German Scientist"          },
 // Those skins were superseded by german_waffenss
    {"german_waffen_officer",                   "DE - Waffen Officer"            },
    {"german_waffen_shutze",                    "DE - Waffen Shutze"             },
 // 1.0 skins
    {"german_waffenss_officer",                 "DE - Waffen Officer"            },
    {"german_waffenss_shutze",                  "DE - Waffen Shutze"             },
    {"german_wehrmacht_officer",                "DE - Wehrmacht Officer"         },
    {"german_wehrmacht_soldier",                "DE - Wehrmacht Soldier"         },
    {"german_winter_1",                         "DE - Winter Infantry 1"         },
    {"german_winter_2",                         "DE - Winter Infantry 2"         },
    {"german_worker",                           "DE - German Mechanic"           },
 // 2.11 skins
    {"allied_british_tank_corporal",            "UK - 6th Royal Tank Regiment"   },
    {"allied_russian_seaman",                   "RA - Red Navy Seaman"           },
    {"german_Panzer_Corporal",                  "DE - 1st Panzer Regiment"       },
    {"allied_technician",                       "US - Air Force Technician"      },
    {"german_stukageschwader",                  "DE - Stukageschwader"           },
 // 2.30 skins
    {"It_AX_Ital_Vol",                          "IT - 10th Army XXI Corps"       },
    {"SC_AX_Ital_Inf",                          "IT - 6th Army 202nd Coastal Div"},
    {"SC_AX_Ital_Inf2",                         "IT - 114th Inf Regiment"        },
    {"SC_AX_ITAL_PARA",                         "IT - Paracadutista Militare"    },
 // 2.40 skins
    {"allied_Wheathers",                        "US - General Wheathers"         },
    {"allied_US_Tank",                          "US - II Corps Tank Commander"   },
    {"allied_US_Mask",                          "US - II Corps Infantry"         },
    {"allied_british_Cmd",                      "UK - 10th Corps"                },
    {"allied_british_Tank",                     "UK - 8th Army"                  },
    {NULL,                                      NULL                             }
};

const char *PM_FilenameToDisplayname(const char *fileName)
{
    int i;

    for (i = 0; pickerModels[i].fileName; i++) {
        const PMPickerModel_t& model = pickerModels[i];

        if (!Q_stricmp(model.fileName, fileName)) {
            return model.displayName;
        }
    }

    return fileName;
}

const char *PM_DisplaynameToFilename(const char *displayName)
{
    int i;

    for (i = 0; pickerModels[i].fileName; i++) {
        const PMPickerModel_t& model = pickerModels[i];

        if (!Q_stricmp(model.displayName, displayName)) {
            return model.fileName;
        }
    }

    return displayName;
}

CLASS_DECLARATION(USignal, PlayerModelPickerClass, NULL) {
    {&EV_UIListBase_ItemSelected,       &PlayerModelPickerClass::FileSelected },
    {&EV_UIListBase_ItemDoubleClicked,  &PlayerModelPickerClass::FileChosen   },
    {&UIFloatingWindow::W_ClosePressed, &PlayerModelPickerClass::OnDeactivated},
    {&W_Deactivated,                    &PlayerModelPickerClass::OnDeactivated},
    {NULL,                              NULL                                  }
};

PlayerModelPickerClass::PlayerModelPickerClass()
{
    window = new UIFloatingWindow();
    window->Create(
        NULL,
        UIRect2D((cls.glconfig.vidWidth - 400) / 2, (cls.glconfig.vidHeight - 300) / 2, 400, 300),
        "Player Model Select...",
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
    listbox->AddColumn("Player Model", 0, 400, false, false);

    listbox->Connect(this, EV_UIListBase_ItemDoubleClicked, EV_UIListBase_ItemDoubleClicked);
    listbox->Connect(this, EV_UIListBase_ItemSelected, EV_UIListBase_ItemSelected);
    listbox->AllowActivate(true);

    // Added in 2.0
    //  Don't localize elements
    listbox->SetDontLocalize();

    m_bGermanModels = false;
}

PlayerModelPickerClass::~PlayerModelPickerClass()
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

void PlayerModelPickerClass::Setup(const char *root_directory, const char *current_directory, qboolean bGermanModels)
{
    Initialize(root_directory, current_directory, bGermanModels);
}

void PlayerModelPickerClass::Initialize(
    const char *root_directory, const char *current_directory, qboolean bGermanModels
)
{
    rootDirectory   = root_directory;
    m_bGermanModels = bGermanModels;
    if (rootDirectory.length() > 1 && rootDirectory[rootDirectory.length() - 1] != '/') {
        rootDirectory += "/";
    }

    currentDirectory = current_directory;
    if (currentDirectory.length() > 1 && currentDirectory[currentDirectory.length() - 1] != '/') {
        currentDirectory += "/";
    }

    SetupFiles();
}

void PlayerModelPickerClass::SetupFiles(void)
{
    str         mappath;
    char        work[128];
    char      **filenames;
    const char *displayName;
    int         numfiles;
    int         i;
    int         iLen;
    qboolean    bIsGerman;

    // cleanup
    listbox->DeleteAllItems();

    if (m_bGermanModels) {
        window->setTitle("Select Your Axis Player Model");
    } else {
        window->setTitle("Select Your Allied Player Model");
    }

    filenames = FS_ListFiles(currentDirectory, ".tik", qfalse, &numfiles);

    for (i = 0; i < numfiles; i++) {
        Q_strncpyz(work, filenames[i], sizeof(work));

        if (strstr(work, "_fps")) {
            // ignore view models
            continue;
        }

        if (work[0] == '_') {
            // ignore hidden models
            continue;
        }

        // allied_manon readded in OPM
        //if (!Q_stricmpn(work, "allied_manon")) {
        //    continue;
        //}

        bIsGerman = !Q_stricmpn(work, "german", 6u) || !Q_stricmpn(work, "axis", 4u) || !Q_stricmpn(work, "it", 2u)
                 || !Q_stricmpn(work, "sc", 2u);

        if (m_bGermanModels != bIsGerman) {
            continue;
        }

        // strip the extension
        work[strlen(work) - 4] = 0;
        displayName            = PM_FilenameToDisplayname(work);

        listbox->AddItem(new PMPickerItem(displayName));
    }

    FS_FreeFileList(filenames);

    listbox->SortByColumn(0);
}

void PlayerModelPickerClass::FileSelected(
    const str& name, const str& currentDirectory, const str& partialName, const str& fullname
)
{
    FileChosen(name, currentDirectory, partialName, fullname);
}

void PlayerModelPickerClass::FileSelected(Event *ev)
{
    str  name;
    char donotshowssindeorfr[64];
    str  fullname;

    name     = listbox->GetItem(listbox->getCurrentItem())->getListItemString(0);
    fullname = PM_DisplaynameToFilename(name);

    if (!Q_stricmpn(fullname, "german_waffen_", 14)) {
        Q_strncpyz(donotshowssindeorfr, "german_waffenss_", sizeof(donotshowssindeorfr));
        Q_strcat(donotshowssindeorfr, sizeof(donotshowssindeorfr), fullname.c_str() + 14);
    } else {
        Q_strncpyz(donotshowssindeorfr, fullname, sizeof(donotshowssindeorfr));
    }

    FileSelected(name, currentDirectory, fullname, currentDirectory + donotshowssindeorfr);
}

void PlayerModelPickerClass::FileChosen(
    const str& name, const str& currentDirectory, const str& partialName, const str& fullname
)
{
    str sCommand;

    if (m_bGermanModels) {
        sCommand += "ui_dm_playergermanmodel \"" + name;
        sCommand += "\" ; ui_dm_playergermanmodel_set " + partialName;
        sCommand += " ; ui_disp_playergermanmodel " + fullname + ".tik";
    } else {
        sCommand += "ui_dm_playermodel \"" + name;
        sCommand += "\" ; ui_dm_playermodel_set " + partialName;
        sCommand += " ; ui_disp_playermodel " + fullname + ".tik";
    }
    sCommand += "\n";

    Cbuf_AddText(sCommand);
    CloseWindow();
}

void PlayerModelPickerClass::FileChosen(Event *ev)
{
    str  name;
    char donotshowssindeorfr[64];
    str  fullname;

    name     = listbox->GetItem(listbox->getCurrentItem())->getListItemString(0);
    fullname = PM_DisplaynameToFilename(name);

    if (!Q_stricmpn(fullname, "german_waffen_", 14)) {
        Q_strncpyz(donotshowssindeorfr, "german_waffen_", sizeof(donotshowssindeorfr));
        Q_strcat(donotshowssindeorfr, sizeof(donotshowssindeorfr), fullname.c_str() + 14);
    } else {
        Q_strncpyz(donotshowssindeorfr, fullname, sizeof(donotshowssindeorfr));
    }

    FileChosen(name, currentDirectory, fullname, currentDirectory + donotshowssindeorfr);
}

void PlayerModelPickerClass::CloseWindow(void)
{
    PostEvent(EV_Remove, 0);
}

void PlayerModelPickerClass::OnDeactivated(Event *ev)
{
    CloseWindow();
}

PMPickerItem::PMPickerItem(const str& string)
{
    m_string = string;
}

int PMPickerItem::getListItemValue(int which) const
{
    return atoi(m_string);
}

griditemtype_t PMPickerItem::getListItemType(int which) const
{
    return griditemtype_t::TYPE_STRING;
}

str PMPickerItem::getListItemString(int which) const
{
    return m_string;
}

void PMPickerItem::DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont *pFont) {}

qboolean PMPickerItem::IsHeaderEntry() const
{
    return qfalse;
}
