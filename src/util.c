#include <eZ8.h>
#include "util.h"
#include "init.h"
#include "main.h"

////////////////////////////////////////////////////////
// Calculate checksum
// This function calculates the checksum for the chars
// between [ to the #
// Param none
// Return cs or ERROR
int calculateChecksum(char* command)
{
  int cs = 0;
  int index = 1;
  
  while ('#' != command[index])
  {
    cs += command[index++];
    if (index > MAX_COMMAND_LENGTH)
    {
      return ERROR;
    }
  }
  return cs;
} // End of calculateChecksum

////////////////////////////////////////////////////////
// Set the Intercom Channel
// Take a binary input and set the appropriate IC
// channel.
// Param channel IC channel to set
// Return none
void setICChannel(int channel)
{
  // Set RD1
  setRD(RD1);
  setDataBus(channel & 0x00FF);
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  clearRD();
  clearDataBus();
  // Set RD3
  setRD(RD3);
  setDataBus((channel & 0xFF00) >> 8);
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  clearRD();
  clearDataBus();
}

////////////////////////////////////////////////////////
// Set the zone relays
// Take a binary input and set the zone relays
// Param channels bits 0-15 corrispond to zone channel
//                1-16
// Return none
void setZone(int channels)
{
  // Set RD0
  setRD(RD0);
  setDataBus(channels & 0x00FF);
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  clearDataBus();
  clearRD();
  // Set RD2
  setRD(RD2);
  setDataBus((channels & 0xFF00) >> 8);
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  clearDataBus();
  clearRD();
}

////////////////////////////////////////////////////////
// Update supervisor relays
// The supervisor relays are special dual latching
// relays so this update function will need to be a little
// more sophisticated.
// Take a binary input and update the supervisor relays
// Param channels bits 0-15 corrispond to supervisor channel
//                1-16
// Return none
void updateSupervisor(int channels)
{
  // Flip the supervisor relays that are being used for intercom first
  clearSupervisor(channels);  

  // Flop the supervisor relays that are not being used for intercom
  setSupervisor(~channels);
  
  // Wait long enough for the relays to switch
  latchingDelay();
  
  // Clear the drivers
  setSupervisor(0x0000);
  clearSupervisor(0x0000);  
}

////////////////////////////////////////////////////////
// Set the supervisor relays
// Take a binary input and set the supervisor relays
// Param channels bits 0-15 corrispond to supervisor channel
//                1-16
// Return none
void setSupervisor(int channels)
{
  int index;
  unsigned char hiByte = 0;
  unsigned char lowByte = 0;
  
  for (index = 0; index < 8; index++)
  {
    lowByte |= ((channels >> index * 2) & 0x0001) << index;
    hiByte  |= ((channels >> (index * 2) + 1) & 0x0001) << index;
  }

  // Set RD6
  setRD(RD6);
  setDataBus(lowByte);
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  clearDataBus();
  clearRD();
  // Set RD7
  setRD(RD7);
  setDataBus(hiByte);
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  clearDataBus();
  clearRD();
}

////////////////////////////////////////////////////////
// Clear the supervisor relays
// Take a binary input and set the supervisor relays
// Param channels bits 0-15 corrispond to supervisor channel
//                1-16
// Return none
void clearSupervisor(int channels)
{
  int index;
  unsigned char hiByte = 0;
  unsigned char lowByte = 0;
  
  for (index = 0; index < 8; index++)
  {
    lowByte |= ((channels >> index * 2) & 0x0001) << index;
    hiByte  |= ((channels >> (index * 2) + 1) & 0x0001) << index;
  }
  
  // Set RD4
  setRD(RD4);
  setDataBus(lowByte);
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  clearDataBus();
  clearRD();
  // Set RD5
  setRD(RD5);
  setDataBus(hiByte);
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  clearDataBus();
  clearRD();
}

////////////////////////////////////////////////////////
// Output data on the Databus
// Param data value to be output on port c
// Return none
void setDataBus(int data)
{
  // Set Port C to Outputs
	PCADDR = DATADIR;   // Port C Data Dir = input: updated
	PCCTL  = 0x00;      // Port C Set to outputs
	PCADDR = NOFUNC;    // Clear Subregister Pointer
  PCOUT  = data;
}

