/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

// DESCRIPTION:
// Renderer types

#ifndef __TR_TYPES_H
#define __TR_TYPES_H


#define	MAX_DLIGHTS		32			// can't be increased, because bit flags are used on surfaces
#define	MAX_ENTITIES	1023		// can't be increased without changing drawsurf bit packing
#define  MAX_POINTS     32
#define  MAX_SPRITES    1024

// refdef flags
#define RDF_NOWORLDMODEL	1		// used for player configuration screen
#define RDF_HYPERSPACE		4		// teleportation effect

typedef struct
   {
   float pos[2];
   float size[2];
   } letterloc_t;

typedef struct fontheader_s
   {
   int               indirection[256];
   letterloc_t       locations[256];
   char              name[64];
   float             height;
   float             aspectRatio;
   void             *shader;
   int               trhandle; // the last renderer handle this font used
   } fontheader_t;

typedef struct {
	vec3_t		xyz;
	float		st[2];
	byte		modulate[4];
} polyVert_t;

typedef struct poly_s {
	qhandle_t			hShader;
	int					numVerts;
	polyVert_t			*verts;
} poly_t;

typedef enum {
	RT_MODEL,
	RT_SPRITE,
	RT_BEAM,
	RT_PORTALSURFACE,		// doesn't draw anything, just info for portals

	RT_MAX_REF_ENTITY_TYPE
} refEntityType_t;

typedef enum {
   lensflare = ( 1 << 0 ),
   viewlensflare = ( 1 << 1 ),
   additive = ( 1 << 2 )
} dlighttype_t;

struct tikiFrame_s;
struct dtiki_s;

typedef struct {
	refEntityType_t	reType;

	int			renderfx;

	qhandle_t	hModel;				         // opaque type outside refresh
	qhandle_t	hOldModel;
                                          
	// most recent data                    
	vec3_t		lightingOrigin;		      // so multi-part models can be lit identically (RF_LIGHTING_ORIGIN)
	int			parentEntity;
                                          
	vec3_t		axis[3];			            // rotation vectors
	qboolean	   nonNormalizedAxes;	      // axis are not normalized, i.e. they have scale
	float		   origin[3];			         // also used as MODEL_BEAM's "from"
	frameInfo_t	   frameInfo[MAX_FRAMEINFOS];
	float       actionWeight;
	short       wasframe;
   float       scale;                     // scale of the thing
                                         
   // previous data for frame interpolation
	float		   oldorigin[3];		         // also used as MODEL_BEAM's "to"

	// texturing
	int			skinNum;			            // inline skin index
	qhandle_t	customShader;		         // use one image for the entire thing
                                          
	// misc                                
	byte		   shaderRGBA[4];		         // colors used by colorSrc=vertex shaders
	float		   shaderTexCoord[2];	      // texture coordinates used by tcMod=vertex modifiers
	float		   shaderTime;			// subtracted from refdef time to control effect start times
                                          
   int         entityNumber;              // the real entity number

   byte        surfaces[MAX_MODEL_SURFACES]; // the surface state of the entity
   float       shader_data[ 2 ];          // data passed in from shader manipulation

   int         *bone_tag;
   vec4_t      *bone_quat;

   // renderer use only
   struct tikiFrame_s   *of,
                        *nf;
   struct dtiki_s       *tiki;
   int         bonestart;
   int         morphstart;
   qboolean    hasMorph;

	// extra sprite information
	float		   radius;		
	float		   rotation;

} refEntity_t;

#define	MAX_RENDER_STRINGS			8
#define	MAX_RENDER_STRING_LENGTH	32

typedef struct {
	int			x, y, width, height;
	float		fov_x, fov_y;
	vec3_t		vieworg;
	vec3_t		viewaxis[3];		// transformation matrix

	// time in milliseconds for shader effects and other time dependent rendering issues
	int			time;

	int			rdflags;			// RDF_NOWORLDMODEL, etc

	// 1 bits will prevent the associated area from rendering at all
	byte		areamask[MAX_MAP_AREA_BYTES];

   // fog stuff
   float       farplane_distance;
   vec3_t      farplane_color;
   qboolean    farplane_cull;

   // sky portal stuff
   qboolean    sky_portal;
   float       sky_alpha;
   vec3_t      sky_origin;
   vec3_t      sky_axis[ 3 ];

} refdef_t;


