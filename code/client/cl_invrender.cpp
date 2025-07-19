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
#include "../qcommon/tiki.h"

static inventory_t            s_processed_inv;
static SafePtr<FakkInventory> s_main_inv;

Event FakkInventory_Timeout
(
    "_timeout",
    EV_DEFAULT,
    NULL,
    NULL,
    "Timeout of the menu from inactivity"
);

CLASS_DECLARATION(UIWidget, FakkInventory, NULL) {
    {&W_MouseMoved,          &FakkInventory::OnMouseMove },
    {&W_MouseEntered,        &FakkInventory::OnMouseEnter},
    {&W_MouseExited,         &FakkInventory::OnMouseLeave},
    {&W_LeftMouseDown,       &FakkInventory::OnMouseDown },
    {&W_RightMouseDown,      &FakkInventory::OnMouseDown },
    {&FakkInventory_Timeout, &FakkInventory::Timeout     },
    {NULL,                   NULL                        }
};

CLASS_DECLARATION(UIWidget, FakkItemList, NULL) {
    {&W_LeftMouseDown,  &FakkItemList::OnLeftMouseDown },
    {&W_RightMouseDown, &FakkItemList::OnRightMouseDown},
    {&W_LeftMouseUp,    &FakkItemList::OnLeftMouseUp   },
    {&W_RightMouseUp,   &FakkItemList::OnRightMouseUp  },
    {&W_MouseMoved,     &FakkItemList::OnMouseMove     },
    {&W_MouseEntered,   &FakkItemList::OnMouseEnter    },
    {&W_MouseExited,    &FakkItemList::OnMouseLeave    },
    {&W_Destroyed,      &FakkItemList::OnMenuKilled    },
    {NULL,              NULL                           }
};

