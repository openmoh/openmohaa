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
#include "qfx_settings.h"
#include "qfx_opengl.h"
#include "qfx_renderer.h"
#include "qfx_shader.h"
#include "qfx_glprogs.h"

#include "script/cplayer.h"
#include "script/vision.h"

#include "glb_local.h"

//=========================================
// QFXRenderer class implementation
//-----------------------------------------
// This is QeffectsGL's core
//=========================================
using namespace gl;

QFXRenderer :: QFXRenderer()
{
	m_hDC = 0;
	m_hGLRC = 0;
	m_uiFrameCounter = 0;
	m_uiRenderFrame = 0;
	m_uiRenderFrameCounter = 0;
	m_uiViewportFrame = 0;
	m_zNear = 1.0f;
	m_zFar = 1000.0f;
	m_bOrtho = GL_FALSE;
	m_bZTrick = GL_FALSE;
	m_bInternalAnisotropy = GL_FALSE;
	m_bInternalMultisample = GL_FALSE;
	m_bMultisample = GL_FALSE;
	m_uiBlurTexture = 0;
	m_uiBlurDepth = 0;
	m_uiScreenRGB = 0;
	m_uiScreenDepth = 0;
	m_pBloomDarkenShader = NULL;
	m_pBloomBlurShader = NULL;
	m_pBloomCombineShader = NULL;
	m_pSSAORenderShader = NULL;
	m_pSSAOBlurShader = NULL;
	m_pSSAOCombineShader = NULL;
	m_pHSLShader = NULL;
	m_pEmbossShader = NULL;
	m_iEnableRender = 1;
	m_flLastDepth = -1.0f;
	m_flLastTime = timeGetTime();
	m_flOldTime = timeGetTime();
	m_flFrameTime = 0.001f;

	GetVariables();
}

void QFXRenderer :: GetVariables( void )
{
	m_varDebug				= r_debug->integer; //QFXSettings::Instance().GetInteger( "Debug", 0 );
	m_varCompareMode		= 0; //QFXSettings::Instance().GetInteger( "CompareMode", 0 );

	m_varZTrickFix			= 1; //QFXSettings::Instance().GetInteger( "ZTrickFix", 0 );
	m_varViewportFix		= 1; //QFXSettings::Instance().GetInteger( "ViewportFix", 0 );

	m_varRenderFrame		= r_debug->integer; //QFXSettings::Instance().GetInteger( "RenderFrame", 0 );

	m_varAnisotropy			= r_texFilterAniso->integer; //QFXSettings::Instance().GetInteger( "Anisotropy", 8 );

	m_varMultisample		= 8; //QFXSettings::Instance().GetInteger( "Multisample", 4 );
	m_varMultisampleHint	= 0; //QFXSettings::Instance().GetInteger( "MultisampleHint", 0 );

#if 0
	m_varBloomEnable		= r_glowEnable->integer && r_glow.integer; //QFXSettings::Instance().GetInteger( "Bloom", 1 );
	m_varBloomNumSteps		= r_glowQuality->integer; //min( 16, QFXSettings::Instance().GetInteger( "BloomSteps", 8 ) );
	m_varBloomDarkenPower	= ( r_glowBloomCutoff.value_current + 1.5f ) * 2.0f; //r_glowRadius0.value_current; //QFXSettings::Instance().GetFloat( "BloomDarken", 3 );
	m_varBloomRadius0		= r_glowRadius0.value_current;
	m_varBloomRadius1		= r_glowRadius1.value_current;
	m_varBloomCombineScale0	= r_glowBloomIntensity0.value_current; //QFXSettings::Instance().GetFloat( "BloomScale", 1.5f );
	m_varBloomCombineScale1	= r_glowBloomIntensity1.value_current;
#endif

	m_varSSAOEnable			= r_ssao->integer; //QFXSettings::Instance().GetInteger( "SSAO", 1 );
	m_varSSAOScale			= 8.0f; //QFXSettings::Instance().GetFloat( "SSAOScale", 8.0f );
	m_varSSAOMinZ			= 0.005f; //QFXSettings::Instance().GetFloat( "SSAOMinZ", 0.005f );
	m_varSSAOMaxZ			= 1.0f; //QFXSettings::Instance().GetFloat( "SSAOMaxZ", 1.0f );

	m_varHue				= 1; //QFXSettings::Instance().GetInteger( "Hue", 0 );
	m_varSaturation			= 1; //QFXSettings::Instance().GetInteger( "Saturation", 0 );
	m_varLightness			= 1; //QFXSettings::Instance().GetInteger( "Lightness", 0 );

	m_varEmboss				= 0; //QFXSettings::Instance().GetInteger( "Emboss", 1 );
	m_varEmbossScale		= 0.5f; //QFXSettings::Instance().GetFloat( "EmbossScale", 0.5f );

	m_varDOF				= 0; //QFXSettings::Instance().GetInteger( "DOF", 1 );
	m_varDOFSpeed			= 10.0f; //QFXSettings::Instance().GetFloat( "DOFSpeed", 1.0f );
	m_varDOFScale			= 100.0f; //QFXSettings::Instance().GetFloat( "DOFScale", 2.0f );
	m_varDOFMinZ			= 0.0f; //0.005f; //QFXSettings::Instance().GetFloat( "DOFMinZ", 0.005f );
}

int QFXRenderer :: ChooseMultisamplePixelFormat( int fmt, HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd )
{
	static bool classRegistered = false;
	GLint sampleBuffers = 0;

	// register temp window class
	if ( !classRegistered ) {
		WNDCLASS wc;
		memset( &wc, 0, sizeof( wc ) );
		wc.lpfnWndProc   = DefWindowProc;
		wc.hInstance     = GetModuleHandle( NULL );
		wc.lpszClassName = QFX_LIBRARY_TITLE;

		if ( !RegisterClass( &wc ) ) {
			QFXLog::Instance().Error( "ChooseMultisamplePixelFormat: RegisterClass failed\n" );
			return -1;
		}
		
		classRegistered = true;
	}

	// create temporary hidden window
	HWND tempHWND = CreateWindowEx ( 0, QFX_LIBRARY_TITLE, "", WS_POPUP, 0, 0, 1, 1, NULL, NULL, GetModuleHandle( NULL ), NULL );
	if ( !tempHWND ) {
		QFXLog::Instance().Error( "ChooseMultisamplePixelFormat: CreateWindowEx failed\n" );
		return -1;
	}

	HDC tempDC = GetDC( tempHWND );
	if ( !tempDC ) {
		QFXLog::Instance().Error( "ChooseMultisamplePixelFormat: GetDC failed\n" );
		DestroyWindow( tempHWND );
		return -1;
	}

	if( !gl::wglSetPixelFormat( tempDC, fmt, ppfd ) ) {
		QFXLog::Instance().Error( "ChooseMultisamplePixelFormat: SetPixelFormat failed\n" );
		ReleaseDC( tempHWND, tempDC );
		DestroyWindow( tempHWND );
		return -1;
    }

	HGLRC tempRC = gl::wglCreateContext( tempDC );
	if ( !tempRC ) {
		QFXLog::Instance().Error( "ChooseMultisamplePixelFormat: wglCreateContext failed\n" );
		ReleaseDC( tempHWND, tempDC );
		DestroyWindow( tempHWND );
		return -1;
	}

	if ( !gl::wglMakeCurrent( tempDC, tempRC ) ) {
		gl::wglDeleteContext( tempRC );
		QFXLog::Instance().Error( "ChooseMultisamplePixelFormat: wglMakeCurrent failed\n" );
		ReleaseDC( tempHWND, tempDC );
		DestroyWindow( tempHWND );
		return -1;
	}

	qglGetIntegerv( GL_SAMPLE_BUFFERS_ARB, &sampleBuffers );
	if ( sampleBuffers )
		m_bInternalMultisample = GL_TRUE;

	PFNWGLCHOOSEPIXELFORMATARBPROC qwglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)gl::wglGetProcAddress( "wglChoosePixelFormatARB" );
	if ( !qwglChoosePixelFormatARB ) {
		gl::wglMakeCurrent( tempDC, NULL );
		gl::wglDeleteContext( tempRC );
		QFXLog::Instance().Warning( "missing extension WGL_ARB_pixel_format\n" );
		QFXLog::Instance().Warning( "multisample is not supported\n" );
		ReleaseDC( tempHWND, tempDC );
		DestroyWindow( tempHWND );
		return -1;
	}

	int pixelFormat;
	UINT numFormats;
	float fAttributes[] = { 0, 0 };

	while ( m_varMultisample >= 2 ) {
		int iAttributes[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_ALPHA_BITS_ARB, ppfd->cAlphaBits,
			WGL_DEPTH_BITS_ARB, (ppfd->cDepthBits > 16) ? 24 : 16,
			WGL_STENCIL_BITS_ARB, ppfd->cStencilBits,
			WGL_DOUBLE_BUFFER_ARB, (ppfd->dwFlags & PFD_DOUBLEBUFFER) ? GL_TRUE : GL_FALSE,
			WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
			WGL_SAMPLES_ARB, m_varMultisample,
			0,0};

		if ( qwglChoosePixelFormatARB( tempDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats ) ) {
			if ( numFormats > 0 )
				break;
		}
		
		QFXLog::Instance().Warning( "%ix FSAA is not supported\n", m_varMultisample );

		if (m_varMultisample > 8)
			m_varMultisample >>= 1;
		else
			m_varMultisample -= 2;
	}

	if ( m_varMultisample >= 2 ) {
		QFXLog::Instance().Printf( "Using %ix FSAA\n", m_varMultisample );
	} else {
		QFXLog::Instance().Warning( "FSAA is not supported by hardware\n" );
		pixelFormat = -1;
	}

	gl::wglMakeCurrent( tempDC, NULL );
	gl::wglDeleteContext( tempRC );
	ReleaseDC( tempHWND, tempDC );
	DestroyWindow( tempHWND );
	return pixelFormat;
}

