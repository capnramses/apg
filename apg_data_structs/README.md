#apg_data_structures#

I don't like STL data structures for several reasons, so I'm writing my own
light-weight data structures library for C. I figured this was a fairly
common case for code re-use; most likely use being 'starter' code for more
project-specific data structures.

##Motivation##

* supports C99
* reliable run-time dynamic deletion (e.g. delete item whilst still traversing
data structure in a loop)
* memory accounting (for stats and leak checks)
* validation and safety checks
* debugging hooks

##Current Features##

* singly linked list
    * add node to front
    * insert node
    * delete node
    * find end node
    * recursive delete nodes/list
* doubly linked list
    * add node to front
    * insert node
    * delete node
    * find end node
    * recursive delete nodes/list
* print memory allocated/freed
* test programme for all of the above
* simple hash table

##To Do##

* binary trees
* vector, list, etc.
* dynamic arrays
* testing/support with more interesting data contents
* possibly add sorting and search algorithms
