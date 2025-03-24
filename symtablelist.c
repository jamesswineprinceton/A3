/*--------------------------------------------------------------------*/
/* symtablelist.c                                                     */
/* Author: James Swinehart                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

/*--------------------------------------------------------------------*/

/* Each binding is stored in a node. Nodes are linked to form a list. */

struct Node {
    /* The identifying key */
    const char *pcKey;

    /* The associated data */
    const void *pvValue;

    /* The address of the next node */
    struct Node *psNextNode;
};

/*--------------------------------------------------------------------*/

/* A SymTable is a "dummy" node that points to the first node. */

struct SymTable {
    /* The address of the first node */
    struct Node *psFirstNode;
    /* The number of nodes */
    size_t length;
};

/*--------------------------------------------------------------------*/

SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;

    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
    if (oSymTable == NULL) return NULL;

    oSymTable->psFirstNode = NULL;
    oSymTable->length = 0;

    return oSymTable;
}

/*--------------------------------------------------------------------*/

void SymTable_free(SymTable_T oSymTable) {
    struct Node *psCurrentNode;
    struct Node *psNextNode;

    assert(oSymTable != NULL);

    for (psCurrentNode = oSymTable->psFirstNode;
         psCurrentNode != NULL;
         psCurrentNode = psNextNode) {
      psNextNode = psCurrentNode->psNextNode;
      /* Free the memory in which the keys reside*/
      free((char*)psCurrentNode->pcKey);
      free(psCurrentNode);
    }

    free(oSymTable);
}

/*--------------------------------------------------------------------*/

size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);

    return oSymTable->length;
}

/*--------------------------------------------------------------------*/

int SymTable_put(SymTable_T oSymTable, 
                 const char *pcKey, 
                 const void *pvValue) {          
   struct Node *psNewNode;

   assert(oSymTable != NULL);
   assert(pcKey != NULL);

   /* Check if oSymTable already contains pcKey */
   if (SymTable_contains(oSymTable, pcKey)) return 0;
   
   psNewNode = (struct Node*)malloc(sizeof(struct Node));
   if (psNewNode == NULL) return 0;

   /* Create a defensive copy of the string to which pcKey points */
   psNewNode->pcKey = (char*)malloc(strlen(pcKey) + 1);
   if (psNewNode->pcKey == NULL) {
    free(psNewNode);
    return 0;
   }
   strcpy(psNewNode->pcKey, pcKey);

   psNewNode->pvValue = pvValue;
   psNewNode->psNextNode = oSymTable->psFirstNode;

   oSymTable->psFirstNode = psNewNode;
   oSymTable->length++;

   return 1;
}

/*--------------------------------------------------------------------*/

void *SymTable_replace(SymTable_T oSymTable, 
                       const char *pcKey, 
                       const void *pvValue) {
    struct Node *psCurrentNode;
    struct Node *psNextNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    for (psCurrentNode = oSymTable->psFirstNode;
         psCurrentNode != NULL;
         psCurrentNode = psNextNode) {
      psNextNode = psCurrentNode->psNextNode;
      if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
            void *oldValue = (void*)psCurrentNode->pvValue;
            psCurrentNode->pvValue = pvValue;
            return oldValue;
        }
    }

    return NULL;
}

/*--------------------------------------------------------------------*/

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    struct Node *psCurrentNode;
    struct Node *psNextNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    for (psCurrentNode = oSymTable->psFirstNode;
         psCurrentNode != NULL;
         psCurrentNode = psNextNode) {
      psNextNode = psCurrentNode->psNextNode;
      if (strcmp(psCurrentNode->pcKey, pcKey) == 0) return 1;
    }

    return 0;
}

/*--------------------------------------------------------------------*/

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    struct Node *psCurrentNode;
    struct Node *psNextNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    for (psCurrentNode = oSymTable->psFirstNode;
         psCurrentNode != NULL;
         psCurrentNode = psNextNode) {
      psNextNode = psCurrentNode->psNextNode;
      if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
            return (void*)psCurrentNode->pvValue;
        }
    }

    return NULL;
}

/*--------------------------------------------------------------------*/

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    struct Node *psCurrentNode;
    struct Node *psNextNode;
    struct Node *psPrevNode = NULL;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    for (psCurrentNode = oSymTable->psFirstNode;
         psCurrentNode != NULL;
         psCurrentNode = psNextNode) {
      psNextNode = psCurrentNode->psNextNode;
      if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
            void *value = (void*)psCurrentNode->pvValue;

            if (psPrevNode == NULL) oSymTable->psFirstNode = psNextNode;
            else psPrevNode->psNextNode = psNextNode;

            free((char*)psCurrentNode->pcKey);
            free(psCurrentNode);
            oSymTable->length--;

            return value;
        }
      psPrevNode = psCurrentNode;
    }

    return NULL;
}

/*--------------------------------------------------------------------*/

void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra) {     
    struct Node *psCurrentNode;
    struct Node *psNextNode;

    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    for (psCurrentNode = oSymTable->psFirstNode;
         psCurrentNode != NULL;
         psCurrentNode = psNextNode) {
      psNextNode = psCurrentNode->psNextNode;
      (*pfApply)(psCurrentNode->pcKey,
                 (void*)psCurrentNode->pvValue,
                 (void*)pvExtra);
    }

    return;
}