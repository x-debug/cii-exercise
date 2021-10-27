//
// Created by nbboy on 2021/10/25.
//

#ifndef CII_HSET_H
#define CII_HSET_H

#define HSET HSET_T
typedef struct hSet *HSET;

HSET HSet_new(int hint,
             int cmp(const void *x, const void *y),
             unsigned hash(const void *x));

void HSet_free(HSET *set);

int HSet_length(HSET set);

int HSet_member(HSET set, const void *member);

void HSet_put(HSET set, const void *member);

void *HSet_remove(HSET set, const void *member);

void HSet_map(HSET set,
             void apply(const void *member, void *cl), void *cl);

void **HSet_toArray(HSET set, void *end);

HSET HSet_union(HSET s, HSET t);

HSET HSet_inter(HSET s, HSET t);

HSET HSet_minus(HSET s, HSET t);

HSET HSet_diff(HSET s, HSET t);

#undef HSET
#endif //CII_HSET_H
