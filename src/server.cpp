#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include "server.h"
#include "frame.h"
#include "headers.h"
#include "response.h"

using namespace std;

const uint32_t MAX_REQUEST = 65536;
const uint32_t MAX_FILE = 64 * 1024 * 1024;

static int handleRequest(int fd, int method, string path, string root) {
    if (path.empty() || path[0] != '/' || method != 1) {
        sendError(fd, 400, "Bad Request");
        return 400;
    }
    if (path.find("..") != string::npos) {
        sendError(fd, 403, "Forbidden");
        return 403;
    }

    string full = root + path;
    struct stat st;
    if (stat(full.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        if (full.back() != '/') full += "/";
        full += "index.html";
    }
    if (stat(full.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) {
        sendError(fd, 404, "Not Found");
        return 404;
    }
    if (st.st_size > MAX_FILE) {
        sendError(fd, 500, "Internal Server Error");
        return 500;
    }
    ifstream in(full, ios::binary);
    if (!in) {
        sendError(fd, 500, "Internal Server Error");
        return 500;
    }
    stringstream ss;
    ss << in.rdbuf();
    sendResponse(fd, 200, contentType(full), ss.str(), st.st_mtime);
    return 200;
}

void handleClient(int fd, string root) {
    while (true) {
        uint8_t h[8];
        if (!readFull(fd, h, 8)) break;
        if (h[0] != MAGIC || h[1] != VERSION) {
            sendError(fd, 400, "Bad Request");
            cout << "bad magic/version -> 400, closing" << endl;
            break;
        }
        uint8_t type = h[2];
        uint32_t len = ((uint32_t)h[4] << 24) | (h[5] << 16) | (h[6] << 8) | h[7];

        if (type != T_REQUEST) {
            if (!skipBytes(fd, len)) break;
            continue;
        }
        if (len > MAX_REQUEST) {
            sendError(fd, 400, "Bad Request");
            cout << "request too big -> 400, closing" << endl;
            break;
        }
        vector<uint8_t> payload(len);
        if (len > 0 && !readFull(fd, payload.data(), len)) break;

        int method = 0;
        string path;
        Headers headers;
        int status;
        if (!parseRequest(payload, method, path, headers)) {
            sendError(fd, 400, "Bad Request");
            status = 400;
        } else {
            status = handleRequest(fd, method, path, root);
        }
        cout << "GET " << path << " -> " << status << endl;
    }
    close(fd);
}
