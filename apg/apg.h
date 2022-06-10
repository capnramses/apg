/* apg.h  Author's generic C utility functions.
Author:   Anton Gerdelan  antongerdelan.net
Licence:  See bottom of this file.
Language: C89 interface, C99 implementation.

Version History and Copyright
-----------------------------
  1.8.1 - 28 Mar 2022. Casting precision fix to gbfs.
  1.8   - 27 Mar 2022. Greedy BFS uses 64-bit integers (suited a project I used it in).
  1.7   - 22 Mar 2022. Greedy BFS speed improvement using bsearch & memmove suffle.
  1.6   - 13 Mar 2022. Greedy Best-First Search first implementation.
  1.5   - 13 Mar 2022. Tidied MSVC build. Added a .bat file for building hash_test.c.
  1.4   - 12 Mar 2022. Hash table functions.
  1.3   - 11 Sep 2020. Fixed apg_file_to_str() portability issue.
  1.2   - 15 May 2020. Updated timers for multi-platform use based on Professional Programming Tools book code. Updated test code.
  1.1   -  4 May 2020. Added custom rand() functions.
  1.0   -  8 May 2015. First version by Anton Gerdelan.

Usage Instructions
-----------------------------
* Just copy-paste the snippets from this file that you want to use.
* Or, to use all of it:
  * In one file #define APG_IMPLEMENTATION above the #include.
  * For backtraces on Windows you need to link against -limagehlp (MinGW/GCC), or /link imagehlp.lib (MSVC/cl.exe).
    You can exclude this by:

  #define APG_NO_BACKTRACES
  #include apg.h

ToDo
-----------------------------
* linearise/unlinearise function. Note(Anton): I can't remember what this even means.
* more string manipulation/trimming utils
* maybe a generic KEY VALUE settings file parser that can be queried for KEY to get VALUE
* swap. everyone has a swap in their utils file
*/

#ifndef _APG_H_
#define _APG_H_

#define _FILE_OFFSET_BITS 64 /* Required for ftello on e.g. MinGW to use 8 bytes instead of 4. This can also be defined in a compile string/build file. */
#include <stdbool.h>
#include <stddef.h>   /* size_t */
#include <stdint.h>   /* types */
#include <stdio.h>    /* FILE* */
#include <sys/stat.h> /* File sizes and details. */

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
// These defines allow support of >2GB files on different platforms. Was not required on my Linux with GCC, but was on Windows with GCC on the same hardware.
// TODO(Anton) Test in MSVC
#ifdef _MSC_VER // This means "if MSVC" because we prefer POSIX stuff on MINGW.
#define apg_fseek _fseeki64
#define apg_ftell _ftelli64
#define apg_stat _stat64
#define apg_stat_t __stat64
#else
#define apg_fseek fseeko
#define apg_ftell ftello
#define apg_stat stat
#define apg_stat_t stat
#endif

/** Represents memory loaded from a file. */
typedef struct apg_file_t {
  void* data_ptr;
  size_t sz; // Size of memory pointed to by data_ptr in bytes.
} apg_file_t;

/** @return size in bytes of file given by filename, or -1 on error. Supports large (multi-GB) files. */
int64_t apg_file_size( const char* filename );

/** Reads an entire file into memory, unaltered. Supports large (multi-GB) files.
 *
 * @return
 *   true on success. In this case record->data is allocated memory and must be freed by the caller.
 *   false on any error. Any allocated memory is freed if false is returned.
 *
 * @warning If you are also writing very large files, be aware some platforms (Windows) will stall if fwrite()s are not split into <=2GB chunks.
 */
bool apg_read_entire_file( const char* filename, apg_file_t* record );

/** Loads file_name's contents into a byte array and always ends with a NULL terminator.
 * @param max_len Maximum bytes available to write into str_ptr.
 * @return false on any error, and if the file size + 1 exceeds max_len bytes.
 */
bool apg_file_to_str( const char* file_name, int64_t max_len, char* str_ptr );

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

// NB `ULL` postfix is necessary or numbers ~4GB will be interpreted as integer constants and overflow.
#define APG_KILOBYTES( value ) ( (value)*1024ULL )
#define APG_MEGABYTES( value ) ( APG_KILOBYTES( value ) * 1024ULL )
#define APG_GIGABYTES( value ) ( APG_MEGABYTES( value ) * 1024ULL )

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
   To explicitly allow (constrained) resizing:
   * After a key is stored with apg_hash_store(), run apg_hash_table_auto_resize( &my_table, max_bytes ).

