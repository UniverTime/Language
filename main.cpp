
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "include/node.h"
#include "include/dump.h"
#include "include/asm.h"
#include "HashList/list.h"
#include "include/lexer.h"

//===============================================

#define DEBUG\
    printf("%s ", __PRETTY_FUNCTION__);\
    node_fmt_print(stdout, WORKING_TAPE->node);\
    printf("\n");

#define NEXT_TAPE WORKING_TAPE = WORKING_TAPE->prev

#define PREV_TAPE WORKING_TAPE = WORKING_TAPE->next

List* WORKING_TAPE = nullptr;//������� �����

//===============================================

const size_t SYNTAX_ERROR = 0;

/*

G Grammar
E Expression
T Term
D Degree
P Primary expression
V Variable
N Number

VU Variables using
VI Variables initialization (preliminary version)
F function
NVV new value of variables
LT letters

G::= { VU | IF | WHILE | MLU }* 'END_OF_TOKENS'

VU::= { { '@'VI';' } | { NVV';' } }*
IF::= 'if(' E ')true{' G '}'  //����������� �� then
WHILE:: = 'while('E')'
MLU:: =  let [a-z,A-Z]+ { ,[a-z,A-Z]+}* as int MRX | LST { and MLU[without let]}*   //int | dot

MRX:: = matrix (N * N)={N{,N}*}
LST:: = list   (N)    ={N{,N}*}

VI::= [a-z,A-Z]+ { '='VI | E{ ','VI }* }*
NVV::= [a-z,A-Z]+'='E

E::= T{ [+-]T }*
T::= D{ [*\]D }*
D::= P{ [^]P }*
P::= '('E')' | N | V
V::= [a-z,A-Z]+
N::= [0-9]+


tree builder:

tree model such as in differentiator

with lexer and parser
*/

//===============================================
typedef struct FuncParameters{

    const char* func_name   = nullptr;

    size_t func_length      = 0;

} FuncParameters;

char* capsule_fusioning(char** cells_name, const size_t cells_length,
                        const FuncParameters* func_param);

Node* getFuncArguments(int* arg_value, const FuncParameters* func_param);
Node* getFuncInit(void);
Node* getG(void);
Node* getIF(void);
Node* getWHILE(void);
Node* getVU(void);

Node* getMLU(void);
Node* getMRX(void);
Node* getLST(const char* l_name, int* list_length);

Node* getE(void);
Node* getT(void);
Node* getD(void);
Node* getP(void);
Node* getV(void);
Node* getN(void);

Node* getVI(void);
Node* getNVV(void);

Node* get_recursive_equal_sign(Node** the_last_equal_node);
Node* get_recursive_FUNC_as(Node** the_last_equal_node, int*);
//===============================================

HashTree* tree = (HashTree*) calloc(1, sizeof (HashTree));

//===============================================

typedef enum ErrorCode{

    FAILED_TYPE,
    FAILED_DATA,

    FAILED_VAR_REDECLARATION,
    FAILED_VAR_NOT_INIT,

    FAILED_FUNCTION_REDECLARATION,
    FAILED_FUNCTION_NOT_INIT,

    FAILED_ANOTHER

} ErrorCode;

void syntax_error_handler(List* list_of_error_node, const char* pretty_function, ErrorCode error_code,
                            Type expected_type          = EMPTY_NODE,
                            OperAndFunc expected_data   = NULL_OPER);

Node* create_list_initialization(const char* variable_name, const char* last_variable_name, int arg_length);

Node* init_variable_node_with_index_name(Node* node, const char* name, int number);

