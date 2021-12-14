# Libnbt
A library to work with [NBT](https://wiki.vg/NBT), written in C.

This library is a WIP, and there may be bugs, especially if the operation includes lists.

## Usage
There are two interfaces you can use in this library, the simple interface and the normal interface.

The simple interface is designed to be easy to use,
while the normal interface is designed to be more customisable and memory efficient.

### Format strings
For functions to extract NBT data, the following format strings are available:

- Byte: `%b`
- Short: `%h`
- Integer: `%i`
- Long: `%l`
- Float: `%f`
- Double: `%d`
- String: `%s`
- Number of characters in a string: `%ns` (An unsigned short)
- Number of bytes in a byte array: `%nb` (A signed integer)
- Number of integers in an integer array: `%ni` (A signed integer)
- Number of longs in a long array: `%nl` (A signed integer)
- Number of elements in a list: `%nt` (A signed integer)

### Path to data
For extracting NBT, there is a simple way to express the path to the data.

1. Name of a compound is enclosed by `{}`
2. Name of a list is enclosed by `[]:x`, where x is the index of the element to access.
3. Name of the data to be accessed is enclosed by `''`

For example, to access a short called `shortTest`, which is enclosed by a compound called `Level`, the path would be `[Level]'shortTest'`.

To store `shortTest` in a variable called `result`, the whole format string would be `"[Level]'shortTest':%h"`, 
and `&result` must be passed to the function via variadic arguments.


## The simple interface

```
void nbt_easy_extract(struct nbt_sized_buffer* content, char* fmt, ...)
```

This function parses the NBT data inside `content` and stores it inside the pointers in the variadic arguments.

## The normal interface

## `nbt_init_parser`:
```
void nbt_init_parser(struct nbt_parser_t* parser, struct nbt_sized_buffer* content, const struct nbt_setting_t* setting)
```
This function initialises `struct nbt_parser_t`, which is needed for operations later on.

This function also allocated the inital space for list metadata.

This function should be called only once, at the start of the program.

Params:
- `parser`: This is the `struct nbt_parser_t` that is initialised. This can be allocated on the stack.
- `content`: The nbt data to be parsed.
- `setting`: The global setting of the parser.

`struct nbt_setting_t setting` is defined as:
```
struct nbt_setting_t{
    const int list_meta_init_len;
    const int list_meta_expand_len;

    const int tok_init_len;
    const int tok_expand_len;
};
```

`list_meta_init_len`: The initial length of the list metadata allocated on the heap.

`list_meta_expand_len`: The additional space allocated to list metadata when there is not enough space.

`tok_init_len`: Initial length of the token allocate to the heap.

`tok_expand_len`: The additional space allocated to token when there is not enough space.

## `nbt_clear_parser`:
```
void nbt_clear_parser(struct nbt_parser_t* parser, struct nbt_sized_buffer* content)
```

This function clears the `struct nbt_parser_t`, so that `struct nbt_token_t* nbt_tokenise` may be reused.

Params:
- `parser`: The address of struct `nbt_parser_t` initialised with `void nbt_init_parser`.
- `content`: The nbt data to be parsed.


## `nbt_destroy_parser`:
```
void nbt_destroy_parser(struct nbt_parser_t* parser)
```

This function destroys the `parser` passed to it.

It will only free the list metadata that is allocated on the heap in this structure.

This function needs to be called once, when parsing operation ends.

## `nbt_init_token`:
```
struct nbt_token_t* nbt_init_token(struct nbt_parser_t* parser)
```
This function initialises and allocates the token on the heap.

This function should be called only once, at the start of parsing operation.

Params:
- `parser`: The address of struct `nbt_parser_t` initialised with `void nbt_init_parser`.

Outputs:
- The address of the `struct nbt_token_t` needed for later operations.

## `nbt_clear_token`:
```
void nbt_clear_token(struct nbt_token_t* tok, struct nbt_parser_t* parser);
```
This function clears a filled up struct `nbt_token_t`, so that `struct nbt_token_t* nbt_tokenise` may be reused.

This function does not free anything, so it does not suffer from the overhead of having too many mallocs.

Params:
- `tok`: the `nbt_token_t` that is being cleared.
- `parser`: The address of struct `nbt_parser_t` initialised with `void nbt_init_parser`.

## `nbt_destroy_token`:
```
struct nbt_token_t* nbt_destroy_token(struct nbt_token_t* tok, struct nbt_parser_t* parser);
```
This function destroys the `nbt_token_t` allocated by `nbt_init_token`.

This function should be called only once, at the end of nbt parsing operations.

Params:
- `tok`: The `struct nbt_token_t` that is being destroyed.
- `parser`: The address of struct `nbt_parser_t` initialised with `void nbt_init_parser`.

## `nbt_tokenise`:
```
struct nbt_token_t* nbt_tokenise(struct nbt_parser_t *parser, struct nbt_token_t* nbt_tok)
```
This function converts NBT data into an array of NBT tokens, essentially "parsing" NBT.

This function needs to be called once per NBT text.

Params:
- `parser`: The address of struct `nbt_parser_t` initialised with `void nbt_init_parser`. This must not be filled up already by another `struct nbt_token_t* nbt_tokenise`.
- `nbt_tok`: The token allocated with `struct nbt_token_t* nbt_init_token`. This must not be filled up already by another `struct nbt_token_t* nbt_tokenise`.

Outputs:
- The updated address of the `struct nbt_token_t` that is passed in to the function. This is because realloc may be called in the function on `nbt_tok`.

## `nbt_extract`:
```
void nbt_extract(struct nbt_parser_t* parser, struct nbt_token_t* tok, char* fmt, ...)
```
This function stores the NBT data parsed with `struct nbt_token_t* nbt_tokenise` into user passed in pointers.

This function could be called several times, for example to get the length of the string the first time, and then get the actual string the second time.

Params:
- `parser`: The address of struct `nbt_parser_t` initialised with `void nbt_init_parser`.
- `tok`: The token filled up by `struct nbt_token_t* nbt_tokenise`.
- `fmt`: The format string. For more infomation look [here](#Format-strings) and [here](#Path-to-data).
- Variadic arguments: The address of the variables to be filled up.

