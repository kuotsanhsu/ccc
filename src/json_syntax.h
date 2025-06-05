#pragma once
#include <stddef.h>

typedef unsigned char char8_t;
struct codepoint_stream;
int get_codepoint(struct codepoint_stream *);
struct codepoint_stream *utf8_stream(const char8_t *, size_t);

enum json_error {
	JSON_ERR_lex_value = -10,
	JSON_ERR_lex_literal = -11,
	JSON_ERR_lex_object_member = -12,
	JSON_ERR_lex_object_name_separator = -13,
	JSON_ERR_lex_end_object_or_value_separator = -14,
	JSON_ERR_lex_end_array_or_value_separator = -15,
	JSON_ERR_lex_xdigit = -16,
	JSON_ERR_lex_digit = -17,
	JSON_ERR_lex_number = -18,
	JSON_ERR_lex_escape = -19,
	JSON_ERR_lex_string = -20,
};

int lex_json_text(struct codepoint_stream *source);
