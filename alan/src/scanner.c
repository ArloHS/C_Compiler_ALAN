/**
 * @file    scanner.c
 * @brief   The scanner for ALAN-2022.
 * @author  W.H.K. Bester (whkbester@cs.sun.ac.za)
 * @date    2022-08-03
 */

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "boolean.h"
#include "error.h"
#include "scanner.h"
#include "token.h"
#include <stdio.h>

/* --- type definitions and constants --------------------------------------- */

typedef struct {
	char      *word;                   /* the reserved word, i.e., the lexeme */
	TokenType  type;                   /* the associated token type           */
} ReservedWord;

/* --- global static variables ---------------------------------------------- */

static FILE *src_file;                 /* the source file pointer             */
static int   ch;                       /* the next source character           */
static int   column_number;            /* the current column number           */
static int   t;

static ReservedWord reserved[] = {     /* reserved words                      */

	{"and", TOKEN_AND}, {"array", TOKEN_ARRAY},{"begin", TOKEN_BEGIN},
	{"boolean", TOKEN_BOOLEAN},	{"call", TOKEN_CALL}, {"do", TOKEN_DO},
	{"else", TOKEN_ELSE}, {"elsif", TOKEN_ELSIF}, {"end", TOKEN_END},
	{"false", TOKEN_FALSE},	{"function", TOKEN_FUNCTION}, {"get", TOKEN_GET},
	{"if", TOKEN_IF}, {"integer", TOKEN_INTEGER},
	{"leave", TOKEN_LEAVE}, {"not", TOKEN_NOT}, {"or", TOKEN_OR},
	{"put", TOKEN_PUT}, {"relax", TOKEN_RELAX}, {"rem", TOKEN_REMAINDER},
	{"source", TOKEN_SOURCE}, {"then", TOKEN_THEN}, {"to", TOKEN_TO},
	{"true", TOKEN_TRUE}, {"while", TOKEN_WHILE}

};

#define NUM_RESERVED_WORDS     (sizeof(reserved) / sizeof(ReservedWord))
#define MAX_INITIAL_STRING_LEN (1024)

/* --- function prototypes -------------------------------------------------- */

static void next_char(void);
static void process_number(Token *token);
static void process_string(Token *token);
static void process_word(Token *token);
static void skip_comment(void);

/* --- scanner interface ---------------------------------------------------- */

void init_scanner(FILE *in_file)
{
	src_file = in_file;
	position.line = 1;
	position.col = column_number = 0;
	next_char();
}

void get_token(Token *token)
{
	/* remove whitespace */

	while (ch == ' ' || ch == '\t' || ch == '\n') {
		next_char();
	}

	/* remember token start */
	position.col = column_number;

	/* get the next token */
	if (ch != EOF) {
		if (isalpha(ch) || ch == '_') {

			/* process a word */
			process_word(token);

		} else if (isdigit(ch)) {

			/* process a number */
			process_number(token);

		} else switch (ch) {

			/* process a string */
			case '"':
				position.col = column_number;
				next_char();
				process_string(token);
				break;

			/* process comment */
			case '{':
				skip_comment();
				next_char();
				get_token(token);

			/*process other tokens */
				break;
			case '(':
				token->type = TOKEN_OPEN_PARENTHESIS;
				next_char();
				break;

			case ')':
				token->type = TOKEN_CLOSE_PARENTHESIS;
				next_char();
				break;

			case '}':
				leprintf("illegal character '%c' (ASCII #%d)", ch, ch);
				token->type = TOKEN_OPEN_PARENTHESIS;
				next_char();
				break;

			case '[':
				token->type = TOKEN_OPEN_BRACKET;
				next_char();
				break;

			case ']':
				token->type = TOKEN_CLOSE_BRACKET;
				next_char();
				break;

			case '+':
				token->type = TOKEN_PLUS;
				next_char();
				break;

			case '-':
				token->type = TOKEN_MINUS;
				next_char();
				break;

			case '/':
				token->type = TOKEN_DIVIDE;
				next_char();
				break;

			case '*':
				token->type = TOKEN_MULTIPLY;
				next_char();
				break;

			case ';':
				token->type = TOKEN_SEMICOLON;
				next_char();
				break;

			case ',':
				token->type = TOKEN_COMMA;
				next_char();
				break;

			case '.':
				token->type = TOKEN_CONCATENATE;
				next_char();
				break;

			case ':':
				t = ch;
				next_char();
				if (ch == '=') {
					token->type = TOKEN_GETS;
					next_char();
					break;

					} else {
						leprintf("illegal character '%c' (ASCII #%d)", t, t);
						break;
					}

			case '<':
				next_char();
				if (ch == ' ') {
					token->type = TOKEN_LESS_THAN;
					next_char();
					break;
				}

				if (ch == '>') {
					token->type = TOKEN_NOT_EQUAL;
					next_char();
					break;
				}

				if (ch == '=') {
					token->type = TOKEN_LESS_EQUAL;
					next_char();
					break;
				}

				if (ch != ' ' || ch != '=' || ch != '>') {
					token->type = TOKEN_LESS_THAN;
					break;
				}

			case '>':
				next_char();
				if (ch == ' ') {
					token->type = TOKEN_GREATER_THAN;
					next_char();
					break;
				}

				if (ch == '=') {
					token->type = TOKEN_GREATER_EQUAL;
					next_char();
					break;
				}

				if (ch != ' ' || ch != '=') {
					token->type = TOKEN_GREATER_THAN;
					break;
				}

			case '=':
				token->type = TOKEN_EQUAL;
				next_char();
				break;

			default:
				leprintf("illegal character '%c' (ASCII #%d)", ch, ch);
		}

	} else {
		token->type = TOKEN_EOF;
	}
}

