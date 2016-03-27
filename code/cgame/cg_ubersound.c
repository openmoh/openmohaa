/*
===========================================================================
Copyright (C) 2010-2011 Michael Rieder

This file is part of OpenMohaa source code.

OpenMohaa source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMohaa source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMohaa source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include "cg_local.h"

#define	MAX_UBERSOUNDS	8192
ubersound_t		snd_indexes[MAX_UBERSOUNDS];
int				snd_numIndexes;
char			*buffer;

#define FILE_HASH_SIZE		256
static	ubersound_t*		hashTable[FILE_HASH_SIZE];

static long generateHashValue( const char *fname ) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (fname[i] != '\0') {
		letter = tolower(fname[i]);
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash &= (FILE_HASH_SIZE-1);
	return hash;
}

static ubersound_t*	US_Find( const char *snd_name ) {
	ubersound_t*	snd;
	long			hash;

	hash = generateHashValue(snd_name);
	
	for (snd=hashTable[hash] ; snd ; snd=snd->hashNext) {
		if (!Q_stricmp(snd_name, snd->name)) {
			return snd;
		}
	}

	return NULL;
}

ubersound_t*	CG_GetUbersound( const char *name ) {
	ubersound_t *snd;

	snd = US_Find( name );

	if ( !snd ) {
		CG_Printf( "CG_GetUbersound: sound \"%s\" not found.\n", name );
		return NULL;
	}
	// if there are more than one version, choose randomly
	if ( snd->variations ) {
		int i, var;

		var = rand();
		var *= snd->variations;
		var /= RAND_MAX+1;

		// CG_Printf( "random sound %i of %i\n", var, snd->variations );
		for (i=0;i<var;i++)
			snd = snd->nextVariation;
	}

	if ( snd->hasLoaded == qfalse )
		snd->sfxHandle = cgi.S_RegisterSound( snd->wavfile, qfalse );

	return snd;
}

void CG_ParseUSline( char **ptr, ubersound_t *snd ) {
	const char *chan;
	const char *token;
	// name
	Q_strncpyz( snd->name, COM_Parse(ptr), sizeof(snd->name) );
	Q_strncpyz( snd->wavfile, COM_Parse(ptr), sizeof(snd->wavfile) );
	// soundparms
	if ( !Q_strncmp( COM_Parse(ptr), "soundparms", MAX_QPATH ) ) {
		snd->basevol	= atof(COM_Parse(ptr));
		snd->volmod		= atof(COM_Parse(ptr));
		snd->basepitch	= atof(COM_Parse(ptr));
		snd->pitchmod	= atof(COM_Parse(ptr));
		snd->minDist	= atof(COM_Parse(ptr));
		snd->maxDist	= atof(COM_Parse(ptr));

		// channel
		chan = COM_Parse(ptr);
		if ( !Q_strncmp( chan, "auto", MAX_QPATH ) )
			snd->channel = CHAN_AUTO;
		else if ( !Q_strncmp( chan, "body", MAX_QPATH ) )
			snd->channel = CHAN_BODY;
		else if ( !Q_strncmp( chan, "item", MAX_QPATH ) )
			snd->channel = CHAN_ITEM;
		else if ( !Q_strncmp( chan, "weaponidle", MAX_QPATH ) )
			snd->channel = CHAN_WEAPONIDLE;
		else if ( !Q_strncmp( chan, "voice", MAX_QPATH ) )
			snd->channel = CHAN_VOICE;
		else if ( !Q_strncmp( chan, "local", MAX_QPATH ) )
			snd->channel = CHAN_LOCAL;
		else if ( !Q_strncmp( chan, "weapon", MAX_QPATH ) )
			snd->channel = CHAN_WEAPON;
		else if ( !Q_strncmp( chan, "dialog_secondary", MAX_QPATH ) )
			snd->channel = CHAN_DIALOG_SECONDARY;
		else if ( !Q_strncmp( chan, "dialog", MAX_QPATH ) )
			snd->channel = CHAN_DIALOG;
		else if ( !Q_strncmp( chan, "menu", MAX_QPATH ) )
			snd->channel = CHAN_MENU;
		else
			CG_Printf( "Ubersound unrecognized channel %s for %s\n", chan, snd->name );
		// loaded /Streamed
		chan = COM_Parse(ptr);
		if ( !Q_strncmp( chan, "loaded", MAX_QPATH ) )
			snd->loaded = qtrue;
		else if ( !Q_strncmp( chan, "streamed", MAX_QPATH ) )
			snd->loaded = qfalse;
		else
			CG_Printf( "Ubersound unrecognized loaded state %s for %s\n", chan, snd->name );
	}
	token = COM_Parse(ptr);
	// subtitle - su44: we need this for ubersound/uberdialog.scr loading
	if ( !Q_stricmp( token, "subtitle" ) ) {
		Q_strncpyz( snd->subtitle, COM_Parse(ptr), sizeof(snd->subtitle) );
		token = COM_Parse(ptr);
	}
	// maplist
	if ( !Q_stricmp( token, "maps" ) ) 
		Q_strncpyz( snd->mapstring, COM_Parse(ptr), MAPSTRING_LENGTH );
}

qboolean US_CheckMapstring( const char *mapstring ) {
	char		*ptr;
	const char	*token;

	ptr = (char*)mapstring;
	token = COM_Parse(&ptr);
	while (*token) {
		if ( strstr( cgs.mapname, token ) )
			return qtrue;
		token = COM_Parse(&ptr);
	}

	return qfalse;
}

void CG_LoadUberSoundFile( const char *fname, qboolean sndVar ) {
	// sndVar decides whether the last digit (if it is a digit) provides a variation of the same sound
	int				len;

	const char		*token;
	char			*ptr;
	qboolean		end;
	ubersound_t		*oldSnd, *newSnd;
	int				i;

	CG_Printf( "=== Loading %s ===\n", fname );

	len = cgi.FS_ReadFile( fname, &buffer, qfalse );

	if( len <= 0 ) {
		CG_Printf( "couldn't load %s. file not found.\n", fname );
		return;
	}

	end = qfalse;
	ptr = buffer;
	token = COM_Parse( &ptr );

	while (*token) {
		newSnd = &snd_indexes[snd_numIndexes];
		if ( !Q_strncmp(token,"aliascache",MAX_QPATH) || !Q_strncmp(token,"alias",MAX_QPATH) ) {
			CG_ParseUSline( &ptr, newSnd );

			if ( US_CheckMapstring( newSnd->mapstring ) == qtrue ) {
				int variation;

				variation = atoi(newSnd->name + strlen(newSnd->name)-1);
				if ( sndVar == qtrue && variation ) {
					newSnd->name[strlen(newSnd->name)-1] = 0;
					oldSnd = US_Find(newSnd->name);
					if ( oldSnd ) {
						oldSnd->variations++;
						for (i=oldSnd->variations;i>2;i--)
							oldSnd = oldSnd->nextVariation;
						oldSnd->nextVariation = newSnd;
						if ( newSnd->loaded == qtrue ) {
							 newSnd->sfxHandle = cgi.S_RegisterSound( newSnd->wavfile, qfalse );
							 newSnd->hasLoaded = qtrue;
						}
						snd_numIndexes++;
						if ( snd_numIndexes >= MAX_UBERSOUNDS ) {
							snd_numIndexes--;
							CG_Error( "CG_LoadUbersound: too many aliascaches in file.\n" );
						}
						continue;
					}
					else newSnd->variations++;
				}

				i = generateHashValue(newSnd->name);
				oldSnd = hashTable[i];
				if (oldSnd) {
					while (oldSnd->hashNext)
						oldSnd = oldSnd->hashNext;
					oldSnd->hashNext = newSnd;
				}
				else
					hashTable[i] = newSnd;
				if ( newSnd->loaded == qtrue ) {
					 newSnd->sfxHandle = cgi.S_RegisterSound( newSnd->wavfile, qfalse );
					 newSnd->hasLoaded = qtrue;
				}
				snd_numIndexes++;
				if ( snd_numIndexes >= MAX_UBERSOUNDS ) {
					snd_numIndexes--;
					CG_Error( "CG_LoadUbersound: too many aliascaches in file.\n" );
				}
			} else {
				Com_Memset(newSnd,0,sizeof(ubersound_t) );
			}
		} //su44: "end" token is missing in MoHAA's uberdialog.scr
		//else if ( !Q_strncmp( token, "end", MAX_QPATH ) ) {
		//	end = qtrue;
		//	break;
		//}
		token = COM_Parse( &ptr );
	}
	//if ( end == qfalse ) 
	//	Com_Printf( "CG_LoadUbersound hit end of file without end statement\n" );

	cgi.FS_FreeFile( buffer );
	CG_Printf( "=== Finished %s ===\n", fname );
}

void CG_LoadUbersound( void ) {
	Com_Memset(snd_indexes, 0, sizeof(snd_indexes));
	Com_Memset(hashTable, 0, sizeof(hashTable));
	snd_numIndexes = 0;

	CG_LoadUberSoundFile(UBERSOUND_FILE, qtrue);
	// su44: used by MoHAA voicechats
	CG_LoadUberSoundFile(UBERDIALOG_FILE, qfalse);
}

const char* CG_LoadMusic( const char *musicfile ) {

	int				len;

	const char		*token;
	char			*ptr;
	char			path[MAX_QPATH];
	char			name[MAX_QPATH];

	if( !musicfile[ 0 ] )
	{
		return "";
	}

	CG_Printf( "=== Loading %s ===\n", musicfile );

	len = cgi.FS_ReadFile( musicfile, &buffer, qfalse );

	if( len <= 0 ) {
		CG_Printf( "couldn't load %s. file not found.\n", musicfile );
		return NULL;
	}

	ptr = buffer;
	token = COM_Parse( &ptr );
	while (*token) {
		if ( !Q_strncmp(token,"path",MAX_QPATH) )
			Q_strncpyz( path, COM_Parse(&ptr), MAX_QPATH );
		else if ( !Q_strncmp(token,"normal",MAX_QPATH) )
			Q_strncpyz( name, COM_Parse(&ptr), MAX_QPATH );
		token = COM_Parse( &ptr );
	}
	Q_snprintf( buffer, MAX_QPATH, "%s/%s", path, name );
	return buffer;
}