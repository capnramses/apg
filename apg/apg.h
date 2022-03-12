/*apg.h - Generic C utility functions.
Version: 1.3. 4th May 2020.
Licence: see bottom of file.
C89 ( Implementation is C99 )
Licence: see bottom of file.
History:
1.4 - 12 Mar 2022. Hash table functions.
1.3 - 11 Sep 2020. Fixed apg_file_to_str() portability issue.
1.2 - 15 May 2020. Updated timers for multi-platform use based on Professional Programming Tools book code. Updated test code.
1.1 -  4 May 2020. Added custom rand() functions.
1.0 -  8 May 2015. By Anton Gerdelan.

Usage Instructions
------------------
* In one file #define APG_IMPLEMENTATION above the #include.
* On MinGW you may have to link against some system libs with -limagehlp

 TODO
------------------
* linearise/unlinearise function
* string manip/trim
*/

#ifndef _APG_H_
#define _APG_H_

#include <assert.h>
#include <math.h>  /* modff() */
#include <stdint.h>/* types */
#include <stdbool.h>
#include <stddef.h>/* size_t */
#include <stdio.h> /* FILE* */
#include <string.h>

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
/* replacement for the deprecated min/max functions from original C spec.
was going to have a series of GL-like functions but it was a lot of fiddly code/alternatives,
so I'm just copying from stb.h here. as much as I dislike pre-processor directives, this makes sense.
I believe the trick is to have all the parentheses. same deal for clamp */
#define APG_MIN( a, b ) ( ( a ) < ( b ) ? ( a ) : ( b ) )
#define APG_MAX( a, b ) ( ( a ) > ( b ) ? ( a ) : ( b ) )
#define APG_CLAMP( x, lo, hi ) ( APG_MIN( hi, APG_MAX( lo, x ) ) )

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
PSEUDO-RANDOM NUMBERS
=================================================================================================*/
/* platform-consistent rand() and srand()
based on http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf pg 312
WARNING - these functions are not thread-safe. */
#define APG_RAND_MAX 32767 // Must be at least 32767 (0x7fff). Windows uses this value.

void apg_srand( unsigned int seed );
int apg_rand( void );

/* same as apg_rand() except returns a value between 0.0 and 1.0 */
float apg_randf( void );

/* useful to re-seed apg_srand later with whatever the pseudo-random sequence is up to now i.e. for saved games. */
unsigned int apg_get_srand_next( void );

/*=================================================================================================
TIME
=================================================================================================*/
/* Set up for using timers.
WARNING - These functions are not thread-safe. */
void apg_time_init( void );

/* Get a monotonic time value in seconds with up to nanoseconds precision.
Value is some arbitrary system time but is invulnerable to clock changes.
Call apg_time_init() once before calling apg_time_s().
WARNING - These functions are not thread-safe. */
double apg_time_s( void );

/* NOTE: for linux -D_POSIX_C_SOURCE=199309L must be defined for glibc to get nanosleep() */
void apg_sleep_ms( int ms );

/*=================================================================================================
STRINGS
=================================================================================================*/
/* Custom strcmp variant to do a partial match avoid commonly-made == 0 bracket soup bugs.
PARAMS
a,b         - Input strings to compare.
a_max,b_max - Maximum lengths of a and b, respectively. Makes function robust to missing nul-terminators.
RETURNS true if both strings are the same, or if the shorter string matches its length up to the longer string at that point. i.e. "ANT" "ANTON" returns true. */
bool apg_strparmatch( const char* a, const char* b, size_t a_max, size_t b_max );

/* because string.h doesn't always have strnlen() */
int apg_strnlen( const char* str, int maxlen );

/* Custom strncat() without the annoying '\0' src truncation issues.
   Resulting string is always '\0' truncated.
   PARAMS
     dest_max - This is the maximum length the destination string is allowed to grow to.
     src_max  - This is the maximum number of bytes to copy from the source string.
*/
void apg_strncat( char* dst, const char* src, const int dest_max, const int src_max );

/*=================================================================================================
FILES
=================================================================================================*/
/* convenience struct and file->memory function */
typedef struct apg_file_t {
  void* data;
  size_t sz;
} apg_file_t;

/*
RETURNS
- true on success. record->data is allocated memory and must be freed by the caller.
- false on any error. Any allocated memory is freed if false is returned
*/
bool apg_read_entire_file( const char* filename, apg_file_t* record );

