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

#define SHARED_MEMORY_PATH "/bin/ls"
#define SHARED_MEMORY_SIZE 4096
#define CACHE_LINE_SIZE 64

#define CHANNEL_INTERVAL 0xBD00        // Regular interval between transmissions (in cycles), adjusted to accommodate 2000 cycle MOE
#define CHANNEL_SYNC_TIMEMASK 0x00FFFF // Adjusted to ensure synchronization time slot falls within the 20000 cycle interval
#define CHANNEL_SYNC_JITTER 0x2E20     // Additional jitter range to accommodate the 3000 cycle MOE every 100 transmissions

#define START_VALUE 2
#define STOP_VALUE 3
#define NUM_RESENDS 80
#define PRINTABLE_ASCII_LOW 32
#define PRINTABLE_ASCII_HIGH 128
#define ACK 6

int probe(char *adrs);
unsigned long probe_timing(char *adrs);
void init_shared_memory(int *shm_fd, void **shm_ptr, int sender);
unsigned long get_time();
void clflush(char *adrs);
void char_to_bitarr(char c, int *bitarr);
unsigned long cc_sync();
unsigned char bitarr_to_char(int *bitarr);
char majority(unsigned char *list);
void mfence();
char getChar(void *shm_ptr);
void sendChar(char c, void *shm_ptr);
void waitCycles(unsigned long cycles);

struct char_counter
{
    char c;
    int count;
};