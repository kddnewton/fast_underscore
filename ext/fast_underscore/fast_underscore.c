#include <ruby.h>
#include <ruby/encoding.h>
#include <stdlib.h>

static int
character_is_lower(unsigned int character) {
  return character >= 'a' && character <= 'z';
}

static int
character_is_upper(unsigned int character) {
  return character >= 'A' && character <= 'Z';
}

static int
character_is_digit(unsigned int character) {
  return character >= '0' && character <= '9';
}

#define codepoint_is_lower(codepoint) character_is_lower(codepoint->character)
#define codepoint_is_upper(codepoint) character_is_upper(codepoint->character)
#define codepoint_is_digit(codepoint) character_is_digit(codepoint->character)

#define builder_result_push(builder, codepoint) \
  builder_result_push_char(builder, codepoint->character, codepoint->size, \
                           codepoint->encoding)
#define builder_result_push_literal(builder, character) \
  builder_result_push_char(builder, character, 1, NULL)

typedef struct codepoint {
  // The encoding of the character
  rb_encoding *encoding;

  // Representation of a character, regardless of encoding
  unsigned int character;

  // The size that this character actually represents
  int size;
} codepoint_t;

typedef struct builder {
  // The state of the DFA in which is the builder
  enum state {
    STATE_DEFAULT,
    STATE_COLON,
    STATE_UPPER_END,
    STATE_UPPER_START
  } state;

  // The current segment of text that we're analyzing
  char *segment;
  long segment_size;

  // The resultant text from the underscore operation
  char *result;
  long result_size;

  // Whether or not the last pushed result character should cause the following
  // one to be spaced by an underscore
  int pushNext;
} builder_t;

static codepoint_t*
codepoint_build(rb_encoding *encoding) {
  codepoint_t *codepoint = (codepoint_t *) malloc(sizeof(codepoint_t));
  if (codepoint == NULL) {
    return NULL;
  }

  codepoint->encoding = encoding;
  return codepoint;
}

static void
codepoint_free(codepoint_t *codepoint) {
  free(codepoint);
}

static builder_t*
builder_build(long str_len) {
  builder_t *builder = (builder_t *) malloc(sizeof(builder_t));
  if (builder == NULL) {
    return NULL;
  }

  builder->state = STATE_DEFAULT;
  builder->segment = (char *) malloc(str_len * sizeof(unsigned int) * 2);

  if (builder->segment == NULL) {
    free(builder);
    return NULL;
  }

  builder->result = (char *) malloc(str_len * sizeof(unsigned int) * 2);

  if (builder->result == NULL) {
    free(builder->segment);
    free(builder);
    return NULL;
  }

  builder->segment_size = 0;
  builder->result_size = 0;
  builder->pushNext = 0;

  return builder;
}

static void
builder_result_push_char(builder_t *builder, unsigned int character, int size,
                         rb_encoding *encoding) {
  if (character_is_upper(character)) {
    if (builder->pushNext == 1) {
      builder->pushNext = 0;
      builder_result_push_literal(builder, '_');
    }

    builder->result[builder->result_size++] = (char) character - 'A' + 'a';
    return;
  }

  builder->pushNext = (character_is_lower(character) || character_is_digit(character));

  if (encoding == NULL) {
    builder->result[builder->result_size++] = (char) character;
  } else {
    rb_enc_mbcput(character, &builder->result[builder->result_size], encoding);
    builder->result_size += size;
  }
}

static void
builder_segment_start(builder_t *builder, codepoint_t *codepoint) {
  builder->segment[0] = (char) codepoint->character;
  builder->segment_size = 1;
}

static void
builder_segment_push(builder_t *builder, codepoint_t *codepoint) {
  builder->segment[builder->segment_size++] = (char) codepoint->character;
}

static void
builder_segment_copy(builder_t *builder, long size) {
  for (long idx = 0; idx < size; idx++) {
    builder_result_push_literal(builder, builder->segment[idx]);
  }
}

static void
builder_restart(builder_t *builder) {
  builder->state = STATE_DEFAULT;
  builder->segment_size = 0;
}

static void
builder_flush(builder_t *builder) {
  switch (builder->state) {
    case STATE_DEFAULT: return;
    case STATE_COLON:
      builder_result_push_literal(builder, ':');
      return;
    case STATE_UPPER_END:
    case STATE_UPPER_START:
      builder_segment_copy(builder, builder->segment_size);
      return;
  }
}

static void
builder_next(builder_t *builder, codepoint_t *codepoint) {
  switch (builder->state) {
    case STATE_DEFAULT:
      if (codepoint->character == '-') {
        builder_result_push_literal(builder, '_');
        return;
      }
      if (codepoint->character == ':') {
        builder->state = STATE_COLON;
        return;
      }
      if (codepoint_is_digit(codepoint) || codepoint_is_upper(codepoint)) {
        builder_segment_start(builder, codepoint);
        builder->state = STATE_UPPER_START;
        return;
      }
      builder_result_push(builder, codepoint);
      return;
    case STATE_COLON:
      if (codepoint->character == ':') {
        builder_result_push_literal(builder, '/');
        builder_restart(builder);
        return;
      }

      builder_result_push_literal(builder, ':');
      builder_restart(builder);
      builder_next(builder, codepoint);
      return;
    case STATE_UPPER_START:
      if (codepoint_is_digit(codepoint)) {
        builder_segment_push(builder, codepoint);
        return;
      }
      if (codepoint_is_upper(codepoint)) {
        builder_segment_push(builder, codepoint);
        builder->state = STATE_UPPER_END;
        return;
      }

      builder_segment_copy(builder, builder->segment_size);
      builder_restart(builder);
      builder_next(builder, codepoint);
      return;
    case STATE_UPPER_END:
      if (codepoint_is_digit(codepoint)) {
        builder_segment_push(builder, codepoint);
        builder->state = STATE_UPPER_START;
        return;
      }
      if (codepoint_is_upper(codepoint)) {
        builder_segment_push(builder, codepoint);
        return;
      }
      if (codepoint_is_lower(codepoint)) {
        builder_segment_copy(builder, builder->segment_size - 1);
        builder_result_push_literal(builder, '_');
        builder_result_push_literal(builder, builder->segment[builder->segment_size - 1]);
        builder_restart(builder);
        builder_next(builder, codepoint);
        return;
      }

      builder_segment_copy(builder, builder->segment_size);
      builder_restart(builder);
      builder_next(builder, codepoint);
      return;
  }
}

static void
builder_free(builder_t *builder) {
  free(builder->segment);
  free(builder->result);
  free(builder);
}

static VALUE
rb_str_underscore(VALUE rb_string) {
  rb_encoding *encoding = rb_enc_from_index(ENCODING_GET(rb_string));

  char *string = RSTRING_PTR(rb_string);
  char *end = RSTRING_END(rb_string);

  builder_t *builder = builder_build(RSTRING_LEN(rb_string) * 2);
  codepoint_t *codepoint = codepoint_build(encoding);

  while (string < end) {
    codepoint->character = rb_enc_codepoint_len(string, end, &codepoint->size, encoding);
    builder_next(builder, codepoint);
    string += codepoint->size;
  }
  builder_flush(builder);

  VALUE resultant = rb_enc_str_new(builder->result, builder->result_size, encoding);
  builder_free(builder);

  return resultant;
}

void Init_fast_underscore(void) {
  rb_define_method(rb_cString, "underscore", rb_str_underscore, 0);
}
