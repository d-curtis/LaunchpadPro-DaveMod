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

// store ADC frame pointer
static const u16 *g_ADC = 0;

// buffer to store pad states for flash save
#define BUTTON_COUNT 100
u8 g_Buttons[BUTTON_COUNT] = {0};

// Create an array to store the state of the square buttons. [0] is index, [1..3] is colour (rgb), [4] is MIDI note
u8 SQUARE_BUTTONS[64][4];           // Actual state of the board, to redraw
u8 SQUARE_BUTTONS_MODEL[64][4];     // What we should return the board to if necessary


typedef struct button button;
struct button {
       u8 index;       //  Physical HAL index (1..98)
       u8 midi;        //  MIDI number
       u8 colour[3];   //  Colour to display when idle
    _Bool type;     //  0: Note     1: CC
       u8 active;   // 0: idle  1: Active
    _Bool highlight;    // 0: No    1: Yes      ----> This should probably be a part of "active" as we can only draw one colour at a time. Might expand that from _Bool to u8?
};

button board_buttons[BUTTON_COUNT];


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



//______________________________________________________________________________

void app_surface_event(u8 type, u8 index, u8 value)
{
    switch (type)
    {
        case  TYPEPAD:
        {
            if (board_buttons[index].type == 0) {   // Note
                if (value)
                {
                    // Send the MIDI data
                    hal_send_midi(  USBMIDI, NOTEON,
                                    board_buttons[index].midi + (trans_octave * 12) + trans_semi,
                                    value   );
                    // Set the pressed LED
                    board_buttons[index].active = 1;
                    // Also set any LEDs of the same note
                    for (u8 i = 0; i < BUTTON_COUNT; ++i)
                    {
                        u8 maxOctaves = 8;
                        for (u8 j = 0; j < maxOctaves; ++j)             
                        {
                            if (board_buttons[i].midi == board_buttons[index].midi)
                            {
                                board_buttons[i].active = 1;
                                break;
                            }
                            else if (board_buttons[i].midi == j*12+10)   // Same note, any octave
                            {
                                board_buttons[i].active = 2;
                            }
                        }
                    }
                } else {
                    // Same as above, but in reverse
                    hal_send_midi(  USBMIDI, NOTEOFF, 
                                    board_buttons[index].midi + (trans_octave * 12) + trans_semi,
                                    value   );
                    board_buttons[index].active = 0;
                    for (u8 i = 0; i < BUTTON_COUNT; ++i)
                    {
                        u8 maxOctaves = 8;
                        for (u8 j = 0; j < maxOctaves; ++j)             
                        {
                            if (board_buttons[i].midi == board_buttons[index].midi)
                            {
                                board_buttons[i].active = 0;
                                break;
                            }
                            else if (board_buttons[i].midi == j*12+10)   // Same note, any octave
                            {
                                board_buttons[i].active = 0;
                            }
                        }
                    }



                    // for (u8 i = 0; i < BUTTON_COUNT; ++i)
                    // {
                    //     if (board_buttons[i].midi == board_buttons[index].midi)
                    //     {
                    //         board_buttons[i].active = 0;
                    //     }
                    // }
                }
            }
        }
        break;
            
        case TYPESETUP:
        {
            if (value)
            {
                u8 debugArray[11] = {
                //  Header          Manufacturer    Model           Sending         Address         ???
                    0xf0,           0x00,           0x20,           0x12,           0x00,           0x10,
                //  My data 0       My data 1       My data 2       My data 3                       End
                    0xde,           0xad,           0xbe,           0xef,                           0xf7
                };
                hal_send_sysex(USBMIDI, debugArray, 11);
            }
        }
        break;
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
    if (type == MIDI_IN_CABLE)
    {
        hal_plot_led(TYPESETUP, 0, 0, value, 0); // green
    }
    else if (type == MIDI_OUT_CABLE)
    {
        hal_plot_led(TYPESETUP, 0, value, 0, 0); // red
    }
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
        for (int i = 0; i < BUTTON_COUNT; ++i)
        {
            if (board_buttons[i].active == 1)
            {
                hal_plot_led(TYPEPAD, board_buttons[i].index, 0xff, 0x00, 0x00);
            }
            else if (board_buttons[i].active == 2)
            {
                hal_plot_led(TYPEPAD, board_buttons[i].index, 0xff, 0xff, 0x00);
            }
            else
            {
                hal_plot_led(TYPEPAD, board_buttons[i].index, board_buttons[i].colour[0], board_buttons[i].colour[1], board_buttons[i].colour[2]);
            }
        }
    }
}

