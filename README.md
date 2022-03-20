# Libnbt
A library to work with [NBT](https://wiki.vg/NBT), written in C.

This library is a WIP, and there may be bugs.

## Usage
There are two parts to this library. nbt_build and nbt_find.


## Nbt build
This part of the library allows you to build NBT data structures.

### Initialisation
The initialisation function `void nbt_init_build(nbt_build* b);` must be called before using any functions in nbt_build.
Since there is no heap allocation, no destroy or cleanup function needs to be called.

Params:
- `nbt_build* b`: The address of the nbt_build structure to be initialised.

### Common parameters
Most functionns in nbt_build start with `nbt_build* b, char* buf, const int buf_len`

- `nbt_build* b` refers to the nbt_build structure initialised with `nbt_init_build`
- `char* buf` refers to the buffer to store the NBT data.
- `const int buf_len` refers to the length of the buffer.

### Building compounds and lists
Compounds can be build with `nbt_start_compound` and `nbt_start_list` respectively. 

The respective end functions, for example `nbt_end_compound` for compounds, must be called to signal the end of the compound or list.

Paramaters:
- `char* name`: The name of the compound or list. Will be ignored if in list.
- `const short name_len`: Length of the name.

Returns:
- `0` if operation succeeded.
- `NBT_WARN` if operation failed.
- `NBT_NOMEM` if there is a lack of memory in `char* buf`.

### Building simple data structures
Other data structures can be build with `int nbt_add_##datatype(nbt_build* b, char* buf, const int buf_len, char* name, const short name_len, ##datatype payload);`
Paramaters:
- `char* name`: The name of the compound or list. Will be ignored if in list.
- `const short name_len`: Length of the name.
- `[datatype] payload`: The payload to be stored in NBT.
- (array only) `payload_len`: The length of the payload.

Returns:
- `0` if operation succeeded.
- `NBT_WARN` if operation failed.
- `NBT_NOMEM` if there is a lack of memory in `char* buf`.

