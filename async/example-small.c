/**
 * This is a very small example that shows how to use
 * async.h. The program consists of two protothreads that wait
 * for each other to toggle a variable.
 */

/* We must always include async.h in our protothreads code. */
#include "async.h"

#include <stdio.h> /* For printf(). */

/* Two flags that the two protothread functions use. */
static int protothread1_flag, protothread2_flag;

/**
 * The first protothread function. A protothread function must always
 * return an integer, but must never explicitly return - returning is
 * performed inside the protothread statements.
 *
 * The protothread function is driven by the main loop further down in
 * the code.
 */
static async
protothread1(struct async *pt)
{
	/* A protothread function must begin with PT_BEGIN() which takes a
	   pointer to a struct pt. */
	async_begin(pt);

	/* We loop forever here. */
	while (1) {
		/* Wait until the other protothread has set its flag. */
		await(protothread2_flag != 0);
		printf("Protothread 1 running\n");

		/* We then reset the other protothread's flag, and set our own
		   flag so that the other protothread can run. */
		protothread2_flag = 0;
		protothread1_flag = 1;

		/* And we loop. */
	}

	/* All protothread functions must end with PT_END() which takes a
	   pointer to a struct pt. */
	async_end;
}

/**
 * The second protothread function. This is almost the same as the
 * first one.
 */
static async
protothread2(struct async *pt)
{
	async_begin(pt);

	while (1) {
		/* Let the other protothread run. */
		protothread2_flag = 1;

		/* Wait until the other protothread has set its flag. */
		await(protothread1_flag != 0);
		printf("Protothread 2 running\n");

		/* We then reset the other protothread's flag. */
		protothread1_flag = 0;

		/* And we loop. */
	}
	async_end;
}

/**
 * Finally, we have the main loop. Here is where the protothreads are
 * initialized and scheduled. First, however, we define the
 * protothread state variables pt1 and pt2, which hold the state of
 * the two protothreads.
 */
static struct async pt1, pt2;
void
example_small(int i)
{
	/* Initialize the protothread state variables with PT_INIT(). */
	async_init(&pt1);
	async_init(&pt2);

	/*
	 * Then we schedule the two protothreads by repeatedly calling their
	 * protothread functions and passing a pointer to the protothread
	 * state variables as arguments.
	 */
	while (--i >= 0) {
		protothread1(&pt1);
		protothread2(&pt2);
	}
}
