#include <stdio.h>
#include <signal.h>
#include "aad_cuda_miner.h"

void handle_sigint(int sig) {
    (void)sig;
    printf("\n[Stopping GPU mining...]\n");
    stop_signal = 1;
}

int main(void) {
    signal(SIGINT, handle_sigint);
    mine_cuda_coins();
    save_coin(NULL);
    return 0;
}
