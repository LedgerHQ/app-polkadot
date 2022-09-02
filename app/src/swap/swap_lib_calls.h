#ifndef _SWAP_LIB_CALLS_H_
#define _SWAP_LIB_CALLS_H_

#include "stdbool.h"
#include <zxformat.h>

// Swap commands macros
// --------------------
#define RUN_APPLICATION 1
#define SIGN_TRANSACTION 2
#define CHECK_ADDRESS 3
#define GET_PRINTABLE_AMOUNT 4
// --------------------

// Max printable amount string size
#define MAX_PRINTABLE_AMOUNT_SIZE 50


// structure that should be send to specific coin application to get address
typedef struct check_address_parameters_s {
    // IN
    unsigned char* coin_configuration;
    unsigned char coin_configuration_length;
    // serialized path, segwit, version prefix, hash used, dictionary etc.
    // fields and serialization format depends on spesific coin app
    unsigned char* address_parameters;
    unsigned char address_parameters_length;
    char* address_to_check;
    char* extra_id_to_check;
    // OUT
    int result;
} check_address_parameters_t;

// structure that should be send to specific coin application to get printable amount
typedef struct get_printable_amount_parameters_s {
    // IN
    unsigned char* coin_configuration;
    unsigned char coin_configuration_length;
    unsigned char* amount;
    unsigned char amount_length;
    bool is_fee;
    // OUT
    char printable_amount[MAX_PRINTABLE_AMOUNT_SIZE];
    // int result;
} get_printable_amount_parameters_t;

typedef struct create_transaction_parameters_s {
    unsigned char* coin_configuration;
    unsigned char coin_configuration_length;
    unsigned char* amount;
    unsigned char amount_length;
    unsigned char* fee_amount;
    unsigned char fee_amount_length;
    char* destination_address;
    char* destination_address_extra_id;
} create_transaction_parameters_t;

// TODO : remove unused swap_str_to_u64
bool swap_str_to_u64(const uint8_t* src, size_t length, uint64_t* result);
int bytes_amount_to_print_str(char *amount, size_t amount_len, char *out, size_t out_len);


#endif  // _SWAP_LIB_CALLS_H_