void syntax_error_handler(List* list_of_error_node, const char* pretty_function, ErrorCode error_code,
                            Type expected_type, OperAndFunc expected_data)
{
    FILE* out_file = stdout;

    fprintf(out_file, "\n----\t ERRORS \t----\n\n");

    size_t list_ind = 0;

    List* main_list = list_of_error_node;

    while (list_of_error_node->node->type != END_OF_TOKENS
           &&
           (list_of_error_node->next->node->type != OPERATOR
              ||
              list_of_error_node->next->node->data.stat != ';')
           &&
           list_ind < 10
          )
    {
        node_fmt_print(out_file, list_of_error_node->node);

        list_of_error_node = list_of_error_node->prev;

        ++list_ind;
    }

    if (list_of_error_node->node->type == END_OF_TOKENS)
        putc('.', out_file);

    else if (list_ind == 10)
        fprintf(out_file, "...");

    fprintf(
            out_file,

            "\n ^\n"
            " ^ error here\n"
            "Line %zu, Column %zu\n\n",

            main_list->string_place,
            main_list->cursor_place
           );

    switch (error_code)
    {
        case FAILED_TYPE:
        {
            fprintf(out_file, "Expected ");

            switch (expected_type)
            {
                case VARIABLE:
                {
                    fprintf(out_file, "variable");

                    break;
                }

                case FUNCTION:
                {
                    fprintf(out_file, "system function ");

                    break;
                }

                case USER_FUNCTION:
                {
                    fprintf(out_file, "function");

                    break;
                }

                case OPERATOR:
                {
                    fprintf(out_file, "operator - %c", expected_data);

                    break;
                }

                case INT:
                {
                    fprintf(out_file, "data with int type");

                    break;
                }

                case DOT:
                {
                    fprintf(out_file, "data with dot type");

                    break;
                }

                case END_OF_TOKENS:
                {
                    fprintf(out_file, "end of program");

                    break;
                }

                default:
                {
                    fprintf(out_file, "System error!!");

                    assert (0);
                }
            }

            //putc('.', out_file);

            break;
        }

        case FAILED_DATA:
        {
            if (expected_type == OPERATOR)
            {
                fprintf(out_file, "expected %c", expected_data);
            }

            break;
        }

        case FAILED_VAR_NOT_INIT:
        {
            fprintf(out_file, "variable %s didn't initialize", main_list->node->cell);

            break;
        }

        case FAILED_VAR_REDECLARATION:
        {
            fprintf(out_file, "redeclaration of this variable - %s ", main_list->node->cell);

            break;
        }

        case FAILED_FUNCTION_REDECLARATION:
        {
            fprintf(out_file, "redeclaration of this function - %s ", main_list->node->cell);

            break;
        }

        case FAILED_FUNCTION_NOT_INIT:
        {
            fprintf(out_file, "this function didn't initialize - %s ", main_list->node->cell);

            break;
        }

        case FAILED_ANOTHER:
        {
            fprintf(out_file, "don't understand the meaning");

            break;
        }
        default:
        {
            fprintf(out_file, "System error!! Program close.");

            assert (0);
        }
    }

    putc('.', out_file); //from 234 line

    putc('\n', out_file);

    fprintf(out_file, "Signal from %s\n\n", pretty_function);

    assert (SYNTAX_ERROR);
}

char* my_itoa(int num, char* buffer, int base)
{
    int curr = 0;

    if (num == 0)
    {
        buffer[curr++] = '0';
        buffer[curr]   = '\0';

        return buffer;
    }

    int num_digits = 0;

    if (num < 0)
    {
        if (base == 10)
        {
            num_digits ++;

            buffer[curr] = '-';

            curr ++;

            // Make it positive and finally add the minus sign
            num *= -1;
        }
        else
            // Unsupported base. Return NULL
            return NULL;
    }

    num_digits += (int)floor(log(num) / log(base)) + 1;

    // Go through the digits one by one
    // from left to right
    while (curr < num_digits)
    {
        // Get the base value. For example, 10^2 = 1000, for the third digit
        int base_val = (int) pow(base, num_digits-1-curr);

        // Get the numerical value
        int num_val = num / base_val;

        char value = num_val + '0';
        buffer[curr] = value;

        curr ++;
        num -= base_val * num_val;
    }

    buffer[curr] = '\0';

    return buffer;
}

int main(void)
{
    Tree* tokens_list = begin_lexering("hell.txt");

    WORKING_TAPE = tokens_list->lst->prev;

    H_list_init(tree, 25);

    Node* root = getFuncInit(); //getG();

    graph_tree_dump(root);

    //do_asm_translation(root);

    do_tree_simplify(&root);

    graph_tree_dump(root);

    H_list_destructor(tree);

    tree_destruct(root);

    /*printf("%d", tokens_list->size);

    List* lll = tokens_list->lst;

    for (int i = 0; i < tokens_list->size; i++)
    {
        node_fmt_print(stdout, lll->prev->node);

        lll = lll->prev;
    }*/

    list_destructor(tokens_list);

    free(tree);

    free(tokens_list);
}

Node* getG(void)
{
    Node* root  = nullptr,
        * daddy = nullptr;

    while ((root = getVU())
           ||
           (root = getIF())
           ||
           (root = getWHILE())
           ||
           (root = getMLU())
          )
    {
        daddy = new_node(EMPTY_NODE, EQUAL, root, daddy);
    }

    /*if (WORKING_TAPE->node->type != END_OF_TOKENS)
    {
        syntax_error_handler(WORKING_TAPE, __PRETTY_FUNCTION__,
                                FAILED_TYPE, END_OF_TOKENS);
    }*/

    return daddy;
}

