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

// writeobj.c - Wavefront Obj exporting, not really needed,
// but used by me to debug absolute vertex positions in skeletal models

#include "md5_2_skX.h"

void WriteOBJ(const char *outFName, tModel_t *m, bone_t *bones) {
	FILE *out;
	int numVerts;
	int i, j;
	tSurf_t *sf;
	tTri_t *t;
	tVert_t *v;


	// calculate absolute vertex positions
	CalcModelXYZVerticesForBones(m,bones);

	T_Printf("Writing Wavefront obj file %s...\n",outFName);

	out = F_Open(outFName,"w");
	
	sf = m->surfs;
	for(i = 0; i < m->numSurfaces; i++, sf++) {
		v = sf->verts;
		for(j = 0; j < sf->numVerts; j++, v++) {
			fprintf(out,"v %f %f %f\n",v->absXYZ[0],v->absXYZ[1],v->absXYZ[2]);
			fprintf(out,"vt %f %f\n",v->texCoords[0],v->texCoords[1]);
		}
	}

	numVerts = 1; // 1, because in obj indexes start at 1, not 0
	sf = m->surfs;
	for(i = 0; i < m->numSurfaces; i++, sf++) {
		fprintf(out,"usemtl %s\n",sf->name);
		t = sf->tris;
		for(j = 0; j < sf->numTris; j++, t++) {
			fprintf(out,"f %i %i %i\n",numVerts + t->indexes[2],numVerts + t->indexes[1],numVerts + t->indexes[0]);
		}
		numVerts += sf->numVerts;
	}

	fclose(out);

	T_Printf("Wrote %s.\n",outFName);
}

void WriteOBJ_FromBaseFrame(const char *outFName, tModel_t *m) {
	WriteOBJ(outFName,m,m->baseFrame);
}

void WriteOBJ_FromAnim(const char *outFName, tModel_t *m, tAnim_t *a) {
	bone_t *bones;
	bones = setupMD5AnimBones(a,0); 
	md5AnimateBones(m,bones);
	WriteOBJ(outFName,m,bones);
}