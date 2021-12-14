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

#include "nbt_utils.h"
#include "libnbt.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>


void* nbt_realloc(void* ptr, size_t new_len, size_t original_len)
{
    void* result = malloc(new_len);

    if (!result) return result;

    memset(result, 0, new_len);

    if (new_len >= original_len){
        memcpy(result, ptr, original_len);
    }
    else{
        memcpy(result, ptr, new_len);
    }

    free(ptr);
    return result;
}

void swap_char_4(char* input, char* output)
{
    output[0] = input[3];
    output[1] = input[2];
    output[2] = input[1];
    output[3] = input[0];
}

void swap_char_8(char* input, char* output)
{
    output[0] = input[7];
    output[1] = input[6];
    output[2] = input[5];
    output[3] = input[4];
    output[4] = input[3];
    output[5] = input[2];
    output[6] = input[1];
    output[7] = input[0];
}

uint16_t char_to_ushort(char* input)
{
    int16_t result;
    memcpy(&result, input, sizeof(int16_t));
    return bswap_16(result);
}

int16_t char_to_short(char* input)
{
    uint16_t result;
    memcpy(&result, input, sizeof(uint16_t));
    return bswap_16(result);
}

int32_t char_to_int(char* input)
{
    int32_t result;
    memcpy(&result, input, sizeof(int32_t));
    return bswap_32(result);
}

int64_t char_to_long(char* input)
{
    int64_t result;
    memcpy(&result, input, sizeof(int64_t));
    return bswap_64(result);
}

float char_to_float(char* input)
{
    float result;
    char input_s[4];
    swap_char_4(input, input_s);
    memcpy(&result, input_s, sizeof(float));

    return result;
}

double char_to_double(char* input)
{
    double result;
    char input_s[8];
    swap_char_8(input, input_s);
    memcpy(&result, input_s, sizeof(double));

    return result;
}

void nbt_fill_token(struct nbt_token_t *token, nbt_type_t type, int start, int end, int len, int parent)
{
    if (type != NBT_UNCHANGED) token->type = type;
    if (start != NBT_UNCHANGED) token->start = start;
    if (end != NBT_UNCHANGED ) token->end = end;
    if (len != NBT_UNCHANGED) token->len = len;
    if (parent != NBT_UNCHANGED) token->parent = parent;

}

void fill_meta(struct nbt_metadata* metadata, nbt_type_t type, int32_t num)
{
    metadata->num_of_entries = num;
    metadata->type = type;
}

nbt_type_t nbt_tok_return_type(struct nbt_token_t* token, int index, int max)
{
    if (index < 0 || index > max || !token) return NBT_WARN;
    return token[index].type;
}

int nbt_tok_return_start(struct nbt_token_t* token, int index, int max)
{
    if (index < 0 || index > max || !token) return NBT_WARN;
    return token[index].start;
}

int nbt_tok_return_end(struct nbt_token_t* token, int index, int max)
{
    if (index < 0 || index > max || !token) return NBT_WARN;
    return token[index].end;
}

int nbt_tok_return_len(struct nbt_token_t* token, int index, int max)
{
    if (index < 0 || index > max || !token) return NBT_WARN;
    return token[index].len;
}

int nbt_tok_return_parent(struct nbt_token_t* token, int index, int max)
{
    if (index < 0 || index > max || !token) return NBT_WARN;
    return token[index].parent;
}

void nbt_init_parser(struct nbt_parser_t* parser, struct nbt_sized_buffer* content, const struct nbt_setting_t* setting)
{
    parser->nbt_data = content;

    parser->current_byte = 0;
    parser->current_token = 0;
    parser->parent_token = NBT_NOT_AVAIL;
    parser->max_token = 0;
    
    parser->cur_index = 0;
    parser->max_list = 0;

    parser->list_meta = nbt_init_meta(setting->list_meta_init_len, parser);

    parser->list_meta->num_of_entries = NBT_NOT_AVAIL;
    parser->list_meta->type = 0;

    parser->setting = setting;
}

void nbt_clear_parser(struct nbt_parser_t* parser, struct nbt_sized_buffer* content)
{
    parser->current_byte = 0;
    parser->current_token = 0;
    parser->parent_token = NBT_NOT_AVAIL;

    parser->cur_index = 0;

    memset(parser->list_meta, 0, sizeof(struct nbt_metadata) * parser->max_list);

    parser->list_meta->num_of_entries = NBT_NOT_AVAIL;
    parser->list_meta->type = 0;

    parser->nbt_data = content;
}

void nbt_destroy_parser(struct nbt_parser_t* parser)
{
    free(parser->list_meta);
    parser->list_meta = NULL;
    parser->max_list = 0;
}

struct nbt_token_t* nbt_init_token(struct nbt_parser_t* parser)
{
    struct nbt_token_t* tok = calloc(sizeof(struct nbt_token_t), parser->setting->tok_init_len + 1);

    if (tok == NULL) return NULL;

    parser->max_token = parser->setting->tok_init_len;
    return tok;
}

struct nbt_token_t* nbt_add_token(struct nbt_token_t* tok, int index, struct nbt_parser_t* parser, const struct nbt_token_t* payload, const int tok_resize_len)
{
    struct nbt_token_t* res_tok = tok;
    if (index > parser->max_token){
        res_tok = nbt_realloc(res_tok, sizeof(struct nbt_token_t) * (parser->max_token + tok_resize_len + 1), sizeof(struct nbt_token_t) * (parser->max_token + 1));

        if (res_tok == NULL || (parser->max_token + tok_resize_len) < index) return NULL;

        parser->max_token += tok_resize_len;
    }
    nbt_fill_token(&res_tok[index], payload->type, payload->start, payload->end, payload->len, payload->parent);

    return res_tok;
}

void nbt_clear_token(struct nbt_token_t* tok, struct nbt_parser_t* parser)
{
    memset(tok, 0, parser->max_token);
}

struct nbt_token_t* nbt_destroy_token(struct nbt_token_t* tok, struct nbt_parser_t* parser)
{
    if (!tok) return tok;

    free(tok);
    parser->max_token = 0;
    return NULL;
}

struct nbt_metadata* nbt_init_meta(int init_len, struct nbt_parser_t* parser)
{
    struct nbt_metadata* meta = calloc(sizeof(struct nbt_metadata), init_len + 1);

    if (!meta) return NULL;

    parser->max_list = init_len;
    return meta;
}

struct nbt_metadata* nbt_add_meta(struct nbt_metadata* meta, int index, struct nbt_parser_t* parser, struct nbt_metadata* payload, const int meta_resize_len)
{
    struct nbt_metadata* res_meta = meta;

    if (index >= parser->max_list){
        res_meta = nbt_realloc(res_meta, sizeof(struct nbt_metadata) * (parser->max_list + meta_resize_len + 1), sizeof(struct nbt_metadata) * (parser->max_list + 1));

        if (!res_meta) free(meta);
        if (!res_meta || (parser->max_list + meta_resize_len) < index) return NULL;

        parser->max_list += meta_resize_len;
    }
    fill_meta(&res_meta[index], payload->type, payload->num_of_entries);

    return res_meta;
}

struct nbt_metadata* nbt_destroy_meta(struct nbt_metadata* meta, struct nbt_parser_t* parser)
{
    if (meta) free(meta);
    parser->max_list = 0;
    return NULL;
}

int nbt_list_meta_return_entries(struct nbt_parser_t* parser, int index)
{
    if (index > parser->max_list) return NBT_WARN;
    if (index < 0) return NBT_WARN;
    if (!parser->list_meta) return NBT_WARN;

    return parser->list_meta[index].num_of_entries;
}