Node* getMLU(void)
{
    Node* left_son  = nullptr,
        * right_son = nullptr,
        * daddy     = nullptr;

    DEBUG

    if (WORKING_TAPE->node->type == FUNCTION
        &&
        WORKING_TAPE->node->data.stat == FUNC_let
       )
    {
        tree_destruct(WORKING_TAPE->node);

        NEXT_TAPE;

        if (WORKING_TAPE->node->type != VARIABLE)
            syntax_error_handler(WORKING_TAPE, __PRETTY_FUNCTION__, FAILED_TYPE, VARIABLE);

        int help_int;

        daddy = get_recursive_FUNC_as(&right_son, &help_int);

    }

    return daddy;
}

Node* get_recursive_FUNC_as(Node** the_last_equal_node, int* arg_length)
{
    DEBUG

    Node* left_son  = nullptr,
            * right_son = nullptr,
            * daddy     = nullptr;

    if (WORKING_TAPE->node->type == VARIABLE
        &&
        WORKING_TAPE->prev->node->type == FUNCTION
        &&
        WORKING_TAPE->prev->node->data.stat == FUNC_as
       )
    {
        Node* a_last_equal_node = nullptr;

        char* variable_name = WORKING_TAPE->node->cell;

        if (H_search_list_by_hash(tree, variable_name))
        {
            printf("REDEFINITION OF VARIABLE");

            assert (SYNTAX_ERROR);
        }
        else
        {
            H_list_insert(tree, 0, variable_name, V_VARIABLE);
        }

        left_son = WORKING_TAPE->node;

        NEXT_TAPE;

        daddy = WORKING_TAPE->node;

        NEXT_TAPE;

        right_son = get_recursive_FUNC_as(&a_last_equal_node, arg_length);

        if ((right_son->type == EMPTY_NODE)
            ||
            (right_son->type == FUNCTION
              &&
              right_son->data.stat == FUNC_as)
           )
        {
            daddy->right_son = a_last_equal_node;

            daddy->left_son = left_son;

            daddy = new_node(EMPTY_NODE, EQUAL, daddy, right_son);

            daddy->right_son->right_son->left_son = create_list_initialization(variable_name, a_last_equal_node->cell, *arg_length);

            *the_last_equal_node = node_cpy(a_last_equal_node);
        }
        else
        {
            *the_last_equal_node = node_cpy(left_son);

            daddy_and_sons_connection(daddy, right_son, left_son);
        }

        return daddy;
    }
    else
    {
        if (WORKING_TAPE->node->type == FUNCTION
            &&
            WORKING_TAPE->node->data.stat == FUNC_int
           )
        {
            tree_destruct(WORKING_TAPE->node);

            NEXT_TAPE;

            if (!(right_son = getMRX())
                &&
                !(right_son = getLST(WORKING_TAPE->next->next->next->node->cell, arg_length))
               )
                syntax_error_handler(WORKING_TAPE, __PRETTY_FUNCTION__, FAILED_TYPE, FUNCTION, FUNC_list);
        }
        else
            syntax_error_handler(WORKING_TAPE, __PRETTY_FUNCTION__,
                                    FAILED_TYPE, FUNCTION);

        return right_son;
    }
}

Node* create_list_initialization(const char* l_name, const char* last_l_name, int arg_length)
{
    Node* equal_daddy       = nullptr,
        * equal_left_son    = nullptr,
        * equal_right_son   = nullptr,
        * daddy             = nullptr;

    char buffer[17];

    size_t l_length = strlen (l_name),
           b_length = 0;

    for (size_t l_ind = 0; l_ind < arg_length; l_ind++)
    {
        equal_left_son = init_variable_node_with_index_name(new_node(VARIABLE), l_name, l_ind);

        equal_right_son = init_variable_node_with_index_name(new_node(VARIABLE), last_l_name, l_ind);


        H_list_insert(tree, 0, equal_left_son->cell, V_VARIABLE);


        equal_daddy = new_node(OPERATOR, EQUAL, equal_right_son, equal_left_son);

        daddy = new_node(EMPTY_NODE, EQUAL, equal_daddy, daddy);
    }

    return daddy;
}

Node* init_variable_node_with_index_name(Node* node, const char* name, int number)
{
    char buffer[17] = {};

    size_t name_length = strlen (name);

    my_itoa(number, buffer, 10);

    size_t b_length = strlen (buffer);

    node->cell = (char*) realloc(node->cell, sizeof (char) * (name_length + b_length + 2));

    assert (name);

    strcpy(node->cell, name);

    node->cell[name_length] = '_';

    strncpy(&node->cell[name_length + 1], buffer, b_length);

    node->cell[b_length + name_length + 1] = '\0';

    return node;
}

Node* getMRX(void)
{
    Node* daddy = nullptr;

    DEBUG

    if (WORKING_TAPE->node->type == FUNCTION
        &&
        WORKING_TAPE->node->data.stat == FUNC_matrix
       )
    {
        daddy = WORKING_TAPE->node;

        NEXT_TAPE;

        if (WORKING_TAPE->node->type == OPERATOR
            &&
            WORKING_TAPE->node->data.stat == '('
           )
        {
            tree_destruct(WORKING_TAPE->node);

            NEXT_TAPE;



            //if (getN());
        }

        //
    }

    return daddy;
}

