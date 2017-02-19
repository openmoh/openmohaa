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

// localization.h : Localization for different languages
// TODO

#ifndef __LOCALIZATION_H__
#define __LOCALIZATION_H__

#include "q_shared.h"
#include "qcommon.h"

#ifdef __cplusplus
#include <container.h>

typedef struct {
	str			m_refName;
	str			m_locName;
	intptr_t	m_r1_rep;
	intptr_t	m_r2_rep;
	intptr_t	m_r3_rep;
	intptr_t	m_l1_rep;
	intptr_t	m_l2_rep;
	intptr_t	m_l3_rep;
} loc_entry_t;

class cLocalization {
	Container< loc_entry_t > m_entries;
	Container< str > missing;

public:
	cLocalization();

	const char		*ConvertString( const char *var );

private:
	void			LoadFile( const char *name );
	void			GenerateMissing( const char *file_name );
};
#endif

#ifdef __cplusplus
extern "C" {
#endif

void Sys_InitLocalization();
void Sys_ShutLocalization();
const char *Sys_LV_ConvertString( const char *var );
const char *Sys_LV_CL_ConvertString( const char *var );

#ifdef __cplusplus
}
#endif


#endif // __LOCALIZATION_H__
