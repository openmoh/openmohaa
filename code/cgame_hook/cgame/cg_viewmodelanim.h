#ifndef __VIEWMODELANIM_H__
#define __VIEWMODELANIM_H__

typedef struct vma_s {
	str				name;
	float			speed;
} vma_t;

void CG_AddViewModelPrefix( const char * weapon_name, const char * prefix );
void __stdcall CG_CalcViewBob( float value );
void CG_ProcessDeltaAngles( void );
void CG_OffsetFirstPersonView( refEntity_t *model, int a2 );
void CG_OffsetThirdPersonView();
void CG_OffsetThirdPersonViewReal();
void CG_ModelAnim( centity_t *ent, int un1 );
void CG_GetViewModelAnimName( char * Dest, char * Format, char * prefix, char * anim_name );
char * CG_GetViewModelAnimPrefix();
const char *CG_GetCurrentWeaponName( void );
const char *CG_GetCurrentWeaponPrefix( void );
vma_t *CG_GetVMA( const char *name );
//int CG_GetVMAnimPrefixString(char * weapon_name);
void CG_HeightCheck( void );
void CG_MatchEyesBone( refEntity_t * model );
void CG_MoveWeapon( int frametime );
void CG_SetVMASpeed( const char *vma, float speed );
void CG_SetViewHeight( int newHeight );
void CG_ViewInit( void );
void CG_ViewModelAnim( char *name, qboolean bFullAnim );
//char * CG_ViewModelAnimCheck();
void CG_ViewModelAnimCheck( clientAnim_t * anim );
void CG_ViewModelUninitialize( void );

extern uintptr_t CG_OffsetFirstPersonView_f;
extern uintptr_t CG_OffsetThirdPersonView_f;
extern uintptr_t CG_ModelAnim_f;

extern vec3_t currentDeltaVMPosOffset;

extern vec3_t oldAngles;
extern vec3_t deltaAngles;

extern double bobMovementMultiplier;

extern int internal_bobOffset;

extern float max_z;
extern int viewHeight ;
extern qboolean modheightfloat_used ;

extern char vm_anim_name[ MAX_STRING_CHARS ];
extern qboolean vm_fullanim;

extern float vm_time;

#endif
