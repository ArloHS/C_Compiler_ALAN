/**
 * @file    alanc.c
 *
 * A recursive-descent compiler for the ALAN-2022 language.
 *
 * All scanning errors are handled in the scanner.  Parser errors MUST be
 * handled by the <code>abort_compile</code> function.  System and environment errors,
 * for example, running out of memory, MUST be handled in the unit in which they
 * occur.  Transient errors, for example, non-existent files, MUST be reported
 * where they occur.  There are no warnings, which is to say, all errors are
 * fatal and MUST cause compilation to terminate with an abnormal error code.
 *
 * @author  W.H.K. Bester (whkbester@cs.sun.ac.za)
 * @date    2022-08-03
 */

/* Include the appropriate system and project header files. */

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "boolean.h"
#include "error.h"
#include "scanner.h"
#include "token.h"
#include <stdio.h>
#include "errmsg.h"
#include "valtypes.h"
#include "symboltable.h"
#include "codegen.h"
#include <stdarg.h>

/* --- type definitions ----------------------------------------------------- */

/* Uncomment the following for use during type checking. */

typedef struct variable_s Variable;
struct variable_s {
	char      *id;     /**< variable identifier                       */
	ValType    type;   /**< variable type                             */
	SourcePos  pos;    /**< variable position in the source           */
	Variable  *next;   /**< pointer to the next variable in the list  */
};

/* --- debugging ------------------------------------------------------------ */

/*  Your Makefile has a variable called DFLAGS.  If it is set to contain
 * -DDEBUG_PARSER, it will cause the following prototypes to be included, and
 *  the functions to which they refer (given at the end of this file) will be
 *  compiled.  If, on the other hand, this flag is comment out, by setting
 *  DFLAGS to #-DDEBUG_PARSER, these functions will be excluded.  These
 *  definitions should be used at the start and end of every parse function.
 *  For an example, see the provided parse_program function.
 */

#ifdef DEBUG_PARSER
void debug_start(const char *fmt, ...);
void debug_end(const char *fmt, ...);
void debug_info(const char *fmt, ...);
#define DBG_start(...) debug_start(__VA_ARGS__)
#define DBG_end(...) debug_end(__VA_ARGS__)
#define DBG_info(...) debug_info(__VA_ARGS__)
#else
#define DBG_start(...)
#define DBG_end(...)
#define DBG_info(...)
#endif /* DEBUG_PARSER */

/* --- global variables ----------------------------------------------------- */

Token    token;        /**< the lookahead token.type                */
FILE    *src_file;     /**< the source code file                    */
ValType  return_type;  /**< the return type of the current function */

/* Uncomment the previous definition for use during type checking. */

/* --- function prototypes: parser routines --------------------------------- */

void parse_source(void);
/* Add the prototypes for the rest of the parse functions. */
void parse_funcdef(void);
void parse_body(void);
void parse_type(ValType *type);
void parse_vardef(void);
void parse_statements(void);
void parse_statement(void);
void parse_assign(void);
void parse_call(void);
void parse_if(void);
void parse_input(void);
void parse_leave(void);
void parse_output(void);
void parse_while(void);
void parse_expr(ValType *type);
void parse_simple(ValType *type);
void parse_term(ValType *type);
void parse_factor(ValType *type);

/* --- helper macros -------------------------------------------------------- */

#define STARTS_FACTOR(toktype) \
	(toktype == TOKEN_ID || toktype == TOKEN_NUMBER || \
	 toktype == TOKEN_OPEN_PARENTHESIS || toktype == TOKEN_NOT || \
	 toktype == TOKEN_TRUE || toktype == TOKEN_FALSE)

#define STARTS_EXPR(toktype) \
	(toktype == TOKEN_MINUS || toktype == TOKEN_ID || toktype == TOKEN_TRUE || \
	toktype == TOKEN_FALSE || toktype == TOKEN_NOT || \
	toktype == TOKEN_NUMBER || toktype == TOKEN_OPEN_PARENTHESIS)

#define IS_ADDOP(toktype) \
	(toktype >= TOKEN_MINUS && toktype <= TOKEN_PLUS)

#define IS_MULOP(toktype) \
	(toktype == TOKEN_AND || toktype == TOKEN_MULTIPLY || \
	toktype == TOKEN_DIVIDE ||toktype == TOKEN_REMAINDER)

