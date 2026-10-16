/* bsearch.c - binary search. */
#include "rush/rush_protos.h"

/* 0x80005e50 */
void *bsearch(void *key, void *base, unsigned long nmemb, unsigned long size, int (*compar)(void *, void *))
{
    void *current;
    int result;
    void *limit;

    if (nmemb == 0 || size == 0)
        return NULL;
    limit = (char *)base + nmemb * size;
    while (nmemb) {
        current = (char *)base + (nmemb >> 1) * size;
        result = compar(key, current);
        if (result < 0) {
            nmemb >>= 1;
        } else if (result > 0) {
            base = (char *)current + size;
            nmemb = (nmemb >> 1) - ((nmemb & 1) ^ 1);
        } else {
            return current;
        }
    }
    if (base < limit && compar(key, base) == 0)
        return base;
    return NULL;
}
