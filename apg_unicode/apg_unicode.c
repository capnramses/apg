/* apg_unicode
Unicode <-> UTF-8 Multibyte String Functions for C & C++
Anton Gerdelan
Version: 0.2.2
Licence: see apg_unicode.h
C99
*/

#include "apg_unicode.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// clang-format off
#define MASK_FIRST_ONE   128 // 128 or 10000000
#define MASK_FIRST_TWO   192 // 192 or 11000000
#define MASK_FIRST_THREE 224 // 224 or 11100000
#define MASK_FIRST_FOUR  240 // 240 or 11110000
#define MASK_FIRST_FIVE  248 // 248 or 11111000
// clang-format on

int apg_cp_to_utf8( uint32_t codepoint, char* mbs ) {
  assert( mbs );
  if ( !mbs ) {
    mbs[0] = (char)0xEF;
    mbs[1] = (char)0xBF;
    mbs[2] = (char)0xBD;
    mbs[3] = '\0';
    return 4;
  }

  if ( codepoint <= 0x7F ) { // ASCII
    mbs[0] = (char)codepoint;
    mbs[1] = '\0';
    return 1;
  }
  // 2 bytes: max 0b11111111111 -> 110xxxxx 10xxxxxx
  if ( codepoint <= 0x07FF ) { // (U+0080 to U+07FF - Latin, Greek, Hebrew, Arabic, ... )
    // ( shift off lower byte(s), cut off anything in front from earlier byte(s), append 110xxxxx to front )
    mbs[0] = (char)( ( ( codepoint >> 6 ) & 0x1F ) | MASK_FIRST_TWO );
    mbs[1] = (char)( ( codepoint & 0x3F ) | MASK_FIRST_ONE );
    mbs[2] = '\0';
    return 2;
  }
  // 3-bytes
  if ( codepoint <= 0xFFFF ) {
    mbs[0] = (char)( ( ( codepoint >> 12 ) & 0x0F ) | MASK_FIRST_THREE );
    mbs[1] = (char)( ( ( codepoint >> 6 ) & 0x3F ) | MASK_FIRST_ONE );
    mbs[2] = (char)( ( codepoint & 0x3F ) | MASK_FIRST_ONE );
    mbs[3] = '\0';
    return 3;
  }
  // 4-bytes
  if ( codepoint <= 0x10FFFF ) {
    mbs[0] = (char)( ( ( codepoint >> 18 ) & 0x07 ) | MASK_FIRST_FOUR );
    mbs[1] = (char)( ( ( codepoint >> 12 ) & 0x3F ) | MASK_FIRST_ONE );
    mbs[2] = (char)( ( ( codepoint >> 6 ) & 0x3F ) | MASK_FIRST_ONE );
    mbs[3] = (char)( ( codepoint & 0x3F ) | MASK_FIRST_ONE );
    mbs[4] = '\0';
    return 4;
  }
  // unknown unicode - use diamond question mark
  mbs[0] = (char)0xEF;
  mbs[1] = (char)0xBF;
  mbs[2] = (char)0xBD;
  mbs[3] = '\0';
  return 4;
}