#define IS_ORDOP(toktype) \

#define IS_RELOP(toktype) \
	(toktype == TOKEN_EQUAL || toktype == TOKEN_GREATER_EQUAL || \
	toktype == TOKEN_GREATER_THAN || toktype == TOKEN_LESS_EQUAL || \
	toktype == TOKEN_LESS_THAN || toktype == TOKEN_NOT_EQUAL)


#define IS_TYPE_TOKEN(toktype) \
	(toktype == TOKEN_BOOLEAN || toktype == TOKEN_INTEGER)

#define IS_TYPE_STATEMENT(toktype) \
	(toktype == TOKEN_ID || toktype == TOKEN_CALL || toktype == TOKEN_IF || \
	toktype == TOKEN_GET || toktype == TOKEN_LEAVE || toktype == TOKEN_PUT || \
	toktype == TOKEN_WHILE)

/* --- function prototypes: helper routines --------------------------------- */

/* Uncomment the following commented-out prototypes for use during type
 * checking.
 */

void check_types(ValType type1, ValType type2, SourcePos *pos, ...);
void expect(TokenType type);
void expect_id(char **id);
IDprop *idprop(ValType type, unsigned int offset, unsigned int nparams,
		ValType *params);
Variable *variable(char *id, ValType type, SourcePos pos);

/* --- function prototypes: error reporting --------------------------------- */

void abort_compile(Error err, ...);
void abort_compile_pos(SourcePos *posp, Error err, ...);

/* --- main routine --------------------------------------------------------- */

int main(int argc, char *argv[])
{
#if 1
	char *jasmin_path;
#endif

	/* Uncomment the previous definition for code generation. */

	/* set up global variables */
	setprogname(argv[0]);

	/* check command-line arguments and environment */
	if (argc != 2) {
		eprintf("usage: %s <filename>", getprogname());
	}

	/* Uncomment the following for code generation */

	if ((jasmin_path = getenv("JASMIN_JAR")) == NULL) {
		eprintf("JASMIN_JAR environment variable not set");
	}

	/* open the source file, and report an error if it cannot be opened */
	if ((src_file = fopen(argv[1], "r")) == NULL) {
		eprintf("file '%s' could not be opened:", argv[1]);
	}
	setsrcname(argv[1]);

	/* initialise all compiler units */
	init_scanner(src_file);
	init_symbol_table();
	init_code_generation();

	/* compile */
	get_token(&token);
	parse_source();

	/* produce the object code, and assemble */
	/* Add calls for code generation. */

	make_code_file();
	assemble(jasmin_path);

	/* release allocated resources */
	/* Release the resources of the symbol table and code generation. */
	release_symbol_table();
	release_code_generation();
	fclose(src_file);
	freeprogname();
	freesrcname();

#ifdef DEBUG_PARSER
	printf("SUCCESS!\n");
#endif

	return EXIT_SUCCESS;
}

/* --- parser routines ------------------------------------------------------ */

/*
 * <source> = "source" <id> { <funcdef> } <body>.
 */
void parse_source(void)
{
	char *class_name;

	DBG_start("<source>");

	/* For code generation, set the class name inside this function, and
	 * also handle initialising and closing the "main" function.  But from the
	 * perspective of simple parsing, this function is complete.
	 */

	expect(TOKEN_SOURCE);
	expect_id(&class_name);

	/* Set the class name here for code generation. */

	set_class_name(class_name);

	while (token.type == TOKEN_FUNCTION) {
		parse_funcdef();
	}

	init_subroutine_codegen("main", idprop(TYPE_CALLABLE, 0, 0, NULL));
	parse_body();
	gen_1(JVM_RETURN);
	close_subroutine_codegen(get_variables_width());

	free(class_name);
	DBG_end("</source>");
}

/* Turn the EBNF into a program by writing one parse function for those
 * productions as instructed in the specification.  I suggest you use the
 * production as comment to the function.  Also, you may only report errors
 * through the abort_compile and abort_compile_pos functions.  You must figure
 * out what arguments you should pass for each particular error.  Keep to the
 * *exact* error messages given in the specification.
 */

/*
 * <funcdef> = “function” <id> “(” [<type> <id> {“,” <type> <id>}] “)” [“to” <type>] <body>.
 */

