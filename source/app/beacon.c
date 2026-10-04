/* Non-blocking ARDF homing beacon integrated into Dennis' normal UI loop. */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "app/beacon.h"
#include "driver/backlight.h"
#include "driver/bk4819.h"
#include "driver/eeprom.h"
#include "external/printf/printf.h"
#include "functions.h"
#include "helper/battery.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "ui/helper.h"
#include "ui/menu.h"
#include "ui/ui.h"

#define BEACON_DEFAULT_FREQUENCY_10HZ 43350000u
#define BEACON_MIN_FREQUENCY_10HZ     43001300u
#define BEACON_MAX_FREQUENCY_10HZ     43998700u
#define BEACON_FREQUENCY_STEP_10HZ    100u
#define BEACON_FREQUENCY_STEPS ((BEACON_MAX_FREQUENCY_10HZ - BEACON_MIN_FREQUENCY_10HZ) / BEACON_FREQUENCY_STEP_10HZ)
#define BEACON_EEPROM_ADDRESS         (199u * 16u)
#define BEACON_CONFIG_MAGIC           0xB34Du
#define BEACON_CONFIG_VERSION         4u
#define BEACON_CONFIG_VERSION_OLD     2u
#define BEACON_CODE_COUNT             6u
#define BEACON_TONE_MIN_HZ            400u
#define BEACON_TONE_MAX_HZ            1500u
#define BEACON_TONE_STEP_HZ           50u
#define BEACON_TONE_STEPS ((BEACON_TONE_MAX_HZ - BEACON_TONE_MIN_HZ) / BEACON_TONE_STEP_HZ)
#define BEACON_WPM                     12u
#define BEACON_UNIT_TICKS              (120u / BEACON_WPM)
#define BEACON_WARMUP_TICKS            75u
#define BEACON_PAUSE_TICKS             500u
#define BEACON_EVENT_MARK              0x80u
#define BEACON_MAX_EVENTS              64u

typedef struct __attribute__((packed)) {
	uint16_t magic;
	uint8_t version;
	uint8_t code_tone; /* bits 0..2: code, bits 3..7: tone index */
	uint16_t frequency_index;
	uint8_t power_percent;
	uint8_t checksum;
} BEACON_Config_t;

_Static_assert(sizeof(BEACON_Config_t) == 8u, "Beacon EEPROM record must remain eight bytes");

typedef enum {
	BEACON_IDLE,
	BEACON_WARMUP,
	BEACON_SEQUENCE,
	BEACON_PAUSE
} BEACON_State_t;

static BEACON_Config_t gBeaconConfig;
static BEACON_State_t gBeaconState;
static bool gBeaconAdmin;
static uint16_t gBeaconTicks;
static uint8_t gBeaconEvents[BEACON_MAX_EVENTS];
static uint8_t gBeaconEventCount;
static uint8_t gBeaconEventIndex;

static const char * const gBeaconCodeNames[BEACON_CODE_COUNT] = {
	"MO", "MOE", "MOI", "MOS", "MOH", "MO5"
};

static uint8_t BEACON_GetCode(void) { return gBeaconConfig.code_tone & 7u; }
static uint8_t BEACON_GetToneIndex(void) { return gBeaconConfig.code_tone >> 3; }
static uint16_t BEACON_GetTone(void) { return BEACON_TONE_MIN_HZ + BEACON_GetToneIndex() * BEACON_TONE_STEP_HZ; }
static uint32_t BEACON_GetFrequency(void) { return BEACON_MIN_FREQUENCY_10HZ + (uint32_t)gBeaconConfig.frequency_index * BEACON_FREQUENCY_STEP_10HZ; }

static uint8_t BEACON_Checksum(const BEACON_Config_t *config)
{
	const uint8_t *bytes = (const uint8_t *)config;
	uint8_t checksum = 0x5Au;
	for (uint8_t i = 0; i < sizeof(*config) - 1u; ++i)
		checksum ^= bytes[i];
	return checksum;
}

static void BEACON_SaveConfig(void);

static void BEACON_DefaultConfig(void)
{
	gBeaconConfig.magic = BEACON_CONFIG_MAGIC;
	gBeaconConfig.version = BEACON_CONFIG_VERSION;
	gBeaconConfig.code_tone = ((1000u - BEACON_TONE_MIN_HZ) / BEACON_TONE_STEP_HZ) << 3;
	gBeaconConfig.frequency_index = (BEACON_DEFAULT_FREQUENCY_10HZ - BEACON_MIN_FREQUENCY_10HZ) / BEACON_FREQUENCY_STEP_10HZ;
	gBeaconConfig.power_percent = 1u;
	gBeaconConfig.checksum = BEACON_Checksum(&gBeaconConfig);
}

