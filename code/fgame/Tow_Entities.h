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

#pragma once

#include "Entities.h"

#define TOWOBJECTIVE_SPAWNFLAG_NO_RESPAWN 2

class TOWObjective : public Objective
{
private:
    int         m_iAxisObjNum;
    int         m_iAlliesObjNum;
    eController m_eController;
    eController m_eLeadingController;
    bool        m_bNoRespawnForLeading;
    str         m_sAxisObjName;
    str         m_sAlliesObjName;

public:
    CLASS_PROTOTYPE(TOWObjective);

    TOWObjective();

    void SetController(Event *ev);
    void GetController(Event *ev);
    void TakeOver(Event *ev);
    void SetAlliesObjNum(Event *ev);
    void SetAxisObjNum(Event *ev);
    void OnInitialize(Event *ev);
    void SetCurrent(Event *ev);

    int GetController() const;

    void Archive(Archiver& arc) override;
};

class TOWObjectiveMan
{
private:
    Container<TOWObjective *> m_towObjectives;
    int                       m_iNumAxisObjectives;
    int                       m_iNumAlliesObjectives;

public:
    TOWObjectiveMan();

    void Reset();
    void AddObjective(TOWObjective *objective);
    void TakeOverObjective(eController controller, eController controllingTeam);
};
