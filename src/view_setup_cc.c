#include "app.h"
#include "view_setup_cc.h"

button_setup board_buttons_setup[BUTTON_COUNT];
u8 board_buttons_setup_size     = sizeof(board_buttons_setup)   / sizeof(button_setup);


//  Some setup stuff for Setup view
u8 ccsetup_tens_indices[13] = {
    11, 12, 13, 14, 15, 16, 17, 18, 21, 22, 23, 24, 25
//  00  10  20  30  40  50  60  70  80  90  100 110 120
};
u8 ccsetup_units_indices[10] = {
    31, 32, 33, 34, 35, 36, 37, 38, 41, 42
//  0   1   2   3   4   5   6   7   8   9
};
u8 ccsetup_selected     = 0;    // Store the board_buttons_setup index of the selected CC button
u8 ccsetup_activetens = 0;
u8 ccsetup_activeunits = 0;

u8 ccbutton_indices[8] = {  19, 29, 39, 49, 59, 69, 79, 89  };
u8 ccbutton_indices_length = sizeof(ccbutton_indices) / sizeof(u8);


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



void setup_cc_setActiveNumberDisplay(button_setup cc)
/*
Push the value of the selected CC button to the tens and units display
*/
{
    u8 ccnum = cc.value;
    u8 ccu  = ccnum %10;
    u8 cct  = (ccnum/10)%10;
    u8 cch  = (ccnum/100)%10;
    cct     = cct+(10*cch);     // 'cause I can have two-digit tens in my system. Take that, maths.

    for (u8 i = 0; i < BUTTON_COUNT; i++)
    {
        for (u8 j = 0; j < sizeof(ccsetup_tens_indices) / sizeof(u8); j++)
        {
            if (board_buttons_setup[i].index == ccsetup_tens_indices[j] && board_buttons_setup[i].value == cct)
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
                        if  (board_buttons_setup[i].index == index && board_buttons_setup[i].index == ccbutton_indices[j])
                        {
                            if (board_buttons_setup[i].active == 0)
                            {
                                intercancel(board_buttons_setup, board_buttons_setup_size, CANCEL_CCS, 0);
                                board_buttons_setup[i].active = 1;
                                setup_cc_setActiveNumberDisplay(board_buttons_setup[i]);
                            } else {
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
                            }
                            else if (board_buttons_setup[i].active == 2)
                            {
                                //  Do nothing? Want to prevent user from deselecting a ten or unit
                            } else {
                                board_buttons_setup[i].active = 1;
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
                            }
                            else if (board_buttons_setup[i].active == 2)
                            {
                                //  Do nothing? Want to prevent user from deselecting a ten or unit
                            } else {
                                board_buttons_setup[i].active = 1;
                            }
                        }
                    }

                }

            } else {
                // hal_send_midi(USBMIDI, NOTEOFF, index, value);
            }
        }
        break;

        case TYPESETUP:
        {
            if (value)
            {
                // Move to note view
                drawBlank();
                currentview = VIEWNOTE;
            }
        }
        break;
    }
}