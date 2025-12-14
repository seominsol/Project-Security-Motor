#include "Core.h"
#include "main.h"
#include "Motor.h"
#include "RC552.h"
#include "IR.h"


extern TIM_HandleTypeDef htim2;

#define MAX_MSG_QUEUE 20
uint8_t core_msg_queue[MAX_MSG_QUEUE];  // 메시지 저장 큐
uint8_t core_msg_head = 0;                           // 입력 위치
uint8_t core_msg_tail = 0;                           // 출력 위치
uint8_t Login = 0;


void core_init(){

	MFRC522_Init();
	UIDManager_Init();
	Motor_Sleep();

	HAL_TIM_Base_Start(&htim2);
	reset_ir_state();
}



void core_update(){
    if (core_msg_head == core_msg_tail) return; // 큐 비어있음
    uint8_t *msg = core_msg_queue;

    char buf[32];
    sprintf(buf, "msg[0]: 0x%02X\r\n", msg[0]);
    HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);

    // === 메시지 해석 및 처리 ===
    switch (msg[0]) {
        case RFID_LOGIN:
        	Motor_Awake();
        	break;

        case RFID_LOGOUT:

        	Motor_Sleep();
        	break;


        case RECEIVE_IR_SIG:

        	uint8_t cmd = getIRCMD();

        	if 		(cmd == IRCMD1) 	setRotateLeft();
        	else if	(cmd == IRCMD2) 	setRotateRight();
        	else if	(cmd == IRCMD3) 	MotorOnOffToggle();

        	else if (cmd == IRCMD4)		{

        		setUIDMgrMode(2);

        	}
        	else if (cmd == IRCMD5)		Logout();
        	// 버튼 5 누를 시 로그아웃

        	break;

        default:
        	break;
    }

    // 큐 인덱스 갱신
    core_msg_tail = (core_msg_tail + 1) % MAX_MSG_QUEUE;


}

void Logout(){
	Motor_Sleep();
	HAL_UART_Transmit(&huart2, (uint8_t*)"Logout\r\n", strlen("Logout\r\n"), HAL_MAX_DELAY);
}
void Core_PostMsg(uint8_t msg_type) {
    core_msg_queue[core_msg_head] = msg_type;
    core_msg_head = (core_msg_head + 1) % MAX_MSG_QUEUE;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_13) // Blue Button
    {
    	setUIDMgrMode(1);
       HAL_UART_Transmit(&huart2, (uint8_t*)"Delete Mode: Please tag your card\r\n", strlen("Delete Mode: Please tag your card\r\n"), HAL_MAX_DELAY);
    }
    else if (GPIO_Pin == GPIO_PIN_10) {
    	irCallBack();
    }

}



