/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_draw.c -- drawing

#include "tr_local.h"

vec4_t r_colorWhite = { 1.0, 1.0, 1.0, 1.0 };

/*
================
Draw_SetColor
================
*/
void Draw_SetColor( const vec4_t rgba ) {
#if 1
	if( !rgba ) {
		rgba = r_colorWhite;
	}

	backEnd.color2D[ 0 ] = rgba[ 0 ] * tr.identityLight;
	backEnd.color2D[ 1 ] = rgba[ 1 ] * tr.identityLight;
	backEnd.color2D[ 2 ] = rgba[ 2 ] * tr.identityLight;
	backEnd.color2D[ 3 ] = rgba[ 3 ];
	qglColor4fv( backEnd.color2D );
#else
	RE_SetColor( rgba );
#endif
}

/*
================
Draw_StretchPic
================
*/
void Draw_StretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader ) {
#if 1
	shader_t	*shader;

	R_SyncRenderThread();

	if( hShader ) {
		shader = R_GetShaderByHandle( hShader );
	} else {
		shader = tr.defaultShader;
	}

	if( w <= 0 ) {
		w = shader->unfoggedStages[ 0 ]->bundle[ 0 ].image[ 0 ]->width;
		h = shader->unfoggedStages[ 0 ]->bundle[ 0 ].image[ 0 ]->height;
	}

	// draw the pic
	RB_Color4f( backEnd.color2D[ 0 ], backEnd.color2D[ 1 ], backEnd.color2D[ 2 ], backEnd.color2D[ 3 ] );
	RB_BeginSurface( shader, 0, 0 );

	RB_Texcoord2f( s1, t1 );
	RB_Vertex2f( x, y );

	RB_Texcoord2f( s2, t1 );
	RB_Vertex2f( x + w, y );

	RB_Texcoord2f( s1, t2 );
	RB_Vertex2f( x, y + h );

	RB_Texcoord2f( s2, t2 );
	RB_Vertex2f( x + w, y + h );

	RB_StreamEnd();
#else
	RE_StretchPic( x, y, w, h, s1, t1, s2, t2, hShader );
#endif
}

/*
================
Draw_TilePic
================
*/
void Draw_TilePic( float x, float y, float w, float h , qhandle_t hShader ) {
	shader_t	*shader;
	float		picw, pich;

	R_SyncRenderThread();

	if( hShader ) {
		shader = R_GetShaderByHandle( hShader );
	}
	else {
		shader = tr.defaultShader;
	}

	if( w <= 0 ) {
		w = shader->unfoggedStages[ 0 ]->bundle[ 0 ].image[ 0 ]->width;
		h = shader->unfoggedStages[ 0 ]->bundle[ 0 ].image[ 0 ]->height;
	}

	picw = shader->unfoggedStages[ 0 ]->bundle[ 0 ].image[ 0 ]->uploadWidth;
	pich = shader->unfoggedStages[ 0 ]->bundle[ 0 ].image[ 0 ]->uploadHeight;

	// draw the pic
	RB_Color4f( backEnd.color2D[ 0 ], backEnd.color2D[ 1 ], backEnd.color2D[ 2 ], backEnd.color2D[ 3 ] );

	RB_StreamBegin( shader );

	RB_Texcoord2f( x / picw, y / pich );
	RB_Vertex2f( x, y );

	RB_Texcoord2f( ( x + w ) / picw, y / pich );
	RB_Vertex2f( x + w, y );

	RB_Texcoord2f( x / picw, ( y + h ) / pich );
	RB_Vertex2f( x, y + h );

	RB_Texcoord2f( ( x + w ) / picw, ( y + h ) / pich );
	RB_Vertex2f( x + w, y + h );

	RB_StreamEnd();
}

/*
================
Draw_TilePicOffset
================
*/
void Draw_TilePicOffset( float x, float y, float w, float h, qhandle_t hShader, int offsetX, int offsetY ) {
	shader_t	*shader;
	float		picw, pich;

	R_SyncRenderThread();

	if( hShader ) {
		shader = R_GetShaderByHandle( hShader );
	} else {
		shader = tr.defaultShader;
	}

	if( w <= 0 ) {
		w = shader->unfoggedStages[ 0 ]->bundle[ 0 ].image[ 0 ]->width;
		h = shader->unfoggedStages[ 0 ]->bundle[ 0 ].image[ 0 ]->height;
	}

	picw = shader->unfoggedStages[ 0 ]->bundle[ 0 ].image[ 0 ]->uploadWidth;
	pich = shader->unfoggedStages[ 0 ]->bundle[ 0 ].image[ 0 ]->uploadHeight;

	// draw the pic
	RB_Color4f( backEnd.color2D[ 0 ], backEnd.color2D[ 1 ], backEnd.color2D[ 2 ], backEnd.color2D[ 3 ] );

	RB_StreamBegin( shader );

	RB_Texcoord2f( x / picw, y / pich );
	RB_Vertex2f( x + offsetX, y + offsetY );

	RB_Texcoord2f( ( x + w ) / picw, y / pich );
	RB_Vertex2f( x + offsetX + w, y + offsetY );

	RB_Texcoord2f( x / picw, ( y + h ) / pich );
	RB_Vertex2f( x + offsetX, y + offsetY + h );

	RB_Texcoord2f( ( x + w ) / picw, ( y + h ) / pich );
	RB_Vertex2f( x + offsetX + w, y + offsetY + h );

	RB_StreamEnd();
}

