#include <time.h>
#include <signal.h>
#include "aad_cpu_avx_miner.h"

void handle_sigint(int sig) {
    (void)sig;
    printf("\n[Stopping...]\n");
    stop_signal = 1;
}

int main(void) {
    #ifndef __AVX__
    fprintf(stderr, "ERROR: AVX not available. Compile with -mavx\n");
    return 1;
    #endif
    
    signal(SIGINT, handle_sigint);
    
    mine_cpu_avx_coins();
    
    save_coin(NULL);
    return 0;
}
