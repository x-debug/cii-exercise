//
// Created by nbboy on 2021/10/20.
//

#ifndef CII_HASHTABLE_H
#define CII_HASHTABLE_H

#define HashTable HashTable_T

typedef struct hashTable *HashTable;

HashTable HT_create(int hint, int (*cmp)(const void *, const void *), unsigned (*hash)(const void *));
void HT_print(HashTable ht);
void *HT_put(HashTable ht, const void *key, void *value);
void *HT_get(HashTable ht, const void *key);
void HT_map(HashTable ht, void (*apply)(const void *key, void **value, void *cl), void *cl);
void HT_mapByOrder(HashTable ht, void (*apply)(const void *key, void **value, void *cl), void *cl);
void HT_free(HashTable *ht);
void *HT_remove(HashTable ht, const void *key);
int HT_length(HashTable ht);
void **HT_toArray(HashTable ht, void *end);
void **HT_toArrayByOrder(HashTable ht, void *end);

#undef HashTable
#endif //CII_HASHTABLE_H
