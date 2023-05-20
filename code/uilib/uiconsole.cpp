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

#include "ui_local.h"

Event W_Console_ChildSizeChanged
(
	"_console_childsizechanged",
	EV_DEFAULT,
	NULL,
	NULL,
	"Signal that the child area of the floating window has changed"
);

Event EV_Console_Print
(
	"print",
	EV_DEFAULT,
	"s",
	"string",
	"Print the string to the console"
);

CLASS_DECLARATION( UIWidget, UIConsole, NULL )
{
	{ &W_SizeChanged,		&UIConsole::OnSizeChanged },
	{ &EV_Console_Print,	&UIConsole::Print },
	{ NULL, NULL }
};

UIConsole::UIConsole()
{
	m_scroll = NULL;
	m_firstitem = 0;
	m_numitems = 0;
	m_caret = 0;

	m_scroll = new UIVertScroll();
	AllowActivate(true);
}

int UIConsole::getFirstItem
	(
	void
	)

{
	return m_firstitem;
}

int UIConsole::getNextItem
	(
	int prev
	)

{
	if (prev == -1) {
		return -1;
	}

	prev = prev + 1;

	if (m_numitems < MAX_CONSOLE_ITEMS)
	{
		if (prev > m_firstitem)
		{
			if (prev - m_firstitem >= m_numitems)
				return -1;
		}
		else if (m_firstitem + m_numitems >= prev + MAX_CONSOLE_ITEMS)
		{
			return -1;
		}
	}

	if (prev >= MAX_CONSOLE_ITEMS) {
		prev = 0;
	}

	if (prev != m_firstitem) {
		return prev;
	}

	return -1;
}

int UIConsole::getLastItem
	(
	void
	)

{
	int item = m_firstitem + m_numitems;

	if (item - 1 >= MAX_CONSOLE_ITEMS) {
		return item - (MAX_CONSOLE_ITEMS + 1);
	}

	return item - 1;
}

int UIConsole::AddLine
	(
	void
	)

{
	int line = getLastItem() + 1;

	if (line >= MAX_CONSOLE_ITEMS) {
		line = 0;
	}

	if (m_numitems < MAX_CONSOLE_ITEMS)
	{
		m_numitems++;
	}
	else
	{
		m_firstitem++;
		if (m_firstitem >= MAX_CONSOLE_ITEMS) {
			m_firstitem = 0;
		}
	}

	assert(line < MAX_CONSOLE_ITEMS);
	return line;
}

void UIConsole::DrawBottomLine
	(
	void
	)

{
	int promptwidth;
	static str prompt = ">";
	int top;
	int iXPos;
	int iMaxStringWidth;
	int widthbeforecaret;

	promptwidth = m_font->getWidth(prompt.c_str(), -1);
	top = m_frame.size.height - m_font->getHeight(qfalse);
	m_font->Print(0.0, top, prompt.c_str(), -1, qfalse);

	iXPos = promptwidth + 1;
	iMaxStringWidth = m_frame.size.width - iXPos - m_scroll->getSize().width;

	if (m_caret > m_currentline.length())
	{
		// Make sure to not overflow
		m_caret = m_currentline.length();
	}

	if (m_caret < 0) m_caret = 0;

	widthbeforecaret = promptwidth + m_font->getWidth(m_currentline.c_str(), m_caret);

	if (m_refreshcompletionbuffer || m_completionbuffer.length() >= m_currentline.length())
	{
		static str indicator;
		const char* pString;
		int indicatorwidth;
		int iChar;
		int iCharLength;
		int iStringLength;

		if (widthbeforecaret < iMaxStringWidth)
		{
			m_font->Print(iXPos, top, m_currentline.c_str(), -1, qfalse);
		}
		else
		{
			indicatorwidth = m_font->getWidth(indicator.c_str(), -1);
			m_font->Print(iXPos, top, indicator.c_str(), -1, 0);

			iXPos += indicatorwidth;
			iMaxStringWidth -= indicatorwidth;

			pString = &m_currentline[m_caret - 1];
			iStringLength = 0;
			iCharLength = m_font->getCharWidth(pString[0]);

			for (iChar = m_caret; iChar > 1; --iChar)
			{
				iStringLength += iCharLength;
				iCharLength = m_font->getCharWidth(pString[iChar - 1]);
			}

			m_font->Print(iXPos, top, pString + 1, -1, qfalse);
			widthbeforecaret = iXPos + m_font->getWidth(pString + 1, m_caret - iChar);
		}
	}
	else
	{
		int completewidth;
		int linewidth;

		completewidth = m_font->getWidth(m_completionbuffer.c_str(), -1);
		linewidth = m_font->getWidth(&m_currentline[m_completionbuffer.length()], -1);
		m_font->Print(iXPos, top, m_completionbuffer.c_str(), -1, qfalse);

		iXPos += completewidth;
		m_font->Print(iXPos, top, &m_currentline[m_completionbuffer.length()], -1, 0);

		DrawBoxWithSolidBorder(UIRect2D(iXPos, top, linewidth, m_font->getHeight(qfalse)), UBlack, URed, 1, 2, 1.0);
	}

	//
	// Make the caret blink
	//
	if ((uid.time % 750) > 375 && IsActive()) {
		DrawBox(UIRect2D(widthbeforecaret, top, 1.0, m_font->getHeight(qfalse)), m_foreground_color, 1.0);
	}
}

