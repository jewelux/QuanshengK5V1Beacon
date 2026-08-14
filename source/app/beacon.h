#ifndef APP_BEACON_H
#define APP_BEACON_H

#include <stdbool.h>
#include <stdint.h>

#include "driver/keyboard.h"

#ifdef ENABLE_BEACON_MO

void BEACON_Init(bool admin_requested);
void BEACON_TimeSlice10ms(void);
bool BEACON_ProcessKey(KEY_Code_t key, bool pressed, bool held);
void BEACON_Display(void);
bool BEACON_IsAdmin(void);

int32_t BEACON_GetMenuValue(uint8_t menu_id);
void BEACON_SetMenuValue(uint8_t menu_id, int32_t value);
void BEACON_GetMenuLimits(uint8_t menu_id, int32_t *minimum, int32_t *maximum);
void BEACON_FormatMenuValue(uint8_t menu_id, int32_t value, char *text);

#endif
#endif
