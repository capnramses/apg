/* =======================================================================================================================
APG_C - A Quake-style Console mini-library
Author:   Anton Gerdelan - @capnramses
Language: C99
Licence:  See bottom of header file.
======================================================================================================================= */
#include "apg_console.h"
#include "apg_pixfont.h" // used for adding glyphs to image output
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct apg_c_func_t {
  char str[APG_C_STR_MAX];
  bool ( *func_ptr )( float );
} apg_c_func_t;

static apg_c_var_t _c_vars[APG_C_VARS_MAX];
static uint32_t _n_c_vars;

static apg_c_func_t _c_funcs[APG_C_FUNCS_MAX];
static uint32_t _n_c_funcs;

static char c_output_lines[APG_C_OUTPUT_LINES_MAX][APG_C_STR_MAX];
static int c_output_lines_oldest = -1, c_output_lines_newest = -1, c_n_output_lines = 0;
static char _c_user_entered_text[APG_C_STR_MAX];

static const int _c_n_built_in_commands            = 5;
static char _c_built_in_commands[5][APG_C_STR_MAX] = { "help", "clear", "list_vars", "list_funcs" };

static bool _c_redraw_required;

/* because string.h doesn't always have strnlen() */
static int apg_c_strnlen( const char* str, int maxlen ) {
  int i = 0;
  while ( i < maxlen && str[i] ) { i++; }
  return i;
}

/* Custom strncat() without the annoying '\0' src truncation issues.
   Resulting string is always '\0' truncated.
   PARAMS
     dest_max - This is the maximum length the destination string is allowed to grow to.
     src_max  - This is the maximum number of bytes to copy from the source string.
*/
static void apg_c_strncat( char* dst, const char* src, const int dest_max, const int src_max ) {
  assert( dst && src );

  int dst_len   = apg_c_strnlen( dst, dest_max );
  dst[dst_len]  = '\0'; // just in case it wasn't already terminated before max length
  int remainder = dest_max - dst_len;
  if ( remainder <= 0 ) { return; }
  const int n = dest_max < src_max ? dest_max : src_max; // use src_max if smaller
  strncat( dst, src, n );                                // strncat manual guarantees null termination.
}

static void _help() {
  apg_c_print( "APG_C by Anton Gerdelan. Autocomplete supported. Built-in functions are:" );
  for ( int i = 0; i < _c_n_built_in_commands; i++ ) { apg_c_print( _c_built_in_commands[i] ); }
}

static void _list_c_vars() {
  apg_c_print( "=====c_vars=====" );
  for ( uint32_t i = 0; i < _n_c_vars; i++ ) { apg_c_print( _c_vars[i].str ); }
}

static void _list_c_funcs() {
  apg_c_print( "=====c_funcs=====" );
  for ( uint32_t i = 0; i < _n_c_funcs; i++ ) { apg_c_print( _c_funcs[i].str ); }
}

// returns index or -1 if did not find
// NOTE(Anton) could replace with a hash table
static int _console_find_var( const char* str ) {
  assert( str );

  for ( uint32_t i = 0; i < _n_c_vars; i++ ) {
    if ( strncmp( str, _c_vars[i].str, APG_C_STR_MAX ) == 0 ) { return i; }
  }
  return -1;
}

static int _console_find_func( const char* str ) {
  assert( str );

  for ( uint32_t i = 0; i < _n_c_funcs; i++ ) {
    if ( strncmp( str, _c_funcs[i].str, APG_C_STR_MAX ) == 0 ) { return i; }
  }
  return -1;
}

static int _console_find_builtin_func( const char* str ) {
  assert( str );

  for ( int i = 0; i < _c_n_built_in_commands; i++ ) {
    if ( strncmp( str, _c_built_in_commands[i], APG_C_STR_MAX ) == 0 ) { return i; }
  }
  return -1;
}

/* =======================================================================================================================
user-entered text API. call these functions based on eg keyboard input.
======================================================================================================================= */

