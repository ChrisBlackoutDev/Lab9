#ifndef BLE_H_
#define BLE_H_

#include <stdbool.h>

bool BLE_Init(void);
bool BLE_Queue(const char *message);
void BLE_Poll(void);

#endif
