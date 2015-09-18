//
// apg_mframes.h - frame-based memory allocator
// first v. 2 sept 2015 - anton gerdelan
// written for C99
//

#pragma once
#include <stdlib.h>
#include <stdbool.h>

// push new frame onto stack
bool apg_open_frame (size_t sz);

// pop top frame
bool apg_close_frame ();

void apg_print_stack_stats ();

