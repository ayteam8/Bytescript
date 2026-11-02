#include "Values/tutils.h"
#include "Globals/error.h"
#include "Lexer/lexer.h"

#include <sys/stat.h>
#include <windows.h>


TValue to_return_value(TValue returnv[], size_t returnc) {
    if (returnc == 1) return *returnv;
    if (returnc == 0) return NIL;
    return new_table(returnv, returnc);
}

TValue import_path_finder(TValue path) {
    struct stat st;
	
    char buff[32767];
    GetModuleFileName(NULL, buff, 32767);
#ifdef _WIN32
    char sep = '\\';
#else
    char sep = '/';
#endif
    char* last = strrchr(buff, sep); if (last) *(last + 1) = '\0';

    TValue exec_path = make_fstring(buff);
    TValue stdlib_path = concat_string((TValue[2]){ exec_path, make_fstring("stdlib/") }, 2);

    TValue stdlib_try = concat_string((TValue[2]){ stdlib_path, path }, 2); 

    if (!stat(AS_STRING(path)->str, &st)) return path;
    if (!stat(AS_STRING(stdlib_try)->str, &st)) return stdlib_try;

    TValue error_message = make_fstring("Could not find file <%s>");
    error_message = format_string((TValue[2]){ error_message, path }, 2);

    call_error(new_failed_allocation_error, AS_STRING(error_message)->str);
}

char* read_string() {
    size_t size = 4;
    size_t count = 0;
    char* str = malloc(size + 1);
    if (!str) return NULL;

    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        if (count >= size) {
            size *= 2;
            char* new_str = realloc(str, size + 1);
            if (!new_str) call_error(new_failed_allocation_error, "Fatal Error : Failed allocation while reading stdout");

            str = new_str;
        }

        str[count++] = (char)c;
    }

    str[count] = '\0';
    return str;
}

void print_tokens(Token* tokens) {
    printf("\n[\n");

    do {
        printf("\tTT : %3d; Value : ", tokens->tt);
        print(&tokens->value, 1);
    } while ((tokens++)->tt != TT_EOF);

    printf("]\n");
}

void print_hash_table(TTable* tab) {
    printf("{\n");

    for (int i = 0; i < tab->hash_table.capacity; i++) {
        Bucket* buck = tab->hash_table.array + i;
        TValue key = buck->pair.key;
        TValue value = buck->pair.value;

        if (!IS_NIL(key) && !IS_NIL(value)) { printf("\t"); print((TValue[2]){ key, value }, 2); }
        if (buck->sub.arr) {
            for (int j = 0; j < buck->sub.capacity; j++) {
                Pair* pair = buck->sub.arr + j;
                TValue sub_key = pair->key;
                TValue sub_value = pair->value;

                if (!IS_NIL(sub_key) && !IS_NIL(sub_value)) { printf("\t\t"); print((TValue[2]){ sub_key, sub_value }, 2); }
            }
        }
    }

    printf("}\n");
}

int get_linked_list_len(struct TValueLinkedList list) {
    int argc = 0;
    struct TValueLinkedList* arg = list.next;

    for (; arg; arg = arg->next) { argc++; }
    return argc;
}

void linked_list_to_array(struct TValueLinkedList list, TValue array[], size_t len) {
    struct TValueLinkedList* arg = list.next;
    for (int i = 0; i < len; (arg = arg->next ,i++)) { array[len - i - 1] = arg->value; }
}

void free_linked_list(struct TValueLinkedList list) {
    for (struct TValueLinkedList* current = list.next; current;) {
        struct TValueLinkedList* to_free = current;
        current = current->next;

        free(to_free);
    }

    list.next = NULL;
}
