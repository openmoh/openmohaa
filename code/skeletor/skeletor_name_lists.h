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

// skeletor_name_lists.h : Skeletor name lists

#ifndef __SKELETOR_NAME_LISTS_FORMAT_H__
#define __SKELETOR_NAME_LISTS_FORMAT_H__

#define MAX_CHANNELS			16384
#define MAX_CHANNEL_NAME		32

typedef struct ChannelName_s {
	char		name[ MAX_CHANNEL_NAME ];
	short		channelNum;
} ChannelName_t;

#ifdef __cplusplus

class ChannelNameTable {
	short int m_iNumChannels;
	ChannelName_t m_Channels[ MAX_CHANNELS ];
	short int m_lookup[ MAX_CHANNELS ];

public:
	ChannelNameTable();

	int				RegisterChannel( const char *name );
	int				FindNameLookup( const char *name );
	void			PrintContents();
	const char		*FindName( int index );
	int				NumChannels() const;

private:
	const char		*FindNameFromLookup( int index );
	bool			FindIndexFromName( const char *name, int *indexPtr );
	void			SortIntoTable( int index );
	void			CopyChannel( ChannelName_t *dest, const ChannelName_t *source );
	void			SetChannelName( ChannelName_t *channel, const char *newName );
};

int GetChannelTypeFromName( const char *name );

#else

typedef struct {
	short int m_iNumChannels;
	ChannelName_t m_Channels[ MAX_CHANNELS ];
	short int m_lookup[ MAX_CHANNELS ];
} ChannelNameTable;

#endif

#endif // __SKELETOR_NAME_LISTS_FORMAT_H__
