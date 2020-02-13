#include <fjson.h>
#include <fjson_parsing.h>
#include <fjson_value.h>
#include <fjson_object.h>
#include <fjson_array.h>
#include <fjson_string.h>
#include <fjson_number.h>
#include <fjson_literals.h>

typedef struct
{
	char *filename;
	FILE *fd_source;

	char *str_source;

	int line;
	int column;
	int symbol;

	char current_sym;
	int skip_next; //get current instead of next
} fjson_parsing_context_t;

typedef enum
{
	LIT_PARSE_TRUE,
	LIT_PARSE_FALSE,
	LIT_PARSE_NULL
} fjson_literal_fsm_state;

typedef enum
{
	NUM_PARSE_START,
	NUM_PARSE_DIGIT_FIRST,
	NUM_PARSE_DIGIT,
	NUM_PARSE_FRAC,
	NUM_PARSE_FRAC_DIGIT,
	NUM_PARSE_EXP,
	NUM_PARSE_EXP_DIGIT
} fjson_number_fsm_state;

typedef enum
{
	ARR_PARSE_STARTED,
	ARR_PARSE_NEXT_VALUE_OR_END,
	ARR_PARSE_NEXT_VALUE,
	ARR_PARSE_DIV_OR_END
} fjson_array_fsm_state;

typedef enum
{
	OBJ_PARSE_STARTED,
	OBJ_PARSE_KEY_NAME_OR_END,
	OBJ_PARSE_KEY_NAME,
	OBJ_PARSE_KEY_DIV,
	OBJ_PARSE_KEY_VAL,
	OBJ_PARSE_VALUE_DIV_OR_END
} fjson_object_fsm_state;

typedef enum
{
	DOC_STARTED,
	DOC_PARSE_FAILED,
	DOC_COMPLETED
} fjson_doc_fsm_state;


fjson_object_t *fjson_parse_object(fjson_parsing_context_t *context);
fjson_array_t *fjson_parse_array(fjson_parsing_context_t *context);
fjson_number_t *fjson_parse_number(fjson_parsing_context_t *context);
fjson_string_t *fjson_parse_string(fjson_parsing_context_t *context);
fjson_literal_t *fjson_parse_literal(fjson_parsing_context_t *context);

char fjson_context_get_next_sim(fjson_parsing_context_t *context)
{
	if (context->skip_next > 0)
	{
		context->skip_next--;
		return context->current_sym;
	}

	context->current_sym = '\0';

	if (context->fd_source && !feof(context->fd_source))
	{
		context->current_sym = fgetc(context->fd_source);
		context->symbol++;
		context->column++;

		if (context->current_sym == '\n' || context->current_sym == '\r')
		{
			context->line++;
			context->column = 0;
		}
	}
	else if (context->str_source)
	{
		context->current_sym = context->str_source[context->symbol];
		context->symbol++;
		context->column++;

		if (context->current_sym == '\n' || context->current_sym == '\r')
		{
			context->line++;
			context->column = 0;
		}
	}

	return context->current_sym;
}

int fjson_ignored_simbol(char c)
{
	if (c == '\n' || c == '\t' || c == ' ' || c == '\r')
		return 1;

	return 0;
}

typedef struct
{
	int step;
	int size;
	int pos;
	char *data;
} fjson_parse_dyn_str_t;

int fjson_parse_dyn_str_append(fjson_parse_dyn_str_t *string, char c)
{
	if (!string->step)
		string->step = 64;

	if (string->pos >= string->size)
	{
		char *tmp = realloc(string->data, (string->size + string->step) * sizeof(char));
		if (!tmp)
		{
			free(string->data);
			return 0;
		}
		else
		{
			string->size += string->step;
			string->data = tmp;
		}
	}

	string->data[string->pos++] = c;

	return 1;
}

