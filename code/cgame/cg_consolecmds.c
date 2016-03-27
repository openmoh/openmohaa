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


void CG_TargetCommand_f( void ) {
	int		targetNum;
	char	*test;

	targetNum = CG_CrosshairPlayer();
	if (!targetNum ) {
		return;
	}

	test = cgi.Argv( 1 );
	cgi.SendConsoleCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}



/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f (void) {
	cgi.Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize->integer+10)));
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f (void) {
	cgi.Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize->integer-10)));
}


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


static void CG_ScoresDown_f( void ) {

	if ( cg.scoresRequestTime + 1000 < cg.time ) {
		// the scores are more than two seconds out of data,
		// so request new ones
		cg.scoresRequestTime = cg.time;
		cgi.SendClientCommand( "score" );

		// leave the current scores up if they were already
		// displayed, but if this is the first hit, clear them out
		if ( !cg.showScores ) {
			cg.showScores = qtrue;
			cg.numScores = 0;
		}
	} else {
		// show the cached contents even if they just pressed if it
		// is within two seconds
		cg.showScores = qtrue;
	}
	cg.scoreBoardShowing = qtrue;
	cgi.Cvar_Set( "ui_showscores", "1" );
}

static void CG_ScoresUp_f( void ) {
	if ( cg.showScores ) {
		cg.showScores = qfalse;
		cg.scoreFadeTime = cg.time;
	}
	cg.scoreBoardShowing = qfalse;
	cgi.Cvar_Set( "ui_showscores", "0" );
}

static void CG_TellTarget_f( void ) {
	int		clientNum;
	char	command[128];
	char	*message;

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	message = cgi.Args();
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	cgi.SendClientCommand( command );
}

static void CG_TellAttacker_f( void ) {
	int		clientNum;
	char	command[128];
	char	*message;

	clientNum = CG_LastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	message = cgi.Args();
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	cgi.SendClientCommand( command );
}

/*
==================
CG_StartOrbit_f
==================
*/

static void CG_StartOrbit_f( void ) {
	char var[MAX_TOKEN_CHARS];

	cgi.Cvar_VariableStringBuffer( "developer", var, sizeof( var ) );
	if ( !atoi(var) ) {
		return;
	}
	if (cg_cameraOrbit->value != 0) {
		cgi.Cvar_Set ("cg_cameraOrbit", "0");
		cgi.Cvar_Set("cg_thirdPerson", "0");
	} else {
		cgi.Cvar_Set("cg_cameraOrbit", "5");
		cgi.Cvar_Set("cg_thirdPerson", "1");
		cgi.Cvar_Set("cg_thirdPersonAngle", "0");
		cgi.Cvar_Set("cg_thirdPersonRange", "100");
	}
}

static void CG_LocationPrint_f(void) {
	char *buffer;
	int x,y;

	buffer = cgi.Argv( 1 );
	x = atoi(buffer);
	buffer = cgi.Argv( 2 );
	y = atoi(buffer);
	buffer = cgi.Argv( 3 );
	buffer[511] = 0;
	CG_LocationPrint( buffer, x, y, SMALLCHAR_WIDTH );
}

// su44: in MoHAA, "useweaponclass pistol/rifle/etc" 
// commands are usually bound to 1,2,3,4,5,6 keys
static void CG_UseWeaponClass_f(void) {
	const char *name;

	if(cgi.Argc() < 2) {
		CG_Printf("usage: useweaponclass <weaponclassname>\n");
		return;
	}

	// get weapon classname
	name = CG_Argv(1);
	if(!Q_stricmp(name,"pistol")) {
		cg.iWeaponCommand = 1;
	} else if(!Q_stricmp(name,"rifle")) {
		cg.iWeaponCommand = 2;
	} else if(!Q_stricmp(name,"smg")) {
		cg.iWeaponCommand = 3;
	} else if(!Q_stricmp(name,"mg")) {
		cg.iWeaponCommand = 4;
	} else if(!Q_stricmp(name,"grenade")) {
		cg.iWeaponCommand = 5;
	} else if(!Q_stricmp(name,"heavy")) {
		cg.iWeaponCommand = 6;
	} else if(!Q_stricmp(name,"item2")) {
		cg.iWeaponCommand = 8; // su44: AFAIK it's not used at all in MoHAA
	} else if(!Q_stricmp(name,"item3")) {
		cg.iWeaponCommand = 9; // su44: AFAIK it's not used at all in MoHAA
	} else if(!Q_stricmp(name,"item4")) {
		cg.iWeaponCommand = 10; // su44: AFAIK it's not used at all in MoHAA
	} else if(!Q_stricmp(name,"item") || !Q_stricmp(name,"item1")) {
		cg.iWeaponCommand = 7; // su44: I think it's used for "Papers" item
	} else {
		CG_Printf("useweaponclass: unknown weapon %s\n", name);
	}
	cg.iWeaponCommandSend = 0;
}

