#include "util.h"
#include "constants.h"
#include <unistd.h>   // include for asm
#include <fcntl.h>    // include for O_RDONLY
#include <sys/mman.h> // include for memory mapping constants
#include <stdlib.h>
#include <stdio.h>

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
        //    "    clflush 0(%1)      \n"
        : "=a"(time)
        : "c"(adrs)
        : "%esi", "%edx");
    return time < PROBE_THRESHOLD;
}

void clflush(char *adrs)
{
    asm volatile(
        "clflush (%0)"
        :
        : "r"(adrs)
        : "memory");
}

unsigned long rdtsc()
{
    volatile unsigned long time;
    asm __volatile__(
        "    lfence             \n"
        "    rdtsc              \n"
        : "=a"(time));
    return time;
}

char *init_shared_memory()
{
    int fd = open(SHARED_MEMORY_PATH, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char *shared_ptr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (shared_ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    close(fd);
    return shared_ptr;
}
unsigned long cc_sync()
{
    while ((rdtsc() & SYNC_TIMEMASK) > SYNC_JITTER)
        ;
    return rdtsc();
}