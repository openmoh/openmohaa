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
    bool IsQueried();
    void SetQueried(bool bIsQueried);
    bool IfQueryFailed();
    void SetQueryFailed(bool bFailed);
    void SetNumPlayers(FAKKServerListItem*, int);
    bool IsFavorite();
    void SetFavorite(bool bIsFavorite);
    str GetListItemVersion();
    void SetListItemVersion(str sNewVer);
    void SetDifferentVersion(bool bIsDifferentVersion);
    bool IsDifferentVersion();
};

FAKKServerListItem::FAKKServerListItem(str string1, str string2, str string3, str string4, str string5, str string6, str ver)
{
    // FIXME: unimplemented
}

griditemtype_t FAKKServerListItem::getListItemType(int index) const
{
    // FIXME: unimplemented
    return griditemtype_t::TYPE_STRING;
}

int FAKKServerListItem::getListItemValue(int i) const
{
    // FIXME: unimplemented
    return 0;
}

str FAKKServerListItem::getListItemString(int i) const
{
    // FIXME: unimplemented
    return str();
}

void FAKKServerListItem::setListItemString(int i, str sNewString)
{
    // FIXME: unimplemented
}

void FAKKServerListItem::DrawListItem(int iColumn, const UIRect2D& drawRect, bool bSelected, UIFont* pFont)
{
    // FIXME: unimplemented
}

qboolean FAKKServerListItem::IsHeaderEntry() const
{
    // FIXME: unimplemented
    return qfalse;
}

bool FAKKServerListItem::IsQueried()
{
    // FIXME: unimplemented
    return false;
}

void FAKKServerListItem::SetQueried(bool bIsQueried)
{
    // FIXME: unimplemented
}

bool FAKKServerListItem::IfQueryFailed()
{
    // FIXME: unimplemented
    return false;
}

void FAKKServerListItem::SetQueryFailed(bool bFailed)
{
    // FIXME: unimplemented
}

void FAKKServerListItem::SetNumPlayers(FAKKServerListItem*, int)
{
    // FIXME: unimplemented
}

bool FAKKServerListItem::IsFavorite()
{
    // FIXME: unimplemented
    return false;
}

void FAKKServerListItem::SetFavorite(bool bIsFavorite)
{
    // FIXME: unimplemented
}

str FAKKServerListItem::GetListItemVersion()
{
    // FIXME: unimplemented
    return str();
}

void FAKKServerListItem::SetListItemVersion(str sNewVer)
{
    // FIXME: unimplemented
}

void FAKKServerListItem::SetDifferentVersion(bool bIsDifferentVersion)
{
    // FIXME: unimplemented
}

bool FAKKServerListItem::IsDifferentVersion()
{
    // FIXME: unimplemented
    return false;
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
	// FIXME: unimplemented
}

void UIFAKKServerList::RefreshLANServerList( Event *ev )
{
	// FIXME: unimplemented
}

void UIFAKKServerList::CancelRefresh( Event *ev )
{
	ServerListHalt(m_serverList);
}

void UIFAKKServerList::NewServerList( void )
{
	// FIXME: unimplemented
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
	// FIXME: unimplemented
	return 0;
}

void UIFAKKServerList::Draw( void )
{
	// FIXME: unimplemented
}

void UIFAKKServerList::SortByColumn( int column )
{
	// FIXME: unimplemented
}
