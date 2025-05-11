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

#pragma once

#include "../gamespy/goaceng.h"

#define NUM_SERVERLISTS 2

struct FAKKServerListInstance {
    int                     iServerType;
    class UIFAKKServerList *serverList;
};

class UIFAKKServerList : public UIListCtrl
{
    friend class FAKKServerListItem;

public:
    CLASS_PROTOTYPE(UIFAKKServerList);

protected:
    // need a new struct instead of gamespy
    struct GServerListImplementation *m_serverList[NUM_SERVERLISTS];
    bool                              m_bHasList;
    bool                              m_bLANListing;

public:
    bool m_bGettingList[NUM_SERVERLISTS];
    bool m_bUpdatingList;

private:
    // Added in OPM
    //  Servers count
    FAKKServerListInstance m_ServerListInst[NUM_SERVERLISTS];
    int                    m_iServerQueryCount;
    int                    m_iServerTotalCount;
    int                    m_iTotalNumPlayers;
    bool                   m_bDoneUpdating[NUM_SERVERLISTS];
    bool                   m_NeedAdditionalLANSearch;

protected:
    void     SelectServer(Event *ev);
    void     ConnectServer(Event *ev);
    qboolean KeyEvent(int key, unsigned int time) override;
    void     UpdateUIElement(void) override;
    void     RefreshServerList(Event *ev);
    void     RefreshLANServerList(Event *ev);
    void     CancelRefresh(Event *ev);
    void     NewServerList(void);
    void     MakeLANListing(Event *ev);
    void     UpdateServer(Event *ev);
    void     RefreshStatus();

    static int  ServerCompareFunction(const UIListCtrlItem *i1, const UIListCtrlItem *i2, int columnname);
    static void UpdateServerListCallBack(GServerList serverlist, int msg, void *instance, void *param1, void *param2);

public:
    UIFAKKServerList();

    void Draw(void) override;
    void SortByColumn(int column) override;
};