void CL_Draw3DModel(
    float     x,
    float     y,
    float     w,
    float     h,
    qhandle_t model,
    vec3_t    origin,
    vec3_t    rotateoffset,
    vec3_t    offset,
    vec3_t    angle,
    vec3_t    color,
    str       anim
)
{
    refdef_t    inv_refdef {};
    refEntity_t ent {};
    vec3_t      unprojoffset;
    vec3_t      entaxis[3];

    inv_refdef.fov_x   = 30;
    inv_refdef.fov_y   = 30;
    inv_refdef.x       = x;
    inv_refdef.y       = y;
    inv_refdef.width   = w;
    inv_refdef.rdflags = RDF_HUD | RDF_NOWORLDMODEL;
    inv_refdef.height  = h;
    inv_refdef.time    = cl.serverTime;
    re.ClearScene();

    AnglesToAxis(angle, entaxis);

    MatrixTransformVector(rotateoffset, entaxis, unprojoffset);

    ent.scale        = 1.0;
    ent.hModel       = model;
    ent.entityNumber = 1023;
    VectorAdd(origin, offset, ent.origin);
    VectorAdd(ent.origin, unprojoffset, ent.origin);

    ent.tiki = re.R_Model_GetHandle(model);
    if (!ent.tiki) {
        return;
    }

    if (anim.length()) {
        static qboolean bInited = false;
        static int      hModel[8];
        static int      iUpdateTime[8];
        static int      iIndex[8];
        static float    fAnimTime[8];
        static char     szAnimName[8][256];
        int             i;
        int             iCurrNum;

        if (!bInited) {
            for (i = 0; i < ARRAY_LEN(hModel); i++) {
                iUpdateTime[i] = 0;
                iIndex[i]      = -1;
                hModel[i]      = -1;
            }
            bInited = true;
        }

        for (i = 0; i < ARRAY_LEN(hModel); i++) {
            if (hModel[i] == model) {
                break;
            }
        }

        if (i == ARRAY_LEN(hModel)) {
            iCurrNum = 0;

            // Find a free slot
            for (i = 0; i < ARRAY_LEN(hModel); i++) {
                if (hModel[i] == -1) {
                    iCurrNum = i;
                    break;
                }
            }

            if (i == ARRAY_LEN(hModel)) {
                int iOldestNum, iOldestTime;

                iOldestTime = iUpdateTime[0];
                iOldestNum  = 0;

                for (i = 1; i < ARRAY_LEN(hModel); i++) {
                    if (iUpdateTime[i] < iOldestTime) {
                        iOldestNum  = i;
                        iOldestTime = iUpdateTime[i];
                    }
                }

                iCurrNum = iOldestNum;
            }

            hModel[iCurrNum] = model;
            iIndex[iCurrNum] = -1;
            Q_strncpyz(szAnimName[iCurrNum], anim, sizeof(szAnimName[iCurrNum]));
        } else {
            iCurrNum = i;

            if (!Q_stricmp(szAnimName[iCurrNum], anim)) {
                fAnimTime[iCurrNum] += (cls.realtime - iUpdateTime[i]) * 0.001f;
                iUpdateTime[iCurrNum] = cls.realtime;

                if (TIKI_Anim_Time(ent.tiki, iIndex[iCurrNum]) < fAnimTime[iCurrNum]) {
                    // Added in 2.0
                    //  Clear tunak easter egg
                    Cvar_Set("tunak", "0");
                    iIndex[iCurrNum] = -1;
                }
            } else {
                iIndex[iCurrNum] = -1;
                Q_strncpyz(szAnimName[iCurrNum], anim, sizeof(szAnimName[iCurrNum]));
            }
        }

        if (iIndex[iCurrNum] == -1) {
            iIndex[iCurrNum]      = TIKI_Anim_NumForName(ent.tiki, szAnimName[iCurrNum]);
            fAnimTime[iCurrNum]   = 0;
            iUpdateTime[iCurrNum] = cls.realtime;
        }

        ent.frameInfo[0].index  = iIndex[iCurrNum];
        ent.frameInfo[0].time   = fAnimTime[iCurrNum];
        ent.frameInfo[0].weight = 1.f;
        ent.actionWeight        = 1.f;
    }

    ent.shaderRGBA[0] = color[0] * 255;
    ent.shaderRGBA[1] = color[1] * 255;
    ent.shaderRGBA[2] = color[2] * 255;
    ent.shaderRGBA[3] = color[3] * 255;

    if (cge) {
        vec3_t vViewAngles;

        cge->CG_EyePosition(&inv_refdef.vieworg);
        cge->CG_EyeAngles(&vViewAngles);
        AnglesToAxis(vViewAngles, inv_refdef.viewaxis);

        VectorCopy(ent.origin, unprojoffset);
        MatrixTransformVector(unprojoffset, inv_refdef.viewaxis, ent.origin);
        MatrixMultiply(entaxis, inv_refdef.viewaxis, ent.axis);
        VectorAdd(ent.origin, inv_refdef.vieworg, ent.origin);
    } else {
        AnglesToAxis(angle, ent.axis);
        AxisClear(inv_refdef.viewaxis);
    }

    re.AddRefEntityToScene(&ent, ENTITYNUM_NONE);
    re.RenderScene(&inv_refdef);
}

bool CL_DoIHaveThisItem(const char *name)
{
    return true;
}

static void ProcessType(inventory_type_t *type)
{
    for (int i = 1; i < type->items.NumObjects(); i++) {
        const inventory_item_t *item = type->items.ObjectAt(i);

        if (!CL_DoIHaveThisItem(item->name)) {
            type->items.RemoveObjectAt(i--);
        }
    }
}

static void ProcessInventory()
{
    for (int i = 1; i <= s_processed_inv.types.NumObjects(); i++) {
        inventory_type_t *type = s_processed_inv.types.ObjectAt(i);

        ProcessType(type);
        if (!type->items.NumObjects()) {
            delete type;
            s_processed_inv.types.RemoveObjectAt(i--);
        }
    }
}

void UI_NextInventory()
{
    UI_DoInventory(false);

    if (s_main_inv) {
        s_main_inv->WarpTo(1);
        s_main_inv->NextItem();
    }
}

void UI_PrevInventory()
{
    UI_DoInventory(false);

    if (s_main_inv) {
        s_main_inv->WarpTo(1);
        s_main_inv->PrevItem();
    }
}

void UI_WarpInventory()
{
    UI_DoInventory(0);

    if (Cmd_Argc() != 2) {
        Com_Printf("Usage: warpinv <slot_name>");
    }

    if (s_main_inv) {
        s_main_inv->WarpTo(Cmd_Argv(1));
    }
}

void UI_DoInventory_f()
{
    UI_DoInventory(1);

    if (s_main_inv) {
        s_main_inv->WarpTo(1);
    }
}