static void BEACON_LoadConfig(void)
{
	EEPROM_ReadBuffer(BEACON_EEPROM_ADDRESS, &gBeaconConfig, sizeof(gBeaconConfig));
	if (gBeaconConfig.magic == BEACON_CONFIG_MAGIC &&
	    (gBeaconConfig.version == BEACON_CONFIG_VERSION_OLD || gBeaconConfig.version == 3u) &&
	    gBeaconConfig.checksum == BEACON_Checksum(&gBeaconConfig) &&
	    gBeaconConfig.frequency_index <= 798u) {
		bool valid = true;
		if (gBeaconConfig.version == BEACON_CONFIG_VERSION_OLD) {
			/* V2 byte 3 held code/power and byte 6 held the tone index. */
			const uint8_t old_selection = gBeaconConfig.code_tone;
			const uint8_t old_tone = gBeaconConfig.power_percent;
			valid = (old_selection & 0xC0u) == 0 && (old_selection & 7u) < BEACON_CODE_COUNT &&
			        ((old_selection >> 3) & 7u) < 5u && old_tone <= BEACON_TONE_STEPS;
			if (valid) {
				gBeaconConfig.code_tone = (old_selection & 7u) | (old_tone << 3);
				gBeaconConfig.power_percent = (((old_selection >> 3) & 7u) + 1u) * 20u;
			}
		} else {
			valid = BEACON_GetCode() < BEACON_CODE_COUNT && BEACON_GetToneIndex() <= BEACON_TONE_STEPS &&
			        gBeaconConfig.power_percent >= 1u && gBeaconConfig.power_percent <= 100u;
		}
		if (valid) {
			/* Migrate the old 12.5-kHz grid to the nearest in-range kHz. */
			uint32_t frequency_khz = (43001250u + (uint32_t)gBeaconConfig.frequency_index * 1250u + 50u) / 100u;
			if (frequency_khz > BEACON_MAX_FREQUENCY_10HZ / 100u)
				frequency_khz = BEACON_MAX_FREQUENCY_10HZ / 100u;
			gBeaconConfig.frequency_index = frequency_khz - BEACON_MIN_FREQUENCY_10HZ / 100u;
			gBeaconConfig.version = BEACON_CONFIG_VERSION;
			BEACON_SaveConfig();
			return;
		}
	}
	if (gBeaconConfig.magic != BEACON_CONFIG_MAGIC || gBeaconConfig.version != BEACON_CONFIG_VERSION ||
	    BEACON_GetCode() >= BEACON_CODE_COUNT || BEACON_GetToneIndex() > BEACON_TONE_STEPS ||
	    gBeaconConfig.power_percent < 1u || gBeaconConfig.power_percent > 100u ||
	    gBeaconConfig.frequency_index > BEACON_FREQUENCY_STEPS ||
	    gBeaconConfig.checksum != BEACON_Checksum(&gBeaconConfig))
		BEACON_DefaultConfig();
}

static void BEACON_SaveConfig(void)
{
	gBeaconConfig.checksum = BEACON_Checksum(&gBeaconConfig);
	EEPROM_WriteBuffer(BEACON_EEPROM_ADDRESS, &gBeaconConfig);
}

static void BEACON_Configure(void)
{
	const uint32_t frequency = BEACON_GetFrequency();
	gEeprom.CROSS_BAND_RX_TX = CROSS_BAND_OFF;
	gEeprom.DUAL_WATCH = DUAL_WATCH_OFF;
	gEeprom.TX_VFO = 0;
	gEeprom.RX_VFO = 0;
	RADIO_SelectVfos();
	gCurrentVfo = gTxVfo;
	gTxVfo->FrequencyReverse = false;
	gTxVfo->Modulation = MODULATION_FM;
	gTxVfo->OUTPUT_POWER = OUTPUT_POWER_LOW;
	gTxVfo->CHANNEL_BANDWIDTH = BK4819_FILTER_BW_NARROW;
	gTxVfo->TX_OFFSET_FREQUENCY_DIRECTION = TX_OFFSET_FREQUENCY_DIRECTION_OFF;
	gTxVfo->TX_OFFSET_FREQUENCY = 0;
	gTxVfo->BUSY_CHANNEL_LOCK = false;
	gTxVfo->SCRAMBLING_TYPE = 0;
	gTxVfo->Compander = 0;
	gTxVfo->freq_config_RX.Frequency = frequency;
	gTxVfo->freq_config_TX.Frequency = frequency;
	gTxVfo->freq_config_RX.CodeType = CODE_TYPE_OFF;
	gTxVfo->freq_config_TX.CodeType = CODE_TYPE_OFF;
	gTxVfo->pRX = &gTxVfo->freq_config_RX;
	gTxVfo->pTX = &gTxVfo->freq_config_TX;
	RADIO_ConfigureSquelchAndOutputPower(gTxVfo);
	gTxVfo->TXP_CalculatedSetting =
		((uint16_t)gTxVfo->TXP_CalculatedSetting * gBeaconConfig.power_percent + 99u) / 100u;
	if (gTxVfo->TXP_CalculatedSetting == 0)
		gTxVfo->TXP_CalculatedSetting = 1;
	RADIO_SetupRegisters(true);
}

