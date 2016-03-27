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
// tr_font.c
// 
//
// The font system uses FreeType 2.x to render TrueType fonts for use within the game.
// As of this writing ( Nov, 2000 ) Team Arena uses these fonts for all of the ui and 
// about 90% of the cgame presentation. A few areas of the CGAME were left uses the old 
// fonts since the code is shared with standard Q3A.
//
// If you include this font rendering code in a commercial product you MUST include the
// following somewhere with your product, see www.freetype.org for specifics or changes.
// The Freetype code also uses some hinting techniques that MIGHT infringe on patents 
// held by apple so be aware of that also.
//
// As of Q3A 1.25+ and Team Arena, we are shipping the game with the font rendering code
// disabled. This removes any potential patent issues and it keeps us from having to 
// distribute an actual TrueTrype font which is 1. expensive to do and 2. seems to require
// an act of god to accomplish. 
//
// What we did was pre-render the fonts using FreeType ( which is why we leave the FreeType
// credit in the credits ) and then saved off the glyph data and then hand touched up the 
// font bitmaps so they scale a bit better in GL.
//
// There are limitations in the way fonts are saved and reloaded in that it is based on 
// point size and not name. So if you pre-render Helvetica in 18 point and Impact in 18 point
// you will end up with a single 18 point data file and image set. Typically you will want to 
// choose 3 sizes to best approximate the scaling you will be doing in the ui scripting system
// 
// In the UI Scripting code, a scale of 1.0 is equal to a 48 point font. In Team Arena, we
// use three or four scales, most of them exactly equaling the specific rendered size. We 
// rendered three sizes in Team Arena, 12, 16, and 20. 
//
// To generate new font data you need to go through the following steps.
// 1. delete the fontImage_x_xx.tga files and fontImage_xx.dat files from the fonts path.
// 2. in a ui script, specificy a font, smallFont, and bigFont keyword with font name and 
//    point size. the original TrueType fonts must exist in fonts at this point.
// 3. run the game, you should see things normally.
// 4. Exit the game and there will be three dat files and at least three tga files. The 
//    tga's are in 256x256 pages so if it takes three images to render a 24 point font you 
//    will end up with fontImage_0_24.tga through fontImage_2_24.tga
// 5. In future runs of the game, the system looks for these images and data files when a s
//    specific point sized font is rendered and loads them for use. 
// 6. Because of the original beta nature of the FreeType code you will probably want to hand
//    touch the font bitmaps.
// 
// Currently a define in the project turns on or off the FreeType code which is currently 
// defined out. To pre-render new fonts you need enable the define ( BUILD_FREETYPE ) and 
// uncheck the exclude from build check box in the FreeType2 area of the Renderer project. 


#include "tr_common.h"
#include "../qcommon/qcommon.h"
#include "../renderergl2/tr_local.h"

#ifdef BUILD_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ERRORS_H
#include FT_SYSTEM_H
#include FT_IMAGE_H
#include FT_OUTLINE_H

#define _FLOOR(x)  ((x) & -64)
#define _CEIL(x)   (((x)+63) & -64)
#define _TRUNC(x)  ((x) >> 6)

FT_Library ftLibrary = NULL;  
#endif

#define MAX_FONTS 255
static int registeredFontCount = 0;
static fontInfo_t registeredFont[MAX_FONTS];

static fontheader_t s_loadedFonts[MAX_FONTS];

#ifdef BUILD_FREETYPE
void R_GetGlyphInfo(FT_GlyphSlot glyph, int *left, int *right, int *width, int *top, int *bottom, int *height, int *pitch) {
	*left  = _FLOOR( glyph->metrics.horiBearingX );
	*right = _CEIL( glyph->metrics.horiBearingX + glyph->metrics.width );
	*width = _TRUNC(*right - *left);

	*top    = _CEIL( glyph->metrics.horiBearingY );
	*bottom = _FLOOR( glyph->metrics.horiBearingY - glyph->metrics.height );
	*height = _TRUNC( *top - *bottom );
	*pitch  = ( qtrue ? (*width+3) & -4 : (*width+7) >> 3 );
}


FT_Bitmap *R_RenderGlyph(FT_GlyphSlot glyph, glyphInfo_t* glyphOut) {
	FT_Bitmap  *bit2;
	int left, right, width, top, bottom, height, pitch, size;

	R_GetGlyphInfo(glyph, &left, &right, &width, &top, &bottom, &height, &pitch);

	if ( glyph->format == ft_glyph_format_outline ) {
		size   = pitch*height; 

		bit2 = ri.Malloc(sizeof(FT_Bitmap));

		bit2->width      = width;
		bit2->rows       = height;
		bit2->pitch      = pitch;
		bit2->pixel_mode = ft_pixel_mode_grays;
		//bit2->pixel_mode = ft_pixel_mode_mono;
		bit2->buffer     = ri.Malloc(pitch*height);
		bit2->num_grays = 256;

		Com_Memset( bit2->buffer, 0, size );

		FT_Outline_Translate( &glyph->outline, -left, -bottom );

		FT_Outline_Get_Bitmap( ftLibrary, &glyph->outline, bit2 );

		glyphOut->height = height;
		glyphOut->pitch = pitch;
		glyphOut->top = (glyph->metrics.horiBearingY >> 6) + 1;
		glyphOut->bottom = bottom;

		return bit2;
	} else {
		ri.Printf(PRINT_ALL, "Non-outline fonts are not supported\n");
	}
	return NULL;
}

