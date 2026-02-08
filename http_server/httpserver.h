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

typedef struct {
    int fd;
} Listener_Socket;

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
ssize_t read_n_bytes(int in, char buf[], size_t n);

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
ssize_t write_n_bytes(int out, char buf[], size_t n);

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

