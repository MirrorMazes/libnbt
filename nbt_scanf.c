#include "libnbt.h"
#include "nbt_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <byteswap.h>

#define MAX_LOOKUP 20
#define MAX_NAME_LEN 150
#define MAX_TOKEN_LEN 100 //@TODO add a mode to get number of tokens needed

struct nbt_lookup_t{
    nbt_type_t type;
    char name[MAX_NAME_LEN];
};

struct fmt_extractor_t{
    int current_byte;
    int lookup_index;
    int lookup_max;
};

void store_key(struct fmt_extractor_t *extractor, struct nbt_lookup_t* lookup, char* fmt, nbt_type_t type)
{
    char key[MAX_NAME_LEN + 1];
    for (size_t i = 0; i < MAX_NAME_LEN; i++)
    {
        char current_char = fmt[extractor->current_byte + i];
        if (current_char == '}' || current_char == ']'){
            key[i] = '\0';
            extractor->current_byte += (i + 1);
            break;
        }    
        key[i] = current_char;
    }
    key[MAX_NAME_LEN] = '\0';

    strncpy(lookup[extractor->lookup_index].name, key, MAX_NAME_LEN); 
    lookup[extractor->lookup_index].type = type;
    extractor->lookup_max++;
    extractor->lookup_index++;   
    
}
void clear_path(struct fmt_extractor_t* extractor, struct nbt_lookup_t* lookup)
{
    for (size_t i = 0; i < extractor->lookup_index; i++)
    {
        memset(lookup[extractor->lookup_index - i - 1].name, 0, MAX_NAME_LEN);
    }
    extractor->lookup_index = 0;
    extractor->lookup_max = 0;
    
}
void return_pr_key(struct fmt_extractor_t *extractor, char* fmt, char* key)
{

    for (size_t i = 0; i < MAX_NAME_LEN; i++)
    {
        char current_char = fmt[extractor->current_byte + i];
        if (current_char == '\''){
            key[i] = '\0';
            extractor->current_byte += (i + 1);
            break;
        }    
        key[i] = current_char;
    }
    key[MAX_NAME_LEN] = '\0';
    
}
char get_format(struct fmt_extractor_t *extractor, char* fmt)
{
    while (fmt[extractor->current_byte] != '%')
    {
        if (fmt[extractor->current_byte] == ' '){
            extractor->current_byte++;
            return NULL;
        }
        extractor->current_byte++;
    }
    char format = fmt[extractor->current_byte + 1];

    extractor->current_byte += 2;

    return format;
    
}