Node* getLST(const char* l_name, int* list_length)
{
    Node* left_son  = nullptr,
        * arg_node  = nullptr,
        * daddy     = nullptr;

    DEBUG

    if (WORKING_TAPE->node->type == FUNCTION
        &&
        WORKING_TAPE->node->data.stat == FUNC_list
       )
    {
        daddy = WORKING_TAPE->node;

        NEXT_TAPE;

        if (WORKING_TAPE->node->type == OPERATOR
            &&
            WORKING_TAPE->node->data.stat == '('
           )
        {
            tree_destruct(WORKING_TAPE->node);

            NEXT_TAPE;

            if ((arg_node = getN())
                &&
                arg_node->type == INT
                &&
                arg_node->data.i_num >= 0
               )
            {
                *list_length = arg_node->data.i_num;

                tree_destruct(arg_node);

                if (WORKING_TAPE->node->type == OPERATOR
                    &&
                    WORKING_TAPE->node->data.stat == ')'
                   )
                {
                    tree_destruct(WORKING_TAPE->node);

                    NEXT_TAPE;

                    if (WORKING_TAPE->node->type == OPERATOR
                        &&
                        WORKING_TAPE->node->data.stat == '='
                       )
                    {
                        tree_destruct(WORKING_TAPE->node);

                        NEXT_TAPE;

                        if (WORKING_TAPE->node->type == OPERATOR
                            &&
                            WORKING_TAPE->node->data.stat == '{'
                           )
                        {
                            tree_destruct(WORKING_TAPE->node);

                            NEXT_TAPE;

                            //printf("%s", l_name);

                            Node* equal_daddy       = nullptr,
                                * equal_left_son    = nullptr,
                                * equal_right_son   = nullptr;

                            char buffer[17];

                            size_t l_length = strlen (l_name),
                                   b_length = 0;

                            for (size_t l_ind = 0; l_ind < *list_length; l_ind++)
                            {
                                left_son = new_node(EMPTY_NODE, EQUAL, equal_daddy, left_son);

                                if (!(equal_right_son = getE()))
                                {
                                    equal_right_son = new_node(INT);

                                    equal_right_son->data.i_num = 0;

                                    // or another type
                                }

                                equal_left_son = new_node(VARIABLE);

                                my_itoa(l_ind, buffer, 10);

                                b_length = strlen (buffer);

                                equal_left_son->cell = (char*) realloc(equal_left_son->cell, sizeof (char) * (l_length + b_length + 2));

                                assert (l_name);

                                strcpy(equal_left_son->cell, l_name);

                                equal_left_son->cell[l_length] = '_';

                                strncpy(&equal_left_son->cell[l_length + 1], buffer, b_length);

                                equal_left_son->cell[b_length + l_length + 1] = '\0';

                                //
                                H_list_insert(tree, 0, equal_left_son->cell, V_VARIABLE);
                                //

                                equal_daddy = new_node(OPERATOR, EQUAL);

                                daddy_and_sons_connection(equal_daddy, equal_right_son, equal_left_son);

                                left_son->right_son = equal_daddy;

                                if (l_ind == *list_length - 1)
                                    continue;

                                if (WORKING_TAPE->node->type == OPERATOR
                                    &&
                                    WORKING_TAPE->node->data.stat == ','
                                   )
                                {
                                    tree_destruct(WORKING_TAPE->node);

                                    NEXT_TAPE;
                                }
                                else
                                {

                                }
                            }

                            if (WORKING_TAPE->node->type == OPERATOR
                                &&
                                WORKING_TAPE->node->data.stat == '}'
                               )
                            {
                                tree_destruct(WORKING_TAPE->node);

                                NEXT_TAPE;
                            }
                            else
                            {

                            }
                        }
                        else
                        {

                        }
                    }
                    else
                    {

                    }
                }
                else
                {

                }
            }
            else
            {

            }
        }
        else
        {

        }
    }

    daddy_and_sons_connection(daddy, nullptr, left_son);

    return daddy;
}

