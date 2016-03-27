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

#ifndef __CL_UIBIND_H__
#define __CL_UIBIND_H__

class bind_item_t {
public:
	str name;
	str command;
	UIReggedMaterial *nameMaterial;

	bind_item_t();
	bind_item_t( str name, str command, UIReggedMaterial *nameMaterial );
};

inline
bind_item_t::bind_item_t()
{
	nameMaterial = NULL;
}

inline
bind_item_t::bind_item_t
	(
	str name,
	str command,
	UIReggedMaterial *nameMaterial
	)

{
	this->name = name;
	this->command = command;
	this->nameMaterial = nameMaterial;
}

class bind_t {
public:
	int width;
	int height;
	int fillwidth;
	int commandwidth;
	int commandheight;
	int primarykeywidth;
	int primarykeyheight;
	int alternatekeywidth;
	int alternatekeyheight;
	int align;
	UColor titlebgcolor;
	UColor titlefgcolor;
	UColor activefgcolor;
	UColor activebgcolor;
	UColor activebordercolor;
	UColor inactivefgcolor;
	UColor inactivebgcolor;
	UColor highlightfgcolor;
	UColor highlightbgcolor;
	UColor selectfgcolor;
	UColor selectbgcolor;
	str changesound;
	str activesound;
	str entersound;
	str headers[ 3 ];
	UIReggedMaterial *headermats[ 3 ];
	UIReggedMaterial *fillmaterial;
	Container<bind_item_t *> binds;

public:
	bind_t();
	~bind_t();

	void Clear( void );
};

class bindlistener : public Listener {
protected:
	bind_t *bind;

public:
	CLASS_PROTOTYPE( bindlistener );

public:
	bindlistener();
	bindlistener( bind_t *b );

	bool	Load( Script& script );
	void	Header( Event *ev );
	void	Width( Event *ev );
	void	FillWidth( Event *ev );
	void	Height( Event *ev );
	void	CommandWidth( Event *ev );
	void	CommandHeight( Event *ev );
	void	PrimaryKeyWidth( Event *ev );
	void	PrimaryKeyHeight( Event *ev );
	void	AlternateKeyWidth( Event *ev );
	void	AlternateKeyHeight( Event *ev );
	void	NewItem( Event *ev );
	void	Align( Event *ev );
	void	TitleForegroundColor( Event *ev );
	void	TitleBackgroundColor( Event *ev );
	void	InactiveForegroundColor( Event *ev );
	void	InactiveBackgroundColor( Event *ev );
	void	ActiveForegroundColor( Event *ev );
	void	ActiveBackgroundColor( Event *ev );
	void	ActiveBorderColor( Event *ev );
	void	HighlightForegroundColor( Event *ev );
	void	HighlightBackgroundColor( Event *ev );
	void	SelectForegroundColor( Event *ev );
	void	SelectBackgroundColor( Event *ev );
	void	ChangeSound( Event *ev );
	void	ActiveSound( Event *ev );
	void	EnterSound( Event *ev );
};

#endif
