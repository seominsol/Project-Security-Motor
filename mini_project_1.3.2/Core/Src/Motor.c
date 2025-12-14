#include "motor.h"

extern UART_HandleTypeDef huart2;
uint8_t motorRotation = 0;
uint8_t isIdle = 0;
uint8_t doSleep;
const uint8_t stepSeq[4][4] = {
  {1, 0, 0, 0},
  {0, 1, 0, 0},
  {0, 0, 1, 0},
  {0, 0, 0, 1}
};

void stepMotor(int step) {
  HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, stepSeq[step][0]);
  HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, stepSeq[step][1]);
  HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, stepSeq[step][2]);
  HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, stepSeq[step][3]);
}

int stepPos = 0;
volatile int moveDirection = 0;


void rotateLeft(void) {
  stepPos--;
  if (stepPos < 0) stepPos = 3;
  stepMotor(stepPos);
}

void rotateRight(void) {
  stepPos++;
  if (stepPos > 3) stepPos = 0;
  stepMotor(stepPos);
}

void setRotateRight(){
	motorRotation = RIGHT;
}
void setRotateLeft(){
	motorRotation = LEFT;
}



void MotorOnOffToggle(){

	isIdle = !isIdle;
	if (isIdle) HAL_UART_Transmit(&huart2, (uint8_t*)"Motor Idle\r\n", strlen("Motor Idle\r\n"), HAL_MAX_DELAY);
	else 		HAL_UART_Transmit(&huart2, (uint8_t*)"Motor run\r\n", strlen("Motor run\r\n"), HAL_MAX_DELAY);
	if (doSleep) HAL_UART_Transmit(&huart2, (uint8_t*)"Motor Sleep\r\n", strlen("Motor Sleep\r\n"), HAL_MAX_DELAY);
	else 		HAL_UART_Transmit(&huart2, (uint8_t*)"Motor awake\r\n", strlen("Motor awake\r\n"), HAL_MAX_DELAY);

}
void Run_Motor(){




	if (isIdle || doSleep)	return;


	if (motorRotation == LEFT)	rotateLeft();
	else						rotateRight();

}
void Motor_Sleep(){
	doSleep = 1;
}
void Motor_Awake(){
	isIdle = 1;
	doSleep = 0;
}


