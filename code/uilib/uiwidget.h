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

#ifndef __UI_WIDGET_H__
#define __UI_WIDGET_H__

class UIReggedMaterial {
	uihandle_t hMat;
	str name;
	bool isSet;
	bool isGot;

public:
	UIReggedMaterial();

	uihandle_t	GetMaterial( void );
	void		ReregisterMaterial( void );
	void		CleanupMaterial( void );
	void		SetMaterial( const str& name );
	void		RefreshMaterial( void );
	str			GetName( void );
};

// widget flags
#define WF_STRETCH_VERTICAL		(1<<0)
#define WF_STRETCH_HORIZONTAL	(1<<1)
#define WF_DIRECTED				(1<<2)
#define WF_TILESHADER			(1<<3)
#define WF_ALWAYS_BOTTOM		(1<<4)
#define WF_ALWAYS_TOP			(1<<5)
#define WF_NOPARENTADJUST		(1<<6)

// widget alignment flags
#define WA_LEFT					(1<<0)
#define WA_RIGHT				(1<<1)
#define WA_TOP					(1<<3)
#define WA_BOTTOM				(1<<4)
#define WA_CENTERX				(1<<5)
#define WA_CENTERY				(1<<6)
#define WA_FULL					( WA_LEFT | WA_RIGHT | WA_TOP | WA_BOTTOM | WA_CENTERX | WA_CENTERY )

typedef class UIReggedMaterial UIReggedMaterial;
typedef enum { border_none, border_3D, border_indent, border_outline } borderstyle_t;
typedef enum { D_NONE, D_FROM_LEFT, D_FROM_RIGHT, D_FROM_BOTTOM, D_FROM_TOP } direction_t;
typedef enum { MOTION_IN, MOTION_OUT } motion_t;

class UIWidget : public USignal {
	friend class Menu;
	friend class UILayout;
	friend class UIWindowManager;

protected:
	UIRect2D m_frame;
	vec2_t m_vVirtualScale;
	UIPoint2D m_startingpos;
	UIPoint2D m_origin;
	UIRect2D m_screenframe;
	UIPoint2D m_screenorigin;
	UIRect2D m_clippedframe;
	UIPoint2D m_clippedorigin;
	UColor m_background_color;
	UColor m_foreground_color;
	UBorderColor m_border_color;
	borderstyle_t m_borderStyle;
	bool m_visible;
	bool m_enabled;
	UIFont *m_font;
	class UIWidget *m_parent;
	Container<UIWidget *> m_children;
	str m_name;
	bool m_canactivate;
	str m_cvarname;
	str m_cvarvalue;
	str m_command;
	Container<str *> m_showcommands;
	Container<str *> m_hidecommands;
	str m_title;
	float m_indent;
	float m_bottomindent;
	UIReggedMaterial *m_material;
	UIReggedMaterial *m_hovermaterial;
	UIReggedMaterial *m_pressedmaterial;
	bool m_hovermaterial_active;
	bool m_pressedmaterial_active;
	direction_t m_direction;
	direction_t m_direction_orig;
	motion_t m_motiontype;
	float m_starttime;
	float m_fadetime;
	float m_alpha;
	float m_local_alpha;
	float m_motiontime;
	str m_stopsound;
	str m_clicksound;
	int m_align;
	int m_flags;
	bool m_dying;
	int m_ordernum;
	int m_configstring_index;
	fonthorzjustify_t m_iFontAlignmentHorizontal;
	fontvertjustify_t m_iFontAlignmentVertical;
	qboolean m_bVirtual;
	str m_enabledCvar;

public:
	CLASS_PROTOTYPE( UIWidget );

	void( *m_commandhandler ) ( const char *, void * );

protected:
	bool			addChild( UIWidget *widget );
	bool			removeChild( UIWidget *widget );
	void			PropogateCoordinateSystem( void );
	void			set2D( void );
	virtual void	Draw( void );
	virtual void	FrameInitialized( void );
	void			DrawTitle( float x, float y );
	void			Motion( void );
	virtual void	AlignPosition( void );
	void			Hide( Event *ev );
	void			Activate( Event *ev );
	void			BringToFront( void );
	static void		SplitWidgets( Container<UIWidget*>& src, Container<UIWidget*>& bottom, Container<UIWidget*>& normal, Container<UIWidget*>& top );
	static void		CombineWidgets( Container<UIWidget*>& dst, Container<UIWidget*>& bottom, Container<UIWidget*>& normal, Container<UIWidget*>& top );
	static void		ArrangeWidgetList( Container<UIWidget*>& list );

public:

	UIWidget();
	virtual ~UIWidget();

