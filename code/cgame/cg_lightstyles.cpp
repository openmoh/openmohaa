/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

This file is part of OpenMoHAA source code.

OpenMoHAA source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMoHAA source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMoHAA source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// DESCRIPTION:
// light style code

#include "cg_local.h"

/*
===================================================================

LIGHT STYLE CODE

===================================================================
*/

#define MAX_LIGHTSTYLE_LENGTH 128
#define MAX_LIGHTSTYLE_NAME_LENGTH 64

typedef struct
	{
   char     name[ MAX_LIGHTSTYLE_NAME_LENGTH ];
   qboolean hasalpha;
	int		length;
	float		map[ MAX_LIGHTSTYLE_LENGTH ][ 4 ];
	} clightstyle_t;

clightstyle_t	cg_lightstyle[ MAX_LIGHTSTYLES * 2 ];

/*
================
CG_LightStyleColor
================
*/
qboolean CG_LightStyleColor( int style, int realtime, float color[4], qboolean clamp )
   {
	clightstyle_t	*ls;
   int            i, time;
   float          frac;
   qboolean       at_end;

   time = realtime / 50;
   frac = ( realtime - ( time * 50.0f ) ) / 50.0f;
   memset( color, 0, sizeof( color ) );
   if ( ( style < 0 ) || ( style >= ( MAX_LIGHTSTYLES * 2 ) ) )
      {
		cgi.DPrintf ("CG_LightStyleColor: style out of range.\n");
      return qtrue;
      }

   ls = &cg_lightstyle[ style ];

   if ( !ls->length )
      {
		cgi.DPrintf ("CG_LightStyleColor: style %d has zero length.\n", style);
      return qtrue;
      }

   // by default we are not at the end
   at_end = qfalse;
   if ( clamp )
      {
      if ( time >= ls->length )
         {
         time = ls->length - 1;
         at_end = qtrue;
         }
      }
   else
      {
      time %= ls->length;
      }

   //
   // only lerp if we are before the end
   //
   if ( time == ls->length - 1 )
      {
      memcpy( color, ls->map[ time ], 4 * sizeof( ls->map[ time ][ 0 ] ) );
      }
   else
      {
      for( i = 0; i < 4; i++ )
         {
		   color[ i ] = ls->map[ time ][ i ] + frac * ( ls->map[ time + 1 ][ i ] - ls->map[ time ][ i ] );
         }
      }

   if ( !ls->hasalpha ) 
      color[ 3 ] = 1;

   return at_end;
   }