/* Loads file_name's contents into a byte array and always ends with a NULL terminator.
Calls apg_read_entire_file, which allocates memory - fills existing buffer up to length max_len.
RETURNS false on error
*/
bool apg_file_to_str( const char* file_name, size_t max_len, char* str );

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
void apg_start_crash_handler( void );

#ifdef APG_UNIT_TESTS
void apg_deliberate_sigsegv( void );
void apg_deliberate_divzero( void );
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
COMPRESSION
=================================================================================================*/
/* Apply run-length encoding to an array of bytes pointed to by bytes_in, over size in bytes given by sz_in.
The result is written to bytes_out, with output size in bytes written to sz_out.
PARAMS
  bytes_in  - If NULL then sz_out is set to 0.
  sz_in     - If 0 then sz_out is set to 0.
  bytes_out - If NULL then sz_out is reported, but no memory is written to. This is useful for determining the size required for output buffer allocation.
  sz_out    - Must not be NULL.
*/
void apg_rle_compress( const uint8_t* bytes_in, size_t sz_in, uint8_t* bytes_out, size_t* sz_out );
void apg_rle_decompress( const uint8_t* bytes_in, size_t sz_in, uint8_t* bytes_out, size_t* sz_out );

/*=================================================================================================
HASH TABLE
Motivation:
 - Avoid performance-disruptive run-time memory allocation, so it's linear probing rather than chained buckets. -> It Still needs to malloc() key strings though.
 - Allow user to check collisions and hash table capacity so user can decide on a good initial table size based on their data.
 - Minimal aux. memory overhead.
 - Fast and simple.
 - Allow user to determine when to rebuild the hash-table. There should never be surprise table reallocations at run-time!
   To do this:
   * Each time a key is stored, check for capacity e.g. >=70%.
   * Then create a new table at ~x2 capacity.
   * And run the hash function on the stored key of each valid element (value_ptr != NULL) in the table.
     * NB If I stored the hash value ( before the % table_n ) then we could skip the first hash function call and start with just applying % n.
     * But if there's a collision in the new table index it gets more complex -> need to rehash still.
   * And memcpy the element to the correct position new table.

Potential improvements:
 - If the user program reliably retains strings as well as values, we could avoid string memory allocation during hash_store calls, and just point to external.
 - If I also stored the hash in apg_hash_table_element_t it would avoid many potentially lengthy strcmp() calls during search.
 ================================================================================================*/

typedef struct apg_hash_table_element_t {
  char* keystr;    // This is either an allocated ASCII string or an integer value.
  void* value_ptr; // Address of value in user code. Value data is not allocated or stored directly in the table. If NULL then element is empty.
} apg_hash_table_element_t;

typedef struct apg_hash_table_t {
  apg_hash_table_element_t* list_ptr;
  int n;
  int count_stored;
} apg_hash_table_t;

/** Allocates memory for a hash table of size `table_n`.
 * @param table_n For a well performing table use a number somewhat larger than required space.
 * @return A generated, empty, hash table, or an empty table ( list_ptr == NULL ) on out of memory error.
 */
apg_hash_table_t apg_hash_table_create( int table_n );

/** Free any memory allocated to the table, including allocated key string memory. */
void apg_hash_table_free( apg_hash_table_t* table_ptr );

/** Returns a hash for a key->table mapping.
 * Be sure to compute hash_index = hash % table_N after calling this function.
 */
uint32_t apg_hash( const char* keystr );

/** A second hash function, using djb2 (based on http://www.cse.yorku.ca/~oz/hash.html),
 * This is used by store and search functions on first collision for a double-hashing approach.
 */
uint32_t apg_hash_rehash( const char* keystr );

/** Store a key-value pair in a given hash table.
 * @param keystr        A null-terminated C string. Must not be NULL.
 * @param value_ptr     Address of external memory to point to. Must not be NULL.
 * @param table_ptr     Address of a hash table previously allocated with a call to apg_hash_table_create().
 * @param collision_ptr Optional argument. If non-NULL, then the integer pointed to is set to the number of collisions incurred by this function call.
 *                      In cases where the function returns false then the collision counter is not incremented.
 * @return              This function returns true on success. It returns false in cases where the table is full,
 *                      the key was already stored in the table, or the parameters are invalid.
 */
bool apg_hash_store( const char* keystr, void* value_ptr, apg_hash_table_t* table_ptr, int* collision_ptr );

