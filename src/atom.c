static char rcsid[] = "$Id: H:/drh/idioms/book/RCS/atom.doc,v 1.10 1997/02/21 19:42:46 drh Exp $";

#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <stdbool.h>
#include "assert.h"
#include "atom.h"
#include "mem.h"
#include "debug.h"

#define DEFAULT_OF_NUMBER 2048

//计算数组内元素个数
#define NELEMS(x) ((sizeof (x))/(sizeof ((x)[0])))

//typedef struct atomPtr atomPtr;
//使用散列表实现,对冲突的元素使用链表维护
struct atom {
    struct atom *link;
    unsigned char freed;
    bool copied;//shallow = 1, set ref to ptr, else duplicate string to str
    const char *ptr;
    int len;
    char str[];//内存分配时，存储的具体内容在其管理结构后面分配，此处指向具体的字符串内容
};

static unsigned long scatter[] = {
        2078917053, 143302914, 1027100827, 1953210302, 755253631, 2002600785,
        1405390230, 45248011, 1099951567, 433832350, 2018585307, 438263339,
        813528929, 1703199216, 618906479, 573714703, 766270699, 275680090,
        1510320440, 1583583926, 1723401032, 1965443329, 1098183682, 1636505764,
        980071615, 1011597961, 643279273, 1315461275, 157584038, 1069844923,
        471560540, 89017443, 1213147837, 1498661368, 2042227746, 1968401469,
        1353778505, 1300134328, 2013649480, 306246424, 1733966678, 1884751139,
        744509763, 400011959, 1440466707, 1363416242, 973726663, 59253759,
        1639096332, 336563455, 1642837685, 1215013716, 154523136, 593537720,
        704035832, 1134594751, 1605135681, 1347315106, 302572379, 1762719719,
        269676381, 774132919, 1851737163, 1482824219, 125310639, 1746481261,
        1303742040, 1479089144, 899131941, 1169907872, 1785335569, 485614972,
        907175364, 382361684, 885626931, 200158423, 1745777927, 1859353594,
        259412182, 1237390611, 48433401, 1902249868, 304920680, 202956538,
        348303940, 1008956512, 1337551289, 1953439621, 208787970, 1640123668,
        1568675693, 478464352, 266772940, 1272929208, 1961288571, 392083579,
        871926821, 1117546963, 1871172724, 1771058762, 139971187, 1509024645,
        109190086, 1047146551, 1891386329, 994817018, 1247304975, 1489680608,
        706686964, 1506717157, 579587572, 755120366, 1261483377, 884508252,
        958076904, 1609787317, 1893464764, 148144545, 1415743291, 2102252735,
        1788268214, 836935336, 433233439, 2055041154, 2109864544, 247038362,
        299641085, 834307717, 1364585325, 23330161, 457882831, 1504556512,
        1532354806, 567072918, 404219416, 1276257488, 1561889936, 1651524391,
        618454448, 121093252, 1010757900, 1198042020, 876213618, 124757630,
        2082550272, 1834290522, 1734544947, 1828531389, 1982435068, 1002804590,
        1783300476, 1623219634, 1839739926, 69050267, 1530777140, 1802120822,
        316088629, 1830418225, 488944891, 1680673954, 1853748387, 946827723,
        1037746818, 1238619545, 1513900641, 1441966234, 367393385, 928306929,
        946006977, 985847834, 1049400181, 1956764878, 36406206, 1925613800,
        2081522508, 2118956479, 1612420674, 1668583807, 1800004220, 1447372094,
        523904750, 1435821048, 923108080, 216161028, 1504871315, 306401572,
        2018281851, 1820959944, 2136819798, 359743094, 1354150250, 1843084537,
        1306570817, 244413420, 934220434, 672987810, 1686379655, 1301613820,
        1601294739, 484902984, 139978006, 503211273, 294184214, 176384212,
        281341425, 228223074, 147857043, 1893762099, 1896806882, 1947861263,
        1193650546, 273227984, 1236198663, 2116758626, 489389012, 593586330,
        275676551, 360187215, 267062626, 265012701, 719930310, 1621212876,
        2108097238, 2026501127, 1865626297, 894834024, 552005290, 1404522304,
        48964196, 5816381, 1889425288, 188942202, 509027654, 36125855,
        365326415, 790369079, 264348929, 513183458, 536647531, 13672163,
        313561074, 1730298077, 286900147, 1549759737, 1699573055, 776289160,
        2143346068, 1975249606, 1136476375, 262925046, 92778659, 1856406685,
        1884137923, 53392249, 1735424165, 1602280572
};

AtomMgr Atom_init(int hint) {
    assert(hint >= 0);

    AtomMgr self;
    self = ALLOC(sizeof(_AtomMgr));

    if (hint == 0) {
        self->count = DEFAULT_OF_NUMBER;
    } else {
        self->count = hint;
    }

    self->elements = ALLOC(DEFAULT_OF_NUMBER * sizeof(struct atom));
    return self;
}

