
#ifndef MAIN
#define MAIN

// The int type is only 16 bits!!
#define int8_t  char
#define int16_t int
#define int32_t long


// Pin Definitions
// Port A
#define R_OE      0x01
#define TxLEDCtl  0x02	
#define IP_RST    0x04
#define RxLEDCtl  0x08
#define IP_RX     0x10     
#define IP_TX     0x20
#define SYSTEM_ID 0x40
#define ADD_EN    0x80

// Port B
#define A_ADDR_0  0x01
#define A_ADDR_1  0x02
#define A_ADDR_2  0x04
#define A_EN_L    0x08
#define R_ADDR_0  0x10
#define R_ADDR_1  0x20
#define R_ADDR_2  0x40
#define R_EN_L    0x80

// Port C
#define DO        0x01
#define D1        0x02
#define D2        0x04
#define D3        0x08
#define D4        0x10
#define D5        0x20
#define D6        0x40
#define D7        0x80

// Port D
#define CLK_COR1_CTL 0x01
#define CLK_COR2_CTL 0x02    
#define CI_INT_H     0x04 
#define RS_IO_CTL    0x08
#define RS_IN        0x10
#define RS_OUT       0x20
#define PGM_RLY_CTL  0x40

#define RD0 0
#define RD1 1
#define RD2 2
#define RD3 3
#define RD4 4
#define RD5 5
#define RD6 6
#define RD7 7

#define AR0 0
#define AR1 1
#define AR2 2
#define AR3 3
#define AR4 4
#define AR5 5
#define MD  6

#define ALL_PAGE_BIT 0x01
#define CLK_COR1_BIT 0x02
#define CLK_COR2_BIT 0x04

///////////////////////////////////////////////////////////
// Function Prototypes
void setStatus( int status );
void toggle_port(void);
void init_led_gpio(void);
void toggle_uart(void);
void init_test_button_gpio(void);
void turn_off_led(void);

#endif
