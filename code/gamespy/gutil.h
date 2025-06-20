/******
gutil.h
GameSpy C Engine SDK
  
Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com

******

 Please see the GameSpy C Engine SDK documentation for more 
 information

******/

typedef unsigned char uchar;


void cengine_gs_encode ( uchar *ins, int size, uchar *result );
void cengine_gs_encrypt ( uchar *key, int key_len, uchar *buffer_ptr, int buffer_len );


		  
#define CRYPT_HEIGHT 16   
#define CRYPT_TABLE_SIZE       256 
#define NUM_KEYSETUP_PASSES 2 
#define NWORDS 16

typedef struct {
  goa_uint32 F[256];          
  goa_uint32 x_stack[CRYPT_HEIGHT];
  goa_uint32 y_stack[CRYPT_HEIGHT];
  goa_uint32 z_stack[CRYPT_HEIGHT];
  int index;      
  goa_uint32 x, y, z;
  goa_uint32 tree_num;
  goa_uint32 keydata[NWORDS];
 unsigned char *keyptr;

} crypt_key;

void
init_crypt_key(const unsigned char *key,
		   unsigned int bytes_in_key,
		   crypt_key *L);

void
crypt_encrypt(crypt_key *L, goa_uint32 *dest, int nodes);
void crypt_docrypt(crypt_key *L, unsigned char *data, int datalen);