/*apg.h - generic C utility functions
First version Dr Anton Gerdelan, 8 May 2015
Licence: see bottom of file.

Usage Instructions
------------------
* in one file #define APG_IMPLEMENTATION above the #include

 TODO
------------------
* linearise/unlinearise function
* ascii or unicode fetcher/printer thing
* string manip/trim
*/

#ifndef _APG_H_
#define _APG_H_

/* types */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>/* size_t */
#include <stdio.h> /* FILE* */

/*=================================================================================================
COMPILER HELPERS
=================================================================================================*/
#ifdef _WIN64
#define APG_BUILD_PLAT_STR "Microsoft Windows (64-bit)."
#elif _WIN32
#define APG_BUILD_PLAT_STR "Microsoft Windows (32-bit)."
#elif __CYGWIN__ /* _WIN32 must not be defined */
#define APG_BUILD_PLAT_STR "Cygwin POSIX under Microsoft Windows."
#elif __linux__
#define APG_BUILD_PLAT_STR "Linux."
#elif __APPLE__ /* Can add checks to detect OSX/iPhone/XCode iPhone emulators. */
#define APG_BUILD_PLAT_STR "OS X."
#elif __unix__ /* Also valid for Linux. __APPLE__ is also BSD. */
#define APG_BUILD_PLAT_STR "BSD."
#else
#define APG_BUILD_PLAT_STR "Unknown."
#endif

#define APG_UNUSED( x ) (void)( x ) /* to suppress compiler warnings */

/* to add function deprecation across compilers */
#ifdef __GNUC__
#define APG_DEPRECATED( func ) func __attribute__( ( deprecated ) )
#elif defined( _MSC_VER )
#define APG_DEPRECATED( func ) __declspec( deprecated ) func
#endif

/*=================================================================================================
MATHS
=================================================================================================*/
/* C99 removed M_PI */
#define _USE_MATH_DEFINES
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* replacement for the deprecated min/max functions from original C spec.
was going to have a series of GL-like functions but it was a lot of fiddly code/alternatives,
so I'm just copying from stb.h here. as much as I dislike pre-processor directives, this makes sense.
I believe the trick is to have all the parentheses. same deal for clamp */
#define APG_MIN( a, b ) ( ( a ) < ( b ) ? ( a ) : ( b ) )
#define APG_MAX( a, b ) ( ( a ) > ( b ) ? ( a ) : ( b ) )
#define APG_CLAMP( x, lo, hi ) ( APG_MIN( hi, APG_MAX( lo, x ) ) )

/* unit conversions */
#define APG_DEG2RAD( a ) ( ( a ) * ( M_PI / 180.0 ) )
#define APG_RAD2DEG( a ) ( ( a ) * ( 180.0 / M_PI ) )

/* like clamp, but if > max then returns min e.g. loops back to 0
and if < min, returns max e.g. loops back to 99
NOTE: does not preserve differences > 1, so suitable for iterating++ counters
but not big differences

NOTE(Anton): would it make sense to offset by the difference here, instead of
a fixed = max or = min? if so a #define might be better b/c it could apply
to more variable types
*/
static inline int apg_loopi( int val, int min, int max ) {
  if ( val < min ) { return max; }
  if ( val > max ) { return min; }
  return val;
}

/*=================================================================================================
TIME
=================================================================================================*/
/* get a monotonic time value in nanoseconds (linux only)
value is some arbitrary system time but is invulnerable to clock changes */
double apg_time_linux();

/* NOTE: for linux -D_POSIX_C_SOURCE=199309L must be defined for glibc to get nanosleep() */
void apg_sleep_ms( int ms );

/*=================================================================================================
STRINGS
=================================================================================================*/
/* get string from file
does not do any malloc - fills existing buffer up to length max_len
returns false on error */
bool apg_file_to_str( const char* file_name, size_t max_len, char* str );

/* custom strcmp to avoid commonly-made ==0 bracket soup bugs
returns true if true so far and one string shorter e.g. "ANT" "ANTON" */
bool apg_strmatchy( const char* a, const char* b );

