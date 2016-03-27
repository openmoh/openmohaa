/***************************************************************************
* Copyright (C) 2012, Chain Studios.
* 
* This file is part of QeffectsGL source code.
* 
* QeffectsGL source code is free software; you can redistribute it 
* and/or modify it under the terms of the GNU General Public License 
* as published by the Free Software Foundation; either version 2 of 
* the License, or (at your option) any later version.
* 
* QeffectsGL source code is distributed in the hope that it will be 
* useful, but WITHOUT ANY WARRANTY; without even the implied 
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software 
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
***************************************************************************/
#ifndef QFX_OPENGL_H
#define QFX_OPENGL_H

/*
 * OpenGL Datatypes
 */
typedef unsigned int	GLenum;
typedef unsigned char	GLboolean;
typedef unsigned int	GLbitfield;
typedef void			GLvoid;
typedef signed char		GLbyte;		/* 1-byte signed */
typedef short			GLshort;	/* 2-byte signed */
typedef int				GLint;		/* 4-byte signed */
typedef unsigned char	GLubyte;	/* 1-byte unsigned */
typedef unsigned short	GLushort;	/* 2-byte unsigned */
typedef unsigned int	GLuint;		/* 4-byte unsigned */
typedef int				GLsizei;	/* 4-byte signed */
typedef float			GLfloat;	/* single precision float */
typedef float			GLclampf;	/* single precision float in [0,1] */
typedef double			GLdouble;	/* double precision float */
typedef double			GLclampd;	/* double precision float in [0,1] */
typedef char			GLchar;

/* Version */
#define GL_VERSION_1_1                    1

/* AccumOp */
#define GL_ACCUM                          0x0100
#define GL_LOAD                           0x0101
#define GL_RETURN                         0x0102
#define GL_MULT                           0x0103
#define GL_ADD                            0x0104

/* AlphaFunction */
#define GL_NEVER                          0x0200
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207

/* AttribMask */
#define GL_CURRENT_BIT                    0x00000001
#define GL_POINT_BIT                      0x00000002
#define GL_LINE_BIT                       0x00000004
#define GL_POLYGON_BIT                    0x00000008
#define GL_POLYGON_STIPPLE_BIT            0x00000010
#define GL_PIXEL_MODE_BIT                 0x00000020
#define GL_LIGHTING_BIT                   0x00000040
#define GL_FOG_BIT                        0x00000080
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_ACCUM_BUFFER_BIT               0x00000200
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_VIEWPORT_BIT                   0x00000800
#define GL_TRANSFORM_BIT                  0x00001000
#define GL_ENABLE_BIT                     0x00002000
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_HINT_BIT                       0x00008000
#define GL_EVAL_BIT                       0x00010000
#define GL_LIST_BIT                       0x00020000
#define GL_TEXTURE_BIT                    0x00040000
#define GL_SCISSOR_BIT                    0x00080000
#define GL_ALL_ATTRIB_BITS                0x000fffff

/* BeginMode */
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006
#define GL_QUADS                          0x0007
#define GL_QUAD_STRIP                     0x0008
#define GL_POLYGON                        0x0009

/* BlendingFactorDest */
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305

/* BlendingFactorSrc */
/*      GL_ZERO */
/*      GL_ONE */
#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_SRC_ALPHA_SATURATE             0x0308
/*      GL_SRC_ALPHA */
/*      GL_ONE_MINUS_SRC_ALPHA */
/*      GL_DST_ALPHA */
/*      GL_ONE_MINUS_DST_ALPHA */

/* Boolean */
#define GL_TRUE                           1
#define GL_FALSE                          0

/* ClearBufferMask */
/*      GL_COLOR_BUFFER_BIT */
/*      GL_ACCUM_BUFFER_BIT */
/*      GL_STENCIL_BUFFER_BIT */
/*      GL_DEPTH_BUFFER_BIT */

/* ClientArrayType */
/*      GL_VERTEX_ARRAY */
/*      GL_NORMAL_ARRAY */
/*      GL_COLOR_ARRAY */
/*      GL_INDEX_ARRAY */
/*      GL_TEXTURE_COORD_ARRAY */
/*      GL_EDGE_FLAG_ARRAY */

/* ClipPlaneName */
#define GL_CLIP_PLANE0                    0x3000
#define GL_CLIP_PLANE1                    0x3001
#define GL_CLIP_PLANE2                    0x3002
#define GL_CLIP_PLANE3                    0x3003
#define GL_CLIP_PLANE4                    0x3004
#define GL_CLIP_PLANE5                    0x3005

/* ColorMaterialFace */
/*      GL_FRONT */
/*      GL_BACK */
/*      GL_FRONT_AND_BACK */

/* ColorMaterialParameter */
/*      GL_AMBIENT */
/*      GL_DIFFUSE */
/*      GL_SPECULAR */
/*      GL_EMISSION */
/*      GL_AMBIENT_AND_DIFFUSE */

/* ColorPointerType */
/*      GL_BYTE */
/*      GL_UNSIGNED_BYTE */
/*      GL_SHORT */
/*      GL_UNSIGNED_SHORT */
/*      GL_INT */
/*      GL_UNSIGNED_INT */
/*      GL_FLOAT */
/*      GL_DOUBLE */

/* CullFaceMode */
/*      GL_FRONT */
/*      GL_BACK */
/*      GL_FRONT_AND_BACK */

/* DataType */
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_2_BYTES                        0x1407
#define GL_3_BYTES                        0x1408
#define GL_4_BYTES                        0x1409
#define GL_DOUBLE                         0x140A

/* DepthFunction */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* DrawBufferMode */
#define GL_NONE                           0
#define GL_FRONT_LEFT                     0x0400
#define GL_FRONT_RIGHT                    0x0401
#define GL_BACK_LEFT                      0x0402
#define GL_BACK_RIGHT                     0x0403
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_LEFT                           0x0406
#define GL_RIGHT                          0x0407
#define GL_FRONT_AND_BACK                 0x0408
#define GL_AUX0                           0x0409
#define GL_AUX1                           0x040A
#define GL_AUX2                           0x040B
#define GL_AUX3                           0x040C

/* Enable */
/*      GL_FOG */
/*      GL_LIGHTING */
/*      GL_TEXTURE_1D */
/*      GL_TEXTURE_2D */
/*      GL_LINE_STIPPLE */
/*      GL_POLYGON_STIPPLE */
/*      GL_CULL_FACE */
/*      GL_ALPHA_TEST */
/*      GL_BLEND */
/*      GL_INDEX_LOGIC_OP */
/*      GL_COLOR_LOGIC_OP */
/*      GL_DITHER */
/*      GL_STENCIL_TEST */
/*      GL_DEPTH_TEST */
/*      GL_CLIP_PLANE0 */
/*      GL_CLIP_PLANE1 */
/*      GL_CLIP_PLANE2 */
/*      GL_CLIP_PLANE3 */
/*      GL_CLIP_PLANE4 */
/*      GL_CLIP_PLANE5 */
/*      GL_LIGHT0 */
/*      GL_LIGHT1 */
/*      GL_LIGHT2 */
/*      GL_LIGHT3 */
/*      GL_LIGHT4 */
/*      GL_LIGHT5 */
/*      GL_LIGHT6 */
/*      GL_LIGHT7 */
/*      GL_TEXTURE_GEN_S */
/*      GL_TEXTURE_GEN_T */
/*      GL_TEXTURE_GEN_R */
/*      GL_TEXTURE_GEN_Q */
/*      GL_MAP1_VERTEX_3 */
/*      GL_MAP1_VERTEX_4 */
/*      GL_MAP1_COLOR_4 */
/*      GL_MAP1_INDEX */
/*      GL_MAP1_NORMAL */
/*      GL_MAP1_TEXTURE_COORD_1 */
/*      GL_MAP1_TEXTURE_COORD_2 */
/*      GL_MAP1_TEXTURE_COORD_3 */
/*      GL_MAP1_TEXTURE_COORD_4 */
/*      GL_MAP2_VERTEX_3 */
/*      GL_MAP2_VERTEX_4 */
/*      GL_MAP2_COLOR_4 */
/*      GL_MAP2_INDEX */
/*      GL_MAP2_NORMAL */
/*      GL_MAP2_TEXTURE_COORD_1 */
/*      GL_MAP2_TEXTURE_COORD_2 */
/*      GL_MAP2_TEXTURE_COORD_3 */
/*      GL_MAP2_TEXTURE_COORD_4 */
/*      GL_POINT_SMOOTH */
/*      GL_LINE_SMOOTH */
/*      GL_POLYGON_SMOOTH */
/*      GL_SCISSOR_TEST */
/*      GL_COLOR_MATERIAL */
/*      GL_NORMALIZE */
/*      GL_AUTO_NORMAL */
/*      GL_VERTEX_ARRAY */
/*      GL_NORMAL_ARRAY */
/*      GL_COLOR_ARRAY */
/*      GL_INDEX_ARRAY */
/*      GL_TEXTURE_COORD_ARRAY */
/*      GL_EDGE_FLAG_ARRAY */
/*      GL_POLYGON_OFFSET_POINT */
/*      GL_POLYGON_OFFSET_LINE */
/*      GL_POLYGON_OFFSET_FILL */

/* ErrorCode */
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_STACK_OVERFLOW                 0x0503
#define GL_STACK_UNDERFLOW                0x0504
#define GL_OUT_OF_MEMORY                  0x0505

/* FeedBackMode */
#define GL_2D                             0x0600
#define GL_3D                             0x0601
#define GL_3D_COLOR                       0x0602
#define GL_3D_COLOR_TEXTURE               0x0603
#define GL_4D_COLOR_TEXTURE               0x0604

/* FeedBackToken */
#define GL_PASS_THROUGH_TOKEN             0x0700
#define GL_POINT_TOKEN                    0x0701
#define GL_LINE_TOKEN                     0x0702
#define GL_POLYGON_TOKEN                  0x0703
#define GL_BITMAP_TOKEN                   0x0704
#define GL_DRAW_PIXEL_TOKEN               0x0705
#define GL_COPY_PIXEL_TOKEN               0x0706
#define GL_LINE_RESET_TOKEN               0x0707

/* FogMode */
/*      GL_LINEAR */
#define GL_EXP                            0x0800
#define GL_EXP2                           0x0801


/* FogParameter */
/*      GL_FOG_COLOR */
/*      GL_FOG_DENSITY */
/*      GL_FOG_END */
/*      GL_FOG_INDEX */
/*      GL_FOG_MODE */
/*      GL_FOG_START */

/* FrontFaceDirection */
#define GL_CW                             0x0900
#define GL_CCW                            0x0901

/* GetMapTarget */
#define GL_COEFF                          0x0A00
#define GL_ORDER                          0x0A01
#define GL_DOMAIN                         0x0A02

/* GetPixelMap */
/*      GL_PIXEL_MAP_I_TO_I */
/*      GL_PIXEL_MAP_S_TO_S */
/*      GL_PIXEL_MAP_I_TO_R */
/*      GL_PIXEL_MAP_I_TO_G */
/*      GL_PIXEL_MAP_I_TO_B */
/*      GL_PIXEL_MAP_I_TO_A */
/*      GL_PIXEL_MAP_R_TO_R */
/*      GL_PIXEL_MAP_G_TO_G */
/*      GL_PIXEL_MAP_B_TO_B */
/*      GL_PIXEL_MAP_A_TO_A */

/* GetPointerTarget */
/*      GL_VERTEX_ARRAY_POINTER */
/*      GL_NORMAL_ARRAY_POINTER */
/*      GL_COLOR_ARRAY_POINTER */
/*      GL_INDEX_ARRAY_POINTER */
/*      GL_TEXTURE_COORD_ARRAY_POINTER */
/*      GL_EDGE_FLAG_ARRAY_POINTER */