void WriteTGA (char *filename, byte *data, int width, int height) {
	byte			*buffer;
	int				i, c;
	int             row;
	unsigned char  *flip;
	unsigned char  *src, *dst;

	buffer = ri.Malloc(width*height*4 + 18);
	Com_Memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = width&255;
	buffer[13] = width>>8;
	buffer[14] = height&255;
	buffer[15] = height>>8;
	buffer[16] = 32;	// pixel size

	// swap rgb to bgr
	c = 18 + width * height * 4;
	for (i=18 ; i<c ; i+=4)
	{
		buffer[i] = data[i-18+2];		// blue
		buffer[i+1] = data[i-18+1];		// green
		buffer[i+2] = data[i-18+0];		// red
		buffer[i+3] = data[i-18+3];		// alpha
	}

	// flip upside down
	flip = (unsigned char *)ri.Malloc(width*4);
	for(row = 0; row < height/2; row++)
	{
		src = buffer + 18 + row * 4 * width;
		dst = buffer + 18 + (height - row - 1) * 4 * width;

		Com_Memcpy(flip, src, width*4);
		Com_Memcpy(src, dst, width*4);
		Com_Memcpy(dst, flip, width*4);
	}
	ri.Free(flip);

	ri.FS_WriteFile(filename, buffer, c);

	//f = fopen (filename, "wb");
	//fwrite (buffer, 1, c, f);
	//fclose (f);

	ri.Free (buffer);
}

static glyphInfo_t *RE_ConstructGlyphInfo(unsigned char *imageOut, int *xOut, int *yOut, int *maxHeight, FT_Face face, const unsigned char c, qboolean calcHeight) {
	int i;
	static glyphInfo_t glyph;
	unsigned char *src, *dst;
	float scaled_width, scaled_height;
	FT_Bitmap *bitmap = NULL;

	Com_Memset(&glyph, 0, sizeof(glyphInfo_t));
	// make sure everything is here
	if (face != NULL) {
		FT_Load_Glyph(face, FT_Get_Char_Index( face, c), FT_LOAD_DEFAULT );
		bitmap = R_RenderGlyph(face->glyph, &glyph);
		if (bitmap) {
			glyph.xSkip = (face->glyph->metrics.horiAdvance >> 6) + 1;
		} else {
			return &glyph;
		}

		if (glyph.height > *maxHeight) {
			*maxHeight = glyph.height;
		}

		if (calcHeight) {
			ri.Free(bitmap->buffer);
			ri.Free(bitmap);
			return &glyph;
		}

/*
		// need to convert to power of 2 sizes so we do not get 
		// any scaling from the gl upload
		for (scaled_width = 1 ; scaled_width < glyph.pitch ; scaled_width<<=1)
			;
		for (scaled_height = 1 ; scaled_height < glyph.height ; scaled_height<<=1)
			;
*/

		scaled_width = glyph.pitch;
		scaled_height = glyph.height;

		// we need to make sure we fit
		if (*xOut + scaled_width + 1 >= 255) {
			*xOut = 0;
			*yOut += *maxHeight + 1;
		}

		if (*yOut + *maxHeight + 1 >= 255) {
			*yOut = -1;
			*xOut = -1;
			ri.Free(bitmap->buffer);
			ri.Free(bitmap);
			return &glyph;
		}


		src = bitmap->buffer;
		dst = imageOut + (*yOut * 256) + *xOut;

		if (bitmap->pixel_mode == ft_pixel_mode_mono) {
			for (i = 0; i < glyph.height; i++) {
				int j;
				unsigned char *_src = src;
				unsigned char *_dst = dst;
				unsigned char mask = 0x80;
				unsigned char val = *_src;
				for (j = 0; j < glyph.pitch; j++) {
					if (mask == 0x80) {
						val = *_src++;
					}
					if (val & mask) {
						*_dst = 0xff;
					}
					mask >>= 1;

					if ( mask == 0 ) {
						mask = 0x80;
					}
					_dst++;
				}

				src += glyph.pitch;
				dst += 256;
			}
		} else {
			for (i = 0; i < glyph.height; i++) {
				Com_Memcpy(dst, src, glyph.pitch);
				src += glyph.pitch;
				dst += 256;
			}
		}

		// we now have an 8 bit per pixel grey scale bitmap 
		// that is width wide and pf->ftSize->metrics.y_ppem tall

		glyph.imageHeight = scaled_height;
		glyph.imageWidth = scaled_width;
		glyph.s = (float)*xOut / 256;
		glyph.t = (float)*yOut / 256;
		glyph.s2 = glyph.s + (float)scaled_width / 256;
		glyph.t2 = glyph.t + (float)scaled_height / 256;

		*xOut += scaled_width + 1;

		ri.Free(bitmap->buffer);
		ri.Free(bitmap);
	}

	return &glyph;
}
#endif

static int fdOffset;
static byte	*fdFile;

int readInt( void ) {
	int i = fdFile[fdOffset]+(fdFile[fdOffset+1]<<8)+(fdFile[fdOffset+2]<<16)+(fdFile[fdOffset+3]<<24);
	fdOffset += 4;
	return i;
}

typedef union {
	byte	fred[4];
	float	ffred;
} poor;

float readFloat( void ) {
	poor	me;
#if defined Q3_BIG_ENDIAN
	me.fred[0] = fdFile[fdOffset+3];
	me.fred[1] = fdFile[fdOffset+2];
	me.fred[2] = fdFile[fdOffset+1];
	me.fred[3] = fdFile[fdOffset+0];
#elif defined Q3_LITTLE_ENDIAN
	me.fred[0] = fdFile[fdOffset+0];
	me.fred[1] = fdFile[fdOffset+1];
	me.fred[2] = fdFile[fdOffset+2];
	me.fred[3] = fdFile[fdOffset+3];
#endif
	fdOffset += 4;
	return me.ffred;
}

