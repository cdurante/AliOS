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
#include <stdio.h>
#include "gic.h"
#include "kernel.h"
#include "libc_glue.h"
#include "mutex_test.h"
#include "queue_test.h"
#include "readline/history.h"
#include "semaphore_test.h"
#include "shell/shell.h"
#include "timer/sp804.h"
#include "timer_test.h"
#include "uart/pl011.h"

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
static unsigned long ALIGNED(16384) l1Table[4096];
static unsigned long ALIGNED(4096) l2Table[1024];

static PL011 pl011 = PL011_CREATE
(
   0x10009000,
   NULL,
   QUEUE_CREATE_PTR("pl011_rx", 1, 8)
);

static SP804 sp804 = SP804_CREATE(0x10011000);
static GIC gic = GIC_CREATE(0x1E000100, 0x1E001000);
static HistoryData historyData = HISTORY_DATA(10);
static Task task[SMP];

/****************************************************************************
 *
 ****************************************************************************/
static void sgiIRQ(unsigned int n, void* arg)
{
   if (n > 0)
      _taskPreempt(n > 1);
}

/****************************************************************************
 *
 ****************************************************************************/
static void taskTick(HWTimer* timer)
{
   _taskTick(1);
   taskPreempt(true);
}

/****************************************************************************
 *
 ****************************************************************************/
static void smpInit()
{
   void** jmpPtr = (void**) 0x10000030;
   extern unsigned long _smpInit[];
   *jmpPtr = _smpInit;
   smpWake(-1);
}

/****************************************************************************
 *
 ****************************************************************************/
void taskTimer(unsigned long ticks) {}

/****************************************************************************
 *
 ****************************************************************************/
void taskWait()
{
   __asm__ __volatile__("wfi");
}

/****************************************************************************
 *
 ****************************************************************************/
void taskPreempt(bool flag)
{
#if TASK_PREEMPTION
   gicSGI(&gic, -1, flag ? 2 : 1);
   _taskPreempt(flag);
#endif
}

/****************************************************************************
 *
 ****************************************************************************/
void smpWake(int cpu)
{
   gicSGI(&gic, cpu, 0);
}

/****************************************************************************
 *
 ****************************************************************************/
void _irqVector()
{
   gicIRQ(0, &gic);
}

/****************************************************************************
 *
 ****************************************************************************/
void smpMain(void* stack, unsigned long size)
{
   __asm__ __volatile__
   (
      "mcr p15, 0, %0, c3, c0, 0  \n"
      "mcr p15, 0, %1, c2, c0, 0  \n"
      "mov r12, #0                \n"
      "mcr p15, 0, r12, c7, c7, 0 \n"
      "mcr p15, 0, r12, c8, c7, 0 \n"
      "mrc p15, 0, r12, c1, c0, 0 \n"
      "orr r12, #0x00001000       \n"
      "orr r12, #0x00000005       \n"
      "mcr p15, 0, r12, c1, c0, 0 \n"
      : : "r" (0xFFFFFFFF), "r" (l1Table) : "r12", "memory"
   );

   gicInitSMP(&gic);
   taskInit(&task[cpuID()], "main+", TASK_LOW_PRIORITY, stack, size);
   enableInterrupts();

   for (;;)
      taskSleep(TASK_TICK_HZ);
}

/****************************************************************************
 *
 ****************************************************************************/
int main(void* vectors, unsigned long vectorSize, void* stack,
         unsigned long stackSize)
{
   unsigned long i;

   l1Table[0] = (unsigned long) l2Table | 0x00000011;

   for (i = 1; i < 4096; i++)
   {
      unsigned long addr = i << 20;

      l1Table[i] = addr | 0x00000C12;

      if ((addr >= 0x60000000) && (addr < (0x60000000 + BOARD_MEM_SIZE)))
         l1Table[i] |= 0x0000000C;
   }

   l2Table[0] = (unsigned long) vectors | 0x0000000E;

   for (i = 1; i < 256; i++)
      l2Table[i] = (i << 12) | 0x00000002;

   __asm__ __volatile__
   (
      "mcr p15, 0, %0, c3, c0, 0  \n"
      "mcr p15, 0, %1, c2, c0, 0  \n"
      "mov r12, #0                \n"
      "mcr p15, 0, r12, c7, c7, 0 \n"
      "mcr p15, 0, r12, c8, c7, 0 \n"
      "mrc p15, 0, r12, c1, c0, 0 \n"
      "orr r12, #0x00001000       \n"
      "orr r12, #0x00000005       \n"
      "mcr p15, 0, r12, c1, c0, 0 \n"
      : : "r" (0xFFFFFFFF), "r" (l1Table) : "r12", "memory"
   );

   taskInit(&task[cpuID()], "main", TASK_HIGH_PRIORITY, stack, stackSize);

   gicInit(&gic);

   pl011Init(&pl011, 4000000, 115200, PL011_DPS_8N1);
   gic.ctrl.addHandler(&gic.ctrl, 37, pl011IRQ, &pl011, false, 1 << cpuID());
   libcInit(&pl011.dev);

   sp804Init(&sp804, 1000000);
   gic.ctrl.addHandler(&gic.ctrl, 34, sp804IRQ, &sp804, true, 1 << cpuID());
   sp804.timer.callback = taskTick;
   sp804.timer.periodic = true;
   sp804.timer.load(&sp804.timer, sp804.timer.clk / TASK_TICK_HZ);
   sp804.timer.enable(&sp804.timer, true);

   gic.ctrl.addHandler(&gic.ctrl, 0, sgiIRQ, NULL, true, 0xFF);
   gic.ctrl.addHandler(&gic.ctrl, 1, sgiIRQ, NULL, true, 0xFF);
   gic.ctrl.addHandler(&gic.ctrl, 2, sgiIRQ, NULL, true, 0xFF);
   smpInit();

   puts("AliOS on ARM");
   enableInterrupts();

   mutexTest();
   queueTest();
   semaphoreTest();
   timerTest();

   taskSetData(HISTORY_DATA_ID, &historyData);

   shellRun(SHELL_CMDS);

   return 0;
}
