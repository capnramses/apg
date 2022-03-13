/* hash_test.h Test of hash functions from apg.h
Author:   Anton Gerdelan  antongerdelan.net
Licence:  Submitted to the public domain 13 March 2022.
Language: C99
*/

#define APG_IMPLEMENTATION
#define APG_NO_BACKTRACES
#include "../apg.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_STORE 666

int main() {
  printf( "===========================================\n" );
  {
    apg_hash_table_t table = apg_hash_table_create( 128 );
    if ( !table.list_ptr ) { return 1; } // OOM

    uint32_t collisions = 0;

    { // Storing random keys, collision test.

#define SN 64 // hash table 4x the size of the items stored seems reasonable collision wise
#define ASTRLENMAX 16

      // Commented the following line out so I can test for consistency between MSVC/gcc etc.
      // apg_srand( (uint32_t)time( NULL ) );
      char values[SN][ASTRLENMAX];
      for ( uint32_t i = 0; i < SN; i++ ) {
        // I use my own rand so i get consistent results on windows vs linux etc.
        for ( uint32_t j = 0; j < ASTRLENMAX - 1; j++ ) { values[i][j] = apg_rand() % 64 + 'A'; }
        values[i][ASTRLENMAX - 1] = 0;
        if ( !apg_hash_store( values[i], &values[i], &table, &collisions ) ) {
          printf( "ERROR: failed to store key %s in table with %u/%u entries.\n", values[i], table.count_stored, table.n );
          if ( table.count_stored >= table.n ) { printf( "  -> Hash table was full\n" ); } // set SN to bigger than 2048 to test this
          return 1;
        }
      }

      int anton_value = 666;
      { // Storing a few easily retrievable keys & search test
        if ( !apg_hash_store( "Anton Gerdelan", &anton_value, &table, &collisions ) ) {
          printf( "ERROR: failed to store key Anton Gerdelan in table with %u/%u entries.\n", table.count_stored, table.n );
          if ( table.count_stored >= table.n ) { printf( "  -> Hash table was full\n" ); } // set SN to bigger than 2048 to test this
          return 1;
        }
        if ( !apg_hash_store( "Anton2", &anton_value, &table, &collisions ) ) {
          printf( "ERROR: failed to store key Anton2 in table with %u/%u entries.\n", table.count_stored, table.n );
          if ( table.count_stored >= table.n ) { printf( "  -> Hash table was full\n" ); } // set SN to bigger than 2048 to test this
          return 1;
        }
      }

      // Test key already in table
      if ( !apg_hash_store( "Anton2", &anton_value, &table, &collisions ) ) {
        printf( "ERROR: failed to store key Anton2 in table with %u/%u entries.\n", table.count_stored, table.n );
        if ( table.count_stored >= table.n ) { printf( "  -> Hash table was full\n" ); } // set SN to bigger than 2048 to test this
        uint32_t search_idx = 0;
        if ( apg_hash_search( "Anton2", &table, &search_idx, &collisions ) ) {
          printf( " -> because key was already in the table at postion %u\n", search_idx );
        } else {
          return 1;
        }
      }

      // print all table entries
      for ( uint32_t i = 0; i < table.n; i++ ) {
        if ( table.list_ptr[i].value_ptr ) { printf( "%u) key: %s. value_addr: %p\n", i, table.list_ptr[i].keystr, table.list_ptr[i].value_ptr ); }
      }
      printf( "String hash collisions=%u at capacity %u/%u (%.2f%%)\n", collisions, table.count_stored, table.n, (float)table.count_stored / (float)table.n );
    }

    { // searching
      uint32_t search_idx = 0;
      if ( apg_hash_search( "Anton", &table, &search_idx, &collisions ) ) {
        printf( "ERROR search found non-existent key `Anton` at index %u\n", search_idx );
        return 1;
      }
      if ( apg_hash_search( "Anton Gerdelan", &table, &search_idx, &collisions ) ) { printf( "search found `Anton Gerdelan` at index %u\n", search_idx ); }
      if ( apg_hash_search( "Anton2", &table, &search_idx, &collisions ) ) { printf( "search found `Anton2` at index %u\n", search_idx ); }
    }

    printf( "table cap %u/%u\n", table.count_stored, table.n );
    // try auto-expand (should be right on the realloc point at 50%)
    if ( !apg_hash_auto_expand( &table, APG_GIGABYTES( 4 ) ) ) {
      printf( "ERROR: realloc failed1\n" );
      return 1;
    }
    printf( "new table cap %u/%u\n", table.count_stored, table.n );
    // try again - should do nothing
    if ( !apg_hash_auto_expand( &table, APG_GIGABYTES( 4 ) ) ) {
      printf( "ERROR: realloc failed2\n" );
      return 1;
    }
    printf( "new table cap %u/%u\n", table.count_stored, table.n );

    apg_hash_table_free( &table );
  }

  printf( "Normal exit.\n" );
  return 0;
}
