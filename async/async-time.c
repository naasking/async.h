#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#include "async.h"
#include "async-time.h"

#ifdef _WIN32

static int clock_time(void)
{
    return (int)GetTickCount();
}

#else /* _WIN32 */

static int clock_time(void)
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

#endif /* _WIN32 */

int timer_expired(struct timer *t)
{
    return (int)(clock_time() - t->start) >= (int)t->interval;
}

void timer_set(struct timer *t, unsigned int interval)
{
    t->interval = interval;
    t->start = clock_time();
}

async async_sleep(struct async_sleep_state *state, unsigned int usecs)
{
    async_begin(state);

    timer_set(&state->timer, usecs);
    await(timer_expired(&state->timer));

    async_end;
}
