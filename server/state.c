#include "state.h"

static const char* states[STATE_NUMBER] = {
    "START", // 0
    "INIT", // 1
    "MAIL_STARTED", // 2
    "RCPT_RECEIVED", // 3
    "DATA_RECEIVING", // 4
    "DATA_RECEIVED", // 5
    "CLOSED" // 6
};

const char* state_to_str(state state)
{
    if (state < 0 || state > STATE_NUMBER - 1) {
        return "UNKNOWN";
    }

    return states[state];
}
