/* homemade http server for bread hosting, using the tcp protocol
 * hopefully it'll work who knows
*/

#include "httpserver.h"
#include "helpers.c"

#define REQ_REGEX "^([a-zA-Z]{1,8}) /([a-zA-Z0-9\\.-]{1,63}) (HTTP/[0-9]\\.[0-9])\r\n" // 1-8 ascii characters

#define HEAD_REGEX "^([:*/a-zA-Z\\._0-9-]{1,128}: [:*/a-zA-Z\\._0-9-]{1,128}\r\n)$" // 32-126 ascii characters

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

// TODO: request parser
// TODO: request handler
// TODO: put and get functions
// TODO: multi-thread with rw-locks

/**
 * for reference, an HTTP request comes in the form of: GET /hello.htm HTTP/1.1
 */
int req_parse(req *r, char *buf, ssize_t read_size) {
    uint8_t offset = 0; // keeping track of where we are in buffer
    regex_t regex; 
    regmatch_t match[5]; // store matches from regex
    uint8_t req_code = 0; // request code to return
    req_code = regcomp(&regex, REQ_REGEX, REG_EXTENDED);
    req_code = regexec(&regex, buf, 5, match, 0);
    r->version = "";
    if (req_code == 0) { // regex returns something valid
        r->command = buf; // gets the command "GET" or "PUT"
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
        dprintf(r->info, "HTTP/1.1 400 Bad Request\r\nContent-Length: %d\r\n\r\nBad Request\n", 12);
        regfree(&regex);
        return 1;
    }

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
        while (strstr(buf, "\r\n\r\n") != NULL) {
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