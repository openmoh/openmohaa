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

typedef struct {
    const char *string;
    int         value;
} equipment_event_table_t;

static equipment_event_table_t s_equipTable[] = {
    {"left",  1 },
    {"right", 2 },
    {"dual",  4 },
    {"use",   8 },
    {"none",  16},
    {NULL,    0 }
};

Event evi_inv_changesound
(
    "inv_changesound",
    EV_DEFAULT,
    "s",
    "soundname",
    "Set the sound to play when the inventory changes items"
);
Event evi_inv_selectsound
(
    "inv_selectsound",
    EV_DEFAULT,
    "s",
    "soundname",
    "Set the sound to play when an inventory item is selected"
);
Event evi_inv_rejectsound
(
    "inv_rejectsound",
    EV_DEFAULT,
    "s",
    "soundname",
    "Set the sound to play when an invalid inventory item is selected"
);
Event evi_inv_width
(
    "inv_width",
    EV_DEFAULT,
    "i",
    "width",
    "Specifies the width of each of the main inv buttons"
);
Event evi_inv_height
(
    "inv_height",
    EV_DEFAULT,
    "i",
    "height",
    "Specifies the height of each of the main inv buttons"
);
Event evi_inv_vertical_offset
(
    "inv_vert_offset",
    EV_DEFAULT,
    "i",
    "offset",
    "Specifies the vertical offset from the top of the screen for the inventory"
);
Event evi_inv_horizontal_offset
(
    "inv_horiz_offset",
    EV_DEFAULT,
    "i",
    "offset",
    "Specifies the horizontal offset from the right of the screen for the inventory"
);
Event evi_inv_align
(
    "inv_align",
    EV_DEFAULT,
    "s",
    "side",
    "Specifies the horizontal offset from the right of the screen for the inventory"
);
Event evi_inv_cascade
(
    "inv_cascade",
    EV_DEFAULT,
    "s",
    "side",
    "Specifies the side which to cascade inventory items"
);
Event evi_typedef
(
    "typedef",
    EV_DEFAULT,
    "s",
    "type",
    "Specifies which class of item you're editing"
);
Event evi_openbrace
(
    "{", EV_DEFAULT,
    NULL,
    NULL,
    "Open brace, useless"
);
Event evi_closebrace
(
    "}", EV_DEFAULT,
    NULL,
    NULL,
    "Close brace, useless"
);
Event evi_button_shader
(
    "button_shader",
    EV_DEFAULT,
    "s",
    "shader",
    "The shader for the class button"
);
Event evi_hover_shader
(
    "hover_shader",
    EV_DEFAULT,
    "s",
    "shader",
    "The shader for the class button when hovering"
);
Event evi_sel_shader
(
    "sel_shader",
    EV_DEFAULT,
    "s",
    "shader",
    "The shader for the class button when selected"
);
Event evi_bg
(
    "bg",
    EV_DEFAULT,
    "s",
    "shader",
    "Shader for the background of the class submenu"
);
Event evi_bg_tile
(
    "bg_tile",
    EV_DEFAULT,
    NULL,
    NULL,
    "Specifies to tile the background of the class submenu"
);
Event evi_width
(
    "width",
    EV_DEFAULT,
    "i",
    "width",
    "The width of the current item, or default width of all items depending on context"
);
Event evi_height
(
    "height",
    EV_DEFAULT,
    "i",
    "height",
    "The height of the current item, or default width of all items depending on context"
);
Event evi_barwidth
(
    "barwidth",
    EV_DEFAULT,
    "i",
    "width",
    "The width of the current item's ammo bar or default width of all items bar's depending on context"
);
Event evi_barheight(
    "barheight",
    EV_DEFAULT,
    "i",
    "barheight",
    "The height of the current item's ammo bar, or default width of all items bar's depending on context"
);
Event evi_baroffsetx
(
    "baroffsetx",
    EV_DEFAULT,
    "i",
    "width",
    "The x offset used to calculate ammo bar"
);
Event evi_baroffsety
(
    "baroffsety",
    EV_DEFAULT,
    "i",
    "width",
    "The y offset used to calculate ammo bar"
);
Event evi_item
(
    "item",
    EV_DEFAULT,
    "s",
    "item_name",
    "The name of the current item"
);
Event evi_ammo
(
    "ammo",
    EV_DEFAULT,
    "s",
    "ammo_name",
    "The name of the current item's ammo"
);
Event evi_equip
(
    "equip",
    EV_DEFAULT,
    "sSSSSSSSSS",
    "use use use use use use use use use use",
    "Which ways you can use this item"
);
Event evi_checkammo
(
    "checkammo",
    EV_DEFAULT,
    "b",
    "bool",
    "Check if the weapon has ammo before using it"
);
Event evi_command
(
    "command",
    EV_DEFAULT,
    "s",
    "command",
    "Command to execute"
);
Event evi_model
(
    "invmodel",
    EV_DEFAULT,
    "s",
    "name",
    "Which model this item is"
);
Event evi_anim
(
    "invanim",
    EV_DEFAULT,
    "s",
    "anim",
    "Which anim this item uses"
);
Event evi_scale
(
    "invscale",
    EV_DEFAULT,
    "f",
    "scale",
    "How much to scale the model"
);
Event evi_angles
(
    "invangles",
    EV_DEFAULT,
    "v",
    "angles",
    "The orientation of the model"
);
Event evi_angledeltas
(
    "invangledeltas",
    EV_DEFAULT,
    "v",
    "angles",
    "How to spin or bob the model"
);
Event evi_move
(
    "invmove",
    EV_DEFAULT,
    "s",
    "movetype",
    "How to move the model when selected.\n either bob or spin"
);
Event evi_rotateoffset
(
    "invrotateoffset",
    EV_DEFAULT,
    "v",
    "offset",
    "Offsets the origin of the model for rotation"
);
Event evi_offset
(
    "invoffset",
    EV_DEFAULT,
    "v",
    "offset",
    "Offsets the origin of the model for moving"
);
Event evi_hudmodel
(
    "hudmodel",
    EV_DEFAULT,
    "s",
    "name",
    "Which model this item is"
);
Event evi_hudanim
(
    "hudanim",
    EV_DEFAULT,
    "s",
    "anim",
    "Which anim this item uses"
);
Event evi_hudscale
(
    "hudscale",
    EV_DEFAULT,
    "f",
    "scale",
    "How much to scale the model"
);
Event evi_hudangles
(
    "hudangles",
    EV_DEFAULT,
    "v",
    "angles",
    "The orientation of the model"
);
Event evi_hudangledeltas
(
    "hudangledeltas",
    EV_DEFAULT,
    "v",
    "angles",
    "How to spin or bob the model"
);
Event evi_hudmove
(
    "hudmove",
    EV_DEFAULT,
    "s",
    "movetype",
    "How to move the model when selected.\n either bob or spin"
);
Event evi_hudcompassangles
(
    "hudcompassangles",
    EV_DEFAULT,
    NULL,
    NULL,
    "Applies the special case compass angles to the model\n"
);
Event evi_hudcompassneedleangles
(
    "hudcompassneedleangles",
    EV_DEFAULT,
    NULL,
    NULL,
    "Applies the special case compass needle angles to the model\n"
);
Event evi_hudrotateoffset
(
    "hudrotateoffset",
    EV_DEFAULT,
    "v",
    "offset",
    "Offsets the origin of the model for rotation"
);
Event evi_hudoffset
(
    "hudoffset",
    EV_DEFAULT,
    "v",
    "offset",
    "Offsets the origin of the model for moving"
);
Event evi_bgshader
(
    "bgshader",
    EV_DEFAULT,
    "s",
    "shader",
    "Shader to draw on the background of the item widget"
);
Event evi_barshader
(
    "barshader",
    EV_DEFAULT,
    "s",
    "shader",
    "Shader to draw on the background of the item widget to display ammo counts"
);
Event evi_selitemshader
(
    "selitem_shader",
    EV_DEFAULT,
    "s",
    "shader",
    "The shader for an item that is currently selected in the class submenu"
);
Event evi_selitemshaderontop
(
    "selitem_shaderontop",
    EV_DEFAULT,
    NULL,
    NULL,
    "Whether the sel shader should be rendered after the model"
);
Event evi_modelwindow
(
    "modelwindow",
    EV_DEFAULT,
    "ffff",
    "x y width height",
    "Specifies the dimensions of the model window for the inventory in normalized coordinates"
);

