#include "libnbt.h"
#include "nbt_utils.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>


static struct nbt_metadata get_nbt_metadata(struct nbt_parser* parser)
{
    struct nbt_metadata result;

    result.type = parser->nbt_data->content[parser->current_byte];
    parser->current_byte++;

    char entries[4];
    for (int i = 0; i < 4; i++)
    {
        entries[3 - i] = parser->nbt_data->content[parser->current_byte + i];
    }
    result.num_of_entries = (int32_t)*entries;
    parser->current_byte += 4;

    return result;
}

static int nbt_get_identifier_len(const struct nbt_parser* parser)
{
    char id_len[2];

    id_len[0] = parser->nbt_data->content[parser->current_byte];
    id_len[1] = parser->nbt_data->content[parser->current_byte+1];

    int length = char_to_short(id_len) + 2;

    return length;

}

static int nbt_get_primitive_len(const struct nbt_parser *parser, const nbt_type_t type)
{
    int len;

    switch (type) {
        case nbt_byte:
            len = 1;
            break;
        case nbt_short:
            len = 2;
            break;
        case nbt_int:
            len = 4;
            break;
        case nbt_long:
            len = 8;
            break;
        case nbt_float:
            len = 4;
            break;
        case nbt_double:
            len = 8;
            break;

        case nbt_byte_array: {
            char pr_len_ba[4];
            for (int i = 0; i < 4; i++)
            {
                pr_len_ba[i] = parser->nbt_data->content[parser->current_byte + i];
            }
            len = char_to_int(pr_len_ba) + 4;

            break;
        }

        case nbt_string: {
            char pr_len_str[2];
            pr_len_str[0] = parser->nbt_data->content[parser->current_byte];
            pr_len_str[1] = parser->nbt_data->content[parser->current_byte + 1];

            len = char_to_ushort(pr_len_str) + 2;

            break;
        }

        case nbt_int_array: {
            char pr_len_int_arr[4];
            for (int i = 0; i < 4; i++) {
                pr_len_int_arr[i] = parser->nbt_data->content[parser->current_byte + i];
            }
            len = char_to_int(pr_len_int_arr) * 4 + 4;

            break;
        }

        case nbt_long_array: {
            char pr_len_long_arr[4];
            for (int i = 0; i < 4; i++) {
                pr_len_long_arr[i] = parser->nbt_data->content[parser->current_byte + i];
            }
            len = char_to_int(pr_len_long_arr) * 8 + 4;

            break;
        }
        default:
            return 0;
    }
    return len;
}

static int nbt_parse_simple_data(struct nbt_parser* parser, nbt_tok* tok, const int tok_len)
{
    nbt_type_t current_type;
    int total_len = 0;
    int sup_token = parser->parent_token; //We need to save the parent token because the ID token is filled once only


    parser->parent_token = parser->current_token;

    parser->current_token++; //Allocates space for the ID token

    /* Getting the type ID of the sata */
    current_type = (nbt_type_t) (parser->nbt_data->content[parser->current_byte]);
    parser->current_byte++;
    total_len++;

    /* Get the identifier of the data */
    int id_len = nbt_get_identifier_len(parser);

    nbt_tok id_payload = {.type = nbt_identifier, .start = parser->current_byte, .end = parser->current_byte + id_len - 1, .len = id_len, .parent = parser->parent_token};
    if (nbt_add_token(tok, tok_len, parser->current_token, &id_payload)) return NBT_NOMEM;

    parser->current_token++;
    parser->current_byte += id_len;
    total_len += id_len;

    /* Get primitve in the data */
    int pr_len = nbt_get_primitive_len(parser, current_type);
    if (0 == pr_len) return NBT_WARN;

    nbt_tok pr_payload = {.type = nbt_primitive, .start = parser->current_byte, .end = parser->current_byte + pr_len - 1, .len = pr_len, .parent = parser->parent_token};
    if (nbt_add_token(tok, tok_len, parser->current_token, &pr_payload)) return NBT_NOMEM;

    parser->current_token++;
    parser->current_byte += pr_len;
    total_len += pr_len;

    /* Fill the ID token */
    nbt_tok _payload = {.type = current_type, .start = parser->current_byte - total_len, .end = parser->current_byte - 1, .len = total_len, .parent = sup_token};
    if (nbt_add_token(tok, tok_len, parser->parent_token, &_payload)) return NBT_NOMEM;

    parser->parent_token = sup_token; // Set parent token back to the original

    return 0;
}

