#include <fjson.h>
#include <fjson_value.h>
#include <fjson_array.h>

#define CAPACITY_STEP 10

fjson_array_t * fjson_array_create()
{
	fjson_array_t *new_array = calloc(1, sizeof(fjson_array_t));
	if (!new_array)
		return NULL;

	new_array->header.type = JSON_ARRAY;

	return new_array;
}

void fjson_array_free(fjson_array_t *array)
{
	if (array->data)
	{
		int i;
		for (i = 0; i < array->count; i++)
		{
			fjson_value_free(array->data[i]);
		}

		free(array->data);
	}

	free(array);
}

int fjson_array_add_value(fjson_array_t *array, fjson_value_t *value)
{
	if (!array->data)
	{
		array->capacity = CAPACITY_STEP;
		array->count = 0;
		array->data = calloc(array->capacity, sizeof(fjson_value_t *));
		if (!array->data)
			return -1;
	}

	if (array->count == array->capacity)
	{
		void *tmp = realloc(array->data, (array->capacity + CAPACITY_STEP) * sizeof(fjson_value_t *));
		if (!tmp)
			return -1;

		array->data = tmp;
		array->capacity += CAPACITY_STEP;
	}

	array->data[array->count++] = value;

	return 0;
}

int fjson_array_stringize(fjson_value_t *value, char *result, int wrap, int start_indent, int indent_step)
{
	fjson_array_t *array = (fjson_array_t *)value;

	int res_cnt = 0;

	if (!result)
	{
		if (array->data && array->count > 0)
		{
			int i;
			for (i = 0; i < array->count; i++)
			{
				res_cnt += start_indent + indent_step;
				res_cnt += fjson_value_stringize(array->data[i], NULL, wrap, start_indent + indent_step, indent_step);

				if (i != (array->count - 1))
					res_cnt += 2;
				else
					res_cnt += wrap ? 1 : 0;
			}
		}

		res_cnt += 2; //[]
		res_cnt += start_indent;
		if (wrap)
			res_cnt += 1;
	}
	else
	{
		char * ptr = result;
		ptr += sprintf(ptr, "[%s", wrap ? "\n" : "");

		int i;
		for (i = 0; i < array->count; i++)
		{
			ptr += sprintf(ptr, "%s", indent_str(start_indent + indent_step));

			ptr += fjson_value_stringize(array->data[i], ptr, wrap, start_indent + indent_step, indent_step);

			if (i != (array->count - 1))
				ptr += sprintf(ptr, ",%s", wrap ? "\n" : " ");
			else
				ptr += sprintf(ptr, "%s", wrap ? "\n" : "");
		}

		ptr += sprintf(ptr, "%s%s", indent_str(start_indent), "]");

		res_cnt = ptr - result;
	}

	return res_cnt;
}