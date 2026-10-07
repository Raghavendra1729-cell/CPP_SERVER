#ifndef FRAME_H
#define FRAME_H

#include <cstdint>
#include <cstddef>
#include <vector>

const uint8_t MAGIC = 'B';
const uint8_t VERSION = 1;
const uint8_t T_REQUEST = 1;
const uint8_t T_RESPONSE = 2;
const uint8_t T_BODY = 3;
const uint8_t F_LAST = 1;

bool readFull(int fd, uint8_t *buf, size_t n);
bool skipBytes(int fd, uint32_t n);
void writeFull(int fd, const uint8_t *buf, size_t n);
void sendFrame(int fd, uint8_t type, uint8_t flags, const std::vector<uint8_t> &payload);

#endif