	virtual void		Shutdown( void );
	virtual void		InitFrame( UIWidget *parentview, float x, float y, float width, float height, int border = -1, const char *fontname = "verdana-12" );
	void				InitFrame( UIWidget *parentview, const UIRect2D& r, int border, const char *fontname );
	void				LayoutSize( Event *ev );
	void				LayoutRect( Event *ev );
	void				LayoutName( Event *ev );
	void				LayoutTitle( Event *ev );
	void				LayoutFont( Event *ev );
	void				LayoutBgColor( Event *ev );
	void				LayoutFgColor( Event *ev );
	void				LayoutBorderStyle( Event *ev );
	void				LayoutBorderColor( Event *ev );
	void				LayoutShader( Event *ev );
	void				LayoutTileShader( Event *ev );
	void				LayoutHoverShader( Event *ev );
	void				LayoutPressedShader( Event *ev );
	void				LayoutFadeIn( Event *ev );
	void				LayoutStopSound( Event *ev );
	void				LayoutClickSound( Event *ev );
	void				LayoutStretch( Event *ev );
	void				LayoutVirtualRes( Event *ev );
	void				LayoutInitData( Event *ev );
	void				LayoutDirection( Event *ev );
	void				LayoutAlign( Event *ev );
	void				LayoutStuffCommand( Event *ev );
	void				LayoutLinkCvar( Event *ev );
	void				LayoutNoParentClipping( Event *ev );
	void				LayoutNoParentAdjustment( Event *ev );
	void				LayoutOrderNumber( Event *ev );
	void				TextAlignment( Event *ev );
	void				LayoutAliasCache( Event *ev );
	void				SetEnabledCvar( Event *ev );
	void				setParent( UIWidget *parent );
	class UIWidget		*getParent( void );
	class UIWidget		*getFirstChild( void );
	class UIWidget		*getNextSibling( void );
	class UIWidget		*getNextChild( UIWidget *child );
	class UIWidget		*getPrevChild( UIWidget *child );
	class UIWidget		*getPrevSibling( void );
	class UIWidget		*getPrevSibling( UIWidget *curr );
	class UIWidget		*getLastSibling( void );
	class UIWidget		*findSibling( str name );
	void				Enable( void );
	void				Disable( void );
	bool				isEnabled( void );
	bool				IsDying( void );
	class UIWidget		*FindResponder( const UIPoint2D& pos );
	void				setFont( const char *name );
	void				setFontHorizontalAlignment( fonthorzjustify_t alignment );
	void				setFontVerticalAlignment( fontvertjustify_t alignment );
	void				setShow( bool visible );
	bool				getShow( void );
	class UColor		getBackgroundColor( void );
	virtual void		setBackgroundColor( const UColor& color, bool setbordercolor );
	class UBorderColor	getBorderColor( void );
	void				setBorderColor( const UBorderColor& color );
	class UColor		getForegroundColor( void );
	virtual void		setForegroundColor( const UColor& color );
	borderstyle_t		getBorderStyle( void );
	void				setBorderStyle( borderstyle_t style );
	class UISize2D		getSize( void );
	void				setSize( const UISize2D& newsize );
	class UIRect2D		getFrame( void );
	void				setFrame( const UIRect2D& newframe );
	class UIPoint2D		getOrigin( void );
	void				setOrigin( const UIPoint2D& neworigin );
	void				setName( str name );
	const char			*getName( void );
	void				setTitle( str title );
	const char			*getTitle( void );
	void				setDirection( direction_t dir );
	direction_t			getDirection( void );
	void				setMotionType( motion_t type );
	motion_t			getMotionType( void );
	void				setMotionTime( float time );
	float				getMotionTime( void );
	void				setAlign( int align );
	int					getAlign( void );
	void				setMaterial( UIReggedMaterial *mat );
	void				setHoverMaterial( UIReggedMaterial *mat );
	void				setPressedMaterial( UIReggedMaterial *mat );
	class UIPoint2D		getLocalPoint( const UIPoint2D& pos );
	class UIPoint2D		getGlobalPoint( const UIPoint2D& pos );
	virtual void		setBackgroundAlpha( float f );
	float				getBackgroundAlpha( void );
	void				Display( const UIRect2D& drawframe, float parent_alpha );
	virtual qboolean	KeyEvent( int key, unsigned int time );
	virtual void		CharEvent( int ch );
	virtual void		UpdateData( void );
	virtual void		UpdateUIElement( void );
	float				getTitleWidth( void );
	float				getTitleHeight( void );
	bool				CanActivate( void );
	void				AllowActivate( bool canactivate );
	bool				IsActive( void );
	bool				IsVisible( void );
	virtual void		LinkCvar( str cvarname );
	void				LinkCommand( str cmd );
	void				ExecuteShowCommands( void );
	void				ExecuteHideCommands( void );
	void				InitializeCommandHandler( void( *fcn) ( const char *, void * ) );
	class UIWidget		*FindWidget( str name );
	void				ResetMotion( motion_t type );
	virtual void		Realign( void );
	void				BringToFrontPropogated( void );
	class UIWidget		*IsThisOrChildActive( void );
	class UIPoint2D		MouseEventToClientPoint( Event *ev );
	class UIRect2D		getClientFrame( void );
	void				setAlwaysOnBottom( bool b );
	void				setAlwaysOnTop( bool b );
	bool				getAlwaysOnBottom( void );
	bool				getAlwaysOnTop( void );
	bool				SendSignal( Event& event );
	void				ShowCommand( Event *ev );
	void				HideCommand( Event *ev );
	void				AddFlag( int flag );
	void				SetHovermaterialActive( bool a );
	void				SetPressedmaterialActive( bool a );
	int					getOrderNum( void );
	class str			getCommand( void );
	void				ActivateOrder( void );
	void				EnableEvent( Event *ev );
	void				DisableEvent( Event *ev );
	void				setConfigstringIndex( int cs );
	int					getConfigstringIndex( void );
	bool				PassEventToWidget( str name, Event *ev );
};

