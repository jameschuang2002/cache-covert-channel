# Flush + Reload

### Project Desciption

This project provides a simple interface for covert communication between two processes. There are two modes of operation: text mode and file mode. 

In text mode, the sender process will prompt the user to enter text to sent to the receiver process. 

In file mode, the sender will be prompted to select files to send from the password folder 

### Memory Organization

A shared memory object is created with shm_open(), simulating the situation where two programs are sharing memory spaces through libraries. Using a memory that both processes has access will also work (ex. /bin/ls)

The cache is assumed to be a set-associative cache with cache lines of 64 bytes. 8 cache lines will be used so the associativity of the cache is irrelevant in this situation. However, we can theoretically achieve higher bandwidth and lower noise by using more lines in the cache.

### Communication Protocol 

The communication protocol is a combination of idea 1 and idea 2 in the appendix, inspired by the original Flush + Reload paper and an implementation by moehajj. 

A byte of data is represented by 8 cache lines with the first line starting at shm_ptr. 

byte = {bit 0, bit 1, bit 2, bit 3, bit 4, bit 5, bit 6, bit 7}
_________
0 | bit 0 (shm_ptr)
_________
1 | bit 1 (shm_ptr + CACHE_LINE)
_________
2 | bit 2 (shm_ptr + 2 * CACHE_LINE)
_________
3 | bit 3 (shm_ptr + 3 * CACHE_LINE)
_________
4 | bit 4 (shm_ptr + 4 * CACHE_LINE)
_________
5 | bit 5 (shm_ptr + 5 * CACHE_LINE)
_________
6 | bit 6 (shm_ptr + 6 * CACHE_LINE)
_________
7 | bit 7 (shm_ptr + 7 * CACHE_LINE)

Cache Coherence Protocol: When a cache line is flushed, all hierarchies of the cache containing data from the same memory location (may not necessarily have the same tag because caches are virtually tagged) will be invalidated.

Sender: The sender process sending the byte will flush the cache lines representing bit values 1 of the byte. This will cause the receiver's cache lines of the same memory to be invalidated and leading to longer access times from the DRAM. 

Receiver: The receiver process receiving the byte will try to access each cache line and tally the hits and misses at each specific location within a specified timeframe (CHANNEL_INTERVAL). If there are more misses, caused by invalidation from sender flushing the cache, the receiver know the bit is a 1. 

### Tradeoffs: Noise and Bandwidth

There are several parameters controlling the tradeoffs between noise and bandwidth located in util.h. 

NUM_RESENDS: The same will be repeated and then the most common value being received will be selected. 

CHANNEL_INTERVAL: The interval which the flushing will continue. The longer the channel interval, the less noise because the number of misses will less likely be greater than the number of hits. 

### Questions
1. How does flush+reload paper did it that way with just timing its own accesses and without exploiting coherence behaviour? 
2. Is it possible to implement it the way as described in the paper?

### References
The project is for educational and learning purposes. Great thanks to the following sources for inspiration to idea and implementation.  
1. A High Resolution, Low Noise, L3 Cache Side-Channel Attack: https://github.com/DanGe42/flush-reload
2. https://github.com/moehajj/Flush-Reload

# Appendix

### Idea Development and Brainstorming

**Initial Idea**
Having two processes communicate by transmitting bytes. 8 cache lines will be used

Sender process: To transmit a 1, access the corresponding cache line, otherwise do nothing

Receiver process: probe through 8 cache lines. If gets a cache hit, it's a 1, otherwise it's a 0 

Why does it not work?

Each process have their own virtual address. We are using mmap to map the shared memory to each process's address space, and it would create two different address each in the process's own virtual address space

The cache is virtually indexed and physically tagged to increase efficiency. The receiver process might not get a cache hit when the sender accesses it because the virtual address of the shared memory may be mapped to a different set in the set associative cache. 

To fix this, we will exploit the coherence protocol that invalidates all cache lines associated with the memory location being flushed. 

**Updated Idea V1 (https://github.com/moehajj/Flush-Reload)**
Sender Process: To send a 1, the sender will keep flushing a location in the shared memory 

Receiver Process: The receiver looks at that location for a fixed amount of time and totals the cache hits and misses. Miss more than hit (the memory location kept getting invalidated due to constant flushing), then it is a 1

What's the problem with this implementation? Noise due to all bits in the byte using the same location in the cache 

**Updated Idea V2 Merge Idea 1 + Idea 2**

Use 8 cache lines, each representing a bit in a byte

Sender: flush each cache line if the bit location of the byte is 1

Receiver: try to repeatedly access all 8 cache lines, if more hits than misses, 0, else 1