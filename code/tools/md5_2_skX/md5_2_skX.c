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

qboolean verbose = qfalse;
qboolean noLimits = qfalse;
qboolean createTIK = qfalse;
qboolean createSKL = qfalse;
qboolean createOBJ = qfalse;
qboolean renameBones = qfalse;
// converting MoHAA models to Doom3 md5
qboolean decompile = qfalse;
char inMesh[MAX_TOOLPATH];
tModel_t *mainModel;
char outSKDMesh[MAX_TOOLPATH];
int numAnims = 0;
char inAnimFNames[MAX_TOOLPATH][256];
char outSKCAnim[MAX_TOOLPATH][256];
tAnim_t *anims[256];
char outTIKI[MAX_TOOLPATH];
// that's a path where skc/skd files should be put
char modelDataDir[MAX_TOOLPATH];


static float null = 0;
void ConvertAnimation(tAnim_t *a, const char *outFName) {
	int i,j;
	tFrame_t *f;
	skcHeader_t h;
	FILE *out;
	bone_t *b;
	tAnimBone_t *bd;
	char name[SKC_MAX_CHANNEL_CHARS];
	int ofsChannelNames;

	out = F_Open(outFName,"wb");
	if(out == 0) {
		T_Error("ConvertAnimation: Cannot open %s\n",outFName);
	}

	memset(&h,0,sizeof(h));
	h.ident = SKC_IDENT;
	h.version = SKC_VERSION;
	h.numFrames = a->numFrames;
	h.frameTime = 1.f / a->frameRate;
	h.numChannels = a->numBones * 2;

	fwrite(&h,sizeof(h),1,out);

	for(i = 0, f = a->frames; i < a->numFrames; i++, f++) {
		skcFrame_t outFrame;
		VectorCopy(f->mins,outFrame.bounds[0]);
		VectorCopy(f->maxs,outFrame.bounds[1]);
		outFrame.radius = RadiusFromBounds(f->mins,f->maxs);
		VectorSet(outFrame.delta,0,0,0);
		outFrame.unknown = 0;
		outFrame.ofsValues = sizeof(h) + sizeof(skcFrame_t) * a->numFrames 
			+ (a->numBones * sizeof(float)*8) * i;

		fwrite(&outFrame,sizeof(outFrame),1,out);
	}

	// write channel values
	for(i = 0, f = a->frames; i < a->numFrames; i++, f++) {
		bone_t *bones;

		bones = setupMD5AnimBones(a,i);

		for(j = 0, b = bones; j < a->numBones; j++, b++) {
			// first quat
			fwrite(&b->q[0],sizeof(quat_t),1,out);
			// then pos
			fwrite(&b->p[0],sizeof(vec3_t),1,out);
			fwrite(&null,sizeof(int),1,out);
		}
	}
	ofsChannelNames = ftell(out);
	// write channel names
	for(i = 0, bd = a->boneData; i < a->numBones; i++, bd++) {
		Q_strncpyz(name,bd->name,SKC_MAX_CHANNEL_CHARS-4);
		strcat(name," rot");
		fwrite(name,SKC_MAX_CHANNEL_CHARS,1,out);
		Q_strncpyz(name,bd->name,SKC_MAX_CHANNEL_CHARS-4);
		strcat(name," pos");
		fwrite(name,SKC_MAX_CHANNEL_CHARS,1,out);

	}

	// reupdate the header
	h.ofsEnd = ftell(out);
	h.ofsChannels = ofsChannelNames;
	fseek(out,0,SEEK_SET);
	fwrite(&h,sizeof(h),1,out);
	if(ftell(out) != sizeof(h)) {
		T_Error("Fatal file write error\n");
	}
	fclose(out);
	T_Printf("Wrote MoHAA animation %s\n",outFName);
}
void CalcModelBB(tModel_t *m, bone_t *bones, vec3_t outMins, vec3_t outMaxs, float *outRadius) {
	int i, j;
	tSurf_t *sf;
	tVert_t *v;
	float rad, l;

	ClearBounds(outMins,outMaxs);

	// calculate absolute vertex positions for given bones
	CalcModelXYZVerticesForBones(m,bones);

	rad = 0;
	sf = m->surfs;
	for(i = 0; i < m->numSurfaces; i++, sf++) {
		v = sf->verts;
		for(j = 0; j < sf->numVerts; j++, v++) {
			AddPointToBounds(v->absXYZ,outMins,outMaxs);
			l = VectorLengthSquared(v->absXYZ);
			if(l > rad) {
				rad = l;
			}
		}
	}
	*outRadius = sqrt(rad);
}
void CalcModelBBFromBaseframe(tModel_t *m) {
	CalcModelBB(m,m->baseFrame,m->baseFrameMins,m->baseFrameMaxs,&m->baseFrameRadius);
}
void ConvertModelBaseFrameToAnim(tModel_t *m, const char *outFName) {
	int i,j;
	skcHeader_t h;
	FILE *out;
	bone_t *b;
	tBone_t *bd;
	char name[SKC_MAX_CHANNEL_CHARS];
	int ofsChannelNames;

	out = F_Open(outFName,"wb");
	if(out == 0) {
		T_Error("ConvertModelBaseFrameToAnim: Cannot open %s\n",outFName);
	}

	CalcModelBBFromBaseframe(m);

	memset(&h,0,sizeof(h));
	h.ident = SKC_IDENT;
	h.version = SKC_VERSION;
	h.numFrames = 1;
	h.frameTime = 0.033333335f;
	h.numChannels = m->numBones * 2;

	fwrite(&h,sizeof(h),1,out);

	// write single frame
	{
		skcFrame_t outFrame;
		VectorCopy(m->baseFrameMins,outFrame.bounds[0]);
		VectorCopy(m->baseFrameMaxs,outFrame.bounds[1]);
		outFrame.radius = RadiusFromBounds(outFrame.bounds[0],outFrame.bounds[1]);
		VectorSet(outFrame.delta,0,0,0);
		outFrame.unknown = 0;
		outFrame.ofsValues = sizeof(h) + sizeof(skcFrame_t) * /*a->numFrames */ 1
			+ (m->numBones * sizeof(float)*8) * 0/*i*/;

		fwrite(&outFrame,sizeof(outFrame),1,out);
	}
	// done.

	// write channel values
	{
		bone_t *bones;

		bones = setupMD5MeshBones(m);

		for(j = 0, b = bones; j < m->numBones; j++, b++) {
			// first quat
			fwrite(&b->q[0],sizeof(quat_t),1,out);
			// then pos
			fwrite(&b->p[0],sizeof(vec3_t),1,out);
			fwrite(&null,sizeof(int),1,out);
		}
	}
	ofsChannelNames = ftell(out);
	// write channel names
	for(i = 0, bd = m->bones; i < m->numBones; i++, bd++) {
		Q_strncpyz(name,bd->name,SKC_MAX_CHANNEL_CHARS-4);
		strcat(name," rot");
		fwrite(name,SKC_MAX_CHANNEL_CHARS,1,out);
		Q_strncpyz(name,bd->name,SKC_MAX_CHANNEL_CHARS-4);
		strcat(name," pos");
		fwrite(name,SKC_MAX_CHANNEL_CHARS,1,out);

	}

	// reupdate the header
	h.ofsEnd = ftell(out);
	h.ofsChannels = ofsChannelNames;
	fseek(out,0,SEEK_SET);
	fwrite(&h,sizeof(h),1,out);
	if(ftell(out) != sizeof(h)) {
		T_Error("Fatal file write error\n");
	}
	fclose(out);
	T_Printf("Wrote MoHAA animation %s\n",outFName);
}
// all bones should be obviously in model space
void CalcModelXYZVerticesForBones(tModel_t *m, bone_t *bones) {
	int i, j, k;
	tSurf_t *sf;
	tVert_t *v;
	tWeight_t *w;
	
	sf = m->surfs;
	for(i = 0; i < m->numSurfaces; i++, sf++) {
		v = sf->verts;
		for(j = 0; j < sf->numVerts; j++, v++) {
			VectorClear(v->absXYZ);
			w = v->weights;
			for(k = 0; k < v->numWeights; k++, w++) {
				quat_t qua, que,res;
				quat_t boneQuat;
//			vec3_t newnormal;

				QuatCopy(bones[w->boneNum].q,boneQuat);

				qua[0] = w->offset[0];
				qua[1] = w->offset[1];
				qua[2] = w->offset[2];
				qua[3] = 0;
				QuaternionMultiply(que,qua,boneQuat);
				QuatInverse(boneQuat);
				QuaternionMultiply(res,boneQuat,que);
				QuatInverse(boneQuat);
				v->absXYZ[0] += (bones[w->boneNum].p[0] + res[0])*w->boneWeight;
				v->absXYZ[1] += (bones[w->boneNum].p[1] + res[1])*w->boneWeight;
				v->absXYZ[2] += (bones[w->boneNum].p[2] + res[2])*w->boneWeight;
			}
		}
	}
}
void R_CalcNormalForTriangle(vec3_t normal, const vec3_t v0, const vec3_t v1, const vec3_t v2)
{
	vec3_t          udir, vdir;

	// compute the face normal based on vertex points
	VectorSubtract(v2, v0, udir);
	VectorSubtract(v1, v0, vdir);
	CrossProduct(udir, vdir, normal);

	VectorNormalize(normal);
}
void CalcModelNormals(tModel_t *m) {
	int i, j;
	tSurf_t *sf;
	tVert_t *v;
	matrix_t inverseMatrices[256];
	bone_t *b;
	tTri_t *t;

	// calculate absolute vertex positions for baseframe
	CalcModelXYZVerticesForBones(m,m->baseFrame);

	// setup inverse matrices (model space -> parent bone space)
	b = m->baseFrame;
	for(i = 0; i < m->numBones; i++, b++) {
		MatrixFromQuat(inverseMatrices[i],b->q);
		VectorCopy(b->p,&inverseMatrices[i][12]);
		MatrixInverse(inverseMatrices[i]);
	}

	// set all vertex normals to null
	sf = m->surfs;
	for(i = 0; i < m->numSurfaces; i++, sf++) {
		v = sf->verts;
		for(j = 0; j < sf->numVerts; j++, v++) {
			VectorClear(v->normal);
		}
	}

	// sum normals of each triangle
	sf = m->surfs;
	for(i = 0; i < m->numSurfaces; i++, sf++) {
		t = sf->tris;
		for(j = 0; j < sf->numTris; j++, t++) {
			int i0, i1, i2;
			vec3_t n;

			i0 = t->indexes[0];
			i1 = t->indexes[1];
			i2 = t->indexes[2];

			R_CalcNormalForTriangle(n,sf->verts[i0].absXYZ,sf->verts[i1].absXYZ,sf->verts[i2].absXYZ);

			VectorAdd(sf->verts[i0].normal,n,sf->verts[i0].normal);
			VectorAdd(sf->verts[i1].normal,n,sf->verts[i1].normal);
			VectorAdd(sf->verts[i2].normal,n,sf->verts[i2].normal);
		}
	}

	// normalize all normals and transform them to first parent bone space
	sf = m->surfs;
	for(i = 0; i < m->numSurfaces; i++, sf++) {
		v = sf->verts;
		for(j = 0; j < sf->numVerts; j++, v++) {
			int bone;

			VectorNormalize(v->normal);

			// use boneIndex of first vertex weight
			// (FAKK's max2skl uses only the first bone here, too)
			bone = v->weights[0].boneNum;

			MatrixTransformNormal2(inverseMatrices[bone],v->normal);
		}
	}
}

