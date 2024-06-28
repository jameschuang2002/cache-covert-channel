#include "sender.h"
#include "util.h"
#include "constants.h"
#include <stdio.h>
#include <signal.h>

char *shared_ptr;

void send_bit(int bit)
{
    unsigned long start_time = cc_sync();
    while (rdtsc() - start_time <= TRANSMIT_INTERVAL)
    {
        clflush(shared_ptr + (bit << LOG_CACHE_LINE_SIZE));
    }
}

void send_byte(char byte)
{
    for (int i = 0; i < 8; i++)
    {
        int bit = byte & 0x1;
        send_bit(bit);
        byte >>= 1;
    }
}

void send_string(char *str, int len)
{
    for (int i = 0; i < len; i++)
    {
        char c = str[i];
        send_byte(c);
    }
}

int main()
{
    shared_ptr = init_shared_memory();
    printf("Shared memory initialized at %p\n", shared_ptr);
    while (1)
    {
        send_byte('a');
    }
}