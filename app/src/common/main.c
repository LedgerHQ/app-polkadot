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

__attribute__((section(".boot"))) int
main(int arg0) {
    // exit critical section
    __asm volatile("cpsie i");

    view_init();
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
            // Todo : implement call to swap feature.
            //library_main(args);
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