void UIConsole::AddHistory
	(
	void
	)

{
	if (m_history.IterateFromTail())
	{
		if (m_history.getCurrent() != m_currentline) {
			m_history.AddTail(m_currentline);
		}
	}
	else
	{
		m_history.AddTail(m_currentline);
	}

	while (m_history.getCount() > 20)
	{
		m_history.IterateFromHead();
		m_history.RemoveCurrentSetNext();
	}

	m_historyposition = 0;
}

void UIConsole::Print
	(
	Event *ev
	)

{
	AddText(ev->GetString(1), NULL);
}

void UIConsole::KeyEnter
	(
	void
	)

{
	AddText(str(">") + m_currentline + "\n", NULL);

	if (m_consolehandler)
	{
		//
		// call the console handler
		//
		m_consolehandler((m_currentline + "\n").c_str());
	}

	// clear the console input
	m_currentline = "";
	m_caret = 0;
}

void UIConsole::setConsoleHandler
	(
	consoleHandler_t handler
	)

{
	m_consolehandler = handler;
}

void UIConsole::AddText
	(
	const char *text,
	UColor *pColor
	)

{
	bool atbottom;
	int newtop;
	int i, size;
	item* curitem;

	atbottom = m_scroll->getPageHeight() + m_scroll->getTopItem() >= m_scroll->getNumItems();
	newtop = m_scroll->getTopItem();

	if (!m_numitems)
	{
		m_numitems = 1;
		m_items[m_firstitem].string = "";
		m_items[m_firstitem].lines = 0;
		m_items[m_firstitem].pColor = &m_foreground_color;
	}

	curitem = &m_items[getLastItem()];
	if (!pColor) {
		pColor = &m_foreground_color;
	}

	for (i = 0; text[i]; i++)
	{
		if (text[i] == '\n')
		{
			curitem->pColor = pColor;
			CalcLineBreaks(*curitem);

			int line = AddLine();
			curitem = &m_items[line];
			newtop -= curitem->lines;
			curitem->lines = 0;
			curitem->string = "";
		}
		else
		{
			curitem->string += text[i];
		}
	}

	curitem->pColor = pColor;
	CalcLineBreaks(*curitem);

	size = 0;
	for (i = m_firstitem; i != -1; i = getNextItem(i)) {
		size += m_items[i].lines;
	}

	m_scroll->setNumItems(size);

	if (atbottom || newtop > m_scroll->getNumItems() - m_scroll->getPageHeight()) {
		m_scroll->setTopItem(m_scroll->getNumItems() - m_scroll->getPageHeight());
	}
	else {
		m_scroll->setTopItem(newtop);
	}

	if (m_scroll->getTopItem() < 0) {
		m_scroll->setTopItem(0);
	}

	if (curitem->lines == 9) {
		AddText("\n", NULL);
	}
}

void UIConsole::CalcLineBreaks
	(
	item& theItem
	)

