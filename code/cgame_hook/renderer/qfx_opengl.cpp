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

#include "glb_local.h"

//=========================================
// OpenGL Wrapping Functions
//-----------------------------------------
// Forward calls to opengl32.dll
//=========================================

void WINAPI glDepthFunc( GLenum func );
void WINAPI glDepthRange( GLclampd zNear, GLclampd zFar );
void WINAPI glDrawElements_h( GLenum mode,  GLsizei count,  GLenum type,  const GLvoid *indices );
void WINAPI glEnable( GLenum cap );
void WINAPI glFrustum( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar );
const GLubyte * WINAPI glGetString( GLenum name );
void WINAPI glLoadMatrixd( const GLdouble *m );
void WINAPI glLoadMatrixf( const GLfloat *m );
void WINAPI glMatrixMode( GLenum mode );
void WINAPI glOrtho( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar );
void WINAPI glTexCoordPointer_h( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer );
void WINAPI glTexImage2D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
void WINAPI glTexParameterf( GLenum target, GLenum pname, GLfloat param );
void WINAPI glTexParameteri( GLenum target, GLenum pname, GLint param );
int WINAPI wrap_wglChoosePixelFormat( HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd );
BOOL WINAPI wrap_wglSetPixelFormat( HDC hdc, int i, CONST PIXELFORMATDESCRIPTOR *pfd );
BOOL WINAPI wrap_wglMakeCurrent( HDC hdc, HGLRC hglrc );
BOOL WINAPI wrap_wglDeleteContext( HGLRC hglrc );
BOOL WINAPI wrap_wglSwapBuffers( HDC hdc );
int WINAPI wrap_wglDescribePixelFormat (HDC hdc, int i, UINT j, LPPIXELFORMATDESCRIPTOR ppfd) { gl::CheckInit(); return gl::wglDescribePixelFormat( hdc, i, j, ppfd ); }
int WINAPI wrap_wglGetPixelFormat(HDC hdc) { gl::CheckInit(); return gl::wglGetPixelFormat( hdc ); }
BOOL WINAPI wrap_wglCopyContext(HGLRC hglrc, HGLRC hglrc2, UINT i) { gl::CheckInit(); return gl::wglCopyContext( hglrc, hglrc2, i ); }
HGLRC WINAPI wrap_wglCreateContext(HDC hdc) { gl::CheckInit(); return gl::wglCreateContext( hdc ); }
HGLRC WINAPI wrap_wglGetCurrentContext(VOID) { gl::CheckInit(); return gl::wglGetCurrentContext(); }
HDC WINAPI wrap_wglGetCurrentDC(VOID) { gl::CheckInit(); return gl::wglGetCurrentDC(); }
PROC WINAPI wrap_wglGetProcAddress(LPCSTR s) { return gl::wglGetProcAddress(s); }
BOOL WINAPI wrap_wglShareLists(HGLRC hglrc, HGLRC hglrc2) { gl::CheckInit(); return gl::wglShareLists(hglrc, hglrc2); }
HGLRC WINAPI wrap_wglCreateLayerContext( HDC hdc, int iLayerPlane ) { gl::CheckInit(); return gl::wglCreateLayerContext(hdc, iLayerPlane); }
BOOL WINAPI wrap_wglDescribeLayerPlane( HDC hdc, int iPixelFormat, int iLayerPlane, UINT nBytes, LPLAYERPLANEDESCRIPTOR plpd ) { gl::CheckInit(); return gl::wglDescribeLayerPlane(hdc, iPixelFormat, iLayerPlane, nBytes, plpd); }
int WINAPI wrap_wglGetLayerPaletteEntries( HDC hdc, int iLayerPlane, int iStart, int cEntries, COLORREF *pcr ) { gl::CheckInit(); return gl::wglGetLayerPaletteEntries(hdc, iLayerPlane, iStart, cEntries, pcr); }
int WINAPI wrap_wglSetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, CONST COLORREF *pcr) { gl::CheckInit(); return gl::wglSetLayerPaletteEntries(hdc, iLayerPlane, iStart, cEntries, pcr); }
BOOL WINAPI wrap_wglRealizeLayerPalette(HDC hdc, int iLayerPlane, BOOL b) { gl::CheckInit(); return gl::wglRealizeLayerPalette(hdc, iLayerPlane, b); }
BOOL WINAPI wrap_wglSwapLayerBuffers(HDC hdc, UINT i) { return gl::wglSwapLayerBuffers(hdc, i); }
BOOL WINAPI wrap_wglUseFontBitmapsA(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3) { gl::CheckInit(); return gl::wglUseFontBitmapsA(hdc, dw1, dw2, dw3); }
BOOL WINAPI wrap_wglUseFontBitmapsW(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3) { gl::CheckInit(); return gl::wglUseFontBitmapsW(hdc, dw1, dw2, dw3); }
BOOL WINAPI wrap_wglUseFontOutlinesA(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3, FLOAT f1, FLOAT f2, int i, LPGLYPHMETRICSFLOAT pgmf) { gl::CheckInit(); return gl::wglUseFontOutlinesA(hdc, dw1, dw2, dw3, f1, f2, i, pgmf); }
BOOL WINAPI wrap_wglUseFontOutlinesW(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3, FLOAT f1, FLOAT f2, int i, LPGLYPHMETRICSFLOAT pgmf) { gl::CheckInit(); return gl::wglUseFontOutlinesW(hdc, dw1, dw2, dw3, f1, f2, i, pgmf); }

