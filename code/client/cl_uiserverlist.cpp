/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

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
#include "../gamespy/goaceng.h"
#include "../gamespy/sv_gamespy.h"

class FAKKServerListItem : public UIListCtrlItem {
    str m_strings[6];
    str m_sVersion;
    bool m_bDifferentVersion;
    bool m_bFavorite;
    bool m_bQueryDone;
    bool m_bQueryFailed;
    int m_iNumPlayers;

public:
    str m_sIP;
    unsigned int m_uiRealIP;
    int m_iPort;
    int m_iGameSpyPort;

public:
    FAKKServerListItem(str string1, str string2, str string3, str string4, str string5, str string6, str ver);

    griditemtype_t getListItemType(int index) const override;
    int getListItemValue(int i) const override;
    str getListItemString(int i) const override;
    void setListItemString(int i, str sNewString);
    void DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont* pFont) override;
    qboolean IsHeaderEntry() const override;
    bool IsQueried() const;
    void SetQueried(bool bIsQueried);
    bool IfQueryFailed() const;
    void SetQueryFailed(bool bFailed);
    void SetNumPlayers(int iNum);
    bool IsFavorite() const;
    void SetFavorite(bool bIsFavorite);
    str GetListItemVersion() const;
    void SetListItemVersion(str sNewVer);
    void SetDifferentVersion(bool bIsDifferentVersion);
    bool IsDifferentVersion() const;
};

void UpdateServerListCallBack(GServerList serverlist, int msg, void* instance, void* param1, void* param2);

static int g_iTotalNumPlayers;
qboolean g_bNumericSort = qfalse;
qboolean g_bReverseSort = qfalse;
qboolean g_NeedAdditionalLANSearch = qfalse;

FAKKServerListItem::FAKKServerListItem(str string1, str string2, str string3, str string4, str string5, str string6, str ver)
{
    m_bFavorite = false;
    m_bQueryDone = false;
    m_bQueryFailed = false;
    m_iNumPlayers = 0;
}

griditemtype_t FAKKServerListItem::getListItemType(int index) const
{
    return griditemtype_t::TYPE_STRING;
}

int FAKKServerListItem::getListItemValue(int i) const
{
    return atoi(m_strings[i]);
}

str FAKKServerListItem::getListItemString(int i) const
{
    return m_strings[i];
}

void FAKKServerListItem::setListItemString(int i, str sNewString)
{
    m_strings[i] = sNewString;
}

