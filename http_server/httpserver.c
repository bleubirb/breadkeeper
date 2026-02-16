/* homemade http server for bread hosting, using the tcp protocol
 * hopefully it'll work who knows
*/

#include "httpserver.h"

#define REQ_REGEX "^([a-zA-Z]{1,8}) /([a-zA-Z0-9\\._-]{1,63}) (HTTP/[0-9]\\.[0-9])" // 1-8 ascii characters

#define HEAD_REGEX "^([a-zA-Z][a-zA-Z0-9_-]*): (.+)$" // 32-126 ascii characters

// TODO: put and get functions
// TODO: multi-thread with rw-locks

int put(req *r) {
    if (r->content_length < 0) {
        dprintf(r->info, "HTTP/1.1 411 Length Required\r\nContent-Length: %d\r\n\r\nLength Required\n", 16);
        return 0;
    } 
    fprintf(stderr, "handling PUT request for file: %s with content length: %d\n", r->target_path, r->content_length);
    // uint8_t fd = open(r->target_path, O_RDWR | __O_DIRECTORY, 0666);
    uint8_t status = 0;

    // if (fd > 0) {
    //     fprintf(stderr, "file is write only or a directory %d\n", fd);
    //     dprintf(r->info, "HTTP/1.1 503 Forbidden\r\nContent-Length: %d\r\n\r\nForbidden\n", 9);
    //     return 0;
    // }
    // if (fd != -1) {
    //     close(fd);
    // }
    // fprintf(stderr, "file does not exist, creating file\n");
    uint8_t fd = open(r->target_path, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd < 0) {
        if (errno == EEXIST) {
            fprintf(stderr, "file already exists\n");
            status = 200;
        } else if (errno == EACCES) {
            fprintf(stderr, "permission denied\n");
            status = 403;
            dprintf(r->info, "HTTP/1.1 403 Forbidden\r\nContent-Length: %d\r\n\r\nForbidden\n", 9);
            return 0;
        } else {
            fprintf(stderr, "failed to create file\n");
            status = 500;
            dprintf(r->info, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: %d\r\n\r\nInternal Server Error\n", 21);
            return 0;
        }
           
    } else {
        status = 201;
    }

    if (status == 200) {
        fd = open(r->target_path, O_WRONLY | O_TRUNC | O_CREAT, 0666);
        if (fd == -1) {
            if (errno == EACCES) {
                dprintf(r->info, "HTTP/1.1 403 Forbidden\r\nContent-Length: %d\r\n\r\nForbidden\n", 9);
                return 0;
            } else {
                dprintf(r->info, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: %d\r\n\r\nInternal Server Error\n", 21);
                return 0;
            }
        }
    }
    fprintf(stderr, "r->info: %d, fd: %d, content_length: %d\n", r->info, fd, r->content_length);
    ssize_t bytes_written = pass_n_bytes(r->info, fd, r->content_length);
    if (bytes_written < 0) {
        fprintf(stderr, "failed to write to file\n");
        dprintf(r->info, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: %d\r\n\r\nInternal Server Error\n", 21);
        return 0;
    }
    if (status == 201) {
        dprintf(r->info, "HTTP/1.1 201 Created\r\nContent-Length: %d\r\n\r\nCreated\n", 8);
    } else {
        dprintf(r->info, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\nOK\n", 3);
    }
    if (fd != -1) {
        close(fd);
    }
    return 0;
}

int trace(req *r) {
    if (r->content_length != -1) {
        dprintf(r->info, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%s", r->content_length, r->message);
        return 0;
    } else {
        dprintf(r->info, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%s", r->bytes, r->message);
        return 0;
    }
    return -1;
}

int req_handle(req *r) {
    if (strncmp(r->version, "HTTP/1.1", 8) == 0) {
        dprintf(r->info, "HTTP/1.1 505 Version Not Supported\r\nContent-Length: %d\r\n\r\nVersion Not Supported\n", 22);
        return 0;
    } else if (strncmp(r->command, "GET", 3) == 0) {
        // return get(r); // not implemented
        return -1;
    } else if (strncmp(r->command, "PUT", 3) == 0) {
        fprintf(stderr, "handling PUT request\n");
        return put(r);
    } else if (strncmp(r->command, "TRACE", 5) == 0) {
        return trace(r); // not implemented
    } else {
        dprintf(r->info, "HTTP/1.1 501 Not Implemented\r\nContent-Length: %d\r\n\r\nNot Implemented\n", 16);
        return 0;
    }
}

/**
 * for reference, an HTTP request comes in the form of: GET /hello.htm HTTP/1.1
 */
int req_parse(req *r, char *buf, ssize_t read_size) {
    fprintf(stderr, "buf: %s\n", buf);
    uint8_t offset = 0; // keeping track of where we are in buffer
    regex_t regex; 
    regmatch_t match[5]; // store matches from regex
    uint8_t req_code = 0; // request code to return
    req_code = regcomp(&regex, REQ_REGEX, REG_EXTENDED);
    req_code = regexec(&regex, buf, 5, match, 0);
    // TODO: fix regex
    if (req_code == REG_NOMATCH) {
        fprintf(stderr, "failed big time\n");
        dprintf(r->info, "HTTP/1.1 400 Bad Request\r\nContent-Length: %d\r\n\r\nBad Request\n", 13);
        regfree(&regex);
        return 1;
    }
    r->version = "";
    if (req_code == 0) { // regex returns something valid
        r->command = buf;
        r->target_path = buf + match[2].rm_so; // pulls the file location
        r->version = buf + match[3].rm_so; // uses http version
        buf[match[1].rm_eo] = '\0'; // null terminate the buffer after reading path and version

        // separating the silly requests
        r->target_path[match[2].rm_eo - match[3].rm_eo] = '\0';
        r->version[match[3].rm_eo - match[3].rm_eo] = '\0';
        // moving buffer and offset to the next line for further requests
        buf += match[3].rm_eo + 2;
        offset += match[3].rm_eo + 2;
    } else {
        fprintf(stderr, "failed to parse request\n");
        dprintf(r->info, "HTTP/1.1 400 Bad Request\r\nContent-Length: %d\r\n\r\nBad Request\n", 11);
        regfree(&regex);
        return 1;
    }
    r->content_length = -1;
    req_code = regcomp(&regex, HEAD_REGEX, REG_EXTENDED);
    req_code = regexec(&regex, buf, 4, match, 0);
    char *tok = strtok(buf, "\n");

    while (tok != NULL) {
        uint8_t len = strlen(tok);
        tok[len - 1] = '\0';
        if (strncmp(tok, "Content-Length", 14) == 0) {
            uint8_t val = strtoull(tok + 16, NULL, 10); 
            if (errno == EINVAL) {
                dprintf(r->info, "HTTP/1.1 400 Bad Request\r\nContent-Length: %d\r\n\r\nBad Request\n", 11);
            }
            r->content_length = val;
        }
        buf += strlen(tok) + 1;
        offset += strlen(tok) + 1;
        tok = strtok(NULL, "\n");
    }
    r->message = buf + 2; // move past the \r\n\r\n to the message body
    offset += 2;
    r->bytes = read_size - offset;

    memset(buf, '\0', BUFFER + 1);
    regfree(&regex);
    return 0;
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
        fprintf(stderr, "client accepted\n");
        req r;
        r.info = sock_fd;
        // read bytes until we get the end of the header, which is denoted by \r\n\r\n
        ssize_t bytes_read = 0;
        ssize_t n = 0;
        while ((strstr(buf, "\r\n\r\n") == NULL) && (n != -1)) {
            n = read_n_bytes(sock_fd, buf + bytes_read, BUFFER - bytes_read);
            fprintf(stderr, "read from client n: %ld\n", n);
            if (n < 0) {
                fprintf(stderr, "failed to read from client\n");
                dprintf(r.info, "HTTP/1.1 400 Bad Request\r\nContent-Length: %d\r\n\r\nBad Request\n", 11);
                close(sock_fd);
                continue;
            } else if (n == 0) {
                fprintf(stderr, "client closed connection\n");
                close(sock_fd);
                continue;
            } else {
                bytes_read += n;
                if (bytes_read >= BUFFER) {
                    fprintf(stderr, "header too large\n");
                    close(sock_fd);
                    continue;
                }
            }
        
        if (req_parse(&r, buf, bytes_read) != 1) {
            fprintf(stderr, "request parsed successfully\n");
            req_handle(&r);
        } 
    }
    close(sock_fd);
    memset(buf, '\0', BUFFER + 1);
    }
    return 0;
}