uint32_t apg_utf8_to_cp( const char* mbs, int* sz ) {
  assert( mbs && sz );
  if ( !mbs || !sz ) { return 0; }

  *sz = 0;
  if ( '\0' == mbs[0] ) { return 0; }

  uint8_t first_byte = (uint8_t)mbs[0];

  // 1 byte: (U+0000 to U+007F - ASCII)
  // 0xxxxxxx + 7 bits for cp
  if ( first_byte < MASK_FIRST_ONE ) {
    *sz = 1;
    return (uint32_t)mbs[0];
  }

  // 2 bytes: 110xxxxx 10xxxxxx - 11 bits for cp (U+0080 to U+07FF - Latin, Greek, Hebrew, Arabic, ... )
  if ( first_byte < MASK_FIRST_THREE ) {
    uint8_t second_byte = (uint8_t)mbs[1];
    if ( second_byte < MASK_FIRST_ONE || second_byte >= MASK_FIRST_TWO ) {
      fprintf( stderr, "ERROR: invalid utf-8 byte - second byte of sequence\n" );
      return 0;
    }
    uint8_t part_a     = first_byte << 3;             // shift 110xxxxx to xxxxx000
    uint8_t part_b     = second_byte & (uint8_t)0x3F; // & with 0x3F (binary 00111111) to zero the first two bits from 10xxxxxx
    uint32_t codepoint = (uint32_t)part_a << 3;       // 00000000 00000000 00000000 xxxxx000 << 3 = 00000000 00000000 00000xxx xx000000
    codepoint |= (uint32_t)part_b;                    // 00000000 00000000 00000xxx xx000000 | pb = 00000000 00000000 00000xxx xxxxxxxx
    *sz = 2;
    return codepoint;
  }

  // 3 bytes: 1110xxxx 10xxxxxx 10xxxxxx - 16 bits for cp - U+0800  to U+FFFF (most CJK)
  if ( first_byte < MASK_FIRST_FOUR ) {
    uint8_t second_byte = (uint8_t)mbs[1];
    if ( second_byte < MASK_FIRST_ONE || second_byte >= MASK_FIRST_TWO ) {
      fprintf( stderr, "ERROR: invalid utf-8 byte - second byte of sequence\n" );
      return 0;
    }
    uint8_t third_byte = (uint8_t)mbs[2];
    if ( third_byte < MASK_FIRST_ONE || third_byte >= MASK_FIRST_TWO ) {
      fprintf( stderr, "ERROR: invalid utf-8 byte - third byte of sequence\n" );
      return 0;
    }
    // TODO(Anton) check why part_b and part_c aren't used here
    uint8_t part_a = first_byte << 4;             // shift 1110xxxx to xxxx0000
    uint8_t part_b = second_byte & (uint8_t)0x3F; // & with 0x3F (binary 00111111) to zero the first two bits from 10xxxxxx
    uint8_t part_c = third_byte & (uint8_t)0x3F;  // & with 0x3F (binary 00111111) to zero the first two bits from 10xxxxxx

		(void)part_b; // unused
		(void)part_c; // unused

    // 00000000 00000000 00000000 xxxx0000 << 2 = 00000000 00000000 000000xx xx000000
    uint32_t codepoint = (uint32_t)part_a << 2;
    // 00000000 00000000 000000xx xx000000 | pb = 00000000 00000000 000000xx xxxxxxxx, << 6 = 00000000 00000000 xxxxxxxx xx000000
    codepoint = ( codepoint | (uint32_t)second_byte ) << 6;
    // 00000000 00000000 000000xx xx000000 | pc = 00000000 00000000 xxxxxxxx xxxxxxxx
    codepoint |= (uint32_t)third_byte;
    *sz = 3;
    return codepoint;
  }

  // 4 bytes - 21 bits for cp - U+10000 to U+10FFFF (less common CJK, historical and math, emoji)
  // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  if ( first_byte < MASK_FIRST_FIVE ) {
    uint8_t second_byte = (uint8_t)mbs[1];
    if ( second_byte < MASK_FIRST_ONE || second_byte >= MASK_FIRST_TWO ) {
      fprintf( stderr, "ERROR: invalid utf-8 byte - second byte of sequence\n" );
      return 0;
    }
    uint8_t third_byte = (uint8_t)mbs[2];
    if ( third_byte < MASK_FIRST_ONE || third_byte >= MASK_FIRST_TWO ) {
      fprintf( stderr, "ERROR: invalid utf-8 byte - third byte of sequence\n" );
      return 0;
    }
    uint8_t fourth_byte = (uint8_t)mbs[3];
    if ( fourth_byte < MASK_FIRST_ONE || fourth_byte >= MASK_FIRST_TWO ) {
      fprintf( stderr, "ERROR: invalid utf-8 byte - fourth byte of sequence\n" );
      return 0;
    }
    uint8_t part_a = first_byte << 4;             // shift 1110xxxx to xxxx0000
    uint8_t part_b = second_byte & (uint8_t)0x3F; // & with 0x3F (binary 00111111) to zero the first two bits from 10xxxxxx
    uint8_t part_c = third_byte & (uint8_t)0x3F;  // & with 0x3F (binary 00111111) to zero the first two bits from 10xxxxxx
    uint8_t part_d = fourth_byte & (uint8_t)0x3F; // & with 0x3F (binary 00111111) to zero the first two bits from 10xxxxxx
    // 00000000 00000000 00000000 xxx00000 << 1 = 00000000 00000000 0000000x xx000000
    uint32_t codepoint = (uint32_t)part_a << 1;
    // 00000000 00000000 0000000x xx000000 | pb = 00000000 00000000 0000000x xxxxxxxx, << 6 = 00000000 00000000 0xxxxxxx xx000000
    codepoint = ( codepoint | (uint32_t)part_b ) << 6;
    // 00000000 00000000 0xxxxxxx xx000000 | pc = 00000000 00000000 0xxxxxxx xxxxxxxx, << 6 = 00000000 000xxxxx xxxxxxxx xx000000
    codepoint = ( codepoint | (uint32_t)part_c ) << 6;
    // 00000000 000xxxxx xxxxxxxx xx000000 | pc = 00000000 000xxxxx xxxxxxxx xxxxxxxx
    codepoint |= (uint32_t)part_d;
    *sz = 4;
    return codepoint;
  }

  fprintf( stderr, "ERROR: invalid utf-8 byte - first byte of sequence\n" );
  return 0;
}