int nbt_get_identifier_index(int current_token, struct nbt_token_t* tok, int max_token)
{
    for (size_t i = 1; i < 6; i++)
    {
        if ((current_token + i) > max_token) return NBT_WARN;
        if (tok[current_token + i].type == nbt_identifier) return current_token + i;
    }
    
}
bool nbt_cmp_tok_id(int token_id, struct nbt_token_t* tok, struct nbt_parser_t* parser, char* str_2)
{
    if (tok[token_id].type != nbt_identifier) return false;

    char _str_len[2];

    for (size_t i = 0; i < 2; i++)
    {
        _str_len[i] = parser->nbt_data->content[tok[token_id].start + i];
    }

    unsigned short str_len = char_to_ushort(_str_len);

    bool result = true;

    for (size_t i = 0; i < str_len; i++)
    {
        char char_1 = parser->nbt_data->content[tok[token_id].start + 2 + i];
        
        char char_2 = str_2[i];

        if (char_1 != char_2){
            result = false;
            break;
        }

        if (char_1 == '\0' || char_2 == '\0') break;
    }
    
    return result;  
}
int nbt_get_pr_index(int current_token, struct nbt_token_t* tok)
{
    for (size_t i = 1; i < 6; i++)
    {
        if (tok[current_token + i].type == nbt_primitive) return current_token + i;
    }
}
int nbt_store_pr(void* out, char mode, char mode_2, nbt_type_t type, int current_index, struct nbt_token_t* tok, struct nbt_parser_t* parser)
{
    
    switch (mode)
    {
        case 'b':{ // byte
            if (type != nbt_byte) return -1;

            char output;

            output = parser->nbt_data->content[tok[current_index].start];
            
            if (out) *(char*)out = output;
            break;
        }    
        case 'h':{ // short 
            if (type != nbt_short) return -1;

            char output[2];
            for (size_t i = 0; i < sizeof(short); i++)
            {
                output[i] = parser->nbt_data->content[tok[current_index].start + i];
            }
            
            if (out) *(short*)out = bswap_16(char_to_short(output));
            break;
        }    
        case 'i':{ // Integer
            if (type != nbt_int) return -1;

            char output[4];
            for (size_t i = 0; i < sizeof(int); i++)
            {
                output[i] = parser->nbt_data->content[tok[current_index].start + i];
            }
            
            if (out) *(int*)out = char_to_int(output);
            break;
        }    
        case 'l':{ // Long
            if (type != nbt_long) return -1;

            char output[8];
            for (size_t i = 0; i < sizeof(long); i++)
            {
                output[i] = parser->nbt_data->content[tok[current_index].start + i];
            }
            
            if (out) *(long*)out = char_to_long(output);
            break;
        }
        case 'f':{ //Float
            if (type != nbt_float) return -1;

            char output[4];
            for (size_t i = 0; i < sizeof(float); i++)
            {
                output[i] = parser->nbt_data->content[tok[current_index].start + i];
            }
            float result = char_to_float(output);
            if (out) *(float*)out = result;
            break;
        }
        case 'd':{ // Double
            if (type != nbt_double) return -1;

            char output[8];
            for (size_t i = 0; i < sizeof(double); i++)
            {
                output[i] = parser->nbt_data->content[tok[current_index].start + i];
            }
            double result = char_to_double(output);
            if (out) *(double*)out = result;
            break;
        }
        case 's':{ // Strings
            if (type != nbt_string) return -1;

            char _len[2];
            for (size_t i = 0; i < 2; i++)
            {
                _len[i] = parser->nbt_data->content[tok[current_index].start + i];
            }
            
            unsigned short len = char_to_ushort(_len);

            if (out){
                for (size_t i = 0; i < len; i++)
                {
                    ((char*)out)[i] = parser->nbt_data->content[tok[current_index].start + i + 2];
                }
                ((char*)out)[len] = '\0';
            }
            break;
        }
        case 'n':{ // Return the sizes of data
            if (mode_2 == 's'){ // string
                if (type != nbt_string) return -1;

                char output[2];
                for (size_t i = 0; i < sizeof(short); i++)
                {
                    output[i] = parser->nbt_data->content[tok[current_index].start + i];
                }

                if (out) *(unsigned short*)out = char_to_ushort(output) + 1;
            }

            if (mode_2 == 'i' || mode_2 == 'l' || mode_2 == 'b'){ // integer and long and byte arrays
                if (type != nbt_byte_array && type != nbt_long_array && type != nbt_long_array) return -1;
                
                char output[4];
                for (size_t i = 0; i < sizeof(int); i++)
                {
                    output[i] = parser->nbt_data->content[tok[current_index].start + i];
                }
                
                if (out) *(int*)out = char_to_int(output);
                
            }

            break;
        }    
        default:
            break;
    
    }
}
void nbt_match(struct nbt_parser_t* parser, struct fmt_extractor_t* extractor, struct nbt_lookup_t* path, struct nbt_token_t* tok, char* pr_name, char mode, char mode_2, void* out, int max_token)
{
    int parent_index = NBT_NOT_AVAIL;
    int current_path = 0;
    int current_token = 0;
    bool store_data = false;
    nbt_type_t current_type = path[0].type;

    /* Get the parent token of the primitive */
    size_t i = 0;
    for (; i < max_token; i++)
    {
        if (current_path >= extractor->lookup_max){
            store_data = true;
            break;
        }

        if (nbt_return_tok_parent(tok, i) == parent_index){
            if (nbt_return_tok_type(tok, i) != path[current_path].type) continue;

            int id = nbt_get_identifier_index(i, tok, max_token);
            if (id == NBT_WARN) continue;

            if (!nbt_cmp_tok_id(id, tok, parser, path[current_path].name)) continue;

            current_path++;
            parent_index = i;
            i = id;
        }
    }
    if (!store_data) return;
    /* Get the primitive token and store data */
    for (; i < max_token; i++)
    {
        if (nbt_return_tok_parent(tok, i) == parent_index){
            int id = nbt_get_identifier_index(i, tok, max_token);
            if (id == NBT_WARN) continue;
            // debug("id is %d", id);
            if (!nbt_cmp_tok_id(id, tok, parser, pr_name)) continue;

            nbt_type_t type = tok[i].type;

            i = nbt_get_pr_index(i, tok);
            if (nbt_store_pr(out, mode, mode_2, type, i, tok, parser) == -1) log_warn("primitive not supported");
            break;
        }
    }
    

    
   
}
void nbt_scanf(struct nbt_parser_t* parser, char* fmt, ...)
{
    struct nbt_token_t* tok = nbt_init_token(50, parser);

    tok = parse_nbt(parser, tok);

    // for (int i = 0; i < parser->max_token; i++)
    // {
    //     debug("Token %d, type: %d, start: %d, end: %d, len: %d, parent: %d", i, tok[i].type, tok[i].start, tok[i].end, tok[i].len, tok[i].parent);
    // }
    
    struct nbt_lookup_t path[MAX_LOOKUP];

    char key[MAX_NAME_LEN + 1];

    /* Initialise extractir */
    struct fmt_extractor_t extractor;
    extractor.current_byte = 0;
    extractor.lookup_index = 0;
    extractor.lookup_max = 0;

    va_list ap;

    char format;
    char format_2 = 0;


    /* Get the number of variable arguments */
    int num = 0;    
    for (size_t i = 0; i < strlen(fmt); i++)
    {
        if (fmt[i] == '%') num++;
    }
    // debug("num is %d", num);



    /* Parse the format string */
    va_start(ap, num);
    while (fmt[extractor.current_byte] != '\0'){
        char current_char = fmt[extractor.current_byte];

        // debug("current index is %d", extractor.current_byte);

        switch (current_char)
        {
       
        case '{': // nbt_compound
            extractor.current_byte++;
            store_key(&extractor, &path, fmt, nbt_compound);

            break;
        case '[': // nbt_list
            extractor.current_byte++;
            store_key(&extractor, &path, fmt, nbt_list);

            break;
        case '\'':{ // identifier of primitive
            extractor.current_byte++;
            
            return_pr_key(&extractor, fmt, key);

            format = get_format(&extractor, fmt);

            if (format == 'n'){
                format_2 = fmt[extractor.current_byte];
                extractor.current_byte++;
            }   

            void* out = va_arg(ap, void*);
            nbt_match(parser, &extractor, path, tok, key, format, format_2, out, parser->max_token);

            clear_path(&extractor, path); // destroy all the elements stored in the path

            break;
        }    
        case ' ':
            extractor.current_byte++;
            break;
        default:
            extractor.current_byte++;
            break;
        }
 
    }
    tok = nbt_destroy_token(tok, parser);
    parser->list_meta = nbt_destroy_list_meta(parser->list_meta, parser);
    va_end(ap);
    // debug("Max path is %d", extractor.lookup_max);
    
}