/* 
 *  This file is part of libnbt
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

#include "libnbt.h"

#include <stdio.h>
#include <stdint.h>

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

void swap_char_4(char* input, char* output);
void swap_char_8(char* input, char* output);

uint16_t char_to_ushort(char* input);
int16_t char_to_short(char* input);
int32_t char_to_int(char* input);
int64_t char_to_long(char* input);
int64_t char_to_long_s(char* input);
float char_to_float(char* input);
double char_to_double(char* input);

struct nbt_token_t* nbt_tokenise(struct nbt_parser_t *parser, struct nbt_token_t* nbt_tok);

struct nbt_token_t* nbt_init_token(int init_length, struct nbt_parser_t* parser);
struct nbt_token_t* nbt_add_token(struct nbt_token_t* tok, int index, struct nbt_parser_t* parser, const struct nbt_token_t* payload, const int tok_resize_len);
struct nbt_token_t* nbt_destroy_token(struct nbt_token_t* tok, struct nbt_parser_t* parser);

nbt_type_t nbt_tok_return_type(struct nbt_token_t* token, int index, int max);
int nbt_tok_return_start(struct nbt_token_t* token, int index, int max);
int nbt_tok_return_end(struct nbt_token_t* token, int index, int max);
int nbt_tok_return_len(struct nbt_token_t* token, int index, int max);
int nbt_tok_return_parent(struct nbt_token_t* token, int index, int max);

struct nbt_metadata* nbt_init_meta(int init_len, struct nbt_parser_t* parser);
struct nbt_metadata* nbt_add_meta(struct nbt_metadata* meta, int index, struct nbt_parser_t* parser, struct nbt_metadata* payload, const int meta_resize_len);
struct nbt_metadata* nbt_destroy_meta(struct nbt_metadata* meta, struct nbt_parser_t* parser);

int nbt_meta_return_entries(struct nbt_parser_t* parser, int index);


