/*
===========================================================================
Copyright (C) 2012 su44

This file is part of OpenMohaa source code.

OpenMohaa source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMohaa source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMohaa source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// cg_skeletor.c -- MoHAA model skeletor related functions


#include "qcommon.h"
#include "../../qcommon/qfiles.h" // su44: I need This for boneType enum

cvar_t *cg_skel_show;
cvar_t *cg_skel_printBoneDirty;
cvar_t *cg_skel_printBonePos;
cvar_t *cg_skel_printHoseParms;
cvar_t *cg_skel_printIKChainParms;
cvar_t *cg_skel_drawBones;
cvar_t *cg_skel_su44;
cvar_t *cg_skel_hookBoneVTable;
cvar_t *cheats;
cvar_t *cg_skel_nullBoneLenghts;
cvar_t *cg_skel_forceBoneLen;

void CG_InitSkeletorCvarsAndCmds() {
	cg_skel_show = cgi.Cvar_Get("cg_skel_show","0",0);
	cg_skel_printBoneDirty = cgi.Cvar_Get("cg_skel_printBoneDirty","0",0);
	cg_skel_printBonePos = cgi.Cvar_Get("cg_skel_printBonePos","0",0);
	cg_skel_drawBones = cgi.Cvar_Get("cg_skel_drawBones","0",0);
	cg_skel_printHoseParms = cgi.Cvar_Get("cg_skel_printHoseParms","0",0);
	cg_skel_printIKChainParms = cgi.Cvar_Get("cg_skel_printIKChainParms","0",0);
	cg_skel_su44 = cgi.Cvar_Get("cg_skel_su44","1",0);
	cg_skel_hookBoneVTable = cgi.Cvar_Get("cg_skel_hookBoneVTable","1",0);
	cg_skel_nullBoneLenghts = cgi.Cvar_Get("cg_skel_nullBoneLenghts","0",0);
	cg_skel_forceBoneLen = cgi.Cvar_Get("cg_skel_forceBoneLen","-1",0);
	// hack, force some cvars to their usefull values
	cheats = cgi.Cvar_Get("cheats","0",0);
	cheats->integer = 1;
	cheats = cgi.Cvar_Get("sv_cheats","0",0);
	cheats->integer = 1;
	cheats = cgi.Cvar_Get("thereisnomonkey","0",0);
	cheats->integer = 1;
}

typedef struct {
	int type;
	char *name;
} boneNameAndType_t;

// su44: known boneType / boneName pairs
// (from human/coxswain.tik)
boneNameAndType_t knownBoneTypes [] = {
	{ JT_ROTATION, "Bip01 Spine" }, // boneType 0
	{ JT_POSROT_SKC, "Bip01 Pelvis" }, // boneType 1
	{ JT_SHOULDER, "Bip01 L Thigh" }, // boneType 2
	{ JT_ELBOW, "Bip01 L Calf" }, // boneType 3
	{ JT_WRIST, "Bip01 L Foot" }, // boneType 4
	// boneType 5, hoseType 2, bendMax 180, bendRatio 1, spinRatio 0.6
	{ JT_HOSEROTBOTH, "helper Rankle" }, 
	// boneType 5, hoseType 1, bendMax 180, bendRatio 0.5, spinRatio 0.9
	{ JT_HOSEROTPARENT, "helper Lhip" }, 
	{ JT_AVROT, "helper Lelbow" }, // boneType 6
};

static int numKnownBoneTypes = sizeof(knownBoneTypes) / sizeof(knownBoneTypes[0]);

qboolean vPtrsExtracted = qfalse;
// su44: pointers to MoHAA bone vTables
skelBoneVTable_t *boneVPtrs[16];
// su44: pointers to MoHAA bone vTable functions
skelBoneVTable_t mohVTBLs[16];

// su44: extract vTable pointer for each bone class
// (This is totally offset-independent  
// and should run with any build of MoH)
void CG_Skel_ExtractVPTRs(skeletor_c *skel) {
	int i, j;

#if 1
	if(vPtrsExtracted)
		return;
#endif

	for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
		skelBone_Base_c *b;
		const char *name;
		boneNameAndType_t *bt;

		b = skel->m_bone[i];
		name = cgi.Tag_NameForNum(skel->m_Tiki,i);
		bt = knownBoneTypes;
		for(j = 0; j < numKnownBoneTypes; j++,bt++) {
			if(!stricmp(bt->name,name)) {
				// match found, so safe the vtable pointer
				if(boneVPtrs[bt->type] == 0) {
					boneVPtrs[bt->type] = (void*)b->vptr;
					mohVTBLs[bt->type] = *b->vptr;
				} else {
					if(boneVPtrs[bt->type] != (void*)b->vptr) {
						cgi.Error(0,"CG_Skel_ExtractVPTRs: This should never happen; ask su44 why\n");
					}
				}

			}
		}
	}	

	if(boneVPtrs[JT_POSROT_SKC] && boneVPtrs[JT_ROTATION] && boneVPtrs[JT_AVROT]
		&& boneVPtrs[JT_ELBOW] && boneVPtrs[JT_SHOULDER] && boneVPtrs[JT_WRIST]
		&& boneVPtrs[JT_HOSEROTBOTH] && boneVPtrs[JT_HOSEROTPARENT]) {
		vPtrsExtracted = qtrue;
		Com_Printf("CG_Skel_ExtractVPTRs: extraced all vPtrs! \n");
	}
}

// su44: some util functions
qboolean CG_Bone_IsHoseRot(skelBone_Base_c *b) {
	if(boneVPtrs[JT_HOSEROTBOTH] == b->vptr
		|| boneVPtrs[JT_HOSEROTPARENT] == b->vptr)
		return qtrue;

	return qfalse;
}

qboolean CG_Bone_IsHoseRotBoth(skelBone_Base_c *b) {
	if(boneVPtrs[JT_HOSEROTBOTH] == b->vptr)
		return qtrue;

	return qfalse;
}
qboolean CG_Bone_IsHoseRotParent(skelBone_Base_c *b) {
	if(boneVPtrs[JT_HOSEROTPARENT] == b->vptr)
		return qtrue;

	return qfalse;
}
qboolean CG_Bone_IsAVRot(skelBone_Base_c *b) {
	if(boneVPtrs[JT_AVROT] == b->vptr)
		return qtrue;

	return qfalse;
}
qboolean CG_Bone_IsWrist(skelBone_Base_c *b) {
	if(boneVPtrs[JT_WRIST] == b->vptr)
		return qtrue;

	return qfalse;
}
qboolean CG_Bone_IsPosRot(skelBone_Base_c *b) {
	if(boneVPtrs[JT_POSROT_SKC] == b->vptr)
		return qtrue;

	return qfalse;
}
qboolean CG_Bone_IsElbow(skelBone_Base_c *b) {
	if(boneVPtrs[JT_ELBOW] == b->vptr)
		return qtrue;

	return qfalse;
}
qboolean CG_Bone_IsShoulder(skelBone_Base_c *b) {
	if(boneVPtrs[JT_SHOULDER] == b->vptr)
		return qtrue;

	return qfalse;
}

// returns skdJointType_t or -1 if unknown (0 is used for JT_ROTATION)
int CG_Bone_GetType(skelBone_Base_c *b) {
	int i;

	if(b->vptr == 0)
		return -2; // invalid bone ptr ?

	for(i = 0; i < 16; i++) {
		if(b->vptr == boneVPtrs[i])
			return i;
	}
	return -1;
}


typedef SkelMat4_t *(__stdcall *GetTransformFunc)(skelBone_Base_c *bone, skelAnimStoreFrameList_c *channels);

int CG_Skel_BoneIndexForPointer(skelBone_Base_c *bone, skeletor_c *skel) {
	int i;

	for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
		skelBone_Base_c *b;

		b = skel->m_bone[i];

		if(b == bone)
			return i;
	}
	return -1;
}
void CG_PrintfBone_Shoulder(skelBone_Base_c *b, skeletor_c *skel, int i) {
	skelBone_IKshoulder_c *s;
	int myWristIndex;
	int parentIndex;

	s = (skelBone_IKshoulder_c*)b;
	parentIndex = CG_Skel_BoneIndexForPointer((skelBone_Base_c *)s->m_parent,skel);
	myWristIndex = CG_Skel_BoneIndexForPointer((skelBone_Base_c *)s->m_wrist,skel);

	// su44: it seems that s->m_cosElbowAngle is calculated on the fly
	cgi.Printf("Bone %i of %i is a shoulder, name %s, parent %s, wrist %s, lowerLen %f, upperLen %f, cosElbowAngle %f\n",
		i,skel->m_Tiki->m_boneList.m_numChannels,cgi.Tag_NameForNum(skel->m_Tiki,i),
		cgi.Tag_NameForNum(skel->m_Tiki,parentIndex),cgi.Tag_NameForNum(skel->m_Tiki,myWristIndex),
		s->m_lowerLength,s->m_upperLength,s->m_cosElbowAngle
	);
	
}
void CG_PrintfBone_Elbow(skelBone_Base_c *b, skeletor_c *skel, int i) {
	skelBone_IKelbow_c *e;
	int myShoulderIndex;
	int parentIndex;

	e = (skelBone_IKelbow_c*)b;
	parentIndex = CG_Skel_BoneIndexForPointer((skelBone_Base_c*)e->m_parent,skel);
	myShoulderIndex = CG_Skel_BoneIndexForPointer((skelBone_Base_c*)e->m_shoulder,skel);

	cgi.Printf("Bone %i of %i is a elbow, name %s, parent %s, shoulder %s\n",
		i,skel->m_Tiki->m_boneList.m_numChannels,cgi.Tag_NameForNum(skel->m_Tiki,i),
		cgi.Tag_NameForNum(skel->m_Tiki,parentIndex),cgi.Tag_NameForNum(skel->m_Tiki,myShoulderIndex)
	);
}
void CG_PrintfBone_Wrist(skelBone_Base_c *b, skeletor_c *skel, int i) {
	skelBone_IKwrist_c *w;
	int myShoulderIndex;
	int parentIndex;

	w = (skelBone_IKwrist_c*)b;
	parentIndex = CG_Skel_BoneIndexForPointer((skelBone_Base_c*)w->m_parent,skel);
	myShoulderIndex = CG_Skel_BoneIndexForPointer((skelBone_Base_c*)w->m_shoulder,skel);

	cgi.Printf("Bone %i of %i is a wrist, name %s, parent %s, shoulder %s\n",
		i,skel->m_Tiki->m_boneList.m_numChannels,cgi.Tag_NameForNum(skel->m_Tiki,i),
		cgi.Tag_NameForNum(skel->m_Tiki,parentIndex),cgi.Tag_NameForNum(skel->m_Tiki,myShoulderIndex)
	);
}
void CG_ClearAnimStoreFrameList(skelAnimStoreFrameList_c *l) {
	int i;
	memset(l,0,sizeof(*l));
	for(i = 0; i < 64; i++) {
		l->m_blendInfo[i].pAnimationData = (struct skelAnimDataGameHeader_s*)0xbaadf00d;
	}
}
#ifndef __linux__
// this is windows-only ....
#include <windows.h>
void overwrite(int *vptr, int rep) {
	int hooked = 0;
	HANDLE process = GetCurrentProcess();
	DWORD protection = PAGE_READWRITE;
	DWORD oldProtection;
	if ( VirtualProtectEx( process, vptr, sizeof(int), protection, &oldProtection ) )
	{
			*vptr = rep;
#if 1
			if ( VirtualProtectEx( process, vptr, sizeof(int), oldProtection, &oldProtection ) )
					hooked = 1;
#endif
	}
}
#else
void overwrite(int *vptr, int rep) {
	//*vptr = rep;
}
#endif
void CG_SkelMatIdentity(SkelMat4_t *m) {
	// su44: that's an axis and a vector
	VectorSet(m->val[0],1,0,0);
	VectorSet(m->val[1],0,1,0);
	VectorSet(m->val[2],0,0,1);
	VectorSet(m->val[3],0,0,0);

}
const char *boneTypeStrings [] = {
	"JT_ROTATION",
	"JT_POSROT_SKC",
	"JT_IKSHOULDER",
	"JT_IKELBOW",
	"JT_IKWRIST",
	"JT_HOSEROT",
	"JT_AVROT",
	"JT_ZERO",
	"JT_NUMBONETYPES", 

	"JT_WORLD",
	"JT_HOSEROTBOTH",
	"JT_HOSEROTPARENT",	

};

// static vars, ugly but easy to debug
int index;
skelAnimStoreFrameList_c *l;
skelBone_Base_c *This;
skdJointType_t thisBoneType;
const char *thisBoneTypeString;
SkelMat4_t *r;
boneData_t *b;

// TEMPORARY function pointers,
// changed almost every time when something  
// from skelBone vtable is called !
void *getDirtyTransformMOH;
void *getChannelIndexMOH;
void *setBaseValMOH;
void *getBoneRefMOH;

// takes one argument (integer) and returns an integer, too
void __declspec(naked) getChannelIndex() {
	// get "this" pointer which is stored in ecx register
	__asm mov This, ecx; 
	// get our single argument from stack
	__asm
	{
		push eax
		mov eax, dword ptr [esp + 0x8]
		mov index, eax
		pop eax        
	}
	__asm pushad // save registers on the stack   
		//
		// print joint type
		//
		thisBoneType = (skdJointType_t)CG_Bone_GetType(This);
		// su44: this big 'if' block is only here so you can put
		// a breakpoint there to debug specific bone type
		if(thisBoneType == JT_ROTATION) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_POSROT_SKC) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_SHOULDER) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_ELBOW) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_WRIST) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_HOSEROT) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_HOSEROTBOTH) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_HOSEROTPARENT) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_AVROT) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else {
			cgi.Error(1,"getChannelIndex: Unknown boneType %i!\n",thisBoneType);
		}
		cgi.Printf("getChannelIndex: boneType %i, channelIndex %i\n", thisBoneType,index);
		//
		// select an appropriate virtual function 
		// from orig MoHAA vtables backup and call it
		//
		getChannelIndexMOH = mohVTBLs[thisBoneType].funcs[BONEFUNC_GETCHANNELINDEX];
	__asm popad // restore registers from the stack
	__asm
	{
		push index // push single argument
		call getChannelIndexMOH
		retn 4 // return dummy ???
	} //put parameters on stack and call the orginal function
}
// su44: This function is called only on bone
// initialization, so you may need to spawn
// a new model trough "spawn" console command
// to see results of your changes.
// The argument is a pointer to boneData_s struct.
void __declspec(naked) setBaseVal() {
	// get "this" pointer which is stored in ecx register
	__asm mov This, ecx; 
	// get our single argument from stack
	__asm
	{
		push eax
		mov eax, dword ptr [esp + 0x8]
		mov b, eax
		pop eax        
	}
	__asm pushad // save registers on the stack   
		//
		// print joint type
		//
		thisBoneType = (skdJointType_t)CG_Bone_GetType(This);
		// su44: this big 'if' block is only here so you can put
		// a breakpoint there to debug specific bone type
		if(thisBoneType == JT_ROTATION) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_POSROT_SKC) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_SHOULDER) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_ELBOW) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_WRIST) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_HOSEROT) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_HOSEROTBOTH) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_HOSEROTPARENT) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_AVROT) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else {
			cgi.Error(1,"setBaseVal: Unknown boneType %i!\n",thisBoneType);
		}
		// cgi.Printf("setBaseVal: boneType %i, boneData_s channel %i, type %i, parent %i\n",thisBoneType,b->channel,b->type,b->parent);

		// su44: this affects only human models legs
		// (so-called "ik-chains" made of SHOULDER, WRIST, and ELBOW bones)
		if(cg_skel_nullBoneLenghts->integer) {
			b->length = 0;
		}
		// su44: hmm, they look interesting with cg_skel_forceBoneLen == ~100.f
		// It seems that really some kind of IK [?] is applied to them
		if(cg_skel_forceBoneLen->integer != -1) {
			b->length = cg_skel_forceBoneLen->value;
		}
		/////////b->offset[2] = 100;
		//
		// select an appropriate virtual function 
		// from orig MoHAA vtables backup and call it
		//
		setBaseValMOH = mohVTBLs[thisBoneType].funcs[1];
	__asm popad // restore registers from the stack
	__asm
	{
		push b // push single argument
		call setBaseValMOH
		retn 4 // return dummy ???
	} //put parameters on stack and call the orginal function
}
// su44: it doesnt seem to be called for posrot bones
void __declspec(naked) getBoneRef() {
	// get "this" pointer which is stored in ecx register
	__asm mov This, ecx; 
	__asm pushad // save registers on the stack   
		//
		// print joint type
		//
		thisBoneType = (skdJointType_t)CG_Bone_GetType(This);
		// su44: this big 'if' block is only here so you can put
		// a breakpoint there to debug specific bone type
		if(thisBoneType == JT_ROTATION) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_POSROT_SKC) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_SHOULDER) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_ELBOW) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_WRIST) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_HOSEROT) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_HOSEROTBOTH) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_HOSEROTPARENT) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_AVROT) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else {
			cgi.Error(1,"getBoneRef: Unknown boneType %i!\n",thisBoneType);
		}
		// cgi.Printf("getBoneRef: boneType %i, boneData_s channel %i, type %i, parent %i\n",thisBoneType,b->channel,b->type,b->parent);
		//
		// select an appropriate virtual function 
		// from orig MoHAA vtables backup and call it
		//
		getBoneRefMOH = mohVTBLs[thisBoneType].funcs[BONEFUNC_GETBONEREF];
	__asm popad // restore registers from the stack
	__asm
	{
		call getBoneRefMOH
		retn 4 // return 4 byte pointer to bone ref
	} //put parameters on stack and call the orginal function
}


// su44: since getDirtyTransform is called recursively,
// I decided to keep a track of its bone arguments

typedef struct gdCallInfo_s {
	const char *typeStr;
	skdJointType_t type;
	skelBone_Base_c *ptr;
} gdCallInfo_t;

gdCallInfo_t boneStack[64];
gdCallInfo_t *top = boneStack;
int numBonesOnStack = 0;
int maxNumBonesOnStack = 0;

// su44: getDirtyTransform should return &this->m_cachedValue
// This might be called trought cgi.TIKI_Orientation,
// cgi.TIKI_SetEyeTargetPos or cgi.Anim_Time (why ?)
// (see our cg_tiki.c for detours of those functions)
void __declspec(naked) getDirtyTransform() {
	// get "this" pointer which is stored in ecx register
	__asm mov This, ecx; 
	// get our single argument from stack
	__asm
	{
		push eax
		mov eax, dword ptr [esp + 0x8]
		mov l, eax
		pop eax        
	}
	__asm pushad // save registers on the stack  
		//
		// print joint type
		//
		thisBoneType = (skdJointType_t)CG_Bone_GetType(This);
		// su44: this big 'if' block is only here so you can put
		// a breakpoint there to debug specific bone type
		if(thisBoneType == JT_ROTATION) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_POSROT_SKC) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_SHOULDER) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_ELBOW) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_WRIST) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_HOSEROT) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_HOSEROTBOTH) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_HOSEROTPARENT) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else if(thisBoneType == JT_AVROT) {
			thisBoneTypeString = boneTypeStrings[thisBoneType];
		} else {
			cgi.Error(1,"getDirtyTransform: Unknown boneType %i!\n",thisBoneType);
		}
		//cgi.Printf("getDirtyTransform: boneType %i, skelAnimStoreFrameList actionWeight %f\n",thisBoneType,l->actionWeight);
		//
		// select an appropriate virtual function 
		// from orig MoHAA vtables backup and call it
		//
		getDirtyTransformMOH = mohVTBLs[thisBoneType].funcs[0];

		// add bone to stack....
		// NOTE: numBonesOnStack usually reaches 8, 
		// but sometimes its even 9
		if(numBonesOnStack < 64) {
			top->typeStr = thisBoneTypeString;
			top->type = thisBoneType;
			top->ptr = This;
			top++;
			numBonesOnStack++;
			if(maxNumBonesOnStack < numBonesOnStack) {
				maxNumBonesOnStack = numBonesOnStack;
			}
		}

	__asm popad // restore registers from the stack
#if 0
	// just call original getDirtyTransform func
	// and return the result
	__asm
	{
		push l // push single argument
		call getDirtyTransformMOH
		retn 4 // return 4 byte pointer to m_cachedValue
	} //put parameters on stack and call the orginal function
#else
	// call original getDirtyTransformMOH
	// and retrieve the result,
	// so we can use it on our own
	__asm
	{
		push l // push single argument
		call getDirtyTransformMOH
	} //put parameters on stack and call the orginal function
	__asm
	{
		push eax
		mov r, eax
		pop eax        
	}
	__asm pushad // save registers on the stack 
	//
	//if(thisBoneType == JT_ROTATION) {
	//	CG_SkelMatIdentity(r);
	//}
	//

	// pop this bone from stack
	if(numBonesOnStack > 0) {
		top->typeStr = "bad";
		top->type = -1;
		top->ptr = 0;
		top--;
		numBonesOnStack--;
	}

	__asm popad // save registers on the stack  
	__asm 
	{
		retn 4
	}
#endif
}

// su44: This is called for *every* entity with valid TIKI and skel pointers
qboolean CG_TIKISkeletor(refEntity_t *ent, skeletor_c *skel) {
	vec3_t end;
	int i, j;

	CG_Skel_ExtractVPTRs(skel);

	if(cg_skel_show->integer) {
		CG_MakeCross( ent->origin );
	}

	if(cg_skel_printBoneDirty->integer) {
		for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
			skelBone_Base_c *b = skel->m_bone[i];

			cgi.Printf("Bone %i of %i, name %s, model %s dirty %i\n",
				i,skel->m_Tiki->m_boneList.m_numChannels,cgi.Tag_NameForNum(skel->m_Tiki,i),
				ent->tiki->name,b->m_isDirty);
		}	
	}

	if(cg_skel_printBonePos->integer) {
		for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
			skelBone_Base_c *b = skel->m_bone[i];

			cgi.Printf("Bone %i of %i, name %s, model %s pos %f %f %f\n",
				i,skel->m_Tiki->m_boneList.m_numChannels,cgi.Tag_NameForNum(skel->m_Tiki,i),
				ent->tiki->name,b->m_cachedValue.val[3][0],b->m_cachedValue.val[3][1],b->m_cachedValue.val[3][2]);
		}	
	}
	if(cg_skel_printHoseParms->integer && vPtrsExtracted) {
		for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
			skelBone_Base_c *b = skel->m_bone[i];

			if(CG_Bone_IsHoseRot(b)) {
				skelBone_HoseRot_c *h;

				h = (skelBone_HoseRot_c*)b;

				cgi.Printf("Bone %i of %i is a hoserot, name %s, model %s bendMax %f bendRatio %f spinRatio %f\n",
					i,skel->m_Tiki->m_boneList.m_numChannels,cgi.Tag_NameForNum(skel->m_Tiki,i),
					ent->tiki->name,
					h->m_bendMax,h->m_bendRatio,h->m_spinRatio
				);
			}
		}	
	}


	if(cg_skel_printIKChainParms->integer && vPtrsExtracted) {
		for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
			skelBone_Base_c *b = skel->m_bone[i];

			if(CG_Bone_IsElbow(b)) {
				CG_PrintfBone_Elbow(b,skel,i);
			} else if(CG_Bone_IsShoulder(b)) {
				CG_PrintfBone_Shoulder(b,skel,i);
			} else if(CG_Bone_IsWrist(b)) {
				CG_PrintfBone_Wrist(b,skel,i);
			}
		}	
	}


	if(cg_skel_drawBones->integer) {
		matrix_t m;
		matrix_t mb[128];
		MatrixSetupTransformFromVectorsFLU(m,ent->axis[0],ent->axis[1],ent->axis[2],ent->origin);
		for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
			skelBone_Base_c *b;
			matrix_t bm;

			b = skel->m_bone[i];
			MatrixSetupTransformFromVectorsFLU(bm,b->m_cachedValue.val[0],
				b->m_cachedValue.val[1],b->m_cachedValue.val[2],b->m_cachedValue.val[3]);
			
			Matrix4x4Multiply(m,bm,mb[i]);
		}	
		for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
			vec3_t p;
			vec3_t axis[3];
			skelBone_Base_c *b;
			int parent;

			b = skel->m_bone[i];
			
			MatrixToVectorsFLU(mb[i],axis[0],axis[1],axis[2]);
			CG_DrawAxisAtPoint(&mb[i][12],axis);

			if(b->m_parent) {


				parent = CG_Skel_BoneIndexForPointer(b->m_parent,skel);

				cgi.R_DebugLine( &mb[i][12], &mb[parent][12], 1, 1, 1, 1 );

				
			}
		//	
		}	
		return qfalse;
	}
	if(cg_skel_hookBoneVTable->integer && vPtrsExtracted) {
		// hook vTables of all know boneTypes
		skelBoneVTable_t *t;
		for(i = 0; i < 16; i++) {
			t = boneVPtrs[i];
			if(t == 0)
				continue;
			overwrite(&t->funcs[0],getDirtyTransform);
			overwrite(&t->funcs[1],setBaseVal);
			overwrite(&t->funcs[2],getChannelIndex);
			overwrite(&t->funcs[3],getBoneRef);	
		}
		cg_skel_hookBoneVTable->integer = 0;
	}
	if(cg_skel_su44->integer && vPtrsExtracted) {
#if 1
		for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
			//GetTransformFunc GetDirtyTransform;
			//skelBone_Base_c *b;
			//int **v;
			//SkelMat4_t *m;
			//skelAnimStoreFrameList_c dummy;
			////void (*test) ();

			//CG_ClearAnimStoreFrameList(&dummy);

			//// access bone virtual function table
			//b = skel->m_bone[i];

			//if(CG_Bone_IsPosRot(b) == qfalse)
			//	continue;
			//v[0] = getDirtyTransform;
//#if 1
//			// hook getDirtyTransform
//			getDirtyTransformMOH = b->vptr->funcs[0];
//			overwrite(&b->vptr->funcs[0],getDirtyTransform);
//
//			// hook setBaseVal
//			setBaseValMOH = b->vptr->funcs[1];
//			overwrite(&b->vptr->funcs[1],setBaseVal);
//
//			// hook getChannelIndex
//			getChannelIndexMOH = b->vptr->funcs[2];
//			overwrite(&b->vptr->funcs[2],getChannelIndex);
//
//			// hook getBoneRef
//			getBoneRefMOH = b->vptr->funcs[3];
//			overwrite(&b->vptr->funcs[3],getBoneRef);
//
//#else
//			//overwrite(&b->vptr->funcs[3],getBoneRef);
//			overwrite(&b->vptr->funcs[1],(int)setBaseVal);
//
//
//
//#endif
//			cg_skel_su44->integer = 0;
//			break;
			//test = v[0];

			//	test();

			//	for(j = 0; j < 10; j++) {
			//		cgi.Printf("vptrBase %i index %i ofs %i\n",b->vptr,j,v[j]);
			//	}

			//	j = 2;
			//	GetDirtyTransform = v[j];
			////	m = GetDirtyTransform(b,&skel->m_frameList);

			///cgi.Printf("t.");
		}	
#else
		if(vPtrsExtracted) {
			for(i = 0; i < skel->m_Tiki->m_boneList.m_numChannels; i++) {
				skelBone_Base_c *b = skel->m_bone[i];

				if(CG_Bone_IsHoseRot(b)) {
					skelBone_HoseRot_c *h;

					h = (skelBone_HoseRot_c*)b;

				//	h->m_bendMax = 90;
				//	h->m_bendRatio = 100;
					h->m_spinRatio = 1;
				}
			}	
		}
#endif
	}
	return qtrue;

}

// su44: we need to access skeletor_c class somehow
void R_AddRefEntityToScene(refEntity_t *ent) {
	skeletor_c *skel;
	
	if(ent->tiki) {
		// su44: to get entity skeletor pointer, we need to know
		// its TIKI pointer and entityNumber (the one send 
		// through entityState_t). Bad things happen if TIKI
		// pointer passed to TIKI_GetSkeletor is NULL.
		skel = cgi.TIKI_GetSkeletor(ent->tiki,ent->entityNumber);
		// got it
		if(CG_TIKISkeletor(ent,skel) == qfalse)
			return; // dont add it to scene
	} else {
		skel = 0;
	}

	cgi.R_AddRefEntityToScene(ent);
}