int apg_utf8_count_cp( const char* buf ) {
  assert( buf );
  if ( !buf ) { return -1; }

  int ncode_points = 0;
  int current_byte = 0;
  int sz           = strnlen( buf, APG_UNICODE_MAX_STR );
  int nbytes       = 0;
  for ( int i = 0; i < sz; i += nbytes ) {
    uint32_t code = apg_utf8_to_cp( &buf[current_byte], &nbytes );
		(void)code; // unused
    current_byte += nbytes;
    ncode_points++;
  }
  return ncode_points;
}

void apg_utf8_reverse( char* out, const char* in ) {
  assert( out && in );
  if ( !out || !in ) { return; }

  int sz        = strnlen( in, APG_UNICODE_MAX_STR );
  char* tmp_ptr = malloc( sz + 1 );
  tmp_ptr[sz]   = 0;

  int byte_idx = 0;
  while ( byte_idx < sz ) {
    int nbytes  = 0;
    uint32_t cp = apg_utf8_to_cp( &in[byte_idx], &nbytes );
		(void)cp; // unused
    int out_idx = sz - byte_idx - nbytes;
    memcpy( &tmp_ptr[out_idx], &in[byte_idx], nbytes );
    byte_idx += nbytes;
  }
  out[0] = 0;
  strncat( out, tmp_ptr, sz );
  free( tmp_ptr );
}

