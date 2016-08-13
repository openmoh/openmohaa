/*
===========================================================================
Copyright (C) 2008 the OpenMoHAA team

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

// g_spawn.cpp : spawner for scripts.

#include "glb_local.h"
#include "g_spawn.h"
#include <scriptvariable.h>
#include <object.h>

#ifdef GAME_DLL
#include "../game/entity.h"
#include "../game/actor.h"
#endif

#if defined ( GAME_DLL )
#define scriptcheck g_scriptcheck
#elif defined( CGAME_DLL )
#define scriptcheck cg_scriptcheck
#else
#define scriptcheck g_scriptcheck
#endif

CLASS_DECLARATION( Class, SpawnArgs, NULL )
{
	{ NULL, NULL }
};

Container< SafePtr< Listener > > g_spawnlist;

SpawnArgs::SpawnArgs()
{
}

SpawnArgs::SpawnArgs( SpawnArgs &otherlist )
{
	int num;
	int i;
	
	num = otherlist.NumArgs();
	keyList.Resize( num );
	valueList.Resize( num );
	for( i = 1; i <= num; i++ )
	{
		keyList.AddObject( otherlist.keyList.ObjectAt( i ) );
		valueList.AddObject( otherlist.valueList.ObjectAt( i ) );
	}
}

void SpawnArgs::Clear( void )
{
	keyList.FreeObjectList();
	valueList.FreeObjectList();
}

/*
====================
Parse

Parses spawnflags out of the given string, returning the new position.
Clears out any previous args.
====================
*/
char *SpawnArgs::Parse( char *data, bool bAllowUtils )
{
	str         keyname;
	const char	*com_token;
	
	Clear();
	
	// parse the opening brace
	com_token = COM_Parse( &data );
	if ( !data )
	{
		return NULL;
	}
	
	if ( com_token[ 0 ] != '{' )
	{
		glbs.Error( ERR_DROP, "SpawnArgs::Parse : found %s when expecting {", com_token );
	}
	
	// go through all the dictionary pairs
	while( 1 )
	{
		// parse key
		com_token = COM_Parse( &data );
		if ( com_token[ 0 ] == '}' )
		{
			break;
		}
		
		if ( !data )
		{
			glbs.Error( ERR_DROP, "SpawnArgs::Parse : EOF without closing brace" );
		}
		
		keyname = com_token;
		
		// parse value
		com_token = COM_Parse( &data );
		if ( !data )
		{
			glbs.Error( ERR_DROP, "SpawnArgs::Parse : EOF without closing brace" );
		}
		
		if ( com_token[ 0 ] == '}' )
		{
			glbs.Error( ERR_DROP, "SpawnArgs::Parse : closing brace without data" );
		}
		
		// keynames with a leading underscore are used for utility comments,
		// and are immediately discarded by the game
		if ( !bAllowUtils && keyname[ 0 ] == '_' )
		{
			continue;
		}
		
		setArg( keyname.c_str(), com_token );
	}
	
	return data;
}

const char *SpawnArgs::getArg( const char *key, const char *defaultValue )
{
	int i;
	int num;
	
	num = keyList.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		if ( keyList.ObjectAt( i ) == key )
		{
			return valueList.ObjectAt( i );
		}
	}
	
	return defaultValue;
}

void SpawnArgs::setArg( const char *key, const char *value )
{
	int i;
	int num;
	
	num = keyList.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		if ( keyList.ObjectAt( i ) == key )
		{
			valueList.ObjectAt( i ) = value;
			return;
		}
	}
	
	keyList.AddObject( str( key ) );
	valueList.AddObject( str( value ) );
}

void SpawnArgs::operator=( SpawnArgs &otherlist )
{
	int num;
	int i;
	
	Clear();
	
	num = otherlist.NumArgs();
	keyList.Resize( num );
	valueList.Resize( num );
	
	for( i = 1; i <= num; i++ )
	{
		keyList.AddObject( otherlist.keyList.ObjectAt( i ) );
		valueList.AddObject( otherlist.valueList.ObjectAt( i ) );
	}
}

int SpawnArgs::NumArgs( void )
{
	return keyList.NumObjects();
}

const char *SpawnArgs::getKey( int index )
{
	return keyList.ObjectAt( index + 1 );
}

const char *SpawnArgs::getValue( int index )
{
	return valueList.ObjectAt( index + 1 );
}

void SpawnArgs::Archive( Archiver &arc )
{
	Class::Archive( arc );
	
	//keyList.Archive( arc );
	//valueList.Archive( arc );
}

/*
===============
getClass

Finds the spawn function for the entity and returns ClassDef *
===============
*/