static bool _parse_user_entered_instruction( const char* str ) {
  assert( str );

  char one[APG_C_STR_MAX], two[APG_C_STR_MAX], three[APG_C_STR_MAX], four[APG_C_STR_MAX];
  char tmp[APG_C_STR_MAX];
  one[0] = two[0] = three[0] = '\0';
  int n                      = sscanf( str, "%s %s %s %s", one, two, three, four );
  switch ( n ) {
  case 0: return true; // this would be simply '\n'
  case 1: {
    // search for func match
    int func_idx = _console_find_func( one );
    if ( func_idx >= 0 ) {
      bool res = _c_funcs[func_idx].func_ptr( 0.0f );
      if ( !res ) {
        snprintf( tmp, APG_C_STR_MAX, "ERROR: function `%s` returned error.", one );
        apg_c_print( tmp );
      }
      return true;
    }

    // search for command match
    if ( strncmp( one, "help", APG_C_STR_MAX ) == 0 ) {
      _help();
      return true;
    }

    if ( strncmp( one, "clear", APG_C_STR_MAX ) == 0 ) {
      apg_c_output_clear();
      return true;
    }

    if ( strncmp( one, "list_vars", APG_C_STR_MAX ) == 0 ) {
      _list_c_vars();
      return true;
    }
    if ( strncmp( one, "list_funcs", APG_C_STR_MAX ) == 0 ) {
      _list_c_funcs();
      return true;
    }

    // then variable. equivalent to 'get myvariable' but no 'get' command required in this console.
    int var_idx = _console_find_var( one );
    if ( var_idx >= 0 ) {
      apg_c_var_datatype_t dt = _c_vars[var_idx].datatype;
      void* var_ptr           = _c_vars[var_idx].var_ptr;
      assert( var_ptr );
      tmp[0] = '\0';
      switch ( dt ) {
      case APG_C_BOOL: {
        snprintf( tmp, APG_C_STR_MAX, "%s %u.", one, *(bool*)var_ptr );
      } break;
      case APG_C_INT32: {
        snprintf( tmp, APG_C_STR_MAX, "%s %i.", one, *(int32_t*)var_ptr );
      } break;
      case APG_C_UINT32: {
        snprintf( tmp, APG_C_STR_MAX, "%s %u.", one, *(uint32_t*)var_ptr );
      } break;
      case APG_C_FLOAT: {
        snprintf( tmp, APG_C_STR_MAX, "%s %f.", one, *(float*)var_ptr );
      } break;
      default: { snprintf( tmp, APG_C_STR_MAX, "%s OTHER", one ); } break; // some other data type
      }                                                                    // endswitch
      apg_c_print( tmp );
      return true;
    }

    // give up
    snprintf( tmp, APG_C_STR_MAX, "ERROR: `%s` is not a recognised command or variable name.", one );
    apg_c_print( tmp );
    return false;
  } break;

  case 2: {
    float fval = (float)atof( two );

    int func_idx = _console_find_func( one );
    if ( func_idx >= 0 ) {
      bool res = _c_funcs[func_idx].func_ptr( fval );
      if ( !res ) {
        snprintf( tmp, APG_C_STR_MAX, "ERROR: function `%s` returned error.", one );
        apg_c_print( tmp );
      }
      return true;
    }

    // assume this is equiv to "set myvariable value" with an implied "set"
    int var_idx = _console_find_var( one );
    if ( var_idx >= 0 ) {
      apg_c_var_datatype_t dt = _c_vars[var_idx].datatype;
      void* var_ptr           = _c_vars[var_idx].var_ptr;
      assert( var_ptr );
      tmp[0] = '\0';
      switch ( dt ) {
      case APG_C_BOOL: {
        *(bool*)var_ptr = (bool)atoi( two );
      } break;
      case APG_C_INT32: {
        *(int32_t*)var_ptr = (int32_t)atoi( two );
      } break;
      case APG_C_UINT32: {
        *(uint32_t*)var_ptr = (uint32_t)atoi( two );
      } break;
      case APG_C_FLOAT: {
        *(float*)var_ptr = fval;
      } break;
      default: break; // do nothing for complex data types
      }               // endswitch
      return true;
    } else {
      snprintf( tmp, APG_C_STR_MAX, "ERROR: `%s` is not a recognised variable name.", one );
      apg_c_print( tmp );
      return false;
    }
  } break;

  default: {
    snprintf( tmp, APG_C_STR_MAX, "ERROR: too many tokens in instruction." );
    apg_c_print( tmp );
    return false;
  } break;
  } // endswitch

  // shouldn't get here
  assert( false );
  return false;
}

bool apg_c_append_user_entered_text( const char* str ) {
  assert( str );

  // check for buffer overflow
  int uet_len   = apg_c_strnlen( _c_user_entered_text, APG_C_STR_MAX );
  int len       = apg_c_strnlen( str, APG_C_STR_MAX );
  int total_len = uet_len + len;
  if ( total_len > APG_C_STR_MAX ) {
    apg_c_clear_user_entered_text();
    return false;
  }

  // append
  apg_c_strncat( _c_user_entered_text, str, APG_C_STR_MAX, APG_C_STR_MAX );

  // check for line break and if so parse and then delete the rest of the string - no complex carry-on stuff.
  for ( int i = uet_len; i < total_len; i++ ) {
    if ( _c_user_entered_text[i] == '\n' ) {
      _c_user_entered_text[i] = '\0';
      apg_c_print( _c_user_entered_text );
      bool parsed             = _parse_user_entered_instruction( _c_user_entered_text );
      _c_user_entered_text[0] = '\0';
      return parsed;
    }
  }

  _c_redraw_required = true;
  return true;
}