/* GetTarget */
#define GL_CURRENT_COLOR                  0x0B00
#define GL_CURRENT_INDEX                  0x0B01
#define GL_CURRENT_NORMAL                 0x0B02
#define GL_CURRENT_TEXTURE_COORDS         0x0B03
#define GL_CURRENT_RASTER_COLOR           0x0B04
#define GL_CURRENT_RASTER_INDEX           0x0B05
#define GL_CURRENT_RASTER_TEXTURE_COORDS  0x0B06
#define GL_CURRENT_RASTER_POSITION        0x0B07
#define GL_CURRENT_RASTER_POSITION_VALID  0x0B08
#define GL_CURRENT_RASTER_DISTANCE        0x0B09
#define GL_POINT_SMOOTH                   0x0B10
#define GL_POINT_SIZE                     0x0B11
#define GL_POINT_SIZE_RANGE               0x0B12
#define GL_POINT_SIZE_GRANULARITY         0x0B13
#define GL_LINE_SMOOTH                    0x0B20
#define GL_LINE_WIDTH                     0x0B21
#define GL_LINE_WIDTH_RANGE               0x0B22
#define GL_LINE_WIDTH_GRANULARITY         0x0B23
#define GL_LINE_STIPPLE                   0x0B24
#define GL_LINE_STIPPLE_PATTERN           0x0B25
#define GL_LINE_STIPPLE_REPEAT            0x0B26
#define GL_LIST_MODE                      0x0B30
#define GL_MAX_LIST_NESTING               0x0B31
#define GL_LIST_BASE                      0x0B32
#define GL_LIST_INDEX                     0x0B33
#define GL_POLYGON_MODE                   0x0B40
#define GL_POLYGON_SMOOTH                 0x0B41
#define GL_POLYGON_STIPPLE                0x0B42
#define GL_EDGE_FLAG                      0x0B43
#define GL_CULL_FACE                      0x0B44
#define GL_CULL_FACE_MODE                 0x0B45
#define GL_FRONT_FACE                     0x0B46
#define GL_LIGHTING                       0x0B50
#define GL_LIGHT_MODEL_LOCAL_VIEWER       0x0B51
#define GL_LIGHT_MODEL_TWO_SIDE           0x0B52
#define GL_LIGHT_MODEL_AMBIENT            0x0B53
#define GL_SHADE_MODEL                    0x0B54
#define GL_COLOR_MATERIAL_FACE            0x0B55
#define GL_COLOR_MATERIAL_PARAMETER       0x0B56
#define GL_COLOR_MATERIAL                 0x0B57
#define GL_FOG                            0x0B60
#define GL_FOG_INDEX                      0x0B61
#define GL_FOG_DENSITY                    0x0B62
#define GL_FOG_START                      0x0B63
#define GL_FOG_END                        0x0B64
#define GL_FOG_MODE                       0x0B65
#define GL_FOG_COLOR                      0x0B66
#define GL_DEPTH_RANGE                    0x0B70
#define GL_DEPTH_TEST                     0x0B71
#define GL_DEPTH_WRITEMASK                0x0B72
#define GL_DEPTH_CLEAR_VALUE              0x0B73
#define GL_DEPTH_FUNC                     0x0B74
#define GL_ACCUM_CLEAR_VALUE              0x0B80
#define GL_STENCIL_TEST                   0x0B90
#define GL_STENCIL_CLEAR_VALUE            0x0B91
#define GL_STENCIL_FUNC                   0x0B92
#define GL_STENCIL_VALUE_MASK             0x0B93
#define GL_STENCIL_FAIL                   0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS        0x0B96
#define GL_STENCIL_REF                    0x0B97
#define GL_STENCIL_WRITEMASK              0x0B98
#define GL_MATRIX_MODE                    0x0BA0
#define GL_NORMALIZE                      0x0BA1
#define GL_VIEWPORT                       0x0BA2
#define GL_MODELVIEW_STACK_DEPTH          0x0BA3
#define GL_PROJECTION_STACK_DEPTH         0x0BA4
#define GL_TEXTURE_STACK_DEPTH            0x0BA5
#define GL_MODELVIEW_MATRIX               0x0BA6
#define GL_PROJECTION_MATRIX              0x0BA7
#define GL_TEXTURE_MATRIX                 0x0BA8
#define GL_ATTRIB_STACK_DEPTH             0x0BB0
#define GL_CLIENT_ATTRIB_STACK_DEPTH      0x0BB1
#define GL_ALPHA_TEST                     0x0BC0
#define GL_ALPHA_TEST_FUNC                0x0BC1
#define GL_ALPHA_TEST_REF                 0x0BC2
#define GL_DITHER                         0x0BD0
#define GL_BLEND_DST                      0x0BE0
#define GL_BLEND_SRC                      0x0BE1
#define GL_BLEND                          0x0BE2
#define GL_LOGIC_OP_MODE                  0x0BF0
#define GL_INDEX_LOGIC_OP                 0x0BF1
#define GL_COLOR_LOGIC_OP                 0x0BF2
#define GL_AUX_BUFFERS                    0x0C00
#define GL_DRAW_BUFFER                    0x0C01
#define GL_READ_BUFFER                    0x0C02
#define GL_SCISSOR_BOX                    0x0C10
#define GL_SCISSOR_TEST                   0x0C11
#define GL_INDEX_CLEAR_VALUE              0x0C20
#define GL_INDEX_WRITEMASK                0x0C21
#define GL_COLOR_CLEAR_VALUE              0x0C22
#define GL_COLOR_WRITEMASK                0x0C23
#define GL_INDEX_MODE                     0x0C30
#define GL_RGBA_MODE                      0x0C31
#define GL_DOUBLEBUFFER                   0x0C32
#define GL_STEREO                         0x0C33
#define GL_RENDER_MODE                    0x0C40
#define GL_PERSPECTIVE_CORRECTION_HINT    0x0C50
#define GL_POINT_SMOOTH_HINT              0x0C51
#define GL_LINE_SMOOTH_HINT               0x0C52
#define GL_POLYGON_SMOOTH_HINT            0x0C53
#define GL_FOG_HINT                       0x0C54
#define GL_TEXTURE_GEN_S                  0x0C60
#define GL_TEXTURE_GEN_T                  0x0C61
#define GL_TEXTURE_GEN_R                  0x0C62
#define GL_TEXTURE_GEN_Q                  0x0C63
#define GL_PIXEL_MAP_I_TO_I               0x0C70
#define GL_PIXEL_MAP_S_TO_S               0x0C71
#define GL_PIXEL_MAP_I_TO_R               0x0C72
#define GL_PIXEL_MAP_I_TO_G               0x0C73
#define GL_PIXEL_MAP_I_TO_B               0x0C74
#define GL_PIXEL_MAP_I_TO_A               0x0C75
#define GL_PIXEL_MAP_R_TO_R               0x0C76
#define GL_PIXEL_MAP_G_TO_G               0x0C77
#define GL_PIXEL_MAP_B_TO_B               0x0C78
#define GL_PIXEL_MAP_A_TO_A               0x0C79
#define GL_PIXEL_MAP_I_TO_I_SIZE          0x0CB0
#define GL_PIXEL_MAP_S_TO_S_SIZE          0x0CB1
#define GL_PIXEL_MAP_I_TO_R_SIZE          0x0CB2
#define GL_PIXEL_MAP_I_TO_G_SIZE          0x0CB3
#define GL_PIXEL_MAP_I_TO_B_SIZE          0x0CB4
#define GL_PIXEL_MAP_I_TO_A_SIZE          0x0CB5
#define GL_PIXEL_MAP_R_TO_R_SIZE          0x0CB6
#define GL_PIXEL_MAP_G_TO_G_SIZE          0x0CB7
#define GL_PIXEL_MAP_B_TO_B_SIZE          0x0CB8
#define GL_PIXEL_MAP_A_TO_A_SIZE          0x0CB9
#define GL_UNPACK_SWAP_BYTES              0x0CF0
#define GL_UNPACK_LSB_FIRST               0x0CF1
#define GL_UNPACK_ROW_LENGTH              0x0CF2
#define GL_UNPACK_SKIP_ROWS               0x0CF3
#define GL_UNPACK_SKIP_PIXELS             0x0CF4
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_SWAP_BYTES                0x0D00
#define GL_PACK_LSB_FIRST                 0x0D01
#define GL_PACK_ROW_LENGTH                0x0D02
#define GL_PACK_SKIP_ROWS                 0x0D03
#define GL_PACK_SKIP_PIXELS               0x0D04
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_MAP_COLOR                      0x0D10
#define GL_MAP_STENCIL                    0x0D11
#define GL_INDEX_SHIFT                    0x0D12
#define GL_INDEX_OFFSET                   0x0D13
#define GL_RED_SCALE                      0x0D14
#define GL_RED_BIAS                       0x0D15
#define GL_ZOOM_X                         0x0D16
#define GL_ZOOM_Y                         0x0D17
#define GL_GREEN_SCALE                    0x0D18
#define GL_GREEN_BIAS                     0x0D19
#define GL_BLUE_SCALE                     0x0D1A
#define GL_BLUE_BIAS                      0x0D1B
#define GL_ALPHA_SCALE                    0x0D1C
#define GL_ALPHA_BIAS                     0x0D1D
#define GL_DEPTH_SCALE                    0x0D1E
#define GL_DEPTH_BIAS                     0x0D1F
#define GL_MAX_EVAL_ORDER                 0x0D30
#define GL_MAX_LIGHTS                     0x0D31
#define GL_MAX_CLIP_PLANES                0x0D32
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_PIXEL_MAP_TABLE            0x0D34
#define GL_MAX_ATTRIB_STACK_DEPTH         0x0D35
#define GL_MAX_MODELVIEW_STACK_DEPTH      0x0D36
#define GL_MAX_NAME_STACK_DEPTH           0x0D37
#define GL_MAX_PROJECTION_STACK_DEPTH     0x0D38
#define GL_MAX_TEXTURE_STACK_DEPTH        0x0D39
#define GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GL_MAX_CLIENT_ATTRIB_STACK_DEPTH  0x0D3B
#define GL_SUBPIXEL_BITS                  0x0D50
#define GL_INDEX_BITS                     0x0D51
#define GL_RED_BITS                       0x0D52
#define GL_GREEN_BITS                     0x0D53
#define GL_BLUE_BITS                      0x0D54
#define GL_ALPHA_BITS                     0x0D55
#define GL_DEPTH_BITS                     0x0D56
#define GL_STENCIL_BITS                   0x0D57
#define GL_ACCUM_RED_BITS                 0x0D58
#define GL_ACCUM_GREEN_BITS               0x0D59
#define GL_ACCUM_BLUE_BITS                0x0D5A
#define GL_ACCUM_ALPHA_BITS               0x0D5B
#define GL_NAME_STACK_DEPTH               0x0D70
#define GL_AUTO_NORMAL                    0x0D80
#define GL_MAP1_COLOR_4                   0x0D90
#define GL_MAP1_INDEX                     0x0D91
#define GL_MAP1_NORMAL                    0x0D92
#define GL_MAP1_TEXTURE_COORD_1           0x0D93
#define GL_MAP1_TEXTURE_COORD_2           0x0D94
#define GL_MAP1_TEXTURE_COORD_3           0x0D95
#define GL_MAP1_TEXTURE_COORD_4           0x0D96
#define GL_MAP1_VERTEX_3                  0x0D97
#define GL_MAP1_VERTEX_4                  0x0D98
#define GL_MAP2_COLOR_4                   0x0DB0
#define GL_MAP2_INDEX                     0x0DB1
#define GL_MAP2_NORMAL                    0x0DB2
#define GL_MAP2_TEXTURE_COORD_1           0x0DB3
#define GL_MAP2_TEXTURE_COORD_2           0x0DB4
#define GL_MAP2_TEXTURE_COORD_3           0x0DB5
#define GL_MAP2_TEXTURE_COORD_4           0x0DB6
#define GL_MAP2_VERTEX_3                  0x0DB7
#define GL_MAP2_VERTEX_4                  0x0DB8
#define GL_MAP1_GRID_DOMAIN               0x0DD0
#define GL_MAP1_GRID_SEGMENTS             0x0DD1
#define GL_MAP2_GRID_DOMAIN               0x0DD2
#define GL_MAP2_GRID_SEGMENTS             0x0DD3
#define GL_TEXTURE_1D                     0x0DE0
#define GL_TEXTURE_2D                     0x0DE1
#define GL_FEEDBACK_BUFFER_POINTER        0x0DF0
#define GL_FEEDBACK_BUFFER_SIZE           0x0DF1
#define GL_FEEDBACK_BUFFER_TYPE           0x0DF2
#define GL_SELECTION_BUFFER_POINTER       0x0DF3
#define GL_SELECTION_BUFFER_SIZE          0x0DF4
/*      GL_TEXTURE_BINDING_1D */
/*      GL_TEXTURE_BINDING_2D */
/*      GL_VERTEX_ARRAY */
/*      GL_NORMAL_ARRAY */
/*      GL_COLOR_ARRAY */
/*      GL_INDEX_ARRAY */
/*      GL_TEXTURE_COORD_ARRAY */
/*      GL_EDGE_FLAG_ARRAY */
/*      GL_VERTEX_ARRAY_SIZE */
/*      GL_VERTEX_ARRAY_TYPE */
/*      GL_VERTEX_ARRAY_STRIDE */
/*      GL_NORMAL_ARRAY_TYPE */
/*      GL_NORMAL_ARRAY_STRIDE */
/*      GL_COLOR_ARRAY_SIZE */
/*      GL_COLOR_ARRAY_TYPE */
/*      GL_COLOR_ARRAY_STRIDE */
/*      GL_INDEX_ARRAY_TYPE */
/*      GL_INDEX_ARRAY_STRIDE */
/*      GL_TEXTURE_COORD_ARRAY_SIZE */
/*      GL_TEXTURE_COORD_ARRAY_TYPE */
/*      GL_TEXTURE_COORD_ARRAY_STRIDE */
/*      GL_EDGE_FLAG_ARRAY_STRIDE */
/*      GL_POLYGON_OFFSET_FACTOR */
/*      GL_POLYGON_OFFSET_UNITS */

/* GetTextureParameter */
/*      GL_TEXTURE_MAG_FILTER */
/*      GL_TEXTURE_MIN_FILTER */
/*      GL_TEXTURE_WRAP_S */
/*      GL_TEXTURE_WRAP_T */
#define GL_TEXTURE_WIDTH                  0x1000
#define GL_TEXTURE_HEIGHT                 0x1001
#define GL_TEXTURE_INTERNAL_FORMAT        0x1003
#define GL_TEXTURE_BORDER_COLOR           0x1004
#define GL_TEXTURE_BORDER                 0x1005
/*      GL_TEXTURE_RED_SIZE */
/*      GL_TEXTURE_GREEN_SIZE */
/*      GL_TEXTURE_BLUE_SIZE */
/*      GL_TEXTURE_ALPHA_SIZE */
/*      GL_TEXTURE_LUMINANCE_SIZE */
/*      GL_TEXTURE_INTENSITY_SIZE */
/*      GL_TEXTURE_PRIORITY */
/*      GL_TEXTURE_RESIDENT */

