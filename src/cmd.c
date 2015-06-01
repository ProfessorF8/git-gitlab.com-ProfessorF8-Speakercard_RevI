#include <eZ8.h>
#include <stdio.h>
#include <sio.h> // non-standard I/O
#include "string.h"
#include "stdlib.h"
#include "main.h"
#include "cmd.h"
#include "util.h"
#include "uart.h"

////////////////////////////////////////////////////////
// Command processor
// This function reads commands from the UART or TCP
// and processes them.
// Param cardAddress
// Return none
void processCommands(int cardAddress)
{
  char commandStr[17]; // Longest command is 16 characters plus one for null termination
  CMD_T command;
  int cs = 0;
  
  for (;;)
  {
    if (ERROR == getCommand(&commandStr))
    {
#ifdef DEBUG
      printf("Command error: Command exceeded %d characters\n", MAX_COMMAND_LENGTH);
#endif
    }
    else
    {
#ifdef DEBUG
      printf("Command string -> %s\n", commandStr);
#endif      
      decodeCommandStr(&commandStr, &command);
      
#ifdef DEBUG
      printf("Command: Address -> %X Command -> %s, Data0 -> %X, Data1 -> %X, Data2 -> %X, Data3 -> %X, Data4 -> %X, Data5 -> %X, CS -> %x\n",
             command.address, command.command, command.data[0], command.data[1], command.data[2], command.data[3], command.data[4],
             command.data[5], command.cs);
#endif
      
      // See if this command is for us. Commands with an
      // address of 0x00 are global commands
      if ((0x00 == command.address) || (cardAddress == command.address))
      {
        // See if checksumming is enabled. If so see if the
        // checksums match
        if (checksumEnabled)
        {
          cs = calculateChecksum(&commandStr);
#ifdef DEBUG
          printf("Command cs -> %x, CS => %x\n", command.cs, cs);
#endif          
        }
      
        // Compare the checksums
        if ((!checksumEnabled) || (cs == command.cs))
        {
          switch (command.command)
          {
            case 'A' :
              processA(command);
              break;
            case 'B' :
              processB(command);
              break;
            case 'C' :
              processC(command);
              break;
            case 'E' :
              processE(command);
              break;
            case 'F' :
              processF(command);
              break;
            case 'G' :
              processG(command);
              break;
            case 'P' :
              processP(command);
              break;
            case 'R' :
              processR(command);
              break;
            case 'S' :
              processS(command);
              break;
            case 'T' :
              processT(command);
              break;
//            case 'U' :
//              processU(command);
//              break;
            case 'W' :
              processW(command);
              break;
            default :
              sendBadCmdMsg();
              break;
          }
        }
        else
        {
          // Bad checksum. Tell the host the command was bad.
          sendBadCmdMsg();
        }
      }
    }
  }
} // End of processCommands

////////////////////////////////////////////////////////
// Get command
// This function reads commands from the UART or TCP
// Param command - pointer to buffer to store command
// Return OK or ERROR if the message termination
// character is not found
int getCommand(char* command)
{
  int index = 0;
  int result = OK;
  
  // Detect which UART to talk to
  for (;;)
    {
    	if (U0STAT0 & 0x80)
      {
      	select_port(_UART0);
        break;
      }
    	if (U1STAT0 & 0x80)
      {
      	select_port(_UART1);
        break;
      }
    }
  
  // Wait until the start of message character arrives
  for (;;)
  {
    command[index] = getc();
    if ('[' == command[index])
    {
      break;
    }
  }
  
  index++;
  
  // Turn on RX light here
  PAOUT &= ~RxLEDCtl;
  
  // Store the command until the ] is received
  for (;;)
  {
    command[index] = getc();
    if (']' == command[index])
    {
      command[index + 1] = 0;
      result = OK;
      break;
    }
    
    // Try to recover if a start of message is
    // received while in the middle of a command
    if ('[' == command[index])
    {
      // Start a new command
      index = 0;
      command[index] = '[';
    }
    
    index++;
    
    if (index > MAX_COMMAND_LENGTH)
    {
      result = ERROR;
      break;
    }
  }

  // Turn off RX light here
  PAOUT |= RxLEDCtl;

  return result;
} // End of getCommand