void UI_DoInventory(qboolean activate_mouse)
{
    if ((cl.snap.ps.stats[STAT_CINEMATIC] & 1) || s_main_inv) {
        return;
    }

    ProcessInventory();

    if (!s_processed_inv.types.NumObjects()) {
        UI_UpdateCenterPrint("- No inventory -", 1);
        return;
    }

    s_main_inv = new FakkInventory();
    s_main_inv->setInventory(&s_processed_inv);

    if (client_inv.align == INV_ALIGN_RIGHT) {
        s_main_inv->InitFrame(
            NULL,
            client_inv.horizoffset + uid.vidWidth - client_inv.typewidth,
            client_inv.vertoffset,
            client_inv.typewidth,
            client_inv.typeheight * client_inv.types.NumObjects(),
            0
        );
    } else if (client_inv.align == INV_ALIGN_LEFT) {
        s_main_inv->InitFrame(
            NULL,
            client_inv.horizoffset,
            client_inv.vertoffset,
            client_inv.typewidth,
            client_inv.typeheight * client_inv.types.NumObjects(),
            0
        );
    }

    uWinMan.ActivateControl(s_main_inv);

    if (activate_mouse) {
        IN_MouseOn();
    }

    s_main_inv->m_mouse_active = true;
}

qboolean UI_CloseInventory()
{
    menuManager.PassEventToAllWidgets(EV_ClearInvItemReference);

    if (!s_main_inv || s_main_inv->isDying()) {
        return false;
    }

    delete s_main_inv;

    UI_FocusMenuIfExists();
    return true;
}

FakkInventory::FakkInventory()
{
    m_currenttype = NULL;
    AllowActivate(false);
    UIWidget::setBackgroundColor(UColor(), true);
}

FakkInventory::~FakkInventory()
{
    if (!uWinMan.IsDead()) {
        VerifyTypeUp(NULL, false);
    }
}

int FakkInventory::FindFirstItem(int itemindex, int sign)
{
    int i;

    for (i = itemindex; i > 0 && i <= m_currenttype->items.NumObjects(); i += sign) {
        const inventory_item_t *item = m_currenttype->items.ObjectAt(i);

        if (CL_HasInventoryItem(item->name)) {
            break;
        }
    }

    return i;
}

void FakkInventory::ChangeItem(int sign)
{
    int               itemindex;
    int               groupindex;
    inventory_item_t *selitem;

    if (!m_currenttype && !m_currentlist) {
        WarpTo(1);
    }

    itemindex = m_currenttype->IndexOfItemPtr(m_currentlist->getHoverItem());
    if (itemindex) {
        itemindex = FindFirstItem(itemindex + sign, sign);
        if (itemindex <= 0 || itemindex > m_currenttype->items.NumObjects()) {
            groupindex = m_inv->types.IndexOfObject(m_currenttype);
            if (groupindex) {
                groupindex += sign;
                if (groupindex > m_inv->types.NumObjects()) {
                    groupindex = 1;
                } else if (groupindex < 1) {
                    groupindex = m_inv->types.NumObjects();
                }

                VerifyTypeUp(m_inv->types.ObjectAt(groupindex), false);

                if (sign > 0) {
                    itemindex = FindFirstItem(1, sign);
                } else {
                    itemindex = FindFirstItem(m_inv->types.ObjectAt(groupindex)->items.NumObjects(), sign);
                }

                if (itemindex > m_inv->types.ObjectAt(groupindex)->items.NumObjects()) {
                    itemindex = 1;
                }
            } else {
                VerifyTypeUp(m_inv->types.ObjectAt(1), false);
                itemindex = FindFirstItem(1, sign);
            }
        }
    } else {
        itemindex = 1;
    }

    selitem = m_currenttype->items.ObjectAt(itemindex);
    return m_currentlist->VerifyItemUp(selitem, false);
}

void FakkInventory::NextItem(void)
{
    ChangeItem(1);
}

void FakkInventory::PrevItem(void)
{
    ChangeItem(-1);
}

void FakkInventory::Timeout(Event *ev)
{
    UI_CloseInventory();
}

void FakkInventory::PlaySound(invsound_t type)
{
    switch (type) {
    case invsound_t::selectsound:
        S_StartLocalSound(m_inv->selectsound, qfalse);
        break;
    case invsound_t::rejectsound:
        S_StartLocalSound(m_inv->rejectsound, qfalse);
        break;
    case invsound_t::changesound:
        S_StartLocalSound(m_inv->changesound, qfalse);
        break;
    default:
        break;
    }
}

