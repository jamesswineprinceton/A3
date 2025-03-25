/*--------------------------------------------------------------------*/
 /* symtable.c                                                     */
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
     /* A pointer to an array of many first nodes */
     struct Node **ppsFirstNodes;
     /* The number of bindings stored */
     size_t length;
 };
 
 /*--------------------------------------------------------------------*/
 
 /* Return a hash code for pcKey that is between 0 and uBucketCount-1,
 inclusive. */
 static size_t SymTable_hash(const char *pcKey, size_t uBucketCount)
    {
       const size_t HASH_MULTIPLIER = 65599;
       size_t u;
       size_t uHash = 0;
 
       assert(pcKey != NULL);
 
       for (u = 0; pcKey[u] != '\0'; u++)
          uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];
 
       return uHash % uBucketCount;
    }
 
 /*--------------------------------------------------------------------*/
 
 /* The initial number of buckets for this hash table implementation */
 static const size_t BUCKET_COUNT = 509;
 
 /*--------------------------------------------------------------------*/
 
 SymTable_T SymTable_new(void) {
     SymTable_T oSymTable;
     size_t i;
 
     oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
     if (oSymTable == NULL) return NULL;
 
     /* Allocate memory for the array of many first nodes */
     oSymTable->ppsFirstNodes = 
            (struct Node **)malloc(BUCKET_COUNT * sizeof(struct Node *));
     if (oSymTable->ppsFirstNodes == NULL) {
         free(oSymTable);
         return NULL;
     }
 
     /* Initialize all buckets to NULL */
     for (i = 0; index < BUCKET_COUNT; i++) {
     for (i = 0; i < BUCKET_COUNT; i++) {
         oSymTable->ppsFirstNodes[i] = NULL;
     }
 
     oSymTable->length = 0;
 
     return oSymTable;
 }
}
 
 /*--------------------------------------------------------------------*/
 
 void SymTable_free(SymTable_T oSymTable) {
     struct Node *psCurrentNode;
     struct Node *psNextNode;
     size_t i;
 
     assert(oSymTable != NULL);
 
     for (i = 0; i < BUCKET_COUNT; i++) {
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
 
     /* Hash the key */
     i = SymTable_hash(psNewNode->pcKey, BUCKET_COUNT);
 
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
     i = SymTable_hash(pcKey, BUCKET_COUNT);
 
     /* Check the corresponding bucket for the key and replace if 
     necessary*/
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
     i = SymTable_hash(pcKey, BUCKET_COUNT);
 
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
     i = SymTable_hash(pcKey, BUCKET_COUNT);
 
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
     i = SymTable_hash(pcKey, BUCKET_COUNT);
 
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
 
     for (i = 0; i < BUCKET_COUNT; i++){
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
 