fjson_object_t *fjson_parse_object(fjson_parsing_context_t *context)
{
	fjson_array_fsm_state state = ARR_PARSE_STARTED;

	fjson_object_t *result = fjson_object_create();
	if (!result)
	{
		fprintf(stderr, "failed to parse object in array\n");
		return NULL;
	}

	fjson_string_t *key_str = NULL;

	char cur;
	for (cur = context->current_sym; cur != '\0' && cur != EOF; cur = fjson_context_get_next_sim(context))
	{
		if (fjson_ignored_simbol(cur))
		{
			continue;
		}

		switch (state)
		{
			case OBJ_PARSE_STARTED:
				if (cur == '{')
				{
					state = OBJ_PARSE_KEY_NAME_OR_END;
				}
				else
				{
					fprintf(stderr, "parse object: unexpected symbol '%c'(%d)\n", cur, cur);
					goto exit;
				}
				break;

			case OBJ_PARSE_KEY_DIV:
				if (cur == ':')
				{
					state = OBJ_PARSE_KEY_VAL;
				}
				else
				{
					fprintf(stderr, "parse object: unexpected symbol '%c'(%d)\n", cur, cur);
					goto exit;
				}
				break;

			case OBJ_PARSE_VALUE_DIV_OR_END:
				if (cur == ',')
				{
					state = OBJ_PARSE_KEY_NAME;
				}
				else if (cur == '}')
				{
					return result;
				}
				break;

			case OBJ_PARSE_KEY_NAME:
			case OBJ_PARSE_KEY_NAME_OR_END:
				if (cur == '}' && state == OBJ_PARSE_KEY_NAME_OR_END)
				{
					return result;
				}
				else if (cur == '\"')
				{
					key_str = fjson_parse_string(context);
					if (!key_str)
					{
						fprintf(stderr, "parse object: failed to parse key string\n");
						goto exit;
					}

					state = OBJ_PARSE_KEY_DIV;
				}
				else
				{
					fprintf(stderr, "parse object: unexpected symbol '%c'(%d)\n", cur, cur);
					goto exit;
				}
				break;

			case OBJ_PARSE_KEY_VAL:
				if (cur == '{')
				{
					fjson_object_t *parsed = fjson_parse_object(context);
					if (!parsed)
					{
						fprintf(stderr, "failed to parse object in object\n");
						goto exit;
					}
					else
					{
						char *key_name = fjson_string_take(key_str);
						fjson_string_free(key_str);
						key_str = NULL;

						if (0 != fjson_object_add_value(result, key_name, (fjson_value_t *)parsed, 1))
						{
							fprintf(stderr, "parse object: failed to add object in object\n");
							goto exit;
						}

						state = OBJ_PARSE_VALUE_DIV_OR_END;
					}
				}
				else if (cur == '[')
				{
					fjson_array_t *parsed = fjson_parse_array(context);
					if (!parsed)
					{
						fprintf(stderr, "failed to parse array in array\n");
						goto exit;
					}
					else
					{
						char *key_name = fjson_string_take(key_str);
						fjson_string_free(key_str);
						key_str = NULL;

						if (0 != fjson_object_add_value(result, key_name, (fjson_value_t *)parsed, 1))
						{
							fprintf(stderr, "parse object: failed to add array in object\n");
							goto exit;
						}

						state = OBJ_PARSE_VALUE_DIV_OR_END;
					}
				}
				else if (cur == '\"')
				{
					fjson_string_t *parsed = fjson_parse_string(context);
					if (!parsed)
					{
						fprintf(stderr, "failed to parse string in array\n");
						goto exit;
					}
					else
					{
						char *key_name = fjson_string_take(key_str);
						fjson_string_free(key_str);
						key_str = NULL;

						if (0 != fjson_object_add_value(result, key_name, (fjson_value_t *)parsed, 1))
						{
							fprintf(stderr, "parse object: failed to add string in object\n");
							goto exit;
						}

						state = OBJ_PARSE_VALUE_DIV_OR_END;
					}
				}
				else if (cur >= '0' && cur <= '9' || cur == '-')
				{
					fjson_number_t *parsed = fjson_parse_number(context);
					if (!parsed)
					{
						fprintf(stderr, "failed to parse number in array\n");
						goto exit;
					}
					else
					{
						char *key_name = fjson_string_take(key_str);
						fjson_string_free(key_str);
						key_str = NULL;

						if (0 != fjson_object_add_value(result, key_name, (fjson_value_t *)parsed, 1))
						{
							fprintf(stderr, "parse object: failed to add number in object\n");
							goto exit;
						}

						state = OBJ_PARSE_VALUE_DIV_OR_END;
					}
				}
				else if (cur == 't' || cur == 'n' || cur == 'f')
				{
					fjson_literal_t *parsed = fjson_parse_literal(context);
					if (!parsed)
					{
						fprintf(stderr, "failed to parse literal in array\n");
						goto exit;
					}
					else
					{
						char *key_name = fjson_string_take(key_str);
						fjson_string_free(key_str);
						key_str = NULL;

						if (0 != fjson_object_add_value(result, key_name, (fjson_value_t *)parsed, 1))
						{
							fprintf(stderr, "parse object: failed to add literal in object\n");
							goto exit;
						}

						state = OBJ_PARSE_VALUE_DIV_OR_END;
					}
				}
				else
				{
					fprintf(stderr, "parse array: unexpected symbol '%c'(%d)\n", cur, cur);
					goto exit;
				}
				break;

			default:
				fprintf(stderr, "parse object: unexpected state (%d)\n", state);
				goto exit;
		}
	}

exit:
	if (key_str)
		fjson_string_free(key_str);

	fjson_object_free(result);
	return NULL;
}