BOOL QFXRenderer :: OnSetPixelFormat( HDC hdc, int pixelformat, CONST PIXELFORMATDESCRIPTOR *ppfd )
{
	if ( m_varMultisample > 1 ) {
		int multisamplePixelFormat = ChooseMultisamplePixelFormat( pixelformat, hdc, ppfd );
		if ( multisamplePixelFormat >= 0 ) {
			BOOL b = gl::wglSetPixelFormat( hdc, multisamplePixelFormat, ppfd );
			if ( b ) {
				m_bMultisample = GL_TRUE;
				return b;
			}
		}
	}

	return gl::wglSetPixelFormat( hdc, pixelformat, ppfd );
}

void QFXRenderer :: OnMakeCurrent( HDC hdc, HGLRC hglrc )
{
	if ( !hdc || !hglrc )
		return;

	if ( hdc != m_hDC || hglrc != m_hGLRC ) {
		m_hDC = hdc;
		m_hGLRC = hglrc;
		gl::InitializeExtensions();
		InitializeGL();

		if ( m_varMultisample ) {
			qglEnable( GL_MULTISAMPLE_ARB );
			if ( m_varMultisampleHint )
				qglHint( GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST );
		}
	}
}

void QFXRenderer :: OnDeleteContext( HGLRC hglrc )
{
	if ( !hglrc )
		return;

	if ( hglrc == m_hGLRC )
		ShutdownGL();
}

void QFXRenderer :: OnSwapBuffers( void )
{
	if ( -1 == m_varRenderFrame )
		RenderGL();

	m_uiFrameCounter++;
}

void QFXRenderer :: SetupAnisotropy( void )
{
	m_bInternalAnisotropy = GL_TRUE;
}

void QFXRenderer :: OnTexImage2D( void )
{
	if ( ext.maxAnisotropy > 1 && m_varAnisotropy > 1 ) {
		int value = min( ext.maxAnisotropy, m_varAnisotropy );
		qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value );
	}
}

void QFXRenderer :: CheckHardware( void )
{
	QFXLog::Instance().Printf( "Checking hardware caps:\n" );

	m_bSupportsBloom = GL_TRUE; //(( ( ext.maxActiveTextures >= 2 ) && ext.VertexShader && ext.FragmentShader && ext.RectangleTexture ) ? GL_TRUE : GL_FALSE );
	m_bSupportsSSAO = GL_TRUE; //(( ( ext.maxActiveTextures >= 2 ) && ext.VertexShader && ext.FragmentShader && ext.RectangleTexture && ext.DepthTexture ) ? GL_TRUE : GL_FALSE );
	m_bSupportsColorCorrection = GL_TRUE; //(( ext.VertexShader && ext.FragmentShader && ext.RectangleTexture ) ? GL_TRUE : GL_FALSE );
	m_bSupportsEmboss = GL_TRUE; //(( ext.VertexShader && ext.FragmentShader && ext.RectangleTexture ) ? GL_TRUE : GL_FALSE );
	m_bSupportsDOF = GL_TRUE; //(( ( ext.maxActiveTextures >= 3 ) && ext.VertexShader && ext.FragmentShader && ext.RectangleTexture && ext.DepthTexture ) ? GL_TRUE : GL_FALSE );
}