void RE_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font) {
#ifdef BUILD_FREETYPE
	FT_Face face;
	int j, k, xOut, yOut, lastStart, imageNumber;
	int scaledSize, newSize, maxHeight, left;
	unsigned char *out, *imageBuff;
	glyphInfo_t *glyph;
	image_t *image;
	qhandle_t h;
	float max;
	float dpi = 72;
	float glyphScale;
#endif
	void *faceData;
	int i, len;
	char name[1024];

	if (!fontName) {
		ri.Printf(PRINT_ALL, "RE_RegisterFont: called with empty name\n");
		return;
	}

	if (pointSize <= 0) {
		pointSize = 12;
	}

	R_IssuePendingRenderCommands();

	if (registeredFontCount >= MAX_FONTS) {
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: Too many fonts registered already.\n");
		return;
	}

	// IneQuation: try a RitualFont first
	Com_sprintf( name, sizeof( name ), "fonts/%s.RitualFont", fontName );
	for( i = 0; i < registeredFontCount; i++ ) {
		if( !Q_stricmp( name, registeredFont[ i ].name ) ) {
			Com_Memcpy( font, &registeredFont[ i ], sizeof( fontInfo_t ) );
			return;
		}
	}

	len = ri.FS_ReadFile( name, &faceData );
	if( len > 0 ) {
		char *p = ( char * )faceData;
		const char *token;

		Q_strncpyz( font->name, name, sizeof( font->name ) );

		token = COM_Parse( &p );
		if( !strcmp( token, "RitFont" ) ) {
			int			j;
			float		height, aspect;	// aspect is the aspect ratio of the tga file
			char		indirections[ 256 ];
			shader_t	*fontShader;
			vec4_t		locations;

			Com_sprintf( name, sizeof( name ), "gfx/fonts/%s", fontName );
			fontShader = R_FindShader( name, LIGHTMAP_2D, qfalse );
			Com_Memset( font->glyphs, 0, sizeof( font->glyphs ) );
			font->glyphScale = 1.f;

			token = COM_Parse( &p );
			while( token && token[ 0 ] ) {
				if( !Q_stricmp( token, "height" ) ) {
					token = COM_Parse( &p );
					height = atof( token );
				}
				else if( !Q_stricmp( token, "aspect" ) ) {
					token = COM_Parse( &p );
					aspect = atof( token );
				}
				else if( !Q_stricmp( token, "indirections" ) ) {
					// skip the opening brace
					COM_Parse( &p );
					for( i = 0; i < 256; i++ ) {
						token = COM_Parse( &p );
						indirections[ i ] = atoi( token );
					}
					// skip the trailing brace
					COM_Parse( &p );
				}
				else if( !Q_stricmp( token, "locations" ) ) {
					// skip the opening brace
					COM_Parse( &p );
					for( i = 0; i < 256; i++ ) {
						for( j = 0; j < 256; j++ ) {
							if( indirections[ j ] == i )
								break;
						}
						// we'll need the value of j in a while, and parsing all the locations is required anyway in order to exit cleanly
						// skip the opening brace
						COM_Parse( &p );
						token = COM_Parse( &p );
						locations[ 0 ] = atoi( token );
						token = COM_Parse( &p );
						locations[ 1 ] = atoi( token );
						token = COM_Parse( &p );
						locations[ 2 ] = atoi( token );
						token = COM_Parse( &p );
						locations[ 3 ] = atoi( token );
						// skip the trailing brace
						COM_Parse( &p );
						if( j == 256 )
							continue;	// character not supported by font, skip it
						//font->glyphs[i].top = locations[1];
						//font->glyphs[i].bottom = font->glyphs[i].top + locations[3];
						font->glyphs[ j ].s = ( float )locations[ 0 ] / fontShader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->width;
						font->glyphs[ j ].t = ( float )locations[ 1 ] / fontShader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->height;
						font->glyphs[ j ].s2 = ( float )( locations[ 0 ] + locations[ 2 ] ) / fontShader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->width;
						font->glyphs[ j ].t2 = ( float )( locations[ 1 ] + locations[ 3 ] ) / fontShader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->height;
						font->glyphs[ j ].imageWidth = font->glyphs[ j ].xSkip = font->glyphs[ j ].pitch = locations[ 2 ];
						font->glyphs[ j ].imageHeight = font->glyphs[ j ].height = locations[ 3 ];
						font->glyphs[ j ].glyph = fontShader->index;
						Q_strncpyz( font->glyphs[ j ].shaderName, name, sizeof( font->glyphs[ 0 ].shaderName ) );
					}
					// skip the trailing brace
					COM_Parse( &p );
				}
				else {
					ri.Printf( PRINT_WARNING, "RE_RegisterFont: unknown token %s.\n", token );
					return;
				}
				token = COM_Parse( &p );
			}
			if( registeredFontCount < MAX_FONTS - 1 )
				Com_Memcpy( &registeredFont[ registeredFontCount++ ], font, sizeof( fontInfo_t ) );
			else
				ri.Printf( PRINT_WARNING, "RE_RegisterFont: MAX_FONTS (%d) exceeded.\n", MAX_FONTS );
			return;	// successfully loaded a RitualFont
		}
		else
			ri.Printf( PRINT_WARNING, "RE_RegisterFont: %s is not a valid RitualFont (invalid ident %s).\n", name, token );
	}
	// if this fails, try loading the FreeType-rendered fonts

	Com_sprintf(name, sizeof(name), "fonts/fontImage_%i.dat",pointSize);
	for (i = 0; i < registeredFontCount; i++) {
		if (Q_stricmp(name, registeredFont[i].name) == 0) {
			Com_Memcpy(font, &registeredFont[i], sizeof(fontInfo_t));
			return;
		}
	}

	len = ri.FS_ReadFile(name, NULL);
	if (len == sizeof(fontInfo_t)) {
		ri.FS_ReadFile(name, &faceData);
		fdOffset = 0;
		fdFile = faceData;
		for(i=0; i<GLYPHS_PER_FONT; i++) {
			font->glyphs[i].height		= readInt();
			font->glyphs[i].top			= readInt();
			font->glyphs[i].bottom		= readInt();
			font->glyphs[i].pitch		= readInt();
			font->glyphs[i].xSkip		= readInt();
			font->glyphs[i].imageWidth	= readInt();
			font->glyphs[i].imageHeight = readInt();
			font->glyphs[i].s			= readFloat();
			font->glyphs[i].t			= readFloat();
			font->glyphs[i].s2			= readFloat();
			font->glyphs[i].t2			= readFloat();
			font->glyphs[i].glyph		= readInt();
			Q_strncpyz(font->glyphs[i].shaderName, (const char *)&fdFile[fdOffset], sizeof(font->glyphs[i].shaderName));
			fdOffset += sizeof(font->glyphs[i].shaderName);
		}
		font->glyphScale = readFloat();
		Com_Memcpy(font->name, &fdFile[fdOffset], MAX_QPATH);

//		Com_Memcpy(font, faceData, sizeof(fontInfo_t));
		Q_strncpyz(font->name, name, sizeof(font->name));
		for (i = GLYPH_START; i <= GLYPH_END; i++) {
			font->glyphs[i].glyph = RE_RegisterShaderNoMip(font->glyphs[i].shaderName);
		}
		Com_Memcpy(&registeredFont[registeredFontCount++], font, sizeof(fontInfo_t));
		ri.FS_FreeFile(faceData);
		return;
	}

#ifndef BUILD_FREETYPE
	ri.Printf(PRINT_WARNING, "RE_RegisterFont: FreeType code not available\n");
#else
	if (ftLibrary == NULL) {
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: FreeType not initialized.\n");
		return;
	}

	len = ri.FS_ReadFile(fontName, &faceData);
	if (len <= 0) {
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: Unable to read font file '%s'\n", fontName);
		return;
	}

	// allocate on the stack first in case we fail
	if (FT_New_Memory_Face( ftLibrary, faceData, len, 0, &face )) {
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: FreeType, unable to allocate new face.\n");
		return;
	}


	if (FT_Set_Char_Size( face, pointSize << 6, pointSize << 6, dpi, dpi)) {
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: FreeType, unable to set face char size.\n");
		return;
	}

	//*font = &registeredFonts[registeredFontCount++];

	// make a 256x256 image buffer, once it is full, register it, clean it and keep going 
	// until all glyphs are rendered

	out = ri.Malloc(256*256);
	if (out == NULL) {
		ri.Printf(PRINT_WARNING, "RE_RegisterFont: ri.Malloc failure during output image creation.\n");
		return;
	}
	Com_Memset(out, 0, 256*256);

	maxHeight = 0;

	for (i = GLYPH_START; i <= GLYPH_END; i++) {
		RE_ConstructGlyphInfo(out, &xOut, &yOut, &maxHeight, face, (unsigned char)i, qtrue);
	}

	xOut = 0;
	yOut = 0;
	i = GLYPH_START;
	lastStart = i;
	imageNumber = 0;

	while ( i <= GLYPH_END + 1 ) {

		if ( i == GLYPH_END + 1 ) {
			// upload/save current image buffer
			xOut = yOut = -1;
		} else {
			glyph = RE_ConstructGlyphInfo(out, &xOut, &yOut, &maxHeight, face, (unsigned char)i, qfalse);
		}

		if (xOut == -1 || yOut == -1)  {
			// ran out of room
			// we need to create an image from the bitmap, set all the handles in the glyphs to this point
			// 

			scaledSize = 256*256;
			newSize = scaledSize * 4;
			imageBuff = ri.Malloc(newSize);
			left = 0;
			max = 0;
			for ( k = 0; k < (scaledSize) ; k++ ) {
				if (max < out[k]) {
					max = out[k];
				}
			}

			if (max > 0) {
				max = 255/max;
			}

			for ( k = 0; k < (scaledSize) ; k++ ) {
				imageBuff[left++] = 255;
				imageBuff[left++] = 255;
				imageBuff[left++] = 255;

				imageBuff[left++] = ((float)out[k] * max);
			}

			Com_sprintf (name, sizeof(name), "fonts/fontImage_%i_%i.tga", imageNumber++, pointSize);
			if (r_saveFontData->integer) { 
				WriteTGA(name, imageBuff, 256, 256);
			}

			//Com_sprintf (name, sizeof(name), "fonts/fontImage_%i_%i", imageNumber++, pointSize);
			image = R_CreateImage(name, imageBuff, 256, 256, IMGTYPE_COLORALPHA, IMGFLAG_CLAMPTOEDGE, 0 );
			h = RE_RegisterShaderFromImage(name, LIGHTMAP_2D, image, qfalse);
			for (j = lastStart; j < i; j++) {
				font->glyphs[j].glyph = h;
				Q_strncpyz(font->glyphs[j].shaderName, name, sizeof(font->glyphs[j].shaderName));
			}
			lastStart = i;
			Com_Memset(out, 0, 256*256);
			xOut = 0;
			yOut = 0;
			ri.Free(imageBuff);
			if ( i == GLYPH_END + 1 )
				i++;
		} else {
			Com_Memcpy(&font->glyphs[i], glyph, sizeof(glyphInfo_t));
			i++;
		}
	}

	// change the scale to be relative to 1 based on 72 dpi ( so dpi of 144 means a scale of .5 )
	glyphScale = 72.0f / dpi;

	// we also need to adjust the scale based on point size relative to 48 points as the ui scaling is based on a 48 point font
	glyphScale *= 48.0f / pointSize;

	registeredFont[registeredFontCount].glyphScale = glyphScale;
	font->glyphScale = glyphScale;
	Com_Memcpy(&registeredFont[registeredFontCount++], font, sizeof(fontInfo_t));

	if (r_saveFontData->integer) {
		ri.FS_WriteFile(va("fonts/fontImage_%i.dat", pointSize), font, sizeof(fontInfo_t));
	}

	ri.Free(out);

	ri.FS_FreeFile(faceData);
#endif
}