// WARNING(Anton) not unicode-aware!
void apg_c_backspace( void ) {
  int uet_len = apg_c_strnlen( _c_user_entered_text, APG_C_STR_MAX );
  if ( uet_len < 1 ) { return; }
  _c_user_entered_text[uet_len - 1] = '\0';
  _c_redraw_required                = true;
}

void apg_c_clear_user_entered_text( void ) {
  _c_user_entered_text[0] = '\0';
  _c_redraw_required      = true;
}

// WARNING(Anton) - assumes string is ASCII
void apg_c_autocomplete() {
  size_t len = strlen( _c_user_entered_text );
  if ( 0 == len ) { return; }

  int i;
  for ( i = len - 1; i >= 0; i-- ) {
    if ( isspace( _c_user_entered_text[i] ) ) { break; }
  }
  i++;
  int token_span = len - i;
  if ( 0 == token_span ) { return; }
  char token[APG_C_STR_MAX + 1];
  for ( int k = 0; k < token_span; k++ ) { token[k] = _c_user_entered_text[k + i]; }
  token[token_span] = '\0';

  int n_matching        = 0;
  int last_matching_idx = -1;
  int section_matching  = -1;
  // check built-in funcs
  for ( int l = 0; l < _c_n_built_in_commands; l++ ) {
    char* res = strstr( _c_built_in_commands[l], token );
    if ( _c_built_in_commands[l] == res ) {
      n_matching++;
      last_matching_idx = l;
      section_matching  = 0;
      apg_c_print( _c_built_in_commands[l] );
    }
  }

  // check cfuncs
  for ( uint32_t m = 0; m < _n_c_funcs; m++ ) {
    char* res = strstr( _c_funcs[m].str, token );
    if ( _c_funcs[m].str == res ) {
      n_matching++;
      last_matching_idx = m;
      section_matching  = 1;
      apg_c_print( _c_funcs[m].str );
    }
  }

  // check variables
  for ( uint32_t o = 0; o < _n_c_vars; o++ ) {
    char* res = strstr( _c_vars[o].str, token );
    if ( _c_vars[o].str == res ) {
      n_matching++;
      last_matching_idx = o;
      section_matching  = 2;
      apg_c_print( _c_vars[o].str );
    }
  }
  if ( 1 == n_matching ) {
    switch ( section_matching ) {
    case 0: apg_c_strncat( _c_user_entered_text, &_c_built_in_commands[last_matching_idx][token_span], APG_C_STR_MAX, APG_C_STR_MAX ); break;
    case 1: apg_c_strncat( _c_user_entered_text, &_c_funcs[last_matching_idx].str[token_span], APG_C_STR_MAX, APG_C_STR_MAX ); break;
    case 2: apg_c_strncat( _c_user_entered_text, &_c_vars[last_matching_idx].str[token_span], APG_C_STR_MAX, APG_C_STR_MAX ); break;
    default: assert( false ); break;
    } // endswitch
  }
}

/* =======================================================================================================================
console output text API.
======================================================================================================================= */
void apg_c_output_clear( void ) {
  c_output_lines_oldest = c_output_lines_newest = -1;
  c_n_output_lines                              = 0;
  _c_redraw_required                            = true;
}

int apg_c_count_lines( void ) { return c_n_output_lines; }

void apg_c_print( const char* str ) {
  assert( str );

  c_output_lines_newest = ( c_output_lines_newest + 1 ) % APG_C_OUTPUT_LINES_MAX;
  c_n_output_lines      = c_n_output_lines < APG_C_OUTPUT_LINES_MAX ? c_n_output_lines + 1 : APG_C_OUTPUT_LINES_MAX;
  if ( c_output_lines_newest == c_output_lines_oldest ) { c_output_lines_oldest = ( c_output_lines_oldest + 1 ) % APG_C_OUTPUT_LINES_MAX; }
  if ( -1 == c_output_lines_oldest ) { c_output_lines_oldest = c_output_lines_newest; }
  strncpy( c_output_lines[c_output_lines_newest], str, APG_C_STR_MAX - 1 );

  _c_redraw_required = true;
}