// they all have general unicode, isolated form, end form. only some have middle and beginning form
// if there is a middle form then there is also a beginning form, and it's value is the middle value - 1
static uint32_t arabic_cp_end_cursive[256];    // subtract 0x0600 from codepoint to get its index into this
static uint32_t arabic_cp_middle_cursive[256]; // subtract 0x0600 from codepoint to get its index into this
uint32_t apg_cursive_arabic_cp( uint32_t cp_left, uint32_t cp, uint32_t cp_right ) {
  if ( cp < 0x0600 || cp > 0x06FF ) {
    fprintf( stderr, "  - using original cp - NOT ARABIC CURSIVE FORM CP\n" );
    return cp;
  }
  static bool already_set_up = false;

  if ( !already_set_up ) {
    {
      // NOTE(Anton) there are also ligatures eg FEFC  ARABIC LIGATURE LAM WITH ALEF FINAL FORM ( 0644 + 0627)

      arabic_cp_end_cursive[0x622 - 0x0600] = 0xFE82; // FE82  ARABIC LETTER ALEF WITH MADDA ABOVE FINAL FORM
      arabic_cp_end_cursive[0x623 - 0x0600] = 0xFE84; // FE84  ARABIC LETTER ALEF WITH HAMZA ABOVE FINAL FORM
      arabic_cp_end_cursive[0x624 - 0x0600] = 0xFE86; // FE86  ARABIC LETTER WAW WITH HAMZA ABOVE FINAL FORM
      arabic_cp_end_cursive[0x625 - 0x0600] = 0xFE88; // FE88  ARABIC LETTER ALEF WITH HAMZA BELOW FINAL FORM
      arabic_cp_end_cursive[0x626 - 0x0600] = 0xFE8A; // FE8A  ARABIC LETTER YEH WITH HAMZA ABOVE FINAL FORM
      arabic_cp_end_cursive[0x627 - 0x0600] = 0xFE8E; // FE8E  ARABIC LETTER ALEF FINAL FORM
      arabic_cp_end_cursive[0x628 - 0x0600] = 0xFE90; // FE90  ARABIC LETTER BEH FINAL FORM
      arabic_cp_end_cursive[0x629 - 0x0600] = 0xFE94; // FE94  ARABIC LETTER TEH MARBUTA FINAL FORM
      arabic_cp_end_cursive[0x62A - 0x0600] = 0xFE96; // FE96  ARABIC LETTER TEH FINAL FORM
      arabic_cp_end_cursive[0x62B - 0x0600] = 0xFE9A; // FE9A  ARABIC LETTER THEH FINAL FORM
      arabic_cp_end_cursive[0x62C - 0x0600] = 0xFE9E; // FE9E  ARABIC LETTER JEEM FINAL FORM
      arabic_cp_end_cursive[0x62D - 0x0600] = 0xFEA2; // FEA2  ARABIC LETTER HAH FINAL FORM
      arabic_cp_end_cursive[0x62E - 0x0600] = 0xFEA6; // FEA6  ARABIC LETTER KHAH FINAL FORM
      arabic_cp_end_cursive[0x62F - 0x0600] = 0xFEAA; // FEAA  ARABIC LETTER DAL FINAL FORM
      arabic_cp_end_cursive[0x630 - 0x0600] = 0xFEAC; // FEAC  ARABIC LETTER THAL FINAL FORM
      arabic_cp_end_cursive[0x631 - 0x0600] = 0xFEAE; // FEAE  ARABIC LETTER REH FINAL FORM
      arabic_cp_end_cursive[0x632 - 0x0600] = 0xFEB0; // FEB0  ARABIC LETTER ZAIN FINAL FORM
      arabic_cp_end_cursive[0x633 - 0x0600] = 0xFEB2; // FEB2  ARABIC LETTER SEEN FINAL FORM
      arabic_cp_end_cursive[0x634 - 0x0600] = 0xFEB6; // FEB6  ARABIC LETTER SHEEN FINAL FORM
      arabic_cp_end_cursive[0x635 - 0x0600] = 0xFEBA; // FEBA  ARABIC LETTER SAD FINAL FORM
      arabic_cp_end_cursive[0x636 - 0x0600] = 0xFEBE; // FEBE  ARABIC LETTER DAD FINAL FORM
      arabic_cp_end_cursive[0x637 - 0x0600] = 0xFEC2; // FEC2  ARABIC LETTER TAH FINAL FORM
      arabic_cp_end_cursive[0x638 - 0x0600] = 0xFEC6; // FEC6  ARABIC LETTER ZAH FINAL FOR
      arabic_cp_end_cursive[0x639 - 0x0600] = 0xFECA; // FECA  ARABIC LETTER AIN FINAL FORM
      arabic_cp_end_cursive[0x63A - 0x0600] = 0xFECE; // FECE  ARABIC LETTER GHAIN FINAL FORM
      arabic_cp_end_cursive[0x641 - 0x0600] = 0xFED2; // FED2  ARABIC LETTER FEH FINAL FORM
      arabic_cp_end_cursive[0x642 - 0x0600] = 0xFED6; // FED6  ARABIC LETTER QAF FINAL FORM
      arabic_cp_end_cursive[0x643 - 0x0600] = 0xFEDA; // FEDA  ARABIC LETTER KAF FINAL FORM
      arabic_cp_end_cursive[0x644 - 0x0600] = 0xFEDE; // FEDE  ARABIC LETTER LAM FINAL FORM
      arabic_cp_end_cursive[0x645 - 0x0600] = 0xFEE2; // FEE2  ARABIC LETTER MEEM FINAL FORM
      arabic_cp_end_cursive[0x646 - 0x0600] = 0xFEE6; // FEE6  ARABIC LETTER NOON FINAL FORM
      arabic_cp_end_cursive[0x647 - 0x0600] = 0xFEEA; // FEEA  ARABIC LETTER HEH FINAL FORM
      arabic_cp_end_cursive[0x648 - 0x0600] = 0xFEEE; // FEEE  ARABIC LETTER WAW FINAL FORM
      arabic_cp_end_cursive[0x649 - 0x0600] = 0xFEF0; // FEF0  ARABIC LETTER ALEF MAKSURA FINAL FORM
      arabic_cp_end_cursive[0x64A - 0x0600] = 0xFEF2; // FEF2  ARABIC LETTER YEH FINAL FORM

      // NOTE(Anton) not sure about these glyphs for spacing forms:
      // FE71  ARABIC TATWEEL WITH FATHATAN ABOVE ≈ <medial>     
      // FE77  ARABIC FATHA MEDIAL FORM ≈ <medial>     
      // FE79  ARABIC DAMMA MEDIAL FORM ≈ <medial>     
      // FE7B  ARABIC KASRA MEDIAL FORM ≈ <medial>     
      // FE7D  ARABIC SHADDA MEDIAL FORM ≈ <medial>     
      // FE7F  ARABIC SUKUN MEDIAL FORM ≈ <medial>     

      arabic_cp_middle_cursive[0x0626 - 0x0600] = 0xFE8C; // FE8C  ARABIC LETTER YEH WITH HAMZA ABOVE MEDIAL FORM ≈ <medial>  
      arabic_cp_middle_cursive[0x0628 - 0x0600] = 0xFE92; // FE92  ARABIC LETTER BEH MEDIAL FORM
      arabic_cp_middle_cursive[0x062A - 0x0600] = 0xFE98; // FE98  ARABIC LETTER TEH MEDIAL FORM
      arabic_cp_middle_cursive[0x062B - 0x0600] = 0xFE9C; // FE9C  ARABIC LETTER THEH MEDIAL FORM
      arabic_cp_middle_cursive[0x062C - 0x0600] = 0xFEA0; // FEA0  ARABIC LETTER JEEM MEDIAL FORM
      arabic_cp_middle_cursive[0x062D - 0x0600] = 0xFEA4; // FEA4  ARABIC LETTER HAH MEDIAL FORM
      arabic_cp_middle_cursive[0x062E - 0x0600] = 0xFEA8; // FEA8  ARABIC LETTER KHAH MEDIAL FORM

      arabic_cp_middle_cursive[0x0633 - 0x0600] = 0xFEB4; // FEB4  ARABIC LETTER SEEN MEDIAL FORM
      arabic_cp_middle_cursive[0x0634 - 0x0600] = 0xFEB8; // FEB8  ARABIC LETTER SHEEN MEDIAL FORM
      arabic_cp_middle_cursive[0x0635 - 0x0600] = 0xFEBC; // FEBC  ARABIC LETTER SAD MEDIAL FORM
      arabic_cp_middle_cursive[0x0636 - 0x0600] = 0xFEC0; // FEC0  ARABIC LETTER DAD MEDIAL FORM
      arabic_cp_middle_cursive[0x0637 - 0x0600] = 0xFEC4; // FEC4  ARABIC LETTER TAH MEDIAL FORM
      arabic_cp_middle_cursive[0x0638 - 0x0600] = 0xFEC8; // FEC8  ARABIC LETTER ZAH MEDIAL FORM
      arabic_cp_middle_cursive[0x0639 - 0x0600] = 0xFECC; // FECC  ARABIC LETTER AIN MEDIAL FORM
      arabic_cp_middle_cursive[0x063A - 0x0600] = 0xFED0; // FED0  ARABIC LETTER GHAIN MEDIAL FORM

      arabic_cp_middle_cursive[0x0641 - 0x0600] = 0xFED4; // FED4  ARABIC LETTER FEH MEDIAL FORM
      arabic_cp_middle_cursive[0x0642 - 0x0600] = 0xFED8; // FED8  ARABIC LETTER QAF MEDIAL FORM
      arabic_cp_middle_cursive[0x0643 - 0x0600] = 0xFEDC; // FEDC  ARABIC LETTER KAF MEDIAL FORM
      arabic_cp_middle_cursive[0x0644 - 0x0600] = 0xFEE0; // FEE0  ARABIC LETTER LAM MEDIAL FORM
      arabic_cp_middle_cursive[0x0645 - 0x0600] = 0xFEE4; // FEE4  ARABIC LETTER MEEM MEDIAL FORM
      arabic_cp_middle_cursive[0x0646 - 0x0600] = 0xFEE8; // FEE8  ARABIC LETTER NOON MEDIAL FORM
      arabic_cp_middle_cursive[0x0647 - 0x0600] = 0xFEEC; // FEEC  ARABIC LETTER HEH MEDIAL FORM

      arabic_cp_middle_cursive[0x064A - 0x0600] = 0xFEF4; // FEF4  ARABIC LETTER YEH MEDIAL FORM
    }
    already_set_up = true;
  }
  bool has_middle_and_beginning = ( arabic_cp_middle_cursive[cp - 0x0600] );
  bool left_receives            = false;
  bool right_sends              = false;
  if ( cp_left >= 0x0622 && cp_left <= 0x064A ) { // almost every Modern Standard Arabic glyph has a cursive form
    left_receives = true;
  }
  if ( cp_right >= 0x0622 && cp_right <= 0x064A && arabic_cp_middle_cursive[cp_right - 0x0600] ) { // if there is a mid+start option then yes it can connect
    right_sends = true;
  }
  if ( has_middle_and_beginning && left_receives && right_sends ) {
    uint32_t medial_cp = arabic_cp_middle_cursive[cp - 0x0600];
    return medial_cp;
  }
  if ( left_receives ) {
    uint32_t medial_cp = arabic_cp_middle_cursive[cp - 0x0600];
    if ( medial_cp ) {
      return medial_cp - 1; // beginning cursive form is medial form - 1
    }
  }
  if ( right_sends ) {
    uint32_t final_form_cp = arabic_cp_end_cursive[cp - 0x0600];
    if ( final_form_cp ) { return final_form_cp; }
    // glog( "  - FINAL FORM cp not in look-up table so defaulting to original cp %#06x\n", cp );
  }
  // glog( "  - using original cp - no appropriate cursive form\n" );
  return cp; // no cigar!
}