////////////////////////////////////////////////////////
// decodeCommandStr
// This function takes the received command string and
// parses the contents and coverts it to a CMD_T
// structurte
// Param commandStr - The received command from the host
// Param command - CMD_T decoded command
// Return 
void decodeCommandStr(char* commandStr, CMD_T* command)
{
  char tempStr[3] = 0;
  int index = 4;
  int index1 = 0;
  int count = 0;
  
  // Get the address
  memcpy(tempStr, &commandStr[1], 2);
  command->address = asciiToChar(tempStr);
  
  // Get the command
  command->command = commandStr[3];
  
  // Get the data
  while ('#' != commandStr[index])
  {
    count++;
    if (2 == count)
    {
      // We have a hex value
      memcpy(tempStr, &commandStr[index - 1], 2);
      command->data[index1++] = asciiToChar(tempStr);
#ifdef DEBUG
      printf("Temp str -> %s Data -> %X\n", tempStr, command->data[index1-1]);
#endif
      count = 0;
    }
    index++;
  }
  // Handle left over nibble
  if (1 == count)
  {
    memcpy(tempStr, &commandStr[index - 1], 1);
    command->data[index1] = asciiToNib(tempStr);
#ifdef DEBUG
    printf("Temp str -> %s Data -> %X\n", tempStr, command->data[index1]);
#endif
  }
  
  // Skip #
  index++;
  
  // Get the checksum
    memcpy(tempStr, &commandStr[index], 2);
    command->cs = asciiToChar(tempStr);
}

////////////////////////////////////////////////////////
// sendGoodCmdMsg
// This function sends a response to the host indicating
// the command is OK
// Param none
// Return none
void sendGoodCmdMsg(void)
{
  // [aa0#cc]
  // "aa" is the hex address (in ascii) echoed back.
  // "0"  for good.
  // "cc" Checksum   (in ascii)
  
  // Initialize the default good message data
  char response[8] = {'[', '?', '?', '0', '#', '0', '0', ']'};
  int cs = 0;
  
  // Fill in the card address
  charToAscii(&response[1], cardAddress);

  // If checksumming is enabled calculate the checksum and
  // update the response
  if (checksumEnabled)
  {
    cs = calculateChecksum(&response);
  }
  charToAscii(&response[5], cs);

  // Send the message
  sendMsg(&response);
}

////////////////////////////////////////////////////////
// sendBadCmdMsg
// This function sends a response to the host indicating
// there was a problem with the command
// Param none
// Return none
void sendBadCmdMsg(void)
{
  // [aa1#cc]
  // "aa" is the hex address (in ascii) echoed back.
  // "1"  for bad.
  // "cc" Checksum   (in ascii)

  // Initialize the default bad message data
  char response[8] = {'[', '?', '?', '1', '#', '0', '0', ']'};
  int cs = 0;
  
  // Fill in the card address
  charToAscii(&response[1], cardAddress);

  
  // If checksumming is enabled calculate the checksum and
  // update the response
  if (checksumEnabled)
  {
    cs = calculateChecksum(&response);
  }
  charToAscii(&response[5], cs);
  
  // Send the message
  sendMsg(&response);
}

////////////////////////////////////////////////////////
// sendMsg
// This function takes a buffer and sends it to the host
// Param buffer
// Return none
void sendMsg(char* buffer)
{
  int index = 0;
  
  // Enable RS485 transmitter
  PDOUT |= RS_IO_CTL;

  // Turn on TX light here
  PAOUT &= ~TxLEDCtl;
  
  do
  {
    putchar(buffer[index]);
  } while (buffer[index++] != ']');
  
  // Turn off TX light here
  PAOUT |= TxLEDCtl;

  // Wait until all the RS485 characters have been
  //  transmitted
  rs485Delay();
  
  // Disable RS485 transmitter
  PDOUT &= ~RS_IO_CTL;
}

