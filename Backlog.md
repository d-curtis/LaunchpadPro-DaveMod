# Intro

This file is where I plan to sketch out ideas for features.
These are unlikely to be complete specs, but rather serve as a brain dump of ideas - depending on how bothered I am at the time.

-----

**CC Config**
Note; MVP is latching 0-127. Consider configuring latch/momentary and value to send on the fly. Perhaps a kind of inverse MIDI learn?
- [ ] I want to enter a setup mode by pressing the setup button so that I can configure my CCs on the fly
- [ ] I want to select one of the circle "play" buttons on the right hand side when I am in the setup mode so that I can then apply a CC value to it
- [ ] I want the LEDs under square buttons 11-18, 21-24 to light blue, so that I know these buttons can be interacted with to set the CC number "tens"
- [ ] I want the LEDs under square buttons 31-38, 41-42 to light purple, so that I know these buttons can be interacted with to set the CC number "units"
- [ ] I want the button selection to intercancel within (tens) or (units), so that it's clear what is being entered
- [ ] I want the default selection to be 11, 31 (CC #00)
- [ ] I want my CC selection to save to flash when deselected so that I don't have to configure CCs every reboot
- [ ] I want the square button LEDs to update when selecting a new circle button so I am always viewing the value for the selected button.


**Note Velocity Scaling**
- [ ] I want a log scale offset applied to the ADC value so that playing the FSRs feels more expressive than directly piping ADC -> Velocity



