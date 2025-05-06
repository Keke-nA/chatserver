#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket creation failed.");
        return 0;
    }
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5001);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))
        == -1) {
        perror("setsockopt failed");
        close(sock_fd);
        return 0;
    }
    int ret = bind(sock_fd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (ret == -1) {
        perror("bind failed.");
        return 0;
    }
    ret = listen(sock_fd, 5);
    if (ret == -1) {
        perror("listen failed.");
        return 0;
    }
    ret = accept(sock_fd, (struct sockaddr*)nullptr, nullptr);
    if (ret == -1) {
        perror("accept failed.");
        return 0;
    }
    char buff[1024];
    int n = recv(ret, buff, 1024, 0);
    buff[n] = '\0';
    printf("%s\n", buff);
    send(ret, buff, n, 0);
    close(sock_fd);
    return 0;
}