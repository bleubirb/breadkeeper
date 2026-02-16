#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SA struct sockaddr // shorthand for the sockaddr struct to not hate myself

#define BUFFER 4096 // buffer intake size when taking in a message

typedef struct Listener_Socket
{
    int fd;
    int connfd;
    int len;
    struct sockaddr_in cli, servaddr;
} Listener_Socket;

typedef struct req
{
    char *command;     // get or put command
    char *target_path; // path to file requested
    char *version;     // http version
    char *message;     // message to send
    // TODO: alter to be unsigned ints when figuring out max byte sizes
    int info;           // fd for input file
    int content_length; // content length of file
    int bytes;          // bytes read
} req;

/** @brief initializes listener socket that listens on the provided port on all of the interfaces for the host
 *
 * @param sock the listener_socket to initialize
 *
 * @param port the port on which to listen
 *
 * @return should return 0 for success, -1 for failing to listen
 */
int listener_init(Listener_Socket *sock, int port);

/** @brief accept a new connection and initialize a 5 second timeout
 *
 * @param sock the listener_socket from which to get the new connection
 *
 * @return a socket for the new connection, -1 if there is an error and sets the errno
 */
int listener_accept(Listener_Socket *sock);

/** @brief reads bytes from in into buf until either:
 *          (1) in has read n bytes,
 *          (2) in is out of bytes to return,
 *          (3) or there is an error reading bytes
 *
 * @param in the file descriptor or socket to read from
 *
 * @param buf buffer to put the data into
 *
 * @param n maximum bytes to read, <= size of buf
 *
 * @return number of bytes read, or -1 if error occurs. timeout is an error
 */
ssize_t read_n_bytes(int in, char *buf, size_t n);

/** @brief writes bytes to out from buf until either:
 *          (1) it has written exactly n bytes or
 *          (2) it encounters an error on write
 *
 * @param out the fd or sock to write to
 *
 * @param buf the buffer containing data to write
 *
 * @param n the number of bytes to write, must be <= buf
 *
 * @return number of bytes written or -1 for error
 */
ssize_t write_n_bytes(int out, char *buf, size_t n);

/** @brief reads bytes from src and places them in dest until either:
 *          (1) it has read/written exactly nbytes,
 *          (2) read returns 0, or
 *          (3) it encounters an error on read/write
 *
 * @param src fd or sock to read from
 *
 * @param dst fd or sock to write to
 *
 * @param n number of bytes to read/write
 *
 * @return number of bytes written or -1 for error
 */
ssize_t pass_n_bytes(int src, int dst, size_t n);

/** @brief parses request from src in order to determine to handle PUT or GET
 *
 * @param r request to parse
 *
 * @param buf buffer to put request into
 *
 * @param read_size size of message
 */
int req_parse(req *r, char *buf, ssize_t read_size);

/** @brief handles a request by calling the appropriate function for the command
 *
 * @param r request to handle
 *
 * @return 0 for success, -1 for error
 */
int handle_req(req *r);

/** @brief handles a GET request by sending the appropriate response to the client
 * 
 *  @param r request to handle
 * 
 *  @return 0 for success, -1 for error
 */
int get(req *r);

/** @brief handles a PUT request by sending the appropriate response to the client
 * 
 *  @param r request to handle
 * 
 *  @return 0 for success, -1 for error
 */
int put(req *r);

/** @brief handles a TRACE request by sending the appropriate response to the client
 * 
 *  @param r request to handle
 * 
 *  @return 0 for success, -1 for error
 */
int trace(req *r);

#endif /* HTTPSERVER_H */