#include "apg_wav.h"
#include <stdio.h>

int main() {
  int n_chans         = 2;
  int sample_rate     = 22050;
  int n_samples       = 7;
  int bits_per_sample = 16;
  unsigned char wav_data[7 * 2 * 16 / 8] = {
    0x00, 0x00, 0x00, 0x00, // 1st sample. 1st chan:{ byte1 byte2 } 2nd chan{ byte3 byte 4}
    0x24, 0x17, 0x1E, 0xF3,
    0x3C, 0x13, 0x3C, 0x14,
    0x16, 0xF9, 0x18, 0xF9,
    0x34, 0xE7, 0x23, 0xA6,
    0x3C, 0xF2, 0x24, 0xF2,
    0x11, 0xCE, 0x1A, 0x0D,
  };

  int result = apg_write_wav( "testout.wav", wav_data, n_chans, sample_rate, n_samples, bits_per_sample );
  return 0;
}
