/******************************************************************************
 
 Copyright (c) 2015, Focusrite Audio Engineering Ltd.
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 * Neither the name of Focusrite Audio Engineering Ltd., nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 *****************************************************************************/

//______________________________________________________________________________
//
// Headers
//______________________________________________________________________________

#include "app.h"
#include "font.h"

// store ADC frame pointer
static const u16 *g_ADC = 0;

// buffer to store pad states for flash save
#define BUTTON_COUNT 100
u8 g_Buttons[BUTTON_COUNT] = {0};


// Allllll the surface information is going to live in here.
#define TYPE_NOTE   0
#define TYPE_CC     1
#define CANCEL_NONE     0
#define CANCEL_CCS      1
#define CANCEL_TENS     2
#define CANCEL_UNITS    3




// Create one for each view so that the buttons can do different stuff.
button_note board_buttons_note[BUTTON_COUNT];
button_setup board_buttons_setup[BUTTON_COUNT];
u8 board_buttons_note_size      = sizeof(board_buttons_note)    / sizeof(button_note);
u8 board_buttons_setup_size     = sizeof(board_buttons_setup)   / sizeof(button_setup);


// Transpose offsets
signed char trans_octave = 0;
signed char trans_semi  = 0;

// MIDI Nums
#define MIDI_C      12
#define MIDI_CSH    13
#define MIDI_D      14
#define MIDI_DSH    15
#define MIDI_E      16
#define MIDI_F      17
#define MIDI_FSH    18
#define MIDI_G      19
#define MIDI_GSH    20
#define MIDI_A      21
#define MIDI_ASH    22
#define MIDI_B      23

#define MIDI_CC_ON          127
#define MIDI_CC_OFF         0
#define MIDI_CC_SUSTAIN     64


// Currently active view (global)
#define VIEWNOTE    0
#define VIEWSETUP   1
// #define some others in future...?
u8 currentview = 0;





//      --              --              --              --              --
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




//______________________________________________________________________________

void intercancel(button_setup targets[], u8 length, u8 group, u8 cancelval)
{
    for (int i = 0; i < length; i++)
    {
        if (targets[i].active != cancelval && targets[i].cancelgroup == group)
        {
            targets[i].active = cancelval;
        }
    }
}


void app_surface_event(u8 type, u8 index, u8 value)
{
    switch (currentview)
    {
        // Main note view
        case VIEWNOTE:
        {
            switch (type)
            {
                case  TYPEPAD:
                {
                    if (board_buttons_note[index].type == 0) {   // Note
                        if (value)
                        {
                            // Send the MIDI data
                            hal_send_midi(  USBMIDI, NOTEON,
                                            board_buttons_note[index].midi + (trans_octave * 12) + trans_semi,
                                            value   );
                            // Set the pressed LED
                            board_buttons_note[index].active = 1;
                            // Also set any LEDs of the same note
                            for (u8 i = 0; i < BUTTON_COUNT; i++)
                            {
                                u8 maxOctaves = 8;
                                for (u8 j = 0; j < maxOctaves; j++)             
                                {
                                    if (board_buttons_note[i].midi == board_buttons_note[index].midi)
                                    {
                                        board_buttons_note[i].active = 1;
                                        break;
                                    }
                                    // else if (board_buttons_note[i].midi == j*12+10)   // Same note, any octave
                                    // {
                                    //     board_buttons_note[i].active = 2;
                                    // }
                                }
                            }
                        } else {
                            // Same as above, but in reverse
                            hal_send_midi(  USBMIDI, NOTEOFF, 
                                            board_buttons_note[index].midi + (trans_octave * 12) + trans_semi,
                                            value   );
                            board_buttons_note[index].active = 0;
                            for (u8 i = 0; i < BUTTON_COUNT; i++)
                            {
                                u8 maxOctaves = 8;
                                for (u8 j = 0; j < maxOctaves; j++)             
                                {
                                    if (board_buttons_note[i].midi == board_buttons_note[index].midi)
                                    {
                                        board_buttons_note[i].active = 0;
                                        break;
                                    }
                                    // else if (board_buttons_note[i].midi == j*12+10)   // Same note, any octave
                                    // {
                                    //     board_buttons_note[i].active = 0;
                                    // }
                                }
                            }
                        }
                    } else if (board_buttons_note[index].type == 1) {
                        hal_plot_led(TYPESETUP, 0, 0x00, 0xff, 0x00);
                        hal_send_midi(USBMIDI, CC, index, value);
                    }
                }
                break;
                    
                case TYPESETUP:
                {
                    if (value)
                    {   
                        //  Move to setup view
                        drawBlank();
                        currentview = VIEWSETUP;
                    }
                }
                break;
            }
        }
        break;

        // Setup view
        case VIEWSETUP:
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
                                        ccsetup_setActiveNumberDisplay(board_buttons_setup[i]);
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
    }
}