ClassDef *SpawnArgs::getClassDef( qboolean *tikiWasStatic )
{
#if defined ( GAME_DLL )
	const char	*classname;
	ClassDef	*cls = NULL;
	dtiki_t		*tiki = NULL;
	dtikianim_t	*a = NULL;
	
	classname = getArg( "classname" );
	
	if ( tikiWasStatic )
	{
		*tikiWasStatic = false;
	}
	
	//
	// check normal spawn functions
	// see if the class name is stored within the model
	//
	if ( classname )
	{
		//
		// explicitly inhibit lights
		//
		if ( !Q_stricmp( classname, "light" ) )
		{
			//
			// HACK HACK HACK
			// hack to suppress a warning message
			//
			if ( tikiWasStatic )
            {
				*tikiWasStatic = true;
            }
			return NULL;
		}
		
		cls = getClassForID( classname );
		if ( !cls )
		{
			cls = getClass( classname );
		}
	}
	
	if ( !cls )
	{
		const char *model;
		
		//
		// get Object in case we cannot find an alternative
		//
		cls = &Object::ClassInfo;
		model = getArg( "model" );
		if ( model )
		{
			int i;

			model = CanonicalTikiName( model );
			tiki = gi.modeltiki( model );

			//
			// get handle to def file
			//
			if ( tiki )
            {
				const char * s;
				
				s = getArg( "make_static" );

				if ( s && atoi( s ) )
				{
					//
					// if make_static then we don't want to spawn
					//
					if ( tikiWasStatic )
					{
						*tikiWasStatic = true;
					}
					
					return NULL;
				}

				a = tiki->a;
				
				if( a->num_server_initcmds )
				{
					for( i = 0; i < a->num_server_initcmds; i++ )
					{
						if( !Q_stricmp( a->server_initcmds[ i ].args[ 0 ], "classname" ) )
						{
							cls = getClass( a->server_initcmds[ i ].args[ 1 ] );
							break;
						}
					}

					if( i == a->num_server_initcmds )
					{
						glbs.DPrintf( "Classname %s used, but 'classname' was not found in Initialization commands, using Object.\n", classname );
					}
				}
				else
					glbs.DPrintf( "Classname %s used, but TIKI had no Initialization commands, using Object.\n", classname );
            }
			else
				glbs.DPrintf( "Classname %s used, but model was not a TIKI, using Object.\n", classname );
		}
		else
		{
			glbs.DPrintf( "Classname %s' used, but no model was set, using Object.\n", classname );
		}
	}
	
	return cls;
#else
	const char	*classname;
	ClassDef	*cls = NULL;
	
	classname = getArg( "classname" );
	
	if ( tikiWasStatic )
	{
		*tikiWasStatic = false;
	}
	
	//
	// check normal spawn functions
	// see if the class name is stored within the model
	//
	if ( classname )
	{
		cls = getClassForID( classname );
		if ( !cls )
		{
			cls = getClass( classname );
		}
	}

	return cls;
#endif
}

/*
===============
Spawn

Finds the spawn function for the entity and calls it.
Returns pointer to Entity
===============
*/

Listener *SpawnArgs::Spawn( void )
{
	Listener *ent = ( Listener * )SpawnInternal();

	if( ent )
	{
		ent->ProcessPendingEvents();
	}

	return ent;
}

