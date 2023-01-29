/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

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

// lz77.cpp: LZ77 Compression Algorithm

// FIXME: MOHAA devs decided to make things on their own, so we need to find out their lz77 compression algorithm...
// we need this to make .pth files work properly
// so we currently use an ugly decompile hack to do so

#include "lz77.h"
#include <stdio.h>
#include <string.h>

unsigned int cLZ77::m_pDictionary[ 65535 ];

cLZ77 g_lz77;

/*************************************************************************
* Constants used for LZ77 coding
*************************************************************************/

/* Maximum offset (can be any size < 2^31). Lower values give faster
compression, while higher values gives better compression. The default
value of 100000 is quite high. Experiment to see what works best for
you. */
#define LZ_MAX_OFFSET 100000



/*************************************************************************
*                           INTERNAL FUNCTIONS                           *
*************************************************************************/


/*************************************************************************
* _LZ_StringCompare() - Return maximum length string match.
*************************************************************************/

static unsigned int _LZ_StringCompare( unsigned char * str1,
	unsigned char * str2, unsigned int minlen, unsigned int maxlen )
{
	unsigned int len;

	for( len = minlen; ( len < maxlen ) && ( str1[ len ] == str2[ len ] ); ++len );

	return len;
}


/*************************************************************************
* _LZ_WriteVarSize() - Write unsigned integer with variable number of
* bytes depending on value.
*************************************************************************/

static int _LZ_WriteVarSize( unsigned int x, unsigned char * buf )
{
	unsigned int y;
	int num_bytes, i, b;

	/* Determine number of bytes needed to store the number x */
	y = x >> 3;
	for( num_bytes = 5; num_bytes >= 2; --num_bytes )
	{
		if( y & 0xfe000000 ) break;
		y <<= 7;
	}

	/* Write all bytes, seven bits in each, with 8:th bit set for all */
	/* but the last byte. */
	for( i = num_bytes - 1; i >= 0; --i )
	{
		b = ( x >> ( i * 7 ) ) & 0x0000007f;
		if( i > 0 )
		{
			b |= 0x00000080;
		}
		*buf++ = ( unsigned char )b;
	}

	/* Return number of bytes written */
	return num_bytes;
}


/*************************************************************************
* _LZ_ReadVarSize() - Read unsigned integer with variable number of
* bytes depending on value.
*************************************************************************/

static int _LZ_ReadVarSize( unsigned int * x, unsigned char * buf )
{
	unsigned int y, b, num_bytes;

	/* Read complete value (stop when byte contains zero in 8:th bit) */
	y = 0;
	num_bytes = 0;
	do
	{
		b = ( unsigned int )( *buf++ );
		y = ( y << 7 ) | ( b & 0x0000007f );
		++num_bytes;
	} while( b & 0x00000080 );

	/* Store value in x */
	*x = y;

	/* Return number of bytes read */
	return num_bytes;
}



/*************************************************************************
*                            PUBLIC FUNCTIONS                            *
*************************************************************************/

cLZ77::cLZ77()
{
}

/*************************************************************************
* cLZ77::CompressData - Compress a block of data using an LZ77 coder.
*  in      - Input (uncompressed) buffer.
*  out     - Output (compressed) buffer. This buffer must be 0.4% larger
*            than the input buffer, plus one byte.
*  insize  - Number of input bytes.
*  out_len - Output length
* The function returns the size of the compressed data.
*************************************************************************/

