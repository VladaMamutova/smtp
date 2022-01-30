#ifndef STATE_H
#define STATE_H

#define STATE_NUMBER 3

typedef enum {
    STATE_START = 0,
    STATE_INIT = 1,
    STATE_CLOSED = 2,
} state;

const char* state_to_str(state state);

#endif // STATE_H