void R_InitFreeType(void) {
#ifdef BUILD_FREETYPE
	if (FT_Init_FreeType( &ftLibrary )) {
		ri.Printf(PRINT_WARNING, "R_InitFreeType: Unable to initialize FreeType.\n");
	}
#endif
	registeredFontCount = 0;
}


void R_DoneFreeType(void) {
#ifdef BUILD_FREETYPE
	if (ftLibrary) {
		FT_Done_FreeType( ftLibrary );
		ftLibrary = NULL;
	}
#endif
	registeredFontCount = 0;
}

static int s_numLoadedFonts = 0;
static float s_fontHeightScale = 1.0;
static float s_fontGeneralScale = 1.0;
static float s_fontZ = 0.0;

void R_SetFontHeightScale( float scale ) {
	s_fontHeightScale = scale;
}

void R_SetFontScale( float scale ) {
	s_fontGeneralScale = scale;
}

void R_SetFontZ( float zed ) {
	s_fontZ = zed;
}

fontheader_t *R_LoadFont( const char *name ) {
	int i;
	const char *theFile;
	fontheader_t *header;
	char *ref;
	const char *token;
	qboolean error = qfalse;
	char *p;

	for( i = 0; i < s_numLoadedFonts; i++ )
	{
		if( !Q_stricmp( name, s_loadedFonts[ i ].name ) ) {
			return &s_loadedFonts[ i ];
		}
	}

	if( s_numLoadedFonts >= MAX_FONTS ) {
		ri.Printf( PRINT_WARNING, "LoadFont: Too many fonts loaded!  Couldn't load %s\n", name );
		return NULL;
	}

	theFile = va( "fonts/%s.RitualFont", name );

	if( ri.FS_ReadFile( theFile, ( void ** )&ref ) == -1 )
	{
		ri.Printf( PRINT_WARNING, "LoadFont: Couldn't load font %s\n", name );
		return NULL;
	}

	header = &s_loadedFonts[ s_numLoadedFonts ];
	header->height = 0.0;
	header->aspectRatio = 0.0;
	strcpy( header->name, name );

	p = ref;
	token = COM_Parse( &p );
	if( !strcmp( token, "RitFont" ) ) {
		token = COM_Parse( &p );
		while( token && token[ 0 ] ) {
			if( !Q_stricmp( token, "height" ) ) {
				token = COM_Parse( &p );
				header->height = atof( token );
			}
			else if( !Q_stricmp( token, "aspect" ) ) {
				token = COM_Parse( &p );
				header->aspectRatio = atof( token );
			}
			else if( !Q_stricmp( token, "indirections" ) ) {
				// check the opening brace
				token = COM_Parse( &p );
				if( Q_stricmp( token, "{" ) )
				{
					error = qtrue;
					break;
				}
				for( i = 0; i < 256; i++ ) {
					token = COM_Parse( &p );
					if( !*token ) {
						error = qtrue;
						break;
					}
					header->indirection[ i ] = atoi( token );
				}
				if( error ) {
					break;
				}
				// check the trailing brace
				token = COM_Parse( &p );
				if( Q_stricmp( token, "}" ) )
				{
					error = qtrue;
					break;
				}
			}
			else if( !Q_stricmp( token, "locations" ) ) {
				// check the opening brace
				token = COM_Parse( &p );
				if( Q_stricmp( token, "{" ) )
				{
					error = qtrue;
					break;
				}
				for( i = 0; i < 256; i++ ) {
					/*for( j = 0; j < 256; j++ ) {
						if( indirections[ j ] == i )
							break;
					}*/
					// we'll need the value of j in a while, and parsing all the locations is required anyway in order to exit cleanly
					// check the opening brace
					token = COM_Parse( &p );
					if( Q_stricmp( token, "{" ) )
					{
						error = qtrue;
						break;
					}

					token = COM_Parse( &p );
					header->locations[ i ].pos[ 0 ] = ( float )atoi( token ) * 0.00390625;
					token = COM_Parse( &p );
					header->locations[ i ].pos[ 1 ] = ( float )atoi( token ) * header->aspectRatio * 0.00390625;
					token = COM_Parse( &p );
					header->locations[ i ].pos[ 2 ] = ( float )atoi( token ) * 0.00390625;
					token = COM_Parse( &p );
					header->locations[ i ].pos[ 3 ] = ( float )atoi( token ) * header->aspectRatio * 0.00390625;

					// check the trailing brace
					token = COM_Parse( &p );
					if( Q_stricmp( token, "}" ) )
					{
						error = qtrue;
						break;
					}
				}
				if( error ) {
					break;
				}
				// check the trailing brace
				token = COM_Parse( &p );
				if( Q_stricmp( token, "}" ) )
				{
					error = qtrue;
					break;
				}
			} else {
				ri.Printf( PRINT_WARNING, "WARNING: Unknown token '%s' parsing font '%s'.\n", token, name );
				error = qtrue;
				break;
			}
			token = COM_Parse( &p );
		}
	} else {
		ri.Printf( PRINT_WARNING, "WARNING: %s is not a valid RitualFont (invalid ident '%s').\n", name, token );
	}

	R_LoadFontShader( header );

	if( header->height == 0.0 || header->aspectRatio == 0.0 ) {
		error = qtrue;
	}

	ri.FS_FreeFile( ref );

	if( error )
	{
		ri.Printf( PRINT_WARNING, "WARNING: Error parsing font %s.\n", name );
		return NULL;
	}

	s_numLoadedFonts++;
	return header;
}

