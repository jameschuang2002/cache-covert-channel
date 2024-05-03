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
    // char *temp;
    // temp = (char *)(shm_ptr + CACHE_LINE_SIZE * 8);
    for (int i = 0; i < 8; i++)
    {
        int bit = c & 0x01;
        c >>= 1;
        if (bit)
        {
            // a bit was sent by access different cache lines, each line is assumed to have 8 bytes
            // temp = (char *)(shm_ptr + CACHE_LINE_SIZE * i);
        }
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
    }

    return 0;
}
