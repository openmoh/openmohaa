#include "glb_local.h"
#include "cg_hud.h"
#include <iostream>
#include <fstream>
#include <string>

//Stuff for menu credit to okidoki
uidef_t *uid; 
qkey_t *keys;
qboolean *inGuiMouse = ( qboolean * )0x12F6FF8;  

//	***fonts***
fontHeader_t *facfont20 = NULL;
fontHeader_t *courier16 = NULL;
fontHeader_t *courier18 = NULL;
fontHeader_t *courier20 = NULL;
fontHeader_t *handle16 = NULL;
fontHeader_t *handle18 = NULL;
fontHeader_t *marlett = NULL;
fontHeader_t *verdana12 = NULL;
fontHeader_t *verdana14 = NULL;

/*Cvars*/
cvar_t *stats;
cvar_t *menu;

//Stat Variables
int headshots;
int killingspree;
int unstoppable;
int rampage;
int godlike;
int kills;
int bashes;
int dominating;
int double_kill_time;
int gotdouble;

int doublekill;
int multikill;
int ultrakill;
int monsterkill;

float hintAlpha = 1.0f;
int hintAlphaTime = 0;
qboolean hintClear = false;
char hintString[ MAX_STRING_TOKENS ];

// colors
vec4_t colorDkRed		= {0.5, 0, 0, 1};
vec4_t colorDkGreen		= {0, 0.5, 0, 1};
vec4_t colorDkBlue		= {0, 0, 0.5, 1};
vec4_t colorLtRed		= {1, 0.5, 0.5, 1};
vec4_t colorLtGreen		= {0.5, 1, 0.5, 1};
vec4_t colorLtBlue		= {0.5, 0.5, 1, 1};
vec4_t colorAlBlue		= {0, 0, 1, 0.25};
vec4_t colorAlRed		= {1, 0, 0, 0.25};
vec4_t colorLtzzTEST	= {1, 1, 1, 0.25};
vec4_t alphaLight		= { 0, 0, 0, 0.25 };
vec4_t alphaMid			= { 0, 0, 0, 0.5 };
vec4_t alphaDark		= { 0, 0, 0, 0.75 };
vec4_t colorTransRed	= { 1, 0, 0, 0.5 };
vec4_t colorTransBlue	= { 0, 0, 1, 0.5 };
vec4_t colorTransWhite	= { 1, 1, 1, 0.5 };
vec4_t colorDarkRed  	= { 0.34117647058823529411764705882353f, 0.035294117647058823529411764705882f, 0.035294117647058823529411764705882f, 1 };

hdExtend_t *huddrawExtends;

