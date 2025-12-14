#include "IR.h"

#include "stdio.h"
#include "string.h"

volatile uint32_t ir_data = 0;
volatile uint8_t bit_cnt = 0;
volatile uint8_t ir_key_ready = 0;
volatile uint8_t receiving = 0;
volatile uint32_t last_time = 0;
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart2;


uint8_t cmd;

uint8_t isIRKeyReady(){	return ir_key_ready; }

uint8_t getIRCMD(){
	return cmd;
}


void reset_ir_state(void) {
  ir_data = 0;
  bit_cnt = 0;
  receiving = 0;
  ir_key_ready = 0;
}
void irCallBack(){




			uint32_t now = __HAL_TIM_GET_COUNTER(&htim2);
	        uint32_t duration = (now - last_time);
	        last_time = now;

	    		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET){
	    			return;
	    		}




	        if (!receiving) {
	        	if (getUIDMgrMode()) return;

	          if (duration >= LEADER_MIN && duration <= LEADER_MAX) {
	        	HAL_UART_Transmit(&huart2, (uint8_t*)"IR!!\r\n", strlen("IR!!\r\n"), HAL_MAX_DELAY);
	            receiving = 1;
	            bit_cnt = 0;
	            ir_data = 0;
	          }
	        }
	    	else {
	          if (duration >= BIT0_MIN && duration <= BIT0_MAX) {
	            ir_data >>= 1;
	          }
	    			else if (duration >= BIT1_MIN && duration <= BIT1_MAX) {
	            ir_data >>= 1;
	            ir_data |= 0x80000000;
	          }
	    			else {
	            reset_ir_state();
	          }

	          if (++bit_cnt == 32) {
	            ir_key_ready = 1;
	            receiving = 0;
	          }
	        }
}


uint8_t decode(void) {
  char msg[64];
  snprintf(msg, sizeof(msg), "Received: 0x%08X\r\n", ir_data);
  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  cmd = (ir_data >> 16) & 0xFF;

	sprintf(msg, "CMD=0x%02X\r\n", cmd);
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
  reset_ir_state();
	return cmd;
}






