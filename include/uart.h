/*************************************************
 *  Copyright (C) 1999-2008 by Zilog, Inc.
 *  All Rights Reserved
 *************************************************/

#ifndef UART
#define UART

///////////////////////////////////////////////////////////
// Definitions
#define UART0 0
#define UART1 1

///////////////////////////////////////////////////////////
//Function Prototypes
void init_uart0(void);
void init_uart1(void);
int getc(void);
int readUart(char* buffer, int uart, int count, int timeout);

#endif
