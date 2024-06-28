#include "receiver.h"
#include "util.h"
#include "constants.h"
#include <stdio.h>
#include <signal.h>

char *shared_ptr;

static const int lookup[2][2] = {
    {0, -1},
    {1, 0}};

int recv_bit()
{
    int bit = 0;
    *shared_ptr;
    *(shared_ptr + CACHE_LINE_SIZE);

    unsigned long start_time = cc_sync();
    while (rdtsc() - start_time <= TRANSMIT_INTERVAL)
    {
        bit += lookup[probe(shared_ptr)][probe(shared_ptr + CACHE_LINE_SIZE)];
    }
    return bit > 0;
}

char recv_byte()
{
    char c = 0;
    for (int i = 0; i < 8; i++)
    {
        c += recv_bit() << i;
    }
    printf("%02x\n", c);
    return c;
}

int main()
{
    shared_ptr = init_shared_memory();
    printf("Shared memory initialized at %p\n", shared_ptr);
    while (1)
    {
        char byte = recv_byte();
        // printf("%c", byte);
    }
}