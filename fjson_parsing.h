#ifndef __FJSON_PARSING_H
#define __FJSON_PARSING_H

#include <fjson_value.h>

fjson_value_t *fjson_parse_json_string(char *json_string);
fjson_value_t *fjson_parse_json_file(char *json_file);

#endif