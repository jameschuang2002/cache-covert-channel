#include "util.h"
#define NUM_CHARS 256

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
    asm volatile(
        "lfence \n"
        "rdtsc \n"
        : "=a"(time));
    return time;
}

void mfence()
{
    asm volatile("mfence");
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
    *shm_fd = open(SHARED_MEMORY_PATH, O_RDONLY);
    if (*shm_fd == -1)
    {
        perror("shm_open");
        exit(1);
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

unsigned char bitarr_to_char(int *bitarr)
{
    unsigned char c = 0;
    for (int i = 0; i < 8; i++)
    {
        c += bitarr[i] << i;
    }
    return c;
}

char majority(unsigned char *list)
{
    int count[NUM_CHARS] = {0};
    for (int i = 0; i < NUM_RESENDS; i++)
    {
        count[(int)(list[i])]++;
    }

    int max_count = 0;
    char c = 0;
    for (int i = 0; i < NUM_CHARS; i++)
    {
        if (count[i] > max_count)
        {
            max_count = count[i];
            c = i;
        }
    }
    return c;
}

char getChar(void *shm_ptr)
{
    int miss_count[8], hit_count[8], bitarr[8];
    unsigned char results[NUM_RESENDS];

    for (int j = 0; j < NUM_RESENDS; j++)
    {
        for (int i = 0; i < 8; i++)
        {
            miss_count[i] = 0;
            hit_count[i] = 0;
            *(char *)(shm_ptr + i * CACHE_LINE_SIZE);
        }

        mfence();
        unsigned long startTime = cc_sync();
        unsigned long endTime = get_time();
        while (endTime - startTime < CHANNEL_INTERVAL)
        {
            for (int i = 0; i < 8; i++)
            {
                if (probe((char *)(shm_ptr + CACHE_LINE_SIZE * i)))
                {
                    hit_count[i]++;
                }
                else
                {
                    miss_count[i]++;
                }
            }
            endTime = get_time();
        }

        for (int i = 0; i < 8; i++)
        {
            bitarr[i] = miss_count[i] >= hit_count[i];
        }
        results[j] = bitarr_to_char(bitarr);
    }

    char out_char = majority(results);
    return out_char;
}

// send a single character
void sendChar(char c, void *shm_ptr)
{
    int bitarr[8];
    char_to_bitarr(c, bitarr);

    // resend for receiver to implement voting to minimize noise
    for (int i = 0; i < NUM_RESENDS; i++)
    {
        mfence();
        unsigned long startTime = cc_sync();
        unsigned long currentTime = get_time();
        while (currentTime - startTime < CHANNEL_INTERVAL)
        {
            for (int j = 0; j < 8; j++)
            {
                if (bitarr[j])
                {
                    clflush((char *)(shm_ptr + j * CACHE_LINE_SIZE));
                }
            }
            currentTime = get_time();
        }
    }
    waitCycles(6000);
}

void waitCycles(unsigned long cycles)
{
    unsigned long start = get_time();
    unsigned long currentTime = get_time();
    while (currentTime - start < cycles)
    {
        currentTime = get_time();
    }
}
