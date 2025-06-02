#include "json.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

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

void number_42()
{
	const char8_t code_units[] = u8"42";
	struct utf8_iter source = {code_units,
				   code_units + sizeof(code_units) - 1};
	const int c = json_parse(&source);
	assert(c == -1);
}

/**
 * Returns 0 if success.
 *
 * Returns -1 if the number of bytes read differs from the file size.
 *
 * Returns errno (ALWAYS positive) upon error.
 */
int read_json(FILE *stream)
{
	if (fseek(stream, 0L, SEEK_END) == -1)
		return errno;
	const long file_size = ftell(stream);
	if (file_size == -1L)
		return errno;
	rewind(stream);
	char8_t *buf = malloc(file_size);
	if (buf == NULL)
		return errno;
	if (fread(buf, 1, file_size, stream) != file_size) {
		free(buf);
		return -1;
	}
	struct utf8_iter source = {buf, buf + file_size};
	const int c = json_parse(&source);
	assert(c == -1);
	free(buf);
	return 0;
}

int open_and_read_json(const char *restrict filename)
{
	FILE *const stream = fopen(filename, "r");
	if (stream == NULL)
		return errno;
	assert(read_json(stream) == 0);
	if (fclose(stream) == EOF)
		return errno;
	return 0;
}

int main()
{
	literal_false();
	literal_null();
	literal_true();
	string_hello_world();
	number_42();
	assert(open_and_read_json("Image.json") == 0);
	assert(open_and_read_json("San_Francisco_and_Sunnyvale.json") == 0);
}
