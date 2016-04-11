#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// note: can't save/load whole struct directly because:
// * endianness differences on write/read machines
// * struct byte alignment/padding which may be inconsistent between systems

struct Save_Data{
	int counter;
	float real;
	bool truth;
	char name[256];
};
typedef struct Save_Data Save_Data;

void print_sd( Save_Data sd ){
	printf("\
counter %i\n\
real %f\n\
truth %i\n\
name %s\n",
	sd.counter, sd.real, sd.truth, sd.name
	);
}

Save_Data create_sd(){
	Save_Data sd;
	memset( &sd, 0, sizeof( Save_Data ));
	sd.counter = 1;
	sd.real = 22.2f;
	sd.truth = true;
	strcpy( sd.name, "anton" );
	printf ("sd.real is %f\n", sd.real);
	return sd;
}

void write_sd( Save_Data sd ){
	FILE* fp = fopen( "save.dat", "wb" );
	assert( fp );
	assert( fwrite( &sd.counter, sizeof( int ), 1, fp ) > 0 );
	assert( fwrite( &sd.real, sizeof( float ), 1, fp ) > 0 );
	assert( fwrite( &sd.truth, sizeof( bool ), 1, fp ) > 0 );
	assert( fwrite( sd.name, sizeof( char ), 256, fp ) > 0 );
	fclose( fp );
}

Save_Data read_sd(){
	Save_Data sd;
	FILE* fp = fopen( "save.dat", "rb" );
	assert( fp );
	assert( fread( &sd.counter, sizeof( int ), 1, fp ) > 0 );
	assert( fread( &sd.real, sizeof( float ), 1, fp ) > 0 );
	assert( fread( &sd.truth, sizeof( bool ), 1, fp ) > 0 );
	assert( fread( sd.name, sizeof( char ), 256, fp ) > 0 );
	fclose( fp );
	return sd;
}

int main(){
	Save_Data sda = create_sd();
	write_sd( sda );

	Save_Data sdb = read_sd();
	print_sd( sdb );
	return 0;
}
