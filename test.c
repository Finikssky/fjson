#include <fjson.h>
#include <fjson_value.h>
#include <fjson_object.h>
#include <fjson_array.h>
#include <fjson_string.h>
#include <fjson_number.h>
#include <fjson_literals.h>
#include <fjson_parsing.h>

int main()
{/*
	fjson_object_t *root_obj = fjson_object_create();

	fjson_literal_t *lit1 = fjson_literal_create(JSON_LITERAL_TRUE);
	fjson_string_t *str1 = fjson_string_create("foobar-string", 0);
	fjson_number_t *num1 = fjson_number_create("120", 0);

	fjson_object_t *obj1 = fjson_object_create();
	fjson_array_t *arr1 = fjson_array_create();

	fjson_literal_t *lit2 = fjson_literal_create(JSON_LITERAL_FALSE);
	fjson_string_t *str2 = fjson_string_create("bro-string", 0);
	fjson_number_t *num2 = fjson_number_create("11", 0);

	fjson_object_add_value(root_obj, "lit1", (fjson_value_t *)lit1, 0);
	fjson_object_add_value(root_obj, "str1", (fjson_value_t *)str1, 0);
	fjson_object_add_value(root_obj, "num1", (fjson_value_t *)num1, 0);
	fjson_object_add_value(root_obj, "obj1", (fjson_value_t *)obj1, 0);
	fjson_object_add_value(root_obj, "arr1", (fjson_value_t *)arr1, 0);

	fjson_object_add_value(obj1, "lit2", (fjson_value_t *)lit2, 0);
	fjson_object_add_value(obj1, "str2", (fjson_value_t *)str2, 0);
	fjson_object_add_value(obj1, "num2", (fjson_value_t *)num2, 0);

	fjson_array_add_value(arr1, (fjson_value_t *)obj1);
	fjson_array_add_value(arr1, (fjson_value_t *)obj1);

	char *json_str = fjson_value_to_json((fjson_value_t *)root_obj, 1, 0, 4);
	if (!json_str)
	{
		printf("failed to print json!\n");
	}
	else
	{
		printf("Formatted json: \n%s\n", json_str);
		free(json_str);
	}

	json_str = fjson_value_to_json((fjson_value_t *)root_obj, 0, 0, 0);
	if (!json_str)
	{
		printf("failed to print json!\n");
	}
	else
	{
		printf("Unformatted json: \n%s\n", json_str);
		free(json_str);
	}
*/
	fjson_value_t *doc = fjson_parse_json_file("test_parse.json");
	if (!doc)
	{
		printf("Failed to parse json file test_parse.json\n");
	}
	else
	{
		char *json_doc_str = fjson_value_to_json((fjson_value_t *)doc, 1, 0, 4);
		if (!json_doc_str)
		{
			printf("failed to print parsed json!\n");
		}
		else
		{
			printf("Parsed json: \n%s\n", json_doc_str);
			free(json_doc_str);
		}
	}

	return 0;
}