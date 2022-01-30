#include "state.h"

static const char* states[STATE_NUMBER] = {
    "START", // 0
    "INIT", // 1
    "CLOSED" // 2
};

const char* state_to_str(state state)
{
    if (state < 0 || state > STATE_NUMBER - 1) {
        return "UNKNOWN";
    }

    return states[state];
}
