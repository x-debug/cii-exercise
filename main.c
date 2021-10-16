#include <stdio.h>
#include <string.h>
#include <time.h>
#include "util.h"
#include "atom.h"
#include "mem.h"
#include "debug.h"

#define STR_LEN 1000000

//(1):寻找Hashtable hash计算函数 http://www.cse.yorku.ca/~oz/hash.html
//(2):Atom_new中不使用strcmp原因 strcmp只能比较字符串，但是atom实现也可以不是字符串
//(3): atom结构中,char str[1] 少引用指针,用柔性数组? c99中加入了柔性数组,可以采用柔性数组实现
//(4):在atom结构中存储hash值，Atom_new中比较两个atom是否一样，可以比较hash值，这样是否可行？不同的两个字符串str1,str2,其hash值,hash(str1)和hash(str2)不一定一样
//(5):修改Atom_length实现，优化，和Atom_new尽量一样
//(6):添加Atom_init(int hint),对ht做初始化，对hint做入参数检查
//(7):实现Atom_free可以释放单个原子，并且实现Atom_reset可以释放所有原子
//(8):实现Atom_vload(const char* str, ...)批量初始化原子，实现Atom_aload(const char* strs[])数组初始化原子
//(9):实现Atom_add(const char* str, int len)浅拷贝str,并且调整free,reset
int main(int argc, const char* argv[]) {
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
    return 0;
}