Node* getIF(void)
{
    Node* if_root = nullptr;

    DEBUG

    if (WORKING_TAPE->node->type == FUNCTION
        &&
        WORKING_TAPE->node->data.stat == FUNC_if
       )
    {
        if_root = WORKING_TAPE->node;

        //daddy_and_sons_connection(if_root, new_node(FUNCTION, FUNC_then));

        NEXT_TAPE;

        if (WORKING_TAPE->node->type == OPERATOR
            &&
            WORKING_TAPE->node->data.stat == '('
           )
        {
            tree_destruct(WORKING_TAPE->node);

            NEXT_TAPE;

            if_root->left_son = getE();

            if (WORKING_TAPE->node->type == OPERATOR
                &&
                WORKING_TAPE->node->data.stat == ')'
               )
            {
                tree_destruct(WORKING_TAPE->node);

                NEXT_TAPE;

                if (WORKING_TAPE->node->type == FUNCTION
                    &&
                    WORKING_TAPE->node->data.stat == FUNC_true
                   )
                {
                    daddy_and_sons_connection(if_root, WORKING_TAPE->node);

                    NEXT_TAPE;

                    if (WORKING_TAPE->node->type == OPERATOR
                        &&
                        WORKING_TAPE->node->data.stat == '{'
                       )
                    {
                        tree_destruct(WORKING_TAPE->node);

                        NEXT_TAPE;

                        daddy_and_sons_connection(if_root->right_son, getNVV());

                        if (WORKING_TAPE->node->type == OPERATOR
                            &&
                            WORKING_TAPE->node->data.stat == '}'
                           )
                        {
                            tree_destruct(WORKING_TAPE->node);

                            NEXT_TAPE;


                        }
                        else
                            assert (SYNTAX_ERROR);

                    }
                    else
                        assert (SYNTAX_ERROR);


                }
                else
                    assert (SYNTAX_ERROR);
            }
            else
                assert (SYNTAX_ERROR);

        }
        else
            assert (0);
    }

    return if_root;
}

Node* getWHILE(void)
{
    Node* while_root = nullptr;

    DEBUG

    if (WORKING_TAPE->node->type == FUNCTION
        &&
        WORKING_TAPE->node->data.stat == FUNC_while
       )
    {
        while_root = WORKING_TAPE->node;

        NEXT_TAPE;

        if (WORKING_TAPE->node->type == OPERATOR
            &&
            WORKING_TAPE->node->data.stat == '('
           )
        {
            tree_destruct(WORKING_TAPE->node);

            NEXT_TAPE;

            while_root->left_son = getE();

            if (WORKING_TAPE->node->type == OPERATOR
                &&
                WORKING_TAPE->node->data.stat == ')'
               )
            {
                tree_destruct(WORKING_TAPE->node);

                NEXT_TAPE;
            }
            else
                assert (0);

        }
        else
            assert (0);
    }

    return while_root;
}

Node* getVU(void)
{
    Node* left_son  = nullptr,
        * right_son = nullptr,
        * daddy     = nullptr;

    DEBUG

    while (WORKING_TAPE->node->type == OPERATOR
           &&
           WORKING_TAPE->node->data.stat == '@'
           ||
           (WORKING_TAPE->prev->node->type == OPERATOR
             &&
             WORKING_TAPE->prev->node->data.stat == '='
             &&
             H_search_list_by_hash(tree, WORKING_TAPE->node->cell))
          )
    {
        DEBUG

        while (WORKING_TAPE->node->type == OPERATOR
               &&
               WORKING_TAPE->node->data.stat == '@'
              )
        {
            tree_destruct(WORKING_TAPE->node);

            NEXT_TAPE;

            daddy = new_node(EMPTY_NODE, EQUAL, getVI(), daddy);

            if (WORKING_TAPE->node->type != OPERATOR
                ||
                WORKING_TAPE->node->data.stat != ';'
               )
                assert (SYNTAX_ERROR);
            else
                tree_destruct(WORKING_TAPE->node);

            NEXT_TAPE;
        }

        while (H_search_list_by_hash(tree, WORKING_TAPE->node->cell)
               &&
               WORKING_TAPE->prev->node->type == OPERATOR
               &&
               WORKING_TAPE->prev->node->data.stat == '='
              )
        {
            DEBUG

            daddy = new_node(EMPTY_NODE, EQUAL, getNVV(), daddy);

            if (WORKING_TAPE->node->type != OPERATOR
                ||
                WORKING_TAPE->node->data.stat != ';'
               )
                assert (SYNTAX_ERROR);
            else
                tree_destruct(WORKING_TAPE->node);

            NEXT_TAPE;
        }
    }

    return daddy;
}

