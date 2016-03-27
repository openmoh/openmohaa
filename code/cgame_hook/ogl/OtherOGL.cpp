/*
* Game-Deception Blank Wrapper v2
* Copyright (c) Crusader 2002
*/

/*
* Functions not mainly used by half-life, or functions that would allow limited to no
* advantage by hooking.

* Todo? : These functions should be naked to save on time/space
*/

#include "opengl_api.h"

 HMODULE hOpenGL = 0;
 func_glActiveTextureARB_t orig_glActiveTextureARB;
 func_BindTextureEXT_t orig_BindTextureEXT;
 func_glMultiTexCoord2fARB_t orig_glMultiTexCoord2fARB;
 func_glAccum_t	orig_glAccum;
 func_glAlphaFunc_t	orig_glAlphaFunc;
 func_glAreTexturesResident_t	orig_glAreTexturesResident;
 func_glArrayElement_t	orig_glArrayElement;
 func_glBegin_t	orig_glBegin;
 func_glBindTexture_t	orig_glBindTexture;
 func_glBitmap_t	orig_glBitmap;
 func_glBlendFunc_t	orig_glBlendFunc;
 func_glCallList_t	orig_glCallList;
 func_glCallLists_t	orig_glCallLists;
 func_glClear_t	orig_glClear;
 func_glClearAccum_t	orig_glClearAccum;
 func_glClearColor_t	orig_glClearColor;
 func_glClearDepth_t	orig_glClearDepth;
 func_glClearIndex_t	orig_glClearIndex;
 func_glClearStencil_t	orig_glClearStencil;
 func_glClipPlane_t	orig_glClipPlane;
 func_glColor3b_t	orig_glColor3b;
 func_glColor3bv_t	orig_glColor3bv;
 func_glColor3d_t	orig_glColor3d;
 func_glColor3dv_t	orig_glColor3dv;
 func_glColor3f_t	orig_glColor3f;
 func_glColor3fv_t	orig_glColor3fv;
 func_glColor3i_t	orig_glColor3i;
 func_glColor3iv_t	orig_glColor3iv;
 func_glColor3s_t	orig_glColor3s;
 func_glColor3sv_t	orig_glColor3sv;
 func_glColor3ub_t	orig_glColor3ub;
 func_glColor3ubv_t	orig_glColor3ubv;
 func_glColor3ui_t	orig_glColor3ui;
 func_glColor3uiv_t	orig_glColor3uiv;
 func_glColor3us_t	orig_glColor3us;
 func_glColor3usv_t	orig_glColor3usv;
 func_glColor4b_t	orig_glColor4b;
 func_glColor4bv_t	orig_glColor4bv;
 func_glColor4d_t	orig_glColor4d;
 func_glColor4dv_t	orig_glColor4dv;
 func_glColor4f_t	orig_glColor4f;
 func_glColor4fv_t	orig_glColor4fv;
 func_glColor4i_t	orig_glColor4i;
 func_glColor4iv_t	orig_glColor4iv;
 func_glColor4s_t	orig_glColor4s;
 func_glColor4sv_t	orig_glColor4sv;
 func_glColor4ub_t	orig_glColor4ub;
 func_glColor4ubv_t	orig_glColor4ubv;
 func_glColor4ui_t	orig_glColor4ui;
 func_glColor4uiv_t	orig_glColor4uiv;
 func_glColor4us_t	orig_glColor4us;
 func_glColor4usv_t	orig_glColor4usv;
 func_glColorMask_t	orig_glColorMask;
 func_glColorMaterial_t	orig_glColorMaterial;
 func_glColorPointer_t	orig_glColorPointer;
 func_glCopyPixels_t	orig_glCopyPixels;
 func_glCopyTexImage1D_t	orig_glCopyTexImage1D;
 func_glCopyTexImage2D_t	orig_glCopyTexImage2D;
 func_glCopyTexSubImage1D_t	orig_glCopyTexSubImage1D;
 func_glCopyTexSubImage2D_t	orig_glCopyTexSubImage2D;
 func_glCullFace_t	orig_glCullFace;
 func_glDebugEntry_t	orig_glDebugEntry;
 func_glDeleteLists_t	orig_glDeleteLists;
 func_glDeleteTextures_t	orig_glDeleteTextures;
 func_glDepthFunc_t	orig_glDepthFunc;
 func_glDepthMask_t	orig_glDepthMask;
 func_glDepthRange_t	orig_glDepthRange;
 func_glDisable_t	orig_glDisable;
 func_glDisableClientState_t	orig_glDisableClientState;
 func_glDrawArrays_t	orig_glDrawArrays;
 func_glDrawBuffer_t	orig_glDrawBuffer;
 func_glDrawElements_t	orig_glDrawElements;
 func_glDrawPixels_t	orig_glDrawPixels;
 func_glEdgeFlag_t	orig_glEdgeFlag;
 func_glEdgeFlagPointer_t	orig_glEdgeFlagPointer;
 func_glEdgeFlagv_t	orig_glEdgeFlagv;
 func_glEnable_t	orig_glEnable;
 func_glEnableClientState_t	orig_glEnableClientState;
 func_glEnd_t	orig_glEnd;
 func_glEndList_t	orig_glEndList;
 func_glEvalCoord1d_t	orig_glEvalCoord1d;
 func_glEvalCoord1dv_t	orig_glEvalCoord1dv;
 func_glEvalCoord1f_t	orig_glEvalCoord1f;
 func_glEvalCoord1fv_t	orig_glEvalCoord1fv;
 func_glEvalCoord2d_t	orig_glEvalCoord2d;
 func_glEvalCoord2dv_t	orig_glEvalCoord2dv;
 func_glEvalCoord2f_t	orig_glEvalCoord2f;
 func_glEvalCoord2fv_t	orig_glEvalCoord2fv;
 func_glEvalMesh1_t	orig_glEvalMesh1;
 func_glEvalMesh2_t	orig_glEvalMesh2;
 func_glEvalPoint1_t	orig_glEvalPoint1;
 func_glEvalPoint2_t	orig_glEvalPoint2;
 func_glFeedbackBuffer_t	orig_glFeedbackBuffer;
 func_glFinish_t	orig_glFinish;
 func_glFlush_t	orig_glFlush;
 func_glFogf_t	orig_glFogf;
 func_glFogfv_t	orig_glFogfv;
 func_glFogi_t	orig_glFogi;
 func_glFogiv_t	orig_glFogiv;
 func_glFrontFace_t	orig_glFrontFace;
 func_glFrustum_t	orig_glFrustum;
 func_glGenLists_t	orig_glGenLists;
 func_glGenTextures_t	orig_glGenTextures;
 func_glGetBooleanv_t	orig_glGetBooleanv;
 func_glGetClipPlane_t	orig_glGetClipPlane;
 func_glGetDoublev_t	orig_glGetDoublev;
 func_glGetError_t	orig_glGetError;
 func_glGetFloatv_t	orig_glGetFloatv;
 func_glGetIntegerv_t	orig_glGetIntegerv;
 func_glGetLightfv_t	orig_glGetLightfv;
 func_glGetLightiv_t	orig_glGetLightiv;
 func_glGetMapdv_t	orig_glGetMapdv;
 func_glGetMapfv_t	orig_glGetMapfv;
 func_glGetMapiv_t	orig_glGetMapiv;
 func_glGetMaterialfv_t	orig_glGetMaterialfv;
 func_glGetMaterialiv_t	orig_glGetMaterialiv;
 func_glGetPixelMapfv_t	orig_glGetPixelMapfv;
 func_glGetPixelMapuiv_t	orig_glGetPixelMapuiv;
 func_glGetPixelMapusv_t	orig_glGetPixelMapusv;
 func_glGetPointerv_t	orig_glGetPointerv;
 func_glGetPolygonStipple_t	orig_glGetPolygonStipple;
 func_glGetString_t	orig_glGetString;
 func_glGetTexEnvfv_t	orig_glGetTexEnvfv;
 func_glGetTexEnviv_t	orig_glGetTexEnviv;
 func_glGetTexGendv_t	orig_glGetTexGendv;
 func_glGetTexGenfv_t	orig_glGetTexGenfv;
 func_glGetTexGeniv_t	orig_glGetTexGeniv;
 func_glGetTexImage_t	orig_glGetTexImage;
 func_glGetTexLevelParameterfv_t	orig_glGetTexLevelParameterfv;
 func_glGetTexLevelParameteriv_t	orig_glGetTexLevelParameteriv;
 func_glGetTexParameterfv_t	orig_glGetTexParameterfv;
 func_glGetTexParameteriv_t	orig_glGetTexParameteriv;
 func_glHint_t	orig_glHint;
 func_glIndexMask_t	orig_glIndexMask;
 func_glIndexPointer_t	orig_glIndexPointer;
 func_glIndexd_t	orig_glIndexd;
 func_glIndexdv_t	orig_glIndexdv;
 func_glIndexf_t	orig_glIndexf;
 func_glIndexfv_t	orig_glIndexfv;
 func_glIndexi_t	orig_glIndexi;
 func_glIndexiv_t	orig_glIndexiv;
 func_glIndexs_t	orig_glIndexs;
 func_glIndexsv_t	orig_glIndexsv;
 func_glIndexub_t	orig_glIndexub;
 func_glIndexubv_t	orig_glIndexubv;
 func_glInitNames_t	orig_glInitNames;
 func_glInterleavedArrays_t	orig_glInterleavedArrays;
 func_glIsEnabled_t	orig_glIsEnabled;
 func_glIsList_t	orig_glIsList;
 func_glIsTexture_t	orig_glIsTexture;
 func_glLightModelf_t	orig_glLightModelf;
 func_glLightModelfv_t	orig_glLightModelfv;
 func_glLightModeli_t	orig_glLightModeli;
 func_glLightModeliv_t	orig_glLightModeliv;
 func_glLightf_t	orig_glLightf;
 func_glLightfv_t	orig_glLightfv;
 func_glLighti_t	orig_glLighti;
 func_glLightiv_t	orig_glLightiv;
 func_glLineStipple_t	orig_glLineStipple;
 func_glLineWidth_t	orig_glLineWidth;
 func_glListBase_t	orig_glListBase;
 func_glLoadIdentity_t	orig_glLoadIdentity;
 func_glLoadMatrixd_t	orig_glLoadMatrixd;
 func_glLoadMatrixf_t	orig_glLoadMatrixf;
 func_glLoadName_t	orig_glLoadName;
 func_glLogicOp_t	orig_glLogicOp;
 func_glMap1d_t	orig_glMap1d;
 func_glMap1f_t	orig_glMap1f;
 func_glMap2d_t	orig_glMap2d;
 func_glMap2f_t	orig_glMap2f;
 func_glMapGrid1d_t	orig_glMapGrid1d;
 func_glMapGrid1f_t	orig_glMapGrid1f;
 func_glMapGrid2d_t	orig_glMapGrid2d;
 func_glMapGrid2f_t	orig_glMapGrid2f;
 func_glMaterialf_t	orig_glMaterialf;
 func_glMaterialfv_t	orig_glMaterialfv;
 func_glMateriali_t	orig_glMateriali;
 func_glMaterialiv_t	orig_glMaterialiv;
 func_glMatrixMode_t	orig_glMatrixMode;
 func_glMultMatrixd_t	orig_glMultMatrixd;
 func_glMultMatrixf_t	orig_glMultMatrixf;
 func_glNewList_t	orig_glNewList;
 func_glNormal3b_t	orig_glNormal3b;
 func_glNormal3bv_t	orig_glNormal3bv;
 func_glNormal3d_t	orig_glNormal3d;
 func_glNormal3dv_t	orig_glNormal3dv;
 func_glNormal3f_t	orig_glNormal3f;
 func_glNormal3fv_t	orig_glNormal3fv;
 func_glNormal3i_t	orig_glNormal3i;
 func_glNormal3iv_t	orig_glNormal3iv;
 func_glNormal3s_t	orig_glNormal3s;
 func_glNormal3sv_t	orig_glNormal3sv;
 func_glNormalPointer_t	orig_glNormalPointer;
 func_glOrtho_t	orig_glOrtho;
 func_glPassThrough_t	orig_glPassThrough;
 func_glPixelMapfv_t	orig_glPixelMapfv;
 func_glPixelMapuiv_t	orig_glPixelMapuiv;
 func_glPixelMapusv_t	orig_glPixelMapusv;
 func_glPixelStoref_t	orig_glPixelStoref;
 func_glPixelStorei_t	orig_glPixelStorei;
 func_glPixelTransferf_t	orig_glPixelTransferf;
 func_glPixelTransferi_t	orig_glPixelTransferi;
 func_glPixelZoom_t	orig_glPixelZoom;
 func_glPointSize_t	orig_glPointSize;
 func_glPolygonMode_t	orig_glPolygonMode;
 func_glPolygonOffset_t	orig_glPolygonOffset;
 func_glPolygonStipple_t	orig_glPolygonStipple;
 func_glPopAttrib_t	orig_glPopAttrib;
 func_glPopClientAttrib_t	orig_glPopClientAttrib;
 func_glPopMatrix_t	orig_glPopMatrix;
 func_glPopName_t	orig_glPopName;
 func_glPrioritizeTextures_t	orig_glPrioritizeTextures;
 func_glPushAttrib_t	orig_glPushAttrib;
 func_glPushClientAttrib_t	orig_glPushClientAttrib;
 func_glPushMatrix_t	orig_glPushMatrix;
 func_glPushName_t	orig_glPushName;
 func_glRasterPos2d_t	orig_glRasterPos2d;
 func_glRasterPos2dv_t	orig_glRasterPos2dv;
 func_glRasterPos2f_t	orig_glRasterPos2f;
 func_glRasterPos2fv_t	orig_glRasterPos2fv;
 func_glRasterPos2i_t	orig_glRasterPos2i;
 func_glRasterPos2iv_t	orig_glRasterPos2iv;
 func_glRasterPos2s_t	orig_glRasterPos2s;
 func_glRasterPos2sv_t	orig_glRasterPos2sv;
 func_glRasterPos3d_t	orig_glRasterPos3d;
 func_glRasterPos3dv_t	orig_glRasterPos3dv;
 func_glRasterPos3f_t	orig_glRasterPos3f;
 func_glRasterPos3fv_t	orig_glRasterPos3fv;
 func_glRasterPos3i_t	orig_glRasterPos3i;
 func_glRasterPos3iv_t	orig_glRasterPos3iv;
 func_glRasterPos3s_t	orig_glRasterPos3s;
 func_glRasterPos3sv_t	orig_glRasterPos3sv;
 func_glRasterPos4d_t	orig_glRasterPos4d;
 func_glRasterPos4dv_t	orig_glRasterPos4dv;
 func_glRasterPos4f_t	orig_glRasterPos4f;
 func_glRasterPos4fv_t	orig_glRasterPos4fv;
 func_glRasterPos4i_t	orig_glRasterPos4i;
 func_glRasterPos4iv_t	orig_glRasterPos4iv;
 func_glRasterPos4s_t	orig_glRasterPos4s;
 func_glRasterPos4sv_t	orig_glRasterPos4sv;
 func_glReadBuffer_t	orig_glReadBuffer;
 func_glReadPixels_t	orig_glReadPixels;
 func_glRectd_t	orig_glRectd;
 func_glRectdv_t	orig_glRectdv;
 func_glRectf_t	orig_glRectf;
 func_glRectfv_t	orig_glRectfv;
 func_glRecti_t	orig_glRecti;
 func_glRectiv_t	orig_glRectiv;
 func_glRects_t	orig_glRects;
 func_glRectsv_t	orig_glRectsv;
 func_glRenderMode_t	orig_glRenderMode;
 func_glRotated_t	orig_glRotated;
 func_glRotatef_t	orig_glRotatef;
 func_glScaled_t	orig_glScaled;
 func_glScalef_t	orig_glScalef;
 func_glScissor_t	orig_glScissor;
 func_glSelectBuffer_t	orig_glSelectBuffer;
 func_glShadeModel_t	orig_glShadeModel;
 func_glStencilFunc_t	orig_glStencilFunc;
 func_glStencilMask_t	orig_glStencilMask;
 func_glStencilOp_t	orig_glStencilOp;
 func_glSwapBuffers_t	orig_glSwapBuffers;
 func_glTexCoord1d_t	orig_glTexCoord1d;
 func_glTexCoord1dv_t	orig_glTexCoord1dv;
 func_glTexCoord1f_t	orig_glTexCoord1f;
 func_glTexCoord1fv_t	orig_glTexCoord1fv;
 func_glTexCoord1i_t	orig_glTexCoord1i;
 func_glTexCoord1iv_t	orig_glTexCoord1iv;
 func_glTexCoord1s_t	orig_glTexCoord1s;
 func_glTexCoord1sv_t	orig_glTexCoord1sv;
 func_glTexCoord2d_t	orig_glTexCoord2d;
 func_glTexCoord2dv_t	orig_glTexCoord2dv;
 func_glTexCoord2f_t	orig_glTexCoord2f;
 func_glTexCoord2fv_t	orig_glTexCoord2fv;
 func_glTexCoord2i_t	orig_glTexCoord2i;
 func_glTexCoord2iv_t	orig_glTexCoord2iv;
 func_glTexCoord2s_t	orig_glTexCoord2s;
 func_glTexCoord2sv_t	orig_glTexCoord2sv;
 func_glTexCoord3d_t	orig_glTexCoord3d;
 func_glTexCoord3dv_t	orig_glTexCoord3dv;
 func_glTexCoord3f_t	orig_glTexCoord3f;
 func_glTexCoord3fv_t	orig_glTexCoord3fv;
 func_glTexCoord3i_t	orig_glTexCoord3i;
 func_glTexCoord3iv_t	orig_glTexCoord3iv;
 func_glTexCoord3s_t	orig_glTexCoord3s;
 func_glTexCoord3sv_t	orig_glTexCoord3sv;
 func_glTexCoord4d_t	orig_glTexCoord4d;
 func_glTexCoord4dv_t	orig_glTexCoord4dv;
 func_glTexCoord4f_t	orig_glTexCoord4f;
 func_glTexCoord4fv_t	orig_glTexCoord4fv;
 func_glTexCoord4i_t	orig_glTexCoord4i;
 func_glTexCoord4iv_t	orig_glTexCoord4iv;
 func_glTexCoord4s_t	orig_glTexCoord4s;
 func_glTexCoord4sv_t	orig_glTexCoord4sv;
 func_glTexCoordPointer_t	orig_glTexCoordPointer;
 func_glTexEnvf_t	orig_glTexEnvf;
 func_glTexEnvfv_t	orig_glTexEnvfv;
 func_glTexEnvi_t	orig_glTexEnvi;
 func_glTexEnviv_t	orig_glTexEnviv;
 func_glTexGend_t	orig_glTexGend;
 func_glTexGendv_t	orig_glTexGendv;
 func_glTexGenf_t	orig_glTexGenf;
 func_glTexGenfv_t	orig_glTexGenfv;
 func_glTexGeni_t	orig_glTexGeni;
 func_glTexGeniv_t	orig_glTexGeniv;
 func_glTexImage1D_t	orig_glTexImage1D;
 func_glTexImage2D_t	orig_glTexImage2D;
 func_glTexParameterf_t	orig_glTexParameterf;
 func_glTexParameterfv_t	orig_glTexParameterfv;
 func_glTexParameteri_t	orig_glTexParameteri;
 func_glTexParameteriv_t	orig_glTexParameteriv;
 func_glTexSubImage1D_t	orig_glTexSubImage1D;
 func_glTexSubImage2D_t	orig_glTexSubImage2D;
 func_glTranslated_t	orig_glTranslated;
 func_glTranslatef_t	orig_glTranslatef;
 func_glVertex2d_t	orig_glVertex2d;
 func_glVertex2dv_t	orig_glVertex2dv;
 func_glVertex2f_t	orig_glVertex2f;
 func_glVertex2fv_t	orig_glVertex2fv;
 func_glVertex2i_t	orig_glVertex2i;
 func_glVertex2iv_t	orig_glVertex2iv;
 func_glVertex2s_t	orig_glVertex2s;
 func_glVertex2sv_t	orig_glVertex2sv;
 func_glVertex3d_t	orig_glVertex3d;
 func_glVertex3dv_t	orig_glVertex3dv;
 func_glVertex3f_t	orig_glVertex3f;
 func_glVertex3fv_t	orig_glVertex3fv;
 func_glVertex3i_t	orig_glVertex3i;
 func_glVertex3iv_t	orig_glVertex3iv;
 func_glVertex3s_t	orig_glVertex3s;
 func_glVertex3sv_t	orig_glVertex3sv;
 func_glVertex4d_t	orig_glVertex4d;
 func_glVertex4dv_t	orig_glVertex4dv;
 func_glVertex4f_t	orig_glVertex4f;
 func_glVertex4fv_t	orig_glVertex4fv;
 func_glVertex4i_t	orig_glVertex4i;
 func_glVertex4iv_t	orig_glVertex4iv;
 func_glVertex4s_t	orig_glVertex4s;
 func_glVertex4sv_t	orig_glVertex4sv;
 func_glVertexPointer_t	orig_glVertexPointer;
 func_glViewport_t	orig_glViewport;
 func_wglChoosePixelFormat_t	orig_wglChoosePixelFormat;
 func_wglCopyContext_t	orig_wglCopyContext;
 func_wglCreateContext_t	orig_wglCreateContext;
 func_wglCreateLayerContext_t	orig_wglCreateLayerContext;
 func_wglDeleteContext_t	orig_wglDeleteContext;
 func_wglDescribeLayerPlane_t	orig_wglDescribeLayerPlane;
 func_wglDescribePixelFormat_t	orig_wglDescribePixelFormat;
 func_wglGetCurrentContext_t	orig_wglGetCurrentContext;
 func_wglGetCurrentDC_t	orig_wglGetCurrentDC;
 func_wglGetDefaultProcAddress_t	orig_wglGetDefaultProcAddress;
 func_wglGetLayerPaletteEntries_t	orig_wglGetLayerPaletteEntries;
 func_wglGetPixelFormat_t	orig_wglGetPixelFormat;
 func_wglGetProcAddress_t	orig_wglGetProcAddress;
 func_wglMakeCurrent_t	orig_wglMakeCurrent;
 func_wglRealizeLayerPalette_t	orig_wglRealizeLayerPalette;
 func_wglSetLayerPaletteEntries_t	orig_wglSetLayerPaletteEntries;
 func_wglSetPixelFormat_t	orig_wglSetPixelFormat;
 func_wglShareLists_t	orig_wglShareLists;
 func_wglSwapBuffers_t	orig_wglSwapBuffers;
 func_wglSwapLayerBuffers_t	orig_wglSwapLayerBuffers;
 func_wglUseFontBitmapsA_t	orig_wglUseFontBitmapsA;
 func_wglUseFontBitmapsW_t	orig_wglUseFontBitmapsW;
 func_wglUseFontOutlinesA_t	orig_wglUseFontOutlinesA;
 func_wglUseFontOutlinesW_t	orig_wglUseFontOutlinesW;

 func_glBegin_t orig_exp_glBegin;
 DWORD *exp_glBegin = (DWORD*)0x013E4198;
 func_glDrawElements_t orig_exp_glDrawElements;
 DWORD *exp_glDrawElements = (DWORD*)0x013E3D04;
 func_glViewport_t orig_exp_glViewport;
 DWORD *exp_glViewport = (DWORD*)0x013E3FD0;

