# apg_maths #

Minimal maths functions for 3D graphics.
Functions designed to resemble GLSL and GLU.
Column-major matrix operations and memory layout.

* `apg_maths.h,.c` - C99
* `apg_maths.js`   - A fairly complete port to JavaScript for use with WebGL
* `tests/`         - Test programs for the C99 functions.

## Data Types ##

Data types are encoded as 1D arrays in column-major memory order.

* `vec2` 2D vector.
* `vec3` 3D vector.
* `vec4` 4D vector.
* `mat3` 3x3 matrix.
* `mat4` 4x4 matrix.
* `versor` quaternion.

See inside source files for instructions on usage.

## Functions

* `print()` for each data type (contents).
* Vector length, squared length, normalisation, dot and cross products.
* Some converstions between direction vectors and angles.
* Matrix identity, inverse, determinant, transpose, rotations, and scaling.
* Affine matrix construction.
* Virtual camera matrix construction in GLU format ("look at" etc.).
* Quaternion construction from axis-angle.
* Unit quaternion (versor) normalisation etc.
* Versor to matrix converstion.
* Slerp (spherical interpolation).