/* --- utility functions ---------------------------------------------------- */

void next_char(void)
{
	static char last_read = '\0';
    /*
     * - Allocate heap space of the size of the maximum initial string length.
     * - If a string is *about* to overflow while scanning it, double the amount
     *   of space available.
     * - *Only* printable ASCII characters are allowed; see man 3 isalpha.
     * - Check the legality of escape codes.
     * - Set the appropriate token type.
     */
	last_read = ch;
	ch = fgetc(src_file);

	if (ch != EOF) {
		if (last_read == '\n') {
			position.line++;
			column_number = 1;
		}

		if (last_read != '\n') {
			column_number++;
		}
	}
}

void process_number(Token *token)
{

	/*
     * - Build number up to the specified maximum magnitude.
     * - Store the value in the appropriatetoken field.
     * - Set the appropriate token type.
     * - "Remember" the correct column number globally.
     */


	int d = ch - '0';
	int v = 0;

	while (isdigit(ch)) {
		d = ch - '0';

		if (v <= ((INT_MAX - d)/10)) {
			v = 10 * v + d;
			next_char();

		} else {
			leprintf("number too large");
		}
	}

	token-> value = v;
	token->type = TOKEN_NUMBER;
}

void process_string(Token *token)
{
	size_t i, nstring = MAX_INITIAL_STRING_LEN;

	 /*
     * - Allocate heap space of the size of the maximum initial string length.
     * - If a string is *about* to overflow while scanning it, double the amount
     *   of space available.
     * - *Only* printable ASCII characters are allowed; see man 3 isalpha.
     * - Check the legality of escape codes.
     * - Set the appropriate token type.
     */

	char *string;
	string = (char *)malloc(nstring);
	i = 0;

	while (ch != '"' && ch != EOF) {
		if(!isprint(ch)) {
			position.col = column_number;
			leprintf("non-printable character (ASCII #%d) in string", ch);
		}

		if (ch == '\\') {
			next_char();
			position.col = column_number;
			position.col--;
			if (ch == 'a' || ch == 'b' || ch == 'f' || ch == 'r'
				|| ch == 'v' || ch == '\''|| ch == '?') {
					leprintf("illegal escape code '\\%c' in string", ch);
					break;

			} else {
				string[i] = '\\';
				i++;
			}
		}

		string[i] = ch;
		i++;
		next_char();

		if (i >= nstring) {
			nstring *= 2;
			string = realloc(string, nstring);
		}
	}

	if (i > 0) {
		string = realloc(string, i);
	}

	if (i == 0) {
		i++;
		string = realloc(string, i);
	}

	if (ch == EOF) {
		leprintf("string not closed");
	}

	i++;
	string[i] = '\0';
	token-> string = string;
	token->type = TOKEN_STRING;
	next_char();
}

void process_word(Token *token)
{
	char lexeme[MAX_ID_LENGTH+1];
	int i, low, mid, high;
	i = 0;
	position.col = column_number;

	/* check that the id length is less than the maximum */

    /* do a binary search through the array of reserved words */

    /* if id was not recognised as a reserved word, it is an identifier */

	int c = 0;
	while (isalpha(ch) || isdigit(ch) || ch == '_') {
		if (i < MAX_ID_LENGTH) {
			lexeme[i] = ch;
			c++;
			next_char();
			i++;

		} else {
			leprintf("identifier too long");
			break;
		}
	}

	lexeme[i] = '\0';
	low = 0;
	high = NUM_RESERVED_WORDS - 1;
	mid = 0;
	int index = -1;

	while (low <= high) {
		if (low > high) {
			index = -1;
			break;
		}

		mid = low + (high - low) / 2;

		if (strcmp(lexeme, reserved[mid].word) < 0) {
			high = mid - 1;
		}

		if (strcmp(lexeme, reserved[mid].word) == 0) {
			index = mid;
			break;
		}

		if (strcmp(lexeme, reserved[mid].word) > 0) {
			low = mid + 1;
		}
	}

	/* if id was not recognised as a reserved word, it is an identifier */
	if (index == -1) {
		token->type = TOKEN_ID;
		strcpy(token->lexeme, lexeme);

	} else {
		token->type = reserved[index].type;
	}
}

void skip_comment(void)
{
	SourcePos start_pos;

	/*
     * - Skip nested comments *recursively*, which is to say, counting
     *   strategies are not allowed.
     * - Terminate with an error if comments are not nested properly.
     */

	start_pos.line = position.line;
	start_pos.col = column_number;
	next_char();

	while (ch != '}' && ch != EOF) {
		if (ch == '{') {
			skip_comment();
		}

		next_char();
	}

	/* force line number of error reporting */
	if (ch == EOF) {
		position = start_pos;
		leprintf("comment not closed");
	}

}
