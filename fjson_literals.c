#include <fjson.h>
#include <fjson_literals.h>

fjson_literal_t * fjson_literal_create(fjson_literal_type_t literal_type)
{
	fjson_literal_t *new_literal = calloc(1, sizeof(fjson_literal_t));
	if (!new_literal)
		return NULL;

	new_literal->header.type = JSON_LITERAL;
	new_literal->literal_type = literal_type;

	return new_literal;
}

void fjson_literal_free(fjson_literal_t *literal)
{
	free(literal);
}

int fjson_literals_stringize(fjson_value_t *value, char *result, int wrap, int start_indent, int indent_step)
{
	fjson_literal_t *literal = (fjson_literal_t *)value;

	switch (literal->literal_type)
	{
		case JSON_LITERAL_NULL:
			if (!result)
				return 4;
			else
				return sprintf(result, "%s", "null");

		case JSON_LITERAL_TRUE:
			if (!result)
				return 4;
			else
				return sprintf(result, "%s", "true");

		case JSON_LITERAL_FALSE:
			if (!result)
				return 5;
			else
				return sprintf(result, "%s", "false");

		default:
			break;
	}

	return 0;
}