/*
===========================================================================
Copyright (C) 2012 su44

This file is part of md5_2_skX source code.

md5_2_skX source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

md5_2_skX source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with md5_2_skX source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "md5_2_skX.h"

// ===========================================================================

void T_Printf(const char *format, ...) {
	char	buffer[4096];
	va_list	argptr;

	va_start(argptr, format);
	vsprintf(buffer, format, argptr);
	va_end(argptr);

	printf(buffer);
}

void T_VerbPrintf(const char *format, ...) {
	char	buffer[4096];
	va_list	argptr;

	if(verbose == qfalse)
		return;

	va_start(argptr, format);
	vsprintf(buffer, format, argptr);
	va_end(argptr);

	T_Printf(buffer);
}

void T_Error(const char *format, ...) {
	char	buffer[4096];
	va_list	argptr;

	va_start(argptr, format);
	vsprintf(buffer, format, argptr);
	va_end(argptr);

	T_Printf("ERROR:");
	T_Printf(buffer);
	T_Printf("\n");
	system("pause");
	exit(-1);
}

// ===========================================================================

// su44: this is only here so functions used in q_shared.c and q_math.c can link
void QDECL Com_Error ( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	T_Error( "%s", text);
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	T_Printf ("%s", text);
}

// ===========================================================================

int F_LoadBuf(const char *fname, byte **out) {
	FILE *f;
	int len;
	byte *buf;

	f = fopen(fname,"rb");

	if(f == 0) {
		*out = 0;
		return -1; // failed
	}

	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);

	buf = malloc(len+1);
	fread(buf,1,len,f);
	// ensure that there is a trailing zero for text files
	buf[len] = 0; 

	*out = buf;

	// done.
	fclose(f);
	return len;
}

void F_FreeBuf(byte *b) {
	free(b);
}

void MakeDirSafe(const char *fpath) {
	char buf[MAX_TOOLPATH];
	char fixed[MAX_TOOLPATH];
	const char *p;
	// first fix slashes
	strcpy(fixed,fpath);
	backSlashesToSlashes(fixed);

	// then ensure that all needed directories are created
	p = fixed;
	while(*p) {
		if(*p == '/') {
			memcpy(buf,fixed,p - fixed);
			buf[p-fixed] = 0;
			_mkdir(buf);
		}
		p++;
	}
}

FILE *F_Open(const char *fname, const char *mode) {
	FILE *r;
	MakeDirSafe(fname);
	r = fopen(fname,mode);
	return r;
}

qboolean F_Exists(const char *fname) {
	FILE *f;

	f = fopen(fname,"rb");

	if(f == 0)
		return qfalse;

	fclose(f);
	return qtrue;
}

// ===========================================================================

void stripExt(char *s) {
	int l;
	char *p;

	l = strlen(s);

	p = s + l - 1;
	while(p != s) {
		if(*p == '.') {
			*p = 0;
			return;
		}
		if(*p == '/' || *p == '\\')
			return;
		p--;
	}
}
const char *strchr_r(const char *s, char c, char c2) {
	const char *p = s + strlen(s);
	while(p != s) {
		if(*p == c || *p == c2)
			return p;
		p--;
	}
	return 0;
}
// this will change all '\' to '/'
void backSlashesToSlashes(char *s) {
	while(*s) {
		if(*s == '\\') {
			*s = '/';
		}
		s++;
	}
}
const char *getGamePath(const char *s) {
	char buf[MAX_TOOLPATH];
	const char *p;

	strcpy(buf,s);
	backSlashesToSlashes(buf);
	p = buf;

	while(*p) {
		// this will also work with expansion packs "maintt" (BT) and "mainta" (SH) directories
		if(!Q_stricmpn(p,"/main",strlen("/main"))) {
			const char *r;
			r = strchr(p+1,'/');
			r++;
			return r;
		}
		p++;
	}

	// if it fail, lets assume that "models" directory residents in "main"
	p = buf;

	while(*p) {
		if(!Q_stricmpn(p,"/models/",strlen("/models/"))) {
			const char *r;
			r = p;
			r++;
			return r;
		}
		p++;
	}

	return 0;
}

const char *getFName(const char *s) {
	const char *p;

	p = strchr_r(s,'/','\\');

	if(p == 0)
		return s;

	p++;

	return p;
}

// ===========================================================================

void *T_Malloc(unsigned int size) {
	void *r = malloc(size);
	memset(r,0,size);
	return r;
}

// ===========================================================================

void PrintBoneOr(bone_t *b) {
	//vec3_t a;
	//QuatToAngles(b->q,a);
	//T_Printf("pos %f %f %f, quat %f %f %f %f, angles %f %f %f\n",
	//	b->p[0],);
}

void InverseBoneQuats(bone_t *bones, int numBones) {
	int i;
	for(i = 0; i < numBones; i++) {
		QuatInverse(bones[i].q);
	}
}
