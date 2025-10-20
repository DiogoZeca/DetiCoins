#include <time.h>
#include "aad_cpu_miner.h"

void handle_sigint(int sig) {
    (void)sig;
    printf("\n[Stopping...]\n");
    stop_signal = 1;
}

int main(void) {
    signal(SIGINT, handle_sigint);
    srand((unsigned int)time(NULL));
    printf("Single-threaded scalar miner. Ctrl+C to stop.\n");
    
    mine_cpu_coins();
    
    save_coin(NULL);
    return 0;
}