CLASS_DECLARATION(Listener, invlistener, NULL) {
    {&evi_inv_width,              &invlistener::InvWidth              },
    {&evi_inv_height,             &invlistener::InvHeight             },
    {&evi_inv_vertical_offset,    &invlistener::InvVertOffset         },
    {&evi_inv_horizontal_offset,  &invlistener::InvHorizOffset        },
    {&evi_inv_selectsound,        &invlistener::InvSelectSound        },
    {&evi_inv_rejectsound,        &invlistener::InvRejectSound        },
    {&evi_inv_changesound,        &invlistener::InvChangeSound        },
    {&evi_inv_align,              &invlistener::InvAlign              },
    {&evi_inv_cascade,            &invlistener::InvCascade            },
    {&evi_typedef,                &invlistener::Typedef               },
    {&evi_openbrace,              &invlistener::OpenBrace             },
    {&evi_closebrace,             &invlistener::CloseBrace            },
    {&evi_button_shader,          &invlistener::ButtonShader          },
    {&evi_hover_shader,           &invlistener::HoverShader           },
    {&evi_sel_shader,             &invlistener::SelShader             },
    {&evi_bg,                     &invlistener::Background            },
    {&evi_bg_tile,                &invlistener::BackgroundTile        },
    {&evi_width,                  &invlistener::Width                 },
    {&evi_height,                 &invlistener::Height                },
    {&evi_barwidth,               &invlistener::BarWidth              },
    {&evi_barheight,              &invlistener::BarHeight             },
    {&evi_baroffsetx,             &invlistener::BarOffsetX            },
    {&evi_baroffsety,             &invlistener::BarOffsetY            },
    {&evi_item,                   &invlistener::Item                  },
    {&evi_ammo,                   &invlistener::Ammo                  },
    {&evi_equip,                  &invlistener::Equip                 },
    {&evi_checkammo,              &invlistener::CheckAmmo             },
    {&evi_command,                &invlistener::Command               },
    {&evi_bgshader,               &invlistener::BGShader              },
    {&evi_barshader,              &invlistener::BarShader             },
    {&evi_selitemshader,          &invlistener::SelItemShader         },
    {&evi_selitemshaderontop,     &invlistener::SelItemShaderOnTop    },
    {&evi_modelwindow,            &invlistener::ModelWindow           },
    {&evi_model,                  &invlistener::Model                 },
    {&evi_anim,                   &invlistener::Anim                  },
    {&evi_move,                   &invlistener::Move                  },
    {&evi_rotateoffset,           &invlistener::RotateOffset          },
    {&evi_offset,                 &invlistener::Offset                },
    {&evi_scale,                  &invlistener::Scale                 },
    {&evi_angles,                 &invlistener::Angles                },
    {&evi_angledeltas,            &invlistener::AngleDeltas           },
    {&evi_hudmodel,               &invlistener::HudModel              },
    {&evi_hudanim,                &invlistener::HudAnim               },
    {&evi_hudmove,                &invlistener::HudMove               },
    {&evi_hudcompassangles,       &invlistener::HudCompassAngles      },
    {&evi_hudcompassneedleangles, &invlistener::HudCompassNeedleAngles},
    {&evi_hudrotateoffset,        &invlistener::HudRotateOffset       },
    {&evi_hudoffset,              &invlistener::HudOffset             },
    {&evi_hudscale,               &invlistener::HudScale              },
    {&evi_hudangles,              &invlistener::HudAngles             },
    {&evi_hudangledeltas,         &invlistener::HudAngleDeltas        },
    {NULL,                        NULL                                }
};

