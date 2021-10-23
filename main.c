#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "util.h"
#include "atom.h"
#include "mem.h"
#include "debug.h"
#include "list.h"
#include "hashtable.h"

#define STR_LEN 1000000

void apply(void **x, void *cl) {
    printf("value: %s\r\n", (char *) *x);
}

void tableApply(const void *key, void **value, void *cl) {
    printf("key: %s, value: %s\r\n", (char *) key, (char *) (*value));
}

//(1):寻找Hashtable hash计算函数 http://www.cse.yorku.ca/~oz/hash.html
//(2):Atom_new中不使用strcmp原因 strcmp只能比较字符串，但是atom实现也可以不是字符串
//(3): atom结构中,char str[1] 少引用指针,用柔性数组? c99中加入了柔性数组,可以采用柔性数组实现
//(4):在atom结构中存储hash值，Atom_new中比较两个atom是否一样，可以比较hash值，这样是否可行？不同的两个字符串str1,str2,其hash值,hash(str1)和hash(str2)不一定一样
//(5):修改Atom_length实现，优化，和Atom_new尽量一样
//(6):添加Atom_init(int hint),对ht做初始化，对hint做入参数检查
//(7):实现Atom_free可以释放单个原子，并且实现Atom_reset可以释放所有原子
//(8):实现Atom_vload(const char* str, ...)批量初始化原子，实现Atom_aload(const char* strs[])数组初始化原子
//(9):实现Atom_add(const char* str, int len)浅拷贝str,并且调整free,reset
int main(int argc, const char *argv[]) {
#ifdef DEBUG_ATOM
    char **strBuffer;
    AtomMgr atom;

    debugInit(&argc, argv);
    Random_init();
    atom = Atom_init(0);

    strBuffer = ALLOC(STR_LEN * sizeof(char *));

    for (int i = 0; i < STR_LEN; ++i) {
        char *str = Random_stringRetain();
        strBuffer[i] = str;
    }

    time_t startTime = time(NULL);
    for (int i = 0; i < STR_LEN; ++i) {
        Atom_new(atom, strBuffer[i], strlen(strBuffer[i]));
    }
    time_t endTime = time(NULL);
    printf("Atom_new Escape %ld\r\n", endTime - startTime);

    for (int i = 0; i < STR_LEN; ++i) {
        Random_free(strBuffer[i]);
    }

    int length;
    const char *pVal;

    Atom_vload(atom, "aaa", "bbb");
    const char* vt[] = {"a1", "a2", NULL};
    Atom_aload(atom, vt);
    pVal = Atom_new(atom, "abc", strlen("abc"));
    length = Atom_length(atom, pVal);
    printf("Atom_length: %d\r\n", length);
    char *tips = "hello,world";
    Atom_add(atom, tips, strlen(tips));
    Atom_free(atom, tips);
    FREE(strBuffer);
    Atom_reset(atom);
#endif

#ifdef DEBUG_LIST
    int length;

    ListMgr listMgr = List_init();
    List_push(listMgr, "hello1");
    List_push(listMgr, "world1");
    List_push(listMgr, "apple1");
    length = List_length(listMgr);
    printf("listMgr: %d\r\n", length);

    ListMgr listMgr1 = List_list("hello2", "world2", "people2", NULL);
    length = List_length(listMgr1);
    printf("listMgr1: %d\r\n", length);

    listMgr = List_append(listMgr, listMgr1);
    length = List_length(listMgr);
    printf("after List_append, listMgr: %d\r\n", length);

    List_map(listMgr, apply, NULL);
    listMgr = List_reverse(listMgr);
    printf("after List_reverse: \r\n");
    List_map(listMgr, apply, NULL);

    printf("after List_copy: \r\n");
    ListMgr listMgr2 = List_copy(listMgr);
    List_map(listMgr2, apply, NULL);

    char *popValue;
    List_pop(listMgr2, (void **) &popValue);
    printf("pop value: %s\r\n", popValue);
    List_map(listMgr2, apply, NULL);

    char **arrayValue;
    char *p = NULL;

    arrayValue = (char **) List_toArray(listMgr2, NULL);
    printf("after List_toArray: \r\n");
    int i = 0;
    p = arrayValue[i];
    while (p) {
        printf("value: %s\r\n", p);
        i++;
        p = arrayValue[i];
    }

    List_pop(listMgr, (void **) &popValue);
    List_pop(listMgr, (void **) &popValue);
    List_pop(listMgr, (void **) &popValue);
    List_pop(listMgr, (void **) &popValue);
    List_pop(listMgr, (void **) &popValue);
    List_pop(listMgr, (void **) &popValue);
    printf("length of listMgr: %d\r\n", List_length(listMgr));
    assert(listMgr->head == NULL);
    assert(listMgr->tail == NULL);

    List_free(listMgr);
    List_free(listMgr1);
    List_free(listMgr2);
    FREE(arrayValue);
#endif

    HashTable_T t = HT_create(100, NULL, NULL);
    HT_put(t, "key1", "value1");
    HT_put(t, "key2", "value2");
    HT_map(t, tableApply, NULL);
    void *value = HT_get(t, "key2");
    printf("HT_get: %s\r\n", (char *) value);
    HT_put(t, "key3", "value3");
    HT_put(t, "key4", "value4");
    HT_put(t, "key5", "value5");
    HT_put(t, "key6", "value6");
    HT_put(t, "key7", "value7");
    HT_remove(t, "key2");
    HT_remove(t, "key6");
    HT_map(t, tableApply, NULL);
    int length = HT_length(t);
    printf("HT_length: %d\r\n", length);
    void **array = HT_toArray(t, NULL);
    printf("after HT_toArray: \r\n");
    int i = 0;

    while (array[i]) {
        char *key = array[i++];
        char *val = (char *)array[i++];
        printf("key: %s, value: %s\r\n", key, val);
    }

    HT_remove(t, "key7");
    HT_remove(t, "key1");
    HT_remove(t, "key4");
    HT_remove(t, "key4");
    HT_put(t, "key5", "value(3+2)");
    printf("HT_mapByOrder\r\n");
    HT_mapByOrder(t, tableApply, NULL);
    HT_free(&t);
    return 0;
}
