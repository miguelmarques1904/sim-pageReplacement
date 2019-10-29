#ifndef MCLOCK_H_
#define MCLOCK_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_SIZE 100


typedef struct pte_t {
    int value;
    int dirty, reference;
    int lazy; // only for nvram

    int candidate; //0 for hot-dirty (managed by the D-Hand) and 1 for candidate (managed by the C-Hand)
} pte;


int parse_request(char *line, int *page, char **rw);
int locate_dram(int page_id, char *rw, pte **table);
int locate_nvram(int page_id, char *rw, pte **nvram, pte **dram);
int place(int page_id, char *rw, pte **table);
int mclock(int page_id, char *rw, pte **dram, pte **nvram);

#endif