Listener *SpawnArgs::SpawnInternal( void )
{
	str				classname;
	ClassDef		*cls;
	Listener		*obj;
	Event			*ev;
	int				i;
	qboolean		tikiWasStatic; // used to determine if entity was intentionally suppressed
	const char		*key, *value;
	str				keyname;
#ifndef NO_SCRIPTENGINE
	unsigned int	eventnum;
	EventDef		*def;
	ScriptVariable	*var;
	ScriptVariableList *varList;
#endif

#if defined ( GAME_DLL )

	if( !g_spawnentities->integer )
	{
		value = getArg( "classname", NULL );

		// only spawn a playerstart or the world
		if( value )
		{
			if( ( Q_stricmp( value, "info_player_start" ) && Q_stricmp( value, "worldspawn" ) ) )
			{
				Com_Printf( "%s\n", value );
				return NULL;
			}
		}
	}

	if( g_spawnai->integer )
	{
		value = getArg( "classname", NULL );

		if( value )
		{
			if( !Q_stricmpn( value, "ai_", 3 ) )
			{
				Com_Printf( "%s\n", value );
				return NULL;
			}
		}
	}

#endif
	
	classname = getArg( "classname", "Unspecified" );
	cls = getClassDef( &tikiWasStatic );

	if ( !cls )
	{
		if ( !tikiWasStatic )
		{
			glbs.DPrintf( "%s doesn't have a spawn function\n", classname.c_str() );

			if( scriptcheck->integer ) {
				glbs.Error( ERR_DROP, "Script check failed" );
			}
		}
		
		return NULL;
	}
	
	obj = ( Listener * )cls->newInstance();

	for( int i = g_spawnlist.NumObjects(); i > 0; i-- )
	{
		if( g_spawnlist.ObjectAt( i ) == NULL )
		{
			g_spawnlist.RemoveObjectAt( i );
		}
	}

	g_spawnlist.AddObject( obj );

	// post spawnarg events
	for( i = 0; i < NumArgs(); i++ )
	{
		key = getKey( i );
		value = getValue( i );

		if( *key == '#' )
		{
			// don't count the prefix
			keyname = ( key + 1 );

#ifndef NO_SCRIPTENGINE
			// initialize the object vars
			varList = obj->Vars();

			var = varList->GetVariable( keyname );
			if( var )
			{
				Com_Printf(
					"^~^~^ variable '%s' already set with string value '%s' - failed to attempt to set with numeric value '%s'\n",
					keyname.c_str(),
					var->stringValue().c_str(),
					value );

				continue;
			}

			eventnum = Event::FindSetterEventNum( keyname );

			if( !eventnum || !( def = cls->GetDef( eventnum ) ) )
			{
				if( strchr( keyname.c_str(), '.' ) )
				{
					// it's a float
					varList->SetVariable( keyname, ( float )atof( value ) );
				}
				else
				{
					varList->SetVariable( keyname, atoi( value ) );
				}
			}
			else if( def->type != EV_SETTER )
			{
				Com_Printf( "^~^~^ Cannot set a read-only variable '%s'\n", keyname.c_str() );
			}
			else
			{
				ev = new Event( keyname, EV_SETTER );

				if( strchr( keyname.c_str(), '.' ) )
				{
					ev->AddFloat( atof( value ) );
				}
				else
				{
					ev->AddInteger( atoi( value ) );
				}

				obj->PostEvent( ev, EV_SPAWNARG );
			}
#endif
		}
		else if( *key == '$' )
		{
			keyname = ( key + 1 );

#ifndef NO_SCRIPTENGINE
			varList = obj->Vars();

			var = varList->GetVariable( keyname );
			if( var )
			{
				Com_Printf(
					"^~^~^ variable '%s' already set with string value '%s' - failed to attempt to set with string value '%s'\n",
					keyname.c_str(),
					var->stringValue().c_str(),
					value );

				continue;
			}

			eventnum = Event::FindSetterEventNum( keyname );

			if( !eventnum || !( def = cls->GetDef( eventnum ) ) )
			{
				varList->SetVariable( keyname, value );
			}
			else if( def->type != EV_SETTER )
			{
				Com_Printf( "^~^~^ Cannot set a read-only variable '%s'\n", keyname.c_str() );
			}
			else
			{
				ev = new Event( keyname, EV_SETTER );
				ev->AddString( value );

				obj->PostEvent( ev, EV_SPAWNARG );
			}
#endif
		}
		// if it is the "script" key, execute the script commands individually
		else if( !Q_stricmp( key, "script" ) )
		{
			char *ptr;
			char * token;
			
			ptr = const_cast< char * >( getValue( i ) );
			while ( 1 )
            {
				token = COM_ParseExt( &ptr, true );
				if ( !token[ 0 ] )
					break;
				if ( strchr( token, ':' ) )
				{
					glbs.DPrintf( "Label %s imbedded inside editor script for %s.\n", token, classname.c_str() );
				}
				else
				{
					ev = new Event( token );
					while ( 1 )
					{
						token = COM_ParseExt( &ptr, false );
						if ( !token[ 0 ] )
							break;
						ev->AddToken( token );
					}
					
					obj->PostEvent( ev, EV_SPAWNARG );
				}
            }
		}
		else
		{
			ev = new Event( key );
			ev->AddToken( value );
			
			if( Q_stricmp( key, "model" ) == 0 )
			{
#if defined ( GAME_DLL )
				if( obj->isSubclassOf( SimpleActor ) )
				{
					if( Q_stricmpn( key, "human", 5 ) &&
						Q_stricmpn( key, "models/human", 12 ) &&
						Q_stricmpn( key, "models//human", 13 ) &&
						Q_stricmpn( key, "animal", 6 ) &&
						Q_stricmpn( key, "models/animal", 13 ) )
					{
						Com_Printf( "^~^~^ model '%s' cannot be an actor - please fix the .tik file\n", value );

						delete obj;
						return NULL;
					}

					obj->PostEvent( ev, EV_PRIORITY_SPAWNACTOR );
				}
				else
				{
					obj->PostEvent( ev, EV_PRIORITY_SPAWNARG );
				}
#else
				obj->PostEvent( ev, EV_PRIORITY_SPAWNARG );
#endif
			}
			else
			{
#if defined ( GAME_DLL )
				if( obj->isSubclassOf( SimpleActor ) )
				{
					obj->PostEvent( ev, EV_SPAWNACTOR );
				}
				else
				{
					obj->PostEvent( ev, EV_SPAWNARG );
				}
#else
				obj->PostEvent( ev, EV_SPAWNARG );
#endif
			}
		}
	}
	
	if ( !obj )
	{
		glbs.DPrintf( "%s failed on newInstance\n", classname.c_str() );
		return NULL;
	}
	
	return obj;
}