invlistener::invlistener(inventory_t *i)
{
    inv     = i;
    curtype = 0;
}

invlistener::invlistener() {}

void invlistener::verify_curitem(void)
{
    if (!curitem) {
        throw "no current item";
    }
}

void invlistener::verify_curtype(void)
{
    if (!curtype) {
        throw "no current type";
    }
}

void invlistener::verify_one_arg(Event *ev)
{
    if (ev->NumArgs() != 1) {
        throw "bad arg count";
    }
}

bool invlistener::Load(Script& script)
{
    str token;
    str errortext;

    while (script.TokenAvailable(true)) {
        token = script.GetToken(true);

        if (!token.length() || !ValidEvent(token)) {
            throw "invalid token";
        }

        Event *event = new Event(token);

        while (script.TokenAvailable(false)) {
            event->AddToken(script.GetToken(false));
        }

        ProcessEvent(event);
    }

    return true;
}

bool CL_LoadInventory(const char *filename, inventory_t *inv)
{
    Script script;

    Com_Printf("Loading inventory...\n");

    inv->Clear();
    invlistener listener(inv);

    // Load the inventory file
    script.LoadFile(filename);
    return listener.Load(script);
}

inventory_t::inventory_t()
{
    typewidth   = 64;
    typeheight  = 64;
    horizoffset = 0;
    vertoffset  = 0;
    align       = INV_ALIGN_RIGHT;
    cascade     = INV_CASCADE_LEFT;
}

