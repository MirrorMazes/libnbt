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

## Nbt find
This part of the library allows you to parse NBT data.

### Initialisation and shutdown
The function:
```C
void nbt_init_parser(nbt_parser* parser, struct nbt_sized_buffer* content, const struct nbt_parser_setting_t* setting);
```
has to be called before any other parsing function here is called.

Parameters:
- `nbt_parser* parser`: The parser to be initialised
- `struct nbt_sized_buffer* content`: The NBT data to be used in further parsing
- `const struct nbt_parser_setting_t* setting`: The settings of the parser

Definitions:
```C
struct nbt_sized_buffer {
    char* content;
    int len;
};
```
`content` is the NBT data, and `len` is the length of the NBT data.

```C
struct nbt_parser_setting_t {
    const int list_meta_init_len;
    const int list_meta_expand_len;

    const int tok_init_len;
    const int tok_expand_len;
};
```
`tok_init_len` and `tok_expand_len` is the number of tokens the parser will allocate and expand when there is no memory.

`list_meta_init_len` and `list_meta_expand_len` is the list metadata the parser will allocate and expand.


As parsing utilises the heap, a shutdown function must be called.

```C
void nbt_destroy_parser(nbt_parser* parser);
```

Parameters:
- `nbt_parser* parser`: The parser to be destroyed.

To change the NBT data to be parsed without destroying the parser structure, the function `nbt_clear_parser` may be used.

### Parsing NBT
To parse NBT, this function must be called:
```C
int nbt_tokenise(nbt_parser *parser);
```
This function parse the NBT data stored in the `parser` and tokenises it.

Parameters:
- `parser`: the structure initialised by `nbt_init_parser`.

This function returns 0 if succeeded, returns `NBT_WARN` if the NBT data is invalid.

To get the indexes of the NBT data, this function may be used:
```C
int nbt_find(nbt_parser* parser, struct nbt_lookup_t* path, int path_size, struct nbt_index_t* res);
```
If used on simple data structure that have no length prefix, like byte, int, long, float, etc, this function will return the exact location of the data.

If used on arrays, like byte array, strings, etc, this function will return the data and the length prefix.

if used on compounds or lists, this function will return everything related to the data, like the identifier byte, the name, and everything else included in the compound or list.

Parameters:
- `parser`: the structure initialised by `nbt_init_parser`.
- `path`: array of the path to the data.
- `path_size`: Number of elements in `path`.
- `res`: The resultant index.

Returns 0 if operation succeeded, `NBT_WARN` if unable to parse file.

Definitions:
```c
struct nbt_lookup_t{
    nbt_type_t type;
    char name[MAX_NAME_LEN + 1];

    long index;
};
```
`type` is the type of the NBT data.

`name` is the name of the NBT data. If the data is in list this does not need to be filled in.

`index` is the number of indexes to skip in a list. Only need to be filled in if `type` is `nbt_list`.


```c
struct nbt_index_t {
    int start;
    int end;
    int len;
};
```
This structure contains the results of the function.

`start` is the index of the first character of the data.

`end` is the index of the last character of the data.

`len` is the number of bytes of the data.

