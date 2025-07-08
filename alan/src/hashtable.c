/**
 * @file    hashtable.c
 * @brief   A generic hash table.
 * @author  W.H.K. Bester (whkbester@cs.sun.ac.za)
 * @date    2022-08-03
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

#define INITIAL_DELTA_INDEX  4
#define PRINT_BUFFER_SIZE 1024

/** an entry in the hash table */
typedef struct htentry HTentry;
struct htentry {
	void    *key;       /*<< the key                      */
	void    *value;     /*<< the value                    */
	HTentry *next_ptr;  /*<< the next entry in the bucket */
};

/** a hash table container */
struct hashtab {
	/** a pointer to the underlying table                              */
	HTentry **table;
	/** the current size of the underlying table                       */
	unsigned int size;
	/** the current number of entries                                  */
	unsigned int num_entries;
	/** the maximum load factor before the underlying table is resized */
	float max_loadfactor;
	/** the index into the delta array                                 */
	unsigned short idx;
	/** a pointer to the hash function                                 */
	unsigned int (*hash)(void *, unsigned int);
	/** a pointer to the comparison function                           */
	int (*cmp)(void *, void *);
};

/* --- function prototypes -------------------------------------------------- */

/* For the following functions, refer to the note at the end of the
 * file.
 */

static int getsize(HashTab *ht);
//static HTentry **talloc(int tsize);
static void rehash(HashTab *ht);
static int power(int num, int exponent);

/* For this implementation, we want to ensure we *always* have a hash
 * table that is of prime size.  To that end, the next array stores the
 * different between a power of two and the largest prime less than that
 * particular power-of-two.  When you rehash, compute the new prime size using
 * the following array.
 */

/** the array of differences between a power-of-two and the largest prime less
 * than that power-of-two.                                                    */
unsigned short delta[] = { 0, 0, 1, 1, 3, 1, 3, 1, 5, 3, 3, 9, 3, 1, 3, 19, 15,
1, 5, 1, 3, 9, 3, 15, 3, 39, 5, 39, 57, 3, 35, 1 };

#define MAX_IDX (sizeof(delta) / sizeof(unsigned short))

/* --- hash table interface ------------------------------------------------- */

HashTab *ht_init(float loadfactor,
				 unsigned int (*hash)(void *, unsigned int),
				 int (*cmp)(void *, void *))
{
	HashTab *ht = (HashTab*) malloc(sizeof(HashTab));
	unsigned int i;

	/* - Initialise a hash table structure by calling an allocation function
	 *   twice:
	 *   (1) once to allocate space for a HashTab variable, and
	 *   (2) once to allocate space for the table field of this new HashTab
	 *       variable.
	 * - If any allocation fails, free anything that has already been allocated
	 *   successfully, and return NULL.
	 * - Also set up the other fields of the newly-allocated HashTab structure
	 *   appropriately.
	 */

	i = 13;
	ht->size = i;
	ht->table = (HTentry**) calloc(ht->size, sizeof(HTentry*));
	if (ht == NULL) {
		free(ht->table);
		return NULL;
	}
	if (ht->table == NULL) {
		free(ht);
	}

	ht->idx = INITIAL_DELTA_INDEX;
	ht->cmp = cmp;
	ht->hash = hash;
	ht-> max_loadfactor = loadfactor;
	ht->num_entries = 1;

	return ht;
}

int ht_insert(HashTab *ht, void *key, void *value)
{
	int k;

	/* Insert a new key--value pair, rehashing if necessary.  The best way
	 * to go about rehashing is to put the necessary elements into a static
	 * function called rehash.  Remember to free space (the "old" table) you not
	 * use any longer.  Also, if something goes wrong, use the #define'd
	 * constants in hashtable.h for return values; remember, unless it runs out
	 * of memory, no operation on a hash table may terminate the program.
	 */

	float nentries = ht->num_entries;
	float tsize = ht->size;
	float currload = nentries/tsize;
	float loadf = ht->max_loadfactor;



		if (currload >=loadf) {
			rehash(ht);
		}

		if (!ht_search(ht, key, value)) {
			k = ht->hash(key, ht->size);

			if (ht->table[k] == NULL) {
				HTentry *p;
				p = (HTentry*) malloc(sizeof(HTentry));
				p->value = value;
				p->key = key;
				ht->table[k] = p;
				ht->num_entries++;

			} else {
				HTentry *next = ht->table[k];
				HTentry *prev = NULL;

				while (next != NULL) {
					prev = next;
					next = next->next_ptr;
				}
				HTentry *p;
				p = (HTentry*) malloc(sizeof(HTentry));
				p->key = key;
				p->value = value;
				ht->num_entries++;
				prev->next_ptr = p;
				p->next_ptr = NULL;
			}

		} else {
			return HASH_TABLE_KEY_VALUE_PAIR_EXISTS;

		}

	return EXIT_SUCCESS;
}

