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
#include "client.h"
#include "cl_ui.h"
#include "../uilib/ui_public.h"

/*

key up events are sent even if in console mode

*/

int			nextHistoryLine;		// the last line in the history buffer, not masked
int			historyLine;	// the line being displayed from history buffer
							// will be <= nextHistoryLine

qboolean	chat_team;

int			chat_playerNum;


qboolean	key_overstrikeMode;

int				anykeydown;
qkey_t		keys[MAX_KEYS];
qkey_t		altkeys[MAX_KEYS];
qkey_t		ctrlkeys[MAX_KEYS];

qboolean menubound[MAX_KEYS];

qboolean alt_down;
qboolean ctrl_down;

typedef struct {
	const char	*name;
	int			keynum;
} keyname_t;

// names not in this list can either be lowercase ascii, or '0xnn' hex sequences
keyname_t keynames[] =
{
	{"TAB", K_TAB},
	{"ENTER", K_ENTER},
	{"ESCAPE", K_ESCAPE},
	{"SPACE", K_SPACE},
	{"BACKSPACE", K_BACKSPACE},
	{"UPARROW", K_UPARROW},
	{"DOWNARROW", K_DOWNARROW},
	{"LEFTARROW", K_LEFTARROW},
	{"RIGHTARROW", K_RIGHTARROW},

	{"ALT", K_ALT},
	{"CTRL", K_CTRL},
	{"SHIFT", K_SHIFT},

	{"COMMAND", K_COMMAND},

	{"CAPSLOCK", K_CAPSLOCK},

	
	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},
	{"F13", K_F13},
	{"F14", K_F14},
	{"F15", K_F15},

	{"INS", K_INS},
	{"DEL", K_DEL},
	{"PGDN", K_PGDN},
	{"PGUP", K_PGUP},
	{"HOME", K_HOME},
	{"END", K_END},

	{"MOUSE1", K_MOUSE1},
	{"MOUSE2", K_MOUSE2},
	{"MOUSE3", K_MOUSE3},
	{"MOUSE4", K_MOUSE4},
	{"MOUSE5", K_MOUSE5},

	{"MWHEELUP",	K_MWHEELUP },
	{"MWHEELDOWN",	K_MWHEELDOWN },

	{"JOY1", K_JOY1},
	{"JOY2", K_JOY2},
	{"JOY3", K_JOY3},
	{"JOY4", K_JOY4},
	{"JOY5", K_JOY5},
	{"JOY6", K_JOY6},
	{"JOY7", K_JOY7},
	{"JOY8", K_JOY8},
	{"JOY9", K_JOY9},
	{"JOY10", K_JOY10},
	{"JOY11", K_JOY11},
	{"JOY12", K_JOY12},
	{"JOY13", K_JOY13},
	{"JOY14", K_JOY14},
	{"JOY15", K_JOY15},
	{"JOY16", K_JOY16},
	{"JOY17", K_JOY17},
	{"JOY18", K_JOY18},
	{"JOY19", K_JOY19},
	{"JOY20", K_JOY20},
	{"JOY21", K_JOY21},
	{"JOY22", K_JOY22},
	{"JOY23", K_JOY23},
	{"JOY24", K_JOY24},
	{"JOY25", K_JOY25},
	{"JOY26", K_JOY26},
	{"JOY27", K_JOY27},
	{"JOY28", K_JOY28},
	{"JOY29", K_JOY29},
	{"JOY30", K_JOY30},
	{"JOY31", K_JOY31},
	{"JOY32", K_JOY32},

	{"AUX1", K_AUX1},
	{"AUX2", K_AUX2},
	{"AUX3", K_AUX3},
	{"AUX4", K_AUX4},
	{"AUX5", K_AUX5},
	{"AUX6", K_AUX6},
	{"AUX7", K_AUX7},
	{"AUX8", K_AUX8},
	{"AUX9", K_AUX9},
	{"AUX10", K_AUX10},
	{"AUX11", K_AUX11},
	{"AUX12", K_AUX12},
	{"AUX13", K_AUX13},
	{"AUX14", K_AUX14},
	{"AUX15", K_AUX15},
	{"AUX16", K_AUX16},

	{"KP_HOME",			K_KP_HOME },
	{"KP_UPARROW",		K_KP_UPARROW },
	{"KP_PGUP",			K_KP_PGUP },
	{"KP_LEFTARROW",	K_KP_LEFTARROW },
	{"KP_5",			K_KP_5 },
	{"KP_RIGHTARROW",	K_KP_RIGHTARROW },
	{"KP_END",			K_KP_END },
	{"KP_DOWNARROW",	K_KP_DOWNARROW },
	{"KP_PGDN",			K_KP_PGDN },
	{"KP_ENTER",		K_KP_ENTER },
	{"KP_INS",			K_KP_INS },
	{"KP_DEL",			K_KP_DEL },
	{"KP_SLASH",		K_KP_SLASH },
	{"KP_MINUS",		K_KP_MINUS },
	{"KP_PLUS",			K_KP_PLUS },
	{"KP_NUMLOCK",		K_KP_NUMLOCK },
	{"KP_STAR",			K_KP_STAR },
	{"KP_EQUALS",		K_KP_EQUALS },

	{"PAUSE", K_PAUSE},
	
	{"SEMICOLON", ';'},	// because a raw semicolon separates commands

	{"WORLD_0", K_WORLD_0},
	{"WORLD_1", K_WORLD_1},
	{"WORLD_2", K_WORLD_2},
	{"WORLD_3", K_WORLD_3},
	{"WORLD_4", K_WORLD_4},
	{"WORLD_5", K_WORLD_5},
	{"WORLD_6", K_WORLD_6},
	{"WORLD_7", K_WORLD_7},
	{"WORLD_8", K_WORLD_8},
	{"WORLD_9", K_WORLD_9},
	{"WORLD_10", K_WORLD_10},
	{"WORLD_11", K_WORLD_11},
	{"WORLD_12", K_WORLD_12},
	{"WORLD_13", K_WORLD_13},
	{"WORLD_14", K_WORLD_14},
	{"WORLD_15", K_WORLD_15},
	{"WORLD_16", K_WORLD_16},
	{"WORLD_17", K_WORLD_17},
	{"WORLD_18", K_WORLD_18},
	{"WORLD_19", K_WORLD_19},
	{"WORLD_20", K_WORLD_20},
	{"WORLD_21", K_WORLD_21},
	{"WORLD_22", K_WORLD_22},
	{"WORLD_23", K_WORLD_23},
	{"WORLD_24", K_WORLD_24},
	{"WORLD_25", K_WORLD_25},
	{"WORLD_26", K_WORLD_26},
	{"WORLD_27", K_WORLD_27},
	{"WORLD_28", K_WORLD_28},
	{"WORLD_29", K_WORLD_29},
	{"WORLD_30", K_WORLD_30},
	{"WORLD_31", K_WORLD_31},
	{"WORLD_32", K_WORLD_32},
	{"WORLD_33", K_WORLD_33},
	{"WORLD_34", K_WORLD_34},
	{"WORLD_35", K_WORLD_35},
	{"WORLD_36", K_WORLD_36},
	{"WORLD_37", K_WORLD_37},
	{"WORLD_38", K_WORLD_38},
	{"WORLD_39", K_WORLD_39},
	{"WORLD_40", K_WORLD_40},
	{"WORLD_41", K_WORLD_41},
	{"WORLD_42", K_WORLD_42},
	{"WORLD_43", K_WORLD_43},
	{"WORLD_44", K_WORLD_44},
	{"WORLD_45", K_WORLD_45},
	{"WORLD_46", K_WORLD_46},
	{"WORLD_47", K_WORLD_47},
	{"WORLD_48", K_WORLD_48},
	{"WORLD_49", K_WORLD_49},
	{"WORLD_50", K_WORLD_50},
	{"WORLD_51", K_WORLD_51},
	{"WORLD_52", K_WORLD_52},
	{"WORLD_53", K_WORLD_53},
	{"WORLD_54", K_WORLD_54},
	{"WORLD_55", K_WORLD_55},
	{"WORLD_56", K_WORLD_56},
	{"WORLD_57", K_WORLD_57},
	{"WORLD_58", K_WORLD_58},
	{"WORLD_59", K_WORLD_59},
	{"WORLD_60", K_WORLD_60},
	{"WORLD_61", K_WORLD_61},
	{"WORLD_62", K_WORLD_62},
	{"WORLD_63", K_WORLD_63},
	{"WORLD_64", K_WORLD_64},
	{"WORLD_65", K_WORLD_65},
	{"WORLD_66", K_WORLD_66},
	{"WORLD_67", K_WORLD_67},
	{"WORLD_68", K_WORLD_68},
	{"WORLD_69", K_WORLD_69},
	{"WORLD_70", K_WORLD_70},
	{"WORLD_71", K_WORLD_71},
	{"WORLD_72", K_WORLD_72},
	{"WORLD_73", K_WORLD_73},
	{"WORLD_74", K_WORLD_74},
	{"WORLD_75", K_WORLD_75},
	{"WORLD_76", K_WORLD_76},
	{"WORLD_77", K_WORLD_77},
	{"WORLD_78", K_WORLD_78},
	{"WORLD_79", K_WORLD_79},
	{"WORLD_80", K_WORLD_80},
	{"WORLD_81", K_WORLD_81},
	{"WORLD_82", K_WORLD_82},
	{"WORLD_83", K_WORLD_83},
	{"WORLD_84", K_WORLD_84},
	{"WORLD_85", K_WORLD_85},
	{"WORLD_86", K_WORLD_86},
	{"WORLD_87", K_WORLD_87},
	{"WORLD_88", K_WORLD_88},
	{"WORLD_89", K_WORLD_89},
	{"WORLD_90", K_WORLD_90},
	{"WORLD_91", K_WORLD_91},
	{"WORLD_92", K_WORLD_92},
	{"WORLD_93", K_WORLD_93},
	{"WORLD_94", K_WORLD_94},
	{"WORLD_95", K_WORLD_95},

	{"WINDOWS", K_SUPER},
	{"COMPOSE", K_COMPOSE},
	{"MODE", K_MODE},
	{"HELP", K_HELP},
	{"PRINT", K_PRINT},
	{"SYSREQ", K_SYSREQ},
	{"SCROLLOCK", K_SCROLLOCK },
	{"BREAK", K_BREAK},
	{"MENU", K_MENU},
	{"POWER", K_POWER},
	{"EURO", K_EURO},
	{"UNDO", K_UNDO},

	{"PAD0_A", K_PAD0_A },
	{"PAD0_B", K_PAD0_B },
	{"PAD0_X", K_PAD0_X },
	{"PAD0_Y", K_PAD0_Y },
	{"PAD0_BACK", K_PAD0_BACK },
	{"PAD0_GUIDE", K_PAD0_GUIDE },
	{"PAD0_START", K_PAD0_START },
	{"PAD0_LEFTSTICK_CLICK", K_PAD0_LEFTSTICK_CLICK },
	{"PAD0_RIGHTSTICK_CLICK", K_PAD0_RIGHTSTICK_CLICK },
	{"PAD0_LEFTSHOULDER", K_PAD0_LEFTSHOULDER },
	{"PAD0_RIGHTSHOULDER", K_PAD0_RIGHTSHOULDER },
	{"PAD0_DPAD_UP", K_PAD0_DPAD_UP },
	{"PAD0_DPAD_DOWN", K_PAD0_DPAD_DOWN },
	{"PAD0_DPAD_LEFT", K_PAD0_DPAD_LEFT },
	{"PAD0_DPAD_RIGHT", K_PAD0_DPAD_RIGHT },

	{"PAD0_LEFTSTICK_LEFT", K_PAD0_LEFTSTICK_LEFT },
	{"PAD0_LEFTSTICK_RIGHT", K_PAD0_LEFTSTICK_RIGHT },
	{"PAD0_LEFTSTICK_UP", K_PAD0_LEFTSTICK_UP },
	{"PAD0_LEFTSTICK_DOWN", K_PAD0_LEFTSTICK_DOWN },
	{"PAD0_RIGHTSTICK_LEFT", K_PAD0_RIGHTSTICK_LEFT },
	{"PAD0_RIGHTSTICK_RIGHT", K_PAD0_RIGHTSTICK_RIGHT },
	{"PAD0_RIGHTSTICK_UP", K_PAD0_RIGHTSTICK_UP },
	{"PAD0_RIGHTSTICK_DOWN", K_PAD0_RIGHTSTICK_DOWN },
	{"PAD0_LEFTTRIGGER", K_PAD0_LEFTTRIGGER },
	{"PAD0_RIGHTTRIGGER", K_PAD0_RIGHTTRIGGER },

	{"PAD0_MISC1", K_PAD0_MISC1 },
	{"PAD0_PADDLE1", K_PAD0_PADDLE1 },
	{"PAD0_PADDLE2", K_PAD0_PADDLE2 },
	{"PAD0_PADDLE3", K_PAD0_PADDLE3 },
	{"PAD0_PADDLE4", K_PAD0_PADDLE4 },
	{"PAD0_TOUCHPAD", K_PAD0_TOUCHPAD },

	{NULL,0}
};

