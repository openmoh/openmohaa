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

	R_IssuePendingRenderCommands();

	if( hShader ) {
		shader = R_GetShaderByHandle( hShader );
	} else {
		shader = tr.defaultShader;
	}

	if( w <= 0 ) {
		w = shader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->width;
		h = shader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->height;
	}

	RB_Prepare2D();

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

	R_IssuePendingRenderCommands();

	if( hShader ) {
		shader = R_GetShaderByHandle( hShader );
	}
	else {
		shader = tr.defaultShader;
	}

	if( w <= 0 ) {
		w = shader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->width;
		h = shader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->height;
	}

	picw = shader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->uploadWidth;
	pich = shader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->uploadHeight;

	RB_Prepare2D();

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

	R_IssuePendingRenderCommands();

	if( hShader ) {
		shader = R_GetShaderByHandle( hShader );
	} else {
		shader = tr.defaultShader;
	}

	if( w <= 0 ) {
		w = shader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->width;
		h = shader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->height;
	}

	picw = shader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->uploadWidth;
	pich = shader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->uploadHeight;

	RB_Prepare2D();

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

	R_IssuePendingRenderCommands();

	if( hShader ) {
		shader = R_GetShaderByHandle( hShader );
	} else {
		shader = tr.defaultShader;
	}

	RB_Prepare2D();

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

	R_IssuePendingRenderCommands();
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
=============
RE_StretchRaw

Stretches a raw 32 bit power of 2 bitmap image over the given screen rectangle.
Used for cinematics.
=============
*/
void RE_StretchRaw( int x, int y, int w, int h, int cols, int rows, int components, const byte *data ) {
	int			i, j;
	int			start, end;
	vec4_t quadVerts[4];
	vec2_t texCoords[4];

	if ( !tr.registered ) {
		return;
	}
	R_IssuePendingRenderCommands();

	if ( tess.numIndexes ) {
		RB_EndSurface();
	}

	// we definately want to sync every frame for the cinematics
	qglFinish();

	start = 0;
	if ( r_speeds->integer ) {
		start = ri.Milliseconds();
	}

	// make sure rows and cols are powers of 2
	for ( i = 0 ; ( 1 << i ) < cols ; i++ ) {
	}
	for ( j = 0 ; ( 1 << j ) < rows ; j++ ) {
	}
	if ( ( 1 << i ) != cols || ( 1 << j ) != rows) {
		ri.Error (ERR_DROP, "Draw_StretchRaw: size not a power of 2: %i by %i", cols, rows);
	}

	RE_UploadCinematic (w, h, cols, rows, data, 0, qtrue);

	if ( r_speeds->integer ) {
		end = ri.Milliseconds();
		ri.Printf( PRINT_ALL, "qglTexSubImage2D %i, %i: %i msec\n", cols, rows, end - start );
	}

	RB_Prepare2D();

	RB_SetGL2D();

	VectorSet4(quadVerts[0], x,     y,     0.0f, 1.0f);
	VectorSet4(quadVerts[1], x + w, y,     0.0f, 1.0f);
	VectorSet4(quadVerts[2], x + w, y + h, 0.0f, 1.0f);
	VectorSet4(quadVerts[3], x,     y + h, 0.0f, 1.0f);

	VectorSet2(texCoords[0], 0.5f / cols,          0.5f / rows);
	VectorSet2(texCoords[1], (cols - 0.5f) / cols, 0.5f / rows);
	VectorSet2(texCoords[2], (cols - 0.5f) / cols, (rows - 0.5f) / rows);
	VectorSet2(texCoords[3], 0.5f / cols,          (rows - 0.5f) / rows);

	GLSL_BindProgram(&tr.textureColorShader);
	
	GLSL_SetUniformMat4(&tr.textureColorShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
	GLSL_SetUniformVec4(&tr.textureColorShader, UNIFORM_COLOR, colorWhite);

	RB_InstantQuad2(quadVerts, texCoords);
}

/*
================
AddBox
================
*/
void AddBox( float x, float y, float w, float h ) {
	R_IssuePendingRenderCommands();

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
	R_IssuePendingRenderCommands();

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

	R_IssuePendingRenderCommands();

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
	matrix_t matrix;

	backEnd.projection2D = qtrue;
	backEnd.last2DFBO = glState.currentFBO;

	// set 2D virtual screen size
	qglViewport( 0, 0, w, h );
	qglScissor( 0, 0, w, h );

	Mat4Ortho( 0, w, h, 0, 0, 1, matrix );
	GL_SetProjectionMatrix( matrix );
	Mat4Identity( matrix );
	GL_SetModelviewMatrix( matrix );

	GL_State( GLS_DEPTHTEST_DISABLE |
		GLS_SRCBLEND_SRC_ALPHA |
		GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );

	GL_Cull( CT_TWO_SIDED );
	qglDisable( GL_CLIP_PLANE0 );

	// set time for 2D shaders
	backEnd.refdef.time = ri.Milliseconds();
	backEnd.refdef.floatTime = backEnd.refdef.time * 0.001f;

	// reset color scaling
	backEnd.refdef.colorScale = 1.0f;

	/*qglViewport( x, y, w, h );
	qglScissor( x, y, w, h );

	qglMatrixMode( GL_MODELVIEW0_ARB + 1 );
	qglLoadIdentity();
	qglOrtho( left, right, bottom, top, n, f );

	qglMatrixMode( GL_MODELVIEW0_ARB );
	qglLoadIdentity();
	GL_State( 0x465 );

	qglEnable( GL_BLEND );
	qglDisable( GL_CULL_FACE );
	qglDisable( GL_CLIP_PLANE0 );

	//if( r_reset_tc_array->integer )
	//	qglDisableClientState( 32888 );
	if( !backEnd.projection2D )
	{
		backEnd.refdef.time = ri.Milliseconds();
		backEnd.projection2D = qtrue;
		backEnd.refdef.floatTime = ( float )backEnd.refdef.time * 0.001;
	}*/
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