/*
================
Draw_TrianglePic
================
*/
void Draw_TrianglePic( const vec2_t vPoints[ 3 ], const vec2_t vTexCoords[ 3 ], qhandle_t hShader ) {
	int			i;
	shader_t	*shader;

	R_SyncRenderThread();

	if( hShader ) {
		shader = R_GetShaderByHandle( hShader );
	} else {
		shader = tr.defaultShader;
	}

	// draw the pic
	RB_Color4f( backEnd.color2D[ 0 ], backEnd.color2D[ 1 ], backEnd.color2D[ 2 ], backEnd.color2D[ 3 ] );

	RB_BeginSurface( shader, 0, 0 );

	for( i = 0; i < 3; i++ ) {
		RB_Texcoord2f( vTexCoords[ i ][ 0 ], vTexCoords[ i ][ 1 ] );
		RB_Vertex2f( vPoints[ i ][ 0 ], vPoints[ i ][ 1 ] );
	}

	RB_StreamEnd();
}

/*
================
RE_DrawBackground_TexSubImage
================
*/
void RE_DrawBackground_TexSubImage( int cols, int rows, int bgr, byte *data ) {
	GLenum	format;
	int		w, h;

	w = glConfig.vidWidth;
	h = glConfig.vidHeight;

	R_SyncRenderThread();
	qglFinish();

	if( bgr ) {
		format = GL_BGR_EXT;
	} else {
		format = GL_RGB;
	}

	GL_Bind( tr.scratchImage[ 0 ] );

	if( cols == tr.scratchImage[ 0 ]->width && rows == tr.scratchImage[ 0 ]->height && format == tr.scratchImage[ 0 ]->internalFormat )
	{
		qglTexSubImage2D( 3553, 0, 0, 0, cols, rows, format, 5121, data );
	}
	else
	{
		tr.scratchImage[ 0 ]->uploadWidth = cols;
		tr.scratchImage[ 0 ]->uploadHeight = rows;
		tr.scratchImage[ 0 ]->internalFormat = format;
		qglTexImage2D( GL_TEXTURE_2D, 0, 3, cols, rows, 0, format, 5121, data );
		qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 9729.0 );
		qglTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 9729.0 );
	}

	qglDisable( GL_CULL_FACE );
	qglDisable( GL_DEPTH_TEST );
	qglEnable( GL_TEXTURE_2D );

	qglBegin( GL_QUADS );

	qglTexCoord2f( 0.5 / ( GLfloat )cols, ( ( GLfloat )rows - 0.5 ) / rows );
	qglVertex2f( 0, 0 );

	qglTexCoord2f( ( ( GLfloat )cols - 0.5 ) / cols, ( ( GLfloat )rows - 0.5 ) / rows );
	qglVertex2f( w, 0 );

	qglTexCoord2f( ( ( GLfloat )cols - 0.5 ) / cols, 0.5 / ( GLfloat )rows );
	qglVertex2f( w, h );

	qglTexCoord2f( 0.5 / ( GLfloat )rows, 0.5 / ( GLfloat )rows );
	qglVertex2f( 0, h );

	qglEnd();
}

/*
================
RE_DrawBackground_DrawPixels
================
*/
void RE_DrawBackground_DrawPixels( int cols, int rows, int bgr, byte *data ) {
	// FIXME: stub
}

/*
================
AddBox
================
*/
void AddBox( float x, float y, float w, float h ) {
	R_SyncRenderThread();

	qglColor4fv( backEnd.color2D );
	qglDisable( GL_TEXTURE_2D );
	GL_State( 0x422 );

	qglBegin( GL_QUADS );

	qglVertex2f( x, y );
	qglVertex2f( x + w, y );
	qglVertex2f( x + w, y + h );
	qglVertex2f( x, y + h );

	qglEnd();

	qglEnable( GL_TEXTURE_2D );
}

/*
================
DrawBox
================
*/
void DrawBox( float x, float y, float w, float h ) {
	R_SyncRenderThread();

	qglColor4fv( backEnd.color2D );
	qglDisable( GL_TEXTURE_2D );
	GL_State( 0x465 );

	qglBegin( GL_QUADS );

	qglVertex2f( x, y );
	qglVertex2f( x + w, y );
	qglVertex2f( x + w, y + h );
	qglVertex2f( x, y + h );

	qglEnd();

	qglEnable( GL_TEXTURE_2D );
}

/*
================
DrawLineLoop
================
*/
void DrawLineLoop( const vec2_t *points, int count, int stipple_factor, int stipple_mask ) {
	int		i;

	R_SyncRenderThread();

	qglDisable( GL_TEXTURE_2D );

	if( stipple_factor ) {
		qglEnable( GL_LINE_STIPPLE );
		qglLineStipple( stipple_factor, stipple_mask );
	}

	qglBegin( GL_LINE_LOOP );

	for( i = 0; i < count; i++ ) {
		qglVertex2i( points[ i ][ 0 ], points[ i ][ 1 ] );
	}

	qglEnd();

	qglEnable( GL_TEXTURE_2D );

	if( stipple_factor ) {
		qglDisable( GL_LINE_STIPPLE );
	}
}

/*
================
Set2DWindow
================
*/
void Set2DWindow( int x, int y, int w, int h, float left, float right, float bottom, float top, float n, float f ) {
	// FIXME: unimplemented
}

/*
================
RE_Scissor
================
*/
void RE_Scissor( int x, int y, int width, int height ) {
	qglEnable( GL_SCISSOR_TEST );
	qglScissor( x, y, width, height );
}
