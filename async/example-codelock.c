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
 * This file is part of the async.h library.
 *
 * Author: Adam Dunkels <adam@sics.se>, Sandro Magi <naasking@gmail.com>
 *
 * $Id: example-codelock.c,v 1.5 2005/10/06 07:57:08 adam Exp $
 */

/*
 *
 * This example shows how to implement a simple code lock. The code
 * lock waits for key presses from a numeric keyboard and if the
 * correct code is entered, the lock is unlocked. There is a maximum
 * time of one second between each key press, and after the correct
 * code has been entered, no more keys must be pressed for 0.5 seconds
 * before the lock is opened.
 *
 * This is an example that shows two things:
 * - how to implement a code lock key input mechanism, and
 * - how to implement a sequential timed routine.
 *
 * The program consists of two async functions, one that implements the
 * code lock reader and one that implements simulated keyboard input.
 *
 *
 */

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif
#include <stdio.h>

#include "async.h"

/*---------------------------------------------------------------------------*/
/*
 * The following definitions are just for the simple timer library
 * used in this example. The actual implementation of the functions
 * can be found at the end of this file.
 */
struct timer { int start, interval; };
static int  timer_expired(struct timer *t);
static void timer_set(struct timer *t, int usecs);
/*---------------------------------------------------------------------------*/
/*
 * This example uses two timers: one for the code lock async and
 * one for the simulated key input async.
 */
static struct timer codelock_timer, input_timer;
/*---------------------------------------------------------------------------*/
/*
 * This is the code that has to be entered.
 */
static const char code[4] = {'1', '4', '2', '3'};
/*---------------------------------------------------------------------------*/
/*
 * This example has two async and therefore has two async
 * control structures of type struct async. These are initialized with
 * async_init() in the entry function below.
 */
static struct async codelock_pt, input_pt;
/*---------------------------------------------------------------------------*/
/*
 * The following code implements a simple key input. Input is made
 * with the press_key() function, and the function key_pressed()
 * checks if a key has been pressed. The variable "key" holds the
 * latest key that was pressed. The variable "key_pressed_flag" is set
 * when a key is pressed and cleared when a key press is checked.
 */
static char key, key_pressed_flag;

static void
press_key(char k)
{
  printf("--- Key '%c' pressed\n", k);
  key = k;
  key_pressed_flag = 1;
}