// TODO(Anton) if out is NULL then it should still run but just reporting out_sz required
void apg_utf8_convert_to_arabic_cursive( char* out, const char* in, int* out_sz ) {
  assert( in && out && out_sz );
  if ( !out || !in || !out_sz ) { return; }

  int in_sz        = strnlen( in, APG_UNICODE_MAX_STR );
  int in_byte_idx  = 0;
  *out_sz          = 0;
  uint32_t prev_cp = 0;
  out[0]           = '\0';
  for ( int i = 0; in_byte_idx < in_sz && i < APG_UNICODE_MAX_STR; i++ ) {
    int nbytes          = 0;
    uint32_t right_cp   = prev_cp;
    uint32_t code_point = apg_utf8_to_cp( &in[in_byte_idx], &nbytes );

    uint32_t left_cp  = 0;
    int nbytes_leftcp = 0;
    // TODO(Anton) can re-arrange to just do 1 decoding per iteration
    if ( in_byte_idx + nbytes < in_sz ) { left_cp = apg_utf8_to_cp( &in[in_byte_idx + nbytes], &nbytes_leftcp ); }

    char tmp[5];
    tmp[0] = 0;
    strncat( tmp, &in[in_byte_idx], nbytes );
    tmp[0] = 0;
    strncat( tmp, &in[in_byte_idx + nbytes], nbytes_leftcp );

    uint32_t cursive_cp = apg_cursive_arabic_cp( left_cp, code_point, right_cp );
    { // copy mbs into output string
      char mbs[5];
      int nbytes_c = apg_cp_to_utf8( cursive_cp, mbs );
      strncat( out, mbs, nbytes_c );
    }
    prev_cp = code_point;
    in_byte_idx += nbytes;
  }
  *out_sz = strnlen( out, APG_UNICODE_MAX_STR );
}