/*
================
Message_Key

In game talk message
================
*/
void Message_Key( int key ) {

	char	buffer[MAX_STRING_CHARS];


	if (key == K_ESCAPE) {
		Key_SetCatcher( Key_GetCatcher( ) & ~KEYCATCH_MESSAGE );
		return;
	}

	if ( key == K_ENTER || key == K_KP_ENTER )
	{
		Key_SetCatcher( Key_GetCatcher( ) & ~KEYCATCH_MESSAGE );
		return;
	}
}

//============================================================================


qboolean Key_GetOverstrikeMode( void ) {
	return key_overstrikeMode;
}


void Key_SetOverstrikeMode( qboolean state ) {
	key_overstrikeMode = state;
}


/*
===================
Key_IsDown
===================
*/
qboolean Key_IsDown( int keynum ) {
	if ( keynum < 0 || keynum >= MAX_KEYS ) {
		return qfalse;
	}

	return keys[keynum].down;
}


/*
===================
Key_StringToKeynum

Returns a key number to be used to index keys[] by looking at
the given string.  Single ascii characters return themselves, while
the K_* names are matched up.

0x11 will be interpreted as raw hex, which will allow new controlers

to be configured even if they don't have defined names.
===================
*/
int Key_StringToKeynum( const char *str ) {
	keyname_t	*kn;

	if ( !str || !str[0] ) {
		return -1;
	}
	if ( !str[1] ) {
		return str[0];
	}

	// check for hex code
	if ( str[0] == '0' && str[1] == 'x' && strlen( str ) == 4) {
		int		n1, n2;

		n1 = str[2];
		if ( n1 >= '0' && n1 <= '9' ) {
			n1 -= '0';
		} else if ( n1 >= 'a' && n1 <= 'f' ) {
			n1 = n1 - 'a' + 10;
		} else {
			n1 = 0;
		}

		n2 = str[3];
		if ( n2 >= '0' && n2 <= '9' ) {
			n2 -= '0';
		} else if ( n2 >= 'a' && n2 <= 'f' ) {
			n2 = n2 - 'a' + 10;
		} else {
			n2 = 0;
		}

		return n1 * 16 + n2;
	}

	// scan for a text match
	for ( kn=keynames ; kn->name ; kn++ ) {
		if ( !Q_stricmp( str,kn->name ) )
			return kn->keynum;
	}

	return -1;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, a K_* name, or a 0x11 hex string) for the
