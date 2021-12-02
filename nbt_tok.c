#include "libnbt.h"
#include "nbt_utils.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>



struct nbt_metadata get_nbt_metadata(struct nbt_parser_t *parser)
{
    struct nbt_metadata result;

    result.type = parser->nbt_data->content[parser->current_byte];
    parser->current_byte++;

    char entries[4];
    for (int i = 0; i < 4; i++){
        entries[3 - i] = parser->nbt_data->content[parser->current_byte + i];
    }
    result.num_of_entries = (int32_t)*entries;
    parser->current_byte += 4;

    return result;
}

void nbt_init_parser(struct nbt_parser_t *parser, struct nbt_sized_buffer* content)
{
    parser->nbt_data = content;


    parser->current_byte = 0;
    parser->current_token = 0;
    parser->parent_token = NBT_NOT_AVAIL;
    
    parser->cur_index = 0;
    parser->list_meta->num_of_entries = NBT_NOT_AVAIL;
    parser->list_meta->type = 0;
    parser->store_token = true;
}

void nbt_fill_token(struct nbt_token_t *token, nbt_type_t type, int start, int end, int len, int parent)
{
    if (type != NBT_UNCHANGED) token->type = type;
    if (start != NBT_UNCHANGED) token->start = start;
    if (end != NBT_UNCHANGED ) token->end = end;
    if (len != NBT_UNCHANGED) token->len = len;
    if (parent != NBT_UNCHANGED) token->parent = parent;

}


int nbt_get_identifier_len(const struct nbt_parser_t *parser)
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

        case nbt_byte_array:{
            char pr_len_ba[4];
            for (int i = 0; i < 4; i++){
                pr_len_ba[i] = parser->nbt_data->content[parser->current_byte + i];
            }
            len = char_to_int(pr_len_ba) + 4;

            break;
        }

        case nbt_string:{
            char pr_len_str[2];
            pr_len_str[0] = parser->nbt_data->content[parser->current_byte];
            pr_len_str[1] = parser->nbt_data->content[parser->current_byte + 1];

            len = char_to_ushort(pr_len_str) + 2;

            break;
        }
        case nbt_int_array:{
            char pr_len_int_arr[4];
            for (int i = 0; i < 4; i++) {
                pr_len_int_arr[i] = parser->nbt_data->content[parser->current_byte + i];
            }
            len = char_to_int(pr_len_int_arr) * 4 + 4;

            break;
        }
        case nbt_long_array:{
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

void nbt_parse_simple_data(struct nbt_parser_t* parser, struct nbt_token_t* tok)
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

    nbt_fill_token(&tok[parser->current_token], nbt_identifier, parser->current_byte, parser->current_byte + id_len - 1, id_len, parser->parent_token);

    parser->current_token++;
    parser->current_byte += id_len;
    total_len += id_len;

    /* Get primitve in the data */
    int pr_len = nbt_get_primitive_len(parser, current_type);

    nbt_fill_token(&tok[parser->current_token], nbt_primitive, parser->current_byte, parser->current_byte + pr_len - 1, pr_len, parser->parent_token);

    parser->current_token++;
    parser->current_byte += pr_len;
    total_len += pr_len;

    /* Fill the ID token */
    nbt_fill_token(&tok[parser->parent_token], current_type, parser->current_byte - total_len, parser->current_byte - 1, total_len, sup_token);


    parser->parent_token = sup_token; // Set parent token back to the original
    
}

void nbt_parse_list_element(struct nbt_parser_t* parser, struct nbt_token_t* tok, nbt_type_t current_type)
{
    int total_len = 0;
    int sup_token = parser->parent_token; //We need to save the parent token because the ID token is filled once only

    parser->parent_token = parser->current_token;

    parser->current_token++; //Allocates space for the ID token

    /* Get primitve in the data */
    int pr_len = nbt_get_primitive_len(parser, current_type);

    nbt_fill_token(&tok[parser->current_token], nbt_primitive, parser->current_byte, parser->current_byte + pr_len - 1, pr_len, parser->parent_token);

    parser->current_token++;
    parser->current_byte += pr_len;
    total_len += pr_len;
    

    /* Fill the ID token */
    nbt_fill_token(&tok[parser->parent_token], current_type, parser->current_byte - total_len, parser->current_byte - 1, total_len, sup_token);

    parser->parent_token = sup_token; // Set parent token back to the original
}