inventory_t::~inventory_t()
{
    Clear();
}

inventory_t::inventory_t(const inventory_t& other)
{
    typewidth   = other.typewidth;
    typeheight  = other.typeheight;
    horizoffset = other.horizoffset;
    vertoffset  = other.vertoffset;
    align       = other.align;
    cascade     = other.cascade;
    rejectsound = other.rejectsound;
    selectsound = other.selectsound;
    changesound = other.changesound;
    types       = other.types;
}

inventory_t& inventory_t::operator=(const inventory_t& other)
{
    Clear();

    typewidth   = other.typewidth;
    typeheight  = other.typeheight;
    horizoffset = other.horizoffset;
    vertoffset  = other.vertoffset;
    align       = other.align;
    cascade     = other.cascade;
    rejectsound = other.rejectsound;
    selectsound = other.selectsound;
    changesound = other.changesound;
    types       = other.types;

    return *this;
}

void inventory_t::Clear()
{
    types.ClearObjectList();
}

void invlistener::InvAlign(Event *ev)
{
    str side;

    verify_one_arg(ev);

    side = ev->GetString(1);

    if (!str::icmp(side, "left")) {
        inv->align = INV_ALIGN_LEFT;
    } else if (!str::icmp(side, "right")) {
        inv->align = INV_ALIGN_RIGHT;
    } else {
        warning(__FUNCTION__, "Invalid align side '%s'\n", side.c_str());
    }
}

void invlistener::InvCascade(Event *ev)
{
    str side;

    verify_one_arg(ev);

    side = ev->GetString(1);

    if (!str::icmp(side, "left")) {
        inv->cascade = INV_CASCADE_LEFT;
    } else if (!str::icmp(side, "right")) {
        inv->cascade = INV_CASCADE_RIGHT;
    } else {
        warning(__FUNCTION__, "Invalid cascade side '%s'\n", side.c_str());
    }
}

void invlistener::InvWidth(Event *ev)
{
    verify_one_arg(ev);

    inv->typewidth = ev->GetInteger(1);
}

void invlistener::InvHeight(Event *ev)
{
    verify_one_arg(ev);

    inv->typeheight = ev->GetInteger(1);
}

void invlistener::InvVertOffset(Event *ev)
{
    verify_one_arg(ev);

    inv->vertoffset = ev->GetInteger(1);
}

void invlistener::InvHorizOffset(Event *ev)
{
    verify_one_arg(ev);

    inv->horizoffset = ev->GetInteger(1);
}

void invlistener::InvSelectSound(Event *ev)
{
    verify_one_arg(ev);

    inv->selectsound = ev->GetString(1);
}

void invlistener::InvRejectSound(Event *ev)
{
    verify_one_arg(ev);

    inv->rejectsound = ev->GetString(1);
}

void invlistener::InvChangeSound(Event *ev)
{
    verify_one_arg(ev);

    inv->changesound = ev->GetString(1);
}

void invlistener::Typedef(Event *ev)
{
    str               type;
    inventory_type_t *t;

    verify_one_arg(ev);

    type = ev->GetString(1);

    t          = new inventory_type_t();
    t->name    = type;
    t->bg_tile = false;

    curtype           = t;
    curitem           = NULL;
    defaultWidth      = 0;
    defaultHeight     = 0;
    defaultBarWidth   = 0;
    defaultBarHeight  = 0;
    defaultBarOffsetX = 0;
    defaultBarOffsetY = 0;

    inv->types.AddObject(t);
}

void invlistener::OpenBrace(Event *ev) {}

void invlistener::CloseBrace(Event *ev) {}

void invlistener::ButtonShader(Event *ev)
{
    verify_one_arg(ev);
    verify_curtype();

    curtype->texture = uWinMan.RegisterShader(ev->GetString(1));
}

void invlistener::HoverShader(Event *ev)
{
    verify_one_arg(ev);
    verify_curtype();

    curtype->hoverTexture = uWinMan.RegisterShader(ev->GetString(1));
}