namespace gl
{
pfn_glAccum qglAccum = NULL;
pfn_glAlphaFunc qglAlphaFunc = NULL;
pfn_glAreTexturesResident qglAreTexturesResident = NULL;
pfn_glArrayElement qglArrayElement = NULL;
pfn_glBegin qglBegin = NULL;
pfn_glBindTexture qglBindTexture = NULL;
pfn_glBitmap qglBitmap = NULL;
pfn_glBlendFunc qglBlendFunc = NULL;
pfn_glCallList qglCallList = NULL;
pfn_glCallLists qglCallLists = NULL;
pfn_glClear qglClear = NULL;
pfn_glClearAccum qglClearAccum = NULL;
pfn_glClearColor qglClearColor = NULL;
pfn_glClearDepth qglClearDepth = NULL;
pfn_glClearIndex qglClearIndex = NULL;
pfn_glClearStencil qglClearStencil = NULL;
pfn_glClipPlane qglClipPlane = NULL;
pfn_glColor3b qglColor3b = NULL;
pfn_glColor3bv qglColor3bv = NULL;
pfn_glColor3d qglColor3d = NULL;
pfn_glColor3dv qglColor3dv = NULL;
pfn_glColor3f qglColor3f = NULL;
pfn_glColor3fv qglColor3fv = NULL;
pfn_glColor3i qglColor3i = NULL;
pfn_glColor3iv qglColor3iv = NULL;
pfn_glColor3s qglColor3s = NULL;
pfn_glColor3sv qglColor3sv = NULL;
pfn_glColor3ub qglColor3ub = NULL;
pfn_glColor3ubv qglColor3ubv = NULL;
pfn_glColor3ui qglColor3ui = NULL;
pfn_glColor3uiv qglColor3uiv = NULL;
pfn_glColor3us qglColor3us = NULL;
pfn_glColor3usv qglColor3usv = NULL;
pfn_glColor4b qglColor4b = NULL;
pfn_glColor4bv qglColor4bv = NULL;
pfn_glColor4d qglColor4d = NULL;
pfn_glColor4dv qglColor4dv = NULL;
pfn_glColor4f qglColor4f = NULL;
pfn_glColor4fv qglColor4fv = NULL;
pfn_glColor4i qglColor4i = NULL;
pfn_glColor4iv qglColor4iv = NULL;
pfn_glColor4s qglColor4s = NULL;
pfn_glColor4sv qglColor4sv = NULL;
pfn_glColor4ub qglColor4ub = NULL;
pfn_glColor4ubv qglColor4ubv = NULL;
pfn_glColor4ui qglColor4ui = NULL;
pfn_glColor4uiv qglColor4uiv = NULL;
pfn_glColor4us qglColor4us = NULL;
pfn_glColor4usv qglColor4usv = NULL;
pfn_glColorMask qglColorMask = NULL;
pfn_glColorMaterial qglColorMaterial = NULL;
pfn_glColorPointer qglColorPointer = NULL;
pfn_glCopyPixels qglCopyPixels = NULL;
pfn_glCopyTexImage1D qglCopyTexImage1D = NULL;
pfn_glCopyTexImage2D qglCopyTexImage2D = NULL;
pfn_glCopyTexSubImage1D qglCopyTexSubImage1D = NULL;
pfn_glCopyTexSubImage2D qglCopyTexSubImage2D = NULL;
pfn_glCullFace qglCullFace = NULL;
pfn_glDeleteLists qglDeleteLists = NULL;
pfn_glDeleteTextures qglDeleteTextures = NULL;
pfn_glDepthFunc qglDepthFunc = NULL;
pfn_glDepthMask qglDepthMask = NULL;
pfn_glDepthRange qglDepthRange = NULL;
pfn_glDisable qglDisable = NULL;
pfn_glDisableClientState qglDisableClientState = NULL;
pfn_glDrawArrays qglDrawArrays = NULL;
pfn_glDrawBuffer qglDrawBuffer = NULL;
pfn_glDrawElements qglDrawElements = NULL;
pfn_glDrawPixels qglDrawPixels = NULL;
pfn_glEdgeFlag qglEdgeFlag = NULL;
pfn_glEdgeFlagPointer qglEdgeFlagPointer = NULL;
pfn_glEdgeFlagv qglEdgeFlagv = NULL;
pfn_glEnable qglEnable = NULL;
pfn_glEnableClientState qglEnableClientState = NULL;
pfn_glEnd qglEnd = NULL;
pfn_glEndList qglEndList = NULL;
pfn_glEvalCoord1d qglEvalCoord1d = NULL;
pfn_glEvalCoord1dv qglEvalCoord1dv = NULL;
pfn_glEvalCoord1f qglEvalCoord1f = NULL;
pfn_glEvalCoord1fv qglEvalCoord1fv = NULL;
pfn_glEvalCoord2d qglEvalCoord2d = NULL;
pfn_glEvalCoord2dv qglEvalCoord2dv = NULL;
pfn_glEvalCoord2f qglEvalCoord2f = NULL;
pfn_glEvalCoord2fv qglEvalCoord2fv = NULL;
pfn_glEvalMesh1 qglEvalMesh1 = NULL;
pfn_glEvalMesh2 qglEvalMesh2 = NULL;
pfn_glEvalPoint1 qglEvalPoint1 = NULL;
pfn_glEvalPoint2 qglEvalPoint2 = NULL;
pfn_glFeedbackBuffer qglFeedbackBuffer = NULL;
pfn_glFinish qglFinish = NULL;
pfn_glFlush qglFlush = NULL;
pfn_glFogf qglFogf = NULL;
pfn_glFogfv qglFogfv = NULL;
pfn_glFogi qglFogi = NULL;
pfn_glFogiv qglFogiv = NULL;
pfn_glFrontFace qglFrontFace = NULL;
pfn_glFrustum qglFrustum = NULL;
pfn_glGenLists qglGenLists = NULL;
pfn_glGenTextures qglGenTextures = NULL;
pfn_glGetBooleanv qglGetBooleanv = NULL;
pfn_glGetClipPlane qglGetClipPlane = NULL;
pfn_glGetDoublev qglGetDoublev = NULL;
pfn_glGetError qglGetError = NULL;
pfn_glGetFloatv qglGetFloatv = NULL;
pfn_glGetIntegerv qglGetIntegerv = NULL;
pfn_glGetLightfv qglGetLightfv = NULL;
pfn_glGetLightiv qglGetLightiv = NULL;
pfn_glGetMapdv qglGetMapdv = NULL;
pfn_glGetMapfv qglGetMapfv = NULL;
pfn_glGetMapiv qglGetMapiv = NULL;
pfn_glGetMaterialfv qglGetMaterialfv = NULL;
pfn_glGetMaterialiv qglGetMaterialiv = NULL;
pfn_glGetPixelMapfv qglGetPixelMapfv = NULL;
pfn_glGetPixelMapuiv qglGetPixelMapuiv = NULL;
pfn_glGetPixelMapusv qglGetPixelMapusv = NULL;
pfn_glGetPointerv qglGetPointerv = NULL;
pfn_glGetPolygonStipple qglGetPolygonStipple = NULL;
pfn_glGetString qglGetString = NULL;
pfn_glGetTexEnvfv qglGetTexEnvfv = NULL;
pfn_glGetTexEnviv qglGetTexEnviv = NULL;
pfn_glGetTexGendv qglGetTexGendv = NULL;
pfn_glGetTexGenfv qglGetTexGenfv = NULL;
pfn_glGetTexGeniv qglGetTexGeniv = NULL;
pfn_glGetTexImage qglGetTexImage = NULL;
pfn_glGetTexLevelParameterfv qglGetTexLevelParameterfv = NULL;
pfn_glGetTexLevelParameteriv qglGetTexLevelParameteriv = NULL;
pfn_glGetTexParameterfv qglGetTexParameterfv = NULL;
pfn_glGetTexParameteriv qglGetTexParameteriv = NULL;
pfn_glHint qglHint = NULL;
pfn_glIndexMask qglIndexMask = NULL;
pfn_glIndexPointer qglIndexPointer = NULL;
pfn_glIndexd qglIndexd = NULL;
pfn_glIndexdv qglIndexdv = NULL;
pfn_glIndexf qglIndexf = NULL;
pfn_glIndexfv qglIndexfv = NULL;
pfn_glIndexi qglIndexi = NULL;
pfn_glIndexiv qglIndexiv = NULL;
pfn_glIndexs qglIndexs = NULL;
pfn_glIndexsv qglIndexsv = NULL;
pfn_glIndexub qglIndexub = NULL;
pfn_glIndexubv qglIndexubv = NULL;
pfn_glInitNames qglInitNames = NULL;
pfn_glInterleavedArrays qglInterleavedArrays = NULL;
pfn_glIsEnabled qglIsEnabled = NULL;
pfn_glIsList qglIsList = NULL;
pfn_glIsTexture qglIsTexture = NULL;
pfn_glLightModelf qglLightModelf = NULL;
pfn_glLightModelfv qglLightModelfv = NULL;
pfn_glLightModeli qglLightModeli = NULL;
pfn_glLightModeliv qglLightModeliv = NULL;
pfn_glLightf qglLightf = NULL;
pfn_glLightfv qglLightfv = NULL;
pfn_glLighti qglLighti = NULL;
pfn_glLightiv qglLightiv = NULL;
pfn_glLineStipple qglLineStipple = NULL;
pfn_glLineWidth qglLineWidth = NULL;
pfn_glListBase qglListBase = NULL;
pfn_glLoadIdentity qglLoadIdentity = NULL;
pfn_glLoadMatrixd qglLoadMatrixd = NULL;
pfn_glLoadMatrixf qglLoadMatrixf = NULL;
pfn_glLoadName qglLoadName = NULL;
pfn_glLogicOp qglLogicOp = NULL;
pfn_glMap1d qglMap1d = NULL;
pfn_glMap1f qglMap1f = NULL;
pfn_glMap2d qglMap2d = NULL;
pfn_glMap2f qglMap2f = NULL;
pfn_glMapGrid1d qglMapGrid1d = NULL;
pfn_glMapGrid1f qglMapGrid1f = NULL;
pfn_glMapGrid2d qglMapGrid2d = NULL;
pfn_glMapGrid2f qglMapGrid2f = NULL;
pfn_glMaterialf qglMaterialf = NULL;
pfn_glMaterialfv qglMaterialfv = NULL;
pfn_glMateriali qglMateriali = NULL;
pfn_glMaterialiv qglMaterialiv = NULL;
pfn_glMatrixMode qglMatrixMode = NULL;
pfn_glMultMatrixd qglMultMatrixd = NULL;
pfn_glMultMatrixf qglMultMatrixf = NULL;
pfn_glNewList qglNewList = NULL;
pfn_glNormal3b qglNormal3b = NULL;
pfn_glNormal3bv qglNormal3bv = NULL;
pfn_glNormal3d qglNormal3d = NULL;
pfn_glNormal3dv qglNormal3dv = NULL;
pfn_glNormal3f qglNormal3f = NULL;
pfn_glNormal3fv qglNormal3fv = NULL;
pfn_glNormal3i qglNormal3i = NULL;
pfn_glNormal3iv qglNormal3iv = NULL;
pfn_glNormal3s qglNormal3s = NULL;
pfn_glNormal3sv qglNormal3sv = NULL;
pfn_glNormalPointer qglNormalPointer = NULL;
pfn_glOrtho qglOrtho = NULL;
pfn_glPassThrough qglPassThrough = NULL;
pfn_glPixelMapfv qglPixelMapfv = NULL;
pfn_glPixelMapuiv qglPixelMapuiv = NULL;
pfn_glPixelMapusv qglPixelMapusv = NULL;
pfn_glPixelStoref qglPixelStoref = NULL;
pfn_glPixelStorei qglPixelStorei = NULL;
pfn_glPixelTransferf qglPixelTransferf = NULL;
pfn_glPixelTransferi qglPixelTransferi = NULL;
pfn_glPixelZoom qglPixelZoom = NULL;
pfn_glPointSize qglPointSize = NULL;
pfn_glPolygonMode qglPolygonMode = NULL;
pfn_glPolygonOffset qglPolygonOffset = NULL;
pfn_glPolygonStipple qglPolygonStipple = NULL;
pfn_glPopAttrib qglPopAttrib = NULL;
pfn_glPopClientAttrib qglPopClientAttrib = NULL;
pfn_glPopMatrix qglPopMatrix = NULL;
pfn_glPopName qglPopName = NULL;
pfn_glPrioritizeTextures qglPrioritizeTextures = NULL;
pfn_glPushAttrib qglPushAttrib = NULL;
pfn_glPushClientAttrib qglPushClientAttrib = NULL;
pfn_glPushMatrix qglPushMatrix = NULL;
pfn_glPushName qglPushName = NULL;
pfn_glRasterPos2d qglRasterPos2d = NULL;
pfn_glRasterPos2dv qglRasterPos2dv = NULL;
pfn_glRasterPos2f qglRasterPos2f = NULL;
pfn_glRasterPos2fv qglRasterPos2fv = NULL;
pfn_glRasterPos2i qglRasterPos2i = NULL;
pfn_glRasterPos2iv qglRasterPos2iv = NULL;
pfn_glRasterPos2s qglRasterPos2s = NULL;
pfn_glRasterPos2sv qglRasterPos2sv = NULL;
pfn_glRasterPos3d qglRasterPos3d = NULL;
pfn_glRasterPos3dv qglRasterPos3dv = NULL;
pfn_glRasterPos3f qglRasterPos3f = NULL;
pfn_glRasterPos3fv qglRasterPos3fv = NULL;
pfn_glRasterPos3i qglRasterPos3i = NULL;
pfn_glRasterPos3iv qglRasterPos3iv = NULL;
pfn_glRasterPos3s qglRasterPos3s = NULL;
pfn_glRasterPos3sv qglRasterPos3sv = NULL;
pfn_glRasterPos4d qglRasterPos4d = NULL;
pfn_glRasterPos4dv qglRasterPos4dv = NULL;
pfn_glRasterPos4f qglRasterPos4f = NULL;
pfn_glRasterPos4fv qglRasterPos4fv = NULL;
pfn_glRasterPos4i qglRasterPos4i = NULL;
pfn_glRasterPos4iv qglRasterPos4iv = NULL;
pfn_glRasterPos4s qglRasterPos4s = NULL;
pfn_glRasterPos4sv qglRasterPos4sv = NULL;
pfn_glReadBuffer qglReadBuffer = NULL;
pfn_glReadPixels qglReadPixels = NULL;
pfn_glRectd qglRectd = NULL;
pfn_glRectdv qglRectdv = NULL;
pfn_glRectf qglRectf = NULL;
pfn_glRectfv qglRectfv = NULL;
pfn_glRecti qglRecti = NULL;
pfn_glRectiv qglRectiv = NULL;
pfn_glRects qglRects = NULL;
pfn_glRectsv qglRectsv = NULL;
pfn_glRenderMode qglRenderMode = NULL;
pfn_glRotated qglRotated = NULL;
pfn_glRotatef qglRotatef = NULL;
pfn_glScaled qglScaled = NULL;
pfn_glScalef qglScalef = NULL;
pfn_glScissor qglScissor = NULL;
pfn_glSelectBuffer qglSelectBuffer = NULL;
pfn_glShadeModel qglShadeModel = NULL;
pfn_glStencilFunc qglStencilFunc = NULL;
pfn_glStencilMask qglStencilMask = NULL;
pfn_glStencilOp qglStencilOp = NULL;
pfn_glTexCoord1d qglTexCoord1d = NULL;
pfn_glTexCoord1dv qglTexCoord1dv = NULL;
pfn_glTexCoord1f qglTexCoord1f = NULL;
pfn_glTexCoord1fv qglTexCoord1fv = NULL;
pfn_glTexCoord1i qglTexCoord1i = NULL;
pfn_glTexCoord1iv qglTexCoord1iv = NULL;
pfn_glTexCoord1s qglTexCoord1s = NULL;
pfn_glTexCoord1sv qglTexCoord1sv = NULL;
pfn_glTexCoord2d qglTexCoord2d = NULL;
pfn_glTexCoord2dv qglTexCoord2dv = NULL;
pfn_glTexCoord2f qglTexCoord2f = NULL;
pfn_glTexCoord2fv qglTexCoord2fv = NULL;
pfn_glTexCoord2i qglTexCoord2i = NULL;
pfn_glTexCoord2iv qglTexCoord2iv = NULL;
pfn_glTexCoord2s qglTexCoord2s = NULL;
pfn_glTexCoord2sv qglTexCoord2sv = NULL;
pfn_glTexCoord3d qglTexCoord3d = NULL;
pfn_glTexCoord3dv qglTexCoord3dv = NULL;
pfn_glTexCoord3f qglTexCoord3f = NULL;
pfn_glTexCoord3fv qglTexCoord3fv = NULL;
pfn_glTexCoord3i qglTexCoord3i = NULL;
pfn_glTexCoord3iv qglTexCoord3iv = NULL;
pfn_glTexCoord3s qglTexCoord3s = NULL;
pfn_glTexCoord3sv qglTexCoord3sv = NULL;
pfn_glTexCoord4d qglTexCoord4d = NULL;
pfn_glTexCoord4dv qglTexCoord4dv = NULL;
pfn_glTexCoord4f qglTexCoord4f = NULL;
pfn_glTexCoord4fv qglTexCoord4fv = NULL;
pfn_glTexCoord4i qglTexCoord4i = NULL;
pfn_glTexCoord4iv qglTexCoord4iv = NULL;
pfn_glTexCoord4s qglTexCoord4s = NULL;
pfn_glTexCoord4sv qglTexCoord4sv = NULL;
pfn_glTexCoordPointer qglTexCoordPointer = NULL;
pfn_glTexEnvf qglTexEnvf = NULL;
pfn_glTexEnvfv qglTexEnvfv = NULL;
pfn_glTexEnvi qglTexEnvi = NULL;
pfn_glTexEnviv qglTexEnviv = NULL;
pfn_glTexGend qglTexGend = NULL;
pfn_glTexGendv qglTexGendv = NULL;
pfn_glTexGenf qglTexGenf = NULL;
pfn_glTexGenfv qglTexGenfv = NULL;
pfn_glTexGeni qglTexGeni = NULL;
pfn_glTexGeniv qglTexGeniv = NULL;
pfn_glTexImage1D qglTexImage1D = NULL;
pfn_glTexImage2D qglTexImage2D = NULL;
pfn_glTexParameterf qglTexParameterf = NULL;
pfn_glTexParameterfv qglTexParameterfv = NULL;
pfn_glTexParameteri qglTexParameteri = NULL;
pfn_glTexParameteriv qglTexParameteriv = NULL;
pfn_glTexSubImage1D qglTexSubImage1D = NULL;
pfn_glTexSubImage2D qglTexSubImage2D = NULL;
pfn_glTranslated qglTranslated = NULL;
pfn_glTranslatef qglTranslatef = NULL;
pfn_glVertex2d qglVertex2d = NULL;
pfn_glVertex2dv qglVertex2dv = NULL;
pfn_glVertex2f qglVertex2f = NULL;
pfn_glVertex2fv qglVertex2fv = NULL;
pfn_glVertex2i qglVertex2i = NULL;
pfn_glVertex2iv qglVertex2iv = NULL;
pfn_glVertex2s qglVertex2s = NULL;
pfn_glVertex2sv qglVertex2sv = NULL;
pfn_glVertex3d qglVertex3d = NULL;
pfn_glVertex3dv qglVertex3dv = NULL;
pfn_glVertex3f qglVertex3f = NULL;
pfn_glVertex3fv qglVertex3fv = NULL;
pfn_glVertex3i qglVertex3i = NULL;
pfn_glVertex3iv qglVertex3iv = NULL;
pfn_glVertex3s qglVertex3s = NULL;
pfn_glVertex3sv qglVertex3sv = NULL;
pfn_glVertex4d qglVertex4d = NULL;
pfn_glVertex4dv qglVertex4dv = NULL;
pfn_glVertex4f qglVertex4f = NULL;
pfn_glVertex4fv qglVertex4fv = NULL;
pfn_glVertex4i qglVertex4i = NULL;
pfn_glVertex4iv qglVertex4iv = NULL;
pfn_glVertex4s qglVertex4s = NULL;
pfn_glVertex4sv qglVertex4sv = NULL;
pfn_glVertexPointer qglVertexPointer = NULL;
pfn_glViewport qglViewport = NULL;
pfn_wglChoosePixelFormat wglChoosePixelFormat = NULL;
pfn_wglDescribePixelFormat wglDescribePixelFormat = NULL;
pfn_wglGetPixelFormat wglGetPixelFormat = NULL;
pfn_wglSetPixelFormat wglSetPixelFormat = NULL;
pfn_wglSwapBuffers wglSwapBuffers = NULL;
pfn_wglCopyContext wglCopyContext = NULL;
pfn_wglCreateContext wglCreateContext = NULL;
pfn_wglDeleteContext wglDeleteContext = NULL;
pfn_wglGetCurrentContext wglGetCurrentContext = NULL;
pfn_wglGetCurrentDC wglGetCurrentDC = NULL;
pfn_wglGetProcAddress wglGetProcAddress = NULL;
pfn_wglMakeCurrent wglMakeCurrent = NULL;
pfn_wglShareLists wglShareLists = NULL;
pfn_wglCreateLayerContext wglCreateLayerContext = NULL;
pfn_wglDescribeLayerPlane wglDescribeLayerPlane = NULL;
pfn_wglGetLayerPaletteEntries wglGetLayerPaletteEntries = NULL;
pfn_wglSetLayerPaletteEntries wglSetLayerPaletteEntries = NULL;
pfn_wglRealizeLayerPalette wglRealizeLayerPalette = NULL;
pfn_wglSwapLayerBuffers wglSwapLayerBuffers = NULL;
pfn_wglUseFontBitmapsA wglUseFontBitmapsA = NULL;
pfn_wglUseFontBitmapsW wglUseFontBitmapsW = NULL;
pfn_wglUseFontOutlinesA wglUseFontOutlinesA = NULL;
pfn_wglUseFontOutlinesW wglUseFontOutlinesW = NULL;

// GL extensions
PFNGLGETHANDLEARBPROC qglGetHandleARB = NULL;
PFNGLACTIVETEXTUREARBPROC qglActiveTextureARB = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC qglClientActiveTextureARB = NULL;
PFNGLMULTITEXCOORD2FARBPROC qglMultiTexCoord2fARB = NULL;
PFNGLMULTITEXCOORD2FVARBPROC qglMultiTexCoord2fvARB = NULL;
PFNGLDELETEOBJECTARBPROC qglDeleteObjectARB = NULL;
PFNGLDETACHOBJECTARBPROC qglDetachObjectARB = NULL;
PFNGLCREATESHADEROBJECTARBPROC qglCreateShaderObjectARB = NULL;
PFNGLSHADERSOURCEARBPROC qglShaderSourceARB = NULL;
PFNGLCOMPILESHADERARBPROC qglCompileShaderARB = NULL;
PFNGLCREATEPROGRAMOBJECTARBPROC qglCreateProgramObjectARB = NULL;
PFNGLATTACHOBJECTARBPROC qglAttachObjectARB = NULL;
PFNGLLINKPROGRAMARBPROC	qglLinkProgramARB = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC qglUseProgramObjectARB = NULL;
PFNGLVALIDATEPROGRAMARBPROC	qglValidateProgramARB = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC qglGetUniformLocationARB = NULL;
PFNGLUNIFORM1IARBPROC qglUniform1iARB = NULL;
PFNGLUNIFORM4FARBPROC qglUniform4fARB = NULL;
PFNGLUNIFORM4FVARBPROC qglUniform4fvARB = NULL;
PFNGLGETINFOLOGARBPROC qglGetInfoLogARB = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC qglGetObjectParameterivARB = NULL;
PFNGLGETATTACHEDOBJECTSARBPROC qglGetAttachedObjectsARB = NULL;

PFNGLBINDFRAMEBUFFERPROC qglBindFramebuffer = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC qglCheckFramebufferStatus = NULL;
PFNGLDELETEFRAMEBUFFERSPROC qglDeleteFramebuffers = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC qglFramebufferTexture2D = NULL;
PFNGLFRAMEBUFFERTEXTUREPROC qglFramebufferTexture = NULL;
PFNGLGENFRAMEBUFFERSPROC qglGenFramebuffers = NULL;
PFNGLGENERATEMIPMAPPROC qglGenerateMipmap = NULL;

PFNGLBINDFRAMEBUFFERPROC qglBindFramebufferEXT = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC qglCheckFramebufferStatusEXT = NULL;
PFNGLDELETEFRAMEBUFFERSPROC qglDeleteFramebuffersEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC qglFramebufferTexture2DEXT = NULL;
PFNGLFRAMEBUFFERTEXTUREPROC qglFramebufferTextureEXT = NULL;
PFNGLGENFRAMEBUFFERSPROC qglGenFramebuffersEXT = NULL;
PFNGLGENERATEMIPMAPPROC qglGenerateMipmapEXT = NULL;

DWORD *proc_glDepthFunc					= ( DWORD * )0x13E3D28;
DWORD *proc_glDepthRange				= ( DWORD * )0x13E3D88;
DWORD *proc_glDrawElements				= ( DWORD * )0x13E3D04;
DWORD *proc_glEnable					= ( DWORD * )0x13E3D08;
DWORD *proc_glFrustum					= ( DWORD * )0x13E3C8C;
DWORD *proc_glGetString					= ( DWORD * )0x13E3FD8;
DWORD *proc_glLoadMatrixd				= ( DWORD * )0x13E3D9C;
DWORD *proc_glLoadMatrixf				= ( DWORD * )0x13E3D98;
DWORD *proc_glMatrixMode				= ( DWORD * )0x13E3DF8;
DWORD *proc_glOrtho						= ( DWORD * )0x13E3E5C;
DWORD *proc_glTexCoordPointer			= ( DWORD * )0x13E3E74;
DWORD *proc_glTexImage2D				= ( DWORD * )0x13E3E48;
DWORD *proc_glTexParameterf				= ( DWORD * )0x13E3F3C;
DWORD *proc_glTexParameteri				= ( DWORD * )0x13E3F40;

DWORD *proc_wglChoosePixelFormat		= ( DWORD * )0x13E3E2C;
DWORD *proc_wglCopyContext				= ( DWORD * )0x13E4030;
DWORD *proc_wglCreateContext			= ( DWORD * )0x13E3DEC;
DWORD *proc_wglCreateLayerContext		= ( DWORD * )0x13E3DFC;
DWORD *proc_wglDeleteContext			= ( DWORD * )0x13E40FC;
DWORD *proc_wglDescribeLayerPlane		= ( DWORD * )0x13E3E78;
DWORD *proc_wglDescribePixelFormat		= ( DWORD * )0x13E3F74;
DWORD *proc_wglGetCurrentContext		= ( DWORD * )0x13E3DA4;
DWORD *proc_wglGetCurrentDC				= ( DWORD * )0x13E4044;
DWORD *proc_wglGetLayerPaletteEntries	= ( DWORD * )0x13E3E00;
DWORD *proc_wglGetPixelFormat			= ( DWORD * )0x13E3E84;
DWORD *proc_wglGetProcAddress			= ( DWORD * )0x13E3FBC;
DWORD *proc_wglMakeCurrent				= ( DWORD * )0x13E418C;
DWORD *proc_wglRealizeLayerPalette		= ( DWORD * )0x13E3E1C;
DWORD *proc_wglSetLayerPaletteEntries	= ( DWORD * )0x13E4164;
DWORD *proc_wglSetPixelFormat			= ( DWORD * )0x13E3E28;
DWORD *proc_wglShareLists				= ( DWORD * )0x13E4138;
DWORD *proc_wglSwapBuffers				= ( DWORD * )0x13E40C8;
DWORD *proc_wglSwapLayerBuffers			= ( DWORD * )0x13E4048;
DWORD *proc_wglUseFontBitmapsA 			= ( DWORD * )0x13E405C;
DWORD *proc_wglUseFontOutlinesA			= ( DWORD * )0x13E3FC0;

DWORD proc_glDepthFunc_f;
DWORD proc_glDepthRange_f;
DWORD proc_glDrawElements_f;
DWORD proc_glEnable_f;
DWORD proc_glFrustum_f;
DWORD proc_glGetString_f;
DWORD proc_glLoadMatrixd_f;
DWORD proc_glLoadMatrixf_f;
DWORD proc_glMatrixMode_f;
DWORD proc_glOrtho_f;
DWORD proc_glTexCoordPointer_f;
DWORD proc_glTexImage2D_f;
DWORD proc_glTexParameterf_f;
DWORD proc_glTexParameteri_f;

DWORD proc_wglChoosePixelFormat_f;
DWORD proc_wglCopyContext_f;
DWORD proc_wglCreateContext_f;
DWORD proc_wglCreateLayerContext_f;
DWORD proc_wglDeleteContext_f;
DWORD proc_wglDescribeLayerPlane_f;
DWORD proc_wglDescribePixelFormat_f;
DWORD proc_wglGetCurrentContext_f;
DWORD proc_wglGetCurrentDC_f;
DWORD proc_wglGetLayerPaletteEntries_f;
DWORD proc_wglGetPixelFormat_f;
DWORD proc_wglGetProcAddress_f;
DWORD proc_wglMakeCurrent_f;
DWORD proc_wglRealizeLayerPalette_f;
DWORD proc_wglSetLayerPaletteEntries_f;
DWORD proc_wglSetPixelFormat_f;
DWORD proc_wglShareLists_f;
DWORD proc_wglSwapBuffers_f;
DWORD proc_wglSwapLayerBuffers_f;
DWORD proc_wglUseFontBitmapsA_f;
DWORD proc_wglUseFontOutlinesA_f;

HMODULE hOpenGLDll = NULL;
bool bInitError = false;
ExtensionSupport_t ext;

#define QFX_GPA( func )			q##func = ( pfn_##func )GetProcAddress( hOpenGLDll, #func ); if ( !q##func ) { QFXLog::Instance().Printf( "failed\n" ); QFXLog::Instance().Error( "Cannot find function \"%s\" in opengl32.dll!\n", #func ); bInitError = true; return false; }
#define QFX_GPAW( func )		func = ( pfn_##func )GetProcAddress( hOpenGLDll, #func ); if ( !func ) { QFXLog::Instance().Printf( "failed\n" ); QFXLog::Instance().Error( "Cannot find function \"%s\" in opengl32.dll!\n", #func ); bInitError = true; return false; }
#define QFX_EXT( type, func )	q##func = (type)gl::wglGetProcAddress( #func ); if ( !q##func ) { QFXLog::Instance().Error( "ERROR: cannot get proc address of \"%s\"!\n", #func );  }

bool Initialize( void )
{
	char szSystemLibrary[1024];

	GetSystemDirectory( szSystemLibrary, sizeof( szSystemLibrary ) );
	strcat_s( szSystemLibrary, "\\opengl32.dll" );

	QFXLog::Instance().Printf( "Loading \"%s\": ", szSystemLibrary );
	hOpenGLDll = GetModuleHandle( szSystemLibrary );
	if ( !hOpenGLDll ) {
		QFXLog::Instance().Printf( "failed\n" );
		bInitError = true;
		return false;
	}

	QFXLog::Instance().Printf( "success\n" );
	QFXLog::Instance().Printf( "Loading functions: " );

	QFX_GPA( glAccum );
	QFX_GPA( glAlphaFunc );
	QFX_GPA( glAreTexturesResident );
	QFX_GPA( glArrayElement );
	QFX_GPA( glBegin );
	QFX_GPA( glBindTexture );
	QFX_GPA( glBitmap );
	QFX_GPA( glBlendFunc );
	QFX_GPA( glCallList );
	QFX_GPA( glCallLists );
	QFX_GPA( glClear );
	QFX_GPA( glClearAccum );
	QFX_GPA( glClearColor );
	QFX_GPA( glClearDepth );
	QFX_GPA( glClearIndex );
	QFX_GPA( glClearStencil );
	QFX_GPA( glClipPlane );
	QFX_GPA( glColor3b );
	QFX_GPA( glColor3bv );
	QFX_GPA( glColor3d );
	QFX_GPA( glColor3dv );
	QFX_GPA( glColor3f );
	QFX_GPA( glColor3fv );
	QFX_GPA( glColor3i );
	QFX_GPA( glColor3iv );
	QFX_GPA( glColor3s );
	QFX_GPA( glColor3sv );
	QFX_GPA( glColor3ub );
	QFX_GPA( glColor3ubv );
	QFX_GPA( glColor3ui );
	QFX_GPA( glColor3uiv );
	QFX_GPA( glColor3us );
	QFX_GPA( glColor3usv );
	QFX_GPA( glColor4b );
	QFX_GPA( glColor4bv );
	QFX_GPA( glColor4d );
	QFX_GPA( glColor4dv );
	QFX_GPA( glColor4f );
	QFX_GPA( glColor4fv );
	QFX_GPA( glColor4i );
	QFX_GPA( glColor4iv );
	QFX_GPA( glColor4s );
	QFX_GPA( glColor4sv );
	QFX_GPA( glColor4ub );
	QFX_GPA( glColor4ubv );
	QFX_GPA( glColor4ui );
	QFX_GPA( glColor4uiv );
	QFX_GPA( glColor4us );
	QFX_GPA( glColor4usv );
	QFX_GPA( glColorMask );
	QFX_GPA( glColorMaterial );
	QFX_GPA( glColorPointer );
	QFX_GPA( glCopyPixels );
	QFX_GPA( glCopyTexImage1D );
	QFX_GPA( glCopyTexImage2D );
	QFX_GPA( glCopyTexSubImage1D );
	QFX_GPA( glCopyTexSubImage2D );
	QFX_GPA( glCullFace );
	QFX_GPA( glDeleteLists );
	QFX_GPA( glDeleteTextures );
	QFX_GPA( glDepthFunc );
	QFX_GPA( glDepthMask );
	QFX_GPA( glDepthRange );
	QFX_GPA( glDisable );
	QFX_GPA( glDisableClientState );
	QFX_GPA( glDrawArrays );
	QFX_GPA( glDrawBuffer );
	QFX_GPA( glDrawElements );
	QFX_GPA( glDrawPixels );
	QFX_GPA( glEdgeFlag );
	QFX_GPA( glEdgeFlagPointer );
	QFX_GPA( glEdgeFlagv );
	QFX_GPA( glEnable );
	QFX_GPA( glEnableClientState );
	QFX_GPA( glEnd );
	QFX_GPA( glEndList );
	QFX_GPA( glEvalCoord1d );
	QFX_GPA( glEvalCoord1dv );
	QFX_GPA( glEvalCoord1f );
	QFX_GPA( glEvalCoord1fv );
	QFX_GPA( glEvalCoord2d );
	QFX_GPA( glEvalCoord2dv );
	QFX_GPA( glEvalCoord2f );
	QFX_GPA( glEvalCoord2fv );
	QFX_GPA( glEvalMesh1 );
	QFX_GPA( glEvalMesh2 );
	QFX_GPA( glEvalPoint1 );
	QFX_GPA( glEvalPoint2 );
	QFX_GPA( glFeedbackBuffer );
	QFX_GPA( glFinish );
	QFX_GPA( glFlush );
	QFX_GPA( glFogf );
	QFX_GPA( glFogfv );
	QFX_GPA( glFogi );
	QFX_GPA( glFogiv );
	QFX_GPA( glFrontFace );
	QFX_GPA( glFrustum );
	QFX_GPA( glGenLists );
	QFX_GPA( glGenTextures );
	QFX_GPA( glGetBooleanv );
	QFX_GPA( glGetClipPlane );
	QFX_GPA( glGetDoublev );
	QFX_GPA( glGetError );
	QFX_GPA( glGetFloatv );
	QFX_GPA( glGetIntegerv );
	QFX_GPA( glGetLightfv );
	QFX_GPA( glGetLightiv );
	QFX_GPA( glGetMapdv );
	QFX_GPA( glGetMapfv );
	QFX_GPA( glGetMapiv );
	QFX_GPA( glGetMaterialfv );
	QFX_GPA( glGetMaterialiv );
	QFX_GPA( glGetPixelMapfv );
	QFX_GPA( glGetPixelMapuiv );
	QFX_GPA( glGetPixelMapusv );
	QFX_GPA( glGetPointerv );
	QFX_GPA( glGetPolygonStipple );
	QFX_GPA( glGetString );
	QFX_GPA( glGetTexEnvfv );
	QFX_GPA( glGetTexEnviv );
	QFX_GPA( glGetTexGendv );
	QFX_GPA( glGetTexGenfv );
	QFX_GPA( glGetTexGeniv );
	QFX_GPA( glGetTexImage );
	QFX_GPA( glGetTexLevelParameterfv );
	QFX_GPA( glGetTexLevelParameteriv );
	QFX_GPA( glGetTexParameterfv );
	QFX_GPA( glGetTexParameteriv );
	QFX_GPA( glHint );
	QFX_GPA( glIndexMask );
	QFX_GPA( glIndexPointer );
	QFX_GPA( glIndexd );
	QFX_GPA( glIndexdv );
	QFX_GPA( glIndexf );
	QFX_GPA( glIndexfv );
	QFX_GPA( glIndexi );
	QFX_GPA( glIndexiv );
	QFX_GPA( glIndexs );
	QFX_GPA( glIndexsv );
	QFX_GPA( glIndexub );
	QFX_GPA( glIndexubv );
	QFX_GPA( glInitNames );
	QFX_GPA( glInterleavedArrays );
	QFX_GPA( glIsEnabled );
	QFX_GPA( glIsList );
	QFX_GPA( glIsTexture );
	QFX_GPA( glLightModelf );
	QFX_GPA( glLightModelfv );
	QFX_GPA( glLightModeli );
	QFX_GPA( glLightModeliv );
	QFX_GPA( glLightf );
	QFX_GPA( glLightfv );
	QFX_GPA( glLighti );
	QFX_GPA( glLightiv );
	QFX_GPA( glLineStipple );
	QFX_GPA( glLineWidth );
	QFX_GPA( glListBase );
	QFX_GPA( glLoadIdentity );
	QFX_GPA( glLoadMatrixd );
	QFX_GPA( glLoadMatrixf );
	QFX_GPA( glLoadName );
	QFX_GPA( glLogicOp );
	QFX_GPA( glMap1d );
	QFX_GPA( glMap1f );
	QFX_GPA( glMap2d );
	QFX_GPA( glMap2f );
	QFX_GPA( glMapGrid1d );
	QFX_GPA( glMapGrid1f );
	QFX_GPA( glMapGrid2d );
	QFX_GPA( glMapGrid2f );
	QFX_GPA( glMaterialf );
	QFX_GPA( glMaterialfv );
	QFX_GPA( glMateriali );
	QFX_GPA( glMaterialiv );
	QFX_GPA( glMatrixMode );
	QFX_GPA( glMultMatrixd );
	QFX_GPA( glMultMatrixf );
	QFX_GPA( glNewList );
	QFX_GPA( glNormal3b );
	QFX_GPA( glNormal3bv );
	QFX_GPA( glNormal3d );
	QFX_GPA( glNormal3dv );
	QFX_GPA( glNormal3f );
	QFX_GPA( glNormal3fv );
	QFX_GPA( glNormal3i );
	QFX_GPA( glNormal3iv );
	QFX_GPA( glNormal3s );
	QFX_GPA( glNormal3sv );
	QFX_GPA( glNormalPointer );
	QFX_GPA( glOrtho );
	QFX_GPA( glPassThrough );
	QFX_GPA( glPixelMapfv );
	QFX_GPA( glPixelMapuiv );
	QFX_GPA( glPixelMapusv );
	QFX_GPA( glPixelStoref );
	QFX_GPA( glPixelStorei );
	QFX_GPA( glPixelTransferf );
	QFX_GPA( glPixelTransferi );
	QFX_GPA( glPixelZoom );
	QFX_GPA( glPointSize );
	QFX_GPA( glPolygonMode );
	QFX_GPA( glPolygonOffset );
	QFX_GPA( glPolygonStipple );
	QFX_GPA( glPopAttrib );
	QFX_GPA( glPopClientAttrib );
	QFX_GPA( glPopMatrix );
	QFX_GPA( glPopName );
	QFX_GPA( glPrioritizeTextures );
	QFX_GPA( glPushAttrib );
	QFX_GPA( glPushClientAttrib );
	QFX_GPA( glPushMatrix );
	QFX_GPA( glPushName );
	QFX_GPA( glRasterPos2d );
	QFX_GPA( glRasterPos2dv );
	QFX_GPA( glRasterPos2f );
	QFX_GPA( glRasterPos2fv );
	QFX_GPA( glRasterPos2i );
	QFX_GPA( glRasterPos2iv );
	QFX_GPA( glRasterPos2s );
	QFX_GPA( glRasterPos2sv );
	QFX_GPA( glRasterPos3d );
	QFX_GPA( glRasterPos3dv );
	QFX_GPA( glRasterPos3f );
	QFX_GPA( glRasterPos3fv );
	QFX_GPA( glRasterPos3i );
	QFX_GPA( glRasterPos3iv );
	QFX_GPA( glRasterPos3s );
	QFX_GPA( glRasterPos3sv );
	QFX_GPA( glRasterPos4d );
	QFX_GPA( glRasterPos4dv );
	QFX_GPA( glRasterPos4f );
	QFX_GPA( glRasterPos4fv );
	QFX_GPA( glRasterPos4i );
	QFX_GPA( glRasterPos4iv );
	QFX_GPA( glRasterPos4s );
	QFX_GPA( glRasterPos4sv );
	QFX_GPA( glReadBuffer );
	QFX_GPA( glReadPixels );
	QFX_GPA( glRectd );
	QFX_GPA( glRectdv );
	QFX_GPA( glRectf );
	QFX_GPA( glRectfv );
	QFX_GPA( glRecti );
	QFX_GPA( glRectiv );
	QFX_GPA( glRects );
	QFX_GPA( glRectsv );
	QFX_GPA( glRenderMode );
	QFX_GPA( glRotated );
	QFX_GPA( glRotatef );
	QFX_GPA( glScaled );
	QFX_GPA( glScalef );
	QFX_GPA( glScissor );
	QFX_GPA( glSelectBuffer );
	QFX_GPA( glShadeModel );
	QFX_GPA( glStencilFunc );
	QFX_GPA( glStencilMask );
	QFX_GPA( glStencilOp );
	QFX_GPA( glTexCoord1d );
	QFX_GPA( glTexCoord1dv );
	QFX_GPA( glTexCoord1f );
	QFX_GPA( glTexCoord1fv );
	QFX_GPA( glTexCoord1i );
	QFX_GPA( glTexCoord1iv );
	QFX_GPA( glTexCoord1s );
	QFX_GPA( glTexCoord1sv );
	QFX_GPA( glTexCoord2d );
	QFX_GPA( glTexCoord2dv );
	QFX_GPA( glTexCoord2f );
	QFX_GPA( glTexCoord2fv );
	QFX_GPA( glTexCoord2i );
	QFX_GPA( glTexCoord2iv );
	QFX_GPA( glTexCoord2s );
	QFX_GPA( glTexCoord2sv );
	QFX_GPA( glTexCoord3d );
	QFX_GPA( glTexCoord3dv );
	QFX_GPA( glTexCoord3f );
	QFX_GPA( glTexCoord3fv );
	QFX_GPA( glTexCoord3i );
	QFX_GPA( glTexCoord3iv );
	QFX_GPA( glTexCoord3s );
	QFX_GPA( glTexCoord3sv );
	QFX_GPA( glTexCoord4d );
	QFX_GPA( glTexCoord4dv );
	QFX_GPA( glTexCoord4f );
	QFX_GPA( glTexCoord4fv );
	QFX_GPA( glTexCoord4i );
	QFX_GPA( glTexCoord4iv );
	QFX_GPA( glTexCoord4s );
	QFX_GPA( glTexCoord4sv );
	QFX_GPA( glTexCoordPointer );
	QFX_GPA( glTexEnvf );
	QFX_GPA( glTexEnvfv );
	QFX_GPA( glTexEnvi );
	QFX_GPA( glTexEnviv );
	QFX_GPA( glTexGend );
	QFX_GPA( glTexGendv );
	QFX_GPA( glTexGenf );
	QFX_GPA( glTexGenfv );
	QFX_GPA( glTexGeni );
	QFX_GPA( glTexGeniv );
	QFX_GPA( glTexImage1D );
	QFX_GPA( glTexImage2D );
	QFX_GPA( glTexParameterf );
	QFX_GPA( glTexParameterfv );
	QFX_GPA( glTexParameteri );
	QFX_GPA( glTexParameteriv );
	QFX_GPA( glTexSubImage1D );
	QFX_GPA( glTexSubImage2D );
	QFX_GPA( glTranslated );
	QFX_GPA( glTranslatef );
	QFX_GPA( glVertex2d );
	QFX_GPA( glVertex2dv );
	QFX_GPA( glVertex2f );
	QFX_GPA( glVertex2fv );
	QFX_GPA( glVertex2i );
	QFX_GPA( glVertex2iv );
	QFX_GPA( glVertex2s );
	QFX_GPA( glVertex2sv );
	QFX_GPA( glVertex3d );
	QFX_GPA( glVertex3dv );
	QFX_GPA( glVertex3f );
	QFX_GPA( glVertex3fv );
	QFX_GPA( glVertex3i );
	QFX_GPA( glVertex3iv );
	QFX_GPA( glVertex3s );
	QFX_GPA( glVertex3sv );
	QFX_GPA( glVertex4d );
	QFX_GPA( glVertex4dv );
	QFX_GPA( glVertex4f );
	QFX_GPA( glVertex4fv );
	QFX_GPA( glVertex4i );
	QFX_GPA( glVertex4iv );
	QFX_GPA( glVertex4s );
	QFX_GPA( glVertex4sv );
	QFX_GPA( glVertexPointer );
	QFX_GPA( glViewport );

	QFX_GPAW( wglCopyContext );
	QFX_GPAW( wglCreateContext );
	QFX_GPAW( wglDeleteContext );
	QFX_GPAW( wglGetCurrentContext );
	QFX_GPAW( wglGetCurrentDC );
	QFX_GPAW( wglGetProcAddress );
	QFX_GPAW( wglMakeCurrent );
	QFX_GPAW( wglShareLists );
	QFX_GPAW( wglChoosePixelFormat );
	QFX_GPAW( wglDescribePixelFormat );
	QFX_GPAW( wglGetPixelFormat );
	QFX_GPAW( wglSetPixelFormat );
	QFX_GPAW( wglSwapBuffers );
	QFX_GPAW( wglCreateLayerContext );
	QFX_GPAW( wglDescribeLayerPlane );
	QFX_GPAW( wglGetLayerPaletteEntries );
	QFX_GPAW( wglSetLayerPaletteEntries );
	QFX_GPAW( wglRealizeLayerPalette );
	QFX_GPAW( wglSwapLayerBuffers );
	QFX_GPAW( wglUseFontBitmapsA );
	QFX_GPAW( wglUseFontBitmapsW );
	QFX_GPAW( wglUseFontOutlinesA );
	QFX_GPAW( wglUseFontOutlinesW );

	proc_glDepthFunc_f					= *proc_glDepthFunc;
	proc_glDepthRange_f					= *proc_glDepthRange;
	proc_glDrawElements_f				= *proc_glDrawElements;
	proc_glEnable_f						= *proc_glEnable;
	proc_glFrustum_f					= *proc_glFrustum;
	proc_glGetString_f					= *proc_glGetString;
	proc_glLoadMatrixd_f				= *proc_glLoadMatrixd;
	proc_glLoadMatrixf_f				= *proc_glLoadMatrixf;
	proc_glMatrixMode_f					= *proc_glMatrixMode;
	proc_glOrtho_f						= *proc_glOrtho;
	proc_glTexCoordPointer_f			= *proc_glTexCoordPointer;
	proc_glTexImage2D_f					= *proc_glTexImage2D;
	proc_glTexParameterf_f				= *proc_glTexParameterf;
	proc_glTexParameteri_f				= *proc_glTexParameteri;

	proc_wglChoosePixelFormat_f			= *proc_wglChoosePixelFormat;
	proc_wglCopyContext_f				= *proc_wglCopyContext;
	proc_wglCreateContext_f				= *proc_wglCreateContext;
	proc_wglCreateLayerContext_f		= *proc_wglCreateLayerContext;
	proc_wglDeleteContext_f 			= *proc_wglDeleteContext;
	proc_wglDescribeLayerPlane_f		= *proc_wglDescribeLayerPlane;
	proc_wglDescribePixelFormat_f		= *proc_wglDescribePixelFormat;
	proc_wglGetCurrentContext_f			= *proc_wglGetCurrentContext;
	proc_wglGetCurrentDC_f				= *proc_wglGetCurrentDC;
	proc_wglGetLayerPaletteEntries_f	= *proc_wglGetLayerPaletteEntries;
	proc_wglGetPixelFormat_f			= *proc_wglGetPixelFormat;
	proc_wglGetProcAddress_f			= *proc_wglGetProcAddress;
	proc_wglMakeCurrent_f				= *proc_wglMakeCurrent;
	proc_wglRealizeLayerPalette_f		= *proc_wglRealizeLayerPalette;
	proc_wglSetLayerPaletteEntries_f	= *proc_wglSetLayerPaletteEntries;
	proc_wglSetPixelFormat_f			= *proc_wglSetPixelFormat;
	proc_wglShareLists_f				= *proc_wglShareLists;
	proc_wglSwapBuffers_f				= *proc_wglSwapBuffers;
	proc_wglSwapLayerBuffers_f 			= *proc_wglSwapLayerBuffers;
	proc_wglUseFontBitmapsA_f			= *proc_wglUseFontBitmapsA;
	proc_wglUseFontOutlinesA_f			= *proc_wglUseFontOutlinesA;

	*proc_glDepthFunc					= ( DWORD )( pfn_glDepthFunc )glDepthFunc;
	*proc_glDepthRange					= ( DWORD )( pfn_glDepthRange )glDepthRange;
	*proc_glDrawElements				= ( DWORD )( pfn_glDrawElements )glDrawElements_h;
	*proc_glEnable						= ( DWORD )( pfn_glEnable )glEnable;
	*proc_glFrustum						= ( DWORD )( pfn_glFrustum )glFrustum;
	*proc_glGetString					= ( DWORD )( pfn_glGetString )glGetString;
	*proc_glLoadMatrixd					= ( DWORD )( pfn_glLoadMatrixd )glLoadMatrixd;
	*proc_glLoadMatrixf					= ( DWORD )( pfn_glLoadMatrixf )glLoadMatrixf;
	*proc_glMatrixMode					= ( DWORD )( pfn_glMatrixMode )glMatrixMode;
	*proc_glOrtho						= ( DWORD )( pfn_glOrtho )glOrtho;
	*proc_glTexCoordPointer				= ( DWORD )( pfn_glTexCoordPointer )glTexCoordPointer_h;
	*proc_glTexImage2D					= ( DWORD )( pfn_glTexImage2D )glTexImage2D;
	*proc_glTexParameterf				= ( DWORD )( pfn_glTexParameterf )glTexParameterf;
	*proc_glTexParameteri				= ( DWORD )( pfn_glTexParameteri )glTexParameteri;

	*proc_wglChoosePixelFormat			= ( DWORD )( pfn_wglChoosePixelFormat )wrap_wglChoosePixelFormat;
	*proc_wglCopyContext				= ( DWORD )( pfn_wglCopyContext )wrap_wglCopyContext;
	*proc_wglCreateContext				= ( DWORD )( pfn_wglCreateContext )wrap_wglCreateContext;
	*proc_wglCreateLayerContext			= ( DWORD )( pfn_wglCreateLayerContext )wrap_wglCreateLayerContext;
	*proc_wglDeleteContext 				= ( DWORD )( pfn_wglDeleteContext )wrap_wglDeleteContext;
	*proc_wglDescribeLayerPlane			= ( DWORD )( pfn_wglDescribeLayerPlane )wrap_wglDescribeLayerPlane;
	*proc_wglDescribePixelFormat		= ( DWORD )( pfn_wglDescribePixelFormat )wrap_wglDescribePixelFormat;
	*proc_wglGetCurrentContext			= ( DWORD )( pfn_wglGetCurrentContext )wrap_wglGetCurrentContext;
	*proc_wglGetCurrentDC				= ( DWORD )( pfn_wglGetCurrentDC )wrap_wglGetCurrentDC;
	*proc_wglGetLayerPaletteEntries		= ( DWORD )( pfn_wglGetLayerPaletteEntries )wrap_wglGetLayerPaletteEntries;
	*proc_wglGetPixelFormat				= ( DWORD )( pfn_wglGetPixelFormat )wrap_wglGetPixelFormat;
	*proc_wglGetProcAddress				= ( DWORD )( pfn_wglGetProcAddress )wrap_wglGetProcAddress;
	*proc_wglMakeCurrent				= ( DWORD )( pfn_wglMakeCurrent )wrap_wglMakeCurrent;
	*proc_wglRealizeLayerPalette		= ( DWORD )( pfn_wglRealizeLayerPalette )wrap_wglRealizeLayerPalette;
	*proc_wglSetLayerPaletteEntries		= ( DWORD )( pfn_wglSetLayerPaletteEntries )wrap_wglSetLayerPaletteEntries;
	*proc_wglSetPixelFormat				= ( DWORD )( pfn_wglSetPixelFormat )wrap_wglSetPixelFormat;
	*proc_wglShareLists					= ( DWORD )( pfn_wglShareLists )wrap_wglShareLists;
	*proc_wglSwapBuffers				= ( DWORD )( pfn_wglSwapBuffers )wrap_wglSwapBuffers;
	*proc_wglSwapLayerBuffers 			= ( DWORD )( pfn_wglSwapLayerBuffers )wrap_wglSwapLayerBuffers;
	*proc_wglUseFontBitmapsA			= ( DWORD )( pfn_wglUseFontBitmapsA )wrap_wglUseFontBitmapsA;
	*proc_wglUseFontOutlinesA			= ( DWORD )( pfn_wglUseFontOutlinesA )wrap_wglUseFontOutlinesA;

	QFXLog::Instance().Printf( "success\n" );
	return true;
}

void InitializeExtensions( void )
{
	// clear extensions
	memset( &ext, 0, sizeof( ext ) );
	ext.edgeClampMode = GL_CLAMP;

	// get extension string
	const char *pszExtensions = (const char *)qglGetString( GL_EXTENSIONS );

	if ( !pszExtensions || !strlen( pszExtensions ) )
		return;

	// check for multitexture
	if ( strstr( pszExtensions, "GL_ARB_multitexture" ) ) {
		QFX_EXT( PFNGLACTIVETEXTUREARBPROC, glActiveTextureARB );
		QFX_EXT( PFNGLACTIVETEXTUREARBPROC, glClientActiveTextureARB );
		QFX_EXT( PFNGLMULTITEXCOORD2FARBPROC, glMultiTexCoord2fARB );
		QFX_EXT( PFNGLMULTITEXCOORD2FVARBPROC, glMultiTexCoord2fvARB );
		qglGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &ext.maxActiveTextures );
		if ( ext.maxActiveTextures > 1 ) {
			ext.Multitexture = 1;
			QFXLog::Instance().Printf( "Using extension: GL_ARB_multitexture (%i TMU)\n", ext.maxActiveTextures );
		}
	}

