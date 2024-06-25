# Covert Channel Implementations 
1. Flush+Reload V1
2. Flush+Reload V2

### How to Run the Code
Prerequisite: A linux environment (e.g. Windows Subsystem for Linux, Ubuntu)

1. Prepare 2 terminals 
2. Execute the following command
```
cd flush_reload
make
```
3. Run the sender process
```
./sender
```
4. Run the receiver process
```
./receiver
```

### Project Desciption

This project provides a simple interface for covert communication between two processes. There are two modes of operation: text mode and file mode. 

In text mode, the sender process will prompt the user to enter text to sent to the receiver process. 

In file mode, the sender will be prompted to select files to send from the password folder

### Protocol and Ideation

Protocol and ideation process for each project is documented in detail within protocol.md in each subfolder

### Observation 
1. When battery is low, the cache covert channel for somehow does not work properly. I assume it is based on some optimizations to prevent overusing battery power to flush caches. 

### References
The project is for educational and learning purposes. Great thanks to the following sources for inspiration to idea and implementation.  

1. Yarom, Y., & Falkner, K. (2014). FLUSH+RELOAD: a high resolution, low noise, L3 cache side-channel attack. In Proceedings of the 23rd USENIX Conference on Security Symposium (pp. 719–732). USENIX Association.
2. https://github.com/moehajj/Flush-Reload
3. F. Liu, Y. Yarom, Q. Ge, G. Heiser and R. B. Lee, "Last-Level Cache Side-Channel Attacks are Practical," 2015 IEEE Symposium on Security and Privacy, San Jose, CA, USA, 2015, pp. 605-622, doi: 10.1109/SP.2015.43.

### Notes and Disclaimer

Specific information about the cache organization may vary upon the processor, please modify the attack according to the manufacturer's specifications