void parse_funcdef(void)
{
	char *fname;
	ValType vt;
	expect(TOKEN_FUNCTION);
	expect_id(&fname);
	char *function = fname;
	expect(TOKEN_OPEN_PARENTHESIS);
	Variable *next, *prev;
	unsigned int numparams = 0;
	if (IS_TYPE_TOKEN(token.type)) {
		char *tname;
		parse_type(&vt);
		expect_id(&tname);
		next = variable(tname, vt, position);
		prev = next;
		numparams++;

		while (token.type == TOKEN_COMMA) {
			char *t2name;
			ValType vt2;
			expect(TOKEN_COMMA);
			parse_type(&vt2);
			expect_id(&t2name);
			next->next = variable(t2name, vt2, position);
			numparams++;
		}
	}

	expect(TOKEN_CLOSE_PARENTHESIS);

	if (token.type == TOKEN_TO) {
		expect(TOKEN_TO);
		parse_type(&return_type);
		ValType *v = (ValType*) malloc(numparams * sizeof(ValType));
		unsigned int i;
		for (i = 0; i < numparams; i++) {
			v[i] = next->type;
			next = next->next;
		}

		SET_AS_CALLABLE(return_type);
		open_subroutine(function, idprop(return_type, get_variables_width(), numparams, v));
		init_subroutine_codegen(function, idprop(return_type, get_variables_width(), numparams, v));
		next = prev;

		while (next != NULL) {
			if (!insert_name(next->id, idprop(next->type, get_variables_width(), 0, NULL))) {
				leprintf("multiple defenition of %s", next->id);
			}

			next = next->next;
		}

	}
	parse_body();
	close_subroutine();
	close_subroutine_codegen(get_variables_width());
}

/*
 * <body> = “begin” {<vardef>} <statements> "end".
 */

void parse_body(void)
{
	expect(TOKEN_BEGIN);
	while (IS_TYPE_TOKEN(token.type)) {
		parse_vardef();
	}

	parse_statements();
	expect(TOKEN_END);
}

/*
 * <type> = ("boolean" | "integer") ["array"].
 */

void parse_type(ValType *type)
{
	switch (token.type) {
		case TOKEN_BOOLEAN:
			expect(TOKEN_BOOLEAN);
			*type = TYPE_BOOLEAN;
			break;

		case TOKEN_INTEGER:
			expect(TOKEN_INTEGER);
			*type = TYPE_INTEGER;
			break;

		default: abort_compile(ERR_TYPE_EXPECTED, token.type);
			break;
	}

	if (token.type == TOKEN_ARRAY) {
		expect(TOKEN_ARRAY);
		SET_AS_ARRAY(*type);
	}
}

/*
 * <vardef> = <type> <id> {"," <id>} ";".
 */

void parse_vardef(void)
{
	char *vname;
	ValType v;

	parse_type(&v);
	expect_id(&vname);

	if (!insert_name(vname, idprop(v, get_variables_width(), 0, NULL))) {
		leprintf("multiple defenition of %s", vname);
	}

	while (token.type == TOKEN_COMMA) {
		get_token(&token);
		expect_id(&vname);

		if (!insert_name(vname, idprop(v, get_variables_width(), 0, NULL))) {
			leprintf("multiple defenition of %s", vname);
		}
	}
	expect(TOKEN_SEMICOLON);
}

/*
 * <statements> = "relax" | <statement> {";" statement}.
 */
void parse_statements(void)
{
	if (token.type == TOKEN_RELAX) {
		expect(TOKEN_RELAX);
	}

	else if (IS_TYPE_STATEMENT(token.type)) {
		parse_statement();

	} else {
		abort_compile(ERR_STATEMENT_EXPECTED, token.type);
	}

	while (token.type == TOKEN_SEMICOLON) {
		expect(TOKEN_SEMICOLON);
		parse_statement();
	}
}

/*
 * <statement> = <assign> | <call> | <if> | <input> | <leave> | <output> |
 * <while>.
 */
void parse_statement(void)
{
	switch (token.type) {
		case TOKEN_ID:
			parse_assign();
			break;

		case TOKEN_CALL:
			parse_call();
			break;

		case TOKEN_IF:
			parse_if();
			break;

		case TOKEN_GET:
			parse_input();
			break;

		case TOKEN_LEAVE:
			parse_leave();
			break;

		case TOKEN_PUT:
			parse_output();
			break;

		case TOKEN_WHILE:
			parse_while();
			break;

		default: abort_compile(ERR_STATEMENT_EXPECTED, token.type);
			break;
	}
}

