// Copyright Anton Gerdelan <antonofnote@gmail.com>. 2019
#include "utils.h"
#ifdef APG_TRACE_UTILS
#ifdef _WIN32
#include <Windows.h>
#include <dbghelp.h> // SymInitialize
#else
#include <execinfo.h>
#endif
#include <signal.h>
#endif // APG_TRACE_UTILS
#ifdef APG_SLEEP
#include <unistd.h>
#endif // APG_SLEEP
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

// ---------------------------------- backtraces and dumps -----------------------------------------
// =================================================================================================
#ifdef APG_TRACE_UTILS
#define BT_BUF_SIZE 100

static void _crash_handler( int sig ) {
  switch ( sig ) {
  case SIGSEGV: {
    glog_err( "FATAL ERROR: SIGSEGV- signal %i\nOut of bounds memory access or dereferencing a null pointer:\n", sig );
  } break;
  case SIGABRT: {
    glog_err( "FATAL ERROR: SIGABRT - signal %i\nabort or assert:\n", sig );
  } break;
  case SIGFPE: {
    glog_err( "FATAL ERROR: SIGFPE - signal %i\nArithmetic - probably a divide-by-zero or integer overflow:\n", sig );
  } break;
  case SIGILL: {
    glog_err( "FATAL ERROR: SIGILL - signal %i\nIllegal instruction - probably function pointer invalid or stack overflow:\n", sig );
  } break;
  default: { glog_err( "FATAL ERROR: signal %i:\n", sig ); } break;
  } // endswitch
  // note(anton) sigbus didnt exist on my mingw32 gcc

  FILE* file = fopen( GLOG_FILE, "a" );
  if ( file ) {
    print_trace( file );
    fclose( file );
  }
  print_trace( stderr );
  exit( 1 );
}

void print_trace( FILE* stream ) {
  assert( stream );

#ifdef _WIN32
  { // NOTE: need a .pdb to read symbols on windows. gcc just needs -g -rdynamic on linux/mac. call cv2pdb myprog.exe -- https://github.com/rainers/cv2pdb
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
    // NOTE(anton) this is for x64. for _M_IA64 or _M_IX86 use different names. read this for shipping: http://blog.morlad.at/blah/mingw_postmortem
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
    } // endfor
    SymCleanup( process );
  }
#else // TODO(anton) test on OS X
  void* array[BT_BUF_SIZE];
  int size       = backtrace( array, BT_BUF_SIZE );
  char** strings = backtrace_symbols( array, size );
  if ( strings == NULL ) {
    perror( "backtrace_symbols" ); // also print internal error to stderr
    exit( EXIT_FAILURE );
  }
  fprintf( stream, "Obtained %i stack frames.\n", size );
  for ( int i = 0; i < size; i++ ) fprintf( stream, "%s\n", strings[i] );
  free( strings );
#endif
}

// to deliberately cause a sigsev: call a function containing bad ptr: int *foo = (int*)-1;
void start_crash_handler() {
  signal( SIGSEGV, _crash_handler );
  signal( SIGABRT, _crash_handler ); // assert
  signal( SIGILL, _crash_handler );
  signal( SIGFPE, _crash_handler ); // ~ int div 0
  // no sigbus on my mingw
}

#endif // APG_TRACE_UTILS

#ifdef UNIT_TESTS
void deliberate_sigsegv() {
  int* bad = (int*)-1;
  printf( "%i\n", *bad );
}

void deliberate_divzero() {
  int a   = rand();
  int b   = a - a;
  int bad = a / b;
  printf( "%i\n", bad );
}
#endif

// --------------------------------------- log files -----------------------------------------------
// =================================================================================================
void glog_start() {
  FILE* file = fopen( GLOG_FILE, "w" ); // NOTE it was getting massive with "a"
  if ( !file ) {
    fprintf( stderr, "ERROR: could not open GLOG_FILE log file %s for writing\n", GLOG_FILE );
    return;
  }
  time_t now = time( NULL );
  char* date = ctime( &now );
  fprintf( file, "\n------------ %s log. local time %s\n", GLOG_FILE, date );
  fclose( file );
}

