/**
 * This is a another small example that shows how to use parts of async.h.
 * The program consists of an async subroutine which does the following:
 * 
 * 1. Prints that is starting a coroutine with a provided delay
 * 2. Demonstrates yielding from a subroutine
 * 3. Sleeps for a specified amount of time
 * 4. Prints the duration it slept before completion
 * 
 * This program also shows the use of the `async_run` macro for running
 * async coroutines from a regular function
 */
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "async.h"
#include "async-time.h"

/**
 * This structure is required because there will be an internal call to another
 * async function (async_sleep) which contains its own state. Rather than having
 * each state live in the top-level function call, this struct contains will
 * save the state uniquely for each call.
 */
struct print_after_state
{
    async_state;
    struct async_sleep_state async_sleep_state;
};

/**
 * print_after takes a duration of time to sleep
 * 
 * This demonstrates how async_yield works and also how multiple subroutines
 * seemingly operate simultaneously
 */
static async print_after(struct print_after_state *state, unsigned int duration)
{
    /* Declare the beginning of the async subroutine */
    async_begin(state);

    /* This print line will show in the order in which the coroutines are called
    However, each of coroutines will print this line before "Yield from..."
    line is printed. This demonstrates how the async_yield operates by yielding
    control to the next coroutine. */
    printf("Starting %d second counter...\n", duration);
    async_yield;
    printf("Yield from %d second counter demo'd!\n", duration);

    /* This will initialize the async_sleep_state to 0 */
    async_init(&state->async_sleep_state);
    /* Asynchronously sleep for the specified duration */
    await(async_sleep(&state->async_sleep_state, duration * 1000));

    /* This coroutine demonstration has slept for the provided duration */
    printf("Slept: %d seconds\n", duration);

    /* Declare the end of the async subroutine */
    async_end;
}

/**
 * This is the main function for this example.
 * 
 * This demo will be using 3 coroutines and each require their own state.
 * Each print_after call will print that they are starting in the order in which
 * they are called, yield control, then sleep the specified duration before
 * printing they have completed
 */
int example_print_after(void)
{
    /* Create 3 states. Memsetting each to zero like this effectively initialize
    each state at once */
    struct print_after_state a = {}, b = {}, c = {};

    /* Because we are not in an async portion of code, we cannot call 'await'.
    In order to gather and call our coroutines, we will use 'async_run' to run
    each. The & operator is used when you each call to run until completion,
    while the | operator is used when only want each call to run until the first
    completion occurs */
    async_run(
        print_after(&a, 3) &
        print_after(&b, 5) &
        print_after(&c, 1)
    );
    
    return 0;
}