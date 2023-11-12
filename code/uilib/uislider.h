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

#ifndef __UISLIDER_H__
#define __UISLIDER_H__

typedef enum { SLIDER_FLOAT, SLIDER_INTEGER } slidertype_t;

class UISlider : public UIWidget {
	bool m_initialized;
	float m_minvalue;
	float m_maxvalue;
	int m_minx;
	int m_maxx;
	int m_arrow_width;
	float m_value;
	float m_oldvalue;
	float m_sliderwidth; // Note: this is actually the width of only the thumb, not the whole slider
	UIRect2D m_sliderregion;
	UIRect2D m_prev_arrow_region;
	UIRect2D m_next_arrow_region;
	slidertype_t m_slidertype;
	float m_stepsize;
	bool m_prev_arrow_depressed;
	bool m_next_arrow_depressed;
	bool m_thumb_depressed;
	UIReggedMaterial *m_bar_material;
	UIReggedMaterial *m_prev_arrow_material;
	UIReggedMaterial *m_next_arrow_material;
	UIReggedMaterial *m_thumbmaterial;
	UIReggedMaterial *m_prev_arrow_material_pressed;
	UIReggedMaterial *m_next_arrow_material_pressed;
	UIReggedMaterial *m_thumbmaterial_pressed;

public:
	CLASS_PROTOTYPE( UISlider );

private:
	void	setThumb( void );
	void	Pressed( Event *ev );
	void	Released( Event *ev );
	void	Increment( void );
	void	Decrement( void );
	void	AutoIncrement( Event *ev );
	void	AutoDecrement( Event *ev );
	void	MouseExited( Event *ev );
	void	MouseEntered( Event *ev );
	void	MouseDragged( Event *ev );
	void	LayoutSetRange( Event *ev );
	void	LayoutSetType( Event *ev );
	void	LayoutSetStep( Event *ev );
	void	LayoutSliderBGShader( Event *ev );
	void	LayoutSliderThumbShader( Event *ev );
	void	LayoutSliderLeftShader( Event *ev );
	void	LayoutSliderRightShader( Event *ev );

public:
	UISlider();

	void		Draw( void ) override;
	void		FrameInitialized( void ) override;
	qboolean	KeyEvent( int key, unsigned int time ) override;
	void		UpdateData( void ) override;
	void		UpdateUIElement( void ) override;
	void		setType( slidertype_t type );
	void		setRange( float min, float max );
	void		setStep( float value );
};

#endif

