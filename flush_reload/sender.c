#include "util.h"

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

    // resend for receiver to implement voting to minimize noise
    for (int i = 0; i < NUM_RESENDS; i++)
    {
        /* clear all relevant memory lines before resending */
        // for (int j = 0; j < 8; j++)
        // {
        //     clflush((char *)(shm_ptr + j * CACHE_LINE_SIZE));
        // }
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

    char input_str[128];

    while (1)
    {
        printf("Enter the string to be sent: ");
        scanf("%s", input_str);
        printf("\n");
        sendChar(START_VALUE);
        sendString(input_str);
        sendChar(STOP_VALUE);
    }

    return 0;
}
