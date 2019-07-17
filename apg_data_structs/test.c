// apg_data_structures test
// https://github.com/capnramses/apg_data_structures
// First version 13 April 2015, by Anton Gerdelan
// Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland
// gerdela AT scss DOT tcd DOT ie

#include "apg_data_structs.h"
#include <stdio.h>
#include <assert.h>

int main() {
  printf( "apg_data_structures test\n" );
  printf( "sizeof llist_node_t %i\nsizeof dllist_node_t %i\n", (int)sizeof( llist_node_t ), (int)sizeof( dllist_node_t ) );
  {
    printf( "\n0) linked list:\n" );
    llist_node_t* llist_list_ptr = NULL;
    // add nodes
    char data = 'a';
    llist_add_to_front( &llist_list_ptr, &data, 1 );
    data = 'b';
    llist_add_to_front( &llist_list_ptr, &data, 1 );
    data = 'c';
    llist_add_to_front( &llist_list_ptr, &data, 1 );
    data                = 'd';
    llist_node_t* d_ptr = llist_add_to_front( &llist_list_ptr, &data, 1 );
    data                = 'e';
    llist_add_to_front( &llist_list_ptr, &data, 1 );
    data = 'f';
    llist_add_to_front( &llist_list_ptr, &data, 1 );
    // add g 'after' d
    data                = 'g';
    llist_node_t* g_ptr = llist_insert_after( d_ptr, &data, 1 );
    printf( "contents of list:\n" );
    llist_node_t* p = llist_list_ptr;
    while ( p ) {
      printf( "  %c\n", *(char*)p->data );
      p = p->next;
    }
    print_mem_allocd();
    // remove a node from front, end, middle
    // delete 'f'
    assert( llist_delete_node( &llist_list_ptr, llist_list_ptr ) );
    // delete 'a'
    llist_node_t* end_ptr = llist_find_end_node( llist_list_ptr );
    assert( llist_delete_node( &llist_list_ptr, end_ptr ) );
    // delete 'd'
    assert( llist_delete_node( &llist_list_ptr, d_ptr ) );
    printf( "contents of list:\n" );
    p = llist_list_ptr;
    while ( p ) {
      printf( "  %c\n", *(char*)p->data );
      p = p->next;
    }
    print_mem_allocd();
    // delete the entire list
    assert( llist_recursive_delete( &llist_list_ptr ) );
    printf( "contents of list:\n" );
    p = llist_list_ptr;
    while ( p ) {
      printf( "  %c\n", *(char*)p->data );
      p = p->next;
    }
    print_mem_allocd();
  }
  ///////////////////////////////////////////////////////////////////////////
  {
    printf( "\n1) doubly-linked list:\n" );
    dllist_node_t* dllist_list_ptr = NULL;
    // add nodes
    char data = 'a';
    dllist_add_to_front( &dllist_list_ptr, &data, 1 );
    data = 'b';
    dllist_add_to_front( &dllist_list_ptr, &data, 1 );
    data = 'c';
    dllist_add_to_front( &dllist_list_ptr, &data, 1 );
    data                 = 'd';
    dllist_node_t* d_ptr = dllist_add_to_front( &dllist_list_ptr, &data, 1 );
    data                 = 'e';
    dllist_add_to_front( &dllist_list_ptr, &data, 1 );
    data = 'f';
    dllist_add_to_front( &dllist_list_ptr, &data, 1 );
    // add g 'after' d
    data                 = 'g';
    dllist_node_t* g_ptr = dllist_insert_after( d_ptr, &data, 1 );
    printf( "contents of list:\n" );
    dllist_node_t* p = dllist_list_ptr;
    while ( p ) {
      printf( "  %c\n", *(char*)p->data );
      p = p->next;
    }
    print_mem_allocd();
    // remove a node from front, end, middle
    // delete 'f'
    assert( dllist_delete_node( &dllist_list_ptr, dllist_list_ptr ) );
    // delete 'a'
    dllist_node_t* end_ptr = dllist_find_end_node( dllist_list_ptr );
    assert( dllist_delete_node( &dllist_list_ptr, end_ptr ) );
    // delete 'd'
    assert( dllist_delete_node( &dllist_list_ptr, d_ptr ) );
    printf( "contents of list:\n" );
    p = dllist_list_ptr;
    while ( p ) {
      printf( "  %c\n", *(char*)p->data );
      p = p->next;
    }
    print_mem_allocd();
    // delete the entire list
    assert( dllist_recursive_delete( &dllist_list_ptr ) );
    printf( "contents of list:\n" );
    p = dllist_list_ptr;
    while ( p ) {
      printf( "  %c\n", *(char*)p->data );
      p = p->next;
    }
    print_mem_allocd();
  }
  ///////////////////////////////////////////////////////////////////////////
#if 0
	{
		printf ("\n2) hash table length 16:\n");
		int k = 16;
		printf ("dog = %i\n", hash_index ("dog", k));
		printf ("god = %i\n", hash_index ("god", k));
		printf ("cat = %i\n", hash_index ("cat", k));
		printf ("hamster = %i\n", hash_index ("hamster", k));
		printf ("albatross = %i\n", hash_index ("albatross", k));
		printf ("unfavourable stare = %i\n", hash_index ("unfavourable stare", k));
		printf ("anton = %i\n", hash_index ("anton", k));
		printf ("is = %i\n", hash_index ("is", k));
		printf ("and outstanding gentleman, and incredibly humble = %i\n",
			hash_index ("and outstanding gentleman, and incredibly humble", k));
		printf ("suddenly = %i\n", hash_index ("suddenly", k));
		printf ("it = %i\n", hash_index ("it", k));
		printf ("sprang = %i\n", hash_index ("sprang", k));
		printf ("from = %i\n", hash_index ("from", k));
	}
#endif
  {
    int u = 1000, v = 250;
    reduce_frac( &u, &v );
    printf( "1000/250 reduces to %i/%i\n", u, v );
  }
  return 0;
}
