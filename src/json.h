#pragma once
#include "unicode.h"

struct json {
	char8_t *pos;
};

enum json_marker : char8_t {
	JSON_begin_array = '[',
	JSON_begin_object = '{',
	JSON_end_array = ']',
	JSON_end_object = '}',
	JSON_name_separator = ':',
	JSON_value_separator = ',',
	JSON_false = 'f',
	JSON_null = 'n',
	JSON_true = 't',
	JSON_e = '/', // -./0123456789
	JSON_begin_string = '"',
	JSON_end_string = u8'\xC0',
};

int json_parse(struct json *json, struct utf8_iter *source);
