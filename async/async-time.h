#include "async.h"

/**
 * Timer structure for tracking when a timer has expired
 */
struct timer
{
    unsigned int start;
    unsigned int interval;
};

/**
 * State structure for calls to the async_sleep function
 */
struct async_sleep_state
{
    async_state;
    struct timer timer;
};

/**
 * Initialize a timer with a specified time duration
 *
 * @param t timer struct to initialize
 * @param usecs number of milliseconds the timer is good for
 */
void timer_set(struct timer *t, unsigned int usecs);

/**
 * Determine if the supplied timer is expired
 * 
 * @param t timer struct to examine for expiration
 */
int timer_expired(struct timer *t);

/**
 * Sleep asynchronously for a specified duration
 * 
 * @param state async_sleep_state structure to hold state of operation
 * @param usecs number of milliseconds to sleep for
 */
async async_sleep(struct async_sleep_state *state, unsigned int usecs);