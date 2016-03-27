/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"
#include "../ui/ui_shared.h"

/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f (void) {
	CG_Printf ("(%i %i %i) : %i\n", (int)cg.refdef.vieworg[0],
		(int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2], 
		(int)cg.refdefViewAngles[YAW]);
}

typedef struct {
	char	*cmd;
	void	(*function)(void);
} consoleCommand_t;

static consoleCommand_t	commands[] = {
	{ "viewpos", CG_Viewpos_f },
};


/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void ) {
	const char	*cmd;
	int		i;

	cmd = CG_Argv(0);

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
			commands[i].function();
			return qtrue;
		}
	}

	return qfalse;
}


/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands( void ) {
	int		i;

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		trap_AddCommand( commands[i].cmd );
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	// wombat: mohaa servers understand the following
	trap_AddCommand ("god"); // Sets the god mode cheat or toggles it.
	trap_AddCommand ("notarget"); // Toggles the notarget cheat.
	trap_AddCommand ("noclip"); // Toggles the noclip cheat.
	
	trap_AddCommand ("weapdrop"); // Drops the player's current weapon.
	trap_AddCommand ("reload"); // Reloads the player's weapon
	trap_AddCommand ("give"); // Gives the player the specified thing (weapon, ammo, item, etc.) and optionally the amount.
	trap_AddCommand ("jump"); // Makes the player jump.
	trap_AddCommand ("holster"); // Holsters all wielded weapons, or unholsters previously put away weapons
	// su44
	trap_AddCommand ("join_team"); // Join the specified team (allies or axis)
	trap_AddCommand ("auto_join_team"); // Join the team with fewer players
	trap_AddCommand ("spectator"); // Become a spectator

	// wombat: mohaa arena what is that???
	trap_AddCommand ("join_arena"); // Join the specified arena
	trap_AddCommand ("leave_arena"); // Leave the current arena
	trap_AddCommand ("create_team"); // Create a team in the current arena
	trap_AddCommand ("leave_team"); // Leave the current team
	trap_AddCommand ("arena_ui"); // Refresh the arena UI

	trap_AddCommand ("callvote"); // Player calls a vote
	trap_AddCommand ("vote"); // Player votes either yes or no
	
	trap_AddCommand ("primarydmweapon"); // Sets the player's primary DM weapon
	// primarydmweapon rifle / smg / mg / heavy / shotgun

	trap_AddCommand ("kill");
	trap_AddCommand ("say");
	trap_AddCommand ("sayone"); // talk to one client
	trap_AddCommand ("sayprivate"); // same as sayprivate??
	trap_AddCommand ("sayteam"); // talk to team
	trap_AddCommand ("teamsay"); // talk to team
}
