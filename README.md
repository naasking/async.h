# Async.h - asynchronous, stackless subroutines

Taking inspiration from protothreads and async/await as found in C#, Rust and JS,
this is a header-only async/await implementation for C based on Duff's device.

# Features

1. It's a bit simpler to understand than protothreads because the async state
   is caller-saved rather than callee-saved.
2. Subroutines can have persistent state that isn't just static state, because
   each async subroutine accepts its own struct it uses as a parameter, and
   the async state is stored there.
3. Because of the more flexible state, async subroutines can be nested
   in tree-like fashion which permits fork/join concurrency patterns.

# API

Function|Description
--------|-----------
*async_begin(state)*|Mark the beginning of an async subroutine
*async_end*|Mark the end of an async subroutine
*async_yield*|Yield execution until it's invoked again
*await(cond)*|Block progress until the `cond` is true
*await_while(cond)*|Block progress while `cond` is true
*async_exit*|Terminate the current async subroutine
*async_call(func, state)*|Asynchronously call `func(state)` and return true if done executing
*async_init(state)*|Initialize async subroutine state
*async_done(state)*|Returns true if async subroutine has completed execution, otherwise false

# Examples

I ported the examples found in the protothreads distribution to async.h. Here
is the async.h equivalent of the protothreads sample on the home page:
```
#include "async.h"

struct async pt;
struct timer timer;

async example(struct async *pt) {
    async_begin(pt);
    
    while(1) {
        if(initiate_io()) {
            timer_start(&timer);
            await(io_completed() || timer_expired(&timer));
            read_data();
        }
    }
    async_end;
}
```
Most of the code looks very similar, with the main exceptions being the
more concise names, and the fact that the async.h calls mostly don't need
to accept the async structure as an argument.

Here is the same example as above, but where the timer is lifted to
a local parameter:
```
#include "async.h"

typedef struct { 
    async_state;    // declare the asynchronous state
    timer timer;    // declare local state
} example_state;
example_state pt;

async example(example_state *pt) {
    async_begin(pt);
    
    while(1) {
        if(initiate_io()) {
            timer_start(&pt->timer);
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
```
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
    
    // fork two nested async  subroutines and wait until at least one completes
    async_init(&pt->nested1);
    async_init(&pt->nested2);
    await(async_call(nested, &pt->nested1) | async_call(nested, &pt->nested2));

    async_end;
}
```

# Caveats

 1. Due to compile-time bug, MSVC requires changing:
     Project Properties > Configuration Properties > C/C++ > General > Debug Information Format
    From "Program Database for Edit And Continue" to "Program Database".
 2. As with protothreads, you can't use switch statements within an
    async subroutine.
 3. As with protothreads, you can't make blocking system calls and preserve
    the async semantics. These must be changed into non-blocking calls that
    test a condition.
