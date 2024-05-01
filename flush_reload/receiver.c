#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>

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

/* --------------------------------------*/

/* functions implemented by Flush + Reload */

int probe(char *adrs)
{
    volatile unsigned long time;

    asm __volatile__(
        "    mfence             \n"
        "    lfence             \n"
        "    rdtsc              \n"
        "    lfence             \n"
        "    movl %%eax, %%esi  \n"
        "    movl (%1), %%eax   \n"
        "    lfence             \n"
        "    rdtsc              \n"
        "    subl %%esi, %%eax  \n"
        "    clflush 0(%1)      \n"
        : "=a"(time)
        : "c"(adrs)
        : "%esi", "%edx");
    return time < PROBE_THRESHOLD;
}

unsigned long probe_timing(char *adrs)
{
    volatile unsigned long time;

    asm __volatile__(
        "    mfence             \n"
        "    lfence             \n"
        "    rdtsc              \n"
        "    lfence             \n"
        "    movl %%eax, %%esi  \n"
        "    movl (%1), %%eax   \n"
        "    lfence             \n"
        "    rdtsc              \n"
        "    subl %%esi, %%eax  \n"
        "    clflush 0(%1)      \n"
        : "=a"(time)
        : "c"(adrs)
        : "%esi", "%edx");
    return time;
}

/* --------------------------------------- */

int shm_fd;
void *shm_ptr;

// signal handler for sigint
void memory_cleanup(int sig)
{
    if (munmap(shm_ptr, SHARED_MEMORY_SIZE) == -1)
    {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    // Close the shared memory object
    if (close(shm_fd) == -1)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

int main(void)
{
    int shm_fd;
    void *shm_ptr;
    char *adrs[SHARED_MEMORY_SIZE];
    char str[128];

    // install signal handler for ctrl + c to clean up resources
    signal(SIGINT, memory_cleanup);

    // open shared memory object
    shm_fd = shm_open(SHARED_MEMORY_PATH, O_RDONLY, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open");
        exit(1);
    }

    // map shared memory to current proces memory
    shm_ptr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    int it = 0;
    while (1)
    {
        unsigned char val = 0;

        // read a single byte using covert channel
        for (int i = 0; i < 8; i++)
        {
            unsigned long time = probe_timing((char *)((shm_ptr + (i << 3))));
            // printf("%lu\n", time);
            if (time < 600)
                val += 1 << i;
        }

        // slow down printing or analysis
        if (it % 100000 == 0)
        {
            printf("%d\n", val);
            it = 0;
        }
        //     // probe 8 addresses to extract a byte of data
        //     unsigned char length = 0;
        //     for (int i = 0; i < 8; i++)
        //     {
        //         int bit = probe((volatile char *)((shm_ptr + (i << 3))));
        //         length += bit << i;
        //     }

        //     for (int i = 0; i < length; i++)
        //     {
        //         char c = 0;
        //         for (int j = 0; j < 8; j++)
        //         {
        //             int bit = probe((volatile char *)((shm_ptr + (i << 3))));
        //             c += bit << i;
        //         }
        //         str[i] = c;
        //     }
        //     str[length] = '\0';
        //     printf("%s\n", str);
        it++;
    }

    return 0;
}