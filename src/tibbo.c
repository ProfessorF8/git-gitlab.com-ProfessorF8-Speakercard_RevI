#include <stdio.h>
#include <eZ8.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "util.h"
#include "tibbo.h"
#include "uart.h"


////////////////////////////////////////////////////////
// initTibbo
// This function checks to see if the Tibbo Ethernet
// module is istalled and working. If it is then it
// will be initialized and made ready for use.
// Param none
// Return OK or ERROR
int initTibbo( void )
{
  char tibboVersion[] = {0x02, 'V', 0x0D};
  char tibboInit[] = {0x02, 'I', 0x0D};
  char tibboGIP[] = {0x02, 'G', 'I', 'P', 0x0D};

  
  char tibboFC[] = {0x02, 'S', 'F', 'C', '0', 0x0D};
  char tibboTP[] = {0x02, 'S', 'T', 'P', '1', 0x0D};
  char tibboIP[] = {0x02, 'S', 'I', 'P', '1', '9', '2', '.', '1', '6', '9', '.', '0', '0', '1', '.', '0', '0', '0', 0x0D}; 
  char tibboResponse[100];
  int  result = 0;
	int  count = 0;
  int  bytesRead;
  int  EOS = 0;
  int  initFlag = 0;    
    
  char IPAddrResponse[16];  

  // Lets talk Tibbo
  init_uart0();  // Intialize Uart 0 connected to Tibbo EM200
                 // Ethernet to RS-232 module

  // Enable the Tibbo Ethernet to RS232 module
  releaseTibboReset();
  
  // Wait for the module to come up
	mSecDelay(200);
  
  // Put the Tibbo module in programming mode
  enableTibboSerialProgramming();
	
  for (;;)
	{
    // Get the version
    memset(tibboResponse, 0, 100);
    sendMsgTibbo(tibboVersion, sizeof(tibboVersion));
    bytesRead = readUart(tibboResponse, UART0, 20, 50);
  
    if (bytesRead <= 0)
    {
    	// No response from Tibbo, Try to initialize the
      // module
      memset(tibboResponse, 0, 100);
      sendMsgTibbo(tibboInit, sizeof(tibboInit));
      bytesRead = readUart(tibboResponse, UART0, 20, 500);
    
      if (bytesRead <= 0)
      {
        // No response to init command so exit
        result = ERROR;
        break;
      }
      else
      {
      	// We got a response, Set initFlag
        initFlag = 1;
      }
    }
    
    // See if the IP address is correct
    memset(tibboResponse, 0, 100);
    sendMsgTibbo(tibboGIP, sizeof(tibboGIP));
    bytesRead = readUart(tibboResponse, UART0, 20, 50);
    
    if (bytesRead > 0)
    {
      // Find the end of the IP address string
      while(0x0D != tibboResponse[EOS])
      {
      	EOS++;
      }
      strncpy(IPAddrResponse, &tibboResponse[2], 
              EOS - 2);
     	if (!initFlag && !checkIPAddr(IPAddrResponse, IPAddrOctet1,
                       IPAddrOctet2, IPAddrOctet3,
                       IPAddrOctet4))
      {
      	// IP address is not OK so init Tibbo Ethernet to
        // RS232 module and then set it.
        memset(tibboResponse, 0, 100);
        sendMsgTibbo(tibboInit, sizeof(tibboInit));
        bytesRead = readUart(tibboResponse, UART0, 20, 500);
    
        if (bytesRead <= 0)
        {
          // No response to init command so exit
          result = ERROR;
          break;
        }
        else
        {
      	  // We got a response, Set initFlag
          initFlag = 1;
        }
      }
    }
    
    if (initFlag)
    {
      // Set the Flow Contorl for the serial
      // side of the Tibbo module to disabled
      sendMsgTibbo(tibboFC, sizeof(tibboFC));
      bytesRead = readUart(tibboResponse, UART0, 3, 50);
      if (bytesRead <= 0)
      {
        // No response so set ERROR and exit
        result = ERROR;
        break;
      }

      // Set the Transport Protocol for the network
      // side of the Tibbo module to TCP
      sendMsgTibbo(tibboTP, sizeof(tibboTP));
      bytesRead = readUart(tibboResponse, UART0, 3, 50);
      if (bytesRead <= 0)
      {
        // No response so set ERROR and exit
        result = ERROR;
        break;
      }
 
      // Fill in IP address
      sprintf(&tibboIP[4], "%03d.%03d.%03d.%03d",
              IPAddrOctet1, IPAddrOctet2,
              IPAddrOctet3, IPAddrOctet4);
      tibboIP[19] = 0x0D;
      sendMsgTibbo(tibboIP, sizeof(tibboIP));
      bytesRead = readUart(tibboResponse, UART0, 3, 50);
      if (bytesRead <= 0)
      {
        // No response so set ERROR and exit
        result = ERROR;
        break;
      }
    }
    result = 1;
    break;
  }

  // Reset the Tibbo module
  resetTibbo();

  return result;	
}

////////////////////////////////////////////////////////
// parseIPAddrString
// This function takes an IP address string in the form
// of 192.168.1.1 and returns the idividual nibbles as
// integers
// Param char* IPAddrString
// Param int*  octet1
// Param int*  octet2
// Param int*  octet3
// Param int*  octet4
// Return OK or ERROR
void parseIPAddrString(char* IPAddrString, int* octet1, int* octet2, int* octet3, int* octet4)
{
  char* t;
  
  t = strtok(IPAddrString, ".");
  *octet1 = atoi(t);
  
  t = strtok(NULL, ".");
  *octet2 = atoi(t);

  t = strtok(NULL, ".");
  *octet3 = atoi(t);

  t = strtok(NULL, ".");
  *octet4 = atoi(t);
}

