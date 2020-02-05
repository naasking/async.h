/*
 * Copyright (c) 2004-2005, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the protothreads library.
 *
 * Authors: Adam Dunkels <adam@sics.se>, Sandro Magi <naasking@gmail.com>
 *
 * $Id: example-buffer.c,v 1.5 2005/10/07 05:21:33 adam Exp $
 */

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>

#include "async-sem.h"

#define NUM_ITEMS 32
#define BUFSIZE 8

static int buffer[BUFSIZE];
static int bufptr;

static void
add_to_buffer(int item)
{
	printf("Item %d added to buffer at place %d\n", item, bufptr);
	buffer[bufptr] = item;
	bufptr = (bufptr + 1) % BUFSIZE;
}
static int
get_from_buffer(void)
{
	int item;
	item = buffer[bufptr];
	printf("Item %d retrieved from buffer at place %d\n",
		item, bufptr);
	bufptr = (bufptr + 1) % BUFSIZE;
	return item;
}

static int
produce_item(void)
{
	static int item = 0;
	printf("Item %d produced\n", item);
	return item++;
}

static void
consume_item(int item)
{
	printf("Item %d consumed\n", item);
}

static struct async_sem full, empty;

static async
producer(struct async *pt)
{
	static int produced;

	async_begin(pt);

	for (produced = 0; produced < NUM_ITEMS; ++produced) {

		await_sem(&full);

		add_to_buffer(produce_item());

		signal_sem(&empty);
	}

	async_end;
}

static async
consumer(struct async *pt)
{
	static int consumed;

	async_begin(pt);

	for (consumed = 0; consumed < NUM_ITEMS; ++consumed) {

		await_sem(&empty);

		consume_item(get_from_buffer());

		signal_sem(&full);
	}

	async_end;
}

static async
driver_thread(struct async *pt)
{
	static struct async pt_producer, pt_consumer;

	async_begin(pt);

	init_sem(&empty, 0);
	init_sem(&full, BUFSIZE);

	async_init(&pt_producer);
	async_init(&pt_consumer);

	await(producer(&pt_producer) & consumer(&pt_consumer));

	async_end;
}


int
example_buffer(void)
{
	struct async driver_pt;

	async_init(&driver_pt);

	while (!driver_thread(&driver_pt)) {

		/*
		 * When running this example on a multitasking system, we must
		 * give other processes a chance to run too and therefore we call
		 * usleep() resp. Sleep() here. On a dedicated embedded system,
		 * we usually do not need to do this.
		 */
#ifdef _WIN32
		Sleep(0);
#else
		usleep(10);
#endif
	}
	return 0;
}