//______________________________________________________________________________

void app_init(const u16 *adc_raw)
{
    flashtoggle = 0;





    // Populate SQUARE_BUTTONS with 11-88, skipping circle button IDs
    u8 fillIndex = 0;
    for (u8 i = 1; i < 9; ++i)
    {
        for (u8 j = 1; j < 9; ++j)
        {
            SQUARE_BUTTONS[fillIndex][0] = i*10+j;
            SQUARE_BUTTONS[fillIndex][1] = 0x00;    SQUARE_BUTTONS[fillIndex][2] = 0x00;    SQUARE_BUTTONS[fillIndex][3] = 0x00;
            ++fillIndex;
        }
    }

    buttons_init();







    // Copy SQUARE_BUTTONS into a model so we can refer back to what the "blank" screen should look like
    for (u8 i = 0; i < 64; ++i)
    {
        SQUARE_BUTTONS_MODEL[i][0] = SQUARE_BUTTONS[i][0];
        SQUARE_BUTTONS_MODEL[i][1] = SQUARE_BUTTONS[i][1];
        SQUARE_BUTTONS_MODEL[i][2] = SQUARE_BUTTONS[i][2];
        SQUARE_BUTTONS_MODEL[i][3] = SQUARE_BUTTONS[i][3];
    }
	
	// store off the raw ADC frame pointer for later use
	g_ADC = adc_raw;
}


void drawScale()
{
    // Apply the scale highlighting to our pad LEDs
    u8 octaves[6] = {
        0, 15, 18, 36, 54, 57
    };

    u8 minor[24];
    for (u8 i = 0; i < 6; ++i)
    {
        minor[i] = octaves[i] + 1;   // minor second
        minor[6+i] = octaves[i] + 3; // minor third
    };

    u8 major[24];
    for (u8 i = 0; i < 6; ++i)
    {
        major[i] = octaves[i] + 2;   // major second
        major[6+i] = octaves[i] + 4; // minor third
    };


    for (u8 i = 0; i < 6; ++i)
    {
        // OCTAVES
        SQUARE_BUTTONS[octaves[i]][1] = 0xf0;           SQUARE_BUTTONS[octaves[i]][2] = 0xf0;           SQUARE_BUTTONS[octaves[i]][3] = 0xf0;
        // MINOR SECONDS
        SQUARE_BUTTONS[minor[i]][1] = 0x20;           SQUARE_BUTTONS[minor[i]][2] = 0x20;           SQUARE_BUTTONS[minor[i]][3] = 0xa0;
        // MAJOR SECONDS
        SQUARE_BUTTONS[major[i]][1] = 0xa0;           SQUARE_BUTTONS[major[i]][2] = 0x20;           SQUARE_BUTTONS[major[i]][3] = 0x20;
        // MINOR THIRDS
        SQUARE_BUTTONS[minor[6+i]][1] = 0x00;           SQUARE_BUTTONS[minor[6+i]][2] = 0x00;           SQUARE_BUTTONS[minor[6+i]][3] = 0xa0;
        // MAJOR THIRDS
        SQUARE_BUTTONS[major[i]][1] = 0xa0;           SQUARE_BUTTONS[major[i]][2] = 0x00;           SQUARE_BUTTONS[major[i]][3] = 0x00;



    }
}


