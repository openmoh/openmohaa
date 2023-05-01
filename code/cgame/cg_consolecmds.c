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

//
// DESCRIPTION:
// text commands typed in at the local console, or executed by a key binding

#include "cg_local.h"

void CG_TargetCommand_f(void);

/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f(void)
{
    cgi.Cvar_Set("viewsize", va("%i", (int)(cg_viewsize->integer + 10)));
}

/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f(void)
{
    cgi.Cvar_Set("viewsize", va("%i", (int)(cg_viewsize->integer - 10)));
}

/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f(void)
{
    cgi.Printf("(%i %i %i) : %i\n", (int)cg.refdef.vieworg[0],
               (int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2],
               (int)cg.refdefViewAngles[YAW]);
}

void CG_SetDesiredObjectiveAlpha(float fAlpha)
{
    cg.ObjectivesDesiredAlpha = fAlpha;
    cg.ObjectivesAlphaTime = (float)(cg.time + 250);
    cg.ObjectivesBaseAlpha = cg.ObjectivesCurrentAlpha;
}

void CG_ScoresDown_f( void )
{
    if (cgs.gametype != GT_SINGLE_PLAYER)
    {
        if (!cg.scoresRequestTime) {
            cg.scoresRequestTime = cg.time;
            CG_SetDesiredObjectiveAlpha(1.0f);
        }

        return;
    }
	// don't constantly send requests, or we would overflow
	if ( cg.showScores && cg.scoresRequestTime + 2000 > cg.time ) {
		return;
	}
    
    cg.scoresRequestTime = cg.time;
	cgi.SendClientCommand( "score" );

	if ( !cg.showScores ) {
		// don't display anything until first score returns
		cg.showScores = qtrue;
        CG_PrepScoreBoardInfo();
        cgi.UI_ShowScoreBoard(cg.scoresMenuName);
	}
}

void CG_ScoresUp_f( void )
{
    if (cgs.gametype != GT_SINGLE_PLAYER)
    {
        if (!cg.scoresRequestTime) {
            cg.scoresRequestTime = cg.time;
            CG_SetDesiredObjectiveAlpha(0.0f);
        }

        return;
    }

    if (!cg.showScores) {
        return;
    }

    cg.showScores = qfalse;
    cgi.UI_HideScoreBoard(cg.scoresMenuName);
}

#if 0


/*
=============================================================================

  MODEL TESTING

The viewthing and gun positioning tools from Q2 have been integrated and
enhanced into a single model testing facility.

Model viewing can begin with either "testmodel <modelname>" or "testgun <modelname>".

The names must be the full pathname after the basedir, like 
"models/weapons/v_launch/tris.md3" or "players/male/tris.md3"

Testmodel will create a fake entity 100 units in front of the current view
position, directly facing the viewer.  It will remain immobile, so you can
move around it to view it from different angles.

Testgun will cause the model to follow the player around and supress the real
view weapon model.  The default frame 0 of most guns is completely off screen,
so you will probably have to cycle a couple frames to see it.

"nextframe", "prevframe", "nextskin", and "prevskin" commands will change the
frame or skin of the testmodel.  These are bound to F5, F6, F7, and F8 in
q3default.cfg.

Note that none of the model testing features update while the game is paused, so
it may be convenient to test with deathmatch set to 1 so that bringing down the
console doesn't pause the game.

=============================================================================
*/

/*
=================
CG_TestModel_f

Creates an entity in front of the current position, which
can then be moved around
=================
*/
void CG_TestModel_f (void) {
	vec3_t		angles;

	memset( &cg.testModelEntity, 0, sizeof(cg.testModelEntity) );
	if ( cgi.Argc() < 2 ) {
		return;
	}

	Q_strncpyz (cg.testModelName, cgi.Argv( 1 ), MAX_QPATH );
	cg.testModelEntity.hModel = cgi.R_RegisterModel( cg.testModelName );

	if ( cgi.Argc() == 3 ) {
		cg.testModelEntity.backlerp = atof( cgi.Argv( 2 ) );
		cg.testModelEntity.frame = 1;
		cg.testModelEntity.oldframe = 0;
	}
	if (! cg.testModelEntity.hModel ) {
		cgi.Printf( "Can't register model\n" );
		return;
	}

	VectorMA( cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], cg.testModelEntity.origin );

	angles[PITCH] = 0;
	angles[YAW] = 180 + cg.refdefViewAngles[1];
	angles[ROLL] = 0;

	AnglesToAxis( angles, cg.testModelEntity.axis );
	cg.testGun = qfalse;
}

