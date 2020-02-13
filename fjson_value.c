#include <fjson.h>
#include <fjson_value.h>
#include <fjson_object.h>
#include <fjson_array.h>
#include <fjson_string.h>
#include <fjson_number.h>
#include <fjson_literals.h>

void fjson_value_free(fjson_value_t *value)
{
	switch (value->type)
	{
		case JSON_OBJECT:
			fjson_object_free((fjson_object_t *)value);
			break;

		case JSON_STRING:
			fjson_string_free((fjson_string_t *)value);
			break;

		case JSON_NUMBER:
			fjson_number_free((fjson_number_t *)value);
			break;

		case JSON_ARRAY:
			fjson_array_free((fjson_array_t *)value);
			break;

		case JSON_LITERAL:
			fjson_literal_free((fjson_literal_t *)value);
			break;

		default:
			break;
	}
}

char *fjson_value_to_json(fjson_value_t *value, int wrap, int start_indent, int indent_step)
{
	char *result = NULL;
	fjson_stringize_fn_t stringize_function = NULL;

	switch (value->type)
	{
		case JSON_OBJECT:
			stringize_function = fjson_object_stringize;
			break;

		case JSON_STRING:
			stringize_function = fjson_string_stringize;
			break;

		case JSON_NUMBER:
			stringize_function = fjson_number_stringize;
			break;

		case JSON_ARRAY:
			stringize_function = fjson_array_stringize;
			break;

		case JSON_LITERAL:
			stringize_function = fjson_literals_stringize;
			break;

		default:
			return NULL;
	}

	int cnt_predicted = stringize_function(value, NULL, wrap, start_indent, indent_step);
	if (cnt_predicted == 0)
		return NULL;

	result = calloc(cnt_predicted + 1, sizeof(char));
	if (!result)
		return NULL;

	int cnt_writed = stringize_function(value, result, wrap, start_indent, indent_step);
	if (cnt_writed == 0 || cnt_writed != cnt_predicted)
	{
		free(result);
		return NULL;
	}

	return result;
}

int fjson_value_stringize(fjson_value_t *value, char *result, int wrap, int start_indent, int indent_step)
{
	fjson_stringize_fn_t stringize_function = NULL;

	switch (value->type)
	{
		case JSON_OBJECT:
			stringize_function = fjson_object_stringize;
			break;

		case JSON_STRING:
			stringize_function = fjson_string_stringize;
			break;

		case JSON_NUMBER:
			stringize_function = fjson_number_stringize;
			break;

		case JSON_ARRAY:
			stringize_function = fjson_array_stringize;
			break;

		case JSON_LITERAL:
			stringize_function = fjson_literals_stringize;
			break;

		default:
			return 0;
	}

	return stringize_function(value, result, wrap, start_indent, indent_step);
}