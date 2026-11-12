#ifndef BLE_H_
#define BLE_H_

#include <stdbool.h>

bool ble_init(void);
bool ble_queue(const char *message);
void ble_poll(void);

#endif
