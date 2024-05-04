# Flush + Reload

### Idea Development

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

Question: How does flush+reload paper did it that way with just timing its own accesses and without exploiting coherence behaviour? Is it possible to implement it the way as described in the paper?

### References
The project is for educational and learning purposes. Great thanks to the following sources for inspiration to idea and implementation.  
1. A High Resolution, Low Noise, L3 Cache Side-Channel Attack: https://github.com/DanGe42/flush-reload
2. https://github.com/moehajj/Flush-Reload