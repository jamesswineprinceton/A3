/*--------------------------------------------------------------------*/
/* symtablehash.c                                                     */
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
    char *pcKey;

    /* The associated data */
    const void *pvValue;

    /* The address of the next node */
    struct Node *psNextNode;
};

/*--------------------------------------------------------------------*/

/* A SymTable is a "dummy" node that points to the first node. */

struct SymTable {
    /* A pointer to an array of many first nodes (buckets) */
    struct Node **ppsFirstNodes;
    /* The number of buckets pointed to */
    size_t bucketCount;
    /* The number of bindings stored */
    size_t length;
};

/*--------------------------------------------------------------------*/

/* Return a hash code for pcKey that is between 0 and uBucketCount-1,
inclusive. */
static size_t SymTable_hash(const char *pcKey, size_t uBucketCount) {
      const size_t HASH_MULTIPLIER = 65599;
      size_t u;
      size_t uHash = 0;
   
      assert(pcKey != NULL);
   
      for (u = 0; pcKey[u] != '\0'; u++)
         uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];
   
      return uHash % uBucketCount;
}

/*--------------------------------------------------------------------*/

/* Dynamically increases the number of buckets and repositions all 
bindings whenever a call of SymTable_put causes the number of bindings 
to become too large */
static void SymTable_expand(SymTable_T oSymTable, 
                            size_t newBucketCount) {
    struct Node **ppsOldFirstNodes = NULL;
    size_t oldBucketCount;
    struct Node **ppsNewFirstNodes;
    struct Node *psCurrentNode;
    struct Node *psNextNode;
    size_t i;
    size_t newHash;

    assert(oSymTable != NULL);

    ppsOldFirstNodes = oSymTable->ppsFirstNodes;
    oldBucketCount = oSymTable->bucketCount;

    /* Allocate memory for the new array of many first nodes */
    ppsNewFirstNodes = 
    (struct Node**)malloc(newBucketCount * sizeof(struct Node*));
    if (ppsNewFirstNodes == NULL) {
        return;
    }

    /* Initialize all buckets to NULL */
    for (i = 0; i < newBucketCount; i++) {
        ppsNewFirstNodes[i] = NULL;
    }

    /* Hash all old bindings into new buckets */
    for (i = 0; i < oldBucketCount; i++) {
        for (psCurrentNode = ppsOldFirstNodes[i];
             psCurrentNode != NULL;
             psCurrentNode = psNextNode) {
        psNextNode = psCurrentNode->psNextNode;
        newHash = SymTable_hash(psCurrentNode->pcKey, newBucketCount);
        psCurrentNode->psNextNode = ppsNewFirstNodes[newHash];
        ppsNewFirstNodes[newHash] = psCurrentNode;
        }
    }

    /* Update the symbol table with the expanded hash table */
    oSymTable->ppsFirstNodes = ppsNewFirstNodes;
    oSymTable->bucketCount = newBucketCount;

    /* Free the memory in which the old array of many first nodes 
    resides */
    free(ppsOldFirstNodes);

    return;
}

/*--------------------------------------------------------------------*/

SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;
    size_t initialBucketCount = 509;
    size_t i;

    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
    if (oSymTable == NULL) return NULL;

    oSymTable->ppsFirstNodes = 
    (struct Node**)malloc(initialBucketCount * sizeof(struct Node*));
    if (oSymTable->ppsFirstNodes == NULL) {
        free(oSymTable);
        return NULL;
    }
    
    /* Initialize all buckets to NULL */
    for (i = 0; i < initialBucketCount; i++) {
        oSymTable->ppsFirstNodes[i] = NULL;
    }

    oSymTable->length = 0;

    return oSymTable;
}

/*--------------------------------------------------------------------*/

