# apg_data_structures #

A place for manually writing data structures from time to time to refresh my
memory or to copy-paste as starter code into new projects.

## Motivation ##

* supports C99
* reliable run-time dynamic deletion (e.g. delete item whilst still traversing
data structure in a loop)
* memory accounting (for stats and leak checks)
* validation and safety checks
* debugging hooks

## Current Features ##

* singly-linked list
    * add node to front
    * insert after node
    * delete node
    * find end node
    * recursive delete nodes/list
* doubly-linked list
    * add node to front
    * insert after node
    * delete node
    * find end node
    * recursive delete nodes/list
* print memory allocated/freed
* test program for all of the above

## To Do ##

* binary trees, quadtree
* ADT list, etc.
* dynamic arrays (contrast to C99s)
* search and sort algorithms, hash tables -> apg_algorithms.h