fjson_array_t *fjson_parse_array(fjson_parsing_context_t *context)
{
	fjson_array_fsm_state state = ARR_PARSE_STARTED;

	fjson_array_t *result = fjson_array_create();
	if (!result)
	{
		fprintf(stderr, "failed to parse object in array\n");
		return NULL;
	}

	char cur;
	for (cur = context->current_sym; cur != '\0' && cur != EOF; cur = fjson_context_get_next_sim(context))
	{
		if (fjson_ignored_simbol(cur))
		{
			continue;
		}

		switch (state)
		{
			case ARR_PARSE_STARTED:
				if (cur == '[')
				{
					state = ARR_PARSE_NEXT_VALUE_OR_END;
				}
				else
				{
					fprintf(stderr, "parse array: unexpected symbol '%c'(%d)\n", cur, cur);
					goto exit;
				}
				break;

			case ARR_PARSE_DIV_OR_END:
				if (cur == ',')
				{
					state = ARR_PARSE_NEXT_VALUE;
				}
				else if (cur == ']')
				{
					return result;
				}
				else
				{
					fprintf(stderr, "parse array: unexpected symbol '%c'(%d)\n", cur, cur);
					goto exit;
				}
				break;

			case ARR_PARSE_NEXT_VALUE:
			case ARR_PARSE_NEXT_VALUE_OR_END:
				if (cur == ']' && state == ARR_PARSE_NEXT_VALUE_OR_END)
				{
					return result;
				}
				else if (cur == '{')
				{
					fjson_object_t *parsed = fjson_parse_object(context);
					if (!parsed)
					{
						fprintf(stderr, "failed to parse object in array\n");
						goto exit;
					}
					else
					{
						if (0 != fjson_array_add_value(result, (fjson_value_t *)parsed))
						{
							fprintf(stderr, "parse array: failed to add object in array\n");
							goto exit;
						}

						state = ARR_PARSE_DIV_OR_END;
					}
				}
				else if (cur == '[')
				{
					fjson_array_t *parsed = fjson_parse_array(context);
					if (!parsed)
					{
						fprintf(stderr, "failed to parse array in array\n");
						goto exit;
					}
					else
					{
						if (0 != fjson_array_add_value(result, (fjson_value_t *)parsed))
						{
							fprintf(stderr, "parse array: failed to add array in array\n");
							goto exit;
						}

						state = ARR_PARSE_DIV_OR_END;
					}
				}
				else if (cur == '\"')
				{
					fjson_string_t *parsed = fjson_parse_string(context);
					if (!parsed)
					{
						fprintf(stderr, "failed to parse string in array\n");
						goto exit;
					}
					else
					{
						if (0 != fjson_array_add_value(result, (fjson_value_t *)parsed))
						{
							fprintf(stderr, "parse array: failed to add string in array\n");
							goto exit;
						}

						state = ARR_PARSE_DIV_OR_END;
					}
				}
				else if (cur >= '0' && cur <= '9' || cur == '-')
				{
					fjson_number_t *parsed = fjson_parse_number(context);
					if (!parsed)
					{
						fprintf(stderr, "failed to parse number in array\n");
						goto exit;
					}
					else
					{
						if (0 != fjson_array_add_value(result, (fjson_value_t *)parsed))
						{
							fprintf(stderr, "parse array: failed to add number in array\n");
							goto exit;
						}

						state = ARR_PARSE_DIV_OR_END;
					}
				}
				else if (cur == 't' || cur == 'n' || cur == 'f')
				{
					fjson_literal_t *parsed = fjson_parse_literal(context);
					if (!parsed)
					{
						fprintf(stderr, "failed to parse literal in array\n");
						goto exit;
					}
					else
					{
						if (0 != fjson_array_add_value(result, (fjson_value_t *)parsed))
						{
							fprintf(stderr, "parse array: failed to add literal in array\n");
							goto exit;
						}

						state = ARR_PARSE_DIV_OR_END;
					}
				}
				else
				{
					fprintf(stderr, "parse array: unexpected symbol '%c'(%d)\n", cur, cur);
					goto exit;
				}
				break;

			default:
				fprintf(stderr, "parse array: unexpected state (%d)\n", state);
				goto exit;
		}
	}

exit:
	fjson_array_free(result);
	return NULL;
}

