/* Host-side EEPROM regression test; compile against the prepared upstream tree. */
#include <assert.h>
#include "app/beacon.c"

static BEACON_Config_t saved;

#ifdef BEACON_PLATFORM_V3
void PY25Q16_ReadBuffer(uint32_t address, void *buffer, uint32_t size)
#else
void EEPROM_ReadBuffer(uint16_t address, void *buffer, uint8_t size)
#endif
{
	assert(address == BEACON_EEPROM_ADDRESS);
#ifdef BEACON_PLATFORM_V3
	assert(size == sizeof(saved));
#endif
	memcpy(buffer, &saved, size);
}

#ifdef BEACON_PLATFORM_V3
void PY25Q16_WriteBuffer(uint32_t address, const void *buffer, uint32_t size, bool append)
#else
void EEPROM_WriteBuffer(uint16_t address, const void *buffer)
#endif
{
	assert(address == BEACON_EEPROM_ADDRESS);
#ifdef BEACON_PLATFORM_V3
	assert(size == sizeof(saved));
#endif
#ifdef BEACON_PLATFORM_V3
	assert(!append);
#endif
	memcpy(&saved, buffer, sizeof(saved));
}

#ifdef BEACON_PLATFORM_V3
#include "app/beacon-admin.c"
bool gUpdateDisplay;
static void press(KEY_Code_t key) { BEACON_V3AdminKey(key, true, false); }
static void type(const char *text) { while (*text) press((KEY_Code_t)(*text++ - '0')); }
#endif

int main(void)
{
	memset(&saved, 255, sizeof(saved));
	BEACON_LoadConfig();
	assert(gBeaconConfig.power_percent == 1);
	assert(BEACON_GetMenuValue(MENU_BCN_FR) == 433500);

	BEACON_SetMenuValue(MENU_BCN_PW, 17);
	BEACON_SetMenuValue(MENU_BCN_FR, 433092);
	BEACON_LoadConfig();
	assert(gBeaconConfig.power_percent == 17);
	assert(BEACON_GetFrequency() == 43309200);

	BEACON_SetMenuValue(MENU_BCN_PW, 0);
	assert(saved.power_percent == 17);
	BEACON_SetMenuValue(MENU_BCN_PW, 101);
	assert(saved.power_percent == 17);
	BEACON_SetMenuValue(MENU_BCN_FR, 430012);
	BEACON_SetMenuValue(MENU_BCN_FR, 439988);
	assert(BEACON_GetFrequency() == 43309200);

	/* Every frequency from the old 12.5-kHz grid. */
	for (unsigned i = 0; i <= 798; ++i) {
		BEACON_DefaultConfig();
		gBeaconConfig.version = 3;
		gBeaconConfig.frequency_index = i;
		gBeaconConfig.power_percent = 17;
		BEACON_SaveConfig();
		BEACON_LoadConfig();
		unsigned expected = (43001250u + i * 1250u + 50u) / 100u;
		if (expected > 439987u)
			expected = 439987u;
		assert(gBeaconConfig.version == 4 && gBeaconConfig.power_percent == 17);
		assert(BEACON_GetFrequency() == expected * 100u);
		BEACON_LoadConfig();
		assert(BEACON_GetFrequency() == expected * 100u);
	}

	/* V2 stored five power levels and a separate tone index. */
	for (unsigned power = 0; power < 5; ++power) {
		BEACON_DefaultConfig();
		gBeaconConfig.version = 2;
		gBeaconConfig.code_tone = 5 | (power << 3);
		gBeaconConfig.power_percent = 12;
		gBeaconConfig.frequency_index = 279;
		BEACON_SaveConfig();
		BEACON_LoadConfig();
		assert(gBeaconConfig.power_percent == (power + 1) * 20);
		assert(BEACON_GetCode() == 5 && BEACON_GetToneIndex() == 12);
	}

#ifdef BEACON_PLATFORM_V3
	BEACON_DefaultConfig();
	BEACON_V3AdminInit();
	press(KEY_MENU); type("43309"); press(KEY_MENU);
	assert(editing && BEACON_GetFrequency() == 43350000); // incomplete
	press(KEY_2); press(KEY_MENU);
	assert(!editing && BEACON_GetFrequency() == 43309200);
	press(KEY_MENU); type("439988"); press(KEY_MENU);
	assert(editing && BEACON_GetFrequency() == 43309200); // out of range
	press(KEY_EXIT);
	press(KEY_DOWN); press(KEY_MENU); type("17"); press(KEY_MENU);
	BEACON_LoadConfig(); assert(gBeaconConfig.power_percent == 17);
	press(KEY_MENU); type("101"); press(KEY_MENU);
	assert(editing && gBeaconConfig.power_percent == 17);
	press(KEY_EXIT); press(KEY_MENU); type("50"); press(KEY_EXIT);
	assert(gBeaconConfig.power_percent == 17);
	press(KEY_MENU); type("0"); press(KEY_MENU);
	assert(editing && gBeaconConfig.power_percent == 17);
	press(KEY_EXIT); press(KEY_MENU); press(KEY_UP); press(KEY_MENU);
	assert(gBeaconConfig.power_percent == 18);
	BEACON_V3AdminKey(KEY_PTT, true, false);
	assert(gBeaconConfig.power_percent == 18);
#endif

	/* MO5 is the longest sequence: two complete words fit in the event buffer. */
	gBeaconConfig.code_tone = (gBeaconConfig.code_tone & ~7u) | 5u;
	BEACON_BuildSequence();
	assert(gBeaconEventCount == 40 && gBeaconEvents[19] == 7 && gBeaconEvents[39] == 7);

	saved.checksum ^= 1;
	BEACON_LoadConfig();
	assert(gBeaconConfig.power_percent == 1);
	return 0;
}
