#ifndef STATE_H
#define STATE_H

#define STATE_NUMBER 11

typedef enum {
    STATE_START = 0,
    STATE_INIT = 1,
    STATE_MAIL_STARTED = 2,
    STATE_RCPT_RECEIVED = 3,
    STATE_DATA_RECEIVING = 4,
    STATE_DATA_RECEIVED = 5,
    STATE_CLOSED = 6
} state;

const char* state_to_str(state state);

#endif // STATE_H
