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

    return 0;
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

static int nbt_change_state(nbt_build* b, enum nbtb_state_type new_state)
{
    b->top->state = new_state;

    return 0;
}

static void incre_list_meta(nbt_build* b, char* buf, int buf_len)
{
    int list_count = 0;
    swap_char_4(buf + b->top->payload + 1, (char*)&list_count);

    list_count++;

    char new_list_count[4];
    swap_char_4((char*)&list_count, new_list_count);

    nbt_replace_raw_nbt(b, b->top->payload + 1, new_list_count, 4, buf, buf_len);
}

void nbt_init_build(nbt_build* b)
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
    case S_OBJ_OR_CLOSE: {
        if (nbtb_incre_state(b, S_OBJ_OR_CLOSE)) return NBT_WARN;

        int total_len = 1 + 2 + name_len;

        if (nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len)) return NBT_NOMEM;

        char id = nbt_compound;
        char len[2];
        swap_char_2((char*)&name_len, len);

        nbt_replace_raw_nbt(b, b->offset, &id, 1, buf, buf_len);
        nbt_replace_raw_nbt(b, b->offset, len, 2, buf, buf_len);
        nbt_replace_raw_nbt(b, b->offset, name, name_len, buf, buf_len);

        break;
    }    
    
    case S_LST_VAL_OR_CLOSE: {
        char id = nbt_compound;
        nbt_replace_raw_nbt(b, b->top->payload, &id, 1, buf, buf_len);

        if (nbt_change_state(b, S_NXT_LST_VAL_OR_CLOSE)) return NBT_WARN;
    } /* Fall through */
    case S_NXT_LST_VAL_OR_CLOSE: {
        if (buf[b->top->payload] != (char)nbt_compound) return NBT_WARN;

        incre_list_meta(b, buf, buf_len);

        if (nbtb_incre_state(b, S_OBJ_OR_CLOSE)) return NBT_WARN;        
        break;
    }

    default:
        return NBT_WARN;
        break;
    }

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
        if (nbt_decre_state(b)) return NBT_WARN;
        break;
    default:
        return NBT_WARN;
        break;
    }

    char payload = nbt_end;

    if (nbt_add_raw_nbt(b, b->offset, &payload, 1, buf, buf_len)) return NBT_NOMEM;

    return 0;
}

int nbt_start_list(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len)
{
    int total_len = 0;
    /* update state */
    switch (b->top->state)
    {
    case S_OBJ_OR_CLOSE: {
        total_len = 1 + 2 + name_len + 5;

        char id = nbt_list;
        char len[2];
        swap_char_2((char*)&name_len, len);

        if (nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len)) return NBT_NOMEM;
        nbt_replace_raw_nbt(b, b->offset, &id, 1, buf, buf_len);
        nbt_replace_raw_nbt(b, b->offset, len, 2, buf, buf_len);
        nbt_replace_raw_nbt(b, b->offset, name, name_len, buf, buf_len);

        break;
    }
    case S_LST_VAL_OR_CLOSE: {
        char id = nbt_list;
        nbt_replace_raw_nbt(b, b->top->payload, &id, 1, buf, buf_len);

        nbt_change_state(b, S_NXT_LST_VAL_OR_CLOSE);
    } /* Fall through */
    case S_NXT_LST_VAL_OR_CLOSE: {
        if (buf[b->top->payload] != (char)nbt_list) return NBT_WARN;

        incre_list_meta(b, buf, buf_len);
        if (nbt_allocate_raw_nbt(b, b->offset, 5, buf, buf_len)) return NBT_NOMEM;
        break;
    }

    default:
        return NBT_WARN;
        break;
    }

    if (nbtb_incre_state(b, S_LST_VAL_OR_CLOSE)) return NBT_WARN;
    b->top->payload = b->offset;

    char prefix[5] = {0};
    nbt_replace_raw_nbt(b, b->offset, prefix, 5, buf, buf_len);

    return 0;
}

int nbt_end_list(nbt_build* b, char* buf, const int buf_len)
{
    /* update state */
    switch (b->top->state)
    {
    case S_NXT_LST_VAL_OR_CLOSE:    
    case S_LST_VAL_OR_CLOSE:
        if (nbt_decre_state(b)) return NBT_WARN;
        break;
    default:
        return NBT_WARN;
        break;
    }
    return 0;
}