given keynum.
===================
*/
const char *Key_KeynumToString( int keynum ) {
	keyname_t	*kn;
	static	char	tinystr[5];
	int			i, j;

	if ( keynum == -1 ) {
		return "<KEY NOT FOUND>";
	}

	if ( keynum < 0 || keynum >= MAX_KEYS ) {
		return "<OUT OF RANGE>";
	}

	// check for printable ascii (don't use quote)
	if ( keynum > 32 && keynum < 127 && keynum != '"' && keynum != ';' ) {
		tinystr[0] = keynum;
		tinystr[1] = 0;
		return tinystr;
	}

	// check for a key string
	for ( kn=keynames ; kn->name ; kn++ ) {
		if (keynum == kn->keynum) {
			return kn->name;
		}
	}

	// make a hex string
	i = keynum >> 4;
	j = keynum & 15;

	tinystr[0] = '0';
	tinystr[1] = 'x';
	tinystr[2] = i > 9 ? i - 10 + 'a' : i + '0';
	tinystr[3] = j > 9 ? j - 10 + 'a' : j + '0';
	tinystr[4] = 0;

	return tinystr;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, a K_* name, or a 0x11 hex string) for the
given keynum.
===================
*/
const char *Key_KeynumToBindString( int keynum ) {
	keyname_t	*kn;
	static	char	tinystr[5];

	if ( keynum == -1 ) {
		return "Not Bound";
	}

	if ( keynum < 0 || keynum >= MAX_KEYS ) {
		return "Out of Range";
	}

	// check for printable ascii (don't use quote)
	if ( keynum > 32 && keynum < 127 && keynum != '"' && keynum != ';' ) {
		tinystr[0] = keynum;
		tinystr[1] = 0;
		return tinystr;
	}

	// check for a key string
	for ( kn=keynames ; kn->name ; kn++ ) {
		if (keynum == kn->keynum) {
			return kn->name;
		}
	}

	return "Unknown Key";
}