void SymTable_free(SymTable_T oSymTable) {
    struct Node *psCurrentNode;
    struct Node *psNextNode;
    size_t i;

    assert(oSymTable != NULL);

    /* Iterate through the buckets */
    for (i = 0; i < oSymTable->bucketCount; i++) {
        /* Walk through the list at each bucket */
        for (psCurrentNode = oSymTable->ppsFirstNodes[i];
             psCurrentNode != NULL;
             psCurrentNode = psNextNode) {
          psNextNode = psCurrentNode->psNextNode;
          /* Free the memory in which the keys reside*/
          free((char*)psCurrentNode->pcKey);
          free(psCurrentNode);
        }
    }

    /* Free the memory in which the array of many first nodes resides */
    free(oSymTable->ppsFirstNodes);
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
    size_t i;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Expand hash table if necessary */
    if (oSymTable->length == 509) SymTable_expand(oSymTable, 1021);
    if (oSymTable->length == 1021) SymTable_expand(oSymTable, 2039);
    if (oSymTable->length == 2039) SymTable_expand(oSymTable, 4093);    
    if (oSymTable->length == 4093) SymTable_expand(oSymTable, 8191);
    if (oSymTable->length == 8191) SymTable_expand(oSymTable, 16381);
    if (oSymTable->length == 16381) SymTable_expand(oSymTable, 32749);
    if (oSymTable->length == 32749) SymTable_expand(oSymTable, 65521);
        
    /* Check if oSymTable already contains pcKey */
    if (SymTable_contains(oSymTable, pcKey)) return 0;
    
    /* Allocate memory for the new node */
    psNewNode = (struct Node*)malloc(sizeof(struct Node));
    if (psNewNode == NULL) return 0;

    /* Create a defensive copy of the string to which pcKey points */
    psNewNode->pcKey = (char*)malloc(strlen(pcKey) + 1);
    if (psNewNode->pcKey == NULL) {
    free(psNewNode);
    return 0;
    } 
    strcpy(psNewNode->pcKey, pcKey);

    /* Hash the key */
    i = SymTable_hash(psNewNode->pcKey, oSymTable->bucketCount);

    psNewNode->pvValue = pvValue;
    /* Put the node in the appropriate bucket */
    psNewNode->psNextNode = oSymTable->ppsFirstNodes[i];
    oSymTable->ppsFirstNodes[i] = psNewNode;

    oSymTable->length++;
    
    return 1;
}

/*--------------------------------------------------------------------*/

void *SymTable_replace(SymTable_T oSymTable, 
                       const char *pcKey, 
                       const void *pvValue) {
    struct Node *psCurrentNode;
    struct Node *psNextNode;
    size_t i;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Hash the key */
    i = SymTable_hash(pcKey, oSymTable->bucketCount);

    /* Check the corresponding bucket for the key and replace if 
    found */
    for (psCurrentNode = oSymTable->ppsFirstNodes[i];
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
    size_t i;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Hash the key */
    i = SymTable_hash(pcKey, oSymTable->bucketCount);

    /* Check the corresponding bucket for the key */
    for (psCurrentNode = oSymTable->ppsFirstNodes[i];
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
    size_t i;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Hash the key */
    i = SymTable_hash(pcKey, oSymTable->bucketCount);

    /* Check the corresponding bucket for the key and return its 
    value */
    for (psCurrentNode = oSymTable->ppsFirstNodes[i];
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
    size_t i;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Hash the key */
    i = SymTable_hash(pcKey, oSymTable->bucketCount);

    /* Check the corresponding bucket for the key, remove it, and return
    its value */
    for (psCurrentNode = oSymTable->ppsFirstNodes[i];
         psCurrentNode != NULL;
         psCurrentNode = psNextNode) {
      psNextNode = psCurrentNode->psNextNode;
      if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
            void *value = (void*)psCurrentNode->pvValue;

            if (psPrevNode == NULL) {
                oSymTable->ppsFirstNodes[i] = psNextNode;
            }
            else psPrevNode->psNextNode = psNextNode;

            /* Free the memory in which the key resides */
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
    size_t i;

    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    /* Iterate through the buckets */
    for (i = 0; i < oSymTable->bucketCount; i++){
        /* Walk through the list at each bucket */
        for (psCurrentNode = oSymTable->ppsFirstNodes[i];
             psCurrentNode != NULL;
             psCurrentNode = psNextNode) {
         psNextNode = psCurrentNode->psNextNode;

         (*pfApply)(psCurrentNode->pcKey,
                    (void*)psCurrentNode->pvValue,
                    (void*)pvExtra);
        }
   }

   return;
}
    
    
