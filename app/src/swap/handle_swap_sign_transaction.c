#include "handle_swap_sign_transaction.h"
#include "swap_globals.h"
#include "swap_lib_calls.h"
#include "app_main.h"

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
    char amount[MAX_PRINTABLE_AMOUNT_SIZE];
    // TODO : Fees field useless for DOT transaction as it is not included in
    // transaction data ? 
    uint64_t fees;
    MEMZERO(destination_address, sizeof(destination_address));
    MEMZERO(amount, sizeof(amount));
    MEMZERO(&fees, sizeof(fees));
    strncpy(destination_address,
            sign_transaction_params->destination_address,
            sizeof(destination_address) - 1);

    // Sanity checks.
    if ((destination_address[sizeof(destination_address) - 1] != '\0') ||
        (sign_transaction_params->amount_length > 39 )) // Balance type is u128 as per the doc (so max number has 39 digits).
    {
        return false;
    }
    
    // Store amount as printable string
    bytes_amount_to_print_str((char*)sign_transaction_params->amount,sign_transaction_params->amount_length,amount,sizeof(amount));
    
    // Copy parameters to globals.
    G_swap_state.fees = read_u64_be((uint8_t*)&fees, 0);
    memcpy(G_swap_state.amount,
           amount,
           sizeof(G_swap_state.amount));
    memcpy(G_swap_state.destination_address,
           destination_address,
           sizeof(G_swap_state.destination_address));
    return true;
}

void handle_swap_sign_transaction(void) {
    G_swap_state.called_from_swap = 1;
    
    // TODO : check what needs to be reset between calls.
    //reset_transaction_context();
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