void FAKKServerListItem::DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont* pFont)
{
    static cvar_t *pColoringType = Cvar_Get("cl_browserdetailedcolors", "0", CVAR_ARCHIVE);
    
    if (!pColoringType->integer) {
        if (IfQueryFailed() || (IsDifferentVersion() && IsQueried())) {
            if (bSelected) {
                DrawBox(drawRect, UColor(0.2f, 0.0f, 0.0f), 1.0);
                pFont->setColor(UColor(0.9f, 0.0f, 0.0f));
            } else {
                DrawBox(drawRect, UColor(0.1f, 0.0f, 0.0f), 1.0);
                pFont->setColor(UColor(0.55f, 0.0f, 0.0f));
            }
        } else if (IsQueried()) {
            if (bSelected) {
                DrawBox(drawRect, UColor(0.2f, 0.18f, 0.015f), 1.0);
                pFont->setColor(UColor(0.9f, 0.8f, 0.6f));
            } else {
                DrawBox(drawRect, UColor(0.02f, 0.07f, 0.004f), 1.0);
                pFont->setColor(UHudColor);
            }
        } else {
            if (bSelected) {
                DrawBox(drawRect, UColor(0.15f, 0.18f, 0.18f), 1.0);
                pFont->setColor(UColor(0.6f, 0.7f, 0.8f));
            } else {
                DrawBox(drawRect, UColor(0.005f, 0.07f, 0.02f), 1.0);
                pFont->setColor(UColor(0.05f, 0.5f, 0.6f));
            }
        }

        pFont->Print(
            drawRect.pos.x + 1.0,
            drawRect.pos.y,
            getListItemString(iColumn).c_str(),
            -1,
            qfalse
        );
    } else {
        if (IsDifferentVersion()) {
            if (IsQueried()) {
                if (bSelected) {
                    DrawBox(drawRect, UColor(0.25f, 0.0f, 0.0f), 1.0);
                    pFont->setColor(UColor(0.0f, 0.5f, 0.0f));
                } else {
                    DrawBox(drawRect, UColor(0.005f, 0.07f, 0.02f), 1.0);
                    pFont->setColor(UColor(0.0f, 0.35f, 0.0f));
                }
            } else {
                if (bSelected) {
                    DrawBox(drawRect, UColor(0.25f, 0.0f, 0.0f), 1.0);
                    pFont->setColor(UColor(0.5f, 0.6f, 0.7f));
                } else {
                    DrawBox(drawRect, UColor(0.15f, 0.0f, 0.0f), 1.0);
                    pFont->setColor(UColor(0.05f, 0.4f, 0.5f));
                }
            }
        } else if (IsQueried()) {
            if (bSelected) {
                DrawBox(drawRect, UColor(0.2f, 0.18f, 0.015f), 1.0);
                pFont->setColor(UColor(0.9f, 0.8f, 0.6f));
            } else {
                DrawBox(drawRect, UColor(0.02f, 0.07f, 0.005f), 1.0);
                pFont->setColor(UHudColor);
            }
        } else {
            if (bSelected) {
                DrawBox(drawRect, UColor(0.15f, 0.18f, 0.18f), 1.0);
                pFont->setColor(UColor(0.6f, 0.7f, 0.8f));
            } else {
                DrawBox(drawRect, UColor(0.005f, 0.07f, 0.02f), 1.0);
                pFont->setColor(UColor(0.05f, 0.5f, 0.6f));
            }

            if (IfQueryFailed()) {
                DrawBox(
                    UIRect2D(
                        drawRect.pos.x,
                        drawRect.pos.y + drawRect.size.height * 0.5 - 1.0 + drawRect.pos.y,
                        drawRect.size.width,
                        drawRect.size.height * 0.5 - 1.0 + drawRect.pos.y
                    ),
                    URed,
                    0.3f
                );
            }

            pFont->Print(
                drawRect.pos.x + 1.0,
                drawRect.pos.y,
                getListItemString(iColumn).c_str(),
                -1,
                qfalse
            );

            if (IsDifferentVersion()) {
                DrawBox(
                    UIRect2D(
                        drawRect.pos.x,
                        drawRect.pos.y + drawRect.size.height * 0.5 - 1.0 + drawRect.pos.y,
                        drawRect.size.width,
                        1.0
                    ),
                    URed,
                    0.3f
                );
            }
        }
    }
}

qboolean FAKKServerListItem::IsHeaderEntry() const
{
    return m_bFavorite;
}

bool FAKKServerListItem::IsQueried() const
{
    return m_bQueryDone;
}

void FAKKServerListItem::SetQueried(bool bIsQueried)
{
    m_bQueryDone = bIsQueried;
    if (m_bQueryDone) {
        m_bQueryFailed = false;
    } else {
        SetNumPlayers(0);
    }
}

bool FAKKServerListItem::IfQueryFailed() const
{
    return m_bQueryFailed;
}

void FAKKServerListItem::SetQueryFailed(bool bFailed)
{
    m_bQueryFailed = bFailed;
}

void FAKKServerListItem::SetNumPlayers(int iNum)
{
    if (m_iNumPlayers) {
        g_iTotalNumPlayers -= m_iNumPlayers;
    }

    m_iNumPlayers = iNum;
    g_iTotalNumPlayers += iNum;
    Cvar_Set("dm_playercount", va("%d", g_iTotalNumPlayers));
}

bool FAKKServerListItem::IsFavorite() const
{
    return m_bFavorite;
}

void FAKKServerListItem::SetFavorite(bool bIsFavorite)
{
    m_bFavorite = bIsFavorite;
}

str FAKKServerListItem::GetListItemVersion() const
{
    return m_sVersion;
}

void FAKKServerListItem::SetListItemVersion(str sNewVer)
{
    m_sVersion = sNewVer;
}

void FAKKServerListItem::SetDifferentVersion(bool bIsDifferentVersion)
{
    m_bDifferentVersion = bIsDifferentVersion;
}

bool FAKKServerListItem::IsDifferentVersion() const
{
    return m_bDifferentVersion;
}

CLASS_DECLARATION( UIListCtrl, UIFAKKServerList, NULL )
{
	{ NULL, NULL }
};

