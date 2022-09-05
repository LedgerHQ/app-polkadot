#include <string.h>
#include <zxmacros.h>
#include "coin.h"
#include "swap_lib_calls.h"

#define INT128_LENGTH 16 // 128 / 8 = 16 bytes

static __attribute__((no_instrument_function)) inline int allzeroes(void *buf, size_t n) {
    uint8_t *p = (uint8_t *) buf;
    for (size_t i = 0; i < n; ++i) {
        if (p[i]) {
            return 0;
        }
    }
    return 1;
}

static bool uint128_to_decimal(const uint8_t *value, size_t value_len, char *out, size_t out_len) {
    if (value_len > INT128_LENGTH) {
        // value len is bigger than INT128_LENGTH ?!
        return false;
    }

    uint16_t n[8] = {0};
    // Copy and right-align the number
    memcpy((uint8_t *) n + INT128_LENGTH - value_len, value, value_len);

    // Special case when value is 0
    if (allzeroes(n, INT128_LENGTH)) {
        if (out_len < 2) {
            // Not enough space to hold "0" and \0.
            return false;
        }
        strncat(out, "0", out_len);
        return true;
    }

    uint16_t *p = n;
    for (int i = 0; i < 8; i++) {
        n[i] = __builtin_bswap16(*p++);
    }
    int pos = out_len;
    while (!allzeroes(n, sizeof(n))) {
        if (pos == 0) {
            return false;
        }
        pos -= 1;
        unsigned int carry = 0;
        for (int i = 0; i < 8; i++) {
            int rem = ((carry << 16) | n[i]) % 10;
            n[i] = ((carry << 16) | n[i]) / 10;
            carry = rem;
        }
        out[pos] = '0' + carry;
    }
    memmove(out, out + pos, out_len - pos);
    out[out_len - pos] = 0;
    return true;
}

int bytes_amount_to_print_str(char *amount, size_t amount_len, char *out, size_t out_len){
    
    //Convert byte array (up to 128bits/16bytes) to decimal string
    if (!uint128_to_decimal((uint8_t*)amount, amount_len, out, out_len)) {
        return 0;
    }

    // Format number.
    if (!intstr_to_fpstr_inplace(out, out_len, COIN_AMOUNT_DECIMAL_PLACES)) {
        return 0;
    }

    // Add ticker prefix.
    if (z_str3join(out, out_len, COIN_TICKER, "") != zxerr_ok) {
        return 0;
    }

    return 1;
}

bool swap_str_to_u64(const uint8_t* src, size_t length, uint64_t* result) {
    const size_t num_bytes = 8;
    uint8_t buffer[8];

    if (length > sizeof(buffer)) {
        return false;
    }

    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer + sizeof(buffer) - length, src, length);

    uint64_t value = 0;
    for (uint8_t i = 0; i < num_bytes; ++i) {
        PRINTF("BUFFER %d : %x -- shift : %d\n",i,buffer[i],num_bytes * 8u - i * 8u - 8u);
        value |= (uint64_t) buffer[i] << (num_bytes * 8u - i * 8u - 8u);
    }

    *result = value;

    return true;
}