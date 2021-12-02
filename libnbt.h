#pragma once

#include <stdbool.h>
#include <stdint.h>

#define NBT_NOT_AVAIL -3
#define NBT_UNCHANGED -4
#define NBT_MAX_NESTED_LIST 30

typedef enum {
    nbt_end = 0,
    nbt_byte = 1,
    nbt_short = 2,
    nbt_int = 3,
    nbt_long = 4,
    nbt_float = 5,
    nbt_double = 6,
    nbt_byte_array = 7,
    nbt_string = 8,
    nbt_list = 9,
    nbt_compound = 10,
    nbt_int_array = 11,
    nbt_long_array = 12,

    nbt_identifier = -1,
    nbt_primitive = -2
} nbt_type_t;

struct nbt_sized_buffer{
    char* content;
    int len;
};

struct nbt_token_t{
    nbt_type_t type;
    int start;
    int end;
    int len;
    int parent;
};

struct nbt_metadata{
    nbt_type_t type;
    int32_t num_of_entries;
};

struct nbt_list_data{
    struct nbt_metadata list_meta[NBT_MAX_NESTED_LIST];
    int cur_index;
    bool directly_in_list;

};

struct nbt_parser_t{
    struct nbt_sized_buffer *nbt_data;
    int current_byte;

    int current_token;
    int parent_token;

    struct nbt_metadata list_meta[NBT_MAX_NESTED_LIST];
    int cur_index;
    bool store_token;

};


void nbt_init_parser(struct nbt_parser_t *parser, struct nbt_sized_buffer* content);

void nbt_scanf(struct nbt_parser_t* parser, char* fmt, ...);