void CG_DrawMenu()
{
	time_t rawtime;
	struct tm * timeinfo = (tm * )malloc(sizeof(tm));
	time( &rawtime );
	localtime_s( timeinfo, &rawtime);

	char buff[64];
	/*Draw the menu*/
	if(menu->integer)
	{
		//Enable the mouse
		*inGuiMouse = uid->uiHasMouse = true;

		//Get Cursor coordinates
		uid->mouseX; 
		uid->mouseY;

		/*Widgets*/
		//We want to make a few selections(widgets)
//-------------First Widget---------------------
		cgi.R_SetColor( colorWhite ); //outer outline will be white
		cgi.R_DrawBox(10, 160, 101, 16);
		cgi.R_SetColor( colorDarkRed ); //inner red box
		cgi.R_DrawBox(11, 161, 99, 14);
		cgi.R_SetColor( colorWhite ); //font color
		sprintf_s( buff, "First Widget");//name of widget
		cgi.R_DrawString( verdana12, buff, 12, 161, -1, 0 );

//-------------Second Widget---------------------
		cgi.R_SetColor( colorWhite ); //outer outline will be white
		cgi.R_DrawBox(10, 180, 101, 16);
		cgi.R_SetColor( colorDarkRed ); //inner red box
		cgi.R_DrawBox(11, 181, 99, 14);
		cgi.R_SetColor( colorWhite ); //font color
		sprintf_s( buff, "Second Widget");//name of widget
		cgi.R_DrawString( verdana12, buff, 12, 181, -1, 0 );

//-------------Third Widget---------------------
		cgi.R_SetColor( colorWhite ); //outer outline will be white
		cgi.R_DrawBox(10, 200, 101, 16);
		cgi.R_SetColor( colorDarkRed ); //inner red box
		cgi.R_DrawBox(11, 201, 99, 14);
		cgi.R_SetColor( colorWhite ); //font color
		sprintf_s( buff, "Third Widget");//name of widget
		cgi.R_DrawString( verdana12, buff, 12, 201, -1, 0 );

//-------------Fourth Widget---------------------
		cgi.R_SetColor( colorWhite ); //outer outline will be white
		cgi.R_DrawBox(10, 220, 101, 16);
		cgi.R_SetColor( colorDarkRed ); //inner red box
		cgi.R_DrawBox(11, 221, 99, 14);
		cgi.R_SetColor( colorWhite ); //font color
		sprintf_s( buff, "Fourth Widget");//name of widget
		cgi.R_DrawString( verdana12, buff, 12, 221, -1, 0 );

		//Check if it was clicked
		if(keys[181].down) 
		{
			if((uid->mouseX >= 11)&&(uid->mouseX <= 110)&&(uid->mouseY >= 161)&&(uid->mouseY <= 175))
			{
				cgi.R_SetColor( colorWhite ); //font color
				sprintf_s( buff, (const char *)cg->snap->ps.stats);//name of widget
				cgi.R_DrawString( verdana12, buff, 12, 300, -1, 0 );
				
				cgi.Printf("I clicked the first widget \n");
			}

			if((uid->mouseX >= 11)&&(uid->mouseX <= 110)&&(uid->mouseY >= 181)&&(uid->mouseY <= 195))
				cgi.Printf("I clicked the second widget \n");

			if((uid->mouseX >= 11)&&(uid->mouseX <= 110)&&(uid->mouseY >= 201)&&(uid->mouseY <= 215))
				cgi.Printf("I clicked the third widget \n");

			if((uid->mouseX >= 11)&&(uid->mouseX <= 110)&&(uid->mouseY >= 211)&&(uid->mouseY <= 235))
				cgi.Printf("I clicked the fourth widget \n");
		}
//-------------Time---------------------
		if(timeinfo->tm_hour == 00)
		{
			cgi.R_SetColor( colorWhite );
			sprintf_s( buff, "%02i:%02i:%02i AM", timeinfo->tm_hour + 12, timeinfo->tm_min, timeinfo->tm_sec);
			cgi.R_DrawString( verdana12, buff, 12, 241, 10, 0 );
		}
		else if(timeinfo->tm_hour > 13)
		{
			cgi.R_SetColor( colorWhite );
			sprintf_s( buff, "%i:%02i:%02i PM", ((timeinfo->tm_hour)%12), timeinfo->tm_min, timeinfo->tm_sec);
			cgi.R_DrawString( verdana12, buff, 12, 241, 10, 0 );
		}
		else if(timeinfo->tm_hour > 00 && timeinfo->tm_hour < 13)
		{
			cgi.R_SetColor( colorWhite );
			sprintf_s( buff, "%i:%02i:%02i AM", ((timeinfo->tm_hour)%12), timeinfo->tm_min, timeinfo->tm_sec);
			cgi.R_DrawString( verdana12, buff, 12, 241, 10, 0 );
		}

		cgi.R_SetColor( NULL );
	}
}

qboolean CG_WorldToScreen(vec3_t pt, int *x, int *y, qboolean bOptionalSeen, int shader_width, int shader_height)
{
	float	xzi, yzi;
	vec3_t	local, transformed;
	vec3_t	vfwd, vright, vup;
	qboolean bNotShown = false;

	AngleVectors(cg->refdefViewAngles, vfwd, vright, vup);

	VectorSubtract(pt, cg->refdef.viewOrg, local);

	transformed[0] = DotProduct( local, vright );
	transformed[1] = DotProduct( local, vup );
	transformed[2] = DotProduct( local, vfwd );

	if(transformed[2] < 0.01f)
	{
		if(!bOptionalSeen)
			return false;

		bNotShown = true;

		//transformed[0] = -transformed[0];
		//transformed[1] = -transformed[1];
		//return qfalse;
	}

	int centerX = (cgs->glConfig.vidWidth / 2);
	int centerY = (cgs->glConfig.vidHeight / 2);
	xzi			= centerX / transformed[2] * ( 90.0f / cg->refdef.fovX );
	yzi			= centerY / transformed[2] * ( 90.0f / cg->refdef.fovY );
	*x			= centerX + (int)( xzi * transformed[0] );
	*y			= centerY - (int)( yzi * transformed[1] );

	if(!bOptionalSeen)
		return true;

	int limitX = cgs->glConfig.vidWidth - shader_width;
	int limitY = cgs->glConfig.vidHeight - shader_height;

	if(bNotShown)
	{
		if(*x < limitX)
			*x = limitX;
		else if(*x > 0)
			*x = 0;
		else if(*y < limitY)
			*y = limitY;
		else if(*y > 0)
			*y = 0;
	}

	if(*x > limitX)
		*x = limitX;
	else if(*x < 0)
		*x = 0;

	if(*y > limitY)
		*y = limitY;
	else if(*y < 0)
		*y = 0;

	return true;
}

