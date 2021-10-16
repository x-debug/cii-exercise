//
// Created by nbboy on 2021/10/15.
//

#ifndef CII_UTIL_H
#define CII_UTIL_H

void Random_init();
//You must free it on call Random_free
char *Random_stringRetain();
void Random_free(char *str);
#endif //CII_UTIL_H