////////////////////////////////////////////////////////
// Clear the Databus
// Switch Port C back to inputs abd set outputs to 0
// Param none
// Return none
void clearDataBus(void)
{
  // Set Port C to Inputs
	PCADDR = DATADIR;   // Port C Data Dir = input: updated
	PCCTL  = 0xFF;      // Port C Set to inputs
	PCADDR = NOFUNC;    // Clear Subregister Pointer
  PCOUT  = 0x00;
}

////////////////////////////////////////////////////////
// Set the RD set of address selects
// Param address of the chip selecct to set
// 0 = zone relays 1-8
// 1 = IC relays 1-8
// 2 = zone relays 9-16
// 3 = IC relays 9-16
// 4 = set sup relays 1-8
// 5 = set sup relays 9-16
// 6 = clear sup relays 1-8
// 7 = clear sup relays 9-16
// Return none
void setRD(int address)
{ 
  // Set the address bits first
  PBOUT = PBOUT & 0x8F;
  PBOUT = PBOUT | ((address << 4) & 0x70);
  
  asm("NOP");
  asm("NOP");
  asm("NOP");
  
  // Set the enable
  PBOUT &= ~R_EN_L;
  
  asm("NOP");
  asm("NOP");
  asm("NOP");
}

////////////////////////////////////////////////////////
// Clear the RD set of address selects
// Param none
// Return none
void clearRD(void)
{
  // Clear the enable
  PBOUT |= R_EN_L;
  asm("NOP");
  asm("NOP");
  asm("NOP");
  PBOUT = 0xFF;
  asm("NOP");
  asm("NOP");
  asm("NOP");
}

////////////////////////////////////////////////////////
// Set the AR set of address selects
// Param address of the chip selecct to set
// 0 = 
// 1 = 
// 2 = 
// 3 = 
// 4 = read sup status channels 1-8
// 5 = read sup status channels 9-16
// Return none
void setAR(int address)
{ 
  // Set the address bits first
  PBOUT = PBOUT & 0xF8;
  PBOUT = PBOUT | (address & 0x07);
  
  asm("NOP");
  asm("NOP");
  asm("NOP");
  
  // Set the enable
  PBOUT &= ~A_EN_L;
  
  asm("NOP");
  asm("NOP");
  asm("NOP");
}

////////////////////////////////////////////////////////
// Clear the AR set of address selects
// Param none
// Return none
void clearAR(void)
{
  // Clear the enable
  PBOUT |= A_EN_L;
  asm("NOP");
  asm("NOP");
  asm("NOP");
  PBOUT = 0xFF;
  asm("NOP");
  asm("NOP");
  asm("NOP");
}

////////////////////////////////////////////////////////
// Close the Clock Correction 1 relay
// Param none
// Return none
void setClkCor1Ctl(void)
{
	// Set Clock Control 1
	PDOUT |= CLK_COR1_CTL;
}

////////////////////////////////////////////////////////
// Open the Clock Correction 1 relay
// Param none
// Return none
void clearClkCor1Ctl(void)
{
	// Set Clock Control 1
	PDOUT &= ~CLK_COR1_CTL;
}

////////////////////////////////////////////////////////
// Close the Clock Correction 2 relay
// Param none
// Return none
void setClkCor2Ctl(void)
{
	// Set Clock Control 2
	PDOUT |= CLK_COR2_CTL;
}

////////////////////////////////////////////////////////
// Open the Clock Correction 2 relay
// Param none
// Return none
void clearClkCor2Ctl(void)
{
	// Set Clock Control 2
	PDOUT &= ~CLK_COR2_CTL;
}

////////////////////////////////////////////////////////
// Set the Program relay to the Zone bus
// Param none
// Return none
void setPgmRlyCtl(void)
{
	// Set PGM_RLY_CTL 
	PDOUT |= PGM_RLY_CTL;
}

////////////////////////////////////////////////////////
// Set the Program relay to the All Call/Program bus
// Param none
// Return none
void clearPgmRlyCtl(void)
{
	// Set PGM_RLY_CTL
	PDOUT &= ~PGM_RLY_CTL;
}

