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
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <byteswap.h>

static bool check_if_in_list(struct nbt_lookup_t* lookup, int index)
{
    if (index <= 0) return false;

    if (lookup[index - 1].type != nbt_list) return false;

    return true;
}

static int nbt_get_identifier_index(int current_token, struct nbt_token_t* tok, int max_token)
{
    for (size_t i = 1; i < 6; i++)
    {
        if (nbt_tok_return_type(tok, current_token + i, max_token) == nbt_identifier) return current_token + i;
    }
    return NBT_WARN;
}

static int nbt_get_pr_index(int current_token, struct nbt_token_t* tok, int max_token)
{
    for (size_t i = 0; i < 3; i++)
    {
        if (nbt_tok_return_type(tok, current_token + i, max_token) == nbt_primitive) return current_token + i;
    }
    return NBT_WARN;
}

static bool nbt_cmp_tok_id(int token_id, struct nbt_token_t* tok, nbt_parser* parser, char* str_2)
{
    if (nbt_tok_return_type(tok, token_id, parser->max_token) != nbt_identifier) return false;

    char _str_len[2];

    for (size_t i = 0; i < 2; i++)
    {
        _str_len[i] = parser->nbt_data->content[nbt_tok_return_start(tok, token_id, parser->max_token) + i];
    }

    unsigned short str_len = char_to_ushort(_str_len);

    bool result = true;

    for (size_t i = 0; i < str_len; i++)
    {
        char char_1 = parser->nbt_data->content[nbt_tok_return_start(tok, token_id, parser->max_token) + 2 + i];
        
        char char_2 = str_2[i];

        if (char_1 != char_2) {
            result = false;
            break;
        }

        if (char_1 == '\0' || char_2 == '\0') break;
    }
    
    return result;  
}

