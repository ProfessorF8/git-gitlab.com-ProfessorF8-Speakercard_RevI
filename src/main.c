/********************************************************************
 * Teradon Speaker Card Rev I firmware  
 *  
 ********************************************************************/
#include <eZ8.h>
#include <stdio.h>
#include <sio.h> // non-standard I/O
#include "main.h"
#include "cmd.h"
#include "timer.h"
#include "test_button.h"
#include "uart.h"
#include "string.h"
#include "stdlib.h"
#include "init.h"
#include "util.h"
#include "tibbo.h"

////////////////////////////////////////////////////////
// Global variables
char cardAddress;
char checksumEnabled;
int  icChannel          = 0;
int  zone               = 0;
int  supervisorChannels = 0;
char apccRelays         = 0; // Bit 0 is all page
                             // Bit 1 is clk cor 1
                             // Bit 2 is clk cor 2
int  tibboOnline        = 0;

////////////////////////////////////////////////////////
// Main program begins here 
void main ()
{
  int index;
  int callButtons = 0;

  // perform initialization
  Init ( );
	
  // Read the card address from the dip switch
  cardAddress = readAddress();

  // Set the supervisor relays
  updateSupervisor(0x0000);    

  // Use the card address switch setting of zero
  // to indicate that a selftest should be performed
  if (0 == cardAddress)
  {
    // Do selftest
    setClkCor1Ctl();
    delay();
    clearClkCor1Ctl();
    setClkCor2Ctl();
    delay();
    clearClkCor2Ctl();
    setPgmRlyCtl();
    delay();
    clearPgmRlyCtl();

    // Cycle channels
    for (index = 0; index < 16; index++)
    {
      setICChannel(0x0001 << index + 1);
      delay();
    }

    // Set the channels back to 0
    setICChannel(icChannel);
  
    // Cycle zone
    for (index = 0; index < 16; index++)
    {
      setZone(0x0001 << index + 1);
      delay();
    }

    // Set zone back to 0
    setZone(zone);
  }
  else
  {
    // Set the channels to 0
    setICChannel(icChannel);

    // Set zone to 0
    setZone(zone);
  }
  
  
  // Some card addresses are not allowed with the Tibbo Ethernet
  // to RS232 module. If the address switch is set to one of them
  // do not attemp to initialize the module.
  if ((0 != cardAddress) && (1 != cardAddress) && (255 != cardAddress))
  {
  // Initialize the Tibbo Ethernet to RS232 module
  tibboOnline = (initTibbo() == OK);
  }

  // Go to the command processing loop
  processCommands(cardAddress);  
} //End of main program
