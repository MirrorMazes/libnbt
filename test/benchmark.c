#include "../libnbt.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <byteswap.h>
#include <assert.h>

#include "../nbt_utils.h"
#include "cog-utils.h"
#include "clock.h"

/* Code used to benchmark this library */

void libnbt_parse()
{
    char* file_contents;
    size_t file_len = 0;
    file_contents = cog_load_whole_file("test/bigtest.nbt.uncompressed", &file_len);

    struct nbt_sized_buffer buf = {.content = file_contents, .len = file_len};
    struct nbt_parser parser;
    struct nbt_parser_setting_t setting = {.list_meta_init_len = 30, .alloc = malloc, .free = free};
    nbt_init_parser(&parser, &buf, &setting);

    int tok_init_len = 80;
    
    clock_t start_parse, end_parse;
    start_parse = clock();
    // Benchmarking starts before token allocation to ensure fairness, even though in this library token can be reused.
    nbt_tok* tok = malloc(sizeof(nbt_tok) * tok_init_len);
    int res = nbt_tokenise(&parser, tok, tok_init_len);
    end_parse = clock();

    double cpu_time_used_parse = ((double) (end_parse - start_parse)) / CLOCKS_PER_SEC;

    assert(res != NBT_NOMEM);
    
    clock_t start_find, end_find;
    start_find = clock();

    struct nbt_lookup_t path[2];
    path[0] = (struct nbt_lookup_t){.type = nbt_compound, .name = "Level"};
    path[1] = (struct nbt_lookup_t){.type = nbt_byte, .name = "byteTest"};

    struct nbt_index_t res_tok = {0};
    int res_code = nbt_find(tok, tok_init_len, &parser, path, 2, &res_tok);

    assert(res_code == 0);

    char nbt_result = 0;
    nbt_result = file_contents[res_tok.end];

    assert(nbt_result == 65);
    end_find = clock();

    double cpu_time_used_find = ((double) (end_find - start_find)) / CLOCKS_PER_SEC;

    nbt_destroy_parser(&parser);
    free(file_contents);
    free(tok);
    // printf("Time used in parsing: %lf. Time used in finding: %lf. Total time: %lf\n", cpu_time_used_parse, cpu_time_used_find, cpu_time_used_find + cpu_time_used_parse);
    printf("%lf\n", cpu_time_used_parse + cpu_time_used_find);
}

int main(int argc, char const *argv[])
{
    libnbt_parse();

    return 0;
}