/* HintMode */
#define GL_DONT_CARE                      0x1100
#define GL_FASTEST                        0x1101
#define GL_NICEST                         0x1102

/* HintTarget */
/*      GL_PERSPECTIVE_CORRECTION_HINT */
/*      GL_POINT_SMOOTH_HINT */
/*      GL_LINE_SMOOTH_HINT */
/*      GL_POLYGON_SMOOTH_HINT */
/*      GL_FOG_HINT */
/*      GL_PHONG_HINT */

/* IndexPointerType */
/*      GL_SHORT */
/*      GL_INT */
/*      GL_FLOAT */
/*      GL_DOUBLE */

/* LightModelParameter */
/*      GL_LIGHT_MODEL_AMBIENT */
/*      GL_LIGHT_MODEL_LOCAL_VIEWER */
/*      GL_LIGHT_MODEL_TWO_SIDE */

/* LightName */
#define GL_LIGHT0                         0x4000
#define GL_LIGHT1                         0x4001
#define GL_LIGHT2                         0x4002
#define GL_LIGHT3                         0x4003
#define GL_LIGHT4                         0x4004
#define GL_LIGHT5                         0x4005
#define GL_LIGHT6                         0x4006
#define GL_LIGHT7                         0x4007

/* LightParameter */
#define GL_AMBIENT                        0x1200
#define GL_DIFFUSE                        0x1201
#define GL_SPECULAR                       0x1202
#define GL_POSITION                       0x1203
#define GL_SPOT_DIRECTION                 0x1204
#define GL_SPOT_EXPONENT                  0x1205
#define GL_SPOT_CUTOFF                    0x1206
#define GL_CONSTANT_ATTENUATION           0x1207
#define GL_LINEAR_ATTENUATION             0x1208
#define GL_QUADRATIC_ATTENUATION          0x1209

/* InterleavedArrays */
/*      GL_V2F */
/*      GL_V3F */
/*      GL_C4UB_V2F */
/*      GL_C4UB_V3F */
/*      GL_C3F_V3F */
/*      GL_N3F_V3F */
/*      GL_C4F_N3F_V3F */
/*      GL_T2F_V3F */
/*      GL_T4F_V4F */
/*      GL_T2F_C4UB_V3F */
/*      GL_T2F_C3F_V3F */
/*      GL_T2F_N3F_V3F */
/*      GL_T2F_C4F_N3F_V3F */
/*      GL_T4F_C4F_N3F_V4F */

/* ListMode */
#define GL_COMPILE                        0x1300
#define GL_COMPILE_AND_EXECUTE            0x1301

/* ListNameType */
/*      GL_BYTE */
/*      GL_UNSIGNED_BYTE */
/*      GL_SHORT */
/*      GL_UNSIGNED_SHORT */
/*      GL_INT */
/*      GL_UNSIGNED_INT */
/*      GL_FLOAT */
/*      GL_2_BYTES */
/*      GL_3_BYTES */
/*      GL_4_BYTES */

/* LogicOp */
#define GL_CLEAR                          0x1500
#define GL_AND                            0x1501
#define GL_AND_REVERSE                    0x1502
#define GL_COPY                           0x1503
#define GL_AND_INVERTED                   0x1504
#define GL_NOOP                           0x1505
#define GL_XOR                            0x1506
#define GL_OR                             0x1507
#define GL_NOR                            0x1508
#define GL_EQUIV                          0x1509
#define GL_INVERT                         0x150A
#define GL_OR_REVERSE                     0x150B
#define GL_COPY_INVERTED                  0x150C
#define GL_OR_INVERTED                    0x150D
#define GL_NAND                           0x150E
#define GL_SET                            0x150F

/* MapTarget */
/*      GL_MAP1_COLOR_4 */
/*      GL_MAP1_INDEX */
/*      GL_MAP1_NORMAL */
/*      GL_MAP1_TEXTURE_COORD_1 */
/*      GL_MAP1_TEXTURE_COORD_2 */
/*      GL_MAP1_TEXTURE_COORD_3 */
/*      GL_MAP1_TEXTURE_COORD_4 */
/*      GL_MAP1_VERTEX_3 */
/*      GL_MAP1_VERTEX_4 */
/*      GL_MAP2_COLOR_4 */
/*      GL_MAP2_INDEX */
/*      GL_MAP2_NORMAL */
/*      GL_MAP2_TEXTURE_COORD_1 */
/*      GL_MAP2_TEXTURE_COORD_2 */
/*      GL_MAP2_TEXTURE_COORD_3 */
/*      GL_MAP2_TEXTURE_COORD_4 */
/*      GL_MAP2_VERTEX_3 */
/*      GL_MAP2_VERTEX_4 */

/* MaterialFace */
/*      GL_FRONT */
/*      GL_BACK */
/*      GL_FRONT_AND_BACK */

/* MaterialParameter */
#define GL_EMISSION                       0x1600
#define GL_SHININESS                      0x1601
#define GL_AMBIENT_AND_DIFFUSE            0x1602
#define GL_COLOR_INDEXES                  0x1603
/*      GL_AMBIENT */
/*      GL_DIFFUSE */
/*      GL_SPECULAR */

/* MatrixMode */
#define GL_MODELVIEW                      0x1700
#define GL_PROJECTION                     0x1701
#define GL_TEXTURE                        0x1702

/* MeshMode1 */
/*      GL_POINT */
/*      GL_LINE */

/* MeshMode2 */
/*      GL_POINT */
/*      GL_LINE */
/*      GL_FILL */

/* NormalPointerType */
/*      GL_BYTE */
/*      GL_SHORT */
/*      GL_INT */
/*      GL_FLOAT */
/*      GL_DOUBLE */

/* PixelCopyType */
#define GL_COLOR                          0x1800
#define GL_DEPTH                          0x1801
#define GL_STENCIL                        0x1802

/* PixelFormat */
#define GL_COLOR_INDEX                    0x1900
#define GL_STENCIL_INDEX                  0x1901
#define GL_DEPTH_COMPONENT                0x1902
#define GL_RED                            0x1903
#define GL_GREEN                          0x1904
#define GL_BLUE                           0x1905
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A

/* PixelMap */
/*      GL_PIXEL_MAP_I_TO_I */
/*      GL_PIXEL_MAP_S_TO_S */
/*      GL_PIXEL_MAP_I_TO_R */
/*      GL_PIXEL_MAP_I_TO_G */
/*      GL_PIXEL_MAP_I_TO_B */
/*      GL_PIXEL_MAP_I_TO_A */
/*      GL_PIXEL_MAP_R_TO_R */
/*      GL_PIXEL_MAP_G_TO_G */
/*      GL_PIXEL_MAP_B_TO_B */
/*      GL_PIXEL_MAP_A_TO_A */

/* PixelStore */
/*      GL_UNPACK_SWAP_BYTES */
/*      GL_UNPACK_LSB_FIRST */
/*      GL_UNPACK_ROW_LENGTH */
/*      GL_UNPACK_SKIP_ROWS */
/*      GL_UNPACK_SKIP_PIXELS */
/*      GL_UNPACK_ALIGNMENT */
/*      GL_PACK_SWAP_BYTES */
/*      GL_PACK_LSB_FIRST */
/*      GL_PACK_ROW_LENGTH */
/*      GL_PACK_SKIP_ROWS */
/*      GL_PACK_SKIP_PIXELS */
/*      GL_PACK_ALIGNMENT */

/* PixelTransfer */
/*      GL_MAP_COLOR */
/*      GL_MAP_STENCIL */
/*      GL_INDEX_SHIFT */
/*      GL_INDEX_OFFSET */
/*      GL_RED_SCALE */
/*      GL_RED_BIAS */
/*      GL_GREEN_SCALE */
/*      GL_GREEN_BIAS */
/*      GL_BLUE_SCALE */
/*      GL_BLUE_BIAS */
/*      GL_ALPHA_SCALE */
/*      GL_ALPHA_BIAS */
/*      GL_DEPTH_SCALE */
/*      GL_DEPTH_BIAS */

/* PixelType */
#define GL_BITMAP                         0x1A00
/*      GL_BYTE */
/*      GL_UNSIGNED_BYTE */
/*      GL_SHORT */
/*      GL_UNSIGNED_SHORT */
/*      GL_INT */
/*      GL_UNSIGNED_INT */
/*      GL_FLOAT */

/* PolygonMode */
#define GL_POINT                          0x1B00
#define GL_LINE                           0x1B01
#define GL_FILL                           0x1B02

/* ReadBufferMode */
/*      GL_FRONT_LEFT */
/*      GL_FRONT_RIGHT */
/*      GL_BACK_LEFT */
/*      GL_BACK_RIGHT */
/*      GL_FRONT */
/*      GL_BACK */
/*      GL_LEFT */
/*      GL_RIGHT */
/*      GL_AUX0 */
/*      GL_AUX1 */
/*      GL_AUX2 */
/*      GL_AUX3 */

/* RenderingMode */
#define GL_RENDER                         0x1C00
#define GL_FEEDBACK                       0x1C01
#define GL_SELECT                         0x1C02

/* ShadingModel */
#define GL_FLAT                           0x1D00
#define GL_SMOOTH                         0x1D01


/* StencilFunction */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* StencilOp */
/*      GL_ZERO */
#define GL_KEEP                           0x1E00
#define GL_REPLACE                        0x1E01
#define GL_INCR                           0x1E02
#define GL_DECR                           0x1E03
/*      GL_INVERT */

/* StringName */
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03

/* TextureCoordName */
#define GL_S                              0x2000
#define GL_T                              0x2001
#define GL_R                              0x2002
#define GL_Q                              0x2003

/* TexCoordPointerType */
/*      GL_SHORT */
/*      GL_INT */
/*      GL_FLOAT */
/*      GL_DOUBLE */

/* TextureEnvMode */
#define GL_MODULATE                       0x2100
#define GL_DECAL                          0x2101
/*      GL_BLEND */
/*      GL_REPLACE */

/* TextureEnvParameter */
#define GL_TEXTURE_ENV_MODE               0x2200
#define GL_TEXTURE_ENV_COLOR              0x2201

/* TextureEnvTarget */
#define GL_TEXTURE_ENV                    0x2300

/* TextureGenMode */
#define GL_EYE_LINEAR                     0x2400
#define GL_OBJECT_LINEAR                  0x2401
#define GL_SPHERE_MAP                     0x2402

/* TextureGenParameter */
#define GL_TEXTURE_GEN_MODE               0x2500
#define GL_OBJECT_PLANE                   0x2501
#define GL_EYE_PLANE                      0x2502

/* TextureMagFilter */
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601

/* TextureMinFilter */
/*      GL_NEAREST */
/*      GL_LINEAR */
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703

/* TextureParameterName */
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
/*      GL_TEXTURE_BORDER_COLOR */
/*      GL_TEXTURE_PRIORITY */

/* TextureTarget */
/*      GL_TEXTURE_1D */
/*      GL_TEXTURE_2D */
/*      GL_PROXY_TEXTURE_1D */
/*      GL_PROXY_TEXTURE_2D */

/* TextureWrapMode */
#define GL_CLAMP                          0x2900
#define GL_REPEAT                         0x2901

/* VertexPointerType */
/*      GL_SHORT */
/*      GL_INT */
/*      GL_FLOAT */
/*      GL_DOUBLE */

/* ClientAttribMask */
#define GL_CLIENT_PIXEL_STORE_BIT         0x00000001
#define GL_CLIENT_VERTEX_ARRAY_BIT        0x00000002
#define GL_CLIENT_ALL_ATTRIB_BITS         0xffffffff

/* polygon_offset */
#define GL_POLYGON_OFFSET_FACTOR          0x8038
#define GL_POLYGON_OFFSET_UNITS           0x2A00
#define GL_POLYGON_OFFSET_POINT           0x2A01
#define GL_POLYGON_OFFSET_LINE            0x2A02
#define GL_POLYGON_OFFSET_FILL            0x8037

/* texture */
#define GL_ALPHA4                         0x803B
#define GL_ALPHA8                         0x803C
#define GL_ALPHA12                        0x803D
#define GL_ALPHA16                        0x803E
#define GL_LUMINANCE4                     0x803F
#define GL_LUMINANCE8                     0x8040
#define GL_LUMINANCE12                    0x8041
#define GL_LUMINANCE16                    0x8042
#define GL_LUMINANCE4_ALPHA4              0x8043
#define GL_LUMINANCE6_ALPHA2              0x8044
#define GL_LUMINANCE8_ALPHA8              0x8045
#define GL_LUMINANCE12_ALPHA4             0x8046
#define GL_LUMINANCE12_ALPHA12            0x8047
#define GL_LUMINANCE16_ALPHA16            0x8048
#define GL_INTENSITY                      0x8049
#define GL_INTENSITY4                     0x804A
#define GL_INTENSITY8                     0x804B
#define GL_INTENSITY12                    0x804C
#define GL_INTENSITY16                    0x804D
#define GL_R3_G3_B2                       0x2A10
#define GL_RGB4                           0x804F
#define GL_RGB5                           0x8050
#define GL_RGB8                           0x8051
#define GL_RGB10                          0x8052
#define GL_RGB12                          0x8053
#define GL_RGB16                          0x8054
#define GL_RGBA2                          0x8055
#define GL_RGBA4                          0x8056
#define GL_RGB5_A1                        0x8057
#define GL_RGBA8                          0x8058
#define GL_RGB10_A2                       0x8059
#define GL_RGBA12                         0x805A
#define GL_RGBA16                         0x805B
#define GL_TEXTURE_RED_SIZE               0x805C
#define GL_TEXTURE_GREEN_SIZE             0x805D
#define GL_TEXTURE_BLUE_SIZE              0x805E
#define GL_TEXTURE_ALPHA_SIZE             0x805F
#define GL_TEXTURE_LUMINANCE_SIZE         0x8060
#define GL_TEXTURE_INTENSITY_SIZE         0x8061
#define GL_PROXY_TEXTURE_1D               0x8063
#define GL_PROXY_TEXTURE_2D               0x8064

