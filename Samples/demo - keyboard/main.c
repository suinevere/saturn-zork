/*
** Jo Sega Saturn Engine
** Copyright (c) 2012-2024, Johannes Fetz (johannesfetz@gmail.com)
** All rights reserved.
*/

#include <jo/jo.h>

static char typed_text[32] = {0};
static int typed_len = 0;
static bool was_key_pressed = false;

void my_draw(void) {
    jo_printf(10, 1, "* Keyboard Demo *");
    jo_printf(2, 4, "Type something:");

    unsigned char key = jo_keyboard_get_char();
    jo_keyboard_special_key special = jo_keyboard_get_special_key();

    // Debug info for the peripheral
    PerKeyBoard *kbd = (PerKeyBoard *) &Smpc_Peripheral[0];
    jo_printf(2, 17, "RAW SMPC[0] id: 0x%02x ext: 0x%02x ", kbd->id, kbd->ext);
    jo_printf(2, 18, "RAW SMPC[0] code: 0x%02x cond: 0x%02x ", kbd->code, kbd->cond);
    jo_printf(2, 19, "Input type: %d               ", jo_get_input_type(0));

    // Simple key debounce
    if (key != 0 || special != JO_KEYBOARD_NO_SPECIAL_KEY) {
        if (!was_key_pressed) {
            was_key_pressed = true;

            if (key != 0 && typed_len < 30) {
                typed_text[typed_len] = key;
                typed_len++;
                typed_text[typed_len] = '\0';
            } else if (special == JO_KEYBOARD_BACKSPACE && typed_len > 0) {
                typed_len--;
                typed_text[typed_len] = '\0';
            } else if (special == JO_KEYBOARD_ENTER) {
                typed_len = 0;
                typed_text[typed_len] = '\0';
            }
        }
    } else {
        was_key_pressed = false;
    }

    jo_printf(2, 6, "> %s_                        ", typed_text);

    if (key != 0) {
        jo_printf(2, 9, "Last char: %c (ASCII: %d)       ", key, key);
    } else {
        jo_printf(2, 9, "Last char: none                  ");
    }

    if (special != JO_KEYBOARD_NO_SPECIAL_KEY) {
        jo_printf(2, 11, "Special key: %d                  ", special);
    } else {
        jo_printf(2, 11, "Special key: none                ");
    }

    jo_printf(2, 14, "Press BACKSPACE to delete");
    jo_printf(2, 15, "Press ENTER to clear");
}

void jo_main(void) {
    jo_core_init(JO_COLOR_Black);

    jo_core_add_callback(my_draw);
    jo_core_run();
}

/*
** END OF FILE
*/
