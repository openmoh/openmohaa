#pragma once

#include "ida_common.h"

typedef struct {
  vec3_t i_vBarrel;
  vec3_t i_vStart;
  vec3_t *i_vEnd;
  int i_iNumBullets;
  qboolean iLarge;
  int iTracerVisible;
  qboolean bIgnoreEntities;
} bullet_tracer_t;

typedef struct Event2_s Event_CGAMEDLL;

#define MAX_TRACERS 32
#define MAX_BULLETS 1024
#define MAX_IMPACTS 64