/* texture_object */
#define GL_TEXTURE_PRIORITY               0x8066
#define GL_TEXTURE_RESIDENT               0x8067
#define GL_TEXTURE_BINDING_1D             0x8068
#define GL_TEXTURE_BINDING_2D             0x8069

/* vertex_array */
#define GL_VERTEX_ARRAY                   0x8074
#define GL_NORMAL_ARRAY                   0x8075
#define GL_COLOR_ARRAY                    0x8076
#define GL_INDEX_ARRAY                    0x8077
#define GL_TEXTURE_COORD_ARRAY            0x8078
#define GL_EDGE_FLAG_ARRAY                0x8079
#define GL_VERTEX_ARRAY_SIZE              0x807A
#define GL_VERTEX_ARRAY_TYPE              0x807B
#define GL_VERTEX_ARRAY_STRIDE            0x807C
#define GL_NORMAL_ARRAY_TYPE              0x807E
#define GL_NORMAL_ARRAY_STRIDE            0x807F
#define GL_COLOR_ARRAY_SIZE               0x8081
#define GL_COLOR_ARRAY_TYPE               0x8082
#define GL_COLOR_ARRAY_STRIDE             0x8083
#define GL_INDEX_ARRAY_TYPE               0x8085
#define GL_INDEX_ARRAY_STRIDE             0x8086
#define GL_TEXTURE_COORD_ARRAY_SIZE       0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE       0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE     0x808A
#define GL_EDGE_FLAG_ARRAY_STRIDE         0x808C
#define GL_VERTEX_ARRAY_POINTER           0x808E
#define GL_NORMAL_ARRAY_POINTER           0x808F
#define GL_COLOR_ARRAY_POINTER            0x8090
#define GL_INDEX_ARRAY_POINTER            0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER    0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER        0x8093
#define GL_V2F                            0x2A20
#define GL_V3F                            0x2A21
#define GL_C4UB_V2F                       0x2A22
#define GL_C4UB_V3F                       0x2A23
#define GL_C3F_V3F                        0x2A24
#define GL_N3F_V3F                        0x2A25
#define GL_C4F_N3F_V3F                    0x2A26
#define GL_T2F_V3F                        0x2A27
#define GL_T4F_V4F                        0x2A28
#define GL_T2F_C4UB_V3F                   0x2A29
#define GL_T2F_C3F_V3F                    0x2A2A
#define GL_T2F_N3F_V3F                    0x2A2B
#define GL_T2F_C4F_N3F_V3F                0x2A2C
#define GL_T4F_C4F_N3F_V4F                0x2A2D

/* Extensions */
#define GL_EXT_vertex_array               1
#define GL_EXT_bgra                       1
#define GL_EXT_paletted_texture           1
#define GL_WIN_swap_hint                  1
#define GL_WIN_draw_range_elements        1
// #define GL_WIN_phong_shading              1
// #define GL_WIN_specular_fog               1

/* EXT_vertex_array */
#define GL_VERTEX_ARRAY_EXT               0x8074
#define GL_NORMAL_ARRAY_EXT               0x8075
#define GL_COLOR_ARRAY_EXT                0x8076
#define GL_INDEX_ARRAY_EXT                0x8077
#define GL_TEXTURE_COORD_ARRAY_EXT        0x8078
#define GL_EDGE_FLAG_ARRAY_EXT            0x8079
#define GL_VERTEX_ARRAY_SIZE_EXT          0x807A
#define GL_VERTEX_ARRAY_TYPE_EXT          0x807B
#define GL_VERTEX_ARRAY_STRIDE_EXT        0x807C
#define GL_VERTEX_ARRAY_COUNT_EXT         0x807D
#define GL_NORMAL_ARRAY_TYPE_EXT          0x807E
#define GL_NORMAL_ARRAY_STRIDE_EXT        0x807F
#define GL_NORMAL_ARRAY_COUNT_EXT         0x8080
#define GL_COLOR_ARRAY_SIZE_EXT           0x8081
#define GL_COLOR_ARRAY_TYPE_EXT           0x8082
#define GL_COLOR_ARRAY_STRIDE_EXT         0x8083
#define GL_COLOR_ARRAY_COUNT_EXT          0x8084
#define GL_INDEX_ARRAY_TYPE_EXT           0x8085
#define GL_INDEX_ARRAY_STRIDE_EXT         0x8086
#define GL_INDEX_ARRAY_COUNT_EXT          0x8087
#define GL_TEXTURE_COORD_ARRAY_SIZE_EXT   0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE_EXT   0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE_EXT 0x808A
#define GL_TEXTURE_COORD_ARRAY_COUNT_EXT  0x808B
#define GL_EDGE_FLAG_ARRAY_STRIDE_EXT     0x808C
#define GL_EDGE_FLAG_ARRAY_COUNT_EXT      0x808D
#define GL_VERTEX_ARRAY_POINTER_EXT       0x808E
#define GL_NORMAL_ARRAY_POINTER_EXT       0x808F
#define GL_COLOR_ARRAY_POINTER_EXT        0x8090
#define GL_INDEX_ARRAY_POINTER_EXT        0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER_EXT 0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER_EXT    0x8093
#define GL_DOUBLE_EXT                     GL_DOUBLE

/* EXT_bgra */
#define GL_BGR_EXT                        0x80E0
#define GL_BGRA_EXT                       0x80E1

/* EXT_paletted_texture */

/* These must match the GL_COLOR_TABLE_*_SGI enumerants */
#define GL_COLOR_TABLE_FORMAT_EXT         0x80D8
#define GL_COLOR_TABLE_WIDTH_EXT          0x80D9
#define GL_COLOR_TABLE_RED_SIZE_EXT       0x80DA
#define GL_COLOR_TABLE_GREEN_SIZE_EXT     0x80DB
#define GL_COLOR_TABLE_BLUE_SIZE_EXT      0x80DC
#define GL_COLOR_TABLE_ALPHA_SIZE_EXT     0x80DD
#define GL_COLOR_TABLE_LUMINANCE_SIZE_EXT 0x80DE
#define GL_COLOR_TABLE_INTENSITY_SIZE_EXT 0x80DF

#define GL_COLOR_INDEX1_EXT               0x80E2
#define GL_COLOR_INDEX2_EXT               0x80E3
#define GL_COLOR_INDEX4_EXT               0x80E4
#define GL_COLOR_INDEX8_EXT               0x80E5
#define GL_COLOR_INDEX12_EXT              0x80E6
#define GL_COLOR_INDEX16_EXT              0x80E7

/* WIN_draw_range_elements */
#define GL_MAX_ELEMENTS_VERTICES_WIN      0x80E8
#define GL_MAX_ELEMENTS_INDICES_WIN       0x80E9

/* WIN_phong_shading */
#define GL_PHONG_WIN                      0x80EA 
#define GL_PHONG_HINT_WIN                 0x80EB 

/* WIN_specular_fog */
#define GL_FOG_SPECULAR_TEXTURE_WIN       0x80EC

/* For compatibility with OpenGL v1.0 */
#define GL_LOGIC_OP GL_INDEX_LOGIC_OP
#define GL_TEXTURE_COMPONENTS GL_TEXTURE_INTERNAL_FORMAT

/* For multisample */
#define WGL_DRAW_TO_WINDOW_ARB				0x2001
#define WGL_ACCELERATION_ARB				0x2003
#define WGL_SUPPORT_OPENGL_ARB				0x2010
#define WGL_DOUBLE_BUFFER_ARB				0x2011
#define WGL_DRAW_TO_PBUFFER_ARB				0x202D
#define WGL_PIXEL_TYPE_ARB					0x2013
#define WGL_TYPE_RGBA_ARB					0x202B
#define WGL_RED_BITS_ARB					0x2015
#define WGL_GREEN_BITS_ARB					0x2017
#define WGL_BLUE_BITS_ARB					0x2019
#define WGL_ALPHA_BITS_ARB					0x201B
#define WGL_DEPTH_BITS_ARB					0x2022
#define WGL_STENCIL_BITS_ARB				0x2023
#define WGL_SAMPLE_BUFFERS_ARB				0x2041
#define WGL_SAMPLES_ARB						0x2042
#define WGL_FULL_ACCELERATION_ARB			0x2027
#define WGL_COLOR_BITS_ARB					0x2014

#include "glext.h"

/*
 * OpenGL Prototypes
 */
