#include <stdint.h>
#include <string.h>

#include <zxmacros.h>
#include "handle_check_address.h"
#include "bip32_path.h"
#include "substrate_coin.h"
#include "os.h"
#include "os_seed.h"

// static int os_strcmp(const char* s1, const char* s2) {
//     size_t size = strlen(s1) + 1;
//     return memcmp(s1, s2, size);
// }

int handle_check_address(check_address_parameters_t* params) {
    PRINTF("Params on the address %d\n", (unsigned int) params);
    PRINTF("Address to check %s\n", params->address_to_check);
    PRINTF("Inside handle_check_address\n");
    if (params->address_to_check == 0) {
        PRINTF("Address to check == 0\n");
        return 0;
    }

    /** address_parameters :
     * | address kind (1 byte) | path length (1 byte) | bip44 path (5 bytes) | 
     */
    /* Extract address kind */
    //key_kind_e kind = (key_kind_e) *((params->address_parameters)++);
    
    /* Parse serialized path (extract path and path length) */
    bip32_path_t path;
    path.length = MAX_BIP32_PATH;
    MEMZERO(path.path,sizeof(path.path));    
    
    // if (!parse_serialized_path(&path,
    //                            params->address_parameters,
    //                            params->address_parameters_length - 1)) // param length - 1B for path length)
    // {
    //     PRINTF("Can't parse path !\n");
    //     return 0;
    // }

    /* Generate public key from derivation path */
    uint8_t pub_key[PK_LEN_25519] = {0};
    //crypto_extractPublicKey(0, path.path, pub_key, PK_LEN_25519);

    cx_ecfp_public_key_t cx_publicKey;
    cx_ecfp_private_key_t cx_privateKey;
    uint8_t privateKeyData[SK_LEN_25519];

    // Generate keys
    /* ==============================================================================
     * ==============================================================================
     * ==============================================================================
     * ==============================================================================
     * Calling os_perso_derive_node_bip32 and then cx_ecfp_generate_pair causes a segmentation fault in
     * exchange app when coming back from the libcall and entering the apdu
     * receiving loop of nanos-secure-sdk/src/os_io_seproxyhal.c at line 1217 (nanos sdk v2.1.0)
     * ==============================================================================
     * ==============================================================================
     * ==============================================================================
     * ==============================================================================
     * ==============================================================================
     */
    os_perso_derive_node_bip32(
            CX_CURVE_Ed25519,
            path.path,
            HDPATH_LEN_DEFAULT,
            privateKeyData,
            NULL);

    cx_ecfp_init_private_key(CX_CURVE_Ed25519, privateKeyData, 32, &cx_privateKey);
    cx_ecfp_init_public_key(CX_CURVE_Ed25519, NULL, 0, &cx_publicKey);
    cx_ecfp_generate_pair(CX_CURVE_Ed25519, &cx_publicKey, &cx_privateKey, 1);
    for (unsigned int i = 0; i < PK_LEN_25519; i++) {
        pub_key[i] = cx_publicKey.W[64 - i];
    }

    if ((cx_publicKey.W[PK_LEN_25519] & 1) != 0) {
        pub_key[31] |= 0x80;
    }

    // uint8_t addr[SS58_ADDRESS_MAX_LEN] = {0};
    // size_t outLen = crypto_SS58EncodePubkey(addr,
    //                                         SS58_ADDRESS_MAX_LEN,
    //                                         PK_ADDRESS_TYPE, pub_key);
    // if (outLen == 0) {
    //     MEMZERO(pub_key, PK_LEN_25519);
    //     MEMZERO(addr, SS58_ADDRESS_MAX_LEN);
    //     PRINTF("Cannot generate address !\n");
    //     return 0;
    // }

    /* Compare generated address with parameter address */
    // if (os_strcmp((const char *) addr, params->address_to_check) != 0) {
    //     PRINTF("Addresses don't match !\n");
    //     //return 0;
    // }
    
    PRINTF("Addresses match.\n");
    return 1;
}