static int
key_pressed(void)
{
  if(key_pressed_flag != 0) {
    key_pressed_flag = 0;
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/*
 * Declaration of the async function implementing the code lock
 * logic. The async function is declared using the `async` return type.
 * The function is declared with the "static" keyword since it
 * is local to this file. The name of the function is codelock_thread
 * and it takes one argument, pt, of the type struct async.
 *
 */
static async
codelock_thread(struct async *pt)
{
  /* This is a local variable that holds the number of keys that have
   * been pressed. Note that it is declared with the "static" keyword
   * to make sure that the variable is *not* allocated on the stack.
   */
  static int keys;

  /*
   * Declare the beginning of the async.
   */
  async_begin(pt);

  /*
   * We'll let the async loop until the async is
   * explicitly exited with async_exit.
   */
  while(1) {

    /*
     * We'll be reading key presses until we get the right amount of
     * correct keys.
     */ 
    for(keys = 0; keys < sizeof(code); ++keys) {

      /*
       * If we haven't gotten any keypresses, we'll simply wait for one.
       */
      if(keys == 0) {

	/*
	 * The await() function will block until the condition
	 * key_pressed() is true.
	 */
	await(key_pressed());
      } else {
	
	/*
	 * If the "key" variable was larger than zero, we have already
	 * gotten at least one correct key press. If so, we'll not
	 * only wait for the next key, but we'll also set a timer that
	 * expires in one second. This gives the person pressing the
	 * keys one second to press the next key in the code.
	 */
	timer_set(&codelock_timer, 1000);

	/*
	 * The following statement shows how complex blocking
	 * conditions can be easily expressed with asyncs and
	 * the await() function.
	 */
	await(key_pressed() || timer_expired(&codelock_timer));

	/*
	 * If the timer expired, we should break out of the for() loop
	 * and start reading keys from the beginning of the while(1)
	 * loop instead.
	 */
	if(timer_expired(&codelock_timer)) {
	  printf("Code lock timer expired.\n");
	  
	  /*
	   * Break out from the for() loop and start from the
	   * beginning of the while(1) loop.
	   */
	  break;
	}
      }

      /*
       * Check if the pressed key was correct.
       */
      if(key != code[keys]) {
	printf("Incorrect key '%c' found\n", key);
	/*
	 * Break out of the for() loop since the key was incorrect.
	 */
	break;
      } else {
	printf("Correct key '%c' found\n", key);
      }
    }

    /*
     * Check if we have gotten all keys.
     */
    if(keys == sizeof(code)) {
      printf("Correct code entered, waiting for 500 ms before unlocking.\n");

      /*
       * Ok, we got the correct code. But to make sure that the code
       * was not just a fluke of luck by an intruder, but the correct
       * code entered by a person that knows the correct code, we'll
       * wait for half a second before opening the lock. If another
       * key is pressed during this time, we'll assume that it was a
       * fluke of luck that the correct code was entered the first
       * time.
       */
      timer_set(&codelock_timer, 500);      
      await(key_pressed() || timer_expired(&codelock_timer));

      /*
       * If we continued from the await() statement without
       * the timer expired, we don't open the lock.
       */
      if(!timer_expired(&codelock_timer)) {
	printf("Key pressed during final wait, code lock locked again.\n");
      } else {

	/*
	 * If the timer expired, we'll open the lock and exit from the
	 * async.
	 */
	printf("Code lock unlocked.\n");
	async_exit;
      }
    }
  }

  /*
   * Finally, we'll mark the end of the async.
   */
  async_end;
}
/*---------------------------------------------------------------------------*/
/*
 * This is the second async in this example. It implements a
 * simulated user pressing the keys. This illustrates how a linear
 * sequence of timed instructions can be implemented with
 * asyncs.
 */
static async
input_thread(struct async *pt)
{
  async_begin(pt);

  printf("Waiting 1 second before entering first key.\n");
  
  timer_set(&input_timer, 1000);
  await(timer_expired(&input_timer));

  press_key('1');
  
  timer_set(&input_timer, 100);
  await(timer_expired(&input_timer));
  
  press_key('2');

  timer_set(&input_timer, 100);
  await(timer_expired(&input_timer));
  
  press_key('3');

  timer_set(&input_timer, 2000);
  await(timer_expired(&input_timer));
  
  press_key('1');

  timer_set(&input_timer, 200);
  await(timer_expired(&input_timer));
  
  press_key('4');

  timer_set(&input_timer, 200);
  await(timer_expired(&input_timer));
  
  press_key('2');
  
  timer_set(&input_timer, 2000);
  await(timer_expired(&input_timer));
  
  press_key('3');

  timer_set(&input_timer, 200);
  await(timer_expired(&input_timer));
  
  press_key('1');

  timer_set(&input_timer, 200);
  await(timer_expired(&input_timer));
  
  press_key('4');

  timer_set(&input_timer, 200);
  await(timer_expired(&input_timer));
  
  press_key('2');
  
  timer_set(&input_timer, 100);
  await(timer_expired(&input_timer));
  
  press_key('3');

  timer_set(&input_timer, 100);
  await(timer_expired(&input_timer));
  
  press_key('4');

  timer_set(&input_timer, 1500);
  await(timer_expired(&input_timer));
  
  press_key('1');

  timer_set(&input_timer, 300);
  await(timer_expired(&input_timer));
  
  press_key('4');

  timer_set(&input_timer, 400);
  await(timer_expired(&input_timer));
  
  press_key('2');

  timer_set(&input_timer, 500);
  await(timer_expired(&input_timer));
  
  press_key('3');

  timer_set(&input_timer, 2000);
  await(timer_expired(&input_timer));
  
  async_end;
}
/*---------------------------------------------------------------------------*/
/*
 * This is the main function. It initializes the two async
 * control structures and schedules the two asyncs. The main
 * function returns when the async the runs the code lock exits.
 */
int
example_codelock(void)
{
  /*
   * Initialize the two async control structures.
   */
  async_init(&input_pt);
  async_init(&codelock_pt);

  /*
   * Schedule the two asyncs until the codelock_thread() exits.
   */
  while(!codelock_thread(&codelock_pt)) {
    input_thread(&input_pt);
    
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
/*---------------------------------------------------------------------------*/
/*
 * Finally, the implementation of the simple timer library follows.
 */
#ifdef _WIN32

static int clock_time(void)
{ return (int)GetTickCount(); }

#else /* _WIN32 */

static int clock_time(void)
{
  struct timeval tv;
  struct timezone tz;   
  gettimeofday(&tv, &tz); 
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

#endif /* _WIN32 */

static int timer_expired(struct timer *t)
{ return (int)(clock_time() - t->start) >= (int)t->interval; }

static void timer_set(struct timer *t, int interval)
{ t->interval = interval; t->start = clock_time(); }
/*---------------------------------------------------------------------------*/
