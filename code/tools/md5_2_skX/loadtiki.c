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

// loadtiki.c - loading MoHAA .tik, .skc and .skd files.

#include "md5_2_skX.h"

int getBoneIndex(tModel_t *m, const char *s) {
	int i;

	if(!Q_stricmp(s,"worldbone")) {
		return -1;
	}
	for(i = 0; i < m->numBones; i++) {
		if(!Q_stricmp(m->bones[i].name,s)) {
			return i;
		}
	}
	T_Printf("Warning: cant find bone %s in %s\n",s,m->fname);
	return -1;
}
tModel_t *readSKD(const char *fname, float scale) {
	int len;
	skdHeader_t *h;
	skdSurface_t *sf;
	skdBone_t *b;
	tBone_t *ob;
	tModel_t *out;
	tSurf_t *os;
	int i, j, k;

	T_Printf("Loading MoHAA skd model file %s...\n",fname);

	len = F_LoadBuf(fname,(byte**)&h);

	if(len == -1) {
		T_Printf("readSKD: Cannot open %s\n",fname);
		return 0;
	}

	out = T_Malloc(sizeof(tModel_t));

	strcpy(out->fname,fname);

	out->numSurfaces = h->numSurfaces;
	out->numBones = h->numBones;
	out->surfs = T_Malloc(sizeof(tSurf_t)*out->numSurfaces);
	out->bones = T_Malloc(sizeof(tBone_t)*out->numBones);

	// load bones

	b = (skdBone_t *) ( (byte *)h + h->ofsBones );
	ob = out->bones;
	for ( i = 0; i < h->numBones; i++, ob++) {
		strcpy(ob->name,b->name);
		if(b->jointType != 1) {
			T_Error("readSKD: joints types other than 1 are not supported yet (skd file %s)\n",fname);
		}
		b = (skdBone_t *)( (byte *)b + b->ofsEnd );
	}		

	b = (skdBone_t *) ( (byte *)h + h->ofsBones );
	ob = out->bones;
	for ( i = 0; i < h->numBones; i++, ob++) {
		ob->parent = getBoneIndex(out,b->parent);
		b = (skdBone_t *)( (byte *)b + b->ofsEnd );
	}		

	// load surfaces
	sf = (skdSurface_t *) ( (byte *)h + h->ofsSurfaces );
	os = out->surfs;
	for ( i = 0; i < h->numSurfaces; i++,os++) {
		skdVertex_t *v;
		skdTriangle_t *t;
		tVert_t *ov;

		strcpy(os->name,sf->name);
		os->numTris = sf->numTriangles;
		os->numVerts = sf->numVerts;
		os->tris = T_Malloc(sizeof(tTri_t)*sf->numTriangles);
		os->verts = T_Malloc(sizeof(tVert_t)*sf->numVerts);

		// copy triangles
		t = (skdTriangle_t *) ( (byte *)sf + sf->ofsTriangles );
		assert(sizeof(skdTriangle_t) == sizeof(tTri_t));
		memcpy(os->tris,t,sizeof(skdTriangle_t)*sf->numTriangles);

		// copy and scale vertices data
		v = (skdVertex_t *) ( (byte *)sf + sf->ofsVerts );
		ov = os->verts;
		for ( j = 0; j < sf->numVerts; j++,ov++) {
			skdWeight_t *w;
			tWeight_t *ow;
	
			ov->numWeights = v->numWeights;
			VectorCopy(v->normal,ov->normal);
			ov->texCoords[0] = v->texCoords[0];
			ov->texCoords[1] = v->texCoords[1];
			ov->weights = T_Malloc(sizeof(tWeight_t)*ov->numWeights);

			w = (skdWeight_t *) ( (byte *)v + sizeof(skdVertex_t)+(sizeof(skdMorph_t)*v->numMorphs));
			ow = ov->weights;
			for ( k = 0; k < v->numWeights; k++,ow++) {
				VectorScale(w->offset,scale,ow->offset);
				if(w->boneWeight<0 || w->boneWeight>1) {
					T_Error("invalid bone weight %f in %s \n",w->boneWeight,out->fname);
				}
				ow->boneWeight = w->boneWeight;
				ow->boneNum = w->boneIndex;
				w = (skdWeight_t *) ( (byte *)w + sizeof(skdWeight_t));
			}
			v = (skdVertex_t *) ( (byte *)v + sizeof(skdVertex_t) + v->numWeights*sizeof(skdWeight_t) + v->numMorphs*sizeof(skdMorph_t) );
		}
		sf = (skdSurface_t *)( (byte *)sf + sf->ofsEnd );
	}

	F_FreeBuf((byte*)h);

	T_Printf("Succesfully loaded MoHAA model %s\n",fname);

	return out;
}
int getChannelIndexInternal(skcHeader_t *h, const char *channelName) {
	const char *c;
	int i;

	c = ( (byte *)h + h->ofsChannels );
	for(i = 0; i < h->numChannels; i++) {
		if(!Q_stricmp(c,channelName)) 
			return i;
		c += SKC_MAX_CHANNEL_CHARS;
	}

	return -1;
}
static vec4_t vecBuf[512];
static int vecIndex = 0;
float *getChannelValue(skcHeader_t *h, const char *name, int frameNum) {

	int channelIndex;
	skcFrame_t *f;
	float *values;

	channelIndex = getChannelIndexInternal(h,name);

	if(channelIndex == -1)
		return 0;

	f = (skcFrame_t *)( (byte *)h + sizeof(*h) + sizeof(*f) * frameNum );

	values = (float*)((byte *)h+f->ofsValues);

	values += (4 * channelIndex);

	vecIndex++;
	if(vecIndex >= sizeof(vecBuf) / sizeof(vecBuf[0])) {
		vecIndex = 0;
	}

	QuatCopy(values, vecBuf[vecIndex]);
	return vecBuf[vecIndex];
}
float *findPosChannel(skcHeader_t *h, const char *name, int frameNum) {
	char channelName[32];
	strcpy(channelName,name);
	strcat(channelName," pos");
	return getChannelValue(h,channelName,frameNum);
}
float *findRotChannel_raw(skcHeader_t *h, const char *name, int frameNum) {
	char channelName[32];
	strcpy(channelName,name);
	strcat(channelName," rot");
	return getChannelValue(h,channelName,frameNum);
}
float *findRotChannel(skcHeader_t *h, const char *name, int frameNum, int parentIndex) {
#if 0
	char channelName[32];
	strcpy(channelName,name);
	strcat(channelName," rot");
	return getChannelValue(h,channelName,frameNum);
#else
static int i = 0;
	static quat_t qs[1024];
	float *q;
	char channelName[32];
	float *f;
	float len;
	
	i++;
	i %= 1024;
	q = qs[i];

	strcpy(channelName,name);
	strcat(channelName," rot");
	f = getChannelValue(h,channelName,frameNum);
	if(f == 0) {
		//return quat_identity;
		QuatSet(q,0,0,0,-1);
	} else {
		QuatCopy(f,q);
	}
	//QuatInverse(q);
	////if(parentIndex == -1)
	//	QuatInverse(q);
	len = QuatNormalize(q);
	if(abs(len-1.f) > 0.1) {
		T_Error("Non-normalized quat in skc file (%f)\n",len);
	}
#if 1
	FixQuatForMD5_P(q);
#endif
	return q;
#endif
}
tAnim_t *appendSKC(tModel_t *m, const char *fname, float scale) {
	int len;
	skcHeader_t *h;
	skcFrame_t *f; //, *firstFrame;
	tAnim_t *out;
	tFrame_t *of;
//	const char *c;
	int i, j;
	int cFlags[512];
	bone_t baseFrame[512];
	int numAnimatedComponents;

	T_Printf("Loading MoHAA skc animation file %s...\n",fname);

	len = F_LoadBuf(fname,(byte**)&h);

	if(len == -1) {
		T_Printf("Cannot open %s\n",fname);
		return 0;
	}

	memset(cFlags,0,sizeof(cFlags));

	out = T_Malloc(sizeof(tAnim_t));
	out->frameRate = 1.f / h->frameTime;
	out->numBones = m->numBones;
	out->numFrames = h->numFrames;
	out->frames = T_Malloc(sizeof(tFrame_t)*h->numFrames);
	out->boneData = T_Malloc(sizeof(tAnimBone_t)*m->numBones);

	// copy frame bounding boxes
	f = (skcFrame_t *)( (byte *)h + sizeof(*h) );
	of = out->frames;
	for(i = 0; i < h->numFrames; i++,of++,f++) {
		//anim->frames[i].radius = f->radius;
		VectorCopy(f->bounds[1],of->maxs);
		VectorCopy(f->bounds[0],of->mins);
	}

	// detect which components changes
	for(j = 0; j < m->numBones; j++) {
		float *baseRot, *testRot;
		float *basePos, *testPos;

		basePos = findPosChannel(h,m->bones[j].name,0);
		if(basePos == 0) {
			VectorSet(baseFrame[j].p,0,0,0);
		} else {
			VectorScale(basePos,scale,basePos);
			VectorCopy(basePos,baseFrame[j].p);
			for(i = 1; i < h->numFrames; i++) {
				testPos = findPosChannel(h,m->bones[j].name,i);
				VectorScale(testPos,scale,testPos);
				// detect X change
				if(testPos[0] != basePos[0]) {
					cFlags[j] |= COMPONENT_BIT_TX;
				}
				// detect Y change
				if(testPos[1] != basePos[1]) {
					cFlags[j] |= COMPONENT_BIT_TY;
				}
				// detect Z change
				if(testPos[2] != basePos[2]) {
					cFlags[j] |= COMPONENT_BIT_TZ;
				}
			}	
		}

		baseRot = findRotChannel(h,m->bones[j].name,0,m->bones[j].parent);
		if(baseRot == 0) {
			QuatSet(baseFrame[j].q,0,0,0,-1);
		} else {
			QuatCopy(baseRot,baseFrame[j].q);
			for(i = 1; i < h->numFrames; i++) {
				testRot = findRotChannel(h,m->bones[j].name,i,m->bones[j].parent);
				// detect X change
				if(testRot[0] != baseRot[0]) {
					cFlags[j] |= COMPONENT_BIT_QX;
				}
				// detect Y change
				if(testRot[1] != baseRot[1]) {
					cFlags[j] |= COMPONENT_BIT_QY;
				}
				// detect Z change
				if(testRot[2] != baseRot[2]) {
					cFlags[j] |= COMPONENT_BIT_QZ;
				}
				// NOTE: quaternion W component is not stored at all in md5 files
			}	
		}
	}

	// count the number of animated components and copy some bone data
	numAnimatedComponents = 0;
	for(j = 0; j < m->numBones; j++) {
		//int c;

		out->boneData[j].firstComponent = numAnimatedComponents;
		//c = 0;

		for(i = 0; i < 6; i++) {
			if(cFlags[j] & (1 << i)) {
				numAnimatedComponents++;
			//	c++;
			}
		}

		//out->boneData[j].numAnimatedComponents = c;
		out->boneData[j].componentBits = cFlags[j];
		strcpy(out->boneData[j].name,m->bones[j].name);
		out->boneData[j].parent = m->bones[j].parent;
	}

	// copy results out
	out->baseFrame = T_Malloc(sizeof(bone_t)*m->numBones);
	memcpy(out->baseFrame,baseFrame,sizeof(bone_t)*m->numBones);
	out->numAnimatedComponents = numAnimatedComponents;
	of = out->frames;
	for(i = 0; i < h->numFrames; i++,of++) {
		int c;
		float *cp;

		cp = of->components = T_Malloc(numAnimatedComponents*sizeof(float));

		//c = 0;
		for(j = 0; j < m->numBones; j++) {
			float *pos, *rot;

			pos = findPosChannel(h,m->bones[j].name,i);
			if(pos) {
				VectorScale(pos,scale,pos);
				// write X change
				if(cFlags[j] & COMPONENT_BIT_TX) {
					*cp = pos[0];
					cp++;
				}
				// write Y change
				if(cFlags[j] & COMPONENT_BIT_TY) {
					*cp = pos[1];
					cp++;
				}
				// write Z change
				if(cFlags[j] & COMPONENT_BIT_TZ) {
					*cp = pos[2];
					cp++;
				}
			}

			rot = findRotChannel(h,m->bones[j].name,i,m->bones[j].parent);
			if(rot) {
				// write X change
				if(cFlags[j] & COMPONENT_BIT_QX) {
					*cp = rot[0];
					cp++;
				}
				// write Y change
				if(cFlags[j] & COMPONENT_BIT_QY) {
					*cp = rot[1];
					cp++;
				}
				// write Z change
				if(cFlags[j] & COMPONENT_BIT_QZ) {
					*cp = rot[2];
					cp++;
				}	
			}
		}

		c = cp - of->components;
		assert(c == numAnimatedComponents);
	}

#if 0
	// validate generated tAnim_t components
	for(i = 0; i < out->numFrames; i++) {
		bone_t *b = setupMD5AnimBones(out,0); 
		for(j = 0; j < m->numBones; j++, b++) {
			float *o;
			o = findRotChannel_raw(h,m->bones[j].name,i,m->bones[j].parent);
			if(o) {
				T_Printf("Generated: %f %f %f %f, original %f %f %f %f\n",b->q[0],b->q[1],b->q[2],b->q[3],
					o[0],o[1],o[2],o[3]);
			} else {
				T_Printf("Generated: %f %f %f %f, original <none>\n",b->q[0],b->q[1],b->q[2],b->q[3]);
			}

			
		}
	}

#endif

	// generate baseFrame, but only once,
	// from the first appended SKC
	if(m->baseFrame == 0) {
#if 0
		// FIXME!
		bone_t *b = setupMD5AnimBones(out,0); 
		//for(i = 0; i < m->numBones; i++) {
		//	QuatInverse(b[i].q);
		//}
#else
		bone_t b[512];
		for(i = 0; i < m->numBones; i++) {
			float *p, *q;
			
			p = findPosChannel(h,m->bones[i].name,0);
			if(p) {
				VectorScale(p,scale,b[i].p);
			} else {
				VectorSet(b[i].p,0,0,0);
			}

			q = findRotChannel(h,m->bones[i].name,0,m->bones[i].parent);
			if(q) {
				QuatCopy(q,b[i].q);
			} else {
				QuatSet(b[i].q,0,0,0,1);
			}
		}
#endif
		md5AnimateBones(m,b);
		m->baseFrame = T_Malloc(m->numBones*sizeof(bone_t));
		memcpy(m->baseFrame,b,m->numBones*sizeof(bone_t));
	}

	F_FreeBuf((byte*)h);

	T_Printf("Succesfully loaded MoHAA animation %s\n",fname);


	return out;
}

