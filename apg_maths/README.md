# apg_maths #

Minimal maths functions for 3d graphics.
Functions designed to resemble GLSL and GLU.
Column-major matrix operations and memory layout.

* `apg_maths.h,.c` - C99
* `apg_maths.js` - mostly complete port to JavaScript for use with WebGL
* `test.c` - tests for the C99 functions.

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

* `print()` for each data type (contents)
* vector length, squared length, normalisation, dot and cross products
* some converstions between direction vectors and angles
* matrix identity, inverse, determinant, transpose
* affine matrix construction
* virtual camera matrix construction in GLU format (look at etc.)
* quaternion construction from axis-angle
* unit quaternion (versor) normalisation etc.
* versor to matrix
* slerp (spherical interpolation)
