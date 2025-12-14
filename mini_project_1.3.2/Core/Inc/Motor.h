#include "main.h"

#define LEFT   1
#define RIGHT  2
#define OFF    0

#define THRESHOLD        500
#define STEP_DELAY_MS    2
#define PRINT_INTERVAL   500


extern uint8_t motorRotation;
extern uint8_t motorOn;


void stepMotor(int step);
void rotateLeft(void);
void rotateRight(void);
void setRotateLeft(void);
void setRotateRight(void);
void Run_Motor(void);

void MotorOnOffToggle();
void Motor_Sleep();
void Motor_Awake();
