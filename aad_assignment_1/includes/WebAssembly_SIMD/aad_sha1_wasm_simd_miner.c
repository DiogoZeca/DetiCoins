//
// Arquiteturas de Alto Desempenho 2025/2026
//
// WebAssembly SIMD DETI Coin Miner - Emscripten Entry Point
//

#include <emscripten.h>
#include "aad_wasm_simd_miner.h"

EMSCRIPTEN_KEEPALIVE
void miner_init(uint32_t seed_low, uint32_t seed_high)
{
    g_counter = ((u64_t)seed_high << 32) | seed_low;
    g_total_hashes = 0;
    g_coins_found = 0;
    g_coin_buffer_count = 0;
    g_running = 1;
}

EMSCRIPTEN_KEEPALIVE
void miner_start(void) { g_running = 1; }

EMSCRIPTEN_KEEPALIVE
void miner_stop(void) { g_running = 0; }

EMSCRIPTEN_KEEPALIVE
int miner_is_running(void) { return g_running; }

EMSCRIPTEN_KEEPALIVE
int miner_mine_batch(uint32_t batch_size, uint32_t seed)
{
    if (!g_running) return 0;
    return mine_batch_simd(batch_size, seed);
}

EMSCRIPTEN_KEEPALIVE
uint32_t miner_get_hashes_low(void) { return (uint32_t)(g_total_hashes & 0xFFFFFFFFu); }

EMSCRIPTEN_KEEPALIVE
uint32_t miner_get_hashes_high(void) { return (uint32_t)((g_total_hashes >> 32) & 0xFFFFFFFFu); }

EMSCRIPTEN_KEEPALIVE
int miner_get_coins_found(void) { return g_coins_found; }

EMSCRIPTEN_KEEPALIVE
int miner_get_coin_buffer_count(void) { return g_coin_buffer_count; }

EMSCRIPTEN_KEEPALIVE
uint32_t* miner_get_coin_buffer(void) { return g_coin_buffer; }

EMSCRIPTEN_KEEPALIVE
void miner_clear_coin_buffer(void) { g_coin_buffer_count = 0; }

EMSCRIPTEN_KEEPALIVE
uint32_t miner_get_counter_low(void) { return (uint32_t)(g_counter & 0xFFFFFFFFu); }

EMSCRIPTEN_KEEPALIVE
uint32_t miner_get_counter_high(void) { return (uint32_t)((g_counter >> 32) & 0xFFFFFFFFu); }
