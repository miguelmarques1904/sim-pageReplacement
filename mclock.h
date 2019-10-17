#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_SIZE 100
#define UNSET_INTERVAL 100


typedef struct pte_t {
    int value;
    int dirty, reference;
    int lazy; // only for nvram

    int unset_count_dirty, unset_count_reference;
} pte;


int parse_request(char *line, int *page, char **rw);
int locate_dram(int page_id, char *rw, pte **table, int size);
int locate_nvram(int page_id, char *rw, pte **nvram, int nvram_size, pte **dram, int dram_size, int *nvram_writes);
int place(int page_id, char *rw, pte **table, int size);
int free_dram(int page_id, char *rw, pte **dram, int dram_size, pte **nvram, int nvram_size);
void step(pte **table, int size);