/*
 * <assign> = <id> ["[" <simple> "]"] ":=" (<expr>) | "array" <simple>).
 */
void parse_assign(void)
{
	char *aname;
	ValType type;
	expect_id(&aname);

	if (token.type == TOKEN_OPEN_BRACKET) {
		expect(TOKEN_OPEN_BRACKET);

		IDprop *p;
		if (find_name(aname, &p)) {
			gen_2(JVM_ALOAD, p->offset);
		}

		parse_simple(&type);
		expect(TOKEN_CLOSE_BRACKET);
	}

	expect(TOKEN_GETS);

	if (STARTS_EXPR(token.type)) {
		parse_expr(&type);
		IDprop *p;
		if (find_name(aname, &p)) {

			if (IS_ARRAY_TYPE(p->type)) {
				gen_1(JVM_IASTORE);
			} else {
				gen_2(JVM_ISTORE, p->offset);
			}
		}

	} else if (token.type == TOKEN_ARRAY) {
		expect(TOKEN_ARRAY);
		parse_simple(&type);
		IDprop *p;
		if (find_name(aname, &p)) {
			if (IS_BOOLEAN_TYPE(p->type)) {
				gen_newarray(T_INT);
			} else if (IS_INTEGER_TYPE(p->type)) {
				gen_newarray(T_INT);
			}
		gen_2(JVM_ASTORE, p->offset);

		}

	} else {
		abort_compile(ERR_ARRAY_ALLOCATION_OR_EXPRESSION_EXPECTED);
	}

	free(aname);
}

/*
 * <call> = “call” <id> "(" [expr {"," <expr> }] ")".
 */
void parse_call(void)
{
	char *cname;
	IDprop *p;
	ValType type;
	expect(TOKEN_CALL);
	expect_id(&cname);
	expect(TOKEN_OPEN_PARENTHESIS);

	if (STARTS_EXPR(token.type)) {
		parse_expr(&type);

		while (token.type == TOKEN_COMMA) {
			expect(TOKEN_COMMA);
			parse_expr(&type);
		}
	}
	
	if (find_name(cname, &p)) {
		gen_call(cname, p);
	}

	expect(TOKEN_CLOSE_PARENTHESIS);
	free(cname);
}

/*
 * <if> = “if” <expr> "then" <statements> {"elsif" <expr> "then" <statements> }
 * ["else" <statements> ] "end".
 */
void parse_if(void)
{
	Label start, end, next;
	start = get_label();
	end = get_label();
	next = get_label();

	ValType type;
	expect(TOKEN_IF);
	parse_expr(&type);
	gen_2_label(JVM_IFEQ, next);
	expect(TOKEN_THEN);
	parse_statements();
	gen_2_label(JVM_GOTO, end);
	gen_label(next);

	while (token.type == TOKEN_ELSIF) {
		start = get_label();
		expect(TOKEN_ELSIF);
		parse_expr(&type);
		gen_2_label(JVM_IFEQ, start);
		expect(TOKEN_THEN);
		parse_statements();
		gen_2_label(JVM_GOTO, end);
		gen_label(start);
	}

	if (token.type == TOKEN_ELSE) {
		expect(TOKEN_ELSE);
		parse_statements();
	}

	gen_label(end);
	expect(TOKEN_END);
}

/*
 * <input> = “get” <id> ["[" <simple> "]"].
 */
void parse_input(void)
{
	char *iname;
	ValType type;
	expect(TOKEN_GET);
	expect_id(&iname);

	if (token.type == TOKEN_OPEN_BRACKET) {
		expect(TOKEN_OPEN_BRACKET);
		parse_simple(&type);
		expect(TOKEN_CLOSE_BRACKET);
	}

	IDprop *p;
	if (find_name(iname, &p)) {
		gen_read(p->type);
		gen_2(JVM_ISTORE, p->offset);
	}

	free(iname);
}

/*
 * <leave> = “leave” [<expr>].
 */
