#apg_maths#

Minimal maths functions header for 3d graphics.
Functions designed to resemble GLSL and GLU.
Column-major matrix operations and memory layout.

* `apg_maths.hpp` - the C++ original
* `apg_maths.js` - mostly complete port to JavaScript for use with WebGL
* `apg_maths.h` - new C99 port with slightly revised interface
* `test.c` - test for the C99 header

The C++ version is the easiest to use because of operator overloading.

##data types##

* `vec2` 2d vector
* `vec3` 3d vector
* `vec4` 4d vector
* `mat3` 3x3 matrix - memory stored in column-major order
* `mat4` 4x4 matrix - memory stored in column-major order
* `versor` quaternion

Data is stored in simple 1d arrays of floats inside data types. I didn't bother
with messy recreation of myvector.x access.

To access the x component of a vector:

    vec3 v = vec3 (1.0f, 0.0f, 0.0f);
    float x = v.v[0];

To access the row 0, col 1 of a 4X4 matrix:

    float f = my_matrix.m[4];

##functions##

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

Matrix functions assume column-major layouts

##future improvements##

I've mostly made these improvements in the new C99 version.

* remove matrix parameters from matrix construction functions, because in
practice it's almost always a pointless multiplication with an identity matrix
* replace vector memory array with `.x` `.y` `.z` and `.w` - downside is that passing
whole vector memory blocks assumes contiguous memory order, which may not
always be true on some compilers (should be fine in C99 though according to spec).
A `return_mem()` function could address this. Alternatively, could add `.x()` functions in C++.
* remove matrix constructors, because memory order there is a point of confusion
* a matrix function that returns an element given a specified row and column
(rather than using the 1d index number)
* function to project a vector orthogonally onto another vector (dot prod etc)
* double check C matrix multiplications - argument order seems backward

##design ideas

* it would also be a good idea to make a much leaner library using using arrays of floats for vectors, matrices and no custom structures, but I didn't beacause it doesn't read as nicely, you lose type protection, you can't return an array from a function as nicely as one in a struct, and passing arrays as function parameters is a very messy business in C/C++.
* there has to be a nicer way to do vector and matrix multiplication in C99, even if it means using macros, because the `mult_mat4_vec4(identity_mat4(), vec4_from_4f(1.0,1.0,1.0,1.0))` very explicity style of functions are extremel tedious, and in fact this is the number 1 argument for choosing C++ over C in a 3d graphics programme, which is a shame.