/*
===================
Key_SetBinding
===================
*/
void Key_SetBinding( int keynum, const char *binding ) {
	if ( keynum < 0 || keynum >= MAX_KEYS ) {
		return;
	}

	// free old bindings
	if ( keys[ keynum ].binding ) {
		Z_Free( keys[ keynum ].binding );
	}

	// allocate memory for new binding
	keys[keynum].binding = CopyString( binding );

	// consider this like modifying an archived cvar, so the
	// file write will be triggered at the next oportunity
	cvar_modifiedFlags |= CVAR_ARCHIVE;
}

/*
===================
Key_SetAltBinding
===================
*/
void Key_SetAltBinding( int keynum, const char *binding ) {
	if ( keynum < 0 || keynum >= MAX_KEYS ) {
		return;
	}

	// free old bindings
	if ( altkeys[ keynum ].binding ) {
		Z_Free( altkeys[ keynum ].binding );
	}

	// allocate memory for new binding
	altkeys[keynum].binding = CopyString( binding );

	// consider this like modifying an archived cvar, so the
	// file write will be triggered at the next oportunity
	cvar_modifiedFlags |= CVAR_ARCHIVE;
}

/*
===================
Key_SetCtrlBinding
===================
*/
void Key_SetCtrlBinding( int keynum, const char *binding ) {
	if ( keynum < 0 || keynum >= MAX_KEYS ) {
		return;
	}

	// free old bindings
	if ( ctrlkeys[ keynum ].binding ) {
		Z_Free( ctrlkeys[ keynum ].binding );
	}

	// allocate memory for new binding
	ctrlkeys[ keynum ].binding = CopyString( binding );

	// consider this like modifying an archived cvar, so the
	// file write will be triggered at the next oportunity
	cvar_modifiedFlags |= CVAR_ARCHIVE;
}