void Atom_free(AtomMgr self, const char *str) {
    atomPtr p;
    int i;

    assert(self);
    assert(str);
    for (i = 0; i < self->count; i++)
        for (p = self->elements[i]; p; p = p->link)
            if (p->str == str || p->ptr == str) {
                vDebug("free: %d", p->copied);
                p->freed = 1;
            }
}

void Atom_reset(AtomMgr self) {
    atomPtr p;
    atomPtr next;
    int i;

    assert(self);
    for (i = 0; i < self->count; i++)
        for (p = self->elements[i]; p; p = next) {
            next = p->link;
            FREE(p);
        }
    FREE(self);
}

const char *Atom_string(AtomMgr self, const char *str) {
    assert(self);
    assert(str);
    return Atom_new(self, str, strlen(str));
}

//把整数n转换成字符串后创建atom
const char *Atom_int(AtomMgr self, long n) {
    char str[43];
    char *s = str + sizeof str;
    unsigned long m;

    assert(self);
    if (n == LONG_MIN)
        m = LONG_MAX + 1UL;
    else if (n < 0)
        m = -n;
    else
        m = n;
    do
        *--s = m % 10 + '0';
    while ((m /= 10) > 0);
    if (n < 0)
        *--s = '-';
    return Atom_new(self, s, (str + sizeof str) - s);
}

static unsigned long
hash(const char *str) {
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

static const char *new(AtomMgr self, const char *str, int len, bool shouldCopy) {
    unsigned long h;
    int i;
    struct atom *p;
    char *ptr;

    assert(self);
    assert(str);
    assert(len >= 0);

    //对字符串hash函数计算
    h = hash(str);
    h %= self->count;
    for (p = self->elements[h]; p; p = p->link)//根据hash函数获得hash值后然后计算一个下标，在遍历这个hash槽(链表)
        if (len == p->len) {//长度和内容都一样，则返回之前的atom
            for (i = 0; i < len && p->str[i] == str[i];)
                i++;
            if (i == len && !p->freed)
                return p->str;
        }

    for (p = self->elements[h]; p; p = p->link) {
        //reuse deleted element for reduce alloc times
        if (p->freed == 1 && p->copied == shouldCopy) {
            if (shouldCopy && p->len >= len) {
                strncpy(p->str, str, len);//deep copy
                p->str[len] = '\0';
                p->len = len;
                p->freed = 0;
                return p->str;
            }

            if (!shouldCopy) {
                debug("ref copy");
                p->ptr = str;
                p->len = len;
                p->freed = 0;
                return p->str;
            }
        }
    }

    if (shouldCopy) {
        //否则，就进行分配，分配容量为头部管理结构 + 实际的内容值，这种方法在C语言中经常用到，也可以用结构体中的柔性数组实现
        p = ALLOC(sizeof(*p) + len + 1);
        //对原来内容做深度拷贝，这样原来内容调用者可以释放也不影响
        if (len > 0)
            memcpy(p->str, str, len);
        p->str[len] = '\0';
    } else {
        p = ALLOC(sizeof(*p));
        p->ptr = str;
    }

    p->copied = shouldCopy;
    p->len = len;
    //对把元素插入单链表的头部节点，并且把槽的指针也指向该节点
    p->link = self->elements[h];
    p->freed = 0;
    self->elements[h] = p;
    return p->str;
}

const char *Atom_new(AtomMgr self, const char *str, int len) {
    return new(self, str, len, true);
}

//在所有atom里找该字符串，找到则返回长度
int Atom_length(AtomMgr self, const char *str) {
    unsigned long h;
    atomPtr p;

    assert(self);
    assert(str);
    h = hash(str);
    h = h % self->count;
    for (p = self->elements[h]; p; p = p->link)
        if ((p->str == str || p->ptr == str) && !p->freed)//因为atom是全局唯一的，所以可以认为指针相同，则内容也一定相同
            return p->len;
    assert(0);
    return 0;
}

void Atom_vload(AtomMgr self, const char *str, ...) {
    va_list arguments;
    const char *ptr;

    Atom_string(self, str);
    va_start(arguments, str);
    for (; ptr = va_arg(arguments, const char *), ptr != NULL;) {
        Atom_string(self, ptr);
    }
    va_end(arguments);
}

void Atom_aload(AtomMgr self, const char *strs[]) {
    int i;
    for (i = 0; strs[i] != NULL; i++) {
        Atom_string(self, strs[i]);
    }
    vDebug("%d", i);
}

const char *Atom_add(AtomMgr self, const char *str, int len) {
    return new(self, str, len, false);
}
