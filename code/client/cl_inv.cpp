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
    // FIXME: stub
}

invlistener::invlistener()
{
    // FIXME: stub
}

void invlistener::verify_curitem(void)
{
    // FIXME: stub
}

void invlistener::verify_curtype(void)
{
    // FIXME: stub
}

void invlistener::verify_one_arg(Event *ev)
{
    // FIXME: stub
}

bool invlistener::Load(Script& script)
{
    // FIXME: stub
    return false;
}

bool CL_LoadInventory(const char *filename, inventory_t *inv)
{
    // FIXME: stub
    return false;
}

void invlistener::InvAlign(Event *ev)
{
    // FIXME: stub
}

void invlistener::InvCascade(Event *ev)
{
    // FIXME: stub
}

void invlistener::InvWidth(Event *ev)
{
    // FIXME: stub
}

void invlistener::InvHeight(Event *ev)
{
    // FIXME: stub
}

void invlistener::InvVertOffset(Event *ev)
{
    // FIXME: stub
}

void invlistener::InvHorizOffset(Event *ev)
{
    // FIXME: stub
}

void invlistener::InvSelectSound(Event *ev)
{
    // FIXME: stub
}

void invlistener::InvRejectSound(Event *ev)
{
    // FIXME: stub
}

void invlistener::InvChangeSound(Event *ev)
{
    // FIXME: stub
}

void invlistener::Typedef(Event *ev)
{
    // FIXME: stub
}

void invlistener::OpenBrace(Event *ev)
{
    // FIXME: stub
}

void invlistener::CloseBrace(Event *ev)
{
    // FIXME: stub
}

void invlistener::ButtonShader(Event *ev)
{
    // FIXME: stub
}

void invlistener::HoverShader(Event *ev)
{
    // FIXME: stub
}

void invlistener::SelShader(Event *ev)
{
    // FIXME: stub
}

void invlistener::Background(Event *ev)
{
    // FIXME: stub
}

void invlistener::BackgroundTile(Event *ev)
{
    // FIXME: stub
}

void invlistener::BGShader(Event *ev)
{
    // FIXME: stub
}

void invlistener::SelItemShader(Event *ev)
{
    // FIXME: stub
}

void invlistener::SelItemShaderOnTop(Event *ev)
{
    // FIXME: stub
}

void invlistener::BarShader(Event *ev)
{
    // FIXME: stub
}

void invlistener::Width(Event *ev)
{
    // FIXME: stub
}

void invlistener::Height(Event *ev)
{
    // FIXME: stub
}

void invlistener::BarWidth(Event *ev)
{
    // FIXME: stub
}

void invlistener::BarHeight(Event *ev)
{
    // FIXME: stub
}

void invlistener::BarOffsetY(Event *ev)
{
    // FIXME: stub
}

void invlistener::BarOffsetX(Event *ev)
{
    // FIXME: stub
}

void invlistener::Item(Event *ev)
{
    // FIXME: stub
}

void invlistener::Ammo(Event *ev)
{
    // FIXME: stub
}

void invlistener::RotateOffset(Event *ev)
{
    // FIXME: stub
}

void invlistener::HudRotateOffset(Event *ev)
{
    // FIXME: stub
}

void invlistener::Offset(Event *ev)
{
    // FIXME: stub
}

void invlistener::HudOffset(Event *ev)
{
    // FIXME: stub
}

void invlistener::Command(Event *ev)
{
    // FIXME: stub
}

void invlistener::CheckAmmo(Event *ev)
{
    // FIXME: stub
}

void invlistener::Equip(Event *ev)
{
    // FIXME: stub
}

void invlistener::Model(Event *ev)
{
    // FIXME: stub
}

void invlistener::ModelWindow(Event *ev)
{
    // FIXME: stub
}

void invlistener::HudModel(Event *ev)
{
    // FIXME: stub
}

void invlistener::Anim(Event *ev)
{
    // FIXME: stub
}

void invlistener::HudAnim(Event *ev)
{
    // FIXME: stub
}

void invlistener::Scale(Event *ev)
{
    // FIXME: stub
}

void invlistener::HudScale(Event *ev)
{
    // FIXME: stub
}

void invlistener::Angles(Event *ev)
{
    // FIXME: stub
}

void invlistener::HudAngles(Event *ev)
{
    // FIXME: stub
}

void invlistener::AngleDeltas(Event *ev)
{
    // FIXME: stub
}

void invlistener::HudAngleDeltas(Event *ev)
{
    // FIXME: stub
}

void invlistener::Move(Event *ev)
{
    // FIXME: stub
}

void invlistener::HudMove(Event *ev)
{
    // FIXME: stub
}

void invlistener::HudCompassAngles(Event *ev)
{
    // FIXME: stub
}

void invlistener::HudCompassNeedleAngles(Event *ev)
{
    // FIXME: stub
}

inventory_item_t *CL_GetInvItemByName(inventory_t *inv, const char *name)
{
    // FIXME: stub
    return NULL;
}

qboolean CL_HasInventoryItem(const char *name)
{
    // FIXME: stub
    return qfalse;
}

void CL_AmmoCount(const char *name, int *ammo_count, int *max_ammo_count)
{
    // FIXME: stub
}