/**
 * @return This function returns true if the key is found in the table. In this case the integer pointed to by `idx_ptr` is set to the corresponding table
 * index. This function returns false if the table is empty, the parameters are invalid, or the key is not stored in the table.
 */
bool apg_hash_search( const char* keystr, apg_hash_table_t* table_ptr, uint32_t* idx_ptr, int* collision_ptr );

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
#include <strings.h> /* for strcasecmp */
#include <time.h>
#include <unistd.h> /* linux-only? */
#ifdef _WIN32
#include <windows.h> /* for backtraces and timers */
#include <dbghelp.h> /* SymInitialize */
#else
#include <execinfo.h>
#endif
/* includes for timers */
#ifdef _WIN32
#include <profileapi.h>
#elif __APPLE__
#include <mach/mach_time.h>
#else
#include <sys/time.h>
#endif

/*=================================================================================================
PSEUDO-RANDOM NUMBERS IMPLEMENTATION
=================================================================================================*/
static unsigned long int _srand_next = 1;

void apg_srand( unsigned int seed ) { _srand_next = seed; }

int apg_rand( void ) {
  _srand_next = _srand_next * 1103515245 + 12345;
  return (unsigned int)( _srand_next / ( ( APG_RAND_MAX + 1 ) * 2 ) ) % ( APG_RAND_MAX + 1 );
}

float apg_randf( void ) { return (float)apg_rand() / (float)APG_RAND_MAX; }

unsigned int apg_get_srand_next( void ) { return _srand_next; }

/*=================================================================================================
TIME IMPLEMENTATION
=================================================================================================*/
static uint64_t _frequency = 1000000, _offset;

void apg_time_init( void ) {
#ifdef _WIN32
  _frequency = 1000; // QueryPerformanceCounter default
  QueryPerformanceFrequency( (LARGE_INTEGER*)&_frequency );
  QueryPerformanceCounter( (LARGE_INTEGER*)&_offset );
#elif __APPLE__
  mach_timebase_info_data_t info;
  mach_timebase_info( &info );
  _frequency       = ( info.denom * 1e9 ) / info.numer;
  _offset          = mach_absolute_time();
#else
  _frequency = 1000000000; // nanoseconds
  struct timespec ts;
  clock_gettime( CLOCK_MONOTONIC, &ts );
  _offset = (uint64_t)ts.tv_sec * (uint64_t)_frequency + (uint64_t)ts.tv_nsec;
#endif
}

double apg_time_s( void ) {
#ifdef _WIN32
  uint64_t counter = 0;
  QueryPerformanceCounter( (LARGE_INTEGER*)&counter );
  return (double)( counter - _offset ) / _frequency;
#elif __APPLE__
  uint64_t counter = mach_absolute_time();
  return (double)( counter - _offset ) / _frequency;
#else
  struct timespec ts;
  clock_gettime( CLOCK_MONOTONIC, &ts );
  uint64_t counter = (uint64_t)ts.tv_sec * (uint64_t)_frequency + (uint64_t)ts.tv_nsec;
  return (double)( counter - _offset ) / _frequency;
#endif
}

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
bool apg_strparmatch( const char* a, const char* b, size_t a_max, size_t b_max ) {
  size_t len = APG_MAX( strnlen( a, a_max ), strnlen( b, b_max ) );
  for ( size_t i = 0; i < len; i++ ) {
    if ( a[i] != b[i] ) { return false; }
  }
  return true;
}

