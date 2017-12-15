#include "fast_underscore.h"

#define STR_ENC_GET(str) rb_enc_from_index(ENCODING_GET(str))

static VALUE rb_str_underscore(VALUE rb_string) {
  rb_encoding *encoding = STR_ENC_GET(rb_string);
  char *string = RSTRING_PTR(rb_string);
  char *end = RSTRING_END(rb_string);
  int diff;

  while (string < end) {
    unsigned int codepoint = rb_enc_codepoint_len(string, end, &diff, encoding);
    string += diff;
  }
  return rb_string;
}

void Init_fast_underscore(void) {
  rb_define_method(rb_cString, "underscore", rb_str_underscore, 0);
}
