//
// apg.h - generic C99 utility functions
// first version Dr Anton Gerdelan, 8 May 2015
//

//
// TODO
// * linearise/unlinearise function
// * ascii or unicode fetcher/printer thing
// * string manip/trim

#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <strings.h> // for strcasecmp
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

//
// some stuff for multi-platform builds
#ifdef WIN32
#define BUILD_PLAT "win32"
#elif LINUX32
#define BUILD_PLAT "linux32"
#elif LINUX64
#define BUILD_PLAT "linux64"
#elif OSX64
#define BUILD_PLAT "osx64"
#else // probably this should be more sensible
#define BUILD_PLAT "undefined"
#endif

// file name for log
#define APG_LOG_FN "apg.log"

//
// replacement for the deprecated min/max functions from original C spec
// was going to have a series of GL-like functions but it was a lot of
// fiddly code/alternatives, so I'm just copying from stb.h here. as much as I
// dislike pre-processor directives, this makes sense. I believe the trick is
// to have all the parentheses. same deal for clamp
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x ,lo, hi) (MIN (hi, MAX (lo, x)))

// C99 removed M_PI
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TAU 2.0 * M_PI

// unit conversions
#define DEG2RAD(a) ((a)*(M_PI/180.0))
#define RAD2DEG(a) ((a)*(180.0/M_PI))

//
// shorthand types for byte and bit manipulation (I don't like the _t suffix)
//
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8; // most useful one for byte in memory
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef unsigned int uint;

//
// like clamp, but if > max then returns min e.g. loops back to 0
// and if < min, returns max e.g. loops back to 99
// NOTE: does not preserve differences > 1, so suitable for iterating counters
// but not big differences
int loopi (int val, int min, int max);

// checks argv params for a keyword. if so returns index in argv
// i copied this trick from the DOOM source code
int check_param (const char* check);

// open/refresh a new log file and print timestamp
bool restart_apg_log ();
// write a log entry
bool apg_log (const char* message, ...);
// write a log entry and print to stderr
bool apg_log_err (const char* message, ...);

/*****************************************************************************
GLOBAL VAR DECLARATIONS (implement these in main.c)
*****************************************************************************/

extern int g_argc;
extern char** g_argv;

/*****************************************************************************
IMPLEMENTATION
*****************************************************************************/

// NOTE: would it make sense to offset by the difference here, instead of
// a fixed = max or = min? if so a #define might be better b/c it could apply
// to more variable types
inline int loopi (int val, int min, int max) {
	if (val < min) {
		return max;
	}
	if (val > max) {
		return min;
	}
	return val;
}

//
// Checks for given parameter in main's command-line arguments
// returns the argument number if present (1 to argc - 1)
// otherwise returns 0
int check_param (const char* check) {
  for (int i = 1; i < g_argc; i++) {
		// NOTE: the original used strcasecmp() here which is the case insenstive
		// version, but it might require strings.h instead, depending on compiler
		// it makes sense to ignore case on multi-plat command line
    if (strcasecmp (check, g_argv[i]) == 0) {
      return i;
    }
  }
  return 0;
}

// open/refresh a new log file and print timestamp and OS
// returns false if could not open file
inline bool restart_apg_log () {
	FILE* file = fopen (APG_LOG_FN, "w");
  if (!file) {
    fprintf (stderr,
      "ERROR: could not open APG_LOG_FN log file %s for writing\n",
      APG_LOG_FN);
    return false;
  }
  time_t now = time (NULL);
  char* date = ctime (&now);
  fprintf (file, "APG_LOG_FN log. local time %s\n", date);
  fprintf (file, "build version: %s %s %s\n", BUILD_PLAT, __DATE__, __TIME__);
  fclose (file);
  return true;
}

// append a log entry
// returns false if couldn't open file
inline bool apg_log (const char* message, ...) {
  va_list argptr;
  FILE* file = fopen (APG_LOG_FN, "a");
  if (!file) {
    fprintf (stderr,
      "ERROR: could not open APG_LOG_FN %s file for appending\n",
      APG_LOG_FN);
    return false;
  }
  va_start (argptr, message);
  vfprintf (file, message, argptr);
  va_end (argptr);
  fclose (file);
  return true;
}

// append a log entry and print to stderr
// returns false if couldn't open file
inline bool apg_log_err (const char* message, ...) {
	va_list argptr;

	// print to stderr. if log fails at least we still see this
	va_start (argptr, message);
  vfprintf (stderr, message, argptr);
	va_end (argptr);

	// print to log
  FILE* file = fopen (APG_LOG_FN, "a");
  if (!file) {
    fprintf (stderr,
      "ERROR: could not open APG_LOG_FILE %s file for appending\n",
      APG_LOG_FN);
    return false;
  }
  va_start (argptr, message);
  vfprintf (file, message, argptr);
  va_end (argptr);
  va_start (argptr, message);
  va_end (argptr);
  fclose (file);

  return true;
}