/* because string.h doesn't always have strnlen() */
static inline int apg_strnlen( const char* str, int maxlen ) {
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
void apg_strncat( char* dst, const char* src, const int dest_max, const int src_max );

/*=================================================================================================
LOG FILES
=================================================================================================*/
/* make bad log args print compiler warnings. note: mingw sucks for this */
#if defined( __clang__ )
#define ATTRIB_PRINTF( fmt, args ) __attribute__( ( __format__( __printf__, fmt, args ) ) )
#elif defined( __MINGW32__ )
#define ATTRIB_PRINTF( fmt, args ) __attribute__( ( format( ms_printf, fmt, args ) ) )
#elif defined( __GNUC__ )
#define ATTRIB_PRINTF( fmt, args ) __attribute__( ( format( printf, fmt, args ) ) )
#else
#define ATTRIB_PRINTF( fmt, args )
#endif

/* open/refresh a new log file and print timestamp */
void apg_start_log();
/* write a log entry */
void apg_log( const char* message, ... ) ATTRIB_PRINTF( 1, 2 );
/* write a log entry and print to stderr */
void apg_log_err( const char* message, ... ) ATTRIB_PRINTF( 1, 2 );

/*=================================================================================================
BACKTRACES AND DUMPS
=================================================================================================*/
/* Obtain a backtrace and print it to an open file stream or eg stdout
note: to convert trace addresses into line numbers you can use gdb:
(gdb) info line *print_trace+0x5e
Line 92 of "src/utils.c" starts at address 0x6c745 <print_trace+74> and ends at 0x6c762 <print_trace+103>. */
void apg_print_trace( FILE* stream );

/* writes a backtrace on sigsegv */
void apg_start_crash_handler();

#ifdef APG_UNIT_TESTS
void apg_deliberate_sigsegv();
void apg_deliberate_divzero();
#endif

/*=================================================================================================
COMMAND LINE PARAMETERS
=================================================================================================*/
/* i learned this trick from the DOOM source code */
int apg_check_param( const char* check );

extern int g_apg_argc;
extern char** g_apg_argv;

/*=================================================================================================
MEMORY
=================================================================================================*/
/* fix used in bgfx and imgui to get around mingw not supplying alloca.h */
#if defined( _MSC_VER ) || defined( __MINGW32__ )
#include <malloc.h>
#else
#include <alloca.h>
#endif

#define APG_KILOBYTES( value ) ( (value)*1024 )
#define APG_MEGABYTES( value ) ( APG_KILOBYTES( value ) * 1024 )
#define APG_GIGABYTES( value ) ( APG_MEGABYTES( value ) * 1024 )

/* avoid use of malloc at runtime. use alloca() for up to ~1MB, or scratch_mem() for reusing a larger preallocated heap block */

/* call once at program start, after starting logs
(re)allocates scratch memory for use by scratch_mem
not thread safe
scratch_a_sz - size of main scratch in bytes
scratch_b_sz - size of second scratch in bytes ( can be zero if not needed )
scratch_c_sz - size of second scratch in bytes ( can be zero if not needed )
asserts on out of memory */
void apg_scratch_init( size_t scratch_a_sz, size_t scratch_b_sz, size_t scratch_c_sz );

/* frees both scratch memory allocations
asserts if nothing allocated */
void apg_scratch_free();

/* returns a pointer to a pre-allocated, zeroed, block of memory to use for eg image resizes to avoid alloc
asserts if sz requested in bytes is larger than pre-allocated size or if prealloc_scratch() hasn't been called */
void* apg_scratch_mem_a( size_t sz );

/* use the second scratch */
void* apg_scratch_mem_b( size_t sz );

/* use the third scratch */
void* apg_scratch_mem_c( size_t sz );

/*=================================================================================================
------------------------------------------IMPLEMENTATION------------------------------------------
=================================================================================================*/
#ifdef APG_IMPLEMENTATION
#undef APG_IMPLEMENTATION

#include <assert.h>
#include <signal.h> /* for crash handling */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>                /* for strcasecmp */
/*#define _POSIX_C_SOURCE 199309L*/ /* for the timer on linux. may already be defined */
#include <time.h>
#include <unistd.h> /* linux-only? */
#ifdef _WIN32
#include <Windows.h> /* for backtraces */
#include <dbghelp.h> /* SymInitialize */
#else
#include <execinfo.h>
#endif

/*=================================================================================================
TIME IMPLEMENTATION
=================================================================================================*/
/* get a monotonic time value in seconds w/ nanosecond precision (linux only)
value is some arbitrary system time but is invulnerable to clock changes
CLOCK_MONOTONIC -- vulnerable to adjtime() and NTP changes
CLOCK_MONOTONIC_RAW -- vulnerable to voltage and heat changes */
#ifdef __linux__
inline double apg_time_linux() {
  struct timespec t;
  static double prev_value = 0.0;
  int r                    = clock_gettime( CLOCK_MONOTONIC, &t );
  if ( r < 0 ) {
    fprintf( stderr, "WARNING: could not get time value\n" );
    return prev_value;
  }
  double ns  = t.tv_nsec;
  double s   = ns * 0.000000001;
  time_t tts = t.tv_sec;
  s += difftime( tts, 0 );
  prev_value = s;
  return s;
}
#endif /* __linux__ */

/* NOTE: for linux -D_POSIX_C_SOURCE=199309L must be defined for glibc to get nanosleep() */
void apg_sleep_ms( int ms ) {
#ifdef WIN32
  Sleep( ms ); /* NOTE(Anton) may not need this since using gcc on Windows and usleep() works */
#elif _POSIX_C_SOURCE >= 199309L
  struct timespec ts;
  ts.tv_sec  = ms / 1000;
  ts.tv_nsec = ( ms % 1000 ) * 1000000;
  nanosleep( &ts, NULL );
#else
  usleep( ms * 1000 );
#endif
}

/*=================================================================================================
STRINGS IMPLEMENTATION
=================================================================================================*/
bool apg_file_to_str( const char* file_name, size_t max_len, char* str ) {
  FILE* fp = fopen( file_name, "r" );
  if ( !fp ) {
    fprintf( stderr, "ERROR: opening file %s\n", file_name );
    return false;
  }
  size_t cnt = fread( str, 1, max_len - 1, fp );
  if ( cnt >= max_len - 1 ) { fprintf( stderr, "WARNING: file %s too big - truncated.\n", file_name ); }
  if ( ferror( fp ) ) {
    fprintf( stderr, "ERROR: reading shader file %s\n", file_name );
    fclose( fp );
    return false;
  }
  /* append \0 to end of file string */
  str[cnt] = 0;
  fclose( fp );
  return true;
}

bool apg_strmatchy( const char* a, const char* b ) {
  int len = APG_MAX( strlen( a ), strlen( b ) );
  for ( int i = 0; i < len; i++ ) {
    if ( a[i] != b[i] ) { return false; }
  }
  return true;
}

void apg_strncat( char* dst, const char* src, const int dest_max, const int src_max ) {
  assert( dst && src );

  int dst_len   = apg_strnlen( dst, dest_max );
  dst[dst_len]  = '\0'; // just in case it wasn't already terminated before max length
  int remainder = dest_max - dst_len;
  if ( remainder <= 0 ) { return; }
  const int n = dest_max < src_max ? dest_max : src_max; // use src_max if smaller
  strncat( dst, src, n );                                // strncat manual guarantees null termination.
}

/*=================================================================================================
LOG FILES IMPLEMENTATION
=================================================================================================*/
#define APG_LOG_FILE "apg.log" /* file name for log */

void apg_start_log() {
  FILE* file = fopen( APG_LOG_FILE, "w" ); /* NOTE it was getting massive with "a" */
  if ( !file ) {
    fprintf( stderr, "ERROR: could not open APG_LOG_FILE log file %s for writing\n", APG_LOG_FILE );
    return;
  }
  time_t now = time( NULL );
  char* date = ctime( &now );
  fprintf( file, "\n------------ %s log. local time %s\n", APG_LOG_FILE, date );
  fclose( file );
}

void apg_log( const char* message, ... ) {
  va_list argptr;
  FILE* file = fopen( APG_LOG_FILE, "a" );
  if ( !file ) {
    fprintf( stderr, "ERROR: could not open APG_LOG_FILE %s file for appending\n", APG_LOG_FILE );
    return;
  }
  va_start( argptr, message );
  vfprintf( file, message, argptr );
  va_end( argptr );
  fclose( file );
}

void apg_log_err( const char* message, ... ) {
  va_list argptr;
  FILE* file = fopen( APG_LOG_FILE, "a" );
  if ( !file ) {
    fprintf( stderr, "ERROR: could not open APG_LOG_FILE %s file for appending\n", APG_LOG_FILE );
    return;
  }
  va_start( argptr, message );
  vfprintf( file, message, argptr );
  va_end( argptr );
  fclose( file );
  va_start( argptr, message );
  vfprintf( stderr, message, argptr );
  va_end( argptr );
}

/*=================================================================================================
BACKTRACES AND DUMPS IMPLEMENTATION
=================================================================================================*/
static void _crash_handler( int sig ) {
  switch ( sig ) {
  case SIGSEGV: {
    apg_log_err( "FATAL ERROR: SIGSEGV- signal %i\nOut of bounds memory access or dereferencing a null pointer:\n", sig );
  } break;
  case SIGABRT: {
    apg_log_err( "FATAL ERROR: SIGABRT - signal %i\nabort or assert:\n", sig );
  } break;
  case SIGFPE: {
    apg_log_err( "FATAL ERROR: SIGFPE - signal %i\nArithmetic - probably a divide-by-zero or integer overflow:\n", sig );
  } break;
  case SIGILL: {
    apg_log_err( "FATAL ERROR: SIGILL - signal %i\nIllegal instruction - probably function pointer invalid or stack overflow:\n", sig );
  } break;
  default: { apg_log_err( "FATAL ERROR: signal %i:\n", sig ); } break;
  }
  /* note(anton) sigbus didnt exist on my mingw32 gcc */

  FILE* file = fopen( APG_LOG_FILE, "a" );
  if ( file ) {
    apg_print_trace( file );
    fclose( file );
  }
  apg_print_trace( stderr );
  exit( 1 );
}

void apg_print_trace( FILE* stream ) {
  assert( stream );

#ifdef _WIN32
  { /* NOTE: need a .pdb to read symbols on windows. gcc just needs -g -rdynamic on linux/mac. call cv2pdb myprog.exe -- https://github.com/rainers/cv2pdb */
    HANDLE process = GetCurrentProcess();
    HANDLE thread  = GetCurrentThread();

    CONTEXT context;
    memset( &context, 0, sizeof( CONTEXT ) );
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext( &context );

    SymInitialize( process, NULL, TRUE );

    DWORD image = IMAGE_FILE_MACHINE_AMD64;
    STACKFRAME64 stackframe;
    ZeroMemory( &stackframe, sizeof( STACKFRAME64 ) );
    /* NOTE(anton) this is for x64. for _M_IA64 or _M_IX86 use different names. read this for shipping: http://blog.morlad.at/blah/mingw_postmortem */
    stackframe.AddrPC.Offset    = context.Rip;
    stackframe.AddrPC.Mode      = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Rsp;
    stackframe.AddrFrame.Mode   = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Rsp;
    stackframe.AddrStack.Mode   = AddrModeFlat;

    for ( size_t i = 0; i < 25; i++ ) {
      BOOL result = StackWalk64( image, process, thread, &stackframe, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL );
      if ( !result ) { break; }

      char buffer[sizeof( SYMBOL_INFO ) + MAX_SYM_NAME * sizeof( TCHAR )];
      PSYMBOL_INFO symbol  = (PSYMBOL_INFO)buffer;
      symbol->SizeOfStruct = sizeof( SYMBOL_INFO );
      symbol->MaxNameLen   = MAX_SYM_NAME;

      DWORD64 displacement = 0;
      if ( SymFromAddr( process, stackframe.AddrPC.Offset, &displacement, symbol ) ) {
        fprintf( stream, "[%i] %-30s - 0x%0X\n", (int)i, symbol->Name, (unsigned int)symbol->Address );
      } else {
        fprintf( stream, "[%i] ??\n", (int)i );
      }
    } /* endfor */
    SymCleanup( process );
  }
#else /* TODO(anton) test on OS X */
#define BT_BUF_SIZE 100
  void* array[BT_BUF_SIZE];
  int size       = backtrace( array, BT_BUF_SIZE );
  char** strings = backtrace_symbols( array, size );
  if ( strings == NULL ) {
    perror( "backtrace_symbols" ); /* also print internal error to stderr */
    exit( EXIT_FAILURE );
  }
  fprintf( stream, "Obtained %i stack frames.\n", size );
  for ( int i = 0; i < size; i++ ) fprintf( stream, "%s\n", strings[i] );
  free( strings );
#endif
} /* endfunc apg_print_trace() */

/* to deliberately cause a sigsegv: call a function containing bad ptr: int *foo = (int*)-1; */
void apg_start_crash_handler() {
  signal( SIGSEGV, _crash_handler );
  signal( SIGABRT, _crash_handler ); /* assert */
  signal( SIGILL, _crash_handler );
  signal( SIGFPE, _crash_handler ); /* ~ int div 0 */
  /* no sigbus on my mingw */
}

#ifdef APG_UNIT_TESTS
void apg_deliberate_sigsegv() {
  int* bad = (int*)-1;
  printf( "%i\n", *bad );
}

void apg_deliberate_divzero() {
  int a   = rand();
  int b   = a - a;
  int bad = a / b;
  printf( "%i\n", bad );
}
#endif /* APG_UNIT_TESTS */

/*=================================================================================================
COMMAND LINE PARAMETERS IMPLEMENTATION
=================================================================================================*/
int g_apg_argc;
char** g_apg_argv;

/* Checks for given parameter in main's command-line arguments
returns the argument number if present (1 to argc - 1)
otherwise returns 0 */
int apg_check_param( const char* check ) {
  for ( int i = 1; i < g_apg_argc; i++ ) {
    /* NOTE: the original used strcasecmp() here which is the case insenstive
    version, but it might require strings.h instead, depending on compiler
    it makes sense to ignore case on multi-plat command line */
    if ( strcasecmp( check, g_apg_argv[i] ) == 0 ) { return i; }
  }
  return -1;
}

#endif /* APG_IMPLEMENTATION */
#endif /* _APG_H_ */

/*
-------------------------------------------------------------------------------------
This software is available under two licences - you may use it under either licence.
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
*/