{
	int i;
	str sofar, checking;
	int lensofar, lenchecking, len_of_space;
	const char* remaining;

	lensofar = 0;
	lenchecking = 0;
	len_of_space = m_font->getCharWidth(' ');
	theItem.lines = 0;
	remaining = theItem.string.c_str();

	for (i = 0; remaining[i]; i++)
	{
		if (remaining[i] == ' ')
		{
			sofar += ' ';
			checking = "";
			lensofar += lenchecking + len_of_space;
			lenchecking = 0;
		}
		else
		{
			int charlen = m_font->getCharWidth(remaining[i]);
			if (m_frame.size.width - m_scroll->getSize().width < (charlen + lenchecking + lensofar))
			{
				if (lensofar)
				{
					lensofar = lenchecking;
					theItem.breaks[theItem.lines] = sofar.length();
					sofar = checking;
					lenchecking = charlen;
					checking = remaining[i];
				}
				else
				{
					theItem.breaks[theItem.lines] = checking.length();
					sofar = remaining[i];
					lenchecking = 0;
					checking = "";
				}

				theItem.lines++;

				if (theItem.lines == 8) {
					break;
				}
			}

			checking += remaining[i];
			lenchecking += charlen;
		}
	}

	if (remaining[i] || lenchecking || lensofar) {
		theItem.breaks[theItem.lines++] = -1;
	}

	lensofar = 0;
	for (i = 0; i < theItem.lines; i++)
	{
		theItem.begins[i] = lensofar;
		if (theItem.breaks[i] != -1) {
			lensofar += theItem.breaks[i];
		}
	}
}

void UIConsole::Clear
	(
	void
	)

{
	int i;

	// clear items line
	for (i = 0; i < MAX_CONSOLE_ITEMS; i++)
	{
		m_items[i].lines = 0;
	}

	m_numitems = 0;
	m_firstitem = 0;
	m_caret = 0;
	m_currentline = "";

	if (m_scroll) {
		m_scroll->setNumItems(0);
	}
}

void UIConsole::FrameInitialized
	(
	void
	)

{
	Connect(this, W_SizeChanged, W_SizeChanged);
	OnSizeChanged(NULL);
}

void UIConsole::Draw
	(
	void
	)

{
	UColor* pCurrColor;

	m_font->setColor(m_foreground_color);
	pCurrColor = &m_foreground_color;

	m_scroll->setSolidBorderColor(m_background_color);
	m_scroll->setBackgroundColor(m_background_color, true);
	m_scroll->setBackgroundAlpha(m_alpha);
	m_scroll->setForegroundColor(m_foreground_color);

	if (m_numitems)
	{
		int i, item;
		int top;
		int lines_drawn, at_line;
		int topitem;

		top = (int)(m_frame.size.height - m_font->getHeight(false) * (m_scroll->getPageHeight() + 2));
		item = m_firstitem;
		topitem = m_scroll->getTopItem() - 1;

		if (item == -1) {
			return;
		}

		i = m_items[item].lines;
		while (i < topitem) {
			item = getNextItem(item);
			if (item == -1) {
				break;
			}

			i += m_items[item].lines;
		}

		if (item == -1) {
			// no item available
			return;
		}

		at_line = m_items[item].lines - (i - topitem);
		lines_drawn = 0;
		while (lines_drawn < m_scroll->getPageHeight() + 1)
		{
			if (item != -1 && at_line >= m_items[item].lines)
			{
				at_line = 0;
				do {
					item = getNextItem(item);
				}
				while (item != -1 && m_items[item].lines <= 0);
			}

			if (at_line >= 0)
			{
				if (item == -1) {
					break;
				}

				if (at_line >= m_items[item].lines) {
					break;
				}

				const ::item* theItem = &m_items[item];
				if (theItem->pColor != pCurrColor)
				{
					m_font->setColor(*theItem->pColor);
					pCurrColor = theItem->pColor;
				}

				m_font->setAlpha(m_alpha);
				m_font->Print(0.0, top, &theItem->string.c_str()[theItem->begins[at_line]], theItem->breaks[at_line], 0);
			}

			top += m_font->getHeight(false);
			lines_drawn++;
			at_line++;
		}
	}

	if (pCurrColor != &m_foreground_color) {
		m_font->setColor(m_foreground_color);
	}

	DrawBottomLine();
}

void UIConsole::CharEvent
	(
	int ch
	)

{
	if (ch <= 31) {
		// Not a valid char for the console
		return;
	}

	m_refreshcompletionbuffer = true;

	if (m_caret > m_currentline.length()) {
		m_caret = m_currentline.length();
	}

	if (m_caret < 0) {
		m_caret = 0;
	}

	if (m_currentline == m_caret)
	{
		m_currentline.append(str(char(ch)));
		m_caret++;
	}
	else if (m_caret)
	{
		m_currentline = str(m_currentline, 0, m_caret) + str(char(ch)) + (m_currentline.c_str() + m_caret);
		m_caret++;
	}
	else
	{
		m_currentline = str(char(ch)) + m_currentline;
		m_caret++;
	}
}

