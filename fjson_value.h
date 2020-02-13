#ifndef __FJSON_VALUE_H
#define __FJSON_VALUE_H

typedef enum
{
	JSON_OBJECT,
	JSON_ARRAY,
	JSON_STRING,
	JSON_NUMBER,
	JSON_LITERAL
} fjson_value_type_t;

typedef struct
{
	fjson_value_type_t type;
} fjson_value_t;

typedef int (*fjson_stringize_fn_t)(fjson_value_t *value, char *buffer, int wrap, int start_indent, int indent_step);

void fjson_value_free(fjson_value_t *value);

char *fjson_value_to_json(fjson_value_t *value, int wrap,  int start_indent, int indent_step);

int fjson_value_stringize(fjson_value_t *value, char *result, int wrap, int start_indent, int indent_step);

#endif