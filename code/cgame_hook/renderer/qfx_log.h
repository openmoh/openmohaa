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
#ifndef QFX_LOG_H
#define QFX_LOG_H

QFX_SINGLETON class QFXLog
{
private: 
	QFXLog();
	static QFXLog& GetInstance() { static QFXLog logObject; return logObject; }

public:
	~QFXLog();
	static QFXLog& Instance() { typedef QFXLog& (*pfnGetInstance)(); static pfnGetInstance pf = &GetInstance; return pf(); }

	void Printf( const char *fmt, ... );
	void Error( const char *fmt, ... );
	void Warning( const char *fmt, ... );

private:
	FILE*	m_pFile;
	char*	m_pszLogString;
};

#endif //QFX_LOG_H