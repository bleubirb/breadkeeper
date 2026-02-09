/* homemade http server for bread hosting, using the tcp protocol
 * hopefully it'll work who knows
*/

#include "httpserver.h"

#define BUFFER 4096 // buffer intake size when taking in a message

typedef struct {
    char *command; // get or put command
    char *target_path; // path to file requested
    char *version; // http version
    char *message; // message to send
    // TODO: alter to be unsigned ints when figuring out max byte sizes
    int info; // fd for input file
    int content_length; //content length of file
    int bytes; // bytes read
} req;

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
    sock->connfd = accept(sock->fd, (SA *)&sock->cli, &sock->len);
    while (sock->connfd < 0) {
        printf("server accept failed\n");
        // return -1;
    } 
    // else {
    printf("server accepted client\n");
    return 0;
    // }
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port number>\n", argv[0]);
        return 1;
    }

    char buf[BUFFER + 1] = { '\0' };

    Listener_Socket socket;

    int port = strtoull(argv[1], NULL, 10); // take in port to listen on
    if (errno == EINVAL) {
        fprintf(stderr, "invalid port\n");
        return 0;
    }

    int socket_status = listener_init(&socket, port);
    if (socket_status < 0) {
        fprintf(stderr, "invalid port\n");
        return 0;
    }

    while (1) {
        int sock_fd = listener_accept(&socket);
        if (sock_fd < 0) {
            fprintf(stderr, "failed to accept client\n");
            return 0;
        } 
        req r;
        r.info = sock_fd;
        // read bytes until we get the end of the header, which is denoted by \r\n\r\n
        ssize_t bytes_read = 0;
        while (strstr(buf, "\r\n\r\n") == NULL) {
            ssize_t n = read_n_bytes(sock_fd, buf + bytes_read, BUFFER - bytes_read);
            if (n < 0) {
                fprintf(stderr, "failed to read from client\n");
                return 0;
            } else if (n == 0) {
                fprintf(stderr, "client closed connection\n");
                return 0;
            } else {
                bytes_read += n;
                if (bytes_read >= BUFFER) {
                    fprintf(stderr, "header too large\n");
                    return 0;
                }
            }
        printf("header read successfully\n");
        exit(0);
        }
    }
}