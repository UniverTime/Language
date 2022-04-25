
#include "../include/lexer.h"

size_t WORLD_LENGTH = 10;
size_t MAX_TOKEN_NUM = 100;
size_t NOT_EXIST_SYM = 0;

Tree* begin_lexering(const char *const file_name)
{
    assert (file_name);

    FILE* input_file = fopen(file_name, "r");

    assert (input_file);

    //��� ������ ������, ��������� ����������
    //��������� ������������

    Tree token_tree;

    list_init(&token_tree, MAX_TOKEN_NUM);

    make_token(input_file, &token_tree);

    /*fseek(input_file, 0, SEEK_END);

    size_t count = ftell(input_file);

    char* file_buffer = (char*) calloc(count, sizeof (char));

    fseek(input_file, 0, SEEK_SET);

    fread(file_buffer, sizeof (char), count, input_file);*/

    fclose(input_file);

    return &token_tree;
}

void make_token(FILE* input_file, Tree* token_tree)
{
    assert (input_file && token_tree);

    char input_c   = ' ';

    char* string = (char*) calloc(WORLD_LENGTH, sizeof (char));

    Node* current_node = nullptr;

    int i_value = 0;

    size_t n_new_line       = 0,
           cursor_position  = 0,
           str_size         = 0;

    while (input_c != EOF)
    {
        //printf("BEGIN||%d\n", input_c);

        while (input_c == ' ' || input_c == '\n')
        {
            printf("SPACE\n");

            if (input_c == '\n')
            {
                ++n_new_line;

                cursor_position = 1;
            }
            else
                ++cursor_position;

            input_c = getc(input_file);
        }

        if (input_c == EOF)
            continue;


        if (('a' <= input_c && input_c <= 'z')
            ||
            ('A' <= input_c && input_c <= 'Z')
           )
        {
            while (('a' <= input_c && input_c <= 'z')
                   ||
                   ('A' <= input_c && input_c <= 'Z')
                  )
            {
                string[str_size++] = input_c;

                input_c = getc(input_file);
            }

            string[str_size] = '\0';


            #define newfun(name, ...)                           \
                if (!strcmp(string, #name))                      \
                {                                                 \
                    current_node = new_node(FUNCTION, FUNC_##name);\
                                                                    \
                }else                                                \

            #define newoper(...)

            #include "../function"
            {
                // add new type of node
                current_node = new_node(VARIABLE);

                size_t str_len = strlen(string);

                if (str_len > 3)
                    current_node->cell = (char*) realloc(current_node->cell, (str_len+1) * sizeof (char));

                assert (current_node->cell);

                strcpy(current_node->cell, string);
            }

            #undef newfun
            #undef newoper

            str_size = 0;
        }
        else if ('0' <= input_c && input_c <= '9')
        {
            current_node = new_node(INT);

            while ('0' <= input_c && input_c <= '9')
            {
                i_value = i_value * 10 + input_c - '0';

                input_c = getc(input_file);
            }

            current_node->data.i_num = i_value;
        }
        else
        {
            switch (input_c)
            {

                #define newfun(...)

                #define newoper(name, symbol, codir,...)     \
                    case codir:                               \
                    {                                          \
                        current_node = new_node(OPERATOR, name);\
                                                                 \
                        break;                                    \
                    }


                #include "../function"

                #undef newfun
                #undef newoper

                default:
                {
                    printf("\n__%d__\n", input_c);

                    assert (NOT_EXIST_SYM);
                }
            }

            input_c = getc(input_file);
        }

        list_insert(token_tree, 0, current_node, cursor_position, n_new_line);
    }
}
