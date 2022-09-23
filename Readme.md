# apg Libraries

Small utility libraries and copy-paste snippets of reusable code.

## Contents

| Library     | Description                                     | Language | # Files         | Version | Fuzzed With                             |
|-------------|-------------------------------------------------|----------|-----------------|---------|-----------------------------------------|
| apg         | Generic C programming utils.                    | C        | 1               | 1.10    | No                                      |
| apg_bmp     | BMP bitmap image reader/writer library.         | C        | 2               | 3.2     | [AFL](https://lcamtuf.coredump.cx/afl/) |
| apg_console | Quake-style graphical console. API-independent. | C        | 2 + apg_pixfont | 0.13    | No                                      |
| apg_jobs    | Simple worker/jobs thread pool system.          | C        | 2               | 0.2     | No                                      |
| apg_gldb    | OpenGL debug drawing (lines, boxes, ... )       | C        | 2               | 0.3     | No                                      |
| apg_interp  | Interpolation / "tweening" / "easing".          | C, JS    | 1, 1            | 0.7     | No                                      |
| apg_plot    | Quick line-plot bitmaps from 2D data series.    | C        | 2               | 0.2     | No                                      |
| apg_maths   | 3D maths for graphics/games.                    | C, JS    | 2, 1            | 0.16    | No                                      |
| apg_mod     | Unannounced work-in-progress.                   | C        | 2               | 0.1     | No                                      |
| apg_pixfont | String-to-image with a pixel font.              | C        | 2               | 0.1.1   | No                                      |
| apg_ply     | Stanford PLY mesh format read/write.            | C        |                 | ?       | No                                      |
| apg_tga     | Single-header TGA image reader/writer.          | C        | 1               | 0.3.1   | No                                      |
| apg_unicode | Unicode codepoint <-> UTF-8 String Functions.   | C        | 2               | 0.2     | No                                      |
| apg_wav     | WAV file format read/write.                     | C        | 2               | 0.2     | No                                      |

## LICENCE

Libraries in this repository are dual-licensed and may be used by you under the terms of your
choice of licence. See subsections below for licences of optional third-party components.

-------------------------------------------------------------------------------------
FIRST LICENCE OPTION

>                                  Apache License
>                            Version 2.0, January 2004
>                         http://www.apache.org/licenses/
> Copyright 2019 Anton Gerdelan.
> Licensed under the Apache License, Version 2.0 (the "License");
> you may not use this file except in compliance with the License.
> You may obtain a copy of the License at
> <http://www.apache.org/licenses/LICENSE-2.0>
> Unless required by applicable law or agreed to in writing, software
> distributed under the License is distributed on an "AS IS" BASIS,
> WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
> See the License for the specific language governing permissions and
> limitations under the License.
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

## Dependencies Used in Example Programs

Please note that some optional test and demonstration programs contained in this repository make use of third-party libraries with their own licences,
and it is the user's responsibility to also meet these requirements if you use or reproduce these works.

### PortAudio

[PortAudio](http://portaudio.com/) is used here under its V19 Licence:

> PortAudio Portable Real-Time Audio Library
> Copyright (c) 1999-2011 Ross Bencina and Phil Burk
>
> Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in > the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the > Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
> IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

### stb_image and stb_image_write

[stb](https://github.com/nothings/stb) libraries `stb_image.h` and `stb_image_write.h` are used here under the Public Domain licence option:

> This software is available under 2 licenses -- choose whichever you prefer.
>
> ALTERNATIVE A - MIT License
> Copyright (c) 2017 Sean Barrett
> Permission is hereby granted, free of charge, to any person obtaining a copy of
> this software and associated documentation files (the "Software"), to deal in
> the Software without restriction, including without limitation the rights to
> use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
> of the Software, and to permit persons to whom the Software is furnished to do
> so, subject to the following conditions:
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.
>
> ALTERNATIVE B - Public Domain (www.unlicense.org)
> This is free and unencumbered software released into the public domain.
> Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
> software, either in source code form or as a compiled binary, for any purpose,
> commercial or non-commercial, and by any means.
> In jurisdictions that recognize copyright laws, the author or authors of this
> software dedicate any and all copyright interest in the software to the public
> domain. We make this dedication for the benefit of the public at large and to
> the detriment of our heirs and successors. We intend this dedication to be an
> overt act of relinquishment in perpetuity of all present and future rights to
> this software under copyright law.
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
> ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
> WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
>
