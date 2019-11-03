#include "app.h"
#include "view_setup_cc.h"
#include "font.h"
#include "stdbool.h"
#include <string.h>

button_setup board_buttons_setup[BUTTON_COUNT];
u8 board_buttons_setup_size = sizeof(board_buttons_setup) / sizeof(button_setup);

//  Some setup stuff for Setup view
u8 ccsetup_tens_indices[13] = {
    11, 12, 13, 14, 15, 16, 17, 18, 21, 22, 23, 24, 25
    //  00  10  20  30  40  50  60  70  80  90  100 110 120
};
u8 ccsetup_units_indices[10] = {
    31, 32, 33, 34, 35, 36, 37, 38, 41, 42
    //  0   1   2   3   4   5   6   7   8   9
};
button_setup *ccbutton_selected;
u8 ccsetup_activetens = 0;
u8 ccsetup_activeunits = 0;
u8 ccsetup_newvalue = 0;

u8 ccbutton_indices[8] = {19, 29, 39, 49, 59, 69, 79, 89};
u8 ccbutton_indices_length = sizeof(ccbutton_indices) / sizeof(u8);

//      --      --      --      --      --      --

void setup_cc_init()
{
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        board_buttons_setup[i].index = i;
        board_buttons_setup[i].value = 0;
        board_buttons_setup[i].active = 0;
        board_buttons_setup[i].colour[0] = 0x00;
        board_buttons_setup[i].colour[1] = 0x00;
        board_buttons_setup[i].colour[2] = 0x10;
        board_buttons_setup[i].altcolour[0] = 0x00;
        board_buttons_setup[i].altcolour[1] = 0x10;
        board_buttons_setup[i].altcolour[2] = 0x00;
    }

    //  Set some pretty colours for setup mode
    for (u8 i = 0; i < BUTTON_COUNT; i++)
    {
        for (u8 j = 0; j < sizeof(ccsetup_tens_indices) / sizeof(u8); j++)
        {
            if (board_buttons_setup[i].index == ccsetup_tens_indices[j])
            {
                board_buttons_setup[i].colour[0] = 0x30;
                board_buttons_setup[i].colour[1] = 0xa0;
                board_buttons_setup[i].colour[2] = 0x50;
                board_buttons_setup[i].altcolour[0] = 0xff;
                board_buttons_setup[i].altcolour[1] = 0x05;
                board_buttons_setup[i].altcolour[2] = 0x00;
                board_buttons_setup[i].value = j;
                board_buttons_setup[i].cancelgroup = CANCEL_TENS;
                board_buttons_setup[i].active = 1;
            }
            if (board_buttons_setup[i].value == 0 && board_buttons_setup[i].cancelgroup == CANCEL_TENS) // Default CC 00
            {
                board_buttons_setup[i].active = 2;
            }
        }
        for (u8 j = 0; j < sizeof(ccsetup_units_indices) / sizeof(u8); j++)
        {
            if (board_buttons_setup[i].index == ccsetup_units_indices[j])
            {
                board_buttons_setup[i].colour[0] = 0x10;
                board_buttons_setup[i].colour[1] = 0x50;
                board_buttons_setup[i].colour[2] = 0xa0;
                board_buttons_setup[i].altcolour[0] = 0xff;
                board_buttons_setup[i].altcolour[1] = 0x00;
                board_buttons_setup[i].altcolour[2] = 0x05;
                board_buttons_setup[i].value = j;
                board_buttons_setup[i].cancelgroup = CANCEL_UNITS;
                board_buttons_setup[i].active = 1;
            }
            if (board_buttons_setup[i].value == 0 && board_buttons_setup[i].cancelgroup == CANCEL_UNITS) // Default CC 00
            {
                board_buttons_setup[i].active = 2;
            }
        }
        for (u8 j = 0; j < sizeof(ccbutton_indices) / sizeof(u8); j++)
        {
            if (board_buttons_setup[i].index == ccbutton_indices[j])
            {
                board_buttons_setup[i].value = 89;
                board_buttons_setup[i].active = 0;
                board_buttons_setup[i].flashing = 0;
                board_buttons_setup[i].colour[0] = 0x10;
                board_buttons_setup[i].colour[1] = 0x00;
                board_buttons_setup[i].colour[2] = 0x00;
                board_buttons_setup[i].altcolour[0] = 0xff;
                board_buttons_setup[i].altcolour[1] = 0x00;
                board_buttons_setup[i].altcolour[2] = 0x00;
                board_buttons_setup[i].cancelgroup = CANCEL_CCS;
            }
        }
    }
}

