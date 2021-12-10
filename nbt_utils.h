#pragma once

#include "libnbt.h"

#include <stdio.h>
#include <stdint.h>

#define NBT_RESIZE_LEN 25
#define NBT_META_RESIZE_LEN 10

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

nbt_type_t nbt_return_tok_type(struct nbt_token_t* token, int index, int max);
int nbt_return_tok_start(struct nbt_token_t* token, int index, int max);
int nbt_return_tok_end(struct nbt_token_t* token, int index, int max);
int nbt_return_tok_len(struct nbt_token_t* token, int index, int max);
int nbt_return_tok_parent(struct nbt_token_t* token, int index, int max);

struct nbt_token_t* parse_nbt(struct nbt_parser_t *parser, struct nbt_token_t* nbt_tok);

struct nbt_token_t* nbt_init_token(int init_length, struct nbt_parser_t* parser);
struct nbt_token_t* nbt_add_token(struct nbt_token_t* tok, int index, struct nbt_parser_t* parser, const struct nbt_token_t* payload);
struct nbt_token_t* nbt_destroy_token(struct nbt_token_t* tok, struct nbt_parser_t* parser);

struct nbt_metadata* nbt_init_list_meta(int init_len, struct nbt_parser_t* parser);
struct nbt_metadata* nbt_add_list_meta(struct nbt_metadata* meta, int index, struct nbt_parser_t* parser, struct nbt_metadata* payload);
struct nbt_metadata* nbt_destroy_list_meta(struct nbt_metadata* meta, struct nbt_parser_t* parser);

int nbt_list_meta_return_entries(struct nbt_parser_t* parser, int index);