void parse_leave(void)
{
	ValType type;
	expect(TOKEN_LEAVE);

	if (STARTS_EXPR(token.type)) {
		parse_expr(&type);
		gen_1(JVM_IRETURN);
	}
}

/*
 * <output> = “put” (<string> | <expr>) {"." (<string> | <expr)}.
 */
void parse_output(void)
{
	ValType type;
	expect(TOKEN_PUT);

	if (token.type == TOKEN_STRING) {
		gen_print_string(token.string);
		expect(TOKEN_STRING);

	} else if (STARTS_EXPR(token.type)) {
		parse_expr(&type);
		gen_print(type);

	} else {
		abort_compile(ERR_EXPRESSION_OR_STRING_EXPECTED, token.type);
	}

	while (token.type == TOKEN_CONCATENATE) {
		expect(TOKEN_CONCATENATE);

		if (token.type == TOKEN_STRING) {
		gen_print_string(token.string);
			expect(TOKEN_STRING);
		}

		else if (STARTS_EXPR(token.type)) {
			parse_expr(&type);
			gen_print(type);
		} else {
			abort_compile(ERR_EXPRESSION_OR_STRING_EXPECTED, token.type);
		}
	}
}

/*
 * <while> = “while” <expr> "do" <statements> "end".
 */
void parse_while(void)
{
	Label start, end;
	start = get_label();
	end = get_label();
	ValType type;
	expect(TOKEN_WHILE);
	gen_label(start);
	parse_expr(&type);
	gen_2_label(JVM_IFEQ, end);
	expect(TOKEN_DO);
	parse_statements();
	gen_2_label(JVM_GOTO, start);
	gen_label(end);
	expect(TOKEN_END);
}

/*
 * <expr> = <simple> [<relop> <simple>].
 */
void parse_expr(ValType *type)
{
	parse_simple(type);

	if (IS_RELOP(token.type)) {
		switch (token.type) {
			case TOKEN_EQUAL:
				expect(TOKEN_EQUAL);
				parse_simple(type);
				gen_cmp(JVM_IF_ICMPEQ);
				break;

			case TOKEN_GREATER_EQUAL:
				expect(TOKEN_GREATER_EQUAL);
				parse_simple(type);
				gen_cmp(JVM_IF_ICMPGE);
				break;

			case TOKEN_GREATER_THAN:
				expect(TOKEN_GREATER_THAN);
				parse_simple(type);
				gen_cmp(JVM_IF_ICMPGT);
				break;

			case TOKEN_LESS_EQUAL:
				expect(TOKEN_LESS_EQUAL);
				parse_simple(type);
				gen_cmp(JVM_IF_ICMPLE);
				break;

			case TOKEN_LESS_THAN:
				expect(TOKEN_LESS_THAN);
				parse_simple(type);
				gen_cmp(JVM_IF_ICMPLT);
				break;

			case TOKEN_NOT_EQUAL:
				expect(TOKEN_NOT_EQUAL);
				parse_simple(type);
				gen_cmp(JVM_IF_ICMPNE);
				break;
			default:
				break;
		}
	}
}

/*
 * <simple> = ["-"] <term> {<addop> <term>}.
 */
void parse_simple(ValType *type)
{
	int min = 0;
	if (token.type == TOKEN_MINUS) {
		gen_2(JVM_LDC, 0);
		expect(TOKEN_MINUS);
		min = 1;
	}

	parse_term(type);
	if (min == 1) {
		gen_1(JVM_ISUB);
	}

	while (IS_ADDOP(token.type)) {
		switch (token.type) {
			case TOKEN_PLUS:
				expect(TOKEN_PLUS);
				parse_term(type);
				gen_1(JVM_IADD);
				break;

			case TOKEN_MINUS:
				expect(TOKEN_MINUS);
				parse_term(type);
				gen_1(JVM_ISUB);
				break;

			case TOKEN_OR:
				expect(TOKEN_OR);
				parse_term(type);
				gen_1(JVM_IOR);
				break;

			default:
				break;
		}
	}
}

/*
 * <term> = <factor> {<mulop> <factor>}.
 */
