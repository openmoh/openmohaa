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

#pragma once

#include "../corepp/listener.h"
#include "../corepp/mem_blockalloc.h"
#include "g_public.h"

class ConsoleEvent : public Event
{
private:
    gentity_t *m_consoleedict;

public:
    CLASS_PROTOTYPE(ConsoleEvent);

    void *operator new(size_t size);
    void  operator delete(void *ptr);

    ConsoleEvent();

    ConsoleEvent(str name)
        : Event(name)
    {
        m_consoleedict = NULL;
    }

    void       SetConsoleEdict(gentity_t *edict);
    gentity_t *GetConsoleEdict(void) const;

    void ErrorInternal(Listener *l, str text) const override;
};

extern MEM_BlockAlloc<ConsoleEvent> ConsoleEvent_allocator;
