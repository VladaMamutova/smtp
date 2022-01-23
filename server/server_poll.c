#include "log.h"
#include "server_poll.h"

#include <sys/poll.h>
#include <stdio.h>
#include <stdlib.h>

int start_poll(poll_args server_poll)
{
    int ready = poll(server_poll.fds, (nfds_t)server_poll.nfds, -1); // -1 - infinite timeout
    if (ready < -1) {
        log_e("%s", "poll() failed");
        return -1;
    }

    log_i("Ready descriptors: %d.\n", ready);


    return 1;
}