void QFXRenderer :: InitializeGL( void )
{
	GLboolean stateRectangleEnabled;
	//GLboolean state2DEnabled;
	GLuint stateRectangleBinding = 0;
	GLuint state2DBinding = 0;
	qglGetIntegerv( GL_VIEWPORT, m_3DViewport );

	m_uiTextureIndex = QFX_TEXTURE_IDENTIFIER_BASE;
	
	QFXLog::Instance().Printf( "InitializeGL\n" );

	HWND hCurrentWnd = WindowFromDC( gl::wglGetCurrentDC() );
	RECT rc;
	GetClientRect( hCurrentWnd, &rc );
	m_WindowSize[0] = rc.right - rc.left;
	m_WindowSize[1] = rc.bottom - rc.top;

	CheckHardware();

	// load GLSL shaders
	if ( m_bSupportsBloom ) {
		m_pBloomDarkenShader = new QFXShader( g_szVertexShader_BloomDarken, g_szFragmentShader_BloomDarken, "Bloom Darken" );
		m_pBloomBlurShader = new QFXShader( g_szVertexShader_BloomBlur, g_szFragmentShader_BloomBlur, "Bloom Blur" );
		//m_pBloomBlurVertShader = new QFXShader( g_szVertexShader_BloomBlurVert, g_szFragmentShader_BloomBlurVert, "Bloom Blur Vertical" );
		m_pBloomCombineShader = new QFXShader( g_szVertexShader_BloomCombine, g_szFragmentShader_BloomCombine, "Bloom Combine" );
		if ( !m_pBloomDarkenShader || !m_pBloomDarkenShader->IsValid() ) m_bSupportsBloom = GL_FALSE;
		if ( !m_pBloomBlurShader || !m_pBloomBlurShader->IsValid() ) m_bSupportsBloom = GL_FALSE;
		if ( !m_pBloomCombineShader || !m_pBloomCombineShader->IsValid() ) m_bSupportsBloom = GL_FALSE;
	}

	if ( m_bSupportsSSAO ) {
		m_pSSAORenderShader = new QFXShader( g_szVertexShader_SSAORender, g_szFragmentShader_SSAORender, "SSA Renderer" );
		m_pSSAOBlurShader = new QFXShader( g_szVertexShader_SSAOBlur, g_szFragmentShader_SSAOBlur, "SSAO Blur" );
		m_pSSAOCombineShader = new QFXShader( g_szVertexShader_SSAOCombine, g_szFragmentShader_SSAOCombine, "SSAO Combine" );
		if ( !m_pSSAORenderShader || !m_pSSAORenderShader->IsValid() ) m_bSupportsSSAO = GL_FALSE;
		if ( !m_pSSAOBlurShader || !m_pSSAOBlurShader->IsValid() ) m_bSupportsSSAO = GL_FALSE;
		if ( !m_pSSAOCombineShader || !m_pSSAOCombineShader->IsValid() ) m_bSupportsSSAO = GL_FALSE;
	}

	if ( m_bSupportsColorCorrection ) {
		m_pHSLShader = new QFXShader( g_szVertexShader_HSL, g_szFragmentShader_HSL, "HSL Shader" );
		if ( !m_pHSLShader || !m_pHSLShader->IsValid() ) m_bSupportsColorCorrection = GL_FALSE;
	}

	if ( m_bSupportsEmboss ) {
		m_pEmbossShader = new QFXShader( g_szVertexShader_Emboss, g_szFragmentShader_Emboss, "Emboss Shader" );
		if ( !m_pEmbossShader || !m_pEmbossShader->IsValid() ) m_bSupportsEmboss = GL_FALSE;
	}

	if ( m_bSupportsDOF ) {
		m_pDOFBlurShader = new QFXShader( g_szVertexShader_DOFBlur, g_szFragmentShader_DOFBlur, "DOF Blur" );
		m_pDOFDepthShader = new QFXShader( g_szVertexShader_DOFDepth, g_szFragmentShader_DOFDepth, "DOF Depth" );
		m_pDOFRenderShader = new QFXShader( g_szVertexShader_DOFRender, g_szFragmentShader_DOFRender, "DOF Render" );
		if ( !m_pDOFBlurShader || !m_pDOFBlurShader->IsValid() ) m_bSupportsDOF = GL_FALSE;
		if ( !m_pDOFDepthShader || !m_pDOFDepthShader->IsValid() ) m_bSupportsDOF = GL_FALSE;
		if ( !m_pDOFRenderShader || !m_pDOFRenderShader->IsValid() ) m_bSupportsDOF = GL_FALSE;
	}

	m_pBrightnessContrastShader = new QFXShader( g_szVertexShader_BrightnessContrast, g_szFragmentShader_BrightnessContrast, "Brightness and contrast" );
	m_pTintShader = new QFXShader( g_szVertexShader_Tint, g_szFragmentShader_Tint, "Tint" );
	m_pBleachShader = new QFXShader( g_szVertexShader_Bleach, g_szFragmentShader_Bleach, "Bleach" );
	/*m_pMotionBlurShader = new QFXShader( g_szVertexShader_MotionBlur, g_szFragmentShader_MotionBlur, "Motion Blur" );
	m_pShadowShader = new QFXShader( g_szVertexShader_Shadow, g_szFragmentShader_Shadow, "Shadow" );
	m_pLightReflectShader = new QFXShader( g_szVertexShader_LightReflection, g_szFragmentShader_LightReflection, "Light Reflection" );*/
	m_pDistortionShader = new QFXShader( g_szVertexShader_Distortion, g_szFragmentShader_Distortion, "Distortion" );
	m_pBlurShader = new QFXShader( g_szVertexShader_Blur, g_szFragmentShader_Blur, "Blur" );

	m_pFXAAShader = new QFXShader( g_szVertexShader_FXAA, g_szFragmentShader_FXAA, "FXAA" );
	m_pDisplacement = new QFXShader( g_szVertexShader_Displacement, g_szFragmentShader_Displacement, "Displacement Mapping" );

	m_pShadowMapping = new QFXShader( g_szVertexShader_ShadowMap, g_szFragmentShader_ShadowMap, "Shadow Mapping" );

	// create textures
	if ( ext.RectangleTexture ) {
		stateRectangleEnabled = qglIsEnabled( GL_TEXTURE_RECTANGLE_ARB );
		qglGetIntegerv( GL_TEXTURE_BINDING_RECTANGLE_ARB, ( GLint* )( &stateRectangleBinding ) );
		qglEnable( GL_TEXTURE_RECTANGLE_ARB );

		if ( m_bSupportsBloom || m_bSupportsSSAO || m_bSupportsDOF ) {
			m_uiBlurTexture = AllocTextureId();
			qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiBlurTexture );
			qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, ext.edgeClampMode );
			qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, ext.edgeClampMode );
			qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			qglTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 3, m_3DViewport[ 2 ] >> 1, m_3DViewport[ 3 ] >> 1, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );
		}
		if ( m_bSupportsDOF ) {
			m_uiBlurDepth = AllocTextureId();
			qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiBlurDepth );
			qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, ext.edgeClampMode );
			qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, ext.edgeClampMode );
			qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			qglTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 3, m_3DViewport[ 2 ] >> 1, m_3DViewport[ 3 ] >> 1, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );
		}

		m_uiScreenRGB = AllocTextureId();
		qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiScreenRGB );
		qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, ext.edgeClampMode );
		qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, ext.edgeClampMode );
		qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		qglTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 3, m_3DViewport[ 2 ], m_3DViewport[ 3 ], 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );

		/*
		//m_eyeLeft = AllocTextureId();
		qglBindTexture( GL_TEXTURE_2D, AllocTextureId() );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
		qglTexImage2D( GL_TEXTURE_2D, 0, 4, m_3DViewport[ 2 ], m_3DViewport[ 3 ], 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 0 );

		//m_eyeRight = AllocTextureId();
		qglBindTexture( GL_TEXTURE_2D, AllocTextureId() );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
		qglTexImage2D( GL_TEXTURE_2D, 0, 4, m_3DViewport[ 2 ], m_3DViewport[ 3 ], 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 0 );
		*/

		if ( m_bSupportsSSAO || m_bSupportsDOF ) {
			m_uiScreenDepth = AllocTextureId();
			qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiScreenDepth );
			qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_TEXTURE_MODE_ARB, GL_LUMINANCE );
			qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, ext.edgeClampMode );
			qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, ext.edgeClampMode );
			qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			qglTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			qglTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT24, m_3DViewport[ 2 ], m_3DViewport[ 3 ], 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0 );
		}

		qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, stateRectangleBinding );
		if ( !stateRectangleEnabled ) qglDisable( GL_TEXTURE_RECTANGLE_ARB );

		// Try to use a texture depth component
		m_uiShadowDepth = AllocTextureId();
		qglBindTexture( GL_TEXTURE_2D, m_uiShadowDepth );

		// GL_LINEAR does not make sense for depth texture. However, next tutorial shows usage of GL_LINEAR and PCF
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

		// Remove artefact on the edges of the shadowmap
		qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

		qglTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 256, 256, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0 );
		qglBindTexture( GL_TEXTURE_2D, 0 );
	}

	if( ext.FBO )
	{
		// create a framebuffer object
		qglGenFramebuffersEXT( 1, &m_uiFrameBuffer );
		qglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_uiFrameBuffer );
		qglFramebufferTexture2DEXT( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_uiShadowDepth, 0 );
		qglDrawBuffer( GL_NONE );
		qglReadBuffer( GL_NONE );
		qglBindFramebufferEXT( GL_FRAMEBUFFER, 0 );
	}

/*	state2DEnabled = qglIsEnabled( GL_TEXTURE_2D );
	qglGetIntegerv( GL_TEXTURE_BINDING_2D, (GLint*)(&state2DBinding) );
	qglEnable( GL_TEXTURE_2D );

	//load 2D textures, if any

	qglBindTexture( GL_TEXTURE_2D, state2DBinding );
	if ( !state2DEnabled ) qglDisable( GL_TEXTURE_2D );*/	

	if ( !m_uiScreenRGB ) {
		m_bSupportsEmboss = GL_FALSE;
		m_bSupportsColorCorrection = GL_FALSE;
		m_bSupportsSSAO = GL_FALSE;
		m_bSupportsDOF = GL_FALSE;
		m_bSupportsBloom = GL_FALSE;
	}
	if ( !m_uiBlurTexture ) {
		m_bSupportsBloom = GL_FALSE;
		m_bSupportsSSAO = GL_FALSE;
		m_bSupportsDOF = GL_FALSE;
	}
	if ( !m_uiScreenDepth ) {
		m_bSupportsSSAO = GL_FALSE;
		m_bSupportsDOF = GL_FALSE;
	}
	if ( !m_uiBlurDepth ) {
		m_bSupportsDOF = GL_FALSE;
	}
}

void QFXRenderer :: ShutdownGL( void )
{
	QFXLog::Instance().Printf( "ShutdownGL\n" );

	if ( m_pBloomDarkenShader ) {
		delete m_pBloomDarkenShader;
		m_pBloomDarkenShader = NULL;
	}
	if ( m_pBloomBlurShader ) {
		delete m_pBloomBlurShader;
		m_pBloomBlurShader = NULL;
	}
	if ( m_pBloomCombineShader ) {
		delete m_pBloomCombineShader;
		m_pBloomCombineShader = NULL;
	}
	if ( m_pSSAORenderShader ) {
		delete m_pSSAORenderShader;
		m_pSSAORenderShader = NULL;
	}
	if ( m_pSSAOBlurShader ) {
		delete m_pSSAOBlurShader;
		m_pSSAOBlurShader = NULL;
	}
	if ( m_pSSAOCombineShader ) {
		delete m_pSSAOCombineShader;
		m_pSSAOCombineShader = NULL;
	}
	if ( m_pHSLShader ) {
		delete m_pHSLShader;
		m_pHSLShader = NULL;
	}
	if ( m_pEmbossShader ) {
		delete m_pEmbossShader;
		m_pEmbossShader = NULL;
	}
	if ( m_pDOFBlurShader ) {
		delete m_pDOFBlurShader;
		m_pDOFBlurShader = NULL;
	}
	if ( m_pDOFDepthShader ) {
		delete m_pDOFDepthShader;
		m_pDOFDepthShader = NULL;
	}
	if ( m_pDOFRenderShader ) {
		delete m_pDOFRenderShader;
		m_pDOFRenderShader = NULL;
	}

	if ( m_uiBlurTexture ) {
		qglDeleteTextures( 1, &m_uiBlurTexture );
		m_uiBlurTexture = 0;
	}
	if ( m_uiBlurDepth ) {
		qglDeleteTextures( 1, &m_uiBlurDepth );
		m_uiBlurDepth = 0;
	}
	if ( m_uiScreenRGB ) {
		qglDeleteTextures( 1, &m_uiScreenRGB );
		m_uiScreenRGB = 0;
	}
	if ( m_uiScreenDepth ) {
		qglDeleteTextures( 1, &m_uiScreenDepth );
		m_uiScreenDepth = 0;
	}
}