	// check for rectangle textures
	if ( strstr( pszExtensions, "GL_ARB_texture_rectangle" ) ) {
		ext.RectangleTexture = 1;
		QFXLog::Instance().Printf( "Using extension: GL_ARB_texture_rectangle\n" );
	}
	else if ( strstr( pszExtensions, "GL_NV_texture_rectangle" ) ) {
		ext.RectangleTexture = 1;
		QFXLog::Instance().Printf( "Using extension: GL_NV_texture_rectangle\n" );
	}
	else if ( strstr( pszExtensions, "GL_EXT_texture_rectangle" ) ) {
		ext.RectangleTexture = 1;
		QFXLog::Instance().Printf( "Using extension: GL_EXT_texture_rectangle\n" );
	}

	// check for depth textures
	if ( strstr( pszExtensions, "GL_ARB_depth_texture" ) ) {
		ext.DepthTexture = 1;
		QFXLog::Instance().Printf( "Using extension: GL_ARB_depth_texture\n" );
	}
	else if ( strstr( pszExtensions, "GL_SGIS_depth_texture" ) ) {
		ext.DepthTexture = 1;
		QFXLog::Instance().Printf( "Using extension: GL_SGIS_depth_texture\n" );
	}

	// check for clamp to edge
	if ( strstr( pszExtensions, "GL_EXT_texture_edge_clamp" ) ) {
		ext.edgeClampMode = GL_CLAMP_TO_EDGE;
		QFXLog::Instance().Printf( "Using extension: GL_EXT_texture_edge_clamp\n" );
	}

