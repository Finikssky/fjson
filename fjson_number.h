#ifndef __FJSON_NUMBER_H
#define __FJSON_NUMBER_H

#include <fjson_value.h>

typedef struct
{
	fjson_value_t header;
	unsigned char *data;
} fjson_number_t;

fjson_number_t * fjson_number_create(char *data, int consume_data);
void fjson_number_free(fjson_number_t *number);

int fjson_number_stringize(fjson_value_t *value, char *result, int wrap, int start_indent, int indent_step);

#endif