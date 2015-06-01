#ifndef UTIL
#define UTIL


///////////////////////////////////////////////////////////
// Definitions
#define MAX_COMMAND_LENGTH   17
#define MAX_TIBBO_MSG_LENGTH 15

#define OK     1
#define ERROR -1

///////////////////////////////////////////////////////////
// Externals
extern int  icChannel;
extern int  zone;
extern int  supervisorChannels;
extern char apccRelays;

///////////////////////////////////////////////////////////
// Function Prototypes
void setClkCor1Ctl(void);
void setClkCor2Ctl(void);
void clearClkCor1Ctl(void);
void clearClkCor2Ctl(void);
void setPgmRlyCtl(void);
void clearPgmRlyCtl(void);

void setICChannel(int channel);
void setZone(int channels);
void setDataBus(int data);
void clearDataBus(void);
void setRD(int address);
void clearRD(void);
void setAR(int address);
void clearAR(void);


void delay(void);
void mSecDelay(int mSeconds);
void uSecDelay(int uSeconds);
void latchingDelay(void);
void rs485Delay(void);

void updateSupervisor(int channels);
void setSupervisor(int channels);
void clearSupervisor(int channels);

unsigned char readAddress(void);

int readCallButtons(void);
int readPrivStatus(void);
int readSupStatus(void);

int calculateChecksum(char* command);

char asciiToNib(char* buffer);
char asciiToChar(char* buffer);
void nibToAscii(char* dst, char src);
void charToAscii(char* dst, char src);
void setRelays( void );

#endif