#if 0
void CG_Draw3DIcon(hdExtend_t * hdex, hdElement_t * hde)
{
	int x, y;
	centity_t * ce;
	int width, height;

	/* Reset if huddraw_rect has been called */
	if(hdex->h3d.x != hde->x || hdex->h3d.y != hde->y)
	{
		memset(&hdex->h3d, 0, sizeof(hdex->h3d));
		return;
	}

	if(hdex->h3d.ent_num != -1)
		ce = CG_GetEntity(hdex->h3d.ent_num);
	else
		ce = NULL;

	if(ce != NULL)
		VectorAdd(hdex->h3d.vector, ce->currentState.origin, hdex->h3d.currentOrg);
	//else
	//	VectorCopy(hdex->h3d.vector, hdex->h3d.currentOrg);

	if(hde->shader)
	{
		width = cgi.R_GetShaderWidth(hde->shader);
		height = cgi.R_GetShaderWidth(hde->shader);
	}
	else
	{
		width = hde->width;
		height = hde->height;
	}

	if(!CG_WorldToScreen(hdex->h3d.currentOrg, &x, &y, hdex->h3d.bAlwaysShow, width, height))
		return;

	hde->horizontalAlign = 0;
	hde->verticalAlign = 0;
	hde->virtualScreen = 0;

	hde->x = x;
	hde->y = y;

	hdex->h3d.x = x;
	hdex->h3d.y = y;
}

void CG_DrawTimers(hdExtend_t * hdex, hdElement_t * hde)
{
	int minutes;
	float seconds;
	char *string;

	if( !hdex->timer.bEnabled || hdex->timer.current <= 0.0f ) {
		return;
	}

	hdex->timer.current -= 0.001000000047497451f * cg_frametime;

	if(hdex->timer.current <= 0.0f)
	{
		if(hdex->timer.fade_out_time >= 0.0f)
		{
			hde->color[3] = 0.0f;
			hdex->timer.alpha = 0.0f;
		}

		hdex->timer.current = 0.0f;
	}

	if(hdex->timer.current >= 60.00000000f)
	{
		seconds = (float)((int)hdex->timer.current % 60);
		minutes = (int)(hdex->timer.current / 60.0f);
	}
	else
	{
		if(hdex->timer.current >= 30.0f)
			seconds = (float)(int)hdex->timer.current;
		else
			seconds = hdex->timer.current;
		minutes = 0;
	}

	if(hdex->timer.current >= 30.0f)
		string = "%d:%02.0f";
	else
	{
		if(hdex->timer.current >= 10.0f)
			string = "%d:%02.1f";
		else
			string = "%d:0%.1f";
	}


	if(hdex->timer.current * 1000.0f < hdex->timer.fade_out_time)
	{
		hde->color[3] -= (2.0f-hdex->timer.alpha)/hdex->timer.fade_out_time * cg_frametime;

		if(hde->color[3] < 0.0f)
			hde->color[3] = 0.0f;
	}
	else
		hdex->timer.alpha = hde->color[3];

	if(!hde->shader)
	{
		if(_strcmpi(hde->string, hdex->timer.string) == 0)
		{
			sprintf(hde->string, string, minutes, seconds);
			strcpy(hdex->timer.string, hde->string);
		}
	}
	else
		memset(hde->string, 0, MAX_STRINGCHARS);
}