/*
===================
Key_GetBinding
===================
*/
const char *Key_GetBinding( int keynum ) {
	if ( keynum < 0 || keynum >= MAX_KEYS ) {
		return "";
	}

	return keys[ keynum ].binding;
}

/*
===================
Key_GetKey
===================
*/

int Key_GetKey(const char *binding) {
  int i;

  if (binding) {
  	for (i=0 ; i < MAX_KEYS ; i++) {
      if (keys[i].binding && Q_stricmp(binding, keys[i].binding) == 0) {
        return i;
      }
    }
  }
  return -1;
}

/*
===================
Key_GetKeynameForCommand
===================
*/
const char *Key_GetKeynameForCommand( const char *command ) {
	int		i;

	for( i = 0; i < MAX_KEYS; i++ ) {
		if( !keys[ i ].binding ) {
			continue;
		}
		if( keys[ i ].down && !stricmp( command, keys[ i ].binding ) ) {
			return Key_KeynumToString( i );
		}
	}

	return "Unknown Command";
}

/*
===================
Key_GetKeysForCommand
===================
*/
void Key_GetKeysForCommand( const char *command, int *key1, int *key2 ) {
	int i;

	*key1 = *key2 = -1;

	for( i = 0; i < MAX_KEYS; i++ ) {
		if( !keys[ i ].binding ) {
			continue;
		}
		if( !stricmp( command, keys[ i ].binding ) ) {
			if( *key1 == -1 ) {
				*key1 = i;
			} else {
				*key2 = i;
				return;
			}
		}
	}
}

/*
===================
Key_Unbind_f
===================
*/
void Key_Unbind_f (void)
{
	int		b;

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("unbind <key> : remove commands from a key\n");
		return;
	}

	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b==-1)
	{
		Com_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	Key_SetBinding (b, "");
}

/*
===================
Key_Unaltbind_f
===================
*/
void Key_Unaltbind_f (void)
{
	int		b;

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("unaltbind <key> : remove commands from a key\n");
		return;
	}

	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b==-1)
	{
		Com_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	Key_SetAltBinding (b, "");
}

/*
===================
Key_Unctrlbind_f
===================
*/
void Key_Unctrlbind_f (void)
{
	int		b;

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("unctrlbind <key> : remove commands from a key\n");
		return;
	}

	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b==-1)
	{
		Com_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	Key_SetCtrlBinding (b, "");
}

/*
===================
Key_Unbindall_f
===================
*/
void Key_Unbindall_f (void)
{
	int		i;

	for (i=0 ; i < MAX_KEYS; i++)
		if (keys[i].binding)
			Key_SetBinding (i, "");
}


/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f (void)
{
	int			i, c, b;
	char		cmd[1024];

	c = Cmd_Argc();

	if (c < 2)
	{
		Com_Printf ("bind <key> [command] : attach a command to a key\n");
		return;
	}
	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b==-1)
	{
		Com_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	if (c == 2)
	{
		if (keys[b].binding)
			Com_Printf ("\"%s\" = \"%s\"\n", Cmd_Argv(1), keys[b].binding );
		else
			Com_Printf ("\"%s\" is not bound\n", Cmd_Argv(1) );
		return;
	}

// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	for (i=2 ; i< c ; i++)
	{
		strcat (cmd, Cmd_Argv(i));
		if (i != (c-1))
			strcat (cmd, " ");
	}

	Key_SetBinding (b, cmd);
}

