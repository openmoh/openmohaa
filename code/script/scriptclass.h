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

// scriptclass.h: Script class.

#pragma once

#include "listener.h"

class Archiver;
class ScriptVM;
class GameScript;
class StateScript;

class ScriptClass : public Listener
{
    friend GameScript;
    friend StateScript;

public:
    // script variable
    GameScript *m_Script; // current game script

    // listener variable
    SafePtr<Listener> m_Self; // self

    // thread variable
    ScriptVM *m_Threads; // threads list

public:
    CLASS_PROTOTYPE(ScriptClass);

    void *operator new(size_t size);
    void  operator delete(void *ptr);

    ScriptClass(GameScript *gameScript, Listener *self);
    ScriptClass();
    ~ScriptClass();

    void        StoppedNotify() override;
    void        Archive(Archiver& arc) override;
    void        ArchiveInternal(Archiver& arc);
    static void ArchiveScript(Archiver& arc, ScriptClass **obj);
    void        ArchiveCodePos(Archiver& arc, unsigned char **codePos);

    ScriptThread *CreateThreadInternal(const ScriptVariable& label) override;
    ScriptThread *CreateScriptInternal(const ScriptVariable& label) override;

    void AddThread(ScriptVM *thread);
    void KillThreads(void);
    void RemoveThread(ScriptVM *thread);

    str            Filename();
    unsigned char *FindLabel(str label);
    unsigned char *FindLabel(const_str label);
    const_str      NearestLabel(unsigned char *pos);

    StateScript *GetCatchStateScript(unsigned char *in, unsigned char *& out);

    GameScript *GetScript();
    Listener   *GetSelf();
};
