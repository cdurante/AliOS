/****************************************************************************
 * Copyright (c) 2016, Christopher Karle
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
#ifndef RL78_UART_H
#define RL78_UART_H

#include "char_dev.h"
#include "kernel.h"

/****************************************************************************
 *
 ****************************************************************************/
#define UART_CREATE(id, txQueue, rxQueue) {{}, id, {txQueue, rxQueue}}

/****************************************************************************
 *
 ****************************************************************************/
#define UART0 0
#define UART1 1
#define UART2 2
#define UART3 3

/****************************************************************************
 *
 ****************************************************************************/
#define UART_DPS_8N1 0x0017

/****************************************************************************
 *
 ****************************************************************************/
typedef struct
{
   CharDev dev;
   int id;

   struct
   {
      Queue* tx;
      Queue* rx;

   } queue;

} UART;

/****************************************************************************
 *
 ****************************************************************************/
void uartTxISR(UART* uart);

/****************************************************************************
 *
 ****************************************************************************/
void uartRxISR(UART* uart);

/****************************************************************************
 *
 ****************************************************************************/
void uartInit(UART* uart, unsigned long clk, unsigned long baud,
              unsigned short dps);

#endif