Node* getVI(void)
{
    Node* left_son  = nullptr,
        * right_son = nullptr,
        * daddy     = nullptr;

    char* variable_name = nullptr;

    DEBUG

    if (WORKING_TAPE->node->type == VARIABLE
        &&
        (WORKING_TAPE->prev->node->type == OPERATOR && WORKING_TAPE->prev->node->data.stat == ';')
       )
    {
        if (H_search_list_by_hash(tree, WORKING_TAPE->node->cell))
        {
            printf("REDEFINITION OF VARIABLE");

            assert (SYNTAX_ERROR);
        }
        else
        {
            H_list_insert(tree, 0, WORKING_TAPE->node->cell, V_VARIABLE);
        }

        //tree->lst->next->value.integer = 0;

        daddy = new_node(OPERATOR, EQUAL, new_node(INT), WORKING_TAPE->node);

        daddy->right_son->data.i_num = 0;

        NEXT_TAPE;

        return daddy;
    }


    do
    {
        if (WORKING_TAPE->next->node->type == OPERATOR
            &&
            WORKING_TAPE->next->node->data.stat == ','
           )
            tree_destruct(WORKING_TAPE->next->node);

        right_son = get_recursive_equal_sign(&left_son);

        daddy = new_node(EMPTY_NODE, EQUAL, right_son, daddy);

    } while (WORKING_TAPE->node->type == OPERATOR
             &&
             WORKING_TAPE->node->data.stat == ','
             &&
             (NEXT_TAPE)
            );

    return daddy;
}

Node* get_recursive_equal_sign(Node** the_last_equal_node)
{
    DEBUG

    if (WORKING_TAPE->node->type == VARIABLE
        &&
        WORKING_TAPE->prev->node->type == OPERATOR
        &&
        WORKING_TAPE->prev->node->data.stat == '='
       )
    {
        Node* left_son  = nullptr,
            * right_son = nullptr,
            * daddy     = nullptr;

        Node* a_last_equal_node = nullptr;

        char* variable_name = WORKING_TAPE->node->cell;

        if (H_search_list_by_hash(tree, variable_name))
        {
            printf("REDEFINITION OF VARIABLE");

            assert (SYNTAX_ERROR);
        }
        else
        {
            H_list_insert(tree, 0, variable_name, V_VARIABLE);
        }

        left_son = WORKING_TAPE->node;

        NEXT_TAPE;

        daddy = WORKING_TAPE->node;

        NEXT_TAPE;

        right_son = get_recursive_equal_sign(&a_last_equal_node);

        if ((right_son->type == EMPTY_NODE)
            ||
            (right_son->type == OPERATOR
              &&
              right_son->data.stat == '=')
           )
        {
            daddy->right_son = a_last_equal_node;

            daddy->left_son = left_son;

            daddy = new_node(EMPTY_NODE, EQUAL, daddy, right_son);

            *the_last_equal_node = node_cpy(a_last_equal_node);
        }
        else
        {
            *the_last_equal_node = node_cpy(left_son);

            daddy_and_sons_connection(daddy, right_son, left_son);
        }

        return daddy;
    }
    else
    {
        Node* daddy = WORKING_TAPE->node;

        NEXT_TAPE;

        return daddy;
    }
}

Node* getNVV(void)
{
    Node* left_son  = nullptr,
        * daddy     = nullptr;

    if (WORKING_TAPE->node->type == VARIABLE)
    {
        HashList* detected_variable
          = H_search_list_by_hash(tree, WORKING_TAPE->node->cell);

        left_son = WORKING_TAPE->node;

        NEXT_TAPE;

        if (detected_variable
            &&
            WORKING_TAPE->node->type == OPERATOR
            &&
            WORKING_TAPE->node->data.stat == '='
           )
        {
            daddy = WORKING_TAPE->node;

            NEXT_TAPE;

            daddy_and_sons_connection(daddy, getE(), left_son);
        }
        else
        {
            printf("VARIABLE OR SIGN '=' DIDN'T FIND!!");

            assert (SYNTAX_ERROR);
        }
    }

    return daddy;
}

Node* getE(void)
{
    Node* left_son  = nullptr,
        * right_son = nullptr,
        * daddy     = nullptr;

    daddy = getT();

    DEBUG

    while (WORKING_TAPE->node->type == OPERATOR
           &&
           (WORKING_TAPE->node->data.stat == '+' || WORKING_TAPE->node->data.stat == '-'))
    {
        left_son = daddy;

        daddy = WORKING_TAPE->node;

        NEXT_TAPE;

        right_son = getT();

        daddy_and_sons_connection(daddy, right_son, left_son);
    }

    return daddy;
}

Node* getT(void)
{
    Node* left_son  = nullptr,
        * right_son = nullptr,
        * daddy     = nullptr;

    daddy = getD();

    DEBUG

    while (WORKING_TAPE->node->type == OPERATOR
           &&
           (WORKING_TAPE->node->data.stat == '*' || WORKING_TAPE->node->data.stat == '/'))
    {
        left_son = daddy;

        daddy = WORKING_TAPE->node;

        NEXT_TAPE;

        right_son = getD();

        daddy_and_sons_connection(daddy, right_son, left_son);
    }

    return daddy;
}