	// check for anisotropic filtering
	if ( strstr( pszExtensions, "GL_EXT_texture_filter_anisotropic" ) ) {
		qglGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &ext.maxAnisotropy);
		QFXLog::Instance().Printf( "Using extension: GL_EXT_texture_filter_anisotropic (max %i)\n", ext.maxAnisotropy );
	}

	// check for multisample filter hint
	if ( strstr( pszExtensions, "GL_NV_multisample_filter_hint" ) ) {
		ext.MultisampleHint = 1;
		QFXLog::Instance().Printf( "Using extension: GL_NV_multisample_filter_hint\n" );
	}

	// check for FBO
	if ( strstr( pszExtensions, "GL_ARB_framebuffer_object" ) )
	{
		QFX_EXT( PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer );
		QFX_EXT( PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus );
		QFX_EXT( PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers );
		QFX_EXT( PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D );
		QFX_EXT( PFNGLFRAMEBUFFERTEXTUREPROC, glFramebufferTexture );
		QFX_EXT( PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers );
		QFX_EXT( PFNGLGENERATEMIPMAPPROC, glGenerateMipmap );
		ext.FBO = 1;
		QFXLog::Instance().Printf( "Using extension: GL_ARB_framebuffer_object\n" );
	}
	if ( strstr( pszExtensions, "GL_EXT_framebuffer_object" ) )
	{
		QFX_EXT( PFNGLBINDFRAMEBUFFERPROC, glBindFramebufferEXT );
		QFX_EXT( PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatusEXT );
		QFX_EXT( PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffersEXT );
		QFX_EXT( PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2DEXT );
		QFX_EXT( PFNGLFRAMEBUFFERTEXTUREPROC, glFramebufferTextureEXT );
		QFX_EXT( PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffersEXT );
		QFX_EXT( PFNGLGENERATEMIPMAPPROC, glGenerateMipmapEXT );
		ext.FBO = 1;
		QFXLog::Instance().Printf( "Using extension: GL_EXT_framebuffer_object\n" );
	}

	// check for shader objects
	if ( strstr( pszExtensions, "GL_ARB_shader_objects" ) &&
		 strstr( pszExtensions, "GL_ARB_shading_language_100" ) ) {
		QFX_EXT( PFNGLGETHANDLEARBPROC, glGetHandleARB );
		QFX_EXT( PFNGLDELETEOBJECTARBPROC, glDeleteObjectARB );
		QFX_EXT( PFNGLDETACHOBJECTARBPROC, glDetachObjectARB );
		QFX_EXT( PFNGLCREATESHADEROBJECTARBPROC, glCreateShaderObjectARB );
		//qglCreateShaderObjectARB = ( PFNGLCREATESHADEROBJECTARBPROC )GetProcAddress( hOpenGLDll, "glCreateShader" );
		QFX_EXT( PFNGLSHADERSOURCEARBPROC, glShaderSourceARB );
		QFX_EXT( PFNGLCOMPILESHADERARBPROC, glCompileShaderARB );
		QFX_EXT( PFNGLCREATEPROGRAMOBJECTARBPROC, glCreateProgramObjectARB );
		QFX_EXT( PFNGLATTACHOBJECTARBPROC, glAttachObjectARB );
		QFX_EXT( PFNGLLINKPROGRAMARBPROC, glLinkProgramARB );
		QFX_EXT( PFNGLUSEPROGRAMOBJECTARBPROC, glUseProgramObjectARB );
		QFX_EXT( PFNGLVALIDATEPROGRAMARBPROC, glValidateProgramARB );
		QFX_EXT( PFNGLGETUNIFORMLOCATIONARBPROC, glGetUniformLocationARB );
		QFX_EXT( PFNGLUNIFORM4FARBPROC, glUniform4fARB );
		QFX_EXT( PFNGLUNIFORM4FVARBPROC, glUniform4fvARB );
		QFX_EXT( PFNGLUNIFORM1IARBPROC, glUniform1iARB );
		QFX_EXT( PFNGLGETINFOLOGARBPROC, glGetInfoLogARB );
		QFX_EXT( PFNGLGETOBJECTPARAMETERIVARBPROC, glGetObjectParameterivARB );
		QFX_EXT( PFNGLGETATTACHEDOBJECTSARBPROC, glGetAttachedObjectsARB );
		QFXLog::Instance().Printf( "Using extension: GL_ARB_shader_objects\n" );
		QFXLog::Instance().Printf( "Using extension: GL_ARB_shading_language_100\n" );

		if ( strstr( pszExtensions, "GL_ARB_vertex_shader" ) ) {
			QFXLog::Instance().Printf( "Using extension: GL_ARB_vertex_shader\n" );
			ext.VertexShader = 1;
		}
		if ( strstr( pszExtensions, "GL_ARB_fragment_shader" ) ) {
			QFXLog::Instance().Printf( "Using extension: GL_ARB_fragment_shader\n" );
			ext.FragmentShader = 1;
		}
	}

	cgi.Printf( "pszExtentions = %s\n", pszExtensions );
}

