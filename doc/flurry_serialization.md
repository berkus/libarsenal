Flurry serialization
====================

Uses msgpack v5 format tag codes. Very simplistic, not tuned for high-speed.

## Tag code ranges

0x00
.
POSITIVE fixnum, 0-127
.
0x7f
0x80
.
fixmap, up to 16 elements
.
0x8f
0x90
.
fixarray, up to 16 elements
.
0x9f
0xa0
.
fixstr up to 31 bytes
.
0xbf
0xc0  NIL
0xc1 <UNUSED>
0xc2  FALSE
0xc3  TRUE
0xc4  blob8
0xc5  blob16
0xc6  blob32
0xc7  ext8
0xc8  ext16
0xc9  ext32
0xca  FLOAT
0xcb  DOUBLE
0xcc  UINT8
0xcd  UINT16
0xce  UINT32
0xcf  UINT64
0xd0  INT8
0xd1  INT16
0xd2  INT32
0xd3  INT64
0xd4  fixext1
0xd5  fixext2
0xd6  fixext4
0xd7  fixext8
0xd8  fixext16
0xd9  STR8
0xda  STR16
0xdb  STR32
0xdc  ARRAY16
0xdd  ARRAY32
0xde  MAP16
0xdf  MAP32
0xe0
.
NEGATIVE fixnum
.
0xff

## Types

`NIL`
`BOOLEAN`
true or false
`POSITIVE_INTEGER`
`NEGATIVE_INTEGER`
limited between -(2^63) up to (2^64)-1
`REAL`
floating-point, single or double precision IEEE 754
`STRING`
with maximum length up to (2^32)-1
`BLOB` (byte array)
binary, with maximum length up to (2^32)-1
`ARRAY` (of typed entities)
array  (sequence), with maximum number of entries up to (2^32)-1
`MAP`
with maximum number of entries up to (2^32)-1
`EXT`
extended, with up to 128 custom application specific types

one byte:
```
+--------+
|        |
+--------+
```
a variable number of bytes:
```
+========+
|        |
+========+
```
variable number of objects stored in MessagePack format:
```
+~~~~~~~~~~~~~~~~~+
|                 |
+~~~~~~~~~~~~~~~~~+
```

 * X, Y, Z, A, B are individual bits
 * N is the length of data


## NIL format

nil:
```
+--------+
|  0xc0  |
+--------+
```

## BOOLEAN format

false:
```
+--------+
|  0xc2  |
+--------+
```
true:
```
+--------+
|  0xc3  |
+--------+
```

## INTEGER format

positive fixnum stores 7-bit positive integer
```
+--------+
|0XXXXXXX|
+--------+
```
negative fixnum stores 5-bit negative integer
```
+--------+
|111YYYYY|
+--------+
```
 * 0XXXXXXX is 8-bit unsigned integer
 * 111YYYYY is 8-bit signed integer (sign-extended)

uint 8 stores a 8-bit unsigned integer
```
+--------+--------+
|  0xcc  |ZZZZZZZZ|
+--------+--------+
```

uint 16 stores a 16-bit big-endian unsigned integer
```
+--------+--------+--------+
|  0xcd  |ZZZZZZZZ|ZZZZZZZZ|
+--------+--------+--------+
```

uint 32 stores a 32-bit big-endian unsigned integer
```
+--------+--------+--------+--------+--------+
|  0xce  |ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ
+--------+--------+--------+--------+--------+
```

uint 64 stores a 64-bit big-endian unsigned integer
```
+--------+--------+--------+--------+--------+--------+--------+--------+--------+
|  0xcf  |ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|
+--------+--------+--------+--------+--------+--------+--------+--------+--------+
```

int 8 stores a 8-bit signed integer
```
+--------+--------+
|  0xd0  |ZZZZZZZZ|
+--------+--------+
```

int 16 stores a 16-bit big-endian signed integer
```
+--------+--------+--------+
|  0xd1  |ZZZZZZZZ|ZZZZZZZZ|
+--------+--------+--------+
```

int 32 stores a 32-bit big-endian signed integer
```
+--------+--------+--------+--------+--------+
|  0xd2  |ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|
+--------+--------+--------+--------+--------+
```

int 64 stores a 64-bit big-endian signed integer
```
+--------+--------+--------+--------+--------+--------+--------+--------+--------+
|  0xd3  |ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|
+--------+--------+--------+--------+--------+--------+--------+--------+--------+
```

## REAL format

