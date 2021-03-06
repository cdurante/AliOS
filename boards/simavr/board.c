/****************************************************************************
 * Copyright (c) 2014, Christopher Karle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   - Neither the name of the author nor the names of its contributors may be
 *     used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, AUTHOR OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************/
#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdio.h>
#include "board.h"
#include "kernel.h"
#include "libc_glue.h"
#include "mutex_test.h"
#include "queue_test.h"
#include "readline/history.h"
#include "readline/readline.h"
#include "semaphore_test.h"
#include "shell/shell.h"
#include "timer_test.h"
#include "uart/avr_uart0.h"

/****************************************************************************
 *
 ****************************************************************************/
extern uint8_t __stack[];

/****************************************************************************
 *
 ****************************************************************************/
static ReadlineData readlineData = READLINE_DATA(256);
static HistoryData historyData = HISTORY_DATA(4);
static Task task0;
static CharDev uart0;

/****************************************************************************
 *
 ****************************************************************************/
static void taskListCmd(int argc, char* argv[])
{
   taskList();
}

/****************************************************************************
 *
 ****************************************************************************/
static const ShellCmd SHELL_CMDS[] =
{
   {"tl", taskListCmd},
   {"mutex_test", mutexTestCmd},
   {"queue_test", queueTestCmd},
   {"semaphore_test", semaphoreTestCmd},
   {"timer_test", timerTestCmd},
   {NULL, NULL}
};

/****************************************************************************
 *
 ****************************************************************************/
unsigned long taskScheduleTick(bool adj, unsigned long ticks)
{
   if (!adj)
   {
      if (ticks)
         TCCR2B = 0x04;
      else
         TCCR2B = 0x00;
   }

   return 0;
}

/****************************************************************************
 *
 ****************************************************************************/
void taskIdle()
{
   cli();
   sleep_enable();
   sei();
   sleep_cpu();
   sleep_disable();
   _NOP();
}

/****************************************************************************
 *
 ****************************************************************************/
ISR(TIMER2_COMPA_vect)
{
   _taskTick(1);
   _taskPreempt(true);
}

/****************************************************************************
 *
 ****************************************************************************/
int main()
{
   void* stackBase = __stack - TASK0_STACK_SIZE + 1;

   taskInit(&task0, "main", TASK_LOW_PRIORITY, stackBase, TASK0_STACK_SIZE);
   uart0Init(&uart0);
   libcInit(&uart0);

   /* timer2 default tick (1ms) */
   OCR2A = F_CPU / TASK_TICK_HZ / 64;
   TCCR2A = 0x02;
   TIMSK2 = 0x02;

   set_sleep_mode(SLEEP_MODE_IDLE);

   puts("AliOS on AVR");
   sei();

   mutexTest();
   queueTest();
   semaphoreTest();
   timerTest();

   taskSetData(READLINE_DATA_ID, &readlineData);
   taskSetData(HISTORY_DATA_ID, &historyData);
   shellRun(SHELL_CMDS);

   return 0;
}
