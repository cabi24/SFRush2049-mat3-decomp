/* statll.c - fixed-size linked-list pools carved out of a static array (singly or doubly linked). */
#include "rush/rush_protos.h"

/* 0x80043108 */
void InitStatLL(tStatList *list, void *array, int elementSize, int numElements, char dbl)
{
    list->list = array;
    list->numElements = numElements;
    list->elementSize = elementSize;
    list->dbl = dbl;
    ClearStatLL(list);
}

/* 0x80043138 */
void ClearStatLL(tStatList *list)
{
    char *array;
    int i;

    array = list->list;
    list->store = array;
    list->head = NULL;
    if (array == NULL)
        return;
    for (i = 0; i < list->numElements - 1; i++)
        *(char **)(array + i * list->elementSize) = array + (i + 1) * list->elementSize;
    *(char **)(array + i * list->elementSize) = NULL;
}

/* 0x80043198 */
void *GetStatLLE(tStatList *list)
{
    tStatLLE *el = list->store;

    if (el) {
        list->store = el->next;
        el->next = list->head;
        if (list->dbl) {
            ((tDLLE *)el)->prev = NULL;
            if (list->head)
                ((tDLLE *)list->head)->prev = (tDLLE *)el;
        }
        list->head = el;
    }
    return el;
}

/* 0x800431e4 */
void FreeStatLLE(tStatList *list, void *element)
{
    tStatLLE *el;
    tStatLLE *prev;
    tStatLLE *cur;

    if (!element)
        return;
    el = element;
    if (list->dbl) {
        if (el->next)
            ((tDLLE *)el->next)->prev = ((tDLLE *)el)->prev;
        if (((tDLLE *)el)->prev)
            ((tStatLLE *)((tDLLE *)el)->prev)->next = el->next;
        else
            list->head = el->next;
    } else {
        prev = NULL;
        cur = list->head;
        while (cur) {
            if (el == cur) {
                if (prev)
                    prev->next = el->next;
                else
                    list->head = el->next;
                break;
            }
            prev = cur;
            cur = cur->next;
        }
    }
    el->next = list->store;
    list->store = el;
}
