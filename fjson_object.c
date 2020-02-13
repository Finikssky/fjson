#include <fjson.h>
#include <fjson_value.h>
#include <fjson_object.h>

typedef struct
{
	char *name;
	fjson_value_t *val;
} fjson_object_key_pair_t;

int compare_obj_by_name(void *a, void *b)
{
	fjson_object_key_pair_t *a_pair = (fjson_object_key_pair_t *)a;
	fjson_object_key_pair_t *b_pair = (fjson_object_key_pair_t *)b;

	return strcmp(a_pair->name, b_pair->name);
}

void free_obj_child(void *a, void *cookie)
{
	fjson_object_key_pair_t *f_pair = (fjson_object_key_pair_t *)a;

	fjson_value_free(f_pair->val);

	free(f_pair->name);
	free(f_pair);
}

fjson_object_t * fjson_object_create()
{
	fjson_object_t *new_obj = calloc(1, sizeof(fjson_object_t));
	if (!new_obj)
		return NULL;

	new_obj->header.type = JSON_OBJECT;

	return new_obj;
}

void fjson_object_free(fjson_object_t *object)
{
	if (object->children)
		avl_tree_clear_all(object->children, free_obj_child, NULL);

	free(object);
}

int fjson_object_add_value(fjson_object_t *object, char *name, fjson_value_t *value, int consume)
{
	if (!object->children)
	{
		object->children = create_avl_tree(compare_obj_by_name, NULL);
		if (!object->children)
			return -1;
	}

	fjson_object_key_pair_t *key_pair = malloc(sizeof(fjson_object_key_pair_t));
	if (!key_pair)
		return -1;

	if (consume)
		key_pair->name = name;
	else
		key_pair->name = strdup(name);

	key_pair->val = value;

	return avl_tree_insert(object->children, key_pair);
}

int fjson_object_stringize(fjson_value_t *value, char *result, int wrap, int start_indent, int indent_step)
{
	fjson_object_t *object = (fjson_object_t *)value;

	int res_cnt = 0;

	if (!result)
	{
		if (object->children && object->children->elements > 0)
		{
			avl_node_t *it;
			avl_node_t *next;
			for (it = avl_tree_first(object->children); it != NULL; it = next)
			{
				next = avl_tree_next(it);

				fjson_object_key_pair_t *it_pair = (fjson_object_key_pair_t *)it->data;

				res_cnt += strlen(it_pair->name) + 5 + start_indent + indent_step;
				res_cnt += fjson_value_stringize(it_pair->val, NULL, wrap, start_indent + indent_step, indent_step);

				if (next)
					res_cnt += 2;
				else
					res_cnt += wrap ? 1 : 0;
			}
		}

		res_cnt += 2; //{}
		res_cnt += start_indent;
		if (wrap)
			res_cnt += 1;
	}
	else
	{
		char * ptr = result;
		ptr += sprintf(ptr, "{%s", wrap ? "\n" : "");

		if (object->children)
		{
			avl_node_t *it;
			avl_node_t *next;
			for (it = avl_tree_first(object->children); it != NULL; it = next)
			{
				next = avl_tree_next(it);

				fjson_object_key_pair_t *it_pair = (fjson_object_key_pair_t *)it->data;

				ptr += sprintf(ptr, "%s\"%s\" : ", indent_str(start_indent + indent_step), it_pair->name);

				ptr += fjson_value_stringize(it_pair->val, ptr, wrap, start_indent + indent_step, indent_step);

				if (next)
					ptr += sprintf(ptr, ",%s", wrap ? "\n" : " ");
				else
					ptr += sprintf(ptr, "%s", wrap ? "\n" : "");
			}
		}

		ptr += sprintf(ptr, "%s%s", indent_str(start_indent), "}");

		res_cnt = ptr - result;
	}

	return res_cnt;
}