qboolean UIConsole::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	// FIXME: Partially implemented

	if (key != K_TAB && key != K_BACKSPACE) {
		m_refreshcompletionbuffer = true;
	}

	switch (key)
	{
	case K_TAB:
		break;

	case K_ENTER:
	case K_KP_ENTER:
		if (m_currentline.length()) {
			KeyEnter();
		}
		break;

	case K_BACKSPACE:
		if (!m_caret) {
			break;
		}

		if (m_refreshcompletionbuffer)
		{
			m_currentline = str(m_currentline, 0, m_caret - 1) + (m_currentline.c_str() + m_caret);
			m_caret--;
		}
		else
		{
			m_currentline = m_completionbuffer;
			m_cntcvarnumber = 0;
			m_cntcmdnumber = 0;
			m_refreshcompletionbuffer = true;

			m_caret = m_currentline.length();
		}
		break;

	case K_UPARROW:
		break;

	case K_DOWNARROW:
		break;

	case K_LEFTARROW:
		if (m_caret > 0) {
			m_caret--;
		}
		break;

	case K_RIGHTARROW:
		if (m_caret < m_currentline.length()) {
			m_caret++;
		}
		break;

	case K_DEL:
		if (m_caret >= m_currentline.length()) {
			break;
		}

		m_currentline = str(m_currentline, 0, m_caret) + (m_currentline.c_str() + m_caret + 1);
		break;

	case K_PGDN:
		break;

	case K_PGUP:
		break;

	case K_HOME:
		break;

	case K_END:
		break;

	case K_MWHEELDOWN:
		break;

	case K_MWHEELUP:
		break;
	}

	return true;
}

void UIConsole::OnSizeChanged
	(
	Event *ev
	)

{
	int linesperpage;
	int numscroll;
	int item;
	int topitem;
	int topline;
	int atscroll;
	bool attop;
	bool atbottom;

	numscroll = 0;
	topitem = -1;
	topline = 0;
	atscroll = 0;
	attop = false;
	atbottom = false;

	linesperpage = (m_frame.size.height / (float)m_font->getHeight(qfalse));
	m_scroll->InitFrameAlignRight(this, 0, 0);

	if (ev)
	{
		attop = m_scroll->getTopItem() == 0;
		atbottom = m_scroll->getPageHeight() + m_scroll->getTopItem() >= m_scroll->getNumItems();
	}

	m_scroll->setPageHeight(linesperpage - 1);
	if (m_scroll->getPageHeight() < 1) {
		m_scroll->setPageHeight(1);
	}

	for (item = m_firstitem; item != -1; item = getNextItem(item))
	{
		atscroll += m_items[item].lines;
		if (atscroll >= m_scroll->getTopItem() && topitem == -1) {
			topitem = item;
		}

		CalcLineBreaks(m_items[item]);

		numscroll += m_items[item].lines;
		if (topitem == -1 || topitem == item) {
			topline += m_items[item].lines;
		}
	}

	m_scroll->setNumItems(numscroll);

	if (topitem != -1)
	{
		if (topline > m_scroll->getNumItems() - m_scroll->getPageHeight()) {
			atbottom = true;
		}

		if (!atbottom)
		{
			if (attop) {
				m_scroll->setTopItem(0);
			}
			else
			{
				if (topline < 0) {
					topline = 0;
				}

				m_scroll->setTopItem(topline);
			}
		}
	}

	m_scroll->setTopItem(m_scroll->getNumItems() - m_scroll->getPageHeight());
	if (m_scroll->getTopItem() < 0) {
		m_scroll->setTopItem(0);
	}
}

CLASS_DECLARATION( UIFloatingWindow, UIFloatingConsole, NULL )
{
	{ &W_Console_ChildSizeChanged,			&UIFloatingConsole::OnChildSizeChanged },
	{ &UIFloatingConsole::W_ClosePressed,	&UIFloatingConsole::OnClosePressed },
	{ NULL, NULL }
};

UIFloatingConsole::UIFloatingConsole()
{
	m_status = NULL;
	m_handler = NULL;
	setConsoleColor(UWhite);
	setConsoleBackground(UBlack, 1.0);
}

UIFloatingConsole::~UIFloatingConsole()
{
	if (m_console) {
		delete m_console;
	}
	if (m_status) {
		delete m_status;
	}
}

