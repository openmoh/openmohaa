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

// cl_consolecmds.cpp : New client console commands.

#include "client.h"
#include "glb_local.h"

/*
===============
CL_EventList_f
===============
*/
void CL_EventList_f( void )
{ 
	const char *mask = NULL;

	if( Cmd_Argc() > 1 ) {
		mask = Cmd_Argv( 1 );
	}

	Event::ListCommands( mask );
}

/*
===============
CL_EventHelp_f
===============
*/
void CL_EventHelp_f( void )
{
	const char *mask = NULL;

	if( Cmd_Argc() > 1 ) {
		mask = Cmd_Argv( 1 );
	}

	Event::ListDocumentation( mask, qfalse );
}

/*
===============
CL_DumpEventHelp_f
===============
*/
void CL_DumpEventHelp_f( void )
{
	const char *mask = NULL;

	if( Cmd_Argc() > 1 ) {
		mask = Cmd_Argv( 1 );
	}

	Event::ListDocumentation( mask, qtrue );
}

/*
===============
CL_PendingEvents_f
===============
*/
void CL_PendingEvents_f( void )
{
	const char *mask = NULL;

	if( Cmd_Argc() > 1 ) {
		mask = Cmd_Argv( 1 );
	}

	Event::PendingEvents( mask );
}

/*
===============
CL_ClassList_f
===============
*/
void CL_ClassList_f( void )
{
	listAllClasses();
}

/*
===============
CL_ClassTree_f
===============
*/
void CL_ClassTree_f( void )
{
	if( Cmd_Argc() < 2 ) {
		Com_Printf( "Syntax: cl_classtree [classname].\n" );
		return;
	}

	listInheritanceOrder( Cmd_Argv( 1 ) );
}

/*
===============
CL_ClassEvents_f
===============
*/
void CL_ClassEvents_f( void )
{
	if( Cmd_Argc() < 2 ) {
		Com_Printf( "Syntax: cl_classevents [classname].\n" );
		return;
	}

	ClassEvents( Cmd_Argv( 1 ), qfalse );
}

/*
===============
CL_DumpClassEvents_f
===============
*/
void CL_DumpClassEvents_f( void )
{
	if( Cmd_Argc() < 2 ) {
		Com_Printf( "Syntax: cl_dumpclassevents [classname].\n" );
		return;
	}

	ClassEvents( Cmd_Argv( 1 ), qtrue );
}

/*
===============
CL_DumpAllClasses_f
===============
*/
void CL_DumpAllClasses_f( void )
{
	DumpAllClasses();
}

/*
===============
CL_InitConsoleCommands
===============
*/
void CL_InitConsoleCommands( void )
{
	Cmd_AddCommand( "cl_eventlist", CL_EventList_f );
	Cmd_AddCommand( "cl_eventhelp", CL_EventHelp_f );
	Cmd_AddCommand( "cl_dumpevents", CL_DumpEventHelp_f );
	Cmd_AddCommand( "cl_pendingevents", CL_PendingEvents_f );
	Cmd_AddCommand( "cl_classlist", CL_ClassList_f );
	Cmd_AddCommand( "cl_classtree", CL_ClassTree_f );
	Cmd_AddCommand( "cl_classevents", CL_ClassEvents_f );
	Cmd_AddCommand( "cl_dumpclassevents", CL_DumpClassEvents_f );
	Cmd_AddCommand( "cl_dumpallclasses", CL_DumpAllClasses_f );
}
