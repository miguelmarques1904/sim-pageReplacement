#include "mclock.h"

int dram_size, nvram_size;
int nvram_writes, dram_writes;

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

int locate_dram(int page_id, char *rw, pte **table) {
    for(int i=0; i < dram_size; i++) {
        if((*table)[i].value == page_id) {
            if(!strcmp(rw, "w")) {
                if((*table)[i].candidate && (*table)[i].dirty && (*table)[i].reference) {
                    // Page is classified as hot-dirty
                    (*table)[i].candidate = 0;
                }
                (*table)[i].dirty = 1;
                dram_writes++;
            }
            (*table)[i].reference = 1;

            return 1;
        }
    }

    return 0;
}

int locate_nvram(int page_id, char *rw, pte **nvram, pte **dram) {
    static int p_hand = 0;

    int viewed_count = 0;
    while(viewed_count < nvram_size) {
        if((*nvram)[p_hand].value == page_id) {
            (*nvram)[p_hand].reference = 1;

            if(!strcmp(rw, "w")) {

                /*LAZY NVRAM-DRAM MIGRATION*/

                if(!place(page_id, rw, dram)) {
                    // DRAM is full and lazy bit is unset

                    if((*nvram)[p_hand].lazy) {
                        printf("LAZY\n");
                        mclock(page_id, rw, dram, nvram);

                        (*nvram)[p_hand].value = 0;
                    }
                    else {
                        printf("NOT LAZY\n");
                        // Overwrite page
                        nvram_writes++;
                        (*nvram)[p_hand].lazy = 1;
                    }

                }
            }

            p_hand = (p_hand + 1) % nvram_size;
            return 1;
        }

        viewed_count++;
        p_hand = (p_hand + 1) % nvram_size;

    }

    return 0;
}

int place(int page_id, char *rw, pte **table) {
    for(int i=0; i < dram_size; i++) {
        if((*table)[i].value == 0) {
            (*table)[i].value = page_id;
            if(!strcmp(rw, "w")) {
                (*table)[i].dirty = 1;
                dram_writes++;
            }
            (*table)[i].reference = 1;
            (*table)[i].candidate = 1;

            return 1;
        }
    }
    return 0;
}

int mclock(int page_id, char *rw, pte **dram, pte **nvram) {
    static int d_hand = 0;
    static int c_hand = 0;

    // First Step: D-Hand updates pages
    int viewed_count = 0; // Makes sure that the cycle does not keep going infinitely if all pages are candidates
    while(viewed_count <= dram_size) {
        if(!((*dram)[d_hand].candidate)) {
            viewed_count = 0; // Reset iteration count so that it reaches this page again if needed (to set as candidate)
            if((*dram)[d_hand].reference) {
                (*dram)[d_hand].reference = 0;
            }
            else {
                (*dram)[d_hand].candidate = 1;

                d_hand = (d_hand + 1) % dram_size;
                break;
            }
        }

        d_hand = (d_hand + 1) % dram_size;
        viewed_count++;
    }

    // Second Step: C-Hand chooses victim page
    while(1) {
        if((*dram)[c_hand].candidate) {
            if(((*dram)[c_hand].dirty == 0) || ((*dram)[c_hand].reference == 0)) {

                // If any of the bits is set, migrate to nvram. Otherwise just reclaim the page
                if(((*dram)[c_hand].dirty == 1) || ((*dram)[c_hand].reference == 1)) {
                    /* DRAM-NVRAM MIGRATION */
                    for(int j=0; j < nvram_size; j++) {
                        if(((*nvram)[j].value == 0) || ((*nvram)[j].lazy)) {
                            (*nvram)[j].value = (*dram)[c_hand].value;

                            (*nvram)[j].reference = 0;
                            (*nvram)[j].dirty = 0;
                            (*nvram)[j].lazy = 0;
                            break;
                        }
                    }

                    // What happens if no space is found in the NVRAM is not specified so I assume it just reclaims the page without migration

                }

                (*dram)[c_hand].value = page_id;
                if(strcmp(rw, "w")) {
                    (*dram)[c_hand].dirty = 1;
                    dram_writes++;
                }
                (*dram)[c_hand].reference = 1;

                c_hand = (c_hand + 1) % dram_size;
                return 1;
            }
            else {
                (*dram)[c_hand].reference = 0;
            }
        }

        c_hand = (c_hand + 1) % dram_size;
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

    dram_size = atoi(argv[2]);
    nvram_size = atoi(argv[3]);

    pte *dram = (pte*) malloc(sizeof(pte) * dram_size);
    pte *nvram = (pte*) malloc(sizeof(pte) * nvram_size);

    int hits_dram = 0;
    int hits_nvram = 0;

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

        if(locate_dram(page, rw, &dram)) {
            printf("DRAM HIT\n");
            hits_dram++;
        }
        else if(locate_nvram(page, rw, &nvram, &dram)) {
            printf("NVRAM HIT\n");
            hits_nvram++;
        }

        // DRAM and NVRAM Miss
        // Find free space in DRAM
        else if((dram_size > 0) && !place(page, rw, &dram)) {
            printf("DRAM FULL\n");
            // Migrate or free a page from DRAM and place the new page on the freed space
            mclock(page, rw, &dram, &nvram);
        }

        // Otherwise there was free space and page is placed in DRAM
    }


    fclose(fp);

    free(dram);
    free(nvram);
    free(rw);

    printf("\n------------------------\n");
    printf("Page Accesses: %d\n", page_accesses);
    printf("\n");
    printf("DRAM Hits: %d\n", hits_dram);
    printf("Hit Rate DRAM: %0.2f%%\n", (((double) hits_dram) / page_accesses) * 100);
    printf("\n");
    printf("DRAM+NVRAM Hits: %d\n", hits_dram + hits_nvram);
    printf("Hit Rate DRAM+NVRAM: %0.2f%%\n", (((double) (hits_dram + hits_nvram)) / page_accesses) * 100);
    printf("\n");
    printf("DRAM Writes: %d\n", dram_writes);
    printf("NVRAM Writes: %d\n", nvram_writes);
    printf("NVRAM-to-DRAM Write Ratio: %0.2f%%\n", (((double) (nvram_writes) / dram_writes) * 100));
    printf("------------------------\n\n");

    return 0;

}
