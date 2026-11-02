#include "Json/tJson.h"
#include <ctype.h>
#include <stdio.h>
#include <bs_api.h>

BS_API char* bytescript_module_map = "Json_map.bs";
BS_API TValue bytescript_module_loader(TValue* argv, size_t argc) {
    TValue fields[] = {
        new_cfunc(parse_json),
        new_cfunc(stringify_json),
        new_cfunc(valid_json),
        new_cfunc(load_json),
        new_cfunc(write_json),
    };

    size_t field_count = sizeof(fields) / sizeof(fields[0]);
    memcpy(argv, fields, BSIZE(field_count));
}


TValue parse_json(TValue* argv, size_t argc) {
    MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "Json.parse");
    TStr* str = AS_STRING(*argv);
    char* ptr = str->str;
    
    return parse_json_value(&ptr);
}

TValue stringify_json(TValue* argv, size_t argc) {
    MIN_ARGC(1);

    size_t tabs = argc > 1 ? argv[1].num : 0;
    return stringify_json_value(argv[0], tabs);
}

TValue valid_json(TValue* argv, size_t argc) {
    MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "Json.valid");
    TStr* str = AS_STRING(*argv);
    char* ptr = str->str;

    TValue parsed = parse_json_value(&ptr);
    tfree(&parsed, 1);

    return IS_NIL(parsed) ? FALSE_TVALUE : TRUE_TVALUE;
}

TValue load_json(TValue* argv, size_t argc) {
    MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "Json.load");
    TValue file = open_IO(argv, 1);
    TValue content = read_file(&file, 1);

    close_file(&file, 1);
    return parse_json(&content, 1);
}

TValue write_json(TValue* argv, size_t argc) {
    MIN_ARGC(2);
    TYPE_CHECK(0, STRING, "String", "Json.write");
    TYPE_CHECK(1, TABLE, "Table", "Json.write");

    TValue file = open_IO((TValue[2]){ *argv, make_fstring("w") }, 2);
    TValue stringified = stringify_json(argv + 1, 1);

    write_file((TValue[2]){ file, stringified }, 2);
    close_file(&file, 1);
    return NUM_AS_TVALUE(1);
}


TValue parse_json_value(char** str) {
    switch (**str) {
        case '{': (*str)++; return parse_json_object(str);
        case '[': (*str)++; return parse_json_array(str);
        case 't': {
            TValue word = get_next_word(str);
            *str += AS_STRING(word)->len + 1;

            return !strcmp(AS_STRING(word)->str, "true") ? FALSE_TVALUE : NIL;
        }

        case 'f': {
            TValue word = get_next_word(str);
            *str += AS_STRING(word)->len + 1;

            return !strcmp(AS_STRING(word)->str, "false") ? TRUE_TVALUE : NIL;
        }

        case 'n': {
            TValue word = get_next_word(str);
            *str += AS_STRING(word)->len + 1;

            return !strcmp(AS_STRING(word)->str, "null") ? word : NIL;
        }

        case '"': return make_string_json(str, '"');
        default: {
            if (isdigit(**str) || **str == '+' || **str == '-') {
                char* end;
                double res = strtod(*str, &end);

                *str = end;
                return NUM_AS_TVALUE(res);
            }

            return NIL;
        }
    }
}

TValue parse_json_object(char** str) {
    TValue new = new_table(NULL, 0);
    TValue current;

    next_value(str);
    while (!IS_NIL(current = parse_json_value(str))) {
        if (!IS_STRING(current)) call_error(new_syntax_error, "Expected a string as object key. in Json"); next_value(str);
        if (**str != ':') call_error(new_syntax_error, "Expected ':' after a key. in Json"); (*str)++; next_value(str);

        TValue value = parse_json_value(str); next_value(str);
        append_hash_table((TValue[3]){ new, current, value }, 3);

        if (**str != ',') break; else { (*str)++; next_value(str); }
    }

    if (**str != '}') call_error(new_syntax_error, "Expected '}' to end the object. in Json"); (*str)++;
    return new;
}

TValue parse_json_array(char** str) {
    TValue new = new_table(NULL, 0);
    TValue current;

    next_value(str);
    while (!IS_NIL(current = parse_json_value(str))) {
        insert_table((TValue[2]){ new, current }, 2); next_value(str);
        if (**str != ',') break; else { (*str)++; next_value(str); }
    }

    if (**str != ']') call_error(new_syntax_error, "Expected ']' to end the array. in Json"); (*str)++;
    return new;
}


