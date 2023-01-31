#include "ida.h"

typedef int uihandle_t;

#include "ui/usignal.h"
#include "ui/uisize2d.h"
#include "ui/uipoint2d.h"
#include "ui/uirect2d.h"
#include "ui/ucolor.h"
#include "ui/uifont.h"
#include "ui/uiwidget.h"
#include "ui/uimenu.h"
#include "ui/ulist.h"
#include "ui/uistatus.h"
#include "ui/uilabel.h"
#include "ui/uislider.h"
#include "ui/uihorizscroll.h"
#include "ui/uivertscroll.h"
#include "ui/uibutton.h"
#include "ui/cl_uibind.h"
#include "ui/uibind.h"
#include "ui/uibindlist.h"
#include "ui/uicheckbox.h"
#include "ui/uifloatwnd.h"
#include "ui/uipopupmenu.h"
#include "ui/uiconsole.h"
#include "ui/uidialog.h"
#include "ui/editfield.h"
#include "ui/uifield.h"
#include "ui/uilangamelist.h"
#include "ui/uiglobalgamelist.h"
#include "ui/uilayout.h"
#include "ui/uilist.h"
#include "ui/uilistbox.h"
#include "ui/uilistctrl.h"
#include "ui/uimenu.h"
#include "ui/uimledit.h"
#include "ui/uinotepad.h"
#include "ui/uipulldownmenu.h"
#include "ui/uipulldownmenucontainer.h"
#include "ui/uiwinman.h"
#include "ui/cl_uidmbox.h"
#include "ui/cl_uigmbox.h"
#include "ui/cl_uiminicon.h"
#include "ui/cl_uifilepicker.h"
#include "ui/cl_uimaprunner.h"
#include "ui/cl_uimpmappicker.h"
#include "ui/cl_uiplayermodelpicker.h"
#include "ui/cl_uisoundpicker.h"
#include "ui/cl_uiview3d.h"
#include "ui/cl_inv.h"
#include "ui/cl_invrender.h"
#include "ui/cl_uistd.h"
#include "ui/cl_uiloadsave.h"

typedef struct ScoreboardListItem_s {
	void *_vptr;
	str strings[ 8 ];
	qboolean bColorSet;
	UColor backColor;
	UColor textColor;
	qboolean bTitleItem;
} ScoreboardListItem;

typedef struct uiGlobals_s {
	UList_str_ fileList;
	void *globalFont;
	str clientData;
} uiGlobals_t;

typedef struct uiimport_s {
	uihandle_t( *Rend_RegisterMaterial )( const char *name );
	uihandle_t( *Rend_RefreshMaterial )( const char *name );
	void( *Rend_Set2D )( int x, int y, int w, int h, float left, float right, float bottom, float top, float n, float f );
	void( *Rend_SetColor )( const float *rgba );
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
	int ( *File_OpenFile )( const char *qpath, void **buffer );
	void ( *File_FreeFile )( void *buffer );
	void ( *File_WriteFile )( const char *qpath, const void *buffer, int size );
	uihandle_t ( *Snd_RegisterSound )( const char *sample, qboolean streamed );
	void ( *Snd_PlaySound )( const char *sound_name );
	qboolean ( *Alias_Add )( const char *alias, const char *name, const char *parameters );
	const char *( *Alias_FindRandom )( const char *alias, aliasListNode_t **ret );
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
	char *( *Key_GetCommandForKey )( int keynum );
	void ( *Key_SetBinding )( int keynum, const char *binding );
	void ( *Key_GetKeysForCommand )( const char *command, int *key1, int *key2 );
	const char *( *Key_KeynumToString )( int keynum );
	const char *( *GetConfigstring )( int index );
	void ( *UI_CloseDMConsole )( void );
} uiimport_t;

typedef struct uiexport_s {
	void ( *AddFileToList ) ( );
	void ( *ResolutionChange ) ( );
	void ( *Init ) ( );
	void ( *Shutdown ) ( );
	int ( *FontStringWidth ) ( );
} uiexport_t;

typedef struct uidef_s {
	int time;
	int vidWidth;
	int vidHeight;
	int mouseX;
	int mouseY;
	unsigned int mouseFlags;
	int uiHasMouse;
} uidef_t;