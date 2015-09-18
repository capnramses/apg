//
// apg_data_structures test
// https://github.com/capnramses/apg_data_structures
// First version 13 April 2015, by Anton Gerdelan
// Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland
// gerdela AT scss DOT tcd DOT ie
//

#include "apg_data_structures.h"
#include <stdio.h>
#include <assert.h>

int main () {
	printf ("apg_data_structures test\n");
	printf ("sizeof SLL_Node %i\nsizeof DLL_Node %i\n", (int)sizeof (SLL_Node),
		(int)sizeof (DLL_Node));
	{
		printf ("\n0) linked list:\n");
		SLL_Node* sll_list_ptr = NULL;
		// add nodes
		char data = 'a';
		sll_add_to_front (&sll_list_ptr, &data, 1);
		data = 'b';
		sll_add_to_front (&sll_list_ptr, &data, 1);
		data = 'c';
		sll_add_to_front (&sll_list_ptr, &data, 1);
		data = 'd';
		SLL_Node* d_ptr = sll_add_to_front (&sll_list_ptr, &data, 1);
		data = 'e';
		sll_add_to_front (&sll_list_ptr, &data, 1);
		data = 'f';
		sll_add_to_front (&sll_list_ptr, &data, 1);
		// add g 'after' d
		data = 'g';
		SLL_Node* g_ptr = sll_insert_after (d_ptr, &data, 1);
		printf ("contents of list:\n");
		SLL_Node* p = sll_list_ptr;
		while (p) {
			printf ("  %c\n", *(char*)p->data);
			p = p->next;
		}
		print_mem_allocd ();
		// remove a node from front, end, middle
		// delete 'f'
		assert (sll_delete_node (&sll_list_ptr, sll_list_ptr));
		// delete 'a'
		SLL_Node* end_ptr = sll_find_end_node (sll_list_ptr);
		assert (sll_delete_node (&sll_list_ptr, end_ptr));
		// delete 'd'
		assert (sll_delete_node (&sll_list_ptr, d_ptr));
		printf ("contents of list:\n");
		p = sll_list_ptr;
		while (p) {
			printf ("  %c\n", *(char*)p->data);
			p = p->next;
		}
		print_mem_allocd ();
		// delete the entire list
		assert (sll_recursive_delete (&sll_list_ptr));
		printf ("contents of list:\n");
		p = sll_list_ptr;
		while (p) {
			printf ("  %c\n", *(char*)p->data);
			p = p->next;
		}
		print_mem_allocd ();
	}
	///////////////////////////////////////////////////////////////////////////
	{
		printf ("\n1) doubly-linked list:\n");
		DLL_Node* dll_list_ptr = NULL;
		// add nodes
		char data = 'a';
		dll_add_to_front (&dll_list_ptr, &data, 1);
		data = 'b';
		dll_add_to_front (&dll_list_ptr, &data, 1);
		data = 'c';
		dll_add_to_front (&dll_list_ptr, &data, 1);
		data = 'd';
		DLL_Node* d_ptr = dll_add_to_front (&dll_list_ptr, &data, 1);
		data = 'e';
		dll_add_to_front (&dll_list_ptr, &data, 1);
		data = 'f';
		dll_add_to_front (&dll_list_ptr, &data, 1);
		// add g 'after' d
		data = 'g';
		DLL_Node* g_ptr = dll_insert_after (d_ptr, &data, 1);
		printf ("contents of list:\n");
		DLL_Node* p = dll_list_ptr;
		while (p) {
			printf ("  %c\n", *(char*)p->data);
			p = p->next;
		}
		print_mem_allocd ();
		// remove a node from front, end, middle
		// delete 'f'
		assert (dll_delete_node (&dll_list_ptr, dll_list_ptr));
		// delete 'a'
		DLL_Node* end_ptr = dll_find_end_node (dll_list_ptr);
		assert (dll_delete_node (&dll_list_ptr, end_ptr));
		// delete 'd'
		assert (dll_delete_node (&dll_list_ptr, d_ptr));
		printf ("contents of list:\n");
		p = dll_list_ptr;
		while (p) {
			printf ("  %c\n", *(char*)p->data);
			p = p->next;
		}
		print_mem_allocd ();
		// delete the entire list
		assert (dll_recursive_delete (&dll_list_ptr));
		printf ("contents of list:\n");
		p = dll_list_ptr;
		while (p) {
			printf ("  %c\n", *(char*)p->data);
			p = p->next;
		}
		print_mem_allocd ();
	}
	///////////////////////////////////////////////////////////////////////////
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
	return 0;
}