void R_LoadFontShader( fontheader_t *font ) {
	int i;
	int save;
	shader_t *fontshader;
	char filename[ 64 ];

	save = r_sequencenumber;
	r_sequencenumber = -1;

	Com_sprintf( filename, sizeof( filename ), "gfx/fonts/%s", font->name );
	fontshader = R_FindShader( filename, -1, qfalse );
	font->shader = fontshader;

	r_sequencenumber = save;

	if( !fontshader ) {
		ri.Error( ERR_DROP, "Could not load font shader for %s\n", filename );
	}

	// set the font to use the global color/alpha
	for( i = 0; i < fontshader->numUnfoggedPasses; i++ )
	{
		if( fontshader->stages[ i ] && fontshader->stages[ i ]->active )
		{
			fontshader->stages[ i ]->rgbGen = CGEN_GLOBAL_COLOR;
			fontshader->stages[ i ]->alphaGen = AGEN_GLOBAL_ALPHA;
		}
	}

	font->trhandle = r_sequencenumber;
}

void R_DrawString( fontheader_t *font, const char *text, float x, float y, int maxlen, qboolean bVirtualScreen ) {
	shader_t *fontshader;
	float charHeight;
	float startx, starty;
	int i;
	float fWidthScale, fHeightScale;

	if( !font ) {
		return;
	}

	startx = x;
	starty = y;
	fWidthScale = glConfig.vidWidth / 640.0;
	fHeightScale = glConfig.vidHeight / 480.0;

	R_IssuePendingRenderCommands();

	if( font->trhandle != r_sequencenumber ) {
		font->shader = NULL;
	}

	fontshader = font->shader;
	if( !fontshader ) {
		R_LoadFontShader( font );
	}

	charHeight = s_fontGeneralScale * s_fontHeightScale * font->height;

	RB_Prepare2D();

	RB_BeginSurface( fontshader, 0, 0 );

	for( i = 0; text[ i ] && ( maxlen == -1 || i < maxlen ); i++ )
	{
		unsigned char c = text[ i ];
		int indirected;
		letterloc_t *loc;

		if( c == '\n' )
		{
			starty += charHeight;
		}
		else if( c == '\r' )
		{
			x = startx;
		}
		else if( c == '\t' )
		{
			indirected = font->indirection[ ' ' ];
			if( indirected == -1 )
			{
				Com_DPrintf( "R_DrawString: no space-character in font!\n" );
				continue;
			}

			x += font->locations[ indirected ].size[ 0 ] * 256.0 * s_fontGeneralScale * 3.0;
		}
		else
		{
			indirected = font->indirection[ c ];

			if( indirected == -1 )
			{
				Com_DPrintf( "R_DrawString: no 0x%02x-character in font!\n", c );
				indirected = font->indirection[ '?' ];
				if( indirected == -1 )
				{
					Com_DPrintf( "R_DrawString: no '?' character in font!\n" );
					continue;
				}

				font->indirection[ c ] = indirected;
			}

			// draw the character
			if( tess.numVertexes + 4 >= 1000 || tess.numIndexes + 6 >= 6000 ) {
				RB_CheckOverflow( 4, 6 );
			}

			loc = &font->locations[ indirected ];

			tess.texCoords[ tess.numVertexes + 0 ][ 0 ][ 0 ] = loc->pos[ 0 ];
			tess.texCoords[ tess.numVertexes + 0 ][ 0 ][ 1 ] = loc->pos[ 1 ];
			tess.texCoords[ tess.numVertexes + 1 ][ 0 ][ 0 ] = loc->pos[ 0 ] + loc->size[ 0 ];
			tess.texCoords[ tess.numVertexes + 1 ][ 0 ][ 1 ] = loc->pos[ 1 ];
			tess.texCoords[ tess.numVertexes + 2 ][ 0 ][ 0 ] = loc->pos[ 0 ];
			tess.texCoords[ tess.numVertexes + 2 ][ 0 ][ 1 ] = loc->pos[ 1 ] + loc->size[ 1 ];
			tess.texCoords[ tess.numVertexes + 3 ][ 0 ][ 0 ] = loc->pos[ 0 ] + loc->size[ 0 ];
			tess.texCoords[ tess.numVertexes + 3 ][ 0 ][ 1 ] = loc->pos[ 1 ] + loc->size[ 1 ];

			tess.xyz[ tess.numVertexes + 0 ][ 0 ] = x;
			tess.xyz[ tess.numVertexes + 0 ][ 1 ] = y;
			tess.xyz[ tess.numVertexes + 0 ][ 2 ] = s_fontZ;
			tess.xyz[ tess.numVertexes + 1 ][ 0 ] = x + loc->size[ 0 ] * 256.0 * s_fontGeneralScale;
			tess.xyz[ tess.numVertexes + 1 ][ 1 ] = y;
			tess.xyz[ tess.numVertexes + 1 ][ 2 ] = s_fontZ;
			tess.xyz[ tess.numVertexes + 2 ][ 0 ] = x;
			tess.xyz[ tess.numVertexes + 2 ][ 1 ] = y + charHeight;
			tess.xyz[ tess.numVertexes + 2 ][ 2 ] = s_fontZ;
			tess.xyz[ tess.numVertexes + 3 ][ 0 ] = x + loc->size[ 0 ] * 256.0 * s_fontGeneralScale;
			tess.xyz[ tess.numVertexes + 3 ][ 1 ] = y + charHeight;
			tess.xyz[ tess.numVertexes + 3 ][ 2 ] = s_fontZ;

			tess.indexes[ tess.numIndexes + 0 ] = tess.numVertexes + 0;
			tess.indexes[ tess.numIndexes + 1 ] = tess.numVertexes + 1;
			tess.indexes[ tess.numIndexes + 2 ] = tess.numVertexes + 2;
			tess.indexes[ tess.numIndexes + 3 ] = tess.numVertexes + 1;
			tess.indexes[ tess.numIndexes + 4 ] = tess.numVertexes + 3;
			tess.indexes[ tess.numIndexes + 5 ] = tess.numVertexes + 2;

			if( bVirtualScreen )
			{
				tess.xyz[ tess.numVertexes + 0 ][ 0 ] *= fWidthScale;
				tess.xyz[ tess.numVertexes + 0 ][ 1 ] *= fHeightScale;
				tess.xyz[ tess.numVertexes + 1 ][ 0 ] *= fWidthScale;
				tess.xyz[ tess.numVertexes + 1 ][ 1 ] *= fHeightScale;
				tess.xyz[ tess.numVertexes + 2 ][ 0 ] *= fWidthScale;
				tess.xyz[ tess.numVertexes + 2 ][ 1 ] *= fHeightScale;
				tess.xyz[ tess.numVertexes + 3 ][ 0 ] *= fWidthScale;
				tess.xyz[ tess.numVertexes + 3 ][ 1 ] *= fHeightScale;
			}

			tess.numVertexes += 4;
			tess.numIndexes += 6;

			x += loc->size[ 0 ] * 256.0 * s_fontGeneralScale;
		}
	}

	RB_EndSurface();
}

