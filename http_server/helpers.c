#include "httpserver.h"

int listener_init(Listener_Socket *sock, int port) {
    sock->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock->fd == -1) {
        printf("socket creation failed\n");
        return -1;
    } else {
        printf("socket created successfully\n");
        bzero(&sock->servaddr, sizeof(sock->servaddr)); // zero out the struct for the server address
        sock->servaddr.sin_family = AF_INET; // set the family to ipv4
        sock->servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // listen to any address on the host
        sock->servaddr.sin_port = htons(port); // set port to listen on
        
        if ((bind(sock->fd, (SA *)&sock->servaddr, sizeof(sock->servaddr))) != 0) {
            printf("sock bind failed :(\n");
            // set errno to EINVAL for invalid port number
            if (port < 0 || port > 65535) {
                errno = EINVAL;
            }
            return -1;
        } else {
            printf("sock successfully binded\n");
        }
    }
}

int listener_accept(Listener_Socket *sock) {
    sock->len = sizeof(sock->cli);

    if (listen(sock->fd, 10) < 0) {
        printf("listen failed\n");
        return -1;
    } else {
        printf("server listening\n");
    }

    sock->connfd = accept(sock->fd, (SA *)&sock->cli, &sock->len);
    printf("%d\n", sock->connfd);

    if (sock->connfd < 0) {
        printf("server accept failed\n");
        return -1;
    } else {
        printf("server accepted client\n");
    return 0;
    }
}

ssize_t read_n_bytes(int in, char buf[], size_t n) {
    size_t bytes_read = 0;
    while (bytes_read < n) {
        ssize_t r = read(in, buf + bytes_read, n - bytes_read);
        if (r < 0) {
            return -1; // error
        } else if (r == 0) {
            break; // end of file
        } else {
            bytes_read += r;
        }
    }
    return bytes_read;
}

ssize_t write_n_bytes(int out, char buf[], size_t n) {
    size_t bytes_written = 0;
    while (bytes_written < n) {
        ssize_t w = write(out, buf + bytes_written, n - bytes_written);
        if (w < 0) {
            return -1; // error
        } else {
            bytes_written += w;
        }
    }
    return bytes_written;
}

ssize_t pass_n_bytes(int src, int dst, size_t n) {
    size_t bytes_passed = 0;
    char buf[BUFFER];
    while (bytes_passed < n) {
        size_t to_read = (n - bytes_passed) < BUFFER ? (n - bytes_passed) : BUFFER;
        ssize_t r = read(src, buf, to_read);
        if (r < 0) {
            return -1; // error
        } else if (r == 0) {
            break; // end of file
        } else {
            ssize_t w = write_n_bytes(dst, buf, r);
            if (w < 0) {
                return -1; // error
            } else {
                bytes_passed += w;
            }
        }
    }
    return bytes_passed;
}