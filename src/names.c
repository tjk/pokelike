#include "names.h"

#include <string.h>
#include <sys/stat.h>

struct names_sub_table {
    int doubles[100];
    int triples[100];
};

struct names_table {
    struct names_sub_table sub_tables[26];
} names_table;

void names_init()
{
    const char fname[] = "./assets/names.txt";

    // load assets/names.txt
    struct stat st;
    stat(fname, &st);

    FILE *f = fopen(fname, "r");

    char *data = MALLOC(st.st_size + 1);
    fread(data, 1, st.st_size, f);
    data[st.st_size] = '\0';

    fclose(f);

    memset(&names_table, 0, sizeof(names_table.sub_tables));

    // TODO generate some sort of probability table
    // make an array of 26 elements that points to struct of 2 arrays of 100 elements
    // where each of the 100 elements contains the ratio of triples or doubles encoded as ints (1 or 2 bytes in 4)
    for (int i = 0; i < st.st_size; ++i) {
    }

    // TODO cache the probability table (does not change given same input file)
}

char *names_generate_name()
{
    // TODO
    return NULL;
}
