#include <fjson.h>
#include <fjson_number.h>

fjson_number_t * fjson_number_create(char *data, int consume_data)
{
	fjson_number_t *new_number = calloc(1, sizeof(fjson_number_t));
	if (!new_number)
		return NULL;

	new_number->header.type = JSON_NUMBER;

	if (consume_data)
		new_number->data = data;
	else
		new_number->data = strdup(data);

	return new_number;
}

void fjson_number_free(fjson_number_t *number)
{
	if (number->data)
		free(number->data);

	free(number);
}

int fjson_number_stringize(fjson_value_t *value, char *result, int wrap, int start_indent, int indent_step)
{
	fjson_number_t *number = (fjson_number_t *)value;

	if (!number->data)
		return 0;

	if (!result)
		return strlen(number->data);
	else
		return sprintf(result, "%s", number->data);
}