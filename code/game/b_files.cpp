//
// b_files.c
//


#include "b_local.h"

#if 0
//
// parse support routines
//

static qboolean Nav_ParseLiteral( char **data, const char *string ) {
	char	*token;

	token = COM_ParseExt( data, qtrue );
	if ( token[0] == 0 ) {
		gi.Printf( ERROR "unexpected EOF\n" );
		return qtrue;
	}

	if ( Q_stricmp( token, string ) ) {
		gi.Printf( ERROR "required string '%s' missing\n", string );
		return qtrue;
	}

	return qfalse;
}


static qboolean Nav_ParseString( char **data, char **s ) {
	*s = COM_ParseExt( data, qfalse );
	if ( s[0] == 0 ) {
		gi.Printf( ERROR "unexpected EOF\n" );
		return qtrue;
	}
	return qfalse;
}


static qboolean Nav_ParseInt( char **data, int *i ) {
	char	*token;

	token = COM_ParseExt( data, qfalse );
	if ( token[0] == 0 ) {
		gi.Printf( ERROR "unexpected EOF\n" );
		return qtrue;
	}

	*i = atoi( token );
	return qfalse;
}


//
// bot parameters file : scripts/bots.cfg
//

char	botParms[0x10000];


static int MethodNameToNumber( const char *name ) {
	if ( !Q_stricmp( name, "EXPONENTIAL" ) ) {
		return METHOD_EXPONENTIAL;
	}
	if ( !Q_stricmp( name, "LINEAR" ) ) {
		return METHOD_LINEAR;
	}
	if ( !Q_stricmp( name, "LOGRITHMIC" ) ) {
		return METHOD_LOGRITHMIC;
	}
	if ( !Q_stricmp( name, "ALWAYS" ) ) {
		return METHOD_ALWAYS;
	}
	if ( !Q_stricmp( name, "NEVER" ) ) {
		return METHOD_NEVER;
	}

	return -1;
}


static int ItemNameToNumber( const char *name, int itemType ) {
	int		n;

	for ( n = 0; n < bg_numItems; n++ ) {
		if ( bg_itemlist[n].type != itemType ) {
			continue;
		}
		if ( Q_stricmp( bg_itemlist[n].classname, name ) == 0 ) {
			return bg_itemlist[n].tag;
		}
	}

	return -1;
}


