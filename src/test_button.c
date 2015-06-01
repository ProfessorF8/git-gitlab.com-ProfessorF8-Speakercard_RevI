/*************************************************
 *  Copyright (C) 1999-2008 by Zilog, Inc.
 *  All Rights Reserved
 *************************************************/

#include <eZ8.h>
#include "main.h"
#include "test_button.h"


//////////////////////////////////////////////////////////
//Interrupt routine

#pragma interrupt
void isr_C0(void) 
{
//   	toggle_port();
}


//////////////////////////////////////////////////////////
//Intialize Test Button 
void init_test_button(void)
{
	SET_VECTOR(C0, isr_C0); 
	IRQ2ENH |= 0x01;	// Set Interrupt Priority High
	IRQ2ENL |= 0x01;	// Set Interrupt Priority High
}