#if 0

////////////////////////////////////////////////////////
// compareIPAddrStrings
// This function takes two IP address strings in the form
// of 192.168.1.1 and compares them. Returns TRUE if they
// are the same address
// Param char* IPAddrString1
// Param char* IPAddrString2
// Return OK or ERROR
int compareIPAddrStrings(char* IPAddrString1, char* IPAddrString2)
{
  int IPAddr1Octet1, IPAddr1Octet2, IPAddr1Octet3, IPAddr1Octet4; 
  int IPAddr2Octet1, IPAddr2Octet2, IPAddr2Octet3, IPAddr2Octet4;

  parseIPAddrString(IPAddrString1, &IPAddr1Octet1, &IPAddr1Octet2,
                    &IPAddr1Octet3, &IPAddr1Octet4);    
  parseIPAddrString(IPAddrString2, &IPAddr2Octet1, &IPAddr2Octet2,
                    &IPAddr2Octet3, &IPAddr2Octet4);

  return((IPAddr1Octet1 == IPAddr2Octet1) &&
         (IPAddr1Octet2 == IPAddr2Octet2) &&
         (IPAddr1Octet3 == IPAddr2Octet3) &&
         (IPAddr1Octet4 == IPAddr2Octet4));  
}

#endif

////////////////////////////////////////////////////////
// checkIPAddr
// This function takes an IP address string in the form
// of 192.168.1.1 and compares them to 4 octets. Returns TRUE if they
// are the same address
// Param char* IPAddrString1
// Param int   IPAddrOctet1;
// Param int   IPAddrOctet2;
// Param int   IPAddrOctet3;
// Param int   IPAddrOctet4;
// Return 1 or 0
int checkIPAddr(char* IPAddrString1, int IPAddr2Octet1,
                int IPAddr2Octet2, int IPAddr2Octet3,
                int IPAddr2Octet4)
{
  int IPAddr1Octet1, IPAddr1Octet2, IPAddr1Octet3, IPAddr1Octet4; 

  parseIPAddrString(IPAddrString1, &IPAddr1Octet1, &IPAddr1Octet2,
                    &IPAddr1Octet3, &IPAddr1Octet4);    

  return((IPAddr1Octet1 == IPAddr2Octet1) &&
         (IPAddr1Octet2 == IPAddr2Octet2) &&
         (IPAddr1Octet3 == IPAddr2Octet3) &&
         (IPAddr1Octet4 == IPAddr2Octet4));  
}

#if 0
////////////////////////////////////////////////////////
// checkTibbo
// This function checks to see if the Tibbo Ethernet
// module is istalled.\
// Param none
// Return OK or ERROR
int checkTibbo( void )
{
  char tibboVersion[] = {0x02, 'V', 0x0D};
  char tibboVersionResponse[20];
  int result = 0;
	int count = 0;
	
  for (;;)
	{
    memset(tibboVersionResponse, 0, 20);
    sendMsgTibbo(tibboVersion, sizeof(tibboVersion));
    result = readUart(tibboVersionResponse, UART0, 20, 500);
    if (result > 0)
    {
      result = OK;
      break;
    }
    count++;
    if (3 <= count)
    {
      result = ERROR;
      break;
    }
  }
  return result;
}
#endif

////////////////////////////////////////////////////////
// sendMsgTibbo
// This function takes a buffer and sends it to the
// Tibbo Ethernet to RS232 module
// Param buffer
// Param size
// Return none
void sendMsgTibbo(char* buffer, int size)
{
  int index = 0;
  
  // Turn on TX light here
  PAOUT &= ~TxLEDCtl;
  
  for(index = 0; index<size; index++)
	putchar(buffer[index]);
  
  // Turn off TX light here
  PAOUT |= TxLEDCtl;
}


#if 0
////////////////////////////////////////////////////////
// Get message Tibboo
// This function reads repsonses back from the Tibbo
// module when it is in the programming mode
// Return OK or ERROR if the message length is
// correct or not
int getMsgTibbo(char* buffer, int count)
{
  int index = 0;
  int result = OK;
  
  // Turn on RX light here
  PAOUT &= ~RxLEDCtl;
  
  // Store the data until the count is received
  for (;;)
  {
    buffer[index++] = getc();
    if (index >= count)
    {
      result = OK;
      break;
    }
    
    if (index > MAX_TIBBO_MSG_LENGTH)
    {
      result = ERROR;
      break;
    }
  }

  // Turn off RX light here
  PAOUT |= RxLEDCtl;

  return result;
} // End of getMsgTibbo
#endif

////////////////////////////////////////////////////////
// resetTibbo
// This function resets the Tibbo Ethernet to RS232
// module
// Param none
// Return none
void resetTibbo(void)
{
  PAOUT |= IP_RST;
  delay();
  PAOUT &= ~IP_RST;
}

////////////////////////////////////////////////////////
// releaseTibboReset
// This function removes the reset from the Tibbo
// Ethernet to RS232 serial module
// Param none
// Return none
void releaseTibboReset(void)
{
  PAOUT &= ~IP_RST;
}

////////////////////////////////////////////////////////
// enableTibboSerialProgramming
// This function sets the MD input on the Tibbo Ethernet
// to serial module enabling programming from the serial
// port
// Param none
// Return none
void enableTibboSerialProgramming(void)
{
	setAR(MD);
	delay();
	delay();
	delay();
	delay();
	clearAR();
}