void invlistener::SelShader(Event *ev)
{
    verify_one_arg(ev);
    verify_curtype();

    curtype->selTexture = uWinMan.RegisterShader(ev->GetString(1));
}

void invlistener::Background(Event *ev)
{
    verify_one_arg(ev);
    verify_curtype();

    curtype->bg = uWinMan.RegisterShader(ev->GetString(1));
}

void invlistener::BackgroundTile(Event *ev)
{
    verify_one_arg(ev);
    verify_curtype();

    curtype->bg      = uWinMan.RegisterShader(ev->GetString(1));
    curtype->bg_tile = true;
}

void invlistener::BGShader(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->bgshader = uWinMan.RegisterShader(ev->GetString(1));
}

void invlistener::SelItemShader(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->selshader = uWinMan.RegisterShader(ev->GetString(1));
}

void invlistener::SelItemShaderOnTop(Event *ev)
{
    verify_curitem();

    curitem->selShaderOnTop = true;
}

void invlistener::BarShader(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->barshader = uWinMan.RegisterShader(ev->GetString(1));
}

void invlistener::Width(Event *ev)
{
    verify_one_arg(ev);

    if (curitem) {
        curitem->width = ev->GetInteger(1);
    } else {
        defaultWidth = ev->GetInteger(1);
    }
}

void invlistener::Height(Event *ev)
{
    verify_one_arg(ev);

    if (curitem) {
        curitem->height = ev->GetInteger(1);
    } else {
        defaultHeight = ev->GetInteger(1);
    }
}

void invlistener::BarWidth(Event *ev)
{
    verify_one_arg(ev);

    if (curitem) {
        curitem->barwidth = ev->GetInteger(1);
    } else {
        defaultBarWidth = ev->GetInteger(1);
    }
}

void invlistener::BarHeight(Event *ev)
{
    verify_one_arg(ev);

    if (curitem) {
        curitem->barheight = ev->GetInteger(1);
    } else {
        defaultBarHeight = ev->GetInteger(1);
    }
}

void invlistener::BarOffsetY(Event *ev)
{
    verify_one_arg(ev);

    if (curitem) {
        curitem->baroffsetY = ev->GetInteger(1);
    } else {
        defaultBarOffsetY = ev->GetInteger(1);
    }
}

void invlistener::BarOffsetX(Event *ev)
{
    verify_one_arg(ev);

    if (curitem) {
        curitem->baroffsetX = ev->GetInteger(1);
    } else {
        defaultBarOffsetX = ev->GetInteger(1);
    }
}

void invlistener::Item(Event *ev)
{
    str               type;
    inventory_item_t *item;

    verify_one_arg(ev);
    verify_curtype();

    item       = new inventory_item_t();
    item->name = ev->GetString(1);

    item->width      = defaultWidth;
    item->height     = defaultHeight;
    item->barwidth   = defaultBarWidth;
    item->barheight  = defaultBarHeight;
    item->baroffsetX = defaultBarOffsetX;
    item->baroffsetY = defaultBarOffsetY;
    item->equip      = 8;

    curtype->items.AddObject(item);
    curitem = curtype->items.ObjectAt(curtype->items.NumObjects());
}

void invlistener::Ammo(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->ammoname = ev->GetString(1);
}

void invlistener::RotateOffset(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->invprops.rotateoffset = ev->GetVector(1);
}

void invlistener::HudRotateOffset(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->hudprops.rotateoffset = ev->GetVector(1);
}

void invlistener::Offset(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->invprops.offset = ev->GetVector(1);
}

void invlistener::HudOffset(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->hudprops.offset = ev->GetVector(1);
}

void invlistener::Command(Event *ev)
{
    verify_curitem();

    if (ev->NumArgs() <= 0) {
        throw "command needs one or more args";
    }

    curitem->command = ev->GetString(1);
}

void invlistener::CheckAmmo(Event *ev)
{
    verify_one_arg(ev);

    curitem->checkammo = ev->GetBoolean(1);
}

