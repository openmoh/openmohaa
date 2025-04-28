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

// scriptexception.h: Script Exception

#pragma once

#include "../qcommon/str.h"

class ScriptException
{
public:
    str string;
    int bAbort;
    int bIsForAnim;

private:
    void CreateException(const char *data);

public:
    ScriptException(str text);
    ScriptException(const char *format, ...);
    ScriptException(char *text);

    static int next_abort;
    static int next_bIsForAnim;
};

void Error(const char *format, ...);

#define ScriptDeprecated(function) throw ScriptException(function ": DEPRECATED. DON'T USE IT ANYMORE")
#define ScriptDeprecatedAltMethod(alternative_name) gi.DPrintf("WARNING: %s: is deprecated and has been superseded by the " alternative_name " method\n", __FUNCTION__)
#define ScriptDeprecatedAltVariable(alternative_name) gi.DPrintf("WARNING: %s: is deprecated and has been superseded by the " alternative_name " variable\n", __FUNCTION__)
#define ScriptError                throw ScriptException