ClassDef *FindClass( const char *name, qboolean *isModel )
{
	ClassDef *cls;
	
	*isModel = false;
	
	// first lets see if it is a registered class name
	cls = getClass( name );
	if ( !cls )
	{
		SpawnArgs args;
		
		// if that didn't work lets try to resolve it as a model
		args.setArg( "model", name );
		
		cls = args.getClassDef();
		if ( cls )
		{
			*isModel = true;
		}
	}
	return cls;
}

#if defined ( GAME_DLL )

/*
==============
G_InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void G_InitClientPersistant( gclient_t *client )
{
	memset( &client->pers, 0, sizeof( client->pers ) );
}

#endif

#if 0

#include "../qcommon/tiki_local.h" // fixme!

qboolean	G_SpawnString( const char *key, const char *defaultString, char **out ) {
	int		i;

	if ( !level.spawning ) {
		*out = (char *)defaultString;
//		G_Error( "G_SpawnString() called while not spawning" );
	}

	for ( i = 0 ; i < level.numSpawnVars ; i++ ) {
		if ( !Q_stricmp( key, level.spawnVars[i][0] ) ) {
			*out = level.spawnVars[i][1];
			return qtrue;
		}
	}

	*out = (char *)defaultString;
	return qfalse;
}

qboolean	G_SpawnFloat( const char *key, const char *defaultString, float *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atof( s );
	return present;
}

qboolean	G_SpawnInt( const char *key, const char *defaultString, int *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atoi( s );
	return present;
}

qboolean	G_SpawnVector( const char *key, const char *defaultString, float *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	sscanf( s, "%f %f %f", &out[0], &out[1], &out[2] );
	return present;
}

//
// fields are needed for spawning from the entity string
//
typedef enum {
	F_INT,
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_ENTITY,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory
	F_IGNORE
} fieldtype_t;

typedef struct
{
	char	*name;
	int		ofs;
	fieldtype_t	type;
	int		flags;
} gfield_t;

gfield_t fields[] = {
	{"classname", FOFS(classname), F_LSTRING},
	{"origin", FOFS(s.origin), F_VECTOR},
	{"model", FOFS(model), F_LSTRING},
	{"model2", FOFS(model2), F_LSTRING},
	{"spawnflags", FOFS(spawnflags), F_INT},
	{"speed", FOFS(speed), F_FLOAT},
	{"target", FOFS(target), F_LSTRING},
	{"targetname", FOFS(targetname), F_LSTRING},
	{"message", FOFS(message), F_LSTRING},
	{"team", FOFS(team), F_LSTRING},
	{"wait", FOFS(wait), F_FLOAT},
	{"random", FOFS(random), F_FLOAT},
	{"count", FOFS(count), F_INT},
	{"health", FOFS(health), F_INT},
	{"light", 0, F_IGNORE},
	{"dmg", FOFS(damage), F_INT},
	{"angles", FOFS(s.angles), F_VECTOR},
	{"angle", FOFS(s.angles), F_ANGLEHACK},
	{"targetShaderName", FOFS(targetShaderName), F_LSTRING},
	{"targetShaderNewName", FOFS(targetShaderNewName), F_LSTRING},

	{NULL}
};


typedef struct {
	char	*name;
	void	(*spawn)(gentity_t *ent);
} spawn_t;

void SP_info_player_start (gentity_t *ent);
void SP_info_player_deathmatch (gentity_t *ent);
void SP_info_player_intermission (gentity_t *ent);
void SP_info_firstplace(gentity_t *ent);
void SP_info_secondplace(gentity_t *ent);
void SP_info_thirdplace(gentity_t *ent);
void SP_info_podium(gentity_t *ent);

void SP_func_plat (gentity_t *ent);
void SP_func_static (gentity_t *ent);
void SP_func_rotating (gentity_t *ent);
void SP_func_bobbing (gentity_t *ent);
void SP_func_pendulum( gentity_t *ent );
void SP_func_button (gentity_t *ent);
void SP_func_door (gentity_t *ent);
void SP_func_rotatingdoor (gentity_t *ent);	// IneQuation
void SP_func_train (gentity_t *ent);
void SP_func_timer (gentity_t *self);

void SP_trigger_always (gentity_t *ent);
void SP_trigger_multiple (gentity_t *ent);
void SP_trigger_push (gentity_t *ent);
void SP_trigger_teleport (gentity_t *ent);
void SP_trigger_hurt (gentity_t *ent);

void SP_target_remove_powerups( gentity_t *ent );
void SP_target_give (gentity_t *ent);
void SP_target_delay (gentity_t *ent);
void SP_target_speaker (gentity_t *ent);
void SP_target_print (gentity_t *ent);
//void SP_target_laser (gentity_t *self);
void SP_target_character (gentity_t *ent);
void SP_target_score( gentity_t *ent );
void SP_target_teleporter( gentity_t *ent );
void SP_target_relay (gentity_t *ent);
void SP_target_kill (gentity_t *ent);
void SP_target_position (gentity_t *ent);
void SP_target_location (gentity_t *ent);
void SP_target_push (gentity_t *ent);

void SP_light (gentity_t *self);
void SP_info_null (gentity_t *self);
void SP_info_notnull (gentity_t *self);
void SP_info_camp (gentity_t *self);
void SP_path_corner (gentity_t *self);

void SP_misc_teleporter_dest (gentity_t *self);
void SP_misc_model(gentity_t *ent);
void SP_misc_portal_camera(gentity_t *ent);
void SP_misc_portal_surface(gentity_t *ent);

void SP_shooter_rocket( gentity_t *ent );
void SP_shooter_plasma( gentity_t *ent );
void SP_shooter_grenade( gentity_t *ent );

void SP_team_CTF_redplayer( gentity_t *ent );
void SP_team_CTF_blueplayer( gentity_t *ent );

void SP_team_CTF_redspawn( gentity_t *ent );
void SP_team_CTF_bluespawn( gentity_t *ent );

#ifdef MISSIONPACK
void SP_team_blueobelisk( gentity_t *ent );
void SP_team_redobelisk( gentity_t *ent );
void SP_team_neutralobelisk( gentity_t *ent );
#endif
void SP_item_botroam( gentity_t *ent ) { }

spawn_t	spawns[] = {
	// info entities don't do anything at all, but provide positional
	// information for things controlled by other processes
	{"info_player_start", SP_info_player_start},
	{"info_player_deathmatch", SP_info_player_deathmatch},
	{"info_player_intermission", SP_info_player_intermission},
	{"info_null", SP_info_null},
	{"info_notnull", SP_info_notnull},		// use target_position instead
	{"info_camp", SP_info_camp},

	{"func_plat", SP_func_plat},
	{"func_button", SP_func_button},
	//{"func_door", SP_func_door},
	{"func_rotatingdoor", SP_func_rotatingdoor},	// IneQuation
	{"func_static", SP_func_static},
	{"func_rotating", SP_func_rotating},
	{"func_bobbing", SP_func_bobbing},
	{"func_pendulum", SP_func_pendulum},
	{"func_train", SP_func_train},
	{"func_group", SP_info_null},
	{"func_timer", SP_func_timer},			// rename trigger_timer?

	// Triggers are brush objects that cause an effect when contacted
	// by a living player, usually involving firing targets.
	// While almost everything could be done with
	// a single trigger class and different targets, triggered effects
	// could not be client side predicted (push and teleport).
	{"trigger_always", SP_trigger_always},
	{"trigger_multiple", SP_trigger_multiple},
	{"trigger_push", SP_trigger_push},
	{"trigger_teleport", SP_trigger_teleport},
	{"trigger_hurt", SP_trigger_hurt},

	// targets perform no action by themselves, but must be triggered
	// by another entity
	{"target_give", SP_target_give},
	{"target_remove_powerups", SP_target_remove_powerups},
	{"target_delay", SP_target_delay},
	{"target_speaker", SP_target_speaker},
	{"target_print", SP_target_print},
	//{"target_laser", SP_target_laser},
	{"target_score", SP_target_score},
	{"target_teleporter", SP_target_teleporter},
	{"target_relay", SP_target_relay},
	{"target_kill", SP_target_kill},
	{"target_position", SP_target_position},
	{"target_location", SP_target_location},
	{"target_push", SP_target_push},

	{"light", SP_light},
	{"path_corner", SP_path_corner},

	{"misc_teleporter_dest", SP_misc_teleporter_dest},
	{"misc_model", SP_misc_model},
	{"misc_portal_surface", SP_misc_portal_surface},
	{"misc_portal_camera", SP_misc_portal_camera},

	{"shooter_rocket", SP_shooter_rocket},
	{"shooter_grenade", SP_shooter_grenade},
	{"shooter_plasma", SP_shooter_plasma},

	// IneQuation
	{"info_player_axis", SP_team_CTF_redspawn},
	{"info_player_allied", SP_team_CTF_bluespawn},

#ifdef MISSIONPACK
	{"team_redobelisk", SP_team_redobelisk},
	{"team_blueobelisk", SP_team_blueobelisk},
	{"team_neutralobelisk", SP_team_neutralobelisk},
#endif
	{"item_botroam", SP_item_botroam},

	{NULL, 0}
};

const char *G_FixTIKIPath(const char *in) {
	static char path[256];
	void *buffer;

	if(in[0] == '*')
		return in; // that's a bsp inline model
	if(gi.FS_ReadFile( in, &buffer ) > 0) {
		return in;
	}
	strcpy(path,"models/");
	strcat(path,in);
	if( gi.FS_ReadFile( in, &buffer ) > 0 ) {
		return path;
	}
	return in;
}

/*
===============
G_CallSpawn

Finds the spawn function for the entity and calls it,
returning qfalse if not found
===============
*/
qboolean G_CallSpawn( gentity_t *ent ) {
	spawn_t	*s;
	tiki_t *tiki;

	if ( !ent->classname ) {
		G_Printf ("G_CallSpawn: NULL classname\n");
		return qfalse;
	}

	// check normal spawn functions
	for ( s=spawns ; s->name ; s++ ) {
		if ( !strcmp(s->name, ent->classname) ) {
			// found it
			s->spawn(ent);
			return qtrue;
		}
	}

	// try to spawn a tiki model
	if(ent->model && ent->model[0] /*&& ent->model[0] != '*'*/) {
		if(ent->model[0] != '*') {
			char *ptr1, *ptr2;
			ptr1 = ent->model;
			ptr2 = ent->model;
			// su44: fix double '/' in TIKI paths from entities lump.
			// The same issue is also in R_LoadStaticModels.
			while(*ptr1) {
				if(ptr1[0] == '/' && ptr1[1] == '/') {
					ptr1++;
				}
				*ptr2 = *ptr1;
				ptr1++;
				ptr2++;
			}
			*ptr2 = 0;
		}
#if 0
		tiki = gi.TIKI_RegisterModel(ent->model);
		if(!tiki) {
			tiki = gi.TIKI_RegisterModel(va("models/%s",ent->model));
		}
		if(tiki) {
			float dummy;
			ent->s.modelindex = G_ModelIndex(tiki->name);
			ent->s.eType = ET_MODELANIM;
			gi.TIKI_AppendFrameBoundsAndRadius(tiki,0,0,&dummy,ent->r.mins);
			ent->s.frameInfo[0].weight = 1.f;
			G_SetOrigin( ent, ent->s.origin );
			VectorCopy( ent->s.angles, ent->s.angles );
			gi.LinkEntity (ent);
			return qtrue;
		}
#else
		ent->s.modelindex = G_ModelIndex( ( char * )G_FixTIKIPath( ent->model ) );
		ent->s.eType = ET_MODELANIM;
		//if(ent->model[0]=='*') {
		//	gi.SetBrushModel(ent, ent->model);
		//} else
		{
			VectorSet (ent->r.mins, -128, -128, -128);
			VectorSet (ent->r.maxs, 128, 128, 128);			
		}
		ent->s.frameInfo[0].weight = 1.f;
		G_SetOrigin( ent, ent->s.origin );
		gi.LinkEntity (ent);
		return qtrue;
#endif
	}

	G_Printf ("%s doesn't have a spawn function\n", ent->classname);
	return qfalse;
}

