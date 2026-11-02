#include "Debug/Debug.h"
#include <bs_api.h>

BS_API char* bytescript_module_map = "Debug_map.bs";
BS_API TValue bytescript_module_loader(TValue* argv, size_t argc) {
    TValue fields[] = {
        new_cfunc(assert_debug),
    };

    TValue AssertionError = new_class(&Error_class, (TValue[1]) { new_cfunc(new_assertion_error) }, 1, 1);
    overwrite_global(AssertionError, make_fstring("AssertionError"));

    size_t field_count = sizeof(fields) / sizeof(fields[0]);
    memcpy(argv, fields, BSIZE(field_count));
}


TValue new_assertion_error(TValue* argv, size_t argc) {
    STRICT_MIN_ARGC(1, "AssertionError.new");
    new_error((TValue[2]){ make_fstring("AssertionError"), argv[0] }, 2);
}

TValue assert_debug(TValue* argv, size_t argc) {
    MIN_ARGC(1); if (argc > 1) TYPE_CHECK(1, STRING, "String", "Debug.assert");

    if (FALSY(*argv)) {
        TValue err_message = make_fstring("Condition result : %s\n\t");
        err_message = format_string((TValue[2]){ err_message, to_string_string(argv, 1) }, 2);
        
        if (argc > 1) err_message = concat_string((TValue[2]){ err_message, argv[1] }, 2);
        call_error(new_assertion_error, AS_STRING(err_message)->str);
    }

    return TRUE_TVALUE;
}