static void BEACON_RFOff(void)
{
	BK4819_EnterTxMute();
	BK4819_SetupPowerAmplifier(0, 0);
	BK4819_ToggleGpioOut(BK4819_GPIO1_PIN29_PA_ENABLE, false);
	BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1_RED, false);
	FUNCTION_Select(FUNCTION_FOREGROUND);
	RADIO_SetupRegisters(true);
}

static void BEACON_AddEvent(bool mark, uint8_t units)
{
	if (gBeaconEventCount < BEACON_MAX_EVENTS)
		gBeaconEvents[gBeaconEventCount++] = (mark ? BEACON_EVENT_MARK : 0u) | units;
}

static void BEACON_AddLetter(char letter)
{
	uint8_t count = 5, units = 1;
	if (letter == 'M') { count = 2; units = 3; }
	else if (letter == 'O') { count = 3; units = 3; }
	else if (letter == 'E') count = 1;
	else if (letter == 'I') count = 2;
	else if (letter == 'S') count = 3;
	else if (letter == 'H') count = 4;
	for (uint8_t i = 0; i < count; ++i) {
		BEACON_AddEvent(true, units);
		BEACON_AddEvent(false, i + 1u == count ? 3u : 1u);
	}
}

static void BEACON_BuildSequence(void)
{
	static const char suffix[BEACON_CODE_COUNT] = { 0, 'E', 'I', 'S', 'H', '5' };
	gBeaconEventCount = 0;
	for (uint8_t repeat = 0; repeat < 2; ++repeat) {
		BEACON_AddLetter('M');
		BEACON_AddLetter('O');
		if (suffix[BEACON_GetCode()] != 0)
			BEACON_AddLetter(suffix[BEACON_GetCode()]);
		/* Change the final character gap into the seven-unit word gap. */
		gBeaconEvents[gBeaconEventCount - 1u] = 7u;
	}
}

static void BEACON_StartRF(void)
{
	BEACON_Configure();
	FUNCTION_Select(FUNCTION_TRANSMIT);
	BK4819_TransmitTone(false, BEACON_GetTone());
	BK4819_EnterTxMute();
	gBeaconState = BEACON_WARMUP;
	gBeaconTicks = BEACON_WARMUP_TICKS;
	gUpdateDisplay = true;
}

static void BEACON_Stop(void)
{
	BEACON_RFOff();
	gBeaconState = BEACON_IDLE;
	gUpdateDisplay = true;
}

static void BEACON_NextEvent(void)
{
	if (gBeaconEventIndex >= gBeaconEventCount) {
		BEACON_RFOff();
		gBeaconState = BEACON_PAUSE;
		gBeaconTicks = BEACON_PAUSE_TICKS;
		gUpdateDisplay = true;
		return;
	}
	const uint8_t event = gBeaconEvents[gBeaconEventIndex++];
	if (event & BEACON_EVENT_MARK)
		BK4819_ExitTxMute();
	else
		BK4819_EnterTxMute();
	gBeaconTicks = (event & 0x7Fu) * BEACON_UNIT_TICKS;
}

void BEACON_Init(bool admin_requested)
{
	BEACON_LoadConfig();
	gBeaconAdmin = admin_requested;
	gBeaconState = BEACON_IDLE;
	BEACON_BuildSequence();
	if (gBeaconAdmin) {
		gMenuCursor = UI_MENU_GetMenuIdx(MENU_BCN_FR);
		gIsInSubMenu = false;
		gRequestDisplayScreen = DISPLAY_MENU;
	} else {
		BEACON_Configure();
		gRequestDisplayScreen = DISPLAY_MAIN;
	}
	gUpdateDisplay = true;
}

void BEACON_TimeSlice10ms(void)
{
	if (gBeaconAdmin || gBeaconState == BEACON_IDLE)
		return;
	if (gBatteryDisplayLevel == 0) {
		BEACON_Stop();
		return;
	}
	if (gBeaconTicks > 0 && --gBeaconTicks > 0)
		return;
	if (gBeaconState == BEACON_WARMUP) {
		gBeaconEventIndex = 0;
		gBeaconState = BEACON_SEQUENCE;
		BEACON_NextEvent();
	} else if (gBeaconState == BEACON_SEQUENCE) {
		BEACON_NextEvent();
	} else if (gBeaconState == BEACON_PAUSE) {
		BEACON_StartRF();
	}
}

