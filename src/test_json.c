#include "json.h"
#include <assert.h>

void literal_false()
{
	const char8_t code_units[] = u8"false";
	struct utf8_iter source = {code_units,
				   code_units + sizeof(code_units) - 1};
	const int c = json_parse(&source);
	assert(c == -1);
}

void literal_null()
{
	const char8_t code_units[] = u8"null";
	struct utf8_iter source = {code_units,
				   code_units + sizeof(code_units) - 1};
	const int c = json_parse(&source);
	assert(c == -1);
}

void literal_true()
{
	const char8_t code_units[] = u8"true";
	struct utf8_iter source = {code_units,
				   code_units + sizeof(code_units) - 1};
	const int c = json_parse(&source);
	assert(c == -1);
}

void string_hello_world()
{
	const char8_t code_units[] = u8"\"Hello world!\"";
	struct utf8_iter source = {code_units,
				   code_units + sizeof(code_units) - 1};
	const int c = json_parse(&source);
	assert(c == -1);
}

int main()
{
	literal_false();
	literal_null();
	literal_true();
	string_hello_world();
}
