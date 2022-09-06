/*******************************************************************************
*   (c) 2016 Ledger
*   (c) 2018, 2019 Zondax GmbH
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/
#include "app_main.h"
#include "view.h"

#include <os_io_seproxyhal.h>

#include "swap/swap_lib_calls.h"

#include "swap/handle_swap_sign_transaction.h"
#include "swap/handle_get_printable_amount.h"
#include "swap/handle_check_address.h"

/* Arguments structure when application is started with libcall. */
struct libargs_s {
    unsigned int id;
    unsigned int command;
    /* Unused config field */
    void * chain_config;
    union {
        check_address_parameters_t *check_address;
        create_transaction_parameters_t *create_transaction;
        get_printable_amount_parameters_t *get_printable_amount;
    };
};

/* Private functions prototypes */
static void app_start(void);
static void app_exit(void);
static void swap_library_main(struct libargs_s *args);
static void swap_library_main_helper(struct libargs_s *args);

__attribute__((section(".boot"))) int
main(int arg0) {
    // exit critical section
    __asm volatile("cpsie i");

    os_boot();

    if (!arg0) {
        // called from dashboard as standalone app
        app_start();
        return 0;
    }

    struct libargs_s *args = (struct libargs_s *) arg0;
    if (args->id != 0x100) {
        app_exit();
        return 0;
    }

    switch (args->command) {
        // This case would not happen with polkadot ?
        case RUN_APPLICATION:
            app_start();
            break;
        default:
            // Called as library
            swap_library_main(args);
            break;
    }
}

/**
 * @brief Start the application (when called from dashboard or libcall with RUN_APPLICATION).
 */
static void app_start(void) {
    BEGIN_TRY
    {
        TRY
        {
            view_init();
            app_init();
            app_main();
        }
        CATCH_OTHER(e)
        {}
        FINALLY
        {}
    }
    END_TRY;
}

/**
 * @brief Exit the application and go back to the dashboard.
 */
static void app_exit(void) {
    BEGIN_TRY_L(exit) {
        TRY_L(exit) {
            os_sched_exit(-1);
        }
        FINALLY_L(exit) {
        }
    }
    END_TRY_L(exit);
}

static void swap_library_main(struct libargs_s *args) {
    bool end = false;
    /* This loop ensures that swap_library_main_helper and os_lib_end are called
     * within a try context, even if an exception is thrown */
    while (1) {
        BEGIN_TRY {
            TRY {
                if (!end) {
                    swap_library_main_helper(args);
                }
                os_lib_end();
            }
            FINALLY {
                end = true;
            }
        }
        END_TRY;
    }
}

static void swap_library_main_helper(struct libargs_s *args) {
    check_api_level(CX_COMPAT_APILEVEL);
    PRINTF("Inside a library \n");
    switch (args->command) {
        case CHECK_ADDRESS:
            // ensure result is zero if an exception is thrown
            args->check_address->result = 0;
            args->check_address->result = handle_check_address(args->check_address);
            break;
        case SIGN_TRANSACTION:
            if (copy_transaction_parameters(args->create_transaction)) {
                // never returns
                handle_swap_sign_transaction();
            }
            break;
        case GET_PRINTABLE_AMOUNT:
            // ensure result is zero if an exception is thrown (compatibility breaking, disabled
            // until LL is ready)
            //args->get_printable_amount->result = 0;
            //args->get_printable_amount->result = 
            handle_get_printable_amount(args->get_printable_amount);
            break;
        default:
            break;
    }
}