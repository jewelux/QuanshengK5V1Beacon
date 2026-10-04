/* V1/V3 host simulation: checks timing and RF control, not physical RF output. */
#include <assert.h>
#include "app/beacon.c"
#include "frequencies.h"

EEPROM_Config_t gEeprom;
static VFO_Info_t vfo;
VFO_Info_t *gTxVfo = &vfo, *gRxVfo = &vfo, *gCurrentVfo = &vfo;
uint8_t gBatteryDisplayLevel = 6;
bool gUpdateDisplay;
static bool rfOn, toneOn;
static unsigned starts, writes;
static BEACON_Config_t saved;
const freq_band_table_t frequencyBandTable[] = {{40000000, 47000000}};

#ifdef BEACON_PLATFORM_V3
void PY25Q16_ReadBuffer(uint32_t address, void *buffer, uint32_t size)
{
    if (address == BEACON_EEPROM_ADDRESS) memcpy(buffer, &saved, size);
    else { assert(address == 0x100D0 && size == 3); memset(buffer, 100, size); }
}
void PY25Q16_WriteBuffer(uint32_t address, const void *buffer, uint32_t size, bool append)
{
    assert(address == BEACON_EEPROM_ADDRESS && size == sizeof(saved) && !append);
    memcpy(&saved, buffer, size); ++writes;
}
#else
void EEPROM_ReadBuffer(uint16_t address, void *buffer, uint8_t size)
{ assert(address == BEACON_EEPROM_ADDRESS); memcpy(buffer, &saved, size); }
void EEPROM_WriteBuffer(uint16_t address, const void *buffer)
{ assert(address == BEACON_EEPROM_ADDRESS); memcpy(&saved, buffer, sizeof(saved)); ++writes; }
#endif
FREQUENCY_Band_t FREQUENCY_GetBand(uint32_t f) { assert(f >= 43001300 && f <= 43998700); return 0; }
uint8_t FREQUENCY_CalculateOutputPower(uint8_t a, uint8_t b, uint8_t c, int32_t l, int32_t m, int32_t u, int32_t f)
{ (void)b; (void)c; (void)l; (void)m; (void)u; (void)f; return a; }
void RADIO_SelectVfos(void) {}
void RADIO_ConfigureSquelchAndOutputPower(VFO_Info_t *p)
{
#ifdef BEACON_PLATFORM_V3
    p->TXP_CalculatedSetting = 4; // Extra Low1 division must be overridden.
#else
    p->TXP_CalculatedSetting = 100;
#endif
}
void RADIO_SetupRegisters(bool f) { (void)f; }
void FUNCTION_Select(FUNCTION_Type_t f) { if (f == FUNCTION_TRANSMIT) { rfOn = true; ++starts; } }
void BK4819_EnterTxMute(void) { toneOn = false; }
void BK4819_ExitTxMute(void) { toneOn = true; }
void BK4819_SetupPowerAmplifier(uint8_t bias, uint32_t f) { (void)f; if (!bias) rfOn = false; }
void BK4819_ToggleGpioOut(BK4819_GPIO_PIN_t pin, bool on) { (void)pin; (void)on; }
void BK4819_TransmitTone(bool local, uint32_t f) { assert(!local && f == 1000); }
bool BEACON_V3AdminKey(KEY_Code_t key, bool pressed, bool held)
{ (void)key; (void)pressed; (void)held; return true; }

int main(void)
{
    BEACON_DefaultConfig();
    gBeaconState = BEACON_IDLE;
    gBeaconAdmin = false;
    BEACON_BuildSequence();
    BEACON_ProcessKey(KEY_PTT, true, false);
    assert(rfOn && !toneOn && starts == 1 && gBeaconState == BEACON_WARMUP);
    assert(vfo.TXP_CalculatedSetting == 1); // 1% of original LOW=100, not divided Low1
    for (unsigned i = 0; i < 74; ++i) BEACON_TimeSlice10ms();
    assert(!toneOn && gBeaconState == BEACON_WARMUP);
    BEACON_TimeSlice10ms(); assert(toneOn && gBeaconState == BEACON_SEQUENCE);
    unsigned elapsed = 0;
    while (gBeaconState != BEACON_PAUSE && ++elapsed < 5000) BEACON_TimeSlice10ms();
    assert(gBeaconState == BEACON_PAUSE && !rfOn && !toneOn);
    for (unsigned i = 0; i < 499; ++i) BEACON_TimeSlice10ms();
    assert(!rfOn);
    BEACON_TimeSlice10ms(); assert(rfOn && starts == 2);
    BEACON_ProcessKey(KEY_PTT, false, false); assert(rfOn); // release does not stop
    BEACON_ProcessKey(KEY_PTT, true, true); assert(rfOn); // held event does not toggle
    BEACON_ProcessKey(KEY_EXIT, true, false); assert(!rfOn && gBeaconState == BEACON_IDLE);
    BEACON_SetMenuValue(MENU_BCN_PW, 17);
    BEACON_ProcessKey(KEY_PTT, true, false); assert(vfo.TXP_CalculatedSetting == 17);
    gBatteryDisplayLevel = 0; BEACON_TimeSlice10ms(); assert(!rfOn && gBeaconState == BEACON_IDLE);
    BEACON_ProcessKey(KEY_PTT, true, false); assert(!rfOn);
    gBatteryDisplayLevel = 7; BEACON_ProcessKey(KEY_PTT, true, false); assert(!rfOn);
    gBatteryDisplayLevel = 6;
    gBeaconAdmin = true; BEACON_ProcessKey(KEY_PTT, true, false); assert(!rfOn);
    gBeaconAdmin = false; BEACON_ProcessKey(KEY_PTT, true, false); assert(rfOn);
    BEACON_ProcessKey(KEY_PTT, true, false); assert(!rfOn);
    BEACON_ProcessKey(KEY_PTT, true, false);
    for (unsigned cycle = 0; cycle < 10; ++cycle) {
        for (unsigned i = 0; i < BEACON_WARMUP_TICKS; ++i) BEACON_TimeSlice10ms();
        assert(rfOn && toneOn);
        while (gBeaconState == BEACON_SEQUENCE) {
            if (toneOn) assert(rfOn);
            BEACON_TimeSlice10ms();
        }
        assert(gBeaconState == BEACON_PAUSE && !rfOn && !toneOn);
        for (unsigned i = 0; i < BEACON_PAUSE_TICKS; ++i) BEACON_TimeSlice10ms();
        assert(rfOn && !toneOn && gBeaconState == BEACON_WARMUP);
    }
    BEACON_ProcessKey(KEY_EXIT, true, false);
    assert(!rfOn && gBeaconState == BEACON_IDLE);
    assert(writes == 1); // running never writes config
    return 0;
}
