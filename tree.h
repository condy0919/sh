#ifndef _TREE_INCLUDED
#define _TREE_INCLUDED

#include <stdlib.h>
#include <assert.h>

#include "command.h"


// type for the tree_node_t
#define TCOM 0x1
#define TLST 0x2
#define TFIL 0x4

typedef struct tree_node_t {
    command_t part_cmd;

    unsigned int type;
    struct {
//	unsigned int f_and:1;
	unsigned int f_cat:1;// >> mode
	unsigned int f_pin:1;
	unsigned int f_pout:1;
    } flag;

    int redirect_in;
    int redirect_out;

    struct tree_node_t* left;
    struct tree_node_t* right;
} tree_node_t;


static inline void init_tree_node(tree_node_t* p)
{
    init_command(&p->part_cmd);

    p->type = 0;
    *(int*)(&p->flag) = 0;
    p->left = p->right = NULL;
    p->redirect_in = p->redirect_out = -1;
}

#endif

