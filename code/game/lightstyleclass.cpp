#include "lightstyleclass.h"

#include "g_local.h"
#include "archive.h"

////////////////////////
//
// LIGHTSTYLE REPOSITORY
// 
////////////////////////

LightStyleClass lightStyles;

CLASS_DECLARATION(Class, LightStyleClass, NULL)
{
	{ NULL, NULL }
};

void LightStyleClass::SetLightStyle(int index, str style)
{
	if ((index < 0) || (index >= MAX_LIGHTSTYLES))
	{
		assert(0);
		return;
	}

	styles[index] = style;
	gi.SetLightStyle(index, style.c_str());
}

void LightStyleClass::Archive(Archiver& arc)
{
	int i;

	for (i = 0; i < MAX_LIGHTSTYLES; i++)
	{
		arc.ArchiveString(&styles[i]);
		if (arc.Loading() && styles[i].length())
		{
			gi.SetLightStyle(i, styles[i].c_str());
		}
	}
}