void QFXRenderer :: SetupZDimensions( GLfloat znear, GLfloat zfar )
{
	m_zNear = znear;
	m_zFar = zfar;

	if ( r_debug->integer )
		QFXLog::Instance().Printf("%5i: SetupZDimensions zn = %f, zf = %f\n", m_uiFrameCounter, m_zNear, m_zFar );
}

void QFXRenderer :: SetupZDimensions( const GLfloat* matrix )
{
	GLfloat fC = matrix[2*4+2];
	GLfloat fD = matrix[3*4+2];
	GLfloat fQ = (1.0f + fC) / (1.0f - fC);
	m_zFar = (fD * (1.0f + fQ)) / (2.0f * fQ);
	m_zNear = (fD * m_zFar) / (fD - 2.0f*m_zFar);
	if ( m_zFar < 0 ) m_zFar = -m_zFar;

	if ( r_debug->integer )
		QFXLog::Instance().Printf("%5i: SetupZDimensions zn = %f, zf = %f\n", m_uiFrameCounter, m_zNear, m_zFar );
}

void QFXRenderer :: SetupZDimensions( const GLdouble* matrix )
{
	GLfloat fC = (GLfloat)matrix[2*4+2];
	GLfloat fD = (GLfloat)matrix[3*4+2];
	GLfloat fQ = (1.0f + fC) / (1.0f - fC);
	m_zFar = (fD * (1.0f + fQ)) / (2.0f * fQ);
	m_zNear = (fD * m_zFar) / (fD - 2.0f*m_zFar);
	if ( m_zFar < 0 ) m_zFar = -m_zFar;

	if ( r_debug->integer )
		QFXLog::Instance().Printf("%5i: SetupZDimensions zn = %f, zf = %f\n", m_uiFrameCounter, m_zNear, m_zFar );
}

void QFXRenderer :: SetupDepthFunc( GLenum func )
{
	if ( m_varZTrickFix && ( func == GL_GEQUAL ) ) {
		m_bZTrick = true;
		if ( r_debug->integer )
			QFXLog::Instance().Printf("%5i: ZTrick attempt detected\n", m_uiFrameCounter );
		qglDepthFunc( GL_LEQUAL );
	} else {
		qglDepthFunc( func );
	}

	if ( m_bZTrick )
		qglClear( GL_DEPTH_BUFFER_BIT );
}

void QFXRenderer :: SetupDepthRange( GLclampd zNear, GLclampd zFar )
{
	if ( m_bZTrick ) {
		if ( zNear > zFar ) {
			zNear = 1.0f - zNear;
			zFar = 1.0f - zFar;
		}
		if ( zFar > 0.45 ) zFar = 1.0;
		if ( zNear > 0.99 ) zNear = 0.0f;
	}
	qglDepthRange( zNear, zFar );
}

void QFXRenderer :: SetupMultisample( void )
{
	m_bInternalMultisample = GL_TRUE;
}

void gluLookAt( float eyeX, float eyeY, float eyeZ, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ )
{
	// i am not using here proper implementation for vectors.
	// if you want, you can replace the arrays with your own
	// vector types
	float f[ 3 ];

	// calculating the viewing vector
	f[ 0 ] = lookAtX - eyeX;
	f[ 1 ] = lookAtY - eyeY;
	f[ 2 ] = lookAtZ - eyeZ;

	float fMag, upMag;
	fMag = sqrt( f[ 0 ] * f[ 0 ] + f[ 1 ] * f[ 1 ] + f[ 2 ] * f[ 2 ] );
	upMag = sqrt( upX*upX + upY*upY + upZ*upZ );

	// normalizing the viewing vector
	if( fMag != 0 )
	{
		f[ 0 ] = f[ 0 ] / fMag;
		f[ 1 ] = f[ 1 ] / fMag;
		f[ 2 ] = f[ 2 ] / fMag;
	}

	// normalising the up vector. no need for this here if you have your
	// up vector already normalised, which is mostly the case.
	if( upMag != 0 )
	{
		upX = upX / upMag;
		upY = upY / upMag;
		upZ = upZ / upMag;
	}

	float s[ 3 ], u[ 3 ];

	CrossProduct( Vector( f[ 0 ], f[ 1 ], f[ 2 ] ), Vector( upX, upY, upZ ), s );
	CrossProduct( Vector( s[ 0 ], s[ 1 ], s[ 2 ] ), Vector( f[ 0 ], f[ 1 ], f[ 2 ] ), u );

	float M[] =
	{
		s[ 0 ], u[ 0 ], -f[ 0 ], 0,
		s[ 1 ], u[ 1 ], -f[ 1 ], 0,
		s[ 2 ], u[ 2 ], -f[ 2 ], 0,
		0, 0, 0, 1
	};

	qglMultMatrixf( M );
	qglTranslatef( -eyeX, -eyeY, -eyeZ );
}

void QFXRenderer::PreRenderScene( struct refDef_s *fd )
{

}

void QFXRenderer :: RenderGL( void )
{
	DWORD t = timeGetTime();
	m_flFrameTime = (t - m_flOldTime) * 0.001f;
	m_flOldTime = t;

	if ( m_varCompareMode ) {
		if ( t - m_flLastTime > 2000 ) {
			m_flLastTime = t;

			m_iEnableRender ^= 1;

			if ( (m_varAnisotropy > 1) && (ext.maxAnisotropy > 1) && !m_bInternalAnisotropy ) {
				int value = m_iEnableRender ? min( ext.maxAnisotropy, m_varAnisotropy ) : 1;
				GLuint state2DBinding;
				qglGetIntegerv( GL_TEXTURE_BINDING_2D, (GLint*)(&state2DBinding) );
				for ( GLuint i = 1; i < 9999; i++ ) {
					if ( qglIsTexture( i ) ) {
						qglBindTexture( GL_TEXTURE_2D, i );
						qglTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value );
					}
				}
				qglBindTexture( GL_TEXTURE_2D, state2DBinding );
			}
			if ( m_bMultisample && !m_bInternalMultisample ) {
				if ( m_iEnableRender )
					qglEnable( GL_MULTISAMPLE_ARB );
				else
					qglDisable( GL_MULTISAMPLE_ARB );
			}
		}
	}

	if ( m_iEnableRender ) {
		if ( r_debug->integer )
			QFXLog::Instance().Printf( "%5i: Postprocess at %i\n", m_uiFrameCounter, m_uiRenderFrame );
		PostProcess();
	}
}

void QFXRenderer :: SetOrtho( GLboolean enable )
{
	if ( r_debug->integer )
		QFXLog::Instance().Printf("%5i|%5i|%5i: ortho = %s\n", m_uiFrameCounter, m_uiRenderFrame, m_uiRenderFrameCounter, enable ? "true" : "false" );

	if ( enable ) {
		if ( m_uiRenderFrameCounter != m_uiFrameCounter ) {
			m_uiRenderFrameCounter = m_uiFrameCounter;
			m_uiRenderFrame = 0;
		} else {
			m_uiRenderFrame++;
		}

		if ( m_uiRenderFrame == m_varRenderFrame )
			RenderGL();
	} else {
		if ( m_uiViewportFrame != m_uiFrameCounter ) {
			m_uiViewportFrame = m_uiFrameCounter;
			qglGetIntegerv( GL_VIEWPORT, m_3DViewport );
		}
	}

	m_bOrtho = enable;
}

void QFXRenderer :: CaptureRGB( void )
{
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiScreenRGB );
	qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[1] - m_3DViewport[3], m_3DViewport[2], m_3DViewport[3] );
}

