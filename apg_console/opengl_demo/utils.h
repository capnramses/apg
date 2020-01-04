// Copyright Anton Gerdelan <antonofnote@gmail.com>. 2019
#pragma once
#include <stdio.h>

// fix used in bgfx and imgui to get around mingw not supplying alloca.h
#if defined( _MSC_VER ) || defined( __MINGW32__ )
#include <malloc.h> // alloca
#else
#include <alloca.h> // alloca
#endif

#define APG_UNUSED( x ) (void)( x ) // to suppress compiler warnings

// to add function deprecation across compilers
#ifdef __GNUC__
#define APG_DEPRECATED( func ) func __attribute__( ( deprecated ) )
#elif defined( _MSC_VER )
#define APG_DEPRECATED( func ) __declspec( deprecated ) func
#endif

// ---------------------------------- backtraces and dumps -----------------------------------------
// =================================================================================================
#ifdef APG_TRACE_UTILS
// Obtain a backtrace and print it to an open file stream or eg stdout
// note: to convert trace addresses into line numbers you can use gdb:
//   (gdb) info line *print_trace+0x5e
//   Line 92 of "src/utils.c" starts at address 0x6c745 <print_trace+74> and ends at 0x6c762 <print_trace+103>.
void print_trace( FILE* stream );

// writes a backtrace on sigsegv
void start_crash_handler();
#endif

#ifdef UNIT_TESTS
void deliberate_sigsegv();
void deliberate_divzero();
#endif

// --------------------------------------- log files -----------------------------------------------
// =================================================================================================
#define GLOG_FILE "game.log"

// make bad log args print compiler warnings. note: mingw sucks for this
#if defined( __clang__ )
#define ATTRIB_PRINTF( fmt, args ) __attribute__( ( __format__( __printf__, fmt, args ) ) )
#elif defined( __MINGW32__ )
#define ATTRIB_PRINTF( fmt, args ) __attribute__( ( format( ms_printf, fmt, args ) ) )
#elif defined( __GNUC__ )
#define ATTRIB_PRINTF( fmt, args ) __attribute__( ( format( printf, fmt, args ) ) )
#else
#define ATTRIB_PRINTF( fmt, args )
#endif

void glog_start();
void glog( const char* message, ... ) ATTRIB_PRINTF( 1, 2 );
void glog_err( const char* message, ... ) ATTRIB_PRINTF( 1, 2 );

// ---------------------------------------- strings ------------------------------------------------
// =================================================================================================
// because string.h doesn't always have strnlen()
static inline int apg_strnlen( const char* str, int maxlen ) {
  int i = 0;
  while ( i < maxlen && str[i] ) { i++; }
  return i;
}

// =================================================================================================
#ifdef APG_SLEEP
// NOTE: for linux -D_POSIX_C_SOURCE=199309L must be defined for glibc to get nanosleep()
void apg_sleep_ms( int ms );
#endif // APG_SLEEP

// ----------------------------------------- memory ------------------------------------------------
// =================================================================================================
#define KILOBYTES( value ) ( (value)*1024 )
#define MEGABYTES( value ) ( KILOBYTES( value ) * 1024 )
#define GIGABYTES( value ) ( MEGABYTES( value ) * 1024 )

// avoid use of malloc at runtime. use alloca() for up to ~1MB, or scratch_mem() for reusing a larger preallocated heap block

// call once at program start, after starting logs
// (re)allocates scratch memory for use by scratch_mem
// not thread safe
// scratch_a_sz - size of main scratch in bytes
// scratch_b_sz - size of second scratch in bytes ( can be zero if not needed )
// scratch_c_sz - size of second scratch in bytes ( can be zero if not needed )
// asserts on out of memory
void scratch_init( size_t scratch_a_sz, size_t scratch_b_sz, size_t scratch_c_sz );

// frees both scratch memory allocations
// asserts if nothing allocated
void scratch_free();

// returns a pointer to a pre-allocated, zeroed, block of memory to use for eg image resizes to avoid alloc
// asserts if sz requested in bytes is larger than pre-allocated size or if prealloc_scratch() hasn't been called
void* scratch_mem_a( size_t sz );

// use the second scratch
void* scratch_mem_b( size_t sz );

// use the third scratch
void* scratch_mem_c( size_t sz );
