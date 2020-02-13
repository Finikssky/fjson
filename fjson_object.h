#ifndef __FJSON_OBJECT_H
#define __FJSON_OBJECT_H

#include <fjson_value.h>
#include <avl_tree.h>

typedef struct
{
	fjson_value_t header;

	avl_tree_t *children;
} fjson_object_t;

fjson_object_t * fjson_object_create();
void fjson_object_free(fjson_object_t *object);

int fjson_object_add_value(fjson_object_t *object, char *name, fjson_value_t *value, int consume);

int fjson_object_stringize(fjson_value_t *value, char *result, int wrap, int start_indent, int indent_step);

#endif