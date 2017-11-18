#ifndef _INCLUDED_ABSTRACT_SYNTAX_TREE_H_
#define _INCLUDED_ABSTRACT_SYNTAX_TREE_H_

#include "token.h"

typedef struct _ABSTRACT_SYNTAX_TREE
{
	struct _ABSTRACT_SYNTAX_TREE *left, *right, *center;
	TOKEN *token;
} ABSTRACT_SYNTAX_TREE;

#endif	// #ifndef _INCLUDED_ABSTRACT_SYNTAX_TREE_H_
