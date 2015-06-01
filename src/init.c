#include <eZ8.h>
#include "init.h"
#include "uart.h"
#include "util.h"

////////////////////////////////////////////////////////
// Power on initialization
// This function initializes the GPIO ports to  valid values
// and sets chip selects to avoid bus contention.
// Param none
// Return none
void Init( void )
{
  // Port Registers
  // PxADDR      Port A-H Address Register (Seletects subregisters )
  // PxCTL       Port A-H Control Register (Provides access to subregisters )
  // PxIN        Port A-H Input Data Register   Read Input Pins
  // PxOUT       Port A-H Output Data Register  Write Output Pins
	
  // Port Subregisters
  // PxDD        Data Direction                   0 = Output 1 = Input
  // PxAF        Alternate Function               0 = GPIO   1 = Alternate Function
  // PxOC        Ouput Control (Open-Drain)       0 = Drains Enabled 1 = Open Drain
  // PxDD        Hi Drive Enable                  0 = Normal 1 = High Power
  // PxSMRE      Stop Mode Recover Source Enable  0 = Normal 1 = Stop Mode 
	
  // PA0 R_OE      OUTPUT
  // PA1 TxLEDCtl  OUTPUT	
  // PA2 IP_RST    OUTPUT
  // PA3 RxLEDCtl  OUTPUT
  // PA4 IP_RX     Alternate Function     
  // PA5 IP_TX     Alternate Function
  // PA6 SYSTEM_ID INPUT
  // PA7 ADD_EN    OUTPUT
	
  // PB0 A_ADDR_0  OUTPUT
  // PB1 A_ADDR_1  OUTPUT
  // PB2 A_ADDR_2  OUTPUT
  // PB3 A_EN_L    OUTPUT
  // PB4 R_ADDR_0  OUTPUT
  // PB5 R_ADDR_1  OUTPUT
  // PB6 R_ADDR_2  OUTPUT
  // PB7 R_EN_L    OUTPUT
	
  // PC0 DO        IO
  // PC1 D1        IO
  // PC2 D2        IO
  // PC3 D3        IO
  // PC4 D4        IO
  // PC5 D5        IO
  // PC6 D6        IO
  // PC7 D7        IO
	
  // PD0 CLK_COR1_CTL  OUTPUT 0 = Relay Open 1 = Relay Closed       
  // PD1 CLK_COR2_CTL  OUTPUT 0 = Relay Open 1 = Relay Closed     
  // PD2 CI_INT_H      INPUT  
  // PD3 RS_IO_CTL     OUTPUT 0 = Receive 1 = Transmit
  // PD4 RS_IN         Alternat  Function
  // PD5 RS_OUT        Alternate Function
  // PD6 PGM_RLY_CTL   OUTPUT 0 = Relay Open 1 = Relay Closed

  // Port Registers
  // PxADDR      Port A-H Address Register (Seletects subregisters )
  // PxCTL       Port A-H Control Register (Provides access to subregisters )
  // PxIN        Port A-H Input Data Register   Read Input Pins
  // PxOUT       Port A-H Output Data Register  Write Output Pins
	
  // Port Subregisters
  // PxDD        Data Direction                   0 = Output 1 = Input
  // PxAF        Alternate Function               0 = GPIO   1 = Alternate Function
  // PxOC        Ouput Control (Open-Drain)       0 = Drains Enabled 1 = Open Drain
  // PxDD        Hi Drive Enable                  0 = Normal 1 = High Power
  // PxSMRE      Stop Mode Recover Source Enable  0 = Normal 1 = Stop Mode 


  // NOFUNC    0x00
  // DATADIR   0x01
  // ALTFUNC   0x02
  // OUTCTRL   0x03
  // HIDRVCTRL 0x04
  // STOPMODE  0x05

  // Initialize Port A
  // PA0 R_OE      OUTPUT  0 = Enable 1 = Disable
  // PA1 TxLEDCtl  OUTPUT	 0 = ON     1 = OFF
  // PA2 IP_RST    OUTPUT  0 = Normal 1 = Reset
  // PA3 RxLEDCtl  OUTPUT  0 = ON     1 = OFF
  // PA4 IP_RX     Alternate Function     
  // PA5 IP_TX     Alternate Function
  // PA6 SYSTEM_ID INPUT
  // PA7 ADD_EN    OUTPUT
  PAADDR = ALTFUNC;   // Select Alternate Function
  PACTL  = 0x30;      // Set Port PA4 and PA5 to Alternate Functions
  PAOUT  = 0x8E;      // Set Outputs to 1
  PAADDR = DATADIR;   // Select Data Direction
  PACTL  = 0x70;      // Set PA0-PA3 to Outputs
  PAADDR = NOFUNC;    // Clear Subregister Pointer
	
  // Initialize Port B
  PBOUT  = 0xFF;      // Set Port B High
  PBADDR = DATADIR;   // Port B Data Dir = input: updated
  PBCTL  = 0x00;      // Port B Set to outputs
  PBADDR = NOFUNC;	  // Clear Subregister Pointer
	
  // Initialize Port C
  // PC0 DO        IO
  // PC1 D1        IO
  // PC2 D2        IO
  // PC3 D3        IO
  // PC4 D4        IO
  // PC5 D5        IO
  // PC6 D6        IO
  // PC7 D7        IO
  PCADDR = DATADIR;   // Port C Data Dir = input: updated
  PCCTL  = 0xFF;      // Port C Set to inputs
  PCADDR = NOFUNC;	  // Clear Subregister Pointer
	
  // Intitialize Port D
  // PD0 CLK_COR1_CTL  OUTPUT 0 = Relay Open 1 = Relay Closed       
  // PD1 CLK_COR2_CTL  OUTPUT 0 = Relay Open 1 = Relay Closed     
  // PD2 CI_INT_H      INPUT  
  // PD3 RS_IO_CTL     OUTPUT 0 = Receive 1 = Transmit
  // PD4 RS_IN         Alternat  Function
  // PD5 RS_OUT        Alternate Function
  // PD6 PGM_RLY_CTL   OUTPUT 0 = Relay Open 1 = Relay Closed
  PDADDR = ALTFUNC;   // Select Alternate Function
  PDCTL  = 0x30;      // Set Port PD4 and PD5 to Alternate Functions
  PDOUT  = 0x00;      // Set Outputs PD0, PD1, PD6 = 1 PD3 = 0
  PDADDR = DATADIR;   // Select Data Direction
  PDCTL  = 0x34;      // Set PD0, PD1, PD3, PD6 to Outputs
  PDADDR = NOFUNC;    // Clear Subregister Pointer
	
  // Initialize channels
  setICChannel(0x0000);

  // Initialize zone
  setZone(0x0000);

  // Initialize clock correction and the program relay
  clearClkCor1Ctl();
  clearClkCor2Ctl();
  clearPgmRlyCtl();

  // Initialize the UART
  init_uart1();                   // Intialize Uart
} // End of initalization