void parse_term(ValType *type)
{
	parse_factor(type);

	while (IS_MULOP(token.type)) {
		switch (token.type) {
			case TOKEN_AND:
				expect(TOKEN_AND);
				parse_factor(type);
				gen_1(JVM_IAND);
				break;

			case TOKEN_MULTIPLY:
				expect(TOKEN_MULTIPLY);
				parse_factor(type);
				gen_1(JVM_IMUL);
				break;

			case TOKEN_DIVIDE:
				expect(TOKEN_DIVIDE);
				parse_factor(type);
				gen_1(JVM_IDIV);
				break;

			case TOKEN_REMAINDER:
				expect(TOKEN_REMAINDER);
				parse_factor(type);
				gen_1(JVM_IREM);
				break;

			default:
				break;
		}
	}
}

/*
 * <factor> = <id> ["[" <simple> "]"] | "(" [<expr>{"," <expr>}] ")"] | <num> |
 * "(" <expr> ")" | "not" <factor> | "true" | "false".
 */
void parse_factor(ValType *type)
{
	switch (token.type) {
		char *finame;
		case TOKEN_ID:
			expect_id(&finame);

			IDprop *p;
			if (find_name(finame, &p)) {

				if (IS_ARRAY_TYPE(p->type)) {
					gen_2(JVM_ALOAD, p->offset);
					*type = p->type;
				} else {
					if (!IS_CALLABLE_TYPE(p->type)) {
						gen_2(JVM_ILOAD, p->offset);
						*type = p->type;
					}
				}
			}


			if (token.type == TOKEN_OPEN_BRACKET) {
				expect(TOKEN_OPEN_BRACKET);
				parse_simple(type);
				gen_1(JVM_IALOAD);
				expect(TOKEN_CLOSE_BRACKET);
			}

			if (token.type == TOKEN_OPEN_PARENTHESIS) {
				expect(TOKEN_OPEN_PARENTHESIS);
				if (STARTS_EXPR(token.type)) {
					parse_expr(type);
					while (token.type == TOKEN_COMMA) {
						expect(TOKEN_COMMA);
						parse_expr(type);
					}
				}
				expect(TOKEN_CLOSE_PARENTHESIS);
				gen_call(finame, p);
			}
			break;

		case TOKEN_NUMBER:
			gen_2(JVM_LDC, token.value);
			*type = TYPE_INTEGER;
			expect(TOKEN_NUMBER);
			break;

		case TOKEN_OPEN_PARENTHESIS:
			expect(TOKEN_OPEN_PARENTHESIS);
			parse_expr(type);
			expect(TOKEN_CLOSE_PARENTHESIS);
			break;

		case TOKEN_NOT:
			expect(TOKEN_NOT);
			parse_factor(type);
			break;

		case TOKEN_TRUE:
			gen_2(JVM_LDC, 1);
			*type = TYPE_BOOLEAN;
			expect(TOKEN_TRUE);
			break;

		case TOKEN_FALSE:
			gen_2(JVM_LDC, 0);
			*type = TYPE_BOOLEAN;
			expect(TOKEN_FALSE);
			break;

		default:
			abort_compile(ERR_FACTOR_EXPECTED, token.type);
			break;

		free(finame);
	}

}

/* --- helper routines ------------------------------------------------------ */

#define MAX_MESSAGE_LENGTH 256

/* Uncomment the following function for use during type checking. */

void check_types(ValType found, ValType expected, SourcePos *pos, ...)
{
	char buf[MAX_MESSAGE_LENGTH], *s;
	va_list ap;

	if (found != expected) {
		buf[0] = '\0';
		va_start(ap, pos);
		s = va_arg(ap, char *);
		vsnprintf(buf, MAX_MESSAGE_LENGTH, s, ap);
		va_end(ap);
		if (pos != NULL) {
			position = *pos;
		}
		leprintf("incompatible types (expected %s, found %s) %s",
			get_valtype_string(expected), get_valtype_string(found), buf);
	}
}

void expect(TokenType type)
{
	if (token.type == type) {
		get_token(&token);
	} else {
		abort_compile(ERR_EXPECT, type);
	}
}

void expect_id(char **id)
{

	if (token.type == TOKEN_ID) {
		*id = strdup(token.lexeme);
		get_token(&token);
	} else {
		abort_compile(ERR_EXPECT, TOKEN_ID);
	}
}

/* Uncomment the following two functions for use during type checking. */

