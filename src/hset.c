//
// Created by nbboy on 2021/10/25.
//

#include <string.h>
#include <assert.h>
#include <limits.h>
#include "arith.h"
#include "mem.h"
#include "hset.h"

#define HSET HSET_T

struct hSet {
    int length;
    int size;
    int timestamp;

    unsigned (*hash)(const void *member);

    int (*cmp)(const void *member1, const void *member2);

    struct binding {
        struct binding *link;
        const void *member;
        unsigned hashcode;
    } **buckets;
};

static int tableCmp(const void *key1, const void *key2) {
    return strcmp(key1, key2);
}

static unsigned tableHash(const void *key) {
    return (unsigned long) key >> 2;
}

HSET HSet_new(int hint,
              int cmp(const void *x, const void *y),
              unsigned hash(const void *x)) {
    HSET set;
    int i;
    static int primes[] = {509, 509, 1021, 2053, 4093,
                           8191, 16381, 32771, 65521, INT_MAX};
    assert(hint >= 0);
    for (i = 1; primes[i] < hint; i++);
    set = ALLOC(sizeof(*set) +
                primes[i - 1] * sizeof(set->buckets[0]));
    set->size = primes[i - 1];
    set->cmp = cmp ? cmp : tableCmp;
    set->hash = hash ? hash : tableHash;
    set->buckets = (struct binding **) (set + 1);
    for (i = 0; i < set->size; i++)
        set->buckets[i] = NULL;
    set->length = 0;
    set->timestamp = 0;
    return set;
}

void HSet_free(HSET *set) {
    assert(set && *set);
    if ((*set)->length > 0) {
        int i;
        struct binding *p, *q;
        for (i = 0; i < (*set)->size; i++)
            for (p = (*set)->buckets[i]; p; p = q) {
                q = p->link;
                FREE(p);
            }
    }
    FREE(*set);
}

int HSet_length(HSET set) {
    return set->length;
}

int HSet_member(HSET set, const void *member) {
    int i;
    unsigned hash;
    struct binding *p;
    assert(set);
    assert(member);
    hash = (*set->hash)(member);
    i = hash % set->size;
    for (p = set->buckets[i]; p; p = p->link)
        if (p->hashcode == hash && (*set->cmp)(member, p->member) == 0)
            break;
    return p != NULL;
}

void HSet_put(HSET set, const void *member) {
    int i;
    struct binding *p;
    unsigned hash;
    assert(set);
    assert(member);

    hash = (*set->hash)(member);
    i = hash % set->size;
    for (p = set->buckets[i]; p; p = p->link)
        if (p->hashcode == hash && (*set->cmp)(member, p->member) == 0)
            break;
    if (p == NULL) {
        NEW(p);
        p->member = member;
        p->link = set->buckets[i];
        set->buckets[i] = p;
        set->length++;
    } else
        p->member = member;
    set->timestamp++;
}

void *HSet_remove(HSET set, const void *member) {
    int i;
    struct binding **pp;
    unsigned hash;

    assert(set);
    assert(member);
    set->timestamp++;
    hash = (*set->hash)(member);
    i = hash % set->size;
    for (pp = &set->buckets[i]; *pp; pp = &(*pp)->link)
        if ((*pp)->hashcode == hash && (*set->cmp)(member, (*pp)->member) == 0) {
            struct binding *p = *pp;
            *pp = p->link;
            member = p->member;
            FREE(p);
            set->length--;
            return (void *) member;
        }
    return NULL;
}

void HSet_map(HSET set,
              void apply(const void *member, void *cl), void *cl) {
    int i;
    unsigned stamp;
    struct binding *p;
    assert(set);
    assert(apply);
    stamp = set->timestamp;
    for (i = 0; i < set->size; i++)
        for (p = set->buckets[i]; p; p = p->link) {
            apply(p->member, cl);
            assert(set->timestamp == stamp);
        }
}

void **HSet_toArray(HSET set, void *end) {
    int i, j = 0;
    void **array;
    struct binding *p;
    assert(set);
    array = ALLOC((set->length + 1) * sizeof(*array));
    for (i = 0; i < set->size; i++)
        for (p = set->buckets[i]; p; p = p->link)
            array[j++] = (void *) p->member;
    array[j] = end;
    return array;
}

