#include <direct.h>
#include <windows.h>
#include <stdio.h>
#include <bs_api.h>

#include "OS/OS.h"

BS_API char* bytescript_module_map = "OS_map.bs";
BS_API TValue bytescript_module_loader(TValue* argv, size_t argc) {
    TValue fields[] = {
        new_cfunc(env_os),
        new_cfunc(cwd_os),
        new_cfunc(chdir_os),
        new_cfunc(absolute_os),
        new_cfunc(normalize_os),
        new_cfunc(filename_os),
        new_cfunc(stem_os),
        new_cfunc(ext_os),
        new_cfunc(parent_os),
        new_cfunc(join_os),
        new_cfunc(run_os),
        new_cfunc(exit_os)
    };

    size_t field_count = sizeof(fields) / sizeof(fields[0]);
    memcpy(argv, fields, BSIZE(field_count));
}


TValue env_os(TValue* argv, size_t argc) {
    MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "OS.env");

    const char* env = getenv(AS_STRING(argv[0])->str);
    return make_fstring((char*)env);
}

TValue cwd_os(TValue* argv, size_t argc) {
    char* cwd = _getcwd(NULL, 0);
    TValue tcwd = make_fstring(cwd);

    free(cwd);
    return tcwd;
}

TValue chdir_os(TValue* argv, size_t argc) {
    MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "OS.chdir");
    _chdir(AS_STRING(argv[0])->str);

    return NIL;
}

TValue absolute_os(TValue* argv, size_t argc) {
    MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "OS.absolute");

    TStr* str = AS_STRING(*argv);
    size_t len = GetFullPathNameA(str->str, 0, NULL, NULL);

    char buffer[len]; GetFullPathNameA(str->str, len, buffer, NULL);
    return make_fstring(buffer);
}

TValue normalize_os(TValue* argv, size_t argc) {
    MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "OS.normalize");

    struct StrLinkedList stack = { .str = NULL, .len = 0, .next = NULL };

    char* str = AS_STRING(argv[0])->str;
    char buffer[AS_STRING(argv[0])->len];
    char* cbuff = buffer;
    char* end;

    while (end = strchr(str, '/')) {
        size_t len = end - str;

        if (!strncmp(str, "..", len > 1 ? len : 2) && str != AS_STRING(argv[0])->str) {
            if (stack.next) {
                struct StrLinkedList* current = stack.next;
                stack.next = current->next;

                free(current);
            }

            goto normalize_os_next;

        } else if (!strncmp(str, ".", len)) goto normalize_os_next;

        struct StrLinkedList* new = malloc(sizeof(struct StrLinkedList));
        new->str = str;
        new->len = len;
        new->next = stack.next;

        stack.next = new;

        normalize_os_next:
        str += len + 1;
    }

    size_t stack_len = linked_str_len(stack);
    struct MiddleStr arr[stack_len]; linked_str_to_array(stack, arr, stack_len);

    for (int i = 0; i < stack_len; i++) {
        strncpy(cbuff, arr[i].str, arr[i].len);
        cbuff[arr[i].len] = '/';
        cbuff += arr[i].len + 1;
    }

    size_t end_len = strlen(str);
    size_t terminator = cbuff - buffer + end_len - !end_len;
    strcpy(cbuff, str);

    buffer[terminator] = '\0';
    return make_fstring(buffer);
}

TValue filename_os(TValue* argv, size_t argc) {
    MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "OS.filename");
    char* filename = strrchr(AS_STRING(argv[0])->str, '/');

    return !filename ? *argv : make_fstring(filename);
}

TValue stem_os(TValue* argv, size_t argc) {
    MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "OS.stem");
    TStr* str = AS_STRING(argv[0]);

    char* filename = strrchr(str->str, '/');
    if (!filename) filename = str->str;

    char* start = strrchr(str->str, '.');
    if (!start || start < filename) start = str->str + str->len;

    return make_string(filename, start - filename);
}

TValue ext_os(TValue* argv, size_t argc) {
    MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "OS.ext");
    char* start = strrchr(AS_STRING(argv[0])->str, '.');

    return start ? make_fstring(start) : make_fstring("");
}

TValue parent_os(TValue* argv, size_t argc) {
    MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "OS.parent");
    TStr* str = AS_STRING(absolute_os(argv, 1));

#ifdef _WIN32


    char* filename = strrchr(str->str, '\\');
#else
    char *filename = strrchr(str->str, '/');
#endif
    if (!filename) filename = str->str;

    return make_string(str->str, filename - str->str);
}

TValue join_os(TValue* argv, size_t argc) {
    MIN_ARGC(2);
    TYPE_CHECK(0, STRING, "String", "OS.join");
    TYPE_CHECK(1, STRING, "String", "OS.join");

    TValue str = concat_string((TValue[2]){ *argv, make_fstring("/") }, 2);
    return concat_string((TValue[2]){ str, argv[1] }, 2);
}

TValue run_os(TValue* argv, size_t argc) {
    MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "OS.run");
    TStr* str = AS_STRING(absolute_os(argv, 1));

    return NUM_AS_TVALUE(system(str->str));
}

TValue exit_os(TValue* argv, size_t argc) {
    MIN_ARGC(1); NUM_CHECK(0, "OS.exit");
    exit(argv[0].num);
}


static size_t linked_str_len(struct StrLinkedList head) {
    int argc = 0;
    struct StrLinkedList* current = head.next;

    for (; current; current = current->next) { argc++; }
    return argc;
}

static void linked_str_to_array(struct StrLinkedList head, struct MiddleStr arr[], size_t argc) {
    struct StrLinkedList* current = head.next;
    for (int i = 0; i < argc; (current = current->next, i++)) {
        arr[argc - i - 1].str = current->str;
        arr[argc - i - 1].len = current->len;
    }
}
