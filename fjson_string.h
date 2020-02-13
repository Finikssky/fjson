#ifndef __FJSON_STRING_H
#define __FJSON_STRING_H

#include <fjson_value.h>

typedef struct
{
	fjson_value_t header;
	unsigned char *data;
} fjson_string_t;

fjson_string_t * fjson_string_create(char *data, int consume);
void fjson_string_free(fjson_string_t *string);

int fjson_string_stringize(fjson_value_t *value, char *result, int wrap, int start_indent, int indent_step);

char *fjson_string_take(fjson_string_t *string);

#endif