void SplitSurfaces(tModel_t *m);
int countVertBytes(tSurf_t *sf) {
	int i;
	int r = sf->numVerts * sizeof(skdVertex_t);
	for(i = 0; i < sf->numVerts; i++) {
		r += (sizeof(skdWeight_t) * sf->verts[i].numWeights);
	}
	return r;
}
void ConvertModel(tModel_t *m, const char *outFName) {
	skdHeader_t h;
	int i,j,k;
	tBone_t *b;
	tSurf_t *s;
	tVert_t *v;
	tWeight_t *w;
	FILE *out;
	int afterBones;

	out = F_Open(outFName,"wb");
	if(out == 0) {
		T_Error("ConvertModel: Cannot open %s\n",outFName);
	}

	memset(&h,0,sizeof(h));
	h.ident = SKD_IDENT;
	h.version = SKD_VERSION;
	h.numSurfaces = m->numSurfaces;
	h.numBones = m->numBones;
	h.ofsBones = sizeof(h);

	fwrite(&h,sizeof(h),1,out);

	for(i = 0, b = m->bones; i < m->numBones; i++,b++) {
		skdBone_t sb;
		char chanRot[SKC_MAX_CHANNEL_CHARS];
		char chanPos[SKC_MAX_CHANNEL_CHARS];
		int channelStringLen;
		vec3_t values;

		memset(&sb,0,sizeof(sb));
		sb.jointType = JT_POSROT_SKC;
		// bone name
		strcpy(sb.name,b->name);
		// parent name
		if(b->parent == -1) {
			strcpy(sb.parent,"worldbone");
		} else {
			strcpy(sb.parent,m->bones[b->parent].name);
		}
		
		// construct channel names
		Q_strncpyz(chanPos,sb.name,SKC_MAX_CHANNEL_CHARS-4);
		strcat(chanPos," pos");

		Q_strncpyz(chanRot,sb.name,SKC_MAX_CHANNEL_CHARS-4);
		strcat(chanRot," rot");

		channelStringLen = strlen(chanPos) + 1 
			+ strlen(chanRot) + 1;

		// calc offsets
		sb.ofsValues = sizeof(skdBone_t);
		sb.ofsChannels = sizeof(skdBone_t) + sizeof(vec3_t);
		sb.ofsEnd = sizeof(skdBone_t) + sizeof(vec3_t) +
			channelStringLen;

		sb.ofsRefs = sb.ofsEnd; // no references for classic POSROT bone.

		// write main bone data
		fwrite(&sb,sizeof(sb),1,out);	
		// write bone values 
		VectorSet(values,1.f,1.f,1.f);
		fwrite(&values,sizeof(values),1,out);	
		// write channel names 
		// su44 NOTE: ROT MUST GO FIRST.
		// (it will NOT work in MoHAA other way!)
		fwrite(chanRot,strlen(chanRot)+1,1,out);
		// .. then pos channel.
		fwrite(chanPos,strlen(chanPos)+1,1,out);
	}

	afterBones = ftell(out);

	for(i = 0, s = m->surfs; i < m->numSurfaces; i++,s++) {
		skdSurface_t sf;
		int numTriBytes;
		int numVertBytes;

		numTriBytes = s->numTris * sizeof(skdTriangle_t);
		//numVertBytes = s->numWeights * sizeof(skdWeight_t) +
		//	s->numVerts * sizeof(skdVertex_t);
		numVertBytes = countVertBytes(s);

		memset(&sf,0,sizeof(sf));
		sf.ident = SKD_SURFACE_IDENT;
		Q_strncpyz(sf.name,s->name,sizeof(sf.name));
		sf.numTriangles = s->numTris;
		sf.numVerts = s->numVerts;
		sf.ofsTriangles = sizeof(skdSurface_t);
		sf.ofsVerts = sizeof(skdSurface_t) + numTriBytes;
		sf.ofsEnd = sizeof(skdSurface_t) + numTriBytes + numVertBytes;
		
		// write surface
		fwrite(&sf,sizeof(sf),1,out);

		// write triangles
		fwrite(s->tris,s->numTris*sizeof(skdTriangle_t),1,out);

		// write vertices
		for(j = 0, v = s->verts; j < s->numVerts; j++, v++) {
			skdVertex_t sv;
			VectorCopy(v->normal,sv.normal);
			sv.numWeights = v->numWeights;
			sv.numMorphs = 0;
			sv.texCoords[0] = v->texCoords[0];
			sv.texCoords[1] = v->texCoords[1];
			fwrite(&sv,sizeof(sv),1,out);
			for(k = 0, w = v->weights; k < v->numWeights; k++, w++) {
				skdWeight_t sw;
				VectorCopy(w->offset,sw.offset);
				sw.boneIndex = w->boneNum;
				sw.boneWeight = w->boneWeight;
				fwrite(&sw,sizeof(sw),1,out);
			}
		}
	}

	// reupdate the header
	h.ofsEnd = ftell(out);
	h.ofsBoxes = h.ofsEnd;
	h.ofsMorphTargets = h.ofsEnd;
	h.ofsSurfaces = afterBones;
	fseek(out,0,SEEK_SET);
	fwrite(&h,sizeof(h),1,out);
	if(ftell(out) != sizeof(h)) {
		T_Error("Fatal file write error\n");
	}
	fclose(out);
	T_Printf("Wrote MoHAA model %s\n",outFName);
}

