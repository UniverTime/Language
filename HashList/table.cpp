
#include <stdlib.h>

#include "table.h"
//#include "listCPP.h"

/*int main()
{
    HashTree* tree_union = H_do_hash_table();

    H_list_insert(&tree_union[0], 0, (size_t) 12);
    H_list_insert(&tree_union[2], 0, (size_t) 56);

    HashList* list = H_search_list_by_hash(&tree_union[0], (size_t) 12);
    printf("%zu", list->hash);

    list = H_search_list_by_hash(&tree_union[2], (size_t) 56);
    printf("%zu", list->hash);


    H_tree_union_destructor(tree_union);

    return 0;
} */

HashTree* H_do_hash_table(void)
{
    HashTree* tree_union = (HashTree*) calloc(H_MAX_TABLE_SIZE, sizeof(HashTree));

    H_all_list_init(tree_union);

    return tree_union;
}

void H_tree_union_destructor(HashTree* tree_union)
{
    H_all_list_destr(tree_union);

    free(tree_union);
}

void H_all_list_init(HashTree* tree_union)
{
    assert (tree_union);

    for (size_t ind = 0; ind < H_MAX_TABLE_SIZE; ind++)
        H_list_init(&tree_union[ind], H_MAX_LIST_SIZE);
}

void H_all_list_destr(HashTree* tree_union)
{
    assert (tree_union);

    for (size_t ind = 0; ind < H_MAX_TABLE_SIZE; ind++)
        H_list_destructor(&tree_union[ind]);
}
