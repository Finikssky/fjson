#ifndef __FJSON_ARRAY_H
#define __FJSON_ARRAY_H

#include <fjson_value.h>
#include <avl_tree.h>

typedef struct
{
	fjson_value_t header;

	fjson_value_t **data;
	int count;
	int capacity;
} fjson_array_t;

fjson_array_t *fjson_array_create();
void fjson_array_free(fjson_array_t *array);

int fjson_array_add_value(fjson_array_t *array, fjson_value_t *value);

int fjson_array_stringize(fjson_value_t *value, char *result, int wrap, int start_indent, int indent_step);

#endif