int apg_strnlen( const char* str, int maxlen ) {
  int i = 0;
  while ( i < maxlen && str[i] ) { i++; }
  return i;
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
FILES IMPLEMENTATION
=================================================================================================*/
bool apg_read_entire_file( const char* filename, apg_file_t* record ) {
  if ( !filename || !record ) { return false; }

  FILE* fp = fopen( filename, "rb" );
  if ( !fp ) { return false; }
  fseek( fp, 0L, SEEK_END );
  record->sz   = (size_t)ftell( fp );
  record->data = malloc( record->sz );
  if ( !record->data ) {
    fclose( fp );
    return false;
  }
  rewind( fp );
  size_t nr = fread( record->data, record->sz, 1, fp );
  fclose( fp );
  if ( 1 != nr ) {
    free( record->data );
    return false;
  }
  return true;
}

bool apg_file_to_str( const char* filename, size_t max_len, char* str ) {
  if ( !filename || 0 == max_len || !str ) { return false; }

  FILE* fp = fopen( filename, "rb" );
  if ( !fp ) { return false; }
  fseek( fp, 0L, SEEK_END );
  size_t file_sz = (size_t)ftell( fp );
  size_t read_sz = file_sz > ( max_len - 1 ) ? ( max_len - 1 ) : file_sz;
  rewind( fp );
  size_t nr = fread( str, read_sz, 1, fp );
  fclose( fp );
  str[read_sz] = '\0';
  if ( 1 != nr ) { return false; }
  return true;
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
  default: {
    apg_log_err( "FATAL ERROR: signal %i:\n", sig );
  } break;
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

/*=================================================================================================
COMPRESSION
=================================================================================================*/

void apg_rle_compress( const uint8_t* bytes_in, size_t sz_in, uint8_t* bytes_out, size_t* sz_out ) {
  assert( sz_out );
  if ( !sz_out ) { return; }
  if ( !bytes_in || sz_in == 0 ) { *sz_out = 0; }

  size_t out_n = 0;
  for ( size_t i = 0; i < sz_in; i++ ) {
    uint8_t count = 1;
    if ( ( i < sz_in - 1 ) && ( bytes_in[i] == bytes_in[i + 1] ) ) { // WARNING clang-tidy "array access from bytes_in results in a null pointer dereference
      count = 2;
      for ( size_t j = i + 2; j < sz_in && count < UINT8_MAX; j++ ) {
        if ( bytes_in[j] != bytes_in[i] ) { break; }
        count++;
      }
    }
    if ( bytes_out ) {
      bytes_out[out_n] = bytes_in[i]; // WARNING clang-tidy "array access from bytes_in results in a null pointer dereference
      if ( count >= 2 ) {             // eg convert AAA to AA3 and AAAA to AA4. AA expands to AA2. A alone stays A
        bytes_out[out_n + 1] = bytes_in[i];
        bytes_out[out_n + 2] = count;
      }
    }
    out_n++;
    if ( count >= 2 ) {
      out_n += 2; // eg DDDD->DD4 so 3 total. D + D4 -> 1 + 2.
      i += ( count - 1 );
    }
  }
  *sz_out = out_n;
}

void apg_rle_decompress( const uint8_t* bytes_in, size_t sz_in, uint8_t* bytes_out, size_t* sz_out ) {
  assert( sz_out );
  if ( !sz_out ) { return; }
  if ( !bytes_in || sz_in == 0 ) { *sz_out = 0; }

  size_t out_n = 0;
  for ( size_t i = 0; i < sz_in; i++ ) {
    uint8_t count = 1;
    // look for 2 in a row then expect a number
    if ( ( i < sz_in - 2 ) && ( bytes_in[i] == bytes_in[i + 1] ) ) { count = bytes_in[i + 2]; }
    if ( bytes_out ) {
      for ( uint8_t j = 0; j < count; j++ ) { bytes_out[out_n + j] = bytes_in[i]; }
    }
    out_n += count;
    if ( count > 1 ) { i += 2; }
  }
  *sz_out = out_n;
}

/*=================================================================================================
HASH TABLE
=================================================================================================*/

apg_hash_table_t apg_hash_table_create( int table_n ) {
  apg_hash_table_t table = ( apg_hash_table_t ){ .n = 0 };
  if ( table_n < 0 ) { return table; }
  table.list_ptr = calloc( table_n, sizeof( apg_hash_table_element_t ) );
  if ( !table.list_ptr ) { return table; } // OOM error.
  table.n = table_n;
  return table;
}

void apg_hash_table_free( apg_hash_table_t* table_ptr ) {
  if ( !table_ptr ) { return; }
  // Free any allocated key strings.
  for ( int i = 0; i < table_ptr->n; i++ ) {
    if ( table_ptr->list_ptr[i].value_ptr ) {
      if ( table_ptr->list_ptr[i].keystr ) { free( table_ptr->list_ptr[i].keystr ); }
    }
  }
  if ( table_ptr->list_ptr ) { free( table_ptr->list_ptr ); }
  *table_ptr = ( apg_hash_table_t ){ .n = 0 };
}

/** Return a hash index ( hash code ) for a single value key->table mapping.

TODO(Anton) reusue this for a hash-set implementation?

* Golden ratio is (1+sqrt(5))/2 = 1.618033988749...
 *  The fractional part is useful as a multiplier.
 *
#define APG_GOLDEN_RATIO_FRAC 0.618033988749

uint32_t apg_hashi( uint32_t key, uint32_t table_n ) {
  double int_part     = 0.0;
  uint32_t hash_index = (uint32_t)( (double)table_n * modf( (double)key * APG_GOLDEN_RATIO_FRAC, &int_part ) );
  return hash_index;
}
*/

uint32_t apg_hash( const char* keystr ) {
  // sdbm based on http://www.cse.yorku.ca/~oz/hash.html
  uint32_t hash = 0;
  size_t len    = strlen( keystr );
  for ( uint32_t i = 0; i < len; i++ ) { hash = keystr[i] + ( hash << 6 ) + ( hash << 16 ) - hash; }
  return hash;
}

uint32_t apg_hash_rehash( const char* keystr ) {
  // djb2 based on http://www.cse.yorku.ca/~oz/hash.html
  uint32_t hash = 5381;
  size_t len    = strlen( keystr );
  for ( uint32_t i = 0; i < len; i++ ) { hash = ( ( hash << 5 ) + hash ) + keystr[i]; }
  return hash;
}

bool apg_hash_store( const char* keystr, void* value_ptr, apg_hash_table_t* table_ptr, int* collision_ptr ) {
  if ( !keystr || !value_ptr || !table_ptr ) { return false; }
  if ( table_ptr->count_stored >= table_ptr->n ) { return false; } // Table full. Should resize before here.

  int collisions = 0;
  uint32_t hash  = apg_hash( keystr );
  uint32_t idx   = hash % table_ptr->n;

  // Check for best case scenario: landed on an empty index first try.
  if ( NULL == table_ptr->list_ptr[idx].value_ptr ) { goto enter_key_label; }

  // Otherwise, first try a rehash.
  if ( strcmp( keystr, table_ptr->list_ptr[idx].keystr ) == 0 ) { return false; } // Key is already in table.
  collisions++;
  hash = apg_hash_rehash( keystr );
  idx  = hash % table_ptr->n;

  // Then proceed with linear probing from the rehashed index.
  for ( int i = 0; i < table_ptr->n; i++ ) {
    if ( NULL == table_ptr->list_ptr[idx].value_ptr ) { goto enter_key_label; }     // Needs to be at top of loop since also covers rehash's first check.
    if ( strcmp( keystr, table_ptr->list_ptr[idx].keystr ) == 0 ) { return false; } // Key is already in table.
    collisions++;
    idx = ( idx + 1 ) % table_ptr->n;
  }

  assert( false && "Shouldn't get here because it means the table is full, and we DO check for that earlier." );
  return false;

enter_key_label:
  table_ptr->list_ptr[idx]        = ( apg_hash_table_element_t ){ .value_ptr = value_ptr };
  table_ptr->list_ptr[idx].keystr = strdup( keystr ); // NOTE(Anton) Could use strndup here to guard against unterminated strings.
  table_ptr->count_stored++;
  if ( collision_ptr ) { *collision_ptr = *collision_ptr + collisions; }
  return true;
}

bool apg_hash_search( const char* keystr, apg_hash_table_t* table_ptr, uint32_t* idx_ptr, int* collision_ptr ) {
  if ( !keystr || !table_ptr || !idx_ptr || table_ptr->count_stored == 0 ) { return false; }

  uint32_t hash = apg_hash( keystr );
  uint32_t idx  = hash % table_ptr->n;
  if ( !table_ptr->list_ptr[idx].value_ptr ) { return false; }

  if ( strcmp( keystr, table_ptr->list_ptr[idx].keystr ) == 0 ) {
    *idx_ptr = idx;
    return true;
  }
  // First do a rehash.
  if ( collision_ptr ) { ( *collision_ptr )++; }
  hash = apg_hash_rehash( keystr );
  idx  = hash % table_ptr->n;
  // With linear probing following on from there.
  for ( int i = 0; i < table_ptr->n; i++ ) {
    if ( !table_ptr->list_ptr[idx].value_ptr ) { return false; }
    if ( strcmp( keystr, table_ptr->list_ptr[idx].keystr ) == 0 ) {
      *idx_ptr = idx;
      return true;
    }
    if ( collision_ptr ) { ( *collision_ptr )++; }
    idx = ( idx + 1 ) % table_ptr->n;
  }
  return false; // This only happens if the table is full, and the key isn't in there.
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
