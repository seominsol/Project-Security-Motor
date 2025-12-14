#include "main.h"
#define LEADER_MIN 3500
#define LEADER_MAX 5500
#define BIT0_MIN   365
#define BIT0_MAX   765
#define BIT1_MIN   1390
#define BIT1_MAX   1990

void reset_ir_state(void);
uint8_t decode(void);


uint8_t getIRCMD();
uint8_t isIRKeyReady(void);
void irCallBack();