////////////////////////////////////////////////////////
// Read the external address switch
// Param none
// Return data the 8 bit switch value
unsigned char readAddress(void)
{
  // Initialize Port A
  // PA0 R_OE      OUTPUT  0 = Enable 1 = Disable
  // PA1 TxLEDCtl  OUTPUT	 0 = ON     1 = OFF
  // PA2 IP_RST    OUTPUT  0 = Normal 1 = Reset
  // PA3 RxLEDCtl  OUTPUT  0 = ON     1 = OFF
  // PA4 IP_RX     Alternate Function     
  // PA5 IP_TX     Alternate Function
  // PA6 SYSTEM_ID INPUT
  // PA7 ADD_EN    OUTPUT
  
  int data;
  unsigned char flipped = 0;
  int index;
  
  // Set the enable
  PAOUT &= ~ADD_EN;
  
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  
  // Read the data
  data = PCIN;

  asm("NOP");
  asm("NOP");

  // Clear the enable
  PAOUT |= ADD_EN;

  for (index=0; index<8; index++)
  {
  flipped |= ((data >> index) & 0x01) << (7 - index);
  }

  return flipped;
}

int readCallButtons(void)
{
  int data = 0;

  // Set AR3
  setAR(AR3);
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  data = data | PCIN;
  asm("NOP");
  asm("NOP");
  clearAR();
  data = data << 8;
  // Set AR2
  setAR(AR2);
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  data = data | PCIN;
  asm("NOP");
  asm("NOP");
  clearAR();
  return data;
}

int readPrivStatus(void)
{
  int data = 0;

  // Set AR1
  setAR(AR1);
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  data = data | PCIN;
  asm("NOP");
  asm("NOP");
  clearAR();
  data = data << 8;
  // Set AR0
  setAR(AR0);
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  data = data | PCIN;
  asm("NOP");
  asm("NOP");
  clearAR();
  return data;
}

int readSupStatus(void)
{
  int data = 0;

  // Set AR5
  setAR(AR2);
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  data = data | PCIN;
  asm("NOP");
  asm("NOP");
  clearAR();
  data = data << 8;
  // Set AR4
  setAR(AR0);
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  data = data | PCIN;
  asm("NOP");
  asm("NOP");
  clearAR();
  return data;
}

////////////////////////////////////////////////////////
// Delay loop
// Param none
// Return none
void delay(void)
{
  int x, y;
  for (x = 0; x < 1; x++)
  {
    for (y = 0; y < 10000; y++)
    {
    }
  }
}

////////////////////////////////////////////////////////
// uSecDelay
// Param mSeconds - Number of milliseconds to delay
// Return none
void mSecDelay(int mSeconds)
{
  int x;
  for (x = 0; x < mSeconds; x++)
  {
    uSecDelay(1000);
  }	
}

////////////////////////////////////////////////////////
// uSecDelay
// Param uSeconds - Number of microseconds to delay
// Return none
void uSecDelay(int uSeconds)
{
  int x;
  for (x = 0; x < uSeconds; x++)
  {
    // Each NOP is approx 54.25ns. 1 cycle at 18.432Mhz
    // To get a 1us delay. 16 NOPS plus a couple cycles
    // each loop for the decrement jump not zero instruction
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
  }
}

////////////////////////////////////////////////////////
// latchingDelay
// Param none
// Return none
void latchingDelay(void)
{
  int x;
  for (x = 0; x < 18432; x++)
  {
    // Each NOP is approx 54.25ns. 1 cycle at 18.432Mhz
    // To get a 10ms delay for the latching relays to
    // switch we must loop 18432 times through this
    // loop that is approximately 542ns long. 8 NOPS
    // plus a couple cycles each loop for the decrement
    // jump not zero instruction
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
  }
}

////////////////////////////////////////////////////////
// rs485Delay
// Param none
// Return none
void rs485Delay(void)
{
  int x;
  for (x = 0; x < 5050; x++)
  {
    // Each NOP is approx 54.25ns. 1 cycle at 18.432Mhz
    // To get a 10ms delay for the latching relays to
    // switch we must loop 18432 times through this
    // loop that is approximately 542ns long. 8 NOPS
    // plus a couple cycles each loop for the decrement
    // jump not zero instruction
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
    asm("NOP"); 
  }
}

