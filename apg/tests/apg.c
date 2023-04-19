#include "apg.h"

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
static apg_rand_t _srand_next = 1;

void apg_srand( apg_rand_t seed ) { _srand_next = seed; }

int apg_rand( void ) {
  _srand_next = _srand_next * 1103515245 + 12345;
  return (int)( _srand_next / ( ( APG_RAND_MAX + 1 ) * 2 ) ) % ( APG_RAND_MAX + 1 );
}

float apg_randf( void ) { return (float)apg_rand() / (float)APG_RAND_MAX; }

apg_rand_t apg_get_srand_next( void ) { return _srand_next; }

int apg_rand_r( apg_rand_t* seed_ptr ) {
  assert( seed_ptr );
  if ( !seed_ptr ) { return 0; }
  *seed_ptr = *seed_ptr * 1103515245 + 12345;
  return (int)( *seed_ptr / ( ( APG_RAND_MAX + 1 ) * 2 ) ) % ( APG_RAND_MAX + 1 );
}

float apg_randf_r( apg_rand_t* seed_ptr ) {
  assert( seed_ptr );
  if ( !seed_ptr ) { return 0.0f; }
  return (float)apg_rand_r( seed_ptr ) / (float)APG_RAND_MAX;
}

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
#ifndef _MSC_VER
#include <dirent.h> /* Directories. */
#endif

bool apg_is_file( const char* path ) {
  struct apg_stat_t path_stat;
  if ( 0 != apg_stat( path, &path_stat ) ) { return false; }
#ifdef _MSC_VER
  return path_stat.st_mode & _S_IFREG;
#else /* POSIX */
  return S_ISREG( path_stat.st_mode );
#endif
}

bool apg_is_dir( const char* path ) {
  char tmp[2048];
  { // Remove trailing slashes because Windows/MinGW stat() can't handle them.
    tmp[0] = '\0';
    apg_strncat( tmp, path, 2047, 2047 );
    int len = (int)strlen( tmp );
    if ( len > 1 && tmp[len - 2] == '\\' && tmp[len - 1] == '\\' ) { tmp[len - 2] = tmp[len - 1] = '\0'; }
    if ( len > 0 && ( tmp[len - 1] == '/' || tmp[len - 1] == '\\' ) ) { tmp[len - 1] = '\0'; }
  }
  struct apg_stat_t path_stat;
  if ( 0 != apg_stat( tmp, &path_stat ) ) { return false; }
#ifdef _MSC_VER
  return path_stat.st_mode & _S_IFDIR;
#else /* POSIX */
  return S_ISDIR( path_stat.st_mode );
#endif
}

int64_t apg_file_size( const char* filename ) {
  struct apg_stat_t buff;
  if ( !filename ) { return -1; }
  int res = apg_stat( filename, &buff );
  if ( res < 0 ) { return -1; }
  int64_t sz = (int64_t)buff.st_size;
  return sz;
}

/** Make sure a path string ends with a Unix-style directory slash. */
static bool _fix_dir_slashes( char* path, int max_len ) {
  int len = (int)strlen( path );
  // "anton\\"
  if ( len > 2 && path[len - 2] == '\\' && path[len - 1] == '\\' ) {
    path[len - 2] = '/';
    path[len - 1] = '\0';
    // "anton\"
  } else if ( len >= 1 && path[len - 1] == '\\' ) {
    path[len - 1] = '/';
    path[len]     = '\0';
    // "anton"
  } else if ( len >= 1 && path[len - 1] != '/' ) {
    if ( len + 1 >= max_len ) { return false; }
    path[len]     = '/';
    path[len + 1] = '\0';
  }
  return true;
}

static int _dir_contents_count( const char* path ) {
  char tmp[2048];
  int count = 0;
  if ( !path ) { return count; }
  if ( !apg_is_dir( path ) ) { return count; }
#ifdef _MSC_VER /* MSVC */
  WIN32_FIND_DATA fdFile;
  HANDLE hFind = NULL;
  sprintf( tmp, "%s/*.*", path ); // Specify a file mask. "*.*" means we want everything!
  if ( ( hFind = FindFirstFile( tmp, &fdFile ) ) == INVALID_HANDLE_VALUE ) { return count; }
  do { count++; } while ( FindNextFile( hFind, &fdFile ) ); // Find the next file.
  FindClose( hFind );                                       // Clean-up global state.
#else                                                       /* POSIX (including MinGW on Windows) */
  struct dirent* entry;
  struct apg_stat_t path_stat;
  DIR* folder = opendir( path );
  if ( folder == NULL ) { return count; }
  while ( ( entry = readdir( folder ) ) ) {
    tmp[0] = '\0';
    apg_strncat( tmp, path, 2045, 2045 );
    if ( !_fix_dir_slashes( tmp, 2047 ) ) { continue; } // Error - path string too long.
    apg_strncat( tmp, entry->d_name, 2047, 2047 );
    if ( 0 != apg_stat( tmp, &path_stat ) ) { continue; }
    if ( S_ISREG( path_stat.st_mode ) || S_ISDIR( path_stat.st_mode ) ) { count++; }
  } // endwhile
  closedir( folder );
#endif
  return count;
}