Potential improvements:
 - If the user program reliably retains strings as well as values, we could avoid string memory allocation during hash_store calls, and just point to external.
 - If I also stored the hash in apg_hash_table_element_t it would avoid many potentially lengthy strcmp() calls during search.
 - String safety isn't checked at all. strndup and strncmp could be used if the user supplies a maximum string length.
 - Could use quadratic probing instead of liner probing.
 ================================================================================================*/

typedef struct apg_hash_table_element_t {
  char* keystr;    // This is either an allocated ASCII string or an integer value.
  void* value_ptr; // Address of value in user code. Value data is not allocated or stored directly in the table. If NULL then element is empty.
} apg_hash_table_element_t;

typedef struct apg_hash_table_t {
  apg_hash_table_element_t* list_ptr;
  uint32_t n;
  uint32_t count_stored;
} apg_hash_table_t;

/** Allocates memory for a hash table of size `table_n`.
 * @param table_n For a well performing table use a number somewhat larger than required space.
 * @return A generated, empty, hash table, or an empty table ( list_ptr == NULL ) on out of memory error.
 */
apg_hash_table_t apg_hash_table_create( uint32_t table_n );

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
bool apg_hash_store( const char* keystr, void* value_ptr, apg_hash_table_t* table_ptr, uint32_t* collision_ptr );

/**
 * @return This function returns true if the key is found in the table. In this case the integer pointed to by `idx_ptr` is set to the corresponding table
 * index. This function returns false if the table is empty, the parameters are invalid, or the key is not stored in the table.
 */
bool apg_hash_search( const char* keystr, apg_hash_table_t* table_ptr, uint32_t* idx_ptr, uint32_t* collision_ptr );

/** Expand when hash table when >= 50% full, and double its size if so, but don't allocate a table of more than `max_bytes`.
 *  This function could be improved in performance (at expense of brevity) by manually writing out apg_hash_store() and excluding string allocations.
 *  This function could be upgraded into _auto_resize() which also scales down on e.g. < 25% load.
 */
bool apg_hash_auto_expand( apg_hash_table_t* table_ptr, size_t max_bytes );

/*=================================================================================================
GREEDY BEST-FIRST SEARCH
=================================================================================================*/

/// If a node can have more than 6 neighbours change this value to set the size of the array of neighbour keys.
#define APG_GBFS_NEIGHBOURS_MAX 6

/// Aux. memory retained to represent a 'vertex' in the search graph.
typedef struct apg_gbfs_node_t {
  int64_t parent_idx; // Index of parent in the evaluated_nodes list.
  int64_t our_key;    // Identifying key of the original node (e.g. a tile or pixel index in an array).
  int64_t h;          // Distance to goal.
} apg_gbfs_node_t;

/** Greedy best-first search.
 * This function was designed so that no heap memory is allocated. It has some stack memory limits but that's usually fine for real-time applications.
 * It will return false if these limits are reached for big mazes. It could be modified to use or realloc() heap memory to solve for these cases.
 * I usually use an index or a handles as unique O(1) look-up for graph nodes/voxels/etc. But these could also have been pointers/addresses.
 *
 * @param start_key,target_key  The user provides initial 2 node/vertex keys, expressed as integers
 * @param h_cb_ptr()            User-defined function to return a distance heuristic, h, for a key.
 * @param neighs_cb_ptr()       User-defined function to pass an array of up to 6 (for now) neighbours' keys.
 *                              It should return the count of keys in the array.
 * @param reverse_path_ptr      Pointer to a user-created array of size `max_path_steps`.
 *                              On success the function will write the reversed path of keys into this array.
 * @param path_n                The number of steps in reverse_path_ptr is written to the integer at address `path_n`.
 * @param evaluated_nodes_ptr   User-allocated array of working memory used. Size in bytes is sizeof(apg_gbfs_node_t) * evaluated_nodes_max.
 * @param evaluated_nodes_max   Count of `apg_gbfs_node_t`s allocated to evaluated_nodes_ptr. Worst case - bounds of search domain.
 * @param visited_set_ptr       User-allocated array of working memory used. Size in bytes is sizeof(int) * visited_set_max.
 * @param visited_set_max       Count of `int`s allocated to evaluated_nodes_ptr. Worst case - bounds of search domain.
 * @param queue_ptr             User-allocated array of working memory used. Size in bytes is sizeof(apg_gbfs_node_t) * queue_max.
 * @param queue_max             Count of `apg_gbfs_node_t`s allocated to evaluated_nodes_ptr. Worst case - bounds of search domain.
 * @return                      If a path is found the function returns `true`.
 *                              If no path is found, or there was an error, such as array overflow, then the function returns `false`.
 *
 * @note I let the user supply the working sets (queue, evualated, and visited set) memory. This allows bigger searches than using small stack arrays,
 * and can avoid syscalls. Repeated searches can reuse any allocated memory.
 */
