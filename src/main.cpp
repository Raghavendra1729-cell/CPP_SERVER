#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server.h"

using namespace std;

int main(int argc, char **argv) {
    if (argc != 3) {
        cerr << "usage: " << argv[0] << " <root> <port>" << endl;
        return 1;
    }
    string root = argv[1];
    while (root.size() > 1 && root.back() == '/') root.pop_back();
    int port = atoi(argv[2]);

    signal(SIGPIPE, SIG_IGN);

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (::bind(lfd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    listen(lfd, 10);
    cout << "serving " << root << " on port " << port << endl;

    while (true) {
        int cfd = accept(lfd, NULL, NULL);
        if (cfd < 0) continue;
        cout << "new connection" << endl;
        // one thread per connection
        thread(handleClient, cfd, root).detach();
    }
}