fjson_string_t *fjson_parse_string(fjson_parsing_context_t *context)
{
	if (context->current_sym != '\"')
		return NULL;

	fjson_parse_dyn_str_t tmp_string = {};

	int escape = 0;
	int done = 0;
	char cur;
	for (cur = fjson_context_get_next_sim(context); cur != '\0' && cur != EOF; cur = fjson_context_get_next_sim(context))
	{
		int adding = 0;

		if (cur == '\\')
		{
			escape = 1;
		}
		else if (cur == '"')
		{
			if (escape)
			{
				escape = 0;
				adding = 1;
			}
			else
			{
				done = 1;
			}
		}
		else if (isprint(cur) && cur != '\n' && cur != '\r')
		{
			adding = 1;
		}

		if (adding)
		{
			if (escape)
			{
				escape = 0;
				if (!fjson_parse_dyn_str_append(&tmp_string, '\\'))
				{
					fprintf(stderr, "parse string: failed to append symbol to string\n");
					free(tmp_string.data);
					return NULL;
				}
			}

			if (!fjson_parse_dyn_str_append(&tmp_string, cur))
			{
				fprintf(stderr, "parse string: failed to append symbol to string\n");
				free(tmp_string.data);
				return NULL;
			}
		}

		if (done)
		{
			fjson_string_t *result = fjson_string_create(tmp_string.data, 1);
			if (!result)
			{
				free(tmp_string.data);
				return NULL;
			}
			else
			{
				return result;
			}
		}
	}

	return NULL;
}

