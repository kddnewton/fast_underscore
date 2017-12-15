#include <ruby.h>
#include <ruby/encoding.h>
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
    STATE_SINGLE_COLON
  } state;

  // The current segment of text that we're analyzing
  char *segment;

  // The resultant text from the underscore operation
  char *result;

  // The size of the result
  long size;
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

  builder->segment[0] = '\0';
  builder->result[0] = '\0';
  builder->size = 0;

  return builder;
}

void builder_push(builder_t *builder, unsigned int codepoint) {
  builder->result[builder->size++] = (char) codepoint;
}

void builder_next(builder_t *builder, unsigned int codepoint) {
  switch (builder->state) {
    case STATE_DEFAULT:
      switch (codepoint) {
        case ':':
          builder->state = STATE_SINGLE_COLON;
          return;
        case '-':
          builder_push(builder, '_');
          return;
      }
    case STATE_SINGLE_COLON:
      switch (codepoint) {
        case ':':
          builder_push(builder, '/');
          builder->state = STATE_DEFAULT;
          return;
      }
  }

  builder_push(builder, codepoint);
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

  VALUE result = rb_enc_str_new(builder->result, builder->size, encoding);
  builder_free(builder);

  return result;
}

void Init_fast_underscore(void) {
  rb_define_method(rb_cString, "underscore", rb_str_underscore, 0);
}
