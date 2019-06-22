#ifndef _APG_WAV_H_
#define _APG_WAV_H_

int apg_write_wav( const char* filename, const void* data, int n_chans, int sample_rate, int n_samples, int bits_per_sample );
unsigned char*  apg_read_wav( const char* filename, int* n_chans, int* sample_rate, int* n_samples, int* bits_per_sample );

#endif