void glog( const char* message, ... ) {
  va_list argptr;
  FILE* file = fopen( GLOG_FILE, "a" );
  if ( !file ) {
    fprintf( stderr, "ERROR: could not open GLOG_FILE %s file for appending\n", GLOG_FILE );
    return;
  }
  va_start( argptr, message );
  vfprintf( file, message, argptr );
  va_end( argptr );
  fclose( file );
  //  va_start( argptr, message );
  //  vfprintf( stdout, message, argptr );
  //  va_end( argptr );
}

void glog_err( const char* message, ... ) {
  va_list argptr;
  FILE* file = fopen( GLOG_FILE, "a" );
  if ( !file ) {
    fprintf( stderr, "ERROR: could not open GLOG_FILE %s file for appending\n", GLOG_FILE );
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

// =================================================================================================
#ifdef APG_SLEEP
void apg_sleep_ms( int ms ) {
#ifdef WIN32
  Sleep( ms ); // NOTE(Anton) may not need this since using gcc on Windows and usleep() works
#elif _POSIX_C_SOURCE >= 199309L
  struct timespec ts;
  ts.tv_sec  = ms / 1000;
  ts.tv_nsec = ( ms % 1000 ) * 1000000;
  nanosleep( &ts, NULL );
#else
  usleep( ms * 1000 );
#endif
}
#endif // APG_SLEEP
// ----------------------------------------- memory ------------------------------------------------
// =================================================================================================
static void *_g_scratch_a_ptr, *_g_scratch_b_ptr, *_g_scratch_c_ptr;
static size_t _g_scratch_a_sz, _g_scratch_b_sz, _g_scratch_c_sz;

void scratch_init( size_t scratch_a_sz, size_t scratch_b_sz, size_t scratch_c_sz ) {
  _g_scratch_a_ptr = realloc( _g_scratch_a_ptr, scratch_a_sz );
  assert( _g_scratch_a_ptr );
  _g_scratch_a_sz = scratch_a_sz;

  _g_scratch_b_ptr = realloc( _g_scratch_b_ptr, scratch_b_sz );
  assert( _g_scratch_b_ptr );
  _g_scratch_b_sz = scratch_b_sz;

  _g_scratch_c_ptr = realloc( _g_scratch_c_ptr, scratch_c_sz );
  assert( _g_scratch_c_ptr );
  _g_scratch_c_sz = scratch_c_sz;
}

void scratch_free() {
  if ( _g_scratch_a_ptr ) {
    free( _g_scratch_a_ptr );
    _g_scratch_a_ptr = NULL;
    _g_scratch_a_sz  = 0;
  }
  if ( _g_scratch_b_ptr ) {
    free( _g_scratch_b_ptr );
    _g_scratch_b_ptr = NULL;
    _g_scratch_b_sz  = 0;
  }
  if ( _g_scratch_c_ptr ) {
    free( _g_scratch_c_ptr );
    _g_scratch_c_ptr = NULL;
    _g_scratch_c_sz  = 0;
  }
}

void* scratch_mem_a( size_t sz ) {
  assert( _g_scratch_a_ptr );
  assert( sz < _g_scratch_a_sz );
  memset( _g_scratch_a_ptr, 0, sz );
  return _g_scratch_a_ptr;
}

void* scratch_mem_b( size_t sz ) {
  assert( _g_scratch_b_ptr );
  assert( sz < _g_scratch_b_sz );
  memset( _g_scratch_b_ptr, 0, sz );
  return _g_scratch_b_ptr;
}

void* scratch_mem_c( size_t sz ) {
  assert( _g_scratch_c_ptr );
  assert( sz < _g_scratch_c_sz );
  memset( _g_scratch_c_ptr, 0, sz );
  return _g_scratch_c_ptr;
}