void QFXRenderer :: CaptureDepth( void )
{
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiScreenDepth );
	qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[1] - m_3DViewport[3], m_3DViewport[2], m_3DViewport[3] );
}

void QFXRenderer :: RenderFSQ( int wide, int tall )
{
	float screenWidth = (float)wide;
	float screenHeight = (float)tall;

	qglBegin( GL_QUADS );
		qglTexCoord2f( 0, screenHeight );
		qglVertex2f( 0, 0 );
		qglTexCoord2f( screenWidth, screenHeight );
		qglVertex2f( screenWidth, 0 );
		qglTexCoord2f( screenWidth, 0 );
		qglVertex2f( screenWidth, screenHeight );
		qglTexCoord2f( 0, 0 );
		qglVertex2f( 0, screenHeight );
	qglEnd();
}

void QFXRenderer :: RenderScaledFSQ( int wide, int tall, int wide2, int tall2 )
{
	float screenWidth = (float)wide;
	float screenHeight = (float)tall;
	float screenWidth2 = (float)wide2;
	float screenHeight2 = (float)tall2;

	qglBegin( GL_QUADS );
		qglTexCoord2f( 0, screenHeight2 );
		qglVertex2f( 0, 0 );
		qglTexCoord2f( screenWidth2, screenHeight2 );
		qglVertex2f( screenWidth, 0 );
		qglTexCoord2f( screenWidth2, 0 );
		qglVertex2f( screenWidth, screenHeight );
		qglTexCoord2f( 0, 0 );
		qglVertex2f( 0, screenHeight );
	qglEnd();
}

void QFXRenderer :: PostProcess( void )
{
	GLboolean stateRectangleEnabled;
	GLboolean state2DEnabled;
	GLboolean stateBlendEnabled;
	GLboolean stateCullEnabled;
	GLboolean stateDepthTestEnabled;
	GLboolean stateScissorEnabled;
	GLuint stateRectangleBinding = 0;
	GLuint state2DBinding = 0;
	GLuint stateRectangleBindingMTex[3] = { 0, 0, 0 };
	GLenum stateSrcBlendFunc = GL_ONE;
	GLenum stateDstBlendFunc = GL_ZERO;
	GLenum stateMatrixMode = GL_MODELVIEW;
	GLenum stateActiveTexture = GL_TEXTURE0_ARB;
	GLenum stateTexEnv = GL_MODULATE;
	GLboolean stateDepthMask = GL_FALSE;
	GLfloat stateCurrentColor[4];
	GLhandleARB currentGLSLShader = 0;
	GLint stateViewport[4];

	// save current state
	state2DEnabled = qglIsEnabled( GL_TEXTURE_2D );
	qglGetIntegerv( GL_TEXTURE_BINDING_2D, (GLint*)(&state2DBinding) );

	if ( ext.RectangleTexture ) {
		stateRectangleEnabled = qglIsEnabled( GL_TEXTURE_RECTANGLE_ARB );
		qglGetIntegerv( GL_TEXTURE_BINDING_RECTANGLE_ARB, (GLint*)(&stateRectangleBinding) );
	}
	if ( ext.VertexShader || ext.FragmentShader ) {
		currentGLSLShader = qglGetHandleARB( GL_PROGRAM_OBJECT_ARB );
	}
	if ( ext.Multitexture ) {
		qglGetIntegerv( GL_ACTIVE_TEXTURE_ARB, (GLint*)(&stateActiveTexture) );
		qglActiveTextureARB( GL_TEXTURE0_ARB );
		if ( ext.RectangleTexture ) {
			qglActiveTextureARB( GL_TEXTURE1_ARB );
			qglGetIntegerv( GL_TEXTURE_BINDING_RECTANGLE_ARB, (GLint*)(&stateRectangleBindingMTex[1]) );
			qglActiveTextureARB( GL_TEXTURE2_ARB );
			qglGetIntegerv( GL_TEXTURE_BINDING_RECTANGLE_ARB, (GLint*)(&stateRectangleBindingMTex[2]) );
			qglActiveTextureARB( GL_TEXTURE0_ARB );
			qglGetIntegerv( GL_TEXTURE_BINDING_RECTANGLE_ARB, (GLint*)(&stateRectangleBindingMTex[0]) );
		}
	}

	stateBlendEnabled = qglIsEnabled( GL_BLEND );
	stateCullEnabled = qglIsEnabled( GL_CULL_FACE );
	stateDepthTestEnabled = qglIsEnabled( GL_DEPTH_TEST );
	stateScissorEnabled = qglIsEnabled( GL_SCISSOR_TEST );
	qglGetIntegerv( GL_BLEND_SRC, (GLint*)(&stateSrcBlendFunc) );
	qglGetIntegerv( GL_BLEND_DST, (GLint*)(&stateDstBlendFunc) );
	qglGetIntegerv( GL_MATRIX_MODE, (GLint*)(&stateMatrixMode) );
	qglGetBooleanv( GL_DEPTH_WRITEMASK, &stateDepthMask );
	qglGetFloatv( GL_CURRENT_COLOR, stateCurrentColor );
	if ( !m_varViewportFix ) qglGetIntegerv( GL_VIEWPORT, stateViewport );
	qglGetIntegerv( GL_SCISSOR_BOX, m_3DScissor );
	qglGetTexEnviv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, (GLint*)(&stateTexEnv) );

	// setup new state
	qglDisable( GL_CULL_FACE );
	qglDisable( GL_DEPTH_TEST );
	qglDisable( GL_BLEND );
	qglEnable( GL_TEXTURE_2D );
	qglEnable( GL_TEXTURE_RECTANGLE_ARB );
	qglEnable( GL_SCISSOR_TEST );
	qglDepthMask( GL_FALSE );
	qglMatrixMode( GL_PROJECTION );
	qglPushMatrix();
	qglLoadIdentity();
	qglOrtho( 0, m_WindowSize[0], m_WindowSize[1], 0, -99999, 99999 );
	qglMatrixMode( GL_MODELVIEW );
	qglPushMatrix();
	qglLoadIdentity();
	qglColor4f( 1, 1, 1, 1 );
	if ( !m_varViewportFix ) qglViewport( m_3DViewport[0], m_3DViewport[1], m_3DViewport[2], m_3DViewport[3] );
	qglScissor( m_3DViewport[0], m_3DViewport[1], m_3DViewport[2], m_3DViewport[3] );

	//PostProcess_Reflections();
	//PostProcess_Shadows();

	// render DOF
	if( m_bSupportsDOF && r_dof->integer ) {
		PostProcess_DOF();
	}

	// render SSAO
	if( m_bSupportsSSAO && r_ssao->integer ) {
		PostProcess_SSAO();
	}

	// render distortions
	if( r_distortionEnable.isEnabled() ) {
		PostProcess_Distortion();
	}

	// render Emboss
	if( m_bSupportsEmboss && m_varEmboss ) {
		PostProcess_Emboss();
	}

	//if( r_filmEnable.isEnabled() )
	//{
		PostProcess_ColorCorrection();
		PostProcess_Tint();
		//PostProcess_BrightnessContrast();
		PostProcess_Bleach();
	//}

	PostProcess_Blur();

	// render bloom effect
	if( m_bSupportsBloom && r_glowEnable->integer && r_glow.isEnabled() ) {
		PostProcess_Bloom();
	}

	if( r_fxaa->integer )
	{
		PostProcess_FXAA();
	}

	//PostProcess_MotionBlur();

	// restore current state
	qglMatrixMode( GL_PROJECTION );
	qglPopMatrix();
	qglMatrixMode( GL_MODELVIEW );
	qglPopMatrix();
	if ( ext.Multitexture ) {
		if ( ext.RectangleTexture ) {
			qglActiveTextureARB( GL_TEXTURE1_ARB );
			qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, stateRectangleBindingMTex[1] );
			qglActiveTextureARB( GL_TEXTURE2_ARB );
			qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, stateRectangleBindingMTex[2] );
			qglActiveTextureARB( GL_TEXTURE0_ARB );
			qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, stateRectangleBindingMTex[0] );
		}
		qglActiveTextureARB( stateActiveTexture );
	}
	if ( ext.RectangleTexture ) {
		qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, stateRectangleBinding );
		if ( !stateRectangleEnabled ) qglDisable( GL_TEXTURE_RECTANGLE_ARB );
	}
	qglBindTexture( GL_TEXTURE_2D, state2DBinding );
	if ( !state2DEnabled ) qglDisable( GL_TEXTURE_2D );
	qglBlendFunc( stateSrcBlendFunc, stateDstBlendFunc );
	qglColor4fv( stateCurrentColor );
	qglMatrixMode( stateMatrixMode );
	if ( stateBlendEnabled ) qglEnable( GL_BLEND ); else qglDisable( GL_BLEND );
	if ( stateCullEnabled ) qglEnable( GL_CULL_FACE );
	if ( stateDepthTestEnabled ) qglEnable( GL_DEPTH_TEST );
	if ( !stateScissorEnabled ) qglDisable( GL_SCISSOR_TEST );
	qglScissor( m_3DScissor[0], m_3DScissor[1], m_3DScissor[2], m_3DScissor[3] );
	if ( !m_varViewportFix ) qglViewport( stateViewport[0], stateViewport[1], stateViewport[2], stateViewport[3] );
	qglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, stateTexEnv );
	if ( ext.VertexShader || ext.FragmentShader ) qglUseProgramObjectARB( currentGLSLShader );
	qglDepthMask( stateDepthMask );
}