bool BEACON_ProcessKey(KEY_Code_t key, bool pressed, bool held)
{
	if (gBeaconAdmin)
		return false;
	if (!pressed || held)
		return true;
	if (key == KEY_PTT) {
		if (gBeaconState == BEACON_IDLE)
			BEACON_StartRF();
		else
			BEACON_Stop();
	} else if (key == KEY_EXIT && gBeaconState != BEACON_IDLE) {
		BEACON_Stop();
	}
	return true;
}

bool BEACON_IsAdmin(void) { return gBeaconAdmin; }

void BEACON_Display(void)
{
	char frequency[18];
	const uint32_t value = BEACON_GetFrequency();
	sprintf(frequency, "%u.%03u MHz", value / 100000u, (value % 100000u) / 100u);
	UI_DisplayClear();
	UI_PrintString(gBeaconCodeNames[BEACON_GetCode()], 0, 127, 0, 10);
	UI_PrintString(frequency, 0, 127, 2, 8);
	if (gBeaconState == BEACON_IDLE) {
		UI_PrintString("PTT = START", 0, 127, 4, 8);
		UI_PrintString("LOW POWER", 0, 127, 6, 8);
	} else if (gBeaconState == BEACON_PAUSE) {
		UI_PrintString("TX PAUSE 5 SEC", 0, 127, 4, 8);
		UI_PrintString("PTT/EXIT STOP", 0, 127, 6, 8);
	} else {
		UI_PrintString(gBeaconState == BEACON_WARMUP ? "TX WARMUP" : "SENDING", 0, 127, 4, 8);
		UI_PrintString("PTT/EXIT STOP", 0, 127, 6, 8);
	}
}

void BEACON_GetMenuLimits(uint8_t menu_id, int32_t *minimum, int32_t *maximum)
{
	*minimum = menu_id == MENU_BCN_PW ? 1 : 0;
	if (menu_id == MENU_BCN_FR) {
		*minimum = BEACON_MIN_FREQUENCY_10HZ / 100u;
		*maximum = BEACON_MAX_FREQUENCY_10HZ / 100u;
	}
	else if (menu_id == MENU_BCN_PW) *maximum = 100;
	else if (menu_id == MENU_BCN_ID) *maximum = BEACON_CODE_COUNT - 1u;
	else *maximum = BEACON_TONE_STEPS;
}

int32_t BEACON_GetMenuValue(uint8_t menu_id)
{
	if (menu_id == MENU_BCN_FR) return BEACON_GetFrequency() / 100u;
	if (menu_id == MENU_BCN_PW) return gBeaconConfig.power_percent;
	if (menu_id == MENU_BCN_ID) return BEACON_GetCode();
	return BEACON_GetToneIndex();
}

void BEACON_SetMenuValue(uint8_t menu_id, int32_t value)
{
	int32_t minimum, maximum;
	BEACON_GetMenuLimits(menu_id, &minimum, &maximum);
	if (value < minimum || value > maximum)
		return;
	if (menu_id == MENU_BCN_FR) gBeaconConfig.frequency_index = value - BEACON_MIN_FREQUENCY_10HZ / 100u;
	else if (menu_id == MENU_BCN_PW) gBeaconConfig.power_percent = value;
	else if (menu_id == MENU_BCN_ID) gBeaconConfig.code_tone = (gBeaconConfig.code_tone & 0xF8u) | (value & 7u);
	else gBeaconConfig.code_tone = (gBeaconConfig.code_tone & 7u) | ((value & 0x1Fu) << 3);
	BEACON_BuildSequence();
	BEACON_SaveConfig();
}

void BEACON_FormatMenuValue(uint8_t menu_id, int32_t selection, char *text)
{
	if (menu_id == MENU_BCN_FR) {
		const uint32_t value = (uint32_t)selection * 100u;
		sprintf(text, "%u.%03u\nMHz", value / 100000u, (value % 100000u) / 100u);
	} else if (menu_id == MENU_BCN_PW) {
		sprintf(text, "%u%%\nof LOW", (unsigned)selection);
	} else if (menu_id == MENU_BCN_ID) {
		strcpy(text, gBeaconCodeNames[selection]);
	} else {
		sprintf(text, "%u Hz", BEACON_TONE_MIN_HZ + (unsigned)selection * BEACON_TONE_STEP_HZ);
	}
}
