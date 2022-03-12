#define APG_IMPLEMENTATION
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

    int collisions = 0;

    { // Storing random keys, collision test.

#define SN 32 // hash table 4x the size of the items stored seems reasonable collision wise
#define ASTRLENMAX 16

      srand( time( NULL ) );
      char values[SN][ASTRLENMAX];
      for ( int i = 0; i < SN; i++ ) {
        for ( int j = 0; j < ASTRLENMAX - 1; j++ ) { values[i][j] = rand() % 64 + 'A'; }
        values[i][ASTRLENMAX - 1] = 0;
        if ( !apg_hash_store( values[i], &values[i], &table, &collisions ) ) {
          printf( "ERROR: failed to store key %s in table with %i/%i entries.\n", values[i], table.count_stored, table.n );
          if ( table.count_stored >= table.n ) { printf( "  -> Hash table was full\n" ); } // set SN to bigger than 2048 to test this
          return 1;
        }
        // printf( "= key %s, -> hash index %u\n", values[i], idx );
      }

      int anton_value = 666;
      { // Storing a few easily retrievable keys & search test
        if ( !apg_hash_store( "Anton Gerdelan", &anton_value, &table, &collisions ) ) {
          printf( "ERROR: failed to store key Anton Gerdelan in table with %i/%i entries.\n", table.count_stored, table.n );
          if ( table.count_stored >= table.n ) { printf( "  -> Hash table was full\n" ); } // set SN to bigger than 2048 to test this
          return 1;
        }
        if ( !apg_hash_store( "Anton2", &anton_value, &table, &collisions ) ) {
          printf( "ERROR: failed to store key Anton2 in table with %i/%i entries.\n", table.count_stored, table.n );
          if ( table.count_stored >= table.n ) { printf( "  -> Hash table was full\n" ); } // set SN to bigger than 2048 to test this
          return 1;
        }
      }

      // Test key already in table
      if ( !apg_hash_store( "Anton2", &anton_value, &table, &collisions ) ) {
        printf( "ERROR: failed to store key Anton2 in table with %i/%i entries.\n", table.count_stored, table.n );
        if ( table.count_stored >= table.n ) { printf( "  -> Hash table was full\n" ); } // set SN to bigger than 2048 to test this
        uint32_t search_idx = 0;
        if ( apg_hash_search( "Anton2", &table, &search_idx, &collisions ) ) {
          printf( " -> because key was already in the table at postion %u\n", search_idx );
        } else {
          return 1;
        }
      }

      // print all table entries
      for ( int i = 0; i < table.n; i++ ) {
        if ( table.list_ptr[i].value_ptr ) { printf( "%i) key: %s. value_addr: %p\n", i, table.list_ptr[i].keystr, table.list_ptr[i].value_ptr ); }
      }
      printf( "String hash collisions=%i at capacity %i/%i (%.2f%%)\n", collisions, table.count_stored, table.n, (float)table.count_stored / (float)table.n );
    }

    { // searching
      uint32_t search_idx = 0;
      if ( apg_hash_search( "Anton", &table, &search_idx, &collisions ) ) {
        printf( "ERROR search found non-existent key `Anton` at index %u\n", search_idx );
        return 1;
      }
      if ( apg_hash_search( "Anton Gerdelan", &table, &search_idx, &collisions ) ) { printf( "search found `Anton Gerdelan` at index %i\n", search_idx ); }
      if ( apg_hash_search( "Anton2", &table, &search_idx, &collisions ) ) { printf( "search found `Anton2` at index %i\n", search_idx ); }
    }

    apg_hash_table_free( &table );
  }

  printf( "Normal exit.\n" );
  return 0;
}
