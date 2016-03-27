#ifndef __CG_HUD_H__
#define __CG_HUD_H__

#include "hud.h"

#ifdef __cplusplus
extern "C" {
#endif

extern cvar_t *stats;
extern cvar_t *menu;

extern uidef_t *uid; 
extern qkey_t *keys;

extern fontHeader_t *facfont20;
extern fontHeader_t *courier16;
extern fontHeader_t *courier18;
extern fontHeader_t *courier20;
extern fontHeader_t *handle16;
extern fontHeader_t *handle18;
extern fontHeader_t *marlett;
extern fontHeader_t *verdana12;
extern fontHeader_t *verdana14;

void CG_Draw2D();
void CG_ShowHint( const char * hint );

typedef struct hdTimer_s
{
	qboolean bEnabled;

	float duration;
	float current;
	float fade_out_time;
	float alpha;
	char string[ MAX_STRING_CHARS ];
} hdTimer_t;

typedef struct hd3d_s
{
	qboolean bEnabled;
	qboolean bAlwaysShow;
	qboolean bDepth;

	vec3_t vector;
	int ent_num;
	vec3_t currentOrg;

	int x, y;
} hd3d_t;

typedef struct hdExtend_s
{
	struct hdTimer_s timer;
	struct hd3d_s h3d;
} hdExtend_t;

extern hdExtend_t *huddrawExtends;

#ifdef __cplusplus
}
#endif

#endif