void FakkInventory::Draw(void)
{
    int       i;
    bool      mouse_in;
    UIPoint2D clientPoint;

    if (!m_inv) {
        return;
    }

    clientPoint = UIPoint2D(uid.mouseX, uid.mouseY);
    mouse_in    = uWinMan.FindResponder(clientPoint) == this;
    clientPoint = getLocalPoint(clientPoint);

    for (i = 1; i <= m_inv->types.NumObjects(); i++) {
        inventory_type_t *type;
        float             top;
        UIReggedMaterial *torender;

        type = m_inv->types.ObjectAt(i);
        top  = m_inv->typeheight * (i - 1);
        if (type == m_currenttype) {
            if (mouse_in) {
                torender = type->hoverTexture;
            } else {
                torender = type->selTexture;
            }
        } else {
            torender = type->texture;
        }

        if (torender) {
            uii.Rend_DrawPicStretched(
                0, top, m_inv->typewidth, m_inv->typeheight, 0, 0, 1.f, 1.f, torender->GetMaterial()
            );
        }
    }
}

void FakkInventory::OnMouseDown(Event *ev)
{
    inventory_type_t *oldtype;
    UIPoint2D         p;
    int               i;

    p = MouseEventToClientPoint(ev);
    i = (int)(p.y / m_inv->typeheight);

    if (i + 1 > m_inv->types.NumObjects()) {
        return;
    }

    oldtype = m_currenttype;
    VerifyTypeUp(m_inv->types.ObjectAt(i + 1), true);

    if (m_currentlist && m_currenttype == oldtype) {
        m_currentlist->VerifyItemUp(m_currenttype->items.ObjectAt(1), true);
    }
}

void FakkInventory::OnMouseMove(Event *ev) {}

void FakkInventory::OnMouseLeave(Event *ev) {}

void FakkInventory::OnMouseEnter(Event *ev)
{
    SetHovermaterialActive(true);
}

void FakkInventory::VerifyTypeUp(inventory_type_t *t, qboolean warpmouse)
{
    CancelEventsOfType(FakkInventory_Timeout);
    PostEvent(new Event(FakkInventory_Timeout), 5);

    if (m_currenttype != t) {
        if (m_currenttype) {
            delete m_currenttype;
        }

        m_currenttype = t;
        if (t) {
            m_currentlist = new FakkItemList();
            m_currentlist->setType(m_currenttype);

            switch (m_inv->cascade) {
            case INV_CASCADE_LEFT:
                m_currentlist->Create(m_screenframe.pos.x, m_screenframe.pos.y, this);
                break;
            case INV_CASCADE_RIGHT:
                m_currentlist->Create(m_screenframe.pos.x + m_screenframe.size.width, m_screenframe.pos.y, this);
                break;
            default:
                break;
            }
        }

        if (m_currenttype) {
            PlaySound(invsound_t::changesound);
        }
    }

    if (warpmouse && m_currentlist) {
        int i;
        int height = 0;

        for (i = 1; i <= m_inv->types.NumObjects() && m_inv->types.ObjectAt(i) != t; i++) {
            height += m_inv->typeheight;
        }

        CL_SetMousePos(
            (int)((m_inv->typewidth / 2) + m_screenframe.pos.x),
            (int)((m_inv->typeheight / 2 + height) + m_screenframe.pos.y)
        );
    }
}

void FakkInventory::WarpTo(int slotnum)
{
    int i;

    for (i = slotnum; i > 0 && i <= m_inv->types.NumObjects(); i++) {
        inventory_type_t *t = m_inv->types.ObjectAt(i);
        if (t) {
            VerifyTypeUp(t, true);
        }
    }
}

void FakkInventory::WarpTo(const char *name)
{
    int i;
    int count;

    count = m_inv->types.NumObjects();
    for (i = 1; i <= count; i++) {
        inventory_type_t *t = m_inv->types.ObjectAt(i);

        if (name && str::icmp(t->name, name)) {
            continue;
        }

        VerifyTypeUp(t, true);
        if (name && m_currentlist && m_currenttype) {
            m_currentlist->VerifyItemUp(m_currenttype->items.ObjectAt(1), true);
        }
    }
}

void FakkInventory::setInventory(inventory_t *i)
{
    m_inv = i;
}

