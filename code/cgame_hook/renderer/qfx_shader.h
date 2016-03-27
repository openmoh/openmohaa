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
#ifndef QFX_SHADER_H
#define QFX_SHADER_H

#define QFX_MAX_SHADER_UNIFORMS		8
#define QFX_MAX_SHADER_TEXTURES		8

class QFXShader
{
public:
	QFXShader( const char *szvp, const char *szfp, const char *program_name );
	~QFXShader();

	bool IsValid( void ) { return m_bValid; }

	void Bind( void );
	void Unbind( void );
	void SetParameter4f( int param, float x, float y, float z, float w );
	void SetParameter4fv( int param, float* v );

private:
	void PrintInfoLog( GLhandleARB object, const char *program_name );

private:
	bool		m_bValid;
	GLhandleARB m_hProgram;
	GLhandleARB m_hVShader;
	GLhandleARB m_hFShader;
	GLint		m_iUniforms[QFX_MAX_SHADER_UNIFORMS];
};

#endif //QFX_SHADER_H
