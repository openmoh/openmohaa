/******
gutil.c
GameSpy C Engine SDK
  
Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com


******/
#if defined(applec) || defined(THINK_C) || defined(__MWERKS__) && !defined(__KATANA__) && !defined(__mips64)
	#include "::nonport.h"
#else
	#include "../nonport.h"
#endif

#include "gutil.h"



/*****************************************************************************/
/* Various encryption / encoding routines */

static void swap_byte ( uchar *a, uchar *b )
{
	uchar swapByte; 
	
	swapByte = *a; 
	*a = *b;      
	*b = swapByte;
}

static uchar encode_ct ( uchar c )
{
	if (c <  26) return ('A'+c);
	if (c <  52) return ('a'+c-26);
	if (c <  62) return ('0'+c-52);
	if (c == 62) return ('+');
	if (c == 63) return ('/');
	
	return 0;
}

void cengine_gs_encode ( uchar *ins, int size, uchar *result )
{
	int    i,pos;
	uchar  trip[3];
	uchar  kwart[4];
	
	i=0;
	while (i < size)
	{
		for (pos=0 ; pos <= 2 ; pos++, i++)
			if (i < size) trip[pos] = *ins++;
			else trip[pos] = '\0';
			kwart[0] =   (trip[0])       >> 2;
			kwart[1] = (((trip[0]) &  3) << 4) + ((trip[1]) >> 4);
			kwart[2] = (((trip[1]) & 15) << 2) + ((trip[2]) >> 6);
			kwart[3] =   (trip[2]) & 63;
			for (pos=0; pos <= 3; pos++) *result++ = encode_ct(kwart[pos]);
	}
	*result='\0';
}

void cengine_gs_encrypt ( uchar *key, int key_len, uchar *buffer_ptr, int buffer_len )
{ 
	short counter;     
	uchar x, y, xorIndex;
	uchar state[256];       
	
	for ( counter = 0; counter < 256; counter++) state[counter] = (uchar) counter;
	
	x = 0; y = 0;
	for ( counter = 0; counter < 256; counter++)
	{
		y = (key[x] + state[counter] + y) % 256;
		x = (x + 1) % key_len;
		swap_byte ( &state[counter], &state[y] );
	}
	
	x = 0; y = 0;
	for ( counter = 0; counter < buffer_len; counter ++)
	{
		x = (x + buffer_ptr[counter] + 1) % 256;
		y = (state[x] + y) % 256;
		swap_byte ( &state[x], &state[y] );
		xorIndex = (state[x] + state[y]) % 256;
		buffer_ptr[counter] ^= state[xorIndex];
	}
}



#define ROT8(x)  (((x) << 8)  | ((x) >>  24))
#define ROT24(x) (((x) << 24) | ((x) >>  8))


#define f(x, y, z, F) ( \
  y = ROT24(y),         \
  x ^= F[x & 0xFF],     \
  y ^= F[y & 0xFF],     \
  y = ROT24(y),         \
  x = ROT8(x),          \
  x ^= F[x & 0xFF],     \
  y ^= F[y & 0xFF],     \
  x = ROT8(x),          \
  z += z                \
)


#define g(x, y, z, F) ( \
  x = ~x,               \
  x = ROT24(x),         \
  x ^= F[x & 0xFF],     \
  y ^= F[y & 0xFF],     \
  x = ROT24(x),         \
  y = ROT8(y),          \
  x ^= F[x & 0xFF],     \
  y ^= F[y & 0xFF],     \
  y = ROT8(y),          \
  z += (z+1)            \
)   


#define d(x, y, z) (    \
  x += z,               \
  y += x,               \
  x += y                \
)


#define h(a, b) (a ^ b)


#define CRYPT_MIN_LEAF_NUM (1 << (CRYPT_HEIGHT))




goa_uint32
crypt_seek(crypt_key *L, unsigned int tree_num, unsigned int leaf_num) {
  int i;
  goa_uint32 x, y, z;

  i = 1 << (CRYPT_HEIGHT-1); 
  x = tree_num;
  y = 0;
  z = 1;

  L->index = 0;
  
  while (i > 0 ) {

    d(x, y, z);

    if (i & leaf_num) {

      g(x, y, z, L->F);  /* right */

    } else {

      L->x_stack[L->index] = x;
      L->y_stack[L->index] = y;
      L->z_stack[L->index] = z;
      L->index++;
      
      f(x, y, z, L->F);  /* left */
    }
    i >>= 1;
  }

  L->x = x; L->y = y; L->z = z;
  L->tree_num = tree_num;
  
  return h(x, y);

}


void
crypt_encrypt(crypt_key *L,  goa_uint32 *dest, int nodes) {
  int index;
  goa_uint32 x, y, z, *dest_max = dest + nodes;
  
  index = L->index;
  x = L->x;  y = L->y;  z = L->z;
  
  while (dest < dest_max) {
  
    while (z < CRYPT_MIN_LEAF_NUM) { 
      d(x, y, z);
      L->x_stack[index] = x;
      L->y_stack[index] = y;
      L->z_stack[index] = z;
      ++index;
      f(x, y, z, L->F);  

    }
    *dest++ = h(x, y);

    --index;
	if (index < 0)
		index = 0; //crt - fix problem
    x = L->x_stack[index];
    y = L->y_stack[index];
    z = L->z_stack[index];
    
    g(x, y, z, L->F);  

  }
  L->index = index;
  L->x = x;  L->y = y;  L->z = z;

}

void
init_crypt_key(const unsigned char *key,
		   unsigned int bytes_in_key, crypt_key *L) {
   int i, j, k, index;
   goa_uint32 tmp;
   goa_uint32 *F = L->F;
   L->keyptr = NULL;
   
   for (i=0; i<CRYPT_TABLE_SIZE; i++)
      F[i] = 0;

   
   for (j=0; j<4; j++) {
      for (i=0; i<CRYPT_TABLE_SIZE; i++)
         F[i] = F[i] * CRYPT_TABLE_SIZE + i;
      index = j;
      for (k=0; k<NUM_KEYSETUP_PASSES; k++) {
        for (i=0; i < CRYPT_TABLE_SIZE; i++) {
          index += (key[i % bytes_in_key] + F[i]);
          index &= (CRYPT_TABLE_SIZE-1);
          tmp = F[i];
          F[i] = F[index];
          F[index] = tmp;
       }
     }
   }
   for (i=0; i < CRYPT_TABLE_SIZE; i++)
     F[i] ^= i;
   crypt_seek(L, 0, 0);
}




void crypt_docrypt(crypt_key *L, unsigned char *data, int datalen)
{
	int i;
	for (i = 0 ; i < datalen ; i++)
	{
		if (L->keyptr == NULL || L->keyptr - (unsigned char *)L->keydata >= sizeof(L->keydata) - 1) //we deliberately skip some keys --- crt
		{
			L->keyptr = (unsigned char *)L->keydata;
			crypt_encrypt(L, L->keydata, NWORDS);
		}
		data[i] ^= *(L->keyptr);
		(L->keyptr)++;		
	}
}


/*****************************************************************************/