UIFAKKServerList::UIFAKKServerList()
{
	setBackgroundColor(UWhite, true);
    Connect(this, EV_UIListBase_ItemDoubleClicked, EV_UIListBase_ItemDoubleClicked);
    Connect(this, EV_UIListBase_ItemSelected, EV_UIListBase_ItemSelected);

	AllowActivate(true);
	setHeaderFont("facfont-20");
	m_serverList = NULL;
	m_bHasList = false;
	m_bGettingList = false;
	m_bUpdatingList = false;
	m_bLANListing = false;
	m_iLastSortColumn = 2;
}

void UIFAKKServerList::SelectServer( Event *ev )
{
}

void UIFAKKServerList::ConnectServer( Event *ev )
{
	// FIXME: unimplemented
}

qboolean UIFAKKServerList::KeyEvent( int key, unsigned int time )
{
	// FIXME: unimplemented
	return qfalse;
}

void UIFAKKServerList::UpdateUIElement( void )
{
	// FIXME: unimplemented
}

void UIFAKKServerList::RefreshServerList( Event *ev )
{
    int i;
    FAKKServerListItem* pNewServerItem;

    for (i = 1; i <= getNumItems(); i++) {
        pNewServerItem = static_cast<FAKKServerListItem*>(GetItem(i));
        pNewServerItem->SetQueried(false);
        pNewServerItem->SetQueryFailed(false);
    }

    if (m_serverList) {
        ServerListClear(m_serverList);
    } else {
        NewServerList();
    }

    Cvar_Set("dm_playercount", "0");
    ServerListUpdate(m_serverList, true);
    m_bUpdatingList = true;
}

void UIFAKKServerList::RefreshLANServerList( Event *ev )
{
    int i;
    FAKKServerListItem* pNewServerItem;

    for (i = 1; i <= getNumItems(); i++) {
        pNewServerItem = static_cast<FAKKServerListItem*>(GetItem(i));
        pNewServerItem->SetQueried(false);
        pNewServerItem->SetQueryFailed(false);
    }

    if (m_serverList) {
        ServerListClear(m_serverList);
    } else {
        NewServerList();
    }

    g_NeedAdditionalLANSearch = true;

    Cvar_Set("dm_playercount", "0");
    // Search all LAN servers from port 12300 to 12316
    ServerListLANUpdate(m_serverList, true, 12300, 12316, 1);
}

void UIFAKKServerList::CancelRefresh( Event *ev )
{
	ServerListHalt(m_serverList);
}

void UIFAKKServerList::NewServerList( void )
{
    int iNumConcurrent;
    const char* secret_key;
    const char* game_name;
    cvar_t* pRateCvar = Cvar_Get("rate", "5000", CVAR_ARCHIVE | CVAR_USERINFO);

    game_name = GS_GetCurrentGameName();
    secret_key = GS_GetCurrentGameKey();
    if (pRateCvar->integer <= 3000) {
        iNumConcurrent = 4;
    } else if (pRateCvar->integer <= 5000) {
        iNumConcurrent = 6;
    } else if (pRateCvar->integer <= 25000) {
        iNumConcurrent = 10;
    } else {
        iNumConcurrent = 15;
    }

    m_serverList = ServerListNew(
        game_name,
        game_name,
        secret_key,
        iNumConcurrent,
        &UpdateServerListCallBack,
        1,
        (void*)this
    );
}

void UIFAKKServerList::MakeLANListing( Event *ev )
{
	m_bLANListing = true;
}

void UIFAKKServerList::UpdateServer( Event *ev )
{
	if (m_currentItem <= 0) {
		return;
	}

    FAKKServerListItem* item = (FAKKServerListItem*)GetItem(getCurrentItem());
    ServerListAuxUpdate(m_serverList, item->m_sIP.c_str(), item->m_iGameSpyPort, true, GQueryType::qt_status);
}