IDprop *idprop(ValType type, unsigned int offset, unsigned int nparams,
		ValType *params)
{
	IDprop *ip;

	ip = emalloc(sizeof(IDprop));
	ip->type = type;
	ip->offset = offset;
	ip->nparams = nparams;
	ip->params = params;

	return ip;
}

Variable *variable(char *id, ValType type, SourcePos pos)
{
	Variable *vp;

	vp = emalloc(sizeof(Variable));
	vp->id = id;
	vp->type = type;
	vp->pos = pos;
	vp->next = NULL;

	return vp;
}

/* --- error handling routine ----------------------------------------------- */

void _abort_compile(SourcePos *posp, Error err, va_list args);

void abort_compile(Error err, ...)
{
	va_list args;

	va_start(args, err);
	_abort_compile(NULL, err, args);
	va_end(args);
}

void abort_compile_pos(SourcePos *posp, Error err, ...)
{
	va_list args;

	va_start(args, err);
	_abort_compile(posp, err, args);
	va_end(args);
}

void _abort_compile(SourcePos *posp, Error err, va_list args)
{
	char expstr[MAX_MESSAGE_LENGTH], *s;
	int t;

	if (posp) {
		position = *posp;
	}

	snprintf(expstr, MAX_MESSAGE_LENGTH, "expected %%s, but found %s",
		get_token_string(token.type));

	switch (err) {
		case ERR_ILLEGAL_ARRAY_OPERATION:
		case ERR_MULTIPLE_DEFINITION:
		case ERR_NOT_A_FUNCTION:
		case ERR_NOT_A_PROCEDURE:
		case ERR_NOT_A_VARIABLE:
		case ERR_NOT_AN_ARRAY:
		case ERR_SCALAR_EXPECTED:
		case ERR_TOO_FEW_ARGUMENTS:
		case ERR_TOO_MANY_ARGUMENTS:
		case ERR_UNREACHABLE:
		case ERR_UNKNOWN_IDENTIFIER:
			s = va_arg(args, char *);
			break;
		default:
			break;
	}

	switch (err) {

		/* Add additional cases here as is necessary, referring to
		 * errmsg.h for all possible errors.  Some errors only become possible
		 * to recognise once we add type checking.  Until you get to type
		 * checking, you can handle such errors by adding the default case.
		 * However, your final submission *must* handle all cases explicitly.
		 */

		case ERR_EXPECT:
			t = va_arg(args, int);
			leprintf(expstr, get_token_string(t));
			break;

		case ERR_FACTOR_EXPECTED:
			leprintf(expstr, "factor");
			break;

		case ERR_UNREACHABLE:
			leprintf("unreachable: %s", s);
			break;

		case ERR_STATEMENT_EXPECTED:
			leprintf("expected statement, but found %s",
			get_token_string(token.type));
			break;

		case ERR_ARRAY_ALLOCATION_OR_EXPRESSION_EXPECTED:
			leprintf("expected expression or string, but found %s",
			get_token_string(token.type));
			break;

		case ERR_TYPE_EXPECTED:
			leprintf("expected type, but found %s", get_token_string(token.type));
			break;

			case ERR_EXPRESSION_OR_STRING_EXPECTED:
			leprintf("expected expression or string, but found %s",
			get_token_string(token.type));
			break;

		default:
			break;
	}
}

/* --- debugging output routines -------------------------------------------- */

#ifdef DEBUG_PARSER

static int indent = 0;

void debug_start(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	debug_info(fmt, ap);
	va_end(ap);
	indent += 2;
}

void debug_end(const char *fmt, ...)
{
	va_list ap;

	indent -= 2;
	va_start(ap, fmt);
	debug_info(fmt, ap);
	va_end(ap);
}

void debug_info(const char *fmt, ...)
{
	int i;
	char buf[MAX_MESSAGE_LENGTH], *buf_ptr;
	va_list ap;

	buf_ptr = buf;

	va_start(ap, fmt);

	for (i = 0; i < indent; i++) {
		*buf_ptr++ = ' ';
	}
	vsprintf(buf_ptr, fmt, ap);

	buf_ptr += strlen(buf_ptr);
	snprintf(buf_ptr, MAX_MESSAGE_LENGTH, " in line %d.\n", position.line);
	fflush(stdout);
	fputs(buf, stdout);
	fflush(NULL);

	va_end(ap);
}

#endif /* DEBUG_PARSER */