unsigned int cLZ77::CompressData( unsigned char *in, size_t in_len, unsigned char *out, size_t *out_len )
{
	unsigned char *v5; // eax@1
	unsigned char *v6; // ebx@2
	unsigned int v7; // eax@3
	int v8; // edx@3
	unsigned int v9; // ecx@3
	cLZ77 *v10; // edi@3
	unsigned int v11; // esi@3
	unsigned int v12; // eax@4
	int v13; // ecx@5
	unsigned int v14; // eax@7
	cLZ77 *v15; // ecx@7
	int v16; // esi@7
	int v17; // ecx@8
	int v18; // edi@9
	int v19; // eax@12
	int v20; // edx@13
	char v21; // zf@15
	char v22; // cf@17
	unsigned int v24; // eax@20
	unsigned char *v25; // ecx@20
	unsigned char *v26; // ebx@20
	unsigned char *v27; // ecx@21
	unsigned char *v28; // eax@23
	unsigned char *v29; // eax@23
	unsigned char *v30; // edx@25
	unsigned int v31; // eax@26
	unsigned char *v32; // ebx@26
	unsigned int v33; // eax@27
	unsigned char *v34; // edx@27
	char v35; // cl@27
	cLZ77 *v36; // ebx@27
	char v37; // cf@29
	unsigned char *v38; // esi@32
	char v39; // dl@32
	unsigned char *v40; // edi@32
	char v41; // dl@32
	unsigned char *v42; // edi@32
	unsigned int v43; // ecx@32
	unsigned char *v44; // esi@33
	char v45; // cl@33
	unsigned char *v46; // edi@33
	char v47; // dl@33
	unsigned char *v48; // edi@33
	unsigned int v49; // ecx@33
	int v50; // esi@39
	unsigned int v51; // eax@41
	unsigned int v52; // edx@41
	unsigned int v53; // eax@45
	unsigned char *v54; // edi@45
	unsigned char *v55; // edx@46
	unsigned char *v56; // edx@47
	cLZ77 *v57; // ecx@47
	int v58; // ecx@49
	unsigned int v59; // eax@51
	unsigned char *v60; // ebx@51
	unsigned char *v61; // edi@52
	unsigned char *v62; // edx@54
	unsigned char *v63; // edi@54
	cLZ77 *v64; // edi@58
	int v65; // edx@60
	unsigned char *v66; // [sp+0h] [bp-10h]@39

	this->op = out;
	v5 = in + 4;
	this->in_end = &in[ in_len ];
	this->ip_end = &in[ in_len - 13 ];
	this->ii = in;
	this->ip = in + 4;
	do
	{
		v6 = v5;
		while( 1 )
		{
			v8 = v6 - in;
			v10 = this;
			v11 = 33 * ( *v6 ^ 32 * ( v6[ 1 ] ^ 32 * ( v6[ 2 ] ^ ( ( unsigned int )v6[ 3 ] << 6 ) ) ) ) >> 5;
			v9 = ( 33 * ( *v6 ^ 32 * ( v6[ 1 ] ^ 32 * ( v6[ 2 ] ^ ( ( unsigned int )v6[ 3 ] << 6 ) ) ) ) >> 5 ) & 0x3FFF;
			this->dindex = v9;
			v7 = m_pDictionary[ v9 ];
			this->m_off = v7;
			if( v6 - in <= v7 || ( v12 = v8 - v7, v10 = this, this->m_off = v12, v12 > 0xBFFF ) )
			{
				m_pDictionary[ v9 ] = v8;
				goto LABEL_64;
			}
			v13 = ( int )&v6[ -v12 ];
			this->m_pos = &v6[ -v12 ];
			if( v12 <= 0x800 || *( unsigned char * )( v13 + 3 ) == v6[ 3 ] )
			{
				if( *( unsigned char * )v13 != *v6 || *( unsigned char * )( v13 + 1 ) != v6[ 1 ] || *( unsigned char * )( v13 + 2 ) != v6[ 2 ] )
				{
LABEL_17:
					m_pDictionary[ dindex ] = v6 - in;
					v5 = this->ip + 1;
					v22 = v5 < this->ip_end;
					this->ip = v5;
					goto LABEL_18;
				}
				goto LABEL_20;
			}
			v15 = this;
			v16 = v11 & 0x7FF ^ 0x201F;
			this->dindex = v16;
			v14 = m_pDictionary[ v16 ];
			this->m_off = v14;
			if( v8 <= v14 )
				break;
			v10 = this;
			v17 = v8 - v14;
			this->m_off = v8 - v14;
			if( v8 - v14 > 0xBFFF )
				goto LABEL_63;
			v18 = ( int )&v6[ -v17 ];
			this->m_pos = &v6[ -v17 ];
			if( ( unsigned int )v17 <= 0x800 || *( unsigned char * )( v18 + 3 ) == v6[ 3 ] )
			{
				if( *( unsigned char * )v18 != *v6 || *( unsigned char * )( v18 + 1 ) != v6[ 1 ] )
					goto LABEL_17;
				v21 = *( unsigned char * )( v18 + 2 ) == v6[ 2 ];
			}
			else
			{
				if( v8 <= ( unsigned int )v17 )
				{
					m_pDictionary[ v16 ] = v8;
					v15 = this;
					goto LABEL_66;
				}
				v10 = this;
				v19 = v8 - v17;
				this->m_off = v8 - v17;
				if( ( unsigned int )( v8 - v17 ) > 0xBFFF )
				{
LABEL_63:
					m_pDictionary[ v16 ] = v8;
LABEL_64:
					v5 = v10->ip + 1;
					v22 = v5 < v10->ip_end;
					v10->ip = v5;
					goto LABEL_18;
				}
				v20 = ( int )&v6[ -v19 ];
				this->m_pos = &v6[ -v19 ];
				if( v6[ -v19 ] != *v6 || *( unsigned char * )( v20 + 1 ) != v6[ 1 ] )
					goto LABEL_17;
				v21 = *( unsigned char * )( v20 + 2 ) == v6[ 2 ];
			}
			if( !v21 )
				goto LABEL_17;
LABEL_20:
			m_pDictionary[ this->dindex ] = v6 - in;
			v26 = this->ip;
			v25 = this->ii;
			v24 = ( unsigned int )&this->ip[ -( unsigned int )v25 ];
			if( ( signed int )&this->ip[ -( unsigned int )v25 ] > 0 )
			{
				v27 = &this->ip[ -( unsigned int )v25 ];
				if( v24 > 3 )
				{
					if( v24 > 0x12 )
					{
						v65 = v24 - 18;
						*this->op++ = 0;
						if( v24 - 18 > 0xFF )
						{
							do
							{
								v65 -= 255;
								*this->op++ = 0;
							} while( ( unsigned int )v65 > 0xFF );
						}
						v64 = this;
						*this->op = v65;
					}
					else
					{
						v64 = this;
						*this->op = v24 - 3;
					}
					++v64->op;
				}
				else
				{
					*( this->op - 2 ) |= v24;
				}
				do
				{
					*this->op = *this->ii;
					v29 = this->ii;
					++this->op;
					v28 = v29 + 1;
					--v27;
					this->ii = v28;
				} while( v27 );
				v26 = this->ip;
				v25 = v28;
			}
			v30 = this->m_pos;
			this->ip = v26 + 4;
			if( v30[ 3 ] == v26[ 3 ] )
			{
				this->ip = v26 + 5;
				if( v30[ 4 ] == v26[ 4 ] )
				{
					this->ip = v26 + 6;
					if( v30[ 5 ] == v26[ 5 ] )
					{
						this->ip = v26 + 7;
						if( v30[ 6 ] == v26[ 6 ] )
						{
							this->ip = v26 + 8;
							if( v30[ 7 ] == v26[ 7 ] )
							{
								this->ip = v26 + 9;
								if( v30[ 8 ] == v26[ 8 ] )
								{
									v50 = ( int )( v30 + 9 );
									v66 = this->in_end;
									if( v26 + 9 < v66 )
									{
										if( v26[ 9 ] == v30[ 9 ] )
										{
											do
											{
												++v50;
												v62 = this->ip;
												v63 = this->ip + 1;
												this->ip = v63;
											} while( v63 < v66 && v62[ 1 ] == *( unsigned char * )v50 );
										}
									}
									v51 = this->m_off;
									v52 = ( unsigned int )&this->ip[ -( unsigned int )v25 ];
									this->m_len = v52;
									if( v51 > 0x4000 )
									{
										v58 = v51 - 16384;
										this->m_off = v51 - 16384;
										if( v52 <= 9 )
										{
											*this->op++ = ( ( v58 & 0x4000u ) >> 11 ) | ( unsigned char )( *( unsigned char * )&( this->m_len ) - 2 ) | 0x10;
											goto LABEL_44;
										}
										v60 = this->op;
										this->m_len = v52 - 9;
										*v60 = ( ( v58 & 0x4000u ) >> 11 ) | 0x10;
										v59 = this->m_len;
										++this->op;
										for( ; v59 > 0xFF; ++this->op )
										{
											v61 = this->op;
											this->m_len = v59 - 255;
											*v61 = 0;
											v59 = this->m_len;
										}
										v56 = this->op;
										v57 = this;
									}
									else
									{
										this->m_off = v51 - 1;
										if( v52 <= 0x21 )
										{
											*this->op++ = ( *( unsigned char * )&( this->m_len ) - 2 ) | 0x20;
LABEL_44:
											v36 = this;
											*this->op = 4 * ( this->m_off & 0x3F );
											v34 = this->op;
											v33 = this->m_off >> 6;
											this->op = v34 + 1;
											goto LABEL_28;
										}
										v54 = this->op;
										this->m_len = v52 - 33;
										*v54 = 32;
										v53 = this->m_len;
										++this->op;
										for( ; v53 > 0xFF; ++this->op )
										{
											v55 = this->op;
											this->m_len = v53 - 255;
											*v55 = 0;
											v53 = this->m_len;
										}
										v57 = this;
										v56 = this->op;
									}
									*v56 = *( unsigned char * )&( v57->m_len );
									++v57->op;
									goto LABEL_44;
								}
							}
						}
					}
				}
			}
			v31 = this->m_off;
			v32 = this->ip - 1;
			this->ip = v32;
			this->m_len = v32 - v25;
			if( v31 > 0x800 )
			{
				if( v31 > 0x4000 )
				{
					v44 = this->op;
					v45 = *( unsigned char * )&( this->m_len );
					this->m_off = v31 - 16384;
					*v44 = ( ( ( v31 - 16384 ) & 0x4000 ) >> 11 ) | ( unsigned char )( v45 - 2 ) | 0x10;
					v46 = this->op;
					v47 = *( unsigned char * )&( this->m_off );
					this->op = v46 + 1;
					v46[ 1 ] = 4 * ( v47 & 0x3F );
					v48 = this->op;
					v49 = this->m_off >> 6;
					this->op = v48 + 1;
					v48[ 1 ] = v49;
					++this->op;
				}
				else
				{
					v38 = this->op;
					v39 = *( unsigned char * )&( this->m_len );
					this->m_off = v31 - 1;
					*v38 = ( v39 - 2 ) | 0x20;
					v40 = this->op;
					v41 = *( unsigned char * )&( this->m_off );
					this->op = v40 + 1;
					v40[ 1 ] = 4 * ( v41 & 0x3F );
					v42 = this->op;
					v43 = this->m_off >> 6;
					this->op = v42 + 1;
					v42[ 1 ] = v43;
					++this->op;
				}
				goto LABEL_29;
			}
			v35 = *( unsigned char * )&( this->m_len );
			v36 = this;
			this->m_off = v31 - 1;
			*this->op = 4 * ( this->m_off & 7 ) | 32 * ( v35 - 1 );
			v34 = this->op;
			v33 = this->m_off >> 3;
			this->op = v34 + 1;
LABEL_28:
			v34[ 1 ] = v33;
			++v36->op;
LABEL_29:
			v37 = this->ip < this->ip_end;
			v6 = this->ip;
			this->ii = this->ip;
			if( !v37 )
				goto LABEL_19;
		}
		m_pDictionary[ v16 ] = v8;
LABEL_66:
		v5 = v15->ip + 1;
		v22 = v5 < v15->ip_end;
		v15->ip = v5;
LABEL_18:
		;
	} while( v22 );
LABEL_19:
	*out_len = ( unsigned int )&this->op[ -( unsigned int )out ];
	return this->in_end - this->ii;
}

