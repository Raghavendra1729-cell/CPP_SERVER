#include <ctime>
#include <vector>
#include "response.h"
#include "frame.h"
#include "headers.h"

using namespace std;

static string httpDate(time_t t) {
    char buf[64];
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
    return buf;
}

string contentType(string path) {
    size_t dot = path.rfind('.');
    string ext = dot == string::npos ? "" : path.substr(dot);
    if (ext == ".html") return "text/html";
    if (ext == ".txt") return "text/plain";
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "text/javascript";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg") return "image/jpeg";
    return "application/octet-stream";
}

void sendResponse(int fd, int status, string type, string body, time_t modified) {
    vector<uint8_t> p;
    p.push_back(status >> 8);
    p.push_back(status);
    Headers h;
    h.push_back(make_pair("content-type", type));
    h.push_back(make_pair("content-length", to_string(body.size())));
    h.push_back(make_pair("server", "bserve/1.0"));
    h.push_back(make_pair("date", httpDate(time(NULL))));
    if (status == 200) h.push_back(make_pair("last-modified", httpDate(modified)));
    h.push_back(make_pair("cache-control", "no-cache"));
    h.push_back(make_pair("connection", "keep-alive"));
    p.push_back(h.size());
    for (size_t i = 0; i < h.size(); i++) putHeader(p, h[i].first, h[i].second);
    sendFrame(fd, T_RESPONSE, 0, p);

    // a RESPONSE is always followed by exactly one BODY frame
    vector<uint8_t> b(body.begin(), body.end());
    sendFrame(fd, T_BODY, F_LAST, b);
}

void sendError(int fd, int code, string text) {
    sendResponse(fd, code, "text/plain", to_string(code) + " " + text + "\n", 0);
}