/*
=============
LoadTGA
=============
*/
static qboolean LoadTGA(const char * name, byte ** pic, int *width, int *height, qboolean * hasalpha )
   {
	int		      columns, rows, numPixels;
	byte	         *pixbuf;
	int		      row, column;
	byte	         *buf_p;
	byte	         *buffer;
	TargaHeader		targa_header;
	byte			   *targa_rgba;
	byte			   targa_palette[ 256 * 4 ];
   int            i;

	*pic = NULL;
   *hasalpha = false;

	//
	// load the file
	//
	cgi.FS_ReadFile(name, (void **)&buffer);
	if (!buffer)
	{
	   cgi.DPrintf( "TGA File not found: %s\n", name );
      return false;
	}

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;
	
	targa_header.colormap_index = LittleShort ( *((short *)buf_p) );
   buf_p += 2;
	targa_header.colormap_length = LittleShort ( *((short *)buf_p) );
   buf_p += 2;
	targa_header.colormap_size = *buf_p++ / 8;
	targa_header.x_origin = LittleShort ( *((short *)buf_p) );
   buf_p += 2;
	targa_header.y_origin = LittleShort ( *((short *)buf_p) );
   buf_p += 2;
	targa_header.width = LittleShort ( *((short *)buf_p) );
   buf_p += 2;
	targa_header.height = LittleShort ( *((short *)buf_p) );
   buf_p += 2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if (targa_header.image_type!=1 
		&& targa_header.image_type!=2
      && targa_header.image_type!=10) 
      {
		cgi.DPrintf("LoadTGA: Only type 1, 2 and 10 targa RGB images supported\n");
      return false;
      }

	if (  (targa_header.colormap_type !=0 && targa_header.colormap_type !=1)  
		|| (targa_header.pixel_size!=32 && targa_header.pixel_size!=24 && targa_header.pixel_size!=8))
      {
		cgi.DPrintf("LoadTGA: Only 24, 32 and colormap images supported (no colormaps)\n");
      return false;
      }

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;
	if (height)
		*height = rows;

   targa_rgba = ( byte * )cgi.Malloc( numPixels * 4 );
	*pic = targa_rgba;

	if (targa_header.id_length != 0)
		buf_p += targa_header.id_length;  // skip TARGA image comment
	
   for (i=0;i<targa_header.colormap_length;i++)
      {
         
         byte blue  = *buf_p++;
         byte green = *buf_p++;
         byte red   = *buf_p++;
         byte alpha = 0xff;

         if (targa_header.colormap_size == 4)
            alpha = *buf_p++;

         targa_palette[i*4+0] = red;
         targa_palette[i*4+1] = green;
         targa_palette[i*4+2] = blue;
         targa_palette[i*4+3] = alpha;
         if (alpha != 0xff)
            *hasalpha = true;
      }

   if (targa_header.image_type==1) 
      {
      for(row=rows-1; row>=0; row--)
         {
			pixbuf = targa_rgba + row*columns;
			for(column=0; column<columns; column++)
            {
				switch (targa_header.pixel_size)
               {
               case 8:
                  *pixbuf++ = targa_palette[ *buf_p * 4 + 0];
                  *pixbuf++ = targa_palette[ *buf_p * 4 + 1];
                  *pixbuf++ = targa_palette[ *buf_p * 4 + 2];
                  *pixbuf++ = targa_palette[ *buf_p * 4 + 3];
                  buf_p++;
						break;
               }
            }
         }
      }
   else if (targa_header.image_type==2) {  // Uncompressed, RGB images
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; column++) {
				unsigned char red,green,blue,alphabyte;
				switch (targa_header.pixel_size) {
					case 24:
							
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
					case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
                     if (alphabyte != 0xff)
                        *hasalpha = true;
							break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) {   // Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column=0; column<columns; ) {
				packetHeader= *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) {        // run-length packet
					switch (targa_header.pixel_size) {
						case 32:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = *buf_p++;
                        if (alphabyte != 0xff)
                           *hasalpha = true;
								break;
						case 24:
                  default:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = 255;
								break;
					}
	
					for(j=0;j<packetSize;j++) {
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						*pixbuf++=alphabyte;
						column++;
						if (column==columns) { // run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}
					}
				}
				else {                            // non run-length packet
					for(j=0;j<packetSize;j++) {
						switch (targa_header.pixel_size) {
                     default:
							case 24:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = 255;
									break;
							case 32:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									alphabyte = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = alphabyte;
                           if (alphabyte != 0xff)
                              *hasalpha = true;
									break;
						}
						column++;
						if (column==columns) { // pixel packet run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*4;
						}						
					}
				}
			}
			breakOut:;
		}
	}
	cgi.FS_FreeFile( buffer );
   return true;
   }

void createLightstyleFromTGA( int num, const char *filename )
   {
   byte     *data=0;
   int      i, j, width, height;
   qboolean hasalpha;

   if (!LoadTGA(filename, &data, &width, &height, &hasalpha))
      {
      return;
      }

   if ( width > MAX_LIGHTSTYLE_LENGTH )
      {
      cgi.DPrintf("createLightstyleFromTGA: Light style too large, truncating.\n",filename);
      width = MAX_LIGHTSTYLE_LENGTH;
      }

   strncpy( cg_lightstyle[ num ].name, filename, MAX_LIGHTSTYLE_NAME_LENGTH );
	cg_lightstyle[ num ].hasalpha = hasalpha;
	cg_lightstyle[ num ].length = width;

   for( i = 0; i < width; i++ )
      {
      for ( j = 0; j < 4; j++ )
         {
         cg_lightstyle[ num ].map[ i ][ j ] = ( float )data[ ( i * 4 ) + j ] / 255.0f;
         }
      }

   if ( data )
      cgi.Free( data );
   }