void CheckInit( void )
{
	if ( !hOpenGLDll && !bInitError ) {
		Initialize();
	}
}

void Shutdown( void )
{
	if ( hOpenGLDll )
	{
		hOpenGLDll = NULL;

		*proc_glDepthFunc					= proc_glDepthFunc_f;
		*proc_glDepthRange					= proc_glDepthRange_f;
		*proc_glDrawElements				= proc_glDrawElements_f;
		*proc_glEnable						= proc_glEnable_f;
		*proc_glFrustum						= proc_glFrustum_f;
		*proc_glGetString					= proc_glGetString_f;
		*proc_glLoadMatrixd					= proc_glLoadMatrixd_f;
		*proc_glLoadMatrixf					= proc_glLoadMatrixf_f;
		*proc_glMatrixMode					= proc_glMatrixMode_f;
		*proc_glOrtho						= proc_glOrtho_f;
		*proc_glTexCoordPointer				= proc_glTexCoordPointer_f;
		*proc_glTexImage2D					= proc_glTexImage2D_f;
		*proc_glTexParameterf				= proc_glTexParameterf_f;
		*proc_glTexParameteri				= proc_glTexParameteri_f;

		*proc_wglChoosePixelFormat			= proc_wglChoosePixelFormat_f;
		*proc_wglCopyContext				= proc_wglCopyContext_f;
		*proc_wglCreateContext				= proc_wglCreateContext_f;
		*proc_wglCreateLayerContext			= proc_wglCreateLayerContext_f;
		*proc_wglDeleteContext 				= proc_wglDeleteContext_f;
		*proc_wglDescribeLayerPlane			= proc_wglDescribeLayerPlane_f;
		*proc_wglDescribePixelFormat		= proc_wglDescribePixelFormat_f;
		*proc_wglGetCurrentContext			= proc_wglGetCurrentContext_f;
		*proc_wglGetCurrentDC				= proc_wglGetCurrentDC_f;
		*proc_wglGetLayerPaletteEntries		= proc_wglGetLayerPaletteEntries_f;
		*proc_wglGetPixelFormat				= proc_wglGetPixelFormat_f;
		*proc_wglGetProcAddress				= proc_wglGetProcAddress_f;
		*proc_wglMakeCurrent				= proc_wglMakeCurrent_f;
		*proc_wglRealizeLayerPalette		= proc_wglRealizeLayerPalette_f;
		*proc_wglSetLayerPaletteEntries		= proc_wglSetLayerPaletteEntries_f;
		*proc_wglSetPixelFormat				= proc_wglSetPixelFormat_f;
		*proc_wglShareLists					= proc_wglShareLists_f;
		*proc_wglSwapBuffers				= proc_wglSwapBuffers_f;
		*proc_wglSwapLayerBuffers 			= proc_wglSwapLayerBuffers_f;
		*proc_wglUseFontBitmapsA			= proc_wglUseFontBitmapsA_f;
		*proc_wglUseFontOutlinesA			= proc_wglUseFontOutlinesA_f;
	}
}
}