static int nbt_parse_element(struct nbt_parser* parser, nbt_type_t current_type, nbt_tok* tok, const int tok_len)
{
    int total_len = 0;
    int sup_token = parser->parent_token; //We need to save the parent token because the ID token is filled once only

    parser->parent_token = parser->current_token;

    parser->current_token++; //Allocates space for the ID token

    /* Get primitve in the data */
    int pr_len = nbt_get_primitive_len(parser, current_type);
    if (0 == pr_len) return NBT_WARN;

    nbt_tok pr_payload = {.type = nbt_primitive, .start = parser->current_byte, .end = parser->current_byte + pr_len - 1, .len = pr_len, .parent = parser->parent_token};
    if (nbt_add_token(tok, tok_len, parser->current_token, &pr_payload)) return NBT_NOMEM;

    parser->current_token++;
    parser->current_byte += pr_len;
    total_len += pr_len;
    

    /* Fill the ID token */
    nbt_tok _payload = {.type = current_type, .start = parser->current_byte - total_len, .end = parser->current_byte - 1, .len = total_len, .parent = sup_token};
    if (nbt_add_token(tok, tok_len, parser->parent_token, &_payload)) return NBT_NOMEM;

    parser->parent_token = sup_token; // Set parent token back to the original

    return 0;
}

static int nbt_parse_compound_start(struct nbt_parser* parser, nbt_tok* tok, const int tok_len)
{
    
    nbt_tok _payload = {.type = nbt_compound, .start = parser->current_byte, .end = NBT_UNCHANGED, .len = NBT_UNCHANGED, .parent = parser->parent_token};
    if (nbt_add_token(tok, tok_len, parser->current_token, &_payload)) return 1;

    parser->current_byte++; // Increment because we expect current byte to be on the byte of the ID

    parser->parent_token = parser->current_token;
    parser->current_token++;
    
    int total_len = 0;
    /* Get the identifier from data */
    int id_len = nbt_get_identifier_len(parser);

    nbt_tok id_payload = {.type = nbt_identifier, .start = parser->current_byte, .end = parser->current_byte + id_len - 1, .len = id_len, .parent = parser->parent_token};
    if (nbt_add_token(tok, tok_len, parser->current_token, &id_payload)) return 1;

    parser->current_token++;
    parser->current_byte += id_len;
    total_len += id_len;

    return 0;
}

static int nbt_parse_element_compound_start(struct nbt_parser* parser, nbt_tok* tok, const int tok_len)
{
    nbt_tok _payload = {.type = nbt_compound, .start = parser->current_byte, .end = NBT_UNCHANGED, .len = NBT_UNCHANGED, .parent = parser->parent_token};
    if (nbt_add_token(tok, tok_len, parser->current_token, &_payload)) return 1;

    parser->parent_token = parser->current_token;
    parser->current_token++;

    return 0;
}

static int nbt_parse_compound_end(struct nbt_parser* parser, nbt_tok* tok, const int tok_len)
{
    int len = parser->current_byte - nbt_tok_return_start(tok, parser->parent_token, tok_len) + 1;

    nbt_tok _payload = {.type = NBT_UNCHANGED, .start = NBT_UNCHANGED, .end = parser->current_byte, .len = len, .parent = NBT_UNCHANGED};
    if (nbt_add_token(tok, tok_len, parser->parent_token, &_payload)) return 1;

    parser->parent_token = nbt_tok_return_parent(tok, parser->parent_token, tok_len);
    parser->current_byte++;

    return 0;
}

static int nbt_parse_list_start(struct nbt_parser* parser, nbt_tok* tok, const int tok_len)
{
    nbt_tok _payload = {.type = nbt_list, .start = parser->current_byte, .end = NBT_UNCHANGED, .len = NBT_UNCHANGED, .parent = parser->parent_token};
    if (nbt_add_token(tok, tok_len, parser->current_token, &_payload)) return 1;

    parser->parent_token = parser->current_token;
    parser->current_token++;
    parser->current_byte++;

    /* Get the identifier from data */
    int id_len = nbt_get_identifier_len(parser);

    nbt_tok id_payload = {.type = nbt_identifier, .start = parser->current_byte, .end = parser->current_byte + id_len - 1, .len = id_len, .parent = parser->parent_token};
    if (nbt_add_token(tok, tok_len, parser->current_token, &id_payload)) return 1;
    
    parser->current_token++;
    parser->current_byte += id_len;

    /* Get Metadata for the list */
    struct nbt_metadata meta = get_nbt_metadata(parser);
    if (parser->list_meta[parser->cur_index].type != nbt_end) parser->cur_index++;
    if (nbt_add_meta(parser->cur_index, parser, &meta)) return NBT_LNOMEM;

    return 0;
}

