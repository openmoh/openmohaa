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

#ifndef __UIPULLDOWNMENUCONTAINER_H__
#define __UIPULLDOWNMENUCONTAINER_H__

class UIPulldownMenuContainer : public UIWidget {
protected:
	Container<uipopup_describe *> m_popups;
	Container<char *> m_dataContainer;
	UIPulldownMenu *m_menu;

public:
	CLASS_PROTOTYPE( UIPulldownMenuContainer );

	UIPulldownMenuContainer();

protected:
	void	FrameInitialized( void ) override;
	void	setBackgroundAlpha( float f ) override;
	void	setBackgroundColor( const UColor& color, bool setbordercolor ) override;
	void	setForegroundColor( const UColor& color ) override;
	void	SetPopupHighlightFGColor( Event *ev );
	void	SetPopupHighlightBGColor( Event *ev );
	void	SetPopupFGColor( Event *ev );
	void	SetPopupBGColor( Event *ev );
	void	SetHighlightFGColor( Event *ev );
	void	SetHighlightBGColor( Event *ev );
	void	SetMenuShader( Event *ev );
	void	SetSelectedMenuShader( Event *ev );
	void	Realign( void ) override;
	void	PulldownVirtualRes( Event *ev );

public:
	void	AddPopup( Event *ev );
};

#endif

