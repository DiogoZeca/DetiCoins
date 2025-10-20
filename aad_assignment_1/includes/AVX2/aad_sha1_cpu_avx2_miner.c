#include <time.h>
#include <signal.h>
#include "aad_cpu_avx2_miner.h"

void handle_sigint(int sig) {
    (void)sig;
    printf("\n[Stopping...]\n");
    stop_signal = 1;
}

int main(void) {
    #ifndef __AVX2__
    fprintf(stderr, "ERROR: AVX2 not available. Compile with -mavx2\n");
    return 1;
    #endif
    
    signal(SIGINT, handle_sigint);
    
    mine_cpu_avx2_coins();
    
    save_coin(NULL);
    return 0;
}
