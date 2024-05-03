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

int value_sent()
{
    return probe((char *)(shm_ptr + (8 * CACHE_LINE_SIZE)));
}

int main(void)
{
    // install signal handler for ctrl + c to clean up resources
    signal(SIGINT, memory_cleanup);

    init_shared_memory(&shm_fd, &shm_ptr, 0);

    printf("This is the receiver, shared memory is mapped to %p\n", shm_ptr);

    while (1)
    {
    }

    return 0;
}