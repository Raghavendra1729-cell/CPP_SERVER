#ifndef HEADERS_H
#define HEADERS_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

typedef std::vector<std::pair<std::string, std::string> > Headers;

void putHeader(std::vector<uint8_t> &out, std::string name, std::string value);
bool parseRequest(const std::vector<uint8_t> &p, int &method, std::string &path, Headers &headers);

#endif
