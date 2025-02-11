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

#include "../sys_local.h"

#include <Windows.h>
#include <dbghelp.h>

#ifdef _MSC_VER
#  ifdef _DEBUG_MEM
#    include <crtdbg.h>
#  endif
#endif

void Sys_PlatformInit_New() {
#ifdef _MSC_VER
#  ifdef _DEBUG_MEM
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#  endif
#endif
}

/*
==============
Sys_PrepareBackTrace
==============
*/
void Sys_PrepareBackTrace() {
}

/*
==============
Sys_PrintBackTrace
==============
*/
void Sys_PrintBackTrace() {
    void* backtrace[128];
    SYMBOL_INFO* symbol;
    HANDLE hProcess;
    unsigned int i;
    unsigned short frames;

    hProcess = GetCurrentProcess();

    // Initialize the symbol handler
    SymInitialize(hProcess, NULL, TRUE);

    // Gather the list of frames
    frames = CaptureStackBackTrace(0, ARRAY_LEN(backtrace), backtrace, NULL);
    // Allocate a symbol structure to get the name of each symbol
    // with a maximum of 127 characters per symbol
    symbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + 128 * sizeof(char));
    symbol->MaxNameLen = 127;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (i = 0; i < frames; i++)
    {
        // Try to translate the address to a symbol with a name
        SymFromAddr(hProcess, (DWORD64)(backtrace[i]), 0, symbol);

        fprintf(stderr, "=> %s[0x%p]\n", symbol->Name, (void*)symbol->Address);
    }

    free(symbol);
}

/*
==============
Sys_DebugPrint
==============
*/
void Sys_DebugPrint(const char* message) {
    OutputDebugStringA(message);
}

/*
==============
Sys_PumpMessageLoop
==============
*/
void Sys_PumpMessageLoop(void)
{
    MSG msg;

    while (PeekMessageA(&msg, 0, 0, 0, 0))
    {
        if (!GetMessageA(&msg, 0, 0, 0)) {
            Com_Quit_f();
        }

        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

/*
==================
SetNormalThreadPriority
==================
*/
void SetNormalThreadPriority(void)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
}

/*
==================
SetBelowNormalThreadPriority
==================
*/
void SetBelowNormalThreadPriority(void)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
}