void apg_c_dump_to_stdout( void ) {
  if ( c_output_lines_oldest < 0 || c_output_lines_newest < 0 ) { return; }
  int idx = c_output_lines_oldest;
  for ( int count = 0; count < APG_C_OUTPUT_LINES_MAX; count++ ) {
    printf( "%i) %s\n", idx, c_output_lines[idx] );
    if ( idx == c_output_lines_newest ) { return; }
    idx = ( idx + 1 ) % APG_C_OUTPUT_LINES_MAX;
  }
  printf( "> %s\n", _c_user_entered_text );
}

/* =======================================================================================================================
program <-> console variable and function linkage API
======================================================================================================================= */
bool apg_c_register_func( const char* str, bool ( *fptr )( float ) ) {
  assert( str );
  assert( fptr );

  if ( _n_c_funcs >= APG_C_FUNCS_MAX ) { return false; }
  int idx = _console_find_func( str );
  if ( idx >= 0 ) { return false; }
  idx = _n_c_funcs++;
  strncpy( _c_funcs[idx].str, str, APG_C_STR_MAX - 1 );
  _c_funcs[idx].func_ptr = fptr;

  return true;
}

bool apg_c_register_var( const char* str, void* var_ptr, apg_c_var_datatype_t datatype ) {
  assert( str && var_ptr );

  if ( _n_c_vars >= APG_C_VARS_MAX ) { return false; }
  int idx = _console_find_var( str );
  if ( idx >= 0 ) { return false; }
  idx = _console_find_builtin_func( str );
  if ( idx >= 0 ) { return false; }
  idx = _console_find_func( str );
  if ( idx >= 0 ) { return false; }
  idx = _n_c_vars++;
  strncpy( _c_vars[idx].str, str, APG_C_STR_MAX - 1 );
  _c_vars[idx].var_ptr  = var_ptr;
  _c_vars[idx].datatype = datatype;
  return true;
}

apg_c_var_t* apg_c_get_var( const char* str ) {
  assert( str );

  int idx = _console_find_var( str );
  if ( idx < 0 ) { return NULL; }
  return &_c_vars[idx];
}

/* =======================================================================================================================
rendering API
======================================================================================================================= */
bool apg_c_draw_to_image_mem( uint8_t* img_ptr, int w, int h, int n_channels, uint8_t* background_colour ) {
  assert( img_ptr );

  const int row_stride    = w * n_channels;
  const int row_height_px = 16;
  // text properties
  const int thickness = 1;
  const int outlines  = 1;

  if ( row_stride < 1 ) { return false; }

  memset( img_ptr, 0, row_stride * h );

  // draw scrolling text output above the prompt line
  int n_lines = apg_c_count_lines();
  for ( int i = 0; i < n_lines; i++ ) {
    int line_idx = c_output_lines_newest - i;
    if ( line_idx < 0 ) { line_idx += APG_C_OUTPUT_LINES_MAX; }
    int row_idx = h * row_stride - ( row_height_px * row_stride ) * ( i + 2 );
    if ( row_idx < 0 ) { break; } // don't bother if entire row above upper image bound
    apg_pixfont_str_into_image( c_output_lines[line_idx], &img_ptr[row_idx], w, row_height_px, n_channels, 0xFF, 0xFF, 0xFF, 0xFF, thickness, outlines );
  }
  { // draw user-entered text on the bottom of the image
    char uet_str[APG_C_STR_MAX];
    strcpy( uet_str, "> " );
    apg_c_strncat( uet_str, _c_user_entered_text, APG_C_STR_MAX, APG_C_STR_MAX );
    int bottom_row_idx = h * row_stride - ( row_height_px * row_stride );
    if ( bottom_row_idx < 0 ) { return false; } // not even space for one line
    apg_pixfont_str_into_image( uet_str, &img_ptr[bottom_row_idx], w, row_height_px, n_channels, 0xFF, 0xFF, 0xFF, 0xFF, thickness, outlines );
  }

  // set background colour wherever there is no text/outline
  // NOTE(Anton) this is not ideal for performance - use a second background image instead and blt onto it or render both
  for ( int y = 0; y < h; y++ ) {
    for ( int x = 0; x < w; x++ ) {
      int sum = 0;
      for ( int c = 0; c < n_channels; c++ ) {
        int idx = row_stride * y + x * n_channels + c;
        sum += img_ptr[idx];
      }
      if ( 0 == sum ) { memcpy( &img_ptr[row_stride * y + x * n_channels], background_colour, n_channels ); }
    }
  }

  _c_redraw_required = false;
  return true;
}

bool apg_c_image_redraw_required() { return _c_redraw_required; }
