#include "util.h"

#define SHARED_MEMORY_PATH "/shared_memory"
#define SHARED_MEMORY_SIZE 4096
#define CACHE_LINE_SIZE 64

int shm_fd;
void *shm_ptr;

// signal handler for signint
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

// send a single character
void sendChar(char c)
{
    int bitarr[8];
    char_to_bitarr(c, bitarr);

    unsigned long startTime = cc_sync();
    unsigned long currentTime = get_time();

    while (currentTime - startTime < CHANNEL_INTERVAL)
    {
        for (int i = 0; i < 8; i++)
        {
            if (bitarr[i])
            {
                clflush((char *)(shm_ptr + i * CACHE_LINE_SIZE));
            }
        }
        currentTime = get_time();
    }
}

void sendString(char *str)
{
    for (int i = 0; i < strlen(str); i++)
    {
        sendChar(str[i]);
    }
}

int main(void)
{
    // signal handler for ctrl + c
    signal(SIGINT, memory_cleanup);
    init_shared_memory(&shm_fd, &shm_ptr, 1);

    printf("This is the sender, shared memory mapped to %p\n", shm_ptr);

    while (1)
    {
        sendChar(12);
    }

    return 0;
}
