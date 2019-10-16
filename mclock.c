#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_SIZE 100

typedef struct pte_t {
    int value;
    int dirty, reference;
} pte;

int parse_request(char *line, int *page, char **rw) {
    char *ptr = strtok(line, " \n");

    if(ptr == NULL) {
        return 0;
    }

    if(!(*page = atoi(ptr))) {
        // could not convert number
        return 0;
    }

    ptr = strtok(NULL, " \n");

    if((ptr == NULL) || (strcmp(ptr, "w") && strcmp(ptr, "r"))) {
        return 0;
    }

    strcpy(*rw, ptr);

    return 1;
}

int locate(int page_id, char *rw, pte *table, int size) {
    for(int i=0; i < size; i++) {
        if(table[i].value == page_id) {
            return 1;
        }
    }

    return 0;
}


int main(int argc, char** argv) {
    
    if(argc != 4) {
        printf("USAGE: [file] [dram_size] [nvram_size]\n");
        return 1;
    }

    FILE *fp;

    if((fp = fopen(argv[1], "r")) == NULL) {
        printf("File not found. USAGE: [file] [dram_size] [nvram_size]\n");
        return 1;
    }

    int dram_size = atoi(argv[2]);
    int nvram_size = atoi(argv[3]);

    pte *dram = (pte*) malloc(sizeof(pte) * dram_size);
    pte *nvram = (pte*) malloc(sizeof(pte) * nvram_size);

    int hits_dram;
    int hits_nvram;

    int page_accesses;

    char line[MAX_LINE_SIZE];

    int page;
    char *rw = (char*) malloc(sizeof(char));

    while(fgets(line, MAX_LINE_SIZE, fp) != NULL) {

        if(!parse_request(line, &page, &rw)) {
            if(strcmp("\n", line)) {
                printf("Malformed Input! Input must match: [page_id] [r|w]\n");
            }
            continue;
        }

        printf("Processing request: page-%d op-%s\n", page, rw);

        page_accesses++;

        if(locate(page, rw, dram, dram_size)) {
            // DRAM Hit
            hits_dram++;
        }
        else if(locate(page, rw, nvram, nvram_size)) {
            // NVRAM Hit
            hits_nvram++;
        }
    }


    fclose(fp);
    
    free(dram);
    free(nvram);
    free(rw);

    return 0;

}
