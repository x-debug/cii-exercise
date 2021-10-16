//
// Created by nbboy on 2021/10/15.
//
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"

static char *asciiTable = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void Random_init() {
    srand(time(NULL));
}

char *Random_stringRetain() {
    unsigned char ascii;
    int len, i;
    char *str;

    len = rand() % 20 + 1;
    str = ALLOC(len * sizeof(char) + 1);
    for (i = 0; i < len; ++i) {
        ascii = rand() % strlen(asciiTable);
        str[i] = asciiTable[ascii];
    }
    str[i] = '\0';
    return str;
}

void Random_free(char *str) {
    if (str != NULL) {
        FREE(str);
    }
}