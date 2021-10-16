/* $Id: H:/drh/idioms/book/RCS/list.doc,v 1.11 1997/02/21 19:46:01 drh Exp $ */
#ifndef LIST_INCLUDED
#define LIST_INCLUDED

typedef struct listNode *listNodePtr;

typedef struct {
    listNodePtr head;
    listNodePtr tail;
    char data[];
}_ListMgr;

typedef _ListMgr *ListMgr;

extern ListMgr List_init();
extern ListMgr     List_list   (void *x, ...);
extern ListMgr     List_append (ListMgr self, ListMgr other);
extern ListMgr     List_copy   (ListMgr self);
extern ListMgr     List_pop    (ListMgr self, void **x);
extern ListMgr     List_push   (ListMgr self, void *x);
extern ListMgr     List_reverse(ListMgr self);
extern int    List_length (ListMgr self);
extern void   List_free   (ListMgr self);
extern void   List_map    (ListMgr self,
	void apply(void **x, void *cl), void *cl);
extern void **List_toArray(ListMgr self, void *end);
#undef T
#endif
