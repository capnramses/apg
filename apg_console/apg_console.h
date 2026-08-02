/* =======================================================================================================================
APG_C - A Quake-style Console mini-library

Author:          Anton Gerdelan - @capnramses
Language:        C99
Status:          Working. Not tested across compilers/platforms yet. Expect warnings.
Contact:         <antongdl@protonmail.com>
Website:         https://github.com/capnramses/apg - http://antongerdelan.net/
Licence:         See bottom of this file.
Version History:

  v0.18   - 2026/08/02 - Callbacks can have >1 arg.
  v0.17.1 - 2025/09/18 - Removed unused vars. Tidy comments.
  v0.17   - 2025/08/24 - Improved autocomplete.
  v0.16   - 2025/08/01 - Remove apg_pixfont dependency & rendering API & coloured text.
  v0.15.2 - 2025/06/13 - Replaced unsafe string functions.
  v0.15   - 2025/01/05 - Better autocomplete.
  v0.14   - 2024/06/15 - Unicode-aware backspace.
  v0.13.1 - 2023/04/19 - Small function prototype fixes.
  v0.13   - 2022/09/23 - Updated API to match new apg_pixfont.
  v0.12   - 2021/10/03 - Fix to missing nul-terminator in long string copies.
  v0.11   - 2021/06/09 - Fixes to warnings reported by MSVC.
  v0.10   - 2021/06/05 - Fixed strncat warnings.
  v0.9    - 2020/11/08 - Added functions to iteratively scroll through history like in BaSh.
  v0.8    - 2020/09/13 - Removed assert on append empty string. Reduced max str len to suit shorter consoles.
  v0.7    - 2020/08/09 - Fixed bug where counter of number of built in commands was too high by 1. Updated docs in this file.
  v0.6    - 2020/08/08 - Fixed issue/bug where dumping to stdout didn't print current user entered text if history was empty.
  v0.5    - 2020/08/08 - apc_c_printf_rgba() to set the colour of particular lines. History text is now grey by default, and cursor text is white.
  v0.4    - 2020/08/08 - Added printf-style variadic arguments and renamed apc_c_print() to apc_c_printf().
  v0.3    - 2020/06/07 - Doubled length of tmp strings to avoid truncation warnings.
  v0.2    - 2020/01/04 - Moved to apg libraries repository. Minor tweaks from testing in a game integration.
  v0.1    - 2020/01/06 - Reduced iface. Moved from stored float cvars to addresses of existing vars. Data type is also specified for bool/int/uint/float support.

Instructions
============

The primary interface for user-entered text is:

  apg_c_append_user_entered_text( str );

Where `str` can be a whole line or eg one character at a time.
Instructions end and are entered and parsed after a line break `\n` byte.
Autocompletion of user-entered text is supported. e.g. After you application detects the TAB key is pressed call:

  apg_c_autocomplete();

Instructions may be of the following forms:

  BUILT-IN-COMMAND
  VARIABLE         - print the value of a registered variable to the drop-down console
  VARIABLE VALUE   - change the value of a registered variable
  FUNCTION [VALUE] - execute a registered function with an optional argument value

Built-in commands include:

  "help"           - list built-in commands.
  "clear"          - invoke the 'clear' command.
  "list_vars"      - print registered variables and their values to drop-down console
  "list_funcs"     - print registered variables and their values to drop-down console

Variables may also be registered, or accessed programmatically:

  apg_c_register_var( str, var_ptr, datatype )
  apg_c_get_var()

C functions can be called from console by registering a command name and function to call.
Your callback functions must be in the form:

  bool my_function( const char* arg_str )

If your function returns false the console prints an error message.

  apg_c_register_func( "my_function", my_function );

Scrolling output text may be interacted with programmatically:

  apg_c_output_clear()   - Clear the output text.
  apg_c_print( str )     - Adds a line of text to the output such as a debug message.
  apg_c_dump_to_stdout() - Writes the current console output text to stdout via printf().
  apg_c_count_lines()    - Counts the number of lines in the console output.

======================================================================================================================= */
#pragma once

