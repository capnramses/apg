# apg Libraries

Small utility libraries and copy-paste snippets of reusable code.

[![Build Status](https://travis-ci.com/capnramses/apg.svg?branch=master)](https://travis-ci.com/capnramses/apg)

## Contents

| Library     | Description                                       | Language | # Files         | Version    | Fuzzed                                        |
| ----------- | ------------------------------------------------- | -------- | --------------- | ---------- | --------------------------------------------- |
| apg         | Generic C programming utils.                      | C        | 1               | 1.2        | No                                            |
| apg_bmp     | BMP bitmap image reader/writer library.           | C        | 2               | 3.1        | Yes - [AFL](https://lcamtuf.coredump.cx/afl/) |
| apg_console | Quake-style graphical console. API-independent.   | C        | 2 + apg_pixfont | 2020/01/06 | No                                            |
| apg_gldb    | OpenGL debug drawing (lines, boxes, ... ) | C        | 2               | 0.3        | No                                            |
| apg_interp  | Interpolation / "tweening" / "easing".            | C, JS    | 1, 1            | 0.7        | No                                            |
| apg_maths   | 3D maths for graphics/games.                      | C, JS    | 1, 1            | 2.10       | No                                            |
| apg_pixfont | String-to-image with a pixel font.                | C        | 2               | 0.0.5      | No                                            |
| apg_ply     | Stanford PLY mesh format read/write.              | C        |                 | ?          | No                                            |
| apg_tga     | Single-header TGA image reader/writer.            | C        | 1               | 0.3.1      | No                                            |
| apg_unicode | Unicode codepoint <-> UTF-8 String Functions.     | C        | 2               | 0.2        | No                                            |
| apg_wav     | WAV file format read/write.                       | C        | 2               | 1          | No                                            |

## LICENCE

Some files in this repository may be from third-party libraries.
It is the user's responsibility to determine and respect the licences of these files.
All other files are are dual-licenced and may be used by you under the terms of your
choice of licence:

-------------------------------------------------------------------------------------
FIRST LICENCE OPTION

>                                  Apache License
>                            Version 2.0, January 2004
>                         http://www.apache.org/licenses/
>    Copyright 2019 Anton Gerdelan.
>    Licensed under the Apache License, Version 2.0 (the "License");
>    you may not use this file except in compliance with the License.
>    You may obtain a copy of the License at
>        http://www.apache.org/licenses/LICENSE-2.0
>    Unless required by applicable law or agreed to in writing, software
>    distributed under the License is distributed on an "AS IS" BASIS,
>    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
>    See the License for the specific language governing permissions and
>    limitations under the License.
-------------------------------------------------------------------------------------
SECOND LICENCE OPTION

> This is free and unencumbered software released into the public domain.
>
> Anyone is free to copy, modify, publish, use, compile, sell, or
> distribute this software, either in source code form or as a compiled
> binary, for any purpose, commercial or non-commercial, and by any
> means.
> 
> In jurisdictions that recognize copyright laws, the author or authors
> of this software dedicate any and all copyright interest in the
> software to the public domain. We make this dedication for the benefit
> of the public at large and to the detriment of our heirs and
> successors. We intend this dedication to be an overt act of
> relinquishment in perpetuity of all present and future rights to this
> software under copyright law.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
> EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
> MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
> IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
> OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
> ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
> OTHER DEALINGS IN THE SOFTWARE.
> 
> For more information, please refer to <http://unlicense.org>
-------------------------------------------------------------------------------------
