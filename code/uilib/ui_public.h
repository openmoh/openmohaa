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

#ifndef __UI_PUBLIC_H__
#define __UI_PUBLIC_H__

class Event;
class Listener;

typedef int uihandle_t;
typedef struct tiki_s tiki_t;
/*typedef struct refEntity_t;
typedef struct polyVert_t;
typedef struct refdef_t;
typedef struct glconfig_t;*/
typedef struct fontheader_s fontheader_t;
typedef struct AliasListNode_s AliasListNode_t;

typedef struct {
	connstate_t		connState;
	int				connectPacketCount;
	int				clientNum;
	char			servername[ MAX_STRING_CHARS ];
	char			updateInfoString[ MAX_STRING_CHARS ];
	char			messageString[ MAX_STRING_CHARS ];
} uiClientState_t;

typedef enum {
	UIMENU_NONE,
	UIMENU_MAIN,
	UIMENU_INGAME,
	UIMENU_NEED_CD,
	UIMENU_BAD_CD_KEY,
	UIMENU_TEAM,
	UIMENU_POSTGAME
} uiMenuCommand_t;

#define SORT_HOST			0
#define SORT_MAP			1
#define SORT_CLIENTS		2
#define SORT_GAME			3
#define SORT_PING			4

typedef struct uiimport_s {
	uihandle_t( *Rend_RegisterMaterial )( const char *name );
	uihandle_t( *Rend_RefreshMaterial )( const char *name );
	void( *Rend_Set2D )( int x, int y, int w, int h, float left, float right, float bottom, float top, float n, float f );
	void( *Rend_SetColor )( const vec4_t rgba );
	void( *Rend_Scissor )( int x, int y, int width, int height );
	void( *Rend_DrawPicStretched )( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
	void( *Rend_DrawPicTiled )( float x, float y, float w, float h, qhandle_t hShader );
	fontheader_t *( *Rend_LoadFont )( const char *name );
	void( *Rend_DrawString )( fontheader_t *font, const char *text, float x, float y, int maxlen, qboolean bVirtualScreen );
	void( *Rend_DrawBox )( float x, float y, float w, float h );
	int( *Rend_GetShaderWidth )( qhandle_t hShader );
	int( *Rend_GetShaderHeight )( qhandle_t hShader );
	void ( *File_PickFile )( const char *name, Listener *obj, Event& event );
	void ( *File_ListFiles )( const char *filespec );
	long ( *File_OpenFile )( const char *qpath, void **buffer );
	void ( *File_FreeFile )( void *buffer );
	void ( *File_WriteFile )( const char *qpath, const void *buffer, int size );
	uihandle_t ( *Snd_RegisterSound )( const char *sample, qboolean streamed );
	void ( *Snd_PlaySound )( const char *sound_name );
	qboolean ( *Alias_Add )( const char *alias, const char *name, const char *parameters );
	const char *( *Alias_FindRandom )( const char *alias, AliasListNode_t **ret );
	const char *( *Cvar_GetString )( const char *name, const char *defval );
	cvar_t *( *Cvar_Find )( const char *var_name );
	void ( *Cvar_Set )( const char *var_name, const char *value );
	void ( *Cvar_Reset )( const char *var_name );
	void ( *Cmd_Stuff )( const char *text );
	void ( *Sys_Printf )( const char *text, ... );
	void ( *Sys_Error )( int error, const char *text, ... );
	void ( *Sys_DPrintf )( const  char *text, ... );
	int ( *Sys_Milliseconds )();
	int ( *Sys_IsKeyDown )( int key );
	const char *( *Sys_GetClipboard )( void );
	void( *Sys_SetClipboard )( const char *foo );
	const char *( *Cmd_CompleteCommandByNumber )( const char *partial, int number );
	const char *( *Cvar_CompleteCvarByNumber )( const char *partial, int number );
	void( *UI_WantsKeyboard )( void );
	const char *( *Client_TranslateWidgetName )( const char *widget );
	void( *Connect )( const char *server );
	const char *( *Key_GetKeynameForCommand )( const char *command );
	const char *( *Key_GetCommandForKey )( int keynum );
	void ( *Key_SetBinding )( int keynum, const char *binding );
	void ( *Key_GetKeysForCommand )( const char *command, int *key1, int *key2 );
	const char *( *Key_KeynumToString )( int keynum );
	const char *( *GetConfigstring )( int index );
	void ( *UI_CloseDMConsole )( void );

	/*
	cvar_t*			(*Cvar_Get)( const char *var_name, const char *var_value, int flags );
	int				(*Argc)( void );
	char*			(*Argv)( int arg );
	void			(*Cmd_ExecuteText)( int exec_when, const char *text );	// don't use EXEC_NOW!
	int				(*FS_FOpenFile)( const char *qpath, fileHandle_t *f, fsMode_t mode );
	int				(*FS_Read)( void *buffer, int len, fileHandle_t f );
	int				(*FS_Write)( const void *buffer, int len, fileHandle_t f );
	void			(*FS_FCloseFile)( fileHandle_t f );
	int				(*FS_GetFileList)( const char *path, const char *extension, char *listbuf, int bufsize );
	int				(*FS_Seek)( fileHandle_t f, long offset, fsOrigin_t origin ); // fsOrigin_t
	qhandle_t		(*R_RegisterModel)( const char *name );
	qhandle_t		(*R_RegisterSkin)( const char *name );
	qhandle_t		(*R_RegisterShader)( const char *name );
	qhandle_t		(*R_RegisterShaderNoMip)( const char *name );
	void			(*R_ClearScene)( void );
	void			(*R_AddRefEntityToScene)( const refEntity_t *re, int parentEntityNumber );
	qboolean		(*R_AddPolyToScene)( qhandle_t hShader, int numVerts, const polyVert_t *verts, int renderfx );
	void			(*R_AddLightToScene)( const vec3_t org, float intensity, float r, float g, float b, int type );
	void			(*R_RenderScene)( const refdef_t *fd );
	void			(*R_SetColor)( const float *rgba );
	void			(*R_DrawStretchPic)( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
	void			(*R_RotatedPic)( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader, float angle );
	void			(*UpdateScreen)( void );
	int				(*CM_LerpTag)( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, float frac, const char *tagName );
	void			(*S_StartLocalSound)( const char *sound_name, qboolean force_load );
	sfxHandle_t		(*S_RegisterSound)( const char *sample, qboolean compressed, qboolean force_load );
	void			(*Key_KeynumToStringBuf)( int keynum, char *buf, int buflen );
	void			(*Key_GetBindingBuf)( int keynum, char *buf, int buflen );
	qboolean		(*Key_IsDown)( int keynum );
	qboolean		(*Key_GetOverstrikeMode)( void );
	void			(*Key_SetOverstrikeMode)( qboolean state );
	void			(*Key_ClearStates)( void );
	int				(*Key_GetCatcher)( void );
	void			(*Key_SetCatcher)( int catcher );
	void			(*GetClipboardData)( char *buf, int bufsize );
	void			(*GetClientState)( uiClientState_t *state );
	void			(*GetGlconfig)( glconfig_t *glconfig );
	int				(*LAN_GetServerCount)( int source );
	void			(*LAN_GetServerAddressString)( int source, int n, char *buf, int buflen );
	void			(*LAN_GetServerInfo)( int source, int n, char *buf, int buflen );
	int				(*LAN_GetPingQueueCount)( void );
	int				(*LAN_ServerStatus)( const char *serverAddress, char *serverStatus, int maxLen );
	void			(*LAN_ClearPing)( int n );
	void			(*LAN_GetPing)( int n, char *buf, int buflen, int *pingtime );
	void			(*LAN_GetPingInfo)( int n, char *buf, int buflen );
	int				(*LAN_GetServerPing)( int source, int n );
	void			(*LAN_MarkServerVisible)( int source, int n, qboolean visible );
	qboolean		(*LAN_UpdateVisiblePings)( int source );
	int				(*MemoryRemaining)( void );
	void			(*GetCDKey)( char *buf, int buflen );
	void			(*SetCDKey)( char *buf );
	qboolean		(*VerifyCDKey)( const char *key, const char *chksum );
	void			(*SetPbClStatus)( int status );
	void			(*R_RegisterFont)( const char *fontName, int pointSize, fontInfo_t *font );
	int				(*R_Text_Width)( fontInfo_t *font, const char *text, int limit, qboolean useColourCodes );
	int				(*R_Text_Height)( fontInfo_t *font, const char *text, int limit, qboolean useColourCodes );
	void			(*R_Text_Paint)( fontInfo_t *font, float x, float y, float scale, float alpha, const char *text, float adjust, int limit, qboolean useColourCodes, qboolean is640 );
	void			(*R_Text_PaintChar)( fontInfo_t *font, float x, float y, float scale, int c, qboolean is640 );
	dtiki_t*		(*TIKI_RegisterModel)( const char *fname );
	bone_t*			(*TIKI_GetBones)( int numBones );
	void			(*TIKI_SetChannels)( tiki_t *tiki, int animIndex, float animTime, float animWeight, bone_t *bones );
	void			(*TIKI_AppendFrameBoundsAndRadius)( struct tiki_s *tiki, int animIndex, float animTime, float *outRadius, vec3_t outBounds[ 2 ] );
	void			(*TIKI_Animate)( tiki_t *tiki, bone_t *bones );
	int				(*TIKI_GetBoneNameIndex)( const char *boneName );
	const char*		(*R_GetShaderName)( qhandle_t shader );
	*/
} uiimport_t;

#if 1
typedef struct uiexport_s {
	void ( *AddFileToList )( const char *name );
	void ( *ResolutionChange )( void );
	void ( *Init )( void );
	void ( *Shutdown )( void );
	int ( *FontStringWidth )( fontheader_t *pFont, const char *pszString, int iMaxLen );
} uiexport_t;
#else

typedef struct uiexport_s {
	void		(*Init)( void );
	void		(*Shutdown)( void );
	void		(*KeyEvent)( int key, int down );
	void		(*MouseEvent)( int dx, int dy );
	void		(*Refresh)( int time );
	void		(*DrawHUD)( playerState_t *ps );
	qboolean	(*IsFullscreen)( void );
	void		(*SetActiveMenu)( uiMenuCommand_t menu );
	qboolean	(*ConsoleCommand)( int realTime );
	void		(*DrawConnectScreen)( qboolean overlay );
	void		(*AddFileToList)( const char *name );
	void		(*ResolutionChange)( void );
	int			(*FontStringWidth)( fontheader_t *pFont, const char *pszString, int iMaxLen );
} uiexport_t;

#endif

typedef struct uidef_s {
	int time;
	int vidWidth;
	int vidHeight;
	int mouseX;
	int mouseY;
	unsigned int mouseFlags;
	int uiHasMouse;
} uidef_t;

extern uidef_t uid;
extern uiexport_t uie;
extern uiimport_t uii;

void UI_Init(void);
void UI_Shutdown(void);
void UI_InitExports();
qboolean CL_FinishedIntro(void);
void UI_StartStageKeyEvent(void);
void UI_GetMouseState( int *mouseX, int *mouseY, int *flags );
int UI_GetCvarInt( const char *name, int def );
float UI_GetCvarFloat( const char *name, float def );
const char *UI_GetCvarString( const char *cvar, char *def );
cvar_t *UI_FindCvar( const char *cvar );
void UI_SetCvarInt( const char *cvar, int value );
void UI_SetCvarFloat( const char *cvar, float value );
void UI_ListFiles( const char *filespec );
const char *UI_ConfigString( int index );
void UI_UpdateCenterPrint(const char* s, float alpha);
void UI_UpdateLocationPrint(int x, int y, const char* s, float alpha);

#endif