void nbt_parse_list_compound_element_start(struct nbt_parser_t* parser, struct nbt_token_t* tok)
{
    nbt_fill_token(&tok[parser->current_token], nbt_compound, parser->current_byte, NBT_UNCHANGED, NBT_UNCHANGED, parser->parent_token);

    parser->parent_token = parser->current_token;
    parser->current_token++;
    
}
void nbt_parse_compound_start(struct nbt_parser_t* parser, struct nbt_token_t* tok)
{
    
    nbt_fill_token(&tok[parser->current_token], nbt_compound, parser->current_byte, NBT_UNCHANGED, NBT_UNCHANGED, parser->parent_token);

    parser->current_byte++; // Increment because we expect current byte to be on the byte of the ID

    parser->parent_token = parser->current_token;
    parser->current_token++;
    
    int total_len = 0;
    /* Get the identifier from data */
    int id_len = nbt_get_identifier_len(parser);

    nbt_fill_token(&tok[parser->current_token], nbt_identifier, parser->current_byte, parser->current_byte + id_len - 1, id_len, parser->parent_token);

    parser->current_token++;
    parser->current_byte += id_len;
    total_len += id_len;

    
}
void nbt_parse_compound_end(struct nbt_parser_t* parser, struct nbt_token_t* tok)
{
    int len = parser->current_byte - tok[parser->parent_token].start + 1;

    nbt_fill_token(&tok[parser->parent_token], NBT_UNCHANGED, NBT_UNCHANGED, parser->current_byte, len, NBT_UNCHANGED); 

    parser->parent_token = tok[parser->parent_token].parent;
    parser->current_byte++;

}

void nbt_parse_list_start(struct nbt_parser_t* parser, struct nbt_token_t* tok)
{
    nbt_fill_token(&tok[parser->current_token], nbt_list, parser->current_byte, NBT_UNCHANGED, NBT_UNCHANGED, parser->parent_token);

    parser->parent_token = parser->current_token;
    parser->current_token++;
    parser->current_byte++;

    /* Get the identifier from data */
    int id_len = nbt_get_identifier_len(parser);

    nbt_fill_token(&tok[parser->current_token], nbt_identifier, parser->current_byte, parser->current_byte + id_len - 1, id_len, parser->parent_token);

    parser->current_token++;
    parser->current_byte += id_len;


    struct nbt_metadata meta = get_nbt_metadata(parser);

    parser->list_meta[parser->cur_index] = meta;
}
void nbt_parse_element_list_start(struct nbt_parser_t* parser, struct nbt_token_t* tok)
{
    nbt_fill_token(&tok[parser->current_token], nbt_list, parser->current_byte, NBT_UNCHANGED, NBT_UNCHANGED, parser->parent_token);

    parser->parent_token = parser->current_token;
    parser->current_token++;

    struct nbt_metadata meta = get_nbt_metadata(parser);

    parser->list_meta[parser->cur_index] = meta;
}
void nbt_parse_list_end(struct nbt_parser_t* parser, struct nbt_token_t* tok)
{
    int len = parser->current_byte - tok[parser->parent_token].start;

    nbt_fill_token(&tok[parser->parent_token], NBT_UNCHANGED, NBT_UNCHANGED, parser->current_byte - 1, len, NBT_UNCHANGED); 

    parser->parent_token = tok[parser->parent_token].parent;

    parser->list_meta[parser->cur_index].num_of_entries = NBT_NOT_AVAIL;
}


int parse_nbt(struct nbt_parser_t *parser, struct nbt_token_t* nbt_tok)
{
    for(;;){
        char current_char;
        if (nbt_tok[parser->parent_token].type == nbt_list){
            current_char = parser->list_meta[parser->cur_index].type;
        }
        else{
            current_char = parser->nbt_data->content[parser->current_byte];
        }
        
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
                
                if (nbt_tok[parser->parent_token].type == nbt_list){
                    nbt_parse_list_element(parser, nbt_tok, current_char);
                    parser->list_meta[parser->cur_index].num_of_entries--;
                }
                else{
                    nbt_parse_simple_data(parser, nbt_tok);
                }
                

                break;
            }

            case nbt_list: {
                // debug("in nbt_list");
                if (parser->list_meta[parser->cur_index].num_of_entries > 0){
                    parser->cur_index++;
                    nbt_parse_element_list_start(parser, nbt_tok);
                }
                else {
                    nbt_parse_list_start(parser, nbt_tok);
                }    
                break;
            }

            case nbt_compound: {
                // debug("In %s:nbt_compound", __FUNCTION__ );
                if (nbt_tok[parser->parent_token].type == nbt_list) {
                    nbt_parse_list_compound_element_start(parser, nbt_tok);
                }
                else {
                    nbt_parse_compound_start(parser, nbt_tok);
                }    
                

                break;
            }
            //Exit point
            case nbt_end: {
                // debug("In %s:nbt_end",__FUNCTION__ );

                nbt_parse_compound_end(parser, nbt_tok);

                if (parser->parent_token == NBT_NOT_AVAIL) return 0;

                if (nbt_tok[parser->parent_token].type == nbt_list) parser->list_meta[parser->cur_index].num_of_entries--;
                break;
            }
        }
        bool has_list_end = false;
        if (parser->list_meta[parser->cur_index].num_of_entries == 0) has_list_end = true;

        while (has_list_end){
            nbt_parse_list_end(parser, nbt_tok);
            if (parser->cur_index > 0) parser->cur_index--;
            if (parser->parent_token != NBT_NOT_AVAIL && nbt_tok[parser->parent_token].type == nbt_list){

                // If parent token is nbt_list
                // It is implied that the type is nbt_list, hence decrement entries

                parser->list_meta[parser->cur_index].num_of_entries--;
                if (parser->list_meta[parser->cur_index].num_of_entries == 0){
                    has_list_end = true;
                }
                else {
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
