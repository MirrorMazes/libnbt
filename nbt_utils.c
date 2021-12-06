#include "nbt_utils.h"
#include "libnbt.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>


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

nbt_type_t nbt_return_tok_type(struct nbt_token_t* token, int index)
{
    if (index < 0 ) return 0;
    return token[index].type;
}
int nbt_return_tok_parent(struct nbt_token_t* token, int index)
{
    if (index < 0 ) return NBT_WARN;
    if (!token) return NBT_WARN;
    return token[index].parent;
}

struct nbt_token_t* nbt_init_token(int init_length, struct nbt_parser_t* parser)
{
    struct nbt_token_t* tok = calloc(sizeof(struct nbt_token_t), init_length);

    if (tok == NULL) return NULL;

    parser->max_token = init_length;
    return tok;
}

struct nbt_token_t* nbt_add_token(struct nbt_token_t* tok, int index, struct nbt_parser_t* parser, const struct nbt_token_t* payload)
{
    struct nbt_token_t* res_tok = tok;
    if (index > parser->max_token){
        res_tok = realloc(res_tok, sizeof(struct nbt_token_t) * (parser->max_token + NBT_RESIZE_LEN));

        if (!res_tok) free(tok);
        if (res_tok == NULL || (parser->max_token + NBT_RESIZE_LEN) < index) return NULL;

        parser->max_token += NBT_RESIZE_LEN;
    }
    nbt_fill_token(&res_tok[index], payload->type, payload->start, payload->end, payload->len, payload->parent);

    return res_tok;
}

struct nbt_token_t* nbt_destroy_token(struct nbt_token_t* tok, struct nbt_parser_t* parser)
{
    if (tok) free(tok);
    parser->max_token = 0;
    return NULL;
}