/* 
 *  Libnbt: A library to work with NBT data, written in C
 *
 *  Copyright (C) 2021  Azbantium <azbantium@firemail.cc>
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define NBT_NOT_AVAIL -3
#define NBT_UNCHANGED -4
#define NBT_WARN -5

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

struct nbt_setting_t{
    const int list_meta_init_len;
    const int list_meta_expand_len;

    const int tok_init_len;
    const int tok_expand_len;
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

struct nbt_parser_t{
    struct nbt_sized_buffer *nbt_data;
    int current_byte;

    int current_token;
    int parent_token;
    int max_token;

    struct nbt_metadata* list_meta;
    int cur_index;
    int max_list;

    const struct nbt_setting_t* setting;
};

/* Normal interface: WIP */
void nbt_init_parser(struct nbt_parser_t *parser, struct nbt_sized_buffer* content, const struct nbt_setting_t* setting);
void nbt_destroy_parser(struct nbt_parser_t* parser);
struct nbt_token_t* nbt_extract(struct nbt_parser_t* parser, struct nbt_token_t* tok, char* fmt, ...);

/* Easy interface */
void nbt_easy_extract(struct nbt_sized_buffer* content, char* fmt, ...);
