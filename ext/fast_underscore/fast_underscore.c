#include <ruby.h>
#include <ruby/encoding.h>

// true if the given codepoint is a lowercase ascii character.
static int character_is_lower(unsigned int character) {
  return character >= 'a' && character <= 'z';
}

// true if the given codepoint is a uppercase ascii character.
static int character_is_upper(unsigned int character) {
  return character >= 'A' && character <= 'Z';
}

// true if the given codepoint is an ascii digit.
static int character_is_digit(unsigned int character) {
  return character >= '0' && character <= '9';
}

// Macros for extracting the character out of the `codepoint_t` struct.
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
 *
 * The internal DFA looks like:
 *
 *      ┌ - ┐ ┌ * ┐                            
 *      │   v │   v                            
 *    ┌─────────────┐           ┌─────────────┐
 *    │             │──── : ───>│             │
 * ──>│   DEFAULT   │<─── : ────│    COLON    │
 *    │             │<─── * ────│             │
 *    └─────────────┘           └─────────────┘
 *      │   ^  ^  ^                            
 *      │   │  │  └───── a-z ─────────────┐    
 *   0-9A-Z *  │                          │    
 *      │   │  └───────── * ────────┐     │    
 *      v   │                       │     │    
 *    ┌─────────────┐           ┌─────────────┐
 *    │             │─── A-Z ──>│             │
 *    │ UPPER_START │           │  UPPER_END  │
 *    │             │<── 0-9 ───│             │
 *    └─────────────┘           └─────────────┘
 *       │     ^                    ^     │    
 *       └ 0-9 ┘                    └ A-Z ┘    
 *
 * Transitions from DEFAULT:
 * - On "-", push an "_" and stay on DEFAULT
 * - On ":", go to COLON
 * - On a digit or upper, start a buffer with the char and go to UPPER_START
 * - On anything else, push the char and stay on DEFAULT
 *
 * Transitions from COLON:
 * - On ":", push a "/" and go to DEFAULT
 * - On anything else, push a ":" and the char and go to DEFAULT
 *
 * Transitions from UPPER_START:
 * - On a digit, push the digit and stay on UPPER_START
 * - On an upper, push the upper and go to UPPER_END
 * - On anything else, push the buffer, go to DEFAULT, then handle the char
 *
 * Transitions from UPPER_END:
 * - On a digit, push the digit onto the buffer and go to UPPER_START
 * - On an upper, push the upper onto the buffer and stay on UPPER_END
 * - On a lower, push the buffer up to the last char, push an "_", then push
 *   the last char of the buffer, go to DEFAULT, then handle the char
 * - On anything else, push the buffer, go to DEFAULT, then handle the char
 *
 * These transitions allow us to accomplish the equivalent of the following code
 * with one pass through the string:
 *
 * def underscore(word)
 *   word.gsub!('::', '/')
 *   word.gsub!(/([A-Z\d]+)([A-Z][a-z])/, '\1_\2')
 *   word.gsub!(/([a-z\d])([A-Z])/, '\1_\2')
 *   word.tr!('-', '_')
 *   word.downcase!
 * end
 */
typedef struct builder {
  // The state of the DFA that the builder is in
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
  int push_next;
} builder_t;

// Push a character onto the resultant string using the given codepoint and
// encoding.
static void builder_result_push_char(builder_t *builder, unsigned int character, int size, rb_encoding *encoding) {
  if (character_is_upper(character)) {
    if (builder->push_next == 1) {
      builder->push_next = 0;
      builder_result_push_literal(builder, '_');
    }

    builder->result[builder->result_size++] = (char) character - 'A' + 'a';
    return;
  }

  builder->push_next = (character_is_lower(character) || character_is_digit(character));

  if (encoding == NULL) {
    builder->result[builder->result_size++] = (char) character;
  } else {
    rb_enc_mbcput(character, &builder->result[builder->result_size], encoding);
    builder->result_size += size;
  }
}

// Push the given codepoint onto the builder.
static void builder_segment_push(builder_t *builder, codepoint_t *codepoint) {
  builder->segment[builder->segment_size++] = (char) codepoint->character;
}

// Copy the given number of characters out of the segment cache onto the result
// string.
static void builder_segment_copy(builder_t *builder, long size) {
  long idx;

  for (idx = 0; idx < size; idx++) {
    builder_result_push_literal(builder, builder->segment[idx]);
  }
}

// Restart the `builder_t` back at the default state (because we've hit a
// character for which we have no allowed transitions).
static void builder_restart(builder_t *builder) {
  builder->state = STATE_DEFAULT;
  builder->segment_size = 0;
}

static void builder_next(builder_t *builder, codepoint_t *codepoint);

// Pull the remaining content out of the cached segment in case we don't end
// parsing while not in the default state.
static void builder_flush(builder_t *builder) {
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

// Perform transitions from the STATE_DEFAULT state.
static inline void builder_default_transition(builder_t *builder, codepoint_t *codepoint) {
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

// Perform transitions from the STATE_COLON state.
static inline void builder_colon_transition(builder_t *builder, codepoint_t *codepoint) {
  if (codepoint->character == ':') {
    builder_result_push_literal(builder, '/');
    builder_restart(builder);
    return;
  }

  builder_result_push_literal(builder, ':');
  builder_restart(builder);
  builder_next(builder, codepoint);
}

// Perform transitions from the STATE_UPPER_START state.
static inline void builder_upper_start_transition(builder_t *builder, codepoint_t *codepoint) {
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

// Perform transitions from the STATE_UPPER_END state.
static inline void builder_upper_end_transition(builder_t *builder, codepoint_t *codepoint) {
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

// Accept the next codepoint, which will move the `builder_t` struct into the next state.
static void builder_next(builder_t *builder, codepoint_t *codepoint) {
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
static VALUE underscore(VALUE string) {
  char segment[RSTRING_LEN(string) * 2 * sizeof(unsigned int) * 2];
  char result[RSTRING_LEN(string) * 2 * sizeof(unsigned int) * 2];

  builder_t builder = {
    .state = STATE_DEFAULT,
    .segment = segment,
    .result = result,
    .segment_size = 0,
    .result_size = 0,
    .push_next = 0
  };

  codepoint_t codepoint = {
    .encoding = rb_enc_from_index(ENCODING_GET(string)),
    .character = 0,
    .size = 0
  };

  char *pointer = RSTRING_PTR(string);
  char *end = RSTRING_END(string);

  while (pointer < end) {
    codepoint.character = rb_enc_codepoint_len(pointer, end, &codepoint.size, codepoint.encoding);
    builder_next(&builder, &codepoint);
    pointer += codepoint.size;
  }

  builder_flush(&builder);
  return rb_enc_str_new(builder.result, builder.result_size, codepoint.encoding);
}

// FastUnderscore::underscore
static VALUE fast_underscore(VALUE self, VALUE string) {
  return underscore(string);
}

// Hook into Ruby and define FastUnderscore::underscore and String#underscore
void Init_fast_underscore(void) {
  VALUE rb_cFastUnderscore = rb_define_module("FastUnderscore");
  rb_define_singleton_method(rb_cFastUnderscore, "underscore", fast_underscore, 1);
  rb_define_method(rb_cString, "underscore", underscore, 0);
}
