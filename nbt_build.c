#include "libnbt.h"
#include "nbt_utils.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>





static int nbt_add_raw_nbt(nbt_build* b, const int skip_size, const char* bytes, const int bytes_size, char* buf, const int buf_size)
{
     if (skip_size > b->offset) return -1;

    /* Calculate new length */
    int new_len = b->offset + bytes_size;

    /* Check if it exceeds buffer */
    if (new_len >= buf_size) return -1;
    
    /* Copy bytes to the back */
    if (b->offset != skip_size) memmove(buf + skip_size + bytes_size, buf + skip_size, b->offset - skip_size);

    /* Copy bytes into buffer */
    memcpy(buf + skip_size, bytes, bytes_size);

    /* Calculate any excess bytes */
    if (skip_size + bytes_size > b->offset) b->offset = skip_size + bytes_size;

    return 0;
}

static int nbt_allocate_raw_nbt(nbt_build* b, const int skip_size, const int bytes_size, char* buf, const int buf_size)
{
    if (skip_size > b->offset) return -1;

    /* Calculate new length */
    int new_len = b->offset + bytes_size;

    /* Check if it exceeds buffer */
    if (new_len >= buf_size) return -1;
    
    /* Copy bytes to the back */
    if (b->offset != skip_size) memmove(buf + skip_size + bytes_size, buf + skip_size, b->offset - skip_size);
}

static int nbt_replace_raw_nbt(nbt_build* b, const int skip_size, const char* bytes, const int bytes_size, char* buf, const int buf_size)
{ 
    /* Copy bytes into buffer */
    memcpy(buf + skip_size, bytes, bytes_size);

    /* Calculate any excess bytes */
    if (skip_size + bytes_size > b->offset) b->offset = skip_size + bytes_size;
    return 0;
}

static int nbtb_incre_state(nbt_build* b, enum nbtb_state_type new_state)
{
    if (b->current_depth >= MAX_DEPTH) return -1;
    b->top++;
    b->current_depth++;

    b->top->state = new_state;

    return 0;
}

static int nbt_decre_state(nbt_build* b)
{
    if (b->current_depth == 0) return 0;
    b->top--;
    b->current_depth--;

    return 0;
}

int nbt_init_build(nbt_build* b)
{
    memset(b->stack, 0, MAX_DEPTH * sizeof(struct nbtb_state));
    b->current_depth = 0;

    b->top = &b->stack[0];
    b->offset = 0;
}

int nbt_start_compound(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len)
{
    /* update state */
    switch (b->top->state)
    {
    case S_CMP:
        if (nbtb_incre_state(b, S_OBJ_OR_CLOSE)) return -1;
        break;
    
    default:
        return -1;
        break;
    }

    int total_len = 1 + 2 + name_len;

    nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len);

    char id = nbt_compound;
    char len[2];
    swap_char_2((char*)&name_len, len);

    nbt_replace_raw_nbt(b, b->offset, &id, 1, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, len, 2, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, name, name_len, buf, buf_len);

    return 0;
}

int nbt_end_compound(nbt_build* b, char* buf, const int buf_len)
{
    /* update state */
    switch (b->top->state)
    {
    case S_CMP:
        return 0;
        break;
    case S_OBJ_OR_CLOSE:
        if (nbt_decre_state(b)) return -1;
        break;
    default:
        return -1;
        break;
    }

    char payload = nbt_end;

    nbt_add_raw_nbt(b, b->offset, &payload, 1, buf, buf_len);

    return 0;
}

int nbt_add_char(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, char payload)
{
    /* update state */
    switch (b->top->state)
    {
    case S_OBJ_OR_CLOSE:
        break;
    default:
        return -1;
        break;
    }

    int total_len = 1 + 2 + name_len + 1;

    char id = nbt_byte;

    char _name_len[2];
    swap_char_2((char*)&name_len, _name_len);

    char nbt = payload;

    nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, &id, 1, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, _name_len, 2, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, name, name_len, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, &nbt, 1, buf, buf_len);

    return 0;
}

int nbt_add_short(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, short payload)
{
    /* update state */
    switch (b->top->state)
    {
    case S_OBJ_OR_CLOSE:
        break;
    default:
        return -1;
        break;
    }

    int total_len = 1 + 2 + name_len + 2;

    char id = nbt_short;

    char _name_len[2];
    swap_char_2((char*)&name_len, _name_len);

    char nbt[4];
    swap_char_2((char*)&payload, nbt);

    nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, &id, 1, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, _name_len, 2, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, name, name_len, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, nbt, 2, buf, buf_len);

    return 0;
}

int nbt_add_integer(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, int payload)
{
    /* update state */
    switch (b->top->state)
    {
    case S_OBJ_OR_CLOSE:
        break;
    default:
        return -1;
        break;
    }

    int total_len = 1 + 2 + name_len + 4;

    char id = nbt_int;

    char _name_len[2];
    swap_char_2((char*)&name_len, _name_len);

    char nbt[4];
    swap_char_4((char*)&payload, nbt);

    nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, &id, 1, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, _name_len, 2, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, name, name_len, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, nbt, 4, buf, buf_len);

    return 0;
}

int nbt_add_long(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, long payload)
{
    /* update state */
    switch (b->top->state)
    {
    case S_OBJ_OR_CLOSE:
        break;
    default:
        return -1;
        break;
    }

    int total_len = 1 + 2 + name_len + 8;

    char id = nbt_long;

    char _name_len[2];
    swap_char_2((char*)&name_len, _name_len);

    char nbt[8];
    swap_char_8((char*)&payload, nbt);

    nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, &id, 1, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, _name_len, 2, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, name, name_len, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, nbt, 8, buf, buf_len);

    return 0;
}