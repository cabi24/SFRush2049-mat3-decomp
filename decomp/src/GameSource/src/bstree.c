/* bstree.c - binary search tree of caller-owned nodes (left/right links at the start of each node). */
#include "rush/rush_protos.h"

typedef struct BSTNode {
    void *left;
    void *right;
} BSTNode;

static void _InsertBSTE(tOrderedList *tree, void *root, void *element);
static void _WalkBST(tOrderedList *tree, void *root, int *num, void (*WalkFunc)(void *, int));

/* 0x8009ccd0 */
void InitBST(tOrderedList *tree, int (*OrderFunc)(void *, void *))
{
    ClearBST(tree);
    tree->OrderFunc = OrderFunc;
}

/* 0x8009cd0c */
void ClearBST(tOrderedList *tree)
{
    tree->root = NULL;
    tree->numElements = 0;
}

/* 0x8009cd1c */
void InsertBSTE(tOrderedList *tree, void *element)
{
    _InsertBSTE(tree, &tree->root, element);
}

/* 0x8009cd44 */
static void _InsertBSTE(tOrderedList *tree, void *root, void *element)
{
    int x;

    if (*(void **)root == NULL) {
        *(void **)root = element;
        ((BSTNode *)*(void **)root)->left = NULL;
        ((BSTNode *)*(void **)root)->right = NULL;
        tree->numElements++;
    } else {
        x = tree->OrderFunc(element, *(void **)root);
        if (x < 0)
            _InsertBSTE(tree, &((BSTNode *)*(void **)root)->left, element);
        else
            _InsertBSTE(tree, &((BSTNode *)*(void **)root)->right, element);
    }
}

/* 0x8009cdf8 */
void WalkBST(tOrderedList *tree, void (*WalkFunc)(void *, int))
{
    int num = 0;

    _WalkBST(tree, tree->root, &num, WalkFunc);
}

/* 0x8009ce2c */
static void _WalkBST(tOrderedList *tree, void *root, int *num, void (*WalkFunc)(void *, int))
{
    if (root) {
        _WalkBST(tree, ((BSTNode *)root)->left, num, WalkFunc);
        WalkFunc(root, *num);
        (*num)++;
        _WalkBST(tree, ((BSTNode *)root)->right, num, WalkFunc);
    }
}
