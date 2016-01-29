#apg_maths#

Minimal maths functions header for 3d graphics.
Functions designed to resemble GLSL and GLU.
Column-major matrix operations and memory layout.

* `apg_maths.hpp` - the C++ original
* `apg_maths.js` - mostly complete port to JavaScript for use with WebGL
* `apg_maths.h` - new C99 port with slightly revised interface
* `test.c` - tests for the C99 header
* `test.cpp` - tests for the C++ header

The C++ version is the easiest to use because of operator overloading.

##data types##

* `vec2` 2d vector
* `vec3` 3d vector
* `vec4` 4d vector
* `mat3` 3x3 matrix - memory stored in column-major order
* `mat4` 4x4 matrix - memory stored in column-major order
* `versor` quaternion

See inside top of file for instructions on usage (differs a little between
ports).

##functions

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

##future improvements

##design ideas

* there has to be a nicer way to do vector and matrix multiplication in C99, even if it means using macros, because the `mult_mat4_vec4(identity_mat4(), vec4_from_4f(1.0,1.0,1.0,1.0))` very explicit style of functions are extremely tedious, and in fact this is the number 1 argument for choosing C++ over C in a 3d graphics programme, which is a shame.
