/**
 * @file    symboltable.c
 * @brief   A symbol table for ALAN-2022.
 * @author  W. H. K. Bester (whkbester@cs.sun.ac.za)
 * @date    2022-08-03
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "boolean.h"
#include "error.h"
#include "hashtable.h"
#include "symboltable.h"
#include "token.h"

/* --- global static variables ---------------------------------------------- */

static HashTab *table, *saved_table;
/* Nothing here, but note that the next variable keeps a running count of
 * the number of variables in the current symbol table.  It will be necessary
 * during code generation to compute the size of the local variable array of a
 * method frame in the Java virtual machine.
 */
static unsigned int curr_offset;

/* --- function prototypes -------------------------------------------------- */

static void valstr(void *key, void *p, char *str);
static void freeprop(void *p);
static unsigned int shift_hash(void *key, unsigned int size);
static int key_strcmp(void *val1, void *val2);
static unsigned power(unsigned int num, unsigned int exp);

/* --- symbol table interface ----------------------------------------------- */

void init_symbol_table(void)
{
	saved_table = NULL;
	if ((table = ht_init(0.75f, shift_hash, key_strcmp)) == NULL) {
		eprintf("Symbol table could not be initialised");
	}
	curr_offset = 1;
}

Boolean open_subroutine(char *id, IDprop *prop)
{
	/* - Insert the subroutine name into the global symbol table; return TRUE or
	 *   FALSE, depending on whether or not the insertion succeeded.
	 * - Save the global symbol table to saved_table, initialise a new hash
	 *   table for the subroutine, and reset the current offset.
	 */

	Boolean b;

	b = insert_name(id, prop);

	if (b) {
		saved_table = table;
		table = ht_init(0.75f, shift_hash, key_strcmp);
		curr_offset = 0;
	}

	return b;
}

void close_subroutine(void)
{
	/* Release the subroutine table, and reactivate the global table. */

		ht_free(table, free, freeprop);
		table = saved_table;

}

Boolean insert_name(char *id, IDprop *prop)
{
	/* Insert the properties of the identifier into the hash table, and
	 * remember to increment the current offset pointer if the identifier is a
	 * variable.
	 *
	 * VERY IMPORTANT: Remember to read the documentation of this function in
	 * the header file.
	 */
	Boolean b;

	if (!find_name(id, &prop)) {
		ht_insert(table, id, prop);
		if (IS_VARIABLE(prop->type)) {
			curr_offset++;
		}

		b = TRUE;

	} else {
		b = FALSE;
	}

	return b;

}

Boolean find_name(char *id, IDprop **prop)
{
	Boolean found;

	/* Nothing, unless you want to.*/
	found = ht_search(table, id, (void **) prop);
	if (!found && saved_table) {
		found = ht_search(saved_table, id, (void **) prop);
		if (found && !IS_CALLABLE_TYPE((*prop)->type)) {
			found = FALSE;
		}
	}

	return found;
}

int get_variables_width(void)
{
	return curr_offset;
}

void release_symbol_table(void)
{
	/* Free the underlying structures of the symbol table. */

	ht_free(table, free, freeprop);
}

void print_symbol_table(void)
{
	ht_print(table, valstr);
}

/* --- utility functions ---------------------------------------------------- */

static void valstr(void *key, void *p, char *str)
{
	char *keystr = (char *) key;
	IDprop *idpp = (IDprop *) p;

	/* Nothing, but this should give you an idea of how to look at the
	 * contents of the symbol table.
	 */

	sprintf(str, "%s.%d", keystr, idpp->offset);
	sprintf(str, "%s@%d[%s]", keystr, idpp->offset,
			get_valtype_string(idpp->type));
}

/* Here you should add your own utility functions, in particular, for
 * deallocation, hashing, and key comparison.  For hashing, you MUST NOT use the
 * simply strategy of summing the integer values of characters.  I suggest you
 * use some kind of cyclic bit shift hash.
 */

static unsigned int shift_hash(void *key, unsigned int size)
{
	unsigned int hash = 0;
	char *keystring = (char *) key;
	size_t len = strlen(keystring);
	unsigned int i, j;
	unsigned cnst = 39;
	for (i = 0; keystring[i] != '\0'; i++) {
		for (j = len; j > 0; j--) {
			hash += keystring[i] * power(cnst + 1, j + 1);
		}
	}
	return hash % size;
}

static unsigned power(unsigned int num, unsigned int exp)
{
	unsigned int newnum = num;
	unsigned int i;
	for (i = 0; i < exp; i++) {
		newnum *= num;

	}
	return newnum;
}

static int key_strcmp(void *val1, void *val2)
{
	char *value1 = (char *) val1;
	char *value2 = (char *) val2;
	int cmp = strcmp(value1, value2);

	return cmp;
}

static void freeprop(void *p)
{
	free(p);

}
