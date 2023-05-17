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

#include "cl_ui.h"

Event EV_GMBox_Goin
(
	"_gmbox_goin",
	EV_DEFAULT,
	NULL,
	NULL,
	"Event to make the gmbox disappear"
);

Event EV_GMBox_Decay
(
	"_gmbox_decay",
	EV_DEFAULT,
	NULL,
	NULL,
	"Event to make the gmbox console line decay"
);

static float s_gmboxWidth = 640.0;

CLASS_DECLARATION( UIWidget, UIGMBox, NULL )
{
	{ &W_SizeChanged,		&UIGMBox::OnSizeChanged },
	{ &EV_GMBox_Goin,		&UIGMBox::MoveInEvent },
	{ &EV_GMBox_Decay,		&UIGMBox::DecayEvent },
	{ NULL, NULL }
};

UIGMBox::UIGMBox()
{
	m_numitems = 0;
	m_reallyshown = true;
	m_fontbold = NULL;
	m_boxstate = boxstate_t::box_out;
	m_iBeginDecay = 0;
	m_boxtime = uid.time;
	m_movespeed = 500;
}

void UIGMBox::VerifyBoxOut( void )
{
	PostMoveinEvent();
	if (m_boxstate != boxstate_t::box_moving_out && m_boxstate != boxstate_t::box_out)
	{
		ChangeBoxState(boxstate_t::box_moving_out);
	}
}

void UIGMBox::ChangeBoxState( boxstate_t state )
{
	m_boxstate = state;
	m_boxtime = uid.time;
	setShowState();

	if (state == box_out) {
		PostMoveinEvent();
	}
}

void UIGMBox::HandleBoxMoving( void )
{
	int delta;
	UIRect2D newRect;

	if (m_boxstate != boxstate_t::box_moving_out && m_boxstate != boxstate_t::box_moving_in) {
		return;
	}

	delta = m_movespeed * (uid.time - m_boxtime) / 1000;
	m_boxtime = 1000 * delta / m_movespeed + m_boxtime;
	if (m_boxstate == boxstate_t::box_moving_out)
	{
		newRect.size.width = m_frame.size.width;
		newRect.size.height = m_frame.size.height;
		newRect.pos.x = m_frame.pos.x;
		newRect.pos.y = delta + m_frame.pos.y;

		if (newRect.pos.y <= 0.0)
		{
			newRect.pos.y = 0.0;
			ChangeBoxState(boxstate_t::box_out);
		}
	}
	else if (m_boxstate == boxstate_t::box_moving_in)
	{
		newRect.size.width = m_frame.size.width;
		newRect.size.height = m_frame.size.height;
		newRect.pos.x = m_frame.pos.x;
		newRect.pos.y = delta - m_frame.pos.y;

		if (newRect.pos.y <= -newRect.size.height)
		{
			newRect.pos.y = -newRect.size.height;
			ChangeBoxState(boxstate_t::box_in);
		}
	} else {
		newRect = m_frame;
	}

	setFrame(newRect);
}

void UIGMBox::PostMoveinEvent( void )
{
	if (m_boxstate != boxstate_t::box_out) {
		return;
	}

	if (!EventPending(EV_GMBox_Goin)) {
		PostEvent(EV_GMBox_Goin, 10.0);
	} else {
		PostponeEvent(EV_GMBox_Goin, 10.0);
	}
}

void UIGMBox::PostDecayEvent( void )
{
	if (!EventPending(EV_GMBox_Decay))
	{
		float fDelayTime;
		int iNumLines;
		int i;
		const char* pszString = m_items[0].string.c_str();

		//
		// Calculate the number of lines
		//
		iNumLines = 1;
		for (i = 0; pszString[i]; i++)
		{
			if (pszString[i] == '\n') {
				iNumLines++;
			}
		}

		//
		// Bold as twice more decay
		//
		if (m_items[0].flags & GMBOX_ITEM_FLAG_BOLD) {
			fDelayTime = iNumLines * 10.0;
		} else {
			fDelayTime = iNumLines * 5.0;
		}

		m_iBeginDecay = cls.realtime;
		m_iEndDecay = (int)(fDelayTime * 1000.0);

		PostEvent(EV_GMBox_Decay, fDelayTime);
	}
}

void UIGMBox::setShowState( void )
{
	if (m_reallyshown) {
		setShow(m_boxstate != box_in);
	} else {
		setShow(false);
	}
}

void UIGMBox::RemoveTopItem( void )
{
	int i;

	for (i = 0; i < m_numitems - 1; i++) {
		m_items[i] = m_items[i + 1];
	}

	m_numitems--;
}