static int nbt_store_pr(void* out, char mode, char mode_2, nbt_type_t type, int current_index, nbt_parser* parser)
{
    
    switch (mode)
    {
        case 'b': { // byte
            if (type != nbt_byte) return -1;

            char output;

            output = parser->nbt_data->content[nbt_tok_return_start(parser->tok, current_index, parser->max_token)];
            
            if (out) *(char*)out = output;
            break;
        }    
        case 'h': { // short 
            if (type != nbt_short) return -1;

            char output[2];
            for (size_t i = 0; i < sizeof(short); i++)
            {
                output[i] = parser->nbt_data->content[nbt_tok_return_start(parser->tok, current_index, parser->max_token) + i];
            }
            
            if (out) *(short*)out = bswap_16(char_to_short(output));
            break;
        }    
        case 'i': { // Integer
            if (type != nbt_int) return -1;

            char output[4];
            for (size_t i = 0; i < sizeof(int); i++)
            {
                output[i] = parser->nbt_data->content[nbt_tok_return_start(parser->tok, current_index, parser->max_token) + i];
            }
            
            if (out) *(int*)out = char_to_int(output);
            break;
        }    
        case 'l': { // Long
            if (type != nbt_long) return -1;

            char output[8];
            for (size_t i = 0; i < sizeof(long); i++)
            {
                output[i] = parser->nbt_data->content[nbt_tok_return_start(parser->tok, current_index, parser->max_token) + i];
            }
            
            if (out) *(long*)out = char_to_long(output);
            break;
        }
        case 'f': { //Float
            if (type != nbt_float) return -1;

            char output[4];
            for (size_t i = 0; i < sizeof(float); i++)
            {
                output[i] = parser->nbt_data->content[nbt_tok_return_start(parser->tok, current_index, parser->max_token) + i];
            }
            float result = char_to_float(output);
            if (out) *(float*)out = result;
            break;
        }
        case 'd': { // Double
            if (type != nbt_double) return -1;

            char output[8];
            for (size_t i = 0; i < sizeof(double); i++)
            {
                output[i] = parser->nbt_data->content[nbt_tok_return_start(parser->tok, current_index, parser->max_token) + i];
            }
            double result = char_to_double(output);
            if (out) *(double*)out = result;
            break;
        }
        case 's': { // Strings
            if (type != nbt_string) return -1;

            char _len[2];
            for (size_t i = 0; i < 2; i++)
            {
                _len[i] = parser->nbt_data->content[nbt_tok_return_start(parser->tok, current_index, parser->max_token) + i];
            }
            
            unsigned short len = char_to_ushort(_len);

            if (out) {
                for (size_t i = 0; i < len; i++)
                {
                    ((char*)out)[i] = parser->nbt_data->content[nbt_tok_return_start(parser->tok, current_index, parser->max_token) + i + 2];
                }
                ((char*)out)[len] = '\0';
            }
            break;
        }
        case 'n': { // Return the sizes of data
            if (mode_2 == 's') { // string
                if (type != nbt_string) return -1;

                char output[2];
                for (size_t i = 0; i < sizeof(short); i++)
                {
                    output[i] = parser->nbt_data->content[nbt_tok_return_start(parser->tok, current_index, parser->max_token) + i];
                }

                if (out) *(unsigned short*)out = char_to_ushort(output) + 1;
            }

            if (mode_2 == 'i' || mode_2 == 'l' || mode_2 == 'b') { // integer and long and byte arrays
                if (type != nbt_byte_array && type != nbt_long_array && type != nbt_long_array) return -1;
                
                char output[4];
                for (size_t i = 0; i < sizeof(int); i++)
                {
                    output[i] = parser->nbt_data->content[nbt_tok_return_start(parser->tok, current_index, parser->max_token) + i];
                }
                
                if (out) *(int*)out = char_to_int(output);
            }

            if (mode_2 == 't') { // List
                if (type != nbt_identifier || type != nbt_list || type != nbt_compound) return -1;

                char output[4];
                int init_index = 0;

                if (type == nbt_identifier) {
                    init_index = nbt_tok_return_end(parser->tok, current_index, parser->max_token) + 2;
                }
                else {
                    init_index = nbt_tok_return_start(parser->tok, current_index, parser->max_token) + 1;
                }

                for (size_t i = 0; i < sizeof(int); i++)
                {
                    output[i] = parser->nbt_data->content[init_index + i];
                }
                if (out) *(int*)out = char_to_int(output);
            }

            break;
        }    
        default:
            break;
    }
    return 0;
}

int nbt_find(nbt_parser* parser, struct nbt_lookup_t* path, int path_size, struct nbt_index_t* res)
{   
    int parent_index = NBT_NOT_AVAIL;
    int current_path = 0;
    
    int i = 0;
    for (; i < parser->max_token; i++)
    {
        if (nbt_tok_return_parent(parser->tok, i, parser->max_token) != parent_index) continue;
        if (nbt_tok_return_type(parser->tok, i, parser->max_token) != path[current_path].type) continue;

        if (check_if_in_list(path, current_path) == false) { // Current token is not in a list

            /* Check the name of the token */
            int id = nbt_get_identifier_index(i, parser->tok, parser->max_token);
            if (id == NBT_WARN) continue;

            if (!nbt_cmp_tok_id(id, parser->tok, parser, path[current_path].name)) continue;

            current_path++;
            parent_index = i;
        }
        else{ // Current token is in a list
            /* Check the number of tokens to skip */

            // debug("List detected. Current path %d");
            if (path[current_path - 1].index <= 0) {
                current_path++;
                parent_index = i;
            }
            else {
                path[current_path - 1].index--;
            }    
        }
        
        if (current_path == path_size) {
            if (path[current_path - 1].type != nbt_compound && path[current_path - 1].type != nbt_list) {
                i = nbt_get_pr_index(i, parser->tok, parser->max_token);
            }

            res->start = nbt_tok_return_start(parser->tok, i, parser->max_token);
            res->end = nbt_tok_return_end(parser->tok, i, parser->max_token);
            res->len = nbt_tok_return_len(parser->tok, i, parser->max_token);

            return 0;
        }  
    }
    return NBT_WARN;
}