qboolean FakkInventory::KeyEvent(int key, unsigned int time)
{
    char        bind[1024];
    const char *cmd;
    const char *name;
    int         index;
    bool        actionitemlist;
    str         command;

    if (cl.snap.ps.stats[STAT_CINEMATIC] & 1) {
        UI_CloseInventory();
        return false;
    }

    if (!m_inv->types.NumObjects()) {
        return true;
    }

    actionitemlist = m_currenttype && m_currentlist && m_currentlist->getHoverItem() != NULL;

    cmd = Key_GetBinding(key);
    if (cmd) {
        Q_strncpyz(bind, cmd, sizeof(bind));
    } else {
        bind[0] = 0;
    }

    if (bind[0]) {
        name = strtok(bind, " ");
        if (!str::icmp(name, "+attackleft")) {
            command = "attackleft";
        } else if (!str::icmp(name, "+attackright")) {
            command = "attackright";
        } else if (!str::icmp(name, "warpinv")) {
            name = strtok(NULL, " ");
        } else {
            if (!str::icmp(name, "inventory")) {
                UI_CloseInventory();
                return qtrue;
            } else if (!str::icmp(name, "nextinv")) {
                NextItem();
                return true;
            } else if (!str::icmp(name, "previnv")) {
                PrevItem();
                return true;
            }
            name = NULL;
        }
    }

    if (actionitemlist) {
        if (m_mouse_active && (key == K_UPARROW || key == K_DOWNARROW) || (key == K_MWHEELUP || key == K_MWHEELDOWN)
            || (name && !str::icmp(name, m_currenttype->name))) {
            inventory_item_t *hoverItem = m_currentlist->getHoverItem();

            if (m_currenttype->items.NumObjects()) {
                index = m_currenttype->IndexOfItemPtr(hoverItem);

                if (!index) {
                    index = 1;
                } else if (key == K_UPARROW || key == K_MWHEELUP) {
                    index--;
                    if (index < 1) {
                        index = m_currenttype->items.NumObjects();
                    }
                } else {
                    index++;
                    if (index > m_currenttype->items.NumObjects()) {
                        index = 1;
                    }
                }

                m_currentlist->VerifyItemUp(m_currenttype->items.ObjectAt(index), true);
                return true;
            }
        } else {
            if (command.length()) {
                if (!str::icmp(command, "attackleft")) {
                    if (m_currentlist) {
                        m_currentlist->OnLeftMouseDown(NULL);
                    }
                } else if (!str::icmp(command, "attackright")) {
                    if (m_currentlist) {
                        m_currentlist->OnRightMouseDown(NULL);
                    }
                }

                PostponeEvent(FakkInventory_Timeout, 5.f);
                return true;
            }

            if (name) {
                WarpTo(name);
                PostponeEvent(FakkInventory_Timeout, 5.f);
                return true;
            }

            CL_Key_ForceCommand(key, time);
            return true;
        }
    }

    if (m_mouse_active && (key == K_UPARROW || key == K_DOWNARROW) || (key == K_MWHEELUP || key == K_MWHEELDOWN)) {
        index = m_inv->types.IndexOfObject(m_currenttype);
        if (!index) {
            index = 1;
        } else if (key == K_UPARROW || key == K_MWHEELUP) {
            index--;
            if (index < 1) {
                index = m_inv->types.NumObjects();
            }
        } else {
            index++;
            if (index > m_inv->types.NumObjects()) {
                index = 1;
            }
        }

        VerifyTypeUp(m_inv->types.ObjectAt(index), true);
        return qtrue;
    }

    if (m_mouse_active && (key == K_RIGHTARROW || key == K_LEFTARROW)) {
        if (!m_currenttype || !m_currentlist) {
            return qtrue;
        }

        if ((m_inv->cascade == INV_CASCADE_LEFT && key == K_LEFTARROW)
            || (m_inv->cascade == INV_CASCADE_RIGHT && key == K_RIGHTARROW)) {
            m_currentlist->VerifyItemUp(m_currenttype->items.ObjectAt(1), true);
            return qtrue;
        }

        PostponeEvent(FakkInventory_Timeout, 5.f);
        return true;
    }

    if (!name) {
        PostponeEvent(FakkInventory_Timeout, 5.f);
        return true;
    }

    WarpTo(name);
    PostponeEvent(FakkInventory_Timeout, 5.f);
    return true;
}

qboolean FakkInventory::isDying(void)
{
    return m_dying;
}

