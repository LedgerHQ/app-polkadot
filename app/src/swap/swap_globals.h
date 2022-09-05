#ifndef _SWAP_GLOBALS_H_
#define _SWAP_GLOBALS_H_

#include <stdint.h>
#include "swap_lib_calls.h"

typedef struct swap_globals_s {
    uint8_t amount[16];
    uint8_t amount_length;
    uint64_t fees;
    char destination_address[65];
    /*Is swap mode*/
    unsigned char called_from_swap;
    unsigned char should_exit;
} swap_globals_t;

extern swap_globals_t G_swap_state;

#endif //_SWAP_GLOBALS_H_