int UIFAKKServerList::ServerCompareFunction( const UIListCtrlItem *i1, const UIListCtrlItem *i2, int columnname )
{
    int iCompResult;
    int val1, val2;
    const FAKKServerListItem* fi1 = static_cast<const FAKKServerListItem*>(i1);
    const FAKKServerListItem* fi2 = static_cast<const FAKKServerListItem*>(i2);

    if (fi1->IsFavorite() != fi2->IsFavorite())
    {
        if (fi1->IsFavorite()) {
            iCompResult = -1;
        } else {
            iCompResult = 1;
        }

        if (g_bReverseSort) {
            iCompResult = -iCompResult;
        }
    } else if (fi1->IsQueried() != fi2->IsQueried()) {
        if (fi1->IsQueried()) {
            iCompResult = -1;
        } else {
            iCompResult = 1;
        }
    } else if (fi1->IsDifferentVersion() != fi2->IsDifferentVersion()) {

        if (fi1->IsDifferentVersion()) {
            iCompResult = 1;
        } else {
            iCompResult = -1;
        }
    } else if (fi1->IfQueryFailed() != fi2->IfQueryFailed()) {
        if (fi1->IfQueryFailed()) {
            iCompResult = 1;
        } else {
            iCompResult = -1;
        }
    } else if (g_bNumericSort) {
        val1 = fi1->getListItemValue(columnname);
        val2 = fi2->getListItemValue(columnname);

        if (val1 < val2) {
            iCompResult = -1;
        } else if (val1 > val2) {
            iCompResult = 1;
        } else {
            iCompResult = 0;
        }
    } else {
        iCompResult = str::icmp(fi1->getListItemString(columnname), fi2->getListItemString(columnname));
    }

    if (!iCompResult)
    {
        if (columnname != -2) {
            val1 = fi1->getListItemValue(2);
            val2 = fi2->getListItemValue(2);

            if (val1 < val2) {
                iCompResult = -1;
            } else if (val1 > val2) {
                iCompResult = 1;
            } else {
                iCompResult = 0;
            }
        }
    }

    if (!iCompResult) {
        if (columnname != 4) {
            iCompResult = str::icmp(fi1->getListItemString(4), fi2->getListItemString(4));
        }
    }

    if (!iCompResult) {
        if (columnname != 5) {
            iCompResult = str::icmp(fi1->getListItemString(5), fi2->getListItemString(5));
        }
    }

    if (!iCompResult) {
        if (columnname != 3) {
            val1 = fi1->getListItemValue(3);
            val2 = fi2->getListItemValue(3);

            if (val1 < val2) {
                iCompResult = 1;
            } else if (val1 > val2) {
                iCompResult = -1;
            } else {
                iCompResult = 0;
            }
        }
    }

    if (!iCompResult) {
        if (columnname != 0) {
            iCompResult = str::icmp(fi1->getListItemString(0), fi2->getListItemString(0));
        }
    }

    if (!iCompResult) {
        if (columnname != -1) {
            iCompResult = str::icmp(fi1->getListItemString(1), fi2->getListItemString(1));
        }
    }

    return g_bReverseSort ? -iCompResult : iCompResult;
}

void UIFAKKServerList::Draw( void )
{
	// FIXME: unimplemented
}

void UIFAKKServerList::SortByColumn( int column )
{
    int i;
    bool exists = 0;
    bool numeric = 0;
    bool reverse = 0;
    UIListCtrlItem* selected = NULL;
    bool selvisible = false;

    for (i = 1; i <= m_columnlist.NumObjects(); i++) {
        const columndef_t& def = m_columnlist.ObjectAt(i);
        if (def.name == column) {
            // found one
            numeric = def.numeric;
            reverse = def.reverse_sort;
            exists = true;
            break;
        }
    }

    if (!exists) {
        m_iLastSortColumn = 0;
        return;
    }

    m_iLastSortColumn = column;
    s_qsortcolumn = column;
    s_qsortobject = this;
    s_qsortreverse = reverse;
    g_bNumericSort = numeric;
    g_bReverseSort = reverse;

    if (getCurrentItem()) {
        selected = m_itemlist.ObjectAt(getCurrentItem());
        if (GetScrollBar()) {
            selvisible = getCurrentItem() >= GetScrollBar()->getTopItem() + 1
                      && getCurrentItem() <= GetScrollBar()->getPageHeight() + GetScrollBar()->getTopItem();
        } else {
            selvisible = getCurrentItem() > 0;
        }
    }

    setCompareFunction(&UIFAKKServerList::ServerCompareFunction);
    if (m_itemlist.NumObjects()) {
        m_itemlist.Sort(&UIListCtrl::QsortCompare);
    }

    if (selected) {
        if (selvisible) {
            TrySelectItem(FindItem(selected));
        } else {
            m_currentItem = FindItem(selected);
        }
    }
}

void UpdateServerListCallBack(GServerList serverlist, int msg, void* instance, void* param1, void* param2)
{
    // FIXME: unimplemented
}