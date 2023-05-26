/*
===========================================================================
Copyright (C) 2008 Leszek Godlewski

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

#include "cm_local.h"
#include "../client/client.h"

byte cmf_dummy_trans_data;
cfencemask_t cmf_dummy_trans;

/*
====================
CM_LoadFTX
====================
*/
qboolean CM_LoadFTX( const char *name, byte **pic, int *width, int *height )
{
	int length;
	int numPixels;
	fileHandle_t h;
	ftx_t header;

	*pic = NULL;
	length = FS_FOpenFileRead( name, &h, qtrue, qtrue );
	if( length <= 0 ) {
		return qtrue;
	}

	FS_Read( &header, sizeof( ftx_t ), h );

	header.has_alpha = LittleLong(header.has_alpha);
	header.height = LittleLong(header.height);
	header.width = LittleLong(header.width);

	if( !header.has_alpha ) {
		FS_FCloseFile( h );
		return qfalse;
	}

	// Load the FTX data
	numPixels = header.width * header.height;
	*height = header.height;
	*width = header.width;
	*pic = Hunk_AllocateTempMemory( numPixels * 4 );

	FS_Read( *pic, numPixels * 4, h );
	FS_FCloseFile( h );

	return qtrue;
}

/*
====================
CM_LoadTGA
====================
*/
qboolean CM_LoadTGA( const char *name, byte **pic, int *width, int *height )
{
	int		columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	byte	*buf_p;
	byte	*buffer;
	int		length;
	TargaHeader		targa_header;
	byte			*targa_rgba;

	*pic = NULL;

	//
	// load the file
	//
	length = FS_ReadFile (name, (void **)&buffer);
	if (!buffer) {
		return qfalse;
	}

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;
	
	targa_header.colormap_index = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.colormap_length = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.y_origin = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.width = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.height = LittleShort ( *((short *)buf_p) );
	buf_p+=2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if (targa_header.image_type!=2
		&& targa_header.image_type!=10
		&& targa_header.image_type!=3) 
		Com_Error (ERR_DROP, "LoadTGA: Only type 2 (RGB), 3 (gray), and 10 (RGB) TGA images supported\n");

	if(targa_header.colormap_type!=0)
		Com_Error (ERR_DROP, "LoadTGA: colormaps not supported\n");

	if (targa_header.colormap_type !=0 
		|| (targa_header.pixel_size!=32 && targa_header.pixel_size!=24))
		Com_Error (ERR_DROP, "LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n");

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;
	if (height)
		*height = rows;

	targa_rgba = Hunk_AllocateTempMemory (numPixels*4);
	*pic = targa_rgba;

	if (targa_header.id_length != 0)
		buf_p += targa_header.id_length;  // skip TARGA image comment
	
	if(targa_header.image_type == 2 || targa_header.image_type == 3) {  // Uncompressed, RGB images
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
						case 24:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = 255;
								break;
						case 32:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = *buf_p++;
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

	FS_FreeFile (buffer);
	return qtrue;
}

/*
====================
CM_LoadFCM
====================
*/
qboolean CM_LoadFCM( const char *szName, cfencemask_t **pMask )
{
	unsigned int version;
	int length;
	int iDataSize;
	fileHandle_t h;
	fcm_t header;
	cfencemask_t *m;
	char tempName[ 256 ];

	*pMask = NULL;

	COM_StripExtension( szName, tempName, sizeof( tempName ) );
	strcat( tempName, ".fcm" );

	length = FS_FOpenFileRead( tempName, &h, qtrue, qtrue );
	if( length <= 0 ) {
		return qfalse;
	}

	FS_Read( &version, sizeof( int ), h );

	if( LittleLong( version ) != FENCEMASK_VERSION )
	{
		FS_FCloseFile( h );
		Com_Printf( "CM_LoadFCM: %s is not correct version, skipping\n", tempName );
		return qfalse;
	}

	FS_Read( &header, sizeof( fcm_t ), h );

	iDataSize = LittleLong( header.iWidth ) * LittleLong( header.iHeight ) + 7;
	if( iDataSize < 0 ) {
		iDataSize = LittleLong( header.iWidth ) * LittleLong( header.iHeight ) + 14;
	}

	iDataSize >>= 3;

	if( iDataSize != ( length - sizeof( fcm_t ) - sizeof( unsigned int ) ) )
	{
		FS_FCloseFile( h );
		Com_Printf( "CM_LoadFCM: size mismatch in %s, skipping\n", tempName );
		return qfalse;
	}

    m = (cfencemask_t*)Hunk_Alloc(sizeof(cfencemask_t), h_dontcare);
	*pMask = m;
	strcpy( m->name, szName );
	m->iWidth = header.iWidth;
    m->iHeight = header.iHeight;
    m->pData = (byte*)Hunk_Alloc(iDataSize, h_dontcare);
	FS_Read( m->pData, iDataSize, h );
	FS_FCloseFile( h );

	sprintf( tempName, "f%s", szName );
	UI_LoadResource( tempName );

	return qtrue;
}

/*
====================
CM_SaveFCM
====================
*/
void CM_SaveFCM( const char *szName, cfencemask_t **pMask )
{
	fileHandle_t h;
	unsigned int version;
	byte *pData;
	int i, j;
	int iMaskPos;
	char endline, solid, clear;
	char tempName[ 256 ];

	COM_StripExtension( szName, tempName, sizeof( tempName ) );
	strcat( tempName, ".fcm" );
	h = FS_FOpenFileWrite( tempName );

	if( !h ) {
		return;
	}

	version = FENCEMASK_VERSION;
	FS_Write( &version, sizeof( unsigned int ), h );
	FS_Write( &( *pMask )->iWidth, sizeof( int ), h );
	FS_Write( &( *pMask )->iHeight, sizeof( int ), h );

	iMaskPos = ( *pMask )->iHeight * ( *pMask )->iWidth + 7;
	if( iMaskPos < 0 ) {
		iMaskPos = ( *pMask )->iHeight * ( *pMask )->iWidth + 7 * 2;
	}
	FS_Write( ( *pMask )->pData, iMaskPos >> 3, h );
	FS_FCloseFile( h );

	// save debug info
	if( developer->integer && cm_FCMdebug->integer )
	{
		endline = '\n';
		solid = 'X';
		clear = '.';

		COM_StripExtension( szName, tempName, sizeof( tempName ) );
		strcat( tempName, ".fcm" );
		h = FS_FOpenFileWrite( tempName );
		if( !h ) {
			return;
		}

		pData = ( *pMask )->pData;
		for( i = 0; i < ( *pMask )->iHeight; i++ )
		{
			for( j = 0; j < ( *pMask )->iWidth; j++ )
			{
				int b1, b2;

				b1 = ( j + ( *pMask )->iWidth * i ) & 7;
				b2 = pData[ ( j + ( *pMask )->iWidth * i ) >> 3 ];

				if(b2 & (1 << b1)) {
					FS_Write( &solid, 1, h );
				} else {
					FS_Write( &clear, 1, h );
				}
			}

			FS_Write( &endline, 1, h );
		}

		FS_FCloseFile( h );
	}
}

/*
====================
CM_GenerateDummyFenceMask
====================
*/
cfencemask_t *CM_GenerateDummyFenceMask( const char *szName, qboolean bOpaque )
{
	cfencemask_t *pMask;

    pMask = Hunk_Alloc(sizeof(cfencemask_t) + sizeof(byte), h_dontcare);
	pMask->iWidth = 2;
	pMask->iHeight = 2;
	pMask->pData = ( byte * )( ( char * )pMask + sizeof( cfencemask_t ) );

	if( bOpaque ) {
		*( byte * )pMask->pData = -1;
	} else {
		*( byte * )pMask->pData = 0;
	}

	return pMask;
}

/*
====================
CM_GenerateFenceMask
====================
*/
qboolean CM_GenerateFenceMask( const char *szName, cfencemask_t **pMask )
{
	int i;
	int iImageWidth;
	int iImageHeight;
	int iImageSize;
	int iMaskSize;
	int iWidth;
	int iHeight;
	byte *pImage;
	byte *pCurrImage;
	byte *pCurrMask;
	qboolean bHasTrans = qfalse;
	qboolean bHasOpaque = qfalse;
	char tempName[ 256 ];

	*pMask = NULL;

	COM_StripExtension( szName, tempName, sizeof( tempName ) );
	strcat( tempName, ".ftx" );

	if( !CM_LoadFTX( tempName, &pImage, &iImageWidth, &iImageHeight ) ) {
		return qfalse;
	}

	if( !pImage )
	{
		if( !CM_LoadTGA( szName, &pImage, &iImageWidth, &iImageHeight ) ) {
			return qfalse;
		}

		if( !pImage ) {
			return qfalse;
		}
	}

	iWidth = iImageWidth;
	iHeight = iImageHeight;
	iImageSize = iImageWidth * iImageHeight;
	iMaskSize = iImageSize + 7;
	if( iMaskSize < 0 ) {
		iMaskSize = iImageSize + 7 * 2;
	}

	pCurrImage = pImage + 3;

	for( i = 0; i < iImageSize; i++, pCurrImage += sizeof( unsigned int ) )
	{
		if( *pCurrImage >= 0 )
		{
			bHasTrans = qtrue;
		} else {
			bHasOpaque = qtrue;
		}

		if( bHasOpaque && bHasTrans ) {
			break;
		}
	}

	// load the mask if it has transparence and is opaque
	if( bHasOpaque && bHasTrans )
    {
        *pMask = Hunk_Alloc((iMaskSize >> 3) + sizeof(cfencemask_t), h_dontcare);
		strcpy( ( *pMask )->name, szName );
		( *pMask )->iWidth = iWidth;
		( *pMask )->iHeight = iHeight;
		( *pMask )->pData = ( byte * )( ( char * )*pMask + sizeof( cfencemask_t ) );

		pCurrMask = ( *pMask )->pData;
		pCurrImage = pImage + 3;
		for( i = 0; i < iImageSize; i++, pCurrImage += sizeof( unsigned int ) )
		{
			if( *pCurrImage < 0 ) {
				pCurrMask[ i >> 3 ] |= 1 << ( i & 7 );
			}
		}
	} else {
		*pMask = CM_GenerateDummyFenceMask( szName, bHasOpaque );
	}

	Hunk_FreeTempMemory( pImage );
	return qtrue;
}

/*
====================
CM_CheckFCMFileRefresh
====================
*/
qboolean CM_CheckFCMFileRefresh( const char *szFile  )
{
	char tempName[ 256 ];

	COM_StripExtension( szFile, tempName, sizeof( tempName ) );
	strcat( tempName, ".fcm" );
	return FS_FileNewer( szFile, tempName ) > 0;
}

/*
====================
CM_GetFenceMask
====================
*/
cfencemask_t *CM_GetFenceMask( const char *szMaskName )
{
	qboolean save;
	cfencemask_t *pMask;

	if( !szMaskName || !*szMaskName ) {
		return NULL;
	}

	if( !memcmp( szMaskName, "nomask", 7 ) ) {
		return NULL;
	}

	if( !memcmp( szMaskName, "ignore", 7 ) ) {
		return &cmf_dummy_trans;
	}

	for( pMask = cm.fencemasks; pMask != NULL; pMask = pMask->pNext )
	{
		if( !strcmp( szMaskName, pMask->name ) ) {
			if( pMask->pData ) {
				return pMask;
			} else {
				return NULL;
			}
		}
	}

	save = qfalse;

	if( developer->integer )
	{
		if( cm_FCMcacheall->integer )
		{
			save = qtrue;
		}
		else
		{
			if( CM_CheckFCMFileRefresh( szMaskName ) ) {
				save = qtrue;
			}
		}
	}

	if( save || !CM_LoadFCM( szMaskName, &pMask ) )
	{
		if( !CM_GenerateFenceMask( szMaskName, &pMask ) )
        {
            pMask = (cfencemask_t*)Hunk_Alloc(sizeof(cfencemask_t), h_dontcare);
			strcpy( pMask->name, szMaskName );
			pMask->pData = NULL;
			pMask->pNext = cm.fencemasks;
			cm.fencemasks = pMask;
			return NULL;
		}
	}

	if( save ) {
		CM_SaveFCM( szMaskName, &pMask );
	}

	pMask->pNext = cm.fencemasks;
	cm.fencemasks = pMask;
	return pMask;
}

/*
====================
CM_TraceThroughFence
====================
*/
qboolean CM_TraceThroughFence( traceWork_t *tw, cbrush_t *brush, cbrushside_t *side, float fTraceFraction )
{
	int				i;
	int				iMaskPos;
	float			fS;
	float			fT;
	float			*vNorm;
	vec3_t			vPos;
	cfencemask_t	*pMask;

	pMask = cm.shaders[ side->shaderNum ].mask;
	if( !pMask ) {
		return qtrue;
	}

	if( pMask == &cmf_dummy_trans ) {
		return qfalse;
	}

	if( !side->pEq ) {
		return qtrue;
	}

	for( i = 0; i < 3; i++ )
	{
		vNorm = side->plane->normal;
		vPos[ i ] = tw->start[ i ] + fTraceFraction * ( tw->end[ i ] - tw->start[ i ] ) - SURFACE_CLIP_EPSILON * vNorm[ i ];
	}

	fS = DotProduct( vPos, side->pEq->fSeq ) + side->pEq->fSeq[ 3 ];
	fS = fS - floor( fS );
	fT = DotProduct( vPos, side->pEq->fTeq ) + side->pEq->fTeq[ 3 ];
	fT = fT - floor( fT );

	if( cm_FCMdebug->integer )
	{
		Com_Printf( "Trace ST coords: (%.2f %.2f) or (%i %i)\n",
			fS, fT,
			( int )( ( float )pMask->iWidth * fS ), ( int )( ( float )pMask->iHeight * fT ) );
	}

	iMaskPos = ( int )( ( float )pMask->iWidth * fS ) + pMask->iWidth * ( int )( ( float )pMask->iHeight * fT );
	return ( 1 << ( iMaskPos & 7 ) ) & pMask->pData[ iMaskPos >> 3 ];
}
