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

#include "ui_local.h"

CLASS_DECLARATION(UILanGameList, UIGlobalGameList, NULL) {
    {NULL, NULL}
};

void UIGlobalGameList::UpdateServers(void)
{
    int i;

    if (!m_noservers_wid) {
        AddNoServer();
    }

    setShow(false);

    if (m_iPrevNumServers != cls.numglobalservers && cls.numglobalservers >= 0) {
        m_iPrevNumServers = cls.numglobalservers;
        DestroyServerWidgets();

        for (i = 0; i < cls.numglobalservers; i++) {
            m_servers.AddUniqueObject(&cls.globalServers[i]);
        }

        m_Vscroll->setPageHeight((getSize().height - 16) / 16.f);
        m_Vscroll->setNumItems(m_servers.NumObjects());
        m_Vscroll->InitFrameAlignRight(this, 0, 0);
        CreateServerWidgets();

        m_Hscroll->setPageWidth(1);
        m_Hscroll->setNumItems(m_iNumColumns);
        m_Hscroll->InitFrameAlignRight(this);
    } else if (!cls.numlocalservers) {
        DrawNoServers(getFrame());
    }
}
