/*************************************************
 *  Copyright (C) 1999-2008 by Zilog, Inc.
 *  All Rights Reserved
 *************************************************/

#include <eZ8.h>
#include <stdio.h>
#include <sio.h> // non-standard I/O
#include "main.h"
#include "uart.h"
#include "util.h"


//////////////////////////////////////////////////////////
//Interrupt routine UART 0

#pragma interrupt
void isr_uart0_rx(void) 
{
    //toggle_uart();
}

//////////////////////////////////////////////////////////
//Interrupt routine UART 1

#pragma interrupt
void isr_uart1_rx(void) 
{
  putchar('S');
}

//////////////////////////////////////////////////////////
//Intialize UART 0 
void init_uart0(void)
{
  init_uart(_UART0,_DEFFREQ,38400);      // Setup Uart0 
  select_port(_UART0);                   // Select port
  SET_VECTOR(UART0_RX, isr_uart0_rx);    // Define interrupt routine
  IRQ0ENH |= 0x10;                       // Set Interrupt Priority High
  IRQ0ENL |= 0x10;                       // Set Interrupt Priority High
}

//////////////////////////////////////////////////////////
//Intialize UART 1 
void init_uart1(void)
{
  init_uart(_UART1,_DEFFREQ,38400);      // Setup Uart1 
  select_port(_UART1);				     // Select port
  SET_VECTOR(UART1_RX, isr_uart1_rx);    // Define interrupt routine
  IRQ1ENH |= 0x10;                       // Set Interrupt Priority High
  IRQ1ENL |= 0x10;                       // Set Interrupt Priority High
}

////////////////////////////////////////////////////////
// getc
// This function reads chars from the uart
// Param none
// Return char
int getc(void)
{
  int temp = EOF;
  
  while (EOF == temp)
  {
    temp = getchar();
  }
  return temp;
}

////////////////////////////////////////////////////////
// readUart
// This function reads chars from the uart
// Param buffer - buffer to store the recieve data
// Param uart - UART to receive data from
// Param count - number of characters to receive
// Param timeout - time out in milliseconds
// Return bytes read
int readUart(char* buffer, int uart, int count, int timeout)
{
  int index = 0;
  int loop_count = 0;
  int timeout_count = 0;
  int result = 0;
  
  for(;;)
  {
    if ((uart == UART0) && (U0STAT0 & 0x80))
	  {
      // Data is ready
      buffer[index++] = U0RXD;
      if (index >= count)
      {
      	// All data received
        result = index;
        break;
      }
	  }
    if ((uart == UART1) && (U1STAT0 & 0x80))
	  {
      // Data is ready
      buffer[index++] = U1RXD;
      if (index >= count)
      {
      	// All data received
        result = index;
        break;
      }
	  }
    else
    {
    	uSecDelay(50);
    }

    loop_count++;
    if (loop_count >= 20)
    {
    	loop_count = 0;
      timeout_count++;
      if (timeout_count >= timeout)
      {
      	//Timedout waiting for message
        //result = ERROR;
        result = index;
				break;
      }
    }      
  }
  return result;
}