void buttons_init() {
    // Zero the array
    for (int i = 0; i < BUTTON_COUNT; ++i)
    {
        board_buttons[i].index      = i;
        board_buttons[i].midi       = 0;
        board_buttons[i].active     = 0;
        board_buttons[i].colour[0]  = 0x00;
        board_buttons[i].colour[1]  = 0x00;
        board_buttons[i].colour[2]  = 0x00;
    }

    //  Set note flags
    for (int i = 1; i < 9; ++i)
    {
        for (int j = 1; j < 9; ++j)
        {
            board_buttons[i*10+j].type = 0;
        }
    }

    //  Set CC flags
    for (int i = 1; i < 9; ++i)
    {
        board_buttons[10*i+9].type = 1;
    }

    //  Set the MIDI notes so each row goes up in fourths cause I'm a lazy asshat guitarist
    u8 row_offset = 0;
    for (int i = 1; i < 9; ++i)
    {
        for (int j = 1; j < 9; ++j)
        {
            board_buttons[i*10+j].midi = (i*10+j) - row_offset + 24 + 1; // Cheeky +1 because we want the bottom corner to start as 12 (MIDI C), not 11 (HAL ID)
        }
        row_offset += 5;
    }

    //  Set some pretty colours
    for (u8 i = 0; i < BUTTON_COUNT; ++i)
    {
        // if (board_buttons[i].type != 0) { break; }      // Escape non-note buttons. I'll do something with these later...

        u8 maxOctaves = 8;
        for (u8 j = 0; j < maxOctaves; ++j)             // Gnarly if-elseif tree incoming...
        {
            if (board_buttons[i].midi == j*12+0)        // C    (Root)
            {
                board_buttons[i].colour[0] = 0xff;
                board_buttons[i].colour[1] = 0xff;
                board_buttons[i].colour[2] = 0xff;
            }
            else if (board_buttons[i].midi == j*12+1)   // C#   (Minor 2nd)
            {
                board_buttons[i].colour[0] = 0xa0;
                board_buttons[i].colour[1] = 0xa0;
                board_buttons[i].colour[2] = 0xff;
            }
            else if (board_buttons[i].midi == j*12+2)   // D    (Major 2nd)
            {
                board_buttons[i].colour[0] = 0xff;
                board_buttons[i].colour[1] = 0xa0;
                board_buttons[i].colour[2] = 0xa0;
            }
            else if (board_buttons[i].midi == j*12+3)   // D#   (Minor 3rd)
            {
                board_buttons[i].colour[0] = 0xa0;
                board_buttons[i].colour[1] = 0xa0;
                board_buttons[i].colour[2] = 0xff;
            }
            else if (board_buttons[i].midi == j*12+4)   // E    (Major 3rd)
            {
                board_buttons[i].colour[0] = 0xff;
                board_buttons[i].colour[1] = 0xa0;
                board_buttons[i].colour[2] = 0xa0;
            }
            else if (board_buttons[i].midi == j*12+5)   // F    (Perfect 4th)
            {
                board_buttons[i].colour[0] = 0x90;
                board_buttons[i].colour[1] = 0xff;
                board_buttons[i].colour[2] = 0x90;
            }
            else if (board_buttons[i].midi == j*12+6)   // F#   (Diminished 5th)
            {
                board_buttons[i].colour[0] = 0xff;
                board_buttons[i].colour[1] = 0x50;
                board_buttons[i].colour[2] = 0xff;
            }
            else if (board_buttons[i].midi == j*12+7)   // G    (Perfect 5th)
            {
                board_buttons[i].colour[0] = 0x50;
                board_buttons[i].colour[1] = 0xff;
                board_buttons[i].colour[2] = 0x50;
            }
            else if (board_buttons[i].midi == j*12+8)   // G#   (Minor 6th)
            {
                board_buttons[i].colour[0] = 0xa0;
                board_buttons[i].colour[1] = 0xa0;
                board_buttons[i].colour[2] = 0xff;
            }
            else if (board_buttons[i].midi == j*12+9)   // A    (Major 6th)
            {
                board_buttons[i].colour[0] = 0xff;
                board_buttons[i].colour[1] = 0xa0;
                board_buttons[i].colour[2] = 0xa0;
            }
            else if (board_buttons[i].midi == j*12+10)   // A#   (Minor 7th)
            {
                board_buttons[i].colour[0] = 0xa0;
                board_buttons[i].colour[1] = 0xa0;
                board_buttons[i].colour[2] = 0xff;
            }
            else if (board_buttons[i].midi == j*12+11)   // B    (Major 7th)
            {
                board_buttons[i].colour[0] = 0xff;
                board_buttons[i].colour[1] = 0xa0;
                board_buttons[i].colour[2] = 0xa0;
            }
        }
    }
}