void setup_cc_redrawBoard()
/*
Draw the contents of board_buttons_setup[] to the surface
*/
{
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        if (board_buttons_setup[i].active == 1)
        {
            hal_plot_led(TYPEPAD, board_buttons_setup[i].index, board_buttons_setup[i].colour[0], board_buttons_setup[i].colour[1], board_buttons_setup[i].colour[2]);
        }
        else if (board_buttons_setup[i].active == 2)
        {
            hal_plot_led(TYPEPAD, board_buttons_setup[i].index, board_buttons_setup[i].altcolour[0], board_buttons_setup[i].altcolour[1], board_buttons_setup[i].altcolour[2]);
        }
        else
        {
            hal_plot_led(TYPEPAD, board_buttons_setup[i].index, 0x00, 0x00, 0x00);
        }
    }
}


void drawFont(u8 anchor, bool symbol[4][3], u8 r, u8 g, u8 b)
{
    for (u8 i = 0; i < BUTTON_COUNT; i++)
    {
        for (u8 j = 0; j < 4; j++)
        {
            for (u8 k = 0; k < 3; k++)
            {
                if (symbol[j][k] == 1 && anchor - (10 * j) + k == board_buttons_setup[i].index)
                {
                    board_buttons_setup[i].colour[0] = r;
                    board_buttons_setup[i].colour[1] = g;
                    board_buttons_setup[i].colour[2] = b;
                    board_buttons_setup[i].active = 1;
                }
            }
        }
    }
}


bool *symbols[] = { // this does not mean 'a pointer to bool[4][3]', it means 'a 4 by 3 array of pointers to bools'
    &FONT_34_0,
    &FONT_34_1,
    &FONT_34_2,
    &FONT_34_3,
    &FONT_34_4,
    &FONT_34_5,
    &FONT_34_6,
    &FONT_34_7,
    &FONT_34_8,
    &FONT_34_9};

void setup_cc_drawNumberFont(u8 h, u8 t, u8 u)
{
    //  Clear
    bool* nptr = &FONT_34_ALL;
    drawFont(86, nptr, 0x00, 0x00, 0x00);
    drawFont(83, nptr, 0x00, 0x00, 0x00);
    drawFont(80, nptr, 0x00, 0x00, 0x00);

    setup_cc_redrawBoard();

    //  Overwrite
    bool* uptr = symbols[u];
    drawFont(86, uptr, 0x20, 0x20, 0xff);

    bool* tptr = symbols[t];
    drawFont(83, tptr, 0xff, 0x20, 0x20);

    if (h != 0)
    {
        bool* hptr = symbols[h];
        drawFont(80, hptr, 0xff, 0x20, 0x20);
    }
}

void setup_cc_setActiveNumberDisplay(button_setup *cc)
/*
Push the value of the selected CC button to the tens and units display
*/
{
    u8 ccnum = cc->value;
    u8 ccu = ccnum % 10;
    u8 cct = (ccnum / 10) % 10;
    u8 cch = (ccnum / 100) % 10;
    u8 cct2 = cct + (10 * cch); // 'cause I can have two-digit tens in my system. Take that, maths.

    for (u8 i = 0; i < BUTTON_COUNT; i++)
    {
        for (u8 j = 0; j < sizeof(ccsetup_tens_indices) / sizeof(u8); j++)
        {
            if (board_buttons_setup[i].index == ccsetup_tens_indices[j] && board_buttons_setup[i].value == cct2)
            {
                intercancel(board_buttons_setup, board_buttons_setup_size, CANCEL_TENS, 1);
                board_buttons_setup[i].active = 2;
            }
        }
        for (u8 j = 0; j < sizeof(ccsetup_units_indices) / sizeof(u8); j++)
        {
            if (board_buttons_setup[i].index == ccsetup_units_indices[j] && board_buttons_setup[i].value == ccu)
            {
                intercancel(board_buttons_setup, board_buttons_setup_size, CANCEL_UNITS, 1);
                board_buttons_setup[i].active = 2;
            }
        }
    }

    setup_cc_drawNumberFont(cch, cct, ccu);
}