Boolean ht_search(HashTab *ht, void *key, void **value)
{
	int k;
	HTentry *p;

	/* Nothing!  This function is complete, and should explain by example
	 * how the hash table looks and must be accessed.
	 */

	k = ht->hash(key, ht->size);
	for (p = ht->table[k]; p; p = p->next_ptr) {
		if (ht->cmp(key, p->key) == 0) {
			*value = p->value;
			break;
		}
	}
	return (p ? TRUE : FALSE);
}

int ht_free(HashTab *ht, void (*freekey)(void *k), void (*freeval)(void *v))
{
	unsigned int i;
	HTentry *p;

	/* free the nodes in the buckets */
	/* free the table and container */

	for (i = 0; i < ht->size; i++) {
		for (p = ht->table[i]; p != NULL; p = p->next_ptr) {
			freekey(p->key);
			freeval(p->value);
			free(p);
		}
	}
	free(ht->table);
	free(ht);
	return EXIT_SUCCESS;
}

void ht_print(HashTab *ht, void (*keyval2str)(void *k, void *v, char *b))
{
	unsigned int i;
	HTentry *p;
	char buffer[PRINT_BUFFER_SIZE];

	/* This function is complete and useful for testing, but you have to
	 * write your own keyval2str function if you want to use it.
	 */

	for (i = 0; i < ht->size; i++) {
		printf("bucket[%2i]", i);
		for (p = ht->table[i]; p != NULL; p = p->next_ptr) {
			keyval2str(p->key, p->value, buffer);
			printf(" --> %s", buffer);
		}
		printf(" --> NULL\n");
	}
}

/* --- utility functions ---------------------------------------------------- */

/* I suggest completing the following helper functions for use in the
 * global functions ("exported" as part of this unit's public API) given above.
 * You may, however, elect not to use them, and then go about it in your own way
 * entirely.  The ball is in your court, so to speak, but remember: I have
 * suggested using these functions for a reason -- they should make your life
 * easier.
 */

static int getsize(HashTab *ht)
{
	/* Compute the next prime size of the hash table. */

	int size = MAX_IDX;
	int next_prime = 0;
	int i;
	int curr_prime = ht->size;

	if (curr_prime < 13) {
		return 13;
	}

	for (i = INITIAL_DELTA_INDEX; i < size; i++) {
		if (curr_prime < power(2, i)) {
			next_prime = power(2, i + 1) - delta[i + 1];
			i = size;
		}
	}
	ht->idx++;
	return next_prime;
}

static void rehash(HashTab *ht)
{
	/* Rehash the hash table by
	 * (1) allocating a new table that uses as size the next prime in the
	 *     "almost-double" array,
	 * (2) moving the entries in the existing table to appropriate positions in
	 *     the new table, and
	 * (3) freeing the old table.
	 */

	HTentry *p;

	HashTab *newht = (HashTab*) malloc(sizeof(HashTab));
	int newsize = getsize(ht);
	newht->size = newsize;
	newht->table = (HTentry**) calloc(newht->size, sizeof(HTentry*));

	newht->idx = ht->idx;
	newht->cmp = ht->cmp;
	newht->hash = ht->hash;
	newht->max_loadfactor = ht->max_loadfactor;
	newht->num_entries = ht->num_entries;

	unsigned int i;
	for (i = 0; i < ht->size; i++) {
		for (p = ht->table[i]; p; p = p->next_ptr) {
				ht_insert(newht, p->key, p->value);
		}
	}
	ht->table = newht->table;
	ht->size = newht->size;
}

static int power(int num, int exponent)
{
	int new_num = num;
	int i;
	for (i = 0; i < exponent - 1; i++) {
		new_num *= num;
	}
	return new_num;
}

