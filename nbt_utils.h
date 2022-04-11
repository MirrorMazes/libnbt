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

#define NBT_TOK_DEFAULT_LEN 50
#define NBT_TOK_DEFAULT_RESIZE_LEN 25
#define NBT_META_DEFAULT_LEN 1
#define NBT_META_DEFAULT_RESIZE_LEN 10

//ansi
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define CYN "\e[0;36m"

#define REDB "\e[41m"
#define GRNB "\e[42m"
#define YELB "\e[43m"
#define CYNB "\e[46m"

#define HWHT "\e[0;97m"

#define ANSI_RESET "\e[0m"



#ifdef NDBG
#define debug(args, ...)
#else
#define debug(args, ...) \
    fprintf(stdout, ANSI_RESET HWHT CYNB "DEBUG" ANSI_RESET " %s:%d: " CYN args ANSI_RESET"\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#ifndef NLOG

#define log_err(args, ...) \
    fprintf(stdout, ANSI_RESET HWHT REDB "[ERROR]" ANSI_RESET " %s:%d: " RED args ANSI_RESET"\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define log_warn(args, ...) \
    fprintf(stdout, ANSI_RESET HWHT YELB "[WARN]" ANSI_RESET " %s:%d: " YEL args ANSI_RESET"\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define log_info(args, ...) \
    fprintf(stdout, ANSI_RESET HWHT GRNB "[INFO]" ANSI_RESET " %s:%d: " GRN args ANSI_RESET"\n", __FILE__, __LINE__, ##__VA_ARGS__)

#else
#define log_err(args, ...)
#define log_warn(args, ...)
#define log_info(args, ...)
#endif

#define MAX_LOOKUP 20

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

    struct nbt_token_t* tok;
    int current_token;
    int parent_token;
    int max_token;

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


struct nbt_token_t* nbt_add_token(struct nbt_token_t* tok, int index, nbt_parser* parser, const struct nbt_token_t* payload, const int tok_resize_len);

nbt_type_t nbt_tok_return_type(struct nbt_token_t* token, int index, int max);
int nbt_tok_return_start(struct nbt_token_t* token, int index, int max);
int nbt_tok_return_end(struct nbt_token_t* token, int index, int max);
int nbt_tok_return_len(struct nbt_token_t* token, int index, int max);
int nbt_tok_return_parent(struct nbt_token_t* token, int index, int max);

struct nbt_metadata* nbt_add_meta(struct nbt_metadata* meta, int index, nbt_parser* parser, struct nbt_metadata* payload, const int meta_resize_len);

int nbt_meta_return_entries(nbt_parser* parser, int index);