void R_DrawFloatingString( fontheader_t *font, const char *text, const vec3_t org, float *color, float scale, int maxlen ) {
	shader_t *fontshader;
	qhandle_t fsh;
	float charWidth;
	float charHeight;
	int i;
	vec3_t pos;
	polyVert_t verts[ 4 ];

	if( !font ) {
		return;
	}

	R_IssuePendingRenderCommands();

	if( font->trhandle != r_sequencenumber ) {
		font->shader = NULL;
	}

	if( !font->shader ) {
		R_LoadFontShader( font );
	}
	fontshader = font->shader;

	fsh = 0;
	for( i = 0; i < tr.numShaders; i++ )
	{
		if( fontshader == tr.shaders[ i ] ) {
			fsh = i;
		}
	}

	charHeight = font->height * s_fontHeightScale * s_fontGeneralScale * scale;
	VectorCopy( org, pos );

	for( i = 0; text[ i ] && ( maxlen == -1 || i < maxlen ); i++ )
	{
		unsigned char c = text[ i ];
		int indirected;
		letterloc_t *loc;

		indirected = font->indirection[ c ];
		if( indirected == -1 )
		{
			Com_Printf( "R_DrawFloatingString: no 0x%02x-character in font!\n", c );
			continue;
		}

		loc = &font->locations[ indirected ];
		charWidth = loc->size[ 0 ] * 256.0 * s_fontGeneralScale * scale;

		verts[ 0 ].modulate[ 0 ] = color[ 0 ] * 255.0;
		verts[ 0 ].modulate[ 1 ] = color[ 1 ] * 255.0;
		verts[ 0 ].modulate[ 2 ] = color[ 2 ] * 255.0;
		verts[ 1 ].modulate[ 0 ] = verts[ 0 ].modulate[ 0 ];
		verts[ 1 ].modulate[ 1 ] = verts[ 0 ].modulate[ 1 ];
		verts[ 1 ].modulate[ 2 ] = verts[ 0 ].modulate[ 2 ];
		verts[ 2 ].modulate[ 0 ] = verts[ 0 ].modulate[ 0 ];
		verts[ 2 ].modulate[ 1 ] = verts[ 0 ].modulate[ 1 ];
		verts[ 2 ].modulate[ 2 ] = verts[ 0 ].modulate[ 2 ];
		verts[ 3 ].modulate[ 0 ] = verts[ 0 ].modulate[ 0 ];
		verts[ 3 ].modulate[ 1 ] = verts[ 0 ].modulate[ 1 ];
		verts[ 3 ].modulate[ 2 ] = verts[ 0 ].modulate[ 2 ];

		verts[ 0 ].st[ 0 ] = loc->pos[ 0 ];
		verts[ 0 ].st[ 1 ] = loc->pos[ 1 ];
		verts[ 1 ].st[ 0 ] = loc->pos[ 0 ] + loc->size[ 0 ];
		verts[ 1 ].st[ 1 ] = loc->pos[ 1 ];
		verts[ 2 ].st[ 0 ] = loc->pos[ 0 ] + loc->size[ 0 ];
		verts[ 2 ].st[ 1 ] = loc->pos[ 1 ] + loc->size[ 1 ];
		verts[ 3 ].st[ 0 ] = loc->pos[ 0 ];
		verts[ 3 ].st[ 1 ] = loc->pos[ 1 ] + loc->size[ 1 ];

		verts[ 0 ].xyz[ 0 ] = pos[ 0 ] + tr.refdef.viewaxis[ 1 ][ 0 ] * -charWidth + tr.refdef.viewaxis[ 1 ][ 0 ] * charWidth + tr.refdef.viewaxis[ 2 ][ 0 ] * charHeight;
		verts[ 0 ].xyz[ 1 ] = pos[ 1 ] + tr.refdef.viewaxis[ 1 ][ 1 ] * -charWidth + tr.refdef.viewaxis[ 1 ][ 1 ] * charWidth + tr.refdef.viewaxis[ 2 ][ 1 ] * charHeight;
		verts[ 0 ].xyz[ 2 ] = pos[ 2 ] + tr.refdef.viewaxis[ 1 ][ 2 ] * -charWidth + tr.refdef.viewaxis[ 1 ][ 2 ] * charWidth + tr.refdef.viewaxis[ 2 ][ 2 ] * charHeight;
		verts[ 1 ].xyz[ 0 ] = pos[ 0 ] + tr.refdef.viewaxis[ 1 ][ 0 ] * -charWidth + tr.refdef.viewaxis[ 2 ][ 0 ] * charHeight;
		verts[ 1 ].xyz[ 1 ] = pos[ 1 ] + tr.refdef.viewaxis[ 1 ][ 1 ] * -charWidth + tr.refdef.viewaxis[ 2 ][ 1 ] * charHeight;
		verts[ 1 ].xyz[ 2 ] = pos[ 2 ] + tr.refdef.viewaxis[ 1 ][ 2 ] * -charWidth + tr.refdef.viewaxis[ 2 ][ 2 ] * charHeight;
		verts[ 2 ].xyz[ 0 ] = pos[ 0 ] + tr.refdef.viewaxis[ 1 ][ 0 ] * -charWidth;
		verts[ 2 ].xyz[ 1 ] = pos[ 1 ] + tr.refdef.viewaxis[ 1 ][ 1 ] * -charWidth;
		verts[ 2 ].xyz[ 2 ] = pos[ 2 ] + tr.refdef.viewaxis[ 1 ][ 2 ] * -charWidth;
		verts[ 3 ].xyz[ 0 ] = pos[ 0 ];
		verts[ 3 ].xyz[ 1 ] = pos[ 1 ];
		verts[ 3 ].xyz[ 2 ] = pos[ 2 ];

		if( RE_AddPolyToScene( fsh, 4, verts, 0 ) ) {
			tr.refdef.numPolys++;
		}

		VectorCopy( verts[ 2 ].xyz, pos );
	}
}

