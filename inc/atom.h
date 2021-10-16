/* $Id: H:/drh/idioms/book/RCS/atom.doc,v 1.10 1997/02/21 19:42:46 drh Exp $ */
#ifndef ATOM_INCLUDED
#define ATOM_INCLUDED

typedef struct atom *atomPtr;

typedef struct {
    atomPtr *elements;
    int count;
}_AtomMgr;

typedef _AtomMgr *AtomMgr;

extern AtomMgr Atom_init(int hint);
extern void Atom_free(AtomMgr self, const char *str);
extern void Atom_reset(AtomMgr self);
extern       int   Atom_length(AtomMgr self, const char *str);
extern const char *Atom_new   (AtomMgr self, const char *str, int len);
extern const char *Atom_string(AtomMgr self, const char *str);
extern const char *Atom_int   (AtomMgr self, long n);
extern void Atom_vload(AtomMgr self, const char* str, ...);
extern void Atom_aload(AtomMgr self, const char* strs[]);
extern const char * Atom_add(AtomMgr self, const char* str, int len);
#endif
