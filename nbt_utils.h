/* 
 *  This file is part of libnbt
 *
 *  Copyright (C) 2021-2022  Azbantium <azbantium@firemail.cc>
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

#include "libnbt.h"

#include <stdio.h>
#include <stdint.h>

#define NBT_NOT_AVAIL -3
#define NBT_UNCHANGED -4

#define MAX_DEPTH 30

enum nbtb_state_type {
    S_INIT = 0,
    S_CMP = S_INIT,
    S_OBJ_OR_CLOSE, /* Object refer to all types */
    S_LST_VAL_OR_CLOSE,
    S_NXT_LST_VAL_OR_CLOSE,
    S_DONE
};

struct nbt_token_t {
    nbt_type_t type;
    int start;
    int end;
    int len;
    int parent;
};

struct nbt_metadata {
    nbt_type_t type;
    int32_t num_of_entries;
};

typedef struct nbt_parser {
    struct nbt_sized_buffer* nbt_data;
    int current_byte;

    int current_token;
    int parent_token;

    struct nbt_metadata* list_meta;
    int cur_index;
    int max_list;

    const struct nbt_parser_setting_t* setting;
} nbt_parser;

struct nbtb_state {
    enum nbtb_state_type state;

    /* Stores the address of the ID byte */
    /* Only used for lists */
    int payload;
};

typedef struct nbt_build {
    struct nbtb_state stack[MAX_DEPTH + 1];

    struct nbtb_state* top;

    int current_depth;

    size_t offset;

} nbt_build;

/* nbt_utils.c */
void* nbt_realloc(void* ptr, size_t new_len, size_t original_len);

void swap_char_2(char* input, char* output);
void swap_char_4(char* input, char* output);
void swap_char_8(char* input, char* output);

void safe_swap_4(char* input, char* output);
void safe_swap_8(char* input, char* output);

uint16_t char_to_ushort(char* input);
int16_t char_to_short(char* input);
int32_t char_to_int(char* input);
int64_t char_to_long(char* input);
int64_t char_to_long_s(char* input);
float char_to_float(char* input);
double char_to_double(char* input);


int nbt_add_token(nbt_tok* tok, const int tok_len, int index, const nbt_tok* payload);

nbt_type_t nbt_tok_return_type(nbt_tok* token, int index, int max);
int nbt_tok_return_start(nbt_tok* token, int index, int max);
int nbt_tok_return_end(nbt_tok* token, int index, int max);
int nbt_tok_return_len(nbt_tok* token, int index, int max);
int nbt_tok_return_parent(nbt_tok* token, int index, int max);

int nbt_add_meta(int index, nbt_parser* parser, struct nbt_metadata* payload);

int nbt_meta_return_entries(nbt_parser* parser, int index);
