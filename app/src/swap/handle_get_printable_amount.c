#include "handle_get_printable_amount.h"
#include "swap_lib_calls.h"

int handle_get_printable_amount(get_printable_amount_parameters_t* params) {
    MEMZERO(params->printable_amount, sizeof(params->printable_amount));

    /* Convert byte array amount to printable amount string.*/
    if(!bytes_amount_to_print_str((char*)params->amount, params->amount_length, params->printable_amount, sizeof(params->printable_amount)))
    {
        return 0;
    }
    
    PRINTF("Printable amount is : %s\n",params->printable_amount);

    return 1;
}

