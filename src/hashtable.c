//
// Created by nbboy on 2021/10/20.
//
#include <limits.h>
#include <assert.h>
#include <string.h>
#include "hashtable.h"
#include "mem.h"

#define HashTable HashTable_T

struct hashTable {
    int length;

    int size;

    int timestamp;

    int (*cmp)(const void *, const void *);

    unsigned (*hash)(const void *);

    struct linked {
        struct linked *next, *prev;
        struct binding *ele;
    } *head, *tail;

    struct binding {
        struct binding *link;
        const void *key;
        void *value;
        struct linked *node;
    } **buckets;
};

static int tableCmp(const void *key1, const void *key2) {
    return strcmp(key1, key2);
}

static unsigned tableHash(const void *key) {
    return (unsigned long) key >> 2;
}

HashTable HT_create(int hint, int (*cmp)(const void *, const void *), unsigned (*hash)(const void *)) {
    HashTable table;
    int size;
    int i;
    static int primes[] = {509, 509, 1021, 2053, 4093,
                           8191, 16381, 32771, 65521, INT_MAX};

    for (i = 1; primes[i] < hint; ++i);
    size = primes[i - 1];

    table = ALLOC(sizeof(*table) + size * sizeof(table->buckets[0]));
    table->buckets = (struct binding **) (table + 1);
    for (int j = 0; j < size; ++j) {
        table->buckets[j] = NULL;
    }
    table->cmp = cmp != NULL ? cmp : tableCmp;
    table->hash = hash != NULL ? hash : tableHash;
    table->size = size;
    table->length = 0;
    table->timestamp = 0;
    NEW(table->head);//sentinel node
    NEW(table->tail);
    table->head->prev = NULL;
    table->head->next = table->tail;
    table->tail->next = NULL;
    table->tail->prev = table->head;
    return table;
}

static void removeLinked(HashTable table, struct linked *ele) {
    struct linked *p;

    assert(ele);
    assert(table->head);

    for (p = table->head->next; p != table->tail; p = p->next) {
        if (p == ele) {
            p->prev->next = p->next;
            p->next->prev = p->prev;
            break;
        }
    }
}

static void appendHead(HashTable table, struct linked *ele) {
    ele->next = table->head->next;
    table->head->next->prev = ele;
    table->head->next = ele;
    ele->prev = table->head;
}

void *HT_put(HashTable ht, const void *key, void *value) {
    unsigned h;
    struct binding *p;
    struct linked *l;
    void *val;

    h = (*ht->hash)(key) % ht->size;
    for (p = ht->buckets[h]; p; p = p->link) {
        if ((*ht->cmp)(p->key, key) == 0) {
            break;
        }
    }

    if (p == NULL) {//new element
        NEW(p);
        p->key = key;
        p->value = value;
        ht->length++;
        p->link = ht->buckets[h];
        ht->buckets[h] = p;
        val = NULL;
        NEW(l);
        p->node = l;
        l->ele = p;
        appendHead(ht, l);
    } else {//
        val = p->value;
        p->key = key;
        p->value = value;
        removeLinked(ht, p->node);
        appendHead(ht, p->node);
    }

    ht->timestamp++;
    return val;
}

void *HT_get(HashTable ht, const void *key) {
    unsigned h;
    struct binding *p;

    h = (*ht->hash)(key) % ht->size;
    for (p = ht->buckets[h]; p; p = p->link) {
        if ((*ht->cmp)(p->key, key) == 0) {
            return p->value;
        }
    }

    return NULL;
}

void HT_map(HashTable ht, void (*apply)(const void *key, void **value, void *cl), void *cl) {
    struct binding *p;
    int i;
    int timestamp;

    timestamp = ht->timestamp;
    for (i = 0; i < ht->size; i++) {
        for (p = ht->buckets[i]; p; p = p->link) {
            (*apply)(p->key, &p->value, cl);
            assert(timestamp == ht->timestamp);
        }
    }
}

void HT_mapByOrder(HashTable ht, void (*apply)(const void *key, void **value, void *cl), void *cl) {
    struct linked *p;
    int timestamp;

    timestamp = ht->timestamp;
    for (p = ht->tail->prev; p != ht->head; p = p->prev) {
        (*apply)(p->ele->key, &(p->ele->value), cl);
        assert(timestamp == ht->timestamp);
    }
}

void *HT_remove(HashTable ht, const void *key) {
    struct binding **pp, *p;
    unsigned h;
    void *val;

    h = (*ht->hash)(key) % ht->size;
    val = NULL;

    ht->timestamp++;
    for (pp = &ht->buckets[h]; (*pp); pp = &(*pp)->link) {
        p = (*pp);
        if ((*ht->cmp)(p->key, key) == 0) {
            (*pp) = p->link;
            val = p->value;
            removeLinked(ht, p->node);
            ht->length--;
            FREE(p);
            break;
        }
    }

    return val;
}

int HT_length(HashTable ht) {
    return ht->length;
}

void **HT_toArray(HashTable ht, void *end) {
    void **array;
    struct binding *p;
    int i, j = 0;

    array = ALLOC((2 * ht->length + 1) * sizeof(void *));
    for (i = 0; i < ht->size; i++) {
        for (p = ht->buckets[i]; p; p = p->link) {
            array[j++] = (void *) p->key;
            array[j++] = p->value;
        }
    }

    array[j] = end;
    return array;
}

void **HT_toArrayByOrder(HashTable ht, void *end) {
    void **array;
    struct linked *p;
    int i;

    array = ALLOC((2 * ht->length + 1) * sizeof(void *));
    for (i = 0, p = ht->tail->prev; p!=ht->head; p = p->prev) {
        array[i++] = (void *) p->ele->key;
        array[i++] = p->ele->value;
    }

    array[i] = NULL;
    return array;
}

void HT_free(HashTable *ht) {
    struct binding *p, *q;
    unsigned size;
    int i;

    if ((*ht)->length > 0) {
        size = (*ht)->size;
        for (i = 0; i < size; i++) {
            for (p = (*ht)->buckets[i]; p; p = q) {
                q = p->link;
                FREE(p);
            }
        }
    }

    FREE((*ht));
}