BOOL glInit (void)
{
	/*char sysdir[MAX_PATH];

	GetSystemDirectory (sysdir, sizeof(sysdir));
	strcat (sysdir, "\\");
	strcat (sysdir, ORIGINAL_DLL);

	hOpenGL = GetModuleHandle(sysdir);
				
	if ( !hOpenGL )
		return FALSE;*/
	
#if 0
	if ( (gl::qglAccum = (pfn_glAccum) GetProcAddress (hOpenGL, "glAccum")) == NULL )
		return FALSE;

	if ( (gl::qglAlphaFunc = (pfn_glAlphaFunc) GetProcAddress (hOpenGL, "glAlphaFunc")) == NULL )
		return FALSE;

	if ( (gl::qglAreTexturesResident = (pfn_glAreTexturesResident) GetProcAddress (hOpenGL, "glAreTexturesResident")) == NULL )
		return FALSE;

	if ( (gl::qglArrayElement = (pfn_glArrayElement) GetProcAddress (hOpenGL, "glArrayElement")) == NULL )
		return FALSE;

	if ( (gl::qglBegin = (pfn_glBegin) GetProcAddress (hOpenGL, "glBegin")) == NULL )
		return FALSE;

	if ( (gl::qglBindTexture = (pfn_glBindTexture) GetProcAddress (hOpenGL, "glBindTexture")) == NULL )
		return FALSE;

	if ( (gl::qglBitmap = (pfn_glBitmap) GetProcAddress (hOpenGL, "glBitmap")) == NULL )
		return FALSE;

	if ( (gl::qglBlendFunc = (pfn_glBlendFunc) GetProcAddress (hOpenGL, "glBlendFunc")) == NULL )
		return FALSE;

	if ( (gl::qglCallList = (pfn_glCallList) GetProcAddress (hOpenGL, "glCallList")) == NULL )
		return FALSE;

	if ( (gl::qglCallLists = (pfn_glCallLists) GetProcAddress (hOpenGL, "glCallLists")) == NULL )
		return FALSE;

	if ( (gl::qglClear = (pfn_glClear) GetProcAddress (hOpenGL, "glClear")) == NULL )
		return FALSE;

	if ( (gl::qglClearAccum = (pfn_glClearAccum) GetProcAddress (hOpenGL, "glClearAccum")) == NULL )
		return FALSE;

	if ( (gl::qglClearColor = (pfn_glClearColor) GetProcAddress (hOpenGL, "glClearColor")) == NULL )
		return FALSE;

	if ( (gl::qglClearDepth = (pfn_glClearDepth) GetProcAddress (hOpenGL, "glClearDepth")) == NULL )
		return FALSE;

	if ( (gl::qglClearIndex = (pfn_glClearIndex) GetProcAddress (hOpenGL, "glClearIndex")) == NULL )
		return FALSE;

	if ( (gl::qglClearStencil = (pfn_glClearStencil) GetProcAddress (hOpenGL, "glClearStencil")) == NULL )
		return FALSE;

	if ( (gl::qglClipPlane = (pfn_glClipPlane) GetProcAddress (hOpenGL, "glClipPlane")) == NULL )
		return FALSE;

	if ( (gl::qglColor3b = (pfn_glColor3b) GetProcAddress (hOpenGL, "glColor3b")) == NULL )
		return FALSE;

	if ( (gl::qglColor3bv = (pfn_glColor3bv) GetProcAddress (hOpenGL, "glColor3bv")) == NULL )
		return FALSE;

	if ( (gl::qglColor3d = (pfn_glColor3d) GetProcAddress (hOpenGL, "glColor3d")) == NULL )
		return FALSE;

	if ( (gl::qglColor3dv = (pfn_glColor3dv) GetProcAddress (hOpenGL, "glColor3dv")) == NULL )
		return FALSE;

	if ( (gl::qglColor3f = (pfn_glColor3f) GetProcAddress (hOpenGL, "glColor3f")) == NULL )
		return FALSE;

	if ( (gl::qglColor3fv = (pfn_glColor3fv) GetProcAddress (hOpenGL, "glColor3fv")) == NULL )
		return FALSE;

	if ( (gl::qglColor3i = (pfn_glColor3i) GetProcAddress (hOpenGL, "glColor3i")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor3i()");
		return FALSE;
	}

	if ( (gl::qglColor3iv = (pfn_glColor3iv) GetProcAddress (hOpenGL, "glColor3iv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor3iv()");
		return FALSE;
	}

	if ( (gl::qglColor3s = (pfn_glColor3s) GetProcAddress (hOpenGL, "glColor3s")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor3s()");
		return FALSE;
	}

	if ( (gl::qglColor3sv = (pfn_glColor3sv) GetProcAddress (hOpenGL, "glColor3sv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor3sv()");
		return FALSE;
	}

	if ( (gl::qglColor3ub = (pfn_glColor3ub) GetProcAddress (hOpenGL, "glColor3ub")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor3ub()");
		return FALSE;
	}

	if ( (gl::qglColor3ubv = (pfn_glColor3ubv) GetProcAddress (hOpenGL, "glColor3ubv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor3ubv()");
		return FALSE;
	}

	if ( (gl::qglColor3ui = (pfn_glColor3ui) GetProcAddress (hOpenGL, "glColor3ui")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor3ui()");
		return FALSE;
	}

	if ( (gl::qglColor3uiv = (pfn_glColor3uiv) GetProcAddress (hOpenGL, "glColor3uiv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor3uiv()");
		return FALSE;
	}

	if ( (gl::qglColor3us = (pfn_glColor3us) GetProcAddress (hOpenGL, "glColor3us")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor3us()");
		return FALSE;
	}

	if ( (gl::qglColor3usv = (pfn_glColor3usv) GetProcAddress (hOpenGL, "glColor3usv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor3usv()");
		return FALSE;
	}

	if ( (gl::qglColor4b = (pfn_glColor4b) GetProcAddress (hOpenGL, "glColor4b")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4b()");
		return FALSE;
	}

	if ( (gl::qglColor4bv = (pfn_glColor4bv) GetProcAddress (hOpenGL, "glColor4bv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4bv()");
		return FALSE;
	}

	if ( (gl::qglColor4d = (pfn_glColor4d) GetProcAddress (hOpenGL, "glColor4d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4d()");
		return FALSE;
	}

	if ( (gl::qglColor4dv = (pfn_glColor4dv) GetProcAddress (hOpenGL, "glColor4dv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4dv()");
		return FALSE;
	}

	if ( (gl::qglColor4f = (pfn_glColor4f) GetProcAddress (hOpenGL, "glColor4f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4f()");
		return FALSE;
	}

	if ( (gl::qglColor4fv = (pfn_glColor4fv) GetProcAddress (hOpenGL, "glColor4fv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4fv()");
		return FALSE;
	}

	if ( (gl::qglColor4i = (pfn_glColor4i) GetProcAddress (hOpenGL, "glColor4i")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4i()");
		return FALSE;
	}

	if ( (gl::qglColor4iv = (pfn_glColor4iv) GetProcAddress (hOpenGL, "glColor4iv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4iv()");
		return FALSE;
	}

	if ( (gl::qglColor4s = (pfn_glColor4s) GetProcAddress (hOpenGL, "glColor4s")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4s()");
		return FALSE;
	}

	if ( (gl::qglColor4sv = (pfn_glColor4sv) GetProcAddress (hOpenGL, "glColor4sv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4sv()");
		return FALSE;
	}

	if ( (gl::qglColor4ub = (pfn_glColor4ub) GetProcAddress (hOpenGL, "glColor4ub")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4ub()");
		return FALSE;
	}

	if ( (gl::qglColor4ubv = (pfn_glColor4ubv) GetProcAddress (hOpenGL, "glColor4ubv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4ubv()");
		return FALSE;
	}

	if ( (gl::qglColor4ui = (pfn_glColor4ui) GetProcAddress (hOpenGL, "glColor4ui")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4ui()");
		return FALSE;
	}

	if ( (gl::qglColor4uiv = (pfn_glColor4uiv) GetProcAddress (hOpenGL, "glColor4uiv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4uiv()");
		return FALSE;
	}

	if ( (gl::qglColor4us = (pfn_glColor4us) GetProcAddress (hOpenGL, "glColor4us")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4us()");
		return FALSE;
	}

	if ( (gl::qglColor4usv = (pfn_glColor4usv) GetProcAddress (hOpenGL, "glColor4usv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColor4usv()");
		return FALSE;
	}

	if ( (gl::qglColorMask = (pfn_glColorMask) GetProcAddress (hOpenGL, "glColorMask")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColorMask()");
		return FALSE;
	}

	if ( (gl::qglColorMaterial = (pfn_glColorMaterial) GetProcAddress (hOpenGL, "glColorMaterial")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColorMaterial()");
		return FALSE;
	}

	if ( (gl::qglColorPointer = (pfn_glColorPointer) GetProcAddress (hOpenGL, "glColorPointer")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glColorPointer()");
		return FALSE;
	}

	if ( (gl::qglCopyPixels = (pfn_glCopyPixels) GetProcAddress (hOpenGL, "glCopyPixels")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glCopyPixels()");
		return FALSE;
	}

	if ( (gl::qglCopyTexImage1D = (pfn_glCopyTexImage1D) GetProcAddress (hOpenGL, "glCopyTexImage1D")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glCopyTexImage1D()");
		return FALSE;
	}

	if ( (gl::qglCopyTexImage2D = (pfn_glCopyTexImage2D) GetProcAddress (hOpenGL, "glCopyTexImage2D")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glCopyTexImage2D()");
		return FALSE;
	}

	if ( (gl::qglCopyTexSubImage1D = (pfn_glCopyTexSubImage1D) GetProcAddress (hOpenGL, "glCopyTexSubImage1D")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glCopyTexSubImage1D()");
		return FALSE;
	}

	if ( (gl::qglCopyTexSubImage2D = (pfn_glCopyTexSubImage2D) GetProcAddress (hOpenGL, "glCopyTexSubImage2D")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glCopyTexSubImage2D()");
		return FALSE;
	}

	if ( (gl::qglCullFace = (pfn_glCullFace) GetProcAddress (hOpenGL, "glCullFace")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glCullFace()");
		return FALSE;
	}

	/*if ( (gl::qglDebugEntry = (pfn_glDebugEntry) GetProcAddress (hOpenGL, "glDebugEntry")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glDebugEntry()");
		return FALSE;
	}*/

	if ( (gl::qglDeleteLists = (pfn_glDeleteLists) GetProcAddress (hOpenGL, "glDeleteLists")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glDeleteLists()");
		return FALSE;
	}

	if ( (gl::qglDeleteTextures = (pfn_glDeleteTextures) GetProcAddress (hOpenGL, "glDeleteTextures")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glDeleteTextures()");
		return FALSE;
	}

	if ( (gl::qglDepthFunc = (pfn_glDepthFunc) GetProcAddress (hOpenGL, "glDepthFunc")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glDepthFunc()");
		return FALSE;
	}

	if ( (gl::qglDepthMask = (pfn_glDepthMask) GetProcAddress (hOpenGL, "glDepthMask")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glDepthMask()");
		return FALSE;
	}

	if ( (gl::qglDepthRange = (pfn_glDepthRange) GetProcAddress (hOpenGL, "glDepthRange")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glDepthRange()");
		return FALSE;
	}

	if ( (gl::qglDisable = (pfn_glDisable) GetProcAddress (hOpenGL, "glDisable")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glDisable()");
		return FALSE;
	}

	if ( (gl::qglDisableClientState = (pfn_glDisableClientState) GetProcAddress (hOpenGL, "glDisableClientState")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glDisableClientState()");
		return FALSE;
	}

	if ( (gl::qglDrawArrays = (pfn_glDrawArrays) GetProcAddress (hOpenGL, "glDrawArrays")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glDrawArrays()");
		return FALSE;
	}

	if ( (gl::qglDrawBuffer = (pfn_glDrawBuffer) GetProcAddress (hOpenGL, "glDrawBuffer")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glDrawBuffer()");
		return FALSE;
	}

	if ( (gl::qglDrawElements = (pfn_glDrawElements) GetProcAddress (hOpenGL, "glDrawElements")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glDrawElements()");
		return FALSE;
	}

	if ( (gl::qglDrawPixels = (pfn_glDrawPixels) GetProcAddress (hOpenGL, "glDrawPixels")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glDrawPixels()");
		return FALSE;
	}

	if ( (gl::qglEdgeFlag = (pfn_glEdgeFlag) GetProcAddress (hOpenGL, "glEdgeFlag")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEdgeFlag()");
		return FALSE;
	}

	if ( (gl::qglEdgeFlagPointer = (pfn_glEdgeFlagPointer) GetProcAddress (hOpenGL, "glEdgeFlagPointer")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEdgeFlagPointer()");
		return FALSE;
	}

	if ( (gl::qglEdgeFlagv = (pfn_glEdgeFlagv) GetProcAddress (hOpenGL, "glEdgeFlagv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEdgeFlagv()");
		return FALSE;
	}

	if ( (gl::qglEnable = (pfn_glEnable) GetProcAddress (hOpenGL, "glEnable")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEnable()");
		return FALSE;
	}

	if ( (gl::qglEnableClientState = (pfn_glEnableClientState) GetProcAddress (hOpenGL, "glEnableClientState")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEnableClientState()");
		return FALSE;
	}

	if ( (gl::qglEnd = (pfn_glEnd) GetProcAddress (hOpenGL, "glEnd")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEnd()");
		return FALSE;
	}

	if ( (gl::qglEndList = (pfn_glEndList) GetProcAddress (hOpenGL, "glEndList")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEndList()");
		return FALSE;
	}

	if ( (gl::qglEvalCoord1d = (pfn_glEvalCoord1d) GetProcAddress (hOpenGL, "glEvalCoord1d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEvalCoord1d()");
		return FALSE;
	}

	if ( (gl::qglEvalCoord1dv = (pfn_glEvalCoord1dv) GetProcAddress (hOpenGL, "glEvalCoord1dv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEvalCoord1dv()");
		return FALSE;
	}

	if ( (gl::qglEvalCoord1f = (pfn_glEvalCoord1f) GetProcAddress (hOpenGL, "glEvalCoord1f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEvalCoord1f()");
		return FALSE;
	}

	if ( (gl::qglEvalCoord1fv = (pfn_glEvalCoord1fv) GetProcAddress (hOpenGL, "glEvalCoord1fv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEvalCoord1fv()");
		return FALSE;
	}

	if ( (gl::qglEvalCoord2d = (pfn_glEvalCoord2d) GetProcAddress (hOpenGL, "glEvalCoord2d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEvalCoord2d()");
		return FALSE;
	}

	if ( (gl::qglEvalCoord2dv = (pfn_glEvalCoord2dv) GetProcAddress (hOpenGL, "glEvalCoord2dv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEvalCoord2dv()");
		return FALSE;
	}

	if ( (gl::qglEvalCoord2f = (pfn_glEvalCoord2f) GetProcAddress (hOpenGL, "glEvalCoord2f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEvalCoord2f()");
		return FALSE;
	}

	if ( (gl::qglEvalCoord2fv = (pfn_glEvalCoord2fv) GetProcAddress (hOpenGL, "glEvalCoord2fv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEvalCoord2fv()");
		return FALSE;
	}

	if ( (gl::qglEvalMesh1 = (pfn_glEvalMesh1) GetProcAddress (hOpenGL, "glEvalMesh1")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEvalMesh1()");
		return FALSE;
	}

	if ( (gl::qglEvalMesh2 = (pfn_glEvalMesh2) GetProcAddress (hOpenGL, "glEvalMesh2")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEvalMesh2()");
		return FALSE;
	}

	if ( (gl::qglEvalPoint1 = (pfn_glEvalPoint1) GetProcAddress (hOpenGL, "glEvalPoint1")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEvalPoint1()");
		return FALSE;
	}

	if ( (gl::qglEvalPoint2 = (pfn_glEvalPoint2) GetProcAddress (hOpenGL, "glEvalPoint2")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glEvalPoint2()");
		return FALSE;
	}

	if ( (gl::qglFeedbackBuffer = (pfn_glFeedbackBuffer) GetProcAddress (hOpenGL, "glFeedbackBuffer")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glFeedbackBuffer()");
		return FALSE;
	}

	if ( (gl::qglFinish = (pfn_glFinish) GetProcAddress (hOpenGL, "glFinish")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glFinish()");
		return FALSE;
	}

	if ( (gl::qglFlush = (pfn_glFlush) GetProcAddress (hOpenGL, "glFlush")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glFlush()");
		return FALSE;
	}

	if ( (gl::qglFogf = (pfn_glFogf) GetProcAddress (hOpenGL, "glFogf")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glFogf()");
		return FALSE;
	}

	if ( (gl::qglFogfv = (pfn_glFogfv) GetProcAddress (hOpenGL, "glFogfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glFogfv()");
		return FALSE;
	}

	if ( (gl::qglFogi = (pfn_glFogi) GetProcAddress (hOpenGL, "glFogi")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glFogi()");
		return FALSE;
	}

	if ( (gl::qglFogiv = (pfn_glFogiv) GetProcAddress (hOpenGL, "glFogiv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glFogiv()");
		return FALSE;
	}

	if ( (gl::qglFrontFace = (pfn_glFrontFace) GetProcAddress (hOpenGL, "glFrontFace")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glFrontFace()");
		return FALSE;
	}

	if ( (gl::qglFrustum = (pfn_glFrustum) GetProcAddress (hOpenGL, "glFrustum")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glFrustum()");
		return FALSE;
	}

	if ( (gl::qglGenLists = (pfn_glGenLists) GetProcAddress (hOpenGL, "glGenLists")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGenLists()");
		return FALSE;
	}

	if ( (gl::qglGenTextures = (pfn_glGenTextures) GetProcAddress (hOpenGL, "glGenTextures")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGenTextures()");
		return FALSE;
	}

	if ( (gl::qglGetBooleanv = (pfn_glGetBooleanv) GetProcAddress (hOpenGL, "glGetBooleanv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetBooleanv()");
		return FALSE;
	}

	if ( (gl::qglGetClipPlane = (pfn_glGetClipPlane) GetProcAddress (hOpenGL, "glGetClipPlane")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetClipPlane()");
		return FALSE;
	}

	if ( (gl::qglGetDoublev = (pfn_glGetDoublev) GetProcAddress (hOpenGL, "glGetDoublev")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetDoublev()");
		return FALSE;
	}

	if ( (gl::qglGetError = (pfn_glGetError) GetProcAddress (hOpenGL, "glGetError")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetError()");
		return FALSE;
	}

	if ( (gl::qglGetFloatv = (pfn_glGetFloatv) GetProcAddress (hOpenGL, "glGetFloatv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetFloatv()");
		return FALSE;
	}

	if ( (gl::qglGetIntegerv = (pfn_glGetIntegerv) GetProcAddress (hOpenGL, "glGetIntegerv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetIntegerv()");
		return FALSE;
	}

	if ( (gl::qglGetLightfv = (pfn_glGetLightfv) GetProcAddress (hOpenGL, "glGetLightfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetLightfv()");
		return FALSE;
	}

	if ( (gl::qglGetLightiv = (pfn_glGetLightiv) GetProcAddress (hOpenGL, "glGetLightiv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetLightiv()");
		return FALSE;
	}

	if ( (gl::qglGetMapdv = (pfn_glGetMapdv) GetProcAddress (hOpenGL, "glGetMapdv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetMapdv()");
		return FALSE;
	}

	if ( (gl::qglGetMapfv = (pfn_glGetMapfv) GetProcAddress (hOpenGL, "glGetMapfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetMapfv()");
		return FALSE;
	}

	if ( (gl::qglGetMapiv = (pfn_glGetMapiv) GetProcAddress (hOpenGL, "glGetMapiv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetMapiv()");
		return FALSE;
	}

	if ( (gl::qglGetMaterialfv = (pfn_glGetMaterialfv) GetProcAddress (hOpenGL, "glGetMaterialfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetMaterialfv()");
		return FALSE;
	}

	if ( (gl::qglGetMaterialiv = (pfn_glGetMaterialiv) GetProcAddress (hOpenGL, "glGetMaterialiv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetMaterialiv()");
		return FALSE;
	}

	if ( (gl::qglGetPixelMapfv = (pfn_glGetPixelMapfv) GetProcAddress (hOpenGL, "glGetPixelMapfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetPixelMapfv()");
		return FALSE;
	}

	if ( (gl::qglGetPixelMapuiv = (pfn_glGetPixelMapuiv) GetProcAddress (hOpenGL, "glGetPixelMapuiv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetPixelMapuiv()");
		return FALSE;
	}

	if ( (gl::qglGetPixelMapusv = (pfn_glGetPixelMapusv) GetProcAddress (hOpenGL, "glGetPixelMapusv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetPixelMapusv()");
		return FALSE;
	}

	if ( (gl::qglGetPointerv = (pfn_glGetPointerv) GetProcAddress (hOpenGL, "glGetPointerv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetPointerv()");
		return FALSE;
	}

	if ( (gl::qglGetPolygonStipple = (pfn_glGetPolygonStipple) GetProcAddress (hOpenGL, "glGetPolygonStipple")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetPolygonStipple()");
		return FALSE;
	}

	if ( (gl::qglGetString = (pfn_glGetString) GetProcAddress (hOpenGL, "glGetString")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetString()");
		return FALSE;
	}

	if ( (gl::qglGetTexEnvfv = (pfn_glGetTexEnvfv) GetProcAddress (hOpenGL, "glGetTexEnvfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetTexEnvfv()");
		return FALSE;
	}

	if ( (gl::qglGetTexEnviv = (pfn_glGetTexEnviv) GetProcAddress (hOpenGL, "glGetTexEnviv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetTexEnviv()");
		return FALSE;
	}

	if ( (gl::qglGetTexGendv = (pfn_glGetTexGendv) GetProcAddress (hOpenGL, "glGetTexGendv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetTexGendv()");
		return FALSE;
	}

	if ( (gl::qglGetTexGenfv = (pfn_glGetTexGenfv) GetProcAddress (hOpenGL, "glGetTexGenfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetTexGenfv()");
		return FALSE;
	}

	if ( (gl::qglGetTexGeniv = (pfn_glGetTexGeniv) GetProcAddress (hOpenGL, "glGetTexGeniv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetTexGeniv()");
		return FALSE;
	}

	if ( (gl::qglGetTexImage = (pfn_glGetTexImage) GetProcAddress (hOpenGL, "glGetTexImage")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetTexImage()");
		return FALSE;
	}

	if ( (gl::qglGetTexLevelParameterfv = (pfn_glGetTexLevelParameterfv) GetProcAddress (hOpenGL, "glGetTexLevelParameterfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetTexLevelParameterfv()");
		return FALSE;
	}

	if ( (gl::qglGetTexLevelParameteriv = (pfn_glGetTexLevelParameteriv) GetProcAddress (hOpenGL, "glGetTexLevelParameteriv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetTexLevelParameteriv()");
		return FALSE;
	}

	if ( (gl::qglGetTexParameterfv = (pfn_glGetTexParameterfv) GetProcAddress (hOpenGL, "glGetTexParameterfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetTexParameterfv()");
		return FALSE;
	}

	if ( (gl::qglGetTexParameteriv = (pfn_glGetTexParameteriv) GetProcAddress (hOpenGL, "glGetTexParameteriv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glGetTexParameteriv()");
		return FALSE;
	}

	if ( (gl::qglHint = (pfn_glHint) GetProcAddress (hOpenGL, "glHint")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glHint()");
		return FALSE;
	}

	if ( (gl::qglIndexMask = (pfn_glIndexMask) GetProcAddress (hOpenGL, "glIndexMask")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glIndexMask()");
		return FALSE;
	}

	if ( (gl::qglIndexPointer = (pfn_glIndexPointer) GetProcAddress (hOpenGL, "glIndexPointer")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glIndexPointer()");
		return FALSE;
	}

	if ( (gl::qglIndexd = (pfn_glIndexd) GetProcAddress (hOpenGL, "glIndexd")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glIndexd()");
		return FALSE;
	}

	if ( (gl::qglIndexdv = (pfn_glIndexdv) GetProcAddress (hOpenGL, "glIndexdv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glIndexdv()");
		return FALSE;
	}

	if ( (gl::qglIndexf = (pfn_glIndexf) GetProcAddress (hOpenGL, "glIndexf")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glIndexf()");
		return FALSE;
	}

	if ( (gl::qglIndexfv = (pfn_glIndexfv) GetProcAddress (hOpenGL, "glIndexfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glIndexfv()");
		return FALSE;
	}

	if ( (gl::qglIndexi = (pfn_glIndexi) GetProcAddress (hOpenGL, "glIndexi")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glIndexi()");
		return FALSE;
	}

	if ( (gl::qglIndexiv = (pfn_glIndexiv) GetProcAddress (hOpenGL, "glIndexiv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glIndexiv()");
		return FALSE;
	}

	if ( (gl::qglIndexs = (pfn_glIndexs) GetProcAddress (hOpenGL, "glIndexs")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glIndexs()");
		return FALSE;
	}

	if ( (gl::qglIndexsv = (pfn_glIndexsv) GetProcAddress (hOpenGL, "glIndexsv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glIndexsv()");
		return FALSE;
	}

	if ( (gl::qglIndexub = (pfn_glIndexub) GetProcAddress (hOpenGL, "glIndexub")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glIndexub()");
		return FALSE;
	}

	if ( (gl::qglIndexubv = (pfn_glIndexubv) GetProcAddress (hOpenGL, "glIndexubv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glIndexubv()");
		return FALSE;
	}

	if ( (gl::qglInitNames = (pfn_glInitNames) GetProcAddress (hOpenGL, "glInitNames")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glInitNames()");
		return FALSE;
	}

	if ( (gl::qglInterleavedArrays = (pfn_glInterleavedArrays) GetProcAddress (hOpenGL, "glInterleavedArrays")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glInterleavedArrays()");
		return FALSE;
	}

	if ( (gl::qglIsEnabled = (pfn_glIsEnabled) GetProcAddress (hOpenGL, "glIsEnabled")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glIsEnabled()");
		return FALSE;
	}

	if ( (gl::qglIsList = (pfn_glIsList) GetProcAddress (hOpenGL, "glIsList")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glIsList()");
		return FALSE;
	}

	if ( (gl::qglIsTexture = (pfn_glIsTexture) GetProcAddress (hOpenGL, "glIsTexture")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glIsTexture()");
		return FALSE;
	}

	if ( (gl::qglLightModelf = (pfn_glLightModelf) GetProcAddress (hOpenGL, "glLightModelf")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glLightModelf()");
		return FALSE;
	}

	if ( (gl::qglLightModelfv = (pfn_glLightModelfv) GetProcAddress (hOpenGL, "glLightModelfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glLightModelfv()");
		return FALSE;
	}

	if ( (gl::qglLightModeli = (pfn_glLightModeli) GetProcAddress (hOpenGL, "glLightModeli")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glLightModeli()");
		return FALSE;
	}

	if ( (gl::qglLightModeliv = (pfn_glLightModeliv) GetProcAddress (hOpenGL, "glLightModeliv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glLightModeliv()");
		return FALSE;
	}

	if ( (gl::qglLightf = (pfn_glLightf) GetProcAddress (hOpenGL, "glLightf")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glLightf()");
		return FALSE;
	}

	if ( (gl::qglLightfv = (pfn_glLightfv) GetProcAddress (hOpenGL, "glLightfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glLightfv()");
		return FALSE;
	}

	if ( (gl::qglLighti = (pfn_glLighti) GetProcAddress (hOpenGL, "glLighti")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glLighti()");
		return FALSE;
	}

	if ( (gl::qglLightiv = (pfn_glLightiv) GetProcAddress (hOpenGL, "glLightiv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glLightiv()");
		return FALSE;
	}

	if ( (gl::qglLineStipple = (pfn_glLineStipple) GetProcAddress (hOpenGL, "glLineStipple")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glLineStipple()");
		return FALSE;
	}

	if ( (gl::qglLineWidth = (pfn_glLineWidth) GetProcAddress (hOpenGL, "glLineWidth")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glLineWidth()");
		return FALSE;
	}

	if ( (gl::qglListBase = (pfn_glListBase) GetProcAddress (hOpenGL, "glListBase")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glListBase()");
		return FALSE;
	}

	if ( (gl::qglLoadIdentity = (pfn_glLoadIdentity) GetProcAddress (hOpenGL, "glLoadIdentity")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glLoadIdentity()");
		return FALSE;
	}

	if ( (gl::qglLoadMatrixd = (pfn_glLoadMatrixd) GetProcAddress (hOpenGL, "glLoadMatrixd")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glLoadMatrixd()");
		return FALSE;
	}

	if ( (gl::qglLoadMatrixf = (pfn_glLoadMatrixf) GetProcAddress (hOpenGL, "glLoadMatrixf")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glLoadMatrixf()");
		return FALSE;
	}

	if ( (gl::qglLoadName = (pfn_glLoadName) GetProcAddress (hOpenGL, "glLoadName")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glLoadName()");
		return FALSE;
	}

	if ( (gl::qglLogicOp = (pfn_glLogicOp) GetProcAddress (hOpenGL, "glLogicOp")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glLogicOp()");
		return FALSE;
	}

	if ( (gl::qglMap1d = (pfn_glMap1d) GetProcAddress (hOpenGL, "glMap1d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glMap1d()");
		return FALSE;
	}

	if ( (gl::qglMap1f = (pfn_glMap1f) GetProcAddress (hOpenGL, "glMap1f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glMap1f()");
		return FALSE;
	}

	if ( (gl::qglMap2d = (pfn_glMap2d) GetProcAddress (hOpenGL, "glMap2d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glMap2d()");
		return FALSE;
	}

	if ( (gl::qglMap2f = (pfn_glMap2f) GetProcAddress (hOpenGL, "glMap2f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glMap2f()");
		return FALSE;
	}

	if ( (gl::qglMapGrid1d = (pfn_glMapGrid1d) GetProcAddress (hOpenGL, "glMapGrid1d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glMapGrid1d()");
		return FALSE;
	}

	if ( (gl::qglMapGrid1f = (pfn_glMapGrid1f) GetProcAddress (hOpenGL, "glMapGrid1f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glMapGrid1f()");
		return FALSE;
	}

	if ( (gl::qglMapGrid2d = (pfn_glMapGrid2d) GetProcAddress (hOpenGL, "glMapGrid2d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glMapGrid2d()");
		return FALSE;
	}

	if ( (gl::qglMapGrid2f = (pfn_glMapGrid2f) GetProcAddress (hOpenGL, "glMapGrid2f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glMapGrid2f()");
		return FALSE;
	}

	if ( (gl::qglMaterialf = (pfn_glMaterialf) GetProcAddress (hOpenGL, "glMaterialf")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glMaterialf()");
		return FALSE;
	}

	if ( (gl::qglMaterialfv = (pfn_glMaterialfv) GetProcAddress (hOpenGL, "glMaterialfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glMaterialfv()");
		return FALSE;
	}

	if ( (gl::qglMateriali = (pfn_glMateriali) GetProcAddress (hOpenGL, "glMateriali")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glMateriali()");
		return FALSE;
	}

	if ( (gl::qglMaterialiv = (pfn_glMaterialiv) GetProcAddress (hOpenGL, "glMaterialiv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glMaterialiv()");
		return FALSE;
	}

	if ( (gl::qglMatrixMode = (pfn_glMatrixMode) GetProcAddress (hOpenGL, "glMatrixMode")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glMatrixMode()");
		return FALSE;
	}

	if ( (gl::qglMultMatrixd = (pfn_glMultMatrixd) GetProcAddress (hOpenGL, "glMultMatrixd")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glMultMatrixd()");
		return FALSE;
	}

	if ( (gl::qglMultMatrixf = (pfn_glMultMatrixf) GetProcAddress (hOpenGL, "glMultMatrixf")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glMultMatrixf()");
		return FALSE;
	}

	if ( (gl::qglNewList = (pfn_glNewList) GetProcAddress (hOpenGL, "glNewList")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glNewList()");
		return FALSE;
	}

	if ( (gl::qglNormal3b = (pfn_glNormal3b) GetProcAddress (hOpenGL, "glNormal3b")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glNormal3b()");
		return FALSE;
	}

	if ( (gl::qglNormal3bv = (pfn_glNormal3bv) GetProcAddress (hOpenGL, "glNormal3bv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glNormal3bv()");
		return FALSE;
	}

	if ( (gl::qglNormal3d = (pfn_glNormal3d) GetProcAddress (hOpenGL, "glNormal3d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glNormal3d()");
		return FALSE;
	}

	if ( (gl::qglNormal3dv = (pfn_glNormal3dv) GetProcAddress (hOpenGL, "glNormal3dv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glNormal3dv()");
		return FALSE;
	}

	if ( (gl::qglNormal3f = (pfn_glNormal3f) GetProcAddress (hOpenGL, "glNormal3f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glNormal3f()");
		return FALSE;
	}

	if ( (gl::qglNormal3fv = (pfn_glNormal3fv) GetProcAddress (hOpenGL, "glNormal3fv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glNormal3fv()");
		return FALSE;
	}

	if ( (gl::qglNormal3i = (pfn_glNormal3i) GetProcAddress (hOpenGL, "glNormal3i")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glNormal3i()");
		return FALSE;
	}

	if ( (gl::qglNormal3iv = (pfn_glNormal3iv) GetProcAddress (hOpenGL, "glNormal3iv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glNormal3iv()");
		return FALSE;
	}

	if ( (gl::qglNormal3s = (pfn_glNormal3s) GetProcAddress (hOpenGL, "glNormal3s")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glNormal3s()");
		return FALSE;
	}

	if ( (gl::qglNormal3sv = (pfn_glNormal3sv) GetProcAddress (hOpenGL, "glNormal3sv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glNormal3sv()");
		return FALSE;
	}

	if ( (gl::qglNormalPointer = (pfn_glNormalPointer) GetProcAddress (hOpenGL, "glNormalPointer")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glNormalPointer()");
		return FALSE;
	}

	if ( (gl::qglOrtho = (pfn_glOrtho) GetProcAddress (hOpenGL, "glOrtho")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glOrtho()");
		return FALSE;
	}

	if ( (gl::qglPassThrough = (pfn_glPassThrough) GetProcAddress (hOpenGL, "glPassThrough")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPassThrough()");
		return FALSE;
	}

	if ( (gl::qglPixelMapfv = (pfn_glPixelMapfv) GetProcAddress (hOpenGL, "glPixelMapfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPixelMapfv()");
		return FALSE;
	}

	if ( (gl::qglPixelMapuiv = (pfn_glPixelMapuiv) GetProcAddress (hOpenGL, "glPixelMapuiv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPixelMapuiv()");
		return FALSE;
	}

	if ( (gl::qglPixelMapusv = (pfn_glPixelMapusv) GetProcAddress (hOpenGL, "glPixelMapusv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPixelMapusv()");
		return FALSE;
	}

	if ( (gl::qglPixelStoref = (pfn_glPixelStoref) GetProcAddress (hOpenGL, "glPixelStoref")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPixelStoref()");
		return FALSE;
	}

	if ( (gl::qglPixelStorei = (pfn_glPixelStorei) GetProcAddress (hOpenGL, "glPixelStorei")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPixelStorei()");
		return FALSE;
	}

	if ( (gl::qglPixelTransferf = (pfn_glPixelTransferf) GetProcAddress (hOpenGL, "glPixelTransferf")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPixelTransferf()");
		return FALSE;
	}

	if ( (gl::qglPixelTransferi = (pfn_glPixelTransferi) GetProcAddress (hOpenGL, "glPixelTransferi")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPixelTransferi()");
		return FALSE;
	}

	if ( (gl::qglPixelZoom = (pfn_glPixelZoom) GetProcAddress (hOpenGL, "glPixelZoom")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPixelZoom()");
		return FALSE;
	}

	if ( (gl::qglPointSize = (pfn_glPointSize) GetProcAddress (hOpenGL, "glPointSize")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPointSize()");
		return FALSE;
	}

	if ( (gl::qglPolygonMode = (pfn_glPolygonMode) GetProcAddress (hOpenGL, "glPolygonMode")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPolygonMode()");
		return FALSE;
	}

	if ( (gl::qglPolygonOffset = (pfn_glPolygonOffset) GetProcAddress (hOpenGL, "glPolygonOffset")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPolygonOffset()");
		return FALSE;
	}

	if ( (gl::qglPolygonStipple = (pfn_glPolygonStipple) GetProcAddress (hOpenGL, "glPolygonStipple")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPolygonStipple()");
		return FALSE;
	}

	if ( (gl::qglPopAttrib = (pfn_glPopAttrib) GetProcAddress (hOpenGL, "glPopAttrib")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPopAttrib()");
		return FALSE;
	}

	if ( (gl::qglPopClientAttrib = (pfn_glPopClientAttrib) GetProcAddress (hOpenGL, "glPopClientAttrib")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPopClientAttrib()");
		return FALSE;
	}

	if ( (gl::qglPopMatrix = (pfn_glPopMatrix) GetProcAddress (hOpenGL, "glPopMatrix")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPopMatrix()");
		return FALSE;
	}

	if ( (gl::qglPopName = (pfn_glPopName) GetProcAddress (hOpenGL, "glPopName")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPopName()");
		return FALSE;
	}

	if ( (gl::qglPrioritizeTextures = (pfn_glPrioritizeTextures) GetProcAddress (hOpenGL, "glPrioritizeTextures")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPrioritizeTextures()");
		return FALSE;
	}

	if ( (gl::qglPushAttrib = (pfn_glPushAttrib) GetProcAddress (hOpenGL, "glPushAttrib")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPushAttrib()");
		return FALSE;
	}

	if ( (gl::qglPushClientAttrib = (pfn_glPushClientAttrib) GetProcAddress (hOpenGL, "glPushClientAttrib")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPushClientAttrib()");
		return FALSE;
	}

	if ( (gl::qglPushMatrix = (pfn_glPushMatrix) GetProcAddress (hOpenGL, "glPushMatrix")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPushMatrix()");
		return FALSE;
	}

	if ( (gl::qglPushName = (pfn_glPushName) GetProcAddress (hOpenGL, "glPushName")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glPushName()");
		return FALSE;
	}

	if ( (gl::qglRasterPos2d = (pfn_glRasterPos2d) GetProcAddress (hOpenGL, "glRasterPos2d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos2d()");
		return FALSE;
	}

	if ( (gl::qglRasterPos2dv = (pfn_glRasterPos2dv) GetProcAddress (hOpenGL, "glRasterPos2dv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos2dv()");
		return FALSE;
	}

	if ( (gl::qglRasterPos2f = (pfn_glRasterPos2f) GetProcAddress (hOpenGL, "glRasterPos2f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos2f()");
		return FALSE;
	}

	if ( (gl::qglRasterPos2fv = (pfn_glRasterPos2fv) GetProcAddress (hOpenGL, "glRasterPos2fv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos2fv()");
		return FALSE;
	}

	if ( (gl::qglRasterPos2i = (pfn_glRasterPos2i) GetProcAddress (hOpenGL, "glRasterPos2i")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos2i()");
		return FALSE;
	}

	if ( (gl::qglRasterPos2iv = (pfn_glRasterPos2iv) GetProcAddress (hOpenGL, "glRasterPos2iv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos2iv()");
		return FALSE;
	}

	if ( (gl::qglRasterPos2s = (pfn_glRasterPos2s) GetProcAddress (hOpenGL, "glRasterPos2s")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos2s()");
		return FALSE;
	}

	if ( (gl::qglRasterPos2sv = (pfn_glRasterPos2sv) GetProcAddress (hOpenGL, "glRasterPos2sv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos2sv()");
		return FALSE;
	}

	if ( (gl::qglRasterPos3d = (pfn_glRasterPos3d) GetProcAddress (hOpenGL, "glRasterPos3d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos3d()");
		return FALSE;
	}

	if ( (gl::qglRasterPos3dv = (pfn_glRasterPos3dv) GetProcAddress (hOpenGL, "glRasterPos3dv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos3dv()");
		return FALSE;
	}

	if ( (gl::qglRasterPos3f = (pfn_glRasterPos3f) GetProcAddress (hOpenGL, "glRasterPos3f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos3f()");
		return FALSE;
	}

	if ( (gl::qglRasterPos3fv = (pfn_glRasterPos3fv) GetProcAddress (hOpenGL, "glRasterPos3fv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos3fv()");
		return FALSE;
	}

	if ( (gl::qglRasterPos3i = (pfn_glRasterPos3i) GetProcAddress (hOpenGL, "glRasterPos3i")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos3i()");
		return FALSE;
	}

	if ( (gl::qglRasterPos3iv = (pfn_glRasterPos3iv) GetProcAddress (hOpenGL, "glRasterPos3iv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos3iv()");
		return FALSE;
	}

	if ( (gl::qglRasterPos3s = (pfn_glRasterPos3s) GetProcAddress (hOpenGL, "glRasterPos3s")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos3s()");
		return FALSE;
	}

	if ( (gl::qglRasterPos3sv = (pfn_glRasterPos3sv) GetProcAddress (hOpenGL, "glRasterPos3sv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos3sv()");
		return FALSE;
	}

	if ( (gl::qglRasterPos4d = (pfn_glRasterPos4d) GetProcAddress (hOpenGL, "glRasterPos4d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos4d()");
		return FALSE;
	}

	if ( (gl::qglRasterPos4dv = (pfn_glRasterPos4dv) GetProcAddress (hOpenGL, "glRasterPos4dv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos4dv()");
		return FALSE;
	}

	if ( (gl::qglRasterPos4f = (pfn_glRasterPos4f) GetProcAddress (hOpenGL, "glRasterPos4f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos4f()");
		return FALSE;
	}

	if ( (gl::qglRasterPos4fv = (pfn_glRasterPos4fv) GetProcAddress (hOpenGL, "glRasterPos4fv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos4fv()");
		return FALSE;
	}

	if ( (gl::qglRasterPos4i = (pfn_glRasterPos4i) GetProcAddress (hOpenGL, "glRasterPos4i")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos4i()");
		return FALSE;
	}

	if ( (gl::qglRasterPos4iv = (pfn_glRasterPos4iv) GetProcAddress (hOpenGL, "glRasterPos4iv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos4iv()");
		return FALSE;
	}

	if ( (gl::qglRasterPos4s = (pfn_glRasterPos4s) GetProcAddress (hOpenGL, "glRasterPos4s")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos4s()");
		return FALSE;
	}

	if ( (gl::qglRasterPos4sv = (pfn_glRasterPos4sv) GetProcAddress (hOpenGL, "glRasterPos4sv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRasterPos4sv()");
		return FALSE;
	}

	if ( (gl::qglReadBuffer = (pfn_glReadBuffer) GetProcAddress (hOpenGL, "glReadBuffer")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glReadBuffer()");
		return FALSE;
	}

	if ( (gl::qglReadPixels = (pfn_glReadPixels) GetProcAddress (hOpenGL, "glReadPixels")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glReadPixels()");
		return FALSE;
	}

	if ( (gl::qglRectd = (pfn_glRectd) GetProcAddress (hOpenGL, "glRectd")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRectd()");
		return FALSE;
	}

	if ( (gl::qglRectdv = (pfn_glRectdv) GetProcAddress (hOpenGL, "glRectdv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRectdv()");
		return FALSE;
	}

	if ( (gl::qglRectf = (pfn_glRectf) GetProcAddress (hOpenGL, "glRectf")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRectf()");
		return FALSE;
	}

	if ( (gl::qglRectfv = (pfn_glRectfv) GetProcAddress (hOpenGL, "glRectfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRectfv()");
		return FALSE;
	}

	if ( (gl::qglRecti = (pfn_glRecti) GetProcAddress (hOpenGL, "glRecti")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRecti()");
		return FALSE;
	}

	if ( (gl::qglRectiv = (pfn_glRectiv) GetProcAddress (hOpenGL, "glRectiv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRectiv()");
		return FALSE;
	}

	if ( (gl::qglRects = (pfn_glRects) GetProcAddress (hOpenGL, "glRects")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRects()");
		return FALSE;
	}

	if ( (gl::qglRectsv = (pfn_glRectsv) GetProcAddress (hOpenGL, "glRectsv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRectsv()");
		return FALSE;
	}

	if ( (gl::qglRenderMode = (pfn_glRenderMode) GetProcAddress (hOpenGL, "glRenderMode")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRenderMode()");
		return FALSE;
	}

	if ( (gl::qglRotated = (pfn_glRotated) GetProcAddress (hOpenGL, "glRotated")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRotated()");
		return FALSE;
	}

	if ( (gl::qglRotatef = (pfn_glRotatef) GetProcAddress (hOpenGL, "glRotatef")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glRotatef()");
		return FALSE;
	}

	if ( (gl::qglScaled = (pfn_glScaled) GetProcAddress (hOpenGL, "glScaled")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glScaled()");
		return FALSE;
	}

	if ( (gl::qglScalef = (pfn_glScalef) GetProcAddress (hOpenGL, "glScalef")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glScalef()");
		return FALSE;
	}

	if ( (gl::qglScissor = (pfn_glScissor) GetProcAddress (hOpenGL, "glScissor")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glScissor()");
		return FALSE;
	}

	if ( (gl::qglSelectBuffer = (pfn_glSelectBuffer) GetProcAddress (hOpenGL, "glSelectBuffer")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glSelectBuffer()");
		return FALSE;
	}

	if ( (gl::qglShadeModel = (pfn_glShadeModel) GetProcAddress (hOpenGL, "glShadeModel")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glShadeModel()");
		return FALSE;
	}

	if ( (gl::qglStencilFunc = (pfn_glStencilFunc) GetProcAddress (hOpenGL, "glStencilFunc")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glStencilFunc()");
		return FALSE;
	}

	if ( (gl::qglStencilMask = (pfn_glStencilMask) GetProcAddress (hOpenGL, "glStencilMask")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glStencilMask()");
		return FALSE;
	}

	if ( (gl::qglStencilOp = (pfn_glStencilOp) GetProcAddress (hOpenGL, "glStencilOp")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glStencilOp()");
		return FALSE;
	}

	if ( (gl::wglSwapBuffers = (pfn_wglSwapBuffers) GetProcAddress (hOpenGL, "wglSwapBuffers")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glStencilOp()");
		return FALSE;
	}

	if ( (gl::qglTexCoord1d = (pfn_glTexCoord1d) GetProcAddress (hOpenGL, "glTexCoord1d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord1d()");
		return FALSE;
	}

	if ( (gl::qglTexCoord1dv = (pfn_glTexCoord1dv) GetProcAddress (hOpenGL, "glTexCoord1dv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord1dv()");
		return FALSE;
	}

	if ( (gl::qglTexCoord1f = (pfn_glTexCoord1f) GetProcAddress (hOpenGL, "glTexCoord1f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord1f()");
		return FALSE;
	}

	if ( (gl::qglTexCoord1fv = (pfn_glTexCoord1fv) GetProcAddress (hOpenGL, "glTexCoord1fv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord1fv()");
		return FALSE;
	}

	if ( (gl::qglTexCoord1i = (pfn_glTexCoord1i) GetProcAddress (hOpenGL, "glTexCoord1i")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord1i()");
		return FALSE;
	}

	if ( (gl::qglTexCoord1iv = (pfn_glTexCoord1iv) GetProcAddress (hOpenGL, "glTexCoord1iv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord1iv()");
		return FALSE;
	}

	if ( (gl::qglTexCoord1s = (pfn_glTexCoord1s) GetProcAddress (hOpenGL, "glTexCoord1s")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord1s()");
		return FALSE;
	}

	if ( (gl::qglTexCoord1sv = (pfn_glTexCoord1sv) GetProcAddress (hOpenGL, "glTexCoord1sv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord1sv()");
		return FALSE;
	}

	if ( (gl::qglTexCoord2d = (pfn_glTexCoord2d) GetProcAddress (hOpenGL, "glTexCoord2d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord2d()");
		return FALSE;
	}

	if ( (gl::qglTexCoord2dv = (pfn_glTexCoord2dv) GetProcAddress (hOpenGL, "glTexCoord2dv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord2dv()");
		return FALSE;
	}

	if ( (gl::qglTexCoord2f = (pfn_glTexCoord2f) GetProcAddress (hOpenGL, "glTexCoord2f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord2f()");
		return FALSE;
	}

	if ( (gl::qglTexCoord2fv = (pfn_glTexCoord2fv) GetProcAddress (hOpenGL, "glTexCoord2fv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord2fv()");
		return FALSE;
	}

	if ( (gl::qglTexCoord2i = (pfn_glTexCoord2i) GetProcAddress (hOpenGL, "glTexCoord2i")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord2i()");
		return FALSE;
	}

	if ( (gl::qglTexCoord2iv = (pfn_glTexCoord2iv) GetProcAddress (hOpenGL, "glTexCoord2iv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord2iv()");
		return FALSE;
	}

	if ( (gl::qglTexCoord2s = (pfn_glTexCoord2s) GetProcAddress (hOpenGL, "glTexCoord2s")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord2s()");
		return FALSE;
	}

	if ( (gl::qglTexCoord2sv = (pfn_glTexCoord2sv) GetProcAddress (hOpenGL, "glTexCoord2sv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord2sv()");
		return FALSE;
	}

	if ( (gl::qglTexCoord3d = (pfn_glTexCoord3d) GetProcAddress (hOpenGL, "glTexCoord3d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord3d()");
		return FALSE;
	}

	if ( (gl::qglTexCoord3dv = (pfn_glTexCoord3dv) GetProcAddress (hOpenGL, "glTexCoord3dv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord3dv()");
		return FALSE;
	}

	if ( (gl::qglTexCoord3f = (pfn_glTexCoord3f) GetProcAddress (hOpenGL, "glTexCoord3f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord3f()");
		return FALSE;
	}

	if ( (gl::qglTexCoord3fv = (pfn_glTexCoord3fv) GetProcAddress (hOpenGL, "glTexCoord3fv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord3fv()");
		return FALSE;
	}

	if ( (gl::qglTexCoord3i = (pfn_glTexCoord3i) GetProcAddress (hOpenGL, "glTexCoord3i")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord3i()");
		return FALSE;
	}

	if ( (gl::qglTexCoord3iv = (pfn_glTexCoord3iv) GetProcAddress (hOpenGL, "glTexCoord3iv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord3iv()");
		return FALSE;
	}

	if ( (gl::qglTexCoord3s = (pfn_glTexCoord3s) GetProcAddress (hOpenGL, "glTexCoord3s")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord3s()");
		return FALSE;
	}

	if ( (gl::qglTexCoord3sv = (pfn_glTexCoord3sv) GetProcAddress (hOpenGL, "glTexCoord3sv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord3sv()");
		return FALSE;
	}

	if ( (gl::qglTexCoord4d = (pfn_glTexCoord4d) GetProcAddress (hOpenGL, "glTexCoord4d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord4d()");
		return FALSE;
	}

	if ( (gl::qglTexCoord4dv = (pfn_glTexCoord4dv) GetProcAddress (hOpenGL, "glTexCoord4dv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord4dv()");
		return FALSE;
	}

	if ( (gl::qglTexCoord4f = (pfn_glTexCoord4f) GetProcAddress (hOpenGL, "glTexCoord4f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord4f()");
		return FALSE;
	}

	if ( (gl::qglTexCoord4fv = (pfn_glTexCoord4fv) GetProcAddress (hOpenGL, "glTexCoord4fv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord4fv()");
		return FALSE;
	}

	if ( (gl::qglTexCoord4i = (pfn_glTexCoord4i) GetProcAddress (hOpenGL, "glTexCoord4i")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord4i()");
		return FALSE;
	}

	if ( (gl::qglTexCoord4iv = (pfn_glTexCoord4iv) GetProcAddress (hOpenGL, "glTexCoord4iv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord4iv()");
		return FALSE;
	}

	if ( (gl::qglTexCoord4s = (pfn_glTexCoord4s) GetProcAddress (hOpenGL, "glTexCoord4s")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord4s()");
		return FALSE;
	}

	if ( (gl::qglTexCoord4sv = (pfn_glTexCoord4sv) GetProcAddress (hOpenGL, "glTexCoord4sv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoord4sv()");
		return FALSE;
	}

	if ( (gl::qglTexCoordPointer = (pfn_glTexCoordPointer) GetProcAddress (hOpenGL, "glTexCoordPointer")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexCoordPointer()");
		return FALSE;
	}

	if ( (gl::qglTexEnvf = (pfn_glTexEnvf) GetProcAddress (hOpenGL, "glTexEnvf")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexEnvf()");
		return FALSE;
	}

	if ( (gl::qglTexEnvfv = (pfn_glTexEnvfv) GetProcAddress (hOpenGL, "glTexEnvfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexEnvfv()");
		return FALSE;
	}

	if ( (gl::qglTexEnvi = (pfn_glTexEnvi) GetProcAddress (hOpenGL, "glTexEnvi")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexEnvi()");
		return FALSE;
	}

	if ( (gl::qglTexEnviv = (pfn_glTexEnviv) GetProcAddress (hOpenGL, "glTexEnviv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexEnviv()");
		return FALSE;
	}

	if ( (gl::qglTexGend = (pfn_glTexGend) GetProcAddress (hOpenGL, "glTexGend")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexGend()");
		return FALSE;
	}

	if ( (gl::qglTexGendv = (pfn_glTexGendv) GetProcAddress (hOpenGL, "glTexGendv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexGendv()");
		return FALSE;
	}

	if ( (gl::qglTexGenf = (pfn_glTexGenf) GetProcAddress (hOpenGL, "glTexGenf")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexGenf()");
		return FALSE;
	}

	if ( (gl::qglTexGenfv = (pfn_glTexGenfv) GetProcAddress (hOpenGL, "glTexGenfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexGenfv()");
		return FALSE;
	}

	if ( (gl::qglTexGeni = (pfn_glTexGeni) GetProcAddress (hOpenGL, "glTexGeni")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexGeni()");
		return FALSE;
	}

	if ( (gl::qglTexGeniv = (pfn_glTexGeniv) GetProcAddress (hOpenGL, "glTexGeniv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexGeniv()");
		return FALSE;
	}

	if ( (gl::qglTexImage1D = (pfn_glTexImage1D) GetProcAddress (hOpenGL, "glTexImage1D")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexImage1D()");
		return FALSE;
	}

	if ( (gl::qglTexImage2D = (pfn_glTexImage2D) GetProcAddress (hOpenGL, "glTexImage2D")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexImage2D()");
		return FALSE;
	}

	if ( (gl::qglTexParameterf = (pfn_glTexParameterf) GetProcAddress (hOpenGL, "glTexParameterf")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexParameterf()");
		return FALSE;
	}

	if ( (gl::qglTexParameterfv = (pfn_glTexParameterfv) GetProcAddress (hOpenGL, "glTexParameterfv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexParameterfv()");
		return FALSE;
	}

	if ( (gl::qglTexParameteri = (pfn_glTexParameteri) GetProcAddress (hOpenGL, "glTexParameteri")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexParameteri()");
		return FALSE;
	}

	if ( (gl::qglTexParameteriv = (pfn_glTexParameteriv) GetProcAddress (hOpenGL, "glTexParameteriv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexParameteriv()");
		return FALSE;
	}

	if ( (gl::qglTexSubImage1D = (pfn_glTexSubImage1D) GetProcAddress (hOpenGL, "glTexSubImage1D")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexSubImage1D()");
		return FALSE;
	}

	if ( (gl::qglTexSubImage2D = (pfn_glTexSubImage2D) GetProcAddress (hOpenGL, "glTexSubImage2D")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTexSubImage2D()");
		return FALSE;
	}

	if ( (gl::qglTranslated = (pfn_glTranslated) GetProcAddress (hOpenGL, "glTranslated")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTranslated()");
		return FALSE;
	}

	if ( (gl::qglTranslatef = (pfn_glTranslatef) GetProcAddress (hOpenGL, "glTranslatef")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glTranslatef()");
		return FALSE;
	}

	if ( (gl::qglVertex2d = (pfn_glVertex2d) GetProcAddress (hOpenGL, "glVertex2d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex2d()");
		return FALSE;
	}

	if ( (gl::qglVertex2dv = (pfn_glVertex2dv) GetProcAddress (hOpenGL, "glVertex2dv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex2dv()");
		return FALSE;
	}

	if ( (gl::qglVertex2f = (pfn_glVertex2f) GetProcAddress (hOpenGL, "glVertex2f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex2f()");
		return FALSE;
	}

	if ( (gl::qglVertex2fv = (pfn_glVertex2fv) GetProcAddress (hOpenGL, "glVertex2fv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex2fv()");
		return FALSE;
	}

	if ( (gl::qglVertex2i = (pfn_glVertex2i) GetProcAddress (hOpenGL, "glVertex2i")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex2i()");
		return FALSE;
	}

	if ( (gl::qglVertex2iv = (pfn_glVertex2iv) GetProcAddress (hOpenGL, "glVertex2iv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex2iv()");
		return FALSE;
	}

	if ( (gl::qglVertex2s = (pfn_glVertex2s) GetProcAddress (hOpenGL, "glVertex2s")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex2s()");
		return FALSE;
	}

	if ( (gl::qglVertex2sv = (pfn_glVertex2sv) GetProcAddress (hOpenGL, "glVertex2sv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex2sv()");
		return FALSE;
	}

	if ( (gl::qglVertex3d = (pfn_glVertex3d) GetProcAddress (hOpenGL, "glVertex3d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex3d()");
		return FALSE;
	}

	if ( (gl::qglVertex3dv = (pfn_glVertex3dv) GetProcAddress (hOpenGL, "glVertex3dv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex3dv()");
		return FALSE;
	}

	if ( (gl::qglVertex3f = (pfn_glVertex3f) GetProcAddress (hOpenGL, "glVertex3f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex3f()");
		return FALSE;
	}

	if ( (gl::qglVertex3fv = (pfn_glVertex3fv) GetProcAddress (hOpenGL, "glVertex3fv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex3fv()");
		return FALSE;
	}

	if ( (gl::qglVertex3i = (pfn_glVertex3i) GetProcAddress (hOpenGL, "glVertex3i")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex3i()");
		return FALSE;
	}

	if ( (gl::qglVertex3iv = (pfn_glVertex3iv) GetProcAddress (hOpenGL, "glVertex3iv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex3iv()");
		return FALSE;
	}

	if ( (gl::qglVertex3s = (pfn_glVertex3s) GetProcAddress (hOpenGL, "glVertex3s")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex3s()");
		return FALSE;
	}

	if ( (gl::qglVertex3sv = (pfn_glVertex3sv) GetProcAddress (hOpenGL, "glVertex3sv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex3sv()");
		return FALSE;
	}

	if ( (gl::qglVertex4d = (pfn_glVertex4d) GetProcAddress (hOpenGL, "glVertex4d")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex4d()");
		return FALSE;
	}

	if ( (gl::qglVertex4dv = (pfn_glVertex4dv) GetProcAddress (hOpenGL, "glVertex4dv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex4dv()");
		return FALSE;
	}

	if ( (gl::qglVertex4f = (pfn_glVertex4f) GetProcAddress (hOpenGL, "glVertex4f")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex4f()");
		return FALSE;
	}

	if ( (gl::qglVertex4fv = (pfn_glVertex4fv) GetProcAddress (hOpenGL, "glVertex4fv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex4fv()");
		return FALSE;
	}

	if ( (gl::qglVertex4i = (pfn_glVertex4i) GetProcAddress (hOpenGL, "glVertex4i")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex4i()");
		return FALSE;
	}

	if ( (gl::qglVertex4iv = (pfn_glVertex4iv) GetProcAddress (hOpenGL, "glVertex4iv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex4iv()");
		return FALSE;
	}

	if ( (gl::qglVertex4s = (pfn_glVertex4s) GetProcAddress (hOpenGL, "glVertex4s")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex4s()");
		return FALSE;
	}

	if ( (gl::qglVertex4sv = (pfn_glVertex4sv) GetProcAddress (hOpenGL, "glVertex4sv")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertex4sv()");
		return FALSE;
	}

	if ( (gl::qglVertexPointer = (pfn_glVertexPointer) GetProcAddress (hOpenGL, "glVertexPointer")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glVertexPointer()");
		return FALSE;
	}

	if ( (gl::qglViewport = (pfn_glViewport) GetProcAddress (hOpenGL, "glViewport")) == NULL )
	{
		//add_log ("Couldn't found a prototype for glViewport()");
		return FALSE;
	}

	if ( (gl::wglChoosePixelFormat = (pfn_wglChoosePixelFormat) GetProcAddress (hOpenGL, "wglChoosePixelFormat")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglChoosePixelFormat()");
		return FALSE;
	}

	if ( (gl::wglCopyContext = (pfn_wglCopyContext) GetProcAddress (hOpenGL, "wglCopyContext")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglCopyContext()");
		return FALSE;
	}

	if ( (gl::wglCreateContext = (pfn_wglCreateContext) GetProcAddress (hOpenGL, "wglCreateContext")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglCreateContext()");
		return FALSE;
	}

	if ( (gl::wglCreateLayerContext = (pfn_wglCreateLayerContext) GetProcAddress (hOpenGL, "wglCreateLayerContext")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglCreateLayerContext()");
		return FALSE;
	}

	if ( (gl::wglDeleteContext = (pfn_wglDeleteContext) GetProcAddress (hOpenGL, "wglDeleteContext")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglDeleteContext()");
		return FALSE;
	}

	if ( (gl::wglDescribeLayerPlane = (pfn_wglDescribeLayerPlane) GetProcAddress (hOpenGL, "wglDescribeLayerPlane")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglDescribeLayerPlane()");
		return FALSE;
	}

	if ( (gl::wglDescribePixelFormat = (pfn_wglDescribePixelFormat) GetProcAddress (hOpenGL, "wglDescribePixelFormat")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglDescribePixelFormat()");
		return FALSE;
	}

	if ( (gl::wglGetCurrentContext = (pfn_wglGetCurrentContext) GetProcAddress (hOpenGL, "wglGetCurrentContext")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglGetCurrentContext()");
		return FALSE;
	}

	if ( (gl::wglGetCurrentDC = (pfn_wglGetCurrentDC) GetProcAddress (hOpenGL, "wglGetCurrentDC")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglGetCurrentDC()");
		return FALSE;
	}

	if ( (gl::wglGetLayerPaletteEntries = (pfn_wglGetLayerPaletteEntries) GetProcAddress (hOpenGL, "wglGetLayerPaletteEntries")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglGetLayerPaletteEntries()");
		return FALSE;
	}

	if ( (gl::wglGetPixelFormat = (pfn_wglGetPixelFormat) GetProcAddress (hOpenGL, "wglGetPixelFormat")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglGetPixelFormat()");
		return FALSE;
	}

	if ( (gl::wglGetProcAddress = (pfn_wglGetProcAddress) GetProcAddress (hOpenGL, "wglGetProcAddress")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglGetProcAddress()");
		return FALSE;
	}

	if ( (gl::wglMakeCurrent = (pfn_wglMakeCurrent) GetProcAddress (hOpenGL, "wglMakeCurrent")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglMakeCurrent()");
		return FALSE;
	}

	if ( (gl::wglRealizeLayerPalette = (pfn_wglRealizeLayerPalette) GetProcAddress (hOpenGL, "wglRealizeLayerPalette")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglRealizeLayerPalette()");
		return FALSE;
	}

	if ( (gl::wglSetLayerPaletteEntries = (pfn_wglSetLayerPaletteEntries) GetProcAddress (hOpenGL, "wglSetLayerPaletteEntries")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglSetLayerPaletteEntries()");
		return FALSE;
	}

	if ( (gl::wglSetPixelFormat = (pfn_wglSetPixelFormat) GetProcAddress (hOpenGL, "wglSetPixelFormat")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglSetPixelFormat()");
		return FALSE;
	}

	if ( (gl::wglShareLists = (pfn_wglShareLists) GetProcAddress (hOpenGL, "wglShareLists")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglShareLists()");
		return FALSE;
	}

	if ( (gl::wglSwapBuffers = (pfn_wglSwapBuffers) GetProcAddress (hOpenGL, "wglSwapBuffers")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglSwapBuffers()");
		return FALSE;
	}

	if ( (gl::wglSwapLayerBuffers = (pfn_wglSwapLayerBuffers) GetProcAddress (hOpenGL, "wglSwapLayerBuffers")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglSwapLayerBuffers()");
		return FALSE;
	}

	if ( (gl::wglUseFontBitmapsA = (pfn_wglUseFontBitmapsA) GetProcAddress (hOpenGL, "wglUseFontBitmapsA")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglUseFontBitmapsA()");
		return FALSE;
	}

	if ( (gl::wglUseFontBitmapsW = (pfn_wglUseFontBitmapsW) GetProcAddress (hOpenGL, "wglUseFontBitmapsW")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglUseFontBitmapsW()");
		return FALSE;
	}

	if ( (gl::wglUseFontOutlinesA = (pfn_wglUseFontOutlinesA) GetProcAddress (hOpenGL, "wglUseFontOutlinesA")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglUseFontOutlinesA()");
		return FALSE;
	}

	if ( (gl::wglUseFontOutlinesW = (pfn_wglUseFontOutlinesW) GetProcAddress (hOpenGL, "wglUseFontOutlinesW")) == NULL )
	{
		//add_log ("Couldn't found a prototype for wglUseFontOutlinesW()");
		return FALSE;
	}
#endif

    return TRUE;
}