void WINAPI glAccum( GLenum op, GLfloat value ) { gl::qglAccum( op, value ); }
void WINAPI glAlphaFunc( GLenum func, GLclampf ref ) { gl::qglAlphaFunc( func, ref ); }
GLboolean WINAPI glAreTexturesResident( GLsizei n, const GLuint *textures, GLboolean *residences ) { return gl::qglAreTexturesResident( n, textures, residences ); }
void WINAPI glArrayElement( GLint i ) { gl::qglArrayElement( i ); }
void WINAPI glBegin( GLenum mode ) { gl::qglBegin( mode ); }
void WINAPI glBindTexture( GLenum target, GLuint texture ) { gl::qglBindTexture( target, texture ); }
void WINAPI glBitmap( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap ) { gl::qglBitmap( width, height, xorig, yorig, xmove, ymove, bitmap ); }
void WINAPI glBlendFunc( GLenum sfactor, GLenum dfactor ) { gl::qglBlendFunc( sfactor, dfactor ); }
void WINAPI glCallList( GLuint list ) { gl::qglCallList( list ); }
void WINAPI glCallLists( GLsizei n, GLenum type, const GLvoid *lists ) { gl::qglCallLists( n, type, lists ); }
void WINAPI glClear( GLbitfield mask ) { gl::qglClear( mask ); }
void WINAPI glClearAccum( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) { gl::qglClearAccum( red, green, blue, alpha ); }
void WINAPI glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha ) { gl::qglClearColor( red, green, blue, alpha ); }
void WINAPI glClearDepth( GLclampd depth ) { gl::qglClearDepth( depth ); }
void WINAPI glClearIndex( GLfloat c ) { gl::qglClearIndex( c ); }
void WINAPI glClearStencil( GLint s ) { gl::qglClearStencil( s ); }
void WINAPI glClipPlane( GLenum plane, const GLdouble *equation ) { gl::qglClipPlane( plane, equation ); }
void WINAPI glColor3b( GLbyte red, GLbyte green, GLbyte blue ) { gl::qglColor3b( red, green, blue ); }
void WINAPI glColor3bv( const GLbyte *v ) { gl::qglColor3bv( v ); }
void WINAPI glColor3d( GLdouble red, GLdouble green, GLdouble blue ) { gl::qglColor3d( red, green, blue ); }
void WINAPI glColor3dv( const GLdouble *v ) { gl::qglColor3dv( v ); }
void WINAPI glColor3f( GLfloat red, GLfloat green, GLfloat blue ) { gl::qglColor3f( red, green, blue ); }
void WINAPI glColor3fv( const GLfloat *v ) { gl::qglColor3fv( v ); }
void WINAPI glColor3i( GLint red, GLint green, GLint blue ) { gl::qglColor3i( red, green, blue ); }
void WINAPI glColor3iv( const GLint *v ) { gl::qglColor3iv( v ); }
void WINAPI glColor3s( GLshort red, GLshort green, GLshort blue ) { gl::qglColor3s( red, green, blue ); }
void WINAPI glColor3sv( const GLshort *v ) { gl::qglColor3sv( v ); }
void WINAPI glColor3ub( GLubyte red, GLubyte green, GLubyte blue ) { gl::qglColor3ub( red, green, blue ); }
void WINAPI glColor3ubv( const GLubyte *v ) { gl::qglColor3ubv( v ); }
void WINAPI glColor3ui( GLuint red, GLuint green, GLuint blue ) { gl::qglColor3ui( red, green, blue ); }
void WINAPI glColor3uiv( const GLuint *v ) { gl::qglColor3uiv( v ); }
void WINAPI glColor3us( GLushort red, GLushort green, GLushort blue ) { gl::qglColor3us( red, green, blue ); }
void WINAPI glColor3usv( const GLushort *v ) { gl::qglColor3usv( v ); }
void WINAPI glColor4b( GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha ) { gl::qglColor4b( red, green, blue, alpha ); }
void WINAPI glColor4bv( const GLbyte *v ) { gl::qglColor4bv( v ); }
void WINAPI glColor4d( GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha ) { gl::qglColor4d( red, green, blue, alpha ); }
void WINAPI glColor4dv( const GLdouble *v ) { gl::qglColor4dv( v ); }
void WINAPI glColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) { gl::qglColor4f( red, green, blue, alpha ); }
void WINAPI glColor4fv( const GLfloat *v ) { gl::qglColor4fv( v ); }
void WINAPI glColor4i( GLint red, GLint green, GLint blue, GLint alpha ) { gl::qglColor4i( red, green, blue, alpha ); }
void WINAPI glColor4iv( const GLint *v ) { gl::qglColor4iv( v ); }
void WINAPI glColor4s( GLshort red, GLshort green, GLshort blue, GLshort alpha ) { gl::qglColor4s( red, green, blue, alpha ); }
void WINAPI glColor4sv( const GLshort *v ) { gl::qglColor4sv( v ); }
void WINAPI glColor4ub( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha ) { gl::qglColor4ub( red, green, blue, alpha ); }
void WINAPI glColor4ubv( const GLubyte *v ) { gl::qglColor4ubv( v ); }
void WINAPI glColor4ui( GLuint red, GLuint green, GLuint blue, GLuint alpha ) { gl::qglColor4ui( red, green, blue, alpha ); }
void WINAPI glColor4uiv( const GLuint *v ) { gl::qglColor4uiv( v ); }
void WINAPI glColor4us( GLushort red, GLushort green, GLushort blue, GLushort alpha ) { gl::qglColor4us( red, green, blue, alpha ); }
void WINAPI glColor4usv( const GLushort *v ) { gl::qglColor4usv( v ); }
void WINAPI glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha ) { gl::qglColorMask( red, green, blue, alpha ); }
void WINAPI glColorMaterial( GLenum face, GLenum mode ) { gl::qglColorMaterial( face, mode ); }
void WINAPI glColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer ) { gl::qglColorPointer( size, type, stride, pointer ); }
void WINAPI glCopyPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type ) { gl::qglCopyPixels( x, y, width, height, type ); }
void WINAPI glCopyTexImage1D( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border ) { gl::qglCopyTexImage1D( target, level, internalFormat, x, y, width, border ); }
void WINAPI glCopyTexImage2D( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border ) { gl::qglCopyTexImage2D( target, level, internalFormat, x, y, width, height, border ); }
void WINAPI glCopyTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width ) { gl::qglCopyTexSubImage1D( target, level, xoffset, x, y, width ); }
void WINAPI glCopyTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height ) { gl::qglCopyTexSubImage2D( target, level, xoffset, yoffset, x, y, width, height ); }
void WINAPI glCullFace( GLenum mode ) { gl::qglCullFace( mode ); }
void WINAPI glDeleteLists( GLuint list, GLsizei range ) { gl::qglDeleteLists( list, range ); }
void WINAPI glDeleteTextures( GLsizei n, const GLuint *textures ) { gl::qglDeleteTextures( n, textures ); }
void WINAPI glDepthMask( GLboolean flag ) { gl::qglDepthMask( flag ); }
void WINAPI glDisable( GLenum cap ) { gl::qglDisable( cap ); }
void WINAPI glDisableClientState( GLenum array ) { gl::qglDisableClientState( array ); }
void WINAPI glDrawArrays( GLenum mode, GLint first, GLsizei count ) { gl::qglDrawArrays( mode, first, count ); }
void WINAPI glDrawBuffer( GLenum mode ) { gl::qglDrawBuffer( mode ); }
void WINAPI glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices ) { gl::qglDrawElements( mode, count, type, indices ); }
void WINAPI glDrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ) { gl::qglDrawPixels( width, height, format, type, pixels ); }
void WINAPI glEdgeFlag( GLboolean flag ) { gl::qglEdgeFlag( flag ); }
void WINAPI glEdgeFlagPointer( GLsizei stride, const GLvoid *pointer ) { gl::qglEdgeFlagPointer( stride, pointer ); }
void WINAPI glEdgeFlagv( const GLboolean *flag ) { gl::qglEdgeFlagv( flag ); }
void WINAPI glEnableClientState( GLenum array ) { gl::qglEnableClientState( array ); }
void WINAPI glEnd( void ) { gl::qglEnd(); }
void WINAPI glEndList( void ) { gl::qglEndList(); }
void WINAPI glEvalCoord1d( GLdouble u ) { gl::qglEvalCoord1d( u ); }
void WINAPI glEvalCoord1dv( const GLdouble *u ) { gl::qglEvalCoord1dv( u ); }
void WINAPI glEvalCoord1f( GLfloat u ) { gl::qglEvalCoord1f( u ); }
void WINAPI glEvalCoord1fv( const GLfloat *u ) { gl::qglEvalCoord1fv( u ); }
void WINAPI glEvalCoord2d( GLdouble u, GLdouble v ) { gl::qglEvalCoord2d( u, v ); }
void WINAPI glEvalCoord2dv( const GLdouble *u ) { gl::qglEvalCoord2dv( u ); }
void WINAPI glEvalCoord2f( GLfloat u, GLfloat v ) { gl::qglEvalCoord2f( u, v ); }
void WINAPI glEvalCoord2fv( const GLfloat *u ) { gl::qglEvalCoord2fv( u ); }
void WINAPI glEvalMesh1( GLenum mode, GLint i1, GLint i2 ) { gl::qglEvalMesh1( mode, i1, i2 ); }
void WINAPI glEvalMesh2( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 ) { gl::qglEvalMesh2( mode, i1, i2, j1, j2 ); }
void WINAPI glEvalPoint1( GLint i ) { gl::qglEvalPoint1( i ); }
void WINAPI glEvalPoint2( GLint i, GLint j ) { gl::qglEvalPoint2( i, j ); }
void WINAPI glFeedbackBuffer( GLsizei size, GLenum type, GLfloat *buffer ) { gl::qglFeedbackBuffer( size, type, buffer ); }
void WINAPI glFinish( void ) { gl::qglFinish(); }
void WINAPI glFlush( void ) { gl::qglFlush(); }
void WINAPI glFogf( GLenum pname, GLfloat param ) { gl::qglFogf( pname, param ); }
void WINAPI glFogfv( GLenum pname, const GLfloat *params ) { gl::qglFogfv( pname, params ); }
void WINAPI glFogi( GLenum pname, GLint param ) { gl::qglFogi( pname, param ); }
void WINAPI glFogiv( GLenum pname, const GLint *params ) { gl::qglFogiv( pname, params ); }
void WINAPI glFrontFace( GLenum mode ) { gl::qglFrontFace( mode ); }
GLuint WINAPI glGenLists( GLsizei range ) { return gl::qglGenLists( range ); }
void WINAPI glGenTextures( GLsizei n, GLuint *textures ) { gl::qglGenTextures( n, textures ); }
void WINAPI glGetBooleanv( GLenum pname, GLboolean *params ) { gl::qglGetBooleanv( pname, params ); }
void WINAPI glGetClipPlane( GLenum plane, GLdouble *equation ) { gl::qglGetClipPlane( plane, equation ); }
void WINAPI glGetDoublev( GLenum pname, GLdouble *params ) { gl::qglGetDoublev( pname, params ); }
GLenum WINAPI glGetError( void ) { return gl::qglGetError(); }
void WINAPI glGetFloatv( GLenum pname, GLfloat *params ) { gl::qglGetFloatv( pname, params ); }
void WINAPI glGetIntegerv( GLenum pname, GLint *params ) { gl::qglGetIntegerv( pname, params ); }
void WINAPI glGetLightfv( GLenum light, GLenum pname, GLfloat *params ) { gl::qglGetLightfv( light, pname, params ); }
void WINAPI glGetLightiv( GLenum light, GLenum pname, GLint *params ) { gl::qglGetLightiv( light, pname, params ); }
void WINAPI glGetMapdv( GLenum target, GLenum query, GLdouble *v ) { gl::qglGetMapdv( target, query, v ); }
void WINAPI glGetMapfv( GLenum target, GLenum query, GLfloat *v ) { gl::qglGetMapfv( target, query, v ); }
void WINAPI glGetMapiv( GLenum target, GLenum query, GLint *v ) { gl::qglGetMapiv( target, query, v ); }
void WINAPI glGetMaterialfv( GLenum face, GLenum pname, GLfloat *params ) { gl::qglGetMaterialfv( face, pname, params ); }
void WINAPI glGetMaterialiv( GLenum face, GLenum pname, GLint *params ) { gl::qglGetMaterialiv( face, pname, params ); }
void WINAPI glGetPixelMapfv( GLenum map, GLfloat *values ) { gl::qglGetPixelMapfv( map, values ); }
void WINAPI glGetPixelMapuiv( GLenum map, GLuint *values ) { gl::qglGetPixelMapuiv( map, values ); }
void WINAPI glGetPixelMapusv( GLenum map, GLushort *values ) { gl::qglGetPixelMapusv( map, values ); }
void WINAPI glGetPointerv( GLenum pname, GLvoid* *params ) { gl::qglGetPointerv( pname, params ); }
void WINAPI glGetPolygonStipple( GLubyte *mask ) { gl::qglGetPolygonStipple( mask ); }
void WINAPI glGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params ) { gl::qglGetTexEnvfv( target, pname, params ); }
void WINAPI glGetTexEnviv( GLenum target, GLenum pname, GLint *params ) { gl::qglGetTexEnviv( target, pname, params ); }
void WINAPI glGetTexGendv( GLenum coord, GLenum pname, GLdouble *params ) { gl::qglGetTexGendv( coord, pname, params ); }
void WINAPI glGetTexGenfv( GLenum coord, GLenum pname, GLfloat *params ) { gl::qglGetTexGenfv( coord, pname, params ); }
void WINAPI glGetTexGeniv( GLenum coord, GLenum pname, GLint *params ) { gl::qglGetTexGeniv( coord, pname, params ); }
void WINAPI glGetTexImage( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels ) { gl::qglGetTexImage( target, level, format, type, pixels ); }
void WINAPI glGetTexLevelParameterfv( GLenum target, GLint level, GLenum pname, GLfloat *params ) { gl::qglGetTexLevelParameterfv( target, level, pname, params ); }
void WINAPI glGetTexLevelParameteriv( GLenum target, GLint level, GLenum pname, GLint *params ) { gl::qglGetTexLevelParameteriv( target, level, pname, params ); }
void WINAPI glGetTexParameterfv( GLenum target, GLenum pname, GLfloat *params ) { gl::qglGetTexParameterfv( target, pname, params ); }
void WINAPI glGetTexParameteriv( GLenum target, GLenum pname, GLint *params ) { gl::qglGetTexParameteriv( target, pname, params ); }
void WINAPI glHint( GLenum target, GLenum mode ) { gl::qglHint( target, mode ); }
void WINAPI glIndexMask( GLuint mask ) { gl::qglIndexMask( mask ); }
void WINAPI glIndexPointer( GLenum type, GLsizei stride, const GLvoid *pointer ) { gl::qglIndexPointer( type, stride, pointer ); }
void WINAPI glIndexd( GLdouble c ) { gl::qglIndexd( c ); }
void WINAPI glIndexdv( const GLdouble *c ) { gl::qglIndexdv( c ); }
void WINAPI glIndexf( GLfloat c ) { gl::qglIndexf( c ); }
void WINAPI glIndexfv( const GLfloat *c ) { gl::qglIndexfv( c ); }
void WINAPI glIndexi( GLint c ) { gl::qglIndexi( c ); }
void WINAPI glIndexiv( const GLint *c ) { gl::qglIndexiv( c ); }
void WINAPI glIndexs( GLshort c ) { gl::qglIndexs( c ); }
void WINAPI glIndexsv( const GLshort *c ) { gl::qglIndexsv( c ); }
void WINAPI glIndexub( GLubyte c ) { gl::qglIndexub( c ); }
void WINAPI glIndexubv( const GLubyte *c ) { gl::qglIndexubv( c ); }
void WINAPI glInitNames( void ) { gl::qglInitNames(); }
void WINAPI glInterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer ) { gl::qglInterleavedArrays( format, stride, pointer ); }
GLboolean WINAPI glIsEnabled( GLenum cap ) { return gl::qglIsEnabled( cap ); }
GLboolean WINAPI glIsList( GLuint list ) { return gl::qglIsList( list ); }
GLboolean WINAPI glIsTexture( GLuint texture ) { return gl::qglIsTexture( texture ); }
void WINAPI glLightModelf( GLenum pname, GLfloat param ) { gl::qglLightModelf( pname, param ); }
void WINAPI glLightModelfv( GLenum pname, const GLfloat *params ) { gl::qglLightModelfv( pname, params ); }
void WINAPI glLightModeli( GLenum pname, GLint param ) { gl::qglLightModeli( pname, param ); }
void WINAPI glLightModeliv( GLenum pname, const GLint *params ) { gl::qglLightModeliv( pname, params ); }
void WINAPI glLightf( GLenum light, GLenum pname, GLfloat param ) { gl::qglLightf( light, pname, param ); }
void WINAPI glLightfv( GLenum light, GLenum pname, const GLfloat *params ) { gl::qglLightfv( light, pname, params ); }
void WINAPI glLighti( GLenum light, GLenum pname, GLint param ) { gl::qglLighti( light, pname, param ); }
void WINAPI glLightiv( GLenum light, GLenum pname, const GLint *params ) { gl::qglLightiv( light, pname, params ); }
void WINAPI glLineStipple( GLint factor, GLushort pattern ) { gl::qglLineStipple( factor, pattern ); }
void WINAPI glLineWidth( GLfloat width ) { gl::qglLineWidth( width ); }
void WINAPI glListBase( GLuint base ) { gl::qglListBase( base ); }
void WINAPI glLoadIdentity( void ) { gl::qglLoadIdentity(); }
void WINAPI glLoadName( GLuint name ) { gl::qglLoadName( name ); }
void WINAPI glLogicOp( GLenum opcode ) { gl::qglLogicOp( opcode ); }
void WINAPI glMap1d( GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points ) { gl::qglMap1d( target, u1, u2, stride, order, points ); }
void WINAPI glMap1f( GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points ) { gl::qglMap1f( target, u1, u2, stride, order, points ); }
void WINAPI glMap2d( GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points ) { gl::qglMap2d( target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points ); }
void WINAPI glMap2f( GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points ) { gl::qglMap2f( target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points ); }
void WINAPI glMapGrid1d( GLint un, GLdouble u1, GLdouble u2 ) { gl::qglMapGrid1d( un, u1, u2 ); }
void WINAPI glMapGrid1f( GLint un, GLfloat u1, GLfloat u2 ) { gl::qglMapGrid1f( un, u1, u2 ); }
void WINAPI glMapGrid2d( GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2 ) { gl::qglMapGrid2d( un, u1, u2, vn, v1, v2 ); }
void WINAPI glMapGrid2f( GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2 ) { gl::qglMapGrid2f( un, u1, u2, vn, v1, v2 ); }
void WINAPI glMaterialf( GLenum face, GLenum pname, GLfloat param ) { gl::qglMaterialf( face, pname, param ); }
void WINAPI glMaterialfv( GLenum face, GLenum pname, const GLfloat *params ) { gl::qglMaterialfv( face, pname, params ); }
void WINAPI glMateriali( GLenum face, GLenum pname, GLint param ) { gl::qglMateriali( face, pname, param ); }
void WINAPI glMaterialiv( GLenum face, GLenum pname, const GLint *params ) { gl::qglMaterialiv( face, pname, params ); }
void WINAPI glMultMatrixd( const GLdouble *m ) { gl::qglMultMatrixd( m ); }
void WINAPI glMultMatrixf( const GLfloat *m ) { gl::qglMultMatrixf( m ); }
void WINAPI glNewList( GLuint list, GLenum mode ) { gl::qglNewList( list, mode ); }
void WINAPI glNormal3b( GLbyte nx, GLbyte ny, GLbyte nz ) { gl::qglNormal3b( nx, ny, nz ); }
void WINAPI glNormal3bv( const GLbyte *v ) { gl::qglNormal3bv( v ); }
void WINAPI glNormal3d( GLdouble nx, GLdouble ny, GLdouble nz ) { gl::qglNormal3d( nx, ny, nz ); }
void WINAPI glNormal3dv( const GLdouble *v ) { gl::qglNormal3dv( v ); }
void WINAPI glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz ) { gl::qglNormal3f( nx, ny, nz ); }
void WINAPI glNormal3fv( const GLfloat *v ) { gl::qglNormal3fv( v ); }
void WINAPI glNormal3i( GLint nx, GLint ny, GLint nz ) { gl::qglNormal3i( nx, ny, nz ); }
void WINAPI glNormal3iv( const GLint *v ) { gl::qglNormal3iv( v ); }
void WINAPI glNormal3s( GLshort nx, GLshort ny, GLshort nz ) { gl::qglNormal3s( nx, ny, nz ); }
void WINAPI glNormal3sv( const GLshort *v ) { gl::qglNormal3sv( v ); }
void WINAPI glNormalPointer( GLenum type, GLsizei stride, const GLvoid *pointer ) { gl::qglNormalPointer( type, stride, pointer ); }
void WINAPI glPassThrough( GLfloat token ) { gl::qglPassThrough( token ); }
void WINAPI glPixelMapfv( GLenum map, GLsizei mapsize, const GLfloat *values ) { gl::qglPixelMapfv( map, mapsize, values ); }
void WINAPI glPixelMapuiv( GLenum map, GLsizei mapsize, const GLuint *values ) { gl::qglPixelMapuiv( map, mapsize, values ); }
void WINAPI glPixelMapusv( GLenum map, GLsizei mapsize, const GLushort *values ) { gl::qglPixelMapusv( map, mapsize, values ); }
void WINAPI glPixelStoref( GLenum pname, GLfloat param ) { gl::qglPixelStoref( pname, param ); }
void WINAPI glPixelStorei( GLenum pname, GLint param ) { gl::qglPixelStorei( pname, param ); }
void WINAPI glPixelTransferf( GLenum pname, GLfloat param ) { gl::qglPixelTransferf( pname, param ); }
void WINAPI glPixelTransferi( GLenum pname, GLint param ) { gl::qglPixelTransferi( pname, param ); }
void WINAPI glPixelZoom( GLfloat xfactor, GLfloat yfactor ) { gl::qglPixelZoom( xfactor, yfactor ); }
void WINAPI glPointSize( GLfloat size ) { gl::qglPointSize( size ); }
void WINAPI glPolygonMode( GLenum face, GLenum mode ) { gl::qglPolygonMode( face, mode ); }
void WINAPI glPolygonOffset( GLfloat factor, GLfloat units ) { gl::qglPolygonOffset( factor, units ); }
void WINAPI glPolygonStipple( const GLubyte *mask ) { gl::qglPolygonStipple( mask ); }
void WINAPI glPopAttrib( void ) { gl::qglPopAttrib(); }
void WINAPI glPopClientAttrib( void ) { gl::qglPopClientAttrib(); }
void WINAPI glPopMatrix( void ) { gl::qglPopMatrix(); }
void WINAPI glPopName( void ) { gl::qglPopName(); }
void WINAPI glPrioritizeTextures( GLsizei n, const GLuint *textures, const GLclampf *priorities ) { gl::qglPrioritizeTextures( n, textures, priorities ); }
void WINAPI glPushAttrib( GLbitfield mask ) { gl::qglPushAttrib( mask ); }
void WINAPI glPushClientAttrib( GLbitfield mask ) { gl::qglPushClientAttrib( mask ); }
void WINAPI glPushMatrix( void ) { gl::qglPushMatrix(); }
void WINAPI glPushName( GLuint name ) { gl::qglPushName( name ); }
void WINAPI glRasterPos2d( GLdouble x, GLdouble y ) { gl::qglRasterPos2d( x, y ); }
void WINAPI glRasterPos2dv( const GLdouble *v ) { gl::qglRasterPos2dv( v ); }
void WINAPI glRasterPos2f( GLfloat x, GLfloat y ) { gl::qglRasterPos2f( x, y ); }
void WINAPI glRasterPos2fv( const GLfloat *v ) { gl::qglRasterPos2fv( v ); }
void WINAPI glRasterPos2i( GLint x, GLint y ) { gl::qglRasterPos2i( x, y ); }
void WINAPI glRasterPos2iv( const GLint *v ) { gl::qglRasterPos2iv( v ); }
void WINAPI glRasterPos2s( GLshort x, GLshort y ) { gl::qglRasterPos2s( x, y ); }
void WINAPI glRasterPos2sv( const GLshort *v ) { gl::qglRasterPos2sv( v ); }
void WINAPI glRasterPos3d( GLdouble x, GLdouble y, GLdouble z ) { gl::qglRasterPos3d( x, y, z ); }
void WINAPI glRasterPos3dv( const GLdouble *v ) { gl::qglRasterPos3dv( v ); }
void WINAPI glRasterPos3f( GLfloat x, GLfloat y, GLfloat z ) { gl::qglRasterPos3f( x, y, z ); }
void WINAPI glRasterPos3fv( const GLfloat *v ) { gl::qglRasterPos3fv( v ); }
void WINAPI glRasterPos3i( GLint x, GLint y, GLint z ) { gl::qglRasterPos3i( x, y, z ); }
void WINAPI glRasterPos3iv( const GLint *v ) { gl::qglRasterPos3iv( v ); }
void WINAPI glRasterPos3s( GLshort x, GLshort y, GLshort z ) { gl::qglRasterPos3s( x, y, z ); }
void WINAPI glRasterPos3sv( const GLshort *v ) { gl::qglRasterPos3sv( v ); }
void WINAPI glRasterPos4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w ) { gl::qglRasterPos4d( x, y, z, w ); }
void WINAPI glRasterPos4dv( const GLdouble *v ) { gl::qglRasterPos4dv( v ); }
void WINAPI glRasterPos4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w ) { gl::qglRasterPos4f( x, y, z, w ); }
void WINAPI glRasterPos4fv( const GLfloat *v ) { gl::qglRasterPos4fv( v ); }
void WINAPI glRasterPos4i( GLint x, GLint y, GLint z, GLint w ) { gl::qglRasterPos4i( x, y, z, w ); }
void WINAPI glRasterPos4iv( const GLint *v ) { gl::qglRasterPos4iv( v ); }
void WINAPI glRasterPos4s( GLshort x, GLshort y, GLshort z, GLshort w ) { gl::qglRasterPos4s( x, y, z, w ); }
void WINAPI glRasterPos4sv( const GLshort *v ) { gl::qglRasterPos4sv( v ); }
void WINAPI glReadBuffer( GLenum mode ) { gl::qglReadBuffer( mode ); }
void WINAPI glReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels ) { gl::qglReadPixels( x, y, width, height, format, type, pixels ); }
void WINAPI glRectd( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 ) { gl::qglRectd( x1, y1, x2, y2 ); }
void WINAPI glRectdv( const GLdouble *v1, const GLdouble *v2 ) { gl::qglRectdv( v1, v2 ); }
void WINAPI glRectf( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 ) { gl::qglRectf( x1, y1, x2, y2 ); }
void WINAPI glRectfv( const GLfloat *v1, const GLfloat *v2 ) { gl::qglRectfv( v1, v2 ); }
void WINAPI glRecti( GLint x1, GLint y1, GLint x2, GLint y2 ) { gl::qglRecti( x1, y1, x2, y2 ); }
void WINAPI glRectiv( const GLint *v1, const GLint *v2 ) { gl::qglRectiv( v1, v2 ); }
void WINAPI glRects( GLshort x1, GLshort y1, GLshort x2, GLshort y2 ) { gl::qglRects( x1, y1, x2, y2 ); }
void WINAPI glRectsv( const GLshort *v1, const GLshort *v2 ) { gl::qglRectsv( v1, v2 ); }
GLint WINAPI glRenderMode( GLenum mode ) { return gl::qglRenderMode( mode ); }
void WINAPI glRotated( GLdouble angle, GLdouble x, GLdouble y, GLdouble z ) { gl::qglRotated( angle, x, y, z ); }
void WINAPI glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z ) { gl::qglRotatef( angle, x, y, z ); }
void WINAPI glScaled( GLdouble x, GLdouble y, GLdouble z ) { gl::qglScaled( x, y, z ); }
void WINAPI glScalef( GLfloat x, GLfloat y, GLfloat z ) { gl::qglScalef( x, y, z ); }
void WINAPI glScissor( GLint x, GLint y, GLsizei width, GLsizei height ) { gl::qglScissor( x, y, width, height ); }
void WINAPI glSelectBuffer( GLsizei size, GLuint *buffer ) { gl::qglSelectBuffer( size, buffer ); }
void WINAPI glShadeModel( GLenum mode ) { gl::qglShadeModel( mode ); }
void WINAPI glStencilFunc( GLenum func, GLint ref, GLuint mask ) { gl::qglStencilFunc( func, ref, mask ); }
void WINAPI glStencilMask( GLuint mask ) { gl::qglStencilMask( mask ); }
void WINAPI glStencilOp( GLenum fail, GLenum zfail, GLenum zpass ) { gl::qglStencilOp( fail, zfail, zpass ); }
void WINAPI glTexCoord1d( GLdouble s ) { gl::qglTexCoord1d( s ); }
void WINAPI glTexCoord1dv( const GLdouble *v ) { gl::qglTexCoord1dv( v ); }
void WINAPI glTexCoord1f( GLfloat s ) { gl::qglTexCoord1f( s ); }
void WINAPI glTexCoord1fv( const GLfloat *v ) { gl::qglTexCoord1fv( v ); }
void WINAPI glTexCoord1i( GLint s ) { gl::qglTexCoord1i( s ); }
void WINAPI glTexCoord1iv( const GLint *v ) { gl::qglTexCoord1iv( v ); }
void WINAPI glTexCoord1s( GLshort s ) { gl::qglTexCoord1s( s ); }
void WINAPI glTexCoord1sv( const GLshort *v ) { gl::qglTexCoord1sv( v ); }
void WINAPI glTexCoord2d( GLdouble s, GLdouble t ) { gl::qglTexCoord2d( s, t ); }
void WINAPI glTexCoord2dv( const GLdouble *v ) { gl::qglTexCoord2dv( v ); }
void WINAPI glTexCoord2f( GLfloat s, GLfloat t ) { gl::qglTexCoord2f( s, t ); }
void WINAPI glTexCoord2fv( const GLfloat *v ) { gl::qglTexCoord2fv( v ); }
void WINAPI glTexCoord2i( GLint s, GLint t ) { gl::qglTexCoord2i( s, t ); }
void WINAPI glTexCoord2iv( const GLint *v ) { gl::qglTexCoord2iv( v ); }
void WINAPI glTexCoord2s( GLshort s, GLshort t ) { gl::qglTexCoord2s( s, t ); }
void WINAPI glTexCoord2sv( const GLshort *v ) { gl::qglTexCoord2sv( v ); }
void WINAPI glTexCoord3d( GLdouble s, GLdouble t, GLdouble r ) { gl::qglTexCoord3d( s, t, r ); }
void WINAPI glTexCoord3dv( const GLdouble *v ) { gl::qglTexCoord3dv( v ); }
void WINAPI glTexCoord3f( GLfloat s, GLfloat t, GLfloat r ) { gl::qglTexCoord3f( s, t, r ); }
void WINAPI glTexCoord3fv( const GLfloat *v ) { gl::qglTexCoord3fv( v ); }
void WINAPI glTexCoord3i( GLint s, GLint t, GLint r ) { gl::qglTexCoord3i( s, t, r ); }
void WINAPI glTexCoord3iv( const GLint *v ) { gl::qglTexCoord3iv( v ); }
void WINAPI glTexCoord3s( GLshort s, GLshort t, GLshort r ) { gl::qglTexCoord3s( s, t, r ); }
void WINAPI glTexCoord3sv( const GLshort *v ) { gl::qglTexCoord3sv( v ); }
void WINAPI glTexCoord4d( GLdouble s, GLdouble t, GLdouble r, GLdouble q ) { gl::qglTexCoord4d( s, t, r, q ); }
void WINAPI glTexCoord4dv( const GLdouble *v ) { gl::qglTexCoord4dv( v ); }
void WINAPI glTexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q ) { gl::qglTexCoord4f( s, t, r, q ); }
void WINAPI glTexCoord4fv( const GLfloat *v ) { gl::qglTexCoord4fv( v ); }
void WINAPI glTexCoord4i( GLint s, GLint t, GLint r, GLint q ) { gl::qglTexCoord4i( s, t, r, q ); }
void WINAPI glTexCoord4iv( const GLint *v ) { gl::qglTexCoord4iv( v ); }
void WINAPI glTexCoord4s( GLshort s, GLshort t, GLshort r, GLshort q ) { gl::qglTexCoord4s( s, t, r, q ); }
void WINAPI glTexCoord4sv( const GLshort *v ) { gl::qglTexCoord4sv( v ); }
void WINAPI glTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer ) { gl::qglTexCoordPointer( size, type, stride, pointer ); }
void WINAPI glTexEnvf( GLenum target, GLenum pname, GLfloat param ) { gl::qglTexEnvf( target, pname, param ); }
void WINAPI glTexEnvfv( GLenum target, GLenum pname, const GLfloat *params ) { gl::qglTexEnvfv( target, pname, params ); }
void WINAPI glTexEnvi( GLenum target, GLenum pname, GLint param ) { gl::qglTexEnvi( target, pname, param ); }
void WINAPI glTexEnviv( GLenum target, GLenum pname, const GLint *params ) { gl::qglTexEnviv( target, pname, params ); }
void WINAPI glTexGend( GLenum coord, GLenum pname, GLdouble param ) { gl::qglTexGend( coord, pname, param ); }
void WINAPI glTexGendv( GLenum coord, GLenum pname, const GLdouble *params ) { gl::qglTexGendv( coord, pname, params ); }
void WINAPI glTexGenf( GLenum coord, GLenum pname, GLfloat param ) { gl::qglTexGenf( coord, pname, param ); }
void WINAPI glTexGenfv( GLenum coord, GLenum pname, const GLfloat *params ) { gl::qglTexGenfv( coord, pname, params ); }
void WINAPI glTexGeni( GLenum coord, GLenum pname, GLint param ) { gl::qglTexGeni( coord, pname, param ); }
void WINAPI glTexGeniv( GLenum coord, GLenum pname, const GLint *params ) { gl::qglTexGeniv( coord, pname, params ); }
void WINAPI glTexImage1D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels ) { gl::qglTexImage1D( target, level, internalformat, width, border, format, type, pixels ); }
void WINAPI glTexParameterfv( GLenum target, GLenum pname, const GLfloat *params ) { gl::qglTexParameterfv( target, pname, params ); }
void WINAPI glTexParameteriv( GLenum target, GLenum pname, const GLint *params ) { gl::qglTexParameteriv( target, pname, params ); }
void WINAPI glTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels ) { gl::qglTexSubImage1D( target, level, xoffset, width, format, type, pixels ); }
void WINAPI glTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ) { gl::qglTexSubImage2D( target, level, xoffset, yoffset, width, height, format, type, pixels ); }
void WINAPI glTranslated( GLdouble x, GLdouble y, GLdouble z ) { gl::qglTranslated( x, y, z ); }
void WINAPI glTranslatef( GLfloat x, GLfloat y, GLfloat z ) { gl::qglTranslatef( x, y, z ); }
void WINAPI glVertex2d( GLdouble x, GLdouble y ) { gl::qglVertex2d( x, y ); }
void WINAPI glVertex2dv( const GLdouble *v ) { gl::qglVertex2dv( v ); }
void WINAPI glVertex2f( GLfloat x, GLfloat y ) { gl::qglVertex2f( x, y ); }
void WINAPI glVertex2fv( const GLfloat *v ) { gl::qglVertex2fv( v ); }
void WINAPI glVertex2i( GLint x, GLint y ) { gl::qglVertex2i( x, y ); }
void WINAPI glVertex2iv( const GLint *v ) { gl::qglVertex2iv( v ); }
void WINAPI glVertex2s( GLshort x, GLshort y ) { gl::qglVertex2s( x, y ); }
void WINAPI glVertex2sv( const GLshort *v ) { gl::qglVertex2sv( v ); }
void WINAPI glVertex3d( GLdouble x, GLdouble y, GLdouble z ) { gl::qglVertex3d( x, y, z ); }
void WINAPI glVertex3dv( const GLdouble *v ) { gl::qglVertex3dv( v ); }
void WINAPI glVertex3f( GLfloat x, GLfloat y, GLfloat z ) { gl::qglVertex3f( x, y, z ); }
void WINAPI glVertex3fv( const GLfloat *v ) { gl::qglVertex3fv( v ); }
void WINAPI glVertex3i( GLint x, GLint y, GLint z ) { gl::qglVertex3i( x, y, z ); }
void WINAPI glVertex3iv( const GLint *v ) { gl::qglVertex3iv( v ); }
void WINAPI glVertex3s( GLshort x, GLshort y, GLshort z ) { gl::qglVertex3s( x, y, z ); }
void WINAPI glVertex3sv( const GLshort *v ) { gl::qglVertex3sv( v ); }
void WINAPI glVertex4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w ) { gl::qglVertex4d( x, y, z, w ); }
void WINAPI glVertex4dv( const GLdouble *v ) { gl::qglVertex4dv( v ); }
void WINAPI glVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w ) { gl::qglVertex4f( x, y, z, w ); }
void WINAPI glVertex4fv( const GLfloat *v ) { gl::qglVertex4fv( v ); }
void WINAPI glVertex4i( GLint x, GLint y, GLint z, GLint w ) { gl::qglVertex4i( x, y, z, w ); }
void WINAPI glVertex4iv( const GLint *v ) { gl::qglVertex4iv( v ); }
void WINAPI glVertex4s( GLshort x, GLshort y, GLshort z, GLshort w ) { gl::qglVertex4s( x, y, z, w ); }
void WINAPI glVertex4sv( const GLshort *v ) { gl::qglVertex4sv( v ); }
void WINAPI glVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer ) { gl::qglVertexPointer( size, type, stride, pointer ); }
void WINAPI glViewport( GLint x, GLint y, GLsizei width, GLsizei height ) { gl::qglViewport( x, y, width, height ); }
void WINAPI glDebugEntry( DWORD dwArg1, DWORD dwArg2 ) {}
