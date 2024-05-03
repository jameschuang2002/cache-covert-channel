#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

/* Constants defined by Flush + Reload */
// Roughly 4 MB. Always ensure that the executable is smaller than this
#define GPG_MAX_SIZE_BYTES 4194304

// See paper for the threshold of probe()
#define PROBE_THRESHOLD 110ul

// Maximum number of addresses to probe
#define MAX_NUM_OF_ADDRS 10u

// Number of time slots to record
#define TIME_SLOTS 50000

#define SHARED_MEMORY_PATH "/shared_memory"
#define SHARED_MEMORY_SIZE 4096
#define CACHE_LINE_SIZE 64
#define TRANSMIT_INTERVAL 0x8000

int probe(char *adrs);
unsigned long probe_timing(char *adrs);
void init_shared_memory(int *shm_fd, void **shm_ptr, int sender);