//
// apg_data_structures: light-weight C data structures
// https://github.com/capnramses/apg_data_structures
// First version 13 April 2015, by Anton Gerdelan
// Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland
// gerdela AT scss DOT tcd DOT ie
//

/* things to think about
* using a proxy malloc that grabs off a custom [e.g. pooled] memory manager
with existing malloc'd blocks
*/

#ifndef _APG_DATA_STRUCTURES_H_
#define _APG_DATA_STRUCTURES_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h> // UINT_MAX

//
// size in bytes of all memory so far allocated with this library
size_t g_apg_ds_mem_allocd;

//
// size in butes of memory in all linked list nodes
size_t g_apg_ll_mem_allocd;

/////////////////////////////// Linked Lists //////////////////////////////////
// Indexing:                   O(n) vs. array which is O(1)
// Insert/delete at beginning: O(1) -- best!
// Insert/delete at end:       O(n) or O(1) if we keep an end ptr
// Insert/delete in middle:    O(1) + search time (shorter for doubly linked)
// Wasted space (avg):         O(n) vs. array which is O(0)

//
// singly-linked list node, size 24 bytes
typedef struct SLL_Node SLL_Node;
struct SLL_Node {
	void* data;
	// size of the block allocated to data
	size_t sz;
	// this ptr is 8 bytes on my 64-bit machine
	SLL_Node* next;
};

//
// doubly-linked list node, size 32 bytes
typedef struct DLL_Node DLL_Node;
struct DLL_Node {
	void* data;
	// size of the block allocated to data
	size_t sz;
	DLL_Node* next;
	DLL_Node* prev;
};

//
// binary tree node
typedef struct BinTree_Node BinTree_Node;
struct BinTree_Node {
	void* data;
	size_t sz;
	BinTree_Node* left;
	BinTree_Node* right;
};

// UNDER CONSTRUCTION
// a container for a malloc block used for arrays with a size and counter
// TODO malloc, free, realloc
// TODO research C99 and GNU/glibc's dynamic array thingies
struct Dyn_Array {
	void* data;
	size_t sz;
	size_t data_type_sz;
	long int count;
};

///////////////////////////////// accounting //////////////////////////////////
void print_mem_allocd ();
/////////////////////////////// dynamic array /////////////////////////////////
Dyn_Array* dyn_array_alloc (size_t data_type_sz, long int reserve_length);
bool dyn_array_free ();
//////////////////////////// singly linked lists //////////////////////////////
SLL_Node* sll_add_to_front (SLL_Node** list_ptr, const void* data, size_t sz);
SLL_Node* sll_insert_after (SLL_Node* prev_ptr, const void* data, size_t sz);
bool sll_delete_node (SLL_Node** list_ptr, SLL_Node* ptr);
SLL_Node* sll_find_end_node (SLL_Node* list_ptr);
bool sll_recursive_delete (SLL_Node** ptr);
//////////////////////////// doubly linked lists //////////////////////////////
DLL_Node* dll_add_to_front (DLL_Node** list_ptr, const void* data, size_t sz);
DLL_Node* dll_insert_after (DLL_Node* prev_ptr, const void* data, size_t sz);
bool dll_delete_node (DLL_Node** list_ptr, DLL_Node* ptr);
DLL_Node* dll_find_end_node (DLL_Node* list_ptr);
bool dll_recursive_delete (DLL_Node** ptr);
//////////////////////////////// hash tables //////////////////////////////////
unsigned int hash_index (const char* str, int k);

//
// prints memory stats
//
void print_mem_allocd () {
	printf ("memory allocated in apg_data_structures:\n");
	printf ("g_apg_ds_mem_allocd: %16i bytes\n", (int)g_apg_ds_mem_allocd);
	printf ("  g_apg_ll_mem_allocd: %13i bytes\n", (int)g_apg_ds_mem_allocd);
}

//
// add a node to the front of a singly-linked list
// list_ptr is the pointer to the front of the list or null
// data is the contents to hold in the node
// sz is the size of that data in bytes
// returns ptr to new node or NULL on error
// note: data pointer is not freed by this function
//
inline SLL_Node* sll_add_to_front (SLL_Node** list_ptr, const void* data,
	size_t sz) {
	SLL_Node* node = (SLL_Node*)malloc (sizeof (SLL_Node));
	if (!node) {
		fprintf (stderr, "ERROR: could not alloc memory for ssl node struct\n");
		return NULL;
	}
	g_apg_ll_mem_allocd += sizeof (SLL_Node);
	node->next = *list_ptr;
	node->data = malloc (sz);
	g_apg_ll_mem_allocd += sz;
	node->sz = sz;
	node->data = memcpy (node->data, data, sz);
	*list_ptr = node;
	g_apg_ds_mem_allocd = g_apg_ds_mem_allocd + sizeof (SLL_Node) + sz;
	return node;
}