Node* getD(void)
{
    Node* left_son  = nullptr,
        * right_son = nullptr,
        * daddy     = nullptr;

    DEBUG

    daddy = getP();

    while (WORKING_TAPE->node->type == OPERATOR
           &&
           WORKING_TAPE->node->data.stat == '^'
          )
    {
        left_son = daddy;

        daddy = WORKING_TAPE->node;

        NEXT_TAPE;

        right_son = getP();

        daddy_and_sons_connection(daddy, right_son, left_son);
    }

    return daddy;
}

Node* getP(void)
{
    Node* recognized_node = nullptr;

    DEBUG

    if (WORKING_TAPE->node->type == OPERATOR
        &&
        WORKING_TAPE->node->data.stat == '('
       )
    {
        tree_destruct(WORKING_TAPE->node);

        NEXT_TAPE;

        recognized_node = getE();

        if (WORKING_TAPE->node->type == OPERATOR
            &&
            WORKING_TAPE->node->data.stat == ')'
           )
        {
            tree_destruct(WORKING_TAPE->node);

            NEXT_TAPE;
        }
        else
        {
            printf("\n SYNTAX ERROR!!");

            assert (0);
        }
    }
    else if (WORKING_TAPE->node->type == INT)
        recognized_node = getN();
    else
        recognized_node = getV();

    return recognized_node;
}

Node* getV(void)
{
    Node*       recognized_node   = nullptr;
    HashList*   detected_variable = nullptr;

    DEBUG

    if (WORKING_TAPE->node->type == VARIABLE)
    {
        detected_variable = H_search_list_by_hash(tree, WORKING_TAPE->node->cell);

        if (!detected_variable)
        {
            fprintf(stdout, "THIS VARIABLE DIDN'T FOUND:\"%s\"\n", WORKING_TAPE->node->cell);

            assert(detected_variable);
        }

        recognized_node = WORKING_TAPE->node;

        NEXT_TAPE;
    }

    return recognized_node;
}

Node* getN(void)
{
    Node* recognized_node = nullptr;

    DEBUG

    if (WORKING_TAPE->node->type == INT
        ||
        WORKING_TAPE->node->type == DOT
       )
    {
        recognized_node = WORKING_TAPE->node;

        NEXT_TAPE;
    }

    return recognized_node;
}

//getFuncInit::= 'function' name_of_function '(' getFuncArguments ')' '{' G '}'
//             { 'function' name_of_function '(' getFuncArguments ')' '{' G '}'
//             }*



