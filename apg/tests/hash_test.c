#define APG_IMPLEMENTATION
#include "../apg.h"
#include <stdio.h>

#define N_STORE 666

int main() {
  {
    apg_hash_table_t table = apg_hash_table_create( 1024 );
    if ( !table.list_ptr ) { return 1; } // OOM

    int collisions = 0;

    char values[N_STORE];
    for ( int i = 0; i < N_STORE; i++ ) {
      values[i]      = ( i * 35 ) % 60 + 'A';
      char keystr[5] = { i + 3, ( i >> 4 ) + 7, ( i >> 8 ) + 33, ( i >> 12 ) + 37, '\0' };
      //  int idx        = apg_hash_store( keystr, &values[i], &table, &collisions );
      int idx = apg_hash_storei( i, &values[i], &table, &collisions );
      printf( "key %i, value %c -> hash index %i\n", i, values[i], idx );
      if ( idx < 0 ) { return 1; }
    }

    apg_hash_table_free( &table );
    printf( "Collisions=%i\n", collisions );
  }/*
  printf( "===========================================\n" ); 
   {
     apg_hash_table_t table = apg_hash_table_create( 1000 );
     if ( !table.list_ptr ) { return 1; } // OOM

     int collisions = 0;

 #define SN 500 // hash table 4x the size of the items stored seems reasonable collision wise

     char values[SN][32];
     for ( int i = 0; i < SN; i++ ) {
       for ( int j = 0; j < 31; j++ ) { values[i][j] = ( ( i + 2 ) * 35 * ( j + 1 ) ) % 127; }
       values[i][31] = 0;
       uint32_t idx  = apg_hash_store( values[i], &values[i], &table, &collisions );
       printf( "= key %s, -> hash index %u\n", values[i], idx );
     }

     apg_hash_table_free( &table );
     printf( "Collisions=%i\n", collisions );
   }*/
 
  printf( "Normal exit.\n" );
  return 0;
}