/*************************************************************************
* cLZ77::Compress - Compress a block of data using an LZ77 coder.
*  in      - Input (uncompressed) buffer.
*  out     - Output (compressed) buffer. This buffer must be 0.4% larger
*            than the input buffer, plus one byte.
*  insize  - Number of input bytes.
*  out_len - Output length
* The function returns the size of the compressed data.
*************************************************************************/

int cLZ77::Compress( unsigned char *in, size_t in_len, unsigned char *out, size_t *out_len )
{
	unsigned char *v5; // edx@1
	unsigned int v6; // ecx@1
	unsigned char *v7; // ebx@3
	unsigned char v8; // al@6
	int result; // eax@7
	int v10; // edx@7
	int v11; // eax@9

	v5 = out;
	v6 = in_len;
	if( in_len > 0xD )
	{
		v6 = CompressData( in, in_len, out, out_len );
		v5 = &out[ *out_len ];
	}
	if( v6 )
	{
		v7 = &in[ in_len ] - v6;
		if( v5 != out || v6 > 0xEE )
		{
			if( v6 <= 3 )
			{
				*( v5 - 2 ) |= v6;
				goto LABEL_6;
			}
			if( v6 > 0x12 )
			{
				*v5 = 0;
				v11 = v6 - 18;
				++v5;
				if( v6 - 18 > 0xFF )
				{
					do
					{
						*v5 = 0;
						v11 -= 255;
						++v5;
					} while( ( unsigned int )v11 > 0xFF );
				}
			}
			else
			{
				*( unsigned char * )&v11 = v6 - 3;
			}
		}
		else
		{
			*( unsigned char * )&v11 = v6 + 17;
		}
		*v5++ = v11;
		do
		{
LABEL_6:
			v8 = *v7++;
			*v5++ = v8;
			--v6;
		} while( v6 );
	}
	*v5 = 17;
	v10 = ( int )( v5 + 1 );
	result = 0;
	*( unsigned char * )v10++ = 0;
	*( unsigned char * )v10 = 0;
	*out_len = v10 + 1 - ( unsigned int )out;
	return result;
}

