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
        // "    clflush 0(%1)      \n"
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

/* from https://github.com/moehajj/Flush-Reload */

void clflush(char *adrs)
{
    asm volatile("clflush (%0)" ::"r"(adrs));
}

unsigned long get_time()
{
    unsigned long time;
    asm volatile("rdtsc" : "=a"(time));
    return time;
}

unsigned long cc_sync()
{
    while ((get_time() & CHANNEL_SYNC_TIMEMASK) > CHANNEL_SYNC_JITTER)
        ;
    return get_time();
}

// -----------------------------------------------------

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

void char_to_bitarr(char c, int *bitarr)
{
    for (int i = 0; i < 8; i++)
    {
        int bit = c & 0x01;
        bitarr[i] = bit;
        c >>= 1;
    }
}

char bitarr_to_char(int *bitarr)
{
    char c = 0;
    for (int i = 0; i < 8; i++)
    {
        c += bitarr[i] << i;
    }
    return c;
}

char majority(char *list)
{
    struct char_counter counter[NUM_RESENDS];
    int length = 0;
    for (int i = 0; i < NUM_RESENDS; i++)
    {
        bool isNew = true;
        for (int j = 0; j < length; j++)
        {
            if (list[i] == counter[j].c)
            {
                counter[j].count++;
                isNew = false;
            }
        }
        if (isNew)
        {
            counter[length].count = 1;
            counter[length].c = list[i];
            length++;
        }
    }

    struct char_counter max_count_char = counter[0];
    for (int i = 0; i < length; i++)
    {
        if (counter[i].count > max_count_char.count)
        {
            max_count_char.count = counter[i].count;
            max_count_char.c = counter[i].c;
        }
    }
    return max_count_char.c;
}