void QFXRenderer :: PostProcess_SSAO( void )
{
	int screenWidth = m_3DViewport[2];
	int screenHeight = m_3DViewport[3];
	int blurTexWidth = m_3DViewport[2]>>1;
	int blurTexHeight = m_3DViewport[3]>>1;

	// capture screen
	CaptureRGB();

	// capture depth
	CaptureDepth();

	// render raw SSAO
	m_pSSAORenderShader->Bind();
	m_pSSAORenderShader->SetParameter4f( 0, 2.0f * m_zNear, m_zFar - m_zNear, (float)screenWidth, (float)screenHeight );
	m_pSSAORenderShader->SetParameter4f( 1, m_varSSAOMinZ, m_varSSAOMaxZ, m_varSSAOScale, 0 );
	RenderScaledFSQ( blurTexWidth, blurTexHeight, screenWidth, screenHeight );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiBlurTexture );
	qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[1] - blurTexHeight, blurTexWidth, blurTexHeight );

	// blur SSAO
	m_pSSAOBlurShader->Bind();
	m_pSSAOBlurShader->SetParameter4f( 0, 1, 0, 0, 0 );
	qglActiveTextureARB( GL_TEXTURE1_ARB );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiScreenDepth );
	qglActiveTextureARB( GL_TEXTURE0_ARB );
	RenderFSQ( blurTexWidth, blurTexHeight );
	qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[1] - blurTexHeight, blurTexWidth, blurTexHeight );
	m_pSSAOBlurShader->SetParameter4f( 0, 0, 1, 0, 0 );
	RenderFSQ( blurTexWidth, blurTexHeight );
	qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[1] - blurTexHeight, blurTexWidth, blurTexHeight );
	
	// combine normal and AO scenes
	m_pSSAOCombineShader->Bind();
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiScreenRGB );
	qglActiveTextureARB( GL_TEXTURE1_ARB );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiBlurTexture );
	qglActiveTextureARB( GL_TEXTURE0_ARB );
	RenderFSQ( screenWidth, screenHeight );

	// unbind shader
	m_pSSAOCombineShader->Unbind();
}

void QFXRenderer :: PostProcess_Bloom( void )
{
	int screenWidth = m_3DViewport[2];
	int screenHeight = m_3DViewport[3];
	int blurTexWidth = m_3DViewport[2]>>1;
	int blurTexHeight = m_3DViewport[3]>>1;

	// capture screen
	CaptureRGB();

	// darken screen
	m_pBloomDarkenShader->Bind();
	m_pBloomDarkenShader->SetParameter4f( 0, ( r_glowBloomCutoff.floatValue() + 1.5f ) * 2.0f, 0, 0, 0 );
	RenderScaledFSQ( blurTexWidth, blurTexHeight, screenWidth, screenHeight );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiBlurTexture );
	qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[1] - blurTexHeight, blurTexWidth, blurTexHeight );

	// blur darkened texture
	m_pBloomBlurShader->Bind();

	if( r_glowQuality->integer < 1 ) {
		r_glowQuality->integer = 1;
	}

	if( r_glowSamples->integer < 1 ) {
		r_glowSamples->integer = 1;
	}

	if( r_glowSamples->integer > 15 ) {
		r_glowSamples->integer = 15;
	}

	float samples = ( float )r_glowSamples->integer * 2;

	for( int i = 0; i < r_glowQuality->integer; i++ )
	{
		m_pBloomBlurShader->SetParameter4f( 0, r_glowRadius0.floatValue(), 0, 0, 0 );
		RenderScaledFSQ( blurTexWidth, blurTexHeight, ( int )( ( float )blurTexWidth * ( 1.0f + ( 0.004f * r_glowBloomStreakX.floatValue() ) ) ), blurTexHeight );
		qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[1] - blurTexHeight, blurTexWidth, blurTexHeight );

		m_pBloomBlurShader->SetParameter4f( 0, 0, r_glowRadius1.floatValue(), 0, 0 );
		RenderFSQ( blurTexWidth, ( int )( ( float )blurTexHeight * ( 1.0f + ( 0.004f * r_glowBloomStreakY.floatValue() ) ) ) );
		qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[1] - blurTexHeight, blurTexWidth, blurTexHeight );
	}

	// combine normal and blurred scenes
	m_pBloomCombineShader->Bind();
	m_pBloomCombineShader->SetParameter4f( 0, r_glowBloomIntensity0.floatValue(), r_glowBloomIntensity1.floatValue(), 0, 0 );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiScreenRGB );
	qglActiveTextureARB( GL_TEXTURE1_ARB );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiBlurTexture );
	qglActiveTextureARB( GL_TEXTURE0_ARB );
	RenderFSQ( screenWidth, screenHeight );

	// unbind shader
	m_pBloomCombineShader->Unbind();
}

void QFXRenderer :: PostProcess_ColorCorrection( void )
{
	int screenWidth = m_3DViewport[2];
	int screenHeight = m_3DViewport[3];

	// capture screen
	CaptureRGB();

	// perform correction
	m_pHSLShader->Bind();
	m_pHSLShader->SetParameter4f( 0, 360.0f - r_filmHue.vectorValue()[ 0 ], r_filmSaturation.vectorValue()[ 0 ], 0, 0 );
	m_pHSLShader->SetParameter4f( 1, 360.0f - r_filmHue.vectorValue()[ 1 ], r_filmSaturation.vectorValue()[ 1 ], 0, 0 );
	m_pHSLShader->SetParameter4f( 2, 360.0f - r_filmHue.vectorValue()[ 2 ], r_filmSaturation.vectorValue()[ 2 ], 0, 0 );
	m_pHSLShader->SetParameter4f( 3, r_filmMidStart.floatValue(), r_filmMidEnd.floatValue(), 0, 0 );
	RenderFSQ( screenWidth, screenHeight );

	// unbind shader
	m_pHSLShader->Unbind();

}

void QFXRenderer :: PostProcess_Emboss( void )
{
	int screenWidth = m_3DViewport[2];
	int screenHeight = m_3DViewport[3];

	// capture screen
	CaptureRGB();

	// perform correction
	m_pEmbossShader->Bind();
	m_pEmbossShader->SetParameter4f( 0, m_varEmbossScale, 0, 0, 0 );
	RenderFSQ( screenWidth, screenHeight );

	// unbind shader
	m_pEmbossShader->Unbind();
}