void Bot_ParseParms( const char *botName, gentity_t *bot, char *userinfo ) {
	char	*token;
	char	*value;
	char	*p;
	int		n;
	int		count;

	if ( !botName || !botName[0]) {
		botName = "Player";
	}

	strcpy( userinfo, "\\name\\" );
	strcat( userinfo, botName );

	// fill in defaults
	bot->bot->reactions			= 3;
	bot->bot->aim				= 3;
	bot->bot->move				= 3;
	bot->bot->aggression		= 3;
	bot->bot->intelligence		= 3;
	bot->bot->hfov				= 90 / 2;
	bot->bot->vfov				= 68 / 2;
	bot->bot->healthMethod		= METHOD_LOGRITHMIC;
	bot->bot->armorMethod		= METHOD_LINEAR;
	bot->bot->ammoMethod		= METHOD_EXPONENTIAL;
	bot->bot->allWeaponOrder[0]	= WP_BFG;
	bot->bot->allWeaponOrder[1]	= WP_ROCKET_LAUNCHER;
	bot->bot->allWeaponOrder[2]	= WP_RAILGUN;
	bot->bot->allWeaponOrder[3]	= WP_PLASMAGUN;
	bot->bot->allWeaponOrder[4]	= WP_GRENADE_LAUNCHER;
	bot->bot->allWeaponOrder[5]	= WP_SHOTGUN;
	bot->bot->allWeaponOrder[6]	= WP_MACHINEGUN;
	bot->bot->allWeaponOrder[7]	= WP_NONE;

	p = botParms;
	COM_BeginParseSession();

	// look for the right bot
	while ( p ) {
		token = COM_ParseExt( &p, qtrue );
		if ( token[0] == 0 )
			return;

		if ( !Q_stricmp( token, botName ) ) {
			break;
		}

		SkipBracedSection( &p );
	}
	if ( !p ) {
		return;
	}

	if ( Nav_ParseLiteral( &p, "{" ) ) {
		return;
	}
		
	// parse the bot info block
	while ( 1 ) {
		token = COM_ParseExt( &p, qtrue );
		if ( !token[0] ) {
			gi.Printf( "ERROR: unexpected EOF while parsing '%s'\n", botName );
			return;
		}

		if ( !Q_stricmp( token, "}" ) ) {
			break;
		}

		// model
		if ( !Q_stricmp( token, "model" ) ) {
			if ( Nav_ParseString( &p, &value ) ) {
				continue;
			}
			strcat ( userinfo, "\\model\\" );
			strcat ( userinfo, value );
			continue;
		}

		// reactions
		if ( !Q_stricmp( token, "reactions" ) ) {
			if ( Nav_ParseInt( &p, &n ) ) {
				SkipRestOfLine( &p );
				continue;
			}
			if ( n < 1 || n > 5 ) {
				gi.Printf( WARNING "bad %s in bot '%s'\n", token, botName );
				continue;
			}
			bot->bot->reactions = n;
			continue;
		}

		// aim
		if ( !Q_stricmp( token, "aim" ) ) {
			if ( Nav_ParseInt( &p, &n ) ) {
				SkipRestOfLine( &p );
				continue;
			}
			if ( n < 1 || n > 5 ) {
				gi.Printf( WARNING "bad %s in bot '%s'\n", token, botName );
				continue;
			}
			bot->bot->aim = n;
			continue;
		}

		// move
		if ( !Q_stricmp( token, "move" ) ) {
			if ( Nav_ParseInt( &p, &n ) ) {
				SkipRestOfLine( &p );
				continue;
			}
			if ( n < 1 || n > 5 ) {
				gi.Printf( WARNING "bad %s in bot '%s'\n", token, botName );
				continue;
			}
			bot->bot->move = n;
			continue;
		}

		// aggression
		if ( !Q_stricmp( token, "aggression" ) ) {
			if ( Nav_ParseInt( &p, &n ) ) {
				SkipRestOfLine( &p );
				continue;
			}
			if ( n < 1 || n > 5 ) {
				gi.Printf( WARNING "bad %s in bot '%s'\n", token, botName );
				continue;
			}
			bot->bot->aggression = n;
			continue;
		}

		// intelligence
		if ( !Q_stricmp( token, "intelligence" ) ) {
			if ( Nav_ParseInt( &p, &n ) ) {
				SkipRestOfLine( &p );
				continue;
			}
			if ( n < 1 || n > 5 ) {
				gi.Printf( WARNING "bad %s in bot '%s'\n", token, botName );
				continue;
			}
			bot->bot->intelligence = n;
			continue;
		}

		// hfov
		if ( !Q_stricmp( token, "hfov" ) ) {
			if ( Nav_ParseInt( &p, &n ) ) {
				SkipRestOfLine( &p );
				continue;
			}
			if ( n < 30 || n > 180 ) {
				gi.Printf( WARNING "bad %s in bot '%s'\n", token, botName );
				continue;
			}
			bot->bot->hfov = n / 2;
			continue;
		}

		// vfov
		if ( !Q_stricmp( token, "vfov" ) ) {
			if ( Nav_ParseInt( &p, &n ) ) {
				SkipRestOfLine( &p );
				continue;
			}
			if ( n < 30 || n > 180 ) {
				gi.Printf( WARNING "bad %s in bot '%s'\n", token, botName );
				continue;
			}
			bot->bot->vfov = n / 2;
			continue;
		}

		// healthMethod
		if ( !Q_stricmp( token, "healthMethod" ) ) {
			if ( Nav_ParseString( &p, &value ) ) {
				continue;
			}
			n = MethodNameToNumber( value );
			if ( n == -1 ) {
				gi.Printf( "WARNING: bad %s in bot '%s'\n", token, botName );
				continue;
			}
			bot->bot->healthMethod = n;
			continue;
		}

		// armorMethod
		if ( !Q_stricmp( token, "armorMethod" ) ) {
			if ( Nav_ParseString( &p, &value ) ) {
				continue;
			}
			n = MethodNameToNumber( value );
			if ( n == -1 ) {
				gi.Printf( "WARNING: bad %s in bot '%s'\n", token, botName );
				continue;
			}
			bot->bot->armorMethod = n;
			continue;
		}

		// ammoMethod
		if ( !Q_stricmp( token, "ammoMethod" ) ) {
			if ( Nav_ParseString( &p, &value ) ) {
				continue;
			}
			n = MethodNameToNumber( value );
			if ( n == -1 ) {
				gi.Printf( "WARNING: bad %s in bot '%s'\n", token, botName );
				continue;
			}
			bot->bot->ammoMethod = n;
			continue;
		}

		// weapons
		if ( !Q_stricmp( token, "weapons" ) ) {
			for ( count = 0; count < MAX_WEAPONS; count++ ) {
				if ( Nav_ParseString( &p, &value ) ) {
					break;
				}
				if ( *value == 0 ) {
					break;
				}
				n = ItemNameToNumber( value, IT_WEAPON );
				if ( n == -1 ) {
					gi.Printf( "WARNING: bad %s in bot '%s'\n", token, botName );
					continue;
				}
				bot->bot->allWeaponOrder[count] = n;
			}
			if ( count < MAX_WEAPONS ) {
				bot->bot->allWeaponOrder[count] = WP_NONE;
			}
			continue;
		}

		// snd
		if ( !Q_stricmp( token, "snd" ) ) {
			if ( Nav_ParseString( &p, &value ) ) {
				continue;
			}
			strcat( userinfo, "\\snd\\" );
			strcat( userinfo, value );
			continue;
		}

		gi.Printf( "WARNING: unknown keyword '%s' while parsing '%s'\n", token, botName );
		SkipRestOfLine( &p );
	}
}


