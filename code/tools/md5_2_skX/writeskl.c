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

// writeskl.c - .skl model export

// NOTE: ".skl" files exported by this code are now 
// compatible with skl_2_skX tool from Spearhead SDK.

#include "md5_2_skX.h"

int getTotalVertCount(tModel_t *m) {
	int i, r;

	r = 0;

	for(i = 0; i < m->numSurfaces; i++) {
		r += m->surfs[i].numVerts;
	}

	return r;
}


int getTotalTriCount(tModel_t *m) {
	int i, r;

	r = 0;

	for(i = 0; i < m->numSurfaces; i++) {
		r += m->surfs[i].numTris;
	}

	return r;
}

void writeSKL(tModel_t *m, tAnim_t *a, const char *outFName) {
	FILE *out;
	int i, j, k, v, t, addVerts;

	out = F_Open(outFName,"wb");

	fprintf(out,"//////////////////////////////////////////////////////////////////////////\n");
	fprintf(out,"//\n");
	fprintf(out,"// Exported by md5_2_skX\n");
	fprintf(out,"//\n");
	fprintf(out,"//////////////////////////////////////////////////////////////////////////\n");
	fprintf(out,"\n");

	fprintf(out,"SKELETON\n");
	fprintf(out,"VERSION 1\n");

	fprintf(out,"\n");
	fprintf(out,"NUMMATERIALS %i\n",m->numSurfaces);	

	for(i = 0; i < m->numSurfaces; i++) {
		fprintf(out,"MATERIAL %i %s\n",i,m->surfs[i].name);
	}

	fprintf(out,"\n");
	fprintf(out,"NUMBONES %i\n",m->numBones);

	for(i = 0; i < m->numBones; i++) {
		fprintf(out,"BONE %i %i %s\n",i,m->bones[i].parent,m->bones[i].name);	
	}

	fprintf(out,"NUMVERTS %i\n",getTotalVertCount(m));
	v = 0;
	for(i = 0; i < m->numSurfaces; i++) {
		tVert_t *vt;
		tSurf_t *sf;
		
		sf = m->surfs + i;
		vt = sf->verts;
		for(j = 0; j < sf->numVerts; j++,v++,vt++) {
			fprintf(out,"VERT %i\n",v);
			fprintf(out,"NORMAL %f %f %f\n",vt->normal[0],vt->normal[1],vt->normal[2]);
			//fprintf(out,"NORMAL 1 0 0\n");
			fprintf(out,"BONES %i\n",vt->numWeights);
			for(k = 0; k < vt->numWeights; k++) {
				fprintf(out,"BONE %i %f %f %f %f\n",vt->weights[k].boneNum,vt->weights[k].boneWeight,
					vt->weights[k].offset[0],vt->weights[k].offset[1],vt->weights[k].offset[2]);
			}
			fprintf(out,"\n");
		}
	}	

	fprintf(out,"\n");
	fprintf(out,"NUMFACES %i\n",getTotalTriCount(m));
	
	t = 0;
	addVerts = 0;
	for(i = 0; i < m->numSurfaces; i++) {
		tTri_t *tt;
		tSurf_t *sf;
		
		sf = m->surfs + i;
		tt = sf->tris;
		for(j = 0; j < sf->numTris; j++,t++,tt++) {
			int v0, v1, v2;
			tVert_t *vp0, *vp1, *vp2;
			
			v0 = tt->indexes[0] + addVerts;
			v1 = tt->indexes[1] + addVerts;			
			v2 = tt->indexes[2] + addVerts;

			vp0 = sf->verts + tt->indexes[0];
			vp1 = sf->verts + tt->indexes[1];
			vp2 = sf->verts + tt->indexes[2];

			// TRI <surface/material index> 
			fprintf(out,"TRI %i",i);
			fprintf(out," %i %f %f", v0, vp0->texCoords[0], vp0->texCoords[1]);
			fprintf(out," %i %f %f", v1, vp1->texCoords[0], vp1->texCoords[1]);
			fprintf(out," %i %f %f", v2, vp2->texCoords[0], vp2->texCoords[1]);
			fprintf(out,"\n");

		}
		addVerts += sf->numVerts;
	}	


	fprintf(out,"\n");
	fprintf(out,"FRAMERATE %f\n",a->frameRate);
	fprintf(out,"NUMFRAMES %i\n",a->numFrames);
	fprintf(out,"\n");
	for(i = 0; i < a->numFrames; i++) {
		tFrame_t *f;
		bone_t *bones, *b;
		
		bones = b = setupMD5AnimBones(a,i);
		md5AnimateBones(m,bones); 
		f = a->frames + i;
		fprintf(out,"\nFRAME %i\n",i);
		for(j = 0; j < a->numBones; j++,b++) {
			vec3_t axis[3];

			fprintf(out,"BONE %i\n",j);
			fprintf(out,"OFFSET %f %f %f\n",
				b->p[0],b->p[1],b->p[2]);

			QuatToAxis(b->q,axis);

			fprintf(out,"X %f %f %f\n",axis[0][0],axis[0][1],axis[0][2]);
			fprintf(out,"Y %f %f %f\n",axis[1][0],axis[1][1],axis[1][2]);
			fprintf(out,"Z %f %f %f\n",axis[2][0],axis[2][1],axis[2][2]);
			

			fprintf(out,"\n");
			
		}
	}


	fprintf(out,"\nNUMBOXES 0\n");
	fprintf(out,"\n");
	fprintf(out,"END\n");
	fclose(out);

	T_Printf("Wrote %s\n",outFName);
}


