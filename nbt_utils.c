#include "libnbt.h"
#include "nbt_utils.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>


void* nbt_realloc(void* ptr, size_t new_len, size_t original_len)
{
    void* result = malloc(new_len);

    if (!result) return result;

    memset(result, 0, new_len);

    if (new_len >= original_len) {
        memcpy(result, ptr, original_len);
    }
    else {
        memcpy(result, ptr, new_len);
    }

    free(ptr);
    return result;
}

void swap_char_2(char* input, char* output)
{
    output[0] = input[1];
    output[1] = input[0];
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

void safe_swap_4(char* input, char* output)
{
    char buf[4];
    swap_char_4(input, buf);
    memcpy(output, buf, 4);
}

void safe_swap_8(char* input, char* output)
{
    char buf[8];
    swap_char_8(input, buf);
    memcpy(output, buf, 8);
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

void nbt_fill_token(nbt_tok *token, nbt_type_t type, int start, int end, int len, int parent)
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

nbt_type_t nbt_tok_return_type(nbt_tok* token, int index, int max)
{
    if (index < 0 || index > max || !token) return NBT_WARN;
    return token[index].type;
}

int nbt_tok_return_start(nbt_tok* token, int index, int max)
{
    if (index < 0 || index > max || !token) return NBT_WARN;
    return token[index].start;
}

int nbt_tok_return_end(nbt_tok* token, int index, int max)
{
    if (index < 0 || index > max || !token) return NBT_WARN;
    return token[index].end;
}

int nbt_tok_return_len(nbt_tok* token, int index, int max)
{
    if (index < 0 || index > max || !token) return NBT_WARN;
    return token[index].len;
}

int nbt_tok_return_parent(nbt_tok* token, int index, int max)
{
    if (index < 0 || index > max || !token) return NBT_WARN;
    return token[index].parent;
}

static struct nbt_metadata* nbt_init_meta(struct nbt_parser* parser)
{
    void* (*alloc) (size_t size);
    if (parser->setting->alloc) {
        alloc = parser->setting->alloc;
    }    
    else {
        alloc = malloc;
    }    

    struct nbt_metadata* meta = alloc(sizeof(struct nbt_metadata) * (parser->setting->list_meta_init_len + 1));
    if (!meta) return NULL;

    parser->max_list = parser->setting->list_meta_init_len;
    return meta;
}

static struct nbt_metadata* nbt_destroy_meta(struct nbt_metadata* meta, struct nbt_parser* parser)
{
    void (*free_) (void* mem);
    if (parser->setting->free) {
        free_ = parser->setting->free;
    }
    else {
        free_ = free;
    }

    if (meta) free_(meta);
    parser->max_list = 0;
    return NULL;
}

static nbt_tok* nbt_init_token(const int init_len, struct nbt_parser* parser)
{
    nbt_tok* tok = calloc(sizeof(nbt_tok), init_len + 1);

    if (tok == NULL) return NULL;

    return tok;
}


int nbt_add_meta(int index, struct nbt_parser* parser, struct nbt_metadata* payload)
{
    if (index >= parser->max_list) return NBT_LNOMEM;
    fill_meta(&parser->list_meta[index], payload->type, payload->num_of_entries);

    return 0;
}

int nbt_add_token(nbt_tok* tok, const int tok_len, int index, const nbt_tok* payload)
{
    if (index > tok_len) return 1;

    nbt_fill_token(&tok[index], payload->type, payload->start, payload->end, payload->len, payload->parent);
    return 0;
}

void nbt_init_parser(struct nbt_parser* parser, struct nbt_sized_buffer* content, const struct nbt_parser_setting_t* setting)
{
    parser->setting = setting;
    
    parser->nbt_data = content;

    parser->current_byte = 0;
    parser->current_token = 0;
    parser->parent_token = NBT_NOT_AVAIL;
    
    parser->cur_index = 0;
    parser->max_list = 0;

    parser->list_meta = nbt_init_meta(parser);

    parser->list_meta->num_of_entries = NBT_NOT_AVAIL;
    parser->list_meta->type = 0;
}

void nbt_clear_parser(struct nbt_parser* parser, struct nbt_sized_buffer* content)
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

void nbt_destroy_parser(struct nbt_parser* parser)
{
    nbt_destroy_meta(parser->list_meta, parser);
    parser->list_meta = NULL;
}

int nbt_meta_return_entries(struct nbt_parser* parser, int index)
{
    if (index > parser->max_list) return NBT_WARN;
    if (index < 0) return NBT_WARN;
    if (!parser->list_meta) return NBT_WARN;

    return parser->list_meta[index].num_of_entries;
}
