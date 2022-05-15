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

static int nbt_get_identifier_index(int current_token, nbt_tok* tok, int max_token)
{
    for (size_t i = 1; i < 6; i++)
    {
        if (nbt_tok_return_type(tok, current_token + i, max_token) == nbt_identifier) return current_token + i;
    }
    return NBT_WARN;
}

static int nbt_get_pr_index(int current_token, nbt_tok* tok, int max_token)
{
    for (size_t i = 0; i < 3; i++)
    {
        if (nbt_tok_return_type(tok, current_token + i, max_token) == nbt_primitive) return current_token + i;
    }
    return NBT_WARN;
}

static bool nbt_cmp_tok_id(int token_id, nbt_tok* tok, const int tok_len, nbt_parser* parser, char* str_2)
{
    if (nbt_tok_return_type(tok, token_id, tok_len) != nbt_identifier) return false;

    char _str_len[2];

    for (size_t i = 0; i < 2; i++)
    {
        _str_len[i] = parser->nbt_data->content[nbt_tok_return_start(tok, token_id, tok_len) + i];
    }

    unsigned short str_len = char_to_ushort(_str_len);

    bool result = true;

    for (size_t i = 0; i < str_len; i++)
    {
        char char_1 = parser->nbt_data->content[nbt_tok_return_start(tok, token_id, tok_len) + 2 + i];
        
        char char_2 = str_2[i];

        if (char_1 != char_2) {
            result = false;
            break;
        }

        if (char_1 == '\0' || char_2 == '\0') break;
    }
    
    return result;  
}


int nbt_find(nbt_tok* tok, const int tok_len, nbt_parser* parser, struct nbt_lookup_t* path, int path_size, struct nbt_index_t* res)
{   
    int parent_index = NBT_NOT_AVAIL;
    int current_path = 0;
    
    int i = 0;
    for (; i < tok_len; i++)
    {
        if (nbt_tok_return_parent(tok, i, tok_len) != parent_index) continue;
        if (nbt_tok_return_type(tok, i, tok_len) != path[current_path].type) continue;

        if (check_if_in_list(path, current_path) == false) { // Current token is not in a list

            /* Check the name of the token */
            int id = nbt_get_identifier_index(i, tok, tok_len);
            if (id == NBT_WARN) continue;

            if (!nbt_cmp_tok_id(id, tok, tok_len, parser, path[current_path].name)) continue;

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
                i = nbt_get_pr_index(i, tok, tok_len);
            }

            res->start = nbt_tok_return_start(tok, i, tok_len);
            res->end = nbt_tok_return_end(tok, i, tok_len);
            res->len = nbt_tok_return_len(tok, i, tok_len);

            return 0;
        }  
    }
    return NBT_WARN;
}