//
// add a node after another node of a singly-linked list
// prev_ptr is the pointer to the node to go after
// data is the contents to hold in the node
// sz is the size of that data in bytes
// returns ptr to new node or NULL on error
//
inline SLL_Node* sll_insert_after (SLL_Node* prev_ptr, const void* data,
	size_t sz) {
	// this is far more likely to be a user mistake than anything - should warn
	if (!prev_ptr) {
		fprintf (stderr, "ERROR: could not insert sll node, prev_ptr was NULL\n");
		return NULL;
	}
	SLL_Node* node = (SLL_Node*)malloc (sizeof (SLL_Node));
	if (!node) {
		fprintf (stderr, "ERROR: could not alloc memory for sll node struct\n");
		return NULL;
	}
	g_apg_ll_mem_allocd += sizeof (SLL_Node);
	node->next = prev_ptr->next;
	node->data = malloc (sz);
	g_apg_ll_mem_allocd += sz;
	node->sz = sz;
	node->data = memcpy (node->data, data, sz);
	prev_ptr->next = node;
	g_apg_ds_mem_allocd = g_apg_ds_mem_allocd + sizeof (SLL_Node) + sz;
	return node;
}

//
// delete node pointed to by ptr
// function searches list from list_ptr to ptr to find prev node
// prev->next will then point to ptr->next
// ptr is freed and then set to NULL
// list_ptr is a pointer to the start of the list (or any node prior to ptr)
// ptr is the node to delete
// returns false on error
//
inline bool sll_delete_node (SLL_Node** list_ptr, SLL_Node* ptr) {
	if (!*list_ptr) {
		fprintf (stderr, "ERROR: can not delete sll node, list_ptr is NULL\n");
		return false;
	}
	if (!ptr) {
		fprintf (stderr, "ERROR: can not delete sll node, ptr is NULL\n");
		return false;
	}
	size_t sz = ptr->sz;
	
	// find prev node to ptr so can adjust
	SLL_Node* p = *list_ptr;
	while (p) {
		// p is first node in list, so adjust list ptr
		if (p == ptr) {
			*list_ptr = ptr->next;
			break;
		}
		// make prev->next equal to ptr->next
		if (p->next == ptr) {
			p->next = ptr->next;
			break;
		}
		p = p->next;
	} // endwhile
	
	free (ptr->data);
	ptr->data = NULL; // pointless, not used again
	free (ptr);
	ptr = NULL; // pointless, not used again
	g_apg_ll_mem_allocd = g_apg_ll_mem_allocd - sz - sizeof (SLL_Node);
	g_apg_ds_mem_allocd = g_apg_ds_mem_allocd - sz - sizeof (SLL_Node);
	
	return true;
}

//
// list_ptr is any node in the list - the farther along, the shorter the
// search
// returns NULL if list is empty
//
inline SLL_Node* sll_find_end_node (SLL_Node* list_ptr) {
	if (!list_ptr) {
		return NULL;
	}
	SLL_Node* p = list_ptr;
	while (p) {
		if (!p->next) {
			break;
		}
		p = p->next;
	}
	return p;
}

