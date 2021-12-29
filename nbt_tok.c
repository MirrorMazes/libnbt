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

#include "libnbt.h"
#include "nbt_utils.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>



struct nbt_metadata get_nbt_metadata(struct nbt_parser_t* parser)
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

int nbt_get_identifier_len(const struct nbt_parser_t* parser)
{
    char id_len[2];

    id_len[0] = parser->nbt_data->content[parser->current_byte];
    id_len[1] = parser->nbt_data->content[parser->current_byte+1];

    int length = char_to_short(id_len) + 2;

    return length;

}
int nbt_get_primitive_len(const struct nbt_parser_t *parser, const nbt_type_t type)
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
            log_err("Unimplemented bytecode %d, or the nbt file has been corrupted", type);
    }
    return len;
    

}

struct nbt_token_t* nbt_parse_simple_data(struct nbt_parser_t* parser, struct nbt_token_t* tok)
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

    struct nbt_token_t id_payload = {.type = nbt_identifier, .start = parser->current_byte, .end = parser->current_byte + id_len - 1, .len = id_len, .parent = parser->parent_token};
    tok = nbt_add_token(tok, parser->current_token, parser, &id_payload, parser->setting->tok_expand_len);

    parser->current_token++;
    parser->current_byte += id_len;
    total_len += id_len;

    /* Get primitve in the data */
    int pr_len = nbt_get_primitive_len(parser, current_type);

    struct nbt_token_t pr_payload = {.type = nbt_primitive, .start = parser->current_byte, .end = parser->current_byte + pr_len - 1, .len = pr_len, .parent = parser->parent_token};
    tok = nbt_add_token(tok, parser->current_token, parser, &pr_payload, parser->setting->tok_expand_len);

    parser->current_token++;
    parser->current_byte += pr_len;
    total_len += pr_len;

    /* Fill the ID token */
    struct nbt_token_t _payload = {.type = current_type, .start = parser->current_byte - total_len, .end = parser->current_byte - 1, .len = total_len, .parent = sup_token};
    tok = nbt_add_token(tok, parser->parent_token, parser, &_payload, parser->setting->tok_expand_len);

    parser->parent_token = sup_token; // Set parent token back to the original

    return tok;    
}

struct nbt_token_t* nbt_parse_element(struct nbt_parser_t* parser, struct nbt_token_t* tok, nbt_type_t current_type)
{
    int total_len = 0;
    int sup_token = parser->parent_token; //We need to save the parent token because the ID token is filled once only

    parser->parent_token = parser->current_token;

    parser->current_token++; //Allocates space for the ID token

    /* Get primitve in the data */
    int pr_len = nbt_get_primitive_len(parser, current_type);

    struct nbt_token_t pr_payload = {.type = nbt_primitive, .start = parser->current_byte, .end = parser->current_byte + pr_len - 1, .len = pr_len, .parent = parser->parent_token};
    tok = nbt_add_token(tok, parser->current_token, parser, &pr_payload, parser->setting->tok_expand_len);

    parser->current_token++;
    parser->current_byte += pr_len;
    total_len += pr_len;
    

    /* Fill the ID token */
    struct nbt_token_t _payload = {.type = current_type, .start = parser->current_byte - total_len, .end = parser->current_byte - 1, .len = total_len, .parent = sup_token};
    tok = nbt_add_token(tok, parser->parent_token, parser, &_payload, parser->setting->tok_expand_len);

    parser->parent_token = sup_token; // Set parent token back to the original

    return tok;
}

struct nbt_token_t* nbt_parse_element_compound_start(struct nbt_parser_t* parser, struct nbt_token_t* tok)
{
    struct nbt_token_t _payload = {.type = nbt_compound, .start = parser->current_byte, .end = NBT_UNCHANGED, .len = NBT_UNCHANGED, .parent = parser->parent_token};
    tok = nbt_add_token(tok, parser->current_token, parser, &_payload, parser->setting->tok_expand_len);

    parser->parent_token = parser->current_token;
    parser->current_token++;
    
    return tok;
}

struct nbt_token_t* nbt_parse_compound_start(struct nbt_parser_t* parser, struct nbt_token_t* tok)
{
    
    struct nbt_token_t _payload = {.type = nbt_compound, .start = parser->current_byte, .end = NBT_UNCHANGED, .len = NBT_UNCHANGED, .parent = parser->parent_token};
    tok = nbt_add_token(tok, parser->current_token, parser, &_payload, parser->setting->tok_expand_len);

    parser->current_byte++; // Increment because we expect current byte to be on the byte of the ID

    parser->parent_token = parser->current_token;
    parser->current_token++;
    
    int total_len = 0;
    /* Get the identifier from data */
    int id_len = nbt_get_identifier_len(parser);

    struct nbt_token_t id_payload = {.type = nbt_identifier, .start = parser->current_byte, .end = parser->current_byte + id_len - 1, .len = id_len, .parent = parser->parent_token};
    tok = nbt_add_token(tok, parser->current_token, parser, &id_payload, parser->setting->tok_expand_len);

    parser->current_token++;
    parser->current_byte += id_len;
    total_len += id_len;

    return tok;
}

struct nbt_token_t* nbt_parse_compound_end(struct nbt_parser_t* parser, struct nbt_token_t* tok)
{
    int len = parser->current_byte - nbt_tok_return_start(tok, parser->parent_token, parser->max_token) + 1;

    struct nbt_token_t _payload = {.type = NBT_UNCHANGED, .start = NBT_UNCHANGED, .end = parser->current_byte, .len = len, .parent = NBT_UNCHANGED};
    tok = nbt_add_token(tok, parser->parent_token, parser, &_payload, parser->setting->tok_expand_len);

