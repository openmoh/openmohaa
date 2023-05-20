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

#ifndef __CL_UIVIEW3D__
#define __CL_UIVIEW3D__

class View3D : public UIWidget {
	float m_printfadetime;
	float m_printalpha;
	str m_printstring;
	UIReggedMaterial *m_print_mat;
	qboolean m_locationprint;
	int m_x_coord;
	int m_y_coord;
	qboolean m_letterbox_active;

public:
	CLASS_PROTOTYPE( View3D );

protected:
	void		Draw( void ) override;
	void		DrawLetterbox( void );
	void		DrawFades( void );
	void		Draw2D( void );
	void		DrawFPS( void );
	void		DrawProf( void );
	void		PrintSound( int channel, const char *name, float vol, int rvol, float pitch, float base, int& line );
	void		DrawSoundOverlay( void );
	void		CenterPrint( void );
	void		LocationPrint( void );
	void		OnActivate( Event *ev );
	void		OnDeactivate( Event *ev );
	void		DrawSubtitleOverlay( void );

public:
	View3D();

	void			ClearCenterPrint( void );
	void			UpdateCenterPrint( const char *s, float alpha );
	void			UpdateLocationPrint( int x, int y, const char *s, float alpha );
	qboolean		LetterboxActive( void );
	void			InitSubtitle( void );
	void			FrameInitialized( void ) override;
	virtual void	Pressed( Event *ev );
};

class ConsoleView : public UIWidget {
public:
	CLASS_PROTOTYPE( ConsoleView );

protected:
	void Draw( void ) override;
};

#endif /* __CL_UIVIEW3D__ */
