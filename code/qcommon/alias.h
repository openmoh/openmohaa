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

#ifndef __ALIAS_H__
#define __ALIAS_H__

#ifdef __cplusplus
extern "C" {
#endif

//
// public implementation
//

const char *   Alias_Find( const char * alias );
qboolean Alias_Add( const char * alias, const char * name, const char * parameters );
qboolean Alias_Delete( const char * alias );
const char *   Alias_FindRandom( const char * alias, AliasListNode_t **ret );
void     Alias_Dump( void );
void     Alias_Clear( void );
AliasList_t *Alias_GetGlobalList();

int S_ChannelNameToNum( const char *pszName );
const char *S_ChannelNumToName( int iChannel );

//
// private implementation
//
#define MAX_ALIAS_NAME_LENGTH 40
#define MAX_REAL_NAME_LENGTH 128
#define MAX_ALIASLIST_NAME_LENGTH 40

typedef struct AliasActorNode_s
	{
   int actor_number;

	int  number_of_times_played;
	byte been_played_this_loop;
	int  last_time_played;

   struct AliasActorNode_s * next;
   } AliasActorNode_t;

typedef struct AliasListNode_s {
	char alias_name[ MAX_ALIAS_NAME_LENGTH ];
	char real_name[ MAX_REAL_NAME_LENGTH ];
	float weight;

	// Static alias info
	byte stop_flag;
	struct AliasListNode_s *next;

	// Global alias info
	float pitch;
	float volume;
	float pitchMod;
	float volumeMod;
	float dist;
	float maxDist;
	int channel;
	int streamed;
	char *subtitle;
} AliasListNode_t;

typedef struct AliasList_s
   {
   char name[ MAX_ALIASLIST_NAME_LENGTH ];
   qboolean    dirty;
   int         num_in_list;
   AliasListNode_t ** sorted_list;
   AliasListNode_t * data_list;
   } AliasList_t;

void Alias_ListClearActors( AliasList_t * list );
AliasList_t * AliasList_New( const char * name );
const char *   Alias_ListFind( AliasList_t * list, const char * alias );
AliasListNode_t *Alias_ListFindNode( AliasList_t * list, const char * alias );
qboolean Alias_ListAdd( AliasList_t * list, const char * alias, const char * name, const char * parameters );
const char *   Alias_ListFindRandom( AliasList_t * list, const char * alias, AliasListNode_t **ret );
void     Alias_ListDump( AliasList_t * list );
void     Alias_ListClear( AliasList_t * list  );
void     Alias_ListDelete( AliasList_t * list );
void     Alias_ListSort( AliasList_t * list );
int Alias_IsGlobal( AliasListNode_t *node, int actor_number );
AliasActorNode_t *Alias_FindActor( AliasListNode_t *node, int actor_number );
void Alias_ListFindRandomRange( AliasList_t * list, const char * alias, int *min_index, int *max_index, float *total_weight );
const char * Alias_ListFindDialog( AliasList_t * list, const char * alias );
void Alias_ListUpdateDialog( AliasList_t * list, const char * alias );
void Alias_ListAddActorDialog( AliasList_t * list, const char * alias );
float randweight( void );

#ifdef __cplusplus
	}
#endif

#endif /* alias.h */