void UIFloatingConsole::FrameInitialized
	(
	void
	)

{
	// call the parent initialisation
	UIFloatingWindow::FrameInitialized();

	m_status = new UIStatusBar(alignment_t::WND_ALIGN_BOTTOM, 20.0);
	m_status->InitFrame(getChildSpace(), UIRect2D(0, 0, 20.0, 20.0), 0, "verdana-12");
	m_status->EnableSizeBox(this);
	m_status->setTitle(str());

	m_console = new UIConsole();
	m_console->InitFrame(getChildSpace(), UIRect2D(0, 0, 20.0, 20.0), 0, "verdana-14");
	setConsoleColor(m_consoleColor);
	setConsoleBackground(m_consoleBackground, m_consoleAlpha);
	m_console->setConsoleHandler(m_handler);

	getChildSpace()->Connect(this, W_SizeChanged, W_Console_ChildSizeChanged);
	getChildSpace()->AllowActivate(false);
	OnChildSizeChanged(NULL);

	m_background_color.a = 0.0;
	m_alpha = 1.0;

	getChildSpace()->setBackgroundAlpha(1.0);
}

void UIFloatingConsole::OnChildSizeChanged
	(
	Event *ev
	)

{
	const UISize2D childSpaceSize = getChildSpace()->getSize();
	const UISize2D statusSize = m_status->getSize();
	const UISize2D newSize(childSpaceSize.width, childSpaceSize.height - statusSize.height);

	m_console->setFrame(UIRect2D(UIPoint2D(0, 0), newSize));
}

void UIFloatingConsole::AddText
	(
	const char *text,
	UColor *pColor
	)

{
	if (m_console) {
		m_console->AddText(text, pColor);
	}
}

void UIFloatingConsole::setConsoleHandler
	(
	consoleHandler_t handler
	)

{
	if (m_console) {
		m_console->setConsoleHandler(handler);
	} else {
		m_handler = handler;
	}
}

void UIFloatingConsole::Clear
	(
	void
	)

{
	if (m_console) {
		m_console->Clear();
	}
}

void UIFloatingConsole::OnClosePressed
	(
	Event *ev
	)

{
	SendSignal(UIFloatingWindow::W_ClosePressed);
}

void UIFloatingConsole::setConsoleBackground
	(
	const UColor& color,
	float alpha
	)

{
	m_alpha = alpha;
	m_consoleBackground = color;
	m_consoleAlpha = alpha;

	if (m_console)
	{
		m_console->setBackgroundAlpha(alpha);
		m_console->setBackgroundColor(color, true);
	}

	if (m_status)
	{
		m_status->setBackgroundAlpha(alpha);
		m_status->setBackgroundColor(color, true);
	}
}

void UIFloatingConsole::setConsoleColor
	(
	const UColor& color
	)

{
	m_consoleColor = color;

	if (m_console) {
		m_console->setForegroundColor(color);
	}

	if (m_status) {
		m_status->setForegroundColor(color);
	}
}

CLASS_DECLARATION( UIConsole, UIDMConsole, NULL )
{
	{ NULL, NULL }
};

UIDMConsole::UIDMConsole()
{
	m_bQuickMessageMode = qfalse;
	m_iMessageMode = 100;
}

void UIDMConsole::KeyEnter
	(
	void
	)