static HSET copy(HSET t, int hint) {
    HSET set = HSet_new(hint, t->cmp, t->hash);
    int i;
    unsigned h;
    struct binding *p;

    for (i = 0; i < t->size; i++) {
        for (p = t->buckets[i]; p; p = p->link) {
            h = (*set->hash)(p->member) % set->size;
            struct binding *q;
            NEW(q);
            q->member = p->member;
            q->link = set->buckets[h];
            set->buckets[h] = q;
            set->length++;
        }
    }
    return set;
}

HSET HSet_union(HSET s, HSET t) {
    if (s == NULL) {
        return copy(t, t->size);
    } else if (t == NULL) {
        return copy(s, s->size);
    } else {
        HSET set = copy(s, Arith_max(s->size, t->size));
        assert(s->cmp == t->cmp && s->hash == t->hash);
        int i;
        struct binding *p;
        for (i = 0; i < t->size; i++) {
            for (p = t->buckets[i]; p; p = p->link) {
                HSet_put(set, p->member);
            }
        }
        return set;
    }
}

HSET HSet_inter(HSET s, HSET t) {
    if (s == NULL) {
        return HSet_new(t->size, t->cmp, t->hash);
    } else if (t == NULL) {
        return HSet_new(s->size, s->cmp, s->hash);
    } else if (s->length < t->length) {
        return HSet_inter(t, s);
    } else {
        HSET set = HSet_new(Arith_min(s->size, t->size), s->cmp, s->hash);
        int i;
        struct binding *p;
        unsigned h;
        for (i = 0; i < s->size; i++) {
            for (p = s->buckets[i]; p; p = p->link) {
                if (HSet_member(t, p->member)) {
                    h = (*set->hash)(p->member) % set->size;
                    struct binding *q;
                    NEW(q);
                    q->member = p->member;
                    q->link = set->buckets[h];
                    set->buckets[h] = q;
                    set->length++;
                }
            }
        }
        return set;
    }
}

HSET HSet_minus(HSET s, HSET t) {
    if (s == NULL) {
        return HSet_new(t->size, t->cmp, t->hash);
    } else if (t == NULL) {
        return copy(s, s->size);
    } else {
        HSET set = HSet_new(Arith_min(s->size, t->size), s->cmp, s->hash);
        int i;
        struct binding *p;
        unsigned h;
        for (i = 0; i < s->size; i++) {
            for (p = s->buckets[i]; p; p = p->link) {
                if (!HSet_member(t, p->member)) {
                    h = (*set->hash)(p->member) % set->size;
                    struct binding *q;
                    NEW(q);
                    q->member = p->member;
                    q->link = set->buckets[h];
                    set->buckets[h] = q;
                    set->length++;
                }
            }
        }
        return set;
    }
}

HSET HSet_diff(HSET s, HSET t) {
    if (s == NULL) {
        return copy(t, t->size);
    } else if (t == NULL) {
        return copy(s, s->size);
    } else {
        HSET set = HSet_new(Arith_min(s->size, t->size), s->cmp, s->hash);
        int i;
        struct binding *p;
        unsigned h;

        //diff(s, t) = minus(s, t) + minus(t, s)
        //在S中，不在T中的元素
        for (i = 0; i < s->size; i++) {
            for (p = s->buckets[i]; p; p = p->link) {
                if (!HSet_member(t, p->member)) {
                    h = (*set->hash)(p->member) % set->size;
                    struct binding *q;
                    NEW(q);
                    q->member = p->member;
                    q->link = set->buckets[h];
                    set->buckets[h] = q;
                    set->length++;
                }
            }
        }

        //在T中, 不在S中的元素
        {
            HSET tmp;
            tmp = s, s = t, t = tmp;
        }
        for (i = 0; i < s->size; i++) {
            for (p = s->buckets[i]; p; p = p->link) {
                if (!HSet_member(t, p->member)) {
                    h = (*set->hash)(p->member) % set->size;
                    struct binding *q;
                    NEW(q);
                    q->member = p->member;
                    q->link = set->buckets[h];
                    set->buckets[h] = q;
                    set->length++;
                }
            }
        }
        return set;
    }
}