void QFXRenderer :: PostProcess_DOF( void )
{
	int screenWidth = m_3DViewport[ 2 ];
	int screenHeight = m_3DViewport[ 3 ];
	int blurTexWidth = m_3DViewport[ 2 ] >> 1;
	int blurTexHeight = m_3DViewport[ 3 ] >> 1;
	float depthValue = 0.0f;

	// get current depth value
	qglReadPixels( blurTexWidth, blurTexHeight, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthValue );
	depthValue = ( 2.0f * m_zNear ) / ( m_zFar + m_zNear - depthValue * ( m_zFar - m_zNear ) );
	if( m_flLastDepth < 0 ) {
		m_flLastDepth = depthValue;
	} else if( m_flLastDepth > depthValue ) {
		m_flLastDepth -= min( m_flLastDepth - depthValue, m_varDOFSpeed * m_flFrameTime );
	} else if( m_flLastDepth < depthValue ) {
		m_flLastDepth += min( depthValue - m_flLastDepth, m_varDOFSpeed * m_flFrameTime );
	}

	// capture screen
	CaptureRGB();

	// blur screen
	m_pDOFBlurShader->Bind();
	m_pDOFBlurShader->SetParameter4f( 0, 1, 0, 0, 0 );
	RenderScaledFSQ( blurTexWidth, blurTexHeight, screenWidth, screenHeight );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiBlurTexture );
	qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[ 1 ] - blurTexHeight, blurTexWidth, blurTexHeight );
	m_pDOFBlurShader->SetParameter4f( 0, 0, 1, 0, 0 );
	RenderFSQ( blurTexWidth, blurTexHeight );
	qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[ 1 ] - blurTexHeight, blurTexWidth, blurTexHeight );

	// blur screen more
	for( int i = 0; i < 3; i++ ) {
		m_pDOFBlurShader->SetParameter4f( 0, 1, 0, 0, 0 );
		RenderFSQ( blurTexWidth, blurTexHeight );
		qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[ 1 ] - blurTexHeight, blurTexWidth, blurTexHeight );
		m_pDOFBlurShader->SetParameter4f( 0, 0, 1, 0, 0 );
		RenderFSQ( blurTexWidth, blurTexHeight );
		qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[ 1 ] - blurTexHeight, blurTexWidth, blurTexHeight );
	}

	// capture depth, if not captured previously in SSAO
	if( m_bSupportsSSAO && r_ssao->integer )
		qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiScreenDepth );
	else
		CaptureDepth();

	// linearize depth
	m_pDOFDepthShader->Bind();
	m_pDOFDepthShader->SetParameter4f( 0, 2.0f * m_zNear, m_zFar - m_zNear, 0, 0 );
	RenderScaledFSQ( blurTexWidth, blurTexHeight, screenWidth, screenHeight );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiBlurDepth );
	qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[ 1 ] - blurTexHeight, blurTexWidth, blurTexHeight );

	// blur depth
	m_pDOFBlurShader->Bind();
	for( int i = 0; i < 4; i++ ) {
		m_pDOFBlurShader->SetParameter4f( 0, 1, 0, 0, 0 );
		RenderFSQ( blurTexWidth, blurTexHeight );
		qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[ 1 ] - blurTexHeight, blurTexWidth, blurTexHeight );
		m_pDOFBlurShader->SetParameter4f( 0, 0, 1, 0, 0 );
		RenderFSQ( blurTexWidth, blurTexHeight );
		qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[ 1 ] - blurTexHeight, blurTexWidth, blurTexHeight );
	}

	// combine normal and blurred scenes
	m_pDOFRenderShader->Bind();
	m_pDOFRenderShader->SetParameter4f( 0, m_varDOFMinZ, m_flLastDepth, m_varDOFScale, 0 );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiScreenRGB );
	qglActiveTextureARB( GL_TEXTURE1_ARB );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiBlurTexture );
	qglActiveTextureARB( GL_TEXTURE2_ARB );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiBlurDepth );
	qglActiveTextureARB( GL_TEXTURE0_ARB );
	RenderFSQ( screenWidth, screenHeight );

	// unbind shader
	m_pDOFRenderShader->Unbind();
}

void QFXRenderer :: PostProcess_BrightnessContrast( void )
{
	int screenWidth = m_3DViewport[2];
	int screenHeight = m_3DViewport[3];

	// capture screen
	CaptureRGB();

	// perform brightness and contrast
	m_pBrightnessContrastShader->Bind();
	m_pBrightnessContrastShader->SetParameter4f( 0, r_filmBrightness.vectorValue()[ 0 ], r_filmBrightness.vectorValue()[ 1 ], r_filmBrightness.vectorValue()[ 2 ], r_filmContrast.floatValue() );
	m_pBrightnessContrastShader->SetParameter4f( 3, r_filmMidStart.floatValue(), r_filmMidEnd.floatValue(), 0, 0 );
	RenderFSQ( screenWidth, screenHeight );

	// unbind shader
	m_pBrightnessContrastShader->Unbind();
}

#define AVG_SIZE_WIDTH		128
#define AVG_SIZE_HEIGHT		128

void QFXRenderer :: PostProcess_Tint( void )
{
	int screenWidth = m_3DViewport[2];
	int screenHeight = m_3DViewport[3];
	Vector light, mid, dark;
	Vector brightness;
	GLubyte *data = ( GLubyte * )malloc( AVG_SIZE_WIDTH * AVG_SIZE_HEIGHT );
	float avg;

	light = r_filmLightTint.vectorValue();
	mid = r_filmMidTint.vectorValue();
	dark = r_filmDarkTint.vectorValue();
	brightness = r_filmBrightness.vectorValue();

	avg = 0.5f;

	if( r_autoluminance->integer )
	{
		qglReadPixels( 0, ( screenHeight >> 1 ) - AVG_SIZE_HEIGHT / 2, AVG_SIZE_WIDTH, AVG_SIZE_HEIGHT, GL_LUMINANCE, GL_UNSIGNED_BYTE, data );

		for( int i = 0; i < AVG_SIZE_WIDTH * AVG_SIZE_HEIGHT; i++ )
		{
			avg += ( float )data[ i ] / 255.0f;
		}

		qglReadPixels( screenWidth - AVG_SIZE_WIDTH, ( screenHeight >> 1 ) - AVG_SIZE_HEIGHT / 2, AVG_SIZE_WIDTH, AVG_SIZE_HEIGHT, GL_LUMINANCE, GL_UNSIGNED_BYTE, data );

		for( int i = 0; i < AVG_SIZE_WIDTH * AVG_SIZE_HEIGHT; i++ )
		{
			avg += ( float )data[ i ] / 255.0f;
		}

		avg /= AVG_SIZE_WIDTH * AVG_SIZE_HEIGHT * 2;

		if( avg <= 0.01f ) {
			avg = 0.01f;
		}
	}

	player.m_fExposure = 0.5f/avg;
	player.m_fExposure = player.m_fExposurePrevious + ( player.m_fExposure - player.m_fExposurePrevious ) * ( 0.002 * cg->frametime );

	player.m_fExposurePrevious = player.m_fExposure;

	float light_r = light[ 0 ];
	float light_g = light[ 1 ];
	float light_b = light[ 2 ];

	float mid_r = mid[ 0 ];
	float mid_g = mid[ 1 ];
	float mid_b = mid[ 2 ];

	float dark_r = dark[ 0 ];
	float dark_g = dark[ 1 ];
	float dark_b = dark[ 2 ];

	free( data );

	CaptureRGB();

	// perform light tint
	m_pTintShader->Bind();
	m_pTintShader->SetParameter4f( 0, light_r, light_g, light_b, 0 );
	m_pTintShader->SetParameter4f( 1, mid_r, mid_g, mid_b, 0 );
	m_pTintShader->SetParameter4f( 2, dark_r, dark_g, dark_b, 0 );
	m_pTintShader->SetParameter4f( 3, r_filmMidStart.floatValue(), r_filmMidEnd.floatValue(), 0, 0 );
	m_pTintShader->SetParameter4f( 4, brightness[ 0 ], brightness[ 1 ], brightness[ 2 ], r_filmContrast.floatValue() );
	m_pTintShader->SetParameter4f( 5, player.m_fExposure, 0, 0, 0 );
	RenderFSQ( screenWidth, screenHeight );

	// unbind shader
	m_pTintShader->Unbind();
}

void QFXRenderer :: PostProcess_Bleach( void )
{
	int screenWidth = m_3DViewport[2];
	int screenHeight = m_3DViewport[3];

	// Optimize performances

	// capture screen
	CaptureRGB();

	// perform invert
	m_pBleachShader->Bind();
	m_pBleachShader->SetParameter4f( 0, r_filmBleach.vectorValue()[ 0 ], r_filmBleach.vectorValue()[ 1 ], r_filmBleach.vectorValue()[ 2 ], 0 );
	m_pBleachShader->SetParameter4f( 3, r_filmMidStart.floatValue(), r_filmMidEnd.floatValue(), 0, 0 );
	RenderFSQ( screenWidth, screenHeight );

	// unbind shader
	m_pBleachShader->Unbind();
}

#if 0
void QFXRenderer :: PostProcess_MotionBlur( void )
{
	int screenWidth = m_3DViewport[2];
	int screenHeight = m_3DViewport[3];
	int blurTexWidth = m_3DViewport[2]>>1;
	int blurTexHeight = m_3DViewport[3]>>1;

	// capture screen
	CaptureRGB();

	// perform invert
	m_pMotionBlurShader->Bind();
	m_pMotionBlurShader->SetParameter4f( 0, 0, 0, 0, 0 );
	RenderFSQ( screenWidth, screenHeight );

	// unbind shader
	m_pMotionBlurShader->Unbind();
}

