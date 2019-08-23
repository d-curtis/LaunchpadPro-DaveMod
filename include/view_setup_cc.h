#ifndef VIEW_SETUP_CC_H

#include "app.h"

#define CANCEL_NONE     0
#define CANCEL_CCS      1
#define CANCEL_TENS     2
#define CANCEL_UNITS    3

u8 ccsetup_tens_indices[13];
u8 ccsetup_units_indices[10];
u8 ccsetup_selected;
u8 ccsetup_activetens;
u8 ccsetup_activeunits;
u8 ccbutton_indices[8];
u8 ccbutton_indices_length;

extern button_setup board_buttons_setup[BUTTON_COUNT];
u8 board_buttons_setup_size;






void setup_cc_redrawBoard();
void setup_cc_setActiveNumberDisplay(button_setup cc);
void setup_cc_surfaceEvent(u8 type, u8 index, u8 value);










#define VIEW_SETUP_CC_H
#endif