int verts[8192];
tTri_t tris[8192];
int numVerts;
int numTris;
int regVert(int v) {
	int i;
	for(i = 0; i < numVerts; i++) {
		if(verts[i] == v)
			return i;
	}
	verts[numVerts] = v;
	numVerts++;
	return i;
}
void addTri(tTri_t t) {
	tTri_t n;

	n.indexes[0] = regVert(t.indexes[0]);
	n.indexes[1] = regVert(t.indexes[1]);
	n.indexes[2] = regVert(t.indexes[2]);

	tris[numTris] = n;

	numTris++;
}
void SplitSurfaces(tModel_t *m) {
	int i,j,k;
	tSurf_t *sf, *nsf, *oldSurfs;
	int oldNumSurfs;
	int add;
	int realNewSurfacesCount;

	sf = m->surfs;
	add = 0;

	for(i = 0; i < m->numSurfaces; i++, sf++) {
		int vertSplits;
		int triSplits;

		vertSplits = 0;
		triSplits = 0;

		// MoHAA cant handle more than 1000 vertices
		if(sf->numVerts > 1000) {
			int tmp = sf->numVerts;
			while(tmp > 1000) {
				vertSplits++;
				tmp -= 1000;
			}
		}
		// or more than 2000 triangles
		if(sf->numTris > 2000) {
			int tmp = sf->numTris;
			while(tmp > 2000) {
				triSplits++;
				tmp -= 2000;
			}
		}
		add += (vertSplits + triSplits);
	}

	if(add == 0)
		return;
	oldNumSurfs = m->numSurfaces;
	oldSurfs = m->surfs;

	realNewSurfacesCount = 0;

	m->numSurfaces += add;
	m->surfs = malloc(m->numSurfaces * sizeof(tSurf_t));
	memset(m->surfs,0,sizeof(tSurf_t)*m->numSurfaces);
	sf = oldSurfs;
	nsf = m->surfs;
	for(i = 0; i < oldNumSurfs; i++, sf++,nsf++,realNewSurfacesCount++) {
		if(sf->numVerts > 1000 || sf->numTris > 2000) {
			numTris = 0;
			numVerts = 0;
			// iterate all the tris while performing the splits
			for(j = 0; j < sf->numTris; j++) {
				addTri(sf->tris[j]);
				if(numVerts + 3 >= 1000 || numTris + 1 >= 2000) {
					// copy out existing data and continue splitting
					strcpy(nsf->name,sf->name);
					nsf->numTris = numTris;
					nsf->tris = malloc(sizeof(tTri_t)*numTris);
					memcpy(nsf->tris,tris,sizeof(tTri_t)*numTris);
					nsf->numVerts = numVerts;
					nsf->verts = malloc(sizeof(tVert_t)*numVerts);
					for(k = 0; k < numVerts; k++) {
						nsf->verts[k] = sf->verts[verts[k]];
					}
					realNewSurfacesCount++;
					nsf++; // next new surface
					numVerts = 0;
					numTris = 0;
				}
			}
			// copy out remaining data
			assert(numTris);
			assert(numVerts);
			strcpy(nsf->name,sf->name);
			nsf->numTris = numTris;
			nsf->tris = malloc(sizeof(tTri_t)*numTris);
			memcpy(nsf->tris,tris,sizeof(tTri_t)*numTris);
			nsf->numVerts = numVerts;
			nsf->verts = malloc(sizeof(tVert_t)*numVerts);
			for(k = 0; k < numVerts; k++) {
				nsf->verts[k] = sf->verts[verts[k]];
			}
			continue;
		}
		*nsf = *sf;
	}
	
	m->numSurfaces = realNewSurfacesCount;
}

