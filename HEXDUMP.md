# Annotated hexdump, GET /index.html

Made with `./bserve ./www 9001` and `./bcurl -v localhost:9001/index.html`.
The file is 176 bytes long.

## Request (client to server): REQUEST frame, 8 + 63 = 71 bytes

```
42 01 01 00 00 00 00 3f          frame header
01 00 0b 2f 69 6e 64 65 78 2e 68 74 6d 6c 04     method, path, header count
01 00 0e 6c 6f 63 61 6c 68 6f 73 74 3a 39 30 30 31
02 00 09 62 63 75 72 6c 2f 31 2e 30
03 00 03 2a 2f 2a
04 00 0a 6b 65 65 70 2d 61 6c 69 76 65
```

| Bytes | Meaning |
|-------|---------|
| `42` | magic, "B" |
| `01` | version 1 |
| `01` | type REQUEST |
| `00` | flags, none |
| `00 00 00 3f` | Length = 63 |
| `01` | method GET |
| `00 0b` | path length 11 |
| `2f 69 6e 64 65 78 2e 68 74 6d 6c` | "/index.html" |
| `04` | 4 header fields follow |
| `01` `00 0e` "localhost:9001" | id 1 = host, length 14 |
| `02` `00 09` "bcurl/1.0" | id 2 = user-agent |
| `03` `00 03` "*/*" | id 3 = accept |
| `04` `00 0a` "keep-alive" | id 4 = connection |

## Response frame 1: RESPONSE, 8 + 122 = 130 bytes

```
42 01 02 00 00 00 00 7a
00 c8 07
05 00 09 74 65 78 74 2f 68 74 6d 6c
06 00 03 31 37 36
07 00 0a 62 73 65 72 76 65 2f 31 2e 30
08 00 1d 54 75 65 2c 20 30 36 20 4f 63 74 20 32 30 32 36 20 30 39 3a 33 37 3a 30 37 20 47 4d 54
09 00 1d 54 75 65 2c 20 30 36 20 4f 63 74 20 32 30 32 36 20 30 39 3a 33 34 3a 33 38 20 47 4d 54
0a 00 08 6e 6f 2d 63 61 63 68 65
04 00 0a 6b 65 65 70 2d 61 6c 69 76 65
```

| Bytes | Meaning |
|-------|---------|
| `42 01 02 00` | magic, version 1, type RESPONSE, no flags |
| `00 00 00 7a` | Length = 122 |
| `00 c8` | status 0x00c8 = 200 |
| `07` | 7 header fields |
| `05` `00 09` "text/html" | content-type |
| `06` `00 03` "176" | content-length |
| `07` `00 0a` "bserve/1.0" | server |
| `08` `00 1d` "Tue, 06 Oct 2026 09:37:07 GMT" | date (29 bytes = 0x1d) |
| `09` `00 1d` "Tue, 06 Oct 2026 09:34:38 GMT" | last-modified |
| `0a` `00 08` "no-cache" | cache-control |
| `04` `00 0a` "keep-alive" | connection |

## Response frame 2: BODY, 8 + 176 = 184 bytes

```
42 01 03 01 00 00 00 b0
3c 68 74 6d 6c 3e 0a 3c 68 65 61 64 3e ...    (176 bytes of the file)
```

| Bytes | Meaning |
|-------|---------|
| `42 01 03` | magic, version, type BODY |
| `01` | flags = LAST |
| `00 00 00 b0` | Length = 176 |
| `3c 68 74 6d 6c 3e ...` | the file "<html>\n<head>...", written to stdout by bcurl |