{
	if (!str::icmp(m_currentline, "exit") || !str::icmp(m_currentline, "quit") || !str::icmp(m_currentline, "close"))
	{
		m_currentline = "";
		uii.UI_CloseDMConsole();
		return;
	}

	AddHistory();

	if (!GetQuickMessageMode())
	{
		if (!str::icmp(m_currentline, "say") || !str::icmp(m_currentline, "all"))
		{
			if (GetMessageMode() != 100) {
				uii.Cmd_Stuff("messagemode_all;wait 1;messagemode_all\n");
			}

			m_currentline = "";
			m_caret = 0;
			return;
		}
		else if (!str::icmp(m_currentline, "sayteam") || !str::icmp(m_currentline, "team") || !str::icmp(m_currentline, "teamsay"))
		{
			if (GetMessageMode() != 200) {
				uii.Cmd_Stuff("messagemode_team;wait 1;messagemode_team\n");
			}

			m_currentline = "";
			m_caret = 0;
			return;
		}
		else if (strstr(m_currentline, "wisper")
			|| strstr(m_currentline, "private")
			// This one is opm-exclusive
			|| strstr(m_currentline, "whisper"))
		{
			char szString[128];
			const char* pszToken;

			strncpy(szString, m_currentline.c_str(), sizeof(szString) / sizeof(szString[0]));
			szString[127] = 0;

			pszToken = strtok(szString, " ");
			if (pszToken)
			{
				if (!Q_stricmp(pszToken, "wisper") || !Q_stricmp(pszToken, "private"))
				{
					pszToken = strtok(0, " ");
					if (!pszToken)
					{
						AddText("Mode Change Error: You need to specify a client number (private #)\n", NULL);
						m_currentline = "";
						m_caret = 0;
						return;
					}

					int mode = atoi(pszToken);
					if (mode && mode != GetMessageMode())
					{
						uii.Cmd_Stuff(va("messagemode_private %i;wait 1;messagemode_private %i\n", mode, mode));
						m_currentline = "";
						m_caret = 0;
						return;
					}
				}
			}
		}
	}

	if (m_consolehandler) {
		m_consolehandler((m_currentline + "\n").c_str());
	}
	else {
		AddText(">" + m_currentline + "\n", NULL);
	}

	m_currentline = "";
	m_caret = 0;

	if (GetQuickMessageMode()) {
		uii.UI_CloseDMConsole();
	}
}

void UIDMConsole::AddDMMessageText
	(
	const char *text,
	UColor *pColor
	)

{
	AddText(text, NULL);
	uii.Snd_PlaySound("objective_text");
}

void UIDMConsole::Draw
	(
	void
	)

{
	if (!IsThisOrChildActive()) {
		SendSignal(W_Deactivated);
	}

	if (GetQuickMessageMode()) {
		m_scroll->ProcessEvent(EV_Widget_Disable);
	} else {
		m_scroll->ProcessEvent(EV_Widget_Enable);
	}

	UIConsole::Draw();
}

qboolean UIDMConsole::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	// FIXME: Partially implemented

	if (key != K_TAB && key != K_BACKSPACE) {
		m_refreshcompletionbuffer = true;
	}

	switch (key)
	{
	case K_TAB:
		break;

	case K_ENTER:
	case K_KP_ENTER:
		if (m_currentline.length()) {
			KeyEnter();
		}
		break;

	case K_BACKSPACE:
		if (!m_caret) {
			break;
		}

		if (m_refreshcompletionbuffer)
		{
			m_currentline = str(m_currentline, 0, m_caret - 1) + (m_currentline.c_str() + m_caret);
			m_caret--;
		}
		else
		{
			m_currentline = m_completionbuffer;
			m_cntcvarnumber = 0;
			m_cntcmdnumber = 0;
			m_refreshcompletionbuffer = true;

			m_caret = m_currentline.length();
		}
		break;

	case K_UPARROW:
		break;

	case K_DOWNARROW:
		break;

	case K_LEFTARROW:
		if (m_caret > 0) {
			m_caret--;
		}
		break;

	case K_RIGHTARROW:
		if (m_caret < m_currentline.length()) {
			m_caret++;
		}
		break;

	case K_DEL:
		if (m_caret >= m_currentline.length()) {
			break;
		}

		m_currentline = str(m_currentline, 0, m_caret) + (m_currentline.c_str() + m_caret + 1);
		break;

	case K_PGDN:
		break;

	case K_PGUP:
		break;

	case K_HOME:
		break;

	case K_END:
		break;

	case K_MWHEELDOWN:
		break;

	case K_MWHEELUP:
		break;
	}

	return true;
}

qboolean UIDMConsole::GetQuickMessageMode
	(
	void
	)

{
	return m_bQuickMessageMode;
}

void UIDMConsole::SetQuickMessageMode
	(
	qboolean bQuickMessage
	)

{
	m_bQuickMessageMode = bQuickMessage;
}

int UIDMConsole::GetMessageMode
	(
	void
	)

{
	return m_iMessageMode;
}

void UIDMConsole::SetMessageMode
	(
	int iMode
	)

{
	m_iMessageMode = iMode;
}

CLASS_DECLARATION( UIFloatingConsole, UIFloatingDMConsole, NULL )
{
	{ &W_Console_ChildSizeChanged,			&UIFloatingDMConsole::OnChildSizeChanged },
	{ &UIFloatingConsole::W_ClosePressed,	&UIFloatingDMConsole::OnClosePressed },
	{ &W_Deactivated,						&UIFloatingDMConsole::OnClosePressed },
	{ NULL, NULL }
};

