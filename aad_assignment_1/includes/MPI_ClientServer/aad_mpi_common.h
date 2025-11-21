//
// Arquiteturas de Alto Desempenho 2025/2026
//
// MPI Client/Server DETI Coin Miner - Common Definitions
//

#ifndef AAD_MPI_COMMON_H
#define AAD_MPI_COMMON_H

#include <mpi.h>
#include <stdint.h>

typedef uint8_t  u08_t;
typedef uint32_t u32_t;
typedef uint64_t u64_t;

#define MPI_MASTER_RANK 0

#define TAG_WORK_ASSIGN   1
#define TAG_COIN_FOUND    2
#define TAG_REQUEST_WORK  3
#define TAG_STATS_UPDATE  4
#define TAG_STOP          5
#define TAG_WORKER_DONE   6

#define WORK_CHUNK_SIZE   100000000ULL
#define COIN_DATA_SIZE    14

typedef struct {
    u64_t start_counter;
    u64_t end_counter;
} work_range_t;

typedef struct {
    u32_t coin_data[COIN_DATA_SIZE];
    int worker_rank;
} coin_message_t;

typedef struct {
    u64_t hashes_done;
    int coins_found;
    int worker_rank;
} stats_message_t;

#endif