typedef void ( APIENTRY * pfn_glAccum )(GLenum op, GLfloat value);
typedef void ( APIENTRY * pfn_glAlphaFunc )(GLenum func, GLclampf ref);
typedef GLboolean ( APIENTRY * pfn_glAreTexturesResident )(GLsizei n, const GLuint *textures, GLboolean *residences);
typedef void ( APIENTRY * pfn_glArrayElement )(GLint i);
typedef void ( APIENTRY * pfn_glBegin )(GLenum mode);
typedef void ( APIENTRY * pfn_glBindTexture )(GLenum target, GLuint texture);
typedef void ( APIENTRY * pfn_glBitmap )(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
typedef void ( APIENTRY * pfn_glBlendFunc )(GLenum sfactor, GLenum dfactor);
typedef void ( APIENTRY * pfn_glCallList )(GLuint list);
typedef void ( APIENTRY * pfn_glCallLists )(GLsizei n, GLenum type, const GLvoid *lists);
typedef void ( APIENTRY * pfn_glClear )(GLbitfield mask);
typedef void ( APIENTRY * pfn_glClearAccum )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void ( APIENTRY * pfn_glClearColor )(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void ( APIENTRY * pfn_glClearDepth )(GLclampd depth);
typedef void ( APIENTRY * pfn_glClearIndex )(GLfloat c);
typedef void ( APIENTRY * pfn_glClearStencil )(GLint s);
typedef void ( APIENTRY * pfn_glClipPlane )(GLenum plane, const GLdouble *equation);
typedef void ( APIENTRY * pfn_glColor3b )(GLbyte red, GLbyte green, GLbyte blue);
typedef void ( APIENTRY * pfn_glColor3bv )(const GLbyte *v);
typedef void ( APIENTRY * pfn_glColor3d )(GLdouble red, GLdouble green, GLdouble blue);
typedef void ( APIENTRY * pfn_glColor3dv )(const GLdouble *v);
typedef void ( APIENTRY * pfn_glColor3f )(GLfloat red, GLfloat green, GLfloat blue);
typedef void ( APIENTRY * pfn_glColor3fv )(const GLfloat *v);
typedef void ( APIENTRY * pfn_glColor3i )(GLint red, GLint green, GLint blue);
typedef void ( APIENTRY * pfn_glColor3iv )(const GLint *v);
typedef void ( APIENTRY * pfn_glColor3s )(GLshort red, GLshort green, GLshort blue);
typedef void ( APIENTRY * pfn_glColor3sv )(const GLshort *v);
typedef void ( APIENTRY * pfn_glColor3ub )(GLubyte red, GLubyte green, GLubyte blue);
typedef void ( APIENTRY * pfn_glColor3ubv )(const GLubyte *v);
typedef void ( APIENTRY * pfn_glColor3ui )(GLuint red, GLuint green, GLuint blue);
typedef void ( APIENTRY * pfn_glColor3uiv )(const GLuint *v);
typedef void ( APIENTRY * pfn_glColor3us )(GLushort red, GLushort green, GLushort blue);
typedef void ( APIENTRY * pfn_glColor3usv )(const GLushort *v);
typedef void ( APIENTRY * pfn_glColor4b )(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
typedef void ( APIENTRY * pfn_glColor4bv )(const GLbyte *v);
typedef void ( APIENTRY * pfn_glColor4d )(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
typedef void ( APIENTRY * pfn_glColor4dv )(const GLdouble *v);
typedef void ( APIENTRY * pfn_glColor4f )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void ( APIENTRY * pfn_glColor4fv )(const GLfloat *v);
typedef void ( APIENTRY * pfn_glColor4i )(GLint red, GLint green, GLint blue, GLint alpha);
typedef void ( APIENTRY * pfn_glColor4iv )(const GLint *v);
typedef void ( APIENTRY * pfn_glColor4s )(GLshort red, GLshort green, GLshort blue, GLshort alpha);
typedef void ( APIENTRY * pfn_glColor4sv )(const GLshort *v);
typedef void ( APIENTRY * pfn_glColor4ub )(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
typedef void ( APIENTRY * pfn_glColor4ubv )(const GLubyte *v);
typedef void ( APIENTRY * pfn_glColor4ui )(GLuint red, GLuint green, GLuint blue, GLuint alpha);
typedef void ( APIENTRY * pfn_glColor4uiv )(const GLuint *v);
typedef void ( APIENTRY * pfn_glColor4us )(GLushort red, GLushort green, GLushort blue, GLushort alpha);
typedef void ( APIENTRY * pfn_glColor4usv )(const GLushort *v);
typedef void ( APIENTRY * pfn_glColorMask )(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void ( APIENTRY * pfn_glColorMaterial )(GLenum face, GLenum mode);
typedef void ( APIENTRY * pfn_glColorPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void ( APIENTRY * pfn_glCopyPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
typedef void ( APIENTRY * pfn_glCopyTexImage1D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
typedef void ( APIENTRY * pfn_glCopyTexImage2D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void ( APIENTRY * pfn_glCopyTexSubImage1D )(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void ( APIENTRY * pfn_glCopyTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void ( APIENTRY * pfn_glCullFace )(GLenum mode);
typedef void ( APIENTRY * pfn_glDeleteLists )(GLuint list, GLsizei range);
typedef void ( APIENTRY * pfn_glDeleteTextures )(GLsizei n, const GLuint *textures);
typedef void ( APIENTRY * pfn_glDepthFunc )(GLenum func);
typedef void ( APIENTRY * pfn_glDepthMask )(GLboolean flag);
typedef void ( APIENTRY * pfn_glDepthRange )(GLclampd zNear, GLclampd zFar);
typedef void ( APIENTRY * pfn_glDisable )(GLenum cap);
typedef void ( APIENTRY * pfn_glDisableClientState )(GLenum array);
typedef void ( APIENTRY * pfn_glDrawArrays )(GLenum mode, GLint first, GLsizei count);
typedef void ( APIENTRY * pfn_glDrawBuffer )(GLenum mode);
typedef void ( APIENTRY * pfn_glDrawElements )(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
typedef void ( APIENTRY * pfn_glDrawPixels )(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void ( APIENTRY * pfn_glEdgeFlag )(GLboolean flag);
typedef void ( APIENTRY * pfn_glEdgeFlagPointer )(GLsizei stride, const GLvoid *pointer);
typedef void ( APIENTRY * pfn_glEdgeFlagv )(const GLboolean *flag);
typedef void ( APIENTRY * pfn_glEnable )(GLenum cap);
typedef void ( APIENTRY * pfn_glEnableClientState )(GLenum array);
typedef void ( APIENTRY * pfn_glEnd )(void);
typedef void ( APIENTRY * pfn_glEndList )(void);
typedef void ( APIENTRY * pfn_glEvalCoord1d )(GLdouble u);
typedef void ( APIENTRY * pfn_glEvalCoord1dv )(const GLdouble *u);
typedef void ( APIENTRY * pfn_glEvalCoord1f )(GLfloat u);
typedef void ( APIENTRY * pfn_glEvalCoord1fv )(const GLfloat *u);
typedef void ( APIENTRY * pfn_glEvalCoord2d )(GLdouble u, GLdouble v);
typedef void ( APIENTRY * pfn_glEvalCoord2dv )(const GLdouble *u);
typedef void ( APIENTRY * pfn_glEvalCoord2f )(GLfloat u, GLfloat v);
typedef void ( APIENTRY * pfn_glEvalCoord2fv )(const GLfloat *u);
typedef void ( APIENTRY * pfn_glEvalMesh1 )(GLenum mode, GLint i1, GLint i2);
typedef void ( APIENTRY * pfn_glEvalMesh2 )(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
typedef void ( APIENTRY * pfn_glEvalPoint1 )(GLint i);
typedef void ( APIENTRY * pfn_glEvalPoint2 )(GLint i, GLint j);
typedef void ( APIENTRY * pfn_glFeedbackBuffer )(GLsizei size, GLenum type, GLfloat *buffer);
typedef void ( APIENTRY * pfn_glFinish )(void);
typedef void ( APIENTRY * pfn_glFlush )(void);
typedef void ( APIENTRY * pfn_glFogf )(GLenum pname, GLfloat param);
typedef void ( APIENTRY * pfn_glFogfv )(GLenum pname, const GLfloat *params);
typedef void ( APIENTRY * pfn_glFogi )(GLenum pname, GLint param);
typedef void ( APIENTRY * pfn_glFogiv )(GLenum pname, const GLint *params);
typedef void ( APIENTRY * pfn_glFrontFace )(GLenum mode);
typedef void ( APIENTRY * pfn_glFrustum )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
typedef GLuint ( APIENTRY * pfn_glGenLists )(GLsizei range);
typedef void ( APIENTRY * pfn_glGenTextures )(GLsizei n, GLuint *textures);
typedef void ( APIENTRY * pfn_glGetBooleanv )(GLenum pname, GLboolean *params);
typedef void ( APIENTRY * pfn_glGetClipPlane )(GLenum plane, GLdouble *equation);
typedef void ( APIENTRY * pfn_glGetDoublev )(GLenum pname, GLdouble *params);
typedef GLenum ( APIENTRY * pfn_glGetError )(void);
typedef void ( APIENTRY * pfn_glGetFloatv )(GLenum pname, GLfloat *params);
typedef void ( APIENTRY * pfn_glGetIntegerv )(GLenum pname, GLint *params);
typedef void ( APIENTRY * pfn_glGetLightfv )(GLenum light, GLenum pname, GLfloat *params);
typedef void ( APIENTRY * pfn_glGetLightiv )(GLenum light, GLenum pname, GLint *params);
typedef void ( APIENTRY * pfn_glGetMapdv )(GLenum target, GLenum query, GLdouble *v);
typedef void ( APIENTRY * pfn_glGetMapfv )(GLenum target, GLenum query, GLfloat *v);
typedef void ( APIENTRY * pfn_glGetMapiv )(GLenum target, GLenum query, GLint *v);
typedef void ( APIENTRY * pfn_glGetMaterialfv )(GLenum face, GLenum pname, GLfloat *params);
typedef void ( APIENTRY * pfn_glGetMaterialiv )(GLenum face, GLenum pname, GLint *params);
typedef void ( APIENTRY * pfn_glGetPixelMapfv )(GLenum map, GLfloat *values);
typedef void ( APIENTRY * pfn_glGetPixelMapuiv )(GLenum map, GLuint *values);
typedef void ( APIENTRY * pfn_glGetPixelMapusv )(GLenum map, GLushort *values);
typedef void ( APIENTRY * pfn_glGetPointerv )(GLenum pname, GLvoid* *params);
typedef void ( APIENTRY * pfn_glGetPolygonStipple )(GLubyte *mask);
typedef const GLubyte * ( APIENTRY * pfn_glGetString )(GLenum name);
typedef void ( APIENTRY * pfn_glGetTexEnvfv )(GLenum target, GLenum pname, GLfloat *params);
typedef void ( APIENTRY * pfn_glGetTexEnviv )(GLenum target, GLenum pname, GLint *params);
typedef void ( APIENTRY * pfn_glGetTexGendv )(GLenum coord, GLenum pname, GLdouble *params);
typedef void ( APIENTRY * pfn_glGetTexGenfv )(GLenum coord, GLenum pname, GLfloat *params);
typedef void ( APIENTRY * pfn_glGetTexGeniv )(GLenum coord, GLenum pname, GLint *params);
typedef void ( APIENTRY * pfn_glGetTexImage )(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
typedef void ( APIENTRY * pfn_glGetTexLevelParameterfv )(GLenum target, GLint level, GLenum pname, GLfloat *params);
typedef void ( APIENTRY * pfn_glGetTexLevelParameteriv )(GLenum target, GLint level, GLenum pname, GLint *params);
typedef void ( APIENTRY * pfn_glGetTexParameterfv )(GLenum target, GLenum pname, GLfloat *params);
typedef void ( APIENTRY * pfn_glGetTexParameteriv )(GLenum target, GLenum pname, GLint *params);
typedef void ( APIENTRY * pfn_glHint )(GLenum target, GLenum mode);
typedef void ( APIENTRY * pfn_glIndexMask )(GLuint mask);
typedef void ( APIENTRY * pfn_glIndexPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void ( APIENTRY * pfn_glIndexd )(GLdouble c);
typedef void ( APIENTRY * pfn_glIndexdv )(const GLdouble *c);
typedef void ( APIENTRY * pfn_glIndexf )(GLfloat c);
typedef void ( APIENTRY * pfn_glIndexfv )(const GLfloat *c);
typedef void ( APIENTRY * pfn_glIndexi )(GLint c);
typedef void ( APIENTRY * pfn_glIndexiv )(const GLint *c);
typedef void ( APIENTRY * pfn_glIndexs )(GLshort c);
typedef void ( APIENTRY * pfn_glIndexsv )(const GLshort *c);
typedef void ( APIENTRY * pfn_glIndexub )(GLubyte c);
typedef void ( APIENTRY * pfn_glIndexubv )(const GLubyte *c);
typedef void ( APIENTRY * pfn_glInitNames )(void);
typedef void ( APIENTRY * pfn_glInterleavedArrays )(GLenum format, GLsizei stride, const GLvoid *pointer);
typedef GLboolean ( APIENTRY * pfn_glIsEnabled )(GLenum cap);
typedef GLboolean ( APIENTRY * pfn_glIsList )(GLuint list);
typedef GLboolean ( APIENTRY * pfn_glIsTexture )(GLuint texture);
typedef void ( APIENTRY * pfn_glLightModelf )(GLenum pname, GLfloat param);
typedef void ( APIENTRY * pfn_glLightModelfv )(GLenum pname, const GLfloat *params);
typedef void ( APIENTRY * pfn_glLightModeli )(GLenum pname, GLint param);
typedef void ( APIENTRY * pfn_glLightModeliv )(GLenum pname, const GLint *params);
typedef void ( APIENTRY * pfn_glLightf )(GLenum light, GLenum pname, GLfloat param);
typedef void ( APIENTRY * pfn_glLightfv )(GLenum light, GLenum pname, const GLfloat *params);
typedef void ( APIENTRY * pfn_glLighti )(GLenum light, GLenum pname, GLint param);
typedef void ( APIENTRY * pfn_glLightiv )(GLenum light, GLenum pname, const GLint *params);
typedef void ( APIENTRY * pfn_glLineStipple )(GLint factor, GLushort pattern);
typedef void ( APIENTRY * pfn_glLineWidth )(GLfloat width);
typedef void ( APIENTRY * pfn_glListBase )(GLuint base);
typedef void ( APIENTRY * pfn_glLoadIdentity )(void);
typedef void ( APIENTRY * pfn_glLoadMatrixd )(const GLdouble *m);
typedef void ( APIENTRY * pfn_glLoadMatrixf )(const GLfloat *m);
typedef void ( APIENTRY * pfn_glLoadName )(GLuint name);
typedef void ( APIENTRY * pfn_glLogicOp )(GLenum opcode);
typedef void ( APIENTRY * pfn_glMap1d )(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
typedef void ( APIENTRY * pfn_glMap1f )(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
typedef void ( APIENTRY * pfn_glMap2d )(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
typedef void ( APIENTRY * pfn_glMap2f )(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
typedef void ( APIENTRY * pfn_glMapGrid1d )(GLint un, GLdouble u1, GLdouble u2);
typedef void ( APIENTRY * pfn_glMapGrid1f )(GLint un, GLfloat u1, GLfloat u2);
typedef void ( APIENTRY * pfn_glMapGrid2d )(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
typedef void ( APIENTRY * pfn_glMapGrid2f )(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
typedef void ( APIENTRY * pfn_glMaterialf )(GLenum face, GLenum pname, GLfloat param);
typedef void ( APIENTRY * pfn_glMaterialfv )(GLenum face, GLenum pname, const GLfloat *params);
typedef void ( APIENTRY * pfn_glMateriali )(GLenum face, GLenum pname, GLint param);
typedef void ( APIENTRY * pfn_glMaterialiv )(GLenum face, GLenum pname, const GLint *params);
typedef void ( APIENTRY * pfn_glMatrixMode )(GLenum mode);
typedef void ( APIENTRY * pfn_glMultMatrixd )(const GLdouble *m);
typedef void ( APIENTRY * pfn_glMultMatrixf )(const GLfloat *m);
typedef void ( APIENTRY * pfn_glNewList )(GLuint list, GLenum mode);
typedef void ( APIENTRY * pfn_glNormal3b )(GLbyte nx, GLbyte ny, GLbyte nz);
typedef void ( APIENTRY * pfn_glNormal3bv )(const GLbyte *v);
typedef void ( APIENTRY * pfn_glNormal3d )(GLdouble nx, GLdouble ny, GLdouble nz);
typedef void ( APIENTRY * pfn_glNormal3dv )(const GLdouble *v);
typedef void ( APIENTRY * pfn_glNormal3f )(GLfloat nx, GLfloat ny, GLfloat nz);
typedef void ( APIENTRY * pfn_glNormal3fv )(const GLfloat *v);
typedef void ( APIENTRY * pfn_glNormal3i )(GLint nx, GLint ny, GLint nz);
typedef void ( APIENTRY * pfn_glNormal3iv )(const GLint *v);
typedef void ( APIENTRY * pfn_glNormal3s )(GLshort nx, GLshort ny, GLshort nz);
typedef void ( APIENTRY * pfn_glNormal3sv )(const GLshort *v);
typedef void ( APIENTRY * pfn_glNormalPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void ( APIENTRY * pfn_glOrtho )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
typedef void ( APIENTRY * pfn_glPassThrough )(GLfloat token);
typedef void ( APIENTRY * pfn_glPixelMapfv )(GLenum map, GLsizei mapsize, const GLfloat *values);
typedef void ( APIENTRY * pfn_glPixelMapuiv )(GLenum map, GLsizei mapsize, const GLuint *values);
typedef void ( APIENTRY * pfn_glPixelMapusv )(GLenum map, GLsizei mapsize, const GLushort *values);
typedef void ( APIENTRY * pfn_glPixelStoref )(GLenum pname, GLfloat param);
typedef void ( APIENTRY * pfn_glPixelStorei )(GLenum pname, GLint param);
typedef void ( APIENTRY * pfn_glPixelTransferf )(GLenum pname, GLfloat param);
typedef void ( APIENTRY * pfn_glPixelTransferi )(GLenum pname, GLint param);
typedef void ( APIENTRY * pfn_glPixelZoom )(GLfloat xfactor, GLfloat yfactor);
typedef void ( APIENTRY * pfn_glPointSize )(GLfloat size);
typedef void ( APIENTRY * pfn_glPolygonMode )(GLenum face, GLenum mode);
typedef void ( APIENTRY * pfn_glPolygonOffset )(GLfloat factor, GLfloat units);
typedef void ( APIENTRY * pfn_glPolygonStipple )(const GLubyte *mask);
typedef void ( APIENTRY * pfn_glPopAttrib )(void);
typedef void ( APIENTRY * pfn_glPopClientAttrib )(void);
typedef void ( APIENTRY * pfn_glPopMatrix )(void);
typedef void ( APIENTRY * pfn_glPopName )(void);
typedef void ( APIENTRY * pfn_glPrioritizeTextures )(GLsizei n, const GLuint *textures, const GLclampf *priorities);
typedef void ( APIENTRY * pfn_glPushAttrib )(GLbitfield mask);
typedef void ( APIENTRY * pfn_glPushClientAttrib )(GLbitfield mask);
typedef void ( APIENTRY * pfn_glPushMatrix )(void);
typedef void ( APIENTRY * pfn_glPushName )(GLuint name);
typedef void ( APIENTRY * pfn_glRasterPos2d )(GLdouble x, GLdouble y);
typedef void ( APIENTRY * pfn_glRasterPos2dv )(const GLdouble *v);
typedef void ( APIENTRY * pfn_glRasterPos2f )(GLfloat x, GLfloat y);
typedef void ( APIENTRY * pfn_glRasterPos2fv )(const GLfloat *v);
typedef void ( APIENTRY * pfn_glRasterPos2i )(GLint x, GLint y);
typedef void ( APIENTRY * pfn_glRasterPos2iv )(const GLint *v);
typedef void ( APIENTRY * pfn_glRasterPos2s )(GLshort x, GLshort y);
typedef void ( APIENTRY * pfn_glRasterPos2sv )(const GLshort *v);
typedef void ( APIENTRY * pfn_glRasterPos3d )(GLdouble x, GLdouble y, GLdouble z);
typedef void ( APIENTRY * pfn_glRasterPos3dv )(const GLdouble *v);
typedef void ( APIENTRY * pfn_glRasterPos3f )(GLfloat x, GLfloat y, GLfloat z);
typedef void ( APIENTRY * pfn_glRasterPos3fv )(const GLfloat *v);
typedef void ( APIENTRY * pfn_glRasterPos3i )(GLint x, GLint y, GLint z);
typedef void ( APIENTRY * pfn_glRasterPos3iv )(const GLint *v);
typedef void ( APIENTRY * pfn_glRasterPos3s )(GLshort x, GLshort y, GLshort z);
typedef void ( APIENTRY * pfn_glRasterPos3sv )(const GLshort *v);
typedef void ( APIENTRY * pfn_glRasterPos4d )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void ( APIENTRY * pfn_glRasterPos4dv )(const GLdouble *v);
typedef void ( APIENTRY * pfn_glRasterPos4f )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void ( APIENTRY * pfn_glRasterPos4fv )(const GLfloat *v);
typedef void ( APIENTRY * pfn_glRasterPos4i )(GLint x, GLint y, GLint z, GLint w);
typedef void ( APIENTRY * pfn_glRasterPos4iv )(const GLint *v);
typedef void ( APIENTRY * pfn_glRasterPos4s )(GLshort x, GLshort y, GLshort z, GLshort w);
typedef void ( APIENTRY * pfn_glRasterPos4sv )(const GLshort *v);
typedef void ( APIENTRY * pfn_glReadBuffer )(GLenum mode);
typedef void ( APIENTRY * pfn_glReadPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
typedef void ( APIENTRY * pfn_glRectd )(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
typedef void ( APIENTRY * pfn_glRectdv )(const GLdouble *v1, const GLdouble *v2);
typedef void ( APIENTRY * pfn_glRectf )(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
typedef void ( APIENTRY * pfn_glRectfv )(const GLfloat *v1, const GLfloat *v2);
typedef void ( APIENTRY * pfn_glRecti )(GLint x1, GLint y1, GLint x2, GLint y2);
typedef void ( APIENTRY * pfn_glRectiv )(const GLint *v1, const GLint *v2);
typedef void ( APIENTRY * pfn_glRects )(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
typedef void ( APIENTRY * pfn_glRectsv )(const GLshort *v1, const GLshort *v2);
typedef GLint ( APIENTRY * pfn_glRenderMode )(GLenum mode);
typedef void ( APIENTRY * pfn_glRotated )(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
typedef void ( APIENTRY * pfn_glRotatef )(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
typedef void ( APIENTRY * pfn_glScaled )(GLdouble x, GLdouble y, GLdouble z);
typedef void ( APIENTRY * pfn_glScalef )(GLfloat x, GLfloat y, GLfloat z);
typedef void ( APIENTRY * pfn_glScissor )(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void ( APIENTRY * pfn_glSelectBuffer )(GLsizei size, GLuint *buffer);
typedef void ( APIENTRY * pfn_glShadeModel )(GLenum mode);
typedef void ( APIENTRY * pfn_glStencilFunc )(GLenum func, GLint ref, GLuint mask);
typedef void ( APIENTRY * pfn_glStencilMask )(GLuint mask);
typedef void ( APIENTRY * pfn_glStencilOp )(GLenum fail, GLenum zfail, GLenum zpass);
typedef void ( APIENTRY * pfn_glTexCoord1d )(GLdouble s);
typedef void ( APIENTRY * pfn_glTexCoord1dv )(const GLdouble *v);
typedef void ( APIENTRY * pfn_glTexCoord1f )(GLfloat s);
typedef void ( APIENTRY * pfn_glTexCoord1fv )(const GLfloat *v);
typedef void ( APIENTRY * pfn_glTexCoord1i )(GLint s);
typedef void ( APIENTRY * pfn_glTexCoord1iv )(const GLint *v);
typedef void ( APIENTRY * pfn_glTexCoord1s )(GLshort s);
typedef void ( APIENTRY * pfn_glTexCoord1sv )(const GLshort *v);
typedef void ( APIENTRY * pfn_glTexCoord2d )(GLdouble s, GLdouble t);
typedef void ( APIENTRY * pfn_glTexCoord2dv )(const GLdouble *v);
typedef void ( APIENTRY * pfn_glTexCoord2f )(GLfloat s, GLfloat t);
typedef void ( APIENTRY * pfn_glTexCoord2fv )(const GLfloat *v);
typedef void ( APIENTRY * pfn_glTexCoord2i )(GLint s, GLint t);
typedef void ( APIENTRY * pfn_glTexCoord2iv )(const GLint *v);
typedef void ( APIENTRY * pfn_glTexCoord2s )(GLshort s, GLshort t);
typedef void ( APIENTRY * pfn_glTexCoord2sv )(const GLshort *v);
typedef void ( APIENTRY * pfn_glTexCoord3d )(GLdouble s, GLdouble t, GLdouble r);
typedef void ( APIENTRY * pfn_glTexCoord3dv )(const GLdouble *v);
typedef void ( APIENTRY * pfn_glTexCoord3f )(GLfloat s, GLfloat t, GLfloat r);
typedef void ( APIENTRY * pfn_glTexCoord3fv )(const GLfloat *v);
typedef void ( APIENTRY * pfn_glTexCoord3i )(GLint s, GLint t, GLint r);
typedef void ( APIENTRY * pfn_glTexCoord3iv )(const GLint *v);
typedef void ( APIENTRY * pfn_glTexCoord3s )(GLshort s, GLshort t, GLshort r);
typedef void ( APIENTRY * pfn_glTexCoord3sv )(const GLshort *v);
typedef void ( APIENTRY * pfn_glTexCoord4d )(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void ( APIENTRY * pfn_glTexCoord4dv )(const GLdouble *v);
typedef void ( APIENTRY * pfn_glTexCoord4f )(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void ( APIENTRY * pfn_glTexCoord4fv )(const GLfloat *v);
typedef void ( APIENTRY * pfn_glTexCoord4i )(GLint s, GLint t, GLint r, GLint q);
typedef void ( APIENTRY * pfn_glTexCoord4iv )(const GLint *v);
typedef void ( APIENTRY * pfn_glTexCoord4s )(GLshort s, GLshort t, GLshort r, GLshort q);
typedef void ( APIENTRY * pfn_glTexCoord4sv )(const GLshort *v);
typedef void ( APIENTRY * pfn_glTexCoordPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void ( APIENTRY * pfn_glTexEnvf )(GLenum target, GLenum pname, GLfloat param);
typedef void ( APIENTRY * pfn_glTexEnvfv )(GLenum target, GLenum pname, const GLfloat *params);
typedef void ( APIENTRY * pfn_glTexEnvi )(GLenum target, GLenum pname, GLint param);
typedef void ( APIENTRY * pfn_glTexEnviv )(GLenum target, GLenum pname, const GLint *params);
typedef void ( APIENTRY * pfn_glTexGend )(GLenum coord, GLenum pname, GLdouble param);
typedef void ( APIENTRY * pfn_glTexGendv )(GLenum coord, GLenum pname, const GLdouble *params);
typedef void ( APIENTRY * pfn_glTexGenf )(GLenum coord, GLenum pname, GLfloat param);
typedef void ( APIENTRY * pfn_glTexGenfv )(GLenum coord, GLenum pname, const GLfloat *params);
typedef void ( APIENTRY * pfn_glTexGeni )(GLenum coord, GLenum pname, GLint param);
typedef void ( APIENTRY * pfn_glTexGeniv )(GLenum coord, GLenum pname, const GLint *params);
typedef void ( APIENTRY * pfn_glTexImage1D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void ( APIENTRY * pfn_glTexImage2D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void ( APIENTRY * pfn_glTexParameterf )(GLenum target, GLenum pname, GLfloat param);
typedef void ( APIENTRY * pfn_glTexParameterfv )(GLenum target, GLenum pname, const GLfloat *params);
typedef void ( APIENTRY * pfn_glTexParameteri )(GLenum target, GLenum pname, GLint param);
typedef void ( APIENTRY * pfn_glTexParameteriv )(GLenum target, GLenum pname, const GLint *params);
typedef void ( APIENTRY * pfn_glTexSubImage1D )(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef void ( APIENTRY * pfn_glTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void ( APIENTRY * pfn_glTranslated )(GLdouble x, GLdouble y, GLdouble z);
typedef void ( APIENTRY * pfn_glTranslatef )(GLfloat x, GLfloat y, GLfloat z);
typedef void ( APIENTRY * pfn_glVertex2d )(GLdouble x, GLdouble y);
typedef void ( APIENTRY * pfn_glVertex2dv )(const GLdouble *v);
typedef void ( APIENTRY * pfn_glVertex2f )(GLfloat x, GLfloat y);
typedef void ( APIENTRY * pfn_glVertex2fv )(const GLfloat *v);
typedef void ( APIENTRY * pfn_glVertex2i )(GLint x, GLint y);
typedef void ( APIENTRY * pfn_glVertex2iv )(const GLint *v);
typedef void ( APIENTRY * pfn_glVertex2s )(GLshort x, GLshort y);
typedef void ( APIENTRY * pfn_glVertex2sv )(const GLshort *v);
typedef void ( APIENTRY * pfn_glVertex3d )(GLdouble x, GLdouble y, GLdouble z);
typedef void ( APIENTRY * pfn_glVertex3dv )(const GLdouble *v);
typedef void ( APIENTRY * pfn_glVertex3f )(GLfloat x, GLfloat y, GLfloat z);
typedef void ( APIENTRY * pfn_glVertex3fv )(const GLfloat *v);
typedef void ( APIENTRY * pfn_glVertex3i )(GLint x, GLint y, GLint z);
typedef void ( APIENTRY * pfn_glVertex3iv )(const GLint *v);
typedef void ( APIENTRY * pfn_glVertex3s )(GLshort x, GLshort y, GLshort z);
typedef void ( APIENTRY * pfn_glVertex3sv )(const GLshort *v);
typedef void ( APIENTRY * pfn_glVertex4d )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void ( APIENTRY * pfn_glVertex4dv )(const GLdouble *v);
typedef void ( APIENTRY * pfn_glVertex4f )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void ( APIENTRY * pfn_glVertex4fv )(const GLfloat *v);
typedef void ( APIENTRY * pfn_glVertex4i )(GLint x, GLint y, GLint z, GLint w);
typedef void ( APIENTRY * pfn_glVertex4iv )(const GLint *v);
typedef void ( APIENTRY * pfn_glVertex4s )(GLshort x, GLshort y, GLshort z, GLshort w);
typedef void ( APIENTRY * pfn_glVertex4sv )(const GLshort *v);
typedef void ( APIENTRY * pfn_glVertexPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void ( APIENTRY * pfn_glViewport )(GLint x, GLint y, GLsizei width, GLsizei height);

typedef int   ( WINAPI * pfn_wglChoosePixelFormat )(HDC, CONST PIXELFORMATDESCRIPTOR *);
typedef int   ( WINAPI * pfn_wglDescribePixelFormat) (HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
typedef int   ( WINAPI * pfn_wglGetPixelFormat)(HDC);
typedef BOOL  ( WINAPI * pfn_wglSetPixelFormat)(HDC, int, CONST PIXELFORMATDESCRIPTOR *);
typedef BOOL  ( WINAPI * pfn_wglSwapBuffers)(HDC);
typedef BOOL  ( WINAPI * pfn_wglCopyContext)(HGLRC, HGLRC, UINT);
typedef HGLRC ( WINAPI * pfn_wglCreateContext)(HDC);
typedef BOOL  ( WINAPI * pfn_wglDeleteContext)(HGLRC);
typedef HGLRC ( WINAPI * pfn_wglGetCurrentContext)(VOID);
typedef HDC   ( WINAPI * pfn_wglGetCurrentDC)(VOID);
typedef PROC  ( WINAPI * pfn_wglGetProcAddress)(LPCSTR);
typedef BOOL  ( WINAPI * pfn_wglMakeCurrent)(HDC, HGLRC);
typedef BOOL  ( WINAPI * pfn_wglShareLists)(HGLRC, HGLRC);
typedef HGLRC ( WINAPI * pfn_wglCreateLayerContext)( HDC hdc, int iLayerPlane );
typedef BOOL ( WINAPI * pfn_wglDescribeLayerPlane)( HDC hdc, int iPixelFormat, int iLayerPlane, UINT nBytes, LPLAYERPLANEDESCRIPTOR plpd );
typedef int ( WINAPI * pfn_wglGetLayerPaletteEntries)( HDC hdc, int iLayerPlane, int iStart, int cEntries, COLORREF *pcr );
typedef int ( WINAPI * pfn_wglSetLayerPaletteEntries)(HDC hdc, int iLayerPlane, int iStart, int cEntries, CONST COLORREF *pcr);
typedef BOOL ( WINAPI * pfn_wglRealizeLayerPalette)(HDC hdc, int iLayerPlane, BOOL b);
typedef BOOL ( WINAPI * pfn_wglSwapLayerBuffers)(HDC hdc, UINT i);
typedef BOOL ( WINAPI * pfn_wglUseFontBitmapsA)(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3);
typedef BOOL ( WINAPI * pfn_wglUseFontBitmapsW)(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3);
typedef BOOL ( WINAPI * pfn_wglUseFontOutlinesA)(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3, FLOAT f1, FLOAT f2, int i, LPGLYPHMETRICSFLOAT pgmf);
typedef BOOL ( WINAPI * pfn_wglUseFontOutlinesW)(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3, FLOAT f1, FLOAT f2, int i, LPGLYPHMETRICSFLOAT pgmf);
typedef BOOL ( WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

typedef struct
{
	GLenum	edgeClampMode;
	int		maxActiveTextures;
	int		maxAnisotropy;
	unsigned char 
			Multitexture	: 1,
			VertexShader	: 1,
			FragmentShader	: 1,
			RectangleTexture: 1,
			DepthTexture	: 1,
			FBO				: 1,
			MultisampleHint : 1,
							: 1;
} ExtensionSupport_t;

namespace gl
{
extern pfn_glAccum qglAccum;
extern pfn_glAlphaFunc qglAlphaFunc;
extern pfn_glAreTexturesResident qglAreTexturesResident;
extern pfn_glArrayElement qglArrayElement;
extern pfn_glBegin qglBegin;
extern pfn_glBindTexture qglBindTexture;
extern pfn_glBitmap qglBitmap;
extern pfn_glBlendFunc qglBlendFunc;
extern pfn_glCallList qglCallList;
extern pfn_glCallLists qglCallLists;
extern pfn_glClear qglClear;
extern pfn_glClearAccum qglClearAccum;
extern pfn_glClearColor qglClearColor;
extern pfn_glClearDepth qglClearDepth;
extern pfn_glClearIndex qglClearIndex;
extern pfn_glClearStencil qglClearStencil;
extern pfn_glClipPlane qglClipPlane;
extern pfn_glColor3b qglColor3b;
extern pfn_glColor3bv qglColor3bv;
extern pfn_glColor3d qglColor3d;
extern pfn_glColor3dv qglColor3dv;
extern pfn_glColor3f qglColor3f;
extern pfn_glColor3fv qglColor3fv;
extern pfn_glColor3i qglColor3i;
extern pfn_glColor3iv qglColor3iv;
extern pfn_glColor3s qglColor3s;
extern pfn_glColor3sv qglColor3sv;
extern pfn_glColor3ub qglColor3ub;
extern pfn_glColor3ubv qglColor3ubv;
extern pfn_glColor3ui qglColor3ui;
extern pfn_glColor3uiv qglColor3uiv;
extern pfn_glColor3us qglColor3us;
extern pfn_glColor3usv qglColor3usv;
extern pfn_glColor4b qglColor4b;
extern pfn_glColor4bv qglColor4bv;
extern pfn_glColor4d qglColor4d;
extern pfn_glColor4dv qglColor4dv;
extern pfn_glColor4f qglColor4f;
extern pfn_glColor4fv qglColor4fv;
extern pfn_glColor4i qglColor4i;
extern pfn_glColor4iv qglColor4iv;
extern pfn_glColor4s qglColor4s;
extern pfn_glColor4sv qglColor4sv;
extern pfn_glColor4ub qglColor4ub;
extern pfn_glColor4ubv qglColor4ubv;
extern pfn_glColor4ui qglColor4ui;
extern pfn_glColor4uiv qglColor4uiv;
extern pfn_glColor4us qglColor4us;
extern pfn_glColor4usv qglColor4usv;
extern pfn_glColorMask qglColorMask;
extern pfn_glColorMaterial qglColorMaterial;
extern pfn_glColorPointer qglColorPointer;
extern pfn_glCopyPixels qglCopyPixels;
extern pfn_glCopyTexImage1D qglCopyTexImage1D;
extern pfn_glCopyTexImage2D qglCopyTexImage2D;
extern pfn_glCopyTexSubImage1D qglCopyTexSubImage1D;
extern pfn_glCopyTexSubImage2D qglCopyTexSubImage2D;
extern pfn_glCullFace qglCullFace;
extern pfn_glDeleteLists qglDeleteLists;
extern pfn_glDeleteTextures qglDeleteTextures;
extern pfn_glDepthFunc qglDepthFunc;
extern pfn_glDepthMask qglDepthMask;
extern pfn_glDepthRange qglDepthRange;
extern pfn_glDisable qglDisable;
extern pfn_glDisableClientState qglDisableClientState;
extern pfn_glDrawArrays qglDrawArrays;
extern pfn_glDrawBuffer qglDrawBuffer;
extern pfn_glDrawElements qglDrawElements;
extern pfn_glDrawPixels qglDrawPixels;
extern pfn_glEdgeFlag qglEdgeFlag;
extern pfn_glEdgeFlagPointer qglEdgeFlagPointer;
extern pfn_glEdgeFlagv qglEdgeFlagv;
extern pfn_glEnable qglEnable;
extern pfn_glEnableClientState qglEnableClientState;
extern pfn_glEnd qglEnd;
extern pfn_glEndList qglEndList;
extern pfn_glEvalCoord1d qglEvalCoord1d;
extern pfn_glEvalCoord1dv qglEvalCoord1dv;
extern pfn_glEvalCoord1f qglEvalCoord1f;
extern pfn_glEvalCoord1fv qglEvalCoord1fv;
extern pfn_glEvalCoord2d qglEvalCoord2d;
extern pfn_glEvalCoord2dv qglEvalCoord2dv;
extern pfn_glEvalCoord2f qglEvalCoord2f;
extern pfn_glEvalCoord2fv qglEvalCoord2fv;
extern pfn_glEvalMesh1 qglEvalMesh1;
extern pfn_glEvalMesh2 qglEvalMesh2;
extern pfn_glEvalPoint1 qglEvalPoint1;
extern pfn_glEvalPoint2 qglEvalPoint2;
extern pfn_glFeedbackBuffer qglFeedbackBuffer;
extern pfn_glFinish qglFinish;
extern pfn_glFlush qglFlush;
extern pfn_glFogf qglFogf;
extern pfn_glFogfv qglFogfv;
extern pfn_glFogi qglFogi;
extern pfn_glFogiv qglFogiv;
extern pfn_glFrontFace qglFrontFace;
extern pfn_glFrustum qglFrustum;
extern pfn_glGenLists qglGenLists;
extern pfn_glGenTextures qglGenTextures;
extern pfn_glGetBooleanv qglGetBooleanv;
extern pfn_glGetClipPlane qglGetClipPlane;
extern pfn_glGetDoublev qglGetDoublev;
extern pfn_glGetError qglGetError;
extern pfn_glGetFloatv qglGetFloatv;
extern pfn_glGetIntegerv qglGetIntegerv;
extern pfn_glGetLightfv qglGetLightfv;
extern pfn_glGetLightiv qglGetLightiv;
extern pfn_glGetMapdv qglGetMapdv;
extern pfn_glGetMapfv qglGetMapfv;
extern pfn_glGetMapiv qglGetMapiv;
extern pfn_glGetMaterialfv qglGetMaterialfv;
extern pfn_glGetMaterialiv qglGetMaterialiv;
extern pfn_glGetPixelMapfv qglGetPixelMapfv;
extern pfn_glGetPixelMapuiv qglGetPixelMapuiv;
extern pfn_glGetPixelMapusv qglGetPixelMapusv;
extern pfn_glGetPointerv qglGetPointerv;
extern pfn_glGetPolygonStipple qglGetPolygonStipple;
extern pfn_glGetString qglGetString;
extern pfn_glGetTexEnvfv qglGetTexEnvfv;
extern pfn_glGetTexEnviv qglGetTexEnviv;
extern pfn_glGetTexGendv qglGetTexGendv;
extern pfn_glGetTexGenfv qglGetTexGenfv;
extern pfn_glGetTexGeniv qglGetTexGeniv;
extern pfn_glGetTexImage qglGetTexImage;
extern pfn_glGetTexLevelParameterfv qglGetTexLevelParameterfv;
extern pfn_glGetTexLevelParameteriv qglGetTexLevelParameteriv;
extern pfn_glGetTexParameterfv qglGetTexParameterfv;
extern pfn_glGetTexParameteriv qglGetTexParameteriv;
extern pfn_glHint qglHint;
extern pfn_glIndexMask qglIndexMask;
extern pfn_glIndexPointer qglIndexPointer;
extern pfn_glIndexd qglIndexd;
extern pfn_glIndexdv qglIndexdv;
extern pfn_glIndexf qglIndexf;
extern pfn_glIndexfv qglIndexfv;
extern pfn_glIndexi qglIndexi;
extern pfn_glIndexiv qglIndexiv;
extern pfn_glIndexs qglIndexs;
extern pfn_glIndexsv qglIndexsv;
extern pfn_glIndexub qglIndexub;
extern pfn_glIndexubv qglIndexubv;
extern pfn_glInitNames qglInitNames;
extern pfn_glInterleavedArrays qglInterleavedArrays;
extern pfn_glIsEnabled qglIsEnabled;
extern pfn_glIsList qglIsList;
extern pfn_glIsTexture qglIsTexture;
extern pfn_glLightModelf qglLightModelf;
extern pfn_glLightModelfv qglLightModelfv;
extern pfn_glLightModeli qglLightModeli;
extern pfn_glLightModeliv qglLightModeliv;
extern pfn_glLightf qglLightf;
extern pfn_glLightfv qglLightfv;
extern pfn_glLighti qglLighti;
extern pfn_glLightiv qglLightiv;
extern pfn_glLineStipple qglLineStipple;
extern pfn_glLineWidth qglLineWidth;
extern pfn_glListBase qglListBase;
extern pfn_glLoadIdentity qglLoadIdentity;
extern pfn_glLoadMatrixd qglLoadMatrixd;
extern pfn_glLoadMatrixf qglLoadMatrixf;
extern pfn_glLoadName qglLoadName;
extern pfn_glLogicOp qglLogicOp;
extern pfn_glMap1d qglMap1d;
extern pfn_glMap1f qglMap1f;
extern pfn_glMap2d qglMap2d;
extern pfn_glMap2f qglMap2f;
extern pfn_glMapGrid1d qglMapGrid1d;
extern pfn_glMapGrid1f qglMapGrid1f;
extern pfn_glMapGrid2d qglMapGrid2d;
extern pfn_glMapGrid2f qglMapGrid2f;
extern pfn_glMaterialf qglMaterialf;
extern pfn_glMaterialfv qglMaterialfv;
extern pfn_glMateriali qglMateriali;
extern pfn_glMaterialiv qglMaterialiv;
extern pfn_glMatrixMode qglMatrixMode;
extern pfn_glMultMatrixd qglMultMatrixd;
extern pfn_glMultMatrixf qglMultMatrixf;
extern pfn_glNewList qglNewList;
extern pfn_glNormal3b qglNormal3b;
extern pfn_glNormal3bv qglNormal3bv;
extern pfn_glNormal3d qglNormal3d;
extern pfn_glNormal3dv qglNormal3dv;
extern pfn_glNormal3f qglNormal3f;
extern pfn_glNormal3fv qglNormal3fv;
extern pfn_glNormal3i qglNormal3i;
extern pfn_glNormal3iv qglNormal3iv;
extern pfn_glNormal3s qglNormal3s;
extern pfn_glNormal3sv qglNormal3sv;
extern pfn_glNormalPointer qglNormalPointer;
extern pfn_glOrtho qglOrtho;
extern pfn_glPassThrough qglPassThrough;
extern pfn_glPixelMapfv qglPixelMapfv;
extern pfn_glPixelMapuiv qglPixelMapuiv;
extern pfn_glPixelMapusv qglPixelMapusv;
extern pfn_glPixelStoref qglPixelStoref;
extern pfn_glPixelStorei qglPixelStorei;
extern pfn_glPixelTransferf qglPixelTransferf;
extern pfn_glPixelTransferi qglPixelTransferi;
extern pfn_glPixelZoom qglPixelZoom;
extern pfn_glPointSize qglPointSize;
extern pfn_glPolygonMode qglPolygonMode;
extern pfn_glPolygonOffset qglPolygonOffset;
extern pfn_glPolygonStipple qglPolygonStipple;
extern pfn_glPopAttrib qglPopAttrib;
extern pfn_glPopClientAttrib qglPopClientAttrib;
extern pfn_glPopMatrix qglPopMatrix;
extern pfn_glPopName qglPopName;
extern pfn_glPrioritizeTextures qglPrioritizeTextures;
extern pfn_glPushAttrib qglPushAttrib;
extern pfn_glPushClientAttrib qglPushClientAttrib;
extern pfn_glPushMatrix qglPushMatrix;
extern pfn_glPushName qglPushName;
extern pfn_glRasterPos2d qglRasterPos2d;
extern pfn_glRasterPos2dv qglRasterPos2dv;
extern pfn_glRasterPos2f qglRasterPos2f;
extern pfn_glRasterPos2fv qglRasterPos2fv;
extern pfn_glRasterPos2i qglRasterPos2i;
extern pfn_glRasterPos2iv qglRasterPos2iv;
extern pfn_glRasterPos2s qglRasterPos2s;
extern pfn_glRasterPos2sv qglRasterPos2sv;
extern pfn_glRasterPos3d qglRasterPos3d;
extern pfn_glRasterPos3dv qglRasterPos3dv;
extern pfn_glRasterPos3f qglRasterPos3f;
extern pfn_glRasterPos3fv qglRasterPos3fv;
extern pfn_glRasterPos3i qglRasterPos3i;
extern pfn_glRasterPos3iv qglRasterPos3iv;
extern pfn_glRasterPos3s qglRasterPos3s;
extern pfn_glRasterPos3sv qglRasterPos3sv;
extern pfn_glRasterPos4d qglRasterPos4d;
extern pfn_glRasterPos4dv qglRasterPos4dv;
extern pfn_glRasterPos4f qglRasterPos4f;
extern pfn_glRasterPos4fv qglRasterPos4fv;
extern pfn_glRasterPos4i qglRasterPos4i;
extern pfn_glRasterPos4iv qglRasterPos4iv;
extern pfn_glRasterPos4s qglRasterPos4s;
extern pfn_glRasterPos4sv qglRasterPos4sv;
extern pfn_glReadBuffer qglReadBuffer;
extern pfn_glReadPixels qglReadPixels;
extern pfn_glRectd qglRectd;
extern pfn_glRectdv qglRectdv;
extern pfn_glRectf qglRectf;
extern pfn_glRectfv qglRectfv;
extern pfn_glRecti qglRecti;
extern pfn_glRectiv qglRectiv;
extern pfn_glRects qglRects;
extern pfn_glRectsv qglRectsv;
extern pfn_glRenderMode qglRenderMode;
extern pfn_glRotated qglRotated;
extern pfn_glRotatef qglRotatef;
extern pfn_glScaled qglScaled;
extern pfn_glScalef qglScalef;
extern pfn_glScissor qglScissor;
extern pfn_glSelectBuffer qglSelectBuffer;
extern pfn_glShadeModel qglShadeModel;
extern pfn_glStencilFunc qglStencilFunc;
extern pfn_glStencilMask qglStencilMask;
extern pfn_glStencilOp qglStencilOp;
extern pfn_glTexCoord1d qglTexCoord1d;
extern pfn_glTexCoord1dv qglTexCoord1dv;
extern pfn_glTexCoord1f qglTexCoord1f;
extern pfn_glTexCoord1fv qglTexCoord1fv;
extern pfn_glTexCoord1i qglTexCoord1i;
extern pfn_glTexCoord1iv qglTexCoord1iv;
extern pfn_glTexCoord1s qglTexCoord1s;
extern pfn_glTexCoord1sv qglTexCoord1sv;
extern pfn_glTexCoord2d qglTexCoord2d;
extern pfn_glTexCoord2dv qglTexCoord2dv;
extern pfn_glTexCoord2f qglTexCoord2f;
extern pfn_glTexCoord2fv qglTexCoord2fv;
extern pfn_glTexCoord2i qglTexCoord2i;
extern pfn_glTexCoord2iv qglTexCoord2iv;
extern pfn_glTexCoord2s qglTexCoord2s;
extern pfn_glTexCoord2sv qglTexCoord2sv;
extern pfn_glTexCoord3d qglTexCoord3d;
extern pfn_glTexCoord3dv qglTexCoord3dv;
extern pfn_glTexCoord3f qglTexCoord3f;
extern pfn_glTexCoord3fv qglTexCoord3fv;
extern pfn_glTexCoord3i qglTexCoord3i;
extern pfn_glTexCoord3iv qglTexCoord3iv;
extern pfn_glTexCoord3s qglTexCoord3s;
extern pfn_glTexCoord3sv qglTexCoord3sv;
extern pfn_glTexCoord4d qglTexCoord4d;
extern pfn_glTexCoord4dv qglTexCoord4dv;
extern pfn_glTexCoord4f qglTexCoord4f;
extern pfn_glTexCoord4fv qglTexCoord4fv;
extern pfn_glTexCoord4i qglTexCoord4i;
extern pfn_glTexCoord4iv qglTexCoord4iv;
extern pfn_glTexCoord4s qglTexCoord4s;
extern pfn_glTexCoord4sv qglTexCoord4sv;
extern pfn_glTexCoordPointer qglTexCoordPointer;
extern pfn_glTexEnvf qglTexEnvf;
extern pfn_glTexEnvfv qglTexEnvfv;
extern pfn_glTexEnvi qglTexEnvi;
extern pfn_glTexEnviv qglTexEnviv;
extern pfn_glTexGend qglTexGend;
extern pfn_glTexGendv qglTexGendv;
extern pfn_glTexGenf qglTexGenf;
extern pfn_glTexGenfv qglTexGenfv;
extern pfn_glTexGeni qglTexGeni;
extern pfn_glTexGeniv qglTexGeniv;
extern pfn_glTexImage1D qglTexImage1D;
extern pfn_glTexImage2D qglTexImage2D;
extern pfn_glTexParameterf qglTexParameterf;
extern pfn_glTexParameterfv qglTexParameterfv;
extern pfn_glTexParameteri qglTexParameteri;
extern pfn_glTexParameteriv qglTexParameteriv;
extern pfn_glTexSubImage1D qglTexSubImage1D;
extern pfn_glTexSubImage2D qglTexSubImage2D;
extern pfn_glTranslated qglTranslated;
extern pfn_glTranslatef qglTranslatef;
extern pfn_glVertex2d qglVertex2d;
extern pfn_glVertex2dv qglVertex2dv;
extern pfn_glVertex2f qglVertex2f;
extern pfn_glVertex2fv qglVertex2fv;
extern pfn_glVertex2i qglVertex2i;
extern pfn_glVertex2iv qglVertex2iv;
extern pfn_glVertex2s qglVertex2s;
extern pfn_glVertex2sv qglVertex2sv;
extern pfn_glVertex3d qglVertex3d;
extern pfn_glVertex3dv qglVertex3dv;
extern pfn_glVertex3f qglVertex3f;
extern pfn_glVertex3fv qglVertex3fv;
extern pfn_glVertex3i qglVertex3i;
extern pfn_glVertex3iv qglVertex3iv;
extern pfn_glVertex3s qglVertex3s;
extern pfn_glVertex3sv qglVertex3sv;
extern pfn_glVertex4d qglVertex4d;
extern pfn_glVertex4dv qglVertex4dv;
extern pfn_glVertex4f qglVertex4f;
extern pfn_glVertex4fv qglVertex4fv;
extern pfn_glVertex4i qglVertex4i;
extern pfn_glVertex4iv qglVertex4iv;
extern pfn_glVertex4s qglVertex4s;
extern pfn_glVertex4sv qglVertex4sv;
extern pfn_glVertexPointer qglVertexPointer;
extern pfn_glViewport qglViewport;
extern pfn_wglChoosePixelFormat	wglChoosePixelFormat;
extern pfn_wglDescribePixelFormat wglDescribePixelFormat;
extern pfn_wglGetPixelFormat wglGetPixelFormat;
extern pfn_wglSetPixelFormat wglSetPixelFormat;
extern pfn_wglSwapBuffers wglSwapBuffers;
extern pfn_wglCopyContext wglCopyContext;
extern pfn_wglCreateContext wglCreateContext;
extern pfn_wglDeleteContext	wglDeleteContext;
extern pfn_wglGetCurrentContext wglGetCurrentContext;
extern pfn_wglGetCurrentDC wglGetCurrentDC;
extern pfn_wglGetProcAddress wglGetProcAddress;
extern pfn_wglMakeCurrent wglMakeCurrent;
extern pfn_wglShareLists wglShareLists;
extern pfn_wglCreateLayerContext wglCreateLayerContext;
extern pfn_wglDescribeLayerPlane wglDescribeLayerPlane;
extern pfn_wglGetLayerPaletteEntries wglGetLayerPaletteEntries;
extern pfn_wglSetLayerPaletteEntries wglSetLayerPaletteEntries;
extern pfn_wglRealizeLayerPalette wglRealizeLayerPalette;
extern pfn_wglSwapLayerBuffers wglSwapLayerBuffers;
extern pfn_wglUseFontBitmapsA wglUseFontBitmapsA;
extern pfn_wglUseFontBitmapsW wglUseFontBitmapsW;
extern pfn_wglUseFontOutlinesA wglUseFontOutlinesA;
extern pfn_wglUseFontOutlinesW wglUseFontOutlinesW;

// GL extensions
extern PFNGLACTIVETEXTUREARBPROC qglActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC qglClientActiveTextureARB;
extern PFNGLMULTITEXCOORD2FARBPROC qglMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2FVARBPROC qglMultiTexCoord2fvARB;

extern PFNGLBINDFRAMEBUFFERPROC qglBindFramebuffer;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC qglCheckFramebufferStatus;
extern PFNGLDELETEFRAMEBUFFERSPROC qglDeleteFramebuffers;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC qglFramebufferTexture2D;
extern PFNGLFRAMEBUFFERTEXTUREPROC qglFramebufferTexture;
extern PFNGLGENFRAMEBUFFERSPROC qglGenFramebuffers;
extern PFNGLGENERATEMIPMAPPROC qglGenerateMipmap;

extern PFNGLBINDFRAMEBUFFERPROC qglBindFramebufferEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC qglCheckFramebufferStatusEXT;
extern PFNGLDELETEFRAMEBUFFERSPROC qglDeleteFramebuffersEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC qglFramebufferTexture2DEXT;
extern PFNGLFRAMEBUFFERTEXTUREPROC qglFramebufferTextureEXT;
extern PFNGLGENFRAMEBUFFERSPROC qglGenFramebuffersEXT;
extern PFNGLGENERATEMIPMAPPROC qglGenerateMipmapEXT;

extern PFNGLGETHANDLEARBPROC qglGetHandleARB;
extern PFNGLDELETEOBJECTARBPROC qglDeleteObjectARB;
extern PFNGLDETACHOBJECTARBPROC qglDetachObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC qglCreateShaderObjectARB;
extern PFNGLSHADERSOURCEARBPROC qglShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC qglCompileShaderARB;
extern PFNGLCREATEPROGRAMOBJECTARBPROC qglCreateProgramObjectARB;
extern PFNGLATTACHOBJECTARBPROC qglAttachObjectARB;
extern PFNGLLINKPROGRAMARBPROC	qglLinkProgramARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC qglUseProgramObjectARB;
extern PFNGLVALIDATEPROGRAMARBPROC	qglValidateProgramARB;
extern PFNGLGETUNIFORMLOCATIONARBPROC qglGetUniformLocationARB;
extern PFNGLUNIFORM1IARBPROC qglUniform1iARB;
extern PFNGLUNIFORM4FARBPROC qglUniform4fARB;
extern PFNGLUNIFORM4FVARBPROC qglUniform4fvARB;
extern PFNGLGETINFOLOGARBPROC qglGetInfoLogARB;
extern PFNGLGETOBJECTPARAMETERIVARBPROC qglGetObjectParameterivARB;
extern PFNGLGETATTACHEDOBJECTSARBPROC qglGetAttachedObjectsARB;

extern bool Initialize( void );
extern void InitializeExtensions( void );
extern void CheckInit( void );
extern void Shutdown( void );

extern ExtensionSupport_t ext;
}

#endif //QFX_OPENGL_H
