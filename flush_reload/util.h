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

#define CHANNEL_INTERVAL 0x10000
#define CHANNEL_SYNC_TIMEMASK 0x000FFFFF
#define CHANNEL_SYNC_JITTER 0x0100

#define START_VALUE 2
#define STOP_VALUE 3
#define NUM_RESENDS 80

int probe(char *adrs);
unsigned long probe_timing(char *adrs);
void init_shared_memory(int *shm_fd, void **shm_ptr, int sender);
unsigned long get_time();
void clflush(char *adrs);
void char_to_bitarr(char c, int *bitarr);
unsigned long cc_sync();
char bitarr_to_char(int *bitarr);
char majority(char *list);

struct char_counter
{
    char c;
    int count;
};