//
// recursively deletes and entire list, starting from ptr
// sets ptr to NULL afterwards
// returns false on error
// note: figured there was no point inlining this
//
bool sll_recursive_delete (SLL_Node** ptr) {
	if (!*ptr) {
		fprintf (stderr, "ERROR: could not recursive delete sll, node was NULL\n");
		return false;
	}
	if ((*ptr)->next) {
		sll_recursive_delete (&(*ptr)->next);
	}
	size_t sz = (*ptr)->sz;
	free ((*ptr)->data);
	(*ptr)->data = NULL; // pointless, not used again
	free (*ptr);
	*ptr = NULL; // pointless, not used again
	g_apg_ll_mem_allocd = g_apg_ll_mem_allocd - sz - sizeof (SLL_Node);
	g_apg_ds_mem_allocd = g_apg_ds_mem_allocd - sz - sizeof (SLL_Node);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

inline DLL_Node* dll_add_to_front (DLL_Node** list_ptr, const void* data,
	size_t sz) {
	DLL_Node* node = (DLL_Node*)malloc (sizeof (DLL_Node));
	if (!node) {
		fprintf (stderr, "ERROR: could not alloc memory for dll node struct\n");
		return NULL;
	}
	g_apg_ll_mem_allocd += sizeof (DLL_Node);
	node->next = *list_ptr;
	if (node->next) {
		node->next->prev = node;
	}
	node->prev = NULL;
	node->data = malloc (sz);
	g_apg_ll_mem_allocd += sz;
	node->sz = sz;
	node->data = memcpy (node->data, data, sz);
	*list_ptr = node;
	g_apg_ds_mem_allocd = g_apg_ds_mem_allocd + sizeof (DLL_Node) + sz;
	return node;
}

inline DLL_Node* dll_insert_after (DLL_Node* prev_ptr, const void* data, size_t sz) {
	// this is far more likely to be a user mistake than anything - should warn
	if (!prev_ptr) {
		fprintf (stderr, "ERROR: could not insert dll node, prev_ptr was NULL\n");
		return NULL;
	}
	DLL_Node* node = (DLL_Node*)malloc (sizeof (DLL_Node));
	if (!node) {
		fprintf (stderr, "ERROR: could not alloc memory for dll node struct\n");
		return NULL;
	}
	g_apg_ll_mem_allocd += sizeof (DLL_Node);
	DLL_Node* next = prev_ptr->next;
	prev_ptr->next = node;
	node->next = next;
	if (next) {
		next->prev = node;
	}
	node->prev = prev_ptr;
	node->data = malloc (sz);
	g_apg_ll_mem_allocd += sz;
	node->sz = sz;
	node->data = memcpy (node->data, data, sz);
	g_apg_ds_mem_allocd = g_apg_ds_mem_allocd + sizeof (DLL_Node) + sz;
	return node;
}

inline bool dll_delete_node (DLL_Node** list_ptr, DLL_Node* ptr) {
	if (!*list_ptr) {
		fprintf (stderr, "ERROR: can not delete dll node, list_ptr is NULL\n");
		return false;
	}
	if (!ptr) {
		fprintf (stderr, "ERROR: can not delete dll node, ptr is NULL\n");
		return false;
	}
	size_t sz = ptr->sz;
	
	// p is first node in list, so adjust list ptr
	if (!ptr->prev) {
		*list_ptr = ptr->next;
		if (ptr->next) {
			ptr->next->prev = NULL;
		}
	} else {
		ptr->prev->next = ptr->next;
		// this only applies if not deleting last node
		if (ptr->next) {
			ptr->next->prev = ptr->prev;
		}
	}
	free (ptr->data);
	ptr->data = NULL; // pointless, not used again
	free (ptr);
	ptr = NULL; // pointless, not used again
	g_apg_ll_mem_allocd = g_apg_ll_mem_allocd - sz - sizeof (DLL_Node);
	g_apg_ds_mem_allocd = g_apg_ds_mem_allocd - sz - sizeof (DLL_Node);
	
	return true;
}

inline DLL_Node* dll_find_end_node (DLL_Node* list_ptr) {
	if (!list_ptr) {
		return NULL;
	}
	DLL_Node* p = list_ptr;
	while (p) {
		if (!p->next) {
			break;
		}
		p = p->next;
	}
	return p;
}

bool dll_recursive_delete (DLL_Node** ptr) {
	if (!*ptr) {
		fprintf (stderr, "ERROR: could not recursive delete dll, node was NULL\n");
		return false;
	}
	if ((*ptr)->next) {
		dll_recursive_delete (&(*ptr)->next);
	}
	size_t sz = (*ptr)->sz;
	free ((*ptr)->data);
	(*ptr)->data = NULL; // pointless, not used again
	free (*ptr);
	*ptr = NULL; // pointless, not used again
	g_apg_ll_mem_allocd = g_apg_ll_mem_allocd - sz - sizeof (DLL_Node);
	g_apg_ds_mem_allocd = g_apg_ds_mem_allocd - sz - sizeof (DLL_Node);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

//
// k is size of hash table
//
inline unsigned int hash_index (const char* str, int k) {
	unsigned int len = strlen (str);
	unsigned int sum = 0;
	unsigned int p = UINT_MAX;
	for (unsigned int i = 0; i < len; i++) {
		// multiply by index so god != dog
		// and p so returned values are sufficiently large and well spaced
		unsigned int val = str[i] * (i + p);
		sum += val;
	}
	unsigned int h = sum % k;
	// could check here for table collision and move to next index along (and
	// record that somewhere in a look-up list for this function)
	return h;
}

///////////////////////////////////////////////////////////////////////////////



#endif
