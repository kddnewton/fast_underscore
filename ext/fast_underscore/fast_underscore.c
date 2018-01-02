#include <ruby.h>
#include <ruby/encoding.h>
#include <stdlib.h>

/**
 * true if the given codepoint is a lowercase ascii character.
 */
static int
character_is_lower(unsigned int character) {
  return character >= 'a' && character <= 'z';
}

/**
 * true if the given codepoint is a uppercase ascii character.
 */
static int
character_is_upper(unsigned int character) {
  return character >= 'A' && character <= 'Z';
}

/**
 * true if the given codepoint is an ascii digit.
 */
static int
character_is_digit(unsigned int character) {
  return character >= '0' && character <= '9';
}

/**
 * Macros for extracting the character out of the `codepoint_t` struct.
 */
#define codepoint_is_lower(codepoint) character_is_lower(codepoint->character)
#define codepoint_is_upper(codepoint) character_is_upper(codepoint->character)
#define codepoint_is_digit(codepoint) character_is_digit(codepoint->character)

/**
 * Macros for shortcuts to call into the `builder_result_push_char` function.
 */
#define builder_result_push(builder, codepoint) \
  builder_result_push_char(builder, codepoint->character, codepoint->size, \
                           codepoint->encoding)
#define builder_result_push_literal(builder, character) \
  builder_result_push_char(builder, character, 1, NULL)

/**
 * A struct for keeping track of a codepoint from the original string. Maintains
 * the original encoding, the actual character codepoint, and the size of the
 * codepoint.
 */
typedef struct codepoint {
  // The encoding of the character
  rb_encoding *encoding;

  // Representation of a character, regardless of encoding
  unsigned int character;

  // The size that this character actually represents
  int size;
} codepoint_t;

/**
 * A struct for tracking the built string as it gets converted. Maintains an
 * internal DFA for transitioning through various inputs to match certain
 * patterns that need to be separated with underscores.
 */
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

/**
 * Allocate and initialize a `codepoint_t` struct.
 */
static codepoint_t*
codepoint_build(rb_encoding *encoding) {
  codepoint_t *codepoint;

  codepoint = (codepoint_t *) malloc(sizeof(codepoint_t));
  if (codepoint == NULL) {
    return NULL;
  }

  codepoint->encoding = encoding;
  return codepoint;
}

/**
 * Free a previously allocated `codepoint_t` struct.
 */
static void
codepoint_free(codepoint_t *codepoint) {
  free(codepoint);
}

/**
 * Allocate and initialize a `builder_t` struct.
 */
