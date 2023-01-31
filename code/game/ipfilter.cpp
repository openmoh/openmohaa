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

// ipfilter.cpp: PACKET FILTERING
// 
// You can add or remove addresses from the filter list with:
// 
// addip <ip>
// removeip <ip>
// 
// The ip address is specified in dot format, and any unspecified digits will match 
// any value, so you can specify an entire class C network with "addip 192.246.40".
// 
// Removeip will only remove an address specified exactly the same way.  You cannot 
// addip a subnet, then removeip a single host.
// 
// listip
// Prints the current list of filters.
// 
// writeip
// Dumps "addip <ip>" commands to listip.cfg so it can be execed at a later date.
// The filter lists are not saved and restored by default, because I beleive it would
// cause too much confusion.
// 
// filterban <0 or 1>
// 
// If 1 (the default), then ip addresses matching the current list will be prohibited
// from entering the game.  This is the default setting.
// 
// If 0, then only addresses matching the list will be allowed.  This lets you easily
// set up a private game, or a game that only allows players from your local network.
//

#include "ipfilter.h"
#include "g_local.h"

typedef struct
   {
	unsigned	mask;
	unsigned	compare;
   } ipfilter_t;

#define	MAX_IPFILTERS	1024

ipfilter_t	ipfilters[ MAX_IPFILTERS ];
int			numipfilters;

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter
   (
   const char *s,
   ipfilter_t *f
   )

   {
	char	num[ 128 ];
	int	i;
   int   j;
	byte	b[ 4 ];
	byte	m[ 4 ];

	for( i = 0; i < 4; i++ )
	   {
		b[ i ] = 0;
		m[ i ] = 0;
	   }

	for( i = 0; i < 4; i++ )
	   {
		if ( *s < '0' || *s > '9' )
		   {
         gi.SendServerCommand( 0, "print \"Bad filter address: %s\n\"", s );
			return false;
		   }

		j = 0;
		while( *s >= '0' && *s <= '9' )
		   {
			num[ j++ ] = *s++;
		   }

		num[ j ] = 0;
		b[ i ] = atoi( num );
		if ( b[ i ] != 0 )
         {
			m[ i ] = 255;
         }

		if ( !*s )
         {
			break;
         }

		s++;
	   }

	f->mask = *( unsigned * )m;
	f->compare = *( unsigned * )b;

	return true;
   }

/*
=================
SV_FilterPacket
=================
*/
qboolean SV_FilterPacket
   (
   const char *from
   )

   {
	int i;
	unsigned	in;
	byte m[ 4 ];
	const char *p;

	i = 0;
	p = from;
	while( *p && i < 4 )
      {
		m[ i ] = 0;
		while( *p >= '0' && *p <= '9' )
         {
			m[ i ] = m[ i ] * 10 + ( *p - '0' );
			p++;
		   }

		if ( !*p || *p == ':' )
         {
			break;
         }

		i++;
      p++;
	   }

	in = *( unsigned * )m;
	for( i = 0; i < numipfilters; i++ )
      {
		if ( ( in & ipfilters[ i ].mask ) == ipfilters[ i ].compare )
         {
         return ( int )filterban->integer;
         }
      }

   return !( int )filterban->integer;
   }


/*
=================
SV_AddIP_f
=================
*/
void SVCmd_AddIP_f
   (
   void
   )

   {
	int i;

	if ( gi.Argc() < 3 )
      {
      gi.SendServerCommand( 0, "print \"Usage: addip <ip-mask>\n\"" );
		return;
      }

	for( i = 0; i < numipfilters; i++ )
      {
		if ( ipfilters[ i ].compare == 0xffffffff )
         {
         // free spot
			break;
         }
      }

	if ( i == numipfilters )
	   {
		if ( numipfilters == MAX_IPFILTERS )
         {
         gi.SendServerCommand( 0, "print \"IP filter list is full\n\"" );
			return;
		   }
		numipfilters++;
	   }

	if ( !StringToFilter( gi.Argv( 2 ), &ipfilters[ i ] ) )
      {
		ipfilters[ i ].compare = 0xffffffff;
      }
   }

/*
=================
SV_RemoveIP_f
=================
*/
void SVCmd_RemoveIP_f
   (
   void
   )

   {
	ipfilter_t	f;
	int			i;
   int         j;

	if ( gi.Argc() < 3 )
      {
      gi.SendServerCommand( 0, "print \"Usage: sv removeip <ip-mask>\n\"" );
		return;
	   }

	if ( !StringToFilter( gi.Argv( 2 ), &f ) )
      {
		return;
      }

	for( i = 0; i < numipfilters; i++ )
      {
		if ( ( ipfilters[ i ].mask == f.mask ) && ( ipfilters[ i ].compare == f.compare ) )
		   {
			for ( j = i + 1; j < numipfilters; j++ )
            {
				ipfilters[ j - 1 ] = ipfilters[ j ];
            }

			numipfilters--;
         gi.SendServerCommand( 0, "print \"Removed.\n\"" );
			return;
		   }
      }

   gi.SendServerCommand( 0, "print \"Didn't find %s.\n\"", gi.Argv( 2 ) );
   }

/*
=================
SV_ListIP_f
=================
*/
void SVCmd_ListIP_f
   (
   void
   )

   {
	int   i;
	byte	b[ 4 ];

   gi.SendServerCommand( 0, "print \"Filter list:\n\"", gi.Argv( 2 ) );
	for( i = 0; i < numipfilters; i++ )
   	{
		*( unsigned * )b = ipfilters[ i ].compare;
      gi.SendServerCommand( 0, "print \"%3i.%3i.%3i.%3i\n\"", b[ 0 ], b[ 1 ], b[ 2 ], b[ 3 ] );
	   }
   }

/*
=================
SV_WriteIP_f
=================
*/
void SVCmd_WriteIP_f
   (
   void
   )

   {
	FILE	 *f;
	char	 name[ MAX_OSPATH ];
	byte	 b[ 4 ];
	int	 i;

	sprintf( name, "%s/listip.cfg", GAMEVERSION );
   gi.SendServerCommand( 0, "print \"Writing %s.\n\"", name );

	f = fopen( name, "wb" );
	if ( !f )
	   {
      gi.SendServerCommand( 0, "print \"Couldn't open %s.\n\"", name );
      return;
	   }

   fprintf( f, "set filterban %d\n", ( int )filterban->integer );

	for( i = 0; i < numipfilters; i++ )
	   {
		*( unsigned * )b = ipfilters[ i ].compare;
		fprintf( f, "sv addip %i.%i.%i.%i\n", b[ 0 ], b[ 1 ], b[ 2 ], b[ 3 ] );
	   }

	fclose( f );
   }

/*
=================
G_ServerCommand

G_ServerCommand will be called when an "sv" command is issued.
The game can issue gi.Argc() / gi.Argv() commands to get the rest
of the parameters
=================
*/
void G_ServerCommand
   (
   void
   )

   {
	const char *cmd;

	cmd = gi.Argv(1);
	if ( Q_stricmp( cmd, "addip" ) == 0 )
      {
		SVCmd_AddIP_f();
      }
	else if ( Q_stricmp( cmd, "removeip" ) == 0 )
      {
		SVCmd_RemoveIP_f();
      }
	else if ( Q_stricmp( cmd, "listip" ) == 0 )
      {
		SVCmd_ListIP_f();
      }
	else if ( Q_stricmp( cmd, "writeip" ) == 0 )
      {
		SVCmd_WriteIP_f();
      }
	else
      {
      gi.SendServerCommand( 0, "print \"Unknown server command %s.\n\"", cmd );
      }
   }
