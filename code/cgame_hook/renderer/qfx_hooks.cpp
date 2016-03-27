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
#include "qfx_library.h"
#include "qfx_log.h"
#include "qfx_opengl.h"
#include "qfx_renderer.h"

//=========================================
// QFX Library Hooks
//-----------------------------------------
// Call our renderer in proper places
//=========================================
static const char s_szCustomVendorName[] = QFX_LIBRARY_VENDOR;
static GLenum s_glMatrixMode = GL_MODELVIEW;

#define IS_3D_PROJECTION_MATRIX(x)	( x[2*4+3] < 0 )

const GLubyte * WINAPI glGetString( GLenum name )
{
	if ( name == GL_VENDOR )
		return reinterpret_cast<const GLubyte*>(s_szCustomVendorName);
	else
		return gl::qglGetString( name ); 
}

void WINAPI glMatrixMode( GLenum mode )
{
	s_glMatrixMode = mode;
	gl::qglMatrixMode( mode );
}

void WINAPI glLoadMatrixd( const GLdouble *m )
{
	gl::qglLoadMatrixd( m );
	if ( s_glMatrixMode == GL_PROJECTION ) {
		if ( gl::ext.FBO ) {
			GLuint fboBinding = 0;
			gl::qglGetIntegerv( GL_FRAMEBUFFER_BINDING_EXT, (GLint*)&fboBinding );
			if ( fboBinding ) return;
		}
		if ( IS_3D_PROJECTION_MATRIX(m) ) {
			QFXRenderer::Instance().SetupZDimensions( m );
			QFXRenderer::Instance().SetOrtho( GL_FALSE );
		} else {
			QFXRenderer::Instance().SetOrtho( GL_TRUE );
		}
	}
}

void WINAPI glLoadMatrixf( const GLfloat *m )
{
	gl::qglLoadMatrixf( m );
	if ( s_glMatrixMode == GL_PROJECTION ) {
		if ( gl::ext.FBO ) {
			GLuint fboBinding = 0;
			gl::qglGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&fboBinding );
			if ( fboBinding ) return;
		}
		if ( IS_3D_PROJECTION_MATRIX(m) ) {
			QFXRenderer::Instance().SetupZDimensions( m );
			QFXRenderer::Instance().SetOrtho( GL_FALSE );
		} else {
			QFXRenderer::Instance().SetOrtho( GL_TRUE );
		}
	}
}

void WINAPI glFrustum( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar )
{ 
	gl::qglFrustum( left, right, bottom, top, zNear, zFar );
	if ( s_glMatrixMode == GL_PROJECTION ) {
		if ( gl::ext.FBO ) {
			GLuint fboBinding = 0;
			gl::qglGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&fboBinding );
			if ( fboBinding ) return;
		}
		QFXRenderer::Instance().SetupZDimensions( (GLfloat)zNear, (GLfloat)zFar );
		QFXRenderer::Instance().SetOrtho( GL_FALSE );
	}
}

void WINAPI glOrtho( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar )
{
	gl::qglOrtho( left, right, bottom, top, zNear, zFar );
	if ( s_glMatrixMode == GL_PROJECTION ) {
		if ( gl::ext.FBO ) {
			GLuint fboBinding = 0;
			gl::qglGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&fboBinding );
			if ( fboBinding ) return;
		}
		QFXRenderer::Instance().SetOrtho( GL_TRUE );
	}
}

void WINAPI glDepthFunc( GLenum func )
{
	QFXRenderer::Instance().SetupDepthFunc( func );
}

void WINAPI glDepthRange( GLclampd zNear, GLclampd zFar )
{
	QFXRenderer::Instance().SetupDepthRange( zNear, zFar );
}

void WINAPI glTexImage2D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
	gl::qglTexImage2D( target, level, internalformat, width, height, border, format, type, pixels );
	if ( (target == GL_TEXTURE_2D) && !level && pixels )
		QFXRenderer::Instance().OnTexImage2D();
}

void WINAPI glTexParameterf( GLenum target, GLenum pname, GLfloat param )
{
	if ( ( pname == GL_TEXTURE_MAX_ANISOTROPY_EXT ) && ( param > 1 ) )
		QFXRenderer::Instance().SetupAnisotropy();

	gl::qglTexParameterf( target, pname, param );
}

void WINAPI glTexParameteri( GLenum target, GLenum pname, GLint param )
{
	if ( ( pname == GL_TEXTURE_MAX_ANISOTROPY_EXT ) && ( param > 1 ) )
		QFXRenderer::Instance().SetupAnisotropy();

	gl::qglTexParameteri( target, pname, param );
}

void WINAPI glEnable( GLenum cap )
{
	if ( cap == GL_MULTISAMPLE_ARB )
		QFXRenderer::Instance().SetupMultisample();

	gl::qglEnable( cap );
}

int WINAPI wrap_wglChoosePixelFormat( HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd )
{ 
	PIXELFORMATDESCRIPTOR newPfd;
	memcpy( &newPfd, ppfd, sizeof( PIXELFORMATDESCRIPTOR ) );
	newPfd.cColorBits = 32;

	gl::CheckInit();
	return gl::wglChoosePixelFormat( hdc, &newPfd );
}

BOOL WINAPI wrap_wglSetPixelFormat( HDC hdc, int i, CONST PIXELFORMATDESCRIPTOR *pfd )
{
	gl::CheckInit();
	return QFXRenderer::Instance().OnSetPixelFormat( hdc, i, pfd );
}

BOOL WINAPI wrap_wglMakeCurrent( HDC hdc, HGLRC hglrc )
{
	gl::CheckInit();
	if (!gl::wglMakeCurrent(hdc, hglrc))
		return FALSE;

	QFXRenderer::Instance().OnMakeCurrent( hdc, hglrc );
	return TRUE;
}

BOOL WINAPI wrap_wglDeleteContext( HGLRC hglrc )
{
	gl::CheckInit();
	QFXRenderer::Instance().OnDeleteContext( hglrc );
	return gl::wglDeleteContext( hglrc );
}

BOOL WINAPI wrap_wglSwapBuffers( HDC hdc )
{
	gl::CheckInit();
	QFXRenderer::Instance().OnSwapBuffers();
	return gl::wglSwapBuffers( hdc );
}
