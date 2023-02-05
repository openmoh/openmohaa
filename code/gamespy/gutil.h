/******

GameSpy / GameMaster Utility Code
  
Copyright 1998 Critical Mass Communications, LLC.

Suite E-204
2900 Bristol Street
Costa Mesa, CA 92626
(714)549-7689
Fax(714)549-0757


******/

typedef unsigned char uchar;

/**********
gs_encode: This function converts a binary buffer to printable text.
It uses the base64 algorithm, which expands 3 byte pairs to 4 bytes.

IN
--
ins: a pointer to the buffer of data you want to encode
size: size of the buffer

OUT
---  
result: pointer to buffer to store result. Size should be (4 * size) / 3 + 1
	result will be null terminated.
**********/
void gs_encode(uchar* ins, int size, uchar* result);

/**********
gs_encrypt: This functions encypts a buffer (in place) with a given
key. The key is assumed to be a null terminated string.
NOTE: Encypting the buffer a second time with the same key will
decrypt it.


IN
--
buffer_ptr: buffer to be encrypted
buffer_len: size of the buffer
key: null terminated string containing the key to encode with.

OUT
---  
buffer_ptr: buffer, encrypted in place
**********/
void gs_encrypt(uchar* key, int key_len, uchar* buffer_ptr, int buffer_len);
