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

#ifndef __UIPULLDOWNMENU_H__
#define __UIPULLDOWNMENU_H__

class uipull_describe {
public:
	str title;
	UColor highlightFGColor;
	UColor highlightBGColor;
	UColor FGColor;
	UColor BGColor;
	UIReggedMaterial *material;
	UIReggedMaterial *selected_material;
	Container<uipopup_describe *> desc;

	uipull_describe();
	uipull_describe( str title, UIReggedMaterial *material, UIReggedMaterial *selected_material );
};

inline
uipull_describe::uipull_describe()
{
	title = "";
	material = NULL;
	selected_material = NULL;
}

inline
uipull_describe::uipull_describe
	(
	str title,
	UIReggedMaterial *material,
	UIReggedMaterial *selected_material
	)

{
	this->title = title;
	this->material = material;
	this->selected_material = selected_material;
}

class UIPulldownMenu : public UIWidget {
protected:
	Container<uipull_describe *> m_desc;
	Listener *m_listener;
	int m_submenu;
	UIPopupMenu *m_submenuptr;
	UColor m_highlightBGColor;
	UColor m_highlightFGColor;

public:
	CLASS_PROTOTYPE( UIPulldownMenu );

protected:
	UIRect2D			getAlignmentRect( UIWidget *parent );
	float				getDescWidth( uipull_describe *desc );
	float				getDescHeight( uipull_describe *desc );
	uipull_describe		*getPulldown( str title );
	void				HighlightBGColor( Event *ev );
	void				HighlightFGColor( Event *ev );

public:
	UIPulldownMenu();

	void	Create( UIWidget *parent, Listener *listener, const UIRect2D& rect );
	void	CreateAligned( UIWidget *parent, Listener *listener );
	void	MousePressed( Event *ev );
	void	MouseDragged( Event *ev );
	void	MouseReleased( Event *ev );
	void	ChildKilled( Event *ev );
	void	AddUIPopupDescribe( const char *title, uipopup_describe *d );
	void	setHighlightFGColor( UColor c );
	void	setHighlightBGColor( UColor c );
	void	setPopupHighlightFGColor( str menu, UColor c );
	void	setPopupHighlightBGColor( str menu, UColor c );
	void	setPopupFGColor( str menu, UColor c );
	void	setPopupBGColor( str menu, UColor c );
	void	setPulldownShader( const char *title, UIReggedMaterial *mat );
	void	setSelectedPulldownShader( const char *title, UIReggedMaterial *mat );
	void	Realign( void );
	void	Draw( void ) override;
};

#endif

