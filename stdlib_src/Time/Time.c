#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <bs_api.h>

#include "Time/Time.h"


double get_time(void)
{
	struct timespec ts;

	timespec_get(&ts, TIME_UTC);

	return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

BS_API char* bytescript_module_map = "Time_map.bs";
BS_API TValue bytescript_module_loader(TValue* argv, size_t argc) {
    TValue fields[] = {
        new_cfunc(time_time),
        new_cfunc(clock_time),
        new_cfunc(sleep_time)
    };

    size_t field_count = sizeof(fields) / sizeof(fields[0]);
    memcpy(argv, fields, BSIZE(field_count));
}


TValue time_time(TValue* argv, size_t argc) {
    return NUM_AS_TVALUE(time(NULL));
}

TValue clock_time(TValue* argv, size_t argc) {
    /*LARGE_INTEGER tick;
    LARGE_INTEGER freq;
    QueryPerformanceCounter(&tick);
    QueryPerformanceFrequency(&freq);
    */
    return NUM_AS_TVALUE(get_time());
}

TValue sleep_time(TValue* argv, size_t argc) {
    if (argc > 0) NUM_CHECK(0, "Time.sleep");
    Sleep(argc > 0 ? argv[0].num : 1000);

    return NIL;
}