void CG_TestModelNextFrame_f (void) {
	cg.testModelEntity.frame++;
	cgi.Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelPrevFrame_f (void) {
	cg.testModelEntity.frame--;
	cgi.Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelNextSkin_f (void) {
	cg.testModelEntity.skinNum++;
	cgi.Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

void CG_TestModelPrevSkin_f (void) {
	cg.testModelEntity.skinNum--;
	if ( cg.testModelEntity.skinNum < 0 ) {
		cg.testModelEntity.skinNum = 0;
	}
	cgi.Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

void CG_AddTestModel (void) {
	// re-register the model, because the level may have changed
	cg.testModelEntity.hModel = cgi.R_RegisterModel( cg.testModelName );
	if (! cg.testModelEntity.hModel ) {
		cgi.Printf ("Can't register model\n");
		return;
	}
	cgi.R_AddRefEntityToScene( &cg.testModelEntity );
}

#endif

typedef struct {
    char* cmd;
    void (*function)(void);
} consoleCommand_t;

static consoleCommand_t commands[] = {
  //	{ "testmodel", CG_TestModel_f },
  //	{ "nextframe", CG_TestModelNextFrame_f },
  //	{ "prevframe", CG_TestModelPrevFrame_f },
  //	{ "nextskin", CG_TestModelNextSkin_f },
  //	{ "prevskin", CG_TestModelPrevSkin_f },
    {"viewpos",            CG_Viewpos_f        },
 //	{ "+scores", CG_ScoresDown_f },
  //	{ "-scores", CG_ScoresUp_f },
  //	{ "+info", CG_InfoDown_f },
  //	{ "-info", CG_InfoUp_f },
    {"sizeup",             CG_SizeUp_f         },
    {"sizedown",           CG_SizeDown_f       },
    {"cg_eventlist",       CG_EventList_f      },
    {"cg_eventhelp",       CG_EventHelp_f      },
    {"cg_dumpevents",      CG_DumpEventHelp_f  },
    {"cg_pendingevents",   CG_PendingEvents_f  },
    {"cg_classlist",       CG_ClassList_f      },
    {"cg_classtree",       CG_ClassTree_f      },
    {"cg_classevents",     CG_ClassEvents_f    },
    {"cg_dumpclassevents", CG_DumpClassEvents_f},
    {"cg_dumpallclasses",  CG_DumpAllClasses_f },
    {"testemitter",        CG_TestEmitter_f    },
    {"dumpemitter",        CG_DumpEmitter_f    },
};

/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand(void)
{
    const char* cmd;
    int i;

    cmd = cgi.Argv(0);

    for (i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        if (!Q_stricmp(cmd, commands[i].cmd)) {
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
void CG_InitConsoleCommands(void)
{
    int i;

    for (i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        cgi.AddCommand(commands[i].cmd);
    }

    cgi.AddCommand("callvote");
    cgi.AddCommand("vote");
}

void CG_Mapinfo_f()
{
    cgi.Printf("---------------------\n");
    cgi.R_PrintBSPFileSizes();
    cgi.CM_PrintBSPFileSizes();
    cgi.Printf("---------------------\n");
}

void CG_PushMenuTeamSelect_f()
{
    if (cgs.gametype == GT_SINGLE_PLAYER) {
        return;
    }

    cgi.Cmd_Execute(EXEC_NOW, "ui_getplayermodel\n");
    switch (cgs.gametype) {
    case GT_FFA:
        cgi.Cmd_Execute(EXEC_NOW, "pushmenu SelectFFAModel\n");
        break;
    case GT_OBJECTIVE:
        cgi.Cmd_Execute(EXEC_NOW, "pushmenu ObjSelectTeam\n");
        break;
    default:
        cgi.Cmd_Execute(EXEC_NOW, "pushmenu SelectTeam\n");
        break;
	}
}

void CG_PushMenuWeaponSelect_f()
{
    if (cgs.gametype == GT_SINGLE_PLAYER) {
        return;
    }

    cgi.Cmd_Execute(EXEC_NOW, "pushmenu SelectPrimaryWeapon\n");
}

void CG_UseWeaponClass_f()
{
    const char* cmd;

    cmd = cgi.Argv(1);

    if (!Q_stricmp(cmd, "pistol")) {
        cg.iWeaponCommand = WEAPON_COMMAND_USE_PISTOL;
    } else if (!Q_stricmp(cmd, "rifle")) {
        cg.iWeaponCommand = WEAPON_COMMAND_USE_RIFLE;
    } else if (!Q_stricmp(cmd, "smg")) {
        cg.iWeaponCommand = WEAPON_COMMAND_USE_SMG;
    } else if (!Q_stricmp(cmd, "mg")) {
        cg.iWeaponCommand = WEAPON_COMMAND_USE_MG;
    } else if (!Q_stricmp(cmd, "grenade")) {
        cg.iWeaponCommand = WEAPON_COMMAND_USE_GRENADE;
    } else if (!Q_stricmp(cmd, "heavy")) {
        cg.iWeaponCommand = WEAPON_COMMAND_USE_HEAVY;
    } else if (!Q_stricmp(cmd, "item1") || !Q_stricmp(cmd, "item")) {
        cg.iWeaponCommand = WEAPON_COMMAND_USE_ITEM1;
    } else if (!Q_stricmp(cmd, "item2")) {
        cg.iWeaponCommand = WEAPON_COMMAND_USE_ITEM2;
    } else if (!Q_stricmp(cmd, "item3")) {
        cg.iWeaponCommand = WEAPON_COMMAND_USE_ITEM3;
    } else if (!Q_stricmp(cmd, "item4")) {
        cg.iWeaponCommand = WEAPON_COMMAND_USE_ITEM4;
    }

    cg.iWeaponCommandSend = 0;
}

void CG_NextWeapon_f()
{
    cg.iWeaponCommand = WEAPON_COMMAND_USE_NEXT_WEAPON;
    cg.iWeaponCommandSend = 0;
}

void CG_PrevWeapon_f()
{
    cg.iWeaponCommand = WEAPON_COMMAND_USE_PREV_WEAPON;
    cg.iWeaponCommandSend = 0;
}

void CG_UseLastWeapon_f()
{
    cg.iWeaponCommand = WEAPON_COMMAND_USE_LAST_WEAPON;
    cg.iWeaponCommandSend = 0;
}

void CG_HolsterWeapon_f()
{
    cg.iWeaponCommand = WEAPON_COMMAND_HOLSTER;
    cg.iWeaponCommandSend = 0;
}

void CG_DropWeapon_f()
{
    cg.iWeaponCommand = WEAPON_COMMAND_DROP;
    cg.iWeaponCommandSend = 0;
}

void CG_ToggleItem_f()
{
    cg.iWeaponCommand = WEAPON_COMMAND_USE_ITEM1;
    cg.iWeaponCommandSend = 0;
}

int CG_WeaponCommandButtonBits()
{
    int iShiftedWeaponCommand;

    iShiftedWeaponCommand = cg.iWeaponCommand;
    if (!iShiftedWeaponCommand) {
        return 0;
    }

    cg.iWeaponCommandSend++;
    if (cg.iWeaponCommandSend > 2) {
        cg.iWeaponCommand = 0;
    }

    return (iShiftedWeaponCommand << 7) & WEAPON_COMMAND_MASK;
}
