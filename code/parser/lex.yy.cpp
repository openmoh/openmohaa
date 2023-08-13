#include <stdio.h>

typedef struct yy_buffer_state* YY_BUFFER_STATE;
typedef unsigned int yy_size_t;

struct yy_buffer_state {
	FILE* yy_input_file;
	char* yy_ch_buf; 
	char* yy_buf_pos; 
	yy_size_t yy_buf_size; 
	int yy_n_chars; 
	int yy_is_our_buffer; 
	int yy_is_interactive; 
	int yy_at_bol; 
	int yy_fill_buffer; 
	int yy_buffer_status; 
};

static YY_BUFFER_STATE yy_current_buffer ;
static char* yy_c_buf_p ;
static int yy_init ;
static int yy_start ;
typedef unsigned char YY_CHAR;
FILE* yyin;
FILE* yyout;
typedef int yy_state_type;
static short int yy_accept[303];
static int yy_ec[256];
static int yy_meta[64];
static short int yy_base[327];
static short int yy_def[327];
static short int yy_nxt[1173];
static short int yy_chk[1173];
static int yy_more_flag;
static int yy_more_len;
static const char* token_names[] =
{
"TOKEN_EOL",
"TOKEN_IF",
"TOKEN_ELSE",
"TOKEN_WHILE",
"TOKEN_FOR",
"TOKEN_IDENTIFIER",
"TOKEN_LEFT_BRACES",
"TOKEN_RIGHT_BRACES",
"TOKEN_LEFT_BRACKET",
"TOKEN_RIGHT_BRACKET",
"TOKEN_LEFT_SQUARE_BRACKET",
"TOKEN_RIGHT_SQUARE_BRACKET",
"TOKEN_EQUALITY",
"TOKEN_ASSIGNMENT",
"TOKEN_COLON",
"TOKEN_DOUBLE_COLON",
"TOKEN_SEMICOLON",
"TOKEN_LOGICAL_OR",
"TOKEN_LOGICAL_AND",
"TOKEN_BITWISE_OR",
"TOKEN_BITWISE_EXCL_OR",
"TOKEN_BITWISE_AND",
"TOKEN_INEQUALITY",
"TOKEN_LESS_THAN",
"TOKEN_GREATER_THAN",
"TOKEN_LESS_THAN_OR_EQUAL",
"TOKEN_GREATER_THAN_OR_EQUAL",
"TOKEN_PLUS",
"TOKEN_PLUS_EQUALS",
"TOKEN_MINUS",
"TOKEN_NEG",
"TOKEN_POS",
"TOKEN_MINUS_EQUALS",
"TOKEN_MULTIPLY",
"TOKEN_DIVIDE",
"TOKEN_PERCENTAGE",
"TOKEN_DOLLAR",
"TOKEN_NOT",
"TOKEN_COMPLEMENT",
"TOKEN_STRING",
"TOKEN_INTEGER",
"TOKEN_FLOAT",
"TOKEN_LISTENER",
"TOKEN_PERIOD",
"TOKEN_NULL",
"TOKEN_NIL",
"TOKEN_INC",
"TOKEN_DEC",
"TOKEN_SCRIPT",
"TOKEN_TRY",
"TOKEN_CATCH",
"TOKEN_SWITCH",
"TOKEN_CASE",
"TOKEN_BREAK",
"TOKEN_CONTINUE",
"TOKEN_SIZE",
"TOKEN_END",
"TOKEN_MAKEARRAY",
"TOKEN_ENDARRAY",
};

extern yy_state_type yy_last_accepting_state;
extern char* yy_last_accepting_cpos;

enum yytokentype
{
	YYEMPTY = -2,
	TOKEN_EOF = 0,                 /* TOKEN_EOF  */
	YYerror = 256,                 /* error  */
	TOKEN_EOL = 257,
	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_WHILE,
	TOKEN_FOR,
	TOKEN_IDENTIFIER,
	TOKEN_LEFT_BRACES,
	TOKEN_RIGHT_BRACES,
	TOKEN_LEFT_BRACKET,
	TOKEN_RIGHT_BRACKET,
	TOKEN_LEFT_SQUARE_BRACKET,
	TOKEN_RIGHT_SQUARE_BRACKET,
	TOKEN_EQUALITY,
	TOKEN_ASSIGNMENT,
	TOKEN_COLON,
	TOKEN_DOUBLE_COLON,
	TOKEN_SEMICOLON,
	TOKEN_LOGICAL_OR,
	TOKEN_LOGICAL_AND,
	TOKEN_BITWISE_OR,
	TOKEN_BITWISE_EXCL_OR,
	TOKEN_BITWISE_AND,
	TOKEN_INEQUALITY,
	TOKEN_LESS_THAN,
	TOKEN_GREATER_THAN,
	TOKEN_LESS_THAN_OR_EQUAL,
	TOKEN_GREATER_THAN_OR_EQUAL,
	TOKEN_PLUS,
	TOKEN_PLUS_EQUALS,
	TOKEN_MINUS,
	TOKEN_NEG,
	TOKEN_POS,
	TOKEN_MINUS_EQUALS,
	TOKEN_MULTIPLY,
	TOKEN_DIVIDE,
	TOKEN_PERCENTAGE,
	TOKEN_DOLLAR,
	TOKEN_NOT,
	TOKEN_COMPLEMENT,
	TOKEN_STRING,
	TOKEN_INTEGER,
	TOKEN_FLOAT,
	TOKEN_LISTENER,
	TOKEN_PERIOD,
	TOKEN_NULL,
	TOKEN_NIL,
	TOKEN_INC,
	TOKEN_DEC,
	TOKEN_SCRIPT,
	TOKEN_TRY,
	TOKEN_CATCH,
	TOKEN_SWITCH,
	TOKEN_CASE,
	TOKEN_BREAK,
	TOKEN_CONTINUE,
	TOKEN_SIZE,
	TOKEN_END,
	TOKEN_MAKEARRAY,
	TOKEN_ENDARRAY,
};
typedef enum yytokentype yytoken_kind_t;

