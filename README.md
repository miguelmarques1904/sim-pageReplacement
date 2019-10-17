# sim-pageReplacement

Page replacement simulator for an hybrid DRAM-NVRAM architecture.
Tries to follow the M-Clock algorithm proposed in https://dl.acm.org/citation.cfm?id=2695675.
Made for educational purposes only.

## Instructions
```
git clone https://github.com/miguelmarques1904/sim-pageReplacement.git
gcc -o mclock *.c
./mclock [file] [dram_size] [nvram_size]
```
__Note:__ both "dram_size" and "nvram_size" define the number of pages each memory type can hold.

## File Structure
The file can have an arbitrary number of lines. One for each page access.
A line must contain a page number and an operation ('r' or 'w'). For example: "30 r" for a read operation on page 30.
