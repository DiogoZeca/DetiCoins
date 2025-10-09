//
// aad_cpu_miner.h
// CPU scalar DETI coin miner (compatible with assignment reference code)
// Writes coins using the same byte layout expected by aad_vault.h
//
// Place this file in: includes/CPU/aad_cpu_miner.h
//

#ifndef AAD_CPU_MINER_H
#define AAD_CPU_MINER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>


#include "../aad_data_types.h"   // u08_t, u32_t
#include "../aad_sha1_cpu.h"     // sha1()
#include "../aad_vault.h"        // save_coin()

/*
 * generate_coin()
 *
 * Fills coin[14] so that:
 *  - bytes 0..11  == "DETI coin 2 "   (12 bytes, last is space)
 *  - bytes 12..53 == printable arbitrary bytes (must NOT contain '\n')
 *  - byte  54     == '\n'
 *  - byte  55     == 0x80 (SHA1 padding layout expected by project's sha1 macro)
 *
 * IMPORTANT: bytes are written using the coin-byte mapping expected by the
 * reference implementation: ((u08_t*)coin)[i ^ 3]  (see aad_vault.h usage).
 */
static void generate_coin(u32_t coin[14])
{
  /* template prefix (12 bytes) */
  const char prefix[12] = { 'D','E','T','I',' ','c','o','i','n',' ','2',' ' };

  /* Fill a small temporary 55-byte buffer (0..54) safely */
  u08_t buf[55];

  /* Set prefix 0..11 */
  for (int i = 0; i < 12; ++i) buf[i] = (u08_t)prefix[i];

  /* Fill bytes 12..53 with printable ASCII characters (32..126), avoiding '\n' (0x0A) */
  for (int i = 12; i <= 53; ++i)
  {
    /* pick a printable char: space..tilde (32..126) */
    int r = (rand() % 95) + 32; /* 95 chars from 32..126 inclusive */

    /* If random happens to be newline (0x0A) — avoid it (but it won't from 32..126) */
    if (r == 0x0A) r = 'X';
    buf[i] = (u08_t)r;
  }

  /* Ensure last message byte (index 54) is newline '\n' */
  buf[54] = (u08_t)'\n';

  /* Now store the 55 message bytes into the coin buffer using the ^3 byte mapping */
  for (int i = 0; i < 55; ++i)
    ((u08_t *)coin)[i ^ 3] = buf[i];

  /* Set the SHA1 padding byte at index 55 as required by the project's SHA1 macro */
  ((u08_t *)coin)[55 ^ 3] = 0x80;
}

/*
 * mine_cpu_coins()
 *
 * Loop generating coins, computing SHA1, testing signature,
 * and exiting after the first valid DETI coin is found.
 */
static void mine_cpu_coins(void)
{
  const unsigned long long REPORT_INTERVAL_ATTEMPTS = 1000000ULL;

  /* Initialize PRNG with time-based seed */
  srand((unsigned)time(NULL) ^ (unsigned)(uintptr_t)&REPORT_INTERVAL_ATTEMPTS);

  u32_t coin[14];
  u32_t hash[5];
  unsigned long long attempts = 0ULL;

  time_t start_time = time(NULL);
  printf("Starting CPU mining loop...\n");

  while (1)
  {
    generate_coin(coin);

    /* compute SHA1 over coin buffer (project's sha1 expects this layout) */
    sha1(coin, hash);

    ++attempts;

    if (hash[0] == 0xAAD20250u)
    {
      double elapsed = difftime(time(NULL), start_time);

      printf("\n====================================\n");
      printf("💰 FOUND DETI COIN! 💰\n");
      printf("Attempts: %llu\n", attempts);
      printf("Time elapsed: %.2f seconds\n", elapsed);
      printf("SHA1: %08X %08X %08X %08X %08X\n",
             hash[0], hash[1], hash[2], hash[3], hash[4]);
      printf("====================================\n\n");

      /* Save coin to vault buffer and flush to disk immediately */
      save_coin(coin);  /* add to internal buffer */
      save_coin(NULL);  /* flush buffer to VAULT_FILE_NAME */

      /* exit program (clean and simple) */
      exit(0);
    }

    if ((attempts % REPORT_INTERVAL_ATTEMPTS) == 0ULL)
    {
      double elapsed = difftime(time(NULL), start_time);
      double rate = elapsed > 0.0 ? (double)attempts / elapsed : 0.0;
      printf("[%.1fs] Attempts: %llu (%.2f attempts/s)\n",
             elapsed, attempts, rate);
      fflush(stdout);
    }
  }

  /* never reached, but keep for completeness */
  save_coin(NULL);
}

#endif /* AAD_CPU_MINER_H */
