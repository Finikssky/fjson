#ifndef __FJSON_LITERAL_H
#define __FJSON_LITERAL_H

#include <fjson_value.h>

typedef enum
{
	JSON_LITERAL_NULL,
	JSON_LITERAL_TRUE,
	JSON_LITERAL_FALSE
} fjson_literal_type_t;

typedef struct
{
	fjson_value_t header;

	fjson_literal_type_t literal_type;
} fjson_literal_t;

fjson_literal_t *fjson_literal_create(fjson_literal_type_t literal_type);
void fjson_literal_free(fjson_literal_t *literal);

int fjson_literals_stringize(fjson_value_t *value, char *result, int wrap, int start_indent, int indent_step);

#endif