////////////////////////////////////////////////////////
// asciiToNib
// This function converts the ascii hex character 
// to an integer value
// Param buffer - ascii hex char
// Return char - integer value
char asciiToNib(char* buffer)
{
  char result = 0;
  
  switch (*buffer)
  {
    case '0' :
      result = 0x0;
      break;
    case '1' :
      result = 0x1;
      break;
    case '2' :
      result = 0x2;
      break;
    case '3' :
      result = 0x3;
      break;
    case '4' :
      result = 0x4;
      break;
    case '5' :
      result = 0x5;
      break;
    case '6' :
      result = 0x6;
      break;
    case '7' :
      result = 0x7;
      break;
    case '8' :
      result = 0x8;
      break;
    case '9' :
      result = 0x9;
      break;
    case 'A' :
      result = 0xA;
      break;
    case 'B' :
      result = 0xB;
      break;
    case 'C' :
      result = 0xC;
      break;
    case 'D' :
      result = 0xD;
      break;
    case 'E' :
      result = 0xE;
      break;
    case 'F' :
      result = 0xF;
      break;
    default :
      result = 0x0;
  }
  
  return result;
}

////////////////////////////////////////////////////////
// asciiToChar
// This function converts the tw digit ascii hex
// character string to an integer value
// Param buffer - two ascii hex chars
// Return char - integer value
char asciiToChar(char* buffer)
{
  char result = 0;
  result = asciiToNib(&buffer[1]);
  result |= (asciiToNib(&buffer[0]) << 4);
  return result;
}

////////////////////////////////////////////////////////
// nibToAscii
// This function takes a pointer and char. It converts
// the char lower 4 bits to one hex characters and
// stores it at the pointer address
// Param dst - pointer to buffer location
// Param src - character to convert to ascii hex
// Return none
void nibToAscii(char* dst, char src)
{
  char translate[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                      '8', '8', 'A', 'B', 'C', 'D', 'E', 'F'};
  *dst = translate[src & 0x0F];
}

////////////////////////////////////////////////////////
// charToAscii
// This function takes a pointer and char. It converts
// the char to two hex characters and stores them at the
// pointer address
// Param dst - pointer to buffer location
// Param src - character to convert to ascii hex
// Return none
void charToAscii(char* dst, char src)
{
  nibToAscii(dst++, (src & 0xF0) >> 4);
  nibToAscii(dst, src & 0x0F);
}

////////////////////////////////////////////////////////
// setRelays
// This function sets the relays based on the global
// relay variables
// Param none
// Return none
void setRelays( void )
{
  // Figure out which supervisor channels have to be
  // switched to pass audio to the speakers. If the
  // all page is set the all the supervisor relays
  // need to be cleared.
  if (ALL_PAGE_BIT == ( apccRelays & ALL_PAGE_BIT))
  {
    // Update the variable for the supervisor channels
    supervisorChannels = 0xFFFF;

    // Override the intercom (speaker) relays
    setICChannel(0x0000);

    // Clear the program relay
    setPgmRlyCtl();
  }
  else
  {
    // Clear the program relay
    clearPgmRlyCtl();

    // Update the variable for the supervisor channels
    supervisorChannels = icChannel | zone;

    // Do the intercom (speaker) relays
    setICChannel(icChannel);
    
    // Do zone (page) relays
    setZone(zone);
  }
  
  // Do the supervisor relays
  updateSupervisor(supervisorChannels);    

  // Do the Clock Correction 1 relay
  if (CLK_COR1_BIT == ( apccRelays & CLK_COR1_BIT))
  {
    setClkCor1Ctl();
  }
  else
  {
    clearClkCor1Ctl();
  }

  // Do the Clock Correction 2 relay
  if (CLK_COR2_BIT == ( apccRelays & CLK_COR2_BIT))
  {
    setClkCor2Ctl();
  }
  else
  {
    clearClkCor2Ctl();
  }
}

