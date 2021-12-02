#include "nbt_utils.h"

#include <stdint.h>
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