// TIKI loading.
// For the purpose of this exporter,
// we need to load setup and animation section.
// Include keywords are currently ignored.

const char *fixPath(const char *fname, const char *path, const char *tikiFilePath) {
	static char tmp[MAX_TOOLPATH];
	static char tmp2[MAX_TOOLPATH];
	const char *main, *p, *models;
	int l;

	if(F_Exists(fname)) {
		return fname;
	}
	strcpy(tmp,path);
	backSlashesToSlashes(tmp);
	if(tmp[strlen(tmp)-1] != '/') {
		strcat(tmp,"/");	
	}
	strcat(tmp,fname);
	if(F_Exists(fname)) {
		return tmp;
	}
	// try to extract path to MoHAA's main/mainta/maintt directory
	main = strstr(tikiFilePath,"main");
	if(main) {
		p = strchr(main,'/');
		l = (p-tikiFilePath)+1;
		memcpy(tmp2,tikiFilePath,l);
		tmp2[l] = 0;
		strcat(tmp2,tmp);
		if(F_Exists(tmp2)) {
			return tmp2;
		}
	}
	// if everything else fail, try to extract "models" path
	models = strstr(tikiFilePath,"models/");
	if(main) {
		l = (models-tikiFilePath)+strlen("models/");
		strncpy(tmp2,tikiFilePath,l);
		tmp2[l] = 0;
		p = strstr(tmp,"models/");
		if(p) {
			strcat(tmp2,p+strlen("models/"));
		} else {
			strcat(tmp2,tmp);
		}
		if(F_Exists(tmp2)) {
			return tmp2;
		}
	}
	return tmp;
}
void loadTIKI(const char *fname) {
	int len;
	char *txt;
	char *p;
	const char *fixedPath;
	const char *token;
	char path[MAX_TOOLPATH];
	float scale;

	len = F_LoadBuf(fname,(byte**)&txt);

	if(len == -1) {
		T_Error("loadTIKI: Cannot open %s\n",fname);
		return;
	}

	path[0] = 0;
	scale = 1.f;

	// NOTE: this will not open the "fname" file!
	COM_BeginParseSession(fname);

	p = txt;
	token = COM_ParseExt(&p, qtrue);
	while(token[0]) {
		if (!Q_stricmp(token, "path") || !Q_stricmp(token, "$path")) {
			token = COM_ParseExt(&p, qtrue);
			strcpy(path,token);
		} else if (!Q_stricmp(token, "scale")) {
			token = COM_ParseExt(&p, qtrue);
			scale = atof(token);
		} else if (!Q_stricmp(token, "skelmodel")) {
			token = COM_ParseExt(&p, qtrue);
			mainModel = readSKD(fixPath(token,path,fname),scale);
		} else if(strstr(token,".skc")) {
			tAnim_t *a;
			fixedPath = fixPath(token,path,fname);
			a = appendSKC(mainModel,fixedPath,scale);
			if(a) {
				strcpy(inAnimFNames[numAnims],fixedPath);
				anims[numAnims] = a;
				numAnims++;
			}
		}
		token = COM_ParseExt(&p, qtrue);
	}

	F_FreeBuf(txt);
}
























