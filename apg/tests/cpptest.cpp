/*
This test just makes sure a C++ compiler can compile with the apg.h code.

Note that I cut out the implementation section from apg.h into apg.c to make this easier to compile,
since the implementation is _not_ a valid C subset of C++, and cannot be compiled with a C++ compiler.

COMPILE:
-------------------
gcc -o apg.o -c .\apg.c -I .. -D APG_NO_BACKTRACES
g++ -o cpptest cpptest.cpp apg.o  -I ..
*/

#include "apg.h" // Don't do #define APG_IMPLEMENTATION since it isn't a C++ subset.

#include <iostream>

int main() {
  apg_file_t record;
  bool cpp_found = apg_file_size( "cpptest.cpp" );

  if ( cpp_found ) {
    std::cout << "Found `cpptest.cpp` in CWD!" << std::endl;
  } else {
    std::cout << "Did NOT find file `cpptest.cpp` in CWD!" << std::endl;
  }

  return 0;
}
