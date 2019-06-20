#include <stdint.h>

struct chunk_descr_t {
  char riff_str[4]; // "RIFF"
  uint32_t chunk_sz;
  char wave[4];
};

struct fmt_subchunk_t {
  char fmt_str[4]; // "fmt?" - last char is val 20
  uint32_t subchunk_1_sz;
  uint16_t audio_fmt; // 1 = PCM
  uint16_t n_chans;
  uint32_t sample_rate;
  uint32_t byte_rate;
  uint16_t block_align;
  uint16_t bits_per_sample; // bps?
};

// of size given by subchunk_2_sz
struct data_subchunk_t {
  char data_str[4]; // "data"
  uint32t_t subchunk_2_sz;

  // includes size of uint8_t* data;
};

