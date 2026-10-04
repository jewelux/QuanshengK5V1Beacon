#ifndef APP_BEACON_H
#define APP_BEACON_H

#include <stdbool.h>
#include <stdint.h>

#include "driver/keyboard.h"

#ifdef ENABLE_BEACON_MO

#ifdef BEACON_PLATFORM_V3
/* V3 has its own four-item admin screen, separate from the stock menu. */
enum { MENU_BCN_FR, MENU_BCN_PW, MENU_BCN_ID, MENU_BCN_TN };
void BEACON_V3AdminInit(void);
bool BEACON_V3AdminKey(KEY_Code_t key, bool pressed, bool held);
void BEACON_V3AdminDisplay(void);
#endif

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
