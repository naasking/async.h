# Async.h - asynchronous, stackless subroutines

Taking inspiration from protothreads and async/await as found in C#, Rust and JS,
this is a header-only async/await implementation for C based on Duff's device.

# Features

1. It's 100% pure, portable C.
2. It requires very little state (2 bytes).
3. It's not dependent on an OS.
4. It's a bit simpler to understand than protothreads because the async state
   is caller-saved rather than callee-saved.

# API

Function|Description
--------|-----------
*async_begin(state)*|Mark the beginning of an async subroutine
*async_end*|Mark the end of an async subroutine
*async_yield*|Yield execution until it's invoked again
*await(cond)*|Block progress until `cond` is true
*await_while(cond)*|Block progress while `cond` is true
*async_exit*|Terminate the current async subroutine
*async_call(func, state)*|Asynchronously call `func(state)` and return true if done executing (optional). You can also simply call `func(state)`directly which returns true/false.
*async_init(state)*|Initialize async subroutine state
*async_done(state)*|Returns true if async subroutine has completed execution, otherwise false
*async_run(cond)*|Runs your async function(s) from a non async function separated by `&` or `\|` to specify when all or one complete respectively

# Examples

The following is a sample program where multiple coroutines print they are starting, sleep for a specified time, and
print they slept the specified time
```c
#include "async.h"
#include "async-time.h"

struct print_after_state
{
    async_state;
    struct async_sleep_state async_sleep_state;
};

static async print_after(struct print_after_state *state, unsigned int duration)
{
    async_begin(state);

    printf("Starting %d second counter...\n", duration);
    async_yield;
    printf("Yield from %d second counter demo'd!\n", duration);
    async_init(&state->async_sleep_state);
    await(async_sleep(&state->async_sleep_state, duration * 1000));
    printf("Slept: %d seconds\n", duration);

    async_end;
}

int example_print_after(void)
{
    struct print_after_state a = {}, b = {}, c = {};

    async_run(
        print_after(&a, 3) &
        print_after(&b, 5) &
        print_after(&c, 1)
    );
    
    return 0;
}
```

Prints the following and runs a total of 5 seconds:

```
Starting 3 second counter...
Starting 5 second counter...
Starting 1 second counter...
Yield from 3 second counter demo'd!
Yield from 5 second counter demo'd!
Yield from 1 second counter demo'd!
Slept: 1 seconds
Slept: 3 seconds
Slept: 5 seconds
```

I ported the examples found in the protothreads distribution to async.h. Here
is the async.h equivalent of the protothreads sample on the home page:
```c
#include "async.h"
#include "async-time.h"

struct async pt;
struct timer timer;

async example(struct async *pt) {
    async_begin(pt);
    
    while(1) {
        if(initiate_io()) {
            timer_set(&timer);
            await(io_completed() || timer_expired(&timer));
            read_data();
        }
    }
    async_end;
}
```
Most of the code looks very similar, with the main exceptions being the
more concise names, and the fact that the async.h calls mostly don't need
to accept the async structure/local continuation as an argument.

Here is the same example as above, but where the timer is lifted to
a local parameter:
```c
#include "async.h"
#include "async-time.h"

typedef struct { 
    async_state;    // declare the asynchronous state
    struct timer timer;    // declare local state
} example_state;
example_state pt;

async example(example_state *pt) {
    async_begin(pt);
    
    while(1) {
        if(initiate_io()) {
            timer_set(&pt->timer);
            await(io_completed() || timer_expired(&pt->timer));
            read_data();
        }
    }
    async_end;
}
```
So local parameters simply need to be lifted to members of the async
state structure.

## Nested Async Calls

You can also execute nested async subroutines in a manner reminiscent of
fork-join parallelism:
```C
#include "async.h"

typedef struct { 
    async_state;
    struct async nested1;
    struct async nested2;
} example_state;
example_state pt;

async nested(struct async *pt){
    async_begin(pt);
    ...
    async_end;
}

async example(example_state *pt) {
    async_begin(pt);

    // fork two nested async subroutines and wait until both complete
    async_init(&pt->nested1);
    async_init(&pt->nested2);
    await(async_call(nested, &pt->nested1) & async_call(nested, &pt->nested2));
    // OR call directly:
    //await(nested(&pt->nested1) & nested(&pt->nested2));
    
    // fork two nested async subroutines and wait until at least one completes
    async_init(&pt->nested1);
    async_init(&pt->nested2);
    await(async_call(nested, &pt->nested1) | async_call(nested, &pt->nested2));
    // OR call the subroutines directly:
    //await(nested(&pt->nested1) | nested(&pt->nested2));

    async_end;
}
```

# Caveats

1. Due to compile-time bug, MSVC requires changing:
    `Project Properties > Configuration Properties > C/C++ > General > Debug Information Format`
   From "Program Database for Edit And Continue" to "Program Database".
2. As with protothreads, you have to be careful with switch statements
   within an async subroutine. Stick to this simple rule and you'll
   never have trouble: place every switch in its own function. This is
   generally a good practice anyway.
3. As with protothreads, you can't make blocking system calls and preserve
   the async semantics. These must be changed into non-blocking calls that
   test a condition.