void apg_utf8_trim_end( char* buf, int n ) {
  if ( n <= 0 ) { return; }
  assert( buf );

  int ncode_points = apg_utf8_count_cp( buf );
  // trim all remaining chars
  if ( ncode_points <= n ) {
    buf[0] = '\0';
    return;
  }

  int current_byte       = 0;
  int sz                 = strnlen( buf, APG_UNICODE_MAX_STR );
  int current_code_point = 0;
  for ( int i = 0; current_byte < sz && i < APG_UNICODE_MAX_STR; i++ ) {
    int nbytes    = 0;
    uint32_t code = apg_utf8_to_cp( &buf[current_byte], &nbytes );
		(void)code; // unused
    current_byte += nbytes;
    current_code_point++;
    if ( current_code_point >= ncode_points - n ) {
      buf[current_byte] = '\0';
      return;
    }
  }
}

#ifdef APG_UNICODE_TESTS
// unit tests for input and output of utf-8 encoding
int main() {
  FILE* fp = fopen( "out.txt", "w" );
  const char* test_string =
    "Fáilte.Ő ΏΘ ЩӜ ݞ  ణ ആ ญ ᚉ ant😈n ᛋ € ✋ is Amazing ✌️ ぢぽ\n习乡乢乣乤乥书乧乨乩乪乫乬乭乮乯𠀀 ";
  char string_b[5];
  string_b[0] = (char)0xEF;
  string_b[1] = (char)0xBF;
  string_b[2] = (char)0xBD;
  string_b[3] = 0;
  fprintf( fp, "[%s]\n", string_b );
  fprintf( fp, "[%s]\n", test_string );
  char* ptr = (char*)test_string;
  int sz    = strnlen( test_string, APG_UNICODE_MAX_STR );
  while ( sz > 0 ) {
    uint32_t cp = apg_utf8_to_cp( ptr, &sz );

    char tmp[5];
    tmp[0] = 0;
    strncat( tmp, ptr, sz );

    char tmpb[5];
    int szb = apg_cp_to_utf8( cp, tmpb );
    if ( 0 != strcmp( tmp, tmpb ) ) { fprintf( fp, "ERROR [%s] vs [%s] cp = %u (U+%04X), sz = %i,%i\n", tmp, tmpb, cp, cp, sz, szb ); }

    fprintf( fp, "[%s] cp = %u (U+%04X), sz = %i\n", tmp, cp, cp, sz );
    ptr += sz;
  }
  fclose( fp );
  return 0;
}
#endif