int _dir_contents_cmp( const void* a, const void* b ) {
  apg_dirent_t* a_ptr = (apg_dirent_t*)a;
  apg_dirent_t* b_ptr = (apg_dirent_t*)b;
  return strcmp( a_ptr->path, b_ptr->path );
}

bool apg_dir_contents( const char* path_ptr, apg_dirent_t** list_ptr, int* n_list ) {
  if ( !path_ptr || !list_ptr || !n_list ) { return false; }
  if ( !apg_is_dir( path_ptr ) ) { return false; }

  apg_dirent_t new_entry;
  char tmp[2048];
  int count = _dir_contents_count( path_ptr ); // Loop over once to let us allocate array in one go.
  int n     = 0;
  *n_list   = 0;
  *list_ptr = calloc( count, sizeof( apg_dirent_t ) );

#ifdef _MSC_VER /* MSVC */
  WIN32_FIND_DATA fdFile;
  HANDLE hFind = NULL;
  sprintf( tmp, "%s/*.*", path_ptr ); // Specify a file mask. "*.*" means we want everything!
  if ( ( hFind = FindFirstFile( tmp, &fdFile ) ) == INVALID_HANDLE_VALUE ) { return count; }
  do {
    tmp[0] = '\0';
    apg_strncat( tmp, path_ptr, 2045, 2045 );
    if ( !_fix_dir_slashes( tmp, 2047 ) ) { continue; } // Error - path string too long.
    apg_strncat( tmp, fdFile.cFileName, 2047, 2047 );

    new_entry.type = APG_DIRENT_FILE;
    if ( fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) { new_entry.type = APG_DIRENT_DIR; }
    new_entry.path     = strdup( fdFile.cFileName );
    ( *list_ptr )[n++] = new_entry;
  } while ( FindNextFile( hFind, &fdFile ) ); // Find the next file.
  FindClose( hFind );                         // Clean-up global state.
#else                                         /* POSIX (including MinGW on Windows) */
  struct apg_stat_t path_stat;
  struct dirent* entry_ptr;
  DIR* folder = opendir( path_ptr );
  if ( folder == NULL ) { return false; }

  while ( ( entry_ptr = readdir( folder ) ) ) {
    tmp[0] = '\0';
    apg_strncat( tmp, path_ptr, 2045, 2045 );
    if ( !_fix_dir_slashes( tmp, 2047 ) ) { continue; } // Error - path string too long.
    apg_strncat( tmp, entry_ptr->d_name, 2047, 2047 );

    if ( 0 != apg_stat( tmp, &path_stat ) ) { continue; }
    new_entry.type = APG_DIRENT_OTHER;
    if ( S_ISREG( path_stat.st_mode ) ) { new_entry.type = APG_DIRENT_FILE; }
    if ( S_ISDIR( path_stat.st_mode ) ) { new_entry.type = APG_DIRENT_DIR; }
    new_entry.path     = strdup( entry_ptr->d_name );
    ( *list_ptr )[n++] = new_entry;
  }
  closedir( folder );
#endif

  *n_list = n;
  // Sort in alphabetical order by default (because mostly I want to print the list).
  qsort( *list_ptr, n, sizeof( apg_dirent_t ), _dir_contents_cmp );
  return true;
}

bool apg_free_dir_contents_list( apg_dirent_t** list_ptr, int n_list ) {
  if ( !list_ptr ) { return false; }
  for ( int i = 0; i < n_list; i++ ) {
    if ( ( *list_ptr )[i].path ) { free( ( *list_ptr )[i].path ); }
  }
  free( *list_ptr );
  *list_ptr = NULL;

  return true;
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

void apg_start_log( void ) {
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
