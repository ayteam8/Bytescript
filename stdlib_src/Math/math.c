#include <math.h>
#include <bs_api.h>

#include "Math/Math.h"
#include "src/Globals/globals.h"

BS_API char* bytescript_module_map = "math_map.bs";
BS_API TValue bytescript_module_loader(TValue* argv, size_t argc) {
    TValue fields[] = {
        pi_math,
        e_math,
        inf_math,
        new_cfunc(max_math),
        new_cfunc(min_math),
        new_cfunc(clamp_math),
        new_cfunc(abs_math),
        new_cfunc(floor_math),
        new_cfunc(ceil_math),
        new_cfunc(round_math),
        new_cfunc(trunc_math),
        new_cfunc(sqrt_math),
        new_cfunc(sin_math),
        new_cfunc(cos_math),
        new_cfunc(tan_math),
        new_cfunc(asin_math),
        new_cfunc(acos_math),
        new_cfunc(atan_math),
        new_cfunc(atan2_math),
        new_cfunc(exp_math),
        new_cfunc(log_math),
        new_cfunc(log10_math),
        new_cfunc(sign_math),
        new_cfunc(is_inf_math),
        new_cfunc(deg_math),
        new_cfunc(rad_math),
    };

    size_t field_count = sizeof(fields) / sizeof(fields[0]);
    memcpy(argv, fields, BSIZE(field_count));
}


TValue pi_math = NUM_AS_TVALUE(3.141592653589793238462643383279502884);
TValue e_math = NUM_AS_TVALUE(2.71828182845904523536);
TValue inf_math = NUM_AS_TVALUE(INFINITY);

TValue max_math(TValue* argv, size_t argc) {
    MIN_ARGC(2);
    return NUM_AS_TVALUE(MAX(argv[0].num, argv[1].num));
}

TValue min_math(TValue* argv, size_t argc) {
    MIN_ARGC(2);
    return NUM_AS_TVALUE(MIN(argv[0].num, argv[1].num));
}

TValue clamp_math(TValue* argv, size_t argc) {
    MIN_ARGC(3);
    return NUM_AS_TVALUE(CLAMP(argv[0].num, argv[1].num, argv[2].num));
}

TValue abs_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(abs(argv[0].num));
}

TValue floor_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(floor(argv[0].num));
}

TValue ceil_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(ceil(argv[0].num));
}

TValue round_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(round(argv[0].num));
}

TValue trunc_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(trunc(argv[0].num));
}

TValue sqrt_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(sqrt(argv[0].num));
}

TValue sin_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(sin(argv[0].num));
}

TValue cos_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(cos(argv[0].num));
}

TValue tan_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(tan(argv[0].num));
}

TValue asin_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(asin(argv[0].num));
}

TValue acos_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(acos(argv[0].num));
}

TValue atan_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(atan(argv[0].num));
}

TValue atan2_math(TValue* argv, size_t argc) {
    MIN_ARGC(2);
    return NUM_AS_TVALUE(atan2(argv[0].num, argv[1].num));
}

TValue exp_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(exp(argv[0].num));
}

TValue log_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(log(argv[0].num));
}

TValue log10_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(log10(argv[0].num));
}

TValue sign_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE((argv[0].num > 0) - (argv[0].num < 0));
}

TValue is_inf_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(EQUAL(inf_math, argv[0]));
}

TValue deg_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(argv[0].num * (180 / pi_math.num));
}

TValue rad_math(TValue* argv, size_t argc) {
    MIN_ARGC(1);
    return NUM_AS_TVALUE(argv[0].num * (pi_math.num / 180));
}
