#ifndef RESPONSE_H
#define RESPONSE_H

#include <ctime>
#include <string>

std::string contentType(std::string path);
void sendResponse(int fd, int status, std::string type, std::string body, time_t modified);
void sendError(int fd, int code, std::string text);

#endif
