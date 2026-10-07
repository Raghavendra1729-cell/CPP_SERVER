#include <unistd.h>
#include "frame.h"

using namespace std;

bool readFull(int fd, uint8_t *buf, size_t n) {
    size_t got = 0;
    while (got < n) {
        ssize_t r = read(fd, buf + got, n - got);
        if (r <= 0) return false;
        got += r;
    }
    return true;
}

// read and throw away n bytes, used for frame types we do not know
bool skipBytes(int fd, uint32_t n) {
    uint8_t tmp[1024];
    while (n > 0) {
        size_t chunk = n < sizeof(tmp) ? n : sizeof(tmp);
        if (!readFull(fd, tmp, chunk)) return false;
        n -= chunk;
    }
    return true;
}

void writeFull(int fd, const uint8_t *buf, size_t n) {
    size_t sent = 0;
    while (sent < n) {
        ssize_t w = write(fd, buf + sent, n - sent);
        if (w <= 0) return;
        sent += w;
    }
}

void sendFrame(int fd, uint8_t type, uint8_t flags, const vector<uint8_t> &payload) {
    uint32_t len = payload.size();
    vector<uint8_t> f;
    f.push_back(MAGIC);
    f.push_back(VERSION);
    f.push_back(type);
    f.push_back(flags);
    f.push_back(len >> 24);
    f.push_back(len >> 16);
    f.push_back(len >> 8);
    f.push_back(len);
    f.insert(f.end(), payload.begin(), payload.end());
    writeFull(fd, f.data(), f.size());
}