fjson_number_t *fjson_parse_number(fjson_parsing_context_t *context)
{
	fjson_number_fsm_state state = NUM_PARSE_START;

	fjson_parse_dyn_str_t number_string = {};

	char cur;
	for (cur = context->current_sym; ; cur = fjson_context_get_next_sim(context))
	{
		int append = 0;
		int done = 0;

		switch (state)
		{
			case NUM_PARSE_START:
				if (cur >= '1' && cur <= '9')
				{
					append = 1;
					state = NUM_PARSE_DIGIT;
				}
				else if (cur == '0')
				{
					append = 1;
					done = 1;
				}
				else if (cur == '-')
				{
					append = 1;
					state = NUM_PARSE_DIGIT_FIRST;
				}
				else
				{
					fprintf(stderr, "parse number: unexpected symbol '%c'(%d)\n", cur, cur);
					free(number_string.data);
					return NULL;
				}
				break;

			case NUM_PARSE_DIGIT_FIRST:
				if (cur >= '1' && cur <= '9')
				{
					append = 1;
					state = NUM_PARSE_DIGIT;
				}
				else if (cur == '0')
				{
					append = 1;
					done = 1;
				}
				else
				{
					fprintf(stderr, "parse number: unexpected symbol '%c'(%d)\n", cur, cur);
					free(number_string.data);
					return NULL;
				}
				break;

			case NUM_PARSE_DIGIT:
				if (cur >= '0' && cur <= '9')
				{
					append = 1;
				}
				else if (cur == '.')
				{
					append = 1;
					state = NUM_PARSE_FRAC;
				}
				else if (cur == 'e' || cur == 'E')
				{
					append = 1;
					state = NUM_PARSE_EXP;
				}
				else
				{
					done = 1;
				}
				break;

			case NUM_PARSE_FRAC_DIGIT:
				if (cur >= '0' && cur <= '9')
				{
					append = 1;
				}
				else if (cur == 'e' || cur == 'E')
				{
					append = 1;
					state = NUM_PARSE_EXP;
				}
				else
				{
					done = 1;
				}
				break;

			case NUM_PARSE_FRAC:
				if (cur >= '0' && cur <= '9')
				{
					append = 1;
					state = NUM_PARSE_FRAC_DIGIT;
				}
				else
				{
					fprintf(stderr, "parse number: unexpected symbol '%c'(%d)\n", cur, cur);
					free(number_string.data);
					return NULL;
				}
				break;

			case NUM_PARSE_EXP_DIGIT:
				if (cur >= '0' && cur <= '9')
				{
					append = 1;
				}
				else
				{
					done = 1;
				}
				break;

			case NUM_PARSE_EXP:
				if (cur == '+' || cur == '-' || (cur >= '0' && cur <= '9'))
				{
					append = 1;
					state = NUM_PARSE_EXP_DIGIT;
				}
				else
				{
					fprintf(stderr, "parse number: unexpected symbol '%c'(%d)\n", cur, cur);
					free(number_string.data);
					return NULL;
				}
				break;

			default:
				fprintf(stderr, "parse number: unexpected state (%d)\n", state);
				free(number_string.data);
				return NULL;
		}

		if (append)
		{
			if (!fjson_parse_dyn_str_append(&number_string, cur))
			{
				fprintf(stderr, "parse number: failed to append symbol to number string\n");
				free(number_string.data);
				return NULL;
			}
		}

		if (done)
		{
			fjson_number_t *result = fjson_number_create(number_string.data, 1);
			if (!result)
			{
				free(number_string.data);
				return NULL;
			}
			else
			{
				context->skip_next += 1;
				return result;
			}
		}

	}

	return NULL;
}

fjson_literal_t *fjson_parse_literal(fjson_parsing_context_t *context)
{
	fjson_literal_fsm_state state;

	switch (context->current_sym)
	{
		case 't':
			state = LIT_PARSE_TRUE;
			break;
		case 'f':
			state = LIT_PARSE_FALSE;
			break;
		case 'n':
			state = LIT_PARSE_NULL;
			break;

		default:
			fprintf(stderr, "parse literal: unexpected symbol '%c'(%d)\n", context->current_sym, context->current_sym);
			return NULL;
	}

	int i;
	char cur;
	for (cur = context->current_sym, i = 0; ; cur = fjson_context_get_next_sim(context), i++)
	{
		switch (state)
		{
			case LIT_PARSE_TRUE:
				if (cur == 't' && i == 0 || cur == 'r' && i == 1 || cur == 'u' && i == 2)
				{
					continue;
				}
				else if (cur == 'e' && i == 3)
				{
					return fjson_literal_create(JSON_LITERAL_TRUE);
				}
				else
				{
					fprintf(stderr, "parse literal true: unexpected symbol '%c'(%d)\n", cur, cur);
					return NULL;
				}
				break;

			case LIT_PARSE_FALSE:
				if (cur == 'f' && i == 0 || cur == 'a' && i == 1 || cur == 'l' && i == 2 || cur == 's' && i == 3)
				{
					continue;
				}
				else if (cur == 'e' && i == 4)
				{
					return fjson_literal_create(JSON_LITERAL_FALSE);
				}
				else
				{
					fprintf(stderr, "parse literal false: unexpected symbol '%c'(%d)\n", cur, cur);
					return NULL;
				}
				break;

			case LIT_PARSE_NULL:
				if (cur == 'n' && i == 0 || cur == 'u' && i == 1 || cur == 'l' && i == 2)
				{
					continue;
				}
				else if (cur == 'l' && i == 3)
				{
					return fjson_literal_create(JSON_LITERAL_NULL);
				}
				else
				{
					fprintf(stderr, "parse literal null: unexpected symbol '%c'(%d)\n", cur, cur);
					return NULL;
				}
				break;

			default:
				fprintf(stderr, "parse literal: unexpected state (%d)\n", state);
				return NULL;
		}
	}

	return NULL;
}

