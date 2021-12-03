#pragma once

#include "libnbt.h"

#include <stdio.h>
#include <stdint.h>

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

int parse_nbt(struct nbt_parser_t *parser, struct nbt_token_t* nbt_tok);


