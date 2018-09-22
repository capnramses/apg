# apg_maths #

Minimal maths functions header for 3d graphics.
Functions designed to resemble GLSL and GLU.
Column-major matrix operations and memory layout.

* `apg_maths.hpp` - the C++ original
* `apg_maths.js` - mostly complete port to JavaScript for use with WebGL
* `apg_maths.h` - new C99 port with slightly revised interface
* `apg_maths_clang.h` - C version with Clang-specific attributes to add built-in vector data types
* `test.c` - tests for the C99 header
* `test.cpp` - tests for the C++ header

The C++ version is the easiest to use because of operator overloading.

## data types ##

* `vec2` 2d vector
* `vec3` 3d vector
* `vec4` 4d vector
* `mat3` 3x3 matrix - memory stored in column-major order
* `mat4` 4x4 matrix - memory stored in column-major order
* `versor` quaternion

See inside top of file for instructions on usage (differs a little between
ports).

## functions

* overloaded basic mathematical and assignment operators for data types
* print() for each data type (contents)
* vector length, squared length, normalisation, dot and cross products
* some converstions between direction vectors and angles
* matrix identity, inverse, determinant, transpose
* affine matrix construction
* virtual camera matrix construction in GLU format (look at etc.)
* quaternion construction from axis-angle
* unit quaternion (versor) normalisation etc.
* versor to matrix
* slerp (spherical interpolation)
