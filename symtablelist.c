/*--------------------------------------------------------------------*/
/* symtablelist.c                                                     */
/* Author: James Swinehart                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdlib.h>
#include "symtable.h"

/*--------------------------------------------------------------------*/

/* Each binding is stored in a node. Nodes are linked to form a list. */

struct Node {
    /* The identifying key */
    const char *pcKey;

    /* The associated data */
    const void *pvValue;

    /* The address of the next node */
    struct Node *psNext;
};

/*--------------------------------------------------------------------*/

/* A SymTable is a "dummy" node that points to the first node. */

struct SymTable {
    /* The address of the first node */
    struct Node *psFirst;
};

/*--------------------------------------------------------------------*/

SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;

    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
    if (oSymTable = NULL) return NULL;

    oSymTable->psFirst = NULL;
    return oSymTable;
}

/*--------------------------------------------------------------------*/

