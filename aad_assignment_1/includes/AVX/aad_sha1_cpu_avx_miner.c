#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "aad_cpu_avx_miner.h"

void handle_sigint(int sig) {
    (void)sig;
    printf("\n[Stopping...]\n");
    stop_signal = 1;
}

int main(int argc, char *argv[]) {
    coin_config_t config;

    // Parse command-line arguments
    if (argc >= 2) {
        // Custom coin with user-provided text
        const char *custom_text = argv[1];

        if (!validate_custom_text(custom_text)) {
            fprintf(stderr, "Error: Invalid custom text '%s'\n", custom_text);
            fprintf(stderr, "  - Must be 1-27 characters\n");
            fprintf(stderr, "  - Cannot contain newline characters\n");
            fprintf(stderr, "\nUsage: %s [custom_text]\n", argv[0]);
            fprintf(stderr, "  No arguments: mine standard DETI coins\n");
            fprintf(stderr, "  With text:    mine custom coins with embedded text\n");
            return EXIT_FAILURE;
        }

        config = coin_config_init(COIN_TYPE_CUSTOM, custom_text);
    } else {
        // Default: standard DETI coins
        config = coin_config_init(COIN_TYPE_DETI, NULL);
    }

    signal(SIGINT, handle_sigint);
    mine_cpu_avx_coins(&config);
    save_coin(NULL);
    return 0;
}
