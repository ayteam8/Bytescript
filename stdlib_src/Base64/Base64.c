#include "Base64/Base64.h"
#include "Math/math_utils.h"

#include <stdio.h>
#include <math.h>
#include <bs_api.h> // nueva cosa añadida en false_windows

char base64_decode_table[256];
const char base64_encode_table[] = {
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/"
};

BS_API char* bytescript_module_map = "Base64_map.bs";
BS_API TValue bytescript_module_loader(TValue* argv, size_t argc) {
    TValue fields[] = {
        new_cfunc(encode_base64),
        new_cfunc(decode_base64),
    };

    for (int i = 0; i < sizeof(base64_encode_table); i++) base64_decode_table[base64_encode_table[i]] = i;

    size_t field_count = sizeof(fields) / sizeof(fields[0]);
    memcpy(argv, fields, BSIZE(field_count));
}


TValue encode_base64(TValue* argv, size_t argc) {
    MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "Base64.encode");

    TStr* str = AS_STRING(*argv);

    size_t mod = str->len % 3;
    size_t cuts = (size_t)floor(str->len / 3) + (mod != 0);
    size_t buff_len = MAX(cuts * 4, 4);
    char buffer[buff_len + 1];

    memset(buffer, '\0', sizeof(buffer));
    for (int i = 0; i < str->len; i += 3) {
        char pad1 = i + 1 < str->len ? str->str[i + 1] : 0;
        char pad2 = i + 2 < str->len ? str->str[i + 2] : 0;
        size_t idx = i / 3 * 4;

        buffer[idx + 0] = (str->str[i] & 0xfc) >> 2;
        buffer[idx + 1] = ((str->str[i] & 0x03) << 4) | ((pad1 & 0xf0) >> 4);
        buffer[idx + 2] = ((pad1 & 0x0f) << 2) | ((pad2 & 0xc0) >> 6);
        buffer[idx + 3] = pad2 & 0x3f;
    }

    for (int i = 0; i < buff_len; i++) buffer[i] = base64_encode_table[(uint8_t)buffer[i]];
    for (int i = 0; i < (mod ? 3 - mod : mod); i++) buffer[buff_len - i - 1] = '=';
    return make_fstring(buffer);
}

TValue decode_base64(TValue* argv, size_t argc) {
    MIN_ARGC(1); TYPE_CHECK(0, STRING, "String", "Base64.encode");

    TStr* str = AS_STRING(*argv);

    size_t padding = 0; for (int i = str->len - 1; str->str[i] == '='; i--) padding++;
    size_t cuts = str->len / 4;
    size_t buff_len = cuts * 3 - padding;
    char buffer[buff_len + 1];

    for (int i = 0; i < str->len; i += 4) {
        size_t idx = i / 4 * 3;
        size_t decode1 = base64_decode_table[str->str[i + 0]];
        size_t decode2 = base64_decode_table[str->str[i + 1]];
        size_t decode3 = str->str[i + 2] == '=' ? '\0' : base64_decode_table[str->str[i + 2]];
        size_t decode4 = str->str[i + 3] == '=' ? '\0' : base64_decode_table[str->str[i + 3]];

        buffer[idx + 0] = (decode1 << 2) | (decode2 >> 4);
        buffer[idx + 1] = (decode2 << 4) | (decode3 >> 2);
        buffer[idx + 2] = (decode3 << 6) | decode4;
    }

    buffer[buff_len] = '\0';
    return make_fstring(buffer);
}