void CG_DrawExtendedHud()
{
	int i;
	hdExtend_t *hdex;
	hdElement_t *hde;

	if( hintString[0] != '\0' )
	{
		int key1 = 0, key2 = 0;
		const char *keyname1, *keyname2;
		char *hintStringFormatted;
		char *hintStringKey;
		char *hintStringPart;
		int i, old_c;

		if( !facfont20 ) {
			return;
		}

		// Fade out the string over time
		if( hintClear )
		{
			if( hintAlpha > 0.0f )
			{
				float ratio = hintAlphaTime/100.0f;
				hintAlphaTime += cg->frametime;

				if( ratio >= 1.0f ) {
					hintAlpha = 0.0f;
				} else {
					hintAlpha = 1.0f + ( 0.0f - 1.0f ) * ratio;
				}
			}
			else
			{
				hintAlpha = 0.0f;
				hintAlphaTime = 0;
				hintClear = false;
				memset( hintString, '\0', strlen( hintString ) );
			}
		} else {
			hintAlpha = 1.0f;
		}

		cgi.Key_GetKeysForCommand( "+use", &key1, &key2 );

		keyname1 = uppercase( cgi.LV_ConvertString ( cgi.Key_KeynumToBindString( key1 ) ) );
		keyname2 = uppercase( cgi.LV_ConvertString ( cgi.Key_KeynumToBindString( key2 ) ) );

		// Format the hint string
		hintStringFormatted = replace( hintString, "&&1", "\t&&1" );
		hintStringFormatted = replace( hintStringFormatted, "&&1", keyname1 );

		char *hintStringLocalized = cgi.LV_ConvertString( hintStringFormatted );

		int w = re.GetFontStringWidth( facfont20, hintStringLocalized ) - 10;

		for( i = 0; i < strlen( hintStringLocalized ); i++ )
		{
			if( hintStringLocalized[ i ] == '\t' )
			{
				hintStringLocalized[ i ] = '\0';
				break;
			}
		}

		int x = ( cgs->glConfig.vidWidth - w ) / 2;
		int y = cgs->glConfig.vidHeight / 1.75;

		vec4_t rgba = { 1, 1, 1, hintAlpha };

		cgi.R_SetColor( rgba );
		cgi.R_DrawString( facfont20, hintStringLocalized, x, y, MAX_STRINGTOKENS, false );

		x += re.GetFontStringWidth( facfont20, hintStringLocalized );

		hintStringKey = hintStringLocalized + i + 1;

		// yellow
		rgba[0] = 1.0f;
		rgba[1] = 1.0f;
		rgba[2] = 0.0f;
		rgba[3] = hintAlpha;

		int len = strlen( keyname1 );

		old_c = hintStringKey[ len ];
		hintStringKey[ len ] = '\0';

		cgi.R_SetColor( rgba );
		cgi.R_DrawString( facfont20, hintStringKey, x, y, MAX_STRINGTOKENS, false );

		hintStringKey[ len ] = old_c;

		x += re.GetFontStringWidth( facfont20, keyname1 );

		hintStringPart = hintStringKey + len;

		rgba[0] = 1.0f;
		rgba[1] = 1.0f;
		rgba[2] = 1.0f;
		rgba[3] = hintAlpha;

		cgi.R_SetColor( rgba );
		cgi.R_DrawString( facfont20, hintStringPart, x, y, MAX_STRINGTOKENS, false );
	}

	for( i = 0; i < 256; i++ )
	{
		hde = cgi.hudDrawElements + i;
		hdex = huddrawExtends + i;

		if( hdex->h3d.bEnabled ) {
			CG_Draw3DIcon( hdex, hde );
		}

		if( hdex->timer.bEnabled ) {
			CG_DrawTimers( hdex, hde );
		}
	}
}
#endif

