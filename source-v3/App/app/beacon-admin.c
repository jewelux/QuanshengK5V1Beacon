/* LX1WJ beacon admin UI for the reald UV-K5 V3 port. Apache-2.0. */
#include <string.h>
#include "app/beacon.h"
#include "external/printf/printf.h"
#include "misc.h"
#include "ui/helper.h"
#include "ui/ui.h"

static uint8_t cursor, digitCount;
static bool editing;
static const char *inputError;
static int32_t selection;
static char digits[7];
static const char *const names[] = {"Freq", "Power", "ID", "Tone"};

void BEACON_V3AdminInit(void)
{
    cursor = digitCount = 0;
    editing = false;
    inputError = NULL;
}

bool BEACON_V3AdminKey(KEY_Code_t key, bool pressed, bool held)
{
    if (!pressed || (held && key != KEY_UP && key != KEY_DOWN))
        return true;
    inputError = NULL;
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
            {
                inputError = cursor == MENU_BCN_FR && digitCount != 6 ? "Need 6 digits" : "Out of range";
                gUpdateDisplay = true;
                return true;
            }
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
    char value[24], row[32];
    UI_DisplayClear();
    if (inputError)
        strcpy(row, inputError);
    else if (editing)
        sprintf(row, "EDIT: %s", names[cursor]);
    else
        strcpy(row, "BEACON SETTINGS");
    UI_PrintStringSmallNormal(row, 0, 127, 0);

    /* All four items remain visible. Each small-font row occupies one page. */
    for (unsigned item = 0; item < 4; ++item) {
        const int32_t current = editing && item == cursor ? selection : BEACON_GetMenuValue(item);
        if (editing && item == cursor && digitCount && item == MENU_BCN_FR) {
            memcpy(value, digits, digitCount);
            unsigned length = digitCount;
            while (length < 6) value[length++] = '_';
            strcpy(value + length, " kHz");
        } else if (item == MENU_BCN_PW) {
            sprintf(value, "%u%% LOW", (unsigned)current);
        } else {
            BEACON_FormatMenuValue(item, current, value);
            for (char *p = value; *p; ++p)
                if (*p == '\n') *p = ' ';
        }
        sprintf(row, "%c%s %s", item == cursor ? '>' : ' ', names[item], value);
        UI_PrintStringSmallNormal(row, 2, 0, item + 1);
    }
    UI_PrintStringSmallNormal(editing ? "MENU: save" : "MENU: edit", 0, 127, 5);
    UI_PrintStringSmallNormal(editing ? "EXIT: cancel" : "UP/DN: select", 0, 127, 6);
}
