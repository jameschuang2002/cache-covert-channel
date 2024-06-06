#include "util.h"

#define SQUIRRELS_FILE "../passwords/squirrels.txt"

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

void sendString(char *str)
{
    for (int i = 0; i < strlen(str); i++)
    {
        sendChar(str[i], shm_ptr);
    }
}

int main(void)
{
    // signal handler for ctrl + c
    signal(SIGINT, memory_cleanup);
    init_shared_memory(&shm_fd, &shm_ptr, 1);

    printf("This is the sender, shared memory mapped to %p\n", shm_ptr);

    char input_str[128];
    int opmode;

    while (1)
    {
        printf("Enter the mode (1 or 2) of operation to be sent (1. file mode 2. text mode): ");
        scanf("%d", &opmode);
        printf("\n");

        if (opmode == 2)
        {
            while (1)
            {
                printf("Enter the string to be sent: ");
                scanf("%s", input_str);
                if (strcmp(input_str, "exit()") == 0)
                    break;
                printf("\n");
                sendChar(START_VALUE, shm_ptr);
                sendString(input_str);
                sendChar(STOP_VALUE, shm_ptr);
            }
        }
        else if (opmode == 1)
        {
            printf("Transmitting file..........\n");
            FILE *inFile = fopen(SQUIRRELS_FILE, "r");

            char file_char;

            sendChar(START_VALUE, shm_ptr);
            while (fscanf(inFile, "%c", &file_char) != EOF)
            {
                printf("%c", file_char);
                sendChar(file_char, shm_ptr);
            }
            sendChar(STOP_VALUE, shm_ptr);
            printf("File successfully transmitted\n");
        }
    }

    return 0;
}
