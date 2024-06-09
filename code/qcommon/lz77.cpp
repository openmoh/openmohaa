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
#include "q_shared.h"

#include <cstdio>
#include <cstring>

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
	uint8_t* ip;
	unsigned int v7;
	unsigned int v8;
	unsigned int v9;
	unsigned int v10;
	unsigned int m_off;
	uint8_t* v12;
	unsigned int v13;
	unsigned int v14;
	unsigned int v15;
	unsigned int v16;
	uint8_t* v17;
	unsigned int v18;
	uint8_t* v19;
	uint8_t* v20;
	int v22;
	uint8_t v23;
	uint8_t* v24;
	unsigned int v25;
	uint8_t* v26;
	uint8_t* m_pos;
	uint8_t* v28;
	bool v29;
	uint8_t v30;
	uint8_t v31;
	uint8_t v32;
	uint8_t v33;
	uint8_t v34;
	uint8_t v35;
	uint8_t v36;
	uint8_t v37;
	uint8_t v38;
	uint8_t v39;
	uint8_t* in_end;
	uint8_t* v41;
	uint8_t* v42;
	unsigned int v43;
	unsigned int v44;
	uint8_t* v45;
	uint8_t* i;
	uint8_t* v47;
	unsigned int v48;
	unsigned int v49;
	unsigned int m_len;
	uint8_t* v51;
	uint8_t* v52;
	uint8_t* ii;
	uint8_t* v54;
	unsigned int v55;
	char v56;
	uint8_t* v57;
	uint8_t v58;
	uint8_t* v59;

	this->in_end = &in[in_len];
	this->ip_end = &in[in_len - 13];
	this->op = out;
	this->ii = in;
	this->ip = in + 4;
	do
	{
		ip = this->ip;
		v7 = this->ip - in;
		v8 = ((unsigned int)(33 * (*this->ip ^ (32 * (this->ip[1] ^ (32 * (this->ip[2] ^ (this->ip[3] << 6))))))) >> 5) & 0x3FFF;
		this->dindex = v8;
		v9 = m_pDictionary[v8];
		this->m_off = v9;
		if (v7 <= v9)
		{
		LABEL_19:
			m_pDictionary[this->dindex] = v7;
			goto LABEL_20;
		}
		v10 = ip - v9 - in;
		this->m_off = v10;
		if (v10 > 0xBFFF)
		{
			m_pDictionary[this->dindex] = v7;
		LABEL_20:
			v19 = this->ip + 1;
			this->ip = v19;
			v20 = v19;
			continue;
		}
		m_off = this->m_off;
		v12 = ip - v10;
		this->m_pos = ip - v10;
		if (m_off <= 0x800 || v12[3] == ip[3])
		{
			if (*v12 != *ip || v12[1] != ip[1] || v12[2] != ip[2])
				goto LABEL_19;
		}
		else
		{
			v13 = (this->dindex & 0x7FF) ^ 0x201F;
			this->dindex = v13;
			v14 = m_pDictionary[v13];
			this->m_off = v14;
			if (v7 <= v14)
				goto LABEL_19;
			v15 = ip - v14 - in;
			this->m_off = v15;
			if (v15 > 0xBFFF)
			{
				m_pDictionary[this->dindex] = v7;
				goto LABEL_20;
			}
			v16 = this->m_off;
			v17 = &in[v14];
			this->m_pos = ip - v15;
			if (v16 > 0x800 && v17[3] != ip[3])
			{
				if (v7 <= v16)
					goto LABEL_19;
				v18 = ip - v16 - in;
				this->m_off = v18;
				if (v18 > 0xBFFF)
				{
					m_pDictionary[this->dindex] = v7;
					goto LABEL_20;
				}
				v17 = &in[v16];
				this->m_pos = &in[v16];
			}
			if (*v17 != *ip || v17[1] != ip[1] || v17[2] != ip[2])
				goto LABEL_19;
		}
		m_pDictionary[this->dindex] = v7;
		v22 = this->ip - this->ii;
		if (v22 > 0)
		{
			if ((unsigned int)v22 > 3)
			{
				if ((unsigned int)v22 > 0x12)
				{
					v23 = v22 - 18;
					*this->op = 0;
					v24 = this->op + 1;
					this->op = v24;
					if ((unsigned int)(v22 - 18) > 0xFF)
					{
						v25 = (v22 - 274) / 0xFFu + 1;
						do
						{
							*v24 = 0;
							++v23;
							v24 = this->op + 1;
							--v25;
							this->op = v24;
						} while (v25);
						v22 = this->ip - this->ii;
					}
					*v24 = v23;
				}
				else
				{
					*this->op = v22 - 3;
				}
				++this->op;
			}
			else
			{
				*(this->op - 2) |= v22;
			}
			do
			{
				*this->op = *this->ii;
				v26 = this->ii + 1;
				--v22;
				++this->op;
				this->ii = v26;
			} while (v22);
		}
		m_pos = this->m_pos;
		v28 = this->ip + 3;
		this->ip = v28;
		v29 = m_pos[3] == *v28;
		this->ip = v28 + 1;
		if (v29)
		{
			v30 = this->m_pos[4];
			v31 = v28[1];
			this->ip = v28 + 2;
			if (v30 == v31)
			{
				v32 = this->m_pos[5];
				v33 = v28[2];
				this->ip = v28 + 3;
				if (v32 == v33)
				{
					v34 = this->m_pos[6];
					v35 = v28[3];
					this->ip = v28 + 4;
					if (v34 == v35)
					{
						v36 = this->m_pos[7];
						v37 = v28[4];
						this->ip = v28 + 5;
						if (v36 == v37)
						{
							v38 = this->m_pos[8];
							v39 = v28[5];
							this->ip = v28 + 6;
							if (v38 == v39)
							{
								in_end = this->in_end;
								v41 = this->m_pos + 9;
								if (v28 + 6 < in_end)
								{
									do
									{
										v42 = this->ip;
										if (*v41 != *this->ip)
											break;
										++v41;
										this->ip = v42 + 1;
									} while (v42 + 1 < in_end);
								}
								v43 = this->m_off;
								v44 = this->ip - this->ii;
								this->m_len = v44;
								if (v43 > 0x4000)
								{
									v48 = v43 - 0x4000;
									this->m_off = v48;
									if (v44 <= 9)
									{
										*this->op = (this->m_len - 2) | ((v48 >> 11) & 8) | 0x10;
									LABEL_65:
										v57 = this->op + 1;
										v58 = 4 * this->m_off;
										this->op = v57;
										*v57 = v58;
										v55 = this->m_off >> 6;
										goto LABEL_66;
									}
									v49 = this->m_off >> 11;
									this->m_len = v44 - 9;
									*this->op = (v49 & 8) | 0x10;
									m_len = this->m_len;
									v51 = this->op + 1;
									this->op = v51;
									for (i = v51; m_len > 0xFF; i = v52)
									{
										this->m_len = m_len - 255;
										*i = 0;
										m_len = this->m_len;
										v52 = this->op + 1;
										this->op = v52;
									}
								}
								else
								{
									this->m_off = v43 - 1;
									if (v44 <= 0x21)
									{
										v44 = (v44 - 2) | 0x20;
									LABEL_63:
										i = this->op;
										goto LABEL_64;
									}
									this->m_len = v44 - 33;
									*this->op = 32;
									v44 = this->m_len;
									v45 = this->op + 1;
									this->op = v45;
									i = v45;
									if (v44 > 0xFF)
									{
										do
										{
											this->m_len = v44 - 255;
											*i = 0;
											v44 = this->m_len;
											v47 = this->op + 1;
											this->op = v47;
											i = v47;
										} while (v44 > 0xFF);
										goto LABEL_64;
									}
								}
								v44 = this->m_len;
							LABEL_64:
								*i = v44;
								goto LABEL_65;
							}
						}
					}
				}
			}
		}
		ii = this->ii;
		v54 = this->ip - 1;
		this->ip = v54;
		this->m_len = v54 - ii;
		v44 = this->m_off;
		if (v44 > 0x800)
		{
			if (v44 > 0x4000)
			{
				v56 = this->m_len;
				v44 -= 0x4000;
				this->m_off = v44;
				v44 = (v56 - 2) | ((v44 >> 11) & 8) | 0x10;
			}
			else
			{
				this->m_off = v44 - 1;
				v44 = (this->m_len - 2) | 0x20;
			}
			goto LABEL_63;
		}
		this->m_off = v44 - 1;
		*this->op = (4 * (this->m_off & 7)) | (32 * (this->m_len - 1));
		v55 = this->m_off >> 3;
	LABEL_66:
		v59 = this->op + 1;
		this->op = v59;
		*v59 = v55;
		++this->op;
		v20 = this->ip;
		this->ii = this->ip;
	} while (v20 < this->ip_end);
	*out_len = this->op - out;
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
	uint8_t* v6;
	size_t v7;
	uint8_t* v8;
	uint8_t v9;
	unsigned int v10;
	uint8_t* v11;
	uint8_t v12;
	byte* v13;
	int result;
	size_t in_lena;
	char outa;

	v6 = out;
	if (in_len > 0xD)
	{
		v7 = cLZ77::CompressData(in, in_len, out, out_len);
		v6 = &out[*out_len];
	}
	else
	{
		v7 = in_len;
	}
	if (v7)
	{
		v8 = &in[in_len - v7];
		in_lena = (size_t)v8;
		if (v6 == out && v7 <= 0xEE)
		{
			v9 = v7 + 17;
		}
		else
		{
			if (v7 <= 3)
			{
				*(v6 - 2) |= v7;
				goto LABEL_18;
			}
			if (v7 <= 0x12)
			{
				*v6 = v7 - 3;
			LABEL_17:
				++v6;
				do
				{
				LABEL_18:
					*v6++ = *v8++;
					--v7;
				} while (v7);
				goto LABEL_19;
			}
			v9 = v7 - 18;
			*v6++ = 0;
			outa = v7 - 18;
			if (v7 - 18 > 0xFF)
			{
				v10 = (v7 - 274) / 0xFF + 1;
				memset(v6, 0, 4 * (v10 >> 2));
				v11 = &v6[4 * (v10 >> 2)];
				v6 += v10;
				memset(v11, 0, v10 & 3);
				do
				{
					v12 = outa + 1;
					--v10;
					++outa;
				} while (v10);
				v8 = (uint8_t*)in_lena;
				v9 = v12;
			}
		}
		*v6 = v9;
		goto LABEL_17;
	}
