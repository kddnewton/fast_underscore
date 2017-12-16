#include <ruby.h>
#include <ruby/encoding.h>
#include <stdio.h>
#include <stdlib.h>

int codepoint_is_lower_alpha(unsigned int codepoint) {
  return codepoint >= 'a' && codepoint <= 'z';
}

int codepoint_is_upper_alpha(unsigned int codepoint) {
  return codepoint >= 'A' && codepoint <= 'Z';
}

int codepoint_is_digit(unsigned int codepoint) {
  return codepoint >= '0' && codepoint <= '9';
}

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
  if (codepoint_is_upper_alpha(codepoint)) {
    builder->result[builder->result_size++] = (char) codepoint - 'A' + 'a';
    return;
  }
  builder->result[builder->result_size++] = (char) codepoint;
}

void builder_segment_start(builder_t *builder, unsigned int codepoint) {
  builder->segment[0] = (char) codepoint;
  builder->segment_size = 1;
}

void builder_segment_push(builder_t *builder, unsigned int codepoint) {
  builder->segment[builder->segment_size++] = (char) codepoint;
}

void builder_segment_copy(builder_t *builder, long size) {
  for (long idx = 0; idx < size; idx++) {
    builder_result_push(builder, builder->segment[idx]);
  }
}

void builder_restart(builder_t *builder) {
  builder->state = STATE_DEFAULT;
  builder->segment_size = 0;
}

void builder_flush(builder_t *builder) {
  switch (builder->state) {
    case STATE_DEFAULT: return;
    case STATE_COLON:
      builder_result_push(builder, ':');
      return;
    case STATE_UPPER_END:
    case STATE_UPPER_START:
      builder_segment_copy(builder, builder->segment_size);
      return;
  }
}

void builder_next(builder_t *builder, unsigned int codepoint) {
  // printf("READING = %c\n", codepoint);
  //
  // printf("STATE = ");
  // switch (builder->state) {
  //   case STATE_DEFAULT:     printf("STATE_DEFAULT"); break;
  //   case STATE_COLON:       printf("STATE_COLON"); break;
  //   case STATE_UPPER_START: printf("STATE_UPPER_START"); break;
  //   case STATE_UPPER_END:   printf("STATE_UPPER_END"); break;
  //   case STATE_DIGIT_START: printf("STATE_DIGIT_START"); break;
  //   case STATE_DIGIT_END:   printf("STATE_DIGIT_END"); break;
  //   case STATE_LOWER:       printf("STATE_LOWER"); break;
  // }
  // printf("\n");
  //
  // printf("SEGMENT = ");
  // for (long idx = 0; idx < builder->segment_size; idx++) {
  //   printf("%c", builder->segment[idx]);
  // }
  // printf("\n");
  //
  // printf("RESULTS = ");
  // for (long idx = 0; idx < builder->result_size; idx++) {
  //   printf("%c", builder->result[idx]);
  // }
  // printf("\n\n");

  switch (builder->state) {
    case STATE_DEFAULT:
      if (codepoint == '-') {
        builder_result_push(builder, '_');
        return;
      }
      if (codepoint == ':') {
        builder->state = STATE_COLON;
        return;
      }
      if (codepoint_is_digit(codepoint) || codepoint_is_upper_alpha(codepoint)) {
        builder_segment_start(builder, codepoint);
        builder->state = STATE_UPPER_START;
        return;
      }
      builder_result_push(builder, codepoint);
      return;
    case STATE_COLON:
      if (codepoint == ':') {
        builder_result_push(builder, '/');
        builder_restart(builder);
        return;
      }

      builder_result_push(builder, ':');
      builder_restart(builder);
      builder_next(builder, codepoint);
      return;
    case STATE_UPPER_START:
      if (codepoint_is_digit(codepoint)) {
        builder_segment_push(builder, codepoint);
        return;
      }
      if (codepoint_is_upper_alpha(codepoint)) {
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
      if (codepoint_is_upper_alpha(codepoint)) {
        builder_segment_push(builder, codepoint);
        return;
      }
      if (codepoint_is_lower_alpha(codepoint)) {
        builder_segment_copy(builder, builder->segment_size - 1);
        builder_result_push(builder, '_');
        builder_result_push(builder, builder->segment[builder->segment_size - 1]);
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

  char result[builder->result_size];
  for (long idx = 0; idx < builder->result_size; idx++) {
    result[idx] = (char) builder->result[idx];
  }

  VALUE resultant = rb_enc_str_new(result, builder->result_size, encoding);
  builder_free(builder);

  return resultant;
}

void Init_fast_underscore(void) {
  rb_define_method(rb_cString, "underscore", rb_str_underscore, 0);
}