////////////////////////////////////////////////////////
// Read Callins
// Function name: processA
// This function reads the privacy and call ins and
// returns a message to the host
// Param command
// Return none
void processA(CMD_T command)
{
  // [aa0ddeeffgg#cc]
  // "aa" is the hex address (in ascii) echoed back.
  // "0"  for good.
  // "dd" Privacy  2 (in ascii)
  // "ee" Privacy  1 (in ascii)
  // "ff" Call ins 2 (in ascii)
  // "gg" Call ins 1 (in ascii)
  // "cc" Checksum   (in ascii)

  // {91, speaker_addr_byte[0], speaker_addr_byte[1]], 48,
  // privacy[3], privacy[2], privacy[1], privacy[0],
  // callin[3], calln[2], callin[1], callin[0],
  // #, checksum[1], checksum[0], 93}

  int callButtons = 0;  
  int privacy = 0;
  char response[16];
  int cs = 0;
  
  // Read the call buttons and the privacy values
  callButtons = ~readCallButtons();
  privacy     = ~readPrivStatus();
  
  // Fill in the response message
  response[0] = '[';
  charToAscii(&response[1],  cardAddress);
  response[3] = '0';
  charToAscii(&response[4],  (privacy & 0xFF00) >> 8);
  charToAscii(&response[6],  privacy & 0x00FF);
  charToAscii(&response[8],  (callButtons & 0xFF00) >> 8);
  charToAscii(&response[10], callButtons & 0x00FF);
  response[12] = '#';
  
  // If checksumming is enabled calculate the checksum and
  // update the response
  //if (checksumEnabled)
  //{
    cs = calculateChecksum(&response);
  //}
 
  charToAscii(&response[13], cs);
  response[15] = ']';
  
  // Send the message
  sendMsg(&response);
}

////////////////////////////////////////////////////////
// Read Callins
// Function name: processB
// This function reads the privacy and call ins and
// returns a message to the host
// Param command
// Return none
void processB(CMD_T command)
{
  // [aa0ddeeffgg#cc]
  // "aa" is the hex address (in ascii) echoed back.
  // "0"  for good.
  // "dd" Privacy  2 (in ascii)
  // "ee" Privacy  1 (in ascii)
  // "ff" Call ins 2 (in ascii)
  // "gg" Call ins 1 (in ascii)
  // "cc" Checksum   (in ascii)
  
  // Same as A command
  processA(command);
}

////////////////////////////////////////////////////////
// Global clear
// This function reset everything to the initial
// values
// Param command
// Return none
void processC(CMD_T command)
{
  // [aa0#cc]
  // "aa" is the hex address (in ascii) echoed back.
  // "0"  for good.
  // "cc" Checksum (in ascii)
  
  // Clear the global relay variables
  icChannel  = 0x0000;
  zone       = 0x0000;
  apccRelays = 0x00;
  
  // Set the relays
  setRelays();
  
  // Send the response message
  sendGoodCmdMsg();
}

////////////////////////////////////////////////////////
// Extended Read Callins
// This function reads the privacy, call ins, and
// emergency state and returns a message to the host
// Param command
// Return none
void processE(CMD_T command)
{
  // [aa0ddeeffgghhjj#cc]
  // "aa" is the hex address (in ascii) echoed back.
  // "0" for good.
  // "dd" Privacy 2   (in ascii)
  // "ee" Privacy 1   (in ascii)
  // "ff" Call ins 2  (in ascii)
  // "gg" Call ins 1  (in ascii)
  // "hh" Emergency 2 (in ascii)
  // "jj" Emergency 1 (in ascii)
  // "cc" Checksum    (in ascii)

  int callButtons = 0;  
  int privacy = 0;
  int emergency = 0;
  char response[19];
  int cs = 0;
  
  // Read the call buttons, the privacy values and
  // the emergency call ins
  callButtons = ~readCallButtons();
  privacy     = ~readPrivStatus();

  // Fill in the response message
  response[0] = '[';
  charToAscii(&response[1],  command.address);
  response[3] = '0';
  charToAscii(&response[4],  (privacy & 0xFF00) >> 8);
  charToAscii(&response[6],  privacy & 0x00FF);
  charToAscii(&response[8],  (callButtons & 0xFF00) >> 8);
  charToAscii(&response[10], callButtons & 0x00FF);
  charToAscii(&response[12], (~emergency & 0xFF00) >> 8);
  charToAscii(&response[14], ~emergency & 0x00FF);
  response[16] = '#';
  
  // If checksumming is enabled calculate the checksum and
  // update the response
  if (checksumEnabled)
  {
    cs = calculateChecksum(&response);
  }
  
  charToAscii(&response[17], cs);
  response[19] = ']';

  // Send the message
  sendMsg(&response);
}

////////////////////////////////////////////////////////
// Extended Read Callins
// This function reads the privacy, call ins, and
// emergency state and returns a message to the host
// Param command
// Return none
void processF(CMD_T command)
{
  // [aa0ddeeffgghhjj#cc]
  // "aa" is the hex address (in ascii) echoed back.
  // "0"  for good.
  // "dd" Privacy 2   (in ascii)
  // "ee" Privacy 1   (in ascii)
  // "ff" Call ins 2  (in ascii)
  // "gg" Call ins 1  (in ascii)
  // "hh" Emergency 2 (in ascii)
  // "jj" Emergency 1 (in ascii)
  // "cc" Checksum    (in ascii)
  
  // Same as E command
  processE(command);
}

