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
#ifndef QFX_RENDERER_H
#define QFX_RENDERER_H

class QFXShader;

QFX_SINGLETON class QFXRenderer
{
private:
	QFXRenderer();
	static QFXRenderer& GetInstance() { static QFXRenderer rObject; return rObject; }

public:
	static QFXRenderer& Instance() { typedef QFXRenderer& (*pfnGetInstance)(); static pfnGetInstance pf = &GetInstance; return pf(); }

	void GetVariables( void );
	void InitializeGL( void );
	BOOL OnSetPixelFormat( HDC hdc, int pixelformat, CONST PIXELFORMATDESCRIPTOR *ppfd );
	void OnMakeCurrent( HDC hdc, HGLRC hglrc );
	void OnDeleteContext( HGLRC hglrc );
	void OnSwapBuffers( void );
	void OnTexImage2D( void );
	void PostProcess( void );
	void RenderGL( void );
	void SetOrtho( GLboolean enable );
	void SetupZDimensions( GLfloat znear, GLfloat zfar );
	void SetupZDimensions( const GLfloat* matrix );
	void SetupZDimensions( const GLdouble* matrix );
	void SetupDepthFunc( GLenum func );
	void SetupDepthRange( GLclampd zNear, GLclampd zFar );
	void SetupMultisample( void );
	void SetupAnisotropy( void );
	void ProcessShadow1( void );
	void ProcessShadow2( void );
	void PreRenderScene( struct refDef_s *fd );
	void PostRenderScene( void );

private:
	GLuint AllocTextureId( void ) { return m_uiTextureIndex++; }
	int  ChooseMultisamplePixelFormat( int fmt, HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd );
	void CheckHardware( void );
	void ShutdownGL( void );
	void CaptureRGB( void );
	void CaptureDepth( void );
	void RenderFSQ( int wide, int tall );
	void RenderScaledFSQ( int wide, int tall, int wide2, int tall2 );

	//void PostProcess( void );
	void PostProcess_SSAO( void );
	void PostProcess_Bloom( void );
	void PostProcess_ColorCorrection( void );
	void PostProcess_Emboss( void );
	void PostProcess_DOF( void );
	void PostProcess_BrightnessContrast( void );
	void PostProcess_Tint( void );
	void PostProcess_Bleach( void );
	void PostProcess_MotionBlur( void );
	void PostProcess_Reflections( void );
	void PostProcess_Shadows( void );
	void PostProcess_FXAA( void );
	void PostProcess_Distortion( void );
	void PostProcess_Blur( void );

private:
	// contexts, internal state
	HDC				m_hDC;
	HGLRC			m_hGLRC;
	GLboolean		m_bOrtho;
	GLboolean		m_bZTrick;
	GLboolean		m_bInternalAnisotropy;
	GLboolean		m_bInternalMultisample;
	GLboolean		m_bMultisample;
	GLuint			m_uiRenderFrame;
	GLuint			m_uiRenderFrameCounter;
	GLuint			m_uiFrameCounter;
	GLuint			m_uiTextureIndex;
	GLint			m_WindowSize[2];
	GLfloat			m_zNear;
	GLfloat			m_zFar;
	DWORD			m_iEnableRender;
	GLfloat			m_flFrameTime;
	DWORD			m_flOldTime;
	DWORD			m_flLastTime;
	GLfloat			m_flLastDepth;
	GLuint			m_uiFrameBuffer;

	// tracked GL state
	GLuint			m_uiViewportFrame;
	GLint			m_3DViewport[4];
	GLint			m_3DScissor[4];

	// hardware caps
	GLboolean		m_bSupportsBloom;
	GLboolean		m_bSupportsSSAO;
	GLboolean		m_bSupportsColorCorrection;
	GLboolean		m_bSupportsEmboss;
	GLboolean		m_bSupportsDOF;

	// renderer settings
	int				m_varDebug;
	int				m_varCompareMode;
	int				m_varZTrickFix;
	int				m_varViewportFix;
	int				m_varRenderFrame;
	int				m_varAnisotropy;
	int				m_varMultisample;
	int				m_varMultisampleHint;
	int				m_varBloomEnable;
	int				m_varBloomNumSteps;
	float			m_varBloomDarkenPower;
	float			m_varBloomRadius0;
	float			m_varBloomRadius1;
	float			m_varBloomCombineScale0;
	float			m_varBloomCombineScale1;
	int				m_varSSAOEnable;
	float			m_varSSAOScale;
	float			m_varSSAOMinZ;
	float			m_varSSAOMaxZ;
	int				m_varHue;
	int				m_varSaturation;
	int				m_varLightness;
	int				m_varEmboss;
	float			m_varEmbossScale;
	int				m_varDOF;
	float			m_varDOFSpeed;
	float			m_varDOFScale;
	float			m_varDOFMinZ;

public:
	// renderer data
	GLuint			m_uiScreenRGB;
	GLuint			m_uiScreenDepth;
	GLuint			m_uiBlurTexture;
	GLuint			m_uiBlurDepth;
	GLuint			m_uiShadowDepth;
	QFXShader*		m_pBloomDarkenShader;
	QFXShader*		m_pBloomBlurShader;
	QFXShader*		m_pBloomBlurVertShader;
	QFXShader*		m_pBloomCombineShader;
	QFXShader*		m_pSSAORenderShader;
	QFXShader*		m_pSSAOBlurShader;
	QFXShader*		m_pSSAOCombineShader;
	QFXShader*		m_pHSLShader;
	QFXShader*		m_pEmbossShader;
	QFXShader*		m_pDOFBlurShader;
	QFXShader*		m_pDOFDepthShader;
	QFXShader*		m_pDOFRenderShader;
	QFXShader*		m_pBrightnessContrastShader;
	QFXShader*		m_pTintShader;
	QFXShader*		m_pLightTintShader;
	QFXShader*		m_pMidTintShader;
	QFXShader*		m_pDarkTintShader;
	QFXShader*		m_pBleachShader;
	QFXShader*		m_pDistortionShader;
	QFXShader*		m_pBlurShader;
	QFXShader*		m_pFXAAShader;
	QFXShader*		m_pDisplacement;
	QFXShader*		m_pShadowMapping;
};

#endif //QFX_RENDERER_H
