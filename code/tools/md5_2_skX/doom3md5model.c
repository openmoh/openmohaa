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

// doom3md5model.c - Doom3 md5mesh and md5anim loading and animation code
// See: http://www.modwiki.net/wiki/MD5_(file_format)

#include "md5_2_skX.h"

// ensure that quaternions W component is negative,
// leaving the rotation unchanged
qboolean FixQuatForMD5_N(quat_t q) {
	// make sure that we will get the same 
	// quaternion if we recalculate W component...
//	quat_t copy;

	if(q[3] > 0) {
		QuatAntipodal(q);
		return qtrue; // quat "changed"!
	}
	return qfalse; // didnt change quat
}

// ensure that quaternions W component is positive,
// leaving the rotation unchanged
qboolean FixQuatForMD5_P(quat_t q) {
	// make sure that we will get the same 
	// quaternion if we recalculate W component...
//	quat_t copy;

	if(q[3] < 0) {
		QuatAntipodal(q);
		return qtrue; // quat "changed"!
	}
	return qfalse; // didnt change quat
}

/*
====================================================================

Doom3 md5mesh/md5anim LOADING

====================================================================
*/

tModel_t *loadMD5Mesh(const char *fname) {
	int i,j,k;
	char *txt;
	int len;
	char *p;
	const char *token;
	tModel_t *out;
	tBone_t *boneDef;
	tVert_t *v;
	tWeight_t *weight;
	tSurf_t *surf;
	tTri_t *tri;
	bone_t *bone;
	int tmpNumSurfs;
	int tmpNumBones;

	len = F_LoadBuf(fname,(byte**)&txt);

	if(len == -1) {
		T_Error("Cannot open %s\n",fname);
		return 0;
	}

	// NOTE: this will not open the "fname" file!
	COM_BeginParseSession(fname);

	p = txt;

	token = COM_ParseExt(&p, qfalse);
	if(Q_stricmp(token,"MD5Version")) {
		T_Error("loadMD5Mesh: Expected \"MD5Version\", found %s in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qfalse);
	if(Q_stricmp(token,"10")) {
		T_Error("loadMD5Mesh: Expected \"10\", found %s in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// skip optional commandline
	token = COM_ParseExt(&p, qtrue);
	if(!Q_stricmp(token,"commandline")) {
		token = COM_ParseExt(&p, qfalse);
		token = COM_ParseExt(&p, qtrue);
	}

	// parse numJoints <number>
	if(Q_stricmp(token, "numJoints")) {
		T_Error("loadMD5Mesh: expected \"numJoints\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qtrue);

	tmpNumBones = atoi(token);

	// parse numMeshes <number>
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "numMeshes")) {
		T_Error("loadMD5Mesh: expected \"numMeshes\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qtrue);

	tmpNumSurfs = atoi(token);

	// parse all the surfaces
	if(tmpNumSurfs < 1) {
		T_Error("loadMD5Mesh: \"%s\" has no surfaces\n", fname);
		return 0;
	}

	// okay, let's assume that md5mesh file is valid, and alloc 
	// output model. Still, it will be fried if any error occur
	out = malloc(sizeof(tModel_t));
	strcpy(out->fname,fname);
	out->numBones = tmpNumBones;
	out->numSurfaces = tmpNumSurfs;
	out->bones = malloc(sizeof(tBone_t)*out->numBones);
	out->baseFrame = malloc(sizeof(bone_t)*out->numBones);

	// parse "joints"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "joints")) {
		T_Error("loadMD5Mesh: expected \"joints\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// parse "{"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "{")) {
		T_Error("loadMD5Mesh: expected \"{\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	boneDef = out->bones;
	bone = out->baseFrame;
	for(i = 0; i < out->numBones; i++,bone++,boneDef++) {
		// bone name (eg "Bip01 Spine1")
		token = COM_ParseExt(&p, qtrue);
		Q_strncpyz(boneDef->name, token, sizeof(boneDef->name));

		token = COM_ParseExt(&p, qfalse);
		boneDef->parent = atoi(token);

		T_VerbPrintf("loadMD5Mesh: bone %i of %i - %s, parent %i\n",
			i,out->numBones,boneDef->name,boneDef->parent);

		// skip (
		token = COM_ParseExt(&p, qfalse);
		if(Q_stricmp(token, "(")) {
			T_Error("loadMD5Mesh: expected \"(\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		for(j = 0; j < 3; j++) {
			token = COM_ParseExt(&p, qfalse);
			bone->p[j] = atof(token);
		}

		// skip )
		token = COM_ParseExt(&p, qfalse);
		if(Q_stricmp(token, ")")) {
			T_Error("loadMD5Mesh: expected \")\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		// skip (
		token = COM_ParseExt(&p, qfalse);
		if(Q_stricmp(token, "(")) {
			T_Error("loadMD5Mesh: expected \"(\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		for(j = 0; j < 3; j++) {
			token = COM_ParseExt(&p, qfalse);
			bone->q[j] = atof(token);
		}
		QuatCalcW(bone->q);

		// skip )
		token = COM_ParseExt(&p, qfalse);
		if(Q_stricmp(token, ")")) {
			T_Error("loadMD5Mesh: expected \")\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
	}

	// parse closing } of joints block
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "}")) {
		T_Error("loadMD5Mesh: expected \"}\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	out->surfs = malloc(sizeof(*surf) * out->numSurfaces);
	for(i = 0, surf = out->surfs; i < out->numSurfaces; i++, surf++) {
		// parse mesh {
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "mesh")) {
			T_Error("loadMD5Mesh: expected \"mesh\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		token = COM_ParseExt(&p, qfalse);
		if(Q_stricmp(token, "{")) {
			T_Error("loadMD5Mesh: expected \"{\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		// parse shader <name>
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "shader")) {
			T_Error("loadMD5Mesh: expected \"shader\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		token = COM_ParseExt(&p, qfalse);
		Q_strncpyz(surf->name, token, sizeof(surf->name));

		// parse numVerts <number>
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "numVerts")) {
			T_Error("loadMD5Mesh: expected \"numVerts\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		token = COM_ParseExt(&p, qfalse);
		surf->numVerts = atoi(token);

		surf->verts = malloc(sizeof(*v) * surf->numVerts);
		for(j = 0, v = surf->verts; j < surf->numVerts; j++, v++) {
			// skip vert <number>
			token = COM_ParseExt(&p, qtrue);
			if(Q_stricmp(token, "vert")) {
				T_Error("loadMD5Mesh: expected \"vert\" found \"%s\" in file %s at line %i\n",
					token,fname,COM_GetCurrentParseLine());
				return 0;
			}
			COM_ParseExt(&p, qfalse);

			// skip (
			token = COM_ParseExt(&p, qfalse);
			if(Q_stricmp(token, "(")) {
				T_Error("loadMD5Mesh: expected \"(\" found \"%s\" in file %s at line %i\n",
					token,fname,COM_GetCurrentParseLine());
				return 0;
			}

			for(k = 0; k < 2; k++) {
				token = COM_ParseExt(&p, qfalse);
				v->texCoords[k] = atof(token);
			}

			// skip )
			token = COM_ParseExt(&p, qfalse);
			if(Q_stricmp(token, ")")) {
				T_Error("loadMD5Mesh: expected \")\" found \"%s\" in file %s at line %i\n",
					token,fname,COM_GetCurrentParseLine());
				return 0;
			}

			token = COM_ParseExt(&p, qfalse);
			v->firstWeight = atoi(token);

			token = COM_ParseExt(&p, qfalse);
			v->numWeights = atoi(token);
		}

		// parse numTris <number>
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "numTris")) 	{
			T_Error("loadMD5Mesh: expected \"numTris\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		token = COM_ParseExt(&p, qfalse);
		surf->numTris = atoi(token);

		surf->tris = malloc(sizeof(*tri) * surf->numTris);
		for(j = 0, tri = surf->tris; j < surf->numTris; j++, tri++) {
			// skip tri <number>
			token = COM_ParseExt(&p, qtrue);
			if(Q_stricmp(token, "tri")) {
				T_Error("loadMD5Mesh: expected \"tri\" found \"%s\" in file %s at line %i\n",
					token,fname,COM_GetCurrentParseLine());
				return 0;
			}
			COM_ParseExt(&p, qfalse);

			for(k = 0; k < 3; k++) {
				token = COM_ParseExt(&p, qfalse);
				tri->indexes[k] = atoi(token);
			}
		}

		// parse numWeights <number>
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "numWeights")) {
			T_Error("loadMD5Mesh: expected \"numWeights\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		token = COM_ParseExt(&p, qfalse);
		surf->numWeights = atoi(token);

		surf->weights = malloc(sizeof(*weight) * surf->numWeights);
		for(j = 0, weight = surf->weights; j < surf->numWeights; j++, weight++) {
			// skip weight <number>
			token = COM_ParseExt(&p, qtrue);
			if(Q_stricmp(token, "weight")) {
				T_Error("loadMD5Mesh: expected \"weight\" found \"%s\" in file %s at line %i\n",
					token,fname,COM_GetCurrentParseLine());
				return 0;
			}
			COM_ParseExt(&p, qfalse);

			token = COM_ParseExt(&p, qfalse);
			weight->boneNum = atoi(token);

			token = COM_ParseExt(&p, qfalse);
			weight->boneWeight = atof(token);

			// skip (
			token = COM_ParseExt(&p, qfalse);
			if(Q_stricmp(token, "(")) {
				T_Error("loadMD5Mesh: expected \"(\" found \"%s\" in file %s at line %i\n",
					token,fname,COM_GetCurrentParseLine());
				return 0;
			}

			for(k = 0; k < 3; k++) {
				token = COM_ParseExt(&p, qfalse);
				weight->offset[k] = atof(token);
			}

			// skip )
			token = COM_ParseExt(&p, qfalse);
			if(Q_stricmp(token, ")")) {
				T_Error("loadMD5Mesh: expected \")\" found \"%s\" in file %s at line %i\n",
					token,fname,COM_GetCurrentParseLine());
				return 0;
			}
		}

		// parse }
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "}")) {
			T_Error("loadMD5Mesh: expected \"}\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		// loop trough all vertices and set up the vertex weights
		for(j = 0, v = surf->verts; j < surf->numVerts; j++, v++) {
			v->weights = malloc(sizeof(*v->weights) * v->numWeights);

			for(k = 0; k < v->numWeights; k++) {
				v->weights[k] = surf->weights[v->firstWeight + k];
			}
		}
		
		//////// TODO: precalculate normals
		//////for(j = 0, v = surf->verts; j < surf->numVerts; j++, v++) {
		//////	VectorSet(v->normal,0,0,1);
		//////}

	}
	CalcModelNormals(out);
	
	// WriteOBJ_FromBaseFrame("e:/test.obj", out); // OK
	T_Printf("Succesfully loaded Doom3 model %s\n",fname);
	return out;
}

tAnim_t *loadMD5Anim(const char *fname) {
	int i,/*j,*/k;
	char *txt;
	int len;
	char *p;
	const char *token;
	tAnim_t *out;
	tFrame_t *frame;
	bone_t *bone;
	tAnimBone_t *boneDef;
	int tmpNumJoints, tmpNumFrames;
	int tmpNumAnimatedComponents;
	float tmpFrameRate;

	len = F_LoadBuf(fname,(byte**)&txt);

	if(len == -1) {
		T_Printf("Cannot open %s\n",fname);
		return 0;
	}

	// NOTE: this will not open the "fname" file!
	COM_BeginParseSession(fname);

	p = txt;

	token = COM_ParseExt(&p, qfalse);
	if(Q_stricmp(token,"MD5Version")) {
		T_Error("loadMD5Anim: Expected \"MD5Version\", found %s in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qfalse);
	if(Q_stricmp(token,"10")) {
		T_Error("loadMD5Anim: Expected \"10\", found %s in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// skip optional commandline
	token = COM_ParseExt(&p, qtrue);
	if(!Q_stricmp(token,"commandline")) {
		token = COM_ParseExt(&p, qfalse);
		token = COM_ParseExt(&p, qtrue);
	}

	// parse numFrames <number>
	if(Q_stricmp(token, "numFrames")) {
		T_Error("loadMD5Anim: expected \"numFrames\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qtrue);

	tmpNumFrames = atoi(token);

	if(tmpNumFrames < 1) {
		T_Error("loadMD5Anim: \"%s\" has no frames\n", fname);
		return 0;
	}

	// parse numJoints <number>
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "numJoints")) {
		T_Error("loadMD5Anim: expected \"numJoints\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qtrue);

	tmpNumJoints = atoi(token);

	if(tmpNumJoints < 1) {
		T_Error("loadMD5Anim: \"%s\" has no joints\n", fname);
		return 0;
	}

	// parse frameRate <number>
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "frameRate")) {
		T_Error("loadMD5Anim: expected \"frameRate\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qtrue);

	tmpFrameRate = atof(token);

	// parse numAnimatedComponents <number>
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "numAnimatedComponents")) {
		T_Error("loadMD5Anim: expected \"numAnimatedComponents\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	token = COM_ParseExt(&p, qtrue);

	tmpNumAnimatedComponents = atoi(token);

	// okay, let's assume that md5anim file is valid, and alloc 
	// output animation. Still, it will be fried if any error occur
	out = malloc(sizeof(tAnim_t));
	
	strcpy(out->fname,fname);

	out->numAnimatedComponents = tmpNumAnimatedComponents;
	out->numBones = tmpNumJoints;
	out->frameRate = tmpFrameRate;
	out->numFrames = tmpNumFrames;
	out->boneData = malloc(sizeof(tAnimBone_t)*out->numBones);
	out->baseFrame = malloc(sizeof(bone_t)*out->numBones);
	out->frames = malloc(sizeof(tFrame_t)*out->numFrames);

	// parse "hierarchy"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "hierarchy")) {
		T_Error("loadMD5Anim: expected \"hierarchy\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// parse "{"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "{")) {
		T_Error("loadMD5Anim: expected \"{\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	boneDef = out->boneData;
	for(i = 0; i < out->numBones; i++,boneDef++) {
		// bone name (eg "Bip01 Spine1")
		token = COM_ParseExt(&p, qtrue);
		Q_strncpyz(boneDef->name, token, sizeof(boneDef->name));

		token = COM_ParseExt(&p, qtrue);
		boneDef->parent = atoi(token);

		T_VerbPrintf("loadMD5Anim: bone %i of %i - %s, parent %i\n",
			i,out->numBones,boneDef->name,boneDef->parent);

		token = COM_ParseExt(&p,qtrue);
		boneDef->componentBits = atoi(token);

		token = COM_ParseExt(&p,qtrue);
		boneDef->firstComponent = atoi(token);
	}

	// parse closing } of joints block
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "}")) {
		T_Error("loadMD5Anim: expected \"}\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// parse "bounds"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "bounds")) {
		T_Error("loadMD5Anim: expected \"bounds\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// parse "{"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "{")) {
		T_Error("loadMD5Anim: expected \"{\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	for(i = 0, frame = out->frames; i < out->numFrames; i++, frame++) {
		// parse mins
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "(")) {
			T_Error("loadMD5Anim: expected \"(\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		for(k = 0; k < 3; k++) {
			token = COM_ParseExt(&p, qtrue);
			frame->mins[k] = atof(token);
		}
	
		// skip )
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, ")")) {
			T_Error("loadMD5Anim: expected \")\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		// parse maxs
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "(")) {
			T_Error("loadMD5Anim: expected \"(\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		for(k = 0; k < 3; k++) {
			token = COM_ParseExt(&p, qfalse);
			frame->maxs[k] = atof(token);
		}
	
		// skip )
		token = COM_ParseExt(&p, qfalse);
		if(Q_stricmp(token, ")")) {
			T_Error("loadMD5Anim: expected \")\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
	}

	// parse "}"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "}")) {
		T_Error("loadMD5Anim: expected \"}\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// parse "baseframe"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "baseframe")) {
		T_Error("loadMD5Anim: expected \"baseframe\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// parse "{"
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "{")) {
		T_Error("loadMD5Anim: expected \"{\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	for(i = 0, bone = out->baseFrame; i < out->numBones; i++, bone++) {
		// parse ofs
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "(")) {
			T_Error("loadMD5Anim: expected \"(\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		for(k = 0; k < 3; k++) {
			token = COM_ParseExt(&p, qtrue);
			bone->p[k] = atof(token);
		}
	
		// skip )
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, ")")) {
			T_Error("loadMD5Anim: expected \")\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		// parse quat
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "(")) {
			T_Error("loadMD5Anim: expected \"(\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		for(k = 0; k < 3; k++) {
			token = COM_ParseExt(&p, qfalse);
			bone->q[k] = atof(token);
		}
	
		// skip )
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, ")")) {
			T_Error("loadMD5Anim: expected \")\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
	}

	// skip }
	token = COM_ParseExt(&p, qtrue);
	if(Q_stricmp(token, "}")) {
		T_Error("loadMD5Anim: expected \"}\" found \"%s\" in file %s at line %i\n",
			token,fname,COM_GetCurrentParseLine());
		return 0;
	}

	// parse frames
	for(i = 0, frame = out->frames; i < out->numFrames; i++, frame++) {
		// parse frame <number>
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "frame")) {
			T_Error("loadMD5Anim: expected \"frame\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		token = COM_ParseExt(&p, qtrue);
		if(atoi(token) != i) {
			T_Error("loadMD5Anim: expected \"frame %i\" found \"frame %s\" in file %s at line %i\n",
				i,token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		frame->components = malloc(sizeof(float)*out->numAnimatedComponents);
		// parse components block
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "{")) {
			T_Error("loadMD5Anim: expected \"{\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}

		for(k = 0; k < out->numAnimatedComponents; k++) {
			token = COM_ParseExt(&p, qtrue);
			frame->components[k] = atof(token);
		}
	
		// skip  }
		token = COM_ParseExt(&p, qtrue);
		if(Q_stricmp(token, "}")) {
			T_Error("loadMD5Anim: expected \"}\" found \"%s\" in file %s at line %i\n",
				token,fname,COM_GetCurrentParseLine());
			return 0;
		}
		
	}

	T_Printf("Succesfully loaded Doom3 animation %s\n",fname);
	return out;
}


/*
====================================================================

Doom3 md5mesh/md5anim WRITING

====================================================================
*/

void writeMD5Mesh(tModel_t *m, const char *outFName) {
	FILE *out;
	int i,j,k;
	tBone_t *boneDef;
	tVert_t *v;
	tWeight_t *w;
	tSurf_t *sf;
	tTri_t *tri;
	bone_t *b;

	T_Printf("Writing Doom3 md5mesh file %s...\n",outFName);

	out = F_Open(outFName,"w");

	//fprintf(out,"//////////////////////////////////////////////////////////////////////////\n");
	//fprintf(out,"//\n");
	//fprintf(out,"// Exported by md5_2_skX\n");
	//fprintf(out,"//\n");
	//fprintf(out,"//////////////////////////////////////////////////////////////////////////\n");
	//fprintf(out,"\n");
	
	fprintf(out,"MD5Version 10\n");
	fprintf(out,"commandline \"\"\n");
	fprintf(out,"\n");
	fprintf(out,"numJoints %i\n",m->numBones);
	fprintf(out,"numMeshes %i\n",m->numSurfaces);
	fprintf(out,"\n");
	// write joints block
	fprintf(out,"joints {\n");

	boneDef = m->bones;
	b = m->baseFrame;
	for(i = 0; i < m->numBones; i++, boneDef++,b++) {
#if 1
		// ensure that md5 loader
		// will be able to recalculate
		// W component correctly
		qboolean changed = FixQuatForMD5_N(b->q);
		if(changed) {
			T_Printf("WriteMD5Mesh: fixed q of bone %i\n",i);
		} else {

		}
#endif
		fprintf(out,"\t\"%s\" %i ( %f %f %f ) ( %f %f %f )",
			boneDef->name,boneDef->parent,
			b->p[0],b->p[1],b->p[2],b->q[0],b->q[1],b->q[2]);
		fprintf(out," //");
		if(boneDef->parent != -1) {
			fprintf(out," %s",m->bones[boneDef->parent].name);
		}
		fprintf(out,"\n");
	}
	fprintf(out,"}\n\n");

	// write meshes
	sf = m->surfs;
	for(i = 0; i < m->numSurfaces; i++,sf++) {
		int weightCount;
		fprintf(out,"mesh {\n");
		fprintf(out,"\tshader \"%s\"\n",sf->name);
		fprintf(out,"\n");
		// write vertices
		fprintf(out,"\tnumverts %i\n",sf->numVerts);
		weightCount = 0;
		for(j = 0, v = sf->verts; j < sf->numVerts; j++, v++) {
			fprintf(out,"\tvert %i ( %f %f ) %i %i\n",
				j,v->texCoords[0],v->texCoords[1],weightCount,v->numWeights);

			weightCount += v->numWeights;
		}
		fprintf(out,"\n");
		// write triangles
		fprintf(out,"\tnumtris %i\n",sf->numTris);
		for(j = 0, tri = sf->tris; j < sf->numTris; j++, tri++) {
			fprintf(out,"\ttri %i %i %i %i\n",
				j,tri->indexes[0],tri->indexes[1],tri->indexes[2]);
		}
		fprintf(out,"\n");
		// write weights
		fprintf(out,"\tnumweights %i\n",weightCount);
		weightCount = 0;
		for(j = 0, v = sf->verts; j < sf->numVerts; j++, v++) {
			for(k = 0, w = v->weights; k < v->numWeights; k++,w++) {
				fprintf(out,"\tweight %i %i %f ( %f %f %f )\n",
					weightCount,w->boneNum,w->boneWeight,w->offset[0],w->offset[1],w->offset[2]);
				weightCount++;
			}
		}
		fprintf(out,"}\n");
		fprintf(out,"\n");
	}
	fclose(out);
	T_Printf("Wrote %s.\n",outFName);
}

#define COMPONENTS_PER_LINE 6

void writeMD5Anim(tAnim_t *a, const char *outFName) {
	FILE *out;
	int i,j; //,k;
	tAnimBone_t *ab;
	tFrame_t *f;
	bone_t *b;

	T_Printf("Writing Doom3 md5anim file %s...\n",outFName);

	out = F_Open(outFName,"w");
	
	fprintf(out,"MD5Version 10\n");
	fprintf(out,"commandline \"\"\n");
	fprintf(out,"\n");
	fprintf(out,"numFrames %i\n",a->numFrames);
	fprintf(out,"numJoints %i\n",a->numBones);

	// floating point frameRate gives Quark error
	//  self.frameRate=int(words[1])
	//	ValueError: invalid literal for int(): 29.999998
	//fprintf(out,"frameRate %f\n",a->frameRate);
	fprintf(out,"frameRate %i\n",((int)(a->frameRate)));

	fprintf(out,"numAnimatedComponents %i\n",a->numAnimatedComponents);
	fprintf(out,"\n");
	// write joint hierarchy
	fprintf(out,"hierarchy {\n");
	for(i = 0, ab = a->boneData; i < a->numBones; i++, ab++) {
		fprintf(out,"\t\"%s\" %i %i %i\n",
			ab->name,ab->parent,
			ab->componentBits,ab->firstComponent);
	}
	fprintf(out,"}\n");
	// write frame bounds
	fprintf(out,"\nbounds {\n");
	for(i = 0, f = a->frames; i < a->numFrames; i++, f++) {
		fprintf(out,"\t( %f %f %f ) ( %f %f %f )\n",
			f->mins[0],f->mins[1],f->mins[2],f->maxs[0],f->maxs[1],f->maxs[2]);
	}
	fprintf(out,"}\n");
	// write baseframe
	fprintf(out,"\nbaseframe {\n");
	for(i = 0, b = a->baseFrame; i < a->numBones; i++, b++) {
#if 0
		// ensure that md5 loader
		// will be able to recalculate
		// W component correctly
		qboolean changed = FixQuatForMD5_P(b->q);
		if(changed) {
			T_Printf("WriteMD5Anim: fixed q of bone %i\n",i);
		} else {

		}
#endif
		fprintf(out,"\t( %f %f %f ) ( %f %f %f )\n",
			b->p[0],b->p[1],b->p[2],b->q[0],b->q[1],b->q[2]);
	}
	fprintf(out,"}\n");

	// write separate frames
	for(i = 0, f = a->frames; i < a->numFrames; i++, f++) {
		fprintf(out,"\nframe %i {",i);
		// write frame components
		for(j = 0; j < a->numAnimatedComponents; j++) {
			if(j % COMPONENTS_PER_LINE == 0)
				fprintf(out,"\n\t");
			fprintf(out,"%f ",f->components[j]);
		}
		if(j % COMPONENTS_PER_LINE != 1)
			fprintf(out,"\n");
		fprintf(out,"}\n\n");
	}

	fclose(out);

	T_Printf("Wrote %s.\n",outFName);

}

/*
====================================================================

Doom3 md5mesh/md5anim ANIMATION code

====================================================================
*/

bone_t *setupMD5AnimBones(tAnim_t *a, int frameNum) {
	static bone_t bones[256];
	tFrame_t *f;
	int i;
	bone_t *baseBone;
	bone_t *out;
	tAnimBone_t *boneDef;

	if(a->numFrames <= frameNum) {
		T_Error("Frame index out of range (frame %i, numFrames %i, anim file %s\n",
			frameNum,a->numFrames,a->fname);
	}

	f = a->frames + frameNum;

	for(i = 0, baseBone = a->baseFrame, boneDef = a->boneData, out = bones;
		i < a->numBones; i++, baseBone++,boneDef++, out++) {
		vec3_t p;
		quat_t q;
		int componentsApplied;

		// set baseframe values
		VectorCopy(baseBone->p, p);
		QuatCopy(baseBone->q, q);

		componentsApplied = 0;

		// update tranlation bits
		if(boneDef->componentBits & COMPONENT_BIT_TX) {
			p[0] = f->components[boneDef->firstComponent + componentsApplied];
			componentsApplied++;
		}

		if(boneDef->componentBits & COMPONENT_BIT_TY) {
			p[1] = f->components[boneDef->firstComponent + componentsApplied];
			componentsApplied++;
		}

		if(boneDef->componentBits & COMPONENT_BIT_TZ) {
			p[2] = f->components[boneDef->firstComponent + componentsApplied];
			componentsApplied++;
		}

		// update quaternion rotation bits
		if(boneDef->componentBits & COMPONENT_BIT_QX) {
			q[0] = f->components[boneDef->firstComponent + componentsApplied];
			componentsApplied++;
		}

		if(boneDef->componentBits & COMPONENT_BIT_QY) {
			q[1] = f->components[boneDef->firstComponent + componentsApplied];
			componentsApplied++;
		}

		if(boneDef->componentBits & COMPONENT_BIT_QZ) {
			q[2] = f->components[boneDef->firstComponent + componentsApplied];
		}

		// calculate quaternion W value, as it isnt stored in md5 anim
		QuatCalcW(q);
		QuatNormalize(q);
		
		// inverse quat for MoHAA
		QuatInverse(q);
		
		// copy out results
		VectorCopy(p,out->p);
		QuatCopy(q,out->q);
	}

	return bones;
}

void md5AnimateBones(tModel_t *m, bone_t *bones) {
	int i;
	tBone_t *b;
	bone_t *bo;

	for(i = 0, b = m->bones, bo = bones; i < m->numBones; i++, b++, bo++) {
		quat_t	pos,res,temp;
		if(b->parent == -1) {
			QuatInverse(bo->q);
			continue;
		}
		VectorCopy(bo->p,pos);
		pos[3]=0;
		QuaternionMultiply(temp,pos,bones[b->parent].q);
		QuatInverse(bones[b->parent].q);
		QuaternionMultiply(res,bones[b->parent].q,temp);
		QuatInverse(bones[b->parent].q);
		bones[i].p[0] = res[0] + bones[b->parent].p[0];
		bones[i].p[1] = res[1] + bones[b->parent].p[1];
		bones[i].p[2] = res[2] + bones[b->parent].p[2];
		QuatCopy(bones[i].q,temp);
		QuatInverse(temp);
		QuaternionMultiply(bones[i].q,temp,bones[b->parent].q);
		QuatNormalize(bones[i].q);
	}
}

bone_t *setupMD5MeshBones(tModel_t *mod) {
	static bone_t bones[256];
	int i;
	bone_t *out;
	bone_t *baseBone;
	tBone_t *bone;

	for(i = 0, bone = mod->bones, baseBone = mod->baseFrame, out = bones;
		i < mod->numBones; i++, bone++, baseBone++, out++) {
		matrix_t pM;
		matrix_t m;
		bone_t *baseParent;

		if(bone->parent == -1) {
			//QuatSet(out->q,0,0,0,1);
			// that's a hack which should be fixed soon...
			QuatFromAngles(out->q,0,0,-90);
			VectorSet(out->p,0,0,0);
			continue; // nothing else to do.
		}

		baseParent = &mod->baseFrame[bone->parent];

		//
		// calculate bone position in local space
		// (relative to parent)
		//

		// get transformation matrix of this bone
		//MatrixFromQuat(pM,baseBone->q);
		MatrixSetupTransformFromQuat(m,baseBone->q,baseBone->p);

		// get the inverse transform matrix of parent bone
		MatrixSetupTransformFromQuat(pM,baseParent->q,baseParent->p);
		QuatFromMatrix(baseParent->q,pM);
		MatrixInverse(pM);

		// multiple them
		MatrixMultiply2(pM,m);

		// convert result matrix back to quaternion and vector
		QuatFromMatrix(out->q,pM);
		QuatInverse(out->q);

		T_Printf("Q: %f %f %f %f\n",out->q[0],out->q[1],out->q[2],out->q[3]);
		VectorCopy(&pM[12],out->p);
	}

	return bones;
}