bool apg_gbfs( int64_t start_key, int64_t target_key, int64_t ( *h_cb_ptr )( int64_t key, int64_t target_key ),
  int64_t ( *neighs_cb_ptr )( int64_t key, int64_t target_key, int64_t* neighs ), int64_t* reverse_path_ptr, int64_t* path_n, int64_t max_path_steps,
  apg_gbfs_node_t* evaluated_nodes_ptr, int64_t evaluated_nodes_max, int64_t* visited_set_ptr, int64_t visited_set_max, apg_gbfs_node_t* queue_ptr, int64_t queue_max );

/*=================================================================================================
------------------------------------------IMPLEMENTATION------------------------------------------
=================================================================================================*/
#ifdef APG_IMPLEMENTATION
#undef APG_IMPLEMENTATION

#include <assert.h>
#include <math.h>   /* modff() */
#include <signal.h> /* for crash handling */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h> /* for backtraces and timers */
#ifndef APG_NO_BACKTRACES
#include <dbghelp.h> /* SymInitialize */
#endif
#else
#include <execinfo.h>
#include <strings.h> /* for strcasecmp */
#include <unistd.h>  /* linux-only? */
#endif
/* includes for timers */
#ifdef _WIN32
#include <profileapi.h>
#elif __APPLE__
#include <mach/mach_time.h>
#else
#include <sys/time.h>
#endif
/* fix used in bgfx and imgui to get around mingw not supplying alloca.h */
#if defined( _MSC_VER ) || defined( __MINGW32__ )
#include <malloc.h>
#else
#include <alloca.h>
#endif
#ifdef _MSC_VER
// not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define strdup _strdup
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
#ifdef _WIN32
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
int64_t apg_file_size( const char* filename ) {
  struct apg_stat_t buff;
  if ( !filename ) { return -1; }
  int res = apg_stat( filename, &buff );
  if ( res < 0 ) { return -1; }
  int64_t sz = (int64_t)buff.st_size;
  return sz;
}

bool apg_read_entire_file( const char* filename, apg_file_t* record ) {
  FILE* f_ptr   = NULL;
  void* mem_ptr = NULL;
  int64_t sz    = 0;

  if ( !filename || !record ) { goto _apg_read_entire_file_fail; }

  sz = apg_file_size( filename );
  if ( sz < 0 ) { goto _apg_read_entire_file_fail; }

  mem_ptr = malloc( (size_t)sz );
  if ( !mem_ptr ) { goto _apg_read_entire_file_fail; }

  f_ptr = fopen( filename, "rb" );
  if ( !f_ptr ) { goto _apg_read_entire_file_fail; }
  size_t nr = fread( mem_ptr, (size_t)sz, 1, f_ptr );
  if ( 1 != nr ) { goto _apg_read_entire_file_fail; }
  fclose( f_ptr );

  record->sz       = (size_t)sz;
  record->data_ptr = mem_ptr;

  return true;

_apg_read_entire_file_fail:
  if ( !f_ptr ) { fclose( f_ptr ); }
  if ( mem_ptr ) { free( mem_ptr ); }
  return false;
}

