#include "headers.h"

using namespace std;

// id 0 means the name is sent as a literal
const char *names[] = {"", "host", "user-agent", "accept", "connection", "content-type",
                       "content-length", "server", "date", "last-modified", "cache-control"};

void putHeader(vector<uint8_t> &out, string name, string value) {
    int id = 0;
    for (int i = 1; i <= 10; i++)
        if (name == names[i]) id = i;
    out.push_back(id);
    if (id == 0) {
        out.push_back(name.size());
        out.insert(out.end(), name.begin(), name.end());
    }
    out.push_back(value.size() >> 8);
    out.push_back(value.size());
    out.insert(out.end(), value.begin(), value.end());
}

bool parseRequest(const vector<uint8_t> &p, int &method, string &path, Headers &headers) {
    if (p.size() < 4) return false;
    method = p[0];
    size_t pathLen = (p[1] << 8) | p[2];
    size_t pos = 3;
    if (pos + pathLen + 1 > p.size()) return false;
    path = string((const char *)&p[pos], pathLen);
    pos += pathLen;
    int count = p[pos++];

    for (int i = 0; i < count; i++) {
        if (pos >= p.size()) return false;
        int id = p[pos++];
        string name;
        if (id == 0) {
            if (pos >= p.size()) return false;
            size_t n = p[pos++];
            if (pos + n > p.size()) return false;
            name = string((const char *)&p[pos], n);
            pos += n;
        } else if (id <= 10) {
            name = names[id];
        }
        // an id above 10 has no name, we still read its value and ignore it
        if (pos + 2 > p.size()) return false;
        size_t vlen = (p[pos] << 8) | p[pos + 1];
        pos += 2;
        if (pos + vlen > p.size()) return false;
        headers.push_back(make_pair(name, string((const char *)&p[pos], vlen)));
        pos += vlen;
    }
    // nothing may be left over after the last header
    return pos == p.size();
}