/*
===================
Key_AltBind_f
===================
*/
void Key_AltBind_f (void)
{
	int			i, c, b;
	char		cmd[1024];

	c = Cmd_Argc();

	if (c < 2)
	{
		Com_Printf ("altbind <key> [command] : attach a command to a key\n");
		return;
	}
	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b==-1)
	{
		Com_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	if (c == 2)
	{
		if (altkeys[b].binding)
			Com_Printf ("\"%s\" = \"%s\"\n", Cmd_Argv(1), altkeys[b].binding );
		else
			Com_Printf ("\"%s\" is not bound\n", Cmd_Argv(1) );
		return;
	}

// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	for (i=2 ; i< c ; i++)
	{
		strcat (cmd, Cmd_Argv(i));
		if (i != (c-1))
			strcat (cmd, " ");
	}

	Key_SetAltBinding (b, cmd);
}

/*
===================
Key_AltBind_f
===================
*/
void Key_CtrlBind_f (void)
{
	int			i, c, b;
	char		cmd[1024];

	c = Cmd_Argc();

	if (c < 2)
	{
		Com_Printf ("ctrlbind <key> [command] : attach a command to a key\n");
		return;
	}
	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b==-1)
	{
		Com_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	if (c == 2)
	{
		if (ctrlkeys[b].binding)
			Com_Printf ("\"%s\" = \"%s\"\n", Cmd_Argv(1), ctrlkeys[b].binding );
		else
			Com_Printf ("\"%s\" is not bound\n", Cmd_Argv(1) );
		return;
	}

// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	for (i=2 ; i< c ; i++)
	{
		strcat (cmd, Cmd_Argv(i));
		if (i != (c-1))
			strcat (cmd, " ");
	}

	Key_SetCtrlBinding (b, cmd);
}

/*
============
Key_WriteBindings

Writes lines containing "bind key value"
============
*/
void Key_WriteBindings( fileHandle_t f ) {
	int		i;

	FS_Printf (f, "unbindall\n" );

	for (i=0 ; i<MAX_KEYS ; i++) {
		if (keys[i].binding && keys[i].binding[0] ) {
			FS_Printf (f, "bind %s \"%s\"\n", Key_KeynumToString(i), keys[i].binding);

		}

	}
}


/*
============
Key_Bindlist_f

============
*/
void Key_Bindlist_f( void ) {
	int		i;

	for ( i = 0 ; i < MAX_KEYS ; i++ ) {
		if ( keys[i].binding && keys[i].binding[0] ) {
			Com_Printf( "%s \"%s\"\n", Key_KeynumToString(i), keys[i].binding );
		}
	}
}

/*
============
Key_AltBindlist_f

============
*/
void Key_AltBindlist_f( void ) {
	int		i;

	for ( i = 0 ; i < MAX_KEYS ; i++ ) {
		if ( altkeys[i].binding && altkeys[i].binding[0] ) {
			Com_Printf( "%s \"%s\"\n", Key_KeynumToString(i), altkeys[i].binding );
		}
	}
}

/*
============
Key_CtrlBindlist_f

============
*/
void Key_CtrlBindlist_f( void ) {
	int		i;

	for ( i = 0 ; i < MAX_KEYS ; i++ ) {
		if ( ctrlkeys[i].binding && ctrlkeys[i].binding[0] ) {
			Com_Printf( "%s \"%s\"\n", Key_KeynumToString(i), ctrlkeys[i].binding );
		}
	}
}

/*
============
Key_KeynameCompletion
============
*/
void Key_KeynameCompletion( void(*callback)(const char *s) ) {
	int		i;

	for( i = 0; keynames[ i ].name != NULL; i++ )
		callback( keynames[ i ].name );
}

/*
===================
CL_InitKeyCommands
===================
*/
void CL_InitKeyCommands( void ) {
	// register our functions
	Cmd_AddCommand ("bind",Key_Bind_f);
	Cmd_AddCommand ("unbind",Key_Unbind_f);
	Cmd_AddCommand ("altbind", Key_AltBind_f);
	Cmd_AddCommand ("unaltbind", Key_Unaltbind_f);
	Cmd_AddCommand ("ctrlbind", Key_CtrlBind_f);
	Cmd_AddCommand ("unctrlbind", Key_Unctrlbind_f);
	Cmd_AddCommand ("unbindall",Key_Unbindall_f);
	Cmd_AddCommand ("bindlist",Key_Bindlist_f);
	Cmd_AddCommand ("altbindlist", Key_AltBindlist_f);
	Cmd_AddCommand ("ctrlbindlist", Key_AltBindlist_f);

	memset( menubound, 0, sizeof( menubound ) );
}