extern ScriptDisplayTokenFunc* _scriptDisplayToken;
extern int prev_yylex;

static void DisplayToken()
{
	scriptDisplayToken(token_names[prev_yylex - TOKEN_EOL], yytext);
}

static void TextEscapeValue(char* str, int len)
{
	char *to = parsetree_malloc( len + 1 );

	yylval.s.val.stringValue = to;

	while( len )
	{
		if( *str == '\\' )
		{
			len--;

			if( !len )
				break;

			str++;
			if( *str == 'n' )
			{
				*to = '\n';
			}
			else if( *str == 't' )
			{
				*to = '\t';
			}
			else if( *str == '"' )
			{
				*to = '\"';
			}
			else
			{
				*to = *str;
			}
		}
		else
		{
			*to = *str;
		}

		len--;
		str++;
		to++;
	}

	*to = 0;
}

static int UseField()
{
	return prev_yylex == TOKEN_DOLLAR || prev_yylex == TOKEN_PERIOD;
}

static int yylex()
{
	// FIXME: unimplemented
	return 0;
}

static int yy_get_next_buffer()
{
	// FIXME: unimplemented
	return 0;
}

/* yy_get_previous_state - get the state just before the EOB char was reached */

static yy_state_type yy_get_previous_state(void)
{
	yy_state_type yy_current_state;
	char* yy_cp;

	yy_current_state = (yy_start);

	for (yy_cp = yytext + yy_more_len; yy_cp < (yy_c_buf_p); ++yy_cp)
	{
		YY_CHAR yy_c = (*yy_cp ? yy_ec[((YY_CHAR)(*yy_cp))] : 1);
		if (yy_accept[yy_current_state])
		{
			(yy_last_accepting_state) = yy_current_state;
			(yy_last_accepting_cpos) = yy_cp;
		}
		while (yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state)
		{
			yy_current_state = (int)yy_def[yy_current_state];
			if (yy_current_state >= 303)
				yy_c = yy_meta[yy_c];
		}
		yy_current_state = yy_nxt[yy_base[yy_current_state] + yy_c];
	}

	return yy_current_state;
}

static yy_state_type yy_try_NUL_trans(yy_state_type yy_current_state)
{
	int yy_is_jam;
	char* yy_cp = (yy_c_buf_p);

	YY_CHAR yy_c = 1;
	if (yy_accept[yy_current_state])
	{
		(yy_last_accepting_state) = yy_current_state;
		(yy_last_accepting_cpos) = yy_cp;
	}
	while (yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state)
	{
		yy_current_state = (int)yy_def[yy_current_state];
		if (yy_current_state >= 303)
			yy_c = yy_meta[yy_c];
	}
	yy_current_state = yy_nxt[yy_base[yy_current_state] + yy_c];
	yy_is_jam = (yy_current_state == 302);

	return yy_is_jam ? 0 : yy_current_state;
}

static void yyunput(int c, char* yy_bp)
{
	// FIXME: unimplemented
}

static int input()
{
	// FIXME: unimplemented
	return 0;
}

void yyrestart(FILE* input_file)
{
	// FIXME: unimplemented
}

void yy_switch_to_buffer(YY_BUFFER_STATE new_buffer)
{
	// FIXME: unimplemented
}

void yy_load_buffer_state()
{
	// FIXME: unimplemented
}

YY_BUFFER_STATE yy_create_buffer(FILE* file, int size)
{
	// FIXME: unimplemented
	return NULL;
}

void yy_delete_buffer(YY_BUFFER_STATE b)
{
	// FIXME: unimplemented
}

void yy_init_buffer(YY_BUFFER_STATE b, FILE* file)
{
	// FIXME: unimplemented
}

void yy_flush_buffer(YY_BUFFER_STATE b)
{
	// FIXME: unimplemented
}

YY_BUFFER_STATE yy_scan_buffer(char* base, yy_size_t size)
{
	// FIXME: unimplemented
	return NULL;
}

YY_BUFFER_STATE yy_scan_string(char* yy_str)
{
	// FIXME: unimplemented
	return NULL;
}

YY_BUFFER_STATE yy_scan_bytes(char* bytes, int len)
{
	// FIXME: unimplemented
	return NULL;
}

static void yy_fatal_error(char* msg)
{
	// FIXME: unimplemented
}

static void* yy_flex_alloc(yy_size_t size)
{
	// FIXME: unimplemented
	return NULL;
}

static void* yy_flex_realloc(void* ptr, yy_size_t size)
{
	// FIXME: unimplemented
	return NULL;
}

static void yy_flex_free(void* ptr)
{
	// FIXME: unimplemented
}

void IntegerValue(char* str)
{
	// FIXME: unimplemented
}

void FloatValue(char* str)
{
	// FIXME: unimplemented
}

void Listener(int val)
{
	// FIXME: unimplemented
}
