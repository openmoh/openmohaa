/******

GameSpy / GameMaster Utility Code
  
Copyright 1998 Critical Mass Communications, LLC.

Suite E-204
2900 Bristol Street
Costa Mesa, CA 92626
(714)549-7689
Fax(714)549-0757

see gsutils.h for documentation

******/

typedef unsigned char uchar;

static void swap_byte(uchar* a, uchar* b)
{
	uchar swapByte;

	swapByte = *a;
	*a = *b;
	*b = swapByte;
}

static uchar encode_ct(uchar c)
{
	if (c < 26) return (uchar)('A' + c);
	if (c < 52) return (uchar)('a' + c - 26);
	if (c < 62) return (uchar)('0' + c - 52);
	if (c == 62) return (uchar)('+');
	if (c == 63) return (uchar)('/');

	return 0;
}

void gs_encode(uchar* ins, int size, uchar* result)
{
	int    i, pos;
	uchar  trip[3];
	uchar  kwart[4];

	i = 0;
	while (i < size)
	{
		for (pos = 0; pos <= 2; pos++, i++)
			if (i < size) trip[pos] = *ins++;
			else trip[pos] = '\0';
		kwart[0] = (unsigned char)((trip[0]) >> 2);
		kwart[1] = (unsigned char)((((trip[0]) & 3) << 4) + ((trip[1]) >> 4));
		kwart[2] = (unsigned char)((((trip[1]) & 15) << 2) + ((trip[2]) >> 6));
		kwart[3] = (unsigned char)((trip[2]) & 63);
		for (pos = 0; pos <= 3; pos++) *result++ = encode_ct(kwart[pos]);
	}
	*result = '\0';
}

void gs_encrypt(uchar* key, int key_len, uchar* buffer_ptr, int buffer_len)
{
	int counter;
	uchar x, y, xorIndex;
	uchar state[256];

	for (counter = 0; counter < 256; counter++) state[counter] = (uchar)counter;

	x = 0; y = 0;
	for (counter = 0; counter < 256; counter++)
	{
		y = (uchar)((key[x] + state[counter] + y) % 256);
		x = (uchar)((x + 1) % key_len);
		swap_byte(&state[counter], &state[y]);
	}

	x = 0; y = 0;
	for (counter = 0; counter < buffer_len; counter++)
	{
		x = (uchar)((x + buffer_ptr[counter] + 1) % 256);
		y = (uchar)((state[x] + y) % 256);
		swap_byte(&state[x], &state[y]);
		xorIndex = (uchar)((state[x] + state[y]) % 256);
		buffer_ptr[counter] ^= state[xorIndex];
	}
}
