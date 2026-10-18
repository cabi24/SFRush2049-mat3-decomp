/* ll.c - linked lists of caller-owned elements: singly or doubly linked (dbl), and either the
   elements themselves or handles (pointers to elements) are linked (handle). */
#include "rush/rush_protos.h"

/* 0x80025338 */
void InitLL(tDLList *list, char handle, char dbl)
{
    list->handle = handle;
    list->dbl = dbl;
    ClearLL(list);
}

/* 0x80025360 */
void ClearLL(tDLList *list)
{
    list->head = NULL;
    list->tail = NULL;
    list->numElements = 0;
}

/* 0x80025374 */
void InsertLLE(tDLList *list, void *insert, void *before)
{
    tDLLE *in;
    tDLLE *direct;
    tDLLE *prev;
    tDLLE **prevh;
    tDLLE *b4;

    if (!insert)
        return;
    if (list->handle) {
        in = *(tDLLE **)insert;
        b4 = before ? *(tDLLE **)before : NULL;
    } else {
        in = insert;
        b4 = before;
    }
    if (before) {
        if (list->dbl) {
            prevh = b4->prev;
            if (prevh) {
                if (list->handle) {
                    prev = *prevh;
                    prev->next = insert;
                } else {
                    ((tDLLE *)prevh)->next = insert;
                }
            } else {
                list->head = insert;
            }
            in->prev = b4->prev;
            b4->prev = insert;
        } else {
            prevh = list->head;
            if ((void *)prevh == before) {
                list->head = insert;
            } else if (list->handle) {
                while (prevh) {
                    prev = *prevh;
                    if (prev->next == before)
                        prev->next = insert;
                    prevh = (*prevh)->next;
                }
            } else {
                direct = (tDLLE *)prevh;
                while (direct) {
                    if (direct->next == before)
                        direct->next = insert;
                    direct = direct->next;
                }
            }
        }
    } else {
        if (list->dbl)
            in->prev = list->tail;
        if (list->tail) {
            if (list->handle)
                (*(tDLLE **)list->tail)->next = insert;
            else
                ((tDLLE *)list->tail)->next = insert;
        } else {
            list->head = insert;
        }
        list->tail = insert;
    }
    in->next = before;
    list->numElements++;
}

/* 0x800254c8 */
void RemoveLLE(tDLList *list, void *element)
{
    tDLLE *prev;
    tDLLE **prevh;
    tDLLE *el;

    if (!element)
        return;
    if (list->handle)
        el = *(tDLLE **)element;
    else
        el = element;
    if (list->dbl) {
        if (el->next) {
            if (list->handle)
                (*(tDLLE **)el->next)->prev = el->prev;
            else
                ((tDLLE *)el->next)->prev = el->prev;
        } else {
            list->tail = el->prev;
        }
        if (el->prev) {
            if (list->handle)
                (*(tDLLE **)el->prev)->next = el->next;
            else
                ((tDLLE *)el->prev)->next = el->next;
        } else {
            list->head = el->next;
        }
    } else {
        prevh = list->head;
        if (element == (void *)prevh) {
            list->head = el->next;
            if (list->head == NULL)
                list->tail = NULL;
        } else if (list->handle) {
            while (prevh) {
                prev = *prevh;
                if (prev->next == element) {
                    prev->next = el->next;
                    if (el->next == NULL)
                        list->tail = prevh;
                    break;
                }
                prevh = prev->next;
            }
        } else {
            prev = (tDLLE *)prevh;
            while (prev) {
                if (prev->next == element) {
                    prev->next = el->next;
                    if (el->next == NULL)
                        list->tail = prev;
                    break;
                }
                prev = prev->next;
            }
        }
    }
    list->numElements--;
}
