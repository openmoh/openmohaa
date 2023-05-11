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

#ifndef __CL_UILOADSAVE_H__
#define __CL_UILOADSAVE_H__

class UIFAKKLoadGameClass : public UIListCtrl { 
	bool m_bRemovePending; 

public:
	CLASS_PROTOTYPE( UIFAKKLoadGameClass );

protected:
	void		SelectGame( Event *ev );
	void		LoadGame( Event *ev );
	void		SaveGame( Event *ev );
	void		RemoveGame( Event *ev );
	void		DeleteGame( Event *ev );
	void		NoDeleteGame( Event *ev );
	qboolean	KeyEvent( int key, unsigned int time ) override;
	void		UpdateUIElement( void ) override;
public:
	UIFAKKLoadGameClass();

	void	SetupFiles( void );
};

#ifdef __cplusplus
extern "C" {
#endif

void UI_SetupFiles( void );

#ifdef __cplusplus
}
#endif

#endif 
