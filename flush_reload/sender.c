#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define SHARED_MEMORY_PATH "/shared_memory"
#define SHARED_MEMORY_SIZE 4096

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
    for (int i = 0; i < 8; i++)
    {
        int bit = c & 0x01;
        c >>= 1;
        if (bit)
        {
            // a bit was sent by access different cache lines, each line is assumed to have 8 bytes
            volatile char *temp = (volatile char *)(shm_ptr + 8 * i);
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

    // open shared memory region
    shm_fd = shm_open(SHARED_MEMORY_PATH, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open");
        return -1;
    }

    // truncate to the size of shared memory (assume 4 kB page)
    if (ftruncate(shm_fd, SHARED_MEMORY_SIZE) == -1)
    {
        perror("ftrucate");
        exit(1);
    }

    // map to current process memory space
    shm_ptr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    // send data
    while (1)
    {
        *(int *)shm_ptr = 4;

        sendChar(12);
        usleep(2000);
        //     sendString("Hello World!");
        //     usleep(100);
    }

    return 0;
}
