#include "main.h"

#define RFID_LOGIN   		0x01
#define RFID_LOGOUT    		0x02
#define RECEIVE_IR_SIG    	0x03

#define IRCMD1 				0x0C
#define IRCMD2 				0x18
#define IRCMD3 				0x5E
#define IRCMD4 				0x08
#define IRCMD5 				0x1C

void core_init();
void core_update();


void Core_PostMsg(uint8_t msg_type);