void invlistener::Equip(Event *ev)
{
    int i;

    verify_curitem();

    if (ev->NumArgs() <= 0) {
        throw "command needs one or more args";
    }

    curitem->equip = 0;

    for (i = 1; i <= ev->NumArgs(); i++) {
        str equipstr = ev->GetString(i);
        int tab;

        for (tab = 0; s_equipTable[tab].string; tab++) {
            if (equipstr == s_equipTable[tab].string) {
                curitem->equip |= s_equipTable[tab].value;
                break;
            }
        }

        if (!s_equipTable[tab].string) {
            throw "bad equip arguments";
        }
    }
}

void invlistener::Model(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->invprops.model = ev->GetString(1);
}

void invlistener::ModelWindow(Event *ev)
{
    if (ev->NumArgs() != 4) {
        throw "ModelWindow: bad arg count";
    }

    verify_curitem();

    curitem->modelWindowX      = ev->GetFloat(1);
    curitem->modelWindowY      = ev->GetFloat(2);
    curitem->modelWindowWidth  = ev->GetFloat(3);
    curitem->modelWindowHeight = ev->GetFloat(4);
}

void invlistener::HudModel(Event *ev)
{
    verify_one_arg(ev);

    verify_one_arg(ev);
    verify_curitem();

    curitem->hudprops.model = ev->GetString(1);
}

void invlistener::Anim(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->invprops.anim = ev->GetString(1);
}

void invlistener::HudAnim(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->hudprops.anim = ev->GetString(1);
}

void invlistener::Scale(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->invprops.scale = ev->GetFloat(1);
}

void invlistener::HudScale(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->hudprops.scale = ev->GetFloat(1);
}

void invlistener::Angles(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->invprops.angles = ev->GetVector(1);
}

void invlistener::HudAngles(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->hudprops.angles = ev->GetVector(1);
}

void invlistener::AngleDeltas(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->invprops.angledeltas = ev->GetVector(1);
}

void invlistener::HudAngleDeltas(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    curitem->hudprops.angledeltas = ev->GetVector(1);
}

void invlistener::Move(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    str arg = ev->GetString(1);

    if (arg == "bob") {
        curitem->invprops.move = INV_MOVE_BOB;
    } else if (arg == "spin") {
        curitem->invprops.move = INV_MOVE_SPIN;
    } else {
        throw "bad move arguments";
    }
}

void invlistener::HudMove(Event *ev)
{
    verify_one_arg(ev);
    verify_curitem();

    str arg = ev->GetString(1);

    if (arg == "bob") {
        curitem->hudprops.move = INV_MOVE_BOB;
    } else if (arg == "spin") {
        curitem->hudprops.move = INV_MOVE_SPIN;
    } else {
        throw "bad move arguments";
    }
}

void invlistener::HudCompassAngles(Event *ev)
{
    verify_curitem();

    curitem->anglesType = INV_HUDANGLES_COMPASS;
}

void invlistener::HudCompassNeedleAngles(Event *ev)
{
    verify_curitem();

    curitem->anglesType = INV_HUDANGLES_COMPASS_NEEDLE;
}

inventory_item_t *CL_GetInvItemByName(inventory_t *inv, const char *name)
{
    if (!inv) {
        return NULL;
    }

    for (int i = 1; i <= inv->types.NumObjects(); i++) {
        const inventory_type_t *type = inv->types.ObjectAt(i);

        for (int ii = 1; ii <= type->items.NumObjects(); ii++) {
            inventory_item_t *item = type->items.ObjectAt(i);

            if (!item) {
                return NULL;
            }

            if (!str::icmp(item->name, name)) {
                return item;
            }
        }
    }

    return NULL;
}

qboolean CL_HasInventoryItem(const char *name)
{
    int i;

    for (i = 2; i < 6; i++) {
        int index = cl.snap.ps.activeItems[i];

        if (index > 0) {
            if (!str::icmp(name, CL_ConfigString(CS_WEAPONS + index))) {
                return true;
            }
        }
    }

    return false;
}

void CL_AmmoCount(const char *name, int *ammo_count, int *max_ammo_count)
{
    int i;

    *ammo_count     = 0;
    *max_ammo_count = 0;

    for (i = 0; i < ARRAY_LEN(cl.snap.ps.ammo_name_index); i++) {
        int index = cl.snap.ps.ammo_name_index[i];
        if (index) {
            if (!str::icmp(name, CL_ConfigString(CS_WEAPONS + index))) {
                *ammo_count     = cl.snap.ps.ammo_amount[i];
                *max_ammo_count = cl.snap.ps.max_ammo_amount[i];
                break;
            }
        }
    }
}
