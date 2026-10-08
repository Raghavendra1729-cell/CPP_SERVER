# BHTTP-B, a small binary HTTP protocol

Version 1. Written by Raghavendra and Prateek.

BHTTP-B is a binary version of HTTP for getting files. A client sends a request frame and the server answers with
a response frame and one body frame. All numbers are big-endian. This file is meant to be enough to write a
client or a server that works with ours.

## 1. Connection

The client opens one TCP connection and can send as many requests on it as it wants. It sends a request,
waits for the whole answer and only then sends the next one. Because of that there is no stream id: the
first answer belongs to the first request, the second to the second and so on.

The server keeps the connection open. The client closes it when it is done.

## 2. Frame header

A frame is an 8 byte header and then the payload.

| Field   | Bits | Meaning |
|---------|------|---------|
| Magic   | 8    | always `0x42`, the letter B |
| Version | 8    | 1 for this spec |
| Type    | 8    | kind of frame |
| Flags   | 8    | one bit per option |
| Length  | 32   | size of the payload in bytes, the header is not counted |

### Why these sizes

HTTP/2 chose 24 / 8 / 8 / 31. We chose differently because our protocol is simpler.

Magic is 8 bits. If a browser or a scanner connects, its first byte is almost never 0x42. The server sees
that at once, answers 400 and hangs up, and does not read garbage as a length and wait for gigabytes.

Version is 8 bits. A server can tell that a client speaks a newer version.

Type is 8 bits. We use three types, the other numbers are for version 2.

Flags is 8 bits, one bit per option. Only one is used now.

Length is 32 bits. A whole file goes in one BODY frame, so the client does not have to put pieces together.
With 16 bits a file could only be 64 KB and with 24 bits 16 MB. To stay safe the server refuses a REQUEST
over 64 KB and does not send files over 64 MB.

There is no stream id. Requests go one after another, so an id would never be used.

The header is 8 bytes, so the length field starts at a multiple of 4.

## 3. Frame types and flags

| Type | Name     | Direction | Payload |
|------|----------|-----------|---------|
| 0x01 | REQUEST  | client to server | method, path and headers, see section 5 |
| 0x02 | RESPONSE | server to client | status and headers, see section 6 |
| 0x03 | BODY     | server to client | bytes of the file |

Flag `0x01` is LAST. It is used on BODY and means this is the final BODY frame of the answer.

If a receiver gets a frame type it does not know, it MUST read `Length` bytes, throw them away and go on
with the next frame. It must not close the connection and must not send an error. This is what leaves room
for version 2: new frame types do not break old programs. Undefined flag bits are ignored too.
A wrong magic or version is different. That program is not speaking BHTTP-B, so the server answers 400 and closes.

## 4. Header fields

REQUEST and RESPONSE both carry header fields. One field looks like this:

| Bytes | Meaning |
|-------|---------|
| 1 | name id. 1 to 10 is the table below, 0 means the name is sent as a literal |
| 1 + N | only if the id is 0: the length N of the name and then the name |
| 2 | length V of the value |
| V | the value |

Table of the ten names that the client and the server really send:

| Id | Name | Id | Name |
|----|------|----|------|
| 1 | host | 6 | content-length |
| 2 | user-agent | 7 | server |
| 3 | accept | 8 | date |
| 4 | connection | 9 | last-modified |
| 5 | content-type | 10 | cache-control |

An id above 10 is not defined. The receiver reads the value, because it has a length, and ignores the header.
Names are lower case.

## 5. REQUEST payload

| Bytes | Meaning |
|-------|---------|
| 1 | method, 1 is GET |
| 2 | length P of the path |
| P | the path, starts with `/` |
| 1 | number of header fields H |
| ... | H header fields |

The payload ends right after the last header. Bytes left over or missing mean it is malformed.
The client sends `host`, `user-agent`, `accept` and `connection`.

## 6. RESPONSE payload and BODY

| Bytes | Meaning |
|-------|---------|
| 2 | status code as a number, like 200 or 404 |
| 1 | number of header fields H |
| ... | H header fields |

A RESPONSE is always followed by exactly one BODY frame with LAST set. The BODY is empty if the file is empty.
The server sends `content-type`, `content-length`, `server`, `date`, `cache-control`, `connection` and,
for a 200, `last-modified`.

The server looks for the file under its root folder. A path of `/` or a folder gives the `index.html` in it.

## 7. Errors

An error uses the normal RESPONSE and BODY, the body is a short text like `404 Not Found`.

| Status | When | Connection |
|--------|------|------------|
| 400 | REQUEST payload malformed (too short, wrong header count, bytes left over), method is not 1, path does not start with `/` | stays open |
| 400 | wrong magic, wrong version, or REQUEST `Length` over 65536 | closed after the answer |
| 403 | path contains `..` | stays open |
| 404 | no such file | stays open |
| 500 | file cannot be read or is over 64 MB | stays open |

## 8. Client

- opens one connection and sends every request on it
- after a REQUEST reads frames until a BODY with LAST
- writes the BODY payloads to stdout
- skips frames of unknown types
- exit code 0 if every status is below 400, 1 if there was a 4xx or 5xx, 2 if the connection or the protocol failed

## 9. Example

`HEXDUMP.md` has a full request and response with every byte explained.

## 10. Future Work (Version 2)

Future revisions of BHTTP-B (`Version = 2`) could introduce:
- **Chunked Body Streaming:** Permitting multiple `BODY` frames without requiring the full length upfront, marking only the terminal frame with `LAST (0x01)`.
- **Stream Multiplexing:** Introducing a 31-bit stream identifier field into the frame header to enable interleaved requests and responses concurrently over a single TCP connection.
- **Dynamic Header Compression:** Augmenting the static table with an indexed dynamic dictionary (similar to HPACK) for repeated header values across requests.
- **Additional HTTP Methods:** Supporting `POST`, `PUT`, and `HEAD` methods (e.g., method IDs 2, 3, 4).
- **Server Push:** Allowing the server to proactively send cacheable assets before explicit client requests.