FakkItemList::FakkItemList()
{
    m_hoveritem    = NULL;
    m_lastmenuitem = NULL;
    m_parent       = NULL;
    UIWidget::setBackgroundColor(UColor(), true);
}

FakkItemList::~FakkItemList() {}

void FakkItemList::Create(float x, float y, FakkInventory *parent)
{
    UIRect2D rect;
    int      i;
    float    maxWidth = 0;

    for (i = 1; i <= type->items.NumObjects(); i++) {
        inventory_item_t *item = type->items.ObjectAt(i);
        if (maxWidth <= item->width) {
            maxWidth = item->width;
        }

        rect.size.height += item->height;
    }

    if (client_inv.cascade == INV_CASCADE_RIGHT) {
        rect.pos.x      = x;
        rect.pos.y      = y;
        rect.size.width = maxWidth;

        InitFrame(NULL, rect, 0);
    } else if (client_inv.cascade == INV_CASCADE_LEFT) {
        rect.pos.x      = x - maxWidth;
        rect.pos.y      = y;
        rect.size.width = maxWidth;

        InitFrame(NULL, rect, 0);
    }

    Com_DPrintf("FakkItemList::Create : Invalid cascade for inventory list\n");
    if (parent) {
        m_parent = parent;
    }
}

bool FakkItemList::HasAnyItems(void)
{
    int i;

    for (i = 1; i <= type->items.NumObjects(); i++) {
        inventory_item_t *item = type->items.ObjectAt(i);

        if (CL_HasInventoryItem(item->name)) {
            return true;
        }
    }

    return false;
}

void FakkItemList::Draw(void) {}

void FakkItemList::OnMouseEnter(Event *ev)
{
    SetHovermaterialActive(true);
}

void FakkItemList::OnLeftMouseDown(Event *ev)
{
    char        bind[1024];
    const char *cmd;
    const char *name;

    if ((cl.snap.ps.stats[STAT_CINEMATIC] & 1)) {
        return;
    }

    if (m_parent) {
        m_parent->CancelEventsOfType(FakkInventory_Timeout);
        m_parent->PostEvent(new Event(FakkInventory_Timeout), 5);
    }

    if (ev) {
        cmd = Key_GetBinding(K_MOUSE1);
        if (cmd) {
            Q_strncpyz(bind, cmd, sizeof(bind));
        } else {
            bind[0] = 0;
        }

        if (bind[0]) {
            name = strtok(bind, " ");
            if (str::icmp(name, "+attackleft")) {
                if (!str::icmp(name, "+attackright")) {
                    OnRightMouseDown(NULL);
                }
                return;
            }
        }
    }

    if (!CL_HasInventoryItem(m_lastmenuitem->name) || (m_lastmenuitem->equip & 16)) {
        if (m_parent) {
            m_parent->PlaySound(invsound_t::rejectsound);
        }
        return;
    }

    if (m_lastmenuitem->checkammo) {
        int count;
        int max;

        CL_AmmoCount(m_lastmenuitem->ammoname, &count, &max);

        if (!count) {
            if (m_parent) {
                m_parent->PlaySound(invsound_t::rejectsound);
            }
            return;
        }
    }

    if (m_lastmenuitem->command.length()) {
        uii.Cmd_Stuff(va("%s\n", m_lastmenuitem->command.c_str()));
    } else if (m_lastmenuitem->equip & 4) {
        uii.Cmd_Stuff(va("use \"%s\" dualhand\n", m_lastmenuitem->name.c_str()));
    } else if (m_lastmenuitem->equip & 1) {
        uii.Cmd_Stuff(va("use \"%s\" lefthand\n", m_lastmenuitem->name.c_str()));
    } else if (m_lastmenuitem->equip & 8) {
        uii.Cmd_Stuff(va("use \"%s\"\n", m_lastmenuitem->name.c_str()));
    } else {
        if (m_parent) {
            m_parent->PlaySound(invsound_t::rejectsound);
        }
    }

    if (m_parent) {
        m_parent->PlaySound(invsound_t::selectsound);
    }

    if (m_parent && !m_parent->m_mouse_active) {
        UI_CloseInventory();
    }
}