float 32 stores a floating point number in IEEE 754 single precision
floating point big-endian number format:
```
+--------+--------+--------+--------+--------+
|  0xca  |XXXXXXXX|XXXXXXXX|XXXXXXXX|XXXXXXXX
+--------+--------+--------+--------+--------+
         | MSB    |                 | LSB    |
         <--------------32 bits-------------->
```

float 64 stores a floating point number in IEEE 754 double precision
floating point big-endian number format:
```
+--------+--------+--------+--------+--------+--------+--------+--------+--------+
|  0xcb  |YYYYYYYY|YYYYYYYY|YYYYYYYY|YYYYYYYY|YYYYYYYY|YYYYYYYY|YYYYYYYY|YYYYYYYY|
+--------+--------+--------+--------+--------+--------+--------+--------+--------+
         | MSB    |                                                     | LSB    |
         <-----------------------------64 bits----------------------------------->
```

## Blob format (variable length opaque)

blob 8 stores a byte array whose length is upto (2^8)-1 bytes:
```
+--------+--------+========+
|  0xc4  |XXXXXXXX|  data  |
+--------+--------+========+
```
 * XXXXXXXX is a 8-bit unsigned integer which represents N

blob 16 stores a byte array whose length is upto (2^16)-1 bytes:
```
+--------+--------+--------+========+
|  0xc5  |YYYYYYYY|YYYYYYYY|  data  |
+--------+--------+--------+========+
```
 * YYYYYYYY_YYYYYYYY is a 16-bit big-endian unsigned integer which represents N

blob 32 stores a byte array whose length is upto (2^32)-1 bytes:
```
+--------+--------+--------+--------+--------+========+
|  0xc6  |ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|  data  |
+--------+--------+--------+--------+--------+========+
```
 * ZZZZZZZZ_ZZZZZZZZ_ZZZZZZZZ_ZZZZZZZZ is a 32-bit big-endian unsigned integer which represents N

## string format

(variable length utf8 string, possibly with corrupt codepoints)

fixstr stores a byte array whose length is upto 31 bytes:
```
+--------+========+
|101XXXXX|  data  |  0xa0-0xbf
+--------+========+
```
 * XXXXX is a 5-bit unsigned integer which represents N

str 8 stores a byte array whose length is upto (2^8)-1 bytes:
```
+--------+--------+========+
|  0xd9  |YYYYYYYY|  data  |
+--------+--------+========+
```
 * YYYYYYYY is a 8-bit unsigned integer which represents N

str 16 stores a byte array whose length is upto (2^16)-1 bytes:
```
+--------+--------+--------+========+
|  0xda  |ZZZZZZZZ|ZZZZZZZZ|  data  |
+--------+--------+--------+========+
```
 * ZZZZZZZZ_ZZZZZZZZ is a 16-bit big-endian unsigned integer which represents N

str 32 stores a byte array whose length is upto (2^32)-1 bytes:
```
+--------+--------+--------+--------+--------+========+
|  0xdb  |AAAAAAAA|AAAAAAAA|AAAAAAAA|AAAAAAAA|  data  |
+--------+--------+--------+--------+--------+========+
```
 * AAAAAAAA_AAAAAAAA_AAAAAAAA_AAAAAAAA is a 32-bit big-endian unsigned integer which represents N

## Array format

fixarray stores an array whose length is upto 15 elements:
0x9X
```
+--------+~~~~~~~~~~~~~~~~~+
|1001XXXX|    N objects    |
+--------+~~~~~~~~~~~~~~~~~+
```
 * XXXX is a 4-bit unsigned integer which represents N

array 16 stores an array whose length is upto (2^16)-1 elements:
```
+--------+--------+--------+~~~~~~~~~~~~~~~~~+
|  0xdc  |YYYYYYYY|YYYYYYYY|    N objects    |
+--------+--------+--------+~~~~~~~~~~~~~~~~~+
```
 * YYYYYYYY_YYYYYYYY is a 16-bit big-endian unsigned integer which represents N

array 32 stores an array whose length is upto (2^32)-1 elements:
```
+--------+--------+--------+--------+--------+~~~~~~~~~~~~~~~~~+
|  0xdd  |ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|    N objects    |
+--------+--------+--------+--------+--------+~~~~~~~~~~~~~~~~~+
```
 * ZZZZZZZZ_ZZZZZZZZ_ZZZZZZZZ_ZZZZZZZZ is a 32-bit big-endian unsigned integer which represents N

