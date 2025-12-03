/*
===========================================================================
Copyright (C) 2025 the OpenMoHAA team

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

#include "consoleevent.h"
#include "g_local.h"

//===============================
// ConsoleEvent
//===============================

MEM_BlockAlloc<ConsoleEvent> ConsoleEvent_allocator;

CLASS_DECLARATION(Event, ConsoleEvent, NULL) {
    {NULL, NULL}
};

/*
=======================
new ConsoleEvent
=======================
*/
void *ConsoleEvent::operator new(size_t size)
{
    return ConsoleEvent_allocator.Alloc();
}

/*
=======================
delete ptr
=======================
*/
void ConsoleEvent::operator delete(void *ptr)
{
    ConsoleEvent_allocator.Free(ptr);
}

/*
=======================
ConsoleEvent
=======================
*/
ConsoleEvent::ConsoleEvent(void)
{
    m_consoleedict = NULL;
}

/*
=======================
SetConsoleEdict
=======================
*/
void ConsoleEvent::SetConsoleEdict(gentity_t *edict)
{
    m_consoleedict = edict;
}

/*
=======================
GetConsoleEdict
=======================
*/
gentity_t *ConsoleEvent::GetConsoleEdict(void) const
{
    if (m_consoleedict) {
        return m_consoleedict;
    }

    return g_entities;
}

/*
=======================
ErrorInternal
=======================
*/
void ConsoleEvent::ErrorInternal(Listener *l, str text) const
{
    gentity_t *edict     = GetConsoleEdict();
    str        eventname = getName();

    gi.DPrintf(
        "^~^~^ Game ( Event '%s', Client '%s' ) : %s\n",
        eventname.c_str(),
        edict->client ? edict->client->pers.netname : "",
        text.c_str()
    );

    gi.SendServerCommand(
        GetConsoleEdict() - g_entities, "print \"Console: '%s' : %s\n\"", eventname.c_str(), text.c_str()
    );
}
