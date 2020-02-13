#include <fjson.h>
#include <fjson_value.h>
#include <fjson_string.h>

fjson_string_t * fjson_string_create(char *data, int consume)
{
	fjson_string_t *new_string = calloc(1, sizeof(fjson_string_t));
	if (!new_string)
		return NULL;

	new_string->header.type = JSON_STRING;
	if (consume)
		new_string->data = data;
	else
		new_string->data = strdup(data);

	return new_string;
}

void fjson_string_free(fjson_string_t *string)
{
	if (string->data)
		free(string->data);

	free(string);
}

char *fjson_string_take(fjson_string_t *string)
{
	char *result = string->data;

	string->data = NULL;

	return result;
}

int fjson_string_stringize(fjson_value_t *value, char *result, int wrap, int start_indent, int indent_step)
{
	fjson_string_t *string = (fjson_string_t *)value;

	if (!string->data)
		return 0;

	if (!result)
		return strlen(string->data) + 2;
	else
		return sprintf(result, "\"%s\"", string->data);
}