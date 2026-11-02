#ifndef GLOBALS_H
#define GLOBALS_H

#include "Values/bytev.h"
#include "Values/namespaces.h"
#include "Values/tutils.h"
#include "Globals/table.h"
#include "Globals/string.h"
#include "Globals/error.h"

#define MIN_ARGC(n)                     { if (argc < n) return NIL; }
#define STRICT_MIN_ARGC(n, fname)       { if (argc < n) call_error(new_invalid_args_error, "function <" fname "> take " #n " arguments"); }
#define NUM_CHECK(n, fname)             { if (!IS_NUM(argv[n])) call_error(new_type_error, "function <" fname "> take type <Number> as " #n "th argument"); }
#define TYPE_CHECK(n, type, tnm, fname) { if (TYPE(argv[n]) != type) call_error(new_type_error, "function <" fname "> take type <" tnm "> as " #n "th argument"); }

extern TValue globals;
extern TValue global_types;
extern TValue global_functypes;
extern TTable* globals_namespace;
extern TValue globals_map;

void initialize_globals();

#define BYTESCRIPT_MAX_INPUT_BUFFER_SIZE 1024

TValue input(TValue* argv, size_t argc); // yay! user input
TValue print(TValue* argv, size_t argc);
TValue tfree(TValue* argv, size_t argc);
TValue ttypeof(TValue* argv, size_t argc);
TValue disassemble(TValue* argv, size_t argc);

int add_global(TValue value, TValue name);
int make_global(TValue value, TValue name, TValue type);
int make_global_function(TValue value, TValue name, TValue desc);
int overwrite_global(TValue value, TValue name);
TValue make_globals_types_from_file(TValue filename);
TValue get_global(TValue name);

#endif