class UIWidgetContainer : public UIWidget {
	UColor m_bgfill;
	qboolean m_fullscreen;
	int m_vidmode;
	int m_currentwidnum;
	int m_maxordernum;

public:
	class UILayout *m_layout;

	CLASS_PROTOTYPE( UIWidgetContainer );

private:
	void		AlignPosition( void ) override;
	void		Draw( void ) override;

public:
	UIWidgetContainer();

	void		Realign( void ) override;
	void		SetBGFill( Event *ev );
	void		SetFullscreen( Event *ev );
	void		SetVidMode( Event *ev );
	int			getVidMode( void );
	qboolean	isFullscreen( void );
	UIWidget	*GetNextWidgetInOrder( void );
	UIWidget	*GetPrevWidgetInOrder( void );
	void		SetActiveWidgetOrderNum( UIWidget *wid );
	void		SetActiveWidgetOrderNum( int num );
	void		SetLastActiveWidgetOrderNum( void );
};

extern Event W_Destroyed;
extern Event W_SizeChanged;
extern Event W_FrameChanged;
extern Event W_OriginChanged;
extern Event W_Activated;
extern Event EV_Widget_Activate;
extern Event W_Deactivated;
extern Event EV_Widget_Hide;
extern Event EV_Widget_Enable;
extern Event EV_Widget_Disable;
extern Event W_RealignWidget;
extern Event W_Draw;
extern Event EV_Layout_Size;
extern Event EV_Layout_Rect;
extern Event EV_Layout_Name;
extern Event EV_Layout_Title;
extern Event EV_Layout_Font;
extern Event EV_Layout_BGColor;
extern Event EV_Layout_FGColor;
extern Event EV_Layout_Borderstyle;
extern Event EV_Layout_BorderColor;
extern Event EV_Layout_Shader;
extern Event EV_Layout_TileShader;
extern Event EV_Layout_HoverShader;
extern Event EV_Layout_PressedShader;
extern Event EV_Layout_FadeIn;
extern Event EV_Layout_StopSound;
extern Event EV_Layout_ClickSound;
extern Event EV_Layout_Stretch;
extern Event EV_Layout_VirtualRes;
extern Event EV_Layout_InitData;
extern Event EV_Layout_Direction;
extern Event EV_Layout_Align;
extern Event EV_Layout_StuffCommand;
extern Event EV_Layout_LinkCvar;
extern Event EV_Layout_NoParentClipping;
extern Event EV_Layout_NoParentAdjustment;
extern Event EV_Widget_HideCommand;
extern Event EV_Widget_ShowCommand;
extern Event EV_Widget_OrderNumber;
extern Event EV_Widget_TextAlignment;
extern Event EV_Widget_EnabledCvar;
extern Event EV_Layout_AliasCache;
extern Event EV_Layout_BGFill;
extern Event EV_Layout_Fullscreen;
extern Event EV_Layout_VidMode;

void SetColor( const UColor& color, float alpha );
void DrawBox( const UIRect2D& rect, const UColor& color, float alpha );
void Draw3DBox( const UIRect2D& rect, bool indent, const UBorderColor& color, float alpha );
void DrawBox( float x, float y, float width, float height, const UColor &color, float alpha );
void Draw3DBox( float x, float y, float width, float height, bool indent, const UBorderColor& color, float alpha );
void DrawMac3DBox( const UIRect2D& rect, bool indent, const UBorderColor& color, int inset, float alpha );
void DrawBoxWithSolidBorder( const UIRect2D& rect, const UColor& inside, const UColor& outside, int size, int flags, float alpha );

#endif /* __UI_WIDGET_H__ */
