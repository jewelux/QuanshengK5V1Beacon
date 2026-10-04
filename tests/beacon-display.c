/* Real upstream fonts/renderers under ASan: catch page/column overflows. */
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "app/beacon.c"
#include "driver/st7565.h"
#ifdef BEACON_PLATFORM_V3
#include "app/beacon-admin.c"
#endif

uint8_t gFrameBuffer[FRAME_LINES][LCD_WIDTH];
uint8_t gStatusLine[LCD_WIDTH];
bool gUpdateDisplay;

static void capture(const char *name)
{
    if (!name) return;
    int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    assert(fd >= 0);
    const char header[] = "P4\n128 64\n";
    assert(write(fd, header, sizeof(header)-1) == sizeof(header)-1);
    for (unsigned y = 0; y < 64; ++y) {
        uint8_t row[16] = {0};
        if (y >= 8) for (unsigned x = 0; x < 128; ++x)
            if (gFrameBuffer[(y-8)/8][x] & (1u << ((y-8)%8)))
                row[x/8] |= 0x80u >> (x%8);
        assert(write(fd, row, sizeof(row)) == sizeof(row));
    }
    close(fd);
}

int main(int argc, char **argv)
{
    BEACON_DefaultConfig();
    for (unsigned id = 0; id < 6; ++id) {
    gBeaconConfig.code_tone = (gBeaconConfig.code_tone & ~7u) | id;
    for (unsigned state = BEACON_IDLE; state <= BEACON_PAUSE; ++state) {
        gBeaconState = state;
        BEACON_Display();
    }
    }
#ifdef BEACON_PLATFORM_V3
    gBeaconAdmin = true;
    BEACON_V3AdminInit();
    gBeaconConfig.power_percent = 100;
    gBeaconConfig.frequency_index = BEACON_FREQUENCY_STEPS;
    gBeaconConfig.code_tone = (BEACON_TONE_STEPS << 3) | 5;
    BEACON_Display();
    BEACON_DefaultConfig();
    for (cursor = 0; cursor < 4; ++cursor) {
        editing = false; digitCount = 0; BEACON_Display();
        if (cursor == 0) capture(argc > 1 ? argv[1] : NULL);
        editing = true; selection = BEACON_GetMenuValue(cursor); BEACON_Display();
        if (cursor == 0) capture(argc > 2 ? argv[2] : NULL);
        if (cursor <= MENU_BCN_PW) {
            strcpy(digits, "100"); digitCount = 3; selection = 100; BEACON_Display();
        }
    }
#else
    gBeaconState = BEACON_IDLE; BEACON_Display();
    capture(argc > 1 ? argv[1] : NULL);
#endif
    return 0;
}
