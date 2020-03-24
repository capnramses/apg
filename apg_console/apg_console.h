/* =======================================================================================================================
APG_C - A Quake-style Console mini-library
Author:   Anton Gerdelan - @capnramses

Language: C99
Status:   Working. Not tested across compilers/platforms yet. Expect warnings.
Contact:  <antongdl@protonmail.com>
Website:  https://github.com/capnramses/apg - http://antongerdelan.net/
Licence:  See bottom of this file.
Version History:
  2020/01/04 - Moved to apg libraries repository. Minor tweaks from testing in a game integration.
  2020/01/06 - Reduced interface. Moved from stored float cvars to addresses of existing vars. Data type is also specified for bool/int/uint/float support.

Instructions
============
* Just drop the 4 files into your project (console and pixel font source code).
* No external image or font assets are required. The font is hard-coded into the source code.
* This file is the console interface - #include "apg_console.h" in your application code.
* Your program handles keyboard capture and drawing.
* You then feed keyboard input to this console.
* You ask this console to render an RGBA image to memory.
* You then use that for drawing any way you like. You don't need a 3D graphics API.

The primary interface for user-entered text is:

  apg_c_append_user_entered_text( str );

Where `str` can be a whole line or eg one character at a time.
Instructions end and are entered and parsed after a line break `\n` byte.

Autocompletion of user-entered text is supported. e.g. After you application detects the TAB key is pressed call:

  apg_c_autocomplete();

Instructions may be of the following forms:

  BUILT-IN-COMMAND [VARIABLE] [VALUE]
  VARIABLE
  FUNCTION [VALUE]

Built-in commands include:

  "help"           - list built-in commands.
  "my_var 2.0"     - set the value of a variable 'my_var'.
  "my_var"         - print the value of variable 'my_var'.
  "clear"          - invoke the 'clear' command.
  "list_vars"      - print registered variables and their values to drop-down console
  "list_funcs"     - print registered variables and their values to drop-down console

Variables may also be registered, or accessed programmatically:

  apg_c_register_var( str, var_ptr, datatype )
  apg_c_get_var()

C functions can be called from console by registering a command name and function to call.
Your callback functions must be in the form `bool my_function( float arg )`. If your function returns false the console prints an error message.
User functions always require 1 float argument but if it is called from the drop-down console without supplying an argument then the argument value is set to 0.

  apg_c_register_func( str, function_ptr );

Scrolling output text may be interacted with programmatically:

  apg_c_output_clear()   - Clear the output text.
  apg_c_print( str )     - Adds a line of text to the output such as a debug message.
  apg_c_dump_to_stdout() - Writes the current console output text to stdout via printf().
  apg_c_count_lines()    - Counts the number of lines in the console output.

The console text may also be rendered out to an image for use in any graphical applications.
This is API-agnostic so must be converted to a texture to be used with 3D APIs.

  apg_c_draw_to_image_mem()       - Writes current console text on top of a pre-allocated image.
  apg_c_image_redraw_required()   - Check if anything has actually changed requiring a redraw. You should also redraw if eg the display area changes size.

TODO
~ Up arrow key scrolls command history or selects previous 1 command.
======================================================================================================================= */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define APG_C_UNUSED( x ) (void)( x ) // to suppress compiler warnings with unused/dummy arguments in callbacks
#define APG_C_STR_MAX 128             // maximum console string length. commands and variable names must be shorter than this.
#define APG_C_VARS_MAX 256            // maximum number of variables stored in console
#define APG_C_FUNCS_MAX 128           // maximum number of console commands
#define APG_C_OUTPUT_LINES_MAX 32     // maximum number of lines retained in output

/* =======================================================================================================================
user-entered text API. call these functions based on eg keyboard input.
======================================================================================================================= */

// returns true if a valid command was parsed out of str.
bool apg_c_append_user_entered_text( const char* str );
void apg_c_backspace( void );
void apg_c_autocomplete( void );
void apg_c_clear_user_entered_text( void );

/* =======================================================================================================================
console output text API.
======================================================================================================================= */

void apg_c_output_clear( void );
// Appends str as an output line to the scrolling output
void apg_c_print( const char* str );
int apg_c_count_lines( void );
// printf everything in console to stdout stream
void apg_c_dump_to_stdout( void );

/* =======================================================================================================================
program <-> console variable and function linkage API
======================================================================================================================= */

typedef enum apg_c_var_datatype_t { APG_C_BOOL, APG_C_INT32, APG_C_UINT32, APG_C_FLOAT, APG_C_OTHER } apg_c_var_datatype_t;

typedef struct apg_c_var_t {
  char str[APG_C_STR_MAX];
  void* var_ptr;
  apg_c_var_datatype_t datatype;
} apg_c_var_t;

// Registers and existing program variable with the console.
// WARNING Pointer is assumed to be valid for lifetime of console registration.
// PARAMS
//   str      - A name for the variable.
//   var_ptr  - Pointer to variable to register and use for this c_var.
//   datatype - Data type of the variable pointed to by var_ptr. Determines how values can be set from the console.
// RETURNS
//   false if failed ie not space left in array of var ptrs.
bool apg_c_register_var( const char* str, void* var_ptr, apg_c_var_datatype_t datatype );

// Fetches the address of a console variable with name `str`.
// RETURNS
//   NULL if the variable does not exist.
//   Address of the c_var entry with pointer and data type.
apg_c_var_t* apg_c_get_var( const char* str );

bool apg_c_register_func( const char* str, bool ( *fptr )( float ) );

/* =======================================================================================================================
rendering API
======================================================================================================================= */

// Draw the current console text into an image buffer you have allocated with dimensions w, h, and n_channels.
// The destination image does not need to exactly match the console text size - it can be bigger or smaller.
// Text starts drawing at the bottom-left of the provided image, and lines scroll upwards.
// PARAMETERS
//   img_ptr    - pointer to the destination image bytes. must not be NULL
//   w,h        - dimensions of the destination image in pixels
//   n_channels - number of channels in the destination image
// RETURNS
//   false on any failure
bool apg_c_draw_to_image_mem( uint8_t* img_ptr, int w, int h, int n_channels, uint8_t* background_colour );

// RETURNS true if console has changed since last call to apg_c_draw_to_image_mem() and can be painted again
bool apg_c_image_redraw_required();

#ifdef __cplusplus
}
#endif

/*
-------------------------------------------------------------------------------------
This software is available under two licences - you may use it under either licence.
-------------------------------------------------------------------------------------
FIRST LICENCE OPTION

                                 Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/
   Copyright 2019 Anton Gerdelan.
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
-------------------------------------------------------------------------------------
SECOND LICENCE OPTION

Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-------------------------------------------------------------------------------------
*/
