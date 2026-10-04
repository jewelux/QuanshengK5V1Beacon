/* LX1WJ beacon admin UI for the reald UV-K5 V3 port. Apache-2.0. */
#include <string.h>
#include "app/beacon.h"
#include "external/printf/printf.h"
#include "misc.h"
#include "ui/helper.h"
#include "ui/ui.h"

static uint8_t cursor, digitCount;
static bool editing;
static int32_t selection;
static char digits[7];
static const char *const names[] = {"BcnFrq", "BcnPwr", "BcnID", "BcnTon"};

void BEACON_V3AdminInit(void)
{
    cursor = digitCount = 0;
    editing = false;
}

bool BEACON_V3AdminKey(KEY_Code_t key, bool pressed, bool held)
{
    if (!pressed || (held && key != KEY_UP && key != KEY_DOWN))
        return true;
    if (key == KEY_MENU) {
        if (!editing) {
            selection = BEACON_GetMenuValue(cursor);
            digitCount = 0;
            editing = true;
        } else {
            int32_t minimum, maximum;
            BEACON_GetMenuLimits(cursor, &minimum, &maximum);
            if (digitCount && ((cursor == MENU_BCN_FR && digitCount != 6) ||
                               selection < minimum || selection > maximum))
                return true;
            BEACON_SetMenuValue(cursor, selection);
            editing = false;
            digitCount = 0;
        }
    } else if (key == KEY_EXIT) {
        editing = false; /* Discard staged input; stay in admin mode until restart. */
        digitCount = 0;
    } else if (key == KEY_UP || key == KEY_DOWN) {
        const int direction = key == KEY_UP ? 1 : -1;
        if (editing) {
            int32_t minimum, maximum;
            BEACON_GetMenuLimits(cursor, &minimum, &maximum);
            /* Invalid typed values do not leak into the UP/DOWN adjustment. */
            if (digitCount)
                selection = BEACON_GetMenuValue(cursor);
            digitCount = 0;
            selection += direction;
            if (selection > maximum) selection = minimum;
            if (selection < minimum) selection = maximum;
        } else {
            cursor = (cursor + (key == KEY_DOWN ? 1 : 3)) % 4;
        }
    } else if (editing && key <= KEY_9 &&
               (cursor == MENU_BCN_FR || cursor == MENU_BCN_PW)) {
        const unsigned limit = cursor == MENU_BCN_FR ? 6 : 3;
        if (digitCount < limit) {
            if (!digitCount) selection = 0;
            digits[digitCount++] = '0' + key;
            selection = selection * 10 + key;
        }
    }
    gUpdateDisplay = true;
    return true; /* Consume PTT and all stock keys in admin mode. */
}

void BEACON_V3AdminDisplay(void)
{
    char value[24];
    UI_DisplayClear();
    UI_PrintString("BEACON ADMIN", 0, 127, 0, 8);
    UI_PrintString(names[cursor], 0, 127, 2, 8);
    if (editing && digitCount) {
        memcpy(value, digits, digitCount);
        unsigned length = digitCount;
        if (cursor == MENU_BCN_FR)
            while (length < 6) value[length++] = '_';
        value[length] = 0;
    } else {
        BEACON_FormatMenuValue(cursor, editing ? selection : BEACON_GetMenuValue(cursor), value);
        /* The simple screen uses one value line rather than stock menu layout. */
        for (char *p = value; *p; ++p)
            if (*p == '\n') *p = ' ';
    }
    UI_PrintString(value, 0, 127, 4, 8);
    UI_PrintString(editing ? "MENU=SAVE EXIT=X" : "MENU EDIT UP/DN", 0, 127, 6, 8);
}
