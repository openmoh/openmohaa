#include "../script/scriptcompiler.h"

typedef int yy_state_type;

static int braces_count;
static sval_t parseValue;
int prev_yylex;
ScriptDisplayTokenFunc* _scriptDisplayToken;
static unsigned int out_pos;
static unsigned int success_pos;
static char* in_ptr;
static char* start_ptr;
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short int* yyssp;
stype_t* yyvsp;
stype_t yyval;
stype_t yylval;
short int yyss[500];
stype_t yyvs[500];
int yyleng;
static char yy_hold_char;
static int yy_n_chars;
static int yy_did_buffer_switch_on_eof;
char* yytext;
yy_state_type yy_last_accepting_state;
char* yy_last_accepting_cpos;
parseStage_e parseStage;

void TextValue(const char* str, int len)
{
	// FIXME: unimplemented
}

static int yyerror(const char* s)
{
	// FIXME: unimplemented
	return 0;
}

int yywrap()
{
	// FIXME: unimplemented
	return 0;
}

unsigned int ScriptParse(void* buffer, ScriptDisplayTokenFunc* scriptDisplayToken, void** parseData, char* type)
{
	// FIXME: unimplemented
	return 0;
}

int yyparse()
{
	// FIXME: unimplemented
	return 0;
}