void QFXRenderer :: PostProcess_Reflections( void )
{
	int screenWidth = m_3DViewport[2];
	int screenHeight = m_3DViewport[3];

	//qglEnable( GL_LIGHTING );

	// capture screen
	CaptureRGB();

	// perform invert
	m_pLightReflectShader->Bind();
	RenderFSQ( screenWidth, screenHeight );

	// unbind shader
	m_pBloomCombineShader->Unbind();

	//qglDisable( GL_LIGHTING );
}

void setTextureMatrix(void)
{
	static double modelView[16];
	static double projection[16];

	// This is matrix transform every coordinate x,y,z
	// x = x* 0.5 + 0.5
	// y = y* 0.5 + 0.5
	// z = z* 0.5 + 0.5
	// Moving from unit cube [-1,1] to [0,1]
	const GLdouble bias[16] = {
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0};

	// Grab modelview and transformation matrices
	qglGetDoublev(GL_MODELVIEW_MATRIX, modelView);
	qglGetDoublev(GL_PROJECTION_MATRIX, projection);


	qglMatrixMode(GL_TEXTURE);
	qglActiveTextureARB(GL_TEXTURE7);

	qglLoadIdentity();
	qglLoadMatrixd(bias);

	// concatating all matrice into one.
	qglMultMatrixd (projection);
	qglMultMatrixd (modelView);

	// Go back to normal matrix mode
	qglMatrixMode(GL_MODELVIEW);
}

void setupMatrices( float position_x, float position_y, float position_z, float lookAt_x, float lookAt_y, float lookAt_z )
{
	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity();
	//qgluPerspective( 45, screenWidth/screenHeight, 10, 40000 );
	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();
	//qgluLookAt(position_x,position_y,position_z,lookAt_x,lookAt_y,lookAt_z,0,1,0);
}

//Camera position
float p_camera[3] = {32,20,0};

//Camera lookAt
float l_camera[3] = {2,0,-10};

//Light position
float p_light[3] = {3,20,0};

//Light lookAt
float l_light[3] = {0,0,-5};

void QFXRenderer :: PostProcess_Shadows( void )
{
	int screenWidth = m_3DViewport[2];
	int screenHeight = m_3DViewport[3];
	vec3_t lightPos[128], lightIntensity[128];

	//First step: Render from the light POV to a FBO, story depth values only
	qglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_Framebuffer );	//Rendering offscreen

	//Using the fixed pipeline to render to the depthbuffer
	qglUseProgramObjectARB( 0 );

	// In the case we render the shadowmap to a higher resolution, the viewport must be modified accordingly.
	qglViewport( 0, 0, screenWidth * 1, screenHeight * 1 );

	// Clear previous frame values
	qglClear( GL_DEPTH_BUFFER_BIT );

	//Disable color rendering, we only want to write to the Z-Buffer
	qglColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

	setupMatrices( p_light[0], p_light[1], p_light[2], l_light[0], l_light[1], l_light[2] );

	cgi.R_GatherLightSources( cent->currentState.origin, lightPos, lightIntensity, 128 );

	// Culling switching, rendering only backface, this is done to avoid self-shadowing
	qglCullFace( GL_FRONT );

	//Save modelview/projection matrice into texture7, also add a biais
	setTextureMatrix();

	// Now rendering from the camera POV, using the FBO to generate shadows
	qglBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

	qglViewport( 0, 0, screenWidth, screenHeight );

	//Enabling color write (previously disabled for light POV z-buffer rendering)
	qglColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	// Clear previous frame values
	qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	CaptureDepth();

	//Using the shadow shader
	m_pShadowShader->Bind();
	m_pShadowShader->SetParameter4f( 0, 0, 0, 0, 0 ); //qglUniform1iARB( shadowMapUniform, 7 );
	qglActiveTextureARB( GL_TEXTURE7 );
	qglBindTexture( GL_TEXTURE_2D, m_uiScreenDepth );
	RenderFSQ( screenWidth, screenHeight );

	m_pShadowShader->Unbind();

	setupMatrices( p_camera[0], p_camera[1], p_camera[2], l_camera[0], l_camera[1], l_camera[2] );

	wglSwapBuffers( NULL );

	/*int screenWidth = m_3DViewport[2];
	int screenHeight = m_3DViewport[3];

	// capture screen
	CaptureDepth();

	// perform invert
	m_pShadowShader->Bind();
	m_pShadowShader->SetParameter4f( 0, 0, 0, 0, 0 );
	RenderFSQ( screenWidth, screenHeight );

	// unbind shader
	m_pShadowShader->Unbind();*/
}
#endif

void QFXRenderer::PostProcess_Distortion( void )
{
	int screenWidth = m_3DViewport[ 2 ];
	int screenHeight = m_3DViewport[ 3 ];

	CaptureRGB();

	m_pDistortionShader->Bind();
	m_pDistortionShader->SetParameter4f( 0, ( float )( screenWidth >> 1 ), ( float )( screenHeight >> 1 ), r_distortionRadius.floatValue(), r_distortionScale.floatValue() );
	RenderFSQ( screenWidth, screenHeight );

	// unbind shader
	m_pDistortionShader->Unbind();
}

void QFXRenderer::PostProcess_Blur( void )
{
	int screenWidth = m_3DViewport[ 2 ];
	int screenHeight = m_3DViewport[ 3 ];
	int blurTexWidth = m_3DViewport[ 2 ] >> 1;
	int blurTexHeight = m_3DViewport[ 3 ] >> 1;

	if( r_tweakBlurPasses->integer <= 0 ) {
		return;
	}

	float passes = ( float )r_tweakBlurPasses->integer;

	float velX = player.avel[ 1 ] * ( 15.0f / cg->frametime );
	float velY = player.avel[ 0 ] * ( 15.0f / cg->frametime );

	float blurX = velX;
	float blurY = velY;

	/*if( blurX <= 0.0f ) {
		blurX = 0.01f;
	}

	if( blurY <= 0.0f ) {
		blurY = 0.01f;
	}*/

	for( int i = 0; i < r_tweakBlurPasses->integer; i++ )
	{
		CaptureRGB();

		m_pBlurShader->Bind();
		m_pBlurShader->SetParameter4f( 0, blurX, 0, 0, 0 );
		RenderFSQ( screenWidth, screenHeight );

		m_pBlurShader->Unbind();

		CaptureRGB();

		m_pBlurShader->Bind();
		m_pBlurShader->SetParameter4f( 0, 0, blurY, 0, 0 );
		RenderFSQ( screenWidth, screenHeight );
	}

	// unbind shader
	m_pBlurShader->Unbind();

#if 0

	CaptureRGB();

	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiBlurTexture );
	qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[ 1 ] - blurTexHeight, blurTexWidth, blurTexHeight );

	for( int i = 0; i < r_tweakBlurPasses->integer; i++ )
	{
		m_pBlurShader->Bind();
		m_pBlurShader->SetParameter4f( 0, r_tweakBlurX->value * r_tweakBlurPasses->integer, 0, 0, 0 );
		RenderFSQ( blurTexWidth, blurTexHeight );
		qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[ 1 ] - blurTexHeight, blurTexWidth, blurTexHeight );

		m_pBlurShader->Bind();
		m_pBlurShader->SetParameter4f( 0, 0, r_tweakBlurY->value * r_tweakBlurPasses->integer, 0, 0 );
		RenderFSQ( blurTexWidth, blurTexHeight );
		qglCopyTexSubImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, m_WindowSize[ 1 ] - blurTexHeight, blurTexWidth, blurTexHeight );
	}

	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiScreenRGB );
	qglActiveTextureARB( GL_TEXTURE1_ARB );
	qglBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_uiBlurTexture );
	qglActiveTextureARB( GL_TEXTURE0_ARB );

	RenderFSQ( screenWidth, screenHeight );

	// unbind shader
	m_pBlurShader->Unbind();
#endif
}


void QFXRenderer::PostProcess_FXAA( void )
{
	int screenWidth = m_3DViewport[ 2 ];
	int screenHeight = m_3DViewport[ 3 ];

	CaptureRGB();

	m_pFXAAShader->Bind();
	m_pFXAAShader->SetParameter4f( 0, 1.0f, 1.0f, 0, 0 );
	RenderFSQ( screenWidth, screenHeight );

	// unbind shader
	m_pFXAAShader->Unbind();
}