/*
=============
G_NewString

Builds a copy of the string, translating \n to real linefeeds
so message texts can be multi-line
=============
*/
char *G_NewString( const char *string ) {
	char	*newb, *new_p;
	int		i,l;

	l = strlen(string) + 1;

	newb = ( char * )G_Alloc( l );

	new_p = newb;

	// turn \n into a real linefeed
	for ( i=0 ; i< l ; i++ ) {
		if (string[i] == '\\' && i < l-1) {
			i++;
			if (string[i] == 'n') {
				*new_p++ = '\n';
			} else {
				*new_p++ = '\\';
			}
		} else {
			*new_p++ = string[i];
		}
	}

	return newb;
}




/*
===============
G_ParseField

Takes a key/value pair and sets the binary values
in a gentity
===============
*/
void G_ParseField( const char *key, const char *value, gentity_t *ent ) {
	gfield_t	*f;
	byte	*b;
	float	v;
	vec3_t	vec;

	for ( f=fields ; f->name ; f++ ) {
		if ( !Q_stricmp(f->name, key) ) {
			// found it
			b = (byte *)ent;

			switch( f->type ) {
			case F_LSTRING:
				*(char **)(b+f->ofs) = G_NewString (value);
				break;
			case F_VECTOR:
				sscanf (value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
				((float *)(b+f->ofs))[0] = vec[0];
				((float *)(b+f->ofs))[1] = vec[1];
				((float *)(b+f->ofs))[2] = vec[2];
				break;
			case F_INT:
				*(int *)(b+f->ofs) = atoi(value);
				break;
			case F_FLOAT:
				*(float *)(b+f->ofs) = atof(value);
				break;
			case F_ANGLEHACK:
				v = atof(value);
				((float *)(b+f->ofs))[0] = 0;
				((float *)(b+f->ofs))[1] = v;
				((float *)(b+f->ofs))[2] = 0;
				break;
			default:
			case F_IGNORE:
				break;
			}
			return;
		}
	}
}




/*
===================
G_SpawnGEntityFromSpawnVars

Spawn an entity and fill in all of the level fields from
level.spawnVars[], then call the class specfic spawn function
===================
*/
void G_SpawnGEntityFromSpawnVars( void ) {
	int			i;
	gentity_t	*ent;
	char		*s, *value, *gametypeName;
	static char *gametypeNames[] = {"ffa", "tournament", "single", "team", "ctf", "oneflag", "obelisk", "harvester", "teamtournament"};

	// get the next free entity
	ent = G_Spawn();

	for ( i = 0 ; i < level.numSpawnVars ; i++ ) {
		G_ParseField( level.spawnVars[i][0], level.spawnVars[i][1], ent );
	}

	// check for "notsingle" flag
	if ( g_gametype->integer == GT_SINGLE_PLAYER ) {
		G_SpawnInt( "notsingle", "0", &i );
		if ( i ) {
			G_FreeEntity( ent );
			return;
		}
	}
	// check for "notteam" flag (GT_FFA, GT_TOURNAMENT, GT_SINGLE_PLAYER)
	if ( g_gametype->integer >= GT_TEAM ) {
		G_SpawnInt( "notteam", "0", &i );
		if ( i ) {
			G_FreeEntity( ent );
			return;
		}
	} else {
		G_SpawnInt( "notfree", "0", &i );
		if ( i ) {
			G_FreeEntity( ent );
			return;
		}
	}

#ifdef MISSIONPACK
	G_SpawnInt( "notta", "0", &i );
	if ( i ) {
		G_FreeEntity( ent );
		return;
	}
#else
	G_SpawnInt( "notq3a", "0", &i );
	if ( i ) {
		G_FreeEntity( ent );
		return;
	}
#endif

	if( G_SpawnString( "gametype", NULL, &value ) ) {
		if( g_gametype->integer >= GT_FFA && g_gametype->integer < GT_MAX_GAME_TYPE ) {
			gametypeName = gametypeNames[g_gametype->integer];

			s = strstr( value, gametypeName );
			if( !s ) {
				G_FreeEntity( ent );
				return;
			}
		}
	}

	// move editor origin to pos
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	// if we didn't get a classname, don't bother spawning anything
	if ( !G_CallSpawn( ent ) ) {
		G_FreeEntity( ent );
	}
}



/*
====================
G_AddSpawnVarToken
====================
*/
char *G_AddSpawnVarToken( const char *string ) {
	int		l;
	char	*dest;

	l = strlen( string );
	if ( level.numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS ) {
		G_Error( "G_AddSpawnVarToken: MAX_SPAWN_CHARS" );
	}

	dest = level.spawnVarChars + level.numSpawnVarChars;
	memcpy( dest, string, l+1 );

	level.numSpawnVarChars += l + 1;

	return dest;
}

/*
====================
G_ParseSpawnVars

Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVars[]

This does not actually spawn an entity.
====================
*/
qboolean G_ParseSpawnVars( void ) {
	char		keyname[MAX_TOKEN_CHARS];
	char		com_token[MAX_TOKEN_CHARS];

	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;

#if 0
	// parse the opening brace
	if ( !gi.GetEntityToken( com_token, sizeof( com_token ) ) ) {
		// end of spawn string
		return qfalse;
	}
	if ( com_token[0] != '{' ) {
		G_Error( "G_ParseSpawnVars: found %s when expecting {",com_token );
	}

	// go through all the key / value pairs
	while ( 1 ) {
		// parse key
		if ( !gi.GetEntityToken( keyname, sizeof( keyname ) ) ) {
			G_Error( "G_ParseSpawnVars: EOF without closing brace" );
		}

		if ( keyname[0] == '}' ) {
			break;
		}

		// parse value
		if ( !gi.GetEntityToken( com_token, sizeof( com_token ) ) ) {
			G_Error( "G_ParseSpawnVars: EOF without closing brace" );
		}

		if ( com_token[0] == '}' ) {
			G_Error( "G_ParseSpawnVars: closing brace without data" );
		}
		if ( level.numSpawnVars == MAX_SPAWN_VARS ) {
			G_Error( "G_ParseSpawnVars: MAX_SPAWN_VARS" );
		}
		level.spawnVars[ level.numSpawnVars ][0] = G_AddSpawnVarToken( keyname );
		level.spawnVars[ level.numSpawnVars ][1] = G_AddSpawnVarToken( com_token );
		level.numSpawnVars++;
	}
#endif

	return qtrue;
}



/*QUAKED worldspawn (0 0 0) ?

Every map should have exactly one worldspawn.
"music"		music wav file
"gravity"	800 is default gravity
"message"	Text to print during connection process
*/
void SP_worldspawn( void ) {
	char	*s;
	int i;

	G_SpawnString( "classname", "", &s );
	if ( Q_stricmp( s, "worldspawn" ) ) {
		G_Error( "SP_worldspawn: The first entity isn't 'worldspawn'" );
	}

	// make some data visible to connecting client
	gi.SetConfigstring( CS_GAME_VERSION, GAME_VERSION );

	gi.SetConfigstring( CS_LEVEL_START_TIME, va("%i", level.startTime ) );

	G_SpawnString( "music", "", &s );
	gi.SetConfigstring( CS_MUSIC, s );

	G_SpawnString( "message", "", &s );
	gi.SetConfigstring( CS_MESSAGE, s );				// map specific message

	gi.SetConfigstring( CS_MOTD, g_motd.string );		// message of the day

	G_SpawnString( "gravity", "800", &s );
	gi.Cvar_Set( "g_gravity", s );

	G_SpawnString( "enableDust", "0", &s );
	gi.Cvar_Set( "g_enableDust", s );

	G_SpawnString( "enableBreath", "0", &s );
	gi.Cvar_Set( "g_enableBreath", s );

	for( i = 0; i < level.numSpawnVars; i++ )
	{
		Event *ev = new Event( level.spawnVars[ i ][ 0 ] );
		ev->AddString( level.spawnVars[ i ][ 1 ] );

		world->PostEvent( ev, EV_SPAWNARG );
	}

	g_entities[ENTITYNUM_WORLD].s.number = ENTITYNUM_WORLD;
	g_entities[ENTITYNUM_WORLD].classname = "worldspawn";

	// see if we want a warmup time
	gi.SetConfigstring( CS_WARMUP, "" );
	if ( g_restarted.integer ) {
		gi.Cvar_Set( "g_restarted", "0" );
		level.warmupTime = 0;
	} else if ( g_doWarmup.integer ) { // Turn it on
		level.warmupTime = -1;
		gi.SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );
		G_LogPrintf( "Warmup:\n" );
	}

}