// H_list_insert(tree, 0, equal_left_son->cell, V_VARIABLE);
// H_search_list_by_hash(tree, WORKING_TAPE->node->cell)
//
Node* getFuncInit(void)
{
    Node* recognized_node = nullptr,
        * root_node       = nullptr;

    FuncParameters func_param;

    DEBUG

    if (WORKING_TAPE->node->type == FUNCTION
        &&
        WORKING_TAPE->node->data.stat == FUNC_function
       )
    {
        NEXT_TAPE;

        if (WORKING_TAPE->node->type == USER_FUNCTION
            &&
            !H_search_list_by_hash(tree, WORKING_TAPE->node->cell)
            &&
            WORKING_TAPE->prev->node->type == OPERATOR
            &&
            WORKING_TAPE->prev->node->data.stat == OB
           )
        {
            func_param.func_name    = WORKING_TAPE->node->cell;
            func_param.func_length  = strlen(func_param.func_name);

            H_list_insert(tree, 0, (char*) func_param.func_name, V_USER_FUNCTION);

            root_node = WORKING_TAPE->node;

            NEXT_TAPE;
            NEXT_TAPE;

            root_node->right_son = new_node(EMPTY_NODE);
            root_node->right_son->left_son = getFuncArguments(&root_node->data.i_num,
                                                                &func_param);

            if (WORKING_TAPE->node->type == OPERATOR
                &&
                WORKING_TAPE->node->data.stat == CB
               )
                NEXT_TAPE;
            else
            {
                syntax_error_handler(WORKING_TAPE, __PRETTY_FUNCTION__,
                                        FAILED_DATA, OPERATOR, CB);
            }

            //NEXT_TAPE;

            if (WORKING_TAPE->node->type == OPERATOR
                &&
                WORKING_TAPE->node->data.stat == FOB // forming open bracket
               )
            {

                NEXT_TAPE;

                root_node->right_son->right_son = getG();

                if (WORKING_TAPE->node->type == OPERATOR
                    &&
                    WORKING_TAPE->node->data.stat == FCB // forming close brackets
                   )
                {
                    NEXT_TAPE;

                    //prev_node = root_node;

                    while (WORKING_TAPE->node->type == FUNCTION
                           &&
                           WORKING_TAPE->node->data.stat == FUNC_function
                          )
                    {
                        NEXT_TAPE;

                        if (WORKING_TAPE->node->type == USER_FUNCTION
                            &&
                            !H_search_list_by_hash(tree, WORKING_TAPE->node->cell)
                            &&
                            WORKING_TAPE->prev->node->type == OPERATOR
                            &&
                            WORKING_TAPE->prev->node->data.stat == OB
                           )
                        {
                            func_param.func_name    = WORKING_TAPE->node->cell;
                            func_param.func_length  = strlen(func_param.func_name);

                            H_list_insert(tree, 0, (char*) func_param.func_name, V_USER_FUNCTION);

                            recognized_node = WORKING_TAPE->node;

                            NEXT_TAPE;
                            NEXT_TAPE;

                            recognized_node->right_son = new_node(EMPTY_NODE);
                            recognized_node->right_son->left_son = getFuncArguments(&recognized_node->data.i_num,
                                                                                        &func_param);

                            if (WORKING_TAPE->node->type == OPERATOR
                                &&
                                WORKING_TAPE->node->data.stat == CB
                               )
                                NEXT_TAPE;
                            else
                            {
                                syntax_error_handler(WORKING_TAPE, __PRETTY_FUNCTION__,
                                                        FAILED_DATA, OPERATOR, CB);
                            }

                            if (WORKING_TAPE->node->type == OPERATOR
                                &&
                                WORKING_TAPE->node->data.stat == FOB // forming open bracket
                               )
                            {
                                NEXT_TAPE;

                                recognized_node->right_son->right_son = getG();

                                if (WORKING_TAPE->node->type == OPERATOR
                                    &&
                                    WORKING_TAPE->node->data.stat == FCB // forming close brackets
                                   )
                                {
                                    recognized_node->left_son = root_node;

                                    root_node = recognized_node;

                                    NEXT_TAPE;
                                }
                            }
                            //WORKING_TAPE->node->type == USER_FUNCTION;
                        }
                        else
                        {
                            if (H_search_list_by_hash(tree, WORKING_TAPE->node->cell))
                                syntax_error_handler(WORKING_TAPE, __PRETTY_FUNCTION__,
                                                     FAILED_FUNCTION_REDECLARATION);
                            else
                                syntax_error_handler(WORKING_TAPE, __PRETTY_FUNCTION__,
                                                     FAILED_ANOTHER);
                        }
                    }
                }
            }
            //WORKING_TAPE->node->type == USER_FUNCTION;
        }
        else
        {
            if (H_search_list_by_hash(tree, WORKING_TAPE->node->cell))
                syntax_error_handler(WORKING_TAPE, __PRETTY_FUNCTION__,
                                     FAILED_FUNCTION_REDECLARATION);
            else
                syntax_error_handler(WORKING_TAPE, __PRETTY_FUNCTION__,
                                     FAILED_ANOTHER);
            //   assert(0);
        }

        if (WORKING_TAPE->node->type != END_OF_TOKENS)
        {
            //printf("11%s11", WORKING_TAPE->node->cell);

            syntax_error_handler(WORKING_TAPE, __PRETTY_FUNCTION__,
                                    FAILED_TYPE, END_OF_TOKENS);
        }
    }
    else
    {
        //there was no any function in file
    }

    //printf("____%s_____", function_name);

    return root_node;
} //*/

Node* getFuncArguments(int* arg_value, const FuncParameters* func_param)
{
    assert(arg_value && func_param && func_param->func_name);

    Node* root = nullptr;

     // капсула для слияния

    DEBUG

    *arg_value = 0;

    while (WORKING_TAPE->node->type == VARIABLE)
    {
        capsule_fusioning(&WORKING_TAPE->node->cell,
                            strlen(WORKING_TAPE->node->cell), func_param);

        if (H_search_list_by_hash(tree, WORKING_TAPE->node->cell))
            syntax_error_handler(WORKING_TAPE, __PRETTY_FUNCTION__,
                                    FAILED_VAR_REDECLARATION);
        else
            H_list_insert(tree, 0, WORKING_TAPE->node->cell, V_VARIABLE);

        WORKING_TAPE->node->left_son = root;

        root = WORKING_TAPE->node;

        NEXT_TAPE;

        ++(*arg_value);

        /*if (WORKING_TAPE->node->type == OPERATOR
            &&
            WORKING_TAPE->node->data.stat == COMMA
           )
            NEXT_TAPE*/
    }

    return root;
}

char* capsule_fusioning(char** cells_name, const size_t cells_length,
                            const FuncParameters* func_param)
{
    assert(cells_name && *cells_name && func_param);

    char* fusion_capsule = (char*) realloc(*cells_name,
                                    sizeof(char) * ( cells_length + func_param->func_length + 2));

    assert(fusion_capsule);

    fusion_capsule[cells_length] = '_';

    strcpy(&fusion_capsule[cells_length + 1], func_param->func_name);

    *cells_name = fusion_capsule;   // double return for
                                    // argument and return
    return fusion_capsule;          // version
}
