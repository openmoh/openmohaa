/***************************************************************************
* Copyright (C) 2012, Chain Studios.
* 
* This file is part of QeffectsGL source code.
* 
* QeffectsGL source code is free software; you can redistribute it 
* and/or modify it under the terms of the GNU General Public License 
* as published by the Free Software Foundation; either version 2 of 
* the License, or (at your option) any later version.
* 
* QeffectsGL source code is distributed in the hope that it will be 
* useful, but WITHOUT ANY WARRANTY; without even the implied 
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software 
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
***************************************************************************/
#ifndef QFX_SETTINGS_H
#define QFX_SETTINGS_H

QFX_SINGLETON class QFXSettings
{
private: 
	QFXSettings();
	static QFXSettings& GetInstance() { static QFXSettings sObject; return sObject; }

public:
	~QFXSettings();
	static QFXSettings& Instance() { typedef QFXSettings& (*pfnGetInstance)(); static pfnGetInstance pf = &GetInstance; return pf(); }

	bool CheckEXE( const char *s );
	int GetInteger( const char *setting, int defaultValue );
	float GetFloat( const char *setting, float defaultValue );

private:
	char m_szIniFile[MAX_PATH];
	char m_szExeName[MAX_PATH];
};

#endif //QFX_SETTINGS_H