str UIGMBox::CalculateBreaks( UIFont *font, str text, float max_width )
{
	str newText, sTmp;
	int i;
	float fX;
	float fwX, fsX;

	Cmd_TokenizeString(text.c_str());
	if (Cmd_Argc())
	{
		fX = 0.0;
		fsX = font->getCharWidth(' ');

		for (i = 0; i < Cmd_Argc(); i++)
		{
			sTmp = Cmd_Argv(i);

			fwX = font->getWidth(sTmp.c_str(), -1);
			if (fwX + i <= max_width)
			{
				if (fwX + i + fX <= max_width)
				{
					newText += sTmp + " ";
				} else {
					newText += "\n" + sTmp + " ";
				}
			}
			else
			{
				sTmp += "\n";
				fX = 0.0;
			}
		}
	} else {
		newText = "";
	}

	return newText;
}

float UIGMBox::PrintWrap( UIFont *font, float x, float y, str text )
{
	const char* p1, * p2;
	size_t n, l;
	float fY;

	fY = y;
	p1 = text.c_str();
	l = text.length();

	for (;;)
	{
		p2 = strchr(p1, '\n');
		if (!p2) {
			break;
		}

		n = p2 - p1;
		if (n >= l) {
			break;
		}

		font->Print(x, fY, p1, p2 - p1, qfalse);
		p1 = p2 + 1;
		l -= n;
		fY += font->getHeight(qfalse);
	}

	font->Print(x, fY, p1, l, qfalse);

	return font->getHeight(qfalse) + (fY - y);
}

float UIGMBox::DrawItem( item_t *in, float x, float y, float alpha )
{
	in->font->setColor(in->color);
	in->font->setAlpha(alpha);

	return PrintWrap(in->font, x, y, in->string);
}

void UIGMBox::Print( const char *text )
{
	const char* text1 = text;

	if (m_numitems > 4)
	{
		//
		// Overwrite an item
		//
		RemoveTopItem();
	}

	m_items[m_numitems].flags = 0;

	if (*text == 3)
	{
		m_items[m_numitems].color = UWhite;
		m_items[m_numitems].font = m_fontbold;
		m_items[m_numitems].flags |= GMBOX_ITEM_FLAG_BOLD;

		text1 = text + 1;
	}
	else
	{
		m_items[m_numitems].color = m_foreground_color;
		m_items[m_numitems].font = m_font;
	}

	m_items[m_numitems].string = CalculateBreaks(m_items[m_numitems].font, text1, s_gmboxWidth);

	m_numitems++;
	VerifyBoxOut();
	PostDecayEvent();
}

void UIGMBox::OnSizeChanged( Event *ev )
{
	s_gmboxWidth = m_frame.size.width;
}

void UIGMBox::Create( const UIRect2D& rect, const UColor& fore, const UColor& back, float alpha )
{
	InitFrame(NULL, rect, 0, "facfont-20");

	if (!m_fontbold) {
		m_fontbold = new UIFont("facfont-20");
	}

	m_fontbold->setColor(URed);
	setBackgroundColor(back, true);
	setForegroundColor(fore);
	setBackgroundAlpha(alpha);

	Connect(this, W_SizeChanged, W_SizeChanged);
	OnSizeChanged(NULL);

	m_movespeed = rect.size.height * 3.0;

	setShowState();
}

void UIGMBox::MoveInEvent( Event *ev )
{
}

void UIGMBox::DecayEvent( Event *ev )
{
	RemoveTopItem();
	if (m_numitems) {
		PostDecayEvent();
	}
}

void UIGMBox::Draw( void )
{
	float fsY;
	int i;
	float alpha;
	float alphaScale;

	alphaScale = 1.0;
	HandleBoxMoving();

	if (!m_numitems) {
		//
		// Nothing to show
		//
		return;
	}

	m_font->setColor(m_foreground_color);
	alpha = (float)(cls.realtime - m_iBeginDecay) / (float)m_iEndDecay;
	if (alpha > 1.0) alpha = 1.0;

	alpha = (1.0 - alpha) * 4.0;
	if (alpha > 1.0) alpha = 1.0;

	if (cge) {
		alphaScale = 1.0 - cge->CG_GetObjectiveAlpha();
	}

	fsY = DrawItem(m_items, 0.0, 120.0, alpha * alphaScale);
	fsY = alpha <= 0.2 ? 120.0 : fsY + 120.0;

	for (i = 1; i < m_numitems; i++)
	{
		fsY += DrawItem(&m_items[i], 0.0, fsY, alphaScale);
		if (fsY > m_frame.size.height)
		{
			if (EventPending(EV_GMBox_Decay)) {
				CancelEventsOfType(EV_GMBox_Decay);
			}

			PostEvent(EV_GMBox_Decay, 0.0);
			break;
		}
	}
}

void UIGMBox::setRealShow( bool b )
{
	this->m_reallyshown = b;
	setShowState();
}

void UIGMBox::Clear( void )
{
	m_numitems = 0;
}

