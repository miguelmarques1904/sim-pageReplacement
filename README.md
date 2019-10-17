# sim-pageReplacement

Page replacement simulator for an hybrid DRAM-NVRAM architecture.
Tries to follow the M-Clock algorithm proposed in https://dl.acm.org/citation.cfm?id=2695675.
Made for educational purposes only.

## Instructions
```gcc -o mclock *.c
./mclock [file] [dram_size] [nvram_size]
```
__Note:__ both "dram_size" and "nvram_size" reflect the number of pages allowed for each of the memory types.

## File Structure
The file should have any number of lines. Each line must contain a page number and an operation ('r' or 'w'). For example: "30 r" for a read operation on page 30.
