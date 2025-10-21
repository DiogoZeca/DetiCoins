#include <signal.h>
#include <stdio.h>
#include "aad_cpu_avx512_miner.h"

void handle_sigint(int sig) {
    (void)sig;
    printf("\n[Stopping...]\n");
    stop_signal = 1;
}

int main(void) {
    signal(SIGINT, handle_sigint);
    mine_cpu_avx512_coins();
    save_coin(NULL);
    return 0;
}