/*
===================
CL_AddKeyUpCommands
===================
*/
void CL_AddKeyUpCommands( int key, char *kb, unsigned time) {
	int i;
	char button[1024], *buttonPtr;
	char	cmd[1024];
	qboolean keyevent;

	if ( !kb ) {
		return;
	}
	keyevent = qfalse;
	buttonPtr = button;
	for ( i = 0; ; i++ ) {
		if ( kb[i] == ';' || !kb[i] ) {
			*buttonPtr = '\0';
			if ( button[0] == '+') {
				// button commands add keynum and time as parms so that multiple
				// sources can be discriminated and subframe corrected
				Com_sprintf (cmd, sizeof(cmd), "-%s %i %i\n", button+1, key, time);
				Cbuf_AddText (cmd);
				keyevent = qtrue;
			} else {
				if (keyevent) {
					// down-only command
					Cbuf_AddText (button);
					Cbuf_AddText ("\n");
				}
			}
			buttonPtr = button;
			while ( (kb[i] <= ' ' || kb[i] == ';') && kb[i] != 0 ) {
				i++;
			}
		}
		*buttonPtr++ = kb[i];
		if ( !kb[i] ) {
			break;
		}
	}
}

static int last_escape_time;

/*
===================
CL_KeyEvent

Called by the system for both key up and key down events
===================
*/
void CL_KeyEvent (int key, qboolean down, unsigned time) {
	char	*kb;
	char	cmd[1024];

	// update auto-repeat status and BUTTON_ANY status
	keys[key].down = down;

	if (down) {
		keys[key].repeats++;
		if ( keys[key].repeats == 1) {
			anykeydown++;
		}
	} else {
		keys[key].repeats = 0;
		anykeydown--;
		if (anykeydown < 0) {
			anykeydown = 0;
		}
	}

	if( ( key == K_ALT || key == K_INS ) && cl_altbindings->integer ) {
		alt_down = down;
	}

	if( ( key == K_CTRL || key == K_DEL ) && cl_ctrlbindings->integer ) {
		ctrl_down = down;
	}

	if (down && !CL_FinishedIntro())
	{
		UI_StartStageKeyEvent();
		return;
	}

	if( clc.state > CA_DISCONNECTED && cge && !Key_GetCatcher() && cge->CG_CheckCaptureKey( key, down, time ) && key != K_ESCAPE )
	{
		if (key == K_CONSOLE || (keys[K_SHIFT].down && key == K_ESCAPE))
			return;
	}

	if (key == K_CONSOLE || (keys[K_SHIFT].down && key == K_ESCAPE))
	{
		if( !down ) {
			return;
		}

		if( Cvar_VariableIntegerValue( "ui_console" ) )
		{
			UI_ToggleConsole();
		}
		else if( clc.state == CA_ACTIVE )
		{
			if( cg_gametype->integer ) {
				UI_ToggleDMConsole( 300 );
			}
		}
		return;
	}

	if( down )
	{
		if( alt_down && key != K_ALT && key != K_INS ) {
			altkeys[ key ].down = down;
		}
		if( down && ctrl_down && key != K_CTRL ) {
			ctrlkeys[ key ].down = down;
		}
	}

	if( key != K_DEL ) {
		ctrlkeys[ key ].down = down;
	}

	if (key == K_ENTER)
	{
		if (down)
		{
			if (keys[K_ALT].down)
			{
				Key_ClearStates();
				Cvar_SetValue( "r_fullscreen",
						!Cvar_VariableIntegerValue( "r_fullscreen" ) );
				return;
			}
		}
	}


	if( key == K_MOUSE1 || key == K_MOUSE2
		|| key == K_MOUSE3 || key == K_MOUSE4
		|| key == K_MOUSE5 )
	{
		if( down ) {
			cl.mouseButtons |= ( 1 << ( key + (256 - K_MOUSE1) ) );
		} else {
			cl.mouseButtons &= ~( 1 << ( key + (256 - K_MOUSE1)) );
		}
	}

	if (!in_guimouse || key <= K_MOUSE3 || key > K_JOY1)
	{
		// keys can still be used for bound actions
		if (down && (key <= K_BACKSPACE || key == K_MOUSE4) &&
			( clc.demoplaying || clc.state == CA_CINEMATIC ) && Key_GetCatcher() == 0 ) {
			Cvar_Set( "nextdemo", "" );
			key = K_ESCAPE;
		}

		// escape is always handled special
		if( key == K_ESCAPE ) {
			if( down ) {
				qboolean wasup = UI_MenuUp();
				UI_DeactiveFloatingWindows();

				if( clc.state == CA_CINEMATIC )
				{
					SCR_StopCinematic();
					return;
				}

				if( cls.realFrametime - last_escape_time <= 1000 ) {
					UI_MenuEscape( "main" );
					return;
				}

				if( wasup ) {
					UI_MenuEscape( "main" );
					return;
				}

				if( cl.snap.ps.stats[ STAT_CINEMATIC ] & 1 && clc.state == CA_ACTIVE ) {
					Cbuf_AddText( "skipcinematic\n" );
					return;
				}

				if( clc.state <= CA_DISCONNECTED || clc.state == CA_ACTIVE ) {
					UI_MenuEscape( "main" );
				}
				return;
			}
		} else if( down ) {
			if ( ( Key_GetCatcher( ) & KEYCATCH_UI && !menubound[ key ] ) || UI_BindActive() ) {
				UI_KeyEvent( key, time );
			} else if( cls.loading & KEYCATCH_MESSAGE ) {
				Message_Key( key );
			} else if( clc.state != CA_DISCONNECTED || menubound[key]) {
				// send the bound action
				kb = altkeys[ key ].binding;
				if( !kb || !altkeys[ key ].down ) {
					kb = ctrlkeys[ key ].binding;
					if( !kb || !ctrlkeys[ key ].down ) {
						kb = keys[ key ].binding;
					}
				}
				if( !kb ) {
					if( key >= 200 ) {
						Com_Printf( "%s is unbound, use controls menu to set.\n"
							, Key_KeynumToString( key ) );
					}
				} else if( kb[ 0 ] == '+' ) {
					int i;
					char button[ 1024 ], *buttonPtr;
					buttonPtr = button;
					for( i = 0;; i++ ) {
						if( kb[ i ] == ';' || !kb[ i ] ) {
							*buttonPtr = '\0';
							if( button[ 0 ] == '+' ) {
								// button commands add keynum and time as parms so that multiple
								// sources can be discriminated and subframe corrected
								Com_sprintf( cmd, sizeof( cmd ), "%s %i %i\n", button, key, time );
								Cbuf_AddText( cmd );
							}
							else {
								// down-only command
								Cbuf_AddText( button );
								Cbuf_AddText( "\n" );
							}
							buttonPtr = button;
							while( ( kb[ i ] <= ' ' || kb[ i ] == ';' ) && kb[ i ] != 0 ) {
								i++;
							}
						}
						*buttonPtr++ = kb[ i ];
						if( !kb[ i ] ) {
							break;
						}
					}
				} else {
					// down-only command
					Cbuf_AddText( kb );
					Cbuf_AddText( "\n" );
				}
			}
			return;
		}

		if( altkeys[ key ].down ) {
			kb = altkeys[ key ].binding;
			altkeys[ key ].down = false;
			CL_AddKeyUpCommands( key, kb, time );
		}
		if( ctrlkeys[ key ].down ) {
			kb = ctrlkeys[ key ].binding;
			ctrlkeys[ key ].down = false;
			CL_AddKeyUpCommands( key, kb, time );
		}

		CL_AddKeyUpCommands( key, keys[ key ].binding, time );
	}
}


/*
===================
CL_CharEvent

Normal keyboard characters, already shifted / capslocked / etc
===================
*/
void CL_CharEvent( int key ) {
	// the console key should never be used as a char
	if ( key == '`' || key == '~' ) {
		return;
	}

	// delete is not a printable character and is
	// otherwise handled by Field_KeyDownEvent
	if ( key == 127 ) {
		return;
	}

	// distribute the key down event to the apropriate handler
	if ( Key_GetCatcher( ) & KEYCATCH_CONSOLE ) {
		return;
	}

	if ( Key_GetCatcher( ) & KEYCATCH_UI ) {
		UI_CharEvent( key );
	}
}


/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates (void)
{
	int		i;

	anykeydown = 0;

	for ( i=0 ; i < MAX_KEYS ; i++ ) {
		if ( keys[i].down ) {
			CL_KeyEvent( i, qfalse, 0 );

		}
		keys[i].down = 0;
		keys[i].repeats = 0;
	}
}

static int keyCatchers = 0;

/*
====================
Key_GetCatcher
====================
*/
int Key_GetCatcher( void ) {
	return cls.keyCatchers;
}

/*
====================
Key_SetCatcher
====================
*/
void Key_SetCatcher( int catcher ) {
	keyCatchers = catcher;
}