int nbt_add_single(nbt_build* b, char* buf, const int buf_len, nbt_type_t type, char* name, const short name_len, char* nbt_payload, const int payload_len)
{  
    /* update state */
    switch (b->top->state)
    {
    case S_OBJ_OR_CLOSE: {
        int total_len = 1 + 2 + name_len + payload_len;

        char _name_len[2];
        swap_char_2((char*)&name_len, _name_len);

        if (nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len)) return NBT_NOMEM;
        nbt_replace_raw_nbt(b, b->offset, (const char*)&type, 1, buf, buf_len);
        nbt_replace_raw_nbt(b, b->offset, _name_len, 2, buf, buf_len);
        nbt_replace_raw_nbt(b, b->offset, name, name_len, buf, buf_len);

        break;
    }

    case S_LST_VAL_OR_CLOSE: {
        nbt_replace_raw_nbt(b, b->top->payload, (const char*)&type, 1, buf, buf_len);

        nbt_change_state(b, S_NXT_LST_VAL_OR_CLOSE);
    } /* Fall through */
    case S_NXT_LST_VAL_OR_CLOSE: {
        if (buf[b->top->payload] != (char)type) return NBT_WARN;

        if (nbt_allocate_raw_nbt(b, b->offset, payload_len, buf, buf_len)) return NBT_NOMEM;

        incre_list_meta(b, buf, buf_len);
        break;
    }

    default:
        return NBT_WARN;
        break;
    }

    nbt_replace_raw_nbt(b, b->offset, nbt_payload, payload_len, buf, buf_len);
    return 0;
}

int nbt_add_char(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, char payload)
{
    char nbt_payload = payload;

    return nbt_add_single(b, buf, buf_len, nbt_byte, name, name_len, &nbt_payload, 1);
}

int nbt_add_short(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, short payload)
{
    char nbt_payload[4];
    swap_char_2((char*)&payload, nbt_payload);

    return nbt_add_single(b, buf, buf_len, nbt_short, name, name_len, nbt_payload, 2);
}

int nbt_add_integer(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, int payload)
{
    char nbt_payload[4];
    swap_char_4((char*)&payload, nbt_payload);
    
    return nbt_add_single(b, buf, buf_len, nbt_int, name, name_len, nbt_payload, 4);
}

int nbt_add_long(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, long payload)
{
    char nbt_payload[8];
    swap_char_8((char*)&payload, nbt_payload);
    
    return nbt_add_single(b, buf, buf_len, nbt_long, name, name_len, nbt_payload, 8);
}

int nbt_add_float(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, float payload)
{
    char nbt_payload[4];
    swap_char_4((char*)&payload, nbt_payload);
    
    return nbt_add_single(b, buf, buf_len, nbt_float, name, name_len, nbt_payload, 4);
}

int nbt_add_double(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, double payload)
{
    char nbt_payload[8];
    swap_char_8((char*)&payload, nbt_payload);
    
    return nbt_add_single(b, buf, buf_len, nbt_double, name, name_len, nbt_payload, 8);
}

int nbt_add_byte_array(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, char payload[], int payload_len)
{
    int total_len = 0;

    /* Update state */
    switch (b->top->state)
    {
    case S_OBJ_OR_CLOSE: {
        total_len = 1 + 2 + name_len + 2 + payload_len;

        char id = nbt_byte_array;

        char _name_len[2];
        swap_char_2((char*)&name_len, _name_len);

        if (nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len)) return NBT_NOMEM;
        nbt_replace_raw_nbt(b, b->offset, &id, 1, buf, buf_len);
        nbt_replace_raw_nbt(b, b->offset, _name_len, 2, buf, buf_len);
        nbt_replace_raw_nbt(b, b->offset, name, name_len, buf, buf_len);

        break;
    }

    case S_LST_VAL_OR_CLOSE: {
        char id = nbt_byte_array;
        nbt_replace_raw_nbt(b, b->top->payload, &id, 1, buf, buf_len);

        nbt_change_state(b, S_NXT_LST_VAL_OR_CLOSE);
    } /* Fall through */
    case S_NXT_LST_VAL_OR_CLOSE: {
        if (buf[b->top->payload] != (char)nbt_byte_array) return NBT_WARN;

        total_len = 4 + payload_len;
        nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len);

        incre_list_meta(b, buf, buf_len);
        break;
    }

    default:
        return NBT_WARN;
        break;
    }

    char _payload_len[4];
    swap_char_4((char*)&payload_len, _payload_len);

    nbt_replace_raw_nbt(b, b->offset, _payload_len, 4, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, payload, payload_len, buf, buf_len);

    return 0;
}