float R_GetFontHeight( fontheader_t *font ) {
	if( font ) {
		return s_fontGeneralScale * s_fontHeightScale * font->height;
	} else {
		return 0.0;
	}
}

float R_GetFontStringWidth( fontheader_t *font, const char *s ) {
	float widths = 0.0;
	int i;

	if( !font ) {
		return 0.0;
	}

	for( i = 0; s[ i ]; i++ )
	{
		unsigned char c = s[ i ];
		int indirected;

		if( c == '\t' )
		{
			indirected = font->indirection[ ' ' ];

			if( indirected == -1 ) {
				Com_Printf( "R_GetFontStringWidth: no space-character in font!\n" );
			} else {
				widths += font->locations[ indirected ].size[ 0 ] * 3.0;
			}
		}
		else
		{
			indirected = font->indirection[ c ];

			if( indirected == -1 ) {
				Com_Printf( "R_GetFontStringWidth: no 0x%02x-character in font!\n", c );
			} else {
				widths += font->locations[ indirected ].size[ 0 ];
			}
		}
	}

	widths *= s_fontGeneralScale * 256.0;
	return widths;
}

// IneQuation: exports to deal with universal text drawing; adapted from cgame

/*
===================
RE_Text_Width
===================
Returns text width in pixels.
*/
int RE_Text_Width( fontInfo_t *font, const char *text, int limit, qboolean useColourCodes ) {
	int count, len;
	float out;
	glyphInfo_t *glyph;
	// FIXME: see ui_main.c, same problem
	//	const unsigned char *s = text;
	const char *s = text;

	if( !font ) {
		font = &registeredFont[ 0 ];
		if( !font ) {
			ri.Printf( PRINT_WARNING, "RE_Text_Width: no fonts registered!\n" );
			return strlen( text ) * 16;	// return something
		}
	}

	out = 0;
	if( text ) {
		len = strlen( text );
		if( limit > 0 && len > limit )
			len = limit;
		count = 0;
		while( s && *s && count < len ) {
			if( useColourCodes && Q_IsColorString( s ) ) {
				s += 2;
				continue;
			}
			else {
				glyph = &font->glyphs[ ( int )*s ];
				out += glyph->xSkip;
				s++;
				count++;
			}
		}
	}
	return out * font->glyphScale;
}