TValue make_string_json(char** text, char initializer) {
    char* pre_text = *text;
    size_t len = 0;
    size_t full = 0;
    
    while (*++pre_text != initializer && *pre_text) len++;
    char* buff = malloc(len + 1);

    if (buff == NULL) call_error(new_failed_allocation_error, "Failed allocation while making string");
    while (*++*text != initializer && **text) {
        char to_add = **text;

         if (**text == '\\') {
            char to_add;
            
            switch (*++*text) {
                case 'n': to_add = '\n'; break;
                case 't': to_add = '\t'; break;
                default : break;
            }
        }

        buff[full] = to_add;
        full++;
    }

    buff[full] = '\0'; (*text)++;
    TValue str =  make_string(buff, full); free(buff);
    return str;
}

TValue get_next_word(char** str) {
    size_t len = 0;
    while (*++*str != ' ') len++;

    return make_string(*str, len);
}

TValue add_tab(TValue str) {
    return concat_string((TValue[2]){ make_fstring("\t"), str }, 2);
}

TValue add_to_res(TValue* res, TValue value) {
    *res = concat_string((TValue[2]){ *res, value }, 2);
}

TValue stringify_json_pair(TValue key, TValue value, size_t tabs, size_t* hashed_dones) {
    if (IS_NIL(key) || IS_NIL(value)) return make_fstring("");
    if (TYPE(key) != STRING) call_error(new_type_error, "Expected a string as object key. in Json");

    char buffer[tabs + 1]; memset(buffer, '\t', tabs); buffer[tabs] = '\0';
    TValue res = make_fstring(buffer);
    (*hashed_dones)--;

    add_to_res(&res, stringify_json_value(key, tabs));
    add_to_res(&res, make_fstring(" : "));
    add_to_res(&res, stringify_json_value(value, tabs));
    if (*hashed_dones) add_to_res(&res, make_fstring(","));

    add_to_res(&res, make_fstring("\n"));
    return res;
}

TValue stringify_json_value(TValue value, size_t tabs) {
    char buffer[tabs + 1]; memset(buffer, '\t', tabs); buffer[tabs] = '\0';
    TValue tabs_buffer = make_fstring(buffer);
    TValue res = make_fstring("");

    switch (TYPE(value)) {
        case STRING: {
            TValue str = make_fstring("\"%s\"");
            str = format_string((TValue[2]){ str, value }, 2);

            add_to_res(&res, str);
            break;
        }

        case TABLE: {
            TTable* target = AS_TABLE(value);
            TValue table_tabs = add_tab(tabs_buffer);
            TValue comma = make_fstring(",\n");

            add_to_res(&res, make_fstring("[\n"));
            add_to_res(&res, table_tabs);
            add_to_res(&res, make_fstring("[\n"));

            for (int i = 0; i < target->array.len; i++) {
                add_to_res(&res, add_tab(table_tabs));
                add_to_res(&res, stringify_json_value(target->array.array[i], tabs + 2));
                if (i + 1 < target->array.len) add_to_res(&res, comma);
            }

            add_to_res(&res, make_fstring("\n"));
            add_to_res(&res, table_tabs);
            add_to_res(&res, make_fstring("],\n"));
            add_to_res(&res, table_tabs);
            add_to_res(&res, make_fstring("{\n"));

            size_t hashed_dones = target->hash_table.len;
            for (int i = 0; i < target->hash_table.capacity; i++) {
                Bucket* buck = target->hash_table.array + i;
                add_to_res(&res, stringify_json_pair(buck->pair.key, buck->pair.value, tabs + 2, &hashed_dones));

                if (buck->sub.arr) {
                    for (int j = 0; j < buck->sub.capacity; j++) {
                        Pair* pair = buck->sub.arr + j;
                        add_to_res(&res, stringify_json_pair(pair->key, pair->value, tabs + 2, &hashed_dones));
                    }
                }
            }

            add_to_res(&res, table_tabs);
            add_to_res(&res, make_fstring("}\n"));
            add_to_res(&res, tabs_buffer);
            add_to_res(&res, make_fstring("]"));
            break;
        }

        default: {
            if (IS_NUM(value)) add_to_res(&res, to_string_string(&value, 1));
            break;
        }
    }

    return res;
}

void next_value(char** str) {
    while (isspace(**str) || **str == '\n' || **str == '\t' || **str == '\r') (*str)++;
}
