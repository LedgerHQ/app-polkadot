#include "handle_swap_sign_transaction.h"
#include "swap_globals.h"
#include "swap_lib_calls.h"
#include "app_main.h"
#include "app_mode.h"

static uint64_t read_u64_be(const uint8_t *ptr, size_t offset) {
    return (uint64_t) ptr[offset + 0] << 56 |  //
           (uint64_t) ptr[offset + 1] << 48 |  //
           (uint64_t) ptr[offset + 2] << 40 |  //
           (uint64_t) ptr[offset + 3] << 32 |  //
           (uint64_t) ptr[offset + 4] << 24 |  //
           (uint64_t) ptr[offset + 5] << 16 |  //
           (uint64_t) ptr[offset + 6] << 8 |   //
           (uint64_t) ptr[offset + 7] << 0;
}

bool copy_transaction_parameters(create_transaction_parameters_t* sign_transaction_params) {
    // First copy parameters to stack, and then to global data.
    // We need this "trick" as the input data position can overlap with globals
    char destination_address[50];
    // TODO : Fees field useless for DOT transaction as it is not included in
    // transaction data ? 
    uint64_t fees;
    MEMZERO(destination_address, sizeof(destination_address));
    MEMZERO(&fees, sizeof(fees));
    strncpy(destination_address,
            sign_transaction_params->destination_address,
            sizeof(destination_address) - 1);

    // Sanity checks.
    if ((destination_address[sizeof(destination_address) - 1] != '\0') ||
        (sign_transaction_params->amount_length > 16 )) // Balance type is u128.
    {
        return false;
    }
    
    // Copy parameters to globals.
    G_swap_state.fees = read_u64_be((uint8_t*)&fees, 0);
    G_swap_state.amount_length = sign_transaction_params->amount_length;
    MEMZERO(G_swap_state.amount,sizeof(G_swap_state.amount));
    memcpy(G_swap_state.amount,sign_transaction_params->amount,sign_transaction_params->amount_length);
    memcpy(G_swap_state.destination_address,destination_address,sizeof(G_swap_state.destination_address));
    
    return true;
}

void handle_swap_sign_transaction(void) {
    G_swap_state.called_from_swap = 1;
    
    // TODO : check what needs to be reset between calls.
    //reset_transaction_context();
    app_mode_set_secret(false);
    io_seproxyhal_init();
    UX_INIT();
    USB_power(0);
    USB_power(1);
    PRINTF("USB power ON/OFF\n");
#ifdef TARGET_NANOX
    // grab the current plane mode setting
    G_io_app.plane_mode = os_setting_get(OS_SETTING_PLANEMODE, NULL, 0);
#endif  // TARGET_NANOX
#ifdef HAVE_BLE
    BLE_power(0, NULL);
    BLE_power(1, "Nano X");
#endif  // HAVE_BLE
    PRINTF("[DOT] Calling main app now.\n");
    app_main();
}