/*
==============
G_SpawnEntitiesFromString

Parses textual entity definitions out of an entstring and spawns gentities.
==============
*/
void G_SpawnEntitiesFromString( void ) {
	// allow calls to G_Spawn*()
	level.spawning = qtrue;
	level.numSpawnVars = 0;

	// the worldspawn is not an actual entity, but it still
	// has a "spawn" function to perform any global setup
	// needed by a level (setting configstrings or cvars, etc)
	if ( !G_ParseSpawnVars() ) {
		G_Error( "SpawnEntities: no entities" );
	}
	SP_worldspawn();

	// parse ents
	while( G_ParseSpawnVars() ) {
		G_SpawnGEntityFromSpawnVars();
	}

	level.spawning = qfalse;			// any future calls to G_Spawn*() will be errors
}

void G_GetNullEntityState(entityState_t *nullState) {

	Com_Memset( nullState, 0, sizeof(entityState_t) );
	nullState->alpha = 1.0f;
	nullState->scale = 1.0f;
	nullState->parent = ENTITYNUM_NONE;
	nullState->tag_num = -1;
	nullState->renderfx = 16;
	nullState->bone_tag[4] = -1;
	nullState->bone_tag[3] = -1;
	nullState->bone_tag[2] = -1;
	nullState->bone_tag[1] = -1;
	nullState->bone_tag[0] = -1;
}
#endif