#ifdef _WIN32
/** Explicit symbol export for building .DLLs with MSVC so it generates a corresponding .LIB. */
#define APG_C_EXPORT __declspec( dllexport )
#else
#define APG_C_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define APG_C_UNUSED( x ) (void)( x ) // To suppress compiler warnings with unused/dummy arguments in callbacks.
#define APG_C_STR_MAX 85              // Maximum console string length. commands and variable names must be shorter than this.
#define APG_C_VARS_MAX 256            // Maximum number of variables stored in console.
#define APG_C_FUNCS_MAX 128           // Maximum number of console commands.
#define APG_C_OUTPUT_LINES_MAX 32     // Maximum number of lines retained in output.

/** Compiler-specific checks to find missing arguments. */
#if defined( __clang__ )
#define APG_C_ATTRIB_PRINTF( fmt, args ) __attribute__( ( __format__( __printf__, fmt, args ) ) )
#elif defined( __GNUC__ )
#define APG_C_ATTRIB_PRINTF( fmt, args ) __attribute__( ( format( printf, fmt, args ) ) )
#else
#define APG_C_ATTRIB_PRINTF( fmt, args )
#endif

/* =======================================================================================================================
User-entered text API. Call these functions based on e.g. keyboard input.
======================================================================================================================= */

/** @return true if a valid command was parsed out of str. */
APG_C_EXPORT bool apg_c_append_user_entered_text( const char* str );

/** Removes the last char from the current user-entered string. */
APG_C_EXPORT void apg_c_backspace( void );

/** autocompletes the current user-entered text if it matches a command. call if i.e. the user presses 'TAB'.
 * @warning Current version assumes that the string to auto-complete is ASCII.
 */
APG_C_EXPORT void apg_c_autocomplete( void );

/** Enters a command from recent history into the current user-entered text field. i.e. if user presses up cursor arrow.
 * @param hist is the number of entries back from the most recent command entered. 0 is the previously-entered command. Greater -> older.
 */
APG_C_EXPORT void apg_c_reuse_hist( int hist_steps );

/** Call this function on i.e. user hitting cursor up arrow to replicate BaSh history scrolling. Calls apg_c_reuse_hist() with the appropriate history item. */
APG_C_EXPORT void apg_c_reuse_hist_back_one( void );

/** Call this function on i.e. user hitting cursor down arrow to replicate BaSh history scrolling. */
APG_C_EXPORT void apg_c_reuse_hist_ahead_one( void );

APG_C_EXPORT void apg_c_clear_user_entered_text( void );

APG_C_EXPORT void apg_c_get_user_entered_text( char* mbs, int max_len );

APG_C_EXPORT const char* apg_c_get_user_entered_mbs( void );

/* =======================================================================================================================
Console output text API.
======================================================================================================================= */

APG_C_EXPORT void apg_c_output_clear( void );

// Appends str as an output line to the scrolling output
APG_C_EXPORT void apg_c_printf( const char* message, ... ) APG_C_ATTRIB_PRINTF( 1, 2 );

APG_C_EXPORT int apg_c_count_lines( void );

APG_C_EXPORT int apg_c_newest_line_idx( void );

APG_C_EXPORT const char* apg_c_get_line_mbs( int line_idx );

// printf everything in console to stdout stream
APG_C_EXPORT void apg_c_dump_to_stdout( void );

/* =======================================================================================================================
Program <-> console variable and function linkage API
======================================================================================================================= */

typedef enum apg_c_var_datatype_t { APG_C_BOOL, APG_C_INT32, APG_C_UINT32, APG_C_FLOAT, APG_C_OTHER } apg_c_var_datatype_t;

typedef struct apg_c_var_t {
  char str[APG_C_STR_MAX];
  void* var_ptr;
  apg_c_var_datatype_t datatype;
} apg_c_var_t;

/** Registers an existing program variable with the console.
 * @warning          Pointer is assumed to be valid for lifetime of console registration.
 * @param str        A name for the variable.
 * @param var_ptr    Pointer to variable to register and use for this c_var.
 * @param datatype   Data type of the variable pointed to by var_ptr. Determines how values can be set from the console.
 * @returns          False if failed i.e. no space left in array of var ptrs.
 */
APG_C_EXPORT bool apg_c_register_var( const char* str, void* var_ptr, apg_c_var_datatype_t datatype );

/** Fetches the address of a console variable with name `str`.
 * @returns
 * `NULL` if the variable does not exist, otherwise the address of the c_var entry with pointer and data type.
 */
APG_C_EXPORT apg_c_var_t* apg_c_get_var( const char* str );

APG_C_EXPORT bool apg_c_register_func( const char* str, bool ( *fptr )( const char* arg_str ) );

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
