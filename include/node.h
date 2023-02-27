
#ifndef NODE_
#define NODE_

#include <stdlib.h>
#include <cstdio>
#include <assert.h>
#include <math.h>
#include <string.h>

#define CELL_FMT "%s"

//===============================================

const size_t ConstHash = 0XA12C78D;

//===============================================

typedef enum OperAndFunc{

    NULL_OPER = -111,

    #define newoper(name, val, codir,...)\
        name = codir,

    #define newfun(name, codir, ...)\
        FUNC_##name = codir,

    #include "../function"

    #undef newfun
    #undef newoper

} OperAndFunc;

typedef enum Type{

    INT         = 'i',
    DOT         = 'd',
    VARIABLE    = 'v',
    FUNCTION    = 'f',
    OPERATOR    = 'o',

    USER_FUNCTION    ,
    EMPTY_NODE       ,
    END_OF_TOKENS

} Type;

typedef union Data{

    int i_num;

    double d_num;

    OperAndFunc stat; //statement

} Data;

typedef struct Node{

    char* cell;

    Type type;
    Data data;

    size_t hash = 0;

    Node* left_son;
    Node* right_son;

} Node;

//===============================================

void node_fmt_print(FILE* out_file, Node* node);
void pro_print(Node* node);
void tree_destruct(Node* node);
Node* tree_construct(void);
Node* new_node(Type a_type, OperAndFunc a_stat = NULL_OPER, Node* right_node = nullptr, Node* left_node = nullptr);
void    do_tree_simplify    (Node** node);
Node*   simple_node         (Node* tested_node);
Node* node_cpy(Node* main_node);
void daddy_and_sons_connection(Node* daddy, Node* a_right_son = nullptr, Node* a_left_son = nullptr);
//===============================================

#endif // NODE_
