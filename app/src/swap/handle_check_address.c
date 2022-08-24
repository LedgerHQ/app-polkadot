#include "handle_check_address.h"

int handle_check_address(check_address_parameters_t* params) {
    PRINTF("Params on the address %d\n", (unsigned int) params);
    PRINTF("Address to check %s\n", params->address_to_check);
    PRINTF("Inside handle_check_address\n");
    if (params->address_to_check == 0) {
        PRINTF("Address to check == 0\n");
        return 0;
    }

    // uint8_t i;
    // const uint8_t* bip32_path_ptr = params->address_parameters;
    // uint8_t bip32PathLength = *(bip32_path_ptr++);
    // cx_sha3_t local_sha3;

    // if (strcmp(locals_union1.address, params->address_to_check + offset_0x) != 0) {
    //     PRINTF("Addresses don't match\n");
    //     return 0;
    // }
    // PRINTF("Addresses  match\n");
    return 1;
}
