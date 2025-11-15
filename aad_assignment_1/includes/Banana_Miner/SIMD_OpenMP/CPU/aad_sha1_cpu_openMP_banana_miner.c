#include <signal.h>
#include <stdio.h>
#include "aad_cpu_openMP_banana_miner.h"

void handle_sigint(int sig) {
  (void)sig;
  printf("\n[Stopping all threads...]\n");
  stop_signal = 1;
}

int main(void) {
  signal(SIGINT, handle_sigint);
  mine_cpu_banana_coins_openmp();
  save_coin(NULL);
  return 0;
}