UIFloatingDMConsole::UIFloatingDMConsole()
	: m_status(NULL)
	, m_handler(NULL)
	, m_consoleColor(0, 0, 0, 1)
	, m_consoleBackground(0, 0, 0, 1)
{
	setConsoleColor(UWhite);
	setConsoleBackground(UBlack, 1);
}

UIFloatingDMConsole::~UIFloatingDMConsole()
{
	if (m_console) {
		delete m_console;
	}
	if (m_status) {
		delete m_status;
	}
}

void UIFloatingDMConsole::FrameInitialized
	(
	void
	)

{
	// call the parent initialisation
	UIFloatingWindow::FrameInitialized();

	m_console = new UIDMConsole();
	m_console->InitFrame(getChildSpace(), UIRect2D(0, 0, 20.0, 20.0), 0, "verdana-12");

	m_console->Connect(this, W_Deactivated, W_Deactivated);
	setConsoleColor(m_consoleColor);
	setConsoleBackground(m_consoleBackground, m_consoleAlpha);
	getChildSpace()->Connect(this, W_SizeChanged, W_Console_ChildSizeChanged);
	m_console->setConsoleHandler(m_handler);

	getChildSpace()->AllowActivate(false);
	OnChildSizeChanged(NULL);

	m_background_color.a = 1.0;
	m_alpha = 1.0;

	getChildSpace()->setBackgroundAlpha(1.0);
}

void UIFloatingDMConsole::OnChildSizeChanged
	(
	Event *ev
	)

{
	const UISize2D childSpaceSize = getChildSpace()->getSize();
	if (m_status)
	{
		const UISize2D statusSize = m_status->getSize();
		const UISize2D newSize(childSpaceSize.width, childSpaceSize.height - statusSize.height);

		m_console->setFrame(UIRect2D(UIPoint2D(0, 0), newSize));
	}
	else
	{

		m_console->setFrame(UIRect2D(UIPoint2D(0, 0), childSpaceSize));
	}
}

void UIFloatingDMConsole::AddText
	(
	const char *text,
	UColor *pColor
	)

{
	if (m_console) {
		m_console->AddText(text, pColor);
	}
}

void UIFloatingDMConsole::AddDMMessageText
	(
	const char *text,
	UColor *pColor
	)

{
	if (m_console) {
		m_console->AddDMMessageText(text, pColor);
	}
}

void UIFloatingDMConsole::setConsoleHandler
	(
	consoleHandler_t handler
	)

{
	if (m_console) {
		m_console->setConsoleHandler(handler);
	}
}

void UIFloatingDMConsole::Clear
	(
	void
	)

{
	if (m_console) {
		m_console->Clear();
	}
}

void UIFloatingDMConsole::OnClosePressed
	(
	Event *ev
	)

{
	SendSignal(UIFloatingWindow::W_ClosePressed);
}

void UIFloatingDMConsole::setConsoleBackground
	(
	const UColor& color,
	float alpha
	)

{
	m_consoleBackground = color;
	m_consoleAlpha = alpha;

	if (m_console)
	{
		m_console->setBackgroundAlpha(alpha);
		m_console->setBackgroundColor(color, true);
	}

	if (m_status)
	{
		m_status->setBackgroundAlpha(alpha);
		m_status->setBackgroundColor(color, true);
	}
}

void UIFloatingDMConsole::setConsoleColor
	(
	const UColor& color
	)

{
	m_consoleColor = color;

	if (m_console) {
		m_console->setForegroundColor(color);
	}

	if (m_status) {
		m_status->setForegroundColor(color);
	}
}

qboolean UIFloatingDMConsole::GetQuickMessageMode
	(
	void
	)

{
	if (m_console) {
		return m_console->GetQuickMessageMode();
	}

	return qfalse;
}

void UIFloatingDMConsole::SetQuickMessageMode
	(
	qboolean bQuickMessage
	)

{
	if (m_console) {
		return m_console->SetQuickMessageMode(bQuickMessage);
	}
}

int UIFloatingDMConsole::GetMessageMode
	(
	void
	)

{
	if (m_console) {
		return m_console->GetMessageMode();
	}

	return qfalse;
}

void UIFloatingDMConsole::SetMessageMode
	(
	int iMode
	)

{
	if (m_console) {
		return m_console->SetMessageMode(iMode);
	}
}
