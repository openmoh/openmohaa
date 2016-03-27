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

#include "cl_ui.h"

CLASS_DECLARATION( USignal, FilePickerClass, NULL )
{
	{ NULL, NULL }
};

FilePickerClass::FilePickerClass()
{
	// FIXME: stub
}

FilePickerClass::~FilePickerClass()
{
	// FIXME: stub
}

void FilePickerClass::SetupFiles( void )
{
	// FIXME: stub
}

void FilePickerClass::GotoParentDirectory( void )
{
	// FIXME: stub
}

void FilePickerClass::GotoSubDirectory( str subdir )
{
	// FIXME: stub
}

void FilePickerClass::Initialize( const char *root_directory, const char *current_directory, const char *ext )
{
	// FIXME: stub
}

void FilePickerClass::CloseWindow( void )
{
	// FIXME: stub
}

void FilePickerClass::FileSelected( Event *ev )
{
	// FIXME: stub
}

void FilePickerClass::FileSelected( str &currentDirectory, str &partialName, str &fullname )
{
	// FIXME: stub
}

void FilePickerClass::FileChosen( Event *ev )
{
	// FIXME: stub
}

void FilePickerClass::FileChosen( str &currentDirectory, str &partialName, str &fullname )
{
	// FIXME: stub
}

void FilePickerClass::OnDeactivated( Event  *ev )
{
	// FIXME: stub
}

void FilePickerClass::Setup( const char *root_directory, const char *current_directory, const char *ext )
{
	// FIXME: stub
}
