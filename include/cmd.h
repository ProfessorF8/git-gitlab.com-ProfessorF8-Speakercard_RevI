#ifndef _CMD_H
#define _CMD_H

///////////////////////////////////////////////////////////
// Types
typedef struct cmd
{
  unsigned char  address;
  char command;
  unsigned char  cs;
  unsigned char data[8];
} CMD_T;

///////////////////////////////////////////////////////////
// Externals
extern char checksumEnabled;
extern char cardAddress;

///////////////////////////////////////////////////////////
// Function Prototypes
void processCommands(int cardAddress);
int getCommand(char* command);
void decodeCommandStr(char* commandStr, CMD_T* command);
void sendGoodCmdMsg(void);
void sendBadCmdMsg(void);
void sendMsg(char* buffer);
void processA(CMD_T command);
void processB(CMD_T command);
void processC(CMD_T command);
void processE(CMD_T command);
void processF(CMD_T command);
void processG(CMD_T command);
void processP(CMD_T command);
void processR(CMD_T command);
void processS(CMD_T command);
void processT(CMD_T command);
void processU(CMD_T command);
void processW(CMD_T command);

#endif // _CMD_H