/*
===================
RE_Text_Height
===================
Returns text height in pixels.
*/
int RE_Text_Height( fontInfo_t *font, const char *text, int limit, qboolean useColourCodes ) {
	int len, count;
	float max;
	glyphInfo_t *glyph;
	// TTimo: FIXME
	//	const unsigned char *s = text;
	const char *s = text;

	if( !font ) {
		font = &registeredFont[ 0 ];
		if( !font ) {
			ri.Printf( PRINT_WARNING, "RE_Text_Height: no fonts registered!\n" );
			return 16;	// return something
		}
	}

	max = 0;
	if( text ) {
		len = strlen( text );
		if( limit > 0 && len > limit )
			len = limit;
		count = 0;
		while( s && *s && count < len ) {
			if( useColourCodes && Q_IsColorString( s ) ) {
				s += 2;
				continue;
			}
			else {
				glyph = &font->glyphs[ ( int )*s ]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
				if( max < glyph->height )
					max = glyph->height;
				s++;
				count++;
			}
		}
	}
	return max * font->glyphScale;
}

/*
===================
RE_Text_PaintChar
===================
Paints a single character.
*/
void RE_Text_PaintChar( fontInfo_t *font, float x, float y, float scale, int c, qboolean is640 ) {
	glyphInfo_t *glyph;
	float xscale;
	float yscale;

	if( is640 ) {
		xscale = glConfig.vidWidth / 640.0f;
		yscale = glConfig.vidHeight / 480.0f;
	}
	else {
		xscale = 1.0f;
		yscale = 1.0f;
	}

	glyph = &font->glyphs[ c ];
	if( c != '\n' && glyph->imageWidth == 0 && glyph->imageHeight == 0 ) {
		glyph = &font->glyphs[ '?' ];
		// this is so annoying..
		//ri.Printf(PRINT_WARNING, "RE_Text_PaintChar: no #%d character in font %s!\n", (int)c, font->name);
	}
	RE_StretchPic( x * xscale, y*yscale - font->glyphScale * glyph->top,
		glyph->imageWidth * font->glyphScale * scale * xscale,
		glyph->imageHeight * font->glyphScale * scale * yscale,
		glyph->s,
		glyph->t,
		glyph->s2,
		glyph->t2,
		glyph->glyph );
}

/*
===================
RE_Text_Paint
===================
Paints a string. The alpha value will be ignored unless useColourCodes is qtrue.
*/
void RE_Text_Paint( fontInfo_t *font, float x, float y, float scale, float alpha, const char *text, float adjust, int limit, qboolean useColourCodes, qboolean is640 ) {
	int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph;
	float xscale;
	float yscale;

	if( !font ) {
		if( !registeredFontCount )
			return;
		font = &registeredFont[ 0 ];
	}
	if( is640 ) {
		xscale = glConfig.vidWidth / 640.0f;
		yscale = glConfig.vidHeight / 480.0f;
	}
	else {
		xscale = 1.0f;
		yscale = 1.0f;
	}

	if( text ) {
		// TTimo: FIXME
		//		const unsigned char *s = text;
		const char *s = text;
		len = strlen( text );
		if( limit > 0 && len > limit )
			len = limit;
		count = 0;
		while( s && *s && count < len ) {
			glyph = &font->glyphs[ ( int )*s ]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
			if( *s != '\n' && glyph->imageWidth == 0 && glyph->imageHeight == 0 ) {
				glyph = &font->glyphs[ '?' ];
				//ri.Printf(PRINT_WARNING, "RE_Text_Paint: no #%d character in font %s!\n", (int)*s, font->name);
			}
			if( useColourCodes && Q_IsColorString( s ) ) {
				memcpy( newColor, g_color_table[ ColorIndex( *( s + 1 ) ) ], sizeof( newColor ) );
				newColor[ 3 ] = alpha;
				RE_SetColor( newColor );
				s += 2;
				continue;
			}
			else {
				float yadj = font->glyphScale * scale * glyph->top;
				/*if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE) {
				int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;
				colorBlack[3] = newColor[3];
				RE_SetColor(colorBlack);
				RE_StretchPic(x + ofs, y - yadj + ofs,
				glyph->imageWidth * font->glyphScale,
				glyph->imageHeight * font->glyphScale,
				glyph->s,
				glyph->t,
				glyph->s2,
				glyph->t2,
				glyph->glyph);
				colorBlack[3] = 1.0;
				RE_SetColor( newColor );
				}*/
				RE_StretchPic( x*xscale, ( y - yadj )*yscale,
					glyph->imageWidth * font->glyphScale * scale*xscale,
					glyph->imageHeight * font->glyphScale * scale*yscale,
					glyph->s,
					glyph->t,
					glyph->s2,
					glyph->t2,
					glyph->glyph );
				x += ( glyph->xSkip * font->glyphScale * scale ) + adjust;
				s++;
				count++;
			}
		}
		if( useColourCodes ) {
			RE_SetColor( NULL );
		}
	}
}