static int nbt_parse_element_list_start(struct nbt_parser* parser, nbt_tok* tok, const int tok_len)
{
    nbt_tok _payload = {.type = nbt_list, .start = parser->current_byte, .end = NBT_UNCHANGED, .len = NBT_UNCHANGED, .parent = parser->parent_token};
    if (nbt_add_token(tok, tok_len, parser->current_token, &_payload)) return 1;

    parser->parent_token = parser->current_token;
    parser->current_token++;

    struct nbt_metadata meta = get_nbt_metadata(parser);
    if (nbt_add_meta(parser->cur_index, parser, &meta)) return NBT_LNOMEM;

    return 0;
}

static int nbt_parse_list_end(struct nbt_parser* parser, nbt_tok* tok, const int tok_len)
{
    int len = parser->current_byte - nbt_tok_return_start(tok, parser->parent_token, tok_len);

    nbt_tok _payload = {.type = NBT_UNCHANGED, .start = NBT_UNCHANGED, .end = parser->current_byte - 1, .len = len, .parent = NBT_UNCHANGED};
    if (nbt_add_token(tok, tok_len, parser->parent_token, &_payload)) return 1;

    parser->parent_token = nbt_tok_return_parent(tok, parser->parent_token, tok_len);

    parser->list_meta[parser->cur_index].num_of_entries = NBT_NOT_AVAIL;

    return 0;
}

int nbt_tokenise(nbt_parser *parser, nbt_tok* tok, const int tok_len)
{
    for (;;)
    {
        char current_char;
        if (nbt_tok_return_type(tok, parser->parent_token, tok_len) == nbt_list) {
            current_char = parser->list_meta[parser->cur_index].type;
        }
        else {
            current_char = parser->nbt_data->content[parser->current_byte];
        }
        // debug("current char is %d, index is %d", current_char, parser->current_byte);
        switch (current_char) {
            case nbt_byte:
            case nbt_short:    
            case nbt_int:
            case nbt_long:
            case nbt_float:
            case nbt_double:
            case nbt_byte_array:
            case nbt_string:
            case nbt_int_array:
            case nbt_long_array: {
                // debug("In %s: type id is %d", __FUNCTION__ , current_char);
                
                if (nbt_tok_return_type(tok, parser->parent_token, tok_len) == nbt_list) {
                    int res = nbt_parse_element(parser, current_char, tok, tok_len);
                    if (res) return res;
                    parser->list_meta[parser->cur_index].num_of_entries--;
                }
                else {
                    int res = nbt_parse_simple_data(parser, tok, tok_len);
                    if (res) return res;
                }
                break;
            }

            case nbt_list: {
                // debug("in nbt_list");
                if (parser->list_meta[parser->cur_index].num_of_entries > 0 && nbt_tok_return_type(tok, parser->parent_token, tok_len) == nbt_list) {
                    parser->cur_index++;
                    if (nbt_parse_element_list_start(parser, tok, tok_len)) return NBT_NOMEM;
                }
                else {
                    if (nbt_parse_list_start(parser, tok, tok_len)) return NBT_NOMEM;
                }    
                break;
            }

            case nbt_compound: {
                // debug("In %s:nbt_compound", __FUNCTION__ );
                if (nbt_tok_return_type(tok, parser->parent_token, tok_len) == nbt_list) {
                    if (nbt_parse_element_compound_start(parser, tok, tok_len)) return NBT_NOMEM;
                }
                else {
                    if (nbt_parse_compound_start(parser, tok, tok_len)) return NBT_NOMEM;
                }    
                break;
            }
            /* Exit point */
            case nbt_end: {
                // debug("In %s:nbt_end",__FUNCTION__ );

                if (nbt_parse_compound_end(parser, tok, tok_len)) return NBT_NOMEM;

                if (parser->parent_token == NBT_NOT_AVAIL) return 0;

                if (nbt_tok_return_type(tok, parser->parent_token, tok_len) == nbt_list && parser->list_meta[parser->cur_index].num_of_entries != NBT_NOT_AVAIL) parser->list_meta[parser->cur_index].num_of_entries--;
                break;
            }
            default:
                return NBT_WARN;
                break;
        }

        bool has_list_end = false;
        int entries = nbt_meta_return_entries(parser, parser->cur_index);
        if (entries == 0 && parser->parent_token != NBT_NOT_AVAIL) has_list_end = true;

        while (has_list_end)
        {
            if (nbt_parse_list_end(parser, tok, tok_len)) return NBT_NOMEM;
            
            if (parser->cur_index > 0) parser->cur_index--;

            if (parser->parent_token != NBT_NOT_AVAIL && nbt_tok_return_type(tok, parser->parent_token, tok_len) == nbt_list) {
                parser->list_meta[parser->cur_index].num_of_entries--;
                if (parser->list_meta[parser->cur_index].num_of_entries > 0) {
                    has_list_end = false;
                }
            }
            else
            {
                has_list_end = false;
            }
        }
    }
}  