    parser->parent_token = nbt_tok_return_parent(tok, parser->parent_token, parser->max_token);
    parser->current_byte++;

    return tok;
}

struct nbt_token_t* nbt_parse_list_start(struct nbt_parser_t* parser, struct nbt_token_t* tok)
{
    struct nbt_token_t _payload = {.type = nbt_list, .start = parser->current_byte, .end = NBT_UNCHANGED, .len = NBT_UNCHANGED, .parent = parser->parent_token};
    tok = nbt_add_token(tok, parser->current_token, parser, &_payload, parser->setting->tok_expand_len);

    parser->parent_token = parser->current_token;
    parser->current_token++;
    parser->current_byte++;

    /* Get the identifier from data */
    int id_len = nbt_get_identifier_len(parser);

    struct nbt_token_t id_payload = {.type = nbt_identifier, .start = parser->current_byte, .end = parser->current_byte + id_len - 1, .len = id_len, .parent = parser->parent_token};
    tok = nbt_add_token(tok, parser->current_token, parser, &id_payload, parser->setting->tok_expand_len);
    
    parser->current_token++;
    parser->current_byte += id_len;

    /* Get Metadata for the list */
    struct nbt_metadata meta = get_nbt_metadata(parser);
    if (parser->list_meta[parser->cur_index].type != nbt_end) parser->cur_index++;
    parser->list_meta = nbt_add_meta(parser->list_meta, parser->cur_index, parser, &meta, parser->setting->list_meta_expand_len);

    return tok;
}

struct nbt_token_t* nbt_parse_element_list_start(struct nbt_parser_t* parser, struct nbt_token_t* tok)
{
    struct nbt_token_t _payload = {.type = nbt_list, .start = parser->current_byte, .end = NBT_UNCHANGED, .len = NBT_UNCHANGED, .parent = parser->parent_token};
    tok = nbt_add_token(tok, parser->current_token, parser, &_payload, parser->setting->tok_expand_len);

    parser->parent_token = parser->current_token;
    parser->current_token++;

    struct nbt_metadata meta = get_nbt_metadata(parser);
    parser->list_meta = nbt_add_meta(parser->list_meta, parser->cur_index, parser, &meta, parser->setting->list_meta_expand_len);

    return tok;
}

struct nbt_token_t* nbt_parse_list_end(struct nbt_parser_t* parser, struct nbt_token_t* tok)
{
    int len = parser->current_byte - nbt_tok_return_start(tok, parser->parent_token, parser->max_token);

    struct nbt_token_t _payload = {.type = NBT_UNCHANGED, .start = NBT_UNCHANGED, .end = parser->current_byte - 1, .len = len, .parent = NBT_UNCHANGED};
    tok = nbt_add_token(tok, parser->parent_token, parser, &_payload, parser->setting->tok_expand_len);

    parser->parent_token = nbt_tok_return_parent(tok, parser->parent_token, parser->max_token);

    parser->list_meta[parser->cur_index].num_of_entries = NBT_NOT_AVAIL;

    return tok;
}

struct nbt_token_t* nbt_tokenise(struct nbt_parser_t *parser, struct nbt_token_t* nbt_tok)
{
    struct nbt_token_t* tok = nbt_tok;

    for(;;)
    {
        char current_char;
        if (nbt_tok_return_type(tok, parser->parent_token, parser->max_token) == nbt_list) {
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
                
                if (nbt_tok_return_type(tok, parser->parent_token, parser->max_token) == nbt_list) {
                    tok = nbt_parse_element(parser, tok, current_char);
                    parser->list_meta[parser->cur_index].num_of_entries--;
                }
                else {
                    tok = nbt_parse_simple_data(parser, tok);
                }
                break;
            }

            case nbt_list: {
                // debug("in nbt_list");
                if (parser->list_meta[parser->cur_index].num_of_entries > 0 && nbt_tok_return_type(tok, parser->parent_token, parser->max_token) == nbt_list) {
                    parser->cur_index++;
                    tok = nbt_parse_element_list_start(parser, tok);
                }
                else {
                    tok = nbt_parse_list_start(parser, tok);
                }    
                break;
            }

            case nbt_compound: {
                // debug("In %s:nbt_compound", __FUNCTION__ );
                if (nbt_tok_return_type(tok, parser->parent_token, parser->max_token) == nbt_list) {
                    tok = nbt_parse_element_compound_start(parser, tok);
                }
                else {
                    tok = nbt_parse_compound_start(parser, tok);
                }    
                break;
            }
            /* Exit point */
            case nbt_end: {
                // debug("In %s:nbt_end",__FUNCTION__ );

                tok = nbt_parse_compound_end(parser, tok);

                if (parser->parent_token == NBT_NOT_AVAIL) return tok;

                if (nbt_tok_return_type(tok, parser->parent_token, parser->max_token) == nbt_list && parser->list_meta[parser->cur_index].num_of_entries != NBT_NOT_AVAIL) parser->list_meta[parser->cur_index].num_of_entries--;
                break;
            }
            default:
                log_err("unknown byte %d, at position %d", current_char, parser->current_byte);
                abort(); /* @TODO: return an error code instead of aborting */
                break;
        }

        bool has_list_end = false;
        int entries = nbt_meta_return_entries(parser, parser->cur_index);
        if (entries == 0 && parser->parent_token != NBT_NOT_AVAIL) has_list_end = true;

        while (has_list_end)
        {
            tok = nbt_parse_list_end(parser, tok);
            
            if (parser->cur_index > 0) parser->cur_index--;

            if (parser->parent_token != NBT_NOT_AVAIL && nbt_tok_return_type(tok, parser->parent_token, parser->max_token) == nbt_list) {
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
