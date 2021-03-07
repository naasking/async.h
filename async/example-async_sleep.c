#include <stdio.h>
#include <stdbool.h>

#include "async.h"


typedef struct {
    async_state;
} AsyncState;

async coroutine1(AsyncState *state) {
    async_begin(state);

    while (1) {
        await(async_sleep(2));
        printf("Hello from coroutine 1.\n\n");
    }
    async_end;
}

async coroutine2(AsyncState *state) {
    async_begin(state);

    while (1) {
        await(async_sleep(8));
        printf("Hello from coroutine 2.\n\n");
    }
    async_end;
}

int main() {
    AsyncState state1;
    async_init(&state1);
    AsyncState state2;
    async_init(&state2);

    while (1) {
        coroutine1(&state1);
        coroutine2(&state2);
    }


    return 0;
}
