/*--------------------------------------------------------------------*/
/* symtablelist.c                                                     */
/* Author: James Swinehart                                            */
/*--------------------------------------------------------------------*/

#include "symtable.h"

struct Node {
    const char *key;
    void *value;
    struct Node *next;
};

struct SymTable_T {
    struct Node *first;
};

SymTable_T SymTable_new(void) {
    struct SymTable_T *s;
    s = calloc(1, sizeof(*s)); 
    if (s == NULL) return NULL;
    return s;
}