/**
 * This is a very small example that shows how to use
 * async.h. The program consists of two async subroutines that wait
 * for each other to toggle a variable.
 */

/* We must always include async.h in our asyncs code. */
#include "async.h"

#include <stdio.h> /* For printf(). */

/* Two flags that the two async functions use. */
static int async1_flag, async2_flag;

/**
 * The first async function. A async function must always
 * return an integer, but must never explicitly return - returning is
 * performed inside the async statements.
 *
 * The async function is driven by the main loop further down in
 * the code.
 */
static async
async1(struct async *pt)
{
	/* A async function must begin with async_begin() which takes a
	   pointer to a struct async. */
	async_begin(pt);

	/* We loop forever here. */
	while (1) {
		/* Wait until the other async has set its flag. */
		await(async2_flag != 0);
		printf("async 1 running\n");

		/* We then reset the other async's flag, and set our own
		   flag so that the other async can run. */
		async2_flag = 0;
		async1_flag = 1;

		/* And we loop. */
	}

	/* All async functions must end with async_end which takes a
	   pointer to a struct pt. */
	async_end;
}

/**
 * The second async function. This is almost the same as the
 * first one.
 */
static async
async2(struct async *pt)
{
	async_begin(pt);

	while (1) {
		/* Let the other async run. */
		async2_flag = 1;

		/* Wait until the other async has set its flag. */
		await(async1_flag != 0);
		printf("async 2 running\n");

		/* We then reset the other async's flag. */
		async1_flag = 0;

		/* And we loop. */
	}
	async_end;
}

/**
 * Finally, we have the main loop. Here is where the asyncs are
 * initialized and scheduled. First, however, we define the
 * async state variables pt1 and pt2, which hold the state of
 * the two asyncs.
 */
static struct async pt1, pt2;
void
example_small(int i)
{
	/* Initialize the async state variables with async_init(). */
	async_init(&pt1);
	async_init(&pt2);

	/*
	 * Then we schedule the two asyncs by repeatedly calling their
	 * async functions and passing a pointer to the async
	 * state variables as arguments.
	 */
	while (--i >= 0) {
		async1(&pt1);
		async2(&pt2);
	}
}
