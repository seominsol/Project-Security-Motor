#include "UID_MANAGER.h"
#include "string.h"
#include "stdio.h"

extern UART_HandleTypeDef huart2;

uint8_t uid_cache[MAX_UID_COUNT][UID_LENGTH];
int uid_cache_count = 0;
uint8_t Mode = 0;

uint8_t getUIDMgrMode() {
    return Mode;
}

void setUIDMgrMode(uint8_t mode) {
    Mode = mode;
}

static uint32_t GetFlashAddress(uint16_t index) {
    return FLASH_UID_BASE_ADDR + (index * UID_TOTAL_LENGTH);
}

void UIDManager_Init(void) {
    uid_cache_count = 0;

    for (int i = 0; i < MAX_UID_COUNT; i++) {
        uint32_t addr = GetFlashAddress(i);
        uint8_t *flash_ptr = (uint8_t*)addr;

        if (flash_ptr[0] == 0xA5) {
            uint8_t *uid_data = &flash_ptr[1];
            int is_zero = 1;

            for (int j = 0; j < UID_LENGTH; j++) {
                if (uid_data[j] != 0x00) {
                    is_zero = 0;
                    break;
                }
            }

            if (!is_zero) {
                memcpy(uid_cache[uid_cache_count], uid_data, UID_LENGTH);
                uid_cache_count++;
            }
        }
    }
}

int UIDManager_IsRegistered(uint8_t *uid) {
    for (int i = 0; i < uid_cache_count; i++) {
        if (memcmp(uid_cache[i], uid, UID_LENGTH) == 0) {
            return 1;
        }
    }
    return 0;
}

void UIDManager_SaveUID(uint8_t *uid) {
    if (UIDManager_IsRegistered(uid)) return;

    HAL_FLASH_Unlock();

    int saved = 0;

    // 우선 사용 가능한 공간(0xFF 또는 0x00)을 찾는다
    for (int i = 0; i < MAX_UID_COUNT; i++) {
        uint32_t addr = GetFlashAddress(i);
        uint8_t *flash_ptr = (uint8_t*)addr;

        if (flash_ptr[0] == 0xFF || flash_ptr[0] == 0x00) {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr, 0xA5) != HAL_OK) break;
            for (int j = 0; j < UID_LENGTH; j++) {
                if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr + 1 + j, uid[j]) != HAL_OK) break;
            }

            memcpy(uid_cache[uid_cache_count], uid, UID_LENGTH);
            uid_cache_count++;
            saved = 1;
            void UIDManager_Init(void);
            break;
        }
    }

    // 저장 실패 시 전체 erase 후 다시 시도
    if (!saved) {
        FLASH_EraseInitTypeDef eraseInit;
        uint32_t pageError = 0;

        eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
        eraseInit.Sector = FLASH_SECTOR_7;
        eraseInit.NbSectors = 1;
        eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

        if (HAL_FLASHEx_Erase(&eraseInit, &pageError) == HAL_OK) {
            uint32_t addr = GetFlashAddress(0);
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr, 0xA5);
            for (int j = 0; j < UID_LENGTH; j++) {
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr + 1 + j, uid[j]);
            }
            memcpy(uid_cache[0], uid, UID_LENGTH);
            uid_cache_count = 1;
        }
    }

    HAL_FLASH_Lock();

    // 캐시 출력
    char msg[64];
    HAL_UART_Transmit(&huart2, (uint8_t*)"=== UID Cache ===\r\n", strlen("=== UID Cache ===\r\n"), HAL_MAX_DELAY);
    for (int i = 0; i < uid_cache_count; i++) {
        snprintf(msg, sizeof(msg), "[%2d] %02X %02X %02X %02X %02X\r\n",
                 i,
                 uid_cache[i][0],
                 uid_cache[i][1],
                 uid_cache[i][2],
                 uid_cache[i][3],
                 uid_cache[i][4]);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }
    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
}

int UIDManager_DeleteUID(uint8_t *uid) {
    HAL_FLASH_Unlock();

    // 1. 먼저 캐시에서 해당 UID를 찾고 삭제
    int found = 0;
    for (int k = 0; k < uid_cache_count; k++) {
        if (memcmp(uid_cache[k], uid, UID_LENGTH) == 0) {
            for (int m = k; m < uid_cache_count - 1; m++) {
                memcpy(uid_cache[m], uid_cache[m + 1], UID_LENGTH);
            }
            uid_cache_count--;
            found = 1;
            break;
        }
    }

    if (!found) {
        HAL_FLASH_Lock();
        return 0; // UID not found
    }

    // 2. UID 저장 영역이 포함된 섹터를 지움
    FLASH_EraseInitTypeDef eraseInit;
    uint32_t sectorError;

    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.Sector    = FLASH_SECTOR_7;  // FLASH_UID_BASE_ADDR이 속한 섹터 번호로 변경
    eraseInit.NbSectors = 1;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3; // 2.7~3.6V

    if (HAL_FLASHEx_Erase(&eraseInit, &sectorError) != HAL_OK) {
        HAL_UART_Transmit(&huart2, (uint8_t*)"Flash erase fail\r\n", strlen("Flash erase fail\r\n"), HAL_MAX_DELAY);
        HAL_FLASH_Lock();
        return 0;
    }

    // 3. 캐시에 남아있는 UID들을 다시 Flash에 저장
    for (int i = 0; i < uid_cache_count; i++) {
        uint32_t addr = GetFlashAddress(i);

        // UID 마킹 바이트(0xA5) 먼저 저장
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr, 0xA5) != HAL_OK) {
            HAL_UART_Transmit(&huart2, (uint8_t*)"Flash write fail\r\n", strlen("Flash write fail\r\n"), HAL_MAX_DELAY);
            HAL_FLASH_Lock();
            return 0;
        }

        // UID 본문 저장
        for (int b = 0; b < UID_LENGTH; b++) {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr + 1 + b, uid_cache[i][b]) != HAL_OK) {
                HAL_UART_Transmit(&huart2, (uint8_t*)"Flash write fail\r\n", strlen("Flash write fail\r\n"), HAL_MAX_DELAY);
                HAL_FLASH_Lock();
                return 0;
            }
        }
    }

    HAL_FLASH_Lock();
    return 1;
}


int UIDManager_ReadAll(uint8_t uids[][UID_LENGTH]) {
    int count = 0;
    for (int i = 0; i < MAX_UID_COUNT; i++) {
        uint32_t addr = GetFlashAddress(i);
        uint8_t *flash_ptr = (uint8_t*)addr;

        if (flash_ptr[0] == 0xA5) {
            memcpy(uids[count], &flash_ptr[1], UID_LENGTH);
            count++;
        }
    }
    return count;
}
