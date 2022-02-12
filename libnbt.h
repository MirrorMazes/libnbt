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

enum nbtb_state_type {
    S_INIT = 0,
    S_CMP = S_INIT,
    S_OBJ_OR_CLOSE, /* Object refer to all types */
    S_LST_VAL_OR_CLOSE,
    S_NXT_LST_VAL_OR_CLOSE,
    S_DONE
};

struct nbt_sized_buffer {
    char* content;
    int len;
};

struct nbt_parser_setting_t {
    const int list_meta_init_len;
    const int list_meta_expand_len;

    const int tok_init_len;
    const int tok_expand_len;
};


struct nbt_metadata;

struct nbt_parser_t;

typedef struct nbt_build nbt_build;

/* Normal interface */

// Parser util functions: in nbt_utils.c
void nbt_init_parser(struct nbt_parser_t* parser, struct nbt_sized_buffer* content, const struct nbt_parser_setting_t* setting);
void nbt_clear_parser(struct nbt_parser_t* parser, struct nbt_sized_buffer* content);
void nbt_destroy_parser(struct nbt_parser_t* parser);

// Tokenise NBT: in nbt_tok.c
int nbt_tokenise(struct nbt_parser_t *parser);

// Extract NBT: in nbt_extract.c
int nbt_extract(struct nbt_parser_t* parser, char* fmt, ...);

// In nbt_build.c
int nbt_init_build(nbt_build* b);
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


/* Easy interface */
int nbt_easy_extract(struct nbt_sized_buffer* content, char* fmt, ...);