//______________________________________________________________________________

void app_midi_event(u8 port, u8 status, u8 d1, u8 d2)
{
    // Respond to incoming MIDI? Ehhh... Later.
}

//______________________________________________________________________________

void app_sysex_event(u8 port, u8 * data, u16 count)
{
    // Respond to UDI?
}

//______________________________________________________________________________

void app_aftertouch_event(u8 index, u8 value)
{
    // example - send poly aftertouch to MIDI ports
    // hal_send_midi(USBMIDI, POLYAFTERTOUCH | 0, index, value);
}

//______________________________________________________________________________

void app_cable_event(u8 type, u8 value)
{
    // Light the Setup LED to indicate cable connections
    // if (type == MIDI_IN_CABLE)
    // {
    //     hal_plot_led(TYPESETUP, 0, 0, value, 0); // green
    // }
    // else if (type == MIDI_OUT_CABLE)
    // {
    //     hal_plot_led(TYPESETUP, 0, value, 0, 0); // red
    // }
}

//______________________________________________________________________________

_Bool flashtoggle;

void app_timer_event() // Gets called on 1kHz clock
{
    static u16 flashms = 0;
    if (++flashms >= 500)
    {
        flashms = 0;
        if (flashtoggle)
        {
            hal_plot_led(TYPEPAD, 98, 0x00, 0xff, 0x00);
            flashtoggle = 0;
        } else {
            hal_plot_led(TYPEPAD, 98, 0x00, 0x10, 0x00);
            flashtoggle = 1;
        }
    }

    static u8 redrawms = 0;
    if (++redrawms >= 50)   // 20fps? -- If we run this at the 1kHz clock we get LED flicker
    {
        redrawms = 0;
        redrawView();
    }
}

void drawBlank()
{
    // For when we need to clear the board
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        hal_plot_led(TYPEPAD, i, 0x00, 0x00, 0x00);
        hal_plot_led(TYPESETUP, 0, 0x00, 0x00, 0x00);
    }
}

