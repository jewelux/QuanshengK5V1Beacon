/* Host-side EEPROM regression test; compile against the prepared upstream tree. */
#include <assert.h>
#include "app/beacon.c"

static BEACON_Config_t saved;

void EEPROM_ReadBuffer(uint16_t address, void *buffer, uint8_t size)
{
	(void)address;
	memcpy(buffer, &saved, size);
}

void EEPROM_WriteBuffer(uint16_t address, const void *buffer)
{
	(void)address;
	memcpy(&saved, buffer, sizeof(saved));
}

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

	saved.checksum ^= 1;
	BEACON_LoadConfig();
	assert(gBeaconConfig.power_percent == 1);
	return 0;
}