fjson_value_t *fjson_parse_doc(fjson_parsing_context_t *context)
{
	fjson_value_t *result = NULL;

	fjson_doc_fsm_state state = DOC_STARTED;

	char c;
	for (c = fjson_context_get_next_sim(context); c != '\0' && c != EOF; c = fjson_context_get_next_sim(context))
	{
		if (fjson_ignored_simbol(c))
			continue;

		switch (state)
		{
			case DOC_STARTED:
				if (c == '{')
				{
					fjson_object_t *parsed_obj = fjson_parse_object(context);
					if (!parsed_obj)
					{
						fprintf(stderr, "failed to parse json root object\n");
						state = DOC_PARSE_FAILED;
					}
					else
					{
						result = (fjson_value_t *)parsed_obj;
						state = DOC_COMPLETED;
					}
				}
				else if (c == '[')
				{
					fjson_array_t *parsed_arr = fjson_parse_array(context);
					if (!parsed_arr)
					{
						fprintf(stderr, "failed to parse json root array\n");
						state = DOC_PARSE_FAILED;
					}
					else
					{
						result = (fjson_value_t *)parsed_arr;
						state = DOC_COMPLETED;
					}
				}
				else if (c == '\"')
				{
					fjson_string_t *parsed_string = fjson_parse_string(context);
					if (!parsed_string)
					{
						fprintf(stderr, "failed to parse json root string\n");
						state = DOC_PARSE_FAILED;
					}
					else
					{
						result = (fjson_value_t *)parsed_string;
						state = DOC_COMPLETED;
					}
				}
				else if (c >= '0' && c <= '9' || c == '-')
				{
					fjson_number_t *parsed_number = fjson_parse_number(context);
					if (!parsed_number)
					{
						fprintf(stderr, "failed to parse json root number\n");
						state = DOC_PARSE_FAILED;
					}
					else
					{
						result = (fjson_value_t *)parsed_number;
						state = DOC_COMPLETED;
					}
				}
				else if (c == 't' || c == 'n' || c == 'f')
				{
					fjson_literal_t *parsed_literal = fjson_parse_literal(context);
					if (!parsed_literal)
					{
						fprintf(stderr, "failed to parse json root literal\n");
						state = DOC_PARSE_FAILED;
					}
					else
					{
						result = (fjson_value_t *)parsed_literal;
						state = DOC_COMPLETED;
					}
				}
				else
				{
					fprintf(stderr, "unexpected symbol '%c(%d)'\n", c, c);
					state = DOC_PARSE_FAILED;
				}
				break;

			case DOC_COMPLETED:
				fprintf(stderr, "Json document must have only one root value\n");
				state = DOC_PARSE_FAILED;

			default:
				break;
		}

		if (state == DOC_PARSE_FAILED)
		{
			fprintf(stderr, "Failed to parse JSON document, error position %d, line %d, column %d\n", context->symbol, context->line, context->column);
			return NULL;
		}
	}

	return result;
}

fjson_value_t *fjson_parse_json_string(char *json_string)
{
	fjson_parsing_context_t context = {};
	context.str_source = json_string;

	return fjson_parse_doc(&context);
}

fjson_value_t *fjson_parse_json_file(char *json_file)
{
	if (!json_file)
		return NULL;

	FILE *f = fopen(json_file, "r");
	if (!f)
		return NULL;

	fjson_parsing_context_t context = {};
	context.filename = json_file;
	context.fd_source = f;

	return fjson_parse_doc(&context);
}