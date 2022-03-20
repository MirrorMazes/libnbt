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

#define MAX_NAME_LEN 150

#define NBT_WARN -5
#define NBT_NOMEM -6

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

struct nbt_sized_buffer {
    char* content;
    int len;
};

struct nbt_lookup_t{
    nbt_type_t type;
    char name[MAX_NAME_LEN + 1];

    long index;
};

struct nbt_index_t {
    int start;
    int end;
    int len;
};

struct nbt_parser_setting_t {
    const int list_meta_init_len;
    const int list_meta_expand_len;

    const int tok_init_len;
    const int tok_expand_len;
};

typedef struct nbt_parser nbt_parser;

typedef struct nbt_build nbt_build;

/* Normal interface */

// nbt_utils.c
void nbt_init_parser(nbt_parser* parser, struct nbt_sized_buffer* content, const struct nbt_parser_setting_t* setting);
void nbt_clear_parser(nbt_parser* parser, struct nbt_sized_buffer* content);
void nbt_destroy_parser(nbt_parser* parser);

// nbt_tok.c
int nbt_tokenise(nbt_parser *parser);

// nbt_find.c
int nbt_find(nbt_parser* parser, struct nbt_lookup_t* path, int path_size, struct nbt_index_t* res);

// nbt_build.c
void nbt_init_build(nbt_build* b);
int nbt_start_compound(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len);
int nbt_end_compound(nbt_build* b, char* buf, const int buf_len);
int nbt_start_list(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len);
int nbt_end_list(nbt_build* b, char* buf, const int buf_len);
int nbt_add_char(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, char payload);
int nbt_add_short(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, short payload);
int nbt_add_integer(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, int payload);
int nbt_add_long(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, long payload);
int nbt_add_float(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, float payload);
int nbt_add_double(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, double payload);
int nbt_add_byte_array(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, char payload[], int payload_len);
int nbt_add_string(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, char payload[], unsigned short payload_len);
int nbt_add_int_array(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, int payload[], int payload_len);
int nbt_add_long_array(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, long payload[], int payload_len);