## map format

* odd elements in objects are keys of a map
* the next element of a key is its associated value

fixmap stores a map whose length is upto 15 elements
0x8X
```
+--------+~~~~~~~~~~~~~~~~~+
|1000XXXX|   N*2 objects   |
+--------+~~~~~~~~~~~~~~~~~+
```
 * XXXX is a 4-bit unsigned integer which represents N

map 16 stores a map whose length is upto (2^16)-1 elements
```
+--------+--------+--------+~~~~~~~~~~~~~~~~~+
|  0xde  |YYYYYYYY|YYYYYYYY|   N*2 objects   |
+--------+--------+--------+~~~~~~~~~~~~~~~~~+
```
 * YYYYYYYY_YYYYYYYY is a 16-bit big-endian unsigned integer which represents N

map 32 stores a map whose length is upto (2^32)-1 elements
```
+--------+--------+--------+--------+--------+~~~~~~~~~~~~~~~~~+
|  0xdf  |ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|   N*2 objects   |
+--------+--------+--------+--------+--------+~~~~~~~~~~~~~~~~~+
```
 * ZZZZZZZZ_ZZZZZZZZ_ZZZZZZZZ_ZZZZZZZZ is a 32-bit big-endian unsigned integer which represents N

## Extended format

* type is an 8-bit signed integer
* type < 0 is reserved for future extension including 2-byte type information

fixext 1 stores an integer and a byte array whose length is 1 byte
```
+--------+--------+--------+
|  0xd4  |  type  |  data  |
+--------+--------+--------+
```

fixext 2 stores an integer and a byte array whose length is 2 bytes
```
+--------+--------+--------+--------+
|  0xd5  |  type  |       data      |
+--------+--------+--------+--------+
```

fixext 4 stores an integer and a byte array whose length is 4 bytes
```
+--------+--------+--------+--------+--------+--------+
|  0xd6  |  type  |                data               |
+--------+--------+--------+--------+--------+--------+
```

fixext 8 stores an integer and a byte array whose length is 8 bytes
```
+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
|  0xd7  |  type  |                                  data                                 |
+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
```

fixext 16 stores an integer and a byte array whose length is 16 bytes
```
+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
|  0xd8  |  type  |                                  data                                  
+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
+--------+--------+--------+--------+--------+--------+--------+--------+
                              data (cont.)                              |
+--------+--------+--------+--------+--------+--------+--------+--------+
```

ext 8 stores an integer and a byte array whose length is upto (2^8)-1 bytes:
```
+--------+--------+--------+========+
|  0xc7  |XXXXXXXX|  type  |  data  |
+--------+--------+--------+========+
```
 * XXXXXXXX is a 8-bit unsigned integer which represents N

ext 16 stores an integer and a byte array whose length is upto (2^16)-1 bytes:
```
+--------+--------+--------+--------+========+
|  0xc8  |YYYYYYYY|YYYYYYYY|  type  |  data  |
+--------+--------+--------+--------+========+
```
 * YYYYYYYY_YYYYYYYY is a 16-bit big-endian unsigned integer which represents N

ext 32 stores an integer and a byte array whose length is upto (2^32)-1 bytes:
```
+--------+--------+--------+--------+--------+--------+========+
|  0xc9  |ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|ZZZZZZZZ|  type  |  data  |
+--------+--------+--------+--------+--------+--------+========+
```
 * ZZZZZZZZ_ZZZZZZZZ_ZZZZZZZZ_ZZZZZZZZ is a big-endian 32-bit unsigned integer which represents N

## THINGS MISSING FROM MSGPACK

* union
* void
* optional
* length-delimited optional

Union is represented via serialized discriminant and then the appropriate arm of the definition.

```
+~~~~~~~~~~~~~~~+~~~~~~~~~~~~~~~+
|  discriminant |  implied arm  |          DISCRIMINATED UNION
+~~~~~~~~~~~~~~~+~~~~~~~~~~~~~~~+
```

Void needs no representation in storage. For cases where this is necessary nil can be used.

Optional is represented as object or nil; represented object cannot be nil itself.

Length-delimited optional is represented by a serialized size and then the actual value for
the optional object if size is non-zero.

```
+~~~~~~~~~~~~~~+~~~~~~~~~~~~~~~+
| byte length  | option value  |          LENGTH-DELIMITED OPTIONAL
+~~~~~~~~~~~~~~+~~~~~~~~~~~~~~~+
```

