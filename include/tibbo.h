#ifndef _TIBBO_H_
#define _TIBBO_H_

///////////////////////////////////////////////////////////
// Definitions
#define IPAddrOctet1 192
#define IPAddrOctet2 169
#define IPAddrOctet3 1
#define IPAddrOctet4 cardAddress

///////////////////////////////////////////////////////////
// Externals
extern char cardAddress;

///////////////////////////////////////////////////////////
// Function Prototypes
int  initTibbo( void );
int  checkTibbo( void );
void sendMsgTibbo(char* buffer, int size);
int  getMsgTibbo(char* buffer, int count);
int checkIPAddr(char* IPAddrString1, int IPAddr2Octet1,
                int IPAddr2Octet2, int IPAddr2Octet3,
                int IPAddr2Octet4);
void releaseTibboReset(void);
void resetTibbo(void);
void enableTibboSerialProgramming(void);


#endif