typedef struct {
	char	*cmd;
	void	(*function)(void);
} consoleCommand_t;

static consoleCommand_t	commands[] = {
	{ "testgun", CG_TestGun_f },
	{ "testmodel", CG_TestModel_f },
	{ "testmodelanim", CG_TestModelAnim_f },
	{ "testmodelanimspeedscale", CG_TestModelAnimSpeedScale_f },
	{ "nextframe", CG_TestModelNextFrame_f },
	{ "prevframe", CG_TestModelPrevFrame_f },
	{ "nextskin", CG_TestModelNextSkin_f },
	{ "prevskin", CG_TestModelPrevSkin_f },
	{ "viewpos", CG_Viewpos_f },
	{ "+scores", CG_ScoresDown_f },
	{ "-scores", CG_ScoresUp_f },
	{ "+zoom", CG_ZoomDown_f },
	{ "-zoom", CG_ZoomUp_f },
	{ "sizeup", CG_SizeUp_f },
	{ "sizedown", CG_SizeDown_f },
	{ "weapnext", CG_NextWeapon_f },
	{ "weapprev", CG_PrevWeapon_f },
	{ "tell_target", CG_TellTarget_f },
	{ "tell_attacker", CG_TellAttacker_f },
	{ "tcmd", CG_TargetCommand_f },
	{ "startOrbit", CG_StartOrbit_f },
	//{ "camera", CG_Camera_f },
	{ "locationprint", CG_LocationPrint_f },
	// su44: MoHAA commands
	{ "toggleitem", CG_ToggleItem_f },
	{ "weapdrop", CG_DropWeapon_f },
	{ "holster", CG_HolsterWeapon_f },
	{ "weapnext", CG_NextWeapon_f },
	{ "weapprev", CG_PrevWeapon_f },
	{ "useweaponclass", CG_UseWeaponClass_f },
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
		cgi.AddCommand( commands[i].cmd, NULL );
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	// wombat: mohaa servers understand the following
	cgi.AddCommand( "dog", NULL ); // Sets the god mode cheat or toggles it.
	cgi.AddCommand ("notarget", NULL ); // Toggles the notarget cheat.
	cgi.AddCommand ("noclip", NULL ); // Toggles the noclip cheat.
	
	cgi.AddCommand ("weapdrop", NULL ); // Drops the player's current weapon.
	cgi.AddCommand ("reload", NULL ); // Reloads the player's weapon
	cgi.AddCommand ("give", NULL ); // Gives the player the specified thing (weapon, ammo, item, etc.) and optionally the amount.
	cgi.AddCommand ("jump", NULL ); // Makes the player jump.
	cgi.AddCommand ("holster", NULL ); // Holsters all wielded weapons, or unholsters previously put away weapons
	// su44
	cgi.AddCommand ("join_team", NULL ); // Join the specified team (allies or axis)
	cgi.AddCommand ("auto_join_team", NULL ); // Join the team with fewer players
	cgi.AddCommand ("spectator", NULL ); // Become a spectator

	// wombat: mohaa arena what is that???
	cgi.AddCommand ("join_arena", NULL ); // Join the specified arena
	cgi.AddCommand ("leave_arena", NULL ); // Leave the current arena
	cgi.AddCommand ("create_team", NULL ); // Create a team in the current arena
	cgi.AddCommand ("leave_team", NULL ); // Leave the current team
	cgi.AddCommand ("arena_ui", NULL ); // Refresh the arena UI

	cgi.AddCommand ("callvote", NULL ); // Player calls a vote
	cgi.AddCommand ("vote", NULL ); // Player votes either yes or no
	
	cgi.AddCommand ("primarydmweapon", NULL ); // Sets the player's primary DM weapon
	// primarydmweapon rifle / smg / mg / heavy / shotgun

	cgi.AddCommand ("kill", NULL );
	cgi.AddCommand ("say", NULL );
	cgi.AddCommand ("sayone", NULL ); // talk to one client
	cgi.AddCommand ("sayprivate", NULL ); // same as sayone??
	cgi.AddCommand ("sayteam", NULL ); // talk to team
	cgi.AddCommand ("teamsay", NULL ); // talk to team
}
