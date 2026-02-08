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

int listener_accept(Listener_Socket *sock);

ssize_t read_n_bytes(int in, char buf[], size_t n);

ssize_t write_n_bytes(int out, char buf[], size_t n);

ssize_t pass_n_bytes(int src, int dst, size_t n);

int main(int argc, char *argv[]) {}