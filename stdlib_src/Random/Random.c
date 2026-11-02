#include <sys/time.h>
#include <bs_api.h>

#include "Random/Random.h"


BS_API char* bytescript_module_map = "Random_map.bs";
BS_API TValue bytescript_module_loader(TValue* argv, size_t argc) {
    TValue fields[] = {
        new_cfunc(seed_random),
        new_cfunc(random_random),
        new_cfunc(range_random),
        new_cfunc(integer_random)
    };

    seed_random(NULL, 0);
    size_t field_count = sizeof(fields) / sizeof(fields[0]);
    memcpy(argv, fields, BSIZE(field_count));
}


TValue seed_random(TValue* argv, size_t argc) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec ^ tv.tv_usec);
}

TValue random_random(TValue* argv, size_t argc) {
    return NUM_AS_TVALUE(rand() / (double)RAND_MAX);
}

TValue range_random(TValue* argv, size_t argc) {
    MIN_ARGC(2);
    return NUM_AS_TVALUE( argv[0].num + (rand() / (double)RAND_MAX) * (argv[1].num - argv[0].num) );
}

TValue integer_random(TValue* argv, size_t argc) {
    MIN_ARGC(2);
    return NUM_AS_TVALUE( (int64_t)( argv[0].num + (rand() / (double)RAND_MAX) * (argv[1].num - argv[0].num) ));
}
