#include "util.h"

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

void init_shared_memory(int *shm_fd, void **shm_ptr, int sender)
{
    *shm_fd = shm_open(SHARED_MEMORY_PATH, O_CREAT | O_RDWR, 0666);
    if (*shm_fd == -1)
    {
        perror("shm_open");
        exit(1);
    }
    if (sender)
    {

        // truncate to the size of shared memory (assume 4 kB page)
        if (ftruncate(*shm_fd, SHARED_MEMORY_SIZE) == -1)
        {
            perror("ftruncate");
            exit(1);
        }
    }
    // map to current process memory space
    *shm_ptr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ, MAP_SHARED, *shm_fd, 0);
    if (*shm_ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }
    printf("%p\n", *shm_ptr);
}