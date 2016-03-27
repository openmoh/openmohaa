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

// console.h: TU Console.

#include "conevent.h"
#include <scriptmaster.h>
#include "ubersdk.h"

ConsoleEvent g_consoleEvent;

Event EV_ConsoleEvent_Shutdown
	(
	"shutdown",
	EV_CONSOLE,
	NULL,
	NULL,
	"Shutdown the utility."
	);

Event EV_ConsoleEvent_Help
	(
	"help",
	EV_CONSOLE,
	NULL,
	NULL,
	"Show help."
	);

Event EV_ConsoleEvent_Print
	(
	"print",
	EV_CONSOLE,
	"sSSS",
	"arg1 arg2 arg3 ...",
	"Print something in the console"
	);

Event EV_ConsoleEvent_ExecScript
	(
	"script",
	EV_CONSOLE,
	"s",
	"filename",
	"Execute a MorpheusScript file."
	);

Event EV_ConsoleEvent_ScriptStatus
	(
	"scriptstatus",
	EV_CONSOLE,
	NULL,
	NULL,
	"Show script status"
	);

Event EV_ConsoleEvent_PrintThread
	(
	"printthread",
	EV_CONSOLE,
	"i",
	"threadnum",
	"Show thread status"
	);

extern bool udk_shutdown;

void ConsoleEvent::CMD_Help( Event *ev )
{
	g_console->PrintHelp();
}

void ConsoleEvent::CMD_Print( Event *ev )
{
	for( int i = 1; i <= ev->NumArgs(); i++ )
	{
		bi.Printf( "%s ", ev->GetString( i ).c_str() );
	}

	bi.Printf( "\n" );
}

void ConsoleEvent::CMD_Shutdown( Event *ev )
{
	udk_shutdown = true;
}

void ConsoleEvent::CMD_ExecScript( Event *ev )
{
	ScriptThread *Thread = Director.CreateThread( ev->GetString( 1 ), "" );

	if( Thread )
	{
		Thread->DelayExecute();
	}
}

void ConsoleEvent::CMD_ScriptStatus( Event *ev )
{
	Director.PrintStatus();
}

void ConsoleEvent::CMD_PrintThread( Event *ev )
{
	Director.PrintThread( ev->GetInteger( 1 ) );
}

CLASS_DECLARATION( Listener, ConsoleEvent, NULL )
{
	{ &EV_ConsoleEvent_Help,				&ConsoleEvent::CMD_Help },
	{ &EV_ConsoleEvent_Print,				&ConsoleEvent::CMD_Print },
	{ &EV_ConsoleEvent_Shutdown,			&ConsoleEvent::CMD_Shutdown },
	{ &EV_ConsoleEvent_ExecScript,			&ConsoleEvent::CMD_ExecScript },
	{ &EV_ConsoleEvent_ScriptStatus,		&ConsoleEvent::CMD_ScriptStatus },
	{ &EV_ConsoleEvent_PrintThread,			&ConsoleEvent::CMD_PrintThread },
	{ NULL, NULL }
};