void constructPath(char *out, const char *source, const char *ext) {
	assert(ext[0] == '.');
	if(modelDataDir[0] == 0) {
		strcpy(out,source);
		stripExt(out);
		strcat(out,ext);
		return;
	} else {
		const char *dataPath;
		const char *p;
	
		dataPath = extractTIKIDataPath(modelDataDir);
		strcpy(out,dataPath);

		p = strchr_r(source,'/','\\');

		p++;

		strcat(out,p);
		stripExt(out);
		strcat(out,ext);
		return;
	}
}
static void GenerateSKLFiles() {
	int i;
	char outFName[512];

	if(noLimits == qfalse) {
		// ensure that there are no surfaces with
		// numVerts >= 1000 or numTris >= 2000
		SplitSurfaces(mainModel);
	}

	for(i = 0; i < numAnims; i++) {
		if(anims[i] == 0)
			continue;
		constructPath(outFName,anims[i]->fname,".skl");
		writeSKL(mainModel,anims[i],outFName);
	}
}
static void GenerateOBJFiles() {
	int i;
	char outFName[512];

	if(noLimits == qfalse) {
		// ensure that there are no surfaces with
		// numVerts >= 1000 or numTris >= 2000
		SplitSurfaces(mainModel);
	}
	constructPath(outFName,mainModel->fname,".obj");
	WriteOBJ_FromBaseFrame(outFName,mainModel);

	for(i = 0; i < numAnims; i++) {
		if(anims[i] == 0)
			continue;
		constructPath(outFName,anims[i]->fname,".obj");
		WriteOBJ_FromAnim(outFName,mainModel,anims[i]);
	}
}
// convert MoHAA model to famous Doom3 md5 format
void DecompileSKX() {
//char fname[MAX_TOOLPATH];
	int i,j;
	int numValidAnims;
	tModel_t *m;
	tAnim_t *a;
	

	// load bone defs and surfaces
	if(inMesh[0] == 0) {
		if(outTIKI[0] == 0) {
			T_Error("No source model specified\n");
		}
		loadTIKI(outTIKI);
	} else {
		mainModel = m = readSKD(inMesh,1.f);
		if(m == 0) {
			T_Error("Couldn't load %s\n",inMesh);
		}

		// load animations
		numValidAnims = 0;
		for(i = 0; i < numAnims; i++) {
			a = appendSKC(m,inAnimFNames[i],1.f);
			if(a == 0) {
				T_Printf("Failed to load: %s\n",inAnimFNames[i]);
			}
			anims[i] = a;
			if(a)
				numValidAnims++;
		}
	}
	
	if(renameBones == qtrue) {
		for(i = 0; i < mainModel->numBones; i++) {
			sprintf(mainModel->bones[i].name,"Bone%i",i);
		}
		for(j = 0; j < numAnims; j++) {
			tAnim_t *a = anims[j];
			if(a == 0 || a->boneData == 0)
				continue;
			for(i = 0; i < a->numBones; i++) {
				sprintf(a->boneData[i].name,"Bone%i",i);
			}
		}
	}

	if(createSKL) {
		GenerateSKLFiles();
		return;
	}
	if(createOBJ) {
		GenerateOBJFiles();
		return;
	}

	if(modelDataDir[0]) {
		constructPath(outSKDMesh,outTIKI,".md5mesh");
	} else {
		constructPath(outSKDMesh,inMesh,".md5mesh");

	}
	writeMD5Mesh(mainModel,outSKDMesh);

	// converts animations
	for(i = 0; i < numAnims; i++) {
		a = anims[i];
		if(a == 0) {
			continue;
		}
		constructPath(outSKCAnim[i],inAnimFNames[i],".md5anim");
		writeMD5Anim(a,outSKCAnim[i]);
	}
}
void CompileSKX() {
//	char fname[MAX_TOOLPATH];
	int i;
	int numValidAnims;
	tModel_t *m;
	tAnim_t *a;
	
	// load bone defs and surfaces
	mainModel = m = loadMD5Mesh(inMesh);
	if(m == 0) {
		T_Error("Couldn't load %s\n",inMesh);
	}

	if(noLimits == qfalse) {
		// ensure that there are no surfaces with
		// numVerts >= 1000 or numTris >= 2000
		SplitSurfaces(m);
	}

	if(renameBones == qtrue) {
		for(i = 0; i < m->numBones; i++) {
			sprintf(m->bones[i].name,"Bone%i",i);
		}
	}

	// load animations
	numValidAnims = 0;
	for(i = 0; i < numAnims; i++) {
		a = loadMD5Anim(inAnimFNames[i]);
		if(a == 0) {
			T_Printf("Failed to load: %s\n",inAnimFNames[i]);
		}

		////	WriteOBJ_FromAnim("e:/test.obj", m, a); // ok.
		anims[i] = a;
		if(a)
			numValidAnims++;
	}
	
	if(createSKL) {
		GenerateSKLFiles();
		return;
	}
	if(createOBJ) {
		GenerateOBJFiles();
		return;
	}

	constructPath(outSKDMesh,inMesh,".skd");
	ConvertModel(m,outSKDMesh);

	// if there are no animation files specified,
	// extract base frame from md5mesh and
	// save it to skc file
	if(numValidAnims == 0) {
		constructPath(outSKCAnim[0],inMesh,".skc");
		ConvertModelBaseFrameToAnim(m,outSKCAnim[0]);
		return;
	}

	// converts animations
	for(i = 0; i < numAnims; i++) {
		a = anims[i];
		if(a == 0) {
			continue;
		}
		constructPath(outSKCAnim[i],inAnimFNames[i],".skc");
		ConvertAnimation(a,outSKCAnim[i]);
	}
}
qboolean help_printed = qfalse;
void printHelp() {
	if(help_printed)
		return;
	help_printed = qtrue;
	T_Printf("=================================================\n");
	T_Printf("\n");
	T_Printf("Usage: md5_2_skX [-decompile] <skelmodel_file> <animation_file_1> "
		" <animation_file_2> <animation_file_n> -outdir <directory>\n"
		"\n"
		"Example 1: (converting Doom3 md5mesh/md5anim to MoHAA skc/skd files): \n"
		"md5_2_skX C:/MoHAA/main/mymodels/mynewgun/mynewgun.md5mesh\n"
		" C:/MoHAA/main/mymodels/mynewgun/mynewgun.md5anim\n"
		" C:/MoHAA/main/mymodels/mynewgun/mynewgun_reload.md5anim\n"
		" -outdir C:/MoHAA/main/models/weapons/mynewgun/\n"
		"\n"
		"Example2: (converting MoHAA skd/skc files to Doom3 md5mesh/md5anim): \n"
		"md5_2_skX -decompile C:/MoHAA/main/pak0_unpacked/models/weapons/mp44/mp44.skd\n"
		" C:/MoHAA/main/pak0_unpacked/models/weapons/mp44/mp44.skc\n"
		" C:/MoHAA/main/pak0_unpacked/models/weapons/mp44/mp44_reload.skc\n"
		" -outdir C:/MoHAA/main/decompiledmodels/\n"
		"\n"
		"Example3: (converting MoHAA tik file to Doom3 md5mesh/md5anim): \n"
		"md5_2_skX -decompile C:/MoHAA/main/pak0_unpacked/models/weapons/mp44.tik\n"
		" -outdir C:/MoHAA/main/decompiledmodels/\n"
		"Note that paths can be relative.\n");
	T_Printf("\n");
	T_Printf("=================================================\n");
}
int main(int argc, const char **argv) {
	int i;

	printf("=================================================\n");
	printf("\n");
	printf("md5_2_skX - v" MD5_2_SKX_VERSION " (c) 2012 su44.\n");
	printf("\n");
	printf("=================================================\n");

	printf("\n");

#if 0
	{
		tModel_t *m;
		tAnim_t *a;

		// rewrite some skc/skd files in order to validate
		// md5_2_skX export/import code...

		m = readSKD("E:/openmohaa/build/main/models/omtests/copyofsarge/body.skd",1.f);
		a = appendSKC(m,"E:/openmohaa/build/main/models/omtests/copyofsarge/run.skc",1.f);

		ConvertModel(m,"E:/openmohaa/build/main/models/omtests/sarge/body.skd");
		ConvertAnimation(a,"E:/openmohaa/build/main/models/omtests/sarge/run.skc");

		return;
	}
#endif
#if 0
	{
		tModel_t *m;
		tAnim_t *a;

		a = loadMD5Anim("E:/openmohaa/build/sdk/models/test - Kopia/run.md5anim");
		writeMD5Anim(a,"E:/openmohaa/build/sdk/models/test/run.md5anim");


		m = loadMD5Mesh("E:/openmohaa/build/sdk/models/test - Kopia/body.md5mesh");
		writeMD5Mesh(m,"E:/openmohaa/build/sdk/models/test/body.md5mesh");

		return;
	}


	
#endif
#if 0
	{
		tModel_t *m;
		tAnim_t *a;

		// NOTE: this will never recreate md5mesh completly
		// because "baseframe" information is lost during conversion
		// (there is no baseframe data in MoHAA skds)

		// d3->moh
		m = loadMD5Mesh("E:/openmohaa/build/sdk/models/test - Kopia/body.md5mesh");
		ConvertModel(m,"E:/openmohaa/build/sdk/models/test/body.skd");
		a = loadMD5Anim("E:/openmohaa/build/sdk/models/test - Kopia/run.md5anim");
		ConvertAnimation(a,"E:/openmohaa/build/sdk/models/test/run.skc");


		// moh->d3
		m = readSKD("E:/openmohaa/build/sdk/models/test/body.skd",1.f);
		a = appendSKC(m,"E:/openmohaa/build/sdk/models/test/run.skc",1.f);
		writeMD5Mesh(m,"E:/openmohaa/build/sdk/models/test/body.md5mesh");
		writeMD5Anim(a,"E:/openmohaa/build/sdk/models/test/run.md5anim");



		return;
	}
#endif
	// parse arguments list
	for(i = 1; i < argc; i++) {
		if(!Q_stricmp(argv[i], "-v")) {
			// print more info
			verbose = qtrue;
		} else if(!Q_stricmp(argv[i], "-nolimit") || !Q_stricmp(argv[i], "-nolimits")) {
			// dont split surfaces that exceed MoHAA limits
			// MoHAA will not load models created with that option.
			noLimits = qtrue;
		} else if(!Q_stricmp(argv[i], "-tik")) {
			// generate TIKI file as well.
			// This will ALSO set modelDataDir path (-outdir option)
			createTIK = qtrue;
			if(i+1 < argc && argv[i+1][0] != '-') {
				i++;
				strcpy(outTIKI,argv[i]);
				strcpy(modelDataDir,argv[i]);
				backSlashesToSlashes(modelDataDir); // fix slashes!
				backSlashesToSlashes(outTIKI); // fix slashes!
			}
		} else if(!Q_stricmp(argv[i], "-anim")) {
			// get anim filename
			i++;
			strcpy(inAnimFNames[numAnims],argv[i]);
			backSlashesToSlashes(inAnimFNames[numAnims]); // fix slashes!
			numAnims++;		
		} else if(!Q_stricmp(argv[i], "-mesh")) {
			// get mesh filename
			i++;
			strcpy(inMesh,argv[i]);
			backSlashesToSlashes(inMesh); // fix slashes!
		} else if(!Q_stricmp(argv[i], "-skl")) {
			createSKL = qtrue;
		} else if(!Q_stricmp(argv[i], "-obj")) {
			createOBJ = qtrue;
		} else if(!Q_stricmp(argv[i], "-renameBones")) {
			// rename bones with their numbers, needed to avoid original skl_2_skx asserts 
			renameBones = qtrue;
		} else if(strstr(argv[i], ".md5mesh") || strstr(argv[i], ".skd")) {
			// that's a mesh filename
			strcpy(inMesh,argv[i]);
			backSlashesToSlashes(inMesh); // fix slashes!
		} else if(strstr(argv[i], ".md5anim") || strstr(argv[i], ".skc")) {
			// that's a anim filename
			strcpy(inAnimFNames[numAnims],argv[i]);
			backSlashesToSlashes(inAnimFNames[numAnims]); // fix slashes!
			numAnims++;
#if 1
		} else if(strstr(argv[i], ".tik")) {
			strcpy(outTIKI,argv[i]);
			backSlashesToSlashes(outTIKI); // fix slashes!
#endif
		} else if(!Q_stricmp(argv[i], "-to") || !Q_stricmp(argv[i], "-outdir") || !Q_stricmp(argv[i], "-destdir")) {
			i++;
			// that's a path where generated skc/skd files should be put
			strcpy(modelDataDir,argv[i]);
			backSlashesToSlashes(modelDataDir); // fix slashes!
		} else if(strstr(argv[i], "-decompile")) {
			decompile = qtrue;
		} else if(!Q_stricmp(argv[i], "-help") || !Q_stricmp(argv[i], "help")) {
			printHelp();
		} else {
			T_Printf("Unknown argument \"%s\"\n",argv[i]);
			printHelp();
		}
	}

	T_Printf("Arguments parsed, beginning conversion...\n");

	if(decompile) {
		DecompileSKX();
	} else {
		CompileSKX();
		if(createTIK) {
			char fname[MAX_TOOLPATH];
			if(outTIKI[0]) {
				strcpy(fname,outTIKI);
			} else {
				strcpy(fname,inMesh);
				stripExt(fname);
				strcat(fname,".tik");
			}
			writeTIKI(mainModel, fname);
		}
	}
	printf("Done.\n");
	// let the user see the results
	system("pause");
}