void CG_SetLightStyle( int num, const char *s )
	{
	const char	*r, *g, *b, *a;
	int	j, k, n;
	size_t len;
	float	t;

   if ( ( num < 0 ) || ( num >= ( MAX_LIGHTSTYLES * 2 ) ) )
      {
		cgi.DPrintf ("CG_SetLightStyle: num out of range.\n");
      return;
      }

   len = strlen( s );

   if ( !strcmpi( &s[ len - 4 ], ".tga" ) )
      {
      createLightstyleFromTGA( num, s );
      return;
      }

   if ( !s )
      return;

	r = strchr( s, 'R' );
	g = strchr( s, 'G' );
	b = strchr( s, 'B' );
	a = strchr( s, 'A' );

   n = 0;
   if (r)
      n++;
   if (g)
      n++;
   if (b)
      n++;
   if (a)
      n++;
   if (!n)
      n++;

   if (!a)
		cg_lightstyle[ num ].hasalpha = false;
   else
		cg_lightstyle[ num ].hasalpha = true;

	j = strlen (s) / n;
	if (j >= MAX_LIGHTSTYLE_LENGTH)
      {
		cgi.DPrintf ( "svc_lightstyle length=%i", j);
      j = MAX_LIGHTSTYLE_LENGTH - 1;
      }

	if ( !r && !g && !b && !a )
		{
		cg_lightstyle[ num ].length = j;
		for( k = 0; k < j; k++ )
			{
			t = ( float )( s[ k ] - 'a' ) * 127.5f / 12.5;
         if ( t > 255 ) 
            {
            t = 255.0f;
            }
			cg_lightstyle[ num ].map[ k ][ 0 ] = t / 255.0f;
			cg_lightstyle[ num ].map[ k ][ 1 ] = t / 255.0f;
			cg_lightstyle[ num ].map[ k ][ 2 ] = t / 255.0f;
			cg_lightstyle[ num ].map[ k ][ 3 ] = 1;
			}
		}
	else
		{
		n = 0;
		// red
		k = 0;
		t = 0;
		if ( r )
			{
			r++;
			while( ( r[ k ] >= 'a' ) && ( r[ k ] <= 'z' ) )
				{
			   t = ( float )( r[ k ] - 'a' ) * 127.5f / 12.5;
            if ( t > 255 ) 
               {
               t = 255.0f;
               }
				cg_lightstyle[ num ].map[ k ][ 0 ] = t / 255.0f;
				k++;
				}
			n = k;
			}
		while( k < MAX_LIGHTSTYLE_LENGTH )
			{
			cg_lightstyle[ num ].map[ k++ ][ 0 ] = t / 255.0f;
			}

		// green
		k = 0;
		t = 0;
		if ( g )
			{
			g++;
			while( ( g[ k ] >= 'a' ) && ( g[ k ] <= 'z' ) )
				{
			   t = ( float )( g[ k ] - 'a' ) * 127.5f / 12.5;
            if ( t > 255 ) 
               {
               t = 255.0f;
               }
				cg_lightstyle[ num ].map[ k ][ 1 ] = t / 255.0f;
				k++;
				}
			n = max( n, k );
			}
		while( k < MAX_LIGHTSTYLE_LENGTH )
			{
			cg_lightstyle[ num ].map[ k++ ][ 1 ] = t / 255.0f;
			}

		// blue
		k = 0;
		t = 0;
		if ( b )
			{
			b++;
			while( ( b[ k ] >= 'a' ) && ( b[ k ] <= 'z' ) )
				{
			   t = ( float )( b[ k ] - 'a' ) * 127.5f / 12.5;
            if ( t > 255 ) 
               {
               t = 255.0f;
               }
				cg_lightstyle[ num ].map[ k ][ 2 ] = t / 255.0f;
				k++;
				}
			n = max( n, k );
			}
		while( k < MAX_LIGHTSTYLE_LENGTH )
			{
			cg_lightstyle[ num ].map[ k++ ][ 2 ] = t / 255.0f;
			}

		// alpha
		k = 0;
		t = 255;
		if ( a )
			{
			a++;
			while( ( a[ k ] >= 'a' ) && ( a[ k ] <= 'z' ) )
				{
			   t = ( float )( a[ k ] - 'a' ) * 127.5f / 25;
            if ( t > 255 ) 
               {
               t = 255.0f;
               }
				cg_lightstyle[ num ].map[ k ][ 3 ] = t / 255.0f;
				k++;
				}
			n = max( n, k );
			}
		while( k < MAX_LIGHTSTYLE_LENGTH )
			{
			cg_lightstyle[ num ].map[ k++ ][ 3 ] = t / 255.0f;
			}

		cg_lightstyle[ num ].length = n;
		}
	}

/*
================
CG_GetLightStyle
================
*/
int CG_GetLightStyle( const char * name )
   {
   int            i;

   for( i = MAX_LIGHTSTYLES; i < MAX_LIGHTSTYLES * 2; i++ )
      {
      if ( !strcmpi( cg_lightstyle[ i ].name, name ) )
         {
         return i;
         }
      }
   return -1;
   }

/*
================
CG_GetFreeLightStyle
================
*/
int CG_GetFreeLightStyle( void )
   {
   int            i;

   for( i = MAX_LIGHTSTYLES; i < MAX_LIGHTSTYLES * 2; i++ )
      {
      if ( !cg_lightstyle[ i ].name[ 0 ] )
         {
         return i;
         }
      }
   return -1;
   }

/*
================
CG_ClearLightStyles
================
*/
void CG_ClearLightStyles( void )
   {
   memset( cg_lightstyle, 0, sizeof( cg_lightstyle ) );
   }

/*
================
CG_RegisterLightStyle
================
*/
int CG_RegisterLightStyle( const char * name )
   {
   int num;

   num = CG_GetLightStyle( name );
   if ( num == -1 )
      {
      num = CG_GetFreeLightStyle();
      if ( num != -1 )
         {
         CG_SetLightStyle( num, name );
         }
      else
         {
   		cgi.DPrintf ("CG_RegisterLightStyle: no free spots for lightstyle %s.\n");
         }
      }

   return num;
   }