bool apg_file_to_str( const char* filename, int64_t max_len, char* str_ptr ) {
  if ( !filename || 0 == max_len || !str_ptr ) { return false; }

  int64_t file_sz = apg_file_size( filename );
  if ( file_sz < 0 ) { return false; }
  if ( file_sz >= max_len - 1 ) { return false; }

  FILE* fp = fopen( filename, "rb" );
  if ( !fp ) { return false; }
  size_t nr = fread( str_ptr, (size_t)file_sz, 1, fp );
  fclose( fp );
  str_ptr[file_sz] = '\0';
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
#ifndef APG_NO_BACKTRACES
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
#endif /* APG_BACKTRACES */
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

apg_hash_table_t apg_hash_table_create( uint32_t table_n ) {
  apg_hash_table_t table = ( apg_hash_table_t ){ .n = 0 };
  if ( table_n == 0 ) { return table; }
  table.list_ptr = calloc( table_n, sizeof( apg_hash_table_element_t ) );
  if ( !table.list_ptr ) { return table; } // OOM error.
  table.n = table_n;
  return table;
}

void apg_hash_table_free( apg_hash_table_t* table_ptr ) {
  if ( !table_ptr ) { return; }
  // Free any allocated key strings.
  for ( uint32_t i = 0; i < table_ptr->n; i++ ) {
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
  for ( size_t i = 0; i < len; i++ ) { hash = ( ( hash << 5 ) + hash ) + keystr[i]; }
  return hash;
}

bool apg_hash_store( const char* keystr, void* value_ptr, apg_hash_table_t* table_ptr, uint32_t* collision_ptr ) {
  if ( !keystr || !value_ptr || !table_ptr ) { return false; }
  if ( table_ptr->count_stored >= table_ptr->n ) { return false; } // Table full. Should resize before here.

  uint32_t collisions = 0;
  uint32_t hash       = apg_hash( keystr );
  uint32_t idx        = hash % table_ptr->n;

  // Check for best case scenario: landed on an empty index first try.
  if ( NULL == table_ptr->list_ptr[idx].value_ptr ) { goto apg_hash_store_enter_key; }

  // Otherwise, first try a rehash.
  if ( strcmp( keystr, table_ptr->list_ptr[idx].keystr ) == 0 ) { return false; } // Key is already in table.
  collisions++;
  hash = apg_hash_rehash( keystr );
  idx  = hash % table_ptr->n;

  // Then proceed with linear probing from the rehashed index.
  for ( uint32_t i = 0; i < table_ptr->n; i++ ) {
    if ( NULL == table_ptr->list_ptr[idx].value_ptr ) { goto apg_hash_store_enter_key; } // Needs to be at top of loop since also covers rehash's first check.
    if ( strcmp( keystr, table_ptr->list_ptr[idx].keystr ) == 0 ) { return false; }      // Key is already in table.
    collisions++;
    idx = ( idx + 1 ) % table_ptr->n;
  }

  assert( false && "Shouldn't get here because it means the table is full, and we DO check for that earlier." );
  return false;

apg_hash_store_enter_key:
  table_ptr->list_ptr[idx]        = ( apg_hash_table_element_t ){ .value_ptr = value_ptr };
  table_ptr->list_ptr[idx].keystr = strdup( keystr ); // NOTE(Anton) Could use strndup here to guard against unterminated strings.
  table_ptr->count_stored++;
  if ( collision_ptr ) { *collision_ptr = *collision_ptr + collisions; }
  return true;
}

bool apg_hash_search( const char* keystr, apg_hash_table_t* table_ptr, uint32_t* idx_ptr, uint32_t* collision_ptr ) {
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
  for ( uint32_t i = 0; i < table_ptr->n; i++ ) {
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

bool apg_hash_auto_expand( apg_hash_table_t* table_ptr, size_t max_bytes ) {
  if ( !table_ptr || 0 == max_bytes ) { return false; }
  if ( table_ptr->count_stored < table_ptr->n / 2 ) { return true; } // Already big enough.
  uint32_t tmp_n = table_ptr->n * 2;
  if ( tmp_n < table_ptr->n ) { return false; } // Overflow check.
  size_t tmp_bytes = tmp_n * sizeof( apg_hash_table_element_t );
  if ( tmp_bytes >= max_bytes ) { return false; } // Too much memory would be used.

  apg_hash_table_t tmp_table = apg_hash_table_create( tmp_n );
  if ( !tmp_table.list_ptr ) { return false; } // OOM.

  // Rehash valid entries to new table size.
  for ( uint32_t i = 0; i < table_ptr->n; i++ ) {
    if ( table_ptr->list_ptr[i].value_ptr ) {
      if ( !apg_hash_store( table_ptr->list_ptr[i].keystr, table_ptr->list_ptr[i].value_ptr, &tmp_table, NULL ) ) {
        apg_hash_table_free( &tmp_table );
        return false;
      }
    }
  }
  apg_hash_table_free( table_ptr ); // free everything including allocated strings.
  *table_ptr = tmp_table;           // allocated list_ptr, including allocated strings, n, count_stored.
  return true;
}

/*=================================================================================================
GREEDY BEST-FIRST SEARCH
=================================================================================================*/

// Called whenever we check if an item has been visited already. should return -ve if key < element.
static int _apg_gbfs_search_vset_comp_cb( const void* key_ptr, const void* element_ptr ) { return (int)( *(int64_t*)key_ptr - *(int64_t*)element_ptr ); }

bool apg_gbfs( int64_t start_key, int64_t target_key, int64_t ( *h_cb_ptr )( int64_t key, int64_t target_key ),
  int64_t ( *neighs_cb_ptr )( int64_t key, int64_t target_key, int64_t* neighs ), int64_t* reverse_path_ptr, int64_t* path_n, int64_t max_path_steps,
  apg_gbfs_node_t* evaluated_nodes_ptr, int64_t evaluated_nodes_max, int64_t* visited_set_ptr, int64_t visited_set_max, apg_gbfs_node_t* queue_ptr, int64_t queue_max ) {
  int64_t n_visited_set = 1, n_queue = 1, n_evaluated_nodes = 0;
  visited_set_ptr[0] = start_key;                                                                                             // Mark start as visited
  queue_ptr[0]       = ( apg_gbfs_node_t ){ .h = h_cb_ptr( start_key, target_key ), .parent_idx = -1, .our_key = start_key }; // and add to queue.
  while ( n_queue > 0 ) {
    // curr is vertex in queue w/ smallest h. Smallest h is always at the end of the queue for easy deletion.
    apg_gbfs_node_t curr = queue_ptr[--n_queue];

    int64_t neigh_keys[APG_GBFS_NEIGHBOURS_MAX];
    int64_t n_neighs = neighs_cb_ptr( curr.our_key, target_key, neigh_keys );
    if ( n_neighs > APG_GBFS_NEIGHBOURS_MAX ) { return false; }
    bool neigh_added = false, found_path = false;
    for ( int64_t neigh_idx = 0; neigh_idx < n_neighs; neigh_idx++ ) {
      if ( neigh_keys[neigh_idx] == target_key ) {
        found_path = neigh_added = true; // Resolve path including the final item's key. Break here and flag so that we add the final node.
        break;
      }

      if ( bsearch( &neigh_keys[neigh_idx], visited_set_ptr, n_visited_set, sizeof( int64_t ), _apg_gbfs_search_vset_comp_cb ) != NULL ) { continue; }

      if ( n_visited_set >= visited_set_max || n_queue >= queue_max ) { return false; }
      { // Custom sort
        // can probably do better than qsort's worst case O(n^2) with our knowledge of the data -> O(n) with a memcpy
        visited_set_ptr[n_visited_set] = neigh_keys[neigh_idx]; // avoids if (comparison not made) check
        for ( int64_t i = 0; i < n_visited_set; i++ ) {
          if ( neigh_keys[neigh_idx] < visited_set_ptr[i] ) {
            // src and dst overlap so using memmove instead of memcpy
            memmove( &visited_set_ptr[i + 1], &visited_set_ptr[i], ( n_visited_set - i ) * sizeof( int64_t ) );
            visited_set_ptr[i] = neigh_keys[neigh_idx];
            break;
          }
        } // endfor
        n_visited_set++;

        int64_t our_h      = h_cb_ptr( neigh_keys[neigh_idx], target_key );
        queue_ptr[n_queue] = ( apg_gbfs_node_t ){ .h = our_h, .parent_idx = n_evaluated_nodes, .our_key = neigh_keys[neigh_idx] };
        for ( int64_t i = 0; i < n_queue; i++ ) {
          if ( our_h > queue_ptr[i].h ) {
            memmove( &queue_ptr[i + 1], &queue_ptr[i], ( n_queue - i ) * sizeof( apg_gbfs_node_t ) );
            queue_ptr[i] = ( apg_gbfs_node_t ){ .h = our_h, .parent_idx = n_evaluated_nodes, .our_key = neigh_keys[neigh_idx] };
            break;
          }
        } // endfor
        n_queue++;
      } // endblock custom sort
      neigh_added = true;
    } // endfor neighbours
    if ( neigh_added ) {
      if ( n_evaluated_nodes >= evaluated_nodes_max ) { return false; }
      evaluated_nodes_ptr[n_evaluated_nodes++] = curr;
    }
    if ( found_path ) {
      int64_t tmp_path_n             = 0;
      int64_t parent_eval_idx        = n_evaluated_nodes - 1;
      reverse_path_ptr[tmp_path_n++] = target_key;
      for ( int64_t i = 0; i < n_evaluated_nodes; i++ ) {     // Some sort of timeout in case of logic error.
        if ( tmp_path_n >= max_path_steps ) { return false; } // Maxed out path length.
        apg_gbfs_node_t path_tmp       = evaluated_nodes_ptr[parent_eval_idx];
        reverse_path_ptr[tmp_path_n++] = path_tmp.our_key;
        parent_eval_idx                = path_tmp.parent_idx;
        if ( path_tmp.parent_idx == -1 ) {
          *path_n = tmp_path_n;
          return true;
        }
      }
      assert( false && "failed to find path back to start" );
      return false;
    }
  } // endwhile queue not empty
  return false;
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
