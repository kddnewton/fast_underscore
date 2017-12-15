#include <ruby.h>
#include <ruby/encoding.h>
#include <stdio.h>
#include <stdlib.h>

// return camel_cased_word unless /[A-Z-]|::/.match?(camel_cased_word)
// word = camel_cased_word.to_s.gsub('::'.freeze, '/'.freeze)
// word.gsub!(/([A-Z\d]+)([A-Z][a-z])/, '\1_\2'.freeze)
// word.gsub!(/([a-z\d])([A-Z])/, '\1_\2'.freeze)
// word.tr!('-'.freeze, '_'.freeze)
// word.downcase!
// word

typedef struct builder {
  // The state of the DFA in which is the builder
  enum state {
    STATE_DEFAULT,
    STATE_SINGLE_COLON,
    STATE_PRE_CAPITAL
  } state;

  // The current segment of text that we're analyzing
  char *segment;
  long segment_size;

  // The resultant text from the underscore operation
  char *result;
  long result_size;
} builder_t;

builder_t* builder_build(long str_len) {
  builder_t *builder = (builder_t *) malloc(sizeof(builder_t));
  if (builder == NULL) {
    return NULL;
  }

  builder->state = STATE_DEFAULT;
  builder->segment = (char *) malloc(str_len * 2);

  if (builder->segment == NULL) {
    free(builder);
    return NULL;
  }

  builder->result = (char *) malloc(str_len * 2);

  if (builder->result == NULL) {
    free(builder->segment);
    free(builder);
    return NULL;
  }

  builder->segment_size = 0;
  builder->result_size = 0;

  return builder;
}

void builder_result_push(builder_t *builder, unsigned int codepoint) {
  builder->result[builder->result_size++] = (char) codepoint;
}

void builder_segment_start(builder_t *builder, unsigned int codepoint) {
  builder->segment[0] = (char) codepoint;
  builder->segment_size = 1;
}

void builder_flush(builder_t *builder) {
  switch (builder->state) {
    case STATE_DEFAULT: return;
    case STATE_SINGLE_COLON:
      builder_result_push(builder, ':');
      return;
    case STATE_PRE_CAPITAL:
      builder_result_push(builder, builder->segment[0]);
      return;
  }
}

void builder_next(builder_t *builder, unsigned int codepoint) {
  // for (int idx = 0; idx < builder->result_size; idx++) {
  //   printf("%c", builder->result[idx]);
  // }
  // printf("\n");

  switch (builder->state) {
    case STATE_DEFAULT:
      if (codepoint == ':') {
        builder->state = STATE_SINGLE_COLON;
        return;
      }
      if (codepoint == '-') {
        builder_result_push(builder, '_');
        return;
      }
      if ((codepoint >= 'a' && codepoint <= 'z') || (codepoint >= '0' && codepoint <= '9')) {
        builder_segment_start(builder, codepoint);
        builder->state = STATE_PRE_CAPITAL;
        return;
      }
    case STATE_SINGLE_COLON:
      switch (codepoint) {
        case ':':
          builder_result_push(builder, '/');
          builder->state = STATE_DEFAULT;
          return;
      }
    case STATE_PRE_CAPITAL:
      builder_result_push(builder, builder->segment[0]);

      if (codepoint >= 'A' && codepoint <= 'Z') {
        builder_result_push(builder, '_');
        builder_result_push(builder, codepoint);
        builder->state = STATE_DEFAULT;
        return;
      } else {
        builder->state = STATE_DEFAULT;
        builder_next(builder, codepoint);
        return;
      }
  }

  builder_result_push(builder, codepoint);
  builder->state = STATE_DEFAULT;
}

void builder_free(builder_t *builder) {
  free(builder->segment);
  free(builder->result);
  free(builder);
}

#define RSTRING_ENC_GET(rb_string) rb_enc_from_index(ENCODING_GET(rb_string))

static VALUE rb_str_underscore(VALUE rb_string) {
  rb_encoding *encoding = RSTRING_ENC_GET(rb_string);

  char *string = RSTRING_PTR(rb_string);
  char *end = RSTRING_END(rb_string);

  int diff;

  builder_t *builder = builder_build(RSTRING_LEN(rb_string) * 2);

  while (string < end) {
    unsigned int codepoint = rb_enc_codepoint_len(string, end, &diff, encoding);
    builder_next(builder, codepoint);
    string += diff;
  }
  builder_flush(builder);

  VALUE result = rb_enc_str_new(builder->result, builder->result_size, encoding);
  builder_free(builder);

  return result;
}

void Init_fast_underscore(void) {
  rb_define_method(rb_cString, "underscore", rb_str_underscore, 0);
}