void CG_DrawHintString()
{
	if( hintString[0] != '\0' )
	{
		int key1 = 0, key2 = 0;
		str keyname1, keyname2;
		char *hintStringFormatted;
		char *hintStringKey;
		char *hintStringPart;
		int i, old_c;

		if( !facfont20 ) {
			return;
		}

		// Fade out the string over time
		if( hintClear )
		{
			if( hintAlpha > 0.0f )
			{
				float ratio = hintAlphaTime/100.0f;
				hintAlphaTime += cg->frametime;

				if( ratio >= 1.0f ) {
					hintAlpha = 0.0f;
				} else {
					hintAlpha = 1.0f + ( 0.0f - 1.0f ) * ratio;
				}
			}
			else
			{
				hintAlpha = 0.0f;
				hintAlphaTime = 0;
				hintClear = false;
				memset( hintString, '\0', strlen( hintString ) );
			}
		} else {
			hintAlpha = 1.0f;
		}

		cgi.Key_GetKeysForCommand( "+use", &key1, &key2 );

		keyname1 = cgi.LV_ConvertString( cgi.Key_KeynumToBindString( key1 ) );
		keyname2 = cgi.LV_ConvertString( cgi.Key_KeynumToBindString( key2 ) );

		keyname1.toupper();
		keyname2.toupper();

		// Format the hint string
		hintStringFormatted = replace( hintString, "&&1", "\t&&1" );
		hintStringFormatted = replace( hintStringFormatted, "&&1", keyname1 );

		char *hintStringLocalized = cgi.LV_ConvertString( hintStringFormatted );

		float w = re.GetFontStringWidth( facfont20, hintStringLocalized ) - 10;

		for( i = 0; i < strlen( hintStringLocalized ); i++ )
		{
			if( hintStringLocalized[ i ] == '\t' )
			{
				hintStringLocalized[ i ] = '\0';
				break;
			}
		}

		float x = ( cgs->glConfig.vidWidth - w ) / 2;
		float y = cgs->glConfig.vidHeight / 1.75;

		vec4_t rgba = { 1, 1, 1, hintAlpha };

		cgi.R_SetColor( rgba );
		cgi.R_DrawString( facfont20, hintStringLocalized, x, y, MAX_STRING_TOKENS, false );

		x += re.GetFontStringWidth( facfont20, hintStringLocalized );

		hintStringKey = hintStringLocalized + i + 1;

		// yellow
		rgba[0] = 1.0f;
		rgba[1] = 1.0f;
		rgba[2] = 0.0f;
		rgba[3] = hintAlpha;

		int len = strlen( keyname1 );

		old_c = hintStringKey[ len ];
		hintStringKey[ len ] = '\0';

		cgi.R_SetColor( rgba );
		cgi.R_DrawString( facfont20, hintStringKey, x, y, MAX_STRING_TOKENS, false );

		hintStringKey[ len ] = old_c;

		x += re.GetFontStringWidth( facfont20, keyname1 );

		hintStringPart = hintStringKey + len;

		rgba[0] = 1.0f;
		rgba[1] = 1.0f;
		rgba[2] = 1.0f;
		rgba[3] = hintAlpha;

		cgi.R_SetColor( rgba );
		cgi.R_DrawString( facfont20, hintStringPart, x, y, MAX_STRING_TOKENS, false );
	}
}

void CG_ShowHint( const char * hint )
{
	if( hint == NULL || strlen( hint ) <= 0 || _stricmp( hint, "" ) == 0 || hint[0] == '\0' )
	{
		hintAlphaTime = 0;
		hintClear = true;
		return;
	}

	hintClear = false;
	strcpy( hintString, hint );
}

void CG_Draw2D(void) 
{
//	CG_DrawStats();
//	CG_DrawMenu();
//	CG_DrawExtendedHud();

	if( facfont20 == NULL ) {
		facfont20 = cgi.R_LoadFont( "facfont-20" );
	}

	if( courier16 == NULL ) {
		courier16 = cgi.R_LoadFont( "courier-16" );
	}

	if( courier18 == NULL ) {
		courier18 = cgi.R_LoadFont( "courier-18" );
	}

	if( courier20 == NULL ) {
		courier20 = cgi.R_LoadFont( "courier-20" );
	}

	if( handle16 == NULL ) {
		handle16 = cgi.R_LoadFont( "handle-16" );
	}

	if( handle18 == NULL ) {
		handle18 = cgi.R_LoadFont( "handle-18" );
	}

	if( marlett == NULL ) {
		marlett = cgi.R_LoadFont( "marlett" );
	}

	if( verdana12 == NULL ) {
		verdana12 = cgi.R_LoadFont( "verdana-12" );
	}

	if( verdana14 == NULL ) {
		verdana14 = cgi.R_LoadFont( "verdana-14" );
	}

	CG_DrawHintString();

	// huddraw elements have the lowest priority
	Hud::ProcessThink();

	vec4_t col = { 1.f, 1.f, 1.f, 1.f };
	cgi.R_SetColor( col );

	cge.CG_Draw2D();
}