typedef enum {
	STEREO_CENTER,
	STEREO_LEFT,
	STEREO_RIGHT
} stereoFrame_t;


/*
** glconfig_t
**
** Contains variables specific to the OpenGL configuration
** being run right now.  These are constant once the OpenGL
** subsystem is initialized.
*/
typedef enum {
    TC_NONE,
    TC_S3TC,  // this is for the GL_S3_s3tc extension.
    TC_S3TC_ARB  // this is for the GL_EXT_texture_compression_s3tc extension.
} textureCompression_t;

typedef enum {
	GLDRV_ICD,					// driver is integrated with window system
								// WARNING: there are tests that check for
								// > GLDRV_ICD for minidriverness, so this
								// should always be the lowest value in this
								// enum set
	GLDRV_STANDALONE,			// driver is a non-3Dfx standalone driver
	GLDRV_VOODOO,				// driver is a 3Dfx standalone driver
        // XreaL BEGIN
    GLDRV_DEFAULT,				// old OpenGL system
    GLDRV_OPENGL3,				// new driver system
    GLDRV_MESA					// crap
    // XreaL END
} glDriverType_t;

typedef enum {
	GLHW_GENERIC,			// where everthing works the way it should
	GLHW_3DFX_2D3D,			// Voodoo Banshee or Voodoo3, relevant since if this is
							// the hardware type then there can NOT exist a secondary
							// display adapter
	GLHW_RIVA128,			// where you can't interpolate alpha
	GLHW_RAGEPRO,			// where you can't modulate alpha on alpha textures
	GLHW_PERMEDIA2,			// where you don't have src*dst
	// XreaL BEGIN
    GLHW_ATI,					// where you don't have proper GLSL support
    GLHW_ATI_DX10,				// ATI Radeon HD series DX10 hardware
    GLHW_NV_DX10				// Geforce 8/9 class DX10 hardware
    // XreaL END
} glHardwareType_t;


typedef struct {
	char					renderer_string[ MAX_STRING_CHARS ];
	char					vendor_string[ MAX_STRING_CHARS ];
	char					version_string[ MAX_STRING_CHARS ];
	char					extensions_string[ MAX_STRING_CHARS * 2 ];

	int						maxTextureSize;			// queried from GL
	int						maxActiveTextures;		// multitexture ability

	int						colorBits, depthBits, stencilBits;

	glDriverType_t			driverType;
	glHardwareType_t		hardwareType;

	qboolean				deviceSupportsGamma;
	textureCompression_t	textureCompression;
	qboolean				textureEnvAddAvailable;

	int						vidWidth, vidHeight;
	// aspect is the screen's physical width / height, which may be different
	// than scrWidth / scrHeight if the pixels are non-square
	// normal screens should be 4/3, but wide aspect monitors may be 16/9
	float					windowAspect;

	int						displayFrequency;

	// synonymous with "does rendering consume the entire screen?", therefore
	// a Voodoo or Voodoo2 will have this set to TRUE, as will a Win32 ICD that
	// used CDS.
	qboolean				isFullscreen;
	qboolean				stereoEnabled;
	qboolean				smpActive;		// dual processor
    int						registerCombinerAvailable;
    qboolean				secondaryColorAvailable;
    qboolean				VAR;
    qboolean				fence;
    // ioq3
    int                     numTextureUnits;
} glconfig_t;

#if !defined _WIN32

#define _3DFX_DRIVER_NAME	"libMesaVoodooGL.so.3.1"
#define OPENGL_DRIVER_NAME	"libGL.so"

#else

#define _3DFX_DRIVER_NAME	"3dfxvgl"
#define OPENGL_DRIVER_NAME	"opengl32"

#endif	// !defined _WIN32


#endif	// __TR_TYPES_H
