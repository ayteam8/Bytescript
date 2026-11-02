#include "Globals/string.h"
#include "Globals/table.h"
#include "Values/oop.h"
#include "stdlib_src/Math/math_utils.h"

#include "Values/bytescript_funcs.h"
#include <stdbool.h>


const TTable* string_cache = NULL;
uint8_t flag_flags[] = { 1 << 0, 1 << 1, 1 << 2, 1 << 3 };
char valid_flags[] = { '0', ' ', '+', '-' };

TValue String_class;
void initialize_string_class() {
    TValue String_fields[] = {
        new_cfunc(compare_string),
        new_cfunc(format_string),
        new_cfunc(len_string),

        new_cfunc(to_string_string),

        new_cfunc(concat_string),
        new_cfunc(index_string)
    };

    clean_node(easy_map("string_map.bs"));
    String_class = new_class(NULL, String_fields, 0, sizeof(String_fields) / sizeof(String_fields[0]));

    add_global(String_class, make_fstring("String"));
}


TValue compare_string(TValue* argv, size_t argc) {
    MIN_ARGC(2);

    TValue self = argv[0];
    TValue other = argv[1];
    if (!IS_STRING(self)) call_error(new_type_error, "function <String.compare> take type <String> as arguments");
    if (!IS_STRING(other)) call_error(new_type_error, "function <String.compare> take type <String> as arguments");

    TStr* self_ptr = AS_STRING(self);
    TStr* other_ptr = AS_STRING(other);
    
    int res = strcmp(self_ptr->str, other_ptr->str);
    return res == 0 ? TRUE_TVALUE : FALSE_TVALUE;
}

TValue format_string(TValue* argv, size_t argc) {
    if (argc < 1) return NIL;
    if (!IS_STRING(argv[0])) call_error(new_type_error, "function <String.format> take type <String> as 1st arguments");

    char specifiers[] = { 's', 'f', 'e', 'g' };

    FormatInfos format_speicifers[argc - 1];
    size_t format_lenghts[argc - 1];
    size_t current_format = 0;

    TStr* ptr = AS_STRING(argv[0]);
    TValue args[argc - 1];
    size_t base_size = 0;
    size_t args_size = 0;

    for (int i = 0; i < ptr->len; i++) {
        if (ptr->str[i] == '%') {
            if (current_format >= argc - 1) call_error(new_invalid_args_error, "not enough arguments in function <String.format>");
            int start_i = i;
            
            format_speicifers[current_format].flags = 0;
            format_speicifers[current_format].precision = -1;
            format_speicifers[current_format].specifier = '\0';
            i++; base_size++;

            for (int j = 0; j < sizeof(valid_flags); j++) {
                if (ptr->str[i] == valid_flags[j]) {
                    i++; base_size++;
                    format_speicifers[current_format].flags |= flag_flags[j]; j = -1;
                };
            }

            char* end;
            int pre_i = i;
            format_speicifers[current_format].width = strtol(ptr->str + i, &end, 10);
            i = end - ptr->str; base_size += i - pre_i;

            if (ptr->str[i] == '.') {
                i++; pre_i = i;
                format_speicifers[current_format].precision = strtol(ptr->str + i, &end, 10);
                i = end - ptr->str; base_size += i - pre_i + 1;
            }

            for (int j = 0; j < sizeof(specifiers); j++) if (ptr->str[i] == specifiers[j]) format_speicifers[current_format].specifier = ptr->str[i];
            if (!format_speicifers[current_format].specifier) call_error(new_invalid_args_error, "Invalid format specifier in function <String.format>");

            format_lenghts[current_format] = i - start_i + 1;
            base_size++;
            current_format++;
        }
    }

    for (int i = 0; i < argc - 1; i++) {
        switch (format_speicifers[i].specifier) {
            case 's': args[i] = format_s(format_speicifers[i], argv[i + 1]); args_size += AS_STRING(args[i])->len; break;
            case 'f': args[i] = format_n(format_speicifers[i], argv[i + 1]); args_size += AS_STRING(args[i])->len; break;
            case 'e': args[i] = format_n(format_speicifers[i], argv[i + 1]); args_size += AS_STRING(args[i])->len; break;
            case 'g': args[i] = format_n(format_speicifers[i], argv[i + 1]); args_size += AS_STRING(args[i])->len; break;
            default: call_error(new_invalid_args_error, "Invalid format specifier in function <String.format>");   break;
        }
    }

    TStr* new = malloc(sizeof(TStr) + ptr->len - base_size + args_size + 1);
    new->len = ptr->len - base_size + args_size;

    current_format = 0;
    char* current = new->str;
    char* pre = ptr->str;
    for (int i = 0; i < ptr->len; i++) {
        if (ptr->str[i] == '%') {
            size_t jump_len = (ptr->str + i) - pre;
            TStr* current_arg = AS_STRING(args[current_format]);

            strncpy(current, pre, jump_len); current += jump_len;
            strncpy(current, current_arg->str, current_arg->len); current += current_arg->len;
            i += format_lenghts[current_format]; current_format++; pre = ptr->str + i;
        }
    }

    strncpy(current, pre, ptr->str + ptr->len - pre);
    new->str[new->len] = '\0';
    return STR_TO_TVALUE(new);
}

TValue len_string(TValue* argv, size_t argc) {
    MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "String.len")

    TValue self = argv[0];
    TStr* ptr = AS_STRING(self);

    return NUM_AS_TVALUE(ptr->len);
}


