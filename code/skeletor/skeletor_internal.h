/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

This file is part of OpenMoHAA source code.

OpenMoHAA source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMoHAA source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMoHAA source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// skeletor_internal.h : General skeletor internal structures

#ifndef __SKELETOR_INTERNAL__
#define __SKELETOR_INTERNAL__

#ifdef __cplusplus

class skelAnimStoreFrameList_c;

class skelBone_Base {
public:
	qboolean		m_isDirty;

protected:
	skelBone_Base	*m_parent;
	SkelMat4		m_cachedValue;

public:
	float			*m_controller;

public:
	skelBone_Base();

	SkelMat4				GetTransform( const skelAnimStoreFrameList_c *frames );
	virtual SkelMat4		GetDirtyTransform( const skelAnimStoreFrameList_c *frames );
	void					SetParent( skelBone_Base *parent );
	virtual void			SetBaseValue( boneData_t *boneData );
	virtual int				GetChannelIndex( int num );
	virtual skelBone_Base	*GetBoneRef( int num );
	static int				GetNumChannels( boneType_t boneType );
	static int				GetNumBoneRefs( boneType_t boneType );
	skelBone_Base			*Parent() const;
	bool					OnGround( const skelAnimStoreFrameList_c *frames, float threshold );
};

class skelBone_World : public skelBone_Base {
public:
	skelBone_World();

private:
	SkelMat4		GetDirtyTransform( const skelAnimStoreFrameList_c *frames );
	void			SetBaseValue( boneData_t *boneData );
	int				GetChannelIndex( int num );
	skelBone_Base	*GetBoneRef( int num );
};

class skelBone_Zero : public skelBone_Base {
private:
	SkelMat4			GetDirtyTransform( const skelAnimStoreFrameList_c *frames );
	void				SetBaseValue( boneData_t *boneData );
	int					GetChannelIndex( int num );
	skelBone_Base		*GetBoneRef( int num );
};

class skelBone_Rotation : public skelBone_Base {
protected:
	SkelVec3 m_baseValue;
	int m_quatChannel;

protected:
	SkelMat4		GetDirtyTransform( const skelAnimStoreFrameList_c *frames );

public:
	void			SetChannels( int num );
	void			SetBaseValue( boneData_t *data );
	int				GetChannelIndex( int num );
	skelBone_Base	*GetBoneRef( int num );
};

class skelBone_PosRot : public skelBone_Base {
protected:
	int m_quatChannel;
	int m_offsetChannel;

public:
protected:
	SkelMat4		GetDirtyTransform( const skelAnimStoreFrameList_c *frames );
public:
	void			SetChannels( int quatChannel, int offsetChannel );
	void			SetBaseValue( boneData_t *boneData );
	int				GetChannelIndex( int num );
	skelBone_Base	*GetBoneRef( int num );
};

class skelBone_Root : public skelBone_PosRot {
public:
	SkelMat4	GetDirtyTransform( const skelAnimStoreFrameList_c *frames );
};

class skelBone_IKshoulder : public skelBone_Base {
public:
	class skelBone_IKwrist *m_wrist;
	float m_upperLength;
	float m_lowerLength;
	SkelVec3 m_baseValue;
	float m_cosElbowAngle;
	SkelVec3 m_wristPos;
	SkelQuat m_wristAngle;

	skelBone_IKshoulder();

	SkelMat4		GetDirtyTransform( const skelAnimStoreFrameList_c *frames );
	void			SetBaseValue( boneData_t *boneData );
	int				GetChannelIndex( int num );
	skelBone_Base	*GetBoneRef( int num );
	void			SetElbowValue( float elbowOffset );
	void			SetWristValue( float wristOffset );
	void			SetWristBone( skelBone_IKwrist *wrist );
	float			GetUpperLength();
	float			GetLowerLength();
};

class skelBone_IKelbow : public skelBone_Base {
public:
	skelBone_IKshoulder *m_shoulder;

public:
	SkelMat4		GetDirtyTransform( const skelAnimStoreFrameList_c *frames );
	void			SetBoneRefs( skelBone_IKshoulder *shoulder );
	void			SetBaseValue( boneData_t *boneData );
	int				GetChannelIndex( int num );
	skelBone_Base	*GetBoneRef( int num );
};

class skelBone_IKwrist : public skelBone_Base {
public:
	skelBone_IKshoulder *m_shoulder;
	int m_quatChannel;
	int m_offsetChannel;

public:
	SkelMat4		GetDirtyTransform( const skelAnimStoreFrameList_c *frame );
	void			SetChannels( int quatChannel, int offsetChannel );
	void			SetBoneRefs( skelBone_IKshoulder *shoulder );
	void			SetBaseValue( boneData_t *boneData );
	int				GetChannelIndex( int num );
	skelBone_Base	*GetBoneRef( int num );
};

class skelBone_AvRot : public skelBone_Base {
public:
	SkelVec3 m_basePos;
	SkelQuat m_cachedQuat;
	skelBone_Base *m_reference1;
	skelBone_Base *m_reference2;
	float m_bone2weight;

public:
	skelBone_AvRot();

	SkelMat4		GetDirtyTransform( const skelAnimStoreFrameList_c *frames );
	void			SetBoneRefs( skelBone_Base *ref1, skelBone_Base *ref2 );
	void			SetBaseValue( boneData_t *boneData );
	int				GetChannelIndex( int num );
	skelBone_Base	*GetBoneRef( int num );
};

class skelBone_HoseRot : public skelBone_Base {
public:
	SkelVec3 m_basePos;
	SkelQuat m_cachedQuat;
	skelBone_Base *m_target;
	float m_bendRatio;
	float m_bendMax;
	float m_spinRatio;

public:
	SkelMat4		GetDirtyTransform( const skelAnimStoreFrameList_c *frames );
	SkelMat4		GetDirtyTransform( SkelMat4& myParentTM, SkelMat4& targetTM );
	void			SetBoneRefs( skelBone_Base *ref );
	void			SetBaseValue( boneData_t *boneData );
	int				GetChannelIndex( int num );
	skelBone_Base	*GetBoneRef( int num );
};

class skelBone_HoseRotBoth : public skelBone_HoseRot {
public:
	SkelMat4		GetDirtyTransform( const skelAnimStoreFrameList_c *frames );
	void			SetBaseValue( boneData_t *boneData );
};

class skelBone_HoseRotParent : public skelBone_HoseRot {
public:
	SkelMat4	GetDirtyTransform( const skelAnimStoreFrameList_c *frames );
	void		SetBaseValue( boneData_t *boneData );
};

#else

typedef void *skelBone_Base;

#endif

typedef char skelChannelName_t[ 32 ];

#endif // __SKELETOR_INTERNAL__
