#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE_SIZE 100

typedef struct pte_t {
    int value;
    int dirty, reference;
} pte;


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


    char line[MAX_LINE_SIZE];
    int page;

    while(fgets(line, MAX_LINE_SIZE, fp) != NULL) {
        page = atoi(line);        
    }


    fclose(fp);

    return 0;

}