////////////////////////////////////////////////////////
// Extended Read Callins (New for Rev I)
// This function reads the privacy, call ins,
// emergency state and supervisor state and returns
// a message to the host
// Param command
// Return none
void processG(CMD_T command)
{
  // [aa0ddeeffgghhjjkkll#cc]
  // "aa" is the hex address (in ascii) echoed back.
  // "0"  for good.
  // "dd" Privacy 2    (in ascii)
  // "ee" Privacy 1    (in ascii)
  // "ff" Call ins 2   (in ascii)
  // "gg" Call ins 1   (in ascii)
  // "hh" Emergency 2  (in ascii)
  // "jj" Emergency 1  (in ascii)
  // "kk" Supervisor 2 (in ascii)
  // "ll" Supervisor 1 (in ascii)
  // "cc" Checksum     (in ascii)
  
  int callButtons = 0;  
  int privacy = 0;
  int emergency = 0;
  int supervisor = 0;
  char response[23];
  int cs = 0;
  
  // Read the call buttons, the privacy values and
  // the emergency call ins
  callButtons = ~readCallButtons();
  privacy     = ~readPrivStatus();
  supervisor  = ~readSupStatus();
  
  // Clear bits for active speaker channels
  supervisor = supervisor & supervisorChannels;

  // Fill in the response message
  response[0] = '[';
  charToAscii(&response[1],  command.address);
  response[3] = '0';
  charToAscii(&response[4],  (privacy & 0xFF00) >> 8);
  charToAscii(&response[6],  privacy & 0x00FF);
  charToAscii(&response[8],  (callButtons & 0xFF00) >> 8);
  charToAscii(&response[10], callButtons & 0x00FF);
  charToAscii(&response[12], (emergency & 0xFF00) >> 8);
  charToAscii(&response[14], emergency & 0x00FF);
  charToAscii(&response[16], (supervisor & 0xFF00) >> 8);
  charToAscii(&response[18], supervisor & 0x00FF);
  response[20] = '#';
  
  // If checksumming is enabled calculate the checksum and
  // update the response
  if (checksumEnabled)
  {
    cs = calculateChecksum(&response);
  }

  charToAscii(&response[21], cs);
  response[23] = ']';
  
  // Send the message
  sendMsg(&response);
}

////////////////////////////////////////////////////////
// Global Page
// This function sets the relays to perform a global
// page. It resets the IC and zone relays
// Param command
// Return none
void processP(CMD_T command)
{
  // All page command
  // Set the ALL Page (clear the other relays, does not include CC# relays)
  // [aa0#cc]
  // "aa" is the hex address (in ascii) echoed back.
  // "0" for good.
  // "cc" Checksum (in ascii)
  
  // Clear relays and set all page
  icChannel  = 0x0000;
  zone       = 0x0000;
  apccRelays |= ALL_PAGE_BIT;
  
  // Set the relays
  setRelays();
  
  // Send the response message
  sendGoodCmdMsg();
}

////////////////////////////////////////////////////////
// Read Relays
// This function reads the currently configured
// state of the relays
// Param command
// Return none
void processR(CMD_T command)
{
  // [aa0ddeeffgghh#cc]
  // "aa" is the hex address (in ascii) echoed back.
  // "0" for good.
  // "dd" Relays 4 (in ascii) for Relays 25 to 32
  // "ee" Relays 3 (in ascii) for Relays 17 to 24
  // "ff" Relays 2 (in ascii) for Relays 9 to 16
  // "gg" Relays 1 (in ascii) for Relays 1 to 8
  // "hh" APCC Relays
  // "cc" Checksum (in ascii)
  
  char response[16];
  int cs = 0;

  // Fill in the response message
  response[0] = '[';
  charToAscii(&response[1],  command.address);
  response[3] = '0';
  charToAscii(&response[4],  (icChannel & 0xFF00) >> 8);
  charToAscii(&response[6],  icChannel & 0x00FF);
  charToAscii(&response[8],  (zone & 0xFF00) >> 8);
  charToAscii(&response[10], zone & 0x00FF);
  nibToAscii(&response[12],  apccRelays & 0x00FF);
  response[13] = '#';
  
  // If checksumming is enabled calculate the checksum and
  // update the response
  if (checksumEnabled)
  {
    cs = calculateChecksum(&response);
  }
  
  charToAscii(&response[14], cs);
  response[16] = ']';

  // Send the message
  sendMsg(&response);
}

