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

Event EV_DMBox_Goin
(
	"_dmbox_goin",
	EV_DEFAULT,
	NULL,
	NULL,
	"Event to make the dmbox disappear"
);

Event EV_DMBox_Decay
(
	"_dmbox_decay",
	EV_DEFAULT,
	NULL,
	NULL,
	"Event to make the dmbox console line decay"
);

static float s_dmboxWidth = 384.0;
static float s_dmboxOffsetX = 3.0f;
static float s_dmboxOffsetY = 8.0f;

CLASS_DECLARATION( UIWidget, UIDMBox, NULL )
{
	{ &W_SizeChanged,		&UIDMBox::OnSizeChanged },
	{ &EV_DMBox_Goin,		&UIDMBox::MoveInEvent },
	{ &EV_DMBox_Decay,		&UIDMBox::DecayEvent },
	{ NULL, NULL }
};

UIDMBox::UIDMBox()
{
	m_numitems = 0;
	m_reallyshown = true;
	m_fontbold = NULL;
	m_boxstate = boxstate_t::box_out;
	m_iBeginDecay = 0;
	m_boxtime = uid.time;
	m_movespeed = 500;
}

void UIDMBox::VerifyBoxOut( void )
{
	PostMoveinEvent();
	if (m_boxstate != boxstate_t::box_moving_out && m_boxstate != boxstate_t::box_out)
	{
		ChangeBoxState(boxstate_t::box_moving_out);
	}
}

void UIDMBox::ChangeBoxState( boxstate_t state )
{
	m_boxstate = state;
	m_boxtime = uid.time;
	setShowState();

	if (state == box_out) {
		PostMoveinEvent();
	}
}

void UIDMBox::HandleBoxMoving( void )
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
	}
	else {
		newRect = m_frame;
	}

	setFrame(newRect);
}

void UIDMBox::PostMoveinEvent( void )
{
	if (m_boxstate != boxstate_t::box_out) {
		return;
	}

	if (!EventPending(EV_DMBox_Goin)) {
		PostEvent(EV_DMBox_Goin, 10.0);
	}
	else {
		PostponeEvent(EV_DMBox_Goin, 10.0);
	}
}

void UIDMBox::PostDecayEvent( void )
{
	if (!EventPending(EV_DMBox_Decay))
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

        if (m_items[0].flags & DMBOX_ITEM_FLAG_BOLD) {
            fDelayTime = iNumLines * 8.0;
        }
        //
        // Bold as twice more decay
        //
		else if (m_items[0].flags & DMBOX_ITEM_FLAG_DEATH) {
			fDelayTime = iNumLines * 6.0;
		}
		else {
			fDelayTime = iNumLines * 5.0;
		}

		m_iBeginDecay = cls.realtime;
		m_iEndDecay = (int)(fDelayTime * 1000.0);

		PostEvent(EV_DMBox_Decay, fDelayTime);
	}
}

void UIDMBox::setShowState( void )
{
	if (m_reallyshown) {
		setShow(m_boxstate != box_in);
	} else {
		setShow(false);
	}
}

void UIDMBox::RemoveTopItem( void )
{
	int i;

	if (m_numitems > 0) {
		for (i = 0; i < m_numitems - 1; i++) {
			m_items[i] = m_items[i + 1];
		}

		m_numitems--;
	}
}

str UIDMBox::CalculateBreaks( UIFont *font, str text, float max_width )
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

float UIDMBox::PrintWrap( UIFont *font, float x, float y, str text )
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

float UIDMBox::DrawItem( item_t *in, float x, float y, float alpha )
{
	in->font->setColor(in->color);
	in->font->setAlpha(alpha);

	return PrintWrap(in->font, x, y, in->string);
}

void UIDMBox::Print( const char *text )
{
	const char* text1 = text;

	if (m_numitems > 5)
	{
		//
		// Overwrite an item
		//
		RemoveTopItem();
	}

	m_items[m_numitems].flags = 0;

	if (*text == MESSAGE_CHAT_WHITE)
	{
		m_items[m_numitems].color = UGrey;
		m_items[m_numitems].font = m_fontbold;
		m_items[m_numitems].flags |= DMBOX_ITEM_FLAG_BOLD;

		text1 = text + 1;
	}
	else if (*text == MESSAGE_CHAT_RED)
	{
		m_items[m_numitems].color = ULightRed;
		m_items[m_numitems].font = m_fontbold;
		m_items[m_numitems].flags |= DMBOX_ITEM_FLAG_DEATH;

		text1 = text + 1;
	}
	else if (*text == MESSAGE_CHAT_GREEN)
	{
		m_items[m_numitems].color = UGreen;
        m_items[m_numitems].font = m_fontbold;
        m_items[m_numitems].flags |= DMBOX_ITEM_FLAG_DEATH;

		text1 = text + 1;
	}
	else
	{
		m_items[m_numitems].color = m_foreground_color;
		m_items[m_numitems].font = m_font;
	}

	m_items[m_numitems].string = CalculateBreaks(m_items[m_numitems].font, text1, s_dmboxWidth);

	m_numitems++;
	VerifyBoxOut();
	PostDecayEvent();
}

void UIDMBox::OnSizeChanged( Event *ev )
{
	s_dmboxWidth = m_frame.size.width;
}

void UIDMBox::Create( const UIRect2D& rect, const UColor& fore, const UColor& back, float alpha )
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

void UIDMBox::MoveInEvent( Event *ev )
{
}

void UIDMBox::DecayEvent( Event *ev )
{
	RemoveTopItem();
	if (m_numitems) {
		PostDecayEvent();
	}
}

void UIDMBox::Draw( void )
{
	float fsY;
	int i;
	float alpha;
	float alphaScale;

	alphaScale = 0.8;
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

	fsY = DrawItem(m_items, s_dmboxOffsetX, s_dmboxOffsetY, alpha * alphaScale);
	fsY = alpha <= 0.2 ? s_dmboxOffsetY : fsY + s_dmboxOffsetY;

	for (i = 1; i < m_numitems; i++)
	{
		fsY += DrawItem(&m_items[i], s_dmboxOffsetX, fsY, alphaScale);
		if (fsY > m_frame.size.height)
		{
			if (EventPending(EV_DMBox_Decay)) {
				CancelEventsOfType(EV_DMBox_Decay);
			}

			PostEvent(EV_DMBox_Decay, 0.0);
			break;
		}
	}
}

void UIDMBox::setRealShow( bool b )
{
	this->m_reallyshown = b;
	setShowState();
}

void UIDMBox::Clear( void )
{
	m_numitems = 0;
}
