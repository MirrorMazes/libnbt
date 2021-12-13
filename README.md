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
- Number of characters in a string: `%ns`
- Number of bytes in a byte array: `%nb`
- Number of integers in an integer array: `%ni`
- Number of longs in a long array: `%nl`
- Number of elements in a list: `%nt`

More will be added as support for tags expand.

### Path to data
For extracting NBT, there is a simple way to express the path to the data.

1. Name of a compound is enclosed by `{}`
2. Name of a list is enclosed by `[]:x`, where x is the index of the element to access.
3. Name of the data to be accessed is enclosed by `''`

For example, to access a short called `shortTest`, which is enclosed by a compound called `Level`, the path would be `[Level]'shortTest'`.

To store `shortTest` in a variable called `result`, the whole format string would be `"[Level]'shortTest':%h"`, 
and `&result` must be passed to the function via variadic arguments.


### The simple interface

```
void nbt_easy_extract(struct nbt_sized_buffer* content, char* fmt, ...)
```

This function parses the NBT data inside `content` and stores it inside the pointers in the variadic arguments.

### The normal interface
Currently a WIP.
