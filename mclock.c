#include "mclock.h"


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

int locate_dram(int page_id, char *rw, pte **table, int size) {
    for(int i=0; i < size; i++) {
        if((*table)[i].value == page_id) {
            if(!strcmp(rw, "w")) {
                (*table)[i].dirty = 1;
                (*table)[i].unset_count_dirty = UNSET_INTERVAL;
            }
            (*table)[i].reference = 1;
            (*table)[i].unset_count_reference = UNSET_INTERVAL;

            return 1;
        }
    }

    return 0;
}

int locate_nvram(int page_id, char *rw, pte **nvram, int nvram_size, pte **dram, int dram_size, int *nvram_writes) {
    for(int i=0; i < nvram_size; i++) {
        if((*nvram)[i].value == page_id) {
            (*nvram)[i].reference = 1;
            (*nvram)[i].unset_count_reference = UNSET_INTERVAL;

            if(!strcmp(rw, "w")) {
                (*nvram)[i].dirty = 1;
                (*nvram)[i].unset_count_dirty = UNSET_INTERVAL;

                /*LAZY NVRAM-DRAM MIGRATION*/
                int j;
                if((*nvram)[i].lazy) {
                    printf("LAZY\n");
                    while(!free_dram(page_id, rw, dram, dram_size, nvram, nvram_size)) {
                        step(dram, dram_size);
                        step(nvram, dram_size);
                    }

                    (*nvram)[i].value = 0;
                }
                else if(!free_dram(page_id, rw, dram, dram_size, nvram, nvram_size)) {
                    printf("NOT LAZY\n");
                    // DRAM is full and lazy bit is unset
                    (*nvram_writes)++;
                    (*nvram)[i].lazy = 1;
                    ;

                }
                else {
                    (*nvram)[i].value = 0;
                }
            }

            return 1;
        }
    }

    return 0;
}

int place(int page_id, char *rw, pte **table, int size) {
    for(int i=0; i < size; i++) {
        if((*table)[i].value == 0) {
            (*table)[i].value = page_id;
            if(!strcmp(rw, "w")) {
                (*table)[i].dirty = 1;
                (*table)[i].unset_count_dirty = UNSET_INTERVAL;
            }
            (*table)[i].reference = 1;
            (*table)[i].unset_count_reference = UNSET_INTERVAL;

            return 1;
        }
    }
    return 0;
}

int free_dram(int page_id, char *rw, pte **dram, int dram_size, pte **nvram, int nvram_size) {
    
    for(int i=0; i < dram_size; i++) {
        if(((*dram)[i].dirty == 0) || ((*dram)[i].reference == 0)) {

            // If any of the bits is set, migrate to nvram. Otherwise just reclaim the page
            if(((*dram)[i].dirty == 1) || ((*dram)[i].reference == 1)) {
                /* DRAM-NVRAM MIGRATION */
                for(int j=0; j < nvram_size; j++) {
                    if(((*nvram)[j].value == 0) || ((*nvram)[j].lazy)) {
                        (*nvram)[j].value = (*dram)[i].value;

                        (*nvram)[j].reference = (*dram)[i].reference;
                        (*nvram)[j].unset_count_reference = (*dram)[i].unset_count_reference;

                        (*nvram)[j].dirty = (*dram)[i].dirty;
                        (*nvram)[j].unset_count_dirty = (*dram)[i].unset_count_dirty;

                        (*nvram)[j].lazy = 0;
                        break;
                    }
                }

            }
            
            (*dram)[i].value = page_id;
            if(strcmp(rw, "w")) {
                (*dram)[i].dirty = 1;
                (*dram)[i].unset_count_dirty = UNSET_INTERVAL;
            }
            (*dram)[i].reference = 1;
            (*dram)[i].unset_count_reference = UNSET_INTERVAL;
            
            return 1;
        }
    }
    
    return 0;
}

void step(pte **table, int size) {
    for(int i=0; i < size; i++) {
        if((*table)[i].value != 0) {
            if((*table)[i].unset_count_reference-- < 0) {
                (*table)[i].reference = 0;
                (*table)[i].unset_count_reference = 999; // prevents integer underflow
            }
            if((*table)[i].unset_count_dirty-- < 0) {
                (*table)[i].dirty = 0;
                (*table)[i].unset_count_dirty = 999;
            }
        }
    }
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

    int hits_dram = 0;
    int hits_nvram = 0;
    int nvram_writes = 0;

    int page_accesses = 0;

    char line[MAX_LINE_SIZE];

    int page;
    char *rw = (char*) malloc(sizeof(char));

    while(fgets(line, MAX_LINE_SIZE, fp) != NULL) {

        if(!parse_request(line, &page, &rw)) {
            if(!strcmp("\n", line)) {
                printf("Malformed Input! Input must match: [page_id] [r|w]\n");
            }
            continue;
        }

        printf("Processing request: page-%d op-%s\n", page, rw);

        page_accesses++;

        if(locate_dram(page, rw, &dram, dram_size)) {
            // DRAM Hit
            printf("DRAM HIT\n");
            hits_dram++;
        }
        else if(locate_nvram(page, rw, &nvram, nvram_size, &dram, dram_size, &nvram_writes)) {
            // NVRAM Hit
            printf("NVRAM HIT\n");
            hits_nvram++;
        }

        // DRAM and NVRAM Miss
        // Find free space in DRAM
        else if(!place(page, rw, &dram, dram_size)) {
            // DRAM full
            // Migrate or free a page from DRAM and place page
            printf("DRAM FULL\n");
            while(!free_dram(page, rw, &dram, dram_size, &nvram, nvram_size)) {
                step(&dram, dram_size);
                step(&nvram, nvram_size);
            }
        }

        // Otherwise there was free space and page is placed in DRAM

        step(&dram, dram_size);
        step(&nvram, nvram_size);
    }
    

    fclose(fp);
    
    free(dram);
    free(nvram);
    free(rw);

    printf("\n------------------------\n");
    printf("Page Accesses: %d\n", page_accesses);
    printf("\n");
    printf("DRAM Hits: %d\n", hits_dram);
    printf("Hit Rate DRAM: %0.2f%%\n", (((double) hits_dram)/page_accesses) * 100);
    printf("\n");
    printf("DRAM+NVRAM Hits: %d\n", hits_dram + hits_nvram);
    printf("Hit Rate DRAM+NVRAM: %0.2f%%\n", (((double) (hits_dram+hits_nvram))/page_accesses) * 100);
    printf("\n");
    printf("NVRAM Writes: %d\n", nvram_writes);
    printf("------------------------\n\n");

    return 0;

}
