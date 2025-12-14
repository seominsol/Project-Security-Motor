#ifndef __UID_MANAGER_H__
#define __UID_MANAGER_H__

#include "main.h"

#define UID_LENGTH           5       // UID
#define UID_TOTAL_LENGTH     6       //  1 + UID 5
#define MAX_UID_COUNT        20
#define FLASH_UID_BASE_ADDR  0x08060000  // Sector 7

void UIDManager_Init(void);
int  UIDManager_IsRegistered(uint8_t *uid);
void UIDManager_SaveUID(uint8_t *uid);
int  UIDManager_DeleteUID(uint8_t *uid);
int  UIDManager_ReadAll(uint8_t uids[][UID_LENGTH]);
uint8_t getUIDMgrMode();
void setUIDMgrMode(uint8_t);
#endif