static builder_t*
builder_build(long str_len) {
  builder_t *builder;

  builder = (builder_t *) malloc(sizeof(builder_t));
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

/**
 * Push a character onto the resultant string using the given codepoint and
 * encoding.
 */
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

/**
 * Push the given codepoint onto the builder.
 */
static void
builder_segment_push(builder_t *builder, codepoint_t *codepoint) {
  builder->segment[builder->segment_size++] = (char) codepoint->character;
}

/**
 * Copy the given number of characters out of the segment cache onto the result
 * string.
 */
static void
builder_segment_copy(builder_t *builder, long size) {
  long idx;

  for (idx = 0; idx < size; idx++) {
    builder_result_push_literal(builder, builder->segment[idx]);
  }
}

/**
 * Restart the `builder_t` back at the default state (because we've hit a
 * character for which we have no allowed transitions).
 */
static void
builder_restart(builder_t *builder) {
  builder->state = STATE_DEFAULT;
  builder->segment_size = 0;
}

static void builder_next(builder_t *builder, codepoint_t *codepoint);

/**
 * Pull the remaining content out of the cached segment in case we don't end
 * parsing while not in the default state.
 */
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

/**
 * Perform transitions from the STATE_DEFAULT state.
 */
static inline void
builder_default_transition(builder_t *builder, codepoint_t *codepoint) {
  if (codepoint->character == '-') {
    builder_result_push_literal(builder, '_');
    return;
  }
  if (codepoint->character == ':') {
    builder->state = STATE_COLON;
    return;
  }
  if (codepoint_is_digit(codepoint) || codepoint_is_upper(codepoint)) {
    builder->segment[0] = (char) codepoint->character;
    builder->segment_size = 1;
    builder->state = STATE_UPPER_START;
    return;
  }
  builder_result_push(builder, codepoint);
}

/**
 * Perform transitions from the STATE_COLON state.
 */
static inline void
builder_colon_transition(builder_t *builder, codepoint_t *codepoint) {
  if (codepoint->character == ':') {
    builder_result_push_literal(builder, '/');
    builder_restart(builder);
    return;
  }

  builder_result_push_literal(builder, ':');
  builder_restart(builder);
  builder_next(builder, codepoint);
}

/**
 * Perform transitions from the STATE_UPPER_START state.
 */
static inline void
builder_upper_start_transition(builder_t *builder, codepoint_t *codepoint) {
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
}

/**
 * Perform transitions from the STATE_UPPER_END state.
 */
static inline void
builder_upper_end_transition(builder_t *builder, codepoint_t *codepoint) {
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
}

/**
 * Accept the next codepoint, which will move the `builder_t` struct into the
 * next state.
 */
static void
builder_next(builder_t *builder, codepoint_t *codepoint) {
  switch (builder->state) {
    case STATE_DEFAULT:
      return builder_default_transition(builder, codepoint);
    case STATE_COLON:
      return builder_colon_transition(builder, codepoint);
    case STATE_UPPER_START:
      return builder_upper_start_transition(builder, codepoint);
    case STATE_UPPER_END:
      return builder_upper_end_transition(builder, codepoint);
  }
}

/**
 * Frees a previously allocated `builder_t` struct.
 */
static void
builder_free(builder_t *builder) {
  free(builder->segment);
  free(builder->result);
  free(builder);
}

/**
 * Makes an underscored, lowercase form from the expression in the string.
 *
 * Changes '::' to '/' to convert namespaces to paths.
 *
 *     underscore('ActiveModel')         # => "active_model"
 *     underscore('ActiveModel::Errors') # => "active_model/errors"
 *
 * As a rule of thumb you can think of +underscore+ as the inverse of
 * #camelize, though there are cases where that does not hold:
 *
 *     camelize(underscore('SSLError'))  # => "SslError"
 */
static VALUE
str_underscore(VALUE rb_string) {
  VALUE resultant;
  rb_encoding *encoding;

  char *string;
  char *end;

  builder_t *builder;
  codepoint_t *codepoint;

  encoding = rb_enc_from_index(ENCODING_GET(rb_string));
  string = RSTRING_PTR(rb_string);
  end = RSTRING_END(rb_string);

  builder = builder_build(RSTRING_LEN(rb_string) * 2);
  codepoint = codepoint_build(encoding);

  while (string < end) {
    codepoint->character = rb_enc_codepoint_len(string, end, &codepoint->size, encoding);
    builder_next(builder, codepoint);
    string += codepoint->size;
  }
  builder_flush(builder);

  resultant = rb_enc_str_new(builder->result, builder->result_size, encoding);
  builder_free(builder);
  codepoint_free(codepoint);

  return resultant;
}

/**
 * A singleton method calls with a string that delegates to `str_underscore` to
 * form an underscored, lowercase form from the expression in the string.
 */
static VALUE
rb_str_underscore(VALUE self, VALUE rb_string) {
  return str_underscore(rb_string);
}

/**
 * Hook into Ruby and define the `FastUnderscore::underscore`.
 */
void
Init_fast_underscore(void) {
  VALUE rb_cFastUnderscore = rb_define_module("FastUnderscore");
  rb_define_singleton_method(rb_cFastUnderscore, "underscore", rb_str_underscore, 1);
  rb_define_method(rb_cString, "underscore", str_underscore, 0);
}