LABEL_19:
	*v6 = 17;
	v13 = v6 + 1;
	*v13++ = 0;
	result = 0;
	*v13 = 0;
	*out_len = v13 - out + 1;
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
	unsigned int t;

	this->ip_end = &in[in_len];
	*out_len = 0;
	this->op = out;
	this->ip = in;
	if (*this->ip > 17u)
	{
		t = *this->ip++ - 17;
		if (t <= 3)
			goto LABEL_52;
		do
		{
			*this->op++ = *this->ip++;
			--t;
		} while (t);
		goto LABEL_17;
	}

LABEL_5:
	t = *this->ip++;
	if (t <= 15)
	{
		if (!t)
		{
			while (!*this->ip)
			{
				t += 255;
				++this->ip;
			}
			t += 15 + *this->ip++;
		}
		*(unsigned int*)this->op = *(unsigned int*)this->ip;
		this->op += 4;
		this->ip += 4;
		if (--t)
		{
			if (t <= 3)
			{
				do
				{
					*this->op++ = *this->ip++;
					--t;
				} while (t);
			}
			else
			{
				do
				{
					*(unsigned int*)this->op = *(unsigned int*)this->ip;
					this->op += 4;
					this->ip += 4;
					t -= 4;
				} while (t > 3);
				for (; t; --t)
					*this->op++ = *this->ip++;
			}
		}
	LABEL_17:
		t = *this->ip++;
		if (t <= 0xF)
		{
			this->m_pos = this->op - 2049;
			this->m_pos -= t >> 2;
			this->m_pos -= 4 * *this->ip++;
			*this->op = *this->m_pos++;
			*++this->op = *this->m_pos++;
			*++this->op = *this->m_pos;
			++this->op;
			goto LABEL_51;
		}
	}
	while (1)
	{
		if (t > 0x3F)
		{
			this->m_pos = this->op - 1;
			this->m_pos -= (t >> 2) & 7;
			this->m_pos -= 8 * *this->ip++;
			t = (t >> 5) - 1;
			*this->op = *this->m_pos++;
			*++this->op = *this->m_pos++;
			++this->op;
			do
			{
				*this->op++ = *this->m_pos++;
				--t;
			} while (t);
			goto LABEL_51;
		}
		if (t > 0x1F)
		{
			t &= 0x1Fu;
			if (!t)
			{
				while (!*this->ip)
				{
					t += 255;
					++this->ip;
				}
				t += 31 + *this->ip++;
			}
			this->m_pos = this->op - 1;
			this->m_pos -= LittleUnsignedShort(*(unsigned short*)this->ip) >> 2;
			this->ip += 2;
			goto LABEL_42;
		}
		if (t <= 0xF)
		{
			this->m_pos = this->op - 1;
			this->m_pos -= t >> 2;
			this->m_pos -= 4 * *this->ip++;
			*this->op = *this->m_pos++;
			*++this->op = *this->m_pos;
			++this->op;
			goto LABEL_51;
		}
		this->m_pos = this->op;
		this->m_pos -= 2048 * (t & 8);
		t &= 7u;
		if (!t)
		{
			while (!*this->ip)
			{
				t += 255;
				++this->ip;
			}
			t += 7 + *this->ip++;
		}
		this->m_pos -= LittleUnsignedShort(*(unsigned short*)this->ip) >> 2;
		this->ip += 2;
		if (this->m_pos == this->op)
			break;
		this->m_pos -= 0x4000;
	LABEL_42:
		if (t <= 5 || this->op - this->m_pos <= 3)
		{
			*this->op = *this->m_pos++;
			*++this->op = *this->m_pos++;
			++this->op;
			do
			{
				*this->op++ = *this->m_pos++;
				--t;
			} while (t);
		}
		else
		{
			*(unsigned int*)this->op = *(unsigned int*)this->m_pos;
			this->op += 4;
			this->m_pos += 4;
			t -= 2;
			do
			{
				*(unsigned int*)this->op = *(unsigned int*)this->m_pos;
				this->op += 4;
				this->m_pos += 4;
				t -= 4;
			} while (t > 3);
			for (; t; --t)
				*this->op++ = *this->m_pos++;
		}
	LABEL_51:
		t = *(this->ip - 2) & 3;

		if (!t)
			goto LABEL_5;

	LABEL_52:
		do
		{
			*this->op++ = *this->ip++;
			--t;
		} while (t);

		t = *this->ip++;
	}

	*out_len = this->op - out;

	if (this->ip == this->ip_end)
		return 0;

	if (this->ip >= this->ip_end)
		return -2;
	return -1;
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

	printf( "Compressed %i bytes into %zi bytes\n", 0x40000, out_len );

	if( lz77.Decompress( out, out_len, in, &in_len ) )
	{
		new_len = in_len;
	}
	else
	{
		new_len = in_len;

		if( in_len == 0x40000 )
		{
			printf( "Decompressed %zi bytes into %i bytes\n", out_len, 0x40000 );
			puts( "Compression Test: Passed" );
			return;
		}
	}

	printf( "Decompression got FuBar'd... %i != %zi\n", 0x40000, new_len );
}