////////////////////////////////////////////////////////
// Activate Checksum Checking
// This function enables checksumming
// Param command
// Return none
void processS(CMD_T command)
{
  // [aa0#cc]
  // "aa" is the hex address (in ascii) echoed back.
  // "0" for good.
  // "cc" Checksum (in ascii)
  
  // Enable the checksum variable
  checksumEnabled = TRUE;

  // Send the response message
  sendGoodCmdMsg();
}

////////////////////////////////////////////////////////
// Read Relays (New for Rev I)
// This function reads the currently configured
// state of the relays including the supervisor
// relays
// Param command
// Return none
void processT(CMD_T command)
{
  // [aa0ddeeffgghh#cc]
  // "aa" is the hex address (in ascii) echoed back.
  // "0" for good.
  // "dd" Relays 4 (in ascii) for Relays 25 to 32
  // "ee" Relays 3 (in ascii) for Relays 17 to 24
  // "ff" Relays 2 (in ascii) for Relays 9 to 16
  // "gg" Relays 1 (in ascii) for Relays 1 to 8
  // "hh" Supervisor Relays high byte
  // "ii" Supervisor relays low byte
  // "jj" APCC Relays
  // "cc" Checksum (in ascii)
  
  char response[20];
  int cs = 0;

  // Fill in the response message
  response[0] = '[';
  charToAscii(&response[1],  command.address);
  response[3] = '0';
  charToAscii(&response[4],  (icChannel & 0xFF00) >> 8);
  charToAscii(&response[6],  icChannel & 0x00FF);
  charToAscii(&response[8],  (zone & 0xFF00) >> 8);
  charToAscii(&response[10], zone & 0x00FF);
  charToAscii(&response[12], (supervisorChannels & 0xFF00) >> 8);
  charToAscii(&response[14], supervisorChannels & 0x00FF);
  nibToAscii(&response[16],  apccRelays & 0x00FF);
  response[17] = '#';
  
  // If checksumming is enabled calculate the checksum and
  // update the response
  if (checksumEnabled)
  {
    cs = calculateChecksum(&response);
  }
  
  charToAscii(&response[18], cs);
  response[20] = ']';

  // Send the message
  sendGoodCmdMsg();
  }

////////////////////////////////////////////////////////
// Read Relays (New for Rev I)
// Set Relays extended
// This function sets the relays
// Param command
// Return none
void processU(CMD_T command)
{
  // [aa0ddeeffggh#cc]
  // "aa" is the hex address (in ascii) echoed back.
  // "dd" Speakers 1
  // "ee" Speakers 2
  // "ff" Zones 1
  // "gg" Zones 2
  // "h" One nibble. All page and Clock correction.
  //                 Bit 0 is all page
  //                 Bit 1 is clk cor 1
  //                 bit 2 is clk cor 2
  // "0" for good.
  // "cc" Checksum (in ascii)
  
  // Set the global relay variables based on
  // the input values
  icChannel  = (command.data[0] << 8) | command.data[1];
  zone       = (command.data[2] << 8) | command.data[3];
  apccRelays = command.data[4];
  
  // Set the relays
  setRelays();

  // Send the response message
  // The response for this message is to return
  // the status of the relays from the T command
  processT(command);
}

////////////////////////////////////////////////////////
// Set Relays
// This function sets the relays
// Param command
// Return none
void processW(CMD_T command)
{
  // [aa0ddeeffggh#cc]
  // "aa" is the hex address (in ascii) echoed back.
  // "dd" Speakers 1
  // "ee" Speakers 2
  // "ff" Zones 1
  // "gg" Zones 2
  // "h" One nibble. All page and Clock correction.
  //                 Bit 0 is all page
  //                 Bit 1 is clk cor 1
  //                 bit 2 is clk cor 2
  // "0" for good.
  // "cc" Checksum (in ascii)
  
  // Set the global relay variables based on
  // the input values
  icChannel  = (command.data[0] << 8) | command.data[1];
  zone       = (command.data[2] << 8) | command.data[3];
  apccRelays = command.data[4];
  
  // Set the relays
  setRelays();

  // Send the response message
  sendGoodCmdMsg();
}
