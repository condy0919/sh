#ifndef _SHELL_INCLUDED
#define _SHELL_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "readline.h"
#include "tree.h"


#define TOKEN_NUMBER 128

static inline int equal(char* str1, char* str2)
{
    if (strcmp(str1, str2) == 0)
	return 1;
    return 0;
}

static inline char* next_filter(char* str)
{
    static const char* filter = ";|&";
    if (!str || !*str)
	return NULL;

    while (*str && !strchr(filter, *str))
	++str;
    return str;
}

static inline char* next_command(char* str)
{
    static const char* filter = " ;|&";
    if (!str || !*str)
	return NULL;

    while (*str && strchr(filter, *str))
	++str;
    return str;
}

/** 
 * the command line entered from stdin or from -c [command].
 * this funciton only divide command line into slots
 * @param str command sequence
 * @param array store command and property of this command
 * @param n limit of array
 * 
 * @return the size of a dependent command including operators
 */
int cope(char* str, tree_node_t* array[], int n);


/** 
 * used in cope for spliting a command between filter.
 * the delim is ' ' but the redirection is special.
 * @param array store command and property of command
 * @param i position in array
 * @param front the start of a string
 * @param end the end of a string
 * the range is [front, end)
 * 
 * @return next position, -1 for error.
 */
int split(tree_node_t* array[], int i, char* front, char* end);


/** 
 * destroy token parameter
 * 
 * @param root root node
 */
void destroy_token(tree_node_t* root);


/** 
 * build a syntax tree, run from top to bottom.
 * 
 * @param token array consists of commands
 * @param n available token in list
 * 
 * @return root node
 */
tree_node_t* syntax_tree(tree_node_t* token[], int n);


/** 
 * parse for the command's redirection.
 * 
 * @param root root node of the syntax tree
 */
void do_analyse(tree_node_t* root);


/** 
 * execute the command DFSly
 * 
 * @param root root node of syntax tree
 * @param fp1 the pipe in end
 * @param fp2 the pipe out end
 * fp1, fp2 points to same array, only use fp1[0], fp2[1]
 */
void execute(tree_node_t* root, int fp1[], int fp2[]);

#endif