void ccsetup_setActiveNumberDisplay(button_setup cc)
{
    u8 ccnum = cc.value;
    u8 ccu  = ccnum %10;
    u8 cct  = (ccnum/10)%10;
    u8 cch  = (ccnum/100)%10;
    cct     = cct+(10*cch);     // 'cause I can have two-digit tens in my system. Take that, maths.

    //DEBUG
    hal_send_midi(USBMIDI, NOTEON, 100, cc.value);
    hal_send_midi(USBMIDI, NOTEOFF, 10, cct);
    hal_send_midi(USBMIDI, NOTEOFF, 1, ccu);


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

void redrawView()
{
    // Moved stuff from the above interrupt to keep it more "timer" related and less "drawing" related.
    // Maybe split out into different views if we end up adding more? For just Note & Setup maybe it's not too messy.
    switch (currentview)
    {
        case VIEWNOTE:
        {
            for (int i = 0; i < BUTTON_COUNT; i++)
            {
                if (board_buttons_note[i].active == 1)
                {
                    hal_plot_led(TYPEPAD, board_buttons_note[i].index, 0xff, 0x00, 0x00);
                }
                else if (board_buttons_note[i].active == 2)
                {
                    hal_plot_led(TYPEPAD, board_buttons_note[i].index, 0xff, 0xff, 0x00);
                }
                else
                {
                    hal_plot_led(TYPEPAD, board_buttons_note[i].index, board_buttons_note[i].colour[0], board_buttons_note[i].colour[1], board_buttons_note[i].colour[2]);
                }
            }
        }
        break;

        case VIEWSETUP:
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
        break;

        default:
        {
            for (int i = 0; i < BUTTON_COUNT; i++)
            {
                hal_plot_led(TYPEPAD, i, 0xff, 0x00, 0x00);
            }
        }
    }
}

//______________________________________________________________________________

void app_init(const u16 *adc_raw)
{
    flashtoggle = 0;
    buttons_init();

	
	// store off the raw ADC frame pointer for later use
	g_ADC = adc_raw;
}


void buttons_init() {
    // Zero the array
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        board_buttons_note[i].index      = i;
        board_buttons_note[i].type       = 0;
        board_buttons_note[i].midi       = 0;
        board_buttons_note[i].active     = 0;
        board_buttons_note[i].colour[0]  = 0x00;
        board_buttons_note[i].colour[1]  = 0x00;
        board_buttons_note[i].colour[2]  = 0x00;

        board_buttons_setup[i].index        = i;
        board_buttons_setup[i].value        = 0;
        board_buttons_setup[i].active       = 0;
        board_buttons_setup[i].colour[0]    = 0x00;
        board_buttons_setup[i].colour[1]    = 0x00;
        board_buttons_setup[i].colour[2]    = 0x00;
        board_buttons_setup[i].altcolour[0] = 0x00;
        board_buttons_setup[i].altcolour[1] = 0x00;
        board_buttons_setup[i].altcolour[2] = 0x00;
    }


    //  Set note flags on square buttons
    for (int i = 1; i < 9; i++)
    {
        for (int j = 1; j < 9; ++j)
        {
            board_buttons_note[i*10+j].type = 0;
        }
    }

    //  Set CC flags on circle buttons
    for (int i = 1; i < 9; i++)
    {
        board_buttons_note[10*i+9].type = 1;
    }

    //  Set the MIDI notes so each row goes up in fourths cause I'm a lazy guitarist
    u8 row_offset = 0;
    for (int i = 1; i < 9; i++)
    {
        for (int j = 1; j < 9; j++)
        {
            //  Cheeky +1 because we want the bottom corner to start as 12 (MIDI C), not 11 (HAL ID)
            board_buttons_note[i*10+j].midi = (i*10+j) - row_offset + 24 + 1; 
        }
        row_offset += 5;
    }

    //  Set some pretty colours for note mode
    for (u8 i = 0; i < BUTTON_COUNT; i++)
    {
        // if (board_buttons_note[i].type != 0) { break; }      // Escape non-note buttons. I'll do something with these later...

        u8 maxOctaves = 8;
        for (u8 j = 0; j < maxOctaves; ++j)             // Gnarly if-elseif tree incoming... This is ugly & definitely not the best way to do this.
        {
            if (board_buttons_note[i].midi == j*12+0)        // C    (Root)
            {
                board_buttons_note[i].colour[0] = 0xff;
                board_buttons_note[i].colour[1] = 0xff;
                board_buttons_note[i].colour[2] = 0xff;
            }
            else if (board_buttons_note[i].midi == j*12+1)   // C#   (Minor 2nd)
            {
                board_buttons_note[i].colour[0] = 0xa0;
                board_buttons_note[i].colour[1] = 0xa0;
                board_buttons_note[i].colour[2] = 0xff;
            }
            else if (board_buttons_note[i].midi == j*12+2)   // D    (Major 2nd)
            {
                board_buttons_note[i].colour[0] = 0xff;
                board_buttons_note[i].colour[1] = 0xa0;
                board_buttons_note[i].colour[2] = 0xa0;
            }
            else if (board_buttons_note[i].midi == j*12+3)   // D#   (Minor 3rd)
            {
                board_buttons_note[i].colour[0] = 0xa0;
                board_buttons_note[i].colour[1] = 0xa0;
                board_buttons_note[i].colour[2] = 0xff;
            }
            else if (board_buttons_note[i].midi == j*12+4)   // E    (Major 3rd)
            {
                board_buttons_note[i].colour[0] = 0xff;
                board_buttons_note[i].colour[1] = 0xa0;
                board_buttons_note[i].colour[2] = 0xa0;
            }
            else if (board_buttons_note[i].midi == j*12+5)   // F    (Perfect 4th)
            {
                board_buttons_note[i].colour[0] = 0x90;
                board_buttons_note[i].colour[1] = 0xff;
                board_buttons_note[i].colour[2] = 0x90;
            }
            else if (board_buttons_note[i].midi == j*12+6)   // F#   (Diminished 5th)
            {
                board_buttons_note[i].colour[0] = 0xff;
                board_buttons_note[i].colour[1] = 0x50;
                board_buttons_note[i].colour[2] = 0xff;
            }
            else if (board_buttons_note[i].midi == j*12+7)   // G    (Perfect 5th)
            {
                board_buttons_note[i].colour[0] = 0x50;
                board_buttons_note[i].colour[1] = 0xff;
                board_buttons_note[i].colour[2] = 0x50;
            }
            else if (board_buttons_note[i].midi == j*12+8)   // G#   (Minor 6th)
            {
                board_buttons_note[i].colour[0] = 0xa0;
                board_buttons_note[i].colour[1] = 0xa0;
                board_buttons_note[i].colour[2] = 0xff;
            }
            else if (board_buttons_note[i].midi == j*12+9)   // A    (Major 6th)
            {
                board_buttons_note[i].colour[0] = 0xff;
                board_buttons_note[i].colour[1] = 0xa0;
                board_buttons_note[i].colour[2] = 0xa0;
            }
            else if (board_buttons_note[i].midi == j*12+10)   // A#   (Minor 7th)
            {
                board_buttons_note[i].colour[0] = 0xa0;
                board_buttons_note[i].colour[1] = 0xa0;
                board_buttons_note[i].colour[2] = 0xff;
            }
            else if (board_buttons_note[i].midi == j*12+11)   // B    (Major 7th)
            {
                board_buttons_note[i].colour[0] = 0xff;
                board_buttons_note[i].colour[1] = 0xa0;
                board_buttons_note[i].colour[2] = 0xa0;
            }
        }
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
                board_buttons_setup[i].cancelgroup  = CANCEL_TENS;
                board_buttons_setup[i].active       = 1;
            }
            if (board_buttons_setup[i].value == 0 && board_buttons_setup[i].cancelgroup == CANCEL_TENS)   // Default CC 00
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
                board_buttons_setup[i].cancelgroup  = CANCEL_UNITS;
                board_buttons_setup[i].active       = 1;
            }
            if (board_buttons_setup[i].value == 0 && board_buttons_setup[i].cancelgroup == CANCEL_UNITS)   // Default CC 00
            {
                board_buttons_setup[i].active = 2;
            }
        }
        for (u8 j = 0; j < sizeof(ccbutton_indices) / sizeof(u8); j++)
        {
            if (board_buttons_setup[i].index == ccbutton_indices[j])
            {
                board_buttons_setup[i].value        = 122;
                board_buttons_setup[i].active       = 0;
                board_buttons_setup[i].flashing     = 0;
                board_buttons_setup[i].colour[0]    = 0x10;
                board_buttons_setup[i].colour[1]    = 0x00;
                board_buttons_setup[i].colour[2]    = 0x00;
                board_buttons_setup[i].altcolour[0] = 0xff;
                board_buttons_setup[i].altcolour[1] = 0x00;
                board_buttons_setup[i].altcolour[2] = 0x00;
                board_buttons_setup[i].cancelgroup  = CANCEL_CCS;
            }
        }
    }
}