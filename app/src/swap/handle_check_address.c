
#include <stdint.h>

#include "handle_check_address.h"
#include "bip32_path.h"
#include "crypto.h"

static int os_strcmp(const char* s1, const char* s2) {
    size_t size = strlen(s1) + 1;
    return memcmp(s1, s2, size);
}

int handle_check_address(check_address_parameters_t* params) {
    PRINTF("Params on the address %d.\n", (unsigned int) params);
    PRINTF("Address to check %s.\n", params->address_to_check);
    PRINTF("Inside handle_check_address.\n");
    if (params->address_to_check == 0) {
        PRINTF("Address to check == 0.\n");
        return 0;
    }
    
    /** address_parameters :
     * | address kind (1 byte) | path length (1 byte) | bip44 path (5 bytes) | 
     */
    /* Extract address kind */
    key_kind_e kind = (key_kind_e) *((params->address_parameters)++);
    
    /* Parse serialized path (extract path and path length) */
    bip32_path_t path;
    if (!parse_serialized_path(&path,
                               params->address_parameters,
                               params->address_parameters_length - 1)) // param length - 1B for path length)
    {
        PRINTF("Can't parse path !\n");
        return 0;
    }

    /* Generate public key from derivation path */
    uint8_t pub_key[PK_LEN_25519] = {0};
    crypto_extractPublicKey(kind, path.path, pub_key, PK_LEN_25519);
    
    uint8_t addr[SS58_ADDRESS_MAX_LEN] = {0};
    size_t outLen = crypto_SS58EncodePubkey(addr,
                                            SS58_ADDRESS_MAX_LEN,
                                            PK_ADDRESS_TYPE, pub_key);
    if (outLen == 0) {
        MEMZERO(pub_key, PK_LEN_25519);
        MEMZERO(addr, SS58_ADDRESS_MAX_LEN);
        PRINTF("Cannot generate address !\n");
        return 0;
    }

    /* Compare generated address with parameter address */
    if (os_strcmp((const char *) addr, params->address_to_check) != 0) {
        PRINTF("Addresses don't match !\n");
        return 0;
    }
    
    PRINTF("Addresses match.\n");
    return 1;
}
