/* homemade http server for bread hosting
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