int nbt_add_string(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, char payload[], unsigned short payload_len)
{
    int total_len = 0;

    /* Update state */
    switch (b->top->state)
    {
    case S_OBJ_OR_CLOSE: {
        total_len = 1 + 2 + name_len + 2 + payload_len;

        char id = nbt_string;

        char _name_len[2];
        swap_char_2((char*)&name_len, _name_len);

        if (nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len)) return NBT_NOMEM;
        nbt_replace_raw_nbt(b, b->offset, &id, 1, buf, buf_len);
        nbt_replace_raw_nbt(b, b->offset, _name_len, 2, buf, buf_len);
        nbt_replace_raw_nbt(b, b->offset, name, name_len, buf, buf_len);

        break;
    }

    case S_LST_VAL_OR_CLOSE: {
        char id = nbt_string;
        nbt_replace_raw_nbt(b, b->top->payload, &id, 1, buf, buf_len);

        nbt_change_state(b, S_NXT_LST_VAL_OR_CLOSE);
    } /* Fall through */
    case S_NXT_LST_VAL_OR_CLOSE: {
        if (buf[b->top->payload] != (char)nbt_string) return NBT_WARN;

        total_len = 2 + payload_len;
        if (nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len)) return NBT_NOMEM;

        incre_list_meta(b, buf, buf_len);
        break;
    }

    default:
        return NBT_WARN;
        break;
    }

    char _payload_len[2];
    swap_char_2((char*)&payload_len, _payload_len);

    nbt_replace_raw_nbt(b, b->offset, _payload_len, 2, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, payload, payload_len, buf, buf_len);

    return 0;
}

int nbt_add_int_array(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, int payload[], int payload_len)
{
    int total_len = 0;

    /* Update state */
    switch (b->top->state)
    {
    case S_OBJ_OR_CLOSE: {
        total_len = 1 + 2 + name_len + 2 + payload_len * sizeof(int);

        char id = nbt_int_array;

        char _name_len[2];
        swap_char_2((char*)&name_len, _name_len);

        if (nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len)) return NBT_NOMEM;
        nbt_replace_raw_nbt(b, b->offset, &id, 1, buf, buf_len);
        nbt_replace_raw_nbt(b, b->offset, _name_len, 2, buf, buf_len);
        nbt_replace_raw_nbt(b, b->offset, name, name_len, buf, buf_len);

        break;
    }

    case S_LST_VAL_OR_CLOSE: {
        char id = nbt_int_array;
        nbt_replace_raw_nbt(b, b->top->payload, &id, 1, buf, buf_len);

        nbt_change_state(b, S_NXT_LST_VAL_OR_CLOSE);
    } /* Fall through */
    case S_NXT_LST_VAL_OR_CLOSE: {
        if (buf[b->top->payload] != (char)nbt_int_array) return NBT_WARN;

        total_len = 4 + payload_len * sizeof(int);
        if (nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len)) return NBT_NOMEM;

        incre_list_meta(b, buf, buf_len);
        break;
    }

    default:
        return NBT_WARN;
        break;
    }

    char _payload_len[4];
    swap_char_4((char*)&payload_len, _payload_len);

    for (size_t i = 0; i < payload_len; i++)
    {
        safe_swap_4((char*)&payload[i], (char*)&payload[i]);
    }

    nbt_replace_raw_nbt(b, b->offset, _payload_len, 4, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, (char*)payload, payload_len * sizeof(int), buf, buf_len);

    return 0;
}

int nbt_add_long_array(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, long payload[], int payload_len)
{
    int total_len = 0;

    /* Update state */
    switch (b->top->state)
    {
    case S_OBJ_OR_CLOSE: {
        total_len = 1 + 2 + name_len + 2 + payload_len * sizeof(long);

        char id = nbt_long_array;

        char _name_len[2];
        swap_char_2((char*)&name_len, _name_len);

        if (nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len)) return NBT_NOMEM;
        nbt_replace_raw_nbt(b, b->offset, &id, 1, buf, buf_len);
        nbt_replace_raw_nbt(b, b->offset, _name_len, 2, buf, buf_len);
        nbt_replace_raw_nbt(b, b->offset, name, name_len, buf, buf_len);

        break;
    }

    case S_LST_VAL_OR_CLOSE: {
        char id = nbt_long_array;
        nbt_replace_raw_nbt(b, b->top->payload, &id, 1, buf, buf_len);

        nbt_change_state(b, S_NXT_LST_VAL_OR_CLOSE);
    } /* Fall through */
    case S_NXT_LST_VAL_OR_CLOSE: {
        if (buf[b->top->payload] != (char)nbt_long_array) return NBT_WARN;

        total_len = 4 + payload_len * sizeof(long);
        if (nbt_allocate_raw_nbt(b, b->offset, total_len, buf, buf_len)) return NBT_NOMEM;

        incre_list_meta(b, buf, buf_len);
        break;
    }

    default:
        return NBT_WARN;
        break;
    }

    char _payload_len[4];
    swap_char_4((char*)&payload_len, _payload_len);

    for (size_t i = 0; i < payload_len; i++)
    {
        safe_swap_8((char*)&payload[i], (char*)&payload[i]);
    }

    nbt_replace_raw_nbt(b, b->offset, _payload_len, 4, buf, buf_len);
    nbt_replace_raw_nbt(b, b->offset, (char*)payload, payload_len * sizeof(long), buf, buf_len);

    return 0;
}