TValue to_string_string(TValue* argv, size_t argc) {
    TValue str1 = NIL;

    for (int i = 0; i < argc; i++) {
        TValue v = argv[i];
        TValue object;
        void* ptr;

        switch (TYPE(v)) {
            case STRING: {
                str1 = IS_NIL(str1) ? v : concat_string(( TValue[2] ){ str1, v }, 2);
                break;
            }

            case TABLE:
            case FUNC: {
                object = TYPE(v) == FUNC ? make_fstring("function at") : make_fstring("table at");;
                void* ptr = (void*)PAYLOAD(v);


                int needed = snprintf(NULL, 0, "<%p>", ptr);
                char buffer[needed + 1];
                snprintf(buffer, needed + 1, "<%p>", ptr);

                TValue str = make_fstring(buffer);
                str = concat_string(( TValue[2] ){ object, str }, 2);

                str1 = IS_NIL(str1) ? str : concat_string(( TValue[2] ){ str1, str }, 2);
                break;
            }

            default:
                int needed = snprintf(NULL, 0, "%.15g", AS_NUM(v));
                char buffer[needed + 1];
                snprintf(buffer, needed + 1, "%.15g", AS_NUM(v));

                str1 = IS_NIL(str1) ? make_fstring(buffer) : concat_string(( TValue[2] ){ str1, make_fstring(buffer) }, 2);
                break;
        }
    }

    return to_return_value(&str1, 1);
}

TValue concat_string(TValue* argv, size_t argc) {
    STRICT_MIN_ARGC(2, "function <String.concat> take 2 arguments")

    TValue self = argv[0];
    TValue other = argv[1];
    if (!IS_STRING(self)) call_error(new_type_error, "function <String.concat> take type <String> as arguments");
    if (!IS_STRING(other)) call_error(new_type_error, "function <String.concat> take type <String> as arguments");

    TStr* self_ptr = AS_STRING(self);
    TStr* other_ptr = AS_STRING(other);

    TStr* new = malloc(sizeof(TStr) + self_ptr->len + other_ptr->len + 1);
    strcpy(new->str, self_ptr->str);
    strcat(new->str, other_ptr->str);
    new->len = self_ptr->len + other_ptr->len;
    new->str[new->len] = '\0';

    return cache_string(new);
}

TValue index_string(TValue* argv, size_t argc) {
    MIN_ARGC(2);
    TYPE_CHECK(0, STRING, "String", "String.index");
    NUM_CHECK(1, "String.index");

    TValue self = argv[0];
    size_t idx = argv[1].num;

    TStr* ptr = AS_STRING(self);
    if (idx < 0 || idx >= ptr->len) call_error(new_index_error, "Index out of scope when trying to index a string");

    return make_string(ptr->str +(size_t)argv[1].num, 1);
}


inline TValue cache_string(TStr* str) {
    TValue tstr = STR_TO_TVALUE(str);
    TValue cached =  get_value_from_hashtable(TBL_TO_TVALUE(string_cache), tstr);

    if (!IS_NIL(cached)) {
        free(str);
        return cached;
    }

    add_value_to_hashtable(TBL_TO_TVALUE(string_cache), tstr, tstr);
    return tstr;
}

inline TValue make_string(char* string, size_t len) {
    TStr* str = malloc(sizeof(TStr) + len + 1);
    strncpy(str->str, string, len);

    str->str[len] = '\0';
    str->len = len;
    return cache_string(str);
}

inline TValue make_fstring(char* str) {
    return make_string(str, strlen(str));
}

TValue format_s(FormatInfos infos, TValue value) {
    if (!IS_STRING(value)) call_error(new_type_error, "Excepted type <String> in function <String.format>.\n\tInvalid format specifier '\%s'");
    
    TStr* ptr = AS_STRING(value);
    size_t precision = infos.precision + 1 ? CLAMP(infos.precision, 0, ptr->len) : ptr->len;
    size_t len = MAX(precision, infos.width);

    TStr* new = malloc(sizeof(TStr) + len + 1);
    new->len = len;

    char* loc = infos.flags & FORMAT_FLAG_LEFT ? new->str : new->str + len - precision;
    char pad = infos.flags & FORMAT_FLAG_ZERO ? '0' : ' ';

    memset(new->str, pad, len);
    strncpy(loc, ptr->str, precision);

    new->str[new->len] = '\0';
    return STR_TO_TVALUE(new);
}

TValue format_n(FormatInfos infos, TValue value) {
    if (!IS_NUM(value)) call_error(new_type_error, "Excepted type <Number> in function <String.format>.\n\tInvalid format specifier '\%f' || '\%e' || '\%g'");

    size_t precision = infos.precision + 1 ? MAX(infos.precision, 0) : 6;

    int needed = snprintf(NULL, 0, ".%zu%c", precision, infos.specifier);
    char buffer[needed + 1]; snprintf(buffer + 1, needed + 1, ".%zu%c", precision, infos.specifier);
    buffer[0] = '%';

    needed = snprintf(NULL, 0, buffer, AS_NUM(value));
    char num_buffer[needed + 1]; snprintf(num_buffer, needed + 1, buffer, AS_NUM(value));
    char res_buffer[needed + 2];

    int flag_space = infos.flags & FORMAT_FLAG_SPACE;
    int flag_sign = infos.flags & FORMAT_FLAG_SIGN;
    int flag_pad = flag_space || flag_sign;

    if (flag_pad && AS_NUM(value) >= 0) {
        char pad = infos.flags & FORMAT_FLAG_SIGN ? '+' : ' ';
        strcpy(res_buffer + 1, num_buffer); res_buffer[0] = pad;
    }

    size_t width = flag_pad ? needed + 2 : needed + 1;
    size_t len = MAX(width, infos.width);

    FormatInfos new_infos = {
        .flags = infos.flags,
        .precision = needed + 1,
        .width = len,
        .specifier = 's'
    };

    return format_s(new_infos, make_fstring(flag_pad ? res_buffer : num_buffer));
}