void FakkItemList::OnRightMouseDown(Event *ev)
{
    char        bind[1024];
    const char *cmd;
    const char *name;

    if ((cl.snap.ps.stats[STAT_CINEMATIC] & 1)) {
        return;
    }

    if (m_parent) {
        m_parent->CancelEventsOfType(FakkInventory_Timeout);
        m_parent->PostEvent(new Event(FakkInventory_Timeout), 5);
    }

    if (ev) {
        cmd = Key_GetBinding(K_MOUSE2);
        if (cmd) {
            Q_strncpyz(bind, cmd, sizeof(bind));
        } else {
            bind[0] = 0;
        }

        if (bind[0]) {
            name = strtok(bind, " ");
            if (str::icmp(name, "+attackright")) {
                if (!str::icmp(name, "+attackleft")) {
                    OnLeftMouseDown(NULL);
                }
                return;
            }
        }
    }

    if (!CL_HasInventoryItem(m_lastmenuitem->name) || (m_lastmenuitem->equip & 16)) {
        if (m_parent) {
            m_parent->PlaySound(invsound_t::rejectsound);
        }
        return;
    }

    if (m_lastmenuitem->checkammo) {
        int count;
        int max;

        CL_AmmoCount(m_lastmenuitem->ammoname, &count, &max);

        if (!count) {
            if (m_parent) {
                m_parent->PlaySound(invsound_t::rejectsound);
            }
            return;
        }
    }

    if (m_lastmenuitem->command.length()) {
        uii.Cmd_Stuff(va("%s\n", m_lastmenuitem->command.c_str()));
    } else if (m_lastmenuitem->equip & 4) {
        uii.Cmd_Stuff(va("use \"%s\" dualhand\n", m_lastmenuitem->name.c_str()));
    } else if (m_lastmenuitem->equip & 1) {
        uii.Cmd_Stuff(va("use \"%s\" righthand\n", m_lastmenuitem->name.c_str()));
    } else if (m_lastmenuitem->equip & 8) {
        uii.Cmd_Stuff(va("use \"%s\"\n", m_lastmenuitem->name.c_str()));
    } else {
        if (m_parent) {
            m_parent->PlaySound(invsound_t::rejectsound);
        }
    }

    if (m_parent) {
        m_parent->PlaySound(invsound_t::selectsound);
    }

    if (m_parent && !m_parent->m_mouse_active) {
        UI_CloseInventory();
    }
}

void FakkItemList::OnLeftMouseUp(Event *ev) {}

void FakkItemList::OnRightMouseUp(Event *ev) {}

void FakkItemList::OnMouseLeave(Event *ev) {}

void FakkItemList::VerifyItemUp(inventory_item_t *item, qboolean warpmouse)
{
    int               i;
    int               height = 0;
    inventory_item_t *ii     = NULL;

    if (m_parent) {
        m_parent->CancelEventsOfType(FakkInventory_Timeout);
        m_parent->PostEvent(new Event(FakkInventory_Timeout), 5);
    }

    m_hoveritem    = item;
    m_lastmenuitem = item;

    if (m_parent) {
        m_parent->PlaySound(changesound);
    }

    if (warpmouse) {
        for (i = 1; i <= type->items.NumObjects(); i++) {
            ii = type->items.ObjectAt(i);
            if (ii == item) {
                break;
            }

            height += ii->height;
        }

        if (ii) {
            CL_SetMousePos(
                (int)((ii->width / 2) + m_screenframe.pos.x), (int)((ii->height / 2 + height) + m_screenframe.pos.y)
            );
        }
    }
}

void FakkItemList::OnMouseMove(Event *ev)
{
    int               i;
    float             top;
    inventory_item_t *newitem;
    UIPoint2D         p;

    top     = 0;
    newitem = NULL;
    p       = MouseEventToClientPoint(ev);

    for (i = 1; i <= type->items.NumObjects(); i++) {
        inventory_item_t *item = type->items.ObjectAt(i);
        if (p.x >= 0 && item->width > p.x && p.y >= top && item->height + top > p.y) {
            newitem = item;
            break;
        }

        top += item->height;
    }

    if (newitem != m_hoveritem) {
        VerifyItemUp(newitem, false);
    }
}

void FakkItemList::OnMenuKilled(Event *ev)
{
    VerifyItemUp(NULL, false);
}

void FakkItemList::setType(inventory_type_t *t)
{
    type = t;
}

void FakkItemList::EquipItem(Event *ev) {}

inventory_item_t *FakkItemList::getHoverItem() const
{
    return m_hoveritem;
}
