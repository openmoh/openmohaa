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

#ifndef __CL_UIMAPRUNNER_H__
#define __CL_UIMAPRUNNER_H__

class MapRunnerClass : public FilePickerClass {
public:
	CLASS_PROTOTYPE( MapRunnerClass );

	void	FileChosen( str &currentDirectory, str &partialName, str &fullname );
};

class ViewSpawnerClass : public FilePickerClass {
public:
	CLASS_PROTOTYPE( ViewSpawnerClass );

	void	FileChosen( str &currentDirectory, str &partialName, str &fullname );
};

class LODSpawnerClass : public FilePickerClass {
public:
	CLASS_PROTOTYPE( LODSpawnerClass );

	void	FileChosen( str &currentDirectory, str &partialName, str &fullname );
};

class UIPickFileClass : public FilePickerClass {
public:
	Listener *retobj;
	Event *retevent;

	CLASS_PROTOTYPE( UIPickFileClass );

	void	FileChosen( str &currentDirectory, str &partialName, str &fullname );
};

void PickFile( const char *name, Listener *obj, Event& event );

#endif /* __CL_UIMAPRUNNER_H__ */
