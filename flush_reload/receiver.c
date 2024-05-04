#include "util.h"

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

char getChar()
{
    int miss_count[8], hit_count[8], bitarr[8];

    for (int i = 0; i < 8; i++)
    {
        miss_count[i] = 0;
        hit_count[i] = 0;
    }

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

    return bitarr_to_char(bitarr);
}

int main(void)
{
    // install signal handler for ctrl + c to clean up resources
    signal(SIGINT, memory_cleanup);

    init_shared_memory(&shm_fd, &shm_ptr, 0);

    printf("This is the receiver, shared memory is mapped to %p\n", shm_ptr);

    int count = 0;
    while (1)
    {
        char c = getChar();
        if (count % 100 == 0)
        {
            printf("c: %d, %d\n", c, c == 12);
        }
        count++;
    }

    return 0;
}