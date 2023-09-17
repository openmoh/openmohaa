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

#include <signal.h>
#include <unistd.h>
#include <execinfo.h>

/*
==================
Sys_PlatformInit_New
==================
*/
void Sys_PlatformInit_New() {

}

/*
==================
Sys_PrepareBackTrace
==================
*/
void Sys_PrepareBackTrace() {
}

/*
==================
Sys_PrintBackTrace
==================
*/
void Sys_PrintBackTrace() {
    void* backtrace_arr[20];
    size_t backtrace_size;

    printf("----\nBacktrace:\n");
    // get void*'s for all entries on the stack
    backtrace_size = backtrace(backtrace_arr, sizeof(backtrace_arr) / sizeof(backtrace_arr[0]));
    backtrace_symbols_fd(backtrace_arr, backtrace_size, STDERR_FILENO);
    printf("----\n");
}

/*
==============
Sys_DebugPrint
==============
*/
void Sys_DebugPrint(const char* message) {
    fprintf(stderr, "%s", message);
}

/*
==============
Sys_PumpMessageLoop
==============
*/
void Sys_PumpMessageLoop(void)
{
}

/*
==================
SetNormalThreadPriority
==================
*/
void SetNormalThreadPriority(void)
{
}

/*
==================
SetBelowNormalThreadPriority
==================
*/
void SetBelowNormalThreadPriority(void)
{
}