void setup_cc_applyNewValue(button_setup *target)
/*
Get the tens and units, munge them together, push it to the cc selected
*/
{
    u8 targetTen = 0;
    u8 targetUnit = 0;

    for (u8 i = 0; i < BUTTON_COUNT; i++)
    {
        //  Get tens
        for (u8 j = 0; j < sizeof(ccsetup_tens_indices) / sizeof(u8); j++)
        {
            if (board_buttons_setup[i].index == ccsetup_tens_indices[j] && board_buttons_setup[i].active == 2)
            {
                targetTen = board_buttons_setup[i].value;
            }
        }
        //  Get units
        for (u8 j = 0; j < sizeof(ccsetup_units_indices) / sizeof(u8); j++)
        {
            if (board_buttons_setup[i].index == ccsetup_units_indices[j] && board_buttons_setup[i].active == 2)
            {
                targetUnit = board_buttons_setup[i].value;
            }
        }
    }

    hal_plot_led(TYPESETUP, 0, 0xff, 0x00, 0x00);

    target->value = targetTen*10+targetUnit;
    hal_send_midi(USBMIDI, NOTEON, target->value, ccbutton_selected->value);
    setup_cc_setActiveNumberDisplay(target);
}

void setup_cc_surfaceEvent(u8 type, u8 index, u8 value)
/*
Equivalent of app_surface_event - specific to view_setup_cc
*/
{
    switch (type)
    {
    case TYPEPAD:
    {
        if (value)
        {
            // hal_send_midi(USBMIDI, NOTEON, index, value);
            for (int i = 0; i < BUTTON_COUNT; i++)
            {
                // CC Buttons
                for (int j = 0; j < sizeof(ccbutton_indices) / sizeof(u8); j++)
                {
                    if (board_buttons_setup[i].index == index && board_buttons_setup[i].index == ccbutton_indices[j])
                    {
                        if (board_buttons_setup[i].active == 0)
                        {
                            intercancel(board_buttons_setup, board_buttons_setup_size, CANCEL_CCS, 0);
                            board_buttons_setup[i].active = 1;
                            setup_cc_setActiveNumberDisplay(&board_buttons_setup[i]);
                            ccbutton_selected = &board_buttons_setup[i];
                        }
                        else
                        {
                            board_buttons_setup[i].active = 0;
                        }
                    }
                }
                //  TENS
                for (int j = 0; j < sizeof(ccsetup_tens_indices) / sizeof(u8); j++)
                {
                    if (board_buttons_setup[i].index == index && board_buttons_setup[i].index == ccsetup_tens_indices[j])
                    {
                        hal_send_midi(USBMIDI, NOTEON, board_buttons_setup[i].value, board_buttons_setup[i].active);
                        if (board_buttons_setup[i].active == 1)
                        {
                            intercancel(board_buttons_setup, board_buttons_setup_size, CANCEL_TENS, 1);
                            board_buttons_setup[i].active = 2;
                            setup_cc_applyNewValue(ccbutton_selected);
                        }
                        else if (board_buttons_setup[i].active == 2)
                        {
                            //  Do nothing? Want to prevent user from deselecting a ten or unit
                            intercancel(board_buttons_setup, board_buttons_setup_size, CANCEL_TENS, 1);
                            board_buttons_setup[i].active = 2;
                            setup_cc_applyNewValue(ccbutton_selected);
                        }
                        else
                        {
                            // board_buttons_setup[i].active = 1;
                        }
                    }
                }
                //  UNITS
                for (int j = 0; j < sizeof(ccsetup_units_indices) / sizeof(u8); j++)
                {
                    if (board_buttons_setup[i].index == index && board_buttons_setup[i].index == ccsetup_units_indices[j])
                    {
                        hal_send_midi(USBMIDI, NOTEON, board_buttons_setup[i].value, board_buttons_setup[i].active);
                        if (board_buttons_setup[i].active == 1)
                        {
                            intercancel(board_buttons_setup, board_buttons_setup_size, CANCEL_UNITS, 1);
                            board_buttons_setup[i].active = 2;
                            setup_cc_applyNewValue(ccbutton_selected);
                        }
                        else if (board_buttons_setup[i].active == 2)
                        {
                            //  Do nothing? Want to prevent user from deselecting a ten or unit
                            intercancel(board_buttons_setup, board_buttons_setup_size, CANCEL_UNITS, 1);
                            board_buttons_setup[i].active = 2;
                            setup_cc_applyNewValue(ccbutton_selected);
                        }
                        else
                        {
                            // board_buttons_setup[i].active = 1;
                        }
                    }
                }
            }
        }
        else
        {
        }
    }
    break;
    }
}