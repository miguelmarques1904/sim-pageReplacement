# sim-pageReplacement

Page replacement simulator for an hybrid DRAM-NVRAM architecture.
Tries to follow the M-Clock algorithm proposed in https://dl.acm.org/citation.cfm?id=2695675.
Made for educational purposes only.

## Extended Version
The extended version additionally considers persistent pages. Persistent pages can only be placed in the NVRAM and are reclaimed as needed.

## Instructions

### Main program
```
git clone https://github.com/miguelmarques1904/sim-pageReplacement.git
cd sim-pageReplacement
gcc -o mclock *.c
python3 page_gen.py
./mclock [file] [dram_size] [nvram_size]
```

### Extended Version
```
cd sim-pageReplacement/extended
gcc -o mclock_ext *.c
python3 page_gen_ext.py
./mclock_ext [file] [dram_size] [nvram_size]
```

__Note:__ Both "dram_size" and "nvram_size" define the number of pages each memory type can hold.

## File Structure
The file can have an arbitrary number of lines. One for each page access.
A line must contain a page number and an operation ('r' or 'w'). For example: "30 r" for a read operation on page 30.

### Extended Version
The extended version must receive an aditional parameter 'p' or 'np' indicating if a page is persistent or volatile.
For example: "30 r p" represents a read access to a persistent page

Use the python script __page_gen.py__ or __page_gen_ext.py__ (extended version) to generate input files.
