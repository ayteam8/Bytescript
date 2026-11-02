#include "Globals/globals.h"
#include "Globals/string.h"
#include "Globals/number.h"
#include "Globals/function.h"
#include "Globals/cdata.h"
#include "Globals/IO/IO.h"
#include "Globals/IO/File.h"
#include "Globals/Module/module.h"
#include "utils/disassembler.h"
#include "Parser/nodes.h"
#include "TypeResolving/type_specifiers.h"
#include "Values/bytescript_funcs.h"
#include <stdio.h>  // yk, just in case
#include <stdlib.h> // this too
#include <string.h> // also

TValue globals;
TValue global_types;
TValue global_functypes;
TTable* globals_namespace;
TValue globals_map;
void initialize_globals() {
    globals = new_table(NULL, 0);
    globals_namespace = AS_TABLE(new_table(NULL, 0));
    globals_map = TBL_TO_TVALUE(globals_namespace);

    add_global(NIL, make_fstring("NIL"));
    add_global(TRUE_TVALUE, make_fstring("true"));
    add_global(FALSE_TVALUE, make_fstring("false"));
    add_global(new_cfunc(print), make_fstring("print"));
    add_global(new_cfunc(tfree), make_fstring("free"));
    add_global(new_cfunc(disassemble), make_fstring("disassemble"));
    add_global(new_cfunc(exec), make_fstring("exec"));
    add_global(new_cfunc(input), make_fstring("input"));

    initialize_function_class();
    initialize_number_class();
    initialize_string_class();
    initialize_table_class();
    initialize_error_class();
    initialize_cdata_class();
    initialize_io_class();
    initialize_module_class();

    make_globals_types_from_file(make_fstring("globals.bs"));
}

// the input function is made by Guillermo Leira Temes
// yk, i wanted an input function

TValue input(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    TValue text = argv[0];
    if (argc > 0 && TYPE(argv[0]) != STRING) call_error(new_type_error, "function <input> take type <String> as argument");
    // ooooooh shit
    // i need to use a buffer and then malloc
    char buffer[BYTESCRIPT_MAX_INPUT_BUFFER_SIZE];
    printf("%s", AS_STRING(text)->str); // first we print the text
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strlen(buffer)-1] = '\0';
    return make_fstring(buffer);
}

TValue print(TValue* argv, size_t argc) {
    for (int i = 0; i < argc; i++) {
        TValue v = argv[i];

        switch (TYPE(v)) {
            case STRING : {
                TStr* ptr = AS_STRING(v);
                printf("%s", ptr->str);
                break;
            }

            case TABLE : {
                TTable* ptr = AS_TABLE(v);
                printf("table <%p>", ptr);
                break;
            }

            case FUNC : {
                TFunc* ptr = AS_FUNC(v);
                printf("function <%p>", ptr);
                break;
            }

            case CLASS: {
                TClass* ptr = AS_CLASS(v);
                printf("class : <%p>", v);
                break;
            }

            case OBJECT: {
                TObject* ptr = AS_OBJECT(v);
                printf("object : <%p>", v);
                break;
            }

            default : {
                double num = AS_NUM(v);
                printf("%f", num);
                break;
            }
        }

        printf("\t");
    }

    printf("\n");
    return NIL;
}

TValue tfree(TValue* argv, size_t argc) {
    for (int i = 0; i < argc; i++) {
        TValue v = argv[i];

        switch (TYPE(v)) {
            case STRING : {
                TStr* ptr = AS_STRING(v);
                free(ptr);
                break;
            }

            case TABLE : {
                free_table(&v, 1);
                break;
            }

            case FUNC : {
                TFunc* ptr = AS_FUNC(v);
                if (ptr->isCfunc) {
                    free((void*)ptr->bytescriptFunction.bytecode);
                    free((void*)ptr->bytescriptFunction.upvalues);

                }

                free(ptr);
                break;
            }
        }
    }

    return NIL;
}

TValue ttypeof(TValue* argv, size_t argc) {
    size_t returnc = argc;
    TValue returnv[returnc];

    for (int i = 0; i < argc; i++) {
        TValue value = argv[i];
        switch (TYPE(value)) {
            case STRING:    returnv[i] = String_class; break;
            case TABLE:     returnv[i] = Table_class; break;
            case FUNC:      returnv[i] = Function_class; break;
            case CLASS:     returnv[i] = value; break;
            case OBJECT:    returnv[i] = CLS_TO_TVALUE(AS_OBJECT(value)->class); break;
            case CDATA:     returnv[i] = CData_class; break;
            default:        returnv[i] = Number_class; break;
        }
    }

    return to_return_value(returnv, returnc);
}

TValue disassemble(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    if (TYPE(argv[0]) != FUNC) call_error(new_type_error, "function <disassemble> take type <Function> as first argument");
    if (argc > 1 && TYPE(argv[1]) != STRING) call_error(new_type_error, "function <disassemble> take type <String> as second argument");

    TValue open_args[2] = { argv[1], make_fstring("w") };

    TValue func = argv[0];
    TValue file = argc > 1 ? open_IO(open_args, 2) : IO_stdout;

    TValue disassembled = full_disassemble(func);
    write_file((TValue[2]){ file, disassembled }, 2);

    return NIL;
}


int add_global(TValue value, TValue name) {
    TTable* glob_table = AS_TABLE(globals);
    int idx = glob_table->array.len;

    add_value_to_hashtable(globals, name, NUM_AS_TVALUE(idx));
    append_to_table(globals, value);

    return idx;
}

int make_global(TValue value, TValue name, TValue type) {
    add_value_to_hashtable(global_types, name, type);
    return add_global(value, name);
}

int make_global_function(TValue value, TValue name, TValue desc) {
    add_value_to_hashtable(global_functypes, name, desc);
    return make_global(value, name, FunctionType);
}

int overwrite_global(TValue value, TValue name) {
    int idx = get_value_from_hashtable(globals, name).num;
    AS_TABLE(globals)->array.array[idx] = value;

    return idx;
}

TValue get_global(TValue name) {
    return get_from_table(globals, get_value_from_hashtable(globals, name));
}

TValue make_globals_types_from_file(TValue filename) {
    Node* ast = easy_parse(AS_STRING(filename)->str);
    NodeArray statements = ast->node.StatementsNode;

    for (int i = 0; i < statements.len; i++) {
        Node* node = statements.nodes[i];

        switch (node->nt) {
            case NT_VAR_DECLARE: {
                add_value_to_hashtable(global_types, node->node.VarDeclareNode.var_name, node->type);
                break;
            }

            case NT_FUNCDEF: {
                int argc = get_linked_list_len(node->node.funcDefNode.arg_types);
                TValue arg_array[argc]; linked_list_to_array(node->node.funcDefNode.args, arg_array, argc);
                TValue argtype_array[argc]; linked_list_to_array(node->node.funcDefNode.arg_types, argtype_array, argc);

                TValue name = node->node.funcDefNode.name;
                TValue desc = make_method_descriptor(argtype_array, node->node.funcDefNode.type_specifier, argc);
                
                add_value_to_hashtable(global_types, name, node->type);
                add_value_to_hashtable(global_functypes, name, desc);
                break;
            }
        }
    }

    clean_node(ast);
}