void Bot_LoadParms( void ) {
	int			len;
	char		filename[MAX_QPATH];
	char		*buffer;

	sprintf( filename, "scripts/bots.cfg" );
	gi.Printf( "Parsing %s\n", filename );
	len = gi.FS_ReadFile( filename, &buffer, qtrue );
	if ( len == -1 ) {
		gi.Printf( "file not found\n" );
	}

	if ( len >= sizeof( botParms ) ) {
		gi.Error( ERR_DROP, "scripts/bots.cfg is too large" );
	}
	strncpy( botParms, buffer, sizeof( botParms ) - 1 );
	gi.FS_FreeFile( buffer );
}
#endif


//
// navigation data : maps/*.nav
//

void Nav_LoadRoutes( void ) {
	int				len;
	navheader_t		*header;
	int				value;
	int				n;
	str            filename;

	Swap_Init();

	surfaceCount = 0;
	surface = NULL;
	neighborCount = 0;
	neighbor = NULL;

	// load the file
	filename = "maps/";
   filename += level.mapname + ".nav";

	gi.Printf( "Loading %s\n", filename.c_str() );
	len = gi.FS_ReadFile( filename.c_str(), ( void ** )&navFileData, qtrue );
	if ( len == -1 ) {
		gi.Printf( WARNING "no navigation data\n" );
		return;
	}
	if ( len < sizeof( navheader_t ) ) {
		gi.Printf( ERROR "no nav file header\n" );
		goto cleanup;
	}

	// process the header
	header = (navheader_t *)navFileData;
	header->id = LittleLong( header->id );
	header->version = LittleLong( header->version );
	header->surfaceCount = LittleLong( header->surfaceCount );
	header->neighborCount = LittleLong( header->neighborCount );

	// validate the header fields
	if ( header->id != NAVFILE_ID ) {
		gi.Printf( ERROR "incorrect nav file id\n" );
		goto cleanup;
	}
	if ( header->version != NAVFILE_VERSION ) {
		gi.Printf( ERROR "incorrect nav file version (%i, should be %i)\n", header->version, NAVFILE_VERSION );
		goto cleanup;
	}

	value = /* header */ sizeof( navheader_t ) +
			/* surfaces */ header->surfaceCount * sizeof( nsurface_t ) +
			/* neighbors */ header->neighborCount * sizeof( nneighbor_t ) +
			/* routes */ header->surfaceCount * header->surfaceCount * sizeof( byte );

	if ( value != len ) {
		gi.Printf( ERROR "incorrect nav file length (%i, should be %i)\n", len, value );
		goto cleanup;
	}

	surfaceCount = header->surfaceCount;
	neighborCount = header->neighborCount;

	// process surfaces
	surface = (nsurface_t *)(navFileData + sizeof( navheader_t ) );
	for ( n = 0; n < surfaceCount; n++ ) {
		surface[n].origin[0] = LittleFloat( surface[n].origin[0] );
		surface[n].origin[1] = LittleFloat( surface[n].origin[1] );
		surface[n].origin[2] = LittleFloat( surface[n].origin[2] );

		surface[n].absmin[0] = LittleFloat( surface[n].absmin[0] );
		surface[n].absmin[1] = LittleFloat( surface[n].absmin[1] );

		surface[n].absmax[0] = LittleFloat( surface[n].absmax[0] );
		surface[n].absmax[1] = LittleFloat( surface[n].absmax[1] );

		surface[n].flags = LittleLong( surface[n].flags );
		surface[n].neighborCount = LittleLong( surface[n].neighborCount );
		surface[n].neighborIndex = LittleLong( surface[n].neighborIndex );
		surface[n].parm = LittleLong( surface[n].parm );
	}

	// process neighbors
	neighbor = (nneighbor_t *)((byte *)surface + surfaceCount * sizeof( nsurface_t ));
	for ( n = 0; n < neighborCount; n++ ) {
		neighbor[n].origin[0] = LittleFloat( neighbor[n].origin[0] );
		neighbor[n].origin[1] = LittleFloat( neighbor[n].origin[1] );
		neighbor[n].origin[2] = LittleFloat( neighbor[n].origin[2] );

		neighbor[n].absmin[0] = LittleFloat( neighbor[n].absmin[0] );
		neighbor[n].absmin[1] = LittleFloat( neighbor[n].absmin[1] );

		neighbor[n].absmax[0] = LittleFloat( neighbor[n].absmax[0] );
		neighbor[n].absmax[1] = LittleFloat( neighbor[n].absmax[1] );

		neighbor[n].surfaceNum = LittleLong( neighbor[n].surfaceNum );
		neighbor[n].flags = LittleLong( neighbor[n].flags );
		neighbor[n].cost = LittleLong( neighbor[n].cost );
		neighbor[n].filler = LittleLong( neighbor[n].filler );
	}

	// process routes
	route = (byte *)neighbor + neighborCount * sizeof( nneighbor_t );

	gi.Printf( "...loaded %i surfaces and %i neighbors\n", surfaceCount, neighborCount );
	return;

cleanup:
	gi.FS_FreeFile ( navFileData );
	navFileData = NULL;
}