/*************************************************************************
* cLZ77::Decompress - Uncompress a block of data using an LZ77 decoder.
*  in      - Input (compressed) buffer.
*  out     - Output (uncompressed) buffer. This buffer must be large
*            enough to hold the uncompressed data.
*  insize  - Number of input bytes.
*  out_len - Output length
*************************************************************************/
int cLZ77::Decompress( unsigned char *in, size_t in_len, unsigned char *out, size_t *out_len )
{
	unsigned char v5; // al@1
	int v6; // esi@2
	int v7; // ebx@4
	unsigned int v8; // esi@4
	unsigned char *v9; // ecx@5
	int v10; // edx@5
	int v11; // edx@5
	unsigned char *v12; // edx@5
	unsigned char *v13; // esi@5
	unsigned char *v14; // esi@6
	unsigned char *v15; // ebx@6
	int v16; // ecx@7
	unsigned char v17; // dl@7
	int v18; // eax@8
	unsigned char *v19; // ebx@12
	int v20; // esi@12
	unsigned int v21; // esi@12
	int v22; // ecx@12
	int v23; // edx@12
	int v24; // eax@12
	int v25; // ecx@12
	unsigned char *v26; // ecx@12
	unsigned char *v27; // ebx@12
	int v28; // esi@16
	int v29; // ecx@17
	int v30; // ebx@17
	int v31; // edx@18
	int v32; // ecx@18
	int v33; // ebx@20
	int v34; // ecx@21
	int v35; // ebx@21
	unsigned int v36; // edx@21
	unsigned char *v37; // edx@23
	int v38; // esi@24
	unsigned char *v39; // ecx@29
	unsigned char *v40; // ebx@29
	int v41; // ebx@34
	int v42; // ecx@35
	int v43; // ebx@35
	int v44; // edx@36
	int v45; // ecx@36
	int v46; // edx@38
	int v47; // ecx@39
	unsigned char *v48; // ecx@39
	unsigned int v49; // edx@39
	unsigned int v50; // eax@41
	int result; // eax@42
	unsigned char *v52; // ebx@45
	int v53; // edx@45
	int v54; // esi@45
	int v55; // ecx@45
	int v56; // edx@45
	int v57; // ebx@49
	int v58; // eax@50
	int v59; // esi@51
	unsigned char *v60; // [sp+0h] [bp-14h]@21

	ip_end = &in[ in_len ];
	*( unsigned int * )out_len = 0;
	ip = in;
	op = ( unsigned char * )out;
	v5 = *in;
	if( *in <= 17u )
	{
		v16 = ( int )ip;
		goto LABEL_46;
	}
	v6 = v5 - 17;
	ip = in + 1;
	if( ( unsigned int )v5 - 17 <= 3 )
	{
		v16 = ( int )( in + 1 );
		goto LABEL_8;
	}
	do
	{
		*op++ = *ip++;
		--v6;
	} while( v6 );
LABEL_4:
	v7 = ( int )ip;
	v8 = *ip++;
	if( v8 <= 0xF )
	{
		v9 = op;
		v10 = ( int )&v9[ -( v8 >> 2 ) - 2049 ];
		m_pos = ( unsigned char * )v10;
		v11 = v10 - 4 * *( unsigned char * )( v7 + 1 );
		m_pos = ( unsigned char * )v11;
		ip = ( unsigned char * )( v7 + 2 );
		*v9 = *( unsigned char * )v11;
		v12 = m_pos;
		m_pos = v12 + 1;
		v13 = op;
		op = v13 + 1;
		v13[ 1 ] = v12[ 1 ];
LABEL_6:
		v14 = m_pos;
		m_pos = v14 + 1;
		v15 = op;
		op = v15 + 1;
		v15[ 1 ] = v14[ 1 ];
		++op;
		goto LABEL_7;
	}
	while( 1 )
	{
		if( v8 > 0x3F )
		{
			v19 = op;
			v21 = v8 >> 2;
			v22 = ( int )&v19[ -( v21 & 7 ) - 1 ];
			v23 = ( int )ip;
			m_pos = ( unsigned char * )v22;
			v24 = *( unsigned char * )v23;
			ip = ( unsigned char * )( v23 + 1 );
			v20 = ( v21 >> 3 ) - 1;
			v25 = v22 - 8 * v24;
			m_pos = ( unsigned char * )v25;
			*v19 = *( unsigned char * )v25;
			v26 = m_pos;
			m_pos = v26 + 1;
			v27 = op;
			op = v27 + 1;
			v27[ 1 ] = v26[ 1 ];
			++m_pos;
			++op;
			do
			{
				*op++ = *m_pos++;
				--v20;
			} while( v20 );
			goto LABEL_7;
		}
		if( v8 > 0x1F )
		{
			v28 = v8 & 0x1F;
			if( !v28 )
			{
				v29 = ( int )ip;
				v30 = ( int )ip;
				if( !*ip )
				{
					do
					{
						v31 = v30 + 1;
						v32 = v30;
						v28 += 255;
						ip = ( unsigned char * )( v30++ + 1 );
					} while( !*( unsigned char * )( v32 + 1 ) );
					v29 = v31;
				}
				v33 = *( unsigned char * )v29;
				ip = ( unsigned char * )( v29 + 1 );
				v28 += v33 + 31;
			}
			v34 = ( int )( op - 1 );
			v60 = op;
			v35 = ( int )ip;
			m_pos = ( unsigned char * )v34;
			v36 = *( unsigned short * )v35;
			ip = ( unsigned char * )( v35 + 2 );
			m_pos = ( unsigned char * )( v34 - ( v36 >> 2 ) );
			goto LABEL_22;
		}
		if( v8 <= 0xF )
		{
			v52 = op;
			v53 = ( int )&v52[ -( v8 >> 2 ) - 1 ];
			v54 = ( int )ip;
			m_pos = ( unsigned char * )v53;
			v55 = *( unsigned char * )v54;
			ip = ( unsigned char * )( v54 + 1 );
			v56 = v53 - 4 * v55;
			m_pos = ( unsigned char * )v56;
			*v52 = *( unsigned char * )v56;
			goto LABEL_6;
		}
		v60 = op;
		v41 = ( int )&v60[ -2048 * ( v8 & 8 ) ];
		v28 = v8 & 7;
		m_pos = ( unsigned char * )v41;
		if( !v28 )
		{
			v42 = ( int )ip;
			v43 = ( int )ip;
			if( !*ip )
			{
				do
				{
					v44 = v43 + 1;
					v45 = v43;
					v28 += 255;
					ip = ( unsigned char * )( v43++ + 1 );
				} while( !*( unsigned char * )( v45 + 1 ) );
				v42 = v44;
			}
			v46 = *( unsigned char * )v42;
			ip = ( unsigned char * )( v42 + 1 );
			v28 += v46 + 7;
		}
		v48 = m_pos;
		v49 = *( unsigned short * )ip;
		ip += 2;
		v47 = ( int )&v48[ -( v49 >> 2 ) ];
		m_pos = ( unsigned char * )v47;
		if( ( unsigned char * )v47 == v60 )
			break;
		m_pos = ( unsigned char * )( v47 - 16384 );
LABEL_22:
		if( ( unsigned int )v28 <= 5 )
		{
			v37 = m_pos;
LABEL_29:
			*v60 = *v37;
			v39 = m_pos;
			m_pos = v39 + 1;
			v40 = op;
			op = v40 + 1;
			v40[ 1 ] = v39[ 1 ];
			++m_pos;
			++op;
			do
			{
				*op++ = *m_pos++;
				--v28;
			} while( v28 );
			goto LABEL_7;
		}
		v37 = m_pos;
		if( ( signed int )( v60 - v37 ) <= 3 )
			goto LABEL_29;
		v38 = v28 - 2;
		*( unsigned int * )v60 = *( unsigned int * )v37;
		op += 4;
		m_pos += 4;
		do
		{
			v38 -= 4;
			*( unsigned int * )op = *( unsigned int * )m_pos;
			op += 4;
			m_pos += 4;
		} while( ( unsigned int )v38 > 3 );
		for( ; v38; --v38 )
			*op++ = *m_pos++;
LABEL_7:
		v16 = ( int )ip;
		v17 = *( ip - 2 ) & 3;
		v6 = v17;
		if( v17 )
		{
LABEL_8:
			while( 1 )
			{
				*op = *( unsigned char * )v16;
				v18 = ( int )ip;
				++op;
				--v6;
				ip = ( unsigned char * )( v18 + 1 );
				if( !v6 )
					break;
				v16 = v18 + 1;
			}
			v8 = *( unsigned char * )( v18 + 1 );
			ip = ( unsigned char * )( v18 + 2 );
		}
		else
		{
LABEL_46:
			v8 = *( unsigned char * )v16;
			ip = ( unsigned char * )( v16 + 1 );
			if( v8 <= 0xF )
			{
				if( !v8 )
				{
					if( !*( unsigned char * )( v16 + 1 ) )
					{
						do
						{
							v57 = ( int )ip;
							v8 += 255;
							++ip;
						} while( !*( unsigned char * )( v57 + 1 ) );
					}
					v58 = *ip++;
					v8 += v58 + 15;
				}
				*( unsigned int * )op = *( unsigned int * )ip;
				op += 4;
				ip += 4;
				v59 = v8 - 1;
				if( v59 )
				{
					if( ( unsigned int )v59 <= 3 )
					{
						do
						{
							*op++ = *ip++;
							--v59;
						} while( v59 );
					}
					else
					{
						do
						{
							v59 -= 4;
							*( unsigned int * )op = *( unsigned int * )ip;
							op += 4;
							ip += 4;
						} while( ( unsigned int )v59 > 3 );
						for( ; v59; --v59 )
							*op++ = *ip++;
					}
				}
				goto LABEL_4;
			}
		}
	}
	*out_len = v47 - ( unsigned int )out;
	v50 = ( unsigned int )ip_end;
	if( ip == ( unsigned char * )v50 )
		result = 0;
	else
		result = -( ip < ( unsigned char * )v50 ) | 0xFFFFFFFE;
	return result;
}

static unsigned char in[ 0x40000 ];
static unsigned char out[ 0x41013 ];

void test_compression()
{
	size_t in_len;
	size_t out_len;
	size_t new_len;
	cLZ77 lz77;

	memset( &in, 0, 0x40000 );

	if( lz77.Compress( in, sizeof( in ), out, &out_len ) )
	{
		puts( "Compression Failed!" );
		return;
	}

	printf( "Compressed %i bytes into %i bytes\n", 0x40000, out_len );

	if( lz77.Decompress( out, out_len, in, &in_len ) )
	{
		new_len = in_len;
	}
	else
	{
		new_len = in_len;

		if( in_len == 0x40000 )
		{
			printf( "Decompressed %i bytes into %i bytes\n", out_len, 0x40000 );
			puts( "Compression Test: Passed" );
			return;
		}
	}

	printf( "Decompression got FuBar'd... %i != %i\n", 0x40000, new_len );
}
