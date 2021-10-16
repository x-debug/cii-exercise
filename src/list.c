static char rcsid[] = "$Id: H:/drh/idioms/book/RCS/list.doc,v 1.11 1997/02/21 19:46:01 drh Exp $";

#include <stdarg.h>
#include <stddef.h>
#include "assert.h"
#include "mem.h"
#include "list.h"

#define LIST_DATA(mgr) ((listDataPtr)(mgr)->data)

struct listNode {
    struct listNode *rest;
    void *first;
};

struct listData {
    int count;
};

typedef struct listData *listDataPtr;

ListMgr List_init() {
    ListMgr mgr;
    mgr = ALLOC(sizeof(_ListMgr) + sizeof(struct listData));
    LIST_DATA(mgr)->count = 0;
    mgr->head = mgr->tail = NULL;
    return mgr;
}

//把节点放入链表
ListMgr List_push(ListMgr self, void *x) {
    assert(self);

    listNodePtr p;
    NEW(p);
    p->first = x;
    p->rest = self->head;//append to head
    if (!self->head) {
        self->tail = p;
    }
    self->head = p;
    LIST_DATA(self)->count += 1;
    return self;
}

//批量创建链表节点
ListMgr List_list(void *x, ...) {
    va_list ap;
    ListMgr mgr;

    mgr = List_init();
    va_start(ap, x);
    for (; x; x = va_arg(ap, void *)) {
        List_push(mgr, x);
    }

    va_end(ap);
    return mgr;
}

//对两个链表做加法
ListMgr List_append(ListMgr self, ListMgr other) {
    assert(self);
    assert(other);

    if (!self->head) {
        return other;
    }

    listNodePtr p = self->head, prev = NULL;
    for (; p; prev = p, p = p->rest) {}
    prev->rest = other->head;
    self->tail = other->tail;
    LIST_DATA(self)->count += LIST_DATA(other)->count;
    other->head = NULL;
    other->tail = NULL;
    LIST_DATA(other)->count = 0;
    return self;
}

//拷贝整个链表，挨个复制整个链表
ListMgr List_copy(ListMgr self) {
    assert(self);

    ListMgr mgr;
    listNodePtr list = self->head;
    mgr = List_init();

    for (; list; list = list->rest) {
        List_push(mgr, list->first);
    }

    List_reverse(mgr);
    return mgr;
}

//弹出第一个节点，并且返回下一个节点地址
ListMgr List_pop(ListMgr self, void **x) {
    assert(self);

    listNodePtr list = self->head;

    if (list) {
        listNodePtr head = list->rest;
        if (x)
            *x = list->first;
        FREE(list);
        self->head = head;
        if (!head) {
            self->tail = head;
        }
        LIST_DATA(self)->count -= 1;
        return self;
    } else
        return self;
}

//反转链表，关键是要记录之前的节点
ListMgr List_reverse(ListMgr self) {
    assert(self);

    listNodePtr head = NULL, next;
    listNodePtr list = self->head, tail = list;

    for (; list; list = next) {
        next = list->rest;
        list->rest = head;
        head = list;
    }

    self->head = head;
    self->tail = tail;
    return self;
}

//可以采用增加字段进行优化
int List_length(ListMgr self) {
//    assert(self);
//
//    int n;
//
//    listNodePtr list = self->head;
//    for (n = 0; list; list = list->rest)
//        n++;
//    return n;
    return LIST_DATA(self)->count;
}

//遍历，并且释放
void List_free(ListMgr self) {
    assert(self);

    listNodePtr next;
    listNodePtr *list = &self->head;

    for (; *list; *list = next) {
        next = (*list)->rest;
        FREE(*list);
    }

    FREE(self);
}

//迭代，并且回调用户函数
void List_map(ListMgr self,
              void apply(void **x, void *cl), void *cl) {
    assert(self);
    assert(apply);

    listNodePtr list = self->head;

    for (; list; list = list->rest)
        apply(&list->first, cl);
}

//调用者去释放Array
void **List_toArray(ListMgr self, void *end) {
    assert(self);

    int i, n = List_length(self);
    void **array = ALLOC((n + 1) * sizeof(*array));
    listNodePtr list = self->head;

    for (i = 0; i < n; i++) {
        array[i] = list->first;
        list = list->rest;